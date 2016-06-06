//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLFBO.h
///
//==================================================================================

//------------------------------ apGLFBO.h ------------------------------

#ifndef __APGLFBO
#define __APGLFBO

// OpenGL
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           apFBOBindObject
// General Description:  This class represents an object that can be attached to an FBO
//                       It contains the point where this object is attached to the FBO,
//                       the type of target connects, and the OpenGL name of the object.
// Author:  AMD Developer Tools Team
// Creation Date:        27/5/2008
// ----------------------------------------------------------------------------------
class AP_API apFBOBindObject : public osTransferableObject
{
public:
    GLenum _attachmentPoint; // - color / depth / etc
    GLenum _attachmentTarget; // - 2D / 3D / render buffer / etc
    GLuint _name;           // Attached object name
    GLuint _textureLayer;   // Attached texture layer

    apFBOBindObject();
    ~apFBOBindObject();
    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

};
// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLFBO : public osTransferableObject
//
// General Description:
//   Represents an OpenGL Frame buffer object.
//   See GL_EXT_framebuffer_object extension documentation for more details.
//
// Author:  AMD Developer Tools Team
// Creation Date:        25/5/2008
// ----------------------------------------------------------------------------------
class AP_API apGLFBO : public apAllocatedObject
{
public:
    // Self functions:
    apGLFBO(GLuint fboName = 0);
    apGLFBO(const apGLFBO& other);
    virtual ~apGLFBO();
    apGLFBO& operator=(const apGLFBO& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Static translation from FBO attachment to string:
    static bool fboAttachmentTargetToString(GLenum attachmentTarget, gtString& targetTypeString);
    static bool isTextureAttachmentTarget(GLenum attachmentTarget, bool& isTextureAttachment);

public:
    // Operations on FBOs:
    bool bindObject(GLenum attachmentPoint, GLenum attachmentTarget, GLuint objectName, GLuint textureLayer);
    void unbindAllObjects();

    // FBO data:
    int amountOfBindedObjects() const {return (int)_bindedObjects.size();};
    GLuint getFBOName() const { return _fboName; };
    gtList<apFBOBindObject*> getBindedObjects() const {return _bindedObjects;};
    void getBindedRenderBuffers(gtList<GLuint>& bindRenderBuffers) const;
    bool getCurrentlyBoundObject(apFBOBindObject& boundObject) const;

private:
    // The OpenGL frame buffer object name:
    GLuint _fboName;

    // The list of this FBO binded textures:
    gtList<apFBOBindObject*> _bindedObjects;
};


#endif  // __APGLFBO
