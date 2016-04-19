//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#ifndef _DC_COMMAND_DEFS_H_
#define _DC_COMMAND_DEFS_H_

#include <d3d11.h>
#include <string>
#include "DCFuncDefs.h"
#include "DCUtils.h"
#include "..\Common\StringUtils.h"

#define SIMPLE_STRING

class DCCommandBuffer;

/// \addtogroup DCCommandRecorder
// @{

enum ID3D11DeviceContextCommandType
{
    DC_CMD_Type_Release, ///< Release resource

    DC_CMD_Type_Begin,///< Mark the beginning of a series of commands.

    DC_CMD_Type_ClearDepthStencilView,///< Clears the depth-stencil resource.

    DC_CMD_Type_ClearRenderTargetView,///< Set all the elements in a render target to one value.

    DC_CMD_Type_ClearState,///< Restore all default settings.

    DC_CMD_Type_ClearUnorderedAccessViewFloat,///< Clears an unordered access resource with a float value.

    DC_CMD_Type_ClearUnorderedAccessViewUint,///< Clears an unordered access resource with bit-precise values.

    DC_CMD_Type_CopyResource,///< Copy the entire contents of the source resource to the destination resource using the GPU.

    DC_CMD_Type_CopyStructureCount,///< Copies data from a buffer holding variable length data.

    DC_CMD_Type_CopySubresourceRegion,///< Copy a region from a source resource to a destination resource.

    DC_CMD_Type_CSGetConstantBuffers,///< Get the constant buffers used by the compute-shader stage.

    DC_CMD_Type_CSGetSamplers,///< Get an array of sampler state interfaces from the compute-shader stage.

    DC_CMD_Type_CSGetShader,///< Get the compute shader currently set on the device.

    DC_CMD_Type_CSGetShaderResources,///< Get the compute-shader resources.

    DC_CMD_Type_CSGetUnorderedAccessViews,///< Gets an array of views for an unordered resource.

    DC_CMD_Type_CSSetConstantBuffers,///< Set the constant buffers used by the compute-shader stage.

    DC_CMD_Type_CSSetSamplers,///< Set an array of sampler states to the compute-shader stage.

    DC_CMD_Type_CSSetShader,///< Set a compute shader to the device.

    DC_CMD_Type_CSSetShaderResources,///< Bind an array of shader resources to the compute-shader stage.

    DC_CMD_Type_CSSetUnorderedAccessViews,///< Sets an array of views for an unordered resource.

    DC_CMD_Type_Dispatch,///< Execute a command list from a thread group.

    DC_CMD_Type_DispatchIndirect,///< Execute a command list to draw GPU-generated primitives over one of more thread groups.

    DC_CMD_Type_Draw,///< Draw non-indexed, non-instanced primitives.

    DC_CMD_Type_DrawAuto,///< Draw geometry of an unknown size.

    DC_CMD_Type_DrawIndexed,///< Draw indexed, non-instanced primitives.

    DC_CMD_Type_DrawIndexedInstanced,///< Draw indexed, instanced primitives.

    DC_CMD_Type_DrawIndexedInstancedIndirect,///< Draw indexed, instanced, GPU-generated primitives.

    DC_CMD_Type_DrawInstanced,///< Draw non-indexed, instanced primitives.

    DC_CMD_Type_DrawInstancedIndirect,///< Draw instanced, GPU-generated primitives.

    DC_CMD_Type_DSGetConstantBuffers,///< Get the constant buffers used by the domain-shader stage.

    DC_CMD_Type_DSGetSamplers,///< Get an array of sampler state interfaces from the domain-shader stage.

    DC_CMD_Type_DSGetShader,///< Get the domain shader currently set on the device.

    DC_CMD_Type_DSGetShaderResources,///< Get the domain-shader resources.

    DC_CMD_Type_DSSetConstantBuffers,///< Set the constant buffers used by the domain-shader stage.

    DC_CMD_Type_DSSetSamplers,///< Set an array of sampler states to the domain-shader stage.

    DC_CMD_Type_DSSetShader,///< Set a domain shader to the device.

    DC_CMD_Type_DSSetShaderResources,///< Bind an array of shader resources to the domain-shader stage.

    DC_CMD_Type_End,///< Mark the end of a series of commands.

    DC_CMD_Type_ExecuteCommandList,///< Queues commands from a command list onto a device.

    DC_CMD_Type_FinishCommandList,///< Create a command list and record graphics commands into it.

    DC_CMD_Type_Flush,///< Send queued-up commands in the command buffer to the GPU.

    DC_CMD_Type_GenerateMips,///< Generate mipmaps for the given shader resource.

    DC_CMD_Type_GetContextFlags,///< Gets the initialization flags associated with the current deferred context.

    DC_CMD_Type_GetData,///< Get data from the GPU asynchronously.

    DC_CMD_Type_GetPredication,///< Get the rendering predicate state.

    DC_CMD_Type_GetResourceMinLOD,///< Gets the minimum level-of-detail (LOD).

    DC_CMD_Type_GetType,///< Gets the type of device context.

    DC_CMD_Type_GSGetConstantBuffers,///< Get the constant buffers used by the geometry shader pipeline stage.

    DC_CMD_Type_GSGetSamplers,///< Get an array of sampler state interfaces from the geometry shader pipeline stage.

    DC_CMD_Type_GSGetShader,///< Get the geometry shader currently set on the device.

    DC_CMD_Type_GSGetShaderResources,///< Get the geometry shader resources.

    DC_CMD_Type_GSSetConstantBuffers,///< Set the constant buffers used by the geometry shader pipeline stage.

    DC_CMD_Type_GSSetSamplers,///< Set an array of sampler states to the geometry shader pipeline stage.

    DC_CMD_Type_GSSetShader,///< Set a geometry shader to the device.

    DC_CMD_Type_GSSetShaderResources,///< Bind an array of shader resources to the geometry shader stage.

    DC_CMD_Type_HSGetConstantBuffers,///< Get the constant buffers used by the hull-shader stage.

    DC_CMD_Type_HSGetSamplers,///< Get an array of sampler state interfaces from the hull-shader stage.

    DC_CMD_Type_HSGetShader,///< Get the hull shader currently set on the device.

    DC_CMD_Type_HSGetShaderResources,///< Get the hull-shader resources.

    DC_CMD_Type_HSSetConstantBuffers,///< Set the constant buffers used by the hull-shader stage.

    DC_CMD_Type_HSSetSamplers,///< Set an array of sampler states to the hull-shader stage.

    DC_CMD_Type_HSSetShader,///< Set a hull shader to the device.

    DC_CMD_Type_HSSetShaderResources,///< Bind an array of shader resources to the hull-shader stage.

    DC_CMD_Type_IAGetIndexBuffer,///< Get a pointer to the index buffer that is bound to the input-assembler stage.

    DC_CMD_Type_IAGetInputLayout,///< Get a pointer to the input-layout object that is bound to the input-assembler stage.

    DC_CMD_Type_IAGetPrimitiveTopology,///< Get information about the primitive type, and data order that describes input data for the input assembler stage.

    DC_CMD_Type_IAGetVertexBuffers,///< Get the vertex buffers bound to the input-assembler stage.

    DC_CMD_Type_IASetIndexBuffer,///< Bind an index buffer to the input-assembler stage.

    DC_CMD_Type_IASetInputLayout,///< Bind an input-layout object to the input-assembler stage.

    DC_CMD_Type_IASetPrimitiveTopology,///< Bind information about the primitive type, and data order that describes input data for the input assembler stage.

    DC_CMD_Type_IASetVertexBuffers,///< Bind an array of vertex buffers to the input-assembler stage.

    DC_CMD_Type_Map,///< Get a pointer to the data contained in a subresource, and deny the GPU access to that subresource.

    DC_CMD_Type_OMGetBlendState,///< Get the blend state of the output-merger stage.

    DC_CMD_Type_OMGetDepthStencilState,///< Gets the depth-stencil state of the output-merger stage.

    DC_CMD_Type_OMGetRenderTargets,///< Get pointers to the resources bound to the output-merger stage.

    DC_CMD_Type_OMGetRenderTargetsAndUnorderedAccessViews,///< Get pointers to the resources bound to the output-merger stage.

    DC_CMD_Type_OMSetBlendState,///< Set the blend state of the output-merger stage.

    DC_CMD_Type_OMSetDepthStencilState,///< Sets the depth-stencil state of the output-merger stage.

    DC_CMD_Type_OMSetRenderTargets,///< Bind one or more render targets atomically and the depth-stencil buffer to the output-merger stage.

    DC_CMD_Type_OMSetRenderTargetsAndUnorderedAccessViews,///< Bind resources to the output-merger stage.

    DC_CMD_Type_PSGetConstantBuffers,///< Get the constant buffers used by the pixel shader pipeline stage.

    DC_CMD_Type_PSGetSamplers,///< Get an array of sampler states from the pixel shader pipeline stage.

    DC_CMD_Type_PSGetShader,///< Get the pixel shader currently set on the device.

    DC_CMD_Type_PSGetShaderResources,///< Get the pixel shader resources.

    DC_CMD_Type_PSSetConstantBuffers,///< Set the constant buffers used by the pixel shader pipeline stage.

    DC_CMD_Type_PSSetSamplers,///< Set an array of sampler states to the pixel shader pipeline stage.

    DC_CMD_Type_PSSetShader,///< Sets a pixel shader to the device.

    DC_CMD_Type_PSSetShaderResources,///< Bind an array of shader resources to the pixel shader stage.

    DC_CMD_Type_ResolveSubresource,///< Copy a multisampled resource into a non-multisampled resource.

    DC_CMD_Type_RSGetScissorRects,///< Get the array of scissor rectangles bound to the rasterizer stage.

    DC_CMD_Type_RSGetState,///< Get the rasterizer state from the rasterizer stage of the pipeline.

    DC_CMD_Type_RSGetViewports,///< Get the array of viewports bound to the rasterizer stage

    DC_CMD_Type_RSSetScissorRects,///< Bind an array of scissor rectangles to the rasterizer stage.

    DC_CMD_Type_RSSetState,///< Set the rasterizer state for the rasterizer stage of the pipeline.

    DC_CMD_Type_RSSetViewports,///< Bind an array of viewports to the rasterizer stage of the pipeline.

    DC_CMD_Type_SetPredication,///< Set a rendering predicate.

    DC_CMD_Type_SetResourceMinLOD,///< Sets the minimum level-of-detail (LOD) for a resource.

    DC_CMD_Type_SOGetTargets,///< Get the target output buffers for the stream-output stage of the pipeline.

    DC_CMD_Type_SOSetTargets,///< Set the target output buffers for the stream-output stage of the pipeline.

    DC_CMD_Type_Unmap,///< Invalidate the pointer to a resource and re-enable the GPU's access to that resource.

    DC_CMD_Type_UpdateSubresource,///< The CPU copies data from memory to a subresource created in non-mappable memory.

    DC_CMD_Type_VSGetConstantBuffers,///< Get the constant buffers used by the vertex shader pipeline stage.

    DC_CMD_Type_VSGetSamplers,///< Get an array of sampler states from the vertex shader pipeline stage.

    DC_CMD_Type_VSGetShader,///< Get the vertex shader currently set on the device.

    DC_CMD_Type_VSGetShaderResources,///< Get the vertex shader resources.

    DC_CMD_Type_VSSetConstantBuffers,///< Set the constant buffers used by the vertex shader pipeline stage.

    DC_CMD_Type_VSSetSamplers,///< Set an array of sampler states to the vertex shader pipeline stage.

    DC_CMD_Type_VSSetShader,///< Set a vertex shader to the device.

    DC_CMD_Type_VSSetShaderResources,///< Bind an array of shader resources to the vertex-shader stage.

    DC_CMD_Type_UNKNOWN ///< Unknown command type
};

//------------------------------------------------------------------------------------
/// Command base abstract class
//------------------------------------------------------------------------------------
class DC_CMDBase
{
protected:
    ID3D11DeviceContextCommandType m_CMDType;    ///< Command type
    ID3D11DeviceContext* m_pObj;                 ///< Deferred context object

public:

    /// Constructor
    DC_CMDBase()
    {
        m_CMDType = DC_CMD_Type_UNKNOWN;
        m_pObj = NULL;
    }

    /// Virtual destructor
    virtual ~DC_CMDBase()
    {

    }

    /// Play, run the command on immediate context
    /// \param pImmediateContext Immediate Context
    virtual void Play(ID3D11DeviceContext* pImmediateContext) = 0;

    /// ToString
    /// \return name string
    virtual std::string ToString() = 0;

#if defined(_DEBUG)
    DWORD m_dwThreadId;     ///< Deferred context Thread ID
#endif

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMDBase(const DC_CMDBase& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMDBase& operator=(const DC_CMDBase& obj);
};

//------------------------------------------------------------------------------------
/// VSSetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_VSSetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::VSSetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::VSSetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::VSSetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_VSSetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::VSSetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::VSSetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::VSSetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        //m_ppConstantBuffers = ppConstantBuffers;
        m_ppConstantBuffers = DCUtils::CopyArrayOfInterfaces(NumBuffers, ppConstantBuffers);
        m_CMDType = DC_CMD_Type_VSSetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->VSSetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Constructor
    DC_CMD_VSSetConstantBuffers()
    {

    }

    /// Destructor
    ~DC_CMD_VSSetConstantBuffers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumBuffers, m_ppConstantBuffers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("VSSetConstantBuffers");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_VSSetConstantBuffers(const DC_CMD_VSSetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_VSSetConstantBuffers& operator= (const DC_CMD_VSSetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// PSSetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_PSSetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::PSSetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::PSSetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::PSSetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_PSSetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::PSSetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::PSSetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::PSSetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        //m_ppShaderResourceViews = ppShaderResourceViews;
        m_ppShaderResourceViews = DCUtils::CopyArrayOfInterfaces(NumViews, ppShaderResourceViews);
        m_CMDType = DC_CMD_Type_PSSetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->PSSetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Constructor
    DC_CMD_PSSetShaderResources()
    {

    }

    /// Destructor
    ~DC_CMD_PSSetShaderResources()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumViews, m_ppShaderResourceViews);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("PSSetShaderResources");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_PSSetShaderResources(const DC_CMD_PSSetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_PSSetShaderResources& operator= (const DC_CMD_PSSetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// PSSetShader
//------------------------------------------------------------------------------------
class DC_CMD_PSSetShader : public DC_CMDBase
{
    ID3D11PixelShader* m_pPixelShader;           ///< Parameters for function ID3D11DeviceContext::PSSetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::PSSetShader()
    UINT m_NumClassInstances;           ///< Parameters for function ID3D11DeviceContext::PSSetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_PSSetShader()
    /// \param pObj Deferred context
    /// \param pPixelShader Parameters for ID3D11DeviceContext::PSSetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::PSSetShader()
    /// \param NumClassInstances Parameters for ID3D11DeviceContext::PSSetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11PixelShader* pPixelShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pObj = pObj;
        m_pPixelShader = pPixelShader;
        //m_ppClassInstances = ppClassInstances;
        m_ppClassInstances = DCUtils::CopyArrayOfInterfaces(NumClassInstances, ppClassInstances);
        m_NumClassInstances = NumClassInstances;
        m_CMDType = DC_CMD_Type_PSSetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->PSSetShader(m_pPixelShader, m_ppClassInstances, m_NumClassInstances);
    }

    /// Constructor
    DC_CMD_PSSetShader()
    {

    }

    /// Destructor
    ~DC_CMD_PSSetShader()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumClassInstances, m_ppClassInstances);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("PSSetShader");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_PSSetShader(const DC_CMD_PSSetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_PSSetShader& operator= (const DC_CMD_PSSetShader& obj);
};

//------------------------------------------------------------------------------------
/// PSSetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_PSSetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::PSSetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::PSSetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::PSSetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_PSSetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::PSSetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::PSSetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::PSSetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        //m_ppSamplers = ppSamplers;
        m_ppSamplers = DCUtils::CopyArrayOfInterfaces(NumSamplers, ppSamplers);
        m_CMDType = DC_CMD_Type_PSSetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->PSSetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Constructor
    DC_CMD_PSSetSamplers()
    {

    }

    /// Destructor
    ~DC_CMD_PSSetSamplers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumSamplers, m_ppSamplers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("PSSetSamplers");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_PSSetSamplers(const DC_CMD_PSSetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_PSSetSamplers& operator= (const DC_CMD_PSSetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// VSSetShader
//------------------------------------------------------------------------------------
class DC_CMD_VSSetShader : public DC_CMDBase
{
    ID3D11VertexShader* m_pVertexShader;            ///< Parameters for function ID3D11DeviceContext::VSSetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::VSSetShader()
    UINT m_NumClassInstances;           ///< Parameters for function ID3D11DeviceContext::VSSetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_VSSetShader()
    /// \param pObj Deferred context
    /// \param pVertexShader Parameters for ID3D11DeviceContext::VSSetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::VSSetShader()
    /// \param NumClassInstances Parameters for ID3D11DeviceContext::VSSetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11VertexShader* pVertexShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pObj = pObj;
        m_pVertexShader = pVertexShader;
        //m_ppClassInstances = ppClassInstances;
        m_ppClassInstances = DCUtils::CopyArrayOfInterfaces(NumClassInstances, ppClassInstances);
        m_NumClassInstances = NumClassInstances;
        m_CMDType = DC_CMD_Type_VSSetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->VSSetShader(m_pVertexShader, m_ppClassInstances, m_NumClassInstances);
    }

    /// Constructor
    DC_CMD_VSSetShader()
    {

    }

    /// Destructor
    ~DC_CMD_VSSetShader()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumClassInstances, m_ppClassInstances);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("VSSetShader");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_VSSetShader(const DC_CMD_VSSetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_VSSetShader& operator= (const DC_CMD_VSSetShader& obj);
};

//------------------------------------------------------------------------------------
/// DrawIndexed
//------------------------------------------------------------------------------------
class DC_CMD_DrawIndexed : public DC_CMDBase
{
    UINT m_IndexCount;            ///< Parameters for function ID3D11DeviceContext::DrawIndexed()
    UINT m_StartIndexLocation;          ///< Parameters for function ID3D11DeviceContext::DrawIndexed()
    INT m_BaseVertexLocation;           ///< Parameters for function ID3D11DeviceContext::DrawIndexed()

public:
    /// This is called in Mine_ID3D11DeviceContext_DrawIndexed()
    /// \param pObj Deferred context
    /// \param IndexCount Parameters for ID3D11DeviceContext::DrawIndexed()
    /// \param StartIndexLocation Parameters for ID3D11DeviceContext::DrawIndexed()
    /// \param BaseVertexLocation Parameters for ID3D11DeviceContext::DrawIndexed()
    void OnCreate(ID3D11DeviceContext* pObj, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
    {
        m_pObj = pObj;
        m_IndexCount = IndexCount;
        m_StartIndexLocation = StartIndexLocation;
        m_BaseVertexLocation = BaseVertexLocation;
        m_CMDType = DC_CMD_Type_DrawIndexed;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DrawIndexed(m_IndexCount, m_StartIndexLocation, m_BaseVertexLocation);
    }

    /// Constructor
    DC_CMD_DrawIndexed()
    {
    }

    /// Destructor
    ~DC_CMD_DrawIndexed()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
#ifdef SIMPLE_STRING
        return std::string("DrawIndexed");
#else
        return StringUtils::FormatString("DrawIndexed( 0x%p, %u, %u, %i )", m_pObj, m_IndexCount, m_StartIndexLocation, m_BaseVertexLocation);
#endif
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DrawIndexed(const DC_CMD_DrawIndexed& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DrawIndexed& operator= (const DC_CMD_DrawIndexed& obj);
};

//------------------------------------------------------------------------------------
/// Draw
//------------------------------------------------------------------------------------
class DC_CMD_Draw : public DC_CMDBase
{
    UINT m_VertexCount;           ///< Parameters for function ID3D11DeviceContext::Draw()
    UINT m_StartVertexLocation;            ///< Parameters for function ID3D11DeviceContext::Draw()

public:
    /// This is called in Mine_ID3D11DeviceContext_Draw()
    /// \param pObj Deferred context
    /// \param VertexCount Parameters for ID3D11DeviceContext::Draw()
    /// \param StartVertexLocation Parameters for ID3D11DeviceContext::Draw()
    void OnCreate(ID3D11DeviceContext* pObj, UINT VertexCount, UINT StartVertexLocation)
    {
        m_pObj = pObj;
        m_VertexCount = VertexCount;
        m_StartVertexLocation = StartVertexLocation;
        m_CMDType = DC_CMD_Type_Draw;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->Draw(m_VertexCount, m_StartVertexLocation);
    }

    /// Constructor
    DC_CMD_Draw()
    {
    }

    /// Destructor
    ~DC_CMD_Draw()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("Draw");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_Draw(const DC_CMD_Draw& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_Draw& operator= (const DC_CMD_Draw& obj);
};

//------------------------------------------------------------------------------------
/// Map
//------------------------------------------------------------------------------------
class DC_CMD_Map : public DC_CMDBase
{
    ID3D11Resource* m_pResource;           ///< Parameters for function ID3D11DeviceContext::Map()
    UINT m_Subresource;           ///< Parameters for function ID3D11DeviceContext::Map()
    D3D11_MAP m_MapType;          ///< Parameters for function ID3D11DeviceContext::Map()
    UINT m_MapFlags;           ///< Parameters for function ID3D11DeviceContext::Map()
    D3D11_MAPPED_SUBRESOURCE* m_pMappedResource;             ///< Parameters for function ID3D11DeviceContext::Map()
    D3D11_MAPPED_SUBRESOURCE m_MappedResource;            ///< Parameters for function ID3D11DeviceContext::Map()
    DCCommandBuffer*              m_pOwner;                        ///< Owner context

public:
    /// This is called in Mine_ID3D11DeviceContext_Map()
    /// \param pObj Deferred context
    /// \param pResource Parameters for ID3D11DeviceContext::Map()
    /// \param Subresource Parameters for ID3D11DeviceContext::Map()
    /// \param MapType Parameters for ID3D11DeviceContext::Map()
    /// \param MapFlags Parameters for ID3D11DeviceContext::Map()
    /// \param pMappedResource Parameters for ID3D11DeviceContext::Map()
    /// \param pOwner Owner command buffer, it contains the member to keep track of the pointer that Map openned
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Resource* pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource, DCCommandBuffer* pOwner);

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext);

    /// Constructor
    DC_CMD_Map()
    {
    }

    /// Destructor
    ~DC_CMD_Map()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("Map");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_Map(const DC_CMD_Map& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_Map& operator= (const DC_CMD_Map& obj);
};

//------------------------------------------------------------------------------------
/// Unmap
//------------------------------------------------------------------------------------
class DC_CMD_Unmap : public DC_CMDBase
{
    ID3D11Resource* m_pResource;           ///< Parameters for function ID3D11DeviceContext::Unmap()
    UINT m_Subresource;           ///< Parameters for function ID3D11DeviceContext::Unmap()
    DCCommandBuffer* m_pOwner;                   ///< Owner Context

public:
    /// This is called in Mine_ID3D11DeviceContext_Unmap()
    /// \param pObj Deferred context
    /// \param pResource Parameters for ID3D11DeviceContext::Unmap()
    /// \param Subresource Parameters for ID3D11DeviceContext::Unmap()
    /// \param pOwner DCContext
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Resource* pResource, UINT Subresource, DCCommandBuffer* pOwner);

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext);

    /// Constructor
    DC_CMD_Unmap()
    {
    }

    /// Destructor
    ~DC_CMD_Unmap()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("Unmap");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_Unmap(const DC_CMD_Unmap& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_Unmap& operator= (const DC_CMD_Unmap& obj);
};

//------------------------------------------------------------------------------------
/// PSSetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_PSSetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::PSSetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::PSSetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::PSSetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_PSSetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::PSSetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::PSSetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::PSSetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        //m_ppConstantBuffers = ppConstantBuffers;
        m_ppConstantBuffers = DCUtils::CopyArrayOfInterfaces(NumBuffers, ppConstantBuffers);
        m_CMDType = DC_CMD_Type_PSSetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->PSSetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Constructor
    DC_CMD_PSSetConstantBuffers()
    {

    }

    /// Destructor
    ~DC_CMD_PSSetConstantBuffers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumBuffers, m_ppConstantBuffers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("PSSetConstantBuffers");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_PSSetConstantBuffers(const DC_CMD_PSSetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_PSSetConstantBuffers& operator= (const DC_CMD_PSSetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// IASetInputLayout
//------------------------------------------------------------------------------------
class DC_CMD_IASetInputLayout : public DC_CMDBase
{
    ID3D11InputLayout* m_pInputLayout;           ///< Parameters for function ID3D11DeviceContext::IASetInputLayout()

public:
    /// This is called in Mine_ID3D11DeviceContext_IASetInputLayout()
    /// \param pObj Deferred context
    /// \param pInputLayout Parameters for ID3D11DeviceContext::IASetInputLayout()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11InputLayout* pInputLayout)
    {
        m_pObj = pObj;
        m_pInputLayout = pInputLayout;
        m_CMDType = DC_CMD_Type_IASetInputLayout;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->IASetInputLayout(m_pInputLayout);
    }

    /// Constructor
    DC_CMD_IASetInputLayout()
    {
    }

    /// Destructor
    ~DC_CMD_IASetInputLayout()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("IASetInputLayout");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_IASetInputLayout(const DC_CMD_IASetInputLayout& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_IASetInputLayout& operator= (const DC_CMD_IASetInputLayout& obj);
};

//------------------------------------------------------------------------------------
/// IASetVertexBuffers
//------------------------------------------------------------------------------------
class DC_CMD_IASetVertexBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::IASetVertexBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::IASetVertexBuffers()
    ID3D11Buffer** m_ppVertexBuffers;            ///< Parameters for function ID3D11DeviceContext::IASetVertexBuffers()
    UINT* m_pStrides;             ///< Parameters for function ID3D11DeviceContext::IASetVertexBuffers()
    UINT* m_pOffsets;             ///< Parameters for function ID3D11DeviceContext::IASetVertexBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_IASetVertexBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::IASetVertexBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::IASetVertexBuffers()
    /// \param ppVertexBuffers Parameters for ID3D11DeviceContext::IASetVertexBuffers()
    /// \param pStrides Parameters for ID3D11DeviceContext::IASetVertexBuffers()
    /// \param pOffsets Parameters for ID3D11DeviceContext::IASetVertexBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppVertexBuffers, const UINT* pStrides, const UINT* pOffsets)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        //m_ppVertexBuffers = ppVertexBuffers;
        m_ppVertexBuffers = DCUtils::CopyArrayOfInterfaces(NumBuffers, ppVertexBuffers);
        //m_pStrides = pStrides;
        //m_pOffsets = pOffsets;
        m_pStrides = new UINT[m_NumBuffers];
        m_pOffsets = new UINT[m_NumBuffers];
        memcpy(m_pStrides, pStrides, m_NumBuffers * sizeof(UINT));
        memcpy(m_pOffsets, pOffsets, m_NumBuffers * sizeof(UINT));
        m_CMDType = DC_CMD_Type_IASetVertexBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->IASetVertexBuffers(m_StartSlot, m_NumBuffers, m_ppVertexBuffers, m_pStrides, m_pOffsets);
    }

    /// Constructor
    DC_CMD_IASetVertexBuffers()
    {

    }

    /// Destructor
    ~DC_CMD_IASetVertexBuffers()
    {
        if (m_pStrides)
        {
            delete [] m_pStrides;
        }

        if (m_pOffsets)
        {
            delete [] m_pOffsets;
        }

        DCUtils::ReleaseArrayOfInterfaces(m_NumBuffers, m_ppVertexBuffers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("IASetVertexBuffers");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_IASetVertexBuffers(const DC_CMD_IASetVertexBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_IASetVertexBuffers& operator= (const DC_CMD_IASetVertexBuffers& obj);
};

//------------------------------------------------------------------------------------
/// IASetIndexBuffer
//------------------------------------------------------------------------------------
class DC_CMD_IASetIndexBuffer : public DC_CMDBase
{
    ID3D11Buffer* m_pIndexBuffer;             ///< Parameters for function ID3D11DeviceContext::IASetIndexBuffer()
    DXGI_FORMAT m_Format;            ///< Parameters for function ID3D11DeviceContext::IASetIndexBuffer()
    UINT m_Offset;          ///< Parameters for function ID3D11DeviceContext::IASetIndexBuffer()

public:
    /// This is called in Mine_ID3D11DeviceContext_IASetIndexBuffer()
    /// \param pObj Deferred context
    /// \param pIndexBuffer Parameters for ID3D11DeviceContext::IASetIndexBuffer()
    /// \param Format Parameters for ID3D11DeviceContext::IASetIndexBuffer()
    /// \param Offset Parameters for ID3D11DeviceContext::IASetIndexBuffer()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset)
    {
        m_pObj = pObj;
        m_pIndexBuffer = pIndexBuffer;
        m_Format = Format;
        m_Offset = Offset;
        m_CMDType = DC_CMD_Type_IASetIndexBuffer;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->IASetIndexBuffer(m_pIndexBuffer, m_Format, m_Offset);
    }

    /// Constructor
    DC_CMD_IASetIndexBuffer()
    {
    }

    /// Destructor
    ~DC_CMD_IASetIndexBuffer()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("IASetIndexBuffer");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_IASetIndexBuffer(const DC_CMD_IASetIndexBuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_IASetIndexBuffer& operator= (const DC_CMD_IASetIndexBuffer& obj);
};

//------------------------------------------------------------------------------------
/// DrawIndexedInstanced
//------------------------------------------------------------------------------------
class DC_CMD_DrawIndexedInstanced : public DC_CMDBase
{
    UINT m_IndexCountPerInstance;          ///< Parameters for function ID3D11DeviceContext::DrawIndexedInstanced()
    UINT m_InstanceCount;            ///< Parameters for function ID3D11DeviceContext::DrawIndexedInstanced()
    UINT m_StartIndexLocation;          ///< Parameters for function ID3D11DeviceContext::DrawIndexedInstanced()
    INT m_BaseVertexLocation;           ///< Parameters for function ID3D11DeviceContext::DrawIndexedInstanced()
    UINT m_StartInstanceLocation;          ///< Parameters for function ID3D11DeviceContext::DrawIndexedInstanced()

public:
    /// This is called in Mine_ID3D11DeviceContext_DrawIndexedInstanced()
    /// \param pObj Deferred context
    /// \param IndexCountPerInstance Parameters for ID3D11DeviceContext::DrawIndexedInstanced()
    /// \param InstanceCount Parameters for ID3D11DeviceContext::DrawIndexedInstanced()
    /// \param StartIndexLocation Parameters for ID3D11DeviceContext::DrawIndexedInstanced()
    /// \param BaseVertexLocation Parameters for ID3D11DeviceContext::DrawIndexedInstanced()
    /// \param StartInstanceLocation Parameters for ID3D11DeviceContext::DrawIndexedInstanced()
    void OnCreate(ID3D11DeviceContext* pObj, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
    {
        m_pObj = pObj;
        m_IndexCountPerInstance = IndexCountPerInstance;
        m_InstanceCount = InstanceCount;
        m_StartIndexLocation = StartIndexLocation;
        m_BaseVertexLocation = BaseVertexLocation;
        m_StartInstanceLocation = StartInstanceLocation;
        m_CMDType = DC_CMD_Type_DrawIndexedInstanced;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DrawIndexedInstanced(m_IndexCountPerInstance, m_InstanceCount, m_StartIndexLocation, m_BaseVertexLocation, m_StartInstanceLocation);
    }

    /// Constructor
    DC_CMD_DrawIndexedInstanced()
    {
    }

    /// Destructor
    ~DC_CMD_DrawIndexedInstanced()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
#ifdef SIMPLE_STRING
        return std::string("DrawIndexedInstanced");
#else
        return StringUtils::FormatString("DrawIndexedInstanced( 0x%p, %u, %u, %u, %i, %u )", m_pObj, m_IndexCountPerInstance, m_InstanceCount, m_StartIndexLocation, m_BaseVertexLocation, m_StartInstanceLocation);
#endif
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DrawIndexedInstanced(const DC_CMD_DrawIndexedInstanced& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DrawIndexedInstanced& operator= (const DC_CMD_DrawIndexedInstanced& obj);
};

//------------------------------------------------------------------------------------
/// DrawInstanced
//------------------------------------------------------------------------------------
class DC_CMD_DrawInstanced : public DC_CMDBase
{
    UINT m_VertexCountPerInstance;            ///< Parameters for function ID3D11DeviceContext::DrawInstanced()
    UINT m_InstanceCount;            ///< Parameters for function ID3D11DeviceContext::DrawInstanced()
    UINT m_StartVertexLocation;            ///< Parameters for function ID3D11DeviceContext::DrawInstanced()
    UINT m_StartInstanceLocation;          ///< Parameters for function ID3D11DeviceContext::DrawInstanced()

public:
    /// This is called in Mine_ID3D11DeviceContext_DrawInstanced()
    /// \param pObj Deferred context
    /// \param VertexCountPerInstance Parameters for ID3D11DeviceContext::DrawInstanced()
    /// \param InstanceCount Parameters for ID3D11DeviceContext::DrawInstanced()
    /// \param StartVertexLocation Parameters for ID3D11DeviceContext::DrawInstanced()
    /// \param StartInstanceLocation Parameters for ID3D11DeviceContext::DrawInstanced()
    void OnCreate(ID3D11DeviceContext* pObj, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
    {
        m_pObj = pObj;
        m_VertexCountPerInstance = VertexCountPerInstance;
        m_InstanceCount = InstanceCount;
        m_StartVertexLocation = StartVertexLocation;
        m_StartInstanceLocation = StartInstanceLocation;
        m_CMDType = DC_CMD_Type_DrawInstanced;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DrawInstanced(m_VertexCountPerInstance, m_InstanceCount, m_StartVertexLocation, m_StartInstanceLocation);
    }

    /// Constructor
    DC_CMD_DrawInstanced()
    {
    }

    /// Destructor
    ~DC_CMD_DrawInstanced()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("DrawInstanced");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DrawInstanced(const DC_CMD_DrawInstanced& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DrawInstanced& operator= (const DC_CMD_DrawInstanced& obj);
};

//------------------------------------------------------------------------------------
/// GSSetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_GSSetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::GSSetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::GSSetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::GSSetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_GSSetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::GSSetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::GSSetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::GSSetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        //m_ppConstantBuffers = ppConstantBuffers;
        m_ppConstantBuffers = DCUtils::CopyArrayOfInterfaces(NumBuffers, ppConstantBuffers);
        m_CMDType = DC_CMD_Type_GSSetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GSSetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Constructor
    DC_CMD_GSSetConstantBuffers()
    {

    }

    /// Destructor
    ~DC_CMD_GSSetConstantBuffers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumBuffers, m_ppConstantBuffers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GSSetConstantBuffers(const DC_CMD_GSSetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GSSetConstantBuffers& operator= (const DC_CMD_GSSetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// GSSetShader
//------------------------------------------------------------------------------------
class DC_CMD_GSSetShader : public DC_CMDBase
{
    ID3D11GeometryShader* m_pShader;             ///< Parameters for function ID3D11DeviceContext::GSSetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::GSSetShader()
    UINT m_NumClassInstances;           ///< Parameters for function ID3D11DeviceContext::GSSetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_GSSetShader()
    /// \param pObj Deferred context
    /// \param pShader Parameters for ID3D11DeviceContext::GSSetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::GSSetShader()
    /// \param NumClassInstances Parameters for ID3D11DeviceContext::GSSetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11GeometryShader* pShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pObj = pObj;
        m_pShader = pShader;
        //m_ppClassInstances = ppClassInstances;
        m_ppClassInstances = DCUtils::CopyArrayOfInterfaces(NumClassInstances, ppClassInstances);
        m_NumClassInstances = NumClassInstances;
        m_CMDType = DC_CMD_Type_GSSetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GSSetShader(m_pShader, m_ppClassInstances, m_NumClassInstances);
    }

    /// Constructor
    DC_CMD_GSSetShader()
    {

    }

    /// Destructor
    ~DC_CMD_GSSetShader()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumClassInstances, m_ppClassInstances);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GSSetShader(const DC_CMD_GSSetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GSSetShader& operator= (const DC_CMD_GSSetShader& obj);
};

//------------------------------------------------------------------------------------
/// IASetPrimitiveTopology
//------------------------------------------------------------------------------------
class DC_CMD_IASetPrimitiveTopology : public DC_CMDBase
{
    D3D11_PRIMITIVE_TOPOLOGY m_Topology;            ///< Parameters for function ID3D11DeviceContext::IASetPrimitiveTopology()

public:
    /// This is called in Mine_ID3D11DeviceContext_IASetPrimitiveTopology()
    /// \param pObj Deferred context
    /// \param Topology Parameters for ID3D11DeviceContext::IASetPrimitiveTopology()
    void OnCreate(ID3D11DeviceContext* pObj, D3D11_PRIMITIVE_TOPOLOGY Topology)
    {
        m_pObj = pObj;
        m_Topology = Topology;
        m_CMDType = DC_CMD_Type_IASetPrimitiveTopology;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->IASetPrimitiveTopology(m_Topology);
    }

    /// Constructor
    DC_CMD_IASetPrimitiveTopology()
    {
    }

    /// Destructor
    ~DC_CMD_IASetPrimitiveTopology()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
#ifdef SIMPLE_STRING
        return std::string("IASetPrimitiveTopology");
#else
        return StringUtils::FormatString("IASetPrimitiveTopology( 0x%p, %s )", m_pObj, DCUtils::Stringify_D3D11_PRIMITIVE_TOPOLOGY(m_Topology).c_str());
#endif
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_IASetPrimitiveTopology(const DC_CMD_IASetPrimitiveTopology& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_IASetPrimitiveTopology& operator= (const DC_CMD_IASetPrimitiveTopology& obj);
};

//------------------------------------------------------------------------------------
/// VSSetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_VSSetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::VSSetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::VSSetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::VSSetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_VSSetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::VSSetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::VSSetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::VSSetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        //m_ppShaderResourceViews = ppShaderResourceViews;
        m_ppShaderResourceViews = DCUtils::CopyArrayOfInterfaces(NumViews, ppShaderResourceViews);
        m_CMDType = DC_CMD_Type_VSSetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->VSSetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Constructor
    DC_CMD_VSSetShaderResources()
    {

    }

    /// Destructor
    ~DC_CMD_VSSetShaderResources()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumViews, m_ppShaderResourceViews);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("VSSetShaderResources");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_VSSetShaderResources(const DC_CMD_VSSetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_VSSetShaderResources& operator= (const DC_CMD_VSSetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// VSSetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_VSSetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::VSSetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::VSSetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::VSSetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_VSSetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::VSSetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::VSSetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::VSSetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        //m_ppSamplers = ppSamplers;
        m_ppSamplers = DCUtils::CopyArrayOfInterfaces<ID3D11SamplerState>(NumSamplers, ppSamplers);
        m_CMDType = DC_CMD_Type_VSSetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->VSSetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Constructor
    DC_CMD_VSSetSamplers()
    {

    }

    /// Destructor
    ~DC_CMD_VSSetSamplers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumSamplers, m_ppSamplers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("VSSetSamplers");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_VSSetSamplers(const DC_CMD_VSSetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_VSSetSamplers& operator= (const DC_CMD_VSSetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// Begin
//------------------------------------------------------------------------------------
class DC_CMD_Begin : public DC_CMDBase
{
    ID3D11Asynchronous* m_pAsync;             ///< Parameters for function ID3D11DeviceContext::Begin()

public:
    /// This is called in Mine_ID3D11DeviceContext_Begin()
    /// \param pObj Deferred context
    /// \param pAsync Parameters for ID3D11DeviceContext::Begin()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync)
    {
        m_pObj = pObj;
        m_pAsync = pAsync;
        m_CMDType = DC_CMD_Type_Begin;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->Begin(m_pAsync);
    }

    /// Constructor
    DC_CMD_Begin()
    {
    }

    /// Destructor
    ~DC_CMD_Begin()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("Begin");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_Begin(const DC_CMD_Begin& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_Begin& operator= (const DC_CMD_Begin& obj);
};

//------------------------------------------------------------------------------------
/// End
//------------------------------------------------------------------------------------
class DC_CMD_End : public DC_CMDBase
{
    ID3D11Asynchronous* m_pAsync;             ///< Parameters for function ID3D11DeviceContext::End()

public:
    /// This is called in Mine_ID3D11DeviceContext_End()
    /// \param pObj Deferred context
    /// \param pAsync Parameters for ID3D11DeviceContext::End()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync)
    {
        m_pObj = pObj;
        m_pAsync = pAsync;
        m_CMDType = DC_CMD_Type_End;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->End(m_pAsync);
    }

    /// Constructor
    DC_CMD_End()
    {
    }

    /// Destructor
    ~DC_CMD_End()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("End");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_End(const DC_CMD_End& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_End& operator= (const DC_CMD_End& obj);
};

//------------------------------------------------------------------------------------
/// GetData
//------------------------------------------------------------------------------------
class DC_CMD_GetData : public DC_CMDBase
{
    ID3D11Asynchronous* m_pAsync;             ///< Parameters for function ID3D11DeviceContext::GetData()
    void* m_pData;             ///< Parameters for function ID3D11DeviceContext::GetData()
    UINT m_DataSize;           ///< Parameters for function ID3D11DeviceContext::GetData()
    UINT m_GetDataFlags;          ///< Parameters for function ID3D11DeviceContext::GetData()

public:
    /// This is called in Mine_ID3D11DeviceContext_GetData()
    /// \param pObj Deferred context
    /// \param pAsync Parameters for ID3D11DeviceContext::GetData()
    /// \param pData Parameters for ID3D11DeviceContext::GetData()
    /// \param DataSize Parameters for ID3D11DeviceContext::GetData()
    /// \param GetDataFlags Parameters for ID3D11DeviceContext::GetData()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Asynchronous* pAsync, void* pData, UINT DataSize, UINT GetDataFlags)
    {
        m_pObj = pObj;
        m_pAsync = pAsync;
        m_pData = pData;
        m_DataSize = DataSize;
        m_GetDataFlags = GetDataFlags;
        m_CMDType = DC_CMD_Type_GetData;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GetData(m_pAsync, m_pData, m_DataSize, m_GetDataFlags);
    }

    /// Constructor
    DC_CMD_GetData()
    {
    }

    /// Destructor
    ~DC_CMD_GetData()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("GetData");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GetData(const DC_CMD_GetData& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GetData& operator= (const DC_CMD_GetData& obj);
};

//------------------------------------------------------------------------------------
/// SetPredication
//------------------------------------------------------------------------------------
class DC_CMD_SetPredication : public DC_CMDBase
{
    ID3D11Predicate* m_pPredicate;            ///< Parameters for function ID3D11DeviceContext::SetPredication()
    BOOL m_PredicateValue;           ///< Parameters for function ID3D11DeviceContext::SetPredication()

public:
    /// This is called in Mine_ID3D11DeviceContext_SetPredication()
    /// \param pObj Deferred context
    /// \param pPredicate Parameters for ID3D11DeviceContext::SetPredication()
    /// \param PredicateValue Parameters for ID3D11DeviceContext::SetPredication()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Predicate* pPredicate, BOOL PredicateValue)
    {
        m_pObj = pObj;
        m_pPredicate = pPredicate;
        m_PredicateValue = PredicateValue;
        m_CMDType = DC_CMD_Type_SetPredication;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->SetPredication(m_pPredicate, m_PredicateValue);
    }

    /// Constructor
    DC_CMD_SetPredication()
    {
    }

    /// Destructor
    ~DC_CMD_SetPredication()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("SetPredication");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_SetPredication(const DC_CMD_SetPredication& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_SetPredication& operator= (const DC_CMD_SetPredication& obj);
};

//------------------------------------------------------------------------------------
/// GSSetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_GSSetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::GSSetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::GSSetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::GSSetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_GSSetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::GSSetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::GSSetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::GSSetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        //m_ppShaderResourceViews = ppShaderResourceViews;
        m_ppShaderResourceViews = DCUtils::CopyArrayOfInterfaces(NumViews, ppShaderResourceViews);
        m_CMDType = DC_CMD_Type_GSSetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GSSetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Constructor
    DC_CMD_GSSetShaderResources()
    {

    }

    /// Destructor
    ~DC_CMD_GSSetShaderResources()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumViews, m_ppShaderResourceViews);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GSSetShaderResources(const DC_CMD_GSSetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GSSetShaderResources& operator= (const DC_CMD_GSSetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// GSSetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_GSSetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::GSSetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::GSSetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::GSSetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_GSSetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::GSSetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::GSSetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::GSSetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        //m_ppSamplers = ppSamplers;
        m_ppSamplers = DCUtils::CopyArrayOfInterfaces<ID3D11SamplerState>(NumSamplers, ppSamplers);
        m_CMDType = DC_CMD_Type_GSSetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GSSetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Constructor
    DC_CMD_GSSetSamplers()
    {

    }

    /// Destructor
    ~DC_CMD_GSSetSamplers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumSamplers, m_ppSamplers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GSSetSamplers(const DC_CMD_GSSetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GSSetSamplers& operator= (const DC_CMD_GSSetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// OMSetRenderTargets
//------------------------------------------------------------------------------------
class DC_CMD_OMSetRenderTargets : public DC_CMDBase
{
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::OMSetRenderTargets()
    ID3D11RenderTargetView** m_ppRenderTargetViews;             ///< Parameters for function ID3D11DeviceContext::OMSetRenderTargets()
    ID3D11DepthStencilView* m_pDepthStencilView;             ///< Parameters for function ID3D11DeviceContext::OMSetRenderTargets()

public:
    /// This is called in Mine_ID3D11DeviceContext_OMSetRenderTargets()
    /// \param pObj Deferred context
    /// \param NumViews Parameters for ID3D11DeviceContext::OMSetRenderTargets()
    /// \param ppRenderTargetViews Parameters for ID3D11DeviceContext::OMSetRenderTargets()
    /// \param pDepthStencilView Parameters for ID3D11DeviceContext::OMSetRenderTargets()
    void OnCreate(ID3D11DeviceContext* pObj, UINT NumViews, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView)
    {
        m_pObj = pObj;
        m_NumViews = NumViews;
        m_ppRenderTargetViews = DCUtils::CopyArrayOfInterfaces(NumViews, ppRenderTargetViews);
        m_pDepthStencilView = pDepthStencilView;

        if (m_pDepthStencilView != NULL)
        {
            m_pDepthStencilView->AddRef();
        }

        m_CMDType = DC_CMD_Type_OMSetRenderTargets;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->OMSetRenderTargets(m_NumViews, m_ppRenderTargetViews, m_pDepthStencilView);
    }

    /// Constructor
    DC_CMD_OMSetRenderTargets()
    {

    }

    /// Destructor
    ~DC_CMD_OMSetRenderTargets()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumViews, m_ppRenderTargetViews);

        if (m_pDepthStencilView != NULL)
        {
            m_pDepthStencilView->Release();
        }
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("OMSetRenderTargets");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_OMSetRenderTargets(const DC_CMD_OMSetRenderTargets& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_OMSetRenderTargets& operator= (const DC_CMD_OMSetRenderTargets& obj);
};

//------------------------------------------------------------------------------------
/// OMSetRenderTargetsAndUnorderedAccessViews
//------------------------------------------------------------------------------------
class DC_CMD_OMSetRenderTargetsAndUnorderedAccessViews : public DC_CMDBase
{
    UINT m_NumRTVs;            ///< Parameters for function ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    ID3D11RenderTargetView** m_ppRenderTargetViews;             ///< Parameters for function ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    ID3D11DepthStencilView* m_pDepthStencilView;             ///< Parameters for function ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    UINT m_UAVStartSlot;          ///< Parameters for function ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    UINT m_NumUAVs;            ///< Parameters for function ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    ID3D11UnorderedAccessView** m_ppUnorderedAccessViews;             ///< Parameters for function ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    UINT* m_pUAVInitialCounts;             ///< Parameters for function ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()

public:
    /// This is called in Mine_ID3D11DeviceContext_OMSetRenderTargetsAndUnorderedAccessViews()
    /// \param pObj Deferred context
    /// \param NumRTVs Parameters for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    /// \param ppRenderTargetViews Parameters for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    /// \param pDepthStencilView Parameters for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    /// \param UAVStartSlot Parameters for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    /// \param NumUAVs Parameters for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    /// \param ppUnorderedAccessViews Parameters for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    /// \param pUAVInitialCounts Parameters for ID3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews()
    void OnCreate(ID3D11DeviceContext* pObj, UINT NumRTVs, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts)
    {
        m_pObj = pObj;
        m_NumRTVs = NumRTVs;
        m_ppRenderTargetViews = DCUtils::CopyArrayOfInterfaces(NumRTVs, ppRenderTargetViews);
        m_pDepthStencilView = pDepthStencilView;

        if (m_pDepthStencilView != NULL)
        {
            m_pDepthStencilView->AddRef();
        }

        m_UAVStartSlot = UAVStartSlot;
        m_NumUAVs = NumUAVs;
        m_ppUnorderedAccessViews = DCUtils::CopyArrayOfInterfaces(NumUAVs, ppUnorderedAccessViews);

        if (pUAVInitialCounts != NULL)
        {
            m_pUAVInitialCounts = (UINT*)malloc(sizeof(UINT) * m_NumUAVs);
            memcpy(m_pUAVInitialCounts, pUAVInitialCounts, sizeof(UINT) * m_NumUAVs);
        }
        else
        {
            m_pUAVInitialCounts = NULL;
        }
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->OMSetRenderTargetsAndUnorderedAccessViews(m_NumRTVs, m_ppRenderTargetViews, m_pDepthStencilView, m_UAVStartSlot, m_NumUAVs, m_ppUnorderedAccessViews, m_pUAVInitialCounts);
    }

    /// Constructor
    DC_CMD_OMSetRenderTargetsAndUnorderedAccessViews()
    {

    }

    /// Destructor
    ~DC_CMD_OMSetRenderTargetsAndUnorderedAccessViews()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumRTVs, m_ppRenderTargetViews);
        DCUtils::ReleaseArrayOfInterfaces(m_NumUAVs, m_ppUnorderedAccessViews);

        if (m_pDepthStencilView != NULL)
        {
            m_pDepthStencilView->Release();
        }

        if (m_pUAVInitialCounts)
        {
            free(m_pUAVInitialCounts);
        }
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("OMSetRenderTargetsAndUnorderedAccessViews");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_OMSetRenderTargetsAndUnorderedAccessViews(const DC_CMD_OMSetRenderTargetsAndUnorderedAccessViews& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_OMSetRenderTargetsAndUnorderedAccessViews& operator= (const DC_CMD_OMSetRenderTargetsAndUnorderedAccessViews& obj);
};

//------------------------------------------------------------------------------------
/// OMSetBlendState
//------------------------------------------------------------------------------------
class DC_CMD_OMSetBlendState : public DC_CMDBase
{
    ID3D11BlendState* m_pBlendState;             ///< Parameters for function ID3D11DeviceContext::OMSetBlendState()
    FLOAT m_BlendFactor[ 4 ];           ///< Parameters for function ID3D11DeviceContext::OMSetBlendState()
    UINT m_SampleMask;            ///< Parameters for function ID3D11DeviceContext::OMSetBlendState()

public:
    /// This is called in Mine_ID3D11DeviceContext_OMSetBlendState()
    /// \param pObj Deferred context
    /// \param pBlendState Parameters for ID3D11DeviceContext::OMSetBlendState()
    /// \param BlendFactor Parameters for ID3D11DeviceContext::OMSetBlendState()
    /// \param SampleMask Parameters for ID3D11DeviceContext::OMSetBlendState()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11BlendState* pBlendState, const FLOAT BlendFactor[ 4 ], UINT SampleMask)
    {
        m_pObj = pObj;
        m_pBlendState = pBlendState;

        m_BlendFactor[0] = BlendFactor[0];
        m_BlendFactor[1] = BlendFactor[1];
        m_BlendFactor[2] = BlendFactor[2];
        m_BlendFactor[3] = BlendFactor[3];

        m_SampleMask = SampleMask;
        m_CMDType = DC_CMD_Type_OMSetBlendState;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->OMSetBlendState(m_pBlendState, m_BlendFactor, m_SampleMask);
    }

    /// Constructor
    DC_CMD_OMSetBlendState()
    {
    }

    /// Destructor
    ~DC_CMD_OMSetBlendState()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("OMSetBlendState");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_OMSetBlendState(const DC_CMD_OMSetBlendState& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_OMSetBlendState& operator= (const DC_CMD_OMSetBlendState& obj);
};

//------------------------------------------------------------------------------------
/// OMSetDepthStencilState
//------------------------------------------------------------------------------------
class DC_CMD_OMSetDepthStencilState : public DC_CMDBase
{
    ID3D11DepthStencilState* m_pDepthStencilState;           ///< Parameters for function ID3D11DeviceContext::OMSetDepthStencilState()
    UINT m_StencilRef;            ///< Parameters for function ID3D11DeviceContext::OMSetDepthStencilState()

public:
    /// This is called in Mine_ID3D11DeviceContext_OMSetDepthStencilState()
    /// \param pObj Deferred context
    /// \param pDepthStencilState Parameters for ID3D11DeviceContext::OMSetDepthStencilState()
    /// \param StencilRef Parameters for ID3D11DeviceContext::OMSetDepthStencilState()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11DepthStencilState* pDepthStencilState, UINT StencilRef)
    {
        m_pObj = pObj;
        m_pDepthStencilState = pDepthStencilState;
        m_StencilRef = StencilRef;
        m_CMDType = DC_CMD_Type_OMSetDepthStencilState;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, m_StencilRef);
    }

    /// Constructor
    DC_CMD_OMSetDepthStencilState()
    {
    }

    /// Destructor
    ~DC_CMD_OMSetDepthStencilState()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("OMSetDepthStencilState");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_OMSetDepthStencilState(const DC_CMD_OMSetDepthStencilState& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_OMSetDepthStencilState& operator= (const DC_CMD_OMSetDepthStencilState& obj);
};

//------------------------------------------------------------------------------------
/// SOSetTargets
//------------------------------------------------------------------------------------
class DC_CMD_SOSetTargets : public DC_CMDBase
{
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::SOSetTargets()
    ID3D11Buffer** m_ppSOTargets;             ///< Parameters for function ID3D11DeviceContext::SOSetTargets()
    UINT* m_pOffsets;             ///< Parameters for function ID3D11DeviceContext::SOSetTargets()

public:
    /// This is called in Mine_ID3D11DeviceContext_SOSetTargets()
    /// \param pObj Deferred context
    /// \param NumBuffers Parameters for ID3D11DeviceContext::SOSetTargets()
    /// \param ppSOTargets Parameters for ID3D11DeviceContext::SOSetTargets()
    /// \param pOffsets Parameters for ID3D11DeviceContext::SOSetTargets()
    void OnCreate(ID3D11DeviceContext* pObj, UINT NumBuffers, ID3D11Buffer* const* ppSOTargets, const UINT* pOffsets)
    {
        m_pObj = pObj;
        m_NumBuffers = NumBuffers;
        /*m_ppSOTargets = ppSOTargets;
        m_pOffsets = pOffsets;*/
        m_ppSOTargets = DCUtils::CopyArrayOfInterfaces(NumBuffers, ppSOTargets);
        m_pOffsets = DCUtils::CopyArrays(NumBuffers, pOffsets);
        m_CMDType = DC_CMD_Type_SOSetTargets;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->SOSetTargets(m_NumBuffers, m_ppSOTargets, m_pOffsets);
    }

    /// Constructor
    DC_CMD_SOSetTargets()
    {

    }

    /// Destructor
    ~DC_CMD_SOSetTargets()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumBuffers, m_ppSOTargets);
        free(m_pOffsets);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("SOSetTargets");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_SOSetTargets(const DC_CMD_SOSetTargets& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_SOSetTargets& operator= (const DC_CMD_SOSetTargets& obj);
};

//------------------------------------------------------------------------------------
/// DrawAuto
//------------------------------------------------------------------------------------
class DC_CMD_DrawAuto : public DC_CMDBase
{

public:
    /// This is called in Mine_ID3D11DeviceContext_DrawAuto()
    /// \param pObj Deferred context
    void OnCreate(ID3D11DeviceContext* pObj)
    {
        m_pObj = pObj;
        m_CMDType = DC_CMD_Type_DrawAuto;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DrawAuto();
    }

    /// Constructor
    DC_CMD_DrawAuto()
    {
    }

    /// Destructor
    ~DC_CMD_DrawAuto()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("DrawAuto");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DrawAuto(const DC_CMD_DrawAuto& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DrawAuto& operator= (const DC_CMD_DrawAuto& obj);
};

//------------------------------------------------------------------------------------
/// DrawIndexedInstancedIndirect
//------------------------------------------------------------------------------------
class DC_CMD_DrawIndexedInstancedIndirect : public DC_CMDBase
{
    ID3D11Buffer* m_pBufferForArgs;           ///< Parameters for function ID3D11DeviceContext::DrawIndexedInstancedIndirect()
    UINT m_AlignedByteOffsetForArgs;          ///< Parameters for function ID3D11DeviceContext::DrawIndexedInstancedIndirect()

public:
    /// This is called in Mine_ID3D11DeviceContext_DrawIndexedInstancedIndirect()
    /// \param pObj Deferred context
    /// \param pBufferForArgs Parameters for ID3D11DeviceContext::DrawIndexedInstancedIndirect()
    /// \param AlignedByteOffsetForArgs Parameters for ID3D11DeviceContext::DrawIndexedInstancedIndirect()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
    {
        m_pObj = pObj;
        m_pBufferForArgs = pBufferForArgs;
        m_AlignedByteOffsetForArgs = AlignedByteOffsetForArgs;
        m_CMDType = DC_CMD_Type_DrawIndexedInstancedIndirect;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DrawIndexedInstancedIndirect(m_pBufferForArgs, m_AlignedByteOffsetForArgs);
    }

    /// Constructor
    DC_CMD_DrawIndexedInstancedIndirect()
    {
    }

    /// Destructor
    ~DC_CMD_DrawIndexedInstancedIndirect()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("DrawIndexedInstancedIndirect");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DrawIndexedInstancedIndirect(const DC_CMD_DrawIndexedInstancedIndirect& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DrawIndexedInstancedIndirect& operator= (const DC_CMD_DrawIndexedInstancedIndirect& obj);
};

//------------------------------------------------------------------------------------
/// DrawInstancedIndirect
//------------------------------------------------------------------------------------
class DC_CMD_DrawInstancedIndirect : public DC_CMDBase
{
    ID3D11Buffer* m_pBufferForArgs;           ///< Parameters for function ID3D11DeviceContext::DrawInstancedIndirect()
    UINT m_AlignedByteOffsetForArgs;          ///< Parameters for function ID3D11DeviceContext::DrawInstancedIndirect()

public:
    /// This is called in Mine_ID3D11DeviceContext_DrawInstancedIndirect()
    /// \param pObj Deferred context
    /// \param pBufferForArgs Parameters for ID3D11DeviceContext::DrawInstancedIndirect()
    /// \param AlignedByteOffsetForArgs Parameters for ID3D11DeviceContext::DrawInstancedIndirect()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
    {
        m_pObj = pObj;
        m_pBufferForArgs = pBufferForArgs;
        m_AlignedByteOffsetForArgs = AlignedByteOffsetForArgs;
        m_CMDType = DC_CMD_Type_DrawInstancedIndirect;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DrawInstancedIndirect(m_pBufferForArgs, m_AlignedByteOffsetForArgs);
    }

    /// Constructor
    DC_CMD_DrawInstancedIndirect()
    {
    }

    /// Destructor
    ~DC_CMD_DrawInstancedIndirect()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("DrawInstancedIndirect");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DrawInstancedIndirect(const DC_CMD_DrawInstancedIndirect& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DrawInstancedIndirect& operator= (const DC_CMD_DrawInstancedIndirect& obj);
};

//------------------------------------------------------------------------------------
/// Dispatch
//------------------------------------------------------------------------------------
class DC_CMD_Dispatch : public DC_CMDBase
{
    UINT m_ThreadGroupCountX;           ///< Parameters for function ID3D11DeviceContext::Dispatch()
    UINT m_ThreadGroupCountY;           ///< Parameters for function ID3D11DeviceContext::Dispatch()
    UINT m_ThreadGroupCountZ;           ///< Parameters for function ID3D11DeviceContext::Dispatch()

public:
    /// This is called in Mine_ID3D11DeviceContext_Dispatch()
    /// \param pObj Deferred context
    /// \param ThreadGroupCountX Parameters for ID3D11DeviceContext::Dispatch()
    /// \param ThreadGroupCountY Parameters for ID3D11DeviceContext::Dispatch()
    /// \param ThreadGroupCountZ Parameters for ID3D11DeviceContext::Dispatch()
    void OnCreate(ID3D11DeviceContext* pObj, UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
    {
        m_pObj = pObj;
        m_ThreadGroupCountX = ThreadGroupCountX;
        m_ThreadGroupCountY = ThreadGroupCountY;
        m_ThreadGroupCountZ = ThreadGroupCountZ;
        m_CMDType = DC_CMD_Type_Dispatch;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->Dispatch(m_ThreadGroupCountX, m_ThreadGroupCountY, m_ThreadGroupCountZ);
    }

    /// Constructor
    DC_CMD_Dispatch()
    {
    }

    /// Destructor
    ~DC_CMD_Dispatch()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("Dispatch");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_Dispatch(const DC_CMD_Dispatch& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_Dispatch& operator= (const DC_CMD_Dispatch& obj);
};

//------------------------------------------------------------------------------------
/// DispatchIndirect
//------------------------------------------------------------------------------------
class DC_CMD_DispatchIndirect : public DC_CMDBase
{
    ID3D11Buffer* m_pBufferForArgs;           ///< Parameters for function ID3D11DeviceContext::DispatchIndirect()
    UINT m_AlignedByteOffsetForArgs;          ///< Parameters for function ID3D11DeviceContext::DispatchIndirect()

public:
    /// This is called in Mine_ID3D11DeviceContext_DispatchIndirect()
    /// \param pObj Deferred context
    /// \param pBufferForArgs Parameters for ID3D11DeviceContext::DispatchIndirect()
    /// \param AlignedByteOffsetForArgs Parameters for ID3D11DeviceContext::DispatchIndirect()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
    {
        m_pObj = pObj;
        m_pBufferForArgs = pBufferForArgs;
        m_AlignedByteOffsetForArgs = AlignedByteOffsetForArgs;
        m_CMDType = DC_CMD_Type_DispatchIndirect;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DispatchIndirect(m_pBufferForArgs, m_AlignedByteOffsetForArgs);
    }

    /// Constructor
    DC_CMD_DispatchIndirect()
    {
    }

    /// Destructor
    ~DC_CMD_DispatchIndirect()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("DispatchIndirect");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DispatchIndirect(const DC_CMD_DispatchIndirect& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DispatchIndirect& operator= (const DC_CMD_DispatchIndirect& obj);
};

//------------------------------------------------------------------------------------
/// RSSetState
//------------------------------------------------------------------------------------
class DC_CMD_RSSetState : public DC_CMDBase
{
    ID3D11RasterizerState* m_pRasterizerState;            ///< Parameters for function ID3D11DeviceContext::RSSetState()

public:
    /// This is called in Mine_ID3D11DeviceContext_RSSetState()
    /// \param pObj Deferred context
    /// \param pRasterizerState Parameters for ID3D11DeviceContext::RSSetState()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11RasterizerState* pRasterizerState)
    {
        m_pObj = pObj;
        m_pRasterizerState = pRasterizerState;
        m_CMDType = DC_CMD_Type_RSSetState;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->RSSetState(m_pRasterizerState);
    }

    /// Constructor
    DC_CMD_RSSetState()
    {
    }

    /// Destructor
    ~DC_CMD_RSSetState()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("RSSetState");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_RSSetState(const DC_CMD_RSSetState& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_RSSetState& operator= (const DC_CMD_RSSetState& obj);
};

//------------------------------------------------------------------------------------
/// RSSetViewports
//------------------------------------------------------------------------------------
class DC_CMD_RSSetViewports : public DC_CMDBase
{
    UINT m_NumViewports;          ///< Parameters for function ID3D11DeviceContext::RSSetViewports()
    D3D11_VIEWPORT* m_pViewports;             ///< Parameters for function ID3D11DeviceContext::RSSetViewports()

public:
    /// This is called in Mine_ID3D11DeviceContext_RSSetViewports()
    /// \param pObj Deferred context
    /// \param NumViewports Parameters for ID3D11DeviceContext::RSSetViewports()
    /// \param pViewports Parameters for ID3D11DeviceContext::RSSetViewports()
    void OnCreate(ID3D11DeviceContext* pObj, UINT NumViewports, const D3D11_VIEWPORT* pViewports)
    {
        m_pObj = pObj;
        m_NumViewports = NumViewports;

        //m_pViewports = pViewports;
        if (m_NumViewports > 0)
        {
            m_pViewports = new D3D11_VIEWPORT[m_NumViewports];
            memcpy(m_pViewports, pViewports, sizeof(D3D11_VIEWPORT) * m_NumViewports);
        }
        else
        {
            m_pViewports = NULL;
        }

        m_CMDType = DC_CMD_Type_RSSetViewports;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->RSSetViewports(m_NumViewports, m_pViewports);
    }

    /// Constructor
    DC_CMD_RSSetViewports()
    {
        m_pViewports = NULL;
    }

    /// Destructor
    ~DC_CMD_RSSetViewports()
    {
        if (m_pViewports)
        {
            delete [] m_pViewports;
        }
    }

    /// ToString, debug use
    std::string ToString()
    {
#ifdef SIMPLE_STRING
        return std::string("RSSetViewports");
#else
        return StringUtils::FormatString("RSSetViewports( 0x%p, %u, 0x%p )", m_pObj, m_NumViewports, m_pViewports);
#endif
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_RSSetViewports(const DC_CMD_RSSetViewports& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_RSSetViewports& operator= (const DC_CMD_RSSetViewports& obj);
};

//------------------------------------------------------------------------------------
/// RSSetScissorRects
//------------------------------------------------------------------------------------
class DC_CMD_RSSetScissorRects : public DC_CMDBase
{
    UINT m_NumRects;           ///< Parameters for function ID3D11DeviceContext::RSSetScissorRects()
    D3D11_RECT* m_pRects;            ///< Parameters for function ID3D11DeviceContext::RSSetScissorRects()

public:
    /// This is called in Mine_ID3D11DeviceContext_RSSetScissorRects()
    /// \param pObj Deferred context
    /// \param NumRects Parameters for ID3D11DeviceContext::RSSetScissorRects()
    /// \param pRects Parameters for ID3D11DeviceContext::RSSetScissorRects()
    void OnCreate(ID3D11DeviceContext* pObj, UINT NumRects, const D3D11_RECT* pRects)
    {
        m_pObj = pObj;
        m_NumRects = NumRects;
        //m_pRects = pRects;
        m_pRects = DCUtils::CopyArrays(NumRects, pRects);
        m_CMDType = DC_CMD_Type_RSSetScissorRects;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->RSSetScissorRects(m_NumRects, m_pRects);
    }

    /// Constructor
    DC_CMD_RSSetScissorRects()
    {
    }

    /// Destructor
    ~DC_CMD_RSSetScissorRects()
    {
        free(m_pRects);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("RSSetScissorRects");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_RSSetScissorRects(const DC_CMD_RSSetScissorRects& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_RSSetScissorRects& operator= (const DC_CMD_RSSetScissorRects& obj);
};

//------------------------------------------------------------------------------------
/// CopySubresourceRegion
//------------------------------------------------------------------------------------
class DC_CMD_CopySubresourceRegion : public DC_CMDBase
{
    ID3D11Resource* m_pDstResource;           ///< Parameters for function ID3D11DeviceContext::CopySubresourceRegion()
    UINT m_DstSubresource;           ///< Parameters for function ID3D11DeviceContext::CopySubresourceRegion()
    UINT m_DstX;            ///< Parameters for function ID3D11DeviceContext::CopySubresourceRegion()
    UINT m_DstY;            ///< Parameters for function ID3D11DeviceContext::CopySubresourceRegion()
    UINT m_DstZ;            ///< Parameters for function ID3D11DeviceContext::CopySubresourceRegion()
    ID3D11Resource* m_pSrcResource;           ///< Parameters for function ID3D11DeviceContext::CopySubresourceRegion()
    UINT m_SrcSubresource;           ///< Parameters for function ID3D11DeviceContext::CopySubresourceRegion()
    D3D11_BOX* m_pSrcBox;            ///< Parameters for function ID3D11DeviceContext::CopySubresourceRegion()
    D3D11_BOX  m_SrcBox;          ///< Parameters for function ID3D11DeviceContext::CopySubresourceRegion()

public:
    /// This is called in Mine_ID3D11DeviceContext_CopySubresourceRegion()
    /// \param pObj Deferred context
    /// \param pDstResource Parameters for ID3D11DeviceContext::CopySubresourceRegion()
    /// \param DstSubresource Parameters for ID3D11DeviceContext::CopySubresourceRegion()
    /// \param DstX Parameters for ID3D11DeviceContext::CopySubresourceRegion()
    /// \param DstY Parameters for ID3D11DeviceContext::CopySubresourceRegion()
    /// \param DstZ Parameters for ID3D11DeviceContext::CopySubresourceRegion()
    /// \param pSrcResource Parameters for ID3D11DeviceContext::CopySubresourceRegion()
    /// \param SrcSubresource Parameters for ID3D11DeviceContext::CopySubresourceRegion()
    /// \param pSrcBox Parameters for ID3D11DeviceContext::CopySubresourceRegion()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource* pSrcResource, UINT SrcSubresource, const D3D11_BOX* pSrcBox)
    {
        m_pObj = pObj;
        m_pDstResource = pDstResource;
        m_DstSubresource = DstSubresource;
        m_DstX = DstX;
        m_DstY = DstY;
        m_DstZ = DstZ;
        m_pSrcResource = pSrcResource;
        m_SrcSubresource = SrcSubresource;

        if (pSrcBox != NULL)
        {
            m_SrcBox = *pSrcBox;
            m_pSrcBox = &m_SrcBox;
        }
        else
        {
            m_pSrcBox = NULL;
        }
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CopySubresourceRegion(m_pDstResource, m_DstSubresource, m_DstX, m_DstY, m_DstZ, m_pSrcResource, m_SrcSubresource, m_pSrcBox);
    }

    /// Constructor
    DC_CMD_CopySubresourceRegion()
    {
    }

    /// Destructor
    ~DC_CMD_CopySubresourceRegion()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("CopySubresourceRegion");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CopySubresourceRegion(const DC_CMD_CopySubresourceRegion& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CopySubresourceRegion& operator= (const DC_CMD_CopySubresourceRegion& obj);
};

//------------------------------------------------------------------------------------
/// CopyResource
//------------------------------------------------------------------------------------
class DC_CMD_CopyResource : public DC_CMDBase
{
    ID3D11Resource* m_pDstResource;           ///< Parameters for function ID3D11DeviceContext::CopyResource()
    ID3D11Resource* m_pSrcResource;           ///< Parameters for function ID3D11DeviceContext::CopyResource()

public:
    /// This is called in Mine_ID3D11DeviceContext_CopyResource()
    /// \param pObj Deferred context
    /// \param pDstResource Parameters for ID3D11DeviceContext::CopyResource()
    /// \param pSrcResource Parameters for ID3D11DeviceContext::CopyResource()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource)
    {
        m_pObj = pObj;
        m_pDstResource = pDstResource;
        m_pSrcResource = pSrcResource;
        m_CMDType = DC_CMD_Type_CopyResource;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CopyResource(m_pDstResource, m_pSrcResource);
    }

    /// Constructor
    DC_CMD_CopyResource()
    {
    }

    /// Destructor
    ~DC_CMD_CopyResource()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("CopyResource");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CopyResource(const DC_CMD_CopyResource& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CopyResource& operator= (const DC_CMD_CopyResource& obj);
};

//------------------------------------------------------------------------------------
/// UpdateSubresource
//------------------------------------------------------------------------------------
class DC_CMD_UpdateSubresource : public DC_CMDBase
{
    ID3D11Resource* m_pDstResource;           ///< Parameters for function ID3D11DeviceContext::UpdateSubresource()
    UINT m_DstSubresource;           ///< Parameters for function ID3D11DeviceContext::UpdateSubresource()
    D3D11_BOX* m_pDstBox;            ///< Parameters for function ID3D11DeviceContext::UpdateSubresource()
    D3D11_BOX m_DstBox;
    const void* m_pSrcData;             ///< Parameters for function ID3D11DeviceContext::UpdateSubresource()
    UINT m_SrcRowPitch;           ///< Parameters for function ID3D11DeviceContext::UpdateSubresource()
    UINT m_SrcDepthPitch;            ///< Parameters for function ID3D11DeviceContext::UpdateSubresource()

public:
    /// This is called in Mine_ID3D11DeviceContext_UpdateSubresource()
    /// \param pObj Deferred context
    /// \param pDstResource Parameters for ID3D11DeviceContext::UpdateSubresource()
    /// \param DstSubresource Parameters for ID3D11DeviceContext::UpdateSubresource()
    /// \param pDstBox Parameters for ID3D11DeviceContext::UpdateSubresource()
    /// \param pSrcData Parameters for ID3D11DeviceContext::UpdateSubresource()
    /// \param SrcRowPitch Parameters for ID3D11DeviceContext::UpdateSubresource()
    /// \param SrcDepthPitch Parameters for ID3D11DeviceContext::UpdateSubresource()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, const D3D11_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch)
    {
        m_pObj = pObj;
        m_pDstResource = pDstResource;
        m_DstSubresource = DstSubresource;

        if (pDstBox != NULL)
        {
            m_DstBox = *pDstBox;
            m_pDstBox = &m_DstBox;
        }
        else
        {
            m_pDstBox = NULL;
        }

        m_pSrcData = pSrcData;
        m_SrcRowPitch = SrcRowPitch;
        m_SrcDepthPitch = SrcDepthPitch;
        m_CMDType = DC_CMD_Type_UpdateSubresource;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->UpdateSubresource(m_pDstResource, m_DstSubresource, m_pDstBox, m_pSrcData, m_SrcRowPitch, m_SrcDepthPitch);
    }

    /// Constructor
    DC_CMD_UpdateSubresource()
    {

    }

    /// Destructor
    ~DC_CMD_UpdateSubresource()
    {

    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("UpdateSubresource");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_UpdateSubresource(const DC_CMD_UpdateSubresource& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_UpdateSubresource& operator= (const DC_CMD_UpdateSubresource& obj);
};

//------------------------------------------------------------------------------------
/// CopyStructureCount
//------------------------------------------------------------------------------------
class DC_CMD_CopyStructureCount : public DC_CMDBase
{
    ID3D11Buffer* m_pDstBuffer;            ///< Parameters for function ID3D11DeviceContext::CopyStructureCount()
    UINT m_DstAlignedByteOffset;           ///< Parameters for function ID3D11DeviceContext::CopyStructureCount()
    ID3D11UnorderedAccessView* m_pSrcView;             ///< Parameters for function ID3D11DeviceContext::CopyStructureCount()

public:
    /// This is called in Mine_ID3D11DeviceContext_CopyStructureCount()
    /// \param pObj Deferred context
    /// \param pDstBuffer Parameters for ID3D11DeviceContext::CopyStructureCount()
    /// \param DstAlignedByteOffset Parameters for ID3D11DeviceContext::CopyStructureCount()
    /// \param pSrcView Parameters for ID3D11DeviceContext::CopyStructureCount()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Buffer* pDstBuffer, UINT DstAlignedByteOffset, ID3D11UnorderedAccessView* pSrcView)
    {
        m_pObj = pObj;
        m_pDstBuffer = pDstBuffer;
        m_DstAlignedByteOffset = DstAlignedByteOffset;
        m_pSrcView = pSrcView;
        m_CMDType = DC_CMD_Type_CopyStructureCount;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CopyStructureCount(m_pDstBuffer, m_DstAlignedByteOffset, m_pSrcView);
    }

    /// Constructor
    DC_CMD_CopyStructureCount()
    {
    }

    /// Destructor
    ~DC_CMD_CopyStructureCount()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("CopyStructureCount");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CopyStructureCount(const DC_CMD_CopyStructureCount& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CopyStructureCount& operator= (const DC_CMD_CopyStructureCount& obj);
};

//------------------------------------------------------------------------------------
/// ClearRenderTargetView
//------------------------------------------------------------------------------------
class DC_CMD_ClearRenderTargetView : public DC_CMDBase
{
    ID3D11RenderTargetView* m_pRenderTargetView;             ///< Parameters for function ID3D11DeviceContext::ClearRenderTargetView()
    FLOAT m_ColorRGBA[ 4 ];          ///< Parameters for function ID3D11DeviceContext::ClearRenderTargetView()

public:
    /// This is called in Mine_ID3D11DeviceContext_ClearRenderTargetView()
    /// \param pObj Deferred context
    /// \param pRenderTargetView Parameters for ID3D11DeviceContext::ClearRenderTargetView()
    /// \param ColorRGBA Parameters for ID3D11DeviceContext::ClearRenderTargetView()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11RenderTargetView* pRenderTargetView, const FLOAT ColorRGBA[ 4 ])
    {
        m_pObj = pObj;
        m_pRenderTargetView = pRenderTargetView;

        m_ColorRGBA[0] = ColorRGBA[0];
        m_ColorRGBA[1] = ColorRGBA[1];
        m_ColorRGBA[2] = ColorRGBA[2];
        m_ColorRGBA[3] = ColorRGBA[3];

        m_CMDType = DC_CMD_Type_ClearRenderTargetView;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, m_ColorRGBA);
    }

    /// Constructor
    DC_CMD_ClearRenderTargetView()
    {
    }

    /// Destructor
    ~DC_CMD_ClearRenderTargetView()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("ClearRenderTargetView");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_ClearRenderTargetView(const DC_CMD_ClearRenderTargetView& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_ClearRenderTargetView& operator= (const DC_CMD_ClearRenderTargetView& obj);
};

//------------------------------------------------------------------------------------
/// ClearUnorderedAccessViewUint
//------------------------------------------------------------------------------------
class DC_CMD_ClearUnorderedAccessViewUint : public DC_CMDBase
{
    ID3D11UnorderedAccessView* m_pUnorderedAccessView;             ///< Parameters for function ID3D11DeviceContext::ClearUnorderedAccessViewUint()
    UINT m_Values[ 4 ];           ///< Parameters for function ID3D11DeviceContext::ClearUnorderedAccessViewUint()

public:
    /// This is called in Mine_ID3D11DeviceContext_ClearUnorderedAccessViewUint()
    /// \param pObj Deferred context
    /// \param pUnorderedAccessView Parameters for ID3D11DeviceContext::ClearUnorderedAccessViewUint()
    /// \param Values Parameters for ID3D11DeviceContext::ClearUnorderedAccessViewUint()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11UnorderedAccessView* pUnorderedAccessView, const UINT Values[ 4 ])
    {
        m_pObj = pObj;
        m_pUnorderedAccessView = pUnorderedAccessView;

        m_Values[0] = Values[0];
        m_Values[1] = Values[1];
        m_Values[2] = Values[2];
        m_Values[3] = Values[3];

        m_CMDType = DC_CMD_Type_ClearUnorderedAccessViewUint;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->ClearUnorderedAccessViewUint(m_pUnorderedAccessView, m_Values);
    }

    /// Constructor
    DC_CMD_ClearUnorderedAccessViewUint()
    {
    }

    /// Destructor
    ~DC_CMD_ClearUnorderedAccessViewUint()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("ClearUnorderedAccessViewUint");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_ClearUnorderedAccessViewUint(const DC_CMD_ClearUnorderedAccessViewUint& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_ClearUnorderedAccessViewUint& operator= (const DC_CMD_ClearUnorderedAccessViewUint& obj);
};

//------------------------------------------------------------------------------------
/// ClearUnorderedAccessViewFloat
//------------------------------------------------------------------------------------
class DC_CMD_ClearUnorderedAccessViewFloat : public DC_CMDBase
{
    ID3D11UnorderedAccessView* m_pUnorderedAccessView;             ///< Parameters for function ID3D11DeviceContext::ClearUnorderedAccessViewFloat()
    FLOAT m_Values[ 4 ];          ///< Parameters for function ID3D11DeviceContext::ClearUnorderedAccessViewFloat()

public:
    /// This is called in Mine_ID3D11DeviceContext_ClearUnorderedAccessViewFloat()
    /// \param pObj Deferred context
    /// \param pUnorderedAccessView Parameters for ID3D11DeviceContext::ClearUnorderedAccessViewFloat()
    /// \param Values Parameters for ID3D11DeviceContext::ClearUnorderedAccessViewFloat()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11UnorderedAccessView* pUnorderedAccessView, const FLOAT Values[ 4 ])
    {
        m_pObj = pObj;
        m_pUnorderedAccessView = pUnorderedAccessView;

        m_Values[0] = Values[0];
        m_Values[1] = Values[1];
        m_Values[2] = Values[2];
        m_Values[3] = Values[3];

        m_CMDType = DC_CMD_Type_ClearUnorderedAccessViewFloat;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->ClearUnorderedAccessViewFloat(m_pUnorderedAccessView, m_Values);
    }

    /// Constructor
    DC_CMD_ClearUnorderedAccessViewFloat()
    {
    }

    /// Destructor
    ~DC_CMD_ClearUnorderedAccessViewFloat()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("ClearUnorderedAccessViewFloat");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_ClearUnorderedAccessViewFloat(const DC_CMD_ClearUnorderedAccessViewFloat& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_ClearUnorderedAccessViewFloat& operator= (const DC_CMD_ClearUnorderedAccessViewFloat& obj);
};

//------------------------------------------------------------------------------------
/// ClearDepthStencilView
//------------------------------------------------------------------------------------
class DC_CMD_ClearDepthStencilView : public DC_CMDBase
{
    ID3D11DepthStencilView* m_pDepthStencilView;             ///< Parameters for function ID3D11DeviceContext::ClearDepthStencilView()
    UINT m_ClearFlags;            ///< Parameters for function ID3D11DeviceContext::ClearDepthStencilView()
    FLOAT m_Depth;          ///< Parameters for function ID3D11DeviceContext::ClearDepthStencilView()
    UINT8 m_Stencil;           ///< Parameters for function ID3D11DeviceContext::ClearDepthStencilView()

public:
    /// This is called in Mine_ID3D11DeviceContext_ClearDepthStencilView()
    /// \param pObj Deferred context
    /// \param pDepthStencilView Parameters for ID3D11DeviceContext::ClearDepthStencilView()
    /// \param ClearFlags Parameters for ID3D11DeviceContext::ClearDepthStencilView()
    /// \param Depth Parameters for ID3D11DeviceContext::ClearDepthStencilView()
    /// \param Stencil Parameters for ID3D11DeviceContext::ClearDepthStencilView()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11DepthStencilView* pDepthStencilView, UINT ClearFlags, FLOAT Depth, UINT8 Stencil)
    {
        m_pObj = pObj;
        m_pDepthStencilView = pDepthStencilView;
        m_ClearFlags = ClearFlags;
        m_Depth = Depth;
        m_Stencil = Stencil;
        m_CMDType = DC_CMD_Type_ClearDepthStencilView;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, m_ClearFlags, m_Depth, m_Stencil);
    }

    /// Constructor
    DC_CMD_ClearDepthStencilView()
    {
    }

    /// Destructor
    ~DC_CMD_ClearDepthStencilView()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("ClearDepthStencilView");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_ClearDepthStencilView(const DC_CMD_ClearDepthStencilView& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_ClearDepthStencilView& operator= (const DC_CMD_ClearDepthStencilView& obj);
};

//------------------------------------------------------------------------------------
/// GenerateMips
//------------------------------------------------------------------------------------
class DC_CMD_GenerateMips : public DC_CMDBase
{
    ID3D11ShaderResourceView* m_pShaderResourceView;            ///< Parameters for function ID3D11DeviceContext::GenerateMips()

public:
    /// This is called in Mine_ID3D11DeviceContext_GenerateMips()
    /// \param pObj Deferred context
    /// \param pShaderResourceView Parameters for ID3D11DeviceContext::GenerateMips()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11ShaderResourceView* pShaderResourceView)
    {
        m_pObj = pObj;
        m_pShaderResourceView = pShaderResourceView;
        m_CMDType = DC_CMD_Type_GenerateMips;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GenerateMips(m_pShaderResourceView);
    }

    /// Constructor
    DC_CMD_GenerateMips()
    {
    }

    /// Destructor
    ~DC_CMD_GenerateMips()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("GenerateMips");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GenerateMips(const DC_CMD_GenerateMips& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GenerateMips& operator= (const DC_CMD_GenerateMips& obj);
};

//------------------------------------------------------------------------------------
/// SetResourceMinLOD
//------------------------------------------------------------------------------------
class DC_CMD_SetResourceMinLOD : public DC_CMDBase
{
    ID3D11Resource* m_pResource;           ///< Parameters for function ID3D11DeviceContext::SetResourceMinLOD()
    FLOAT m_MinLOD;            ///< Parameters for function ID3D11DeviceContext::SetResourceMinLOD()

public:
    /// This is called in Mine_ID3D11DeviceContext_SetResourceMinLOD()
    /// \param pObj Deferred context
    /// \param pResource Parameters for ID3D11DeviceContext::SetResourceMinLOD()
    /// \param MinLOD Parameters for ID3D11DeviceContext::SetResourceMinLOD()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Resource* pResource, FLOAT MinLOD)
    {
        m_pObj = pObj;
        m_pResource = pResource;
        m_MinLOD = MinLOD;
        m_CMDType = DC_CMD_Type_SetResourceMinLOD;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->SetResourceMinLOD(m_pResource, m_MinLOD);
    }

    /// Constructor
    DC_CMD_SetResourceMinLOD()
    {
    }

    /// Destructor
    ~DC_CMD_SetResourceMinLOD()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("SetResourceMinLOD");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_SetResourceMinLOD(const DC_CMD_SetResourceMinLOD& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_SetResourceMinLOD& operator= (const DC_CMD_SetResourceMinLOD& obj);
};

//------------------------------------------------------------------------------------
/// GetResourceMinLOD
//------------------------------------------------------------------------------------
class DC_CMD_GetResourceMinLOD : public DC_CMDBase
{
    ID3D11Resource* m_pResource;           ///< Parameters for function ID3D11DeviceContext::GetResourceMinLOD()

public:
    /// This is called in Mine_ID3D11DeviceContext_GetResourceMinLOD()
    /// \param pObj Deferred context
    /// \param pResource Parameters for ID3D11DeviceContext::GetResourceMinLOD()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Resource* pResource)
    {
        m_pObj = pObj;
        m_pResource = pResource;
        m_CMDType = DC_CMD_Type_GetResourceMinLOD;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GetResourceMinLOD(m_pResource);
    }

    /// Constructor
    DC_CMD_GetResourceMinLOD()
    {
    }

    /// Destructor
    ~DC_CMD_GetResourceMinLOD()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("GetResourceMinLOD");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GetResourceMinLOD(const DC_CMD_GetResourceMinLOD& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GetResourceMinLOD& operator= (const DC_CMD_GetResourceMinLOD& obj);
};

//------------------------------------------------------------------------------------
/// ResolveSubresource
//------------------------------------------------------------------------------------
class DC_CMD_ResolveSubresource : public DC_CMDBase
{
    ID3D11Resource* m_pDstResource;           ///< Parameters for function ID3D11DeviceContext::ResolveSubresource()
    UINT m_DstSubresource;           ///< Parameters for function ID3D11DeviceContext::ResolveSubresource()
    ID3D11Resource* m_pSrcResource;           ///< Parameters for function ID3D11DeviceContext::ResolveSubresource()
    UINT m_SrcSubresource;           ///< Parameters for function ID3D11DeviceContext::ResolveSubresource()
    DXGI_FORMAT m_Format;            ///< Parameters for function ID3D11DeviceContext::ResolveSubresource()

public:
    /// This is called in Mine_ID3D11DeviceContext_ResolveSubresource()
    /// \param pObj Deferred context
    /// \param pDstResource Parameters for ID3D11DeviceContext::ResolveSubresource()
    /// \param DstSubresource Parameters for ID3D11DeviceContext::ResolveSubresource()
    /// \param pSrcResource Parameters for ID3D11DeviceContext::ResolveSubresource()
    /// \param SrcSubresource Parameters for ID3D11DeviceContext::ResolveSubresource()
    /// \param Format Parameters for ID3D11DeviceContext::ResolveSubresource()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Resource* pDstResource, UINT DstSubresource, ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format)
    {
        m_pObj = pObj;
        m_pDstResource = pDstResource;
        m_DstSubresource = DstSubresource;
        m_pSrcResource = pSrcResource;
        m_SrcSubresource = SrcSubresource;
        m_Format = Format;
        m_CMDType = DC_CMD_Type_ResolveSubresource;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->ResolveSubresource(m_pDstResource, m_DstSubresource, m_pSrcResource, m_SrcSubresource, m_Format);
    }

    /// Constructor
    DC_CMD_ResolveSubresource()
    {
    }

    /// Destructor
    ~DC_CMD_ResolveSubresource()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("ResolveSubresource");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_ResolveSubresource(const DC_CMD_ResolveSubresource& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_ResolveSubresource& operator= (const DC_CMD_ResolveSubresource& obj);
};

//------------------------------------------------------------------------------------
/// ExecuteCommandList
//------------------------------------------------------------------------------------
class DC_CMD_ExecuteCommandList : public DC_CMDBase
{
    ID3D11CommandList* m_pCommandList;           ///< Parameters for function ID3D11DeviceContext::ExecuteCommandList()
    BOOL m_RestoreContextState;            ///< Parameters for function ID3D11DeviceContext::ExecuteCommandList()

public:
    /// This is called in Mine_ID3D11DeviceContext_ExecuteCommandList()
    /// \param pObj Deferred context
    /// \param pCommandList Parameters for ID3D11DeviceContext::ExecuteCommandList()
    /// \param RestoreContextState Parameters for ID3D11DeviceContext::ExecuteCommandList()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11CommandList* pCommandList, BOOL RestoreContextState)
    {
        m_pObj = pObj;
        m_pCommandList = pCommandList;
        m_RestoreContextState = RestoreContextState;
        m_CMDType = DC_CMD_Type_ExecuteCommandList;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->ExecuteCommandList(m_pCommandList, m_RestoreContextState);
    }

    /// Constructor
    DC_CMD_ExecuteCommandList()
    {
    }

    /// Destructor
    ~DC_CMD_ExecuteCommandList()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("ExecuteCommandList");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_ExecuteCommandList(const DC_CMD_ExecuteCommandList& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_ExecuteCommandList& operator= (const DC_CMD_ExecuteCommandList& obj);
};

//------------------------------------------------------------------------------------
/// HSSetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_HSSetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::HSSetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::HSSetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::HSSetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_HSSetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::HSSetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::HSSetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::HSSetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        //m_ppShaderResourceViews = ppShaderResourceViews;
        m_ppShaderResourceViews = DCUtils::CopyArrayOfInterfaces(NumViews, ppShaderResourceViews);
        m_CMDType = DC_CMD_Type_HSSetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->HSSetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Constructor
    DC_CMD_HSSetShaderResources()
    {

    }

    /// Destructor
    ~DC_CMD_HSSetShaderResources()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumViews, m_ppShaderResourceViews);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("HSSetShaderResources");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_HSSetShaderResources(const DC_CMD_HSSetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_HSSetShaderResources& operator= (const DC_CMD_HSSetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// HSSetShader
//------------------------------------------------------------------------------------
class DC_CMD_HSSetShader : public DC_CMDBase
{
    ID3D11HullShader* m_pHullShader;             ///< Parameters for function ID3D11DeviceContext::HSSetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::HSSetShader()
    UINT m_NumClassInstances;           ///< Parameters for function ID3D11DeviceContext::HSSetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_HSSetShader()
    /// \param pObj Deferred context
    /// \param pHullShader Parameters for ID3D11DeviceContext::HSSetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::HSSetShader()
    /// \param NumClassInstances Parameters for ID3D11DeviceContext::HSSetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11HullShader* pHullShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pObj = pObj;
        m_pHullShader = pHullShader;
        //m_ppClassInstances = ppClassInstances;
        m_ppClassInstances = DCUtils::CopyArrayOfInterfaces(NumClassInstances, ppClassInstances);
        m_NumClassInstances = NumClassInstances;
        m_CMDType = DC_CMD_Type_HSSetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->HSSetShader(m_pHullShader, m_ppClassInstances, m_NumClassInstances);
    }

    /// Constructor
    DC_CMD_HSSetShader()
    {

    }

    /// Destructor
    ~DC_CMD_HSSetShader()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumClassInstances, m_ppClassInstances);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("HSSetShader");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_HSSetShader(const DC_CMD_HSSetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_HSSetShader& operator= (const DC_CMD_HSSetShader& obj);
};

//------------------------------------------------------------------------------------
/// HSSetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_HSSetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::HSSetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::HSSetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::HSSetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_HSSetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::HSSetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::HSSetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::HSSetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        //m_ppSamplers = ppSamplers;
        m_ppSamplers = DCUtils::CopyArrayOfInterfaces<ID3D11SamplerState>(NumSamplers, ppSamplers);
        m_CMDType = DC_CMD_Type_HSSetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->HSSetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Constructor
    DC_CMD_HSSetSamplers()
    {

    }

    /// Destructor
    ~DC_CMD_HSSetSamplers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumSamplers, m_ppSamplers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("HSSetSamplers");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_HSSetSamplers(const DC_CMD_HSSetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_HSSetSamplers& operator= (const DC_CMD_HSSetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// HSSetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_HSSetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::HSSetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::HSSetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::HSSetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_HSSetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::HSSetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::HSSetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::HSSetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        //m_ppConstantBuffers = ppConstantBuffers;
        m_ppConstantBuffers = DCUtils::CopyArrayOfInterfaces(NumBuffers, ppConstantBuffers);
        m_CMDType = DC_CMD_Type_HSSetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->HSSetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Constructor
    DC_CMD_HSSetConstantBuffers()
    {

    }

    /// Destructor
    ~DC_CMD_HSSetConstantBuffers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumBuffers, m_ppConstantBuffers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("HSSetConstantBuffers");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_HSSetConstantBuffers(const DC_CMD_HSSetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_HSSetConstantBuffers& operator= (const DC_CMD_HSSetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// DSSetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_DSSetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::DSSetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::DSSetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::DSSetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_DSSetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::DSSetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::DSSetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::DSSetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        //m_ppShaderResourceViews = ppShaderResourceViews;
        m_ppShaderResourceViews = DCUtils::CopyArrayOfInterfaces(NumViews, ppShaderResourceViews);
        m_CMDType = DC_CMD_Type_DSSetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DSSetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Constructor
    DC_CMD_DSSetShaderResources()
    {

    }

    /// Destructor
    ~DC_CMD_DSSetShaderResources()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumViews, m_ppShaderResourceViews);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("DSSetShaderResources");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DSSetShaderResources(const DC_CMD_DSSetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DSSetShaderResources& operator= (const DC_CMD_DSSetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// DSSetShader
//------------------------------------------------------------------------------------
class DC_CMD_DSSetShader : public DC_CMDBase
{
    ID3D11DomainShader* m_pDomainShader;            ///< Parameters for function ID3D11DeviceContext::DSSetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::DSSetShader()
    UINT m_NumClassInstances;           ///< Parameters for function ID3D11DeviceContext::DSSetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_DSSetShader()
    /// \param pObj Deferred context
    /// \param pDomainShader Parameters for ID3D11DeviceContext::DSSetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::DSSetShader()
    /// \param NumClassInstances Parameters for ID3D11DeviceContext::DSSetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11DomainShader* pDomainShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pObj = pObj;
        m_pDomainShader = pDomainShader;
        //m_ppClassInstances = ppClassInstances;
        m_ppClassInstances = DCUtils::CopyArrayOfInterfaces(NumClassInstances, ppClassInstances);
        m_NumClassInstances = NumClassInstances;
        m_CMDType = DC_CMD_Type_DSSetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DSSetShader(m_pDomainShader, m_ppClassInstances, m_NumClassInstances);
    }

    /// Constructor
    DC_CMD_DSSetShader()
    {

    }

    /// Destructor
    ~DC_CMD_DSSetShader()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumClassInstances, m_ppClassInstances);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("DSSetShader");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DSSetShader(const DC_CMD_DSSetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DSSetShader& operator= (const DC_CMD_DSSetShader& obj);
};

//------------------------------------------------------------------------------------
/// DSSetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_DSSetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::DSSetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::DSSetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::DSSetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_DSSetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::DSSetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::DSSetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::DSSetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        //m_ppSamplers = ppSamplers;
        m_ppSamplers = DCUtils::CopyArrayOfInterfaces<ID3D11SamplerState>(NumSamplers, ppSamplers);
        m_CMDType = DC_CMD_Type_DSSetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DSSetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Constructor
    DC_CMD_DSSetSamplers()
    {

    }

    /// Destructor
    ~DC_CMD_DSSetSamplers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumSamplers, m_ppSamplers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("DSSetSamplers");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DSSetSamplers(const DC_CMD_DSSetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DSSetSamplers& operator= (const DC_CMD_DSSetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// DSSetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_DSSetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::DSSetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::DSSetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::DSSetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_DSSetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::DSSetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::DSSetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::DSSetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        //m_ppConstantBuffers = ppConstantBuffers;
        m_ppConstantBuffers = DCUtils::CopyArrayOfInterfaces(NumBuffers, ppConstantBuffers);
        m_CMDType = DC_CMD_Type_DSSetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DSSetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Constructor
    DC_CMD_DSSetConstantBuffers()
    {

    }

    /// Destructor
    ~DC_CMD_DSSetConstantBuffers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumBuffers, m_ppConstantBuffers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("DSSetConstantBuffers");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DSSetConstantBuffers(const DC_CMD_DSSetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DSSetConstantBuffers& operator= (const DC_CMD_DSSetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// CSSetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_CSSetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::CSSetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::CSSetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::CSSetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_CSSetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::CSSetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::CSSetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::CSSetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        //m_ppShaderResourceViews = ppShaderResourceViews;
        m_ppShaderResourceViews = DCUtils::CopyArrayOfInterfaces(NumViews, ppShaderResourceViews);
        m_CMDType = DC_CMD_Type_CSSetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CSSetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Constructor
    DC_CMD_CSSetShaderResources()
    {

    }

    /// Destructor
    ~DC_CMD_CSSetShaderResources()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumViews, m_ppShaderResourceViews);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("CSSetShaderResources");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CSSetShaderResources(const DC_CMD_CSSetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CSSetShaderResources& operator= (const DC_CMD_CSSetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// CSSetUnorderedAccessViews
//------------------------------------------------------------------------------------
class DC_CMD_CSSetUnorderedAccessViews : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::CSSetUnorderedAccessViews()
    UINT m_NumUAVs;            ///< Parameters for function ID3D11DeviceContext::CSSetUnorderedAccessViews()
    ID3D11UnorderedAccessView** m_ppUnorderedAccessViews;             ///< Parameters for function ID3D11DeviceContext::CSSetUnorderedAccessViews()
    UINT* m_pUAVInitialCounts;             ///< Parameters for function ID3D11DeviceContext::CSSetUnorderedAccessViews()

public:
    /// This is called in Mine_ID3D11DeviceContext_CSSetUnorderedAccessViews()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::CSSetUnorderedAccessViews()
    /// \param NumUAVs Parameters for ID3D11DeviceContext::CSSetUnorderedAccessViews()
    /// \param ppUnorderedAccessViews Parameters for ID3D11DeviceContext::CSSetUnorderedAccessViews()
    /// \param pUAVInitialCounts Parameters for ID3D11DeviceContext::CSSetUnorderedAccessViews()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumUAVs = NumUAVs;
        //m_ppUnorderedAccessViews = ppUnorderedAccessViews;
        m_ppUnorderedAccessViews = DCUtils::CopyArrayOfInterfaces(NumUAVs, ppUnorderedAccessViews);

        //m_pUAVInitialCounts = pUAVInitialCounts;
        if (pUAVInitialCounts != NULL)
        {
            m_pUAVInitialCounts = new UINT[m_NumUAVs];
            memcpy(m_pUAVInitialCounts, pUAVInitialCounts, sizeof(UINT) * m_NumUAVs);
        }
        else
        {
            m_pUAVInitialCounts = NULL;
        }

        m_CMDType = DC_CMD_Type_CSSetUnorderedAccessViews;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CSSetUnorderedAccessViews(m_StartSlot, m_NumUAVs, m_ppUnorderedAccessViews, m_pUAVInitialCounts);
    }

    /// Constructor
    DC_CMD_CSSetUnorderedAccessViews()
    {
    }

    /// Destructor
    ~DC_CMD_CSSetUnorderedAccessViews()
    {
        if (m_pUAVInitialCounts != NULL)
        {
            delete [] m_pUAVInitialCounts;
        }
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("CSSetUnorderedAccessViews");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CSSetUnorderedAccessViews(const DC_CMD_CSSetUnorderedAccessViews& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CSSetUnorderedAccessViews& operator= (const DC_CMD_CSSetUnorderedAccessViews& obj);
};

//------------------------------------------------------------------------------------
/// CSSetShader
//------------------------------------------------------------------------------------
class DC_CMD_CSSetShader : public DC_CMDBase
{
    ID3D11ComputeShader* m_pComputeShader;             ///< Parameters for function ID3D11DeviceContext::CSSetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::CSSetShader()
    UINT m_NumClassInstances;           ///< Parameters for function ID3D11DeviceContext::CSSetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_CSSetShader()
    /// \param pObj Deferred context
    /// \param pComputeShader Parameters for ID3D11DeviceContext::CSSetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::CSSetShader()
    /// \param NumClassInstances Parameters for ID3D11DeviceContext::CSSetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11ComputeShader* pComputeShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances)
    {
        m_pObj = pObj;
        m_pComputeShader = pComputeShader;
        //m_ppClassInstances = ppClassInstances;
        m_ppClassInstances = DCUtils::CopyArrayOfInterfaces(NumClassInstances, ppClassInstances);
        m_NumClassInstances = NumClassInstances;
        m_CMDType = DC_CMD_Type_CSSetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        // call mine func, not real func
        pImmediateContext->CSSetShader(m_pComputeShader, m_ppClassInstances, m_NumClassInstances);
    }

    /// Constructor
    DC_CMD_CSSetShader()
    {

    }

    /// Destructor
    ~DC_CMD_CSSetShader()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumClassInstances, m_ppClassInstances);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("CSSetShader");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CSSetShader(const DC_CMD_CSSetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CSSetShader& operator= (const DC_CMD_CSSetShader& obj);
};

//------------------------------------------------------------------------------------
/// CSSetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_CSSetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::CSSetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::CSSetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::CSSetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_CSSetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::CSSetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::CSSetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::CSSetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        //m_ppSamplers = ppSamplers;
        m_ppSamplers = DCUtils::CopyArrayOfInterfaces<ID3D11SamplerState>(NumSamplers, ppSamplers);
        m_CMDType = DC_CMD_Type_CSSetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CSSetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Constructor
    DC_CMD_CSSetSamplers()
    {

    }

    /// Destructor
    ~DC_CMD_CSSetSamplers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumSamplers, m_ppSamplers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("CSSetSamplers");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CSSetSamplers(const DC_CMD_CSSetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CSSetSamplers& operator= (const DC_CMD_CSSetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// CSSetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_CSSetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::CSSetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::CSSetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::CSSetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_CSSetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::CSSetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::CSSetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::CSSetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        //m_ppConstantBuffers = ppConstantBuffers;
        m_ppConstantBuffers = DCUtils::CopyArrayOfInterfaces(NumBuffers, ppConstantBuffers);
        m_CMDType = DC_CMD_Type_CSSetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CSSetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Constructor
    DC_CMD_CSSetConstantBuffers()
    {

    }

    /// Destructor
    ~DC_CMD_CSSetConstantBuffers()
    {
        DCUtils::ReleaseArrayOfInterfaces(m_NumBuffers, m_ppConstantBuffers);
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("CSSetConstantBuffers");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CSSetConstantBuffers(const DC_CMD_CSSetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CSSetConstantBuffers& operator= (const DC_CMD_CSSetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// VSGetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_VSGetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::VSGetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::VSGetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::VSGetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_VSGetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::VSGetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::VSGetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::VSGetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        m_ppConstantBuffers = ppConstantBuffers;
        m_CMDType = DC_CMD_Type_VSGetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->VSGetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Constructor
    DC_CMD_VSGetConstantBuffers()
    {

    }

    /// Destructor
    ~DC_CMD_VSGetConstantBuffers()
    {

    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_VSGetConstantBuffers(const DC_CMD_VSGetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_VSGetConstantBuffers& operator= (const DC_CMD_VSGetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// PSGetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_PSGetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::PSGetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::PSGetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::PSGetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_PSGetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::PSGetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::PSGetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::PSGetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        m_ppShaderResourceViews = ppShaderResourceViews;
        m_CMDType = DC_CMD_Type_PSGetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->PSGetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Destructor
    ~DC_CMD_PSGetShaderResources()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_PSGetShaderResources(const DC_CMD_PSGetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_PSGetShaderResources& operator= (const DC_CMD_PSGetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// PSGetShader
//------------------------------------------------------------------------------------
class DC_CMD_PSGetShader : public DC_CMDBase
{
    ID3D11PixelShader** m_ppPixelShader;            ///< Parameters for function ID3D11DeviceContext::PSGetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::PSGetShader()
    UINT* m_pNumClassInstances;            ///< Parameters for function ID3D11DeviceContext::PSGetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_PSGetShader()
    /// \param pObj Deferred context
    /// \param ppPixelShader Parameters for ID3D11DeviceContext::PSGetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::PSGetShader()
    /// \param pNumClassInstances Parameters for ID3D11DeviceContext::PSGetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11PixelShader** ppPixelShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pObj = pObj;
        m_ppPixelShader = ppPixelShader;
        m_ppClassInstances = ppClassInstances;
        m_pNumClassInstances = pNumClassInstances;
        m_CMDType = DC_CMD_Type_PSGetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->PSGetShader(m_ppPixelShader, m_ppClassInstances, m_pNumClassInstances);
    }

    /// Destructor
    ~DC_CMD_PSGetShader()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_PSGetShader(const DC_CMD_PSGetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_PSGetShader& operator= (const DC_CMD_PSGetShader& obj);
};

//------------------------------------------------------------------------------------
/// PSGetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_PSGetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::PSGetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::PSGetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::PSGetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_PSGetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::PSGetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::PSGetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::PSGetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        m_ppSamplers = ppSamplers;
        m_CMDType = DC_CMD_Type_PSGetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->PSGetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Destructor
    ~DC_CMD_PSGetSamplers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_PSGetSamplers(const DC_CMD_PSGetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_PSGetSamplers& operator= (const DC_CMD_PSGetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// VSGetShader
//------------------------------------------------------------------------------------
class DC_CMD_VSGetShader : public DC_CMDBase
{
    ID3D11VertexShader** m_ppVertexShader;             ///< Parameters for function ID3D11DeviceContext::VSGetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::VSGetShader()
    UINT* m_pNumClassInstances;            ///< Parameters for function ID3D11DeviceContext::VSGetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_VSGetShader()
    /// \param pObj Deferred context
    /// \param ppVertexShader Parameters for ID3D11DeviceContext::VSGetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::VSGetShader()
    /// \param pNumClassInstances Parameters for ID3D11DeviceContext::VSGetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11VertexShader** ppVertexShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pObj = pObj;
        m_ppVertexShader = ppVertexShader;
        m_ppClassInstances = ppClassInstances;
        m_pNumClassInstances = pNumClassInstances;
        m_CMDType = DC_CMD_Type_VSGetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->VSGetShader(m_ppVertexShader, m_ppClassInstances, m_pNumClassInstances);
    }

    /// Destructor
    ~DC_CMD_VSGetShader()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_VSGetShader(const DC_CMD_VSGetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_VSGetShader& operator= (const DC_CMD_VSGetShader& obj);
};

//------------------------------------------------------------------------------------
/// PSGetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_PSGetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::PSGetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::PSGetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::PSGetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_PSGetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::PSGetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::PSGetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::PSGetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        m_ppConstantBuffers = ppConstantBuffers;
        m_CMDType = DC_CMD_Type_PSGetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->PSGetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Destructor
    ~DC_CMD_PSGetConstantBuffers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_PSGetConstantBuffers(const DC_CMD_PSGetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_PSGetConstantBuffers& operator= (const DC_CMD_PSGetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// IAGetInputLayout
//------------------------------------------------------------------------------------
class DC_CMD_IAGetInputLayout : public DC_CMDBase
{
    ID3D11InputLayout** m_ppInputLayout;            ///< Parameters for function ID3D11DeviceContext::IAGetInputLayout()

public:
    /// This is called in Mine_ID3D11DeviceContext_IAGetInputLayout()
    /// \param pObj Deferred context
    /// \param ppInputLayout Parameters for ID3D11DeviceContext::IAGetInputLayout()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11InputLayout** ppInputLayout)
    {
        m_pObj = pObj;
        m_ppInputLayout = ppInputLayout;
        m_CMDType = DC_CMD_Type_IAGetInputLayout;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->IAGetInputLayout(m_ppInputLayout);
    }

    /// Destructor
    ~DC_CMD_IAGetInputLayout()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_IAGetInputLayout(const DC_CMD_IAGetInputLayout& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_IAGetInputLayout& operator= (const DC_CMD_IAGetInputLayout& obj);
};

//------------------------------------------------------------------------------------
/// IAGetVertexBuffers
//------------------------------------------------------------------------------------
class DC_CMD_IAGetVertexBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::IAGetVertexBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::IAGetVertexBuffers()
    ID3D11Buffer** m_ppVertexBuffers;            ///< Parameters for function ID3D11DeviceContext::IAGetVertexBuffers()
    UINT* m_pStrides;             ///< Parameters for function ID3D11DeviceContext::IAGetVertexBuffers()
    UINT* m_pOffsets;             ///< Parameters for function ID3D11DeviceContext::IAGetVertexBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_IAGetVertexBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::IAGetVertexBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::IAGetVertexBuffers()
    /// \param ppVertexBuffers Parameters for ID3D11DeviceContext::IAGetVertexBuffers()
    /// \param pStrides Parameters for ID3D11DeviceContext::IAGetVertexBuffers()
    /// \param pOffsets Parameters for ID3D11DeviceContext::IAGetVertexBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppVertexBuffers, UINT* pStrides, UINT* pOffsets)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        m_ppVertexBuffers = ppVertexBuffers;
        m_pStrides = pStrides;
        m_pOffsets = pOffsets;
        m_CMDType = DC_CMD_Type_IAGetVertexBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->IAGetVertexBuffers(m_StartSlot, m_NumBuffers, m_ppVertexBuffers, m_pStrides, m_pOffsets);
    }

    /// Destructor
    ~DC_CMD_IAGetVertexBuffers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_IAGetVertexBuffers(const DC_CMD_IAGetVertexBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_IAGetVertexBuffers& operator= (const DC_CMD_IAGetVertexBuffers& obj);
};

//------------------------------------------------------------------------------------
/// IAGetIndexBuffer
//------------------------------------------------------------------------------------
class DC_CMD_IAGetIndexBuffer : public DC_CMDBase
{
    ID3D11Buffer** m_pIndexBuffer;            ///< Parameters for function ID3D11DeviceContext::IAGetIndexBuffer()
    DXGI_FORMAT* m_Format;           ///< Parameters for function ID3D11DeviceContext::IAGetIndexBuffer()
    UINT* m_Offset;            ///< Parameters for function ID3D11DeviceContext::IAGetIndexBuffer()

public:
    /// This is called in Mine_ID3D11DeviceContext_IAGetIndexBuffer()
    /// \param pObj Deferred context
    /// \param pIndexBuffer Parameters for ID3D11DeviceContext::IAGetIndexBuffer()
    /// \param Format Parameters for ID3D11DeviceContext::IAGetIndexBuffer()
    /// \param Offset Parameters for ID3D11DeviceContext::IAGetIndexBuffer()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Buffer** pIndexBuffer, DXGI_FORMAT* Format, UINT* Offset)
    {
        m_pObj = pObj;
        m_pIndexBuffer = pIndexBuffer;
        m_Format = Format;
        m_Offset = Offset;
        m_CMDType = DC_CMD_Type_IAGetIndexBuffer;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->IAGetIndexBuffer(m_pIndexBuffer, m_Format, m_Offset);
    }

    /// Destructor
    ~DC_CMD_IAGetIndexBuffer()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_IAGetIndexBuffer(const DC_CMD_IAGetIndexBuffer& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_IAGetIndexBuffer& operator= (const DC_CMD_IAGetIndexBuffer& obj);
};

//------------------------------------------------------------------------------------
/// GSGetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_GSGetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::GSGetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::GSGetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::GSGetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_GSGetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::GSGetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::GSGetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::GSGetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        m_ppConstantBuffers = ppConstantBuffers;
        m_CMDType = DC_CMD_Type_GSGetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GSGetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Destructor
    ~DC_CMD_GSGetConstantBuffers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GSGetConstantBuffers(const DC_CMD_GSGetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GSGetConstantBuffers& operator= (const DC_CMD_GSGetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// GSGetShader
//------------------------------------------------------------------------------------
class DC_CMD_GSGetShader : public DC_CMDBase
{
    ID3D11GeometryShader** m_ppGeometryShader;            ///< Parameters for function ID3D11DeviceContext::GSGetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::GSGetShader()
    UINT* m_pNumClassInstances;            ///< Parameters for function ID3D11DeviceContext::GSGetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_GSGetShader()
    /// \param pObj Deferred context
    /// \param ppGeometryShader Parameters for ID3D11DeviceContext::GSGetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::GSGetShader()
    /// \param pNumClassInstances Parameters for ID3D11DeviceContext::GSGetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11GeometryShader** ppGeometryShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pObj = pObj;
        m_ppGeometryShader = ppGeometryShader;
        m_ppClassInstances = ppClassInstances;
        m_pNumClassInstances = pNumClassInstances;
        m_CMDType = DC_CMD_Type_GSGetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GSGetShader(m_ppGeometryShader, m_ppClassInstances, m_pNumClassInstances);
    }

    /// Destructor
    ~DC_CMD_GSGetShader()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GSGetShader(const DC_CMD_GSGetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GSGetShader& operator= (const DC_CMD_GSGetShader& obj);
};

//------------------------------------------------------------------------------------
/// IAGetPrimitiveTopology
//------------------------------------------------------------------------------------
class DC_CMD_IAGetPrimitiveTopology : public DC_CMDBase
{
    D3D11_PRIMITIVE_TOPOLOGY* m_pTopology;             ///< Parameters for function ID3D11DeviceContext::IAGetPrimitiveTopology()

public:
    /// This is called in Mine_ID3D11DeviceContext_IAGetPrimitiveTopology()
    /// \param pObj Deferred context
    /// \param pTopology Parameters for ID3D11DeviceContext::IAGetPrimitiveTopology()
    void OnCreate(ID3D11DeviceContext* pObj, D3D11_PRIMITIVE_TOPOLOGY* pTopology)
    {
        m_pObj = pObj;
        m_pTopology = pTopology;
        m_CMDType = DC_CMD_Type_IAGetPrimitiveTopology;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->IAGetPrimitiveTopology(m_pTopology);
    }

    /// Destructor
    ~DC_CMD_IAGetPrimitiveTopology()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_IAGetPrimitiveTopology(const DC_CMD_IAGetPrimitiveTopology& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_IAGetPrimitiveTopology& operator= (const DC_CMD_IAGetPrimitiveTopology& obj);
};

//------------------------------------------------------------------------------------
/// VSGetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_VSGetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::VSGetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::VSGetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::VSGetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_VSGetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::VSGetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::VSGetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::VSGetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        m_ppShaderResourceViews = ppShaderResourceViews;
        m_CMDType = DC_CMD_Type_VSGetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->VSGetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Destructor
    ~DC_CMD_VSGetShaderResources()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_VSGetShaderResources(const DC_CMD_VSGetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_VSGetShaderResources& operator= (const DC_CMD_VSGetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// VSGetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_VSGetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::VSGetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::VSGetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::VSGetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_VSGetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::VSGetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::VSGetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::VSGetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        m_ppSamplers = ppSamplers;
        m_CMDType = DC_CMD_Type_VSGetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->VSGetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Destructor
    ~DC_CMD_VSGetSamplers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_VSGetSamplers(const DC_CMD_VSGetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_VSGetSamplers& operator= (const DC_CMD_VSGetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// GetPredication
//------------------------------------------------------------------------------------
class DC_CMD_GetPredication : public DC_CMDBase
{
    ID3D11Predicate** m_ppPredicate;             ///< Parameters for function ID3D11DeviceContext::GetPredication()
    BOOL* m_pPredicateValue;            ///< Parameters for function ID3D11DeviceContext::GetPredication()

public:
    /// This is called in Mine_ID3D11DeviceContext_GetPredication()
    /// \param pObj Deferred context
    /// \param ppPredicate Parameters for ID3D11DeviceContext::GetPredication()
    /// \param pPredicateValue Parameters for ID3D11DeviceContext::GetPredication()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11Predicate** ppPredicate, BOOL* pPredicateValue)
    {
        m_pObj = pObj;
        m_ppPredicate = ppPredicate;
        m_pPredicateValue = pPredicateValue;
        m_CMDType = DC_CMD_Type_GetPredication;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GetPredication(m_ppPredicate, m_pPredicateValue);
    }

    /// Destructor
    ~DC_CMD_GetPredication()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GetPredication(const DC_CMD_GetPredication& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GetPredication& operator= (const DC_CMD_GetPredication& obj);
};

//------------------------------------------------------------------------------------
/// GSGetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_GSGetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::GSGetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::GSGetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::GSGetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_GSGetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::GSGetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::GSGetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::GSGetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        m_ppShaderResourceViews = ppShaderResourceViews;
        m_CMDType = DC_CMD_Type_GSGetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GSGetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Destructor
    ~DC_CMD_GSGetShaderResources()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GSGetShaderResources(const DC_CMD_GSGetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GSGetShaderResources& operator= (const DC_CMD_GSGetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// GSGetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_GSGetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::GSGetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::GSGetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::GSGetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_GSGetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::GSGetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::GSGetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::GSGetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        m_ppSamplers = ppSamplers;
        m_CMDType = DC_CMD_Type_GSGetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GSGetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Destructor
    ~DC_CMD_GSGetSamplers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GSGetSamplers(const DC_CMD_GSGetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GSGetSamplers& operator= (const DC_CMD_GSGetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// OMGetRenderTargets
//------------------------------------------------------------------------------------
class DC_CMD_OMGetRenderTargets : public DC_CMDBase
{
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::OMGetRenderTargets()
    ID3D11RenderTargetView** m_ppRenderTargetViews;             ///< Parameters for function ID3D11DeviceContext::OMGetRenderTargets()
    ID3D11DepthStencilView** m_ppDepthStencilView;           ///< Parameters for function ID3D11DeviceContext::OMGetRenderTargets()

public:
    /// This is called in Mine_ID3D11DeviceContext_OMGetRenderTargets()
    /// \param pObj Deferred context
    /// \param NumViews Parameters for ID3D11DeviceContext::OMGetRenderTargets()
    /// \param ppRenderTargetViews Parameters for ID3D11DeviceContext::OMGetRenderTargets()
    /// \param ppDepthStencilView Parameters for ID3D11DeviceContext::OMGetRenderTargets()
    void OnCreate(ID3D11DeviceContext* pObj, UINT NumViews, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView)
    {
        m_pObj = pObj;
        m_NumViews = NumViews;
        m_ppRenderTargetViews = ppRenderTargetViews;
        m_ppDepthStencilView = ppDepthStencilView;
        m_CMDType = DC_CMD_Type_OMGetRenderTargets;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->OMGetRenderTargets(m_NumViews, m_ppRenderTargetViews, m_ppDepthStencilView);
    }

    /// Destructor
    ~DC_CMD_OMGetRenderTargets()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_OMGetRenderTargets(const DC_CMD_OMGetRenderTargets& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_OMGetRenderTargets& operator= (const DC_CMD_OMGetRenderTargets& obj);
};

//------------------------------------------------------------------------------------
/// OMGetRenderTargetsAndUnorderedAccessViews
//------------------------------------------------------------------------------------
class DC_CMD_OMGetRenderTargetsAndUnorderedAccessViews : public DC_CMDBase
{
    UINT m_NumRTVs;            ///< Parameters for function ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
    ID3D11RenderTargetView** m_ppRenderTargetViews;             ///< Parameters for function ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
    ID3D11DepthStencilView** m_ppDepthStencilView;           ///< Parameters for function ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
    UINT m_UAVStartSlot;          ///< Parameters for function ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
    UINT m_NumUAVs;            ///< Parameters for function ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
    ID3D11UnorderedAccessView** m_ppUnorderedAccessViews;             ///< Parameters for function ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()

public:
    /// This is called in Mine_ID3D11DeviceContext_OMGetRenderTargetsAndUnorderedAccessViews()
    /// \param pObj Deferred context
    /// \param NumRTVs Parameters for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
    /// \param ppRenderTargetViews Parameters for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
    /// \param ppDepthStencilView Parameters for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
    /// \param UAVStartSlot Parameters for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
    /// \param NumUAVs Parameters for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
    /// \param ppUnorderedAccessViews Parameters for ID3D11DeviceContext::OMGetRenderTargetsAndUnorderedAccessViews()
    void OnCreate(ID3D11DeviceContext* pObj, UINT NumRTVs, ID3D11RenderTargetView** ppRenderTargetViews, ID3D11DepthStencilView** ppDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews)
    {
        m_pObj = pObj;
        m_NumRTVs = NumRTVs;
        m_ppRenderTargetViews = ppRenderTargetViews;
        m_ppDepthStencilView = ppDepthStencilView;
        m_UAVStartSlot = UAVStartSlot;
        m_NumUAVs = NumUAVs;
        m_ppUnorderedAccessViews = ppUnorderedAccessViews;
        m_CMDType = DC_CMD_Type_OMGetRenderTargetsAndUnorderedAccessViews;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->OMGetRenderTargetsAndUnorderedAccessViews(m_NumRTVs, m_ppRenderTargetViews, m_ppDepthStencilView, m_UAVStartSlot, m_NumUAVs, m_ppUnorderedAccessViews);
    }

    /// Destructor
    ~DC_CMD_OMGetRenderTargetsAndUnorderedAccessViews()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_OMGetRenderTargetsAndUnorderedAccessViews(const DC_CMD_OMGetRenderTargetsAndUnorderedAccessViews& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_OMGetRenderTargetsAndUnorderedAccessViews& operator= (const DC_CMD_OMGetRenderTargetsAndUnorderedAccessViews& obj);
};

//------------------------------------------------------------------------------------
/// OMGetBlendState
//------------------------------------------------------------------------------------
class DC_CMD_OMGetBlendState : public DC_CMDBase
{
    ID3D11BlendState** m_ppBlendState;           ///< Parameters for function ID3D11DeviceContext::OMGetBlendState()
    FLOAT m_BlendFactor[ 4 ];           ///< Parameters for function ID3D11DeviceContext::OMGetBlendState()
    UINT* m_pSampleMask;             ///< Parameters for function ID3D11DeviceContext::OMGetBlendState()

public:
    /// This is called in Mine_ID3D11DeviceContext_OMGetBlendState()
    /// \param pObj Deferred context
    /// \param ppBlendState Parameters for ID3D11DeviceContext::OMGetBlendState()
    /// \param BlendFactor Parameters for ID3D11DeviceContext::OMGetBlendState()
    /// \param pSampleMask Parameters for ID3D11DeviceContext::OMGetBlendState()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11BlendState** ppBlendState, FLOAT BlendFactor[ 4 ], UINT* pSampleMask)
    {
        m_pObj = pObj;
        m_ppBlendState = ppBlendState;

        m_BlendFactor[0] = BlendFactor[0];
        m_BlendFactor[1] = BlendFactor[1];
        m_BlendFactor[2] = BlendFactor[2];
        m_BlendFactor[3] = BlendFactor[3];

        m_pSampleMask = pSampleMask;
        m_CMDType = DC_CMD_Type_OMGetBlendState;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->OMGetBlendState(m_ppBlendState, m_BlendFactor, m_pSampleMask);
    }

    /// Destructor
    ~DC_CMD_OMGetBlendState()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_OMGetBlendState(const DC_CMD_OMGetBlendState& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_OMGetBlendState& operator= (const DC_CMD_OMGetBlendState& obj);
};

//------------------------------------------------------------------------------------
/// OMGetDepthStencilState
//------------------------------------------------------------------------------------
class DC_CMD_OMGetDepthStencilState : public DC_CMDBase
{
    ID3D11DepthStencilState** m_ppDepthStencilState;            ///< Parameters for function ID3D11DeviceContext::OMGetDepthStencilState()
    UINT* m_pStencilRef;             ///< Parameters for function ID3D11DeviceContext::OMGetDepthStencilState()

public:
    /// This is called in Mine_ID3D11DeviceContext_OMGetDepthStencilState()
    /// \param pObj Deferred context
    /// \param ppDepthStencilState Parameters for ID3D11DeviceContext::OMGetDepthStencilState()
    /// \param pStencilRef Parameters for ID3D11DeviceContext::OMGetDepthStencilState()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11DepthStencilState** ppDepthStencilState, UINT* pStencilRef)
    {
        m_pObj = pObj;
        m_ppDepthStencilState = ppDepthStencilState;
        m_pStencilRef = pStencilRef;
        m_CMDType = DC_CMD_Type_OMGetDepthStencilState;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->OMGetDepthStencilState(m_ppDepthStencilState, m_pStencilRef);
    }

    /// Destructor
    ~DC_CMD_OMGetDepthStencilState()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_OMGetDepthStencilState(const DC_CMD_OMGetDepthStencilState& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_OMGetDepthStencilState& operator= (const DC_CMD_OMGetDepthStencilState& obj);
};

//------------------------------------------------------------------------------------
/// SOGetTargets
//------------------------------------------------------------------------------------
class DC_CMD_SOGetTargets : public DC_CMDBase
{
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::SOGetTargets()
    ID3D11Buffer** m_ppSOTargets;             ///< Parameters for function ID3D11DeviceContext::SOGetTargets()

public:
    /// This is called in Mine_ID3D11DeviceContext_SOGetTargets()
    /// \param pObj Deferred context
    /// \param NumBuffers Parameters for ID3D11DeviceContext::SOGetTargets()
    /// \param ppSOTargets Parameters for ID3D11DeviceContext::SOGetTargets()
    void OnCreate(ID3D11DeviceContext* pObj, UINT NumBuffers, ID3D11Buffer** ppSOTargets)
    {
        m_pObj = pObj;
        m_NumBuffers = NumBuffers;
        m_ppSOTargets = ppSOTargets;
        m_CMDType = DC_CMD_Type_SOGetTargets;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->SOGetTargets(m_NumBuffers, m_ppSOTargets);
    }

    /// Destructor
    ~DC_CMD_SOGetTargets()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_SOGetTargets(const DC_CMD_SOGetTargets& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_SOGetTargets& operator= (const DC_CMD_SOGetTargets& obj);
};

//------------------------------------------------------------------------------------
/// RSGetState
//------------------------------------------------------------------------------------
class DC_CMD_RSGetState : public DC_CMDBase
{
    ID3D11RasterizerState** m_ppRasterizerState;             ///< Parameters for function ID3D11DeviceContext::RSGetState()

public:
    /// This is called in Mine_ID3D11DeviceContext_RSGetState()
    /// \param pObj Deferred context
    /// \param ppRasterizerState Parameters for ID3D11DeviceContext::RSGetState()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11RasterizerState** ppRasterizerState)
    {
        m_pObj = pObj;
        m_ppRasterizerState = ppRasterizerState;
        m_CMDType = DC_CMD_Type_RSGetState;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->RSGetState(m_ppRasterizerState);
    }

    /// Destructor
    ~DC_CMD_RSGetState()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_RSGetState(const DC_CMD_RSGetState& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_RSGetState& operator= (const DC_CMD_RSGetState& obj);
};

//------------------------------------------------------------------------------------
/// RSGetViewports
//------------------------------------------------------------------------------------
class DC_CMD_RSGetViewports : public DC_CMDBase
{
    UINT* m_pNumViewports;           ///< Parameters for function ID3D11DeviceContext::RSGetViewports()
    D3D11_VIEWPORT* m_pViewports;             ///< Parameters for function ID3D11DeviceContext::RSGetViewports()

public:
    /// This is called in Mine_ID3D11DeviceContext_RSGetViewports()
    /// \param pObj Deferred context
    /// \param pNumViewports Parameters for ID3D11DeviceContext::RSGetViewports()
    /// \param pViewports Parameters for ID3D11DeviceContext::RSGetViewports()
    void OnCreate(ID3D11DeviceContext* pObj, UINT* pNumViewports, D3D11_VIEWPORT* pViewports)
    {
        m_pObj = pObj;
        m_pNumViewports = pNumViewports;
        m_pViewports = pViewports;
        m_CMDType = DC_CMD_Type_RSGetViewports;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->RSGetViewports(m_pNumViewports, m_pViewports);
    }

    /// Destructor
    ~DC_CMD_RSGetViewports()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_RSGetViewports(const DC_CMD_RSGetViewports& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_RSGetViewports& operator= (const DC_CMD_RSGetViewports& obj);
};

//------------------------------------------------------------------------------------
/// RSGetScissorRects
//------------------------------------------------------------------------------------
class DC_CMD_RSGetScissorRects : public DC_CMDBase
{
    UINT* m_pNumRects;            ///< Parameters for function ID3D11DeviceContext::RSGetScissorRects()
    D3D11_RECT* m_pRects;            ///< Parameters for function ID3D11DeviceContext::RSGetScissorRects()

public:
    /// This is called in Mine_ID3D11DeviceContext_RSGetScissorRects()
    /// \param pObj Deferred context
    /// \param pNumRects Parameters for ID3D11DeviceContext::RSGetScissorRects()
    /// \param pRects Parameters for ID3D11DeviceContext::RSGetScissorRects()
    void OnCreate(ID3D11DeviceContext* pObj, UINT* pNumRects, D3D11_RECT* pRects)
    {
        m_pObj = pObj;
        m_pNumRects = pNumRects;
        m_pRects = pRects;
        m_CMDType = DC_CMD_Type_RSGetScissorRects;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->RSGetScissorRects(m_pNumRects, m_pRects);
    }

    /// Destructor
    ~DC_CMD_RSGetScissorRects()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_RSGetScissorRects(const DC_CMD_RSGetScissorRects& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_RSGetScissorRects& operator= (const DC_CMD_RSGetScissorRects& obj);
};

//------------------------------------------------------------------------------------
/// HSGetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_HSGetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::HSGetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::HSGetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::HSGetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_HSGetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::HSGetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::HSGetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::HSGetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        m_ppShaderResourceViews = ppShaderResourceViews;
        m_CMDType = DC_CMD_Type_HSGetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->HSGetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Destructor
    ~DC_CMD_HSGetShaderResources()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_HSGetShaderResources(const DC_CMD_HSGetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_HSGetShaderResources& operator= (const DC_CMD_HSGetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// HSGetShader
//------------------------------------------------------------------------------------
class DC_CMD_HSGetShader : public DC_CMDBase
{
    ID3D11HullShader** m_ppHullShader;           ///< Parameters for function ID3D11DeviceContext::HSGetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::HSGetShader()
    UINT* m_pNumClassInstances;            ///< Parameters for function ID3D11DeviceContext::HSGetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_HSGetShader()
    /// \param pObj Deferred context
    /// \param ppHullShader Parameters for ID3D11DeviceContext::HSGetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::HSGetShader()
    /// \param pNumClassInstances Parameters for ID3D11DeviceContext::HSGetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11HullShader** ppHullShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pObj = pObj;
        m_ppHullShader = ppHullShader;
        m_ppClassInstances = ppClassInstances;
        m_pNumClassInstances = pNumClassInstances;
        m_CMDType = DC_CMD_Type_HSGetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->HSGetShader(m_ppHullShader, m_ppClassInstances, m_pNumClassInstances);
    }

    /// Destructor
    ~DC_CMD_HSGetShader()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_HSGetShader(const DC_CMD_HSGetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_HSGetShader& operator= (const DC_CMD_HSGetShader& obj);
};

//------------------------------------------------------------------------------------
/// HSGetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_HSGetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::HSGetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::HSGetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::HSGetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_HSGetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::HSGetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::HSGetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::HSGetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        m_ppSamplers = ppSamplers;
        m_CMDType = DC_CMD_Type_HSGetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->HSGetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Destructor
    ~DC_CMD_HSGetSamplers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_HSGetSamplers(const DC_CMD_HSGetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_HSGetSamplers& operator= (const DC_CMD_HSGetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// HSGetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_HSGetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::HSGetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::HSGetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::HSGetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_HSGetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::HSGetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::HSGetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::HSGetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        m_ppConstantBuffers = ppConstantBuffers;
        m_CMDType = DC_CMD_Type_HSGetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->HSGetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Destructor
    ~DC_CMD_HSGetConstantBuffers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_HSGetConstantBuffers(const DC_CMD_HSGetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_HSGetConstantBuffers& operator= (const DC_CMD_HSGetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// DSGetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_DSGetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::DSGetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::DSGetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::DSGetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_DSGetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::DSGetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::DSGetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::DSGetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        m_ppShaderResourceViews = ppShaderResourceViews;
        m_CMDType = DC_CMD_Type_DSGetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DSGetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Destructor
    ~DC_CMD_DSGetShaderResources()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DSGetShaderResources(const DC_CMD_DSGetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DSGetShaderResources& operator= (const DC_CMD_DSGetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// DSGetShader
//------------------------------------------------------------------------------------
class DC_CMD_DSGetShader : public DC_CMDBase
{
    ID3D11DomainShader** m_ppDomainShader;             ///< Parameters for function ID3D11DeviceContext::DSGetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::DSGetShader()
    UINT* m_pNumClassInstances;            ///< Parameters for function ID3D11DeviceContext::DSGetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_DSGetShader()
    /// \param pObj Deferred context
    /// \param ppDomainShader Parameters for ID3D11DeviceContext::DSGetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::DSGetShader()
    /// \param pNumClassInstances Parameters for ID3D11DeviceContext::DSGetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11DomainShader** ppDomainShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pObj = pObj;
        m_ppDomainShader = ppDomainShader;
        m_ppClassInstances = ppClassInstances;
        m_pNumClassInstances = pNumClassInstances;
        m_CMDType = DC_CMD_Type_DSGetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DSGetShader(m_ppDomainShader, m_ppClassInstances, m_pNumClassInstances);
    }

    /// Destructor
    ~DC_CMD_DSGetShader()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DSGetShader(const DC_CMD_DSGetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DSGetShader& operator= (const DC_CMD_DSGetShader& obj);
};

//------------------------------------------------------------------------------------
/// DSGetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_DSGetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::DSGetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::DSGetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::DSGetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_DSGetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::DSGetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::DSGetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::DSGetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        m_ppSamplers = ppSamplers;
        m_CMDType = DC_CMD_Type_DSGetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DSGetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Destructor
    ~DC_CMD_DSGetSamplers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DSGetSamplers(const DC_CMD_DSGetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DSGetSamplers& operator= (const DC_CMD_DSGetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// DSGetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_DSGetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::DSGetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::DSGetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::DSGetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_DSGetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::DSGetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::DSGetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::DSGetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        m_ppConstantBuffers = ppConstantBuffers;
        m_CMDType = DC_CMD_Type_DSGetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->DSGetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Destructor
    ~DC_CMD_DSGetConstantBuffers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_DSGetConstantBuffers(const DC_CMD_DSGetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_DSGetConstantBuffers& operator= (const DC_CMD_DSGetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// CSGetShaderResources
//------------------------------------------------------------------------------------
class DC_CMD_CSGetShaderResources : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::CSGetShaderResources()
    UINT m_NumViews;           ///< Parameters for function ID3D11DeviceContext::CSGetShaderResources()
    ID3D11ShaderResourceView** m_ppShaderResourceViews;            ///< Parameters for function ID3D11DeviceContext::CSGetShaderResources()

public:
    /// This is called in Mine_ID3D11DeviceContext_CSGetShaderResources()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::CSGetShaderResources()
    /// \param NumViews Parameters for ID3D11DeviceContext::CSGetShaderResources()
    /// \param ppShaderResourceViews Parameters for ID3D11DeviceContext::CSGetShaderResources()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView** ppShaderResourceViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumViews = NumViews;
        m_ppShaderResourceViews = ppShaderResourceViews;
        m_CMDType = DC_CMD_Type_CSGetShaderResources;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CSGetShaderResources(m_StartSlot, m_NumViews, m_ppShaderResourceViews);
    }

    /// Destructor
    ~DC_CMD_CSGetShaderResources()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CSGetShaderResources(const DC_CMD_CSGetShaderResources& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CSGetShaderResources& operator= (const DC_CMD_CSGetShaderResources& obj);
};

//------------------------------------------------------------------------------------
/// CSGetUnorderedAccessViews
//------------------------------------------------------------------------------------
class DC_CMD_CSGetUnorderedAccessViews : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::CSGetUnorderedAccessViews()
    UINT m_NumUAVs;            ///< Parameters for function ID3D11DeviceContext::CSGetUnorderedAccessViews()
    ID3D11UnorderedAccessView** m_ppUnorderedAccessViews;             ///< Parameters for function ID3D11DeviceContext::CSGetUnorderedAccessViews()

public:
    /// This is called in Mine_ID3D11DeviceContext_CSGetUnorderedAccessViews()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::CSGetUnorderedAccessViews()
    /// \param NumUAVs Parameters for ID3D11DeviceContext::CSGetUnorderedAccessViews()
    /// \param ppUnorderedAccessViews Parameters for ID3D11DeviceContext::CSGetUnorderedAccessViews()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumUAVs, ID3D11UnorderedAccessView** ppUnorderedAccessViews)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumUAVs = NumUAVs;
        m_ppUnorderedAccessViews = ppUnorderedAccessViews;
        m_CMDType = DC_CMD_Type_CSGetUnorderedAccessViews;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CSGetUnorderedAccessViews(m_StartSlot, m_NumUAVs, m_ppUnorderedAccessViews);
    }

    /// Destructor
    ~DC_CMD_CSGetUnorderedAccessViews()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CSGetUnorderedAccessViews(const DC_CMD_CSGetUnorderedAccessViews& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CSGetUnorderedAccessViews& operator= (const DC_CMD_CSGetUnorderedAccessViews& obj);
};

//------------------------------------------------------------------------------------
/// CSGetShader
//------------------------------------------------------------------------------------
class DC_CMD_CSGetShader : public DC_CMDBase
{
    ID3D11ComputeShader** m_ppComputeShader;           ///< Parameters for function ID3D11DeviceContext::CSGetShader()
    ID3D11ClassInstance** m_ppClassInstances;             ///< Parameters for function ID3D11DeviceContext::CSGetShader()
    UINT* m_pNumClassInstances;            ///< Parameters for function ID3D11DeviceContext::CSGetShader()

public:
    /// This is called in Mine_ID3D11DeviceContext_CSGetShader()
    /// \param pObj Deferred context
    /// \param ppComputeShader Parameters for ID3D11DeviceContext::CSGetShader()
    /// \param ppClassInstances Parameters for ID3D11DeviceContext::CSGetShader()
    /// \param pNumClassInstances Parameters for ID3D11DeviceContext::CSGetShader()
    void OnCreate(ID3D11DeviceContext* pObj, ID3D11ComputeShader** ppComputeShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances)
    {
        m_pObj = pObj;
        m_ppComputeShader = ppComputeShader;
        m_ppClassInstances = ppClassInstances;
        m_pNumClassInstances = pNumClassInstances;
        m_CMDType = DC_CMD_Type_CSGetShader;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CSGetShader(m_ppComputeShader, m_ppClassInstances, m_pNumClassInstances);
    }

    /// Destructor
    ~DC_CMD_CSGetShader()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CSGetShader(const DC_CMD_CSGetShader& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CSGetShader& operator= (const DC_CMD_CSGetShader& obj);
};

//------------------------------------------------------------------------------------
/// CSGetSamplers
//------------------------------------------------------------------------------------
class DC_CMD_CSGetSamplers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::CSGetSamplers()
    UINT m_NumSamplers;           ///< Parameters for function ID3D11DeviceContext::CSGetSamplers()
    ID3D11SamplerState** m_ppSamplers;           ///< Parameters for function ID3D11DeviceContext::CSGetSamplers()

public:
    /// This is called in Mine_ID3D11DeviceContext_CSGetSamplers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::CSGetSamplers()
    /// \param NumSamplers Parameters for ID3D11DeviceContext::CSGetSamplers()
    /// \param ppSamplers Parameters for ID3D11DeviceContext::CSGetSamplers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState** ppSamplers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumSamplers = NumSamplers;
        m_ppSamplers = ppSamplers;
        m_CMDType = DC_CMD_Type_CSGetSamplers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CSGetSamplers(m_StartSlot, m_NumSamplers, m_ppSamplers);
    }

    /// Destructor
    ~DC_CMD_CSGetSamplers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CSGetSamplers(const DC_CMD_CSGetSamplers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CSGetSamplers& operator= (const DC_CMD_CSGetSamplers& obj);
};

//------------------------------------------------------------------------------------
/// CSGetConstantBuffers
//------------------------------------------------------------------------------------
class DC_CMD_CSGetConstantBuffers : public DC_CMDBase
{
    UINT m_StartSlot;          ///< Parameters for function ID3D11DeviceContext::CSGetConstantBuffers()
    UINT m_NumBuffers;            ///< Parameters for function ID3D11DeviceContext::CSGetConstantBuffers()
    ID3D11Buffer** m_ppConstantBuffers;             ///< Parameters for function ID3D11DeviceContext::CSGetConstantBuffers()

public:
    /// This is called in Mine_ID3D11DeviceContext_CSGetConstantBuffers()
    /// \param pObj Deferred context
    /// \param StartSlot Parameters for ID3D11DeviceContext::CSGetConstantBuffers()
    /// \param NumBuffers Parameters for ID3D11DeviceContext::CSGetConstantBuffers()
    /// \param ppConstantBuffers Parameters for ID3D11DeviceContext::CSGetConstantBuffers()
    void OnCreate(ID3D11DeviceContext* pObj, UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppConstantBuffers)
    {
        m_pObj = pObj;
        m_StartSlot = StartSlot;
        m_NumBuffers = NumBuffers;
        m_ppConstantBuffers = ppConstantBuffers;
        m_CMDType = DC_CMD_Type_CSGetConstantBuffers;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->CSGetConstantBuffers(m_StartSlot, m_NumBuffers, m_ppConstantBuffers);
    }

    /// Destructor
    ~DC_CMD_CSGetConstantBuffers()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_CSGetConstantBuffers(const DC_CMD_CSGetConstantBuffers& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_CSGetConstantBuffers& operator= (const DC_CMD_CSGetConstantBuffers& obj);
};

//------------------------------------------------------------------------------------
/// ClearState
//------------------------------------------------------------------------------------
class DC_CMD_ClearState : public DC_CMDBase
{

public:
    /// This is called in Mine_ID3D11DeviceContext_ClearState()
    /// \param pObj Deferred context
    void OnCreate(ID3D11DeviceContext* pObj)
    {
        m_pObj = pObj;
        m_CMDType = DC_CMD_Type_ClearState;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->ClearState();
    }

    DC_CMD_ClearState()
    {
    }

    /// Destructor
    ~DC_CMD_ClearState()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("ClearState");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_ClearState(const DC_CMD_ClearState& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_ClearState& operator= (const DC_CMD_ClearState& obj);
};

//------------------------------------------------------------------------------------
/// Flush
//------------------------------------------------------------------------------------
class DC_CMD_Flush : public DC_CMDBase
{

public:
    /// This is called in Mine_ID3D11DeviceContext_Flush()
    /// \param pObj Deferred context
    void OnCreate(ID3D11DeviceContext* pObj)
    {
        m_pObj = pObj;
        m_CMDType = DC_CMD_Type_Flush;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->Flush();
    }

    DC_CMD_Flush()
    {
    }

    /// Destructor
    ~DC_CMD_Flush()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("Flush");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_Flush(const DC_CMD_Flush& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_Flush& operator= (const DC_CMD_Flush& obj);
};

//------------------------------------------------------------------------------------
/// GetContextFlags
//------------------------------------------------------------------------------------
class DC_CMD_GetContextFlags : public DC_CMDBase
{

public:
    /// This is called in Mine_ID3D11DeviceContext_GetContextFlags()
    /// \param pObj Deferred context
    void OnCreate(ID3D11DeviceContext* pObj)
    {
        m_pObj = pObj;
        m_CMDType = DC_CMD_Type_GetContextFlags;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->GetContextFlags();
    }

    /// Destructor
    ~DC_CMD_GetContextFlags()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_GetContextFlags(const DC_CMD_GetContextFlags& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_GetContextFlags& operator= (const DC_CMD_GetContextFlags& obj);
};

//------------------------------------------------------------------------------------
/// FinishCommandList
//------------------------------------------------------------------------------------
class DC_CMD_FinishCommandList : public DC_CMDBase
{
    BOOL m_RestoreDeferredContextState;          ///< Parameters for function ID3D11DeviceContext::FinishCommandList()
    ID3D11CommandList** m_ppCommandList;            ///< Parameters for function ID3D11DeviceContext::FinishCommandList()

public:
    /// This is called in Mine_ID3D11DeviceContext_FinishCommandList()
    /// \param pObj Deferred context
    /// \param RestoreDeferredContextState Parameters for ID3D11DeviceContext::FinishCommandList()
    /// \param ppCommandList Parameters for ID3D11DeviceContext::FinishCommandList()
    void OnCreate(ID3D11DeviceContext* pObj, BOOL RestoreDeferredContextState, ID3D11CommandList** ppCommandList)
    {
        m_pObj = pObj;
        m_RestoreDeferredContextState = RestoreDeferredContextState;
        m_ppCommandList = ppCommandList;
        m_CMDType = DC_CMD_Type_FinishCommandList;
    }

    /// This is called when we flatten command list and execute commands on immediate context
    void Play(ID3D11DeviceContext* pImmediateContext)
    {
        pImmediateContext->FinishCommandList(m_RestoreDeferredContextState, m_ppCommandList);
    }

    /// Destructor
    ~DC_CMD_FinishCommandList()
    {
    }

    /// ToString, debug use
    std::string ToString()
    {
        return std::string("FinishCommandList");
    }

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    DC_CMD_FinishCommandList(const DC_CMD_FinishCommandList& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    DC_CMD_FinishCommandList& operator= (const DC_CMD_FinishCommandList& obj);
};

// @}

#endif _DC_COMMAND_DEFS_H_
