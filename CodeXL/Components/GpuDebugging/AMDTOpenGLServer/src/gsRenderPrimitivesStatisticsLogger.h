//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsRenderPrimitivesStatisticsLogger.h
///
//==================================================================================

//------------------------------ gsRenderPrimitivesStatisticsLogger.h ------------------------------

#ifndef __GSRENDERPRIMITIVESSTATISTICSLOGGER_H
#define __GSRENDERPRIMITIVESSTATISTICSLOGGER_H

// Forward Declarations:
class gsDisplayListMonitor;
class gsRenderContextMonitor;

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apRenderPrimitivesStatistics.h>

// Local:
#include <src/gsRenderPrimitiveType.h>

// ----------------------------------------------------------------------------------
// Struct Name:   gsRenderPrimitivesStatisticsLogger
// General Description:
//   Logs draw functions primitives rendering statistics
//
// Author:               Yaki Tebeka
// Creation Date:        30/1/2006
// ----------------------------------------------------------------------------------
class gsRenderPrimitivesStatisticsLogger
{
public:
    gsRenderPrimitivesStatisticsLogger();
    virtual ~gsRenderPrimitivesStatisticsLogger();

    // Set render context monitor:
    void setRenderContextMonitor(gsRenderContextMonitor* pRenderContextMonitor) {_pRenderContextMonitor = pRenderContextMonitor;};

    apRenderPrimitivesStatistics& getCurrentStatistics() {return _renderPrimitivesStatistics;};

    void onFrameTerminatorCall();

    // Draw functions events:
    void onDrawArrays(GLint first, GLsizei count, GLenum mode);
    void onDrawElements(GLsizei count, GLenum mode, GLenum dataType);
    void onEvalCoord(int dataSize);
    void onEvalPoint(int dataSize);
    void onRect(int dataSize);
    void onVertex(int dataSize);
    void onDrawRangeElements(GLsizei count, GLenum mode, GLenum type);
    void onMultiDrawElements(const GLsizei* count, GLsizei primcount, GLenum mode, GLenum type);
    void onMultiDrawArrays(const GLsizei* count, GLsizei primcount, GLenum mode);
    void onDrawArraysInstanced(GLsizei count, GLsizei primcount, GLenum mode);
    void onDrawElementsInstanced(GLsizei count, GLenum mode, GLenum dataType, GLsizei primcount);
    void onBegin(GLenum mode);
    void onEnd();
    void onPrimitiveRestart();
    void onPrimitiveRestartIndex(GLuint primitiveRestartIndex);
    void enablePrimitiveRestart(bool enable) {_isPrimitiveRestartIndexEnabled = enable;}
    void onArrayElement(GLuint i);
    void onVertexPointer(GLint numOfCoords, GLenum arrayDataType);

    // Clear statistics:
    bool clearStatistics();

    // Display list functionality:
    void onCallDisplayList(GLuint displayListName);
    void onCallDisplayLists(GLsizei amountOfDispLists, GLenum type, const GLvoid* pListNames);

    void restartPeriodicCounters();

    // Counters get functions:
    double amountOfRenderedPrimitivesPerFrame(gsRenderPrimitiveType type) const ;

private:
    void addBatchStatistics(GLsizei count, GLenum type, GLsizei dataSize);
    void updatePrimitivesCounters(GLsizei count, GLenum mode);

private:
    // Hold the render context monitor:
    gsRenderContextMonitor* _pRenderContextMonitor;

    // Render primitives statistics object:
    apRenderPrimitivesStatistics _renderPrimitivesStatistics;

    // The primitive counters:
    gtUInt64 _fullFramesPrimitivesCounter[GS_AMOUNT_OF_PRIMITIVE_TYPES];
    gtUInt64 _currentFramePrimitivesCounter[GS_AMOUNT_OF_PRIMITIVE_TYPES];

    gtUInt64 _fullFramesCount;

    gtUInt32 _currentImmediateModeVertexCount;
    GLenum _currentImmediateMode;
    bool _isInBeginEndBlock;
    GLsizei _vertexPointerDataSize;
    GLsizei _vertexPointerElementDataSize;

    // Primitive restart index:
    GLuint _primitiveRestartIndex;
    bool _isPrimitiveRestartIndexEnabled;
};

#endif //__GSRENDERPRIMITIVESSTATISTICSLOGGER_H
