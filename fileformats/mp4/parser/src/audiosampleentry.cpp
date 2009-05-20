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
/*********************************************************************************/
/*     -------------------------------------------------------------------       */
/*                        MPEG-4 AudioSampleEntry Class                          */
/*     -------------------------------------------------------------------       */
/*********************************************************************************/
/*
    This AudioSampleEntry Class is used for visual streams.
*/

#define IMPLEMENT_AudioSampleEntry

#include "audiosampleentry.h"
#include "atomutils.h"
#include "atomdefs.h"


// Stream-in ctor
AudioSampleEntry::AudioSampleEntry(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : SampleEntry(fp, size, type)
{
    _pes = NULL;
    _pparent = NULL;

    if (_success)
    {
        // Read reserved values
        if (!AtomUtils::read32read32(fp, _reserved1[0], _reserved1[1]))
            _success = false;
        if (!AtomUtils::read16read16(fp, _channelCount, _sampleSize))
            _success = false;
        if (!AtomUtils::read16read16(fp, _preDefined, _reserved))
            _success = false;

        if (!AtomUtils::read16read16(fp, _timeScale, _sampleRateLo)) //_timeScale and _sampleRateHi are same
            _success = false;

        _sampleRateHi = _timeScale;


        if (_success)
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;

            AtomUtils::getNextAtomType(fp, atomSize, atomType);

            if (atomType == ESD_ATOM)
            {
                PV_MP4_FF_NEW(fp->auditCB, ESDAtom, (fp, atomSize, atomType), _pes);

                if (!_pes->MP4Success())
                {
                    _success = false;
                    _mp4ErrorCode = _pes->GetMP4Error();
                }
                else
                {
                    _pes->setParent(this);
                }
            }
            else
            {
                _success = false;
                _mp4ErrorCode = READ_AUDIO_SAMPLE_ENTRY_FAILED;
            }
        }
        else
        {
            _mp4ErrorCode = READ_AUDIO_SAMPLE_ENTRY_FAILED;
        }
    }
    else
    {
        _mp4ErrorCode = READ_AUDIO_SAMPLE_ENTRY_FAILED;
    }

}

// Destructor
AudioSampleEntry::~AudioSampleEntry()
{
    if (_pes != NULL)
    {
        // Cleanup ESDAtom
        PV_MP4_FF_DELETE(NULL, ESDAtom, _pes);
    }
}

SpeechSampleEntry3GPP2::SpeechSampleEntry3GPP2(MP4_FF_FILE *fp, uint32 size, uint32 type)
        : Atom(fp, size, type)
{
    iMimeType = PVMF_MIME_FORMAT_UNKNOWN;
    _data_reference_index = 0;
    _timeScale = 0;
    _vendor = 0;
    _decoder_version = 0;
    _frames_per_sample = 0;
    _mode_set = 0;
    _media_sampling_frequency = 0;

    if (_success)
    {
        SetMimeType(type);

        //calculate this in case we need to recover from some parsing errors
        uint32 end_of_atom = AtomUtils::getCurrentFilePosition(fp) + (size - DEFAULT_ATOM_SIZE);

        uint32 count = DEFAULT_ATOM_SIZE;
        //skip Reserved_6
        AtomUtils::seekFromCurrPos(fp, 6);
        count += 6;
        _success = false;
        if (AtomUtils::read16(fp, _data_reference_index))
        {
            count += 2;
            //skip Reserved_8, Reserved_2, Reserved_2, Reserved_4 (total 16 bytes)
            AtomUtils::seekFromCurrPos(fp, 16);
            count += 16;
            if (AtomUtils::read16(fp, _timeScale))
            {
                count += 2;
                _success = true;
            }
        }
        if ((_success) && (count < size))
        {
            uint32 atomType = UNKNOWN_ATOM;
            uint32 atomSize = 0;
            AtomUtils::getNextAtomType(fp, atomSize, atomType);
            if ((atomType == EVRC_SPECIFIC_BOX) ||
                    (atomType == EVRCB_SPECIFIC_BOX) ||
                    (atomType == EVRCWB_SPECIFIC_BOX) ||
                    (atomType == SMV_SPECIFIC_BOX))
            {
                _success = false;
                if (AtomUtils::read32(fp, _vendor))
                {
                    if (AtomUtils::read8(fp, _decoder_version))
                    {
                        if (AtomUtils::read8(fp, _frames_per_sample))
                        {
                            _success = true;
                        }
                    }
                }
            }
            else if (atomType == VMR_SPECIFIC_BOX)
            {
                _success = false;
                if (AtomUtils::read32(fp, _vendor))
                {
                    if (AtomUtils::read8(fp, _decoder_version))
                    {
                        if (AtomUtils::read16(fp, _mode_set))
                        {
                            if (AtomUtils::read8(fp, _media_sampling_frequency))
                            {
                                if (AtomUtils::read8(fp, _frames_per_sample))
                                {
                                    _success = true;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                //skip unknown atom
                if (atomSize < DEFAULT_ATOM_SIZE)
                {
                    _success = false;
                }
                else
                {
                    atomSize -= DEFAULT_ATOM_SIZE;
                    AtomUtils::seekFromCurrPos(fp, atomSize);
                }
            }
            //if we were not able to parse the decoder specific box
            //or if there is still data at the end of the atom (there should not be)
            //just get to the end of the atom so that we can continue parsing the file.
            //we really do not need anything from decoder specific box as far as play
            //back goes
            if ((_success == false) || (count < size))
            {
                _success = true;
                AtomUtils::seekFromStart(fp, end_of_atom);
            }
        }
    }
    if (!_success)
    {
        _mp4ErrorCode = READ_3GPP2_SPEECH_SAMPLE_ENTRY_FAILED;
    }
}

void SpeechSampleEntry3GPP2::SetMimeType(uint32 aBoxType)
{
    if (aBoxType == EVRC_SAMPLE_ENTRY)
    {
        iMimeType = PVMF_MIME_EVRC;
    }
    else if (aBoxType == EVRCB_SAMPLE_ENTRY)
    {
        iMimeType = PVMF_MIME_EVRCB;
    }
    else if (aBoxType == EVRCWB_SAMPLE_ENTRY)
    {
        iMimeType = PVMF_MIME_EVRCWB;
    }
    else if (aBoxType == QCELP_SAMPLE_ENTRY)
    {
        iMimeType = PVMF_MIME_QCELP;
    }
    else if (aBoxType == SMV_SAMPLE_ENTRY)
    {
        iMimeType = PVMF_MIME_SMV;
    }
    else if (aBoxType == VMR_SAMPLE_ENTRY)
    {
        iMimeType = PVMF_MIME_VMRWB;
    }
    else
    {
        iMimeType = PVMF_MIME_FORMAT_UNKNOWN;
    }
}





