/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _PLAYERDRIVER_H
#define _PLAYERDRIVER_H


#include <cstring>
#include <ui/ISurface.h>
#include <utils/Errors.h>  // for android::status_t
#include <utils/RefBase.h>  // for android::sp
#include <media/MediaPlayerInterface.h>

#include <utils/List.h>
#include <cutils/properties.h>
#include <media/PVPlayer.h>

#include "oscl_base.h"
#include "oscl_exception.h"
#include "oscl_scheduler.h"
#include "oscl_scheduler_ao.h"
#include "oscl_exception.h"
#include "oscl_mem_basic_functions.h"  // oscl_memcpy
#include "oscl_mem.h"
#include "oscl_mem_audit.h"
#include "oscl_error.h"
#include "oscl_utf8conv.h"
#include "oscl_string_utils.h"

#include "media_clock_converter.h"

#include "pv_player_factory.h"
#include "pv_player_interface.h"
#include "pv_engine_observer.h"
#include "pvmi_media_io_fileoutput.h"
#include "pv_player_datasourceurl.h"
#include "pv_player_datasinkpvmfnode.h"
#include "pvmf_errorinfomessage_extension.h"
#include "pvmf_basic_errorinfomessage.h"
#include "pvmf_fileformat_events.h"
#include "pvmf_mp4ffparser_events.h"
#include "pvmf_duration_infomessage.h"
#include "android_surface_output.h"
#include "android_audio_output.h"
#include "android_audio_stream.h"
#include "pv_media_output_node_factory.h"
#include "pvmf_format_type.h"  // for PVMFFormatType
#include "pvmf_node_interface.h"
#include "pvmf_source_context_data.h"
#include "pvmf_download_data_source.h"
//#include "pvmf_return_codes.h"  // for PVMFStatus
#include "OMX_Core.h"
#include "pv_omxcore.h"

// color converter
#include "cczoomrotation16.h"

//for KMJ DRM Plugin
#include "pvmf_local_data_source.h"
#include "pvmf_recognizer_registry.h"

class  PVPlayerExtensionHandler;

typedef void (*media_completion_f)(android::status_t status, void *cookie, bool cancelled);

// Commands that MediaPlayer sends to the PlayerDriver
// TODO: Move this class and subclass in their own .h
// TODO: Write a class comment.
class PlayerCommand
{
  public:
    // TODO: Explain these codes.
    enum Code {
        // Stops the scheduler. Does not free any resources in the player.
        PLAYER_QUIT                     = 1,
        // Load the player capabilities.
        PLAYER_SETUP                    = 2,
        // Reset must be called before set data source.
        PLAYER_SET_DATA_SOURCE          = 3,
        // TODO: Rename to PLAYER_SET_VIDEO_SINK.
        PLAYER_SET_VIDEO_SURFACE        = 4,
        PLAYER_SET_AUDIO_SINK           = 5,
        // Must be called after a data source has been set or modified.
        PLAYER_INIT                     = 6,
        // PLAYER_PREPARE must be called after the source(s) and sink(s) have
        // been set or after a PLAYER_STOP command.
        PLAYER_PREPARE                  = 7,
        PLAYER_START                    = 8,
        // After a PLAYER_STOP you must issue a PLAYER_PREPARE to replay the
        // same source/sink combination.
        PLAYER_STOP                     = 9,
        PLAYER_PAUSE                    = 10,
        // Discard the sinks. Does not cancel all pending and running
        // commands. Does not remove the data source.
        PLAYER_RESET                    = 11,
        // TODO: What is that? Loop on the data source?
        PLAYER_SET_LOOP                 = 12,
        // TODO: When can this happen? in prepared started and pause?
        PLAYER_SEEK                     = 13,
        // TODO: When can this happend?  in prepared started and pause?
        PLAYER_GET_POSITION             = 14,
        PLAYER_GET_DURATION             = 15,
        // TODO: Can this command be issued anytime?
        PLAYER_GET_STATUS               = 16,
        // TODO: How is that different from reset? Does it leave the sink
        // untouched?
        PLAYER_REMOVE_DATA_SOURCE       = 17,
        // TODO: clarify the scope of PLAYER_CANCEL_ALL_COMMANDS, does it work
        // for asynchronous commands only or for synchronous as well?
        PLAYER_CANCEL_ALL_COMMANDS      = 18,
        PLAYER_EXTENSION_COMMAND        = 19,
        PLAYER_CHECK_LIVE_STREAMING     = 20,
    };

    virtual             ~PlayerCommand() {}
    Code                code() const { return mCode; }
    media_completion_f  callback() { return mCallback; }
    void*               cookie() { return mCookie; }

    // If no callback was supplied, the command runs in synchronous mode.
    bool                hasCallback() const { return NULL != mCallback; }
    void                complete(android::status_t status, bool cancelled) { mCallback(status, mCookie, cancelled); }
    void                set(media_completion_f cbf, void* cookie) { mCallback = cbf; mCookie = cookie; }

    // @return the command code as a string.
    const char*         toString() const;
  protected:
    PlayerCommand(Code code, media_completion_f cbf, void* cookie) :
            mCode(code), mCallback(cbf), mCookie(cookie) {}
  private:
    PlayerCommand();
    Code                mCode;
    media_completion_f  mCallback;
    void*               mCookie;
};

class PlayerQuit : public PlayerCommand
{
  public:
    PlayerQuit(media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_QUIT, cbf, cookie) {}
  private:
    PlayerQuit();
};

class PlayerSetup : public PlayerCommand
{
  public:
    PlayerSetup(media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_SETUP, cbf, cookie) {}
  private:
    PlayerSetup();
};

class PlayerInit : public PlayerCommand
{
  public:
    PlayerInit(media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_INIT, cbf, cookie) {}
  private:
    PlayerInit();
};

class PlayerPrepare: public PlayerCommand
{
  public:
    PlayerPrepare(media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_PREPARE, cbf, cookie) {}
  private:
    PlayerPrepare();
};

class PlayerStart: public PlayerCommand
{
  public:
    PlayerStart(media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_START, cbf, cookie) {}
  private:
    PlayerStart();
};

class PlayerStop: public PlayerCommand
{
  public:
    PlayerStop(media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_STOP, cbf, cookie) {}
  private:
    PlayerStop();
};

class PlayerPause: public PlayerCommand
{
  public:
    PlayerPause(media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_PAUSE, cbf, cookie) {}
  private:
    PlayerPause();
};

class PlayerReset: public PlayerCommand
{
  public:
    PlayerReset(media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_RESET, cbf, cookie) {}
  private:
    PlayerReset();
};

class PlayerSetDataSource : public PlayerCommand
{
  public:
    PlayerSetDataSource(const char* url, media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_SET_DATA_SOURCE, cbf, cookie), mUrl(0) {
        if (url) mUrl = strdup(url); }
    ~PlayerSetDataSource() { if (mUrl) free(mUrl); }
    const char*         url() const { return mUrl; }
  private:
    PlayerSetDataSource();
    char*               mUrl;
};

class PlayerSetVideoSurface : public PlayerCommand
{
  public:
    PlayerSetVideoSurface(const android::sp<android::ISurface>& surface, media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_SET_VIDEO_SURFACE, cbf, cookie), mSurface(surface) {}
    ~PlayerSetVideoSurface() { mSurface.clear(); }
    android::sp<android::ISurface>        surface() const { return mSurface; }
  private:
    PlayerSetVideoSurface();
    android::sp<android::ISurface>        mSurface;
};

class PlayerSetAudioSink : public PlayerCommand
{
  public:
    PlayerSetAudioSink(const android::sp<android::MediaPlayerInterface::AudioSink>& audioSink,
                       media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_SET_AUDIO_SINK, cbf, cookie), mAudioSink(audioSink) {}
    ~PlayerSetAudioSink() { mAudioSink.clear(); }
    android::sp<android::MediaPlayerInterface::AudioSink> audioSink() { return mAudioSink; }
  private:
    PlayerSetAudioSink();
    android::sp<android::MediaPlayerInterface::AudioSink> mAudioSink;
};

class PlayerSetLoop: public PlayerCommand
{
  public:
    PlayerSetLoop(int loop, media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_SET_LOOP, cbf, cookie), mLoop(loop) {}
    int loop() { return mLoop; }
  private:
    PlayerSetLoop();
    int                 mLoop;
};

class PlayerSeek : public PlayerCommand
{
  public:
    PlayerSeek(int msec, media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_SEEK, cbf, cookie), mMsec(msec) {}
    int                 msec() { return mMsec; }
  private:
    PlayerSeek();
    int                 mMsec;
};

class PlayerGetPosition: public PlayerCommand
{
  public:
    PlayerGetPosition(int* msec, media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_GET_POSITION, cbf, cookie), mMsec(msec) {}
    void set(int msecs) { if (mMsec) *mMsec = msecs; }
  private:
    PlayerGetPosition();
    int*                mMsec;
};

class PlayerGetDuration: public PlayerCommand
{
  public:
    PlayerGetDuration(int* msec, media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_GET_DURATION, cbf, cookie), mMsec(msec) {}
    void set(int msecs) { if (mMsec) *mMsec = msecs; }
  private:
    PlayerGetDuration();
    int*                mMsec;
};

class PlayerCheckLiveStreaming: public PlayerCommand
{
  public:
    PlayerCheckLiveStreaming(media_completion_f cbf, void* cookie) :
    PlayerCommand(PLAYER_CHECK_LIVE_STREAMING, cbf, cookie) {}
  private:
    PlayerCheckLiveStreaming();
};

class PlayerGetStatus: public PlayerCommand
{
  public:
    PlayerGetStatus(int *status, media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_GET_STATUS, cbf, cookie), mStatus(status) {}
    void set(int status) { *mStatus = status; }
  private:
    PlayerGetStatus();
    int*                mStatus;
};

class PlayerRemoveDataSource: public PlayerCommand
{
  public:
    PlayerRemoveDataSource(media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_REMOVE_DATA_SOURCE, cbf, cookie) {}
  private:
    PlayerRemoveDataSource();
};

class PlayerCancelAllCommands: public PlayerCommand
{
  public:
    PlayerCancelAllCommands(media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_CANCEL_ALL_COMMANDS, cbf, cookie) {}
  private:
    PlayerCancelAllCommands();
};

class PlayerExtensionCommand: public PlayerCommand
{
  public:
    PlayerExtensionCommand(const Parcel& aData, Parcel& aReply, media_completion_f cbf, void* cookie) :
            PlayerCommand(PLAYER_EXTENSION_COMMAND, cbf, cookie),mData(aData),mReply(aReply),mCompletionHandle(NULL) {}
    Parcel& getReplyParcel(){ return mReply;}
    const Parcel& getDataParcel(){ return mData;}
    void setCompletionHandle(int32 aHandle) {mCompletionHandle=aHandle;}
    int32 getCompletionHandle() { return mCompletionHandle; }
  private:
    PlayerExtensionCommand();
    const Parcel&       mData;
    Parcel&             mReply;
    int32               mCompletionHandle;
};

class PlayerDriver :
        public OsclActiveObject,
        public PVCommandStatusObserver,
        public PVInformationalEventObserver,
        public PVErrorEventObserver
{
  public:
    PlayerDriver(PVPlayer* pvPlayer);
    ~PlayerDriver();

    PlayerCommand* dequeueCommand();
    status_t enqueueCommand(PlayerCommand* code);

    // Dequeues a code from MediaPlayer and gives it to PVPlayer.
    void Run();

    // Handlers for the various commands we can accept.
    void commandFailed(PlayerCommand* command) ;
    void handleSetup(PlayerSetup* command);
    void handleSetDataSource(PlayerSetDataSource* command);
    void handleSetVideoSurface(PlayerSetVideoSurface* command);
    void handleSetAudioSink(PlayerSetAudioSink* command);
    void handleInit(PlayerInit* command);
    void handlePrepare(PlayerPrepare* command);
    void handleStart(PlayerStart* command);
    void handleStop(PlayerStop* command);
    void handlePause(PlayerPause* command);
    void handleSeek(PlayerSeek* command);
    void handleCancelAllCommands(PlayerCancelAllCommands* command);
    void handleRemoveDataSource(PlayerRemoveDataSource* command);
    void handleReset(PlayerReset* command);
    void handleQuit(PlayerQuit* command);
    void handleGetPosition(PlayerGetPosition* command);
    void handleGetDuration(PlayerGetDuration* command);
    void handleGetStatus(PlayerGetStatus* command);
    void handleCheckLiveStreaming(PlayerCheckLiveStreaming* cmd);
    void handleExtensionCommand(PlayerExtensionCommand* command);

    PVMFFormatType getFormatType();
    void CommandCompleted(const PVCmdResponse& aResponse);
    void HandleErrorEvent(const PVAsyncErrorEvent& aEvent);
    void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

  private:
    // PVPlayerExtensionHandler needs  to access mPlayer and FinishSyncCommand()
    // for calling PVPlayerEngine APIs and completing the commands.
    friend class PVPlayerExtensionHandler;
    // Finish up a non-async code in such a way that
    // the event loop will keep running.
    void FinishSyncCommand(PlayerCommand* command);

    void handleGetDurationComplete(PlayerGetDuration* cmd);
    void handleCheckLiveStreamingComplete(PlayerCheckLiveStreaming* cmd);

    int setupRtspStreamPre();
    int setupHttpStreamPre();
    int setupHttpStreamPost();


    // Starts the PV scheduler thread.
    static int startPlayerThread(void *cookie);
    int playerThread();

    // Callback for synchronous commands.
    static void syncCompletion(status_t s, void *cookie, bool cancelled);

    PVPlayer                *mPvPlayer;
    PVPlayerInterface       *mPlayer;
    PVPlayerDataSourceURL   *mDataSource;

    PVPlayerDataSink        *mAudioSink;
    PVMFNodeInterface       *mAudioNode;
    AndroidAudioMIO         *mAudioOutputMIO;

    PVPlayerDataSink        *mVideoSink;
    PVMFNodeInterface       *mVideoNode;
    PvmiMIOControl          *mVideoOutputMIO;

    PvmiCapabilityAndConfig *mPlayerCapConfig;

    OSCL_wHeapString<OsclMemAllocator> mDownloadFilename;
    OSCL_HeapString<OsclMemAllocator> mDownloadProxy;
    OSCL_wHeapString<OsclMemAllocator> mRTSPProxy;
    OSCL_wHeapString<OsclMemAllocator> mDownloadConfigFilename;
    PVMFSourceContextData   *mDownloadContextData;
    PVMFSourceContextData   *mLocalContextData;

    PVPMetadataList mMetaKeyList;
    Oscl_Vector<PvmiKvp,OsclMemAllocator> mMetaValueList;
    int mNumMetaValues;
    PVPMetadataList mCheckLiveKey;
    Oscl_Vector<PvmiKvp,OsclMemAllocator> mCheckLiveValue;
    int mCheckLiveMetaValues;

    // Semaphore used for synchronous commands.
    OsclSemaphore           *mSyncSem;
    // Status cached by syncCompletion for synchronous commands.
    status_t                mSyncStatus;

    // Command queue and its lock.
    List<PlayerCommand*>    mCommandQueue;
    Mutex                   mQueueLock;

    bool                    mIsLooping;
    bool                    mDoLoop;
    bool                    mDataReadyReceived;
    bool                    mPrepareDone;
    bool                    mEndOfData;
    int                     mRecentSeek;
    bool                    mSeekComp;
    bool                    mSeekPending;
    bool                    mIsLiveStreaming;
    bool                    mEmulation;
    void*                   mLibHandle;
    PVPlayerExtensionHandler* mExtensionHandler;
};

#endif // _PLAYERDRIVER_H
