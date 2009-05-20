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
#ifndef PVMF_PROTOCOLENGINE_SHOUTCAST_H_INCLUDED
#define PVMF_PROTOCOLENGINE_SHOUTCAST_H_INCLUDED

#ifndef PVMF_PROTOCOLENGINE_PROGRESSIVE_DOWNLOAD_H_INCLUDED
#include "pvmf_protocol_engine_progressive_download.h"
#endif

// Progressive Streaming is a special form of progressive download
class ShoutcastStreamingState_GET : public ProgressiveStreamingState_GET
{
    public:

        void seek(const uint32 aSeekPosition)
        {
            OSCL_UNUSED_ARG(aSeekPosition);
        }

        uint32 getMediaDataLength();
        uint32 getContenBitrate();

        // constructor
        ShoutcastStreamingState_GET() : ProgressiveStreamingState_GET(), iMediaDataLength(0), iContenBitrate(0)
        {
            iRangeHeaderSupported = false;
        }

    private:
        bool setHeaderFields();
        int32 processMicroStateGetResponse(INPUT_DATA_QUEUE &aDataQueue);

        // supporting methods for processMicroStateGetResponse()
        bool hasICYResponse();
        bool createAndFillMediaData(PVMFSharedMediaDataPtr &aMediaData);

        // supporting method for getMetaDataInterval() and getServerRate()
        uint32 getIntegerItemFromString(char *aPtr, const uint32 aLen);
        void updateOutputDataQueue(OUTPUT_DATA_QUEUE *aOutputQueue)
        {
            OSCL_UNUSED_ARG(aOutputQueue);
        }
        int32 OutputDataAvailable(OUTPUT_DATA_QUEUE *aOutputQueue, const bool isHttpHeader);

    private:
        uint32 iMediaDataLength;
        uint32 iContenBitrate;
};

class ShoutcastStreaming : public ProgressiveStreaming
{
    public:
        ShoutcastStreaming(): ProgressiveStreaming()
        {

            OSCL_DELETE(iState[DEFAULT_STATE_NUMBER_FOR_DOWNLOAD_GET_REQUEST]);
            iState[DEFAULT_STATE_NUMBER_FOR_DOWNLOAD_GET_REQUEST] = OSCL_NEW(ShoutcastStreamingState_GET, ());
            iCurrStateNum = DEFAULT_STATE_NUMBER_FOR_DOWNLOAD_GET_REQUEST;
            iCurrState = iState[iCurrStateNum];
        }

        void seek(const uint32 aSeekPosition)
        {
            OSCL_UNUSED_ARG(aSeekPosition);
        }

    private:
        ProtocolState* getNextState()
        {
            // release the memory for the current state
            iCurrStateNum = DEFAULT_STATE_NUMBER_FOR_DOWNLOAD_GET_REQUEST;
            return iState[iCurrStateNum];
        }
};

#endif
