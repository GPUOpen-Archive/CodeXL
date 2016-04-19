//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdPBufferImageProxy.h
///
//==================================================================================

//------------------------------ gdPBufferImageProxy.h ------------------------------

#ifndef __GDPBUFFERIMAGEPROXY
#define __GDPBUFFERIMAGEPROXY

// Forward decelerations:
struct FIBITMAP;

// Infra:
#include <AMDTAPIClasses/Include/apStaticBuffer.h>
#include <AMDTApplicationComponents/Include/acImageDataProxy.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdPBufferImageProxy
// General Description:  An image proxy to retrieve a PBuffer content buffer
// Author:               Eran Zinman
// Creation Date:        25/1/2008
// ----------------------------------------------------------------------------------
class gdPBufferImageProxy : public acImageDataProxy
{
public:
    // Constructor:
    gdPBufferImageProxy(int pbufferID, const apStaticBuffer& bufferDetails, int pbufferContext, bool isInGLBeginEndBlock) :
        _pbufferID(pbufferID), _bufferDetails(bufferDetails),
        _pbufferContext(pbufferContext), _isInGLBeginEndBlock(isInGLBeginEndBlock) {};

    // Destructor
    virtual ~gdPBufferImageProxy() {};

public:
    // Image generation function
    virtual bool loadImage();

    // Overrides acImageDataProxy:
    virtual void buildTooltipText();

private:
    // Generates the buffer image.
    bool generateBufferImage();

private:
    // The PBuffer ID
    int _pbufferID;

    // The static buffer details
    apStaticBuffer _bufferDetails;

    // PBuffer attached render context spy id
    int _pbufferContext;

    // Are we in GL begin end block:
    bool _isInGLBeginEndBlock;
};



#endif  // __GDPBUFFERIMAGEPROXY
