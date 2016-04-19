//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A simple class to store shader specific information
//==============================================================================

#ifndef GPS_SHADER_DATA_H
#define GPS_SHADER_DATA_H

#include "misc.h"

/// The following class is used in a cache that relates shader code to
/// DX runtime shader pointers.
class ShaderData
{
protected:
    /// The shader bytecode code that is used for this shader
    unsigned char* m_pCode;

    /// Length of the shader bytecode
    UINT32 m_dwSize;

    /// Store the shader source code
    std::string m_strSource;

public:

    //--------------------------------------------------------------------------
    /// Constructor
    //--------------------------------------------------------------------------
    ShaderData()
    {
        m_pCode = NULL;
        m_dwSize = 0;
    }

    //--------------------------------------------------------------------------
    /// destructor - deletes shader code.
    //--------------------------------------------------------------------------
    ~ShaderData()
    {
        SAFE_DELETE_ARRAY(m_pCode);
    }

    //--------------------------------------------------------------------------
    /// Accessor for setting shader code and size
    /// \param pIn The input shader code.
    /// \param dwSize The size of the shader string
    //--------------------------------------------------------------------------
    void Set(char* pIn, UINT32 dwSize)
    {
        // make sure the shader code is empty before trying to allocate more memory.
        SAFE_DELETE_ARRAY(m_pCode);

        if (PsNewArray< unsigned char >(m_pCode, dwSize) == false)
        {
            Log(logERROR, "Failed to allocate memory for shader code\n");
            return;
        }

        m_dwSize = dwSize;
        memcpy_s(m_pCode, dwSize, pIn, dwSize);
    }

    //--------------------------------------------------------------------------
    /// Accessor for getting the shader code.
    /// \return The shader code string.
    //--------------------------------------------------------------------------
    const unsigned char* GetCode()const
    {
        return m_pCode;
    }

    //--------------------------------------------------------------------------
    /// Accessor for getting the size of the shader code string.
    /// \return The size of the shader code string.
    //--------------------------------------------------------------------------
    UINT32 GetSize()
    {
        return m_dwSize;
    }

    //--------------------------------------------------------------------------
    /// Setter for the shader source code string.
    /// \param src The shader source code
    //--------------------------------------------------------------------------
    void SetShaderSource(std::string src)
    {
        m_strSource = src;
    }

    //--------------------------------------------------------------------------
    /// Accessor for getting the shader source code string.
    /// \return A reference to the string object
    //--------------------------------------------------------------------------
    std::string& GetShaderSource()
    {
        return m_strSource ;
    }
};

#endif //GPS_SHADER_DATA_H