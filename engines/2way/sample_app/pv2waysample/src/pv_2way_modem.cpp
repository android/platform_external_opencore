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
#include "pv_2way_modem.h"


PV2WayModem::~PV2WayModem()
{
    OSCL_DELETE(iModem);
}

bool PV2WayModem::Create(HANDLE aComHandle)
{
    iModem = PVCom3GModem::NewL(this);
    iModem->Initialize(aComHandle);

    // listening to you
    iModem->StartListening();

    return true;
}

void PV2WayModem::UpdateConnection()
{
    if (iModem)
        iModem->UpdateConnection();
}

void PV2WayModem::Stop()
{

    if (iModem)
    {
        iModem->StopListening();
    }

    return;
}

void PV2WayModem::Reset()
{
    if (iModem)
    {
        iModem->Hangup();
    }
    return;
}

void PV2WayModem::Remove()
{

    Delete();
}

void PV2WayModem::Delete()
{
    if (iModem)
    {
        iModem->Close();
        OSCL_DELETE(iModem);
        iModem = NULL;
    }
}

void PV2WayModem::ModemStatus(PVComModemStatus::PVComModemStatusType aStatus)
{
    iComModemStatus->ModemStatus(aStatus);
}

void PV2WayModem::Call(const char* aText, int aTextLength)
{
    /// calling
    iModem->MakeCall(aText, aTextLength);
}

void PV2WayModem::Answer()
{
    iModem->Answer();
}

DWORD PV2WayModem::GetStatus()
{
    return iModem->ReportModemEvent();
}



