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
#ifndef   PVMI_EXTERNAL_DOWNLOAD_FILE_MONITOR_H_INCLUDED
#define   PVMI_EXTERNAL_DOWNLOAD_FILE_MONITOR_H_INCLUDED


#ifndef   OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef   OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef   OSCL_TYPES_H_UNDEFINED
#include "oscl_types.h"
#endif
#ifndef   OSCL_SCHEDULER_AO_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef   PVMI_EXTERNAL_DOWNLOAD_EXTENSION_INTERFACES_H_INCLUDED
#include "pvmi_external_download_extension_interfaces.h"
#endif


/*!
 * Compile-Time Tunable Parameters
 */
//! File opening and size polling interval in milliseconds.
#ifndef PVMI_EDLFM_POLL_INTERVAL_MS
#define PVMI_EDLFM_POLL_INTERVAL_MS 100
#endif
//! Default timeout before assuming the download is complete (in milliseconds).
#ifndef PVMI_EDLFM_DEFAULT_TIMEOUT_MS
#define PVMI_EDLFM_DEFAULT_TIMEOUT_MS 3000
#endif


/*!
 * @class PVMIExternalDownloadFileMonitor
 * @brief Implements the external download interface suitable for use by
 *        an external download data stream.
 */
class PVMIExternalDownloadFileMonitor
        : public PVMIExternalDownloadSizeUpdateInterface
        , public OsclTimerObject
{
    public:
        /*!
         * @brief Default constructor.
         */
        OSCL_IMPORT_REF  PVMIExternalDownloadFileMonitor();

        /*!
         * @brief Destructor notifies observers before closing.
         */
        OSCL_IMPORT_REF ~PVMIExternalDownloadFileMonitor();

        /*!
         * @brief     Method to begin monitoring a file.
         * @param[in] aFileName  Name of file to monitor.
         * @param[in] aTimeOutms Period of inactivity before assuming download is
         *                       complete.
         */
        OSCL_IMPORT_REF void MonitorFile
        (
            const char* aFileName,
            uint32      aTimeOutms = PVMI_EDLFM_DEFAULT_TIMEOUT_MS
        );

        /*!
         * @brief     Method to stop monitoring a file.
         */
        OSCL_IMPORT_REF void StopMonitoring(void);

        /*!
         * The following methods are from PVMIExternalDownloadSizeUpdateInterface.
         */

        /*!
         * @brief     Registers the given object as an observer.
         * @param[in] apObs Pointer to observer to add.
         */
        OSCL_IMPORT_REF virtual void   SetObserver(PVMIExternalDownloadSizeObserver* apObs);

        /*!
         * @brief     Removes the given observer.
         * @param[in] apObs Pointer to observer to remove.
         */
        OSCL_IMPORT_REF virtual void   RemoveObserver(PVMIExternalDownloadSizeObserver* apObs);

        /*!
         * @brief     Returns the total number of bytes downloaded so far.
         * @return    Current file size in bytes.
         */
        OSCL_IMPORT_REF virtual uint32 GetDownloadedFileSize();

        /*!
         * @brief     Predicate method that indicates if download is complete.
         *            Does not distinguish between failed, canceled, or successful downloads.
         * @return    True if downloading has ended.
         */
        OSCL_IMPORT_REF virtual bool   IsDownloadComplete();

        /*
         * from PVInterface
         */

        /*!
         * @brief Increments the reference counter.
         *        (Not used at this time)
         */
        OSCL_IMPORT_REF virtual void addRef();

        /*!
         * @brief Decrements the reference counter.
         *        (Not used at this time)
         */
        OSCL_IMPORT_REF virtual void removeRef();

        /*!
         * @brief            Returns this interface if the uuid matches.
         * @param[in]  uuid  Unique identification number of requested interface.
         * @param[out] iface Pointer to interface.  Only modified if uuid matches.
         * @return           True if the interface was found.
         */
        OSCL_IMPORT_REF virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        /*
         * from OsclActiveObject
         */
        virtual void Run(void);

    private:
        enum States
        {
            STATE_IDLE  = 0,
            STATE_OPENING,
            STATE_MONITORING
        };

        Oscl_Vector<PVMIExternalDownloadSizeObserver*, OsclMemAllocator> iObservers;
        bool            iDownloadComplete;
        uint32          iBytesDownloaded;
        States          iState;
        Oscl_File       iFile;
        Oscl_FileServer iFileServer;
        OSCL_HeapString<OsclMemAllocator> iFileName;
        uint32          iLastChange;
        uint32          iTimeOutms;

        /*!
         * @brief Internal method to polls the file for size changes.
         */
        void Poll(void);
};


#endif
