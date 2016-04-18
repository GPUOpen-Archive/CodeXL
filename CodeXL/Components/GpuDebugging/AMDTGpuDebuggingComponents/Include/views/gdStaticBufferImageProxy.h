//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStaticBufferImageProxy.h
///
//==================================================================================

//------------------------------ gdStaticBufferImageProxy.h ------------------------------

#ifndef __GDSTATICBUFFERIMAGEPROXY
#define __GDSTATICBUFFERIMAGEPROXY

// Forward decelerations:
struct FIBITMAP;

// Infra:
#include <AMDTAPIClasses/Include/apStaticBuffer.h>
#include <AMDTApplicationComponents/Include/acImageDataProxy.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdStaticBufferImageProxy
// General Description:  An image proxy to retrieve a static buffer
// Author:               Eran Zinman
// Creation Date:        31/12/2007
// ----------------------------------------------------------------------------------
class gdStaticBufferImageProxy : public acImageDataProxy
{
public:
    // Constructor:
    gdStaticBufferImageProxy(const apStaticBuffer& bufferDetails, int activeContext, bool isInGLBeginEndBlock, bool isFBOBound) :
        _bufferDetails(bufferDetails), _activeContext(activeContext),
        _isInGLBeginEndBlock(isInGLBeginEndBlock), _isFBOBound(isFBOBound) {};

    // Destructor
    virtual ~gdStaticBufferImageProxy() {};

public:
    // Image generation function
    virtual bool loadImage();

    // Overrides acImageDataProxy:
    virtual void buildTooltipText();

private:
    // Generates the buffer image.
    bool generateBufferImage();

private:
    // The static buffer details
    apStaticBuffer _bufferDetails;

    // Currently active context
    int _activeContext;

    // Are we in GL begin end block:
    bool _isInGLBeginEndBlock;

    // Is FBO bound:
    bool _isFBOBound;
};



#endif  // __gdStaticBufferImageProxy
