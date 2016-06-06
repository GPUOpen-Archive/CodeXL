//=====================================================================
// Copyright 2008-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file D3D10ShaderObject.h
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/DX10/D3D10ShaderObject.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================


#pragma once

#ifndef D3D10SHADEROBJECT_H
#define D3D10SHADEROBJECT_H

#include <D3D10_1.h>
#include <D3D10_1shader.h>
#include <string>
#include <vector>
#include <map>
#include "IBlob.h"
#include "../DX11/d3d11tokenizedprogramformat.hpp"


/// Namespace for the D3D10ShaderObject definitions.
/// These structures were reverse engineered from the D3D10 shader object

namespace D3D10ShaderObject
{
/// The top-level header for a D3D10 shader.
typedef struct
{
    DWORD dwMagicNumber;    ///< A FourCC value. Always set to "DXBC".
    DWORD dwCheckSum[4];    ///< A MD5-based (but with differences) checksum. See DXBCChecksum.h for further details.
    DWORD dwReserved;       ///< Unknown. Seems always to be 00000001.
    DWORD dwSize;           ///< The size of the shader object.
    DWORD dwNumChunks;      ///< The number of chunks in the shader object.
} D3D10_ShaderObjectHeader;

/// The per-chunk header for chunks within a D3D10 shader object.
typedef struct
{
    DWORD dwChunkType;      ///< A FourCC value identifying the chunk type.
    DWORD dwChunkDataSize;  ///< The size of the chunk-data i.e. the size not including the size of the chunk header.
} D3D10_ChunkHeader;

/// Simple class encapsulating handling of a D3D10 shader chunk.
class CShaderChunk
{
public:
    /// Default constructor
    CShaderChunk();

    /// Constructor
    /// \param[in] pHeader  The address of the initial chunk header.
    CShaderChunk(const D3D10_ChunkHeader* pHeader);

    /// Destructor
    virtual ~CShaderChunk();

    /// Copy constructor
    CShaderChunk(const CShaderChunk& obj);

    /// Assignment operator=
    CShaderChunk& operator=(const CShaderChunk& obj);

    /// Get a pointer to the chunk header.
    /// \return A pointer to the chunk header.
    D3D10_ChunkHeader* GetChunkHeader() const { return m_pChunkHeader; }

protected:

    /// Initialise the shader chunk with the specified chunk header pointer.
    /// \param[in] pHeader  The address of the initial chunk header.
    void Initialise(const D3D10_ChunkHeader* pHeader);

    /// A pointer to the chunk header.
    D3D10_ChunkHeader* m_pChunkHeader;
};

/// The header for "RDEF" Resource DEFinition chunks.
typedef struct
{
    DWORD dwConstantBuffers;      ///< The number of constant-buffers.
    DWORD dwConstBufferOffset;    ///< The offset from the start of the RDEF chunk data to the first D3D10_ConstantBuffer structure.
    DWORD dwBoundResources;       ///< The number of bound resources.
    DWORD dwBoundResourceOffset;  ///< The offset from the start of the RDEF chunk data to the first D3D10_ResourceBinding structure.
    DWORD dwVersion;              ///< The version of the shader where the hiword = the shader type & the loword = the shader model.
    DWORD dwCompilerFlags;        ///< The flags passed to the D3D10 HLSL compiler.
    DWORD dwCreatorOffset;        ///< The offset from the start of the RDEF chunk data to the Creator string.
} D3D10_RDEF_Header;

/// The header for "RDEF" Resource DEFinition chunks.
typedef struct
{
    DWORD dwConstantBuffers;      ///< The number of constant-buffers.
    DWORD dwConstBufferOffset;    ///< The offset from the start of the RDEF chunk data to the first D3D10_ConstantBuffer structure.
    DWORD dwBoundResources;       ///< The number of bound resources.
    DWORD dwBoundResourceOffset;  ///< The offset from the start of the RDEF chunk data to the first D3D10_ResourceBinding structure.
    DWORD dwVersion;              ///< The version of the shader where the hiword = the shader type & the loword = the shader model.
    DWORD dwCompilerFlags;        ///< The flags passed to the D3D10 HLSL compiler.
    DWORD dwCreatorOffset;        ///< The offset from the start of the RDEF chunk data to the Creator string.
    DWORD dwMagic;                ///< Magic Number - 'R', 'D', '1', '1'.
    DWORD dwReserved[7];          ///< Unknown.
} D3D11_RDEF_Header;

/// A per bound-resource structure in the RDEF chunk.
typedef struct
{
    DWORD dwNameOffset;                    ///< The offset from the start of the RDEF chunk data to the resource name string.
    D3D10_SHADER_INPUT_TYPE Type;          ///< The type of data in the resource.
    D3D10_RESOURCE_RETURN_TYPE ReturnType; ///< For textures the return type otherwise 0.
    D3D10_SRV_DIMENSION Dimension;         ///< The number of dimensions of the data in the resource.
    DWORD dwNumSamples;                    ///< For multisampled textures the multisampled otherwise 0
    DWORD dwBindPoint;                     ///< The starting bind-point.
    DWORD dwBindCount;                     ///< The number of bind-points for arrays(contiguous).
    DWORD dwFlags;                         ///< Flags. See D3D10_SHADER_INPUT_FLAGS.
} D3D10_ResourceBinding;

/// A per constant-buffer structure in the RDEF chunk.
typedef struct
{
    DWORD dwNameOffset;                    ///< The offset from the start of the RDEF chunk data to the constant-buffer name string.
    DWORD dwVariables;                     ///< The number of constants in the constant-buffer.
    DWORD dwOffset;                        ///< The offset from the start of the RDEF chunk data to the first const in this buffer.
    DWORD dwSize;                          ///< The size of the constant-buffer.
    D3D10_CBUFFER_TYPE Type;               ///< The type of the constant-buffer.
    DWORD dwFlags;                         ///< Flags. See D3D10_SHADER_CBUFFER_FLAGS.
} D3D10_ConstantBuffer;

/// A per constant structure in the RDEF chunk.
typedef struct
{
    DWORD dwNameOffset;                    ///< The offset from the start of the RDEF chunk data to the constant name string.
    DWORD dwStartOffset;                   ///< The offset of the constant within the constant-buffer.
    DWORD dwSize;                          ///< The size of the constant.
    DWORD dwFlags;                         ///< Shader-variable flags, see D3D10_SHADER_VARIABLE_FLAGS.
    DWORD dwTypeOffset;                    ///< The offset from the start of the RDEF chunk data to the D3D10_Type for this constant.
    DWORD dwDefaultValue;                  ///< The default value of the constant.
} D3D10_Constant;

/// A per constant structure in the RDEF chunk.
typedef struct
{
    DWORD dwNameOffset;                    ///< The offset from the start of the RDEF chunk data to the constant name string.
    DWORD dwStartOffset;                   ///< The offset of the constant within the constant-buffer.
    DWORD dwSize;                          ///< The size of the constant.
    DWORD dwFlags;                         ///< Shader-variable flags, see D3D10_SHADER_VARIABLE_FLAGS.
    DWORD dwTypeOffset;                    ///< The offset from the start of the RDEF chunk data to the D3D10_Type for this constant.
    DWORD dwDefaultValue;                  ///< The default value of the constant.
    DWORD dwReserved[4];                   ///< Unknown.
} D3D11_Constant;

/// A structure defining the type of variables. Used in the RDEF chunk to define the type of constants.
typedef struct D3D10_Type
{
    WORD wClass;                           ///< The variable class of the type. See D3D10_SHADER_VARIABLE_CLASS.
    WORD wType;                            ///< The variable type. See D3D10_SHADER_VARIABLE_TYPE.
    WORD wRows;                            ///< Number of rows in a matrix. 1 for other numeric types, 0 for other types.
    WORD wColumns;                         ///< Number of columns in a matrix. 1 for other numeric types, 0 for other types.
    WORD wElements;                        ///< Number of elements in an array; otherwise 0.
    WORD wMembers;                         ///< Number of members in the structure; otherwise 0.
    WORD wOffset;                          ///< Offset to something. Seems always to be 0.
    WORD wReserved;                        ///< Unknown. Seems always to be 0.
    DWORD dwReserved[4];                   ///< Unknown.
    DWORD dwNameOffset;                    ///< The offset from the start of the RDEF chunk data to the constant name string.
} D3D10_Type;

/// The header for "ISGN" Input SiGNature & "OSGN" Output SiGNature chunks.
typedef struct
{
    DWORD dwSignatures;                    ///< The number of signatures.
    DWORD dwReserved;                      ///< Unknown. Seems always to be 8.
} D3D10_xSGN_Header;

/// A per signature structure in the ISGN & OSGN chunks.
typedef struct
{
    DWORD dwSemanticNameOffset;            ///< The offset from the start of the chunk data to the semantic name string.
    DWORD dwSemanticIndex;                 ///< The semantic index. Used to differentiate different signatures that use the same semantic.
    DWORD dwSystemValueType;               ///< The type of the system value. See D3D10_NAME.
    DWORD dwComponentType;                 ///< The per-component type of the data. See D3D10_REGISTER_COMPONENT_TYPE.
    DWORD dwRegister;                      ///< The register used for the data.
    WORD  wMask;                           ///< A mask indicating which components of a register are used.
    WORD  wPadding;                        ///< Padding the structure to DWORD divisible width.
} D3D10_Signature;

/// The header for "SHDR" SHaDeR chunks.
typedef struct
{
    DWORD dwVersion;                       ///< The shader model of the shader.
    DWORD dwTokenCount;                    ///< The number of DWORD tokens in the shader.
} D3D10_SHDR_Header;

/// A structure defining the data held in the STAT chunk.
typedef struct
{
    UINT  InstructionCount;                ///< The number of emitted instructions.
    UINT  TempRegisterCount;               ///< The number of temporary registers used.
    UINT  DefCount;                        ///< The number of constant definitions.
    UINT  DclCount;                        ///< The number of declarations (input + output).
    UINT  FloatInstructionCount;           ///< The number of floating point arithmetic instructions used.
    UINT  IntInstructionCount;             ///< The number of signed integer arithmetic instructions used.
    UINT  UintInstructionCount;            ///< The number of unsigned integer arithmetic instructions used.
    UINT  StaticFlowControlCount;          ///< The number of static flow control instructions used.
    UINT  DynamicFlowControlCount;         ///< The number of dynamic flow control instructions used.
    UINT  MacroInstructionCount;           ///< The number of macro instructions used.
    UINT  TempArrayCount;                  ///< The number of temporary arrays used.
    UINT  ArrayInstructionCount;           ///< The number of array instructions used.
    UINT  CutInstructionCount;             ///< The number of cut instructions used.
    UINT  EmitInstructionCount;            ///< The number of emit instructions used.
    UINT  TextureNormalInstructions;       ///< The number of non-categorized texture instructions.
    UINT  TextureLoadInstructions;         ///< The number of texture load instructions.
    UINT  TextureCompInstructions;         ///< The number of texture comparison instructions.
    UINT  TextureBiasInstructions;         ///< The number of texture bias instructions.
    UINT  TextureGradientInstructions;     ///< The number of texture gradient instructions.
    UINT  Reserved[10];                    ///< Unknown stats.
} D3D10_SHADER_STATS;

/// A structure describing an indexable temp register.
typedef struct
{
    DWORD dwIndex;                         ///< The index of the register ie. x0, x1, etc.
    DWORD dwSize;                          ///< The number of indexable elements within the array.
    DWORD dwComponentCount;                ///< The number of components in each array element.
} D3D10_IndexableTempRegister;

/// A structure describing an global memory register.
typedef struct
{
    DWORD dwRegister;                      ///< The operand coding of the register.
    DWORD dwIndex;                         ///< The index of the register ie. g0, g1, etc.
    DWORD dwStride;                        ///< The struct byte stride, 0 for raw memories.
    DWORD dwCount;                         ///< The count of elements in the memory.
} D3D10_GlobalMemoryRegister;

/// CD3D10ShaderObject is a utility class for parsing & creating D3D10 shader objects.

class CD3D10ShaderObject
{
public:
    /// CD3D10ShaderObject default constructor.
    CD3D10ShaderObject();

    /// CD3D10ShaderObject constructor.
    /// \param[in] pShaderData A pointer to the shader data.
    /// \param[in] nShaderDataSize The size of the shader in bytes.
    CD3D10ShaderObject(const void* pShaderData, size_t nShaderDataSize);

    /// CD3D10ShaderObject constructor.
    /// \param[in] pShaderBlob A pointer an IBlob object holding the shader data.
    CD3D10ShaderObject(const IBlob* pShaderBlob);

    /// CD3D10ShaderObject destructor
    ~CD3D10ShaderObject();

    /// CD3D10ShaderObject copy-constructor.
    /// \param[in] shaderObject The shader object to copy.
    CD3D10ShaderObject(const CD3D10ShaderObject& shaderObject);

    /// CD3D10ShaderObject assignment operator.
    /// \param[in] shaderObject The shader object to copy.
    /// \param[out] shaderObject The updated shader object for multiple assignments.
    CD3D10ShaderObject& operator=(const CD3D10ShaderObject& shaderObject);

    /// Is the current shader object valid?
    /// \return True if the shader object is valid, otherwise false.
    bool IsValid() const;

    /// Get the unique check sum identifying the shader.
    /// \return The check sum.
    std::string GetCheckSum() const { return m_strCheckSum; }

    /// Set the shader object.
    /// \param[in] pShaderData A pointer to the shader data.
    /// \param[in] nShaderDataSize The size of the shader in bytes.
    bool SetShaderObject(const void* pShaderData, size_t nShaderDataSize);

    /// Set the shader object.
    /// \param[in] pShaderBlob A pointer to an IBlob object holding the shader data.
    bool SetShaderObject(const IBlob* pShaderBlob);

    /// Flags defining the shader object chunks.
    typedef enum
    {
        RDEF       = 0x001,   ///< Resource definition chunk. Mandatory.
        ISGN       = 0x002,   ///< Input signature chunk. Mandatory.
        OSGN       = 0x004,   ///< Output signature chunk. Mandatory.
        ShaderCode = 0x008,   ///< Shader byte-code definition chunk. Mandatory. SHDR or SHEX chunk.
        STAT       = 0x010,   ///< Statistics chunk. Optional.
        SDBG       = 0x020,   ///< Debug symbol chunk. Optional.
        Aon9       = 0x040,   ///< Unknown.
        SFI0       = 0x080,   ///< Unknown.
        IFCE       = 0x100,   ///< Unknown.
        PCSG       = 0x200,   ///< Unknown.
        OSG5       = 0x400,   ///< Unknown.
    } ShaderObjectChunks;

    static const UINT AllChunks         = 0xffff;                           ///< Flags to request all chunks.
    static const UINT MandatoryChunks   = RDEF | ISGN | OSGN | ShaderCode | STAT;   ///< Flags to request all mandatory chunks.
    static const UINT NoDebug           = RDEF | ISGN | OSGN | ShaderCode | STAT;   ///< Flags to request all chunks except the debug symbol chunk.

    /// Create a new IBlob object containing the current shader object.
    /// \param[in] nChunks Which chunks to include.
    /// \return A pointer to the IBlob if successful, otherwise NULL.
    IBlob* GetShaderObject(UINT nChunks) const;

    /// Get a pointer to the current RDEF (Resource definition) chunk.
    /// \return A pointer to the current RDEF chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetRDEFChunk() const { return m_pRDEFChunk; };

    /// Get a pointer to the current ISGN (Input signature) chunk.
    /// \return A pointer to the current ISGN chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetISGNChunk() const { return m_pISGNChunk; };

    /// Get a pointer to the current OSGN (Output signature) chunk.
    /// \return A pointer to the current OSGN chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetOSGNChunk() const { return m_pOSGNChunk; };

    /// Get a pointer to the current SHDR (Shader byte-code) chunk.
    /// \return A pointer to the current SHDR chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetSHDRChunk() const { return m_pSHDRChunk; };

    /// Get a pointer to the current SHEX (Extended Shader byte-code) chunk.
    /// \return A pointer to the current SHEX chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetSHEXChunk() const { return m_pSHEXChunk; };

    /// Get a pointer to the current shader byte-code chunk.
    /// This will be either the SHDR or SHEX chunk, depending on what is present.
    /// \return A pointer to the current shader byte-code chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetShaderCodeChunk() const { return (GetSHEXChunk() != NULL) ? GetSHEXChunk() : GetSHDRChunk(); };

    /// Get a pointer to the current STAT (Statistics) chunk.
    /// \return A pointer to the current STAT chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetSTATChunk() const { return m_pSTATChunk; };

    /// Get a pointer to the current SDBG (Debug symbol) chunk.
    /// \return A pointer to the current SDBG chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetSDBGChunk() const { return m_pSDBGChunk; };

    /// Get a pointer to the current Aon9 (Unknown) chunk.
    /// \return A pointer to the current Aon9 chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetAon9Chunk() const { return m_pAon9Chunk; };

    /// Get a pointer to the current SFI0 (Unknown) chunk.
    /// \return A pointer to the current SFI0 chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetSFI0Chunk() const { return m_pSFI0Chunk; };

    /// Get a pointer to the current IFCE (Unknown) chunk.
    /// \return A pointer to the current IFCE chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetIFCEChunk() const { return m_pIFCEChunk; };

    /// Get a pointer to the current PCSG (Unknown) chunk.
    /// \return A pointer to the current PCSG chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetPCSGChunk() const { return m_pPCSGChunk; };

    /// Get a pointer to the current OSG5 (Unknown) chunk.
    /// \return A pointer to the current OSG5 chunk if it exists, otherwise NULL.
    const D3D10_ChunkHeader* GetOSG5Chunk() const { return m_pOSG5Chunk; };

    /// Get the size of the current RDEF (Resource definition) chunk.
    /// \return The size of the current RDEF chunk if it exists, otherwise 0.
    DWORD GetRDEFChunkSize() const { return CalculateChunkSize(m_pRDEFChunk); };

    /// Get the size of the current ISGN (Input signature) chunk.
    /// \return The size of the current ISGN chunk if it exists, otherwise 0.
    DWORD GetISGNChunkSize() const { return CalculateChunkSize(m_pISGNChunk); };

    /// Get the size of the current OSGN (Output signature) chunk.
    /// \return The size of the current OSGN chunk if it exists, otherwise 0.
    DWORD GetOSGNChunkSize() const { return CalculateChunkSize(m_pOSGNChunk); };

    /// Get the size of the current SHDR (Shader byte-code) chunk.
    /// \return The size of the current SHDR chunk if it exists, otherwise 0.
    DWORD GetSHDRChunkSize() const { return CalculateChunkSize(m_pSHDRChunk); };

    /// Get the size of the current SHEX (EXtended Shader byte-code) chunk.
    /// \return The size of the current SHEX chunk if it exists, otherwise 0.
    DWORD GetSHEXChunkSize() const { return CalculateChunkSize(m_pSHEXChunk); };

    /// Get the size of the current Shader byte-code chunk.
    /// This will be either the SHDR or SHEX chunk, depending on what is present.
    /// \return The size of the current Shader byte-code chunk if it exists, otherwise 0.
    DWORD GetShaderCodeChunkSize() const { return (GetSHEXChunk() != NULL) ? GetSHEXChunkSize() : GetSHDRChunkSize(); };

    /// Get the size of the current STAT (Statistics) chunk.
    /// \return The size of the current STAT chunk if it exists, otherwise 0.
    DWORD GetSTATChunkSize() const { return CalculateChunkSize(m_pSTATChunk); };

    /// Get the size of the current SDBG (Debug symbol) chunk.
    /// \return The size of the current SDBG chunk if it exists, otherwise 0.
    DWORD GetSDBGChunkSize() const { return CalculateChunkSize(m_pSDBGChunk); };

    /// Get the size of the current Aon9 (Unknown) chunk.
    /// \return The size of the current Aon9 chunk if it exists, otherwise 0.
    DWORD GetAon9ChunkSize() const { return CalculateChunkSize(m_pAon9Chunk); };

    /// Get the size of the current SFI0 (Unknown) chunk.
    /// \return The size of the current SFI0 chunk if it exists, otherwise 0.
    DWORD GetSFI0ChunkSize() const { return CalculateChunkSize(m_pSFI0Chunk); };

    /// Get the size of the current IFCE (Unknown) chunk.
    /// \return The size of the current IFCE chunk if it exists, otherwise 0.
    DWORD GetIFCEChunkSize() const { return CalculateChunkSize(m_pIFCEChunk); };

    /// Get the size of the current PCSG (Unknown) chunk.
    /// \return The size of the current PCSG chunk if it exists, otherwise 0.
    DWORD GetPCSGChunkSize() const { return CalculateChunkSize(m_pPCSGChunk); };

    /// Get the size of the current OSG5 (Unknown) chunk.
    /// \return The size of the current OSG5 chunk if it exists, otherwise 0.
    DWORD GetOSG5ChunkSize() const { return CalculateChunkSize(m_pOSG5Chunk); };

    /// Set the RDEF (Resource definition) chunk.
    /// \param[in] pChunk A pointer to the RDEF chunk.
    /// \return True if successful, otherwise false.
    bool SetRDEFChunk(const D3D10_ChunkHeader* pChunk);

    /// Set the ISGN (Input signature) chunk.
    /// \param[in] pChunk A pointer to the ISGN chunk.
    /// \return True if successful, otherwise false.
    bool SetISGNChunk(const D3D10_ChunkHeader* pChunk);

    /// Set the OSGN (Output signature) chunk.
    /// \param[in] pChunk A pointer to the OSGN chunk.
    /// \return True if successful, otherwise false.
    bool SetOSGNChunk(const D3D10_ChunkHeader* pChunk);

    /// Set the SHDR (Shader byte-code) chunk.
    /// \param[in] pChunk A pointer to the SHDR chunk.
    /// \return True if successful, otherwise false.
    bool SetSHDRChunk(const D3D10_ChunkHeader* pChunk);

    /// Set the SHEX (EXtendeded Shader byte-code) chunk.
    /// \param[in] pChunk A pointer to the SHEX chunk.
    /// \return True if successful, otherwise false.
    bool SetSHEXChunk(const D3D10_ChunkHeader* pChunk);

    /// Set the Shader byte-code chunk.
    /// This will be either the SHDR or SHEX chunk, depending on what is present.
    /// \param[in] pChunk A pointer to the Shader byte-code chunk.
    /// \return True if successful, otherwise false.
    bool SetShaderCodeChunk(const D3D10_ChunkHeader* pChunk);

    /// Set the STAT (Statistics) chunk.
    /// \param[in] pChunk A pointer to the STAT chunk.
    /// \return True if successful, otherwise false.
    bool SetSTATChunk(const D3D10_ChunkHeader* pChunk);

    /// Set the SDBG (Debug symbol) chunk.
    /// \param[in] pChunk A pointer to the SDBG chunk.
    /// \return True if successful, otherwise false.
    bool SetSDBGChunk(const D3D10_ChunkHeader* pChunk);

    /// Set the Aon9 (Unknown) chunk.
    /// \param[in] pChunk A pointer to the Aon9 chunk.
    /// \return True if successful, otherwise false.
    bool SetAon9Chunk(const D3D10_ChunkHeader* pChunk);

    /// Set the SFI0 (Unknown) chunk.
    /// \param[in] pChunk A pointer to the SFI0 chunk.
    /// \return True if successful, otherwise false.
    bool SetSFI0Chunk(const D3D10_ChunkHeader* pChunk);

    /// Set the IFCE (Unknown) chunk.
    /// \param[in] pChunk A pointer to the IFCE chunk.
    /// \return True if successful, otherwise false.
    bool SetIFCEChunk(const D3D10_ChunkHeader* pChunk);

    /// Set the PCSG (Unknown) chunk.
    /// \param[in] pChunk A pointer to the PCSG chunk.
    /// \return True if successful, otherwise false.
    bool SetPCSGChunk(const D3D10_ChunkHeader* pChunk);

    /// Set the OSG5 (Unknown) chunk.
    /// \param[in] pChunk A pointer to the OSG5 chunk.
    /// \return True if successful, otherwise false.
    bool SetOSG5Chunk(const D3D10_ChunkHeader* pChunk);

    /// Get the shader statistics.
    /// \return A reference to the shader statistics.
    const D3D10_SHADER_STATS& GetStats() const { return m_pStats ? *m_pStats : m_EmptyStats; }

    /// Have we got source code for the shader?
    /// \return True if we have source code for the shader, otherwise false.
    bool GotSourceCode() const { return m_bGotSourceCode; }

    /// Get the source code for the shader.
    /// \param[out] strShader The shader source code.
    /// \return True if successful, otherwise false.
    bool GetShaderSource(std::string& strShader) const;

    /// Get the entry-point function name.
    /// \return The entry-point function name.
    const std::string& GetEntrypointName() const { return m_strEntrypointName; }

    /// Get the entry-point function line number.
    /// \return The entry-point function line number.
    const UINT GetEntrypointLine() const { return m_nEntrypointLine; }

    /// Get the shader target.
    /// \return The shader target.
    const std::string& GetShaderTarget() const { return m_strShaderTarget; }

    /// Get the D3D10_SHADER_xxx flags passed to the D3D10 HLSL compiler.
    /// \return The compilation flags.
    UINT GetCompileFlags() const { return m_uiCompileFlags; }

    /// Get the shader disassembly code.
    /// \return The disassembly code.
    const std::string& GetDisassembly() const { return m_strDisassembly; }

    /// Set the shader disassembly code.
    /// \param[in] code The shader disassembly code.
    void SetDisassembly(const std::string& code) { m_strDisassembly = code; }

    /// Get the thread group size.
    /// \return The thread group size.
    const DWORD* GetThreadGroupSize() const { return m_dwThreadGroupSize; }

    /// Get the bound resource count.
    /// \return The bound resource count.
    const DWORD GetBoundResourceCount() const { return (DWORD) m_BoundResources.size(); };

    /// Get the bound resources.
    /// \return A reference to the bound resources.
    const std::vector<D3D10_ResourceBinding>& GetBoundResources() const { return m_BoundResources; };

    /// Get the constant buffer count.
    /// \return The constant buffer count.
    const DWORD GetConstantBufferCount() const { return m_dwConstantBuffers; };

    /// Get the constant buffers.
    /// \return A reference to the constant buffers.
    const std::vector<D3D10_ConstantBuffer>& GetConstantBuffers() const { return m_ConstantBuffers; };

    /// Get the texture count.
    /// \return The texture count.
    const DWORD GetTextureCount() const { return m_dwTextures; };

    /// Get the texture return type.
    /// \param[in] dwIndex  The index of the texture to return the type of.
    /// \return The texture return type.
    const D3D10_RESOURCE_RETURN_TYPE GetTextureReturnType(DWORD dwIndex) const;

    /// Get the texture dimension.
    /// \param[in] dwIndex  The index of the texture to return the dimension of.
    /// \return The texture dimension.
    const D3D10_SRV_DIMENSION GetTextureDimension(DWORD dwIndex) const;

    /// Get the sampler count.
    /// \return The sampler count.
    const DWORD GetSamplerCount() const { return m_dwSamplers; };

    /// Get the compute buffer count.
    /// \return The compute buffer count.
    const DWORD GetComputeBufferCount() const { return m_dwComputeBuffers; };

    /// Get the input signature count.
    /// \return The input signature count.
    const DWORD GetInputSignatureCount() const { return m_dwInputSignatures; };

    /// Get the input signatures.
    /// \return A reference to the input signatures.
    const std::vector<D3D10_Signature>& GetInputSignatures() const { return m_InputSignatures; };

    /// Get the input semantic names.
    /// \return The ISGN semantic names.
    const std::vector<std::string>& GetInputSemanticNames() const { return m_strISGNSemanticNames; }

    /// Get the output signature count.
    /// \return The output signature count.
    const DWORD GetOutputSignatureCount() const { return m_dwOutputSignatures; };

    /// Get the output signatures.
    /// \return A reference to the output signatures.
    const std::vector<D3D10_Signature>& GetOutputSignatures() const { return m_OutputSignatures; };

    /// Get the output semantic names.
    /// \return The OSGN semantic names.
    const std::vector<std::string>& GetOutputSemanticNames() const { return m_strOSGNSemanticNames; }

    /// Set the input signature count.
    /// \param[in] dwInputSignatures The input signature count.
    /// \return The input signature count.
    bool SetInputSignatureCount(const DWORD dwInputSignatures) { m_dwInputSignatures = dwInputSignatures;  return true; };

    /// Set the input signatures.
    /// \param[in] InputSignatures The input signatures.
    /// \return True if successful, otherwise false.
    bool SetInputSignatures(const std::vector<D3D10_Signature>& InputSignatures) { m_InputSignatures = InputSignatures;  return true; };

    /// Set the input semantic names.
    /// \param[in] strISGNSemanticNames The input semantic names.
    /// \return True if successful, otherwise false.
    bool SetInputSemanticNames(const std::vector<std::string>& strISGNSemanticNames) { m_strISGNSemanticNames = strISGNSemanticNames;  return true; }

    /// Set the output signature count.
    /// \param[in] dwOutputSignatures The output signature count.
    /// \return True if successful, otherwise false.
    bool SetOutputSignatureCount(const DWORD dwOutputSignatures) { m_dwOutputSignatures = dwOutputSignatures;  return true; };

    /// Set the output signatures.
    /// \param[in] OutputSignatures The output signatures.
    /// \return True if successful, otherwise false.
    bool SetOutputSignatures(const std::vector<D3D10_Signature>& OutputSignatures) { m_OutputSignatures = OutputSignatures;  return true; };

    /// Set the output semantic names.
    /// \param[in] strOSGNSemanticNames The output semantic names.
    /// \return True if successful, otherwise false.
    bool SetOutputSemanticNames(const std::vector<std::string>& strOSGNSemanticNames) { m_strOSGNSemanticNames = strOSGNSemanticNames; return true; }

    /// Get the render target count.
    /// \return The render target count.
    const DWORD GetRenderTargetCount() const { return m_dwRenderTargets; };

    /// Get the SDBG instruction infos.
    /// \return The SDBG instruction infos.
    const std::vector<D3D10_SHADER_DEBUG_INST_INFO>& GetDebugInstructions() const { return m_Instructions; }

    /// Get the SDBG var infos.
    /// \return The SDBG var infos.
    const std::vector<D3D10_SHADER_DEBUG_VAR_INFO>& GetDebugVariables() const { return  m_Variables; }

    /// Get the SDBG input variable infos.
    /// \return The SDBG input variable infos.
    const std::vector<D3D10_SHADER_DEBUG_INPUT_INFO>& GetDebugInputVariables() const { return m_InputVariables; }

    /// Get the SDBG tokens names.
    /// \return The SDBG tokens names.
    const std::vector<std::string>& GetDebugStrTokens() const { return m_strTokens; }

    /// Get the SDBG token infos.
    /// \return The SDBG token infos.
    const std::vector<D3D10_SHADER_DEBUG_TOKEN_INFO>& GetDebugTokens() const { return m_Tokens; }

    /// Get the SDBG scopes names.
    /// \return The SDBG scopes names.
    const std::vector<std::string>& GetDebugStrScopes() const { return m_strScopes; }

    /// Get the SDBG scopes infos.
    /// \return The SDBG scopes infos.
    const std::vector<D3D10_SHADER_DEBUG_SCOPE_INFO>& GetDebugScopes() const { return m_Scopes; }

    /// Get the SDBG scope variables info.
    /// \return The SDBG scope variable infos.
    const std::vector<D3D10_SHADER_DEBUG_SCOPEVAR_INFO>& GetDebugScopeVariables() const { return m_ScopeVariables; }

    /// Get the SDBG scope indices.  The scope indices is a 2D array indexed by instruction# and scope#.
    /// \return The SDBG scope indices.
    const std::vector< std::vector< UINT > >& GetDebugScopeIndices() const { return m_ScopeIndices; }

    /// Get the SDBG number of lines per file.
    /// \return The SDBG number of lines per file.
    const std::vector< UINT >& GetDebugNumberOfLinesInFiles() const { return m_NumberOfLinesInFiles; }

    /// Get the immediate constant count.
    /// \return The immediate constant count.
    const DWORD GetImmediateConstantCount() const { return m_dwImmediateConstants; };

    /// Get the indexable temp register count.
    /// \return The indexable temp register count.
    const DWORD GetIndexableTempRegisterCount() const { return m_dwIndexableTempRegisters; };

    /// Get the indexable temp registers.
    /// \return The indexable temp registers.
    const std::vector<D3D10_IndexableTempRegister>& GetIndexableTempRegisters() const { return m_IndexableTempRegisters; }

    /// Get the global memory register count.
    /// \return The global memory register count.
    const DWORD GetGlobalMemoryRegisterCount() const { return m_dwGlobalMemoryRegisters; };

    /// Get the global memory registers.
    /// \return The global memory registers.
    const std::vector<D3D10_GlobalMemoryRegister>& GetGlobalMemoryRegisters() const { return m_GlobalMemoryRegisters; }

    /// Get the shader type.
    /// \return The shader type.
    D3D10_SB_TOKENIZED_PROGRAM_TYPE GetShaderType() const { return DECODE_D3D10_SB_TOKENIZED_PROGRAM_TYPE(m_dwSHDRVersion); };

    /// Get the shader model.
    /// \return The shader model.
    WORD GetShaderModel() const { return (WORD)((DECODE_D3D10_SB_TOKENIZED_PROGRAM_MAJOR_VERSION(m_dwSHDRVersion) << D3D10_SB_TOKENIZED_PROGRAM_MAJOR_VERSION_SHIFT) | DECODE_D3D10_SB_TOKENIZED_PROGRAM_MINOR_VERSION(m_dwSHDRVersion)); };

    /// Find the first unused compute slot.
    /// \param[out] dwUnusedComputeSlot The first unused compute slot if successful, otherwise undefined.
    /// \return true if successful, otherwise false.
    bool FindUnusedComputeSlot(DWORD& dwUnusedComputeSlot) const;

    /// Add a Raw UAV definition to the resource definitions.
    /// \param[in] dwIndex  The index of the Raw UAV to add.
    /// \param[in] pszName  The name of the Raw UAV to add.
    /// \return true if successful, otherwise false.
    bool AddRawUAV(const DWORD dwIndex, const char* pszName);

    /// Clear the output signature data.
    void ClearOutputSignatures();

    /// Add a semantic signature to the input signature.
    /// \param[in] strName    Semantic name to add.
    /// \param[in] signature  The signature to add.
    /// \return true if successful, otherwise false.
    bool AddInputSignature(const std::string strName, const D3D10_Signature signature);

    /// Add a semantic signature to the output signature.
    /// \param[in] strName    Semantic name to add.
    /// \param[in] signature  The signature to add.
    /// \return true if successful, otherwise false.
    bool AddOutputSignature(const std::string strName, const D3D10_Signature signature);

    /// Generate the RDEF chunk to based on the resource definition data.
    /// \return True if successful, otherwise false.
    bool GenerateRDEFChunk();

    /// Generate the ISGN chunk to based on the input signature data.
    /// \return True if successful, otherwise false.
    bool GenerateISGNChunk();

    /// Generate the OSGN chunk to based on the output signature data.
    /// \return True if successful, otherwise false.
    bool GenerateOSGNChunk();

private:
    /// Initialize the member data to NULL values.
    void InitialiseData();

    /// Free any member data.
    void FreeData();

    /// Calculate the size of a chunk.
    /// \param[in] pChunkHeader A pointer to the chunk to calculate the size of.
    /// \return The size of the chunk if valid, otherwise false.
    DWORD CalculateChunkSize(const D3D10_ChunkHeader* pChunkHeader) const;

    /// Create a copy of the chunk.
    /// \param[in] pChunkHeader A pointer to the chunk to copy.
    /// \return A pointer to a new copy of the chunk if successful, otherwise false.
    D3D10_ChunkHeader* CopyChunk(const D3D10_ChunkHeader* pChunkHeader) const;


    /// Clear the resource definition data.
    void ClearResourceDefinitions();

    /// Parse the RDEF chunk to extract the resource definition data.
    /// \return True if successful, otherwise false.
    bool ParseRDEFChunk();

    /// Parse a xSGN chunk to extract signature data.
    /// \param[in] pChunk A pointer of the chunk to parse.
    /// \param[out] dwSignatures The number of signatures in the chunk.
    /// \param[out] dwReserved The value of the reserved field within the xSGN header.
    /// \param[out] strSemanticNames A std::vector containing the semantic names of the signatures.
    /// \param[out] signatures A std::vector containing the D3D10_Signature signature structures.
    /// \return True if successful, otherwise false.
    bool ParseSGNChunk(const D3D10_ChunkHeader* pChunk, DWORD& dwSignatures, DWORD& dwReserved,
                       std::vector<std::string>& strSemanticNames, std::vector<D3D10_Signature>& signatures);

    /// Generates a xSGN chunk based on the passed signature data.
    /// \param[in] dwChunkType A FourCC value identifying the chunk type.
    /// \param[in] dwSignatures The number of signatures in the chunk.
    /// \param[in] strSemanticNames A std::vector containing the semantic names of the signatures.
    /// \param[in] signatures A std::vector containing the D3D10_Signature signature structures.
    /// \return A pointer to the xSGN chunk. Memory is malloc'ed in function for chunk. caller responsible for free'ing. NULL if unsuccessful.
    D3D10_ChunkHeader* GenerateSGNChunk(const DWORD dwChunkType, const DWORD dwSignatures,
                                        const std::vector<std::string>& strSemanticNames,
                                        const std::vector<D3D10_Signature>& signatures);

    /// Clear the input signature data.
    void ClearInputSignatures();

    /// Parse the ISGN chunk to extract the input signature data.
    /// \return True if successful, otherwise false.
    bool ParseISGNChunk();

    /// Parse the OSGN chunk to extract the output signature data.
    /// \return True if successful, otherwise false.
    bool ParseOSGNChunk();

    /// Clear the shader byte-code data.
    void ClearShaderBytecode();

    /// Parse the SHDR chunk to extract the shader byte-code data.
    /// \return True if successful, otherwise false.
    bool ParseSHDRChunk();

    /// Parse the SHEX chunk to extract the shader byte-code data.
    /// \return True if successful, otherwise false.
    bool ParseSHEXChunk();

    /// Clear the statistics data.
    void ClearStatistics();

    /// Parse the STAT chunk to extract the statistics data.
    /// \return True if successful, otherwise false.
    bool ParseSTATChunk();

    /// Clear the debug info data.
    void ClearDebugInfo();

    /// Parse the SDBG chunk to extract the debug info data.
    /// \return True if successful, otherwise false.
    bool ParseSDBGChunk();

    /// Parse the Aon9 chunk.
    /// \return True if successful, otherwise false.
    bool ParseAon9Chunk();

    /// Parse the SFI0 chunk.
    /// \return True if successful, otherwise false.
    bool ParseSFI0Chunk();

    /// Parse the IFCE chunk.
    /// \return True if successful, otherwise false.
    bool ParseIFCEChunk();

    /// Parse the PCSG chunk.
    /// \return True if successful, otherwise false.
    bool ParsePCSGChunk();

    /// Parse the OSG5 chunk.
    /// \return True if successful, otherwise false.
    bool ParseOSG5Chunk();

    /// Add an entry to the RDEF name map.
    /// \param[in] pszStringTable A pointer to the RDEF string table.
    /// \param[in] dwOffset       The offset of the name within the string table.
    void AddRDEFName(const char* pszStringTable, DWORD dwOffset);

    /// Add a new entry to the RDEF name map.
    /// \param[in] pszName        The name to add to the RDEF string table.
    /// \return                   The offset of the name within the string table is successful, otherwise false.
    DWORD AddRDEFName(const char* pszName);

    /// Get the Resource Name at the specified offset.
    /// \param[in] dwOffset       The offset from which to retrieve the name.
    /// \return                   The name at this offset if successful, otherwise "".
    const std::string& GetRDEFName(DWORD dwOffset);

    // The shader object
    void* m_pShader;                                                        ///< Pointer to the shader buffer.
    size_t m_nShaderDataSize;                                               ///< The size of the shader buffer.

    std::string m_strCheckSum;                                              ///< Text version of the shader check-sum.

    // Chunk pointers
    D3D10_ChunkHeader* m_pRDEFChunk;                                        ///< A pointer to the Resource Definition (RDEF) chunk.
    D3D10_ChunkHeader* m_pISGNChunk;                                        ///< A pointer to the Input Signature (ISGN) chunk.
    D3D10_ChunkHeader* m_pOSGNChunk;                                        ///< A pointer to the Output Signature (OSGN) chunk.
    D3D10_ChunkHeader* m_pSHDRChunk;                                        ///< A pointer to the Shader Byte-code (SHDR) chunk.
    D3D10_ChunkHeader* m_pSTATChunk;                                        ///< A pointer to the Statistics (STAT) chunk.
    D3D10_ChunkHeader* m_pSDBGChunk;                                        ///< A pointer to the Shader Debug Symbol (SDBG) chunk.
    D3D10_ChunkHeader* m_pSHEXChunk;                                        ///< A pointer to the Extended Shader Byte-code (SHEX) chunk.
    D3D10_ChunkHeader* m_pAon9Chunk;                                        ///< A pointer to the DX10 on DX9 Shader Byte-code (Aon9) chunk.
    D3D10_ChunkHeader* m_pSFI0Chunk;                                        ///< A pointer to the SFI0 chunk.
    D3D10_ChunkHeader* m_pIFCEChunk;                                        ///< A pointer to the IFCE chunk.
    D3D10_ChunkHeader* m_pPCSGChunk;                                        ///< A pointer to the PCSG chunk.
    D3D10_ChunkHeader* m_pOSG5Chunk;                                        ///< A pointer to the OSG5 chunk.

    /// \name Resource Definition (RDEF) Chunk data
    DWORD                               m_dwConstantBuffers;                ///< The number of constant buffers.
    DWORD                               m_dwVersion;                        ///< The number of shader version.
    DWORD                               m_dwFlags;                          ///< The flags passed to the D3D10 HLSL compiler.
    DWORD                               m_dwConstants;                      ///< The number of constants.
    DWORD                               m_dwTextures;                       ///< The number of textures.
    DWORD                               m_dwSamplers;                       ///< The number of samplers.
    DWORD                               m_dwComputeBuffers;                 ///< The number of compute buffers.
    std::string                         m_strRDEFCreator;                   ///< The creator string - aka the name & version of the HLSL compiler.
    std::vector<D3D10_ResourceBinding>  m_BoundResources;                   ///< The bound resources.
    std::vector<D3D10_ConstantBuffer>   m_ConstantBuffers;                  ///< The constant buffers.
    std::vector<D3D10_Constant>         m_Constants;                        ///< The constants.
    std::map<DWORD, std::string>        m_strRDEFNames;                     ///< The names of resources (constant buffers, resources, etc.)
    std::map<DWORD, D3D10_Type>         m_ConstantTypes;                    ///< The constant types.
    D3D11_RDEF_Header*                  m_pRDEF11_Header;                   ///< A pointer to the RDEF11 header if present.

    /// \name Input Signature (ISGN) Chunk data
    DWORD                         m_dwInputSignatures;                      ///< The number of input signatures.
    DWORD                         m_dwInputReserved;                        ///< Unknown.
    std::vector<std::string>      m_strISGNSemanticNames;                   ///< The input signature names.
    std::vector<D3D10_Signature>  m_InputSignatures;                        ///< The input signatures.

    /// \name Output Signature (OSGN) Chunk data
    DWORD                         m_dwOutputSignatures;                     ///< The number of output signatures.
    DWORD                         m_dwOutputReserved;                       ///< Unknown.
    std::vector<std::string>      m_strOSGNSemanticNames;                   ///< The output signature names.
    std::vector<D3D10_Signature>  m_OutputSignatures;                       ///< The output signatures.
    DWORD                         m_dwRenderTargets;                        ///< The number of render-targets.

    /// \name Shader Byte-code (SHDR) Chunk data
    DWORD                                     m_dwSHDRVersion;              ///< The SHDR chunk version.
    DWORD                                     m_dwImmediateConstants;       ///< The number of immediate constants.
    DWORD                                     m_dwIndexableTempRegisters;   ///< The number of indexable temporary registers.
    std::vector<D3D10_IndexableTempRegister>  m_IndexableTempRegisters;     ///< The indexable temporary registers.

    /// \name Extended Shader Byte-code (SHEX) Chunk data
    DWORD                                     m_dwGlobalMemoryRegisters;    ///< The number of global memory registers.
    std::vector<D3D10_GlobalMemoryRegister>   m_GlobalMemoryRegisters;      ///< The global memory registers.
    DWORD m_dwThreadGroupSize[3];                                           ///< The size (width, height & depth) of the thread group.

    /// \name Statistics (STAT) Chunk data
    D3D10_SHADER_STATS* m_pStats;                                     ///< Pointer to the D3D10 shader stats structure.
    D3D10_SHADER_STATS  m_EmptyStats;                                 ///< An empty D3D10 shader stats structure. Used when the shader has no stats chunk.

    /// \name Shader Debug Symbol (SDBG) Chunk data
    std::string m_strCreator;                                         ///< The name & version of the HLSL compiler.
    std::string m_strEntrypointName;                                  ///< The entry-point function name.
    UINT m_nEntrypointLine;                                           ///< The entry-point function line number.
    std::string m_strShaderTarget;                                    ///< The shader target name (ie. shader type & model).
    UINT m_uiCompileFlags;                                            ///< The compilation flags.
    UINT m_uiFiles;                                                   ///< The number of files comprising the shader.
    std::vector<UINT> m_NumberOfLinesInFiles;                         ///< The number of lines per file.
    std::vector<std::string> m_strFiles;                              ///< The file names.
    std::string m_strSource;                                          ///< The shader source code.
    bool m_bGotSourceCode;                                            ///< Have we got source code.

    UINT m_uiInstructions;                                            ///< The number of instructions.
    std::vector<D3D10_SHADER_DEBUG_INST_INFO> m_Instructions;         ///< The instruction infos.

    UINT m_uiVariables;                                               ///< The number of variables.
    std::vector<D3D10_SHADER_DEBUG_VAR_INFO> m_Variables;             ///< The variable infos.

    UINT m_uiInputVariables;                                          ///< The number of input variables.
    std::vector<D3D10_SHADER_DEBUG_INPUT_INFO> m_InputVariables;      ///< The input variable infos.

    UINT m_uiTokens;                                                  ///< The number of tokens.
    std::vector<std::string> m_strTokens;                             ///< The token names.
    std::vector<D3D10_SHADER_DEBUG_TOKEN_INFO> m_Tokens;              ///< The token infos.

    UINT m_uiScopes;                                                  ///< The number of scopes.
    std::vector<std::string> m_strScopes;                             ///< The scope names.
    std::vector<D3D10_SHADER_DEBUG_SCOPE_INFO> m_Scopes;              ///< The scope infos.

    UINT m_uiScopeVariables;                                          ///< The number of scope variables.
    std::vector<D3D10_SHADER_DEBUG_SCOPEVAR_INFO> m_ScopeVariables;   ///< The scope variable infos.

    std::vector< std::vector< UINT > > m_ScopeIndices;                ///< The scope indices.

    std::string m_strDisassembly;                                     /// The shader disassembly text.
};

}; // D3D10ShaderObject

#endif // D3D10SHADEROBJECT_H
