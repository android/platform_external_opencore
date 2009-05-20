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
#include "pv_2way_source_and_sinks_base.h"


#ifndef PV_2WAY_TRACK_INFO_IMPL_H_INCLUDED
#include "pv_2way_track_info_impl.h"
#endif

#include "pv_2way_mio.h"
#include "pv_mime_string_utils.h"


#define FORMAT_SIZE 100

TPVChannelId GetIdFromLocalBuffer(PVAsyncInformationalEvent &aEvent)
{
    TPVChannelId *channel = (TPVChannelId *) & aEvent.GetLocalBuffer()[4];
    return *channel;
}

TPVDirection GetDir(int dir)
{
    return (TPVDirection)dir;
}

PV2WayMediaType GetMediaType(int mediaType)
{
    return (PV2WayMediaType)mediaType;
}


OSCL_EXPORT_REF PV2WayMIO::PV2WayMIO(Oscl_Vector<CodecSpecifier* , OsclMemAllocator>* aFormats) :
        iAdded(false),
        iRemoving(false),
        iClosing(false),
        iFormats(aFormats),
        iMioNode(NULL),
        iMioNodeFactory(NULL),
        iAddId(-1),
        iRemoveId(-1),
        iPauseId(-1),
        iResumeId(-1),
        iChannelId(0),
        iNextChannelId(0),
        iPreviewHandle(0),
        iTerminal(NULL),
        iLogger(NULL)
{
};


OSCL_EXPORT_REF PV2WayMIO::~PV2WayMIO()
{
    uint32 numElems = iFormats->size();
    for (uint32 i = 0; i < numElems; i++)
    {
        CodecSpecifier* temp = iFormats->back();
        iFormats->pop_back();
        OSCL_DELETE(temp);
        temp = NULL;
    }
    Delete();
}

OSCL_EXPORT_REF void PV2WayMIO::Delete()
{
    iAdded = false;
    if (iMioNodeFactory)
    {
        iMioNodeFactory->Delete(&iMioNode);
        OSCL_DELETE(iMioNodeFactory);
        iMioNodeFactory = NULL;
    }
    iMioNode = NULL;
}


OSCL_EXPORT_REF int PV2WayMIO::Create()
{
    // creates MIO Node using the MioNodeFactory
    if (!iMioNodeFactory)
    {
        return PVMFFailure;
    }
    if (!iMioNode)
    {
        iMioNode = iMioNodeFactory->Create();
    }
    if (!iMioNode)
    {
        OutputInfo("PV2WayMIO::Create():: Error creating MIO Node Factory!");
        return PVMFFailure;
    }
    return PVMFSuccess;
}

OSCL_EXPORT_REF void PV2WayMIO::ResetCompleted()
{
    iAdded = false;
}

OSCL_EXPORT_REF void PV2WayMIO::AddCompleted(const PVCmdResponse& aResponse)
{
    if (aResponse.GetCmdStatus() == PVMFSuccess)
    {
        iAdded = true;
    }
    else
    {
        OutputInfo("PV2WayMIO::AddCompleted:: Failed to add MIO");
        Closed();
    }

    iAddId = -1;
}

OSCL_EXPORT_REF PVCommandId PV2WayMIO::Add()
{

    if (iMioNode == 0)
        return -1;
    int32 error = 0;
    if (!iAdded)
    {
        if (iMyDir == INCOMING)
        {
            OSCL_TRY(error, iAddId = iTerminal->AddDataSink(iChannelId,
                                     *iMioNode));
            if (error)
            {
                OutputInfo("PV2WayMIO::Add():: Error Adding Data Sink!");
            }
            return iAddId;
        }
        else
        {
            OSCL_TRY(error, iAddId = iTerminal->AddDataSource(iChannelId,
                                     *iMioNode));
            if (error)
            {
                OutputInfo("PV2WayMIO::Add():: Error Adding Data Source!");
            }
            return iAddId;
        }
    }
    else if (!iRemoving)
    {
        OutputInfo("\nError: MIO already added!\n");
    }
    else if (iRemoving)
    {
        OutputInfo("\nCannot add because attempting to remove MIO!\n");
    }
    return -1;
}

OSCL_EXPORT_REF PVCommandId PV2WayMIO::Remove()
{
    int32 error = 0;
    iRemoving = true;
    if (iMioNode && iRemoveId == -1)
    {
        if (iMyDir == INCOMING)
        {
            OSCL_TRY(error, iRemoveId = iTerminal->RemoveDataSink(*iMioNode));
            if (error)
            {
                OutputInfo("\n Error in RemoveDataSink!\n");
            }
            return iRemoveId;
        }
        else
        {
            OSCL_TRY(error, iRemoveId = iTerminal->RemoveDataSource(*iMioNode));
            if (error)
            {
                OutputInfo("\n Error in RemoveDataSource!\n");
            }
            return iRemoveId;
        }
    }
    else if (!iAdded)
    {
        OutputInfo("\nError: MIO cannot be removed because has not been added!\n");
    }
    return -1;
}

OSCL_EXPORT_REF void PV2WayMIO::HandleClosingTrack()
{
    iClosing = true;
}


OSCL_EXPORT_REF void PV2WayMIO::HandleCloseTrack()
{
    Closed();
}

OSCL_EXPORT_REF void PV2WayMIO::Closed()
{
    iClosing = false;
    iChannelId = 0;
    if (iNextChannelId)
    {
        iChannelId = iNextChannelId;
        iNextChannelId = 0;
        //iObserver->Initialize(iMyType, iMyDir);
        Create();
        Add();
    }
}

OSCL_EXPORT_REF void PV2WayMIO::RemoveCompleted(const PVCmdResponse& aResponse)
{
#ifndef PERF_TEST
    Delete();
#endif
    iRemoveId = -1;
    iRemoving = false;
}

OSCL_EXPORT_REF PVCommandId PV2WayMIO::HandleEvent(const PVAsyncInformationalEvent& aEvent)
{
    PVCommandId retvalue = -1;
    TPVChannelId id = GetIdFromLocalBuffer((PVAsyncInformationalEvent &) aEvent);
    if (PVT_INDICATION_INCOMING_TRACK == aEvent.GetEventType())
    {
        if (((PVAsyncInformationalEvent&)aEvent).GetLocalBuffer()[0] == PV_VIDEO)
        {
            OutputInfo("PVT_INDICATION_INCOMING_TRACK video, id %d\n", id);
        }
        else if (((PVAsyncInformationalEvent&)aEvent).GetLocalBuffer()[0] == PV_AUDIO)
        {
            OutputInfo("PVT_INDICATION_INCOMING_TRACK audio, id %d\n", id);
        }
    }
    else if (PVT_INDICATION_OUTGOING_TRACK == aEvent.GetEventType())
    {
        if (((PVAsyncInformationalEvent&)aEvent).GetLocalBuffer()[0] == PV_VIDEO)
        {
            OutputInfo("PVT_INDICATION_OUTGOING_TRACK video, id %d\n", id);
        }
        else if (((PVAsyncInformationalEvent&)aEvent).GetLocalBuffer()[0] == PV_AUDIO)
        {
            OutputInfo("PVT_INDICATION_OUTGOING_TRACK audio, id %d\n", id);
        }
    }
    if (iChannelId == id)
    {
        OutputInfo("\nDuplicate callback for  id %d\n", id);
        return -1;
    }

    if (iChannelId)
    {
        OutputInfo("\nBusy ... MIO id=%d being closed\n", iChannelId);
        iNextChannelId = id;
        return -1;
    }
    if (!iClosing && !iAdded)
    {
        iChannelId = id;
        // format matching capabilities and create- create can tell us which kind to create
        // including which file we should use, etc.
        CodecSpecifier* selectedCodec = FormatMatchesCapabilities(aEvent);
        if (selectedCodec)
        {
            CreateMioNodeFactory(selectedCodec);
            Create();
            retvalue = Add();
        }
    }
    return retvalue;
}

void PV2WayMIO::ParseResponse(const PVAsyncInformationalEvent& aEvent,
                              PVMFFormatType& aMimeString,
                              int& aMedia_type)
{

    PV2WayTrackInfoInterface* aTrackInfoInterface;
    aTrackInfoInterface = (PV2WayTrackInfoInterface *)aEvent.GetEventExtensionInterface();
    if (aTrackInfoInterface)
        aTrackInfoInterface->GetFormatString(aMimeString);
    aMedia_type = aEvent.GetLocalBuffer()[0];
}

OSCL_EXPORT_REF int PV2WayMIO::AddFormat(PvmiMIOFileInputSettings& format)
{
    CodecSpecifier* temp = OSCL_NEW(MIOFileCodecSpecifier, (format));
    iFormats->push_back(temp);
    return 0;
}

OSCL_EXPORT_REF int PV2WayMIO::AddFormat(PVMFFileInputSettings& format)
{
    CodecSpecifier* temp = OSCL_NEW(FileCodecSpecifier, (format));
    iFormats->push_back(temp);
    return 0;
}

OSCL_EXPORT_REF int PV2WayMIO::AddFormat(PVMFFormatType format)
{
    CodecSpecifier* temp = OSCL_NEW(CharCodecSpecifier, (format));
    iFormats->push_back(temp);
    return 0;
}

CodecSpecifier* PV2WayMIO::FormatInList(const char* formatToFind)
{
    for (uint32 i = 0; i < iFormats->size(); i++)
    {
        if (oscl_strcmp((*iFormats)[i]->GetFormat().getMIMEStrPtr(), formatToFind) == 0)
        {
            return (*iFormats)[i];
        }
    }
    return NULL;
}

OSCL_EXPORT_REF void PV2WayMIO::PrintFormatTypes()
{
    if (iFormats->size() == 0)
    {
        OutputInfo("No formats added.");
        return;
    }
    // loop through each, output values
    for (uint32 i = 0; i < iFormats->size(); i++)
    {
        OutputInfo((*iFormats)[i]->GetFormat().getMIMEStrPtr());
        OutputInfo(" ");
    }
}

CodecSpecifier* PV2WayMIO::FormatInList(PVMFFormatType& type)
{
    return FormatInList(type);
}

// this is FormatMatchesCapabilities for Audio
CodecSpecifier* PV2WayMIO::FormatMatchesCapabilities(const PVAsyncInformationalEvent& aEvent)
{
    PVMFFormatType aMimeString;
    int aMedia_Type = 0;
    ParseResponse(aEvent, aMimeString, aMedia_Type);
    // compare to what in iFormats instead.
    CodecSpecifier* formatInList = FormatInList(aMimeString.getMIMEStrPtr());
    if (!formatInList)
    {
        OutputInfo("Format %s does not match application capability\n", aMimeString.getMIMEStrPtr());
    }
    else
    {
        OutputInfo("Format %s matches application capabilities\n", aMimeString.getMIMEStrPtr());
    }

    return formatInList;
}

OSCL_EXPORT_REF int PV2WayMIO::AddCodec(PVMFFormatType aFormat)
{
    return AddFormat(aFormat);
}


