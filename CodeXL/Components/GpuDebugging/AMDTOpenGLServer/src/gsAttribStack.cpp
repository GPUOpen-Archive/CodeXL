//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsAttribStack.cpp
///
//==================================================================================

//------------------------------ gsAttribStack.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsAttribStack.h>


// ---------------------------------------------------------------------------
// Name:        gsAttribStack::gsAttribStack
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
gsAttribStack::gsAttribStack()
    : _maxStackDepth(0), _currentStackDepth(0)
{
    // Initialize the stack with 1 item - the render context initial attribute set:
    gsAttribStackItem firstItem;
    _attribStack.push(firstItem);
}


// ---------------------------------------------------------------------------
// Name:        gsAttribStack::onFirstTimeContextMadeCurrent
// Description: Is called on the first time in which the render context that
//              its attrib stack we represent is made the current render context.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
void gsAttribStack::onFirstTimeContextMadeCurrent(bool attribStackSupported)
{
    // Get the maximal OpenGL attrib stack depth:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

    // Check the OpenGL version to see if the GL_MAX_ATTRIB_STACK_DEPTH parameter exists:
    if (attribStackSupported)
    {
        gs_stat_realFunctionPointers.glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, &_maxStackDepth);
    }
    else
    {
        // OpenGL 3.1 and higher do not support the attribute stack:
        _maxStackDepth = 0;
    }

    // Get the context context initial view port:
    // (The initial GL_VIEWPORT dimensions are set when an OpenGL context is
    //  first attached to a window to contain the windows width and height)
    gsAttribStackItem& stackTopItem = _attribStack.top();
    GLint topItemViewPort[4] = {0, 0, 0, 0};

    // On the iPhone, there are no static buffers, and an EAGL context is created with
    // no viewport size. Asking for GL_VIEWPORT with no bound framebuffer yield GL_INVALID_ENUM.
    // So, we just set the values to 0 as a default:
#ifndef _GR_IPHONE_BUILD
    gs_stat_realFunctionPointers.glGetIntegerv(GL_VIEWPORT, topItemViewPort);
#endif // ndef _GR_IPHONE_BUILD
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE))
    // Uri, 17/9/09: Mac 64-bit has a problem where glGetIntergerv takes a GLint (= long = 8 bytes) pointer as a parameter, but the output is
    // as int (= 4 bytes). This causes "x" to contain x + (y << 32), "y" to contain width + (height << 32), and "width" and "height" to contain
    // garbage data. So - we cast the pointer to an int* to overcome this:
    int* topItemViewPortAsIntPointer = (int*)topItemViewPort;
    stackTopItem._viewPort[0] = topItemViewPortAsIntPointer[0];
    stackTopItem._viewPort[1] = topItemViewPortAsIntPointer[1];
    stackTopItem._viewPort[2] = topItemViewPortAsIntPointer[2];
    stackTopItem._viewPort[3] = topItemViewPortAsIntPointer[3];
#else
    stackTopItem._viewPort[0] = topItemViewPort[0];
    stackTopItem._viewPort[1] = topItemViewPort[1];
    stackTopItem._viewPort[2] = topItemViewPort[2];
    stackTopItem._viewPort[3] = topItemViewPort[3];
#endif
}


// ---------------------------------------------------------------------------
// Name:        gsAttribStack::onViewPortSet
// Description: Is called when the view port is set.
// Arguments:   x, y, width, height - The new view port dimensions.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
void gsAttribStack::onViewPortSet(GLint x, GLint y, GLsizei width, GLsizei height)
{
    gsAttribStackItem& stackTopItem = _attribStack.top();
    stackTopItem._viewPort[0] = x;
    stackTopItem._viewPort[1] = y;
    stackTopItem._viewPort[2] = width;
    stackTopItem._viewPort[3] = height;
}


// ---------------------------------------------------------------------------
// Name:        gsAttribStack::onSetDrawBuffersCall
// Description: Is called when the drawn buffers are changed.
// Arguments:   drawnBuffers - The new drawn buffers.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
void gsAttribStack::onSetDrawBuffersCall(GLenum drawnBuffers)
{
    gsAttribStackItem& stackTopItem = _attribStack.top();
    stackTopItem._drawnBuffers = drawnBuffers;
}


// ---------------------------------------------------------------------------
// Name:        gsAttribStack::onPolygonRasterModeSet
// Description: Is called when the polygon raster mode is set.
// Arguments:   face - The affected face / faces.
//              rasterMode - The new raster mode for that faces.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
void gsAttribStack::onPolygonRasterModeSet(GLenum face, GLenum rasterMode)
{
    gsAttribStackItem& stackTopItem = _attribStack.top();

    // Store the "real" raster mode of the front and back faces:
    if ((face == GL_FRONT) || (face == GL_FRONT_AND_BACK))
    {
        stackTopItem._rasterModeFrontFace = rasterMode;
    }

    if ((face == GL_BACK) || (face == GL_FRONT_AND_BACK))
    {
        stackTopItem._rasterModeBackFace = rasterMode;
    }
}


// ---------------------------------------------------------------------------
// Name:        gsAttribStack::onPushAttribCalled
// Description: Is called when the attrib stack is pushed.
// Arguments:   mask - A mask that indicates which attributes were saved into
//                     the new stack item.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
void gsAttribStack::onPushAttribCalled(GLbitfield mask)
{
    (void)(mask); // unused
    bool rc = false;

    // If we are not exceeding the maximal OpenGL stack depth:
    if (_currentStackDepth < _maxStackDepth)
    {
        // Copy the current top item:
        gsAttribStackItem stackTopItemCopy = _attribStack.top();

        // Push the copy into the stack:
        _attribStack.push(stackTopItemCopy);
        _currentStackDepth++;
        rc = true;
    }

    // TO_DO: Raise a detected error by the calling method !
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        gsAttribStack::onPopAttribCalled
// Description: Is called when the attrib stack is popped.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
void gsAttribStack::onPopAttribCalled()
{
    bool rc = false;

    // If the stack is not empty:
    if (_currentStackDepth > 0)
    {
        _attribStack.pop();
        _currentStackDepth--;
        rc = true;
    }

    // TO_DO: Raise a detected error by the calling method !
    GT_ASSERT(rc);
}
