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
#ifndef USER_INPUT_TEST_H_INCLUDED
#define USER_INPUT_TEST_H_INCLUDED

#include "test_base.h"
#include "tsc_h324m_config_interface.h"

class user_input_test : public test_base
{
    public:
        user_input_test(bool aUseProxy, bool aIsDTMF) :
                test_base(aUseProxy),
                iIsDTMF(aIsDTMF)
        {};

        ~user_input_test()
        {
        }

        void test();

        void Run();

        void DoCancel();


        void H324MConfigCommandCompletedL(PVMFCmdResp& aResponse);

        void H324MConfigHandleInformationalEventL(PVMFAsyncEvent& aNotification);


    private:
        virtual void RstCmdCompleted();
        virtual void DisCmdSucceeded();
        virtual void DisCmdFailed();
        virtual void ConnectSucceeeded();
        virtual void InitFailed();
        bool start_async_test();
        bool iIsDTMF;
        PVCommandId i324mIFCommandId, iUserInputId;

        CPVUserInput *iUserInput;
};


#endif


