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
#ifndef PV_2WAY_SOURCE_AND_SINKS_BASE_H_INCLUDED
#define PV_2WAY_SOURCE_AND_SINKS_BASE_H_INCLUDED

////////////////////////////////////////////////////////////////////////////
// This file includes the base class for all MIOs (sources and sinks) within
// the application
//
// At this point it consists of one audio sink, one audio source, one video
// sink, one video source.  As all of these use the same base class it could
// be easily modified, however.
//
////////////////////////////////////////////////////////////////////////////

#include "pv_engine_observer.h"
#include "pv_2way_h324m_types.h"
#include "pv_2way_interface.h"
#include "pvmi_mio_control.h"
#include "pvmi_mio_fileinput.h"
#include "pvmi_media_io_fileoutput.h"

#include "pv_2way_mio.h"



#ifdef PERF_TEST
#include "pvmf_fileinput_settings.h"
#include "pvmf_dummy_fileinput_node_factory.h"
#include "pvmf_dummy_fileoutput_factory.h"
#endif
#include "oscl_mem.h"
#include "oscl_mem_audit.h"

#define RUN_IF_NOT_READY_TIME 200000


#define TITLE_LENGTH   300
#define FILENAME_LEN   255
#define OUTPUT_TXT_LEN   20
#define EXTN_LEN       5
#define PORT_LEN 1200
#define LOOP_LEN 500
#define TRANS_LEN 500

#define OUTPUT_MIO_SIZE 300


#ifdef PERF_TEST
#define AUDIO_SOURCE_FILENAME_FOR_PERF _STRLIT("data/amr_ietf_one_frame.amr")
#define VIDEO_SOURCE_FILENAME_FOR_PERF _STRLIT("data/hit040.h263")
#endif


class PV2WaySourceAndSinksObserver
{
    public:
        virtual void StartTimer() = 0;
};


class PV2WaySourceAndSinksBase :   public PVCommandStatusObserver
{
    public:
        OSCL_IMPORT_REF PV2WaySourceAndSinksBase(PV2WaySourceAndSinksObserver* aSourceSinksObserver,
                PV2Way324InitInfo& aSdkInitInfo);
        virtual OSCL_IMPORT_REF ~PV2WaySourceAndSinksBase();

        OSCL_IMPORT_REF void Init();

        OSCL_IMPORT_REF void Cleanup();
        OSCL_IMPORT_REF void ResetCompleted();
        OSCL_IMPORT_REF void HandleClosingTrack(const PVAsyncInformationalEvent& aEvent);
        OSCL_IMPORT_REF void HandleCloseTrack(const PVAsyncInformationalEvent& aEvent);

        OSCL_IMPORT_REF void CreateMIOs();
        OSCL_IMPORT_REF void CommandCompleted(const PVCmdResponse& aResponse);
        OSCL_IMPORT_REF void PrintFormatTypes();

        OSCL_IMPORT_REF void SetTerminal(CPV2WayInterface *aTerminal)
        {
            iTerminal = aTerminal;
            iAudioSource->SetTerminal(iTerminal);
            iAudioSink->SetTerminal(iTerminal);
            iVideoSource->SetTerminal(iTerminal);
            iVideoSink->SetTerminal(iTerminal);
        }

        OSCL_IMPORT_REF bool AllMIOsAdded()
        {
            if (iAudioSource->IsAdded() &&
                    iAudioSink->IsAdded() &&
                    iVideoSource->IsAdded() &&
                    iVideoSink->IsAdded())
                return true;
            return false;

        }
        OSCL_IMPORT_REF bool AllMIOsRemoved()
        {
            if (iAudioSource->IsRemoved() &&
                    iAudioSink->IsRemoved() &&
                    iVideoSource->IsRemoved() &&
                    iVideoSink->IsRemoved())
                return true;
            return false;
        }

        OSCL_IMPORT_REF int ResetPreferredCodec(TPVDirection aDir, PV2WayMediaType aMediaType);


        OSCL_IMPORT_REF int AddPreferredCodec(TPVDirection aDir,
                                              PV2WayMediaType aMediaType,
                                              PvmiMIOFileInputSettings& aFileSettings);
        OSCL_IMPORT_REF int AddPreferredCodec(TPVDirection aDir,
                                              PV2WayMediaType aMediaType,
                                              PVMFFormatType aFormat);
        OSCL_IMPORT_REF int AddPreferredCodec(TPVDirection aDir,
                                              PV2WayMediaType aMediaType,
                                              PVMFFileInputSettings& aFileSettings);

        OSCL_IMPORT_REF void ParseResponse(const PVAsyncInformationalEvent& aEvent,
                                           OSCL_HeapString<OsclMemAllocator>& aMimeString,
                                           int& aMedia_type);


        OSCL_IMPORT_REF void RemoveSourceAndSinks();

        OSCL_IMPORT_REF PVCommandId RemoveAudioSink()
        {
            return iAudioSink->Remove();
        }
        OSCL_IMPORT_REF PVCommandId RemoveAudioSource()
        {
            return iAudioSource->Remove();
        }
        OSCL_IMPORT_REF PVCommandId RemoveVideoSink()
        {
            return iVideoSink->Remove();
        }
        OSCL_IMPORT_REF PVCommandId RemoveVideoSource()
        {
            return iVideoSource->Remove();
        }
        OSCL_IMPORT_REF PVCommandId HandleIncomingVideo(const PVAsyncInformationalEvent& aEvent)
        {
            return iVideoSink->HandleEvent(aEvent);
        }
        OSCL_IMPORT_REF PVCommandId HandleIncomingAudio(const PVAsyncInformationalEvent& aEvent)
        {
            return iAudioSink->HandleEvent(aEvent);
        }
        OSCL_IMPORT_REF PVCommandId HandleOutgoingVideo(const PVAsyncInformationalEvent& aEvent)
        {
            return iVideoSource->HandleEvent(aEvent);
        }
        OSCL_IMPORT_REF PVCommandId HandleOutgoingAudio(const PVAsyncInformationalEvent& aEvent)
        {
            return iAudioSource->HandleEvent(aEvent);
        }

#ifdef PERF_TEST
        OSCL_IMPORT_REF int SetPerfFileSettings();
#endif

    protected:
        OSCL_IMPORT_REF PV2WayMIO* GetMIO(TPVDirection aDir,
                                          PV2WayMediaType aMediaType);

        virtual void OutputInfo(const char * str, ...) = 0;



        PV2WaySourceAndSinksObserver* iObserver;

        /*! @var iAudioStartRecId The command id for start recording incoming audio */
        PVCommandId iAudioStartRecId;
        /*! @var iAudioStopRecId The command id for stop recording of incoming audio */
        PVCommandId iAudioStopRecId;
        PVCommandId iVideoStartRecId;
        /*! @var iVideoStopRecId The command id for stop recording of incoming video */
        PVCommandId iVideoStopRecId;

        PV2Way324InitInfo& iSdkInitInfo;

        PV2WayMIO* iAudioSink;
        PV2WayMIO* iAudioSource;
        PV2WayMIO* iVideoSink;
        PV2WayMIO* iVideoSource;

        CPV2WayInterface *iTerminal;

};



#endif
