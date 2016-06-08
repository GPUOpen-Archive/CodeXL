//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLVBO.cpp
///
//==================================================================================

// -----------------------------   apGLVBO.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apGLVBO.h>

// ---------------------------------------------------------------------------
// Name:        apGLVBO::apGLVBO
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
apGLVBO::apGLVBO()
    : apAllocatedObject(), _vboName(0), m_lastBufferTarget(GL_NONE), _size(0), _bufferFile(L""), _isDirty(true), _displayFormat(OA_TEXEL_FORMAT_V3F), _offset(0), _stride(0),
      _openCLBufferIndex(-1), _openCLBufferName(-1), _openCLSpyID(-1)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLVBO::apGLVBO
// Description: Copy constructor
// Arguments: other - The other VBO class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
apGLVBO::apGLVBO(const apGLVBO& other)
{
    apGLVBO::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apGLVBO::~apGLVBO
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
apGLVBO::~apGLVBO()
{
}

// ---------------------------------------------------------------------------
// Name:        apGLVBO::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
apGLVBO& apGLVBO::operator=(const apGLVBO& other)
{
    _vboName = other._vboName;
    m_lastBufferTarget = other.m_lastBufferTarget;
    size_t activeTargetCount = other.m_activeBufferTargets.size();
    m_activeBufferTargets.resize(activeTargetCount);

    for (size_t i = 0; i < activeTargetCount; ++i) { m_activeBufferTargets[i] = other.m_activeBufferTargets[i]; }

    size_t targetHistorySize = other.m_bufferTargetHistory.size();
    m_bufferTargetHistory.resize(targetHistorySize);

    for (size_t i = 0; i < targetHistorySize; ++i) { m_bufferTargetHistory[i] = other.m_bufferTargetHistory[i]; }

    _size = other._size;
    _bufferFile = other._bufferFile;
    _isDirty = other._isDirty;
    _displayFormat = other._displayFormat;
    _offset = other._offset;
    _stride = other._stride;

    _openCLBufferIndex = other._openCLBufferIndex;
    _openCLBufferName = other._openCLBufferName;
    _openCLSpyID = other._openCLSpyID;

    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apGLVBO::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apGLVBO::type() const
{
    return OS_TOBJ_ID_GL_VBO;
}

// ---------------------------------------------------------------------------
// Name:        apGLVBO::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/5/2008
// ---------------------------------------------------------------------------
bool apGLVBO::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the VBO name:
    ipcChannel << (gtUInt32)_vboName;

    // Write the buffer target data:
    ipcChannel << (gtUInt32)m_lastBufferTarget;

    size_t activeTargetCount = m_activeBufferTargets.size();
    ipcChannel << (gtUInt32)activeTargetCount;

    for (size_t i = 0; i < activeTargetCount; ++i)
    {
        ipcChannel << (gtUInt32)m_activeBufferTargets[i];
    }

    size_t targetHistorySize = m_bufferTargetHistory.size();
    ipcChannel << (gtUInt32)targetHistorySize;

    for (size_t i = 0; i < targetHistorySize; ++i)
    {
        ipcChannel << (gtUInt32)m_bufferTargetHistory[i];
    }

    // Write the VBO size:
    ipcChannel << (gtUInt64)_size;

    // Write the VBO display format:
    ipcChannel << (gtInt32)_displayFormat;

    // Write the VBO offset:
    ipcChannel << (gtUInt64)_offset;

    // Write the VBO stride:
    ipcChannel << (gtUInt64)_stride;

    // Write OpenCL interoperability details:
    ipcChannel << (gtInt32)_openCLBufferIndex;
    ipcChannel << (gtInt32)_openCLBufferName;
    ipcChannel << (gtInt32)_openCLSpyID;

    // Write buffer file path into channel:
    _bufferFile.writeSelfIntoChannel(ipcChannel);

    // Write the allocated object Info:
    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLVBO::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
bool apGLVBO::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the VBO attributes into the channel:
    gtUInt32 vboNameAsUInt32 = 0;
    ipcChannel >> vboNameAsUInt32;
    _vboName = (GLuint)vboNameAsUInt32;

    // Read the buffer target data:
    gtUInt32 lastBufferTargetAsUInt32 = 0;
    ipcChannel >> lastBufferTargetAsUInt32;
    m_lastBufferTarget = (GLenum)lastBufferTargetAsUInt32;

    gtUInt32 activeTargetCount = 0;
    ipcChannel >> activeTargetCount;
    m_activeBufferTargets.resize(activeTargetCount);

    for (size_t i = 0; i < activeTargetCount; ++i)
    {
        gtUInt32 activeTargetAsUInt32 = 0;
        ipcChannel >> activeTargetAsUInt32;
        m_activeBufferTargets[i] = (GLenum)activeTargetAsUInt32;
    }

    gtUInt32 targetHistorySize = 0;
    ipcChannel >> targetHistorySize;
    m_bufferTargetHistory.resize(targetHistorySize);

    for (size_t i = 0; i < targetHistorySize; ++i)
    {
        gtUInt32 historyTargetAsUInt32 = 0;
        ipcChannel >> historyTargetAsUInt32;
        m_bufferTargetHistory[i] = (GLenum)historyTargetAsUInt32;
    }

    gtUInt64 sizeAsUInt64 = 0;
    ipcChannel >> sizeAsUInt64;
    _size = (gtSize_t)sizeAsUInt64;

    gtInt32 bufferDisplayFormatAsInt32 = 0;
    ipcChannel >> bufferDisplayFormatAsInt32;
    _displayFormat = (oaTexelDataFormat)bufferDisplayFormatAsInt32;

    gtUInt64 offsetAsUInt64 = 0;
    ipcChannel >> offsetAsUInt64;
    _offset = (int)offsetAsUInt64;

    gtUInt64 strideAsUInt64 = 0;
    ipcChannel >> strideAsUInt64;
    _stride = (GLsizei)strideAsUInt64;

    gtInt32 int32Var = 0;
    ipcChannel >> int32Var;
    _openCLBufferIndex = (int)int32Var;

    ipcChannel >> int32Var;
    _openCLBufferName = (int)int32Var;

    ipcChannel >> int32Var;
    _openCLSpyID = (int)int32Var;

    // Read buffer file path from channel:
    _bufferFile.readSelfFromChannel(ipcChannel);

    // Read the allocated object Info:
    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLVBO::onBindToTarget
// Description: Called when a buffer object is bound to a target
// Arguments:   target - the buffer target
// Author:  AMD Developer Tools Team
// Date:        5/11/2015
// ---------------------------------------------------------------------------
void apGLVBO::onBindToTarget(GLenum target, bool* o_isRedundant)
{
    GT_IF_WITH_ASSERT(GL_NONE != target)
    {
        m_lastBufferTarget = target;

        // Check if the target is in the active list:
        bool isInActive = false;

        for (const auto& it : m_activeBufferTargets)
        {
            if (it == target)
            {
                isInActive = true;
                break;
            }
        }

        if (!isInActive)
        {
            m_activeBufferTargets.push_back(target);
        }

        if (nullptr != o_isRedundant)
        {
            // Output the value:
            *o_isRedundant = isInActive;
        }

        bool isInHistory = false;

        for (const auto& it : m_bufferTargetHistory)
        {
            if (it == target)
            {
                isInHistory = true;
                break;
            }
        }

        if (!isInHistory)
        {
            m_bufferTargetHistory.push_back(target);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLVBO::onUnbindFromTarget
// Description: Called when a buffer object is unbound from a target
// Arguments:   target - the buffer target
// Author:  AMD Developer Tools Team
// Date:        5/11/2015
// ---------------------------------------------------------------------------
void apGLVBO::onUnbindFromTarget(GLenum target)
{
    GT_IF_WITH_ASSERT(GL_NONE != target)
    {
        size_t activeTargetCount = m_activeBufferTargets.size();
        bool found = false;

        for (size_t i = 0; i < activeTargetCount; ++i)
        {
            if (found)
            {
                // This can only happen with i >= 0:
                m_activeBufferTargets[i - 1] = m_activeBufferTargets[i];
            }
            else if (m_activeBufferTargets[i] == target)
            {
                found = true;
            }
        }

        // We should not unbind from a target that didn't exist:
        GT_IF_WITH_ASSERT(found)
        {
            // Remove the last item, that's either the searched target or a copy of the second-to-last target:
            m_activeBufferTargets.pop_back();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLVBO::setSize
// Description: Set the VBO size
// Arguments: gtSize_t size
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
void apGLVBO::setSize(gtSize_t size)
{
    _size = size;
}


// ---------------------------------------------------------------------------
// Name:        apGLVBO::size
// Description: Gets the VBO size
// Return Val: gtSize_t
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
gtSize_t apGLVBO::size()const
{
    return _size;
}

// ---------------------------------------------------------------------------
// Name:        apGLVBO::setName
// Description: Sets VBO name
// Arguments: GLuint name
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
void apGLVBO::setName(GLuint name)
{
    _vboName = name;
}


// ---------------------------------------------------------------------------
// Name:        apGLVBO::name
// Description: Return the VBO name
// Return Val: GLuint
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
GLuint apGLVBO::name() const
{
    return _vboName;
}


// ---------------------------------------------------------------------------
// Name:        apGLVBO::setBufferDisplayProperties
// Description: Sets the buffer display properties
// Arguments: oaTexelDataFormat displayFormat
//            int offset
//            GLsizei stride
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        26/4/2009
// ---------------------------------------------------------------------------
void apGLVBO::setBufferDisplayProperties(oaTexelDataFormat displayFormat, int offset, GLsizei stride)
{
    _displayFormat = displayFormat;
    _offset = offset;
    _stride = stride;
}

// ---------------------------------------------------------------------------
// Name:        apGLVBO::setBufferDisplayProperties
// Description: Sets the buffer display format
// Arguments: oaTexelDataFormat displayFormat
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        10/8/2009
// ---------------------------------------------------------------------------
void apGLVBO::setBufferDisplayFormat(oaTexelDataFormat displayFormat)
{
    _displayFormat = displayFormat;
}


// ---------------------------------------------------------------------------
// Name:        apGLVBO::getBufferDisplayProperties
// Description: Get the buffer display properties
// Arguments: oaTexelDataFormat& displayFormat
//            unsigned long& offset
//            GLsizei& stride
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        26/4/2009
// ---------------------------------------------------------------------------
void apGLVBO::getBufferDisplayProperties(oaTexelDataFormat& displayFormat, int& offset, GLsizei& stride) const
{
    displayFormat = _displayFormat;
    offset = _offset;
    stride = _stride;
}

