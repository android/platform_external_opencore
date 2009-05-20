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

#include "testcaseparser.h"
#include "oscl_utf8conv.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#define COMMENT_INDICATOR_CHAR '#'
#define COMMENT_INDICATOR_STRING "#"
#define WHITESPACE_STRING " \t\r\n"

#define FILE_INDICATOR_STRING "FILE "

#define NAME_INDICATOR_STRING "NAME "

#define PRIORITY_INDICATOR_STRING "PRIORITY "
#define PRIORITY_MAND_INDICATOR_STRING "MANDATORY"
#define PRIORITY_OPT_INDICATOR_STRING "MANDATORY"

#define TERMINAL_INDICATOR_STRING "TERMINAL "
#define TERMINALA_INDICATOR_STRING "A"
#define TERMINALB_INDICATOR_STRING "B"

#define H223MUX_INDICATOR_STRING "H.223MUXLEVEL "
#define H223MUX_LEVEL0_INDICATOR_STRING "0"
#define H223MUX_LEVEL1_INDICATOR_STRING "1"
#define H223MUX_LEVEL1DF_INDICATOR_STRING "1DF" /* Double flag */
#define H223MUX_LEVEL2_INDICATOR_STRING "2"
#define H223MUX_LEVEL2OH_INDICATOR_STRING "2OH" /* Optional header */
#define H223MUX_LEVEL3_INDICATOR_STRING "3"

#define AL_INDICATOR_STRING "AL "
#define AL3_INDICATOR_STRING "AL3 "
#define AL2_INDICATOR_STRING "AL2 "
#define AL1_INDICATOR_STRING "AL1 "

#define AL2_SN_INDICATOR_STRING "AL2SN "
#define AL3_CFO_INDICATOR_STRING "AL3CFO "

#define MAXSDUSIZE_INDICATOR_STRING "MAXSDUSIZE"
#define MAXSDUSIZER_INDICATOR_STRING "MAXSDUSIZER"
#define MSD_INDICATOR_STRING "MSD "
#define MASTER_INDICATOR_STRING "MASTER"
#define SLAVE_INDICATOR_STRING "SLAVE"

#define CAPS_INDICATOR_STRING "CAPS "
#define RECEIVE_CAPS_INDICATOR_STRING "RECEIVE"
#define SEND_CAPS_INDICATOR_STRING "SEND"
#define AUDIO_AMR_INDICATOR_STRING "AMR"
#define AUDIO_G7321_INDICATOR_STRING "G7321"
#define VIDEO_H263BL_INDICATOR_STRING "H.263BL"
#define VIDEO_MP4SP_INDICATOR_STRING "MP4SP"
#define VIDEO_H263P3_INDICATOR_STRING "H.263P3"

#define AUDIO_INDICATOR_STRING "AUDIO"
#define VIDEO_INDICATOR_STRING "VIDEO"

#define ON_INDICATOR_STRING "ON"
#define OFF_INDICATOR_STRING "OFF"

#define USER_INPUT_CAPABILITY_STRING "USERINPUTCAPABILITY"
#define RECEIVE_INDICATOR_STRING "RECEIVE"
#define TRANSMIT_INDICATOR_STRING "TRANSMIT"
#define RECEIVE_TRANSMIT_INDICATOR_STRING "RECEIVEANDTRANSMIT"

//FILE CAPABILITIES
#define FILE_CAPABILITES_INDICATOR_STRING "FILECAP"
#define FILE_CAPABILITIES_MEDIATYPE_STRING "TYPE "


#define AUDIO_SAMPLING_FREQUENCY_STRING "SAMPLE "
#define AUDIO_NUM_CHANNELS_STRING "CHANNELS "
#define AUDIO_LOOP_INPUT_STRING "ALOOP "
#define AUDIO_FRAME_RATE_SIMULATION_STRING "ASIMULATION "

#define VIDEO_TIMESCALE_STRING "TIMESCALE "
#define VIDEO_FRAME_HEIGHT_STRING "FRAMEHEIGHT "
#define VIDEO_FRAME_WIDTH_STRING "FRAMEWIDTH "
#define VIDEO_FRAMERATE_STRING "FRAMERATE "
#define VIDEO_LOOP_INPUT_STRING "VLOOP "
#define VIDEO_FRAME_RATE_SIMULATION_STRING "VSIMULATION "

#define FILENAME_LEN   300
#define EXTN_LEN       5


void TrimSpaces(string& aStr)
{
    // trim leading whitespace
    string::size_type  lNotWhite = aStr.find_first_not_of(WHITESPACE_STRING);
    aStr.erase(0, lNotWhite);

    // trim trailing whitespace
    lNotWhite = aStr.find_last_not_of(WHITESPACE_STRING);
    aStr.erase(lNotWhite + 1);

    string::size_type lFirstWhite = 0, lSecondWhite = 0;
    //delete multiple whitespace
    while (1)
    {
        lFirstWhite = aStr.find_first_of(WHITESPACE_STRING, lSecondWhite);
        if (lFirstWhite == string::npos)
            break;
        lSecondWhite = aStr.find_first_of(WHITESPACE_STRING, lFirstWhite + 1);
        if (lSecondWhite == string::npos)
            break;
        if ((lSecondWhite - lFirstWhite) == 1)
            aStr.erase(lFirstWhite, 1);
        --lSecondWhite;
    }
}

void StripComments(string& aString)
{
    string::size_type lPos = aString.find_first_of(COMMENT_INDICATOR_STRING);
    if (lPos != string::npos)
    {
        aString.erase(lPos, aString.length());
    }
}

void SqueezeCommentsAndSpaces(string& aString)
{
    StripComments(aString);
    TrimSpaces(aString);
}

void SkipComments(ifstream& aInFile)
{
    while (aInFile.good())
    {
        if (aInFile.peek() == COMMENT_INDICATOR_CHAR)
        {
            string line;
            getline(aInFile, line);
            continue;
        }
        break;
    }
}

int GetProperLineFromFile(ifstream& aInFile, string &aLine)
{
    int lLineLength = 0;
    while (aInFile.good())
    {
        SkipComments(aInFile);
        getline(aInFile, aLine);
        lLineLength = aLine.length();
        SqueezeCommentsAndSpaces(aLine);
        if (aLine.length() != 0)
        {
            break;
        }
    }
    return lLineLength;
}

int strnicmp(const string & s1, const string& s2, const int numChars)
{
    string::const_iterator it1 = s1.begin();
    string::const_iterator it2 = s2.begin();
    int i = 0;

    //stop when either string's end has been reached
    while ((it1 != s1.end()) && (it2 != s2.end()) && (i < numChars))
    {
        if (::toupper(*it1) != ::toupper(*it2)) //letters differ?
            // return -1 to indicate smaller than, 1 otherwise
            return (::toupper(*it1)  < ::toupper(*it2)) ? -1 : 1;
        //proceed to the next character in each string
        ++it1;
        ++it2;
        ++i;
    }
    if (i == numChars)
        return 0;
    /* Should return the smaller/larger of strings ??? */
    return -1;
}

bool isLineType(const char* aSome, string& aStr)
{
    if (strnicmp(aStr, aSome, strlen(aSome)) == 0)
    {
        aStr.erase(0, strlen(aSome));
        return true;
    }
    return false;
}

TPVH223Level ParseMuxLevel(string aLine)
{
    TPVH223Level lMuxlvl;
    if (isLineType(H223MUX_LEVEL0_INDICATOR_STRING, aLine))
    {
        lMuxlvl = H223_LEVEL0;
    }
    else if (isLineType(H223MUX_LEVEL1DF_INDICATOR_STRING, aLine))
    {
        lMuxlvl = H223_LEVEL1_DF;
    }
    else if (isLineType(H223MUX_LEVEL1_INDICATOR_STRING, aLine))
    {
        lMuxlvl = H223_LEVEL1;
    }
    else if (isLineType(H223MUX_LEVEL2OH_INDICATOR_STRING, aLine))
    {
        lMuxlvl = H223_LEVEL2_OH;
    }
    else if (isLineType(H223MUX_LEVEL2_INDICATOR_STRING, aLine))
    {
        lMuxlvl = H223_LEVEL2;
    }
    else if (isLineType(H223MUX_LEVEL3_INDICATOR_STRING, aLine))
    {
        lMuxlvl = H223_LEVEL3;
    }
    return lMuxlvl;
}

bool GetBoolFromText(string& aLine)
{
    if (isLineType(ON_INDICATOR_STRING, aLine))
        return true;
    return false;
}

int GetIntFromText(string& line)
{
    return strtol(line.c_str(), 0, 10);
}

ParseALSettings(ifstream& aInFile, AlSettings *aAlSettings)
{
    int lActualLineLength;
    int *lAlPtr = 0;
    string line;

    while (aInFile.good())
    {
        lActualLineLength = GetProperLineFromFile(aInFile, line);
        if (isLineType(AL3_INDICATOR_STRING, line))
        {
            lAlPtr = &aAlSettings->iAL[AL3];
        }
        else if (isLineType(AL2_INDICATOR_STRING, line))
        {
            lAlPtr = &aAlSettings->iAL[AL2];
        }
        else if (isLineType(AL1_INDICATOR_STRING, line))
        {
            lAlPtr = &aAlSettings->iAL[AL1];
        }
        else
        {
            /* Another terminal statemnt has been hit*/
            aInFile.seekg(-(lActualLineLength + 2), ios::cur);
            break;
        }
        if (lAlPtr)
            *lAlPtr = (int)GetBoolFromText(line);
    }
    return 0;
}

ParseMaxSDUSizes(ifstream& aInFile, SDUSizeSettings* aSizeSettings)
{
    string line;
    int lActualLineLength;
    while (aInFile.good())
    {
        unsigned int *lSduSizePtr = 0;
        lActualLineLength = GetProperLineFromFile(aInFile, line);
        if (isLineType(AL3_INDICATOR_STRING, line))
        {
            lSduSizePtr = &aSizeSettings->iMaxSDUSize[AL3];
        }
        else if (isLineType(AL2_INDICATOR_STRING, line))
        {
            lSduSizePtr = &aSizeSettings->iMaxSDUSize[AL2];
        }
        else
        {
            aInFile.seekg(-(lActualLineLength + 2), ios::cur);
            break;
        }
        if (lSduSizePtr)
            *lSduSizePtr = GetIntFromText(line);
    }
    return 0;
}

ParseAudioCapSettings(ifstream& aInFile, MediaTypes *aMediaTypes)
{
    string line;
    int lActualLineLength;
    aMediaTypes->iMediaTypes.clear();
    while (aInFile.good())
    {
        lActualLineLength = GetProperLineFromFile(aInFile, line);
        if (isLineType(AUDIO_AMR_INDICATOR_STRING, line))
        {
            aMediaTypes->iMediaTypes.push_back(PVMF_MIME_AMR_IF2);
        }
        else if (isLineType(AUDIO_G7321_INDICATOR_STRING, line))
        {
            aMediaTypes->iMediaTypes.push_back(PVMF_MIME_G723);
        }
        else
        {
            aInFile.seekg(-(lActualLineLength + 2), ios::cur);
            break;
        }
    }
    return 0;
}

ParseVideoCapSettings(ifstream& aInFile, MediaTypes *aMediaTypes)
{
    string line;
    int lActualLineLength;
    aMediaTypes->iMediaTypes.clear();
    while (aInFile.good())
    {
        lActualLineLength = GetProperLineFromFile(aInFile, line);
        if (isLineType(VIDEO_H263BL_INDICATOR_STRING, line))
        {
            aMediaTypes->iMediaTypes.push_back(PVMF_MIME_H2632000);
        }
        else if (isLineType(VIDEO_MP4SP_INDICATOR_STRING, line))
        {
            aMediaTypes->iMediaTypes.push_back(PVMF_MIME_M4V);
        }
        else if (isLineType(VIDEO_H263P3_INDICATOR_STRING, line))
        {
            //aMediaTypes->iMediaTypes.push_back(PVMF_H263_P3);
            // Commented as no p3 type exists
            aMediaTypes->iMediaTypes.push_back(PVMF_MIME_H2632000);
        }
        else
        {
            aInFile.seekg(-(lActualLineLength + 2), ios::cur);
            break;
        }
    }
    return 0;
}

ParseCaps(ifstream& aInFile, CapSettings* aCapSettings)
{
    string line;
    int lActualLineLength;
    while (aInFile.good())
    {
        lActualLineLength = GetProperLineFromFile(aInFile, line);
        if (isLineType(AUDIO_INDICATOR_STRING, line))
            ParseAudioCapSettings(aInFile, &aCapSettings->iMediaCaps[AUDIO]);
        else if (isLineType(VIDEO_INDICATOR_STRING, line))
            ParseVideoCapSettings(aInFile, &aCapSettings->iMediaCaps[VIDEO]);
        else
        {
            aInFile.seekg(-(lActualLineLength + 2), ios::cur);
            break;
        }
    }
    return 0;
}

ParseTerminalSettings(ifstream& aInFile, TerminalSettings *aTermSettings)
{
    string line;
    int lActualLineLength;
    unsigned int *lSduSizePtr = 0;
    while (aInFile.good())
    {
        lActualLineLength = GetProperLineFromFile(aInFile, line);
        if (isLineType(H223MUX_INDICATOR_STRING, line))
        {
            aTermSettings->iH223MuxLevel = ParseMuxLevel(line);
            continue;
        }
        else if (isLineType(AL_INDICATOR_STRING, line))
        {
            if (isLineType(AUDIO_INDICATOR_STRING, line))
                ParseALSettings(aInFile, &aTermSettings->iAlSettings[AUDIO]);
            else if (isLineType(VIDEO_INDICATOR_STRING, line))
                ParseALSettings(aInFile, &aTermSettings->iAlSettings[VIDEO]);
            continue;
        }
        else if (isLineType(AL_INDICATOR_STRING, line))
        {
            if (isLineType(AUDIO_INDICATOR_STRING, line))
                ParseALSettings(aInFile, &aTermSettings->iAlSettings[AUDIO]);
            else if (isLineType(VIDEO_INDICATOR_STRING, line))
                ParseALSettings(aInFile, &aTermSettings->iAlSettings[VIDEO]);
            continue;
        }
        else if (isLineType(AL2_SN_INDICATOR_STRING, line))
        {
            aTermSettings->iAl2SequenceNumbers = GetIntFromText(line);
        }
        else if (isLineType(AL3_CFO_INDICATOR_STRING, line))
        {
            aTermSettings->iAl3ControlFieldOctets = GetIntFromText(line);
        }
        else if (isLineType(TERMINAL_INDICATOR_STRING, line))
        {
            /* Another terminal statemnt has been hit*/
            aInFile.seekg(-(lActualLineLength + 2), ios::cur);
            break;
        }
        else if (isLineType(MAXSDUSIZE_INDICATOR_STRING, line))
        {
            ParseMaxSDUSizes(aInFile, &aTermSettings->iSDUSizeSettings[0]);
            continue;
        }
        else if (isLineType(MAXSDUSIZER_INDICATOR_STRING, line))
        {
            ParseMaxSDUSizes(aInFile, &aTermSettings->iSDUSizeSettings[1]);
            continue;
        }
        else if (isLineType(MSD_INDICATOR_STRING, line))
        {
            if (isLineType(MASTER_INDICATOR_STRING, line))
            {
                aTermSettings->iMSD = PVT_MASTER;
            }
            if (isLineType(SLAVE_INDICATOR_STRING, line))
            {
                aTermSettings->iMSD = PVT_SLAVE;
            }
            continue;
        }
        else if (isLineType(CAPS_INDICATOR_STRING, line))
        {
            if (isLineType(RECEIVE_CAPS_INDICATOR_STRING, line))
            {
                ParseCaps(aInFile, &aTermSettings->iCapSettings[RECEIVE]);
            }
            if (isLineType(SEND_CAPS_INDICATOR_STRING, line))
            {
                ParseCaps(aInFile, &aTermSettings->iCapSettings[SEND]);
            }
            continue;
        }
        else if (isLineType(USER_INPUT_CAPABILITY_STRING, line))
        {
            GetProperLineFromFile(aInFile, line);
            if (isLineType(TRANSMIT_INDICATOR_STRING, line))
            {
                aTermSettings->iInputCapability = TerminalSettings::Transmit;
            }
            else if (isLineType(RECEIVE_TRANSMIT_INDICATOR_STRING, line))
            {
                aTermSettings->iInputCapability = TerminalSettings::ReceiveAndTransmit;
            }
            else if (isLineType(RECEIVE_INDICATOR_STRING, line))
            {
                aTermSettings->iInputCapability = TerminalSettings::Receive;
            }
        }
        else
        {
            cout << line << endl;
        }
    }
    return 0;
}

ParseTestFile(ifstream& aInFile , TestSettings::TestTerminalType terminalType,
              TestSettings *aSettings)
{
    TestSettings *lTestSettings = aSettings;
    string line;
    while (aInFile.good())
    {
        GetProperLineFromFile(aInFile, line);
        if (isLineType(NAME_INDICATOR_STRING, line))
        {
            lTestSettings->iName = OSCL_ARRAY_NEW(char, line.length() + 1);
            strcpy(lTestSettings->iName, line.c_str());
            continue;
        }
        else if (isLineType(PRIORITY_INDICATOR_STRING, line))
        {
            if (isLineType(PRIORITY_MAND_INDICATOR_STRING, line))
                lTestSettings->iPrio = TestSettings::TestMandatory;
            else
                lTestSettings->iPrio = TestSettings::TestOptional;
            continue;
        }
        else if (isLineType(TERMINAL_INDICATOR_STRING, line))
        {
            TestSettings::TestTerminalType lTermType;
            if (isLineType(TERMINALA_INDICATOR_STRING, line))
                lTermType = TestSettings::TestTerminalA;
            else
                lTermType = TestSettings::TestTerminalB;
            if (lTermType == terminalType)
            {
                lTestSettings->iTerminalType = terminalType;
                ParseTerminalSettings(aInFile, &lTestSettings->iTerminalSettings);
            }
            continue;
        }
    }
    return 0;
}

void ParseFile(const char *aFileName, TestSettings::TestTerminalType terminalType, TestSettings *aTestSettings)
{
    cout << "Processing File:" << aFileName << endl;
    string line;

    ifstream lInFile(aFileName, ios::in);
    while (lInFile.good())
    {
        int lActualLineLength = GetProperLineFromFile(lInFile, line);
        if (isLineType(FILE_INDICATOR_STRING, line))
        {
            ParseFile(line.c_str(), terminalType, aTestSettings);
        }
        else
        {   /* fill the various values */
            ifstream lDefInFile("DefSettings.txt", ios::in);
            if (lDefInFile.good())
            {
                ParseTestFile(lDefInFile, terminalType, aTestSettings);
            }
            lInFile.seekg(-(lActualLineLength + 2), ios::cur);
            ParseTestFile(lInFile, terminalType, aTestSettings);
        }
    }
    lInFile.close();
}

bool ParseConFile(PvmiMIOFileInputSettings& aCapSettings)
{
    return ParseConFile(aCapSettings.iFileName.get_cstr(), aCapSettings);
}

bool ParseConFile(const oscl_wchar* aFileName, PvmiMIOFileInputSettings& aCapSettings)
{
    string line;

    char configfile[FILENAME_LEN];

    oscl_UnicodeToUTF8(aFileName, oscl_strlen(aFileName), configfile, (FILENAME_LEN - EXTN_LEN));

    oscl_strcat(configfile, ".con");
    ifstream lInFile(configfile, ios::in);

    cout << "Processing con  File:" << configfile << endl;

    if (lInFile.good() && GetProperLineFromFile(lInFile, line) && isLineType(FILE_CAPABILITES_INDICATOR_STRING, line))
    {
        while (lInFile.good())
        {
            GetProperLineFromFile(lInFile, line);
            if (isLineType(FILE_CAPABILITIES_MEDIATYPE_STRING, line))
            {
                if (isLineType(AUDIO_INDICATOR_STRING, line))
                {
                    while (lInFile.good())
                    {
                        GetProperLineFromFile(lInFile, line);
                        if (isLineType(AUDIO_SAMPLING_FREQUENCY_STRING, line))
                        {
                            aCapSettings.iSamplingFrequency = atoi(line.c_str());
                            continue;
                        }
                        else if (isLineType(AUDIO_NUM_CHANNELS_STRING, line))
                        {
                            aCapSettings.iNumChannels = atoi(line.c_str());
                            continue;

                        }
                        else if (isLineType(AUDIO_LOOP_INPUT_STRING, line))
                        {
                            if (isLineType(ON_INDICATOR_STRING, line))
                            {
                                aCapSettings.iLoopInputFile = 1;
                            }
                            else
                            {
                                aCapSettings.iLoopInputFile = 0;
                            }
                            continue;
                        }

                    }
                }
                else if (isLineType(VIDEO_INDICATOR_STRING, line))
                {
                    while (lInFile.good())
                    {
                        GetProperLineFromFile(lInFile, line);
                        if (isLineType(VIDEO_TIMESCALE_STRING, line))
                        {
                            aCapSettings.iTimescale = atoi(line.c_str());
                            continue;
                        }
                        else if (isLineType(VIDEO_FRAME_HEIGHT_STRING, line))
                        {
                            aCapSettings.iFrameHeight = atoi(line.c_str());
                            continue;

                        }
                        else if (isLineType(VIDEO_FRAME_WIDTH_STRING, line))
                        {
                            aCapSettings.iFrameWidth = atoi(line.c_str());
                            continue;
                        }
                        else if (isLineType(VIDEO_FRAMERATE_STRING, line))
                        {
                            aCapSettings.iFrameRate = atof(line.c_str());
                            continue;
                        }
                        else if (isLineType(VIDEO_LOOP_INPUT_STRING, line))
                        {
                            if (isLineType(ON_INDICATOR_STRING, line))
                            {
                                aCapSettings.iLoopInputFile = 1;
                            }
                            else
                            {
                                aCapSettings.iLoopInputFile = 0;
                            }
                            continue;
                        }
                        /*                      else if(isLineType(VIDEO_FRAME_RATE_SIMULATION_STRING,line))
                                            {
                                                if (isLineType(ON_INDICATOR_STRING, line))
                                                {
                                                    aCapSettings.iFrameRateSimulation = 1;
                                                }
                                                else
                                                {
                                                    aCapSettings.iFrameRateSimulation = 0;
                                                }
                                                continue;

                                                }*/
                    }
                }
            }
        }

        lInFile.close();
        return true;
    }
    lInFile.close();
    return false;
}

