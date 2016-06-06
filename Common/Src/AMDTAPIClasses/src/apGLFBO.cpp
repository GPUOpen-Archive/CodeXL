//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLFBO.cpp
///
//==================================================================================

// -----------------------------   apGLFBO.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apGLFBO.h>

// ---------------------------------------------------------------------------
// Name:        apFBOBindObject::type
// Description: Returns apFBOBindObject type
// Return Val: osTransferableObjectType
// Author:  AMD Developer Tools Team
// Date:        4/6/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apFBOBindObject::type() const
{
    return OS_TOBJ_ID_GL_BIND_OBJECT;
}


// ---------------------------------------------------------------------------
// Name:        apFBOBindObject::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/6/2008
// ---------------------------------------------------------------------------
bool apFBOBindObject::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the FBO attributes into the channel:
    ipcChannel << (gtInt32)_attachmentPoint;
    ipcChannel << (gtInt32)_attachmentTarget;
    ipcChannel << (gtUInt32)_name;
    ipcChannel << (gtUInt32)_textureLayer;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apFBOBindObject::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/6/2008
// ---------------------------------------------------------------------------
bool apFBOBindObject::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the FBO attributes into the channel:
    gtInt32 attachmentPointAsInt32 = 0;
    ipcChannel >> attachmentPointAsInt32;
    _attachmentPoint = (GLenum)attachmentPointAsInt32;

    gtInt32 attachmentTargetAsInt32 = 0;
    ipcChannel >> attachmentTargetAsInt32;
    _attachmentTarget = (GLenum)attachmentTargetAsInt32;

    gtUInt32 nameAsUInt32 = 0;
    ipcChannel >> nameAsUInt32;
    _name = (GLuint)nameAsUInt32;

    gtUInt32 texLayerAsUInt32 = 0;
    ipcChannel >> texLayerAsUInt32;
    _textureLayer = (GLuint)texLayerAsUInt32;

    return retVal;
}

apFBOBindObject::apFBOBindObject():
    _attachmentPoint(0), _attachmentTarget(0), _name(0), _textureLayer(0)
{
}
apFBOBindObject::~apFBOBindObject()
{

}

// ---------------------------------------------------------------------------
// Name:        apGLFBO::apGLFBO
// Description: Constructor
// Arguments:   fboName - The OpenGL frame buffer object name.
// Author:  AMD Developer Tools Team
// Date:        25/5/2008
// ---------------------------------------------------------------------------
apGLFBO::apGLFBO(GLuint fboName)
    : apAllocatedObject(), _fboName(fboName)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLFBO::apGLFBO
// Description: Copy constructor
// Arguments: other - The other FBO class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        2/7/2006
// ---------------------------------------------------------------------------
apGLFBO::apGLFBO(const apGLFBO& other)
{
    apGLFBO::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apGLFBO::~apGLFBO
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        25/5/2008
// ---------------------------------------------------------------------------
apGLFBO::~apGLFBO()
{
    // Delete all binded objects:
    gtList<apFBOBindObject*>::iterator iter;
    gtList<apFBOBindObject*>::iterator iterEnd = _bindedObjects.end();

    for (iter = _bindedObjects.begin(); iter != iterEnd; iter++)
    {
        apFBOBindObject* pCurrent = *iter;

        if (pCurrent != NULL)
        {
            delete pCurrent;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLFBO::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        25/5/2008
// ---------------------------------------------------------------------------
apGLFBO& apGLFBO::operator=(const apGLFBO& other)
{
    _fboName = other._fboName;
    gtList<apFBOBindObject*>::const_iterator iter;
    gtList<apFBOBindObject*>::const_iterator iterEnd = other._bindedObjects.end();

    // Iterate and copy the other binded objects:
    for (iter = other._bindedObjects.begin(); iter != iterEnd; iter++)
    {
        const apFBOBindObject* pCurrent = *iter;

        if (pCurrent != NULL)
        {
            apFBOBindObject* newFBOBindedObject = new apFBOBindObject(*pCurrent);
            _bindedObjects.push_back(newFBOBindedObject);
        }
    }

    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apGLFBO::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        25/5/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apGLFBO::type() const
{
    return OS_TOBJ_ID_GL_FBO;
}

// ---------------------------------------------------------------------------
// Name:        apGLFBO::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/5/2008
// ---------------------------------------------------------------------------
bool apGLFBO::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;


    // Write the FBO attributes into the channel:
    ipcChannel << (gtUInt32)_fboName;

    // Write the binded objects amount:
    ipcChannel << (gtUInt64)_bindedObjects.size();

    // Write all binded objects:
    gtList<apFBOBindObject*>::const_iterator iter;
    gtList<apFBOBindObject*>::const_iterator iterEnd = _bindedObjects.end();

    for (iter = _bindedObjects.begin(); iter != iterEnd; iter++)
    {
        apFBOBindObject* pCurrent = *iter;
        pCurrent->writeSelfIntoChannel(ipcChannel);
    }

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLFBO::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/5/2008
// ---------------------------------------------------------------------------
bool apGLFBO::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the FBO attributes into the channel:
    gtUInt32 fboNameAsInt32 = 0;
    ipcChannel >> fboNameAsInt32;
    _fboName = (GLuint)fboNameAsInt32;

    gtUInt64 amoutOfBoundObjects = 0;
    // Read the bound objects amount:
    ipcChannel >> amoutOfBoundObjects;

    for (unsigned int i = 0; i < amoutOfBoundObjects; i++)
    {
        // Allocate a binded object:
        apFBOBindObject* pCurrent = new apFBOBindObject();


        // Read the binded object:
        bool rc = pCurrent->readSelfFromChannel(ipcChannel);
        GT_ASSERT(rc);

        // Add the binded object to the list:
        _bindedObjects.push_back(pCurrent);
    }

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLFBO::bindObject
// Description: Add a new object that is binded to the FBO
// Arguments:   GLenum attachmentPoint - color / depth / etc
//            GLenum attachmentTarget - the binded object type - texture/render buffer / etc'
//            GLuint objectName - OpenGL object name
//            GLenum textureFace - the attached texture face
//            GLuint textureLayer - the attached texture layer
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/5/2008
// ---------------------------------------------------------------------------
bool apGLFBO::bindObject(GLenum attachmentPoint, GLenum attachmentTarget, GLuint objectName, GLuint textureLayer)
{
    bool retVal = false;

    // Search within the binded objects, if there is a binded object with the
    // same attachment point, use it:
    gtList<apFBOBindObject*>::iterator iter;
    gtList<apFBOBindObject*>::iterator iterEnd = _bindedObjects.end();

    for (iter = _bindedObjects.begin(); iter != iterEnd; iter++)
    {
        apFBOBindObject* pCurrent = *iter;
        GT_IF_WITH_ASSERT(pCurrent != NULL)
        {
            if (pCurrent->_attachmentPoint == attachmentPoint)
            {
                pCurrent->_name = objectName;
                pCurrent->_attachmentTarget = attachmentTarget;
                pCurrent->_textureLayer = textureLayer;
                retVal = true;
                break;
            }
        }
    }

    // In case of objectName 0, we should actually detach the attachment target:
    if (objectName == 0 && retVal)
    {
        apFBOBindObject* pCurrent = *iter;
        // Remove the object with the same attachment point:
        _bindedObjects.remove(*iter);

        // Delete the bind object:
        delete pCurrent;

    }

    if (!retVal && objectName != 0)
    {
        // Object with the same attachment point doesn't exist, so create a new binded object:
        // Construct a new bind object, and set its parameters according to the binded object:
        apFBOBindObject* pBindNewObject = new apFBOBindObject();


        // Set parameters
        pBindNewObject->_attachmentPoint = attachmentPoint;
        pBindNewObject->_attachmentTarget = attachmentTarget;
        pBindNewObject->_name = objectName;
        pBindNewObject->_textureLayer = textureLayer;

        // Add the new binded object to the list of binded objects:
        _bindedObjects.push_back(pBindNewObject);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLFBO::getCurrentlyBoundObject
// Description: Goes through the currently bounded objects and returns the one
//              object with the same attachment point, and returns its details
// Arguments: apFBOBindObject& boundObject
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/6/2008
// ---------------------------------------------------------------------------
bool apGLFBO::getCurrentlyBoundObject(apFBOBindObject& boundObject) const
{
    bool retVal = false;

    // Search within the binded objects, if there is a binded object with the
    // same attachment point, use it:
    gtList<apFBOBindObject*>::const_iterator iter;
    gtList<apFBOBindObject*>::const_iterator iterEnd = _bindedObjects.end();

    for (iter = _bindedObjects.begin(); iter != iterEnd; iter++)
    {
        apFBOBindObject* pCurrent = *iter;
        GT_IF_WITH_ASSERT(pCurrent != NULL)
        {
            if (pCurrent->_attachmentPoint == boundObject._attachmentPoint)
            {
                boundObject._name = pCurrent->_name ;
                boundObject._attachmentTarget = pCurrent->_attachmentTarget;
                boundObject._textureLayer = pCurrent->_textureLayer;
                retVal = true;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLFBO::unbindAllObjects
// Description: Removes all the texture name from the list of the FBOs binded textures
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/5/2008
// ---------------------------------------------------------------------------
void apGLFBO::unbindAllObjects()
{
    // Go through the binded objects, and delete them all:
    gtList<apFBOBindObject*>::iterator iter;
    gtList<apFBOBindObject*>::iterator iterEnd = _bindedObjects.end();

    for (iter = _bindedObjects.begin(); iter != iterEnd; iter++)
    {
        apFBOBindObject* pCurrent = *iter;

        if (pCurrent != NULL)
        {
            delete pCurrent;
        }
    }

    _bindedObjects.clear();
}


// ---------------------------------------------------------------------------
// Name:        apGLFBO::fboAttachmentPointToString
// Description: Translates an FBO OpenGL attachment target enum, to a string
// Arguments: GLenum attachmentTarget
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/6/2008
// ---------------------------------------------------------------------------
bool apGLFBO::fboAttachmentTargetToString(GLenum attachmentTarget, gtString& targetTypeString)
{
    bool retVal = false;

    switch (attachmentTarget)
    {
        case GL_RENDERBUFFER_EXT:
        {
            targetTypeString = L"Render Buffer";
            retVal = true;
            break;
        }

        case GL_TEXTURE_1D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_RECTANGLE:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        {
            targetTypeString = L"Texture";
            retVal = true;
            break;
        }

        default:
            GT_ASSERT_EX(false, L"Unknown FBO attachment point");
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLFBO::isTextureAttachmentTarget
// Description: Check if an FBO attachment target is a texture attachment
// Arguments: GLenum attachmentTarget
//            bool& isTextureAttachment
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/8/2009
// ---------------------------------------------------------------------------
bool apGLFBO::isTextureAttachmentTarget(GLenum attachmentTarget, bool& isTextureAttachment)
{
    bool retVal = true;

    if ((attachmentTarget == GL_TEXTURE_1D) || (attachmentTarget == GL_TEXTURE_2D) || (attachmentTarget == GL_TEXTURE_3D) || (attachmentTarget == GL_TEXTURE_RECTANGLE) ||
        (attachmentTarget == GL_TEXTURE_1D_ARRAY) || (attachmentTarget == GL_TEXTURE_2D_ARRAY) ||
        (attachmentTarget == GL_TEXTURE_2D_MULTISAMPLE) || (attachmentTarget == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) ||
        (attachmentTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_X) || (attachmentTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_X) || (attachmentTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_Y) ||
        (attachmentTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y) || (attachmentTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_Z) || (attachmentTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z))
    {
        isTextureAttachment = true;
    }
    else if (attachmentTarget == GL_RENDERBUFFER_EXT)
    {
        isTextureAttachment = false;
    }
    else
    {
        GT_ASSERT_EX(false, L"Unknown FBO attachment type");
        retVal = false;
    }

    return retVal;

}
