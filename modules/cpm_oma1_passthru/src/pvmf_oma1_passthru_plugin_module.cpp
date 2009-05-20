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

#include "oscl_base.h"
#include "pvmf_cpmplugin_interface.h"
#include "pvmf_cpmplugin_passthru_oma1_factory.h"
#include "oscl_shared_lib_interface.h"
#include "pvmf_cpmplugin_passthru_oma1_types.h"
#include "oscl_mem.h"

/*
** This module implements PVMF_CPMPLUGIN_PASSTHRU_OMA1_FACTORY
*/

class PvmfOma1PassthruPluginPopulator: public PVMFCPMPluginRegistryPopulator
{
    public:
        PvmfOma1PassthruPluginPopulator();
        ~PvmfOma1PassthruPluginPopulator();

        //from PVMFCPMPluginRegistryPopulator
        PVMFCPMPluginFactory* GetFactoryAndMimeString(OSCL_String& aMimestring);
        void ReleaseFactory();

    private:
        PVMFOma1PassthruPluginFactory* iImpl;
};

PvmfOma1PassthruPluginPopulator::PvmfOma1PassthruPluginPopulator()
{
    iImpl = NULL;
}

PvmfOma1PassthruPluginPopulator::~PvmfOma1PassthruPluginPopulator()
{
    if (iImpl)
        OSCL_DELETE(iImpl);
}

PVMFCPMPluginFactory* PvmfOma1PassthruPluginPopulator::GetFactoryAndMimeString(OSCL_String& aMimestring)
{
    aMimestring = PVMF_CPM_MIME_PASSTHRU_OMA1;
    if (!iImpl)
    {
        iImpl = OSCL_NEW(PVMFOma1PassthruPluginFactory, ());
    }
    return iImpl;
}

void PvmfOma1PassthruPluginPopulator::ReleaseFactory()
{
    if (iImpl)
    {
        OSCL_DELETE(iImpl);
        iImpl = NULL;
    }
}

class PvmfOma1PassthruPluginModule: public OsclSharedLibraryInterface
{
    public:
        PvmfOma1PassthruPluginModule()
                : iImpl(NULL)
        {}

        ~PvmfOma1PassthruPluginModule();

        OsclAny* SharedLibraryLookup(const OsclUuid& aInterfaceId);

    private:
        PvmfOma1PassthruPluginPopulator* iImpl;
};

PvmfOma1PassthruPluginModule::~PvmfOma1PassthruPluginModule()
{
    if (iImpl)
        OSCL_DELETE(iImpl);
}

OsclAny* PvmfOma1PassthruPluginModule::SharedLibraryLookup(const OsclUuid& aInterfaceId)
{
    if (PVMF_CPM_PLUGIN_REGISTRY_POPULATOR_UUID == aInterfaceId)
    {
        if (!iImpl)
            iImpl = OSCL_NEW(PvmfOma1PassthruPluginPopulator, ());
        return iImpl;
    }
    return NULL;
}

//Module entry points.
extern "C"
{
    OSCL_EXPORT_REF OsclSharedLibraryInterface* PVGetInterface(void)
    {
        return OSCL_NEW(PvmfOma1PassthruPluginModule, ());
    }

    OSCL_EXPORT_REF void PVReleaseInterface(OsclSharedLibraryInterface* aInstance)
    {
        PvmfOma1PassthruPluginModule* module = (PvmfOma1PassthruPluginModule*)aInstance;
        OSCL_DELETE(module);
    }
}






