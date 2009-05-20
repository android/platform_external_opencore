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
#ifndef PV_2WAY_CONSOLE_SOURCE_AND_SINKS_H
#define PV_2WAY_CONSOLE_SOURCE_AND_SINKS_H
////////////////////////////////////////////////////////////////////////////
// This file includes the class definition for all MIOs (sources and sinks)
// for the console application
//
// This class initializes the MIOs with the appropriate values (see Initialize
//  functions).
//
//
////////////////////////////////////////////////////////////////////////////


#include "pv_2way_engine_factory.h"
#include "pvmf_fileoutput_factory.h"

#include "pv_2way_source_and_sinks_base.h"
#include "pv_2way_mio.h"



class PV2WayUnitTestSourceAndSinks : public PV2WaySourceAndSinksBase
{
    public:

        PV2WayUnitTestSourceAndSinks(PV2WaySourceAndSinksObserver* aObserver,
                                     PV2Way324InitInfo& aSdkInitInfo);
        ~PV2WayUnitTestSourceAndSinks();

        int CreateCodecs();


    protected:
        void OutputInfo(const char * str, ...)
        {
            // output to screen everything in formatted string
            va_list ap;
            va_start(ap, str);
            vprintf(str, ap);
            va_end(ap);
        }

    private:

};


#endif
