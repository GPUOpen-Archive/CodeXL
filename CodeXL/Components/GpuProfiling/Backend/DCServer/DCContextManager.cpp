//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include <new>
#include <assert.h>
#include "DCContextManager.h"
#include "DCUtils.h"
//#include "DCFuncDefs.h"
#include "DCID3D11Device_wrapper.h"
#include "DCID3D11DeviceContext_wrapper.h"
#include "..\Common\Logger.h"

using std::vector;
using std::map;
using std::nothrow;
using namespace GPULogger;

typedef vector<map<ID3D11Resource*, ID3D11Resource*>::iterator> ResourceIteratorList;
typedef vector<map<ID3D11UnorderedAccessView*, ID3D11UnorderedAccessView*>::iterator> UAVIteratorList;

DCContextManager::DCContextManager(void)
{
    m_pmutex = new AMDTMutex("ContextManager Mutex");

    m_arrActiveUAVs[0] = NULL;
    m_arrActiveUAVs[1] = NULL;
    m_arrActiveUAVs[2] = NULL;
    m_arrActiveUAVs[3] = NULL;
    m_arrActiveUAVs[4] = NULL;
    m_arrActiveUAVs[5] = NULL;
    m_arrActiveUAVs[6] = NULL;
    m_arrActiveUAVs[7] = NULL;

    m_arrUAVOffset[0] = 0;
    m_arrUAVOffset[1] = 0;
    m_arrUAVOffset[2] = 0;
    m_arrUAVOffset[3] = 0;
    m_arrUAVOffset[4] = 0;
    m_arrUAVOffset[5] = 0;
    m_arrUAVOffset[6] = 0;
    m_arrUAVOffset[7] = 0;

    m_arrOffsetEnable[0] = false;
    m_arrOffsetEnable[1] = false;
    m_arrOffsetEnable[2] = false;
    m_arrOffsetEnable[3] = false;
    m_arrOffsetEnable[4] = false;
    m_arrOffsetEnable[5] = false;
    m_arrOffsetEnable[6] = false;
    m_arrOffsetEnable[7] = false;

    m_pID3D11Device = NULL;
    m_pImmediateDeviceContext = NULL;
    m_pCurrentCS = NULL;
    m_uiRefCounter = 0;
}

DCContextManager::~DCContextManager(void)
{
    if (m_pmutex)
    {
        delete m_pmutex;
    }
}

bool DCContextManager::PushContext()
{
    ID3D11DeviceContext* unwrappedDC = GetRealDeviceContext11(m_pImmediateDeviceContext);
    ID3D11UnorderedAccessView* backUAV = NULL;

    for (UINT i = 0; i < D3D11_PS_CS_UAV_REGISTER_COUNT; i++)
    {
        //copy the backup UAV to the array and call CSSetUnorderedAccessViews()
        if (m_arrActiveUAVs[i] != NULL)
        {
            //try to find the back up UAV
            UAVMap::iterator mapIt = m_UAVTable.find(m_arrActiveUAVs[i]);

            if (mapIt != m_UAVTable.end())
            {
                if (mapIt->second != NULL)
                {
                    backUAV = mapIt->second;
                }
                else
                {
                    //backup UAV is missing, in this case, just use the original UAV
                    //possible reason 1, back up resource creation failed, but AddtoUAVTable() is called
                    Log(logWARNING, "DCContextManager::PushContext() : back up resource creation failed\n");
                    backUAV = m_arrActiveUAVs[i];
                }
            }
            else
            {
                //for some reason, backup UAV is missing, shouldn't happen
                Log(logWARNING, "DCContextManager::PushContext() : back up UAV missing\n");
                backUAV = m_arrActiveUAVs[i];
            }

        }
        else//client may pass NULL to disable UAV binding slots
        {
            backUAV = NULL;
        }

        // Set UAV one by one
        if (m_arrOffsetEnable[i])
        {
            UINT offsets[] = {m_arrUAVOffset[i]};
            unwrappedDC->CSSetUnorderedAccessViews(i,
                                                   1,
                                                   &backUAV,
                                                   offsets);
        }
        else
        {
            unwrappedDC->CSSetUnorderedAccessViews(i,
                                                   1,
                                                   &backUAV,
                                                   NULL);
        }
    }

    return true;
}

bool DCContextManager::PopContext()
{
    ID3D11DeviceContext* pUnwrappedDC = GetRealDeviceContext11(m_pImmediateDeviceContext);
    ID3D11UnorderedAccessView* originalUAV = NULL;

    for (UINT i = 0; i < D3D11_PS_CS_UAV_REGISTER_COUNT; i++)
    {
        //copy the original UAV to the array
        if (m_arrActiveUAVs[i] != NULL)
        {
            originalUAV = m_arrActiveUAVs[i];
        }
        else
        {
            originalUAV = NULL;
        }

        // Set UAV one by one
        if (m_arrOffsetEnable[i])
        {
            UINT offsets[] = {m_arrUAVOffset[i]};
            pUnwrappedDC->CSSetUnorderedAccessViews(i,
                                                    1,
                                                    &originalUAV,
                                                    offsets);
        }
        else
        {
            pUnwrappedDC->CSSetUnorderedAccessViews(i,
                                                    1,
                                                    &originalUAV,
                                                    NULL);
        }
    }

    return true;
}

bool DCContextManager::RestoreContext()
{
    bool res = true;

    for (UINT i = 0; i < D3D11_PS_CS_UAV_REGISTER_COUNT; i++)
    {
        if (m_arrActiveUAVs[i] != NULL)
        {
            res = res && UpdateBackupResource(m_arrActiveUAVs[i]);
        }
    }

    return res;
}

ULONG DCContextManager::Cleanup()
{
    ULONG ret = 0;
    ULONG refCount = 0;
    ResourceIteratorList toBeRemovedResourcesList;
    UAVIteratorList toBeRemovedUAVsList;

    for (ResourceMap::iterator it = m_ResxMap.begin() ; it != m_ResxMap.end(); it++)
    {
        it->first->AddRef();
        refCount = it->first->Release();

        if (refCount <= 1) //original one has been released by client
        {
            it->first->Release();

            if (it->second)
            {
                it->second->Release();
                ret++;
                toBeRemovedResourcesList.push_back(it);
            }
        }
    }

    //m_ResxMap.clear();
    for (UAVMap::iterator it = m_UAVTable.begin() ; it != m_UAVTable.end(); it++)
    {
        it->first->AddRef();
        refCount = it->first->Release();

        if (refCount <= 1) //original one has been released by client
        {
            it->first->Release();

            if (it->second)
            {
                it->second->Release();
                ret++;
                toBeRemovedUAVsList.push_back(it);
            }
        }
    }

    for (ResourceIteratorList::iterator it = toBeRemovedResourcesList.begin(); it != toBeRemovedResourcesList.end(); it++)
    {
        m_ResxMap.erase(*it);
        m_uiRefCounter--;
    }

    for (UAVIteratorList::iterator it = toBeRemovedUAVsList.begin(); it != toBeRemovedUAVsList.end(); it++)
    {
        m_UAVTable.erase(*it);
        m_uiRefCounter--;
    }

    //m_UAVTable.clear();
    // called release twice
    return ret * 2;
}

void DCContextManager::SaveUAV(UINT StartSlot,
                               UINT NumUAVs,
                               ID3D11UnorderedAccessView* const* ppUnorderedAccessViews,
                               const UINT* pUAVInitialCounts)
{
    // Update context
    for (UINT i = 0; i < NumUAVs; i++)
    {
        m_arrActiveUAVs[ i + StartSlot ] = ppUnorderedAccessViews[ i ];

        if (pUAVInitialCounts != NULL)
        {
            m_arrOffsetEnable [ i + StartSlot ] = true;
            m_arrUAVOffset [ i + StartSlot ] = pUAVInitialCounts[ i ];
        }
        else
        {
            m_arrOffsetEnable [ i + StartSlot ] = false;
        }
    }
}


bool DCContextManager::UpdateBackupResource(ID3D11UnorderedAccessView* originalUAV)
{
    ID3D11UnorderedAccessView* backupUAV;
    //try to find backup UAV
    map<ID3D11UnorderedAccessView*, ID3D11UnorderedAccessView*>::iterator mapIt = m_UAVTable.find(originalUAV);

    if (mapIt != m_UAVTable.end())
    {
        backupUAV = mapIt->second;

        if (backupUAV == NULL)
        {
            // backup UAV is missing
            // Shouldn't happen
            Log(logERROR, "DCContextManager::UpdateBackupResource() : backup UAV is missing\n");
            return false;
        }
    }
    else
    {
        // for some reason, UAV is not maintained in the list
        // Shouldn't happen, consider it as a bug
        Log(logERROR, "DCContextManager::UpdateBackupResource() : UAV pair is missing\n");
        return false;
    }

    //if find backup UAV, get the buffer associated with original UAV
    ID3D11Resource* oriRes;
    originalUAV->GetResource(&oriRes);

    ID3D11Resource* backupRes = NULL;

    if (oriRes != NULL)
    {
        //find the backup buffer
        map<ID3D11Resource*, ID3D11Resource*>::iterator resxIt = m_ResxMap.find(oriRes);

        if (resxIt != m_ResxMap.end())
        {
            backupRes = resxIt->second;
            m_pImmediateDeviceContext->CopyResource(backupRes, oriRes);

            if (backupRes == NULL)
            {
                Log(logWARNING, "DCContextManager::UpdateBackupResource() : CopyResource failed\n");
                return false;
            }

            resxIt->second = backupRes;
        }
        else
        {
            //for some reason, backup buffer/tex is missing, don't do copy, simply return
            Log(logWARNING, "DCContextManager::UpdateBackupResource() : backup buffer/tex is missing\n");
            backupRes = NULL;
            return false;
        }

    }
    else
    {
        //shouldn't happen
        SpAssert(!"client UAV is broken");
        return false;
    }


    backupUAV->Release();
    backupUAV = NULL;

    oriRes->Release();

    D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
    originalUAV->GetDesc(&UAVDesc);

    ID3D11Device* pUnwrappedDevice = GetRealDevice11(m_pID3D11Device);
    pUnwrappedDevice->CreateUnorderedAccessView(backupRes, &UAVDesc, &backupUAV);

    m_UAVTable.find(originalUAV)->second = backupUAV;

    return true;
}

ID3D11Texture1D* DCContextManager::CreateBackupTextur1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData)
{
    ID3D11Texture1D* backupTex;
    HRESULT hr;
    ID3D11Device* pUnwrappedDevice = GetRealDevice11(m_pID3D11Device);
    hr = pUnwrappedDevice->CreateTexture1D(pDesc, pInitialData, &backupTex);

    if (SUCCEEDED(hr))
    {
        m_uiRefCounter++;
    }
    else
    {
        SpAssert(!"Backup CreateBackupTextur1D() creation failed");
        return NULL;
    }

    return backupTex;
}

ID3D11Texture2D* DCContextManager::CreateBackupTextur2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData)
{
    ID3D11Texture2D* backupTex;
    HRESULT hr;
    ID3D11Device* pUnwrappedDevice = GetRealDevice11(m_pID3D11Device);
    hr = pUnwrappedDevice->CreateTexture2D(pDesc, pInitialData, &backupTex);

    if (SUCCEEDED(hr))
    {
        m_uiRefCounter++;
    }
    else
    {
        SpAssert(!"Backup CreateBackupTextur2D() creation failed");
        return NULL;
    }

    return backupTex;
}

ID3D11Texture3D* DCContextManager::CreateBackupTextur3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData)
{
    ID3D11Texture3D* backupTex;
    HRESULT hr;
    ID3D11Device* pUnwrappedDevice = GetRealDevice11(m_pID3D11Device);
    hr = pUnwrappedDevice->CreateTexture3D(pDesc, pInitialData, &backupTex);

    if (SUCCEEDED(hr))
    {
        m_uiRefCounter++;
    }
    else
    {
        SpAssert(!"Backup CreateBackupTextur3D() creation failed");
        return NULL;
    }

    return backupTex;
}

ID3D11Buffer* DCContextManager::CreateBackupBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData)
{
    ID3D11Buffer* backupBuffer;
    HRESULT hr;
    ID3D11Device* pUnwrappedDevice = GetRealDevice11(m_pID3D11Device);
    hr = pUnwrappedDevice->CreateBuffer(pDesc, pInitialData, &backupBuffer);

    if (SUCCEEDED(hr))
    {
        m_uiRefCounter++;
    }
    else
    {
        SpAssert(!"Backup CreateBackupBuffer() creation failed");
        return NULL;
    }

    return backupBuffer;
}

ID3D11UnorderedAccessView* DCContextManager::CreateBackupUAV(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc)
{
    ID3D11UnorderedAccessView* backUAV;
    ID3D11Resource* pBackupBuffer = GetBackupResource(pResource);
    HRESULT hr;
    ID3D11Device* pUnwrappedDevice = GetRealDevice11(m_pID3D11Device);

    if (pBackupBuffer == NULL)
    {
        //backup buffer is missing
        //we need to create one here
        /*
        Log(logWARNING, "CreateBackupUAV():backup buffer is missing\n");
        return NULL;
        /*/
        pBackupBuffer = DCUtils::CloneResource(m_pImmediateDeviceContext, pResource);
        m_uiRefCounter++;
        AddBackupResource(pResource, pBackupBuffer);
        //*/
    }

    //create backup UAV
    hr = pUnwrappedDevice->CreateUnorderedAccessView(pBackupBuffer, pDesc, &backUAV);

    if (SUCCEEDED(hr))
    {
        m_uiRefCounter++;
    }
    else
    {
        //Backup UAV creation failed
        SpAssert(!"Backup UAV creation failed");
        return NULL;
    }

    return backUAV;
}

void DCContextManager::SetCurrentComputeShader(ID3D11ComputeShader* pCS)
{
    m_pCurrentCS = pCS;
}

ID3D11ComputeShader* DCContextManager::GetCurrentComputeShader() const
{
    return m_pCurrentCS ;
}

void DCContextManager::AddBackupResource(ID3D11Resource* original, ID3D11Resource* copy)
{
    AMDTScopeLock lock(m_pmutex);
    std::map<ID3D11Resource*, ID3D11Resource*>::iterator it = m_ResxMap.find(original);

    if (it != m_ResxMap.end())
    {
        // try to recover from error
        // remove the existing entry
        // remove the backup resources because the original resources has already been released
        /*it->second->Release();
        m_uiRefCounter--;
        m_ResxMap.erase(it);*/
        Cleanup();
    }

    original->AddRef();
    m_ResxMap.insert(std::pair<ID3D11Resource*, ID3D11Resource*>(original, copy));
}

ID3D11Resource* DCContextManager::GetBackupResource(ID3D11Resource* original)
{
    ResourceMap::iterator it = m_ResxMap.find(original);

    if (it != m_ResxMap.end())
    {
        return it->second;
    }
    else
    {
        // Back up resource is not created
        // Because:
        // 1. Original resource is IMMUTABLE or
        // 2. Bug?
        Log(logWARNING, "Back up resource is missing\n");
        return NULL;
    }
}

void DCContextManager::AddtoUAVTable(ID3D11UnorderedAccessView* original, ID3D11UnorderedAccessView* copy)
{
    AMDTScopeLock lock(m_pmutex);
    std::map<ID3D11UnorderedAccessView*, ID3D11UnorderedAccessView*>::iterator it = m_UAVTable.find(original);

    if (it != m_UAVTable.end())
    {
        // try to recover from error
        // remove the existing entry
        // remove the backup resources because the original resources has already been released
        /*it->second->Release();
        m_uiRefCounter--;
        m_UAVTable.erase(it);*/
        Cleanup();
    }

    m_UAVTable.insert(std::pair<ID3D11UnorderedAccessView*, ID3D11UnorderedAccessView*>(original, copy));
    original->AddRef();
}

