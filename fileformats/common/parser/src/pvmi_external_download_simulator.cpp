/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
#ifndef PVMI_EXTERNAL_DOWNLOAD_SIMULATOR_H_INCLUDED
#include "pvmi_external_download_simulator.h"
#endif

#ifndef OSCL_DLL_H_INCLUDED
#include "oscl_dll.h"
#endif
// Define entry point for this DLL
OSCL_DLL_ENTRY_POINT_DEFAULT()


#define LOGDEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_DEBUG,m);

////////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF
PVMIExternalDownloadSimulator::PVMIExternalDownloadSimulator(PVMIExternalDownloadSimulatorParams& aParams)
        : OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVMIExternalDownloadSimulator")
{
    if ((aParams.iSizeUpdateIntervalInMS == 0) ||
            (aParams.iDownloadRateInKbps == 0) ||
            (aParams.iFileSizeInBytes == 0))
    {
        OSCL_ASSERT(false);
        OSCL_LEAVE(OsclErrArgument);
    }
    iExtensionRefCount = 0;
    iFileSizeInBytes = aParams.iFileSizeInBytes;
    iDownloadRateInKbps = aParams.iDownloadRateInKbps;
    iSizeUpdateIntervalInMS = aParams.iSizeUpdateIntervalInMS;
    iObserverVec.reserve(128);
    iDownloadedFileSizeInBytes = 0;
    iDownloadComplete = false;
    iLogger = PVLogger::GetLoggerObject("PVMIExternalDownloadSimulator");
    iDownloadClock.SetClockTimebase(iDownloadClockTimebase);
    AddToScheduler();
}

OSCL_EXPORT_REF
PVMIExternalDownloadSimulator::~PVMIExternalDownloadSimulator()
{
    if (iExtensionRefCount != 0)
    {
        OSCL_ASSERT(false);
    }
    Reset();
    if (IsAdded())
    {
        RemoveFromScheduler();
    }
};

OSCL_EXPORT_REF
void PVMIExternalDownloadSimulator::SetObserver(PVMIExternalDownloadSizeObserver* aObs)
{
    Oscl_Vector<PVMIExternalDownloadSizeObserver*, OsclMemAllocator>::iterator it;
    for (it = iObserverVec.begin(); it != iObserverVec.end(); it++)
    {
        LOGDEBUG((0, "PVMIExternalDownloadSimulator::SetObserver(before) - Obs=0x%x", (*it)));
    }
    iObserverVec.push_back(aObs);
    LOGDEBUG((0, "PVMIExternalDownloadSimulator::SetObserver - Obs=0x%x, NumObs=%d", aObs, iObserverVec.size()));
    for (it = iObserverVec.begin(); it != iObserverVec.end(); it++)
    {
        LOGDEBUG((0, "PVMIExternalDownloadSimulator::SetObserver(after) - Obs=0x%x", (*it)));
    }
}

OSCL_EXPORT_REF
void PVMIExternalDownloadSimulator::RemoveObserver(PVMIExternalDownloadSizeObserver* aObs)
{
    LOGDEBUG((0, "PVMIExternalDownloadSimulator::RemoveObserver - Obs=0x%x, NumObs(before)=%d", aObs, iObserverVec.size()));
    Oscl_Vector<PVMIExternalDownloadSizeObserver*, OsclMemAllocator>::iterator it;
    for (it = iObserverVec.begin(); it != iObserverVec.end(); it++)
    {
        LOGDEBUG((0, "PVMIExternalDownloadSimulator::RemoveObserver(before) - Obs=0x%x", (*it)));
    }
    it = iObserverVec.begin();
    while (it != iObserverVec.end())
    {
        if (*it == aObs)
        {
            iObserverVec.erase(it);
            it = iObserverVec.begin();
        }
        else
        {
            it++;
        }
    }
    for (it = iObserverVec.begin(); it != iObserverVec.end(); it++)
    {
        LOGDEBUG((0, "PVMIExternalDownloadSimulator::RemoveObserver(after) - Obs=0x%x", (*it)));
    }
    LOGDEBUG((0, "PVMIExternalDownloadSimulator::RemoveObserver - Obs=0x%x, NumObs(after)=%d", aObs, iObserverVec.size()));
}

void PVMIExternalDownloadSimulator::addRef()
{
    ++iExtensionRefCount;
}


void PVMIExternalDownloadSimulator::removeRef()
{
    --iExtensionRefCount;
}

OSCL_EXPORT_REF
bool PVMIExternalDownloadSimulator::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    iface = NULL;
    if (uuid == PVMIExternalDownloadSizeUpdateInterfaceUUID)
    {
        PVMIExternalDownloadSizeUpdateInterface* myInterface = OSCL_STATIC_CAST(PVMIExternalDownloadSizeUpdateInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        addRef();
    }
    else
    {
        return false;
    }
    return true;
}

OSCL_EXPORT_REF
void PVMIExternalDownloadSimulator::Start()
{
    iDownloadClock.Start();
    RunIfNotReady(iSizeUpdateIntervalInMS*1000);
    LOGDEBUG((0, "PVMIExternalDownloadSimulator::Start - SizeUpdateInterval=%d", iSizeUpdateIntervalInMS));
}

OSCL_EXPORT_REF
void PVMIExternalDownloadSimulator::Stop()
{
    iDownloadClock.Stop();
    Cancel();
    LOGDEBUG((0, "PVMIExternalDownloadSimulator::Stop"));
}

OSCL_EXPORT_REF
void PVMIExternalDownloadSimulator::Reset()
{
    iObserverVec.clear();
    iDownloadClock.Stop();
    Cancel();
    iDownloadedFileSizeInBytes = 0;
    iDownloadComplete = false;
    LOGDEBUG((0, "PVMIExternalDownloadSimulator::Reset"));
}

void PVMIExternalDownloadSimulator::Run()
{
    //Read the download clock (in milliseconds)
    uint32 timebase32 = 0;
    uint32 downloadClockInMS = 0;
    bool overflowFlag = false;
    iDownloadClock.GetCurrentTime32(downloadClockInMS, overflowFlag, PVMF_MEDIA_CLOCK_MSEC, timebase32);
    //Update iDownloadedFileSizeInBytes based on time elapsed and iDownloadRateInKbps
    iDownloadedFileSizeInBytes = ((downloadClockInMS) * (iDownloadRateInKbps)) / 8;
    if (iDownloadedFileSizeInBytes < iFileSizeInBytes)
    {
        //Reschedule AO
        RunIfNotReady(iSizeUpdateIntervalInMS*1000);
    }
    else
    {
        //download complete - no need to reschedule
        iDownloadedFileSizeInBytes = iFileSizeInBytes;
        iDownloadComplete = true;
    }
    Oscl_Vector<PVMIExternalDownloadSizeObserver*, OsclMemAllocator>::iterator it;
    for (it = iObserverVec.begin(); it != iObserverVec.end(); it++)
    {
        (*it)->DownloadUpdate(iDownloadedFileSizeInBytes, iDownloadComplete);
    }
    LOGDEBUG((0, "PVMIExternalDownloadSimulator::Run - DownloadSize=%d, DownloadClockInMS=%d, DnldRateInKbps=%d, FileSize=%d ",
              iDownloadedFileSizeInBytes, downloadClockInMS, iDownloadRateInKbps, iFileSizeInBytes));
}

