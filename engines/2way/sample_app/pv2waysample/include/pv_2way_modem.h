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
#ifndef PV_2WAY_MODEM_H_INCLUDED
#define PV_2WAY_MODEM_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
// This file includes the class definition for the PV2WayModem.  This class
// encapsulates all modem calls.
//
////////////////////////////////////////////////////////////////////////////

#include "oscl_thread.h"
#include "oscl_error.h"
#include "oscl_socket.h"
#include "oscl_timer.h"
#include "oscl_mem.h"

#include "pv_com_3g_modem.h"


class PV2WayModem : public PVComModemStatus
{
    public:
        PV2WayModem(PVComModemStatus *aComModemStatus)
                :
                iModem(NULL),
                iComModemStatus(aComModemStatus)
        {
        }

        ~PV2WayModem();

        void UpdateConnection();

        bool Create(HANDLE aComHandle);
        void Stop();
        void Reset();
        void Remove();
        void Call(const char* aText, int aTextLength);
        void Answer();
        DWORD GetStatus();
        void Delete();

        /**
         * From PVComModemStatus
         **/

        /**
         * Get modem status, used to receive dial signal
         * @param aStatus Type of modem even
         **/
        void ModemStatus(PVComModemStatus::PVComModemStatusType aStatus);

    private:

        PVCom3GModem* iModem;
        PVComModemStatus *iComModemStatus;

};

#endif
