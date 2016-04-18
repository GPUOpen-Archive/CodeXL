//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsRenderPrimitivesStatisticsLogger.cpp
///
//==================================================================================

//------------------------------ gsRenderPrimitivesStatisticsLogger.cpp ------------------------------

// Standard C:
#include <string.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <src/gsRenderPrimitivesStatisticsLogger.h>
#include <src/gsDisplayListMonitor.h>
#include <src/gsRenderContextMonitor.h>

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::gsRenderPrimitivesStatisticsLogger
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
gsRenderPrimitivesStatisticsLogger::gsRenderPrimitivesStatisticsLogger():
    _pRenderContextMonitor(NULL), _fullFramesCount(0), _currentImmediateModeVertexCount(0),
    _currentImmediateMode(GL_NONE), _isInBeginEndBlock(false), _vertexPointerDataSize(0), _vertexPointerElementDataSize(0),
    _primitiveRestartIndex(0), _isPrimitiveRestartIndexEnabled(false)
{
    // Clear the statistics:
    _renderPrimitivesStatistics.clearStatistics();

    ::memset(_currentFramePrimitivesCounter, 0, sizeof(gtUInt64) * GS_AMOUNT_OF_PRIMITIVE_TYPES);
    ::memset(_fullFramesPrimitivesCounter, 0, sizeof(gtUInt64) * GS_AMOUNT_OF_PRIMITIVE_TYPES);
}


// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::~gsRenderPrimitivesStatisticsLogger
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
gsRenderPrimitivesStatisticsLogger::~gsRenderPrimitivesStatisticsLogger()
{
}

bool gsRenderPrimitivesStatisticsLogger::clearStatistics()
{
    // Clear the statistics structure:
    _renderPrimitivesStatistics.clearStatistics();
    return true;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onFrameTerminatorCall
// Description: Is called when a frame terminator function is called.
// Author:      Sigal Algranaty
// Date:        3/6/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onFrameTerminatorCall()
{
    // Accumulate primitives rendered in this frame:
    for (int i = 0; i < GS_AMOUNT_OF_PRIMITIVE_TYPES; i++)
    {
        _fullFramesPrimitivesCounter[i] += _currentFramePrimitivesCounter[i];
    }

    // Clear current primitives counters:
    ::memset(_currentFramePrimitivesCounter, 0, sizeof(gtUInt64) * GS_AMOUNT_OF_PRIMITIVE_TYPES);

    // Increase the frames count:
    _fullFramesCount++;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onDrawArrays
// Description: Is called when glDrawArrays is called
// Arguments: GLsizei count
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onDrawArrays(GLint first, GLsizei count, GLenum mode)
{
    bool primitiveRestartOn = (_isPrimitiveRestartIndexEnabled && ((GLint)_primitiveRestartIndex >= first) && ((GLint)_primitiveRestartIndex < (first + count)));

    if (primitiveRestartOn)
    {
        // Add the batch statistics for before the primitive restart:
        addBatchStatistics(_primitiveRestartIndex - first, mode, _vertexPointerDataSize);

        // Restart the primitive count:
        _currentImmediateMode = mode;
        onPrimitiveRestart();

        // Add the batch statistics for after the primitive restart:
        addBatchStatistics(count - (_primitiveRestartIndex + 1), mode, _vertexPointerDataSize);
    }
    else
    {
        // Compute the drawn arrays data size:
        addBatchStatistics(count, mode, _vertexPointerDataSize);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onDrawElements
// Description: Is called when glDrawElements is called
// Arguments: GLsizei count - the count of primitives drawn
//            GLenum mode - the type of primitives drawn
//            GLenum type - the data type
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onDrawElements(GLsizei count, GLenum mode, GLenum type)
{
    // Get the data size according to the data type:
    int dataSize = oaGLEnumToDataSize(type);

    // Add the batch statistics count:
    addBatchStatistics(count, mode, dataSize);
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onEvalCoord
// Description: Is called when glEvalCoord is called
// Return Val: void
// Arguments: int dataSize the data size
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onEvalCoord(int dataSize)
{
    // TO_DO: render primitives counter
    addBatchStatistics(1, GL_POINT, dataSize);
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onEvalPoint
// Description: Is called when glEvalPoint is called
// Return Val: void
// Arguments: int dataSize the data size
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onEvalPoint(int dataSize)
{
    addBatchStatistics(1, GL_POINT, dataSize);
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onRect
// Description: Is called when glRect is called
// Return Val: void
// Arguments: int dataSize the data size
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onRect(int dataSize)
{
    // TO_DO: render primitives counter
    addBatchStatistics(2, GL_QUADS, dataSize);
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onVertex
// Description: Is called when glVertex is called
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onVertex(int dataSize)
{
    addBatchStatistics(1, GL_NONE, dataSize);
}


// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onBegin
// Description: Event handler for immediate mode begin
// Arguments: GLenum mode
// Return Val: void
// Author:      Sigal Algranaty
// Date:        3/6/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onBegin(GLenum mode)
{
    _isInBeginEndBlock = true;
    _currentImmediateModeVertexCount = 0;
    _currentImmediateMode = mode;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onPrimitiveRestart
// Description: Event handler for immediate mode primitive restart
// Return Val: void
// Author:      Sigal Algranaty
// Date:        11/8/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onPrimitiveRestart()
{
    // Primitive restart is simply a end and then a begin calls:
    GLenum currentMode = _currentImmediateMode;
    onEnd();
    onBegin(currentMode);
}


// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onArrayElement
// Description: Handles glArrayElement call. Checks if the drawn element is
//              the primitive restart index, and restart the primitives rendering
//              count if its is.
// Arguments: GLuint i
// Return Val: void
// Author:      Sigal Algranaty
// Date:        11/8/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onArrayElement(GLuint i)
{
    bool primitiveRestartOn = (_isPrimitiveRestartIndexEnabled && (_primitiveRestartIndex == i));

    if (primitiveRestartOn)
    {
        GLenum currentMode = _currentImmediateMode;
        onEnd();
        onBegin(currentMode);
    }
    else
    {
        addBatchStatistics(1, GL_NONE, _vertexPointerElementDataSize);
    }
}
// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onPrimitiveRestartIndex
// Description: Is called when glPrimitiveRestartIndex is called. The function saves
//              the primitives restart index
// Arguments: GLuint primitiveRestartIndex
// Return Val: void
// Author:      Sigal Algranaty
// Date:        11/8/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onPrimitiveRestartIndex(GLuint primitiveRestartIndex)
{
    _primitiveRestartIndex = primitiveRestartIndex;
}
// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onEnd
// Description: Event handler for immediate mode end
// Return Val: void
// Author:      Sigal Algranaty
// Date:        3/6/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onEnd()
{
    // Only after the render context monitor is set, add the batch statistics:
    if (_pRenderContextMonitor != NULL)
    {
        // Clear the current immediate mode counter and mode:
        _isInBeginEndBlock = false;

        // Add the begin-end block rendering to counters:
        updatePrimitivesCounters(_currentImmediateModeVertexCount, _currentImmediateMode);

        _currentImmediateMode = GL_NONE;
        _currentImmediateModeVertexCount = 0;
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onDrawRangeElements
// Description: Is called when glDrawRangeElements is called
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onDrawRangeElements(GLsizei count, GLenum mode, GLenum type)
{
    // Get the data size according to the data type:
    int dataSize = oaGLEnumToDataSize(type);

    addBatchStatistics(count, mode, dataSize);
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onMultiDrawElements
// Description: Is called when glMultiDrawElements is called
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onMultiDrawElements(const GLsizei* count, GLsizei primcount, GLenum mode, GLenum type)
{
    // Summarize the counts:
    GLsizei countSum = 0;

    for (GLsizei i = 0; i < primcount; i++)
    {
        countSum += count[i];
    }

    // Get the data size according to the data type:
    int dataSize = oaGLEnumToDataSize(type);

    addBatchStatistics(countSum, mode, dataSize);

}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onMultiDrawArrays
// Description: Is called when glMultiDrawArrays is called
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onMultiDrawArrays(const GLsizei* count, GLsizei primcount, GLenum mode)
{
    // Summarize the counts:
    GLsizei countSum = 0;

    for (GLsizei i = 0; i < primcount; i++)
    {
        countSum += count[i];
    }

    addBatchStatistics(countSum, mode, _vertexPointerDataSize);

}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onDrawArrayInstance
// Description: Is called when glDrawArrayInstance is called
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onDrawArraysInstanced(GLsizei count, GLsizei primcount, GLenum mode)
{
    // Summarize the counts:
    GLsizei countSum = count * primcount;

    addBatchStatistics(countSum, mode, _vertexPointerDataSize);
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onDrawElementsInstanced
// Description: Is called when glDrawElementsInstanced is called
// Arguments: GLsizei count - the count of primitives drawn
//            GLenum mode - the type of primitives drawn
//            GLenum type - the data type
//            GLsizei primcount - the number of instances
// Author:      Uri Shomroni
// Date:        5/11/2013
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onDrawElementsInstanced(GLsizei count, GLenum mode, GLenum type, GLsizei primcount)
{
    // Get the data size according to the data type:
    int dataSize = oaGLEnumToDataSize(type);

    // Add the batch statistics count:
    addBatchStatistics(count * primcount, mode, dataSize);
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::addBatchStatistics
// Description: Add batch statistics both to display list and statistics
// Arguments: GLsizei count - amount of vertices
//            GLenum mode - primitive mode
//            unsigned int dataSize
// Author:      Sigal Algranaty
// Date:        27/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::addBatchStatistics(GLsizei count, GLenum mode, GLsizei dataSize)
{
    bool addToStatistics = true;

    // Only after the render context monitor is set, add the batch statistics:
    if (_pRenderContextMonitor != NULL)
    {
        // Get the display list monitor:
        gsDisplayListMonitor* pDisplayListMonitor = _pRenderContextMonitor->displayListsMonitor();
        GT_IF_WITH_ASSERT(pDisplayListMonitor != NULL)
        {
            // When only compiling the list, do not add to counter:
            addToStatistics = pDisplayListMonitor->isGeometryDrawnOutsideCurrentList();

            // Get the details of the active display list:
            apGLDisplayList* pDisplayList = pDisplayListMonitor->getActiveDisplayListDetails();

            if (pDisplayList != NULL)
            {
                // Add the geometry to the display list size:
                pDisplayList->addGeometrySize(dataSize * count, count);
            }
        }

        if (addToStatistics)
        {
            // Update the batch statistics:
            _renderPrimitivesStatistics.addBatchStatistics(count);
        }

        // Update the primitives counters and the per-primitive type display list geometry values.
        // Note that we call this even if addToStatistics == false, since we need to upadte the display list.
        updatePrimitivesCounters(count, mode);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::updatePrimitivesCounters
// Description: Add the draw call to the primitives counters by mode
// Arguments: GLsizei count
//            GLenum mode
// Return Val: void
// Author:      Sigal Algranaty
// Date:        3/6/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::updatePrimitivesCounters(GLsizei count, GLenum mode)
{
    if (_isInBeginEndBlock)
    {
        // Only count the vertices (is used for immediate mode):
        _currentImmediateModeVertexCount += count;
    }
    else
    {
        // These variables will hold the type of primitives and their amount:
        gsRenderPrimitiveType primitiveCounterType = GS_VERTICES;
        int amountOfPrimitivesToAdd = 0;

        // Add the primitive count to the relevant counter (by mode):
        switch (mode)
        {
            case GL_POINTS:
            {
                primitiveCounterType = GS_POINTS;
                amountOfPrimitivesToAdd = count;
            }
            break;

            case GL_LINE_LOOP:
            case GL_LINE_STRIP:
            {
                if (count < 2)
                {
                    amountOfPrimitivesToAdd = 0;
                }
                else if (count == 2)
                {
                    amountOfPrimitivesToAdd = 1;
                }
                else if (mode == GL_LINE_STRIP)
                {
                    amountOfPrimitivesToAdd = count - 1;
                }
                else if (mode == GL_LINE_LOOP)
                {
                    amountOfPrimitivesToAdd = count;
                }

                primitiveCounterType = GS_LINES;
            }
            break;

            case GL_LINES_ADJACENCY:
            {
                if (count < 4)
                {
                    amountOfPrimitivesToAdd = 0;
                }
                else if (count == 4)
                {
                    amountOfPrimitivesToAdd = 1;
                }

                amountOfPrimitivesToAdd = count / 4;
                primitiveCounterType = GS_LINES;
            }
            break;

            case GL_LINE_STRIP_ADJACENCY:
            {
                if (count < 4)
                {
                    amountOfPrimitivesToAdd = 0;
                }
                else if (count == 4)
                {
                    amountOfPrimitivesToAdd = 1;
                }
                else
                {
                    amountOfPrimitivesToAdd = count - 3;
                }

                primitiveCounterType = GS_LINES;
            }
            break;

            case GL_LINES:
            {
                primitiveCounterType = GS_LINES;
                amountOfPrimitivesToAdd = count / 2;
            }
            break;

            case GL_TRIANGLE_STRIP:
            case GL_TRIANGLE_FAN:
            case GL_POLYGON:
            {
                if (count < 3)
                {
                    amountOfPrimitivesToAdd = 0;
                }
                else
                {
                    amountOfPrimitivesToAdd = count - 2;
                }

                primitiveCounterType = GS_TRIANGLES;
            }
            break;

            case GL_TRIANGLES_ADJACENCY:
            {
                amountOfPrimitivesToAdd = count / 6;
                primitiveCounterType = GS_TRIANGLES;
            }
            break;

            case GL_TRIANGLE_STRIP_ADJACENCY:
            {
                if (count < 6)
                {
                    amountOfPrimitivesToAdd = 0;
                }
                else
                {
                    amountOfPrimitivesToAdd = (count - 4) / 2;
                }

                primitiveCounterType = GS_TRIANGLES;
            }
            break;

            case GL_TRIANGLES:
            {
                amountOfPrimitivesToAdd = count / 3;
                primitiveCounterType = GS_TRIANGLES;
            }
            break;

            case GL_QUAD_STRIP:
            {
                if (count < 4)
                {
                    amountOfPrimitivesToAdd = 0;
                }
                else
                {
                    amountOfPrimitivesToAdd = (count / 2) - 1;
                }

                // Each quad is actually two triangles:
                amountOfPrimitivesToAdd *= 2;
                primitiveCounterType = GS_TRIANGLES;
            }
            break;

            case GL_QUADS:
            {
                amountOfPrimitivesToAdd = count / 4;
                // Each quad is actually two triangles:
                amountOfPrimitivesToAdd *= 2;
                primitiveCounterType = GS_TRIANGLES;
            }
            break;

            default:
                GT_ASSERT(false);
                break;
        }

        gsDisplayListMonitor* pDisplayListMonitor = _pRenderContextMonitor->displayListsMonitor();
        GT_IF_WITH_ASSERT(pDisplayListMonitor != NULL)
        {
            // Update the display list monitor with the geometry count for the current display list:
            pDisplayListMonitor->updateActiveDisplayListGeometryMapping(primitiveCounterType, count, amountOfPrimitivesToAdd);

            // If the call also draws to the screen (and not only to the GL_COMPILE display list):
            if (pDisplayListMonitor->isGeometryDrawnOutsideCurrentList())
            {
                // If we are just compiling a list, we are not drawing, so only add the counter if we are executing:
                _currentFramePrimitivesCounter[primitiveCounterType] += amountOfPrimitivesToAdd;

                // Add the vertex count to the counter:
                _currentFramePrimitivesCounter[GS_VERTICES] += count;
            }
        }
        else
        {
            // We did not find a display list monitor, better to just log the primitives:
            _currentFramePrimitivesCounter[primitiveCounterType] += amountOfPrimitivesToAdd;
            _currentFramePrimitivesCounter[GS_VERTICES] += count;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onCallDisplayList
// Description: Is called when a display is called
// Arguments: GLuint displayListName
// Return Val: void
// Author:      Sigal Algranaty
// Date:        27/5/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onCallDisplayList(GLuint displayListName)
{
    // Get the display monitor:
    const gsDisplayListMonitor* pDisplayListMonitor = _pRenderContextMonitor->displayListsMonitor();
    GT_IF_WITH_ASSERT(pDisplayListMonitor != NULL)
    {
        // If there is an active display list, do not add this batch (it is calculated within the vertices
        // of the currently active display list:
        if (pDisplayListMonitor->isGeometryDrawnOutsideCurrentList())
        {
            // Check if the current display list mapped:
            const int* pAmountOfPrimitives = pDisplayListMonitor->getDisplayListTotalGeometryMapping(displayListName);

            if (pAmountOfPrimitives != NULL)
            {
                // Add the vertices amount to the statistics object:
                _renderPrimitivesStatistics.addBatchStatistics(pAmountOfPrimitives[GS_VERTICES]);

                // Add the vertices to the counter:
                for (int i = 0; i < GS_AMOUNT_OF_PRIMITIVE_TYPES; i++)
                {
                    _currentFramePrimitivesCounter[i] += pAmountOfPrimitives[i];
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onCallDisplayLists
// Description: Event handler for glCallLists OpenGL function
// Arguments: GLsizei amountOfDispLists
//            GLenum type
//            const GLvoid* pListNames
// Return Val: void
// Author:      Sigal Algranaty
// Date:        7/7/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onCallDisplayLists(GLsizei amountOfDispLists, GLenum type, const GLvoid* pListNames)
{
    switch (type)
    {
        case GL_BYTE:
        {
            const GLbyte* pByteListNames = (GLbyte*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                GLbyte displayListName = pByteListNames[i];
                onCallDisplayList((GLuint)displayListName);
            }

            break;
        }

        case GL_UNSIGNED_BYTE:
        {
            GLubyte* pUbyteListNames = (GLubyte*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                GLubyte displayListName = pUbyteListNames[i];
                onCallDisplayList((GLuint)displayListName);
            }
        }
        break;

        case GL_SHORT:
        {
            GLshort* pShortListNames = (GLshort*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                GLshort displayListName = pShortListNames[i];
                onCallDisplayList((GLuint)displayListName);
            }
        }
        break;

        case GL_UNSIGNED_SHORT:
        {
            GLushort* pUshortListNames = (GLushort*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                GLushort displayListName = pUshortListNames[i];
                onCallDisplayList((GLuint)displayListName);
            }
        }
        break;

        case GL_INT:
        {
            GLint* pIntListNames = (GLint*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                GLint displayListName = pIntListNames[i];
                onCallDisplayList((GLuint)displayListName);
            }
        }
        break;

        case GL_UNSIGNED_INT:
        {
            GLuint* pUintListNames = (GLuint*)pListNames;

            for (int i = 0; i < amountOfDispLists; i++)
            {
                GLuint displayListName = pUintListNames[i];
                onCallDisplayList((GLuint)displayListName);
            }
        }
        break;

        default:
            GT_ASSERT_EX(false, L"Cannot recognize list type");
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::amountOfRenderedPrimitivesPerFrame
// Description: Return the amount of current rendered vertices per frame
// Return Val: double
// Author:      Sigal Algranaty
// Date:        3/6/2009
// ---------------------------------------------------------------------------
double gsRenderPrimitivesStatisticsLogger::amountOfRenderedPrimitivesPerFrame(gsRenderPrimitiveType primitiveType) const
{
    double retVal = 0.0;

    // Get amount of full frames:
    GT_IF_WITH_ASSERT(_pRenderContextMonitor != NULL)
    {
        if (_fullFramesCount)
        {
            if (primitiveType == GS_PRIMITIVES)
            {
                // Summarize each of the primitives:
                retVal += _fullFramesPrimitivesCounter[GS_POINTS];
                retVal += _fullFramesPrimitivesCounter[GS_LINES];
                retVal += _fullFramesPrimitivesCounter[GS_TRIANGLES];
                retVal /= _fullFramesCount;
            }
            else
            {
                // Get the specific primitive counter:
                retVal = (double)_fullFramesPrimitivesCounter[primitiveType] / (double)_fullFramesCount;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::onVertexPointer
// Description: Sets the current vertex array data size
// Arguments: GLint numOfCoords
//            GLenum arrayDataType
// Return Val: void
// Author:      Sigal Algranaty
// Date:        8/6/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::onVertexPointer(GLint numOfCoords, GLenum arrayDataType)
{
    // Get data size for single coordinate:
    int dataTypeSize = oaGLEnumToDataSize(arrayDataType);

    // Compute array size:
    _vertexPointerDataSize = dataTypeSize * numOfCoords;

    // Save array data size (for further use of array elements with glArrayElement):
    _vertexPointerElementDataSize = dataTypeSize;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderPrimitivesStatisticsLogger::restartPeriodicCounters
// Description: Restarts the primitive counters
// Return Val: void
// Author:      Sigal Algranaty
// Date:        26/6/2009
// ---------------------------------------------------------------------------
void gsRenderPrimitivesStatisticsLogger::restartPeriodicCounters()
{
    // Restart the primitives rendering count:
    ::memset(_fullFramesPrimitivesCounter, 0, sizeof(gtUInt64) * GS_AMOUNT_OF_PRIMITIVE_TYPES);

    _fullFramesCount = 0;
}

