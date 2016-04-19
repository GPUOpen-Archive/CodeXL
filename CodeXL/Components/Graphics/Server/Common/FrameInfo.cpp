//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file FrameInfo.cpp
/// \brief Defines a structure that contains basic information about a rendered frame.
//==============================================================================

#include "FrameInfo.h"
#include "misc.h"
#include "xml.h"

//--------------------------------------------------------------------------
/// Default constructor for the FrameInfo class.
//--------------------------------------------------------------------------
FrameInfo::FrameInfo()
    : mTotalElapsedTime(0.0f)
    , mFrameDuration(0.0f)
{
}

//--------------------------------------------------------------------------
/// Default destructor for the FrameInfo class.
//--------------------------------------------------------------------------
FrameInfo::~FrameInfo()
{
}

//--------------------------------------------------------------------------
/// Read a chunk of metadata XML to populate all members.
/// \param inMetadataXML A string of XML metadata that will be parsed.
/// \returns True if parsing was successful. False if an error occurred.
//--------------------------------------------------------------------------
bool FrameInfo::ReadFromXML(const gtASCIIString& inMetadataXML)
{
    TiXmlDocument xmlDocument;
    xmlDocument.Parse(inMetadataXML.asCharArray());

    // Create a FrameInfoXMLVisitor, which will populate this instance.
    FrameInfoXMLVisitor elementVisitor(this);
    bool bVisitedSuccessfully = xmlDocument.Accept(&elementVisitor);

    return bVisitedSuccessfully;
}

//--------------------------------------------------------------------------
/// Write a chunk of XML containing the basic frame information.
/// \param outMetadataXML A generated chunk of XML based on the contents of this instance.
//--------------------------------------------------------------------------
void FrameInfo::WriteToXML(gtASCIIString& outMetadataXML)
{
    // Append the frame info XML onto the end of the incoming string.
    outMetadataXML += XML("FrameNumber", FormatText("%u", mFrameNumber).asCharArray());
    outMetadataXML += XML("ElapsedTime", FormatText("%f", mTotalElapsedTime).asCharArray());
    outMetadataXML += XML("CPUFrameDuration", FormatText("%f", mFrameDuration).asCharArray());
    outMetadataXML += XML("FPS", FormatText("%f", mRunningFPS).asCharArray());
}