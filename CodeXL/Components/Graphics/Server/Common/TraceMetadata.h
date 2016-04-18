//==============================================================================
/// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file TraceMetadata.h
/// \brief A small structure that contains all information included within a trace metadata file.
//==============================================================================

#ifndef TRACEMETADATA_H
#define TRACEMETADATA_H

#include "OSwrappers.h"
#include <AMDTOSWrappers/Include/osProcess.h>

class FrameInfo;

//--------------------------------------------------------------------------
/// A small structure that contains all information included within a trace metadata file.
//--------------------------------------------------------------------------
class TraceMetadata
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor for the TraceMetadata class.
    //--------------------------------------------------------------------------
    TraceMetadata();

    //--------------------------------------------------------------------------
    /// Default destructor for the TraceMetadata class.
    //--------------------------------------------------------------------------
    ~TraceMetadata();

    //--------------------------------------------------------------------------
    /// Read a chunk of metadata XML to populate all members.
    /// \param inMetadataXML A string of XML metadata that will be parsed.
    /// \returns True if parsing was successful. False if an error occurred.
    //--------------------------------------------------------------------------
    bool ReadFromXML(const gtASCIIString& inMetadataXML);

    //--------------------------------------------------------------------------
    /// Write a chunk of XML that will ultimately be written to disk as a metadata file.
    /// \param outMetadataXML A generated chunk of XML based on the contents of this instance.
    //--------------------------------------------------------------------------
    void WriteToXML(gtASCIIString& outMetadataXML);

    //--------------------------------------------------------------------------
    /// Contains basic info about a rendered frame, including timing and API
    /// and draw call counts.
    //--------------------------------------------------------------------------
    FrameInfo* mFrameInfo;

    //--------------------------------------------------------------------------
    /// The path to the cached trace file on disk.
    //--------------------------------------------------------------------------
    std::string mMetadataFilepath;

    //--------------------------------------------------------------------------
    /// The path to the cached trace file on disk.
    //--------------------------------------------------------------------------
    std::string mPathToTraceFile;

    //--------------------------------------------------------------------------
    /// The path to the cached object tree file on disk.
    //--------------------------------------------------------------------------
    std::string mPathToObjectTreeFile;

    //--------------------------------------------------------------------------
    /// The path to the cached object database on disk.
    //--------------------------------------------------------------------------
    std::string mPathToObjectDatabaseFile;

    //--------------------------------------------------------------------------
    /// The path to the captured frame buffer image on disk.
    //--------------------------------------------------------------------------
    std::string mPathToFrameBufferImage;

    //--------------------------------------------------------------------------
    /// The type of trace.
    //--------------------------------------------------------------------------
    UINT mTraceType;

    //--------------------------------------------------------------------------
    /// The bitness of the instrumented application.
    //--------------------------------------------------------------------------
    osModuleArchitecture mArchitecture;

    //--------------------------------------------------------------------------
    /// The total number of API calls that occurred within the traced frame.
    //--------------------------------------------------------------------------
    unsigned int mAPICallCount;

    //--------------------------------------------------------------------------
    /// The total number of draw calls that occurred within the traced frame.
    //--------------------------------------------------------------------------
    unsigned int mDrawCallCount;
};

//--------------------------------------------------------------------------
/// Read a trace's metadata file and store the parsed info into the output argument.
/// \param inPathToMetadataFile The full path to the metadata file on disk.
/// \param outTraceMetadata A structure containing all parsed info from the metadata file.
/// \returns True if reading the metadata file was successful.
//--------------------------------------------------------------------------
bool ReadMetadataFile(const std::string& inPathToMetadataFile, TraceMetadata* outTraceMetadata);

//--------------------------------------------------------------------------
/// Write a trace's metadata file and return the contenst through the out-param.
/// \param inMetadata input trace metadata
/// \param inMetadataFilepath The full response string for a collected linked trace request.
/// \param outMetadataXML The XML metadata string to return to the client.
/// \returns True if writing the metadata file was successful.
//--------------------------------------------------------------------------
bool WriteMetadataFile(TraceMetadata* inMetadata, const std::string& inMetadataFilepath, std::string& outMetadataXML);

#endif // TRACEMETADATA_H