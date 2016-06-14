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

/**
***********************************************************************************************************************
* @file  ags_shader_intrinsics_dx12.hlsl
* @brief
*    AMD D3D Shader Intrinsics HLSL include file.
*    This include file contains the Shader Intrinsics definitions used in shader code by the application.
* @note
*    This does not work with immediate values or values that the compiler determines can produces denorms
*
***********************************************************************************************************************
*/

#ifndef _AMDEXTD3DSHADERINTRINICS_HLSL
#define _AMDEXTD3DSHADERINTRINICS_HLSL
// AMD shader intrinsics designated SpaceId.  Denotes Texture3D resource and static sampler used in conjuction with
// instrinsic instructions.
#define AmdExtD3DShaderIntrinsicsSpaceId space2147420894
// Texture3D and SamplerState used to access AMD shader instrinsics instruction set.
// Applications need to add descriptor table entries for these when creating root descriptor table.
///@note Requires SM 5.1
Texture3D<float4> AmdExtD3DShaderIntrinsicsResource : register(t0, AmdExtD3DShaderIntrinsicsSpaceId);
SamplerState	  AmdExtD3DShaderIntrinsicsSampler  : register(s0, AmdExtD3DShaderIntrinsicsSpaceId);

/**
***********************************************************************************************************************
*   Definitions to construct the intrinsic instruction composed of an opcode and optional immediate data.
***********************************************************************************************************************
*/
#define AmdExtD3DShaderIntrinsics_MagicCodeShift  28
#define AmdExtD3DShaderIntrinsics_MagicCodeMask   0xf
#define AmdExtD3DShaderIntrinsics_DataShift       8
#define AmdExtD3DShaderIntrinsics_DataMask        0xfffff
#define AmdExtD3DShaderIntrinsics_OpcodeShift     0
#define AmdExtD3DShaderIntrinsics_OpcodeMask      0xff

#define AmdExtD3DShaderIntrinsics_MagicCode       0x5


/**
***********************************************************************************************************************
*   Intrinsic opcodes.
***********************************************************************************************************************
*/
#define AmdExtD3DShaderIntrinsicsOpcode_Readfirstlane  0x01
#define AmdExtD3DShaderIntrinsicsOpcode_Readlane       0x02
#define AmdExtD3DShaderIntrinsicsOpcode_LaneId         0x03
#define AmdExtD3DShaderIntrinsicsOpcode_Swizzle        0x04
#define AmdExtD3DShaderIntrinsicsOpcode_Ballot         0x05
#define AmdExtD3DShaderIntrinsicsOpcode_MBCnt          0x06
#define AmdExtD3DShaderIntrinsicsOpcode_Min3U          0x07
#define AmdExtD3DShaderIntrinsicsOpcode_Min3F          0x08
#define AmdExtD3DShaderIntrinsicsOpcode_Med3U          0x09
#define AmdExtD3DShaderIntrinsicsOpcode_Med3F          0x0a
#define AmdExtD3DShaderIntrinsicsOpcode_Max3U          0x0b
#define AmdExtD3DShaderIntrinsicsOpcode_Max3F          0x0c
#define AmdExtD3DShaderIntrinsicsOpcode_BaryCoord      0x0d
#define AmdExtD3DShaderIntrinsicsOpcode_VtxParam       0x0e


/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsicsSwizzle defines for common swizzles.  Can be used as the operation parameter for
*   the AmdExtD3DShaderIntrinsics_Swizzle intrinsic.
***********************************************************************************************************************
*/
#define AmdExtD3DShaderIntrinsicsSwizzle_SwapX1      0x041f
#define AmdExtD3DShaderIntrinsicsSwizzle_SwapX2      0x081f
#define AmdExtD3DShaderIntrinsicsSwizzle_SwapX4      0x101f
#define AmdExtD3DShaderIntrinsicsSwizzle_SwapX8      0x201f
#define AmdExtD3DShaderIntrinsicsSwizzle_SwapX16     0x401f
#define AmdExtD3DShaderIntrinsicsSwizzle_ReverseX2   0x041f
#define AmdExtD3DShaderIntrinsicsSwizzle_ReverseX4   0x0c1f
#define AmdExtD3DShaderIntrinsicsSwizzle_ReverseX8   0x1c1f
#define AmdExtD3DShaderIntrinsicsSwizzle_ReverseX16  0x3c1f
#define AmdExtD3DShaderIntrinsicsSwizzle_ReverseX32  0x7c1f
#define AmdExtD3DShaderIntrinsicsSwizzle_BCastX2     0x003e
#define AmdExtD3DShaderIntrinsicsSwizzle_BCastX4     0x003c
#define AmdExtD3DShaderIntrinsicsSwizzle_BCastX8     0x0038
#define AmdExtD3DShaderIntrinsicsSwizzle_BCastX16    0x0030
#define AmdExtD3DShaderIntrinsicsSwizzle_BCastX32    0x0020


/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsicsBarycentric defines for barycentric interpolation mode.  To be used with
*   AmdExtD3DShaderIntrinsicsOpcode_IjBarycentricCoords to specify the interpolation mode.
***********************************************************************************************************************
*/
#define AmdExtD3DShaderIntrinsicsBarycentric_LinearCenter    0x1
#define AmdExtD3DShaderIntrinsicsBarycentric_LinearCentroid  0x2
#define AmdExtD3DShaderIntrinsicsBarycentric_LinearSample    0x3
#define AmdExtD3DShaderIntrinsicsBarycentric_PerspCenter     0x4
#define AmdExtD3DShaderIntrinsicsBarycentric_PerspCentroid   0x5
#define AmdExtD3DShaderIntrinsicsBarycentric_PerspSample     0x6
#define AmdExtD3DShaderIntrinsicsBarycentric_PerspPullModel  0x7

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsicsBarycentric defines for specifying vertex and parameter indices.  To be used as
*   the inputs to the AmdExtD3DShaderIntrinsicsOpcode_VertexParameter function
***********************************************************************************************************************
*/
#define AmdExtD3DShaderIntrinsicsBarycentric_Vertex0  0x0
#define AmdExtD3DShaderIntrinsicsBarycentric_Vertex1  0x1
#define AmdExtD3DShaderIntrinsicsBarycentric_Vertex2  0x2

#define AmdExtD3DShaderIntrinsicsBarycentric_Param0   0x00
#define AmdExtD3DShaderIntrinsicsBarycentric_Param1   0x01
#define AmdExtD3DShaderIntrinsicsBarycentric_Param2   0x02
#define AmdExtD3DShaderIntrinsicsBarycentric_Param3   0x03
#define AmdExtD3DShaderIntrinsicsBarycentric_Param4   0x04
#define AmdExtD3DShaderIntrinsicsBarycentric_Param5   0x05
#define AmdExtD3DShaderIntrinsicsBarycentric_Param6   0x06
#define AmdExtD3DShaderIntrinsicsBarycentric_Param7   0x07
#define AmdExtD3DShaderIntrinsicsBarycentric_Param8   0x08
#define AmdExtD3DShaderIntrinsicsBarycentric_Param9   0x09
#define AmdExtD3DShaderIntrinsicsBarycentric_Param10  0x0a
#define AmdExtD3DShaderIntrinsicsBarycentric_Param11  0x0b
#define AmdExtD3DShaderIntrinsicsBarycentric_Param12  0x0c
#define AmdExtD3DShaderIntrinsicsBarycentric_Param13  0x0d
#define AmdExtD3DShaderIntrinsicsBarycentric_Param14  0x0e
#define AmdExtD3DShaderIntrinsicsBarycentric_Param15  0x0f
#define AmdExtD3DShaderIntrinsicsBarycentric_Param16  0x10
#define AmdExtD3DShaderIntrinsicsBarycentric_Param17  0x11
#define AmdExtD3DShaderIntrinsicsBarycentric_Param18  0x12
#define AmdExtD3DShaderIntrinsicsBarycentric_Param19  0x13
#define AmdExtD3DShaderIntrinsicsBarycentric_Param20  0x14
#define AmdExtD3DShaderIntrinsicsBarycentric_Param21  0x15
#define AmdExtD3DShaderIntrinsicsBarycentric_Param22  0x16
#define AmdExtD3DShaderIntrinsicsBarycentric_Param23  0x17
#define AmdExtD3DShaderIntrinsicsBarycentric_Param24  0x18
#define AmdExtD3DShaderIntrinsicsBarycentric_Param25  0x19
#define AmdExtD3DShaderIntrinsicsBarycentric_Param26  0x1a
#define AmdExtD3DShaderIntrinsicsBarycentric_Param27  0x1b
#define AmdExtD3DShaderIntrinsicsBarycentric_Param28  0x1c
#define AmdExtD3DShaderIntrinsicsBarycentric_Param29  0x1d
#define AmdExtD3DShaderIntrinsicsBarycentric_Param30  0x1e
#define AmdExtD3DShaderIntrinsicsBarycentric_Param31  0x1f

#define AmdExtD3DShaderIntrinsicsBarycentric_ParamShift 0
#define AmdExtD3DShaderIntrinsicsBarycentric_ParamMask  0x1f
#define AmdExtD3DShaderIntrinsicsBarycentric_VtxShift   0x5
#define AmdExtD3DShaderIntrinsicsBarycentric_VtxMask    0x3


/**
***********************************************************************************************************************
*   MakeAmdShaderIntrinsicsInstruction
*
*   Creates instruction from supplied opcode and immediate data.
*   NOTE: This is an internal function and should not be called by the source HLSL shader directly.
*
***********************************************************************************************************************
*/
uint MakeAmdShaderIntrinsicsInstruction(uint opcode, uint immediateData)
{
    return ((AmdExtD3DShaderIntrinsics_MagicCode << AmdExtD3DShaderIntrinsics_MagicCodeShift) |
            (immediateData << AmdExtD3DShaderIntrinsics_DataShift) |
            (opcode << AmdExtD3DShaderIntrinsics_OpcodeShift));
}


/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_Readfirstlane
*
*   Returns the value of src for the first active lane of the wavefront.
*
***********************************************************************************************************************
*/
float AmdExtD3DShaderIntrinsics_Readfirstlane(float src)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Readfirstlane, 0);

    return AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                         float3(src, 0, 0),
                                                         asfloat(instruction)).x;
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_Readlane
*
*   Returns the value of src for the lane within the wavefront specified by laneId.
*
***********************************************************************************************************************
*/
float AmdExtD3DShaderIntrinsics_Readlane(float src, uint laneId)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Readlane, laneId);

    return AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                        float3(src, 0, 0),
                                                        asfloat(instruction)).x;
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_LaneId
*
*   Returns the current lane id for the thread within the wavefront.
*
***********************************************************************************************************************
*/
uint AmdExtD3DShaderIntrinsics_LaneId()
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_LaneId, 0);

    return AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                         float3(0, 0, 0),
                                                         asfloat(instruction)).x;
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_Swizzle
*
*   Generic instruction to shuffle the src value among different lanes as specified by the operation.
*   Note that the operation parameter must be an immediately specified value not a value from a variable.
*
***********************************************************************************************************************
*/
float AmdExtD3DShaderIntrinsics_Swizzle(float src, uint operation)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Swizzle,
        operation);

    return AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                         float3(src, 0, 0),
                                                         asfloat(instruction)).x;
}


/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_Ballot
*
*   Given an input predicate returns a bit mask indicating for which lanes the predicate is true.
*   Inactive or non-existent lanes will always return 0.  The number of existent lanes is the
*   wavefront size.
*
***********************************************************************************************************************
*/
uint2 AmdExtD3DShaderIntrinsics_Ballot(bool predicate)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Ballot, 0);

    float2 result = AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                                  float3(predicate, 0, 0),
                                                                  asfloat(instruction)).xy;

    return uint2(asuint(result.x), asuint(result.y));
}


/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_BallotAny
*
*   Convenience routine that uses Ballot and returns true if for any of the active lanes the predicate
*   is true.
*
************************************************************************************************************************
*/
bool AmdExtD3DShaderIntrinsics_BallotAny(bool predicate)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Ballot, 0);

    float2 result = AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                                  float3(predicate, 0, 0),
                                                                  asfloat(instruction)).xy;

    return ((asuint(result.x) | asuint(result.y)) != 0 ? true : false);
}


/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_BallotAll
*
*   Convenience routine that uses Ballot and returns true if for all of the active lanes the predicate
*   is true.
*
***********************************************************************************************************************
*/
bool AmdExtD3DShaderIntrinsics_BallotAll(bool predicate)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Ballot, 0);

    float2 result = AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                                  float3(predicate, 0, 0),
                                                                  asfloat(instruction)).xy;

    float2 execMask = AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                                    float3(true, 0, 0),
                                                                    asfloat(instruction)).xy;

    return (((asuint(result.x) & ~asuint(execMask.x)) | (asuint(result.y) & ~asuint(execMask.y))) == 0 ? true : false);
}


/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_MBCnt
*
*   Returns the masked bit count of the source register for this thread within all the active threads
*   within a wavefront.
*
***********************************************************************************************************************
*/
uint AmdExtD3DShaderIntrinsics_MBCnt(uint2 src)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_MBCnt, 0);

    float mbcnt = AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                                float3(asfloat(src.x), asfloat(src.y), 0),
                                                                asfloat(instruction)).x;
    return asuint(mbcnt);
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_Min3F
*
*   Returns the minimum value of the three floating point source arguments.
*
***********************************************************************************************************************
*/
float AmdExtD3DShaderIntrinsics_Min3F(float src0, float src1, float src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Min3F, 0);

    return AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                         float3(src0, src1, src2),
                                                         asfloat(instruction)).x;
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_Min3U
*
*   Returns the minimum value of the three unsigned integer source arguments.
*
***********************************************************************************************************************
*/
uint AmdExtD3DShaderIntrinsics_Min3U(uint src0, uint src1, uint src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Min3U, 0);

    float minimum = AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                                  float3(asfloat(src0), asfloat(src1), asfloat(src2)),
                                                                  asfloat(instruction)).x;
    return asuint(minimum);
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_Med3F
*
*   Returns the median value of the three floating point source arguments.
*
***********************************************************************************************************************
*/
float AmdExtD3DShaderIntrinsics_Med3F(float src0, float src1, float src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Med3F, 0);

    return AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                         float3(src0, src1, src2),
                                                         asfloat(instruction)).x;
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_Med3U
*
*   Returns the median value of the three unsigned integer source arguments.
*
***********************************************************************************************************************
*/
uint AmdExtD3DShaderIntrinsics_Med3U(uint src0, uint src1, uint src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Med3U, 0);

    float median = AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                                 float3(asfloat(src0), asfloat(src1), asfloat(src2)),
                                                                 asfloat(instruction)).x;
    return asuint(median);
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_Max3F
*
*   Returns the maximum value of the three floating point source arguments.
*
***********************************************************************************************************************
*/
float AmdExtD3DShaderIntrinsics_Max3F(float src0, float src1, float src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Max3F, 0);

    return AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                         float3(src0, src1, src2),
                                                         asfloat(instruction)).x;
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_Max3U
*
*   Returns the maximum value of the three unsigned integer source arguments.
*
***********************************************************************************************************************
*/
uint AmdExtD3DShaderIntrinsics_Max3U(uint src0, uint src1, uint src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_Max3U, 0);

    float maximum = AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                                  float3(asfloat(src0), asfloat(src1), asfloat(src2)),
                                                                  asfloat(instruction)).x;
    return asuint(maximum);
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_IjBarycentricCoords
*
*   Returns the (i, j) barycentric coordinate pair for this shader invocation with the specified
*   interpolation mode at the specified pixel location.  Should not be used for "pull-model" interpolation,
*   PullModelBarycentricCoords should be used instead
*
*   Can only be used in pixel shader stages.
*
***********************************************************************************************************************
*/
float2 AmdExtD3DShaderIntrinsics_IjBarycentricCoords(uint interpMode)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_BaryCoord,
                                                          interpMode);

    return AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                         float3(0, 0, 0),
                                                         asfloat(instruction)).xy;
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_PullModelBarycentricCoords
*
*   Returns the (1/W,1/I,1/J) coordinates at the pixel center which can be used for custom interpolation at
*   any location in the pixel.
*
*   Can only be used in pixel shader stages.
*
***********************************************************************************************************************
*/
float3 AmdExtD3DShaderIntrinsics_PullModelBarycentricCoords()
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_BaryCoord,
                                                          AmdExtD3DShaderIntrinsicsBarycentric_PerspPullModel);

    return AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                         float3(0, 0, 0),
                                                         asfloat(instruction)).xyz;
}

/**
***********************************************************************************************************************
*   AmdExtD3DShaderIntrinsics_VertexParameter
*
*   Returns the triangle's parameter information at the specified triangle vertex.
*   The vertex and parameter indices must specified as immediate values.
*
*   Only available in pixel shader stages.
*
***********************************************************************************************************************
*/
float4 AmdExtD3DShaderIntrinsics_VertexParameter(uint vertexIdx, uint parameterIdx)
{
    uint instruction =
        MakeAmdShaderIntrinsicsInstruction(AmdExtD3DShaderIntrinsicsOpcode_VtxParam,
                                         ((vertexIdx << AmdExtD3DShaderIntrinsicsBarycentric_VtxShift) |
                                          (parameterIdx << AmdExtD3DShaderIntrinsicsBarycentric_ParamShift)));


    return AmdExtD3DShaderIntrinsicsResource.SampleLevel(AmdExtD3DShaderIntrinsicsSampler,
                                                         float3(0, 0, 0),
                                                         asfloat(instruction));
}

#endif // AMDEXTD3DSHADERINTRINICS_HLSL

