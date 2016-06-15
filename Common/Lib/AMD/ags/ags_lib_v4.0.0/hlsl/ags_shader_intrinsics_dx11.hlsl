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
*************************************************************************************************************
* @file  ags_shader_intrinsics_dx11.hlsl
*
* @brief
*    AMD D3D Shader Intrinsics API hlsl file.
*    This include file contains the shader intrinsics definitions (structures, enums, constant)
*    and HLSL shader intrinsics functions.
*
* @note
*    This does not work with immediate values or values that the compiler determines can produces denorms
*
*************************************************************************************************************
*/

#ifndef _AMDDXEXTSHADERINTRINSICS_HLSL_
#define _AMDDXEXTSHADERINTRINSICS_HLSL_

/**
*************************************************************************************************************
*   Definitions to construct the intrinsic instruction composed of an opcode and optional immediate data.
*************************************************************************************************************
*/
#define AmdDxExtShaderIntrinsics_MagicCodeShift  28
#define AmdDxExtShaderIntrinsics_MagicCodeMask   0xf
#define AmdDxExtShaderIntrinsics_DataShift       8
#define AmdDxExtShaderIntrinsics_DataMask        0xfffff
#define AmdDxExtShaderIntrinsics_OpcodeShift     0
#define AmdDxExtShaderIntrinsics_OpcodeMask      0xff

#define AmdDxExtShaderIntrinsics_MagicCode           0x5


/**
*************************************************************************************************************
*   Intrinsic opcodes.
*************************************************************************************************************
*/
#define AmdDxExtShaderIntrinsicsOpcode_Readfirstlane  0x01
#define AmdDxExtShaderIntrinsicsOpcode_Readlane       0x02
#define AmdDxExtShaderIntrinsicsOpcode_LaneId         0x03
#define AmdDxExtShaderIntrinsicsOpcode_Swizzle        0x04
#define AmdDxExtShaderIntrinsicsOpcode_Ballot         0x05
#define AmdDxExtShaderIntrinsicsOpcode_MBCnt          0x06
#define AmdDxExtShaderIntrinsicsOpcode_Min3U          0x08
#define AmdDxExtShaderIntrinsicsOpcode_Min3F          0x09
#define AmdDxExtShaderIntrinsicsOpcode_Med3U          0x0a
#define AmdDxExtShaderIntrinsicsOpcode_Med3F          0x0b
#define AmdDxExtShaderIntrinsicsOpcode_Max3U          0x0c
#define AmdDxExtShaderIntrinsicsOpcode_Max3F          0x0d
#define AmdDxExtShaderIntrinsicsOpcode_BaryCoord      0x0e
#define AmdDxExtShaderIntrinsicsOpcode_VtxParam       0x0f


/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsicsSwizzle defines for common swizzles.  Can be used as the operation parameter for
*   the AmdDxExtShaderIntrinsics_Swizzle intrinsic.
*************************************************************************************************************
*/
#define AmdDxExtShaderIntrinsicsSwizzle_SwapX1      0x041f
#define AmdDxExtShaderIntrinsicsSwizzle_SwapX2      0x081f
#define AmdDxExtShaderIntrinsicsSwizzle_SwapX4      0x101f
#define AmdDxExtShaderIntrinsicsSwizzle_SwapX8      0x201f
#define AmdDxExtShaderIntrinsicsSwizzle_SwapX16     0x401f
#define AmdDxExtShaderIntrinsicsSwizzle_ReverseX2   0x041f
#define AmdDxExtShaderIntrinsicsSwizzle_ReverseX4   0x0c1f
#define AmdDxExtShaderIntrinsicsSwizzle_ReverseX8   0x1c1f
#define AmdDxExtShaderIntrinsicsSwizzle_ReverseX16  0x3c1f
#define AmdDxExtShaderIntrinsicsSwizzle_ReverseX32  0x7c1f
#define AmdDxExtShaderIntrinsicsSwizzle_BCastX2     0x003e
#define AmdDxExtShaderIntrinsicsSwizzle_BCastX4     0x003c
#define AmdDxExtShaderIntrinsicsSwizzle_BCastX8     0x0038
#define AmdDxExtShaderIntrinsicsSwizzle_BCastX16    0x0030
#define AmdDxExtShaderIntrinsicsSwizzle_BCastX32    0x0020


/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsicsBarycentric defines for barycentric interpolation mode.  To be used with
*   AmdDxExtShaderIntrinsicsOpcode_IjBarycentricCoords to specify the interpolation mode.
*************************************************************************************************************
*/
#define AmdDxExtShaderIntrinsicsBarycentric_LinearCenter    0x1
#define AmdDxExtShaderIntrinsicsBarycentric_LinearCentroid  0x2
#define AmdDxExtShaderIntrinsicsBarycentric_LinearSample    0x3
#define AmdDxExtShaderIntrinsicsBarycentric_PerspCenter     0x4
#define AmdDxExtShaderIntrinsicsBarycentric_PerspCentroid   0x5
#define AmdDxExtShaderIntrinsicsBarycentric_PerspSample     0x6
#define AmdDxExtShaderIntrinsicsBarycentric_PerspPullModel  0x7

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsicsBarycentric defines for specifying vertex and parameter indices.  To be used as
*   the inputs to the AmdDxExtShaderIntrinsicsOpcode_VertexParameter function
*************************************************************************************************************
*/
#define AmdDxExtShaderIntrinsicsBarycentric_Vertex0  0x0
#define AmdDxExtShaderIntrinsicsBarycentric_Vertex1  0x1
#define AmdDxExtShaderIntrinsicsBarycentric_Vertex2  0x2

#define AmdDxExtShaderIntrinsicsBarycentric_Param0   0x00
#define AmdDxExtShaderIntrinsicsBarycentric_Param1   0x01
#define AmdDxExtShaderIntrinsicsBarycentric_Param2   0x02
#define AmdDxExtShaderIntrinsicsBarycentric_Param3   0x03
#define AmdDxExtShaderIntrinsicsBarycentric_Param4   0x04
#define AmdDxExtShaderIntrinsicsBarycentric_Param5   0x05
#define AmdDxExtShaderIntrinsicsBarycentric_Param6   0x06
#define AmdDxExtShaderIntrinsicsBarycentric_Param7   0x07
#define AmdDxExtShaderIntrinsicsBarycentric_Param8   0x08
#define AmdDxExtShaderIntrinsicsBarycentric_Param9   0x09
#define AmdDxExtShaderIntrinsicsBarycentric_Param10  0x0a
#define AmdDxExtShaderIntrinsicsBarycentric_Param11  0x0b
#define AmdDxExtShaderIntrinsicsBarycentric_Param12  0x0c
#define AmdDxExtShaderIntrinsicsBarycentric_Param13  0x0d
#define AmdDxExtShaderIntrinsicsBarycentric_Param14  0x0e
#define AmdDxExtShaderIntrinsicsBarycentric_Param15  0x0f
#define AmdDxExtShaderIntrinsicsBarycentric_Param16  0x10
#define AmdDxExtShaderIntrinsicsBarycentric_Param17  0x11
#define AmdDxExtShaderIntrinsicsBarycentric_Param18  0x12
#define AmdDxExtShaderIntrinsicsBarycentric_Param19  0x13
#define AmdDxExtShaderIntrinsicsBarycentric_Param20  0x14
#define AmdDxExtShaderIntrinsicsBarycentric_Param21  0x15
#define AmdDxExtShaderIntrinsicsBarycentric_Param22  0x16
#define AmdDxExtShaderIntrinsicsBarycentric_Param23  0x17
#define AmdDxExtShaderIntrinsicsBarycentric_Param24  0x18
#define AmdDxExtShaderIntrinsicsBarycentric_Param25  0x19
#define AmdDxExtShaderIntrinsicsBarycentric_Param26  0x1a
#define AmdDxExtShaderIntrinsicsBarycentric_Param27  0x1b
#define AmdDxExtShaderIntrinsicsBarycentric_Param28  0x1c
#define AmdDxExtShaderIntrinsicsBarycentric_Param29  0x1d
#define AmdDxExtShaderIntrinsicsBarycentric_Param30  0x1e
#define AmdDxExtShaderIntrinsicsBarycentric_Param31  0x1f

#define AmdDxExtShaderIntrinsicsBarycentric_ParamShift 0
#define AmdDxExtShaderIntrinsicsBarycentric_ParamMask  0x1f
#define AmdDxExtShaderIntrinsicsBarycentric_VtxShift   0x5
#define AmdDxExtShaderIntrinsicsBarycentric_VtxMask    0x3


/**
*************************************************************************************************************
*   Resource and sampler slots
*************************************************************************************************************
*/
#define AmdDxExtShaderIntrinsicsResSlot       t127
#define AmdDxExtShaderIntrinsicsSamplerSlot   s15

SamplerState AmdDxExtShaderIntrinsicsSamplerState : register (AmdDxExtShaderIntrinsicsSamplerSlot);

Texture3D<float4> AmdDxExtShaderIntrinsicsResource : register (AmdDxExtShaderIntrinsicsResSlot);


/**
*************************************************************************************************************
*   MakeAmdShaderIntrinsicsInstruction
*
*   Creates instruction from supplied opcode and immediate data.
*   NOTE: This is an internal function and should not be called by the source HLSL shader directly.
*
*************************************************************************************************************
*/
uint MakeAmdShaderIntrinsicsInstruction(uint opcode, uint immediateData)
{
    return ((AmdDxExtShaderIntrinsics_MagicCode << AmdDxExtShaderIntrinsics_MagicCodeShift) |
            (immediateData << AmdDxExtShaderIntrinsics_DataShift) |
            (opcode << AmdDxExtShaderIntrinsics_OpcodeShift));
}


/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_Readfirstlane
*
*   Returns the value of src for the first active lane of the wavefront.
*
*************************************************************************************************************
*/
float AmdDxExtShaderIntrinsics_Readfirstlane(float src)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Readfirstlane, 0);

    return AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(src, 0, 0),
                                                        asfloat(instruction)).x;
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_Readlane
*
*   Returns the value of src for the lane within the wavefront specified by laneId.
*
*************************************************************************************************************
*/
float AmdDxExtShaderIntrinsics_Readlane(float src, uint laneId)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Readlane, laneId);

    return AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(src, 0, 0),
                                                        asfloat(instruction)).x;
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_LaneId
*
*   Returns the current lane id for the thread within the wavefront.
*
*************************************************************************************************************
*/
uint AmdDxExtShaderIntrinsics_LaneId()
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_LaneId, 0);

    return AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(0, 0, 0),
                                                        asfloat(instruction)).x;
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_Swizzle
*
*   Generic instruction to shuffle the src value among different lanes as specified by the operation.
*   Note that the operation parameter must be an immediately specified value not a value from a variable.
*
*************************************************************************************************************
*/
float AmdDxExtShaderIntrinsics_Swizzle(float src, uint operation)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Swizzle,
                                                          operation);

    return AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(src, 0, 0),
                                                        asfloat(instruction)).x;
}


/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_Ballot
*
*   Given an input predicate returns a bit mask indicating for which lanes the predicate is true.
*   Inactive or non-existent lanes will always return 0.  The number of existent lanes is the
*   wavefront size.
*
*************************************************************************************************************
*/
uint2 AmdDxExtShaderIntrinsics_Ballot(bool predicate)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Ballot, 0);

    float2 result = AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                                 float3(predicate, 0, 0),
                                                                 asfloat(instruction)).xy;

    return uint2(asuint(result.x), asuint(result.y));
}


/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_BallotAny
*
*   Convenience routine that uses Ballot and returns true if for any of the active lanes the predicate
*   is true.
*
*************************************************************************************************************
*/
bool AmdDxExtShaderIntrinsics_BallotAny(bool predicate)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Ballot, 0);

    float2 result = AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                                 float3(predicate, 0, 0),
                                                                 asfloat(instruction)).xy;

    return ((asuint(result.x) | asuint(result.y)) != 0 ? true : false);
}


/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_BallotAll
*
*   Convenience routine that uses Ballot and returns true if for all of the active lanes the predicate
*   is true.
*
*************************************************************************************************************
*/
bool AmdDxExtShaderIntrinsics_BallotAll(bool predicate)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Ballot, 0);

    float2 result = AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                                 float3(predicate, 0, 0),
                                                                 asfloat(instruction)).xy;

    float2 execMask = AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                                   float3(true, 0, 0),
                                                                   asfloat(instruction)).xy;

    return (((asuint(result.x) & ~asuint(execMask.x)) | (asuint(result.y) & ~asuint(execMask.y))) == 0 ? true : false);
}


/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_MBCnt
*
*   Returns the masked bit count of the source register for this thread within all the active threads
*   within a wavefront.
*
*************************************************************************************************************
*/
uint AmdDxExtShaderIntrinsics_MBCnt(uint2 src)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_MBCnt,
                                                          0);

    float mbcnt = AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(asfloat(src.x), asfloat(src.y), 0),
                                                        asfloat(instruction)).x;
    return asuint(mbcnt);
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_Min3F
*
*   Returns the minimum value of the three floating point source arguments.
*
*************************************************************************************************************
*/
float AmdDxExtShaderIntrinsics_Min3F(float src0, float src1, float src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Min3F,
                                                          0);

    return AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(src0, src1, src2),
                                                        asfloat(instruction)).x;
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_Min3U
*
*   Returns the minimum value of the three unsigned integer source arguments.
*
*************************************************************************************************************
*/
uint AmdDxExtShaderIntrinsics_Min3U(uint src0, uint src1, uint src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Min3U,
                                                          0);

    float minimum = AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(asfloat(src0), asfloat(src1), asfloat(src2)),
                                                        asfloat(instruction)).x;
    return asuint(minimum);
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_Med3F
*
*   Returns the median value of the three floating point source arguments.
*
*************************************************************************************************************
*/
float AmdDxExtShaderIntrinsics_Med3F(float src0, float src1, float src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Med3F,
                                                          0);

    return AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(src0, src1, src2),
                                                        asfloat(instruction)).x;
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_Med3U
*
*   Returns the median value of the three unsigned integer source arguments.
*
*************************************************************************************************************
*/
uint AmdDxExtShaderIntrinsics_Med3U(uint src0, uint src1, uint src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Med3U,
                                                          0);

    float median = AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(asfloat(src0), asfloat(src1), asfloat(src2)),
                                                        asfloat(instruction)).x;
    return asuint(median);
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_Max3F
*
*   Returns the maximum value of the three floating point source arguments.
*
*************************************************************************************************************
*/
float AmdDxExtShaderIntrinsics_Max3F(float src0, float src1, float src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Max3F,
                                                          0);

    return AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(src0, src1, src2),
                                                        asfloat(instruction)).x;
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_Max3U
*
*   Returns the maximum value of the three unsigned integer source arguments.
*
*************************************************************************************************************
*/
uint AmdDxExtShaderIntrinsics_Max3U(uint src0, uint src1, uint src2)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_Max3U,
                                                          0);

    float maximum = AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(asfloat(src0), asfloat(src1), asfloat(src2)),
                                                        asfloat(instruction)).x;
    return asuint(maximum);
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_IjBarycentricCoords
*
*   Returns the (i, j) barycentric coordinate pair for this shader invocation with the specified
*   interpolation mode at the specified pixel location.  Should not be used for "pull-model" interpolation,
*   PullModelBarycentricCoords should be used instead
*
*   Can only be used in pixel shader stages.
*
*************************************************************************************************************
*/
float2 AmdDxExtShaderIntrinsics_IjBarycentricCoords(uint interpMode)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_BaryCoord,
                                                          interpMode);

    return AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(0, 0, 0),
                                                        asfloat(instruction)).xy;
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_PullModelBarycentricCoords
*
*   Returns the (1/W,1/I,1/J) coordinates at the pixel center which can be used for custom interpolation at
*   any location in the pixel.
*
*   Can only be used in pixel shader stages.
*
*************************************************************************************************************
*/
float3 AmdDxExtShaderIntrinsics_PullModelBarycentricCoords()
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_BaryCoord,
                                                          AmdDxExtShaderIntrinsicsBarycentric_PerspPullModel);

    return AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(0, 0, 0),
                                                        asfloat(instruction)).xyz;
}

/**
*************************************************************************************************************
*   AmdDxExtShaderIntrinsics_VertexParameter
*
*   Returns the triangle's parameter information at the specified triangle vertex.
*   The vertex and parameter indices must specified as immediate values.
*
*   Only available in pixel shader stages.
*
*************************************************************************************************************
*/
float4 AmdDxExtShaderIntrinsics_VertexParameter(uint vertexIdx, uint parameterIdx)
{
    uint instruction = MakeAmdShaderIntrinsicsInstruction(AmdDxExtShaderIntrinsicsOpcode_VtxParam,
                           ((vertexIdx << AmdDxExtShaderIntrinsicsBarycentric_VtxShift) |
                            (parameterIdx << AmdDxExtShaderIntrinsicsBarycentric_ParamShift)));


    return AmdDxExtShaderIntrinsicsResource.SampleLevel(AmdDxExtShaderIntrinsicsSamplerState,
                                                        float3(0, 0, 0),
                                                        asfloat(instruction));
}

#endif // AMD_HLSL_EXTENSION
