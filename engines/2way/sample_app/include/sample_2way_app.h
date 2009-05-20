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
#ifndef SAMPLE_2WAY_APP_H_INCLUDED
#define SAMPLE_2WAY_APP_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_ERROR_H_INCLUDED
#include "oscl_error.h"
#endif
#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif
#ifndef OSCL_CONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVLOGGER_STDERR_APPENDER_H_INCLUDED
#include "pvlogger_stderr_appender.h"
#endif
#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif
#ifndef PVLOGGER_TIME_AND_ID_LAYOUT_H_INCLUDED
#include "pvlogger_time_and_id_layout.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif
#ifndef PV_ENGINE_OBSERVER_H_INCLUDED
#include "pv_engine_observer.h"
#endif
#ifndef PVMI_MIO_CONTROL_H_INCLUDED
#include "pvmi_mio_control.h"
#endif
#ifndef PV_2WAY_INTERFACE_H_INCLUDED
#include "pv_2way_interface.h"
#endif
#ifndef PV_2WAY_ENGINE_FACTORY_H_INCLUDED
#include "pv_2way_engine_factory.h"
#endif
#ifndef PVMI_MIO_COMM_LOOPBACK_FACTORY_H_INCLUDED
#include "pvmi_mio_comm_loopback_factory.h"
#endif
#ifndef PVMI_MEDIA_IO_FILEOUTPUT_H_INCLUDED
#include "pvmi_media_io_fileoutput.h"
#endif
#ifndef NO_2WAY_324
#include "pv_comms_io_node_factory.h"
#include "pvmi_mio_comm_loopback_factory.h"
#endif

// for PVLogger
template<class DestructClass>
class LogAppenderDestructDealloc : public OsclDestructDealloc
{
    public:
        virtual void destruct_and_dealloc(OsclAny *ptr)
        {
            OSCL_DELETE((DestructClass*)ptr);
        }
};

class pv2way_engine_interface : public OsclTimerObject,
        public PVCommandStatusObserver,
        public PVInformationalEventObserver,
        public PVErrorEventObserver
{
    public:
        pv2way_engine_interface();
        ~pv2way_engine_interface();

        // needed for OsclTimerObject
        void Run();
        void CommandCompleted(const PVCmdResponse& aResponse);
        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        enum PV2wayState
        {
            STATE_UNKNOWN = 0,
            STATE_CREATE,
            STATE_INIT,
            STATE_CONNECT,
            STATE_ADDSOURCE_SINK,
            STATE_30SECONDS,
            STATE_DISCONNECT,
            STATE_RESET,
            STATE_CLEANUPANDCOMPLETE
        };

        PV2wayState iState;

        CPV2WayInterface* iTerminal;
        PVMFNodeInterface* iAudioSrcNode;
        PVMFNodeInterface* iVideoSrcNode;

        PVRefFileOutput* iVideoSinkIOControl;
        PVMFNodeInterface* iVideoSinkNode;
        PVRefFileOutput* iAudioSinkIOControl;
        PVMFNodeInterface* iAudioSinkNode;
        OSCL_wHeapString<OsclMemAllocator> iVideoSinkFileName;
        OSCL_wHeapString<OsclMemAllocator> iAudioSinkFileName;

        PV2Way324InitInfo iInitInfo;
        PV2Way324ConnectOptions iConnectOptions;

        PVMFNodeInterface* iCommServer;
        PvmiMIOControl* iCommServerIOControl;
        PvmiMIOCommLoopbackSettings iCommSettings;
        CPV2WaySessionParams sessionParams;

        PVCommandId iCurrentCmdId;
        PVCommandId iAddAudioSrcCmdId;
        PVCommandId iAddVideoSrcCmdId;
        PVCommandId iAddAudioSinkCmdId;
        PVCommandId iAddVideoSinkCmdId;

        void Start2way();

        void RemoveAudioSrc();
        void RemoveVideoSrc();
        void RemoveAudioSink();
        void RemoveVideoSink();
        void Cleanup();

    private:
        //for context data testing
        uint32 iContextObjectRefValue;
        uint32 iContextObject;

        // audio and video data sources
        PvmiMIOControl *audiomio;
        PvmiMIOControl *videomio;

        // audio and video file input settings
        OsclAny* iVideoSettings;
        OsclAny* iAudioSettings;
        bool isConnected;
        bool videoDataSrcAdded;
        bool audioDataSrcAdded;
        bool audioSinkAdded;
        bool videoSinkAdded;
};

#endif
