//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsFBOMonitor.h
///
//==================================================================================

//------------------------------ gsFBOMonitor.h ------------------------------

#ifndef __GSFBOMONITOR
#define __GSFBOMONITOR

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTAPIClasses/Include/apGLFBO.h>

// ----------------------------------------------------------------------------------
// Class Name:           gsFBOMonitor
//
// General Description:
//   Monitors Frame Buffer Objects allocated in a given context.
//   See GL_EXT_framebuffer_object extension documentation for more details.
//
// Author:               Sigal Algranaty
// Creation Date:        25/5/2008
// ----------------------------------------------------------------------------------
class gsFBOMonitor
{
public:
    gsFBOMonitor();
    ~gsFBOMonitor();

public:
    // FBO actions:
    bool removeFbo(GLuint fboName);
    bool onGenFramebuffers(GLuint* fboNames, GLsizei count);

    // FBO data:
    apGLFBO* getFBODetails(GLuint fboName) const ;
    bool getFBOName(int fboIndex, GLuint& fboName) const;
    int amountOfFBOs() const {return _amountOfFBOs;};

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsFBOMonitor& operator=(const gsFBOMonitor& otherMonitor);
    gsFBOMonitor(const gsFBOMonitor& otherMonitor);

private:
    // Holds the amount of allocated frame buffer objects:
    int _amountOfFBOs;

    // Hold the parameters of FBO objects that reside in this render context:
    gtPtrVector<apGLFBO*> _fbos;
};


#endif  // __gsFBOMonitor
