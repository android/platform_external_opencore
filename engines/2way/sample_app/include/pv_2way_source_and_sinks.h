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
#ifndef PV_2WAY_SOURCE_AND_SINKS_H_INCLUDED
#define PV_2WAY_SOURCE_AND_SINKS_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
// This file includes the class definition for all MIOs (sources and sinks)
// for the Win 32 GUI application
//
// This class initializes the MIOs with the appropriate values (see Initialize
//  functions).
//
//
// This class also adds video rendering on top of PV2WaySourceAndSinksBase.
//
////////////////////////////////////////////////////////////////////////////
#include "pv_2way_source_and_sinks_base.h"

#ifdef USE_LOOPBACK_COMMS_MIO
#include "pvmi_mio_comm_loopback_factory.h"
#include "pv_comms_io_node_factory.h"
#else
#include "pvmf_loopback_node.h"
#endif
#include "pvlogger.h"
#include "pv_2way_modem.h"
#include "pv_2way_wins_test_video_render.h"



//test function oscl_str_is_valid_utf8
class PV2WaySourceAndSinks :   public PV2WaySourceAndSinksBase,
        public PlayOnMediaObserver
{
    public:
        PV2WaySourceAndSinks(PV2WaySourceAndSinksObserver* aMIOObserver,
                             PV2Way324InitInfo& aSdkInitInfo);
        ~PV2WaySourceAndSinks();



        int GetEngineCmdState(PVCommandId aCmdId);

        void CloseSourceAndSinks()
        {
            iAudioSource->Closed();
            iVideoSource->Closed();
            iAudioSink->Closed();
            iVideoSink->Closed();
        }

        void OutputInfo(const char * str, ...)
        {
            va_list args;
            va_start(args, str);
            // output to screen
            vprintf(str, args);

            // output to log
            char buffer[256];
            vsprintf(buffer, str, args);
            if (!iLogger)
            {
                iLogger = PVLogger::GetLoggerObject("PV2WaySourceAndSinks");
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "---PV2WaySourceAndSinks:: %s", buffer));
            va_end(args);
        }

        void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);

        void TimerCallback();


        void ResetSourceSinkNodes();

        /**
         * Parse session status,
         * Available incoming and outgoing media channels and the channel ids
         * and capabilities associated with each
         * @param
         **/
        void ParseSessionParams();


        void CreateVideoRender(HWND aHandle);
        bool StopVideoRender();
        void ResetVideoRender();

        // From PlayOnMediaObserver
        virtual int32  PlayOnMedia(unsigned char*, uint32);
        // MIO notification callback that parameters have been received and are valid (true)
        // or invalid (false)
        virtual void MIOParametersValid(bool bValid, int32 iWidth, int32 iHeight);

    private:

        Oscl_FileServer iFileServer;
        PVVideoRender* iVideoRender;

        PVErrorEventObserver *iErrorEventObserver;

        PV2Way324ConnectOptions iConnectOptions;
        HWND iVideoRenderCtrl;

        OsclAny*    iFileParser;

        bool bVideoDisplay;

        PVLogger* iLogger;

};



#endif
