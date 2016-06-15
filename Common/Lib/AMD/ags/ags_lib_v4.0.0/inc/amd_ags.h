//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#ifndef AMD_AGS_H
#define AMD_AGS_H

#define AMD_AGS_VERSION_MAJOR 4
#define AMD_AGS_VERSION_MINOR 0
#define AMD_AGS_VERSION_PATCH 0

#ifdef __cplusplus
extern "C" {
#endif


#define AMD_AGS_API __declspec(dllexport)

// Forward declaration of D3D11 types
struct ID3D11Device;
struct ID3D11Resource;
struct ID3D11Buffer;
struct ID3D11Texture1D;
struct ID3D11Texture2D;
struct ID3D11Texture3D;
struct D3D11_BUFFER_DESC;
struct D3D11_TEXTURE1D_DESC;
struct D3D11_TEXTURE2D_DESC;
struct D3D11_TEXTURE3D_DESC;
struct D3D11_SUBRESOURCE_DATA;
struct tagRECT;
typedef tagRECT D3D11_RECT;

// Forward declaration of D3D12 types
struct ID3D12Device;


enum AGSReturnCode
{
    AGS_SUCCESS,
    AGS_INVALID_ARGS,
    AGS_OUT_OF_MEMORY,
    AGS_ERROR_MISSING_DLL,
    AGS_ERROR_LEGACY_DRIVER,    // returned if driver doesn't support ADL2 (from before AMD Catalyst driver 12.20)
    AGS_EXTENSION_NOT_SUPPORTED,
    AGS_ADL_FAILURE,
};

enum AGSDriverExtensionDX11
{
    AGS_DX11_EXTENSION_QUADLIST                 = 1 << 0,
    AGS_DX11_EXTENSION_SCREENRECTLIST           = 1 << 1,
    AGS_DX11_EXTENSION_UAV_OVERLAP              = 1 << 2,
    AGS_DX11_EXTENSION_DEPTH_BOUNDS_TEST        = 1 << 3,
    AGS_DX11_EXTENSION_MULTIDRAWINDIRECT        = 1 << 4,
    AGS_DX11_EXTENSION_CROSSFIRE_API            = 1 << 5,
    AGS_DX11_EXTENSION_INTRINSIC_READFIRSTLANE  = 1 << 6,
    AGS_DX11_EXTENSION_INTRINSIC_READLANE       = 1 << 7,
    AGS_DX11_EXTENSION_INTRINSIC_LANEID         = 1 << 8,
    AGS_DX11_EXTENSION_INTRINSIC_SWIZZLE        = 1 << 9,
    AGS_DX11_EXTENSION_INTRINSIC_BALLOT         = 1 << 10,
    AGS_DX11_EXTENSION_INTRINSIC_MBCOUNT        = 1 << 11,
    AGS_DX11_EXTENSION_INTRINSIC_COMPARE3       = 1 << 12,
    AGS_DX11_EXTENSION_INTRINSIC_BARYCENTRICS   = 1 << 13
};

const unsigned int AGS_DX11_SHADER_INSTRINSICS_RESOURCE_SLOT = 127;
const unsigned int AGS_DX11_SHADER_INSTRINSICS_SAMPLER_SLOT = 15;

enum AGSDriverExtensionDX12
{
    AGS_DX12_EXTENSION_INTRINSIC_READFIRSTLANE  = 1 << 0,
    AGS_DX12_EXTENSION_INTRINSIC_READLANE       = 1 << 1,
    AGS_DX12_EXTENSION_INTRINSIC_LANEID         = 1 << 2,
    AGS_DX12_EXTENSION_INTRINSIC_SWIZZLE        = 1 << 3,
    AGS_DX12_EXTENSION_INTRINSIC_BALLOT         = 1 << 4,
    AGS_DX12_EXTENSION_INTRINSIC_MBCOUNT        = 1 << 5,
    AGS_DX12_EXTENSION_INTRINSIC_COMPARE3       = 1 << 6,
    AGS_DX12_EXTENSION_INTRINSIC_BARYCENTRICS   = 1 << 7
};


// AMD shader intrinsics designated SpaceID.  Denotes Texture3D resource and static sampler used in conjuction with
// instrinsic instructions.
// Applications should use this value for D3D12_ROOT_DESCRIPTOR::RegisterSpace and
// D3D12_STATIC_SAMPLER_DESC::RegisterSpace when creating root descriptor entries for shader instrinsic Texture3D and
// static sampler.
///@note D3D reserves SpaceIDs in range 0xFFFFFFF0 - 0xFFFFFFFF
const unsigned int AGS_DX12_SHADER_INSTRINSICS_SPACE_ID = 0x7FFF0ADE; // 2147420894

enum AGSPrimitiveTopology
{
    AGS_PRIMITIVE_TOPOLOGY_QUADLIST = 7,
    AGS_PRIMITIVE_TOPOLOGY_SCREENRECTLIST = 9
};

enum AGSCrossfireMode
{
    AGS_CROSSFIRE_MODE_DRIVER_AFR = 0,  // Use the default driver-based AFR rendering
    AGS_CROSSFIRE_MODE_EXPLICIT_AFR,    // Use the AGS Crossfire API functions to perform explicit AFR rendering
    AGS_CROSSFIRE_MODE_DISABLE          // Completely disable AFR rendering
};

enum AGSAfrTransferType
{
    AGS_AFR_TRANSFER_DEFAULT                = 0, // default Crossfire driver resource tracking
    AGS_AFR_TRANSFER_DISABLE                = 1, // turn off driver resource tracking
    AGS_AFR_TRANSFER_1STEP_P2P              = 2, // app controlled GPU to next GPU transfer
    AGS_AFR_TRANSFER_2STEP_NO_BROADCAST     = 3, // app controlled GPU to next GPU transfer using intermediate system memory
    AGS_AFR_TRANSFER_2STEP_WITH_BROADCAST   = 4, // app controlled GPU to all render GPUs transfer using intermediate system memory
};

struct AGSContext;  // All function calls in AGS require a pointer to a context. This is generated via agsInit

struct AGSRect
{
    int iXOffset;
    int iYOffset;
    int iWidth;
    int iHeight;
};

struct AGSEyefinityInfo
{
    int iSLSActive;                 // Indicates if Eyefinity is active for the operating system display
                                    // index passed into atiEyefinityGetConfigInfo(). 1 if enabled and 0 if disabled.

    int iSLSGridWidth;              // Contains width of the multi-monitor grid that makes up the Eyefinity Single Large Surface.
                                    // For example, a 3 display wide by 2 high Eyefinity setup will return 3 for this entry.
    int iSLSGridHeight;             // Contains height of the multi-monitor grid that makes up the Eyefinity Single Large Surface.
                                    // For example, a 3 display wide by 2 high Eyefinity setup will return 2 for this entry.

    int iSLSWidth;                  // Contains width in pixels of the multi-monitor Single Large Surface. The value returned is
                                    // a function of the width of the SLS grid, of the horizontal resolution of each display, and
                                    // of whether or not bezel compensation is enabled.
    int iSLSHeight;                 // Contains height in pixels of the multi-monitor Single Large Surface. The value returned is
                                    // a function of the height of the SLS grid, of the vertical resolution of each display, and
                                    // of whether or not bezel compensation is enabled.

    int iBezelCompensatedDisplay;   // Indicates if bezel compensation is used for the current SLS display area.
                                    // 1 if enabled, and 0 if disabled.
};

struct AGSDisplayInfo
{
    int iGridXCoord;                // Contains horizontal SLS grid coordinate of the display. The value is zero based with
                                    // increasing values from left to right of the overall SLS grid. For example, the left-most
                                    // display of a 3x2 Eyefinity setup will have the value 0, and the right-most will have
                                    // the value 2.
    int iGridYCoord;                // Contains vertical SLS grid coordinate of the display. The value is zero based with
                                    // increasing values from top to bottom of the overall SLS grid. For example, the top
                                    // display of a 3x2 Eyefinity setup will have the value 0, and the bottom will have the
                                    // value 1.

    AGSRect displayRect;            // Contains the base offset and dimensions in pixels of the SLS rendering
                                    // area associated with this display. If bezel compensation is enabled, this
                                    // area will be larger than what the display can natively present to account
                                    // for bezel area. If bezel compensation is disabled, this area will be equal
                                    // to what the display can support natively.

    AGSRect displayRectVisible;     // Contains the base offset and dimensions in pixels of the SLS rendering area
                                    // associated with this display that is visible to the end user. If bezel
                                    // compensation is enabled, this area will be equal to what the display can
                                    // natively, but smaller that the area described in the displayRect entry. If
                                    // bezel compensation is disabled, this area will be equal to what the display
                                    // can support natively and equal to the area described in the displayRect entry.
                                    // Developers wishing to place UI, HUD, or other game assets on a given display
                                    // so that it is visible and accessible to end users need to locate them inside
                                    // of the region defined by this rect.

    int iPreferredDisplay;          // Indicates whether or not this display is the preferred one for rendering of
                                    // game HUD and UI elements. Only one display out of the whole SLS grid will have
                                    // this be true if it is the preferred display and 0 otherwise. Developers wishing
                                    // to place specific UI, HUD, or other game assets on a given display so that it
                                    // is visible and accessible to end users need to locate them inside of the region
                                    // defined by this rect.
};

struct AGSConfiguration
{
    AGSCrossfireMode        crossfireMode;                  // Desired Crossfire mode. See AGSCrossfireMode for more details
};

struct AGSGPUInfo
{
    enum ArchitectureVersion
    {
        ArchitectureVersion_Unknown,
        ArchitectureVersion_PreGCN,
        ArchitectureVersion_GCN
    };

    int                     agsVersionMajor;                // Major field of Major.Minor.Patch AGS version number
    int                     agsVersionMinor;                // Minor field of Major.Minor.Patch AGS version number
    int                     agsVersionPatch;                // Patch field of Major.Minor.Patch AGS version number

    ArchitectureVersion     architectureVersion;            // Set to Unknown if not AMD hardware
    const char*             adapterString;                  // The adapter name string. NULL if not AMD hardware
    int                     deviceId;                       // The device id
    int                     revisionId;                     // The revision id

    const char*             driverVersion;                  // The driver package version
    const char*             radeonSoftwareVersion;          // The Radeon Software Version

    int                     iNumCUs;                        // Number of GCN compute units. Zero if not GCN
    int                     iCoreClock;                     // core clock speed at 100% power in MHz
    int                     iMemoryClock;                   // memory clock speed at 100% power in MHz
    float                   fTFlops;                        // Teraflops of GPU. Zero if not GCN. Calculated from iCoreClock * iNumCUs * 64 Pixels/clk * 2 instructions/MAD
};

// Description
//   Function used to initialize the AGS library.
//   Must be called prior to any of the subsequent AGS API calls.
//   Must be called prior to ID3D11Device or ID3D12Device creation.
//
// Input params
//   context - Address of a pointer to a context. This function allocates a context on the heap which is then required for all subsequent API calls.
//   config  - Optional pointer to a AGSConfiguration struct to override the default library configuration.
//   gpuInfo - Optional pointer to a AGSGPUInfo struct which will get filled in for the primary adapter.
//
AMD_AGS_API AGSReturnCode agsInit( AGSContext** context, const AGSConfiguration* config, AGSGPUInfo* gpuInfo );

// Description
//   Function used to clean up the AGS library.
//
// Input params
//   context - Pointer to a context. This function will deallocate the context from the heap.
//
AMD_AGS_API AGSReturnCode agsDeInit( AGSContext* context );

// Description
//   Function used to query the number of GPUs used for Crossfire acceleration.
//   This may be different from the total number of GPUs present in the system
//   which you can query using agsGetTotalGPUCount which reports all GPUs,
//   even if they are not configured for Crossfire.
//
// Input params
//   context - Pointer to a context.
//
// Output params
//   numGPUs - Number of GPUs used for Crossfire acceleration
//
AMD_AGS_API AGSReturnCode agsGetCrossfireGPUCount( AGSContext* context, int* numGPUs );

// Description
//   Function used to query the number of GPUs in the system.
//   This number may be different from agsGetCrossfireGPUCount as it reports
//   all devices installed in the system, and not only those configured for
//   Crossfire.
//
// Input params
//   context - Pointer to a context.
//
// Output params
//   numGPUs - Number of GPUs in the system.
//
AMD_AGS_API AGSReturnCode agsGetTotalGPUCount( AGSContext* context, int* numGPUs );

// Description
//   Function used to query the memory size of a GPU. The number of GPUs should
//   be obtained using agsGetTotalGPUCount
//
// Input params
//   context - Pointer to a context.
//   gpuIndex - The GPU index to query
//
// Output params
//   sizeInBytes - Memory size on the device in bytes
//
AMD_AGS_API AGSReturnCode agsGetGPUMemorySize( AGSContext* context, int gpuIndex, long long* sizeInBytes );

// Description
//   Function used to query Eyefinity configuration state information relevant to ISVs. State info returned
//   includes: whether Eyefinity is enabled or not, SLS grid configuration, SLS dimensions, whether bezel
//   compensation is enabled or not, SLS grid coordinate for each display, total rendering area for each
//   display, visible rendering area for each display, and a preferred display flag.
//
//   This function needs to be called twice. Firstly to null into eyefinityInfo and displaysInfo. This will
//   return the number of AGSDisplayInfo objects to allocate.
//   Second call requires valid pointers to eyefinityInfo and the newly allocated displaysInfo array. It is the
//   responsibility of the caller to free this memory.
//
//
// Input params
//   context -         Pointer to a context.
//   displayIndex -    Operating system specific display index identifier. The value used should be the
//                     index of the display used for rendering operations. On Windows operating systems,
//                     the value can be queried using the EnumDisplayDevices() API.
//
// Output params
//   eyefinityInfo -   This is a pointer to an AGSEyefinityInfo structure that contains system Eyefinity
//                     configuration information.
//   numDisplaysInfo - Pointer to the number of AGSDisplayInfo structures stored in the returned
//                     displaysInfo array. The value returned is equal to the number of displays
//                     used for the Eyefinity setup.
//   displaysInfo -    Pointer to an array of AGSDisplayInfo structures that contains per display
//                     Eyefinity configuration information.
//
AMD_AGS_API AGSReturnCode agsGetEyefinityConfigInfo( AGSContext* context, int displayIndex, AGSEyefinityInfo* eyefinityInfo, int* numDisplaysInfo, AGSDisplayInfo* displaysInfo );

// Description
//   Function used to initialize the AMD-specific driver extensions for D3D12
//
// Input params
//   context -             Pointer to a context. This is generated by agsInit()
//   device -              The D3D12 device.
//   extensionsSupported - Pointer to a bit mask that this function will fill in to indicate which extensions are supported.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX12_Init( AGSContext* context, ID3D12Device* device, unsigned int* extensionsSupported );

// Description
//   Function used to cleanup any AMD-specific driver extensions for D3D12
//
// Input params
//   context -             Pointer to a context.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX12_DeInit( AGSContext* context );

// Description
//   Function used to initialize the AMD-specific driver extensions for D3D11
//
// Input params
//   context -             Pointer to a context. This is generated by agsInit()
//   device -              The D3D11 device.
//   extensionsSupported - Pointer to a bit mask that this function will fill in to indicate which extensions are supported.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_Init( AGSContext* context, ID3D11Device* device, unsigned int* extensionsSupported );

// Description
//   Function used to cleanup any AMD-specific driver extensions for D3D11
//
// Input params
//   context -             Pointer to a context.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_DeInit( AGSContext* context );

// Description
//   Function used to set the primitive topology. If you are using any of the extended topology types, then this function should
//   be called to set ALL topology types.
//
// Input params
//   context -             Pointer to a context.
//   topology -            The topology to set on the D3D11 device. This can be either an AGS-defined topology such as AGS_PRIMITIVE_TOPOLOGY_QUAD_LIST
//                         or a standard D3D-defined topology such as D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP.
//                         NB. the AGS-defined types will require casting to a D3D_PRIMITIVE_TOPOLOGY type.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_IASetPrimitiveTopology( AGSContext* context, enum D3D_PRIMITIVE_TOPOLOGY topology );

// Description
//   Function used indicate to the driver it can overlap the subsequent batch of back-to-back dispatches
//
// Input params
//   context -             Pointer to a context.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_BeginUAVOverlap( AGSContext* context );

// Description
//   Function used indicate to the driver it can no longer overlap the batch of back-to-back dispatches that has been submitted
//
// Input params
//   context -             Pointer to a context.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_EndUAVOverlap( AGSContext* context );

// Description
//   Function used to set the depth bounds test extension
//
// Input params
//   context -  Pointer to a context.
//   enabled -  Whether to enable or disable the depth bounds testing. If disabled, the next two args are ignored.
//   minDepth - The near depth range to clip against.
//   maxDepth - The far depth range to clip against.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_SetDepthBounds( AGSContext* context, bool enabled, float minDepth, float maxDepth );

// Description
//   Function used to submit a batch of draws via MultiDrawIndirect
//
// Input params
//   context -             Pointer to a context.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_MultiDrawInstancedIndirect( AGSContext* context, unsigned int drawCount, ID3D11Buffer* pBufferForArgs, unsigned int alignedByteOffsetForArgs, unsigned int byteStrideForArgs );

// Description
//   Function used to submit a batch of draws via MultiDrawIndirect
//
// Input params
//   context -             Pointer to a context.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_MultiDrawIndexedInstancedIndirect( AGSContext* context, unsigned int drawCount, ID3D11Buffer* pBufferForArgs, unsigned int alignedByteOffsetForArgs, unsigned int byteStrideForArgs );

// Description
//   Functions to create a Direct3D11 resource with the specified AFR transfer type
//
// Input params
//   context -             Pointer to a context.
//   desc -                Pointer to the D3D11 resource description.
//   initialData -         Optional pointer to the initializing data for the resource.
//   transferType -        The transfer behavior. See AGSAfrTransferType for more details.
//
// Output params
//   buffer/texture -      Returned pointer to the resource.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_CreateBuffer( AGSContext* context, const D3D11_BUFFER_DESC* desc, const D3D11_SUBRESOURCE_DATA* initialData, ID3D11Buffer** buffer, AGSAfrTransferType transferType );
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_CreateTexture1D( AGSContext* context, const D3D11_TEXTURE1D_DESC* desc, const D3D11_SUBRESOURCE_DATA* initialData, ID3D11Texture1D** texture1D, AGSAfrTransferType transferType );
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_CreateTexture2D( AGSContext* context, const D3D11_TEXTURE2D_DESC* desc, const D3D11_SUBRESOURCE_DATA* initialData, ID3D11Texture2D** texture2D, AGSAfrTransferType transferType );
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_CreateTexture3D( AGSContext* context, const D3D11_TEXTURE3D_DESC* desc, const D3D11_SUBRESOURCE_DATA* initialData, ID3D11Texture3D** texture3D, AGSAfrTransferType transferType );

// Description
//   Functions to notify the driver that we have finished writing to the resource this frame.
//      This will initiate a transfer for AGS_AFR_TRANSFER_1STEP_P2P,
//      AGS_AFR_TRANSFER_2STEP_NO_BROADCAST, and AGS_AFR_TRANSFER_2STEP_WITH_BROADCAST.
//
// Input params
//   context -             Pointer to a context.
//   resource -            Pointer to the resource.
//   transferRegions -     An array of transfer regions (can be null to specify the whole area).
//   subresourceArray -    An array of subresource indices (can be null to specify all subresources).
//   numSubresources -     The number of subresources in subresourceArray OR number of transferRegions. Use 0 to specify ALL subresources and one transferRegion (which may be null if specifying the whole area).
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_NotifyResourceEndWrites( AGSContext* context, ID3D11Resource* resource, const D3D11_RECT* transferRegions, const unsigned int* subresourceArray, unsigned int numSubresources );

// Description
//   This will notify the driver that the app will begin read/write access to the resource.
//
// Input params
//   context -             Pointer to a context.
//   resource -            Pointer to the resource.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_NotifyResourceBeginAllAccess( AGSContext* context, ID3D11Resource* resource );

// Description
//   This is used for AGS_AFR_TRANSFER_1STEP_P2P to notify when it is safe to initiate a transfer.
//   This call in frame N-(NumGpus-1) allows a 1 step P2P in frame N to start.
//   This should be called after agsDriverExtensionsDX11_NotifyResourceEndWrites.
//
// Input params
//   context -             Pointer to a context.
//   resource -            Pointer to the resource.
//
AMD_AGS_API AGSReturnCode agsDriverExtensionsDX11_NotifyResourceEndAllAccess( AGSContext* context, ID3D11Resource* resource );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // AMD_AGS_H
