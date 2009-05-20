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
#include "pvmi_external_download_file_monitor.h"

#ifndef   PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef   OSCL_EXCLUSIVE_PTR_H_INCLUDED
#include "oscl_exclusive_ptr.h"
#endif
#ifndef   OSCL_TICKCOUNT_H_INCLUDED
#include "oscl_tickcount.h"
#endif


OSCL_EXPORT_REF PVMIExternalDownloadFileMonitor::PVMIExternalDownloadFileMonitor()
        : OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVMIExternalDownloadFileMonitor")
{
    iDownloadComplete = false;
    iBytesDownloaded = 0;
    iState = STATE_IDLE;
    iLastChange = 0;
    iTimeOutms = 0;
}


OSCL_EXPORT_REF PVMIExternalDownloadFileMonitor::~PVMIExternalDownloadFileMonitor()
{
    if (!iDownloadComplete)
    {
        iDownloadComplete = true;
        //Notify all observers.
        for (uint32 i = 0; i < iObservers.size(); i++)
        {
            iObservers[i]->DownloadUpdate(iBytesDownloaded, iDownloadComplete);
        }
    }
}


OSCL_EXPORT_REF void PVMIExternalDownloadFileMonitor::MonitorFile
(
    const char*  aFileName,
    uint32       aTimeOutms
)
{
    if (iState == STATE_MONITORING)
    {
        //Already monitoring a file.
        OSCL_LEAVE(OsclErrGeneral);
    }

    iDownloadComplete = false;
    iBytesDownloaded  = 0;
    iState = STATE_OPENING;
    iLastChange = OsclTickCount::TicksToMsec(OsclTickCount::TickCount());
    iTimeOutms = aTimeOutms;
    iFileServer.Connect();
    iFileName = aFileName;
    //Run immediately.
    AddToScheduler();
    RunIfNotReady();
}


OSCL_EXPORT_REF void PVMIExternalDownloadFileMonitor::StopMonitoring(void)
{
    if (iState != STATE_MONITORING)
    {
        //Calling StopMonitoring when not monitoring a file is an error.
        OSCL_LEAVE(OsclErrGeneral);
    }
    iState = STATE_IDLE;
    iFile.Close();
    iFileServer.Close();
    RemoveFromScheduler();
}



OSCL_EXPORT_REF void   PVMIExternalDownloadFileMonitor::SetObserver
(
    PVMIExternalDownloadSizeObserver* aObs
)
{
    iObservers.push_back(aObs);
}



OSCL_EXPORT_REF void   PVMIExternalDownloadFileMonitor::RemoveObserver
(
    PVMIExternalDownloadSizeObserver* aObs
)
{
    Oscl_Vector<PVMIExternalDownloadSizeObserver*, OsclMemAllocator>::iterator i;
    for (i = iObservers.begin(); i != iObservers.end(); i++)
    {
        if (*i == aObs)
        {
            iObservers.erase(i);
            break;
        }
    }
}


OSCL_EXPORT_REF uint32 PVMIExternalDownloadFileMonitor::GetDownloadedFileSize()
{
    return iBytesDownloaded;
}


OSCL_EXPORT_REF bool   PVMIExternalDownloadFileMonitor::IsDownloadComplete()
{
    return iDownloadComplete;
}


OSCL_EXPORT_REF void   PVMIExternalDownloadFileMonitor::addRef()
{
}


OSCL_EXPORT_REF void   PVMIExternalDownloadFileMonitor::removeRef()
{
}


OSCL_EXPORT_REF bool   PVMIExternalDownloadFileMonitor::queryInterface
(
    const PVUuid& uuid,
    PVInterface*& iface
)
{
    if (uuid == PVMIExternalDownloadSizeUpdateInterfaceUUID)
    {
        iface = this;
        return true;
    }
    return false;
}


void PVMIExternalDownloadFileMonitor::Run(void)
{
    switch (iState)
    {
        case STATE_IDLE:
            break;

        case STATE_OPENING:
        {
            //Disable PVCache because it prevents file size updates.
            iFile.SetPVCacheSize(0);
            int32 status = iFile.Open
                           (
                               iFileName.get_str(),
                               Oscl_File::MODE_READ | Oscl_File::MODE_BINARY,
                               iFileServer
                           );
            if (0 == status) //success
            {
                iState = STATE_MONITORING;
                Poll();
            }
            //else, if there was an error when trying to open the file (such as
            //when the file doesn't exist yet) we do not proceed to the
            //monitoring state, so we will retry opening in a few milliseconds.
        }
        break;

        case STATE_MONITORING:
            Poll();
            break;
    }

    uint32 now = OsclTickCount::TicksToMsec(OsclTickCount::TickCount());
    if ((now - iLastChange) >= iTimeOutms)
    {
        //Notify all observers.
        iDownloadComplete = true;
        for (uint32 i = 0; i < iObservers.size(); i++)
        {
            iObservers[i]->DownloadUpdate(iBytesDownloaded, iDownloadComplete);
        }
        StopMonitoring();
        return;
    }

    //Run again after PVMI_EDLFM_POLL_INTERVAL_MS * 1000 microseconds.
    RunIfNotReady(PVMI_EDLFM_POLL_INTERVAL_MS * 1000);
}


void PVMIExternalDownloadFileMonitor::Poll(void)
{
    //Flush the read handle so Symbian sees the size change.
    iFile.Flush();
    iFile.Seek(0, Oscl_File::SEEKEND);
    uint32 newSize = iFile.Tell();
    if (newSize != iBytesDownloaded)
    {
        iBytesDownloaded = newSize;
        iLastChange = OsclTickCount::TicksToMsec(OsclTickCount::TickCount());

        //Notify all observers of the file size change.
        for (uint32 i = 0; i < iObservers.size(); i++)
        {
            iObservers[i]->DownloadUpdate(iBytesDownloaded, iDownloadComplete);
        }
    }
}

