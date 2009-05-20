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
#include "pvmf_protocol_engine_node.h"
#include "pvmf_protocol_engine_node_shoutcast.h"
#include "pvmf_protocol_engine_shoutcast.h"
#include "pvmf_protocolengine_node_tunables.h"


#define DEFAULT_PROTOCOL_INFO_VECTOR_RESERVE_NUMBER 4


////////////////////////////////////////////////////////////////////////////////////
//////  ShoutcastContainer implementation
////////////////////////////////////////////////////////////////////////////////////
ShoutcastContainer::ShoutcastContainer(PVMFProtocolEngineNode *aNode) :
        ProgressiveStreamingContainer(aNode)
{
    ;
}

bool ShoutcastContainer::createProtocolObjects()
{
    if (!ProgressiveStreamingContainer::createProtocolObjects()) return false;

    // re-create iProtocol
    if (iProtocol) OSCL_DELETE(iProtocol);
    iProtocol = OSCL_NEW(ShoutcastStreaming, ());
    if (iProtocol == NULL) return false;

    // re-create iDownloadControl
    if (iDownloadControl) OSCL_DELETE(iDownloadControl);
    iDownloadControl = OSCL_NEW(ShoutcastControl, ());
    if (iDownloadControl == NULL) return false;

    DownloadContainer::setEventReporterSupportObjects();
    return true;
}

PVMFStatus ShoutcastContainer::doStop()
{
    PVMFStatus status = DownloadContainer::doStop();
    if (status != PVMFSuccess) return status;
    // For shoutcast streaming, tell the data stream to flush,
    // so that the socket buffer can be returned to socket node for reset
    iNodeOutput->flushDataStream();

    // set resume download mode for stop and play
    OsclSharedPtr<PVDlCfgFile> aCfgFile = iCfgFileContainer->getCfgFile();
    aCfgFile->SetNewSession(true); // don't set resume download session for the next time
    aCfgFile->SetCurrentFileSize(0); // always start reset file size to 0

    // reset data stream write pointer to 0
    iNodeOutput->seekDataStream(0);

    return PVMFSuccess;
}


////////////////////////////////////////////////////////////////////////////////////
//////  ShoutcastControl implementation
////////////////////////////////////////////////////////////////////////////////////
ShoutcastControl::ShoutcastControl() : progressiveStreamingControl(), iSetProtocolInfo(false)
{
    iInfoKvpVec.reserve(DEFAULT_PROTOCOL_INFO_VECTOR_RESERVE_NUMBER);
}

void ShoutcastControl::setProtocolInfo()
{
    if (iSetProtocolInfo) return;
    if (!iProgDownloadSI || !iProtocol) return;

    // get all protocol info
    uint32 aMediaDataLength = iProtocol->getMediaDataLength();
    if (aMediaDataLength == 0) return;
    uint32 aClipBitrate = iProtocol->getContenBitrate();
    iInfoKvpVec.clear();

    // media data length
    PvmiKvp mediaDataLengthKVP;
    OSCL_StackString<256> mediaDataLengthKVPString = _STRLIT_CHAR("x-pvmf/net/shoutcast-media-data-length;valtype=uint32");
    mediaDataLengthKVP.key = mediaDataLengthKVPString.get_str();
    mediaDataLengthKVP.value.uint32_value = aMediaDataLength;
    iInfoKvpVec.push_back(&mediaDataLengthKVP);

    // server rate
    PvmiKvp clipBitrateKVP;
    OSCL_StackString<256> clipBitrateKVPString = _STRLIT_CHAR("x-pvmf/net/shoutcast-clip-bitrate;valtype=uint32");
    clipBitrateKVP.key = clipBitrateKVPString.get_str();
    clipBitrateKVP.value.uint32_value = aClipBitrate;
    iInfoKvpVec.push_back(&clipBitrateKVP);
    iPlaybackByteRate = aClipBitrate >> 3;

    // is shoutcast session
    PvmiKvp shoutcastSessionKVP;
    OSCL_StackString<256> shoutcastSessionKVPString = _STRLIT_CHAR("x-pvmf/net/is-shoutcast-session;valtype=bool");
    shoutcastSessionKVP.key = shoutcastSessionKVPString.get_str();
    shoutcastSessionKVP.value.bool_value = true;
    iInfoKvpVec.push_back(&shoutcastSessionKVP);

    iProgDownloadSI->setProtocolInfo(iInfoKvpVec);
    iSetProtocolInfo = true;
}


void ShoutcastControl::requestResumeNotification(const uint32 currentNPTReadPosition, bool& aDownloadComplete, bool& aNeedSendUnderflowEvent)
{
    uint32 aPlaybackByteRateOrig = iPlaybackByteRate;
    progressiveStreamingControl::requestResumeNotification(currentNPTReadPosition, aDownloadComplete, aNeedSendUnderflowEvent);

    // keep iPlaybackByteRate unchanged in case of shoutcast, i.e. using playback rate from shoutcast response header
    if (aPlaybackByteRateOrig != 0) iPlaybackByteRate = aPlaybackByteRateOrig;
}
