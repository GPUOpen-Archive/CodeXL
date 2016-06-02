//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file TraceMetadata.cpp
/// \brief A small structure that contains all information included within a trace metadata file.
//==============================================================================

#include "TraceMetadata.h"
#include "FrameInfo.h"
#include "misc.h"
#include "xml.h"
#include "OSWrappers.h"
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
//--------------------------------------------------------------------------
/// The MetadataXMLVisitor will visit each XML Element and extract data
/// used to populate the given TraceMetadata instance.
//--------------------------------------------------------------------------
class MetadataXMLVisitor : public TiXmlVisitor
{
public:
    //--------------------------------------------------------------------------
    /// Constructor used to indicate which TraceMetadata instance is going to be populated.
    /// \param inMetadataInstance The TraceMetadata instance that is going to be populated.
    //--------------------------------------------------------------------------
    MetadataXMLVisitor(TraceMetadata* inMetadataInstance)
        : mMetadataInstance(inMetadataInstance)
        , mFrameInfoVisitor(inMetadataInstance->mFrameInfo)
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
        const char* elementName = inElement.Value();
        const char* elementText = inElement.GetText();

        // Step through each element, looking for specific names. Extract what we need to fill out our structure.
        if (elementName != NULL && elementText != NULL)
        {
            if (strcmp(elementName, "API") == 0)
            {
                mMetadataInstance->mAPIString.assign(elementText);
            }
            else if (strcmp(elementName, "Location") == 0)
            {
                mMetadataInstance->mMetadataFilepath.assign(elementText);
            }
            else if (strcmp(elementName, "TraceType") == 0)
            {
                int traceType = atoi(elementText);
                mMetadataInstance->mTraceType = traceType;
            }
            else if (strcmp(elementName, "Architecture") == 0)
            {
                mMetadataInstance->mArchitecture = static_cast<osModuleArchitecture>(atoi(elementText));
            }
            else if (strcmp(elementName, "APICallCount") == 0)
            {
                mMetadataInstance->mAPICallCount = static_cast<unsigned int>(atoi(elementText));
            }
            else if (strcmp(elementName, "DrawCallCount") == 0)
            {
                mMetadataInstance->mDrawCallCount = static_cast<unsigned int>(atoi(elementText));
            }
            else if (strcmp(elementName, "TracedFramesCount") == 0)
            {
                mMetadataInstance->mTracedFramesCount = static_cast<unsigned int>(atoi(elementText));
            }
            else if (strcmp(elementName, "LinkedTrace") == 0)
            {
                mMetadataInstance->mPathToTraceFile.assign(elementText);
            }
            else if (strcmp(elementName, "FrameBufferImage") == 0)
            {
                mMetadataInstance->mPathToFrameBufferImage.assign(elementText);
            }
            else if (strcmp(elementName, "ObjectTree") == 0)
            {
                mMetadataInstance->mPathToObjectTreeFile.assign(elementText);
            }
            else if (strcmp(elementName, "ObjectDatabase") == 0)
            {
                mMetadataInstance->mPathToObjectDatabaseFile.assign(elementText);
            }
            else
            {
                mFrameInfoVisitor.VisitEnter(inElement, inFirstAttribute);
            }
        }

        return true;
    }

private:
    //--------------------------------------------------------------------------
    /// The TraceMetadata instance that's going to be populated with this visitor.
    //--------------------------------------------------------------------------
    TraceMetadata* mMetadataInstance;

    //--------------------------------------------------------------------------
    /// An XML element visitor responsible for extracting FrameInfo elements.
    //--------------------------------------------------------------------------
    FrameInfoXMLVisitor mFrameInfoVisitor;
};

//--------------------------------------------------------------------------
/// Default constructor for the TraceMetadata class.
//--------------------------------------------------------------------------
TraceMetadata::TraceMetadata()
    : mArchitecture(OS_UNKNOWN_ARCHITECTURE)
    , mFrameInfo(NULL)
    , mAPICallCount(0)
    , mDrawCallCount(0)
    , mTracedFramesCount(0)
{
    mPathToTraceFile.clear();
    mPathToFrameBufferImage.clear();
    mPathToObjectTreeFile.clear();
    mPathToObjectDatabaseFile.clear();
}

//--------------------------------------------------------------------------
/// Default destructor for the TraceMetadata class.
//--------------------------------------------------------------------------
TraceMetadata::~TraceMetadata()
{
}

//--------------------------------------------------------------------------
/// Read a chunk of metadata XML to populate all members.
/// \param inMetadataXML A string of XML metadata that will be parsed.
/// \returns True if parsing was successful. False if an error occurred.
//--------------------------------------------------------------------------
bool TraceMetadata::ReadFromXML(const gtASCIIString& inMetadataXML)
{
    TiXmlDocument xmlDocument;
    xmlDocument.Parse(inMetadataXML.asCharArray());

    // Create a visitor, which will populate "this" TraceMetadata instance.
    MetadataXMLVisitor elementVisitor(this);
    bool bVisistedSuccessfully = xmlDocument.Accept(&elementVisitor);

    return bVisistedSuccessfully;
}

//--------------------------------------------------------------------------
/// Write a chunk of XML that will ultimately be written to disk as a metadata file.
/// \param outMetadataXML A generated chunk of XML based on the contents of this instance.
//--------------------------------------------------------------------------
void TraceMetadata::WriteToXML(gtASCIIString& outMetadataXML)
{
    gtASCIIString metadataXML;

    metadataXML += XML("API", FormatText("%s", mAPIString.c_str()).asCharArray());
    metadataXML += XML("Location", FormatText("%s", mMetadataFilepath.c_str()).asCharArray());
    metadataXML += XML("Architecture", FormatText("%u", mArchitecture).asCharArray());
    metadataXML += XML("APICallCount", FormatText("%u", mAPICallCount).asCharArray());
    metadataXML += XML("DrawCallCount", FormatText("%u", mDrawCallCount).asCharArray());
    metadataXML += XML("TracedFramesCount", FormatText("%u", mTracedFramesCount).asCharArray());

    gtASCIIString contentsXML;

    metadataXML += XML("TraceType", FormatText("%u", mTraceType).asCharArray());
    contentsXML += XML("LinkedTrace", FormatText("%s", mPathToTraceFile.c_str()).asCharArray());
    contentsXML += XML("ObjectTree", FormatText("%s", mPathToObjectTreeFile.c_str()).asCharArray());
    contentsXML += XML("ObjectDatabase", FormatText("%s", mPathToObjectDatabaseFile.c_str()).asCharArray());
    contentsXML += XML("FrameBufferImage", FormatText("%s", mPathToFrameBufferImage.c_str()).asCharArray());

    // Pull this section of the contents out of the internal FrameInfo structure.
    mFrameInfo->WriteToXML(contentsXML);

    // Surround the generated contents string with a parent element
    metadataXML += XML("Contents", contentsXML.asCharArray());

    // Now surround the entire chunk with an "XML" block so there's a single root node.
    outMetadataXML = XML("XML", metadataXML.asCharArray());
}

//--------------------------------------------------------------------------
/// Read a trace's metadata file and store the parsed info into the output argument.
/// \param inPathToMetadataFile The full path to the metadata file on disk.
/// \param outTraceMetadata A structure containing all parsed info from the metadata file.
/// \returns True if reading the metadata file was successful.
//--------------------------------------------------------------------------
bool ReadMetadataFile(const std::string& inPathToMetadataFile, TraceMetadata* outTraceMetadata)
{
    bool bReadSuccessfully = true;

    // We have a path to a cached trace metadata file. Load the metadata file, and return the trace response text.
    std::wstring widePathToMetadataFile;
    widePathToMetadataFile.assign(inPathToMetadataFile.begin(), inPathToMetadataFile.end());
    gtString metadataFilepathGTString;
    metadataFilepathGTString.appendFormattedString(L"%s", widePathToMetadataFile.c_str());

    osFilePath metadataFilepath(metadataFilepathGTString);
    osFile metadataFile;
    metadataFile.setPath(metadataFilepath);

    // Open the metadata file, and dump the XML into a string.
    bool bMetadataFileOpened = metadataFile.open(osChannel::OS_ASCII_TEXT_CHANNEL);

    if (bMetadataFileOpened)
    {
        gtASCIIString metadataXML;
        bool bReadSuccessful = metadataFile.readIntoString(metadataXML);

        if (bReadSuccessful)
        {
            bReadSuccessfully = outTraceMetadata->ReadFromXML(metadataXML);
        }
        else
        {
            Log(logERROR, "Failed to read metadata XML from file.\n");
        }
    }
    else
    {
        Log(logERROR, "Failed to open trace metadata XML file at '%s'.\n", inPathToMetadataFile.c_str());
    }

    return bReadSuccessfully;
}

bool WriteMetadataFile(TraceMetadata* inMetadata, const std::string& inMetadataFilepath, std::string& outMetadataXML)
{
    bool bWriteSuccessful = false;

    gtString fullMetadataFilepathAsGTString;
    fullMetadataFilepathAsGTString.fromASCIIString(inMetadataFilepath.c_str());

    osFile metadataFile(fullMetadataFilepathAsGTString);
    bool bMetadataFileOpened = metadataFile.open(osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (bMetadataFileOpened)
    {
        // Write the metadata xml into the output file.
        gtASCIIString metadataXMLString;
        inMetadata->WriteToXML(metadataXMLString);

        // Now, just for fun, convert the gtASCIIString to a gtString.
        gtString metadataXMLAsGTString;
        metadataXMLAsGTString.fromASCIIString(metadataXMLString.asCharArray());

        // Write the metadata XML into the file, and close.
        metadataFile.writeString(metadataXMLAsGTString);
        metadataFile.close();

        // The client will receive the full metadata XML string to parse.
        outMetadataXML.assign(metadataXMLString.asCharArray());

        Log(logMESSAGE, "Wrote XML metadata file to '%s'.\n", inMetadataFilepath.c_str());

        bWriteSuccessful = true;
    }
    else
    {
        Log(logERROR, "Failed to open trace metadata file for writing: '%s'\n", inMetadataFilepath.c_str());
    }

    return bWriteSuccessful;
}