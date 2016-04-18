//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdRenderBufferImageProxy.h
///
//==================================================================================

//------------------------------ gdRenderBufferImageProxy.h ------------------------------

#ifndef _GDRENDERBUFFERIMAGEPROXY
#define _GDRENDERBUFFERIMAGEPROXY

// Forward decelerations:
struct FIBITMAP;

// Infra:
#include <AMDTApplicationComponents/Include/acImageDataProxy.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdRenderBufferImageProxy
// General Description:  An image proxy to retrieve a render buffer's content
// Author:               Eran Zinman
// Creation Date:        25/1/2008
// ----------------------------------------------------------------------------------
class gdRenderBufferImageProxy : public acImageDataProxy
{
public:
    // Constructor:
    gdRenderBufferImageProxy(int renderBufferID, const apGLRenderBuffer& bufferDetails, int renderBufferContext, bool isInGLBeginEndBlock) :
        _renderBufferID(renderBufferID), _renderBufferDetails(bufferDetails),
        _renderBufferContext(renderBufferContext), _isInGLBeginEndBlock(isInGLBeginEndBlock) {};

    // Destructor
    virtual ~gdRenderBufferImageProxy() {};

public:
    // Image generation function
    virtual bool loadImage();

    // Overrides acImageDataProxy:
    virtual void buildTooltipText();

private:
    // Generates the buffer image.
    bool generateRenderBufferImage();

private:
    // The renderBuffer ID
    int _renderBufferID;

    // The static buffer details
    apGLRenderBuffer _renderBufferDetails;

    // render buffer attached render context spy id
    int _renderBufferContext;

    // Are we in GL begin end block:
    bool _isInGLBeginEndBlock;
};



#endif  // __GDRENDERBUFFERIMAGEPROXY
