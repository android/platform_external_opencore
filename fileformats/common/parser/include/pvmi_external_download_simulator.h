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
#define PVMI_EXTERNAL_DOWNLOAD_SIMULATOR_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif
#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif
#ifndef PVMI_EXTERNAL_DOWNLOAD_EXTENSION_INTERFACES_H_INCLUDED
#include "pvmi_external_download_extension_interfaces.h"
#endif

class PVMIExternalDownloadSimulatorParams
{
    public:
        PVMIExternalDownloadSimulatorParams()
        {
            iFileSizeInBytes = 0;
            iDownloadRateInKbps = 0;
            iSizeUpdateIntervalInMS = 0;
        }

        PVMIExternalDownloadSimulatorParams(const PVMIExternalDownloadSimulatorParams& a)
        {
            MyCopy(a);
        }

        virtual ~PVMIExternalDownloadSimulatorParams() {};

        OSCL_IMPORT_REF PVMIExternalDownloadSimulatorParams& operator=(const PVMIExternalDownloadSimulatorParams& a)
        {
            if (&a != this)
            {
                MyCopy(a);
            }
            return *this;
        }

        uint32 iFileSizeInBytes;
        uint32 iDownloadRateInKbps;
        uint32 iSizeUpdateIntervalInMS;

    private:
        void MyCopy(const PVMIExternalDownloadSimulatorParams& a)
        {
            iFileSizeInBytes = a.iFileSizeInBytes;
            iDownloadRateInKbps = a.iDownloadRateInKbps;
            iSizeUpdateIntervalInMS = a.iSizeUpdateIntervalInMS;
        }
};

/*
 * This class simulates a growing file. This class requires
 * the complete file size, download rate at which the
 * file is expected to grow and the intervals at which
 * download size is to be updated. This class implements
 * a very simple algo. It all starts with "Start" API
 * In this API, this class starts a clock. As this clock
 * ticks, download is assumed to progress at the specified
 * rate in an uniform fashion. We schedule PVMIExternalDownloadSimulator
 * AO based on aSizeUpdateIntervalInMS, to update iDownloadedFileSize.
 */
class PVMIExternalDownloadSimulator : public OsclTimerObject,
        public PVMIExternalDownloadSizeUpdateInterface
{
    public:
        OSCL_IMPORT_REF PVMIExternalDownloadSimulator(PVMIExternalDownloadSimulatorParams& aParams);
        OSCL_IMPORT_REF virtual ~PVMIExternalDownloadSimulator();

        OSCL_IMPORT_REF void Start();
        OSCL_IMPORT_REF void Stop();
        OSCL_IMPORT_REF void Reset();

        /* From PVMIExternalDownloadSizeUpdateInterface */
        OSCL_IMPORT_REF void SetObserver(PVMIExternalDownloadSizeObserver* aObs);
        OSCL_IMPORT_REF void RemoveObserver(PVMIExternalDownloadSizeObserver* aObs);
        uint32 GetDownloadedFileSize()
        {
            return iDownloadedFileSizeInBytes;
        }
        bool IsDownloadComplete()
        {
            return iDownloadComplete;
        }

        /* From PVInterface */
        void addRef();
        void removeRef();
        OSCL_IMPORT_REF bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

    private:
        /* from OsclTimerObject */
        void Run();

        int32 iExtensionRefCount;

        uint32 iFileSizeInBytes;
        uint32 iDownloadRateInKbps;
        uint32 iSizeUpdateIntervalInMS;
        Oscl_Vector<PVMIExternalDownloadSizeObserver*, OsclMemAllocator> iObserverVec;

        uint32 iDownloadedFileSizeInBytes;
        bool iDownloadComplete;
        PVMFMediaClock iDownloadClock;
        PVMFTimebase_Tickcount iDownloadClockTimebase;

        PVLogger* iLogger;
};

#endif //PVMI_EXTERNAL_DOWNLOAD_SIMULATOR_H_INCLUDED


