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
#ifndef PV_2WAY_VIDEO_SINK_H_INCLUDED
#define PV_2WAY_VIDEO_SINK_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
// This file includes the class definition for the Video Sink MIO
//
// This class includes functions to create and delete the MIO.
//
////////////////////////////////////////////////////////////////////////////

#include "pv_2way_sink.h"
#include "pv_2way_codecspecifier.h"
#ifdef PV2WAY_WIN32_GUI
#include "pvmi_media_io_video.h"
#endif

class PV2WayVideoSink : public PV2WaySink
{
    public:
        PV2WayVideoSink(Oscl_Vector<CodecSpecifier*, OsclMemAllocator>* aFormats) :
                PV2WaySink(aFormats)
        {
            iMyType = PV_VIDEO;
        };
        virtual ~PV2WayVideoSink()
        {
            Delete();
        };


#ifdef PV2WAY_WIN32_GUI
        void SetPlayOnMediaObserver(PlayOnMediaObserver* ob)
        {
            iPlayOnMediaObserver = ob;
        }
        PlayOnMediaObserver* iPlayOnMediaObserver;
#endif

    protected:

        virtual int CreateMioNodeFactory(CodecSpecifier* aSelectedCodec);

        virtual bool CorrectEvent(const PVAsyncInformationalEvent& aEvent)
        {
            OSCL_UNUSED_ARG(aEvent);
            return true;
        };

#ifdef PV2WAY_WIN32_GUI
        // void CreateIOControl(); the IOControl is created outside this scope and then
        // set in here.
        void DeleteIOControl();

        RefVideoIO* iIOControl;
#endif

#ifdef PERF_TEST
        void DeletePerfTest();

        int CreatePerfTest(PV2WaySourceSinkType aType,
                           PVMFFileInputSettings* aFileSettings);
#endif

};



#endif

