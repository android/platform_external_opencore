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
#ifndef ACCEPTABLE_FORMATS_TEST_H_INCLUDED
#define ACCEPTABLE_FORMATS_TEST_H_INCLUDED
//
//
//
// NOTE: This file is a work-in-progress

#include "av_test.h"

#ifndef PV_2WAY_MIO_H_INCLUDED
#include "pv_2way_mio.h"
#endif

#include "pv_2way_test_extension_interface.h"

class acceptable_formats_test : public av_test
{
    public:
        acceptable_formats_test(bool aUseProxy = false)
                : av_test(aUseProxy),
                iQueryTestInterfaceCmdId(0),
                iTempTestConfigInterface(NULL),
                iTestConfigInterface(NULL)
        {
        }

        ~acceptable_formats_test()
        {
        }

        void AddExpectedFormat(TPVDirection aDir, PV2WayMediaType aType, const char* const aFormat);


        // need to store the expected formats that we'll find

    private:
        void CommandCompleted(const PVCmdResponse& aResponse);
        void CreateParts();
        virtual void InitSucceeded();
        PVCommandId iQueryTestInterfaceCmdId;
        PVInterface* iTempTestConfigInterface;
        PV2WayTestExtensionInterface* iTestConfigInterface;

        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator> iInAudFormatCapability;
        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator> iOutAudFormatCapability;
        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator> iInVidFormatCapability;
        Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator> iOutVidFormatCapability;
};


#endif


