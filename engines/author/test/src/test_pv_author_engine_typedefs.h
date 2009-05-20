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
#ifndef TEST_PV_AUTHOR_ENGINE_TYPEDEFS_H
#define TEST_PV_AUTHOR_ENGINE_TYPEDEFS_H

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_ERROR_H_INCLUDED
#include "oscl_error.h"
#endif
#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif
#ifndef OSCL_CONFIG_IO_H_INCLUDED
#include "osclconfig_io.h"
#endif
#ifndef OSCL_FILE_IO_H_INCLUDED
#include "oscl_file_io.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_VECTOR_H_INCLUDED
#include "oscl_vector.h"
#endif
#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVLOGGER_STDERR_APPENDER_H_INCLUDED
#include "pvlogger_stderr_appender.h"
#endif
#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif
#ifndef PVLOGGER_TIME_AND_ID_LAYOUT_H_INCLUDED
#include "pvlogger_time_and_id_layout.h"
#endif
#ifndef TEST_CASE_H_INCLUDED
#include "test_case.h"
#endif
#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif
#ifndef PV_ENGINE_TYPES_H_INCLUDED
#include "pv_engine_types.h"
#endif
#ifndef PV_ENGINE_OBSERVER_H_INCLUDED
#include "pv_engine_observer.h"
#endif
#ifndef PVAUTHORENGINEFACTORY_H_INCLUDED
#include "pvauthorenginefactory.h"
#endif
#ifndef PVAUTHORENGINEINTERFACE_H_INCLUDED
#include "pvauthorengineinterface.h"
#endif
#ifndef PVAETESTINPUT_H_INCLUDED
#include "pvaetestinput.h"
#endif
#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif
#ifndef OSCL_SCHEDULER_H_INCLUDED
#include "oscl_scheduler.h"
#endif
#ifndef TEXT_TEST_INTERPRETER_H_INCLUDED
#include "text_test_interpreter.h"
#endif
#ifndef PV_MP4_H263_ENC_EXTENSION_H_INCLUDED
#include "pvmp4h263encextension.h"
#endif
#ifndef PVMP4FFCN_CLIPCONFIG_H_INCLUDED
#include "pvmp4ffcn_clipconfig.h"
#endif
#ifndef PVMF_FILEOUTPUT_CONFIG_H_INCLUDED
#include "pvmf_fileoutput_config.h"
#endif
#ifndef PVMF_COMPOSER_SIZE_AND_DURATION_H_INCLUDED
#include "pvmf_composer_size_and_duration.h"
#endif
#ifndef TEST_PV_AUTHOR_ENGINE_CONFIG_H_INCLUDED
#include "test_pv_author_engine_config.h"
#endif
#ifndef UNIT_TEST_ARGS_H_INCLUDED
#include "unit_test_args.h"
#endif
#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif
#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif
#ifndef OSCL_MEM_AUDIT_H_INCLUDED
#include "oscl_mem_audit.h"
#endif
#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif
#ifndef OSCL_STRING_UTILS_H
#include "oscl_string_utils.h"
#endif
#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_stdstring.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_H_INCLUDED
#include "pvmi_config_and_capability.h"
#endif
#ifndef PVMI_CONFIG_AND_CAPABILITY_OBSERVER_H_INCLUDED
#include "pvmi_config_and_capability_observer.h"
#endif

#ifndef OSCLCONFIG_H_INCLUDED
#include "osclconfig.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#ifndef PVAELOGGER_H_INCLUDED
#include "test_pv_author_engine_logger.h"
#endif

//#define _W(x) _STRLIT_WCHAR(x)

//composer mime type
#define K3gpComposerMimeType "/x-pvmf/ff-mux/3gp"
#define KAMRNbComposerMimeType      "/x-pvmf/ff-mux/amr-nb"
#define KAMRWBComposerMimeType      "/x-pvmf/ff-mux/amr-wb"
#define KAACADIFComposerMimeType    "/x-pvmf/ff-mux/adif"
#define KAACADTSComposerMimeType    "/x-pvmf/ff-mux/adts"

//encoder mime type
#define KAMRNbEncMimeType "/x-pvmf/audio/encode/amr-nb"
#define KAMRWbEncMimeType "/x-pvmf/audio/encode/amr-wb"
#define KAACMP4EncMimeType "/x-pvmf/audio/encode/X-MPEG4-AUDIO"
#define KH263EncMimeType "/x-pvmf/video/encode/h263"
#define KH264EncMimeType "/x-pvmf/video/encode/h264"
#define KMp4EncMimeType "/x-pvmf/video/encode/mp4"
#define KAACADIFEncMimeType         "/x-pvmf/audio/encode/aac/adif"
#define KAACADTSEncMimeType         "/x-pvmf/audio/encode/aac/adts"
#define KTextEncMimeType "/x-pvmf/text/encode/txt"

// Default input settings
extern const uint32 KVideoBitrate ;
extern const uint32 KVideoFrameWidth ;
extern const uint32 KVideoFrameHeight;
extern const uint32 KVideoTimescale ;
extern const uint32 KNumLayers;

extern const uint32 KVideoFrameRate;
extern const uint32 KNum20msFramesPerChunk;
extern const uint32 KAudioBitsPerSample;

extern const uint16 KVideoIFrameInterval;
extern const uint8 KH263VideoProfile;
extern const uint8 KH263VideoLevel;
extern const uint32 KAudioBitrate;
extern const uint32 KAudioBitrateWB;
extern const uint32 KAACAudioBitrate;
extern const uint32 KAudioTimescale;
extern const uint32 KAudioTimescaleWB;
extern const uint32 KAudioNumChannels;

extern const uint32 KMaxFileSize;
extern const uint32 KMaxDuration;
extern const uint32 KFileSizeProgressFreq;
extern const uint32 KDurationProgressFreq;
extern const uint32 KTestDuration;
extern const uint32 KTextTimescale;
extern const uint32 KTextFrameWidth;
extern const uint32 KTextFrameHeight;



// it's for setting Authoring Time Unit for selecting counter loop
// this time unit is used as default authoring time for Longetivity tests
//const uint32 KAuthoringSessionUnit = 1800; //in seconds
extern const uint32 KAuthoringSessionUnit;
extern const uint32 KPauseDuration;

// The string to prepend to source filenames
#define SOURCENAME_PREPEND_STRING ""
#define SOURCENAME_PREPEND_WSTRING _STRLIT_WCHAR("")


#define ARRAY_SIZE  512

//enum types for test cases
typedef enum
{
    ERROR_NOSTATE = 0,
    ERROR_VIDEO_START_INIT,
    ERROR_VIDEO_START_ENCODE,
    ERROR_VIDEO_START_ENCODE_5FRAMES,
    ERROR_COMPOSER_START_ADDMEMFRAG,
    ERROR_COMPOSER_START_ADDTRACK,
    ERROR_MEDIAINPUTNODE_ADDDATASOURCE_START,
    ERROR_MEDIAINPUTNODE_ADDDATASOURCE_STOP,
    ERROR_AVC_START_ENCODE,
    ERROR_AVC_START_ENCODE_5FRAMES
} FAIL_STATE;

/**
 * @page PVAETestCase List
 * @code
 *    pvaetest.exe [-test x0 x1]
 * @endcode
 * where x0, x1 gives the range of the tests to run x0 being test start number and x1 being test end number.
 * Other arguments can be checked with -help command line argument.
 * To run a single test, the x0 will be equal to the x1.
 * If a test case required some extra arguments other than those mentioned here, they are listed in
 * those individual test descriptions.
 * If no arguments are specified on command line it runs all the test cases by default.
 * Test cases are divided into parts Compressed and Uncompressed test cases.
 * The default input file location for the tests are as follows
 * Windows - C:\unit_test\pvaetest\
 * Linux - author\test\test_input\
 * Symbian - EPOCROOT\ winscw\c\
 * Winmobile - \Storage Card\
 * To pass the commandline arguments on device (Winmobile, S60 etc) we use a text file  named "input.txt".
 * A sample text file is present in the engines\author\test\build\ms_vc2005\ folder.
 * Default input files are located in folder author\test\test_input\.
 * These files needs to be copied at the above location before running the default tests.
 */

typedef enum
{

    /*********** Compressed Tests Begin****************************/
    /*
     * @page Compressed TestCase - General Info
     * These test cases takes AMR, AMR-WB, YUV, H263, AAC AND TEXT bitstreams as input files.
     * It picks the input files from the specified hard-coded paths if no commandline arguments are mentioned.
     * Some of the compressed inputs need a log file to go along with the bitstream. This logfile is provided
     * using the "-audiologfile" or "-videologfile" commandline option. In case of audio and video bitstreams
     * the log file format is as follows:
     * @code
     *  unsigned int(32) total_num_samples;
     *  unsigned int(32) avg_bitrate; // could be 0 if not available
     *  unsigned int(32) timescale; //this is the timescale of all sample timestamps below
     *  unsigned int(32) max_sample_size; // could be 0 if not available
     *  unsigned int(32) config_size; //could be 0 for streams that have no config, say AMR
     *  unsigned int(32) height; //zero if it is audio stream
     *  unsigned int(32) width; //zero if it is audio stream
     *  unsigned int(32) frame_rate; //zero if audio stream
     *  for (j=0; j < total_num_samples; j++)
     *  {
     *     unsigned int(32) sample_length_in_bytes;
     *     unsigned int(32) sample_timestamp;
     *  }
     * @endcode
     */
    /**
     * @test (0) Takes AMR input and authors it to AMR 3GP file.
     *       -audio <audio filename> -output <output filename>.
     *       If no cmdline arguments given it uses the default file i.e. "amrtestinput.amr"
     */
    AMR_Input_AOnly_3gpTest  = 0,
    /**
     * @test (1) Takes H263 input and authors it to H263 3GP file.
     *       -video <audio filename> -videoconfigfile <video logfilename> -output <output filename>.
     *       If no cmdline arguments given it uses the default file i.e. "h263testinput.h263"
     *       This TC uses no log file by default. If there is no log file it relies on H263 PSC
     *       to detect frame boundaries. It also assumes that the input is QCIF and has a framerate of
     *       15 fps.
     */

    H263_Input_VOnly_3gpTest = 1,
    /**
     * @test (2) Takes AMR and YUV inputs and authors it to AMR+H263 3GP file.
     *       No cmdline args needed. Only setup to work with default inputs,
     *       "amrtestinput.amr" and "yuvtestinput.yuv"
     *       Now that we support AVI files as testinputs we do not have plans
     *       to support arbitary YUV inputs. This testcase is there just to
     *       make sure that H263 encode from YUV inputs works.
     */

    AMR_YUV_Input_AV_3gpTest = 2,
    /**
     * @test (3) Takes AMR and H263 inputs and authors it to AMR+H263 3GP file.
     *       -audio <audio filename> -video <audio filename> -videoconfigfile <video logfilename> -output <output filename>.
     *        If no cmdline arguments given it uses default files "amrtestinput.amr" and "h263testinput.h263".
     *        This TC uses no video log file by default. If there is no log file it relies on H263 PSC
     *        to detect frame boundaries. It also assumes that the input is QCIF and has a framerate of
     *        15 fps.
     */

    AMR_H263_Input_AV_3gpTest = 3,
    /**
     * @test (5) Takes AMR and YUV inputs and authors it to AMR+M4V 3GP file
     *       No cmdline args needed. Only setup to work with default inputs,
     *       "amrtestinput.amr" and "yuvtestinput.yuv"
     *       Now that we support AVI files as testinputs we do not have plans
     *       to support arbitary YUV inputs. This testcase is there just to
     *       make sure that M4V encode from YUV inputs works.
     */

    AMR_YUV_Input_AV_M4V_AMR_Output_3gpTest = 5,
    /**
     * @test (6) Takes TEXT input and authors it to TEXT only 3GP file.
     *       No cmdline args needed. Only setup to work with default inputs,
     *       "naturemulti.txt", "naturemulti_text0.log" and "naturemulti_sd_txt1.txt" .
     */

    TEXT_Input_TOnly_3gpTest = 6,
    /**
     * @test (7) Takes AMR and TIMED TEXT input authors it to TEXT+AMR 3GP file
     *       No cmdline args needed. Only setup to work with default inputs,
     *       "amrtestinput.amr", "MOL004.txt", "MOL004_text0.log" and "MOL004_sd_txt1.txt" .
     */

    AMR_TEXT_Input_AT_3gpTest = 7,
    /**
     * @test (8) Takes TEXT and YUV inputs and authors it to TEXT+M4V 3GP file
     *       No cmdline args needed. Only setup to work with default inputs,
     *       "yuvtestinput.yuv", "MOL004.txt", "MOL004_text0.log" and "MOL004_sd_txt1.txt" .
     */

    YUV_TEXT_Input_VT_3gpTest = 8,
    /**
     * @test (9) Takes AMR, TEXT and YUV inputs and authors it to AMR+TEXT+M4V 3GP file
     *       No cmdline args needed. Only setup to work with default inputs,
     *       "amrtestinput.amr", "yuvtestinput.yuv", "MOL004.txt", "MOL004_text0.log" and "MOL004_sd_txt1.txt" .
     */

    AMR_YUV_TEXT_Input_AVT_3gpTest = 9,
    /**
     * @test (10) Placeholder Test
     */

    K3GP_OUTPUT_TestEnd = 10,
    /**
     * @test(11) Takes AMR raw input and authors another .amr file.
     *       -audio <audio filename> -output <output filename>.
     *       If no cmdline arguments given it uses the default file i.e. "amrtestinput.amr"
     */
    AMR_FOutput_Test = 11,
    /**
     * @test (12) Placeholder Test.
     */
    AMR_OUTPUT_TestEnd = 12,
    /**
     * @test (13) Takes AAC ADIF Input and authors it to another adif output
     *       No command line args needed. Only setup to work with default input,
     *       "aac_adif.aacadif".
     */
    AACADIF_FOutput_Test = 13,
    /**
     * @test (14) Takes AAC ADTS Input and authors it to another adts output
     *       No command line args needed. Only setup to work with default input,
     *       "aac_adif.aacadts".
     */
    AACADTS_FOutput_Test = 14,
    /**
     * @test (15) Placeholder Test.
     */
    AAC_OUTPUT_TestEnd = 15,
    /**
     * @test (16) Takes AMR input and authors it to .AMR file, by
     *       passing output file descriptor to author SDK instead of
     *       output file name.
     *       -audio <audio filename> -output <output filename>.
     *       If no cmdline arguments given it uses the default file i.e. "amrtestinput.amr"
     */
    AMR_FileOutput_Test_UsingExternalFileHandle = 16,
    /**
     * @test (17) Takes AMR-WB input and authors it to AMR-WB 3GP file.
     *       -audio <audio filename> -output <output filename>.
     *       If no cmdline arguments given it uses the default file i.e. "amrtestinput.awb"
     */

    AMRWB_Input_AOnly_3gpTest = 17,
    /**
     * @test(18) Takes AMR-WB input and authors another .awb file.
     *       -audio <audio filename> -output <output filename>.
     *       If no cmdline arguments given it uses the default file i.e. "amrtestinput.awb"
     */
    AMRWB_FOutput_Test = 18,
    /**
     * @test(19) Takes RAW AAC input and corresponding log file and authors AAC 3GP file.
     *       -audio <audio filename> -audioconfigfile <audio logfile> -output <output filename>.
     *       If no cmdline arguments given it uses the default files i.e. "aac_input.aac" and
     *       "aac_input.log"
     */
    AAC_Input_AOnly_3gpTest = 19,
    /**
     * @test(20) Takes M4V bitstream and corresponding log file and authors M4V 3GP file.
     *       -video <video filename> -videoconfigfile <video logfile> -output <output filename>.
     *       If no cmdline arguments given it uses the default files i.e. "m4v_input.m4v" and
     *       "m4v_input.log"
     */
    M4V_Input_AOnly_3gpTest = 20,
    /**
     * @test(20) Takes AVC bitstream and corresponding log file and authors H264 3GP file.
     *       -video <video filename> -videoconfigfile <video logfile> -output <output filename>.
     *       If no cmdline arguments given it uses the default files i.e. "avc_input.avc" and
     *       "avc_input.log"
     */
    AVC_Input_AOnly_3gpTest = 21,
    /**
     * @test (22) Placeholder test
     */
    CompressedNormalTestEnd = 22,
    /**
     * @test (100) Placeholder test
     */
    /*********** Compressed Longetivity Tests *********************/
    /*
     * These tests authors input files in a loop for a specified duration.
     * The duration can be specified by command line argument  -duration.
     * By default these tests use files specified in the default location
     * and are run for a duration of 1 minute.
     */
    CompressedLongetivityTestBegin = 100,
    /**
     * @test (101) This test cases is similar to TEXT_Input_Aonly_Mp4Test.
     *       Only difference is it creates the output file with user given time duration.
     *       No cmdline args needed. Only setup to work with default inputs,
     *       "MOL004.txt", "MOL004_text0.log" and "MOL004_sd_txt1.txt" .
     */
    TEXT_Input_TOnly_3gp_LongetivityTest = 101,
    /**
     * @test (102) This test cases is similar to AMR_TEXT_Input_AT_3gpTest.
     *       Only difference is it creates the output file with user given time duration.
     *       No cmdline args needed. Only setup to work with default inputs,
     *       "amrtestinput.amr", "yuvtestinput.yuv", "MOL004.txt", "MOL004_text0.log" and "MOL004_sd_txt1.txt" .
     */
    AMR_TEXT_Input_AT_3gp_LongetivityTest = 102,
    /**
     * @test (103) This test cases is similar to YUV_TEXT_Input_VT_3gpTest.
     *       Only difference is it creates the output file with user given time duration.
     *       No cmdline args needed. Only setup to work with default inputs,
     *       "yuvtestinput.yuv", "MOL004.txt", "MOL004_text0.log" and "MOL004_sd_txt1.txt" .
     */
    YUV_TEXT_Input_VT_3gp_LongetivityTest = 103,
    /**
     * @test (104) This test cases is similar to AMR_YUV_TEXT_Input_AVT_3gpTest.
     *       Only difference is it creates the output file with user given time duration.
     *       No cmdline args needed. Only setup to work with default inputs,
     *       "amrtestinput.amr", "yuvtestinput.yuv", "MOL004.txt", "MOL004_text0.log" and "MOL004_sd_txt1.txt" .
     */
    AMR_YUV_TEXT_Input_AVT_3gp_LongetivityTest = 104,
    /**
     * @test (105) Placeholder test
     */
    Compressed_LongetivityTestEnd = 105,
    /*********** Compressed Tests End******************************/

    /*********** UnCompressed Tests Begin**************************/
    /*
     * These test cases takes AVI/WAV input files. These files must have
     * have RGB24, RGB12, YUV420 planar or PCM mono 8KHz data only. Other formats
     * are not supported.
     * Only test case PVMediaInput_Open_Compose_Stop_Test takes input files from commandline arguments,
     * Other test cases do not take input files via commandline arguments.
     * It picks the files from the specified hard-coded paths.
     * Other optional command line args for these uncompressed tests are:
     * [-realtime] All uncompressed Normal test cases can be run in real time
     * by passing "-realtime" tag on commadline. It is an optional parameter,
     * by default the authoring mode will be ASAP.
     * [-aviconfigfile] Audio/Video Encoder parameters (i.e.audiobitrate,videobitrate) can
     * be set for uncompressed testcases using command line option -aviconfigfile <config file name>.
     * Syntax of parameters inside config file should be something like
     * videobitrate:52000 <CR-LF>
     * audiobitrate:12200 <CR-LF>
     *
     */
    UnCompressed_NormalTestBegin = 200,
    /**
     * @test (201) This test case takes an AVI file as input and creates a 3gp file as output.
     * -source <input filename> -output <output filename> -encV <video encoder type> -encA  <audio encoder type> [-realtime]
     * The value for
     * -encV :0 M4V
     *       :1 H263
     *       :2 H264
     * -encA 0: AMR-NB
     *       1: AMR-WB
     *       2: AAC-ADIF
     *       3: AAC-ADTS
     *       4: AAC-MPEG4_AUDIO
     * If we do not mention the input file and output filenames, it uses "testinput.avi".
     * If we donot pass [-realtime] it will author in ASAP mode.
     */
    PVMediaInput_Open_Compose_Stop_Test = 201,
    /**
     * @test (202) This test cases is similar to 201, only difference is it authors the output file in real time mode.
     *             It generates a video only M4V+AMR 3gp output file.
     *             No arguments required.
     *             Uses default input file "testinput.avi",
     */
    PVMediaInput_Open_RealTimeCompose_Stop_Test = 202,
    /**
     * @test (203) In this test we pass an AVI file which has only YUV content.
     *             It generates a video only M4V 3gp output file.
     *             No arguments required.
     *             Uses default input file "videoonly.avi",
     */
    YUVFromAVI_Input_VOnly_3gpTest = 203,
    /**
     * @test (204) It takes a WAV file as input file and authors an AMR 3gp file.
     *             No arguments required.
     *             Uses default input file "audioonly.wav",
     */
    PCM16_Input_AOnly_3gpTest = 204,
    /**
     * @test (205) This test cases takes AVI file with YUV and PCM tracks
     *             and generates an M4V+AMR 3gp output file.
     *             No arguments required.
     *             Uses default input file "testoutput_IYUV_8k_16_mono.avi",
     */
    PCM16_YUV_Input_AV_3gpTest = 205,
    /**
     * @test (207) It takes a WAV file as input file and authors .amr file
     *             No arguments required.
     *             Uses default input file "audioonly.wav",
     */
    PCM16In_AMROut_Test = 207,
    /**
     * @test (208) When maximum file size is enabled, the composer will stop the recording session when it detects
     *             that writing further media data to the output file will cause the size of output file to go beyond
     *             the maximum size. Default max file size is KMaxFileSize, which is 50 KB.
     *             This TC generates an M4V+AMR 3gp output file
     *             No arguments required.
     *             Uses default input file "testoutput_IYUV_8k_16_mono.avi",
     */

    KMaxFileSizeTest = 208,
    /**
     * @test (210) This testcase tests the support of 3GPP Download mode file authoring.
     *             In this mode the Meta Data is towards the end of the file and media data
     *             is interleaved. No temporary files are used.
     *             This TC generates an M4V+AMR 3gp output file
     *             No arguments required.
     *             Uses default input file "testoutput_IYUV_8k_16_mono.avi",
     */
    K3GPPDownloadModeTest = 210,
    /**
     * @test (211) This testcase tests the support of 3GPP Progressive Download mode file authoring.
     *             In this mode the Meta Data is towards the end of the file and
     *             media data is interleaved. Temporary files are used.
     *             This TC generates an M4V+AMR 3gp output file
     *             No arguments required.
     *             Uses default input file "testoutput_IYUV_8k_16_mono.avi",
     */
    K3GPPProgressiveDownloadModeTest = 211,
    /**
     * @test (213) This TC tests the support of movie fragment compliant file authoring.
     *             Meta Data is towards the end of the clip in MOOV and MOOF.
     *             Media Data is interleaved. No temp files are used.
     *             This TC generates an M4V+AMR 3gp output file
     *             No arguments required.
     *             Uses default input file "testoutput_IYUV_8k_16_mono.avi",
     */
    KMovieFragmentModeTest = 213,
    /**
     * @test (214) This test case checks the support of CapConfig in node and author.
     *             CapConfig support is added in each node and author engine so that
     *             we can directly change some of the parameters like height, width,
     *             cache-size etc.
     *             This TC generates an M4V+AMR 3gp output file
     *             No arguments required.
     *             Uses default input file "testoutput_IYUV_8k_16_mono.avi",
     */
    CapConfigTest = 214,
    /**
     * @test (215) Media Input Pause Resume Test
     *             It is an audio-video test which perform Pause-resume during record.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Pause_Resume_Test = 215,
    /**
     * @test (216) Media Input Reset after Create Test
     *             It is an audio-video test which perform Reset after Create.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Reset_After_Create_Test = 216,
    /**
     * @test (217) Media Input Reset after Open Test
     *             It is an audio-video test which perform Reset after Open.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Reset_After_Open_Test = 217,
    /**
     * @test (218) Media Input Reset after Add Data Source Test
     *             It is an audio-video test which perform Reset after AddDataSource.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Reset_After_AddDataSource_Test = 218,
    /**
     * @test (219) Media Input Reset after Select Composer Test
     *             It is an audio-video test which perform Reset after SelectComposer.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Reset_After_SelectComposer_Test = 219,
    /**
     * @test (220) Media Input Reset after Add Media Track Test
     *             It is an audio-video test which perform Reset after AddMediaTrack.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Reset_After_AddMediaTrack_Test = 220,
    /**
     * @test (221) Media Input Reset after Init Test
     *             It is an audio-video test which perform Reset after Init.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Reset_After_Init_Test = 221,
    /**
     * @test (222) Media Input Reset after Start Test
     *             It is an audio-video test which perform Reset after Start.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Reset_After_Start_Test = 222,
    /**
     * @test (223) Media Input Reset after Pause Test
     *             It is an audio-video test which perform Reset after Pause.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Reset_After_Pause_Test = 223,
    /**
     * @test (224) Media Input Reset after Recording Test
     *             It is an audio-video test which perform Reset after Recording.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Reset_After_Recording_Test = 224,
    /**
     * @test (225) Media Input Reset after Stop Test
     *             It is an audio-video test which perform Reset after Stop.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Reset_After_Stop_Test = 225,
    /**
     * @test (226) Media Input Delete after Create Test
     *             It is an audio-video test which perform Delete after Create.
     *             We do not guarantee a perfect cleanup, so there could be memleaks.
     *             We only guarantee a crash free execution.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Delete_After_Create_Test = 226,
    /**
     * @test (227) Media Input Delete after Open Test
     *             It is an audio-video test which perform Delete after Open.
     *             We do not guarantee a perfect cleanup, so there could be memleaks.
     *             We only guarantee a crash free execution.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Delete_After_Open_Test = 227,
    /**
     * @test (228) Media Input Delete after Add Data Source Test
     *             It is an audio-video test which perform Delete after AddDataSource.
     *             We do not guarantee a perfect cleanup, so there could be memleaks.
     *             We only guarantee a crash free execution.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Delete_After_AddDataSource_Test = 228,
    /**
     * @test (229) Media Input Delete after SelectComposer
     *             It is an audio-video test which perform Delete after SelectComposer.
     *             We do not guarantee a perfect cleanup, so there could be memleaks.
     *             We only guarantee a crash free execution.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Delete_After_SelectComposer_Test = 229,
    /**
     * @test (230) Media Input Delete after Add Media Track
     *             It is an audio-video test which perform Delete after AddMediaTrack.
     *             We do not guarantee a perfect cleanup, so there could be memleaks.
     *             We only guarantee a crash free execution.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Delete_After_AddMediaTrack_Test = 230,
    /**
     * @test (231) Media Input Delete after Init
     *             It is an audio-video test which perform Delete after Init.
     *             We do not guarantee a perfect cleanup, so there could be memleaks.
     *             We only guarantee a crash free execution.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Delete_After_Init_Test = 231,
    /**
     * @test (232) Media Input Delete after Start
     *             It is an audio-video test which perform Delete after Start.
     *             We do not guarantee a perfect cleanup, so there could be memleaks.
     *             We only guarantee a crash free execution.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Delete_After_Start_Test = 232,
    /**
     * @test (233) Media Input Delete after Pause
     *             It is an audio-video test which perform Delete after Pause.
     *             We do not guarantee a perfect cleanup, so there could be memleaks.
     *             We only guarantee a crash free execution.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Delete_After_Pause_Test = 233,
    /**
     * @test (234) Media Input Delete after Recording
     *             It is an audio-video test which perform Delete after Recording.
     *             We do not guarantee a perfect cleanup, so there could be memleaks.
     *             We only guarantee a crash free execution.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Delete_After_Recording_Test = 234,
    /**
     * @test (235) Media Input Delete after Stop
     *             It is an audio-video test which perform Delete after Stop.
     *             We do not guarantee a perfect cleanup, so there could be memleaks.
     *             We only guarantee a crash free execution.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Delete_After_Stop_Test = 235,
    /**
     * @test (236) Media Input Open-Composer-Stop using external file handle
     *             It is an audio-video test which passes an open file handle
     *             instead of output file name to authorSDK.
     *             No arguments required.
     *             Uses default input file "testinput.avi"
     */
    PVMediaInput_Open_Compose_Stop_Test_UsingExternalFileHandle = 236,
    /**
     * @test (237) Placeholder Test
     */
    UnCompressed_NormalTestEnd = 237,

    /********** Uncompressed Longetivity tests*********************/
    /*
     * These test cases takes AVI/WAV input files. These files must have
     * have RGB24, RGB12, YUV420 planar or PCM mono 8KHz data only. Other formats
     * are not supported.
     * Only test case PVMediaInput_Open_Compose_Stop_Test takes input files from commandline arguments,
     * Other test cases do not take input files via commandline arguments.
     * It picks the files from the specified hard-coded paths.
     * Other optional command line args for these uncompressed tests are:
     * [-realtime] All uncompressed Longetivity test cases can be run in real time
     * by passing "-realtime" tag on commadline. It is an optional parameter,
     * by default the authoring mode will be ASAP.
     * [-aviconfigfile] Audio/Video Encoder parameters (i.e.audiobitrate,videobitrate) can
     * be set for uncompressed testcases using command line option -aviconfigfile <config file name>.
     * Syntax of parameters inside config file should be something like
     * videobitrate:52000 <CR-LF>
     * audiobitrate:12200 <CR-LF>
     * [-duration] By this parameter we can run test for any given duration of time.
     * And test will create an output file of this duration long. It is an optional parameter,
     * by default test will be run for 1 minute.
     */
    UnCompressed_LongetivityTestBegin = 300,            //placeholder
    /**
     * @test (301) This test case takes and AVI file as an input and creates an output file for a given duration of time.
     *             The output file can also be created in realtime mode.
     * -source <input filename> -output <output filename> -encV <video encoder type> -encA  <audio encoder type> [-realtime]
     * The value for
     * -encV :0 M4V
     *       :1 H263
     *       :2 H264
     * -encA 0: AMR-NB
     *       1: AMR-WB
     *       2: AAC-ADIF
     *       3: AAC-ADTS
     *       4: AAC-MPEG4_AUDIO
     * If we do not mention the input file and output filenames, it uses "testinput.avi".
     * If we donot pass [-realtime] it will author in ASAP mode.
     */

    AVI_Input_Longetivity_Test = 301,
    /**
     * @test (302) Max File Size Longetivity test
     *             This test case is almost same as the KMaxFileSizeTest.
     *             It takes an extra parameter i.e. time duration of the output file.
     *             The output file can also be created in realtime mode.
     */

    KMaxFileSizeLongetivityTest = 302,
    /**
     * @test (304) 3GPP Download mode Longetivity test
     *             This test case is almost same as the K3GPPDownloadModeTest.
     *             It takes an extra parameter i.e. time duration of the output file.
     *             The output file can also be created in realtime mode.
     */
    K3GPPDownloadModeLongetivityTest = 304,
    /**
     * @test (305) 3GPP Progressive Download mode Longetivity test
     *             This test case is almost same as the K3GPPProgressiveDownloadModeTest.
     *             It takes an extra parameter i.e. time duration of the output file.
     *             The output file can also be created in realtime mode.
     */
    K3GPPProgressiveDownloadModeLongetivityTest = 305,
    /**
     * @test (307) Movie Fragment Mode Longetivity test
     *             This test case is almost same as the KMovieFragmentModeTest.
     *             It takes an extra parameter i.e. time duration of the output file.
     *             The output file can also be created in realtime mode.
     */
    KMovieFragmentModeLongetivityTest = 307,
    /**
     * @test (308) Placeholder Test
     */
    UnCompressed_LongetivityTestEnd = 308,
    /*********** UnCompressed Tests End****************************/

    /*********** Error Handling Tests Begin************************/
    /*
     * Error Handling tests. These are to test the error handling capability of Author Engine.
     * Some of the tests takes unsupported inputs like RGB16 data (PVMediaInput_ErrorHandling_Test_WrongFormat).
     * Other tests deliberately induces errors at various points in the data path. The error point is send through
     * KVP keys through the test app.
     */

    /*
     * Error handling tests that takes compressed inputs.
     * These test cases takes only AMR,YUV,H263, H264 AND TEXT bitstreams as input files.
     * They perform testing error handling for worng input filename, worng fromat etc.
     * These do not take input files via commandline arguments.
     */
    /**
     * @test (400) Placeholder test
     */
    KCompressed_Errorhandling_TestBegin = 400,
    /**
     * @test (401) Error Handling test with wrong input file name
     *       It is an error handling testcase which induces the error by passing wrong file name.
     */
    ErrorHandling_WrongTextInputFileNameTest = 401,
    /**
     * @test (402) Error Handling test with Node start fail
     *       It is an error handling testcase which induces the error while node start
     */
    ErrorHandling_MediaInputNodeStartFailed = 402,
    /**
     * @test (403) Placeholder test
     */
    KCompressed_Errorhandling_TestEnd = 403,

    /*
     * Error handling tests that takes uncompressed inputs through avi files.
     * They perform testing error handling for worng input filename, worng fromat etc.
     * These do not take input files via commandline arguments.
     */
    /**
     * @test (500) Placeholder
     */
    KUnCompressed_Errorhandling_TestBegin = 500,
    /**
     * @test (501) Error Handling test with Wrong format
     *       It is an error handling testcase which induces error by passing wrong format
     *       Uses testinput_rgb16.avi.
     */
    PVMediaInput_ErrorHandling_Test_WrongFormat = 501,
    /**
     * @test (502) Error Handling test with wrong input file name
     *       It is an error handling testcase which induces error by passing wrong input file name
     *       No arguments required.
     */
    PVMediaInput_ErrorHandling_Test_WrongIPFileName = 502,
    /**
     * @test (503) Error Handling test with wrong output path
     *       It is an error handling testcase which induces error by passing wrong output path
     *       No arguments required.
     */
    ErrorHandling_WrongOutputPathTest = 503,
    /**
     * @test (504) Error Handling test with video Init failed
     *       No arguments required.
     *       This test case forces failure during Video Encoder Initialization.
     *       Error key used: "x-pvmf/encoder/video/error_start_init;valtype=bool"
     */
    ErrorHandling_VideoInitFailed = 504,
    /**
     * @test (505) Error Handling test with video Encode failed
     *       No arguments required.
     *       This test case forces failure when Video Encoder start encoding frames.
     *       Error key used: "x-pvmf/encoder/video/error-encode;mode=frames;valtype=uint32", with a value of 1
     */
    ErrorHandling_VideoEncodeFailed = 505,
    /**
     * @test (506) Error Handling test with video Encode failed after 5 frames
     *       No arguments required.
     *       This test case forces failure after Video Encoder has encoded 5 frames.
     *       Error key used: "x-pvmf/encoder/video/error-encode;mode=frames;valtype=uint32", with a value of 5
     */
    ErrorHandling_VideoEncode5FramesFailed = 506,
    /**
     * @test (507) Error Handling test with Composer Add Fragment failed
     *       No arguments required.
     *       This test case forces MP4 composer addSampleToTrack to fail.
     *       Error key used: "x-pvmf/composer/mp4/error_start_addmemfrag;valtype=bool"
     */
    ErrorHandling_ComposerAddFragFailed = 507,
    /**
     * @test (508) Error Handling test with Composer Add Track failed
     *       No arguments required.
     *       This test case forces MP4 composer AddTrack to fail.
     *       Error key used: "x-pvmf/composer/mp4/error_start_addtrack;valtype=bool"
     */
    ErrorHandling_ComposerAddTrackFailed = 508,
    /**
     * @test (509) Error Handling test with AVC Encode failed
     *       No arguments required.
     *       This test case forces failure when AVC Video Encoder start encoding frames.
     *       Error key used: "x-pvmf/encoder/video/error-encode;mode=frames;valtype=uint32" with a value of 1
     */
    ErrorHandling_AVCVideoEncodeFailed = 509,
    /**
     * @test (510) Error Handling test with AVC Encode failed after 5 frames
     *       No arguments required.
     *       This test case forces failure after Video Encoder has encoded 5 frames.
     *       Error key used: "x-pvmf/encoder/video/error-encode;mode=frames;valtype=uint32", with a value of 5
     */
    ErrorHandling_AVCVideoEncode5FramesFailed = 510,
    /**
     * @test (511) This TC forces error from Media Input Node Stop.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error_adddatasource_stop;valtype=bool"
     */
    ErrorHandling_MediaInputNodeStopFailed = 511,
    /**
     * @test (512) This test case forces failure during audio encoder intialization.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/audio/error_start_init;valtype=bool"
     */
    ErrorHandling_AudioInitFailed = 512,
    /**
     * @test (509) Error Handling test with Audio Encode failed
     *       No arguments required.
     *       This test case forces failure when Audio Encoder start encoding frames.
     *       Error key used: "x-pvmf/encoder/audio/error-encode;mode=frames;valtype=uint32" with a value of 1
     */
    ErrorHandling_AudioEncodeFailed = 513,
    /**
     * @test (514) This test case tests the condition when no memory buffer is available in media input node.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error_no_memorybuffer_avaliable;valtype=bool"
     */
    ErrorHandling_MediaInputNode_NoMemBuffer = 514,
    /**
     * @test (515) This test case tests the condition when outgoing queue busy state occurs at MediaInputNode.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error_out_queue_busy;valtype=bool"
     */
    ErrorHandling_MediaInputNode_Out_Queue_busy = 515,
    /**
     * @test (516) This test case induces large value for first timestamp then subsequenly smaller
     *       timestamp values in decreasing order
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-time-stamp;valtype=ksv".
     *       The ksv is a pointer to key_type structure, with mode=1, duration=0,track-no=numStreams-1
     */
    ErrorHandling_MediaInputNode_large_time_stamp = 516,
    /**
     * @test (517) This test case induces wrong time stamp value after a duration.
     *       timestamp values in decreasing order
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-time-stamp;valtype=ksv".
     *       The ksv is a pointer to key_type structure, with mode=2, duration=60,track-no=numStreams-1
     */
    ErrorHandling_MediaInputNode_wrong_time_stamp_after_duration = 517,
    /**
     * @test (518) This test case sets the timestamp value as zero.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-time-stamp;valtype=ksv".
     *       The ksv is a pointer to key_type structure, with mode=3, duration=0,track-no=numStreams-1
     */
    ErrorHandling_MediaInputNode_zero_time_stamp = 518,
    /**
     * @test (519) This test case induces failure in Epause state of SendMioRequest()of MIO node.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-sendmiorequest;valtype=uint32" with a value of 2
     */
    ErrorHandling_MediaInputNode_StateFailure_EPause_SendMIORequest = 519,
    /**
     * @test (520) This test case induces failure in CancelMioRequest() of MIO Node.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-cancelmiorequest;valtype=bool"
     */
    ErrorHandling_MediaInputNode_StateFailure_CancelMIORequest = 520,
    /**
     * @test (521) This test case corrupts the video input data to check the tolerance of the encoder.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-corruptinputdata;index=0;valtype=uint32"
     */
    ErrorHandling_MediaInputNode_Corrupt_Video_InputData = 521,
    /**
     * @test (522) This test case corrupts the audio input data to check the tolerance of the encoder.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-corruptinputdata;index=1;valtype=uint32".
     */
    ErrorHandling_MediaInputNode_Corrupt_Audio_InputData = 522,
    /**
     * @test (523) This test case stalls the data for MIO Node.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-data-path-stall;valtype=uint32"
     */
    ErrorHandling_MediaInputNode_DataPath_Stall = 523,
    /**
     * @test (524) This test case fails the AddTrack() for PVMF_MIME_AMR_IETF.
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-addtrack;valtype=char*".
     */
    ErrorHandling_MP4Composer_AddTrack_PVMF_AMR_IETF = 524,
    /**
     * @test (525) This test case fails the AddTrack() for PVMF_MIME_3GPP_TIMEDTEXT.
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-addtrack;valtype=char*".
     */
    ErrorHandling_MP4Composer_AddTrack_PVMF_3GPP_TIMEDTEXT = 525,
    /**
     * @test (526) This test case fails the AddTrack() for PVMF_MIME_M4V.
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-addtrack;valtype=char*".
     */
    ErrorHandling_MP4Composer_AddTrack_PVMF_M4V = 526,
    /**
     * @test (527) This test case fails the AddTrack() for PVMF_MIME_H263.
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-addtrack;valtype=char*".
     */
    ErrorHandling_MP4Composer_AddTrack_PVMF_H263 = 527,
    /**
     * @test (528) This test case fails the AddTrack() for PVMF_MIME_H264_MP4.
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-addtrack;valtype=char*".
     */
    ErrorHandling_MP4Composer_AddTrack_PVMF_H264_MP4 = 528,
    /**
     * @test (529) This test case induces failure in creation of composer through createMP4File().
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-create-composer;valtype=bool"
     */
    ErrorHandling_MP4Composer_Create_FileParser = 529,
    /**
     * @test (530) Induce failure in RenderToFile function of MP4 Composer Node .
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-render-to-file;valtype=bool"
     */
    ErrorHandling_MP4Composer_RenderToFile = 530,
    /**
     * @test (531) This test sends error after a particular file size is reached in mp4 composer node.
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-addsample;mode=filesize;valtype=uint32"
     */
    ErrorHandling_MP4Composer_FailAfter_FileSize = 531,
    /**
     * @test (532) This test induces error after composing file for duration of time.
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-addsample;mode=duration;valtype=uint32"
     */
    ErrorHandling_MP4Composer_FailAfter_Duration = 532,
    /**
     * @test (533) This test simulates data path stall by not processing any data from input port of MP4CN.
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/data-path-stall;valtype=uint32".
     */
    ErrorHandling_MP4Composer_DataPathStall = 533,
    /**
     * @test (534) This test induces failure in  GetVolHeader() of video encode node.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-config-header;valtype=bool".
     */
    ErrorHandling_VideoEncodeNode_ConfigHeader = 534,
    /**
     * @test (535) This test simulates data path stall by not processing any data from input port.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/data-path-stall;valtype=uint32"
     */
    ErrorHandling_VideoEncodeNode_DataPathStall_Before_ProcessingData = 535,
    /**
     * @test (536) This test simulates data path stall by encoding the data but stop sending the encoded data on output port.
     *       Basically encode and toss.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/data-path-stall;valtype=uint32"
     */
    ErrorHandling_VideoEncodeNode_DataPathStall_After_ProcessingData = 536,
    /**
     * @test (537) This test fails the encode operation after some duration of time in video encoder node.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-encode;mode=duration;valtype=uint32"
     */
    ErrorHandling_VideoEncodeNode_FailEncode_AfterDuration = 537,
    /**
     * @test (538) This test case fails the encode operation after some duration of time for audio encode node.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/audio/error-encode;mode=duration;valtype=uint32"
     */
    ErrorHandling_AudioEncodeNode_FailEncode_AfterDuration = 538,
    /**
     * @test (539) This test case simulates data path stall by not processing any data from input port.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/audio/data-path-stall;valtype=uint32"
     */
    ErrorHandling_AudioEncodeNode_DataPathStall_Before_ProcessingData = 539,
    /**
     * @test (540) This test case simulates data path stall by encoding the data but stop sending the encoded data on output port.
     *       Basically encode and toss.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/audio/data-path-stall;valtype=uint32"
     */
    ErrorHandling_AudioEncodeNode_DataPathStall_After_ProcessingData = 540,
    /**
     * @test (541) This test case induces failure in getting SPS & PPS values.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-config-header;valtype=bool"
     */
    ErrorHandling_AVCEncodeNode_ConfigHeader = 541,
    /**
     * @test (542) This test case simulates data path stall by not processing any data from input port.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/data-path-stall;valtype=uint32"
     */
    ErrorHandling_AVCEncodeNode_DataPathStall_Before_ProcessingData = 542,
    /**
     * @test (543) This test case simulates data path stall by not processing any data from input port.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/data-path-stall;valtype=uint32"
     */
    ErrorHandling_AVCEncodeNode_DataPathStall_After_ProcessingData = 543,
    /**
     * @test (544) This test fails the encode operation after some duration of time in AVC encoder node.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/data-path-stall;valtype=uint32"
     */
    ErrorHandling_AVCEncodeNode_FailEncode_AfterDuration = 544,
    /**
     * @test (545) This test case induces error in DoStart() of the MIO node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_MediaInputNode_Node_Cmd_Start = 545,
    /**
     * @test (546) This test case induces error in DoStop() of the MIO node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_MediaInputNode_Node_Cmd_Stop = 546,
    /**
     * @test (547) This test case induces error in DoFlush() of the MIO node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_MediaInputNode_Node_Cmd_Flush = 547,
    /**
     * @test (548) This test case induces error in DoPause() of the MIO node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_MediaInputNode_Node_Cmd_Pause = 548,
    /**
     * @test (549) This test case induces error in DoReleasePort() of the MIO node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/datasource/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_MediaInputNode_Node_Cmd_ReleasePort = 549,
    /**
     * @test (550) This test case induces error in DoStart() of the MP4 composer node command.
    .    *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_MP4Composer_Node_Cmd_Start = 550,
    /**
     * @test (551) This test case induces error in DoStop() of the MP4 composer node command.
    .    *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_MP4Composer_Node_Cmd_Stop = 551,
    /**
     * @test (552) This test case induces error in DoFlush() of the MP4 composer node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_MP4Composer_Node_Cmd_Flush = 552,
    /**
     * @test (553) This test case induces error in DoPause() of the MP4 composer node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_MP4Composer_Node_Cmd_Pause = 553,
    /**
     * @test (554) This test case induces error in DoReleasePort() of the MP4 composer node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/composer/mp4/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_MP4Composer_Node_Cmd_ReleasePort = 554,
    /**
     * @test (555) This test case induces error in DoStart() of the video encoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_VideoEncodeNode_Node_Cmd_Start = 555,
    /**
     * @test (556) This test case induces error in DoStop() of the video encoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_VideoEncodeNode_Node_Cmd_Stop = 556,
    /**
     * @test (557) This test case induces error in DoFlush() of the  video encoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_VideoEncodeNode_Node_Cmd_Flush = 557,
    /**
     * @test (558) This test case induces error in DoPause() of the  video encoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_VideoEncodeNode_Node_Cmd_Pause = 558,
    /**
     * @test (559) This test case induces error in DoReleasePort() of the  video encoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_VideoEncodeNode_Node_Cmd_ReleasePort = 559,
    /**
     * @test (560) This test case induces error in DoStart() of the audio encoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/audio/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_AudioEncodeNode_Node_Cmd_Start = 560,
    /**
     * @test (561) This test case induces error in DoStop() of the audio encoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/audio/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_AudioEncodeNode_Node_Cmd_Stop = 561,
    /**
     * @test (562) This test case induces error in DoFlush() of the audio encoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/audio/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_AudioEncodeNode_Node_Cmd_Flush = 562,
    /**
     * @test (563) This test case induces error in DoPause() of the audio encoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/audio/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_AudioEncodeNode_Node_Cmd_Pause = 563,
    /**
     * @test (564) This test case induces error in DoReleasePort() of the  audio encoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/audio/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_AudioEncodeNode_Node_Cmd_ReleasePort = 564,
    /**
     * @test (565) This test case induces error in DoStart() of the AVC enoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_AVCEncodeNode_Node_Cmd_Start = 565,
    /**
     * @test (566) This test case induces error in DoStop() of the AVC enoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_AVCEncodeNode_Node_Cmd_Stop = 566,
    /**
     * @test (567) This test case induces error in DoFlush() of the AVC enoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_AVCEncodeNode_Node_Cmd_Flush = 567,
    /**
     * @test (568) This test case induces error in DoPause() of the AVC enoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_AVCEncodeNode_Node_Cmd_Pause = 568,
    /**
     * @test (569) This test case induces error in DoReleasePort() of the AVC enoder node command.
     *       No arguments required.
     *       Error key used: "x-pvmf/encoder/video/error-node-cmd;index=2;valtype=uint32"
     */
    ErrorHandling_AVCEncodeNode_Node_Cmd_ReleasePort = 569,
    /**
     * @test (570) Placeholder Test
     */
    KUnCompressed_Errorhandling_TestEnd = 570,
    /**
     * @test (1000) Placeholder Test
     */
    LastInteractiveTest = 1000,
    /**
     * @test (LastInteractiveTest+1) Invalid Test
     */
    Invalid_Test

} PVAETestCase;

class pvauthor_async_test_observer;

typedef struct
{
    pvauthor_async_test_observer* iObserver;
    test_case* iTestCase;
    int32 iTestCaseNum;
    FILE* iStdOut;

} PVAuthorAsyncTestParam;




/** Enumeration of types of asychronous commands that can be issued to PV Author Engine */
typedef enum
{
    PVAE_CMD_NONE = 0,
    PVAE_CMD_SET_LOG_APPENDER,
    PVAE_CMD_REMOVE_LOG_APPENDER,
    PVAE_CMD_SET_LOG_LEVEL,
    PVAE_CMD_GET_LOG_LEVEL,
    PVAE_CMD_CREATE,
    PVAE_CMD_OPEN,
    PVAE_CMD_CLOSE,
    PVAE_CMD_ADD_DATA_SOURCE,
    PVAE_CMD_ADD_DATA_SOURCE_AUDIO,
    PVAE_CMD_ADD_DATA_SOURCE_VIDEO,
    PVAE_CMD_ADD_DATA_SOURCE_TEXT,
    PVAE_CMD_REMOVE_DATA_SOURCE,
    PVAE_CMD_SELECT_COMPOSER,
    PVAE_CMD_ADD_MEDIA_TRACK,
    PVAE_CMD_ADD_AUDIO_MEDIA_TRACK,
    PVAE_CMD_ADD_VIDEO_MEDIA_TRACK,
    PVAE_CMD_ADD_TEXT_MEDIA_TRACK,
    PVAE_CMD_ADD_DATA_SINK,
    PVAE_CMD_REMOVE_DATA_SINK,
    PVAE_CMD_INIT,
    PVAE_CMD_RESET,
    PVAE_CMD_START,
    PVAE_CMD_PAUSE,
    PVAE_CMD_RESUME,
    PVAE_CMD_STOP,
    PVAE_CMD_QUERY_UUID,
    PVAE_CMD_QUERY_INTERFACE,
    PVAE_CMD_GET_SDK_INFO,
    PVAE_CMD_GET_SDK_MODULE_INFO,
    PVAE_CMD_CANCEL_ALL_COMMANDS,
    PVAE_CMD_QUERY_INTERFACE2,
    PVAE_CMD_CLEANUPANDCOMPLETE,
    PVAE_CMD_CAPCONFIG_SYNC,
    PVAE_CMD_CAPCONFIG_ASYNC,
    PVAE_CMD_RECORDING,
    PVAE_CMD_QUERY_INTERFACE1,
    PVAE_CMD_CAPCONFIG_SYNC1,
    PVAE_CMD_QUERY_INTERFACE_COMP
} PVAECmdType;


////////////////////////////////////////////////////////////////////////////
class PVLoggerSchedulerSetup
{
    public:
        PVLoggerSchedulerSetup() {};
        ~PVLoggerSchedulerSetup() {};

        void InitLoggerScheduler()
        {
            // Logging by PV Logger
            PVLogger::Init();
            //PVAELogger::ParseConfigFile(_W("uilogger.txt"));
            PVAELogger::ParseConfigFile(KPVAELoggerFile);

            // Construct and install the active scheduler
            OsclScheduler::Init("PVAuthorEngineTestScheduler");
        }

        void CleanupLoggerScheduler()
        {
            OsclScheduler::Cleanup();
            PVLogger::Cleanup();
        }

};
// Observer class for pvPlayer async test to notify completion of test
class pvauthor_async_test_observer
{
    public:
        // Signals completion of test. Test instance can be deleted after this callback completes.
        virtual void CompleteTest(test_case &) = 0;
};


////////////////////////////////////////////////////////////////////////////
class pvauthor_async_test_base : public OsclTimerObject,
        public PVCommandStatusObserver,
        public PVErrorEventObserver,
        public PVInformationalEventObserver
{
    public:
        pvauthor_async_test_base(PVAuthorAsyncTestParam aTestParam)
                : OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVAuthorEngineAsyncTestBase"),
                iObserver(aTestParam.iObserver),
                iTestCase(aTestParam.iTestCase),
                iTestCaseNum(aTestParam.iTestCaseNum),
                iStdOut(aTestParam.iStdOut)
        {};

        virtual ~pvauthor_async_test_base() {}
        virtual void StartTest() = 0;
        virtual void CommandCompleted(const PVCmdResponse& /*aResponse*/) {}
        virtual void HandleErrorEvent(const PVAsyncErrorEvent& /*aEvent*/) {}
        virtual void HandleInformationalEvent(const PVAsyncInformationalEvent& /*aEvent*/) {}

        pvauthor_async_test_observer* iObserver;
        test_case* iTestCase;
        int32 iTestCaseNum;
        FILE* iStdOut;
};


#endif //#ifndef TEST_PV_AUTHOR_ENGINE_TYPEDEFS_H

