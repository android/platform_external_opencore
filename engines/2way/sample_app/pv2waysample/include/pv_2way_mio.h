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
#ifndef PV_2WAY_MIO_H_INCLUDED
#define PV_2WAY_MIO_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
// This file includes the class definition for the base class for an MIO
//
// This class includes pure virtual functions to create and delete the MIO
// that need to be overriden in derived classes (such as the AudioSink, AudioSource,
// VideoSink, VideoSource classes).
//
// This class also defines common MIO behavior: Adding/Removing an MIO, handling
// events with respect to the MIO, etc.
//
// With this design the only code that needs to be in the derived classes is how
// to create and delete the specific MIO.
//
////////////////////////////////////////////////////////////////////////////

#include "pv_engine_observer.h"
#include "pv_2way_interface.h"

#include "pvmi_media_io_fileoutput.h"

#ifdef PERF_TEST
#include "pvmf_fileinput_settings.h"
#endif
#include "pv_2way_mio_node_factory.h"
#include "pv_2way_codecspecifier.h"

#define TITLE_LENGTH   300
#define FILENAME_LEN   255
#define OUTPUT_TXT_LEN   20
#define EXTN_LEN       5
#define PORT_LEN 1200
#define LOOP_LEN 500
#define TRANS_LEN 500

#define OUTPUT_MIO_SIZE 300

typedef enum
{
    EPVFile,
    EPVDevice,
    EPVNone,
    EPVEndSourceSinkType
} PV2WaySourceSinkType;


TPVChannelId GetIdFromLocalBuffer(PVAsyncInformationalEvent &aEvent);


TPVDirection GetDir(int32 dir);
PV2WayMediaType GetMediaType(int32 mediaType);


class PV2WayMIO
{
    public:
        OSCL_IMPORT_REF PV2WayMIO(Oscl_Vector<CodecSpecifier*, OsclMemAllocator>* aFormats);
        virtual OSCL_IMPORT_REF ~PV2WayMIO();

        OSCL_IMPORT_REF PVCommandId Add();
        OSCL_IMPORT_REF PVCommandId Remove();
        OSCL_IMPORT_REF void PrintFormatTypes();


        OSCL_IMPORT_REF void Init()
        {
            iNextChannelId = 0;
            iChannelId = 0;
        }

        OSCL_IMPORT_REF void SetTerminal(CPV2WayInterface *aTerminal)
        {
            iTerminal = aTerminal;
        }

        OSCL_IMPORT_REF PVCommandId HandleEvent(const PVAsyncInformationalEvent& aEvent);
        OSCL_IMPORT_REF void HandleClosingTrack();
        OSCL_IMPORT_REF void HandleCloseTrack();

        OSCL_IMPORT_REF void ResetCompleted();

        OSCL_IMPORT_REF bool EstablishedMIO()
        {
            if (iChannelId != 0)
                return true;
            return false;

        }

        OSCL_IMPORT_REF void SetPreviewHandle(uint32 aPreviewHandle)
        {
            iPreviewHandle = aPreviewHandle;
        }
        virtual OSCL_IMPORT_REF int CreateMioNodeFactory(CodecSpecifier* aSelectedCodec) = 0;
        virtual OSCL_IMPORT_REF int Create();
        virtual OSCL_IMPORT_REF void Delete();


        OSCL_IMPORT_REF void AddCompleted(const PVCmdResponse& aResponse);
        OSCL_IMPORT_REF void RemoveCompleted(const PVCmdResponse& aResponse);

        bool IsAddId(PVCommandId aAddId)
        {
            return (iAddId == aAddId);
        }
        bool IsRemoveId(PVCommandId aRemoveId)
        {
            return (iRemoveId == aRemoveId);
        }
        bool IsPauseId(PVCommandId aPauseId)
        {
            return (iPauseId == aPauseId);
        }
        bool IsResumeId(PVCommandId aResumeId)
        {
            return (iResumeId == aResumeId);
        }

        bool IsChannelId(TPVChannelId aChannelId)
        {
            return (iChannelId == aChannelId);
        }

        bool IsNextChannelId(TPVChannelId aNextChannelId)
        {
            return (iNextChannelId == aNextChannelId);
        }


        bool IsRemoved()
        {
            return (iMioNode == NULL);
        }
        bool IsAdded()
        {
            return (iAdded);
        }

        OSCL_IMPORT_REF void Closed();
        virtual OSCL_IMPORT_REF int AddCodec(PVMFFormatType aFormat);
        virtual OSCL_IMPORT_REF int AddCodec(PvmiMIOFileInputSettings& aFileSettings) = 0;
        virtual OSCL_IMPORT_REF int AddCodec(PVMFFileInputSettings& aFileSettings) = 0;

        OSCL_IMPORT_REF void ClearCodecs()
        {
            iFormats->clear();
        }

        OSCL_IMPORT_REF int AddFormat(PVMFFormatType aFormat);
        OSCL_IMPORT_REF int AddFormat(PvmiMIOFileInputSettings& format);
        OSCL_IMPORT_REF int AddFormat(PVMFFileInputSettings& format);
    protected:
        CodecSpecifier* FormatInList(const char* aFormatToFind);
        CodecSpecifier* FormatInList(PVMFFormatType& aType);


        CodecSpecifier* FormatMatchesCapabilities(const PVAsyncInformationalEvent& aEvent);

        void ParseResponse(const PVAsyncInformationalEvent& aEvent,
                           PVMFFormatType& aMimeString,
                           int& aMedia_type);

        void OutputInfo(const char * str, ...)
        {
            va_list args;
            va_start(args, str);
            // output to screen everything in formatted string
            vprintf(str, args);

            // output to logger
            char buffer[256];
            vsprintf(buffer, str, args);
            if (!iLogger)
            {
                iLogger = PVLogger::GetLoggerObject("PV2WaySourceAndSinks.PV2WayMIO");
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "---PV2WayMIO:: %s", buffer));
            va_end(args);
        }


        bool iAdded;
        bool iRemoving;
        bool iClosing;

        Oscl_Vector<CodecSpecifier*, OsclMemAllocator>* iFormats;


        PVMFNodeInterface*      iMioNode;
        PV2WayMIONodeFactory*   iMioNodeFactory;

        PVCommandId iAddId;
        PVCommandId iRemoveId;
        PVCommandId iPauseId;
        PVCommandId iResumeId;

        TPVChannelId iChannelId;
        TPVChannelId iNextChannelId;

        PV2WayMediaType iMyType;
        TPVDirection iMyDir;


        uint32              iPreviewHandle;
        CPV2WayInterface*   iTerminal;
        PVLogger*           iLogger;
};



#endif
