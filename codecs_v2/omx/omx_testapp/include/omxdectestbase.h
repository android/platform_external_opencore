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
/**
    @file OMX_testapp\include\omxdectestbase.h
*/

#ifndef OMXDECTESTBASE_H_INCLUDED
#define OMXDECTESTBASE_H_INCLUDED

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef OSCL_SCHEDULER_H_INCLUDED
#include "oscl_scheduler.h"
#endif



#ifndef PV_OMXCORE_H_INCLUDED
#include "pv_omxcore.h"
#endif


#if PROXY_INTERFACE

#ifndef OMX_THREADSAFE_CALLBACKS_H_INLCUDED
#include "omx_threadsafe_callbacks.h"
#endif

#endif


//Three different default queue sizes for each callback.
//Idea here is to add flexibility to the code so that these queue sizes can be changed as per requirement
#define EVENT_HANDLER_QUEUE_DEPTH   20
#define EMPTY_BUFFER_DONE_QUEUE_DEPTH   10
#define FILL_BUFFER_DONE_QUEUE_DEPTH    10

#define BIT_BUFF_SIZE_MP3 200000
#define MIN_FRAME_SIZE 8192

#define BIT_BUFF_SIZE_AVC 200000
#define MIN_NAL_SIZE 20

/* Active object's states */
typedef enum STATE_TYPE
{
    StateInvalid,      /**< component has detected that it's internal data
                                structures are corrupted to the point that
                                it cannot determine it's state properly */
    StateUnLoaded,

    StateLoading,

    StateLoaded,      /**< component has been loaded but has not completed
                                initialization.  The OMX_SetParameter macro
                                and the OMX_GetParameter macro are the only
                                valid macros allowed to be sent to the
                                component in this state. */
    StateDecodeHeader,
    StateDisablePort,
    StateDynamicReconfig,
    StateIdle,        /**< component initialization has been completed
                                successfully and the component is ready to
                                to start. */
    StateExecuting,   /**< component has accepted the start command and
                                is processing data (if data is available) */
    StatePause,       /**< component has received pause command */
    StateStopping,
    StateCleanUp,
    StateStop,
    StateIntermediate,
    StateWaitingForEOSCallback,
    StateRestart,
    StateWaitForResources, /**< component is waiting for resources, either after
                                preemption or before it gets the resources requested.
                                See specification for complete details. */
    StateError,             /* Go to this state in case any error occur inbetween*/
    StateMax = 0X7FFFFFFF
} STATE_TYPE;


/* Application's private data */
typedef struct AppPrivateType
{
    OMX_BUFFERHEADERTYPE* pCurrentInputBuffer;
    OMX_HANDLETYPE Handle;
    OMX_S32     MinBufferSize;

} AppPrivateType;

class CallbackParentInt
{
    public:
#if PROXY_INTERFACE
        EventHandlerThreadSafeCallbackAO*    ipThreadSafeHandlerEventHandler;
        EmptyBufferDoneThreadSafeCallbackAO* ipThreadSafeHandlerEmptyBufferDone;
        FillBufferDoneThreadSafeCallbackAO*  ipThreadSafeHandlerFillBufferDone;
#else
        virtual OMX_ERRORTYPE EventHandler(OMX_OUT OMX_HANDLETYPE aComponent,
                                           OMX_OUT OMX_PTR aAppData,
                                           OMX_OUT OMX_EVENTTYPE aEvent,
                                           OMX_OUT OMX_U32 aData1,
                                           OMX_OUT OMX_U32 aData2,
                                           OMX_OUT OMX_PTR aEventData) = 0;

        virtual OMX_ERRORTYPE EmptyBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
                                              OMX_OUT OMX_PTR aAppData,
                                              OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer) = 0;

        virtual OMX_ERRORTYPE FillBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
                                             OMX_OUT OMX_PTR aAppData,
                                             OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer) = 0;
#endif
};



#if PROXY_INTERFACE

class CallbackContainer
{
    public:

        CallbackContainer(CallbackParentInt *obsvr);

        ~CallbackContainer()
        {
            OMX_S32 ii;
            for (ii = 0; ii < EVENT_HANDLER_QUEUE_DEPTH; ii++)
            {
                if (pEventHandlerArray[ii])
                {
                    oscl_free(pEventHandlerArray[ii]);
                    pEventHandlerArray[ii] = NULL;
                }
            }

            for (ii = 0; ii < EMPTY_BUFFER_DONE_QUEUE_DEPTH; ii++)
            {
                if (pEmptyBufferDoneArray[ii])
                {
                    oscl_free(pEmptyBufferDoneArray[ii]);
                    pEmptyBufferDoneArray[ii] = NULL;
                }
            }

            for (ii = 0; ii < FILL_BUFFER_DONE_QUEUE_DEPTH; ii++)
            {
                if (pFillBufferDoneArray[ii])
                {
                    oscl_free(pFillBufferDoneArray[ii]);
                    pFillBufferDoneArray[ii] = NULL;
                }
            }
        }

        bool initCallbacks()
        {
            OMX_S32 ii;

            for (ii = 0; ii < EVENT_HANDLER_QUEUE_DEPTH; ii++)
            {
                pEventHandlerArray[ii] = (EventHandlerSpecificData*) oscl_malloc(sizeof(EventHandlerSpecificData));
                if (NULL == pEventHandlerArray[ii])
                {
                    return false;
                }
                oscl_memset(pEventHandlerArray[ii], 0, sizeof(EventHandlerSpecificData));
            }

            for (ii = 0; ii < EMPTY_BUFFER_DONE_QUEUE_DEPTH; ii++)
            {
                pEmptyBufferDoneArray[ii] = (EmptyBufferDoneSpecificData*) oscl_malloc(sizeof(EmptyBufferDoneSpecificData));
                if (NULL == pEmptyBufferDoneArray[ii])
                {
                    return false;
                }
                oscl_memset(pEmptyBufferDoneArray[ii], 0, sizeof(EmptyBufferDoneSpecificData));
            }

            for (ii = 0; ii < FILL_BUFFER_DONE_QUEUE_DEPTH; ii++)
            {
                pFillBufferDoneArray[ii] = (FillBufferDoneSpecificData*) oscl_malloc(sizeof(FillBufferDoneSpecificData));
                if (NULL == pFillBufferDoneArray[ii])
                {
                    return false;
                }
                oscl_memset(pFillBufferDoneArray[ii], 0, sizeof(FillBufferDoneSpecificData));
            }

            return true;
        }

        OMX_CALLBACKTYPE* getCallbackStruct()
        {
            return &iCallbackType;
        }

    private:

        OMX_CALLBACKTYPE iCallbackType;

        static struct EventHandlerSpecificData* pEventHandlerArray[EVENT_HANDLER_QUEUE_DEPTH];
        static struct EmptyBufferDoneSpecificData* pEmptyBufferDoneArray[EMPTY_BUFFER_DONE_QUEUE_DEPTH];
        static struct FillBufferDoneSpecificData* pFillBufferDoneArray[FILL_BUFFER_DONE_QUEUE_DEPTH];

        static CallbackParentInt* iParent;

        static OMX_ERRORTYPE CallbackEventHandler(
            OMX_OUT OMX_HANDLETYPE aComponent,
            OMX_OUT OMX_PTR aAppData,
            OMX_OUT OMX_EVENTTYPE aEvent,
            OMX_OUT OMX_U32 aData1,
            OMX_OUT OMX_U32 aData2,
            OMX_OUT OMX_PTR aEventData);

        static OMX_ERRORTYPE CallbackEmptyBufferDone(
            OMX_OUT OMX_HANDLETYPE aComponent,
            OMX_OUT OMX_PTR aAppData,
            OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);

        static OMX_ERRORTYPE CallbackFillBufferDone(
            OMX_OUT OMX_HANDLETYPE aComponent,
            OMX_OUT OMX_PTR aAppData,
            OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);
};

#else // !PROXY_INTERFACE


class CallbackContainer
{
    public:

        CallbackContainer(CallbackParentInt *parentinterface);

        ~CallbackContainer()
        {
        }

        bool initCallbacks()
        {
            return OMX_TRUE;
        }

        OMX_CALLBACKTYPE *getCallbackStruct()
        {
            return &iCallbackType;
        }

    private:

        OMX_CALLBACKTYPE iCallbackType;

        static CallbackParentInt* iParent;

        static OMX_ERRORTYPE CallbackEventHandler(
            OMX_OUT OMX_HANDLETYPE aComponent,
            OMX_OUT OMX_PTR aAppData,
            OMX_OUT OMX_EVENTTYPE aEvent,
            OMX_OUT OMX_U32 aData1,
            OMX_OUT OMX_U32 aData2,
            OMX_OUT OMX_PTR aEventData);

        static OMX_ERRORTYPE CallbackEmptyBufferDone(
            OMX_OUT OMX_HANDLETYPE aComponent,
            OMX_OUT OMX_PTR aAppData,
            OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);

        static OMX_ERRORTYPE CallbackFillBufferDone(
            OMX_OUT OMX_HANDLETYPE aComponent,
            OMX_OUT OMX_PTR aAppData,
            OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);
};
#endif // PROXY_INTERFACE

class AVCBitstreamObject;
class Mp3BitstreamObject;

//Main AO class for the TestApplication

class OmxDecTestBase : public OsclTimerObject
        , public CallbackParentInt
{
    public:

        OmxDecTestBase(const char objectName[] = "Omax_Decode_Test");
        virtual ~OmxDecTestBase()
        {
            if (iCallbacks) delete iCallbacks;

            if (IsAdded())
            {
                RemoveFromScheduler();
            }

            iLogger = NULL;
            OsclExecScheduler* sched = OsclExecScheduler::Current();
            if (sched)
            {
                sched->StopScheduler();
            }

        }

        void InitScheduler();
        virtual void StartTestApp();

#if PROXY_INTERFACE
        //Process callback functions. They will be executed in testapp thread context
        OsclReturnCode ProcessCallbackEventHandler(OsclAny* P);
        OsclReturnCode ProcessCallbackEmptyBufferDone(OsclAny* P);
        OsclReturnCode ProcessCallbackFillBufferDone(OsclAny* P);
#endif

    protected:

        virtual OMX_ERRORTYPE EventHandler(OMX_OUT OMX_HANDLETYPE aComponent,
                                           OMX_OUT OMX_PTR aAppData,
                                           OMX_OUT OMX_EVENTTYPE aEvent,
                                           OMX_OUT OMX_U32 aData1,
                                           OMX_OUT OMX_U32 aData2,
                                           OMX_OUT OMX_PTR aEventData);

        virtual OMX_ERRORTYPE EmptyBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
                                              OMX_OUT OMX_PTR aAppData,
                                              OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);

        virtual OMX_ERRORTYPE FillBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
                                             OMX_OUT OMX_PTR aAppData,
                                             OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer);

        virtual OMX_BOOL VerifyAllBuffersReturned();

        CallbackContainer* iCallbacks;

        OMX_S32             iPendingCommands;

        AppPrivateType*     ipAppPriv;

        OMX_BOOL            iStopProcessingInput;
        OMX_BOOL            iHeaderSent;

        OMX_S32             iFrameNum;
        OMX_S32             iFramesWritten;
        OMX_U32             iLastInputFrame;

        OMX_BOOL            iFragmentInProgress;
        OMX_BOOL            iDivideBuffer;
        OMX_S32             iCurFrag;
        OMX_S32             iNumSimFrags;
        OMX_S32             iSimFragSize[50];
        OMX_S32             iInIndex;
        OMX_BOOL            iInputWasRefused;
        OMX_PARAM_PORTDEFINITIONTYPE iParamPort;
        OMX_AUDIO_PARAM_PCMMODETYPE  iPcmMode;
        OMX_AUDIO_PARAM_AMRTYPE      iAmrParam;
        OMX_PORT_PARAM_TYPE iPortInit;
        OMX_BOOL            iStopOutput;
        OMX_BOOL            iStopInput;

        /* Flags to help change states and send input & output buffers to component */
        OMX_BOOL            iInputReady;

        //Flag to check availability of multiple input and output buffers
        OMX_BOOL* ipInputAvail;
        OMX_BOOL* ipOutReleased;

        //Input & output buffer to pass data to the component
        OMX_BUFFERHEADERTYPE** ipInBuffer;
        OMX_BUFFERHEADERTYPE** ipOutBuffer;

        //Input/output buffers required for usebuffer macro
        OMX_U8** ipUseInBuff;
        OMX_U8** ipUseOutBuff;


        //Variable to keep track of state of Application's AO
        STATE_TYPE  iState;

        //Input & output buffer sizes for the input & output buffers of component
        OMX_S32 iInBufferSize;
        OMX_S32 iOutBufferSize;

        //Input & output buffer count
        OMX_S32 iInBufferCount;
        OMX_S32 iOutBufferCount;

        //Count variables to stop & start sending input/output buffers
        OMX_S32 iCount;
        OMX_S32 iCount1;
        OMX_S32 iCount2;
        OMX_S32 iCount3;

        OMX_S32 iStreamCorruptCount;

        //Object of memory allocator class
        OsclMemAllocator iAllocator;
        AVCBitstreamObject* ipAVCBSO;
        Mp3BitstreamObject* ipMp3Bitstream;

        OMX_U8* ipNalBuffer;
        OMX_S32 iNalSize;
        OMX_S32 iMaxSize;
        OMX_S32 iRemainingFrameSize;
        OMX_U8* ipBitstreamBuffer;
        OMX_U8* ipBitstreamBufferPointer;

        // Loggger variable for logging data
        PVLogger* iLogger;

        virtual void Run() = 0;
        virtual bool WriteOutput(OMX_U8* aOutBuff, OMX_U32 aSize) = 0;
        virtual OMX_ERRORTYPE GetInput() = 0;
};

#endif      // OMXDECTESTBASE_H_INCLUDED
