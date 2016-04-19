//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file Wrapped_ID3D12CommandQueue.h
/// \brief A class used to wrap D3D12's ID3D12CommandQueue interface.
//=============================================================================

#ifndef WRAPPED_ID3D12COMMANDQUEUE_H
#define WRAPPED_ID3D12COMMANDQUEUE_H

#include "DX12Defines.h"

class Wrapped_ID3D12Device;
class Wrapped_ID3D12CommandQueueCreateInfo;

//-----------------------------------------------------------------------------
/// Utility function used to wrap the D3D12 interface.
/// \param inParentDevice The parent device for the interface.
/// \param inRealCommandQueue The real runtime instance of the D3D12 interface.
/// \param inCreateInfo A structure containing the interface creation info.
/// \returns True if the interface was wrapped successfully.
//-----------------------------------------------------------------------------
bool WrapD3D12CommandQueue(Wrapped_ID3D12Device* inParentDevice, ID3D12CommandQueue** inRealCommandQueue, Wrapped_ID3D12CommandQueueCreateInfo* inCreateInfo = nullptr);

//-----------------------------------------------------------------------------
/// A class used to wrap D3D12's ID3D12CommandQueue interface.
//-----------------------------------------------------------------------------
class Wrapped_ID3D12CommandQueue : public ID3D12CommandQueue
{
public:
    //-----------------------------------------------------------------------------
    /// Default constructor accepts the real runtime instance.
    /// \param inRealCommandQueue The real runtime instance being wrapped.
    //-----------------------------------------------------------------------------
    Wrapped_ID3D12CommandQueue(ID3D12CommandQueue* inRealCommandQueue) { mRealCommandQueue = inRealCommandQueue; }

    //-----------------------------------------------------------------------------
    /// Default destructor.
    //-----------------------------------------------------------------------------
    virtual ~Wrapped_ID3D12CommandQueue() {}

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // ID3D12Object
    virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData);
    virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData);
    virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData);
    virtual HRESULT STDMETHODCALLTYPE SetName(LPCWSTR Name);

    // ID3D12DeviceChild
    virtual HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppvDevice);

    // ID3D12CommandQueue
    virtual void STDMETHODCALLTYPE UpdateTileMappings(ID3D12Resource* pResource, UINT NumResourceRegions, const D3D12_TILED_RESOURCE_COORDINATE* pResourceRegionStartCoordinates, const D3D12_TILE_REGION_SIZE* pResourceRegionSizes, ID3D12Heap* pHeap, UINT NumRanges, const D3D12_TILE_RANGE_FLAGS* pRangeFlags, const UINT* pHeapRangeStartOffsets, const UINT* pRangeTileCounts, D3D12_TILE_MAPPING_FLAGS Flags);
    virtual void STDMETHODCALLTYPE CopyTileMappings(ID3D12Resource* pDstResource, const D3D12_TILED_RESOURCE_COORDINATE* pDstRegionStartCoordinate, ID3D12Resource* pSrcResource, const D3D12_TILED_RESOURCE_COORDINATE* pSrcRegionStartCoordinate, const D3D12_TILE_REGION_SIZE* pRegionSize, D3D12_TILE_MAPPING_FLAGS Flags);
    virtual void STDMETHODCALLTYPE ExecuteCommandLists(UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
    virtual void STDMETHODCALLTYPE SetMarker(UINT Metadata, const void* pData, UINT Size);
    virtual void STDMETHODCALLTYPE BeginEvent(UINT Metadata, const void* pData, UINT Size);
    virtual void STDMETHODCALLTYPE EndEvent();
    virtual HRESULT STDMETHODCALLTYPE Signal(ID3D12Fence* pFence, UINT64 Value);
    virtual HRESULT STDMETHODCALLTYPE Wait(ID3D12Fence* pFence, UINT64 Value);
    virtual HRESULT STDMETHODCALLTYPE GetTimestampFrequency(UINT64* pFrequency);
    virtual HRESULT STDMETHODCALLTYPE GetClockCalibration(UINT64* pGpuTimestamp, UINT64* pCpuTimestamp);
    virtual D3D12_COMMAND_QUEUE_DESC STDMETHODCALLTYPE GetDesc();

    ID3D12CommandQueue* mRealCommandQueue;      ///< The real runtime instance being wrapped.
};

#endif // WRAPPED_ID3D12COMMANDQUEUE_H