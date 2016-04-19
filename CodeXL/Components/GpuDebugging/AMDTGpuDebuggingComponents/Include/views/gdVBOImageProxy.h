//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdVBOImageProxy.h
///
//==================================================================================

//------------------------------ gdVBOImageProxy.h ------------------------------

#ifndef __GDVBOIMAGEPROXY
#define __GDVBOIMAGEPROXY

// Forward decelerations:
struct FIBITMAP;

// Infra:
#include <AMDTApplicationComponents/Include/acImageDataProxy.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTAPIClasses/Include/apContextID.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdVBOImageProxy
// General Description:  An image proxy to retrieve a VBO image. This proxy is also used
//                       for OpenCL buffers, since it also generates only text thumbnail.
// Author:               Sigal Algranaty
// Creation Date:        28/4/2009
// ----------------------------------------------------------------------------------
class gdVBOImageProxy : public acImageDataProxy
{
public:
    // Constructor:
    gdVBOImageProxy(apContextID contextID, int bufferName): _contextID(contextID), _bufferName(bufferName) {};

    // Destructor:
    virtual ~gdVBOImageProxy() {};

public:
    // Image generation function:
    virtual bool loadImage();

    // Overrides acImageDataProxy:
    virtual void buildTooltipText();

private:
    // Buffer details:
    apContextID _contextID;
    int _bufferName;

};


#endif  // __GDVBOIMAGEPROXY
