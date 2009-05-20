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
#include "pvmf_protocol_engine_shoutcast.h"

#define SHOUTCAST_STATUS_CODE_OK 200
#define SHOUTCAST_NUMBER_DIGITS_FOR_STATUS_CODE 3
#define SHOUTCAST_HTTP_CHAR_CR 13
#define SHOUTCAST_HTTP_CHAR_LF 10
#define SHOUTCAST_INSERTED_HTTP_RESPONSE_LINE_OK "HTTP/1.0 200 OK"
#define SHOUTCAST_INSERTED_EXTRA_RESPONSE_LINE   "Response-Line: "



bool ShoutcastStreamingState_GET::setHeaderFields()
{
    // first add Icy-Metadata:1\r\n
    StrCSumPtrLen fieldKey = "Icy-Metadata";
    StrPtrLen fieldValue   = "1";
    if (!iComposer->setField(fieldKey, &fieldValue)) return false;

    return ProgressiveStreamingState_GET::setHeaderFields();
}


// typedef Oscl_Vector<PVMFSharedMediaMsgPtr, OsclMemAllocator> INPUT_DATA_QUEUE;
int32 ShoutcastStreamingState_GET::processMicroStateGetResponse(INPUT_DATA_QUEUE &aDataQueue)
{

    INPUT_DATA_QUEUE aDataQueueBackup = aDataQueue;
    int32 status = iParser->parseResponse(aDataQueue);

    // check return status for ICY 200 OK response
    if (!(status < 0 && hasICYResponse())) return checkParsingStatus(status);


    // has ICY 200 OK response line, and create a specific PVMFSharedMediaMsgPtr inserted
    // before actual aDataQueue, and re-send to the parser

    // create output media data to be sent to socket node through port
    PVMFSharedMediaDataPtr mediaData;
    if (!createAndFillMediaData(mediaData)) return PROCESS_MEDIA_DATA_CREATE_FAILURE;

    PVMFSharedMediaMsgPtr mediaMsg;
    convertToPVMFMediaMsg(mediaMsg, mediaData);

    INPUT_DATA_QUEUE aMsgQueue;
    aMsgQueue.push_back(mediaMsg);
    for (uint32 i = 0; i < aDataQueueBackup.size(); i++) aMsgQueue.push_back(aDataQueueBackup[i]);

    // re-do the parsing
    iParser->reset();
    do
    {
        status = iParser->parseResponse(aMsgQueue);
        if (iParser->isHttpHeaderParsed())
        {
            getMediaDataLength();
            getContenBitrate();
        }

    }
    while (status == HttpParsingBasicObject::PARSE_NEED_MORE_DATA && !aMsgQueue.empty());

    return checkParsingStatus(status);
}


bool ShoutcastStreamingState_GET::hasICYResponse()
{
    // get first response line
    StrCSumPtrLen responseLine = "Response-Line";
    StrPtrLen responseLineVal;
    if (!iParser->getHttpParser()->getField(responseLine, responseLineVal)) return false;

    // Got the response line and check ICY 200 OK response
    char *ptr = (char *)responseLineVal.c_str();
    uint32 len = responseLineVal.size();

    // get to the first letter
    while (!oscl_isLetter(*ptr) && len > 0)
    {
        ptr++;
        len--;
    }

    // check ICY
    while (!((oscl_tolower(ptr[0]) == 'i') &&
             (oscl_tolower(ptr[1]) == 'c') &&
             (oscl_tolower(ptr[2]) == 'y')) &&
            len >= 3)
    {
        ptr++;
        len--;
    }

    if (len < SHOUTCAST_NUMBER_DIGITS_FOR_STATUS_CODE) return false;
    ptr += 3;
    len -= 3;

    while (!PE_isDigit(*ptr) && len > 0)
    {
        ptr++;
        len--;
    }
    char *start_ptr = ptr;
    uint32 start_len = len;
    while (PE_isDigit(*ptr) && len > 0)
    {
        ptr++;
        len--;
    }
    uint32 aStatusCode = 0;
    PV_atoi(start_ptr, 'd', start_len - len, aStatusCode);
    LOGINFODATAPATH((0, "ShoutcastStreamingState_GET::checkICYResponse, aStatusCode = %d", aStatusCode));

    return (aStatusCode == SHOUTCAST_STATUS_CODE_OK);
}

bool ShoutcastStreamingState_GET::createAndFillMediaData(PVMFSharedMediaDataPtr &aMediaData)
{
    if (!iObserver->GetBufferForRequest(aMediaData)) return false;

    OsclRefCounterMemFrag fragIn;
    aMediaData->getMediaFragment(0, fragIn);

    char *ptr = (char*)fragIn.getMemFragPtr();
    StrPtrLen str1 = SHOUTCAST_INSERTED_HTTP_RESPONSE_LINE_OK;
    StrPtrLen str2 = SHOUTCAST_INSERTED_EXTRA_RESPONSE_LINE;
    oscl_memcpy(ptr, str1.c_str(), str1.length());
    ptr += str1.length();
    *ptr++ = SHOUTCAST_HTTP_CHAR_CR;
    *ptr++ = SHOUTCAST_HTTP_CHAR_LF;
    oscl_memcpy(ptr, str2.c_str(), str2.length());
    aMediaData->setMediaFragFilledLen(0, str1.length() + str2.length() + 2); // don't count NULL
    return true;
}

uint32 ShoutcastStreamingState_GET::getMediaDataLength()
{
    if (iMediaDataLength > 0) return iMediaDataLength;

    // get metadata interval (aka. media data length)
    StrCSumPtrLen mediaDataLengthKey = "icy-metaint";
    StrPtrLen mediaDataLengthValue;
    if (!iParser->getHttpParser()->getField(mediaDataLengthKey, mediaDataLengthValue)) return 0;

    // get playback time
    iMediaDataLength = getIntegerItemFromString((char *)mediaDataLengthValue.c_str(), mediaDataLengthValue.size());
    LOGINFODATAPATH((0, "ShoutcastStreamingState_GET::getMediaDataLength, iMediaDataLength = %d", iMediaDataLength));
    return iMediaDataLength;
}

uint32 ShoutcastStreamingState_GET::getContenBitrate()
{
    if (iContenBitrate > 0) return iContenBitrate;

    // get metadata interval (aka. media data length)
    StrCSumPtrLen serverRateKey = "icy-br";
    StrPtrLen serverRateValue;
    if (!iParser->getHttpParser()->getField(serverRateKey, serverRateValue)) return 0;

    // get playback time
    iContenBitrate = getIntegerItemFromString((char *)serverRateValue.c_str(), serverRateValue.size());
    iContenBitrate *= 1000;
    LOGINFODATAPATH((0, "ShoutcastStreamingState_GET::getContenBitrate, iContenBitrate = %d", iContenBitrate));
    return iContenBitrate;
}

uint32 ShoutcastStreamingState_GET::getIntegerItemFromString(char *aPtr, const uint32 aLen)
{
    char *ptr = aPtr;
    uint32 len = aLen;

    while (!PE_isDigit(*ptr) && len > 0)
    {
        ptr++;
        len--;
    }
    char *startPtr = ptr;
    uint32 startLen = len;
    while (PE_isDigit(*ptr) && len > 0)
    {
        ptr++;
        len--;
    }
    uint32 value = 0;
    PV_atoi(startPtr, 'd', startLen - len, value);
    return value;
}

int32 ShoutcastStreamingState_GET::OutputDataAvailable(OUTPUT_DATA_QUEUE *aOutputQueue, const bool isHttpHeader)
{
    if (isHttpHeader)
    {
        iDataSideInfo.set(ProtocolEngineOutputDataType_HttpHeader);
        iObserver->OutputDataAvailable(*aOutputQueue, iDataSideInfo);
    }
    else    // output data to data stream object
    {
        iDataSideInfo.set(ProtocolEngineOutputDataType_NormalData);
        iObserver->OutputDataAvailable(*aOutputQueue, iDataSideInfo);
    }
    return HttpParsingBasicObject::PARSE_SUCCESS;
}

