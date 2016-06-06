//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apRenderPrimitivesStatistics.h
///
//==================================================================================

//------------------------------ apRenderPrimitivesStatistics.h ------------------------------

#ifndef __APRENDERPRIMITIVESSTATISTICS_H
#define __APRENDERPRIMITIVESSTATISTICS_H


// OpenGL
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTBaseTools/Include/gtMap.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// ----------------------------------------------------------------------------------
// Struct Name:          apRenderPrimitivesStatistics : public osTransferableObject
// General Description:  Contain the statistic data in a debugged process
// Author:  AMD Developer Tools Team
// Creation Date:        15/7/2008
// ----------------------------------------------------------------------------------
class AP_API apRenderPrimitivesStatistics : public osTransferableObject
{
public:
    apRenderPrimitivesStatistics();
    virtual ~apRenderPrimitivesStatistics();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    void addBatchStatistics(int numOfVertices);
    const gtMap<int, gtUInt64>& getStatisticsMap() const {return _numOfVerticesToNumOfBatchesMap;}
    gtUInt64 getBatchesAmount(int amountOfVertices) const;
    int getMaxVerticesAmount() const;
    int getAmountOfItems() const;

    void clearStatistics();

private:

    // Vector that contain the rendered batches counters:
    gtMap<int, gtUInt64> _numOfVerticesToNumOfBatchesMap;
};


#endif //__APRENDERPRIMITIVESSTATISTICS_H

