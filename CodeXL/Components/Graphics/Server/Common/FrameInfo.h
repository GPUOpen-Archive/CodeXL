//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file FrameInfo.h
/// \brief Defines a structure that contains basic information about a rendered frame.
//==============================================================================

#ifndef FRAMEINFO_H
#define FRAMEINFO_H

#include "OSWrappers.h"
#include <tinyxml.h>

//--------------------------------------------------------------------------
/// A structure that contains basic information about a rendered frame.
//--------------------------------------------------------------------------
class FrameInfo
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor for the FrameInfo class.
    //--------------------------------------------------------------------------
    FrameInfo();

    //--------------------------------------------------------------------------
    /// Default destructor for the FrameInfo class.
    //--------------------------------------------------------------------------
    ~FrameInfo();

    //--------------------------------------------------------------------------
    /// Read a chunk of metadata XML to populate all members.
    /// \param inMetadataXML A string of XML metadata that will be parsed.
    /// \returns True if parsing was successful. False if an error occurred.
    //--------------------------------------------------------------------------
    bool ReadFromXML(const gtASCIIString& inMetadataXML);

    //--------------------------------------------------------------------------
    /// Write a chunk of XML containing the basic frame information.
    /// \param outMetadataXML A generated chunk of XML based on the contents of this instance.
    //--------------------------------------------------------------------------
    void WriteToXML(gtASCIIString& outMetadataXML);

    //--------------------------------------------------------------------------
    /// The total elapsed time that the instrumented application has been running for.
    //--------------------------------------------------------------------------
    float mTotalElapsedTime;

    //--------------------------------------------------------------------------
    /// The total duration (CPU-side timing) of the frame since the last present.
    //--------------------------------------------------------------------------
    float mFrameDuration;

    //--------------------------------------------------------------------------
    /// The computed frames per second based on a history of the last N frame times.
    //--------------------------------------------------------------------------
    double mRunningFPS;

    //--------------------------------------------------------------------------
    /// The number of frames that have already been rendered.
    //--------------------------------------------------------------------------
    unsigned int mFrameNumber;
};

//--------------------------------------------------------------------------
/// The FrameInfoXMLVisitor will visit each XML Element and extra data to
/// populate the given FrameInfo instance.
//--------------------------------------------------------------------------
class FrameInfoXMLVisitor : public TiXmlVisitor
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor for the FrameInfoXMLVisitor class.
    /// \param inFrameInfoInstance The FrameInfo instance that will be populated by this visitor.
    //--------------------------------------------------------------------------
    FrameInfoXMLVisitor(FrameInfo* inFrameInfoInstance)
        : mFrameInfoInstance(inFrameInfoInstance)
    {
    }

    //--------------------------------------------------------------------------
    /// A visitor implementation which visits each element within the visited XML document.
    /// \param inElement The current element being visited.
    /// \param inFirstAttribute The first attribute in the element being visited.
    /// \returns True.
    //--------------------------------------------------------------------------
    virtual bool VisitEnter(const TiXmlElement& inElement, const TiXmlAttribute* inFirstAttribute)
    {
        UNREFERENCED_PARAMETER(inFirstAttribute);

        const char* elementName = inElement.Value();
        const char* elementText = inElement.GetText();

        // Step through each element, looking for specific names. Extract what we need to fill out our structure.
        if (elementName != NULL && elementText != NULL)
        {
            if (strcmp(elementName, "FrameNumber") == 0)
            {
                unsigned int frameNumber = static_cast<unsigned int>(atoi(elementText));
                mFrameInfoInstance->mFrameNumber = frameNumber;
            }
            else if (strcmp(elementName, "ElapsedTime") == 0)
            {
                float totalElapsedTime = static_cast<float>(atof(elementText));
                mFrameInfoInstance->mTotalElapsedTime = totalElapsedTime;
            }
            else if (strcmp(elementName, "CPUFrameDuration") == 0)
            {
                float frameDuration = static_cast<float>(atof(elementText));
                mFrameInfoInstance->mFrameDuration = frameDuration;
            }
            else if (strcmp(elementName, "FPS") == 0)
            {
                float fps = static_cast<float>(atof(elementText));
                mFrameInfoInstance->mRunningFPS = fps;
            }
        }

        return true;
    }

private:
    //--------------------------------------------------------------------------
    /// The FrameInfo instance that's going to be populated with this visitor.
    //--------------------------------------------------------------------------
    FrameInfo* mFrameInfoInstance;
};

#endif // FRAMEINFO_H