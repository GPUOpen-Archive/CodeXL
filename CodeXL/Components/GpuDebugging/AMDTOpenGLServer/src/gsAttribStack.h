//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsAttribStack.h
///
//==================================================================================

//------------------------------ gsAttribStack.h ------------------------------

#ifndef __GSATTRIBSTACK
#define __GSATTRIBSTACK

// Infra:
#include <AMDTBaseTools/Include/gtStack.h>

// Local:
#include <src/gsAttribStackItem.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsAttribStack
// General Description:
//   A stack that imitates the OpenGL attribute stack.
//   Holds the "real" (not forced) attributes.
//   See glPupAttrib and glPopAttrib documentation for more details about the
//   OpenGL attribute stack.
// Author:               Yaki Tebeka
// Creation Date:        21/6/2005
// ----------------------------------------------------------------------------------
class gsAttribStack
{
public:
    gsAttribStack();
    const gsAttribStackItem& topItem() const { return _attribStack.top(); };
    gsAttribStackItem& topItem() { return _attribStack.top(); };

    // On event functions:
    void onFirstTimeContextMadeCurrent(const int contextOpenGLVersion[2], bool isCompatibilityContext);
    void onViewPortSet(GLint x, GLint y, GLsizei width, GLsizei height);
    void onSetDrawBuffersCall(GLenum drawnBuffers);
    void onPolygonRasterModeSet(GLenum face, GLenum rasterMode);
    void onPushAttribCalled(GLbitfield mask);
    void onPopAttribCalled();

private:
    // The maximal OpenGL attrib stack depth:
    GLint _maxStackDepth;

    // The current amount of stack items:
    // Notice that when the current stack depth is 0, we have 1 physical item in
    // the stack (the render context initial attribute set).
    GLint _currentStackDepth;

    // The stack itself:
    gtStack<gsAttribStackItem> _attribStack;
};

#endif  // __GSATTRIBSTACK
