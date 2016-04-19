//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Functions to support the writing of XML
//==============================================================================

#ifndef GPS_XML_INCLUDED
#define GPS_XML_INCLUDED

#include "frect.h"
#include "CommandProcessor.h"
#include <sstream>
#include <AMDTBaseTools/Include/gtASCIIString.h>

/// Helper macro for XML generation
#define XMLStream( node, contents ) "<" << (node) << ">" << (contents) << "</" << (node) << ">"

/// Helper macro for XML generation
#define XMLAttribStream( attrib, value ) " " << attrib << "='" << value << "'"

//-----------------------------------------------------------------------------
/// escapes special characters for formating into XML
///
/// \param strValue value that needs to have special characters escaped so that
///    the XML can be parsed properly
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLEscape(gtASCIIString strValue);

//-----------------------------------------------------------------------------
/// generates XML with a node and an URL attribute
///
/// \param strNode node name
/// \param strURL string containing the URL
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLURL(const char* strNode, const char* strURL);

//-----------------------------------------------------------------------------
/// generates XML with a node and an attribute and no value
///
/// \param strNode node name
/// \param strAttrib string containing the attribute names and values
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLAttrib(const char* strNode, const char* strAttrib);

//-----------------------------------------------------------------------------
/// generates XML with a node, an attribute, and a value
///
/// \param strNode node name
/// \param strAttrib string containing the attribute names and values
/// \param strValue string containing the value enclosed within the start and
///    end tags of the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLAttrib(const char* strNode, const char* strAttrib, const char* strValue);

//-----------------------------------------------------------------------------
/// generates XML with a node, an attribute, and a value
///
/// \param strNode node name
/// \param strAttrib string containing the attribute names and values
/// \param dwValue string containing the value enclosed within the start and
///    end tags of the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLAttrib(const char* strNode, const char* strAttrib, unsigned int dwValue);

//-----------------------------------------------------------------------------
/// generates XML with a node, an attribute, and a value
///
/// \param strNode node name
/// \param strAttrib string containing the attribute names and values
/// \param bValue string containing the value enclosed within the start and
///    end tags of the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLBoolAttrib(const char* strNode, const char* strAttrib, bool bValue);

//-----------------------------------------------------------------------------
/// generates XML with a node, an attribute, and a value
///
/// \param strNode node name
/// \param strAttrib string containing the attribute names and values
/// \param fValue string containing the value enclosed within the start and
///    end tags of the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLFloatAttrib(const char* strNode, const char* strAttrib, float fValue);

//-----------------------------------------------------------------------------
/// generates XML with a node and a value
///
/// \param strNode node name
/// \param strValue string containing the value enclosed within the start and
///    end tags of the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, const char* strValue);

//-----------------------------------------------------------------------------
/// generates XML with a node and an unsigned long value
///
/// \param strNode node name
/// \param ulValue unsigned long value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, unsigned long ulValue);

//-----------------------------------------------------------------------------
/// generates XML with a node and a bool value
///
/// \param strNode node name
/// \param bValue bool value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLBool(const char* strNode, bool bValue);

//-----------------------------------------------------------------------------
/// generates XML with a node and a BOOL value
///
/// \param strNode node name
/// \param bValue BOOL value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLBOOL(const char* strNode, int bValue);

//-----------------------------------------------------------------------------
/// generates XML with a node and a pointer value in hex
///
/// \param strNode node name
/// \param ptr string containing the URL
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLHexPtr(const char* strNode, const void* ptr);

//-----------------------------------------------------------------------------
/// generates XML with a node and a long value
///
/// \param strNode node name
/// \param lValue long value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, long lValue);

//-----------------------------------------------------------------------------
/// generates XML with a node and a UINT64 value
///
/// \param strNode node name
/// \param lValue long value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, UINT64 lValue);

//-----------------------------------------------------------------------------
/// generates XML with a node and a float value
///
/// \param strNode node name
/// \param fValue float value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, float fValue);

//-----------------------------------------------------------------------------
/// generates XML with a node and a double value
///
/// \param strNode node name
/// \param dValue double value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, double dValue);

//-----------------------------------------------------------------------------
/// generates XML with a node and an unsigned int value
///
/// \param strNode node name
/// \param uValue unsigned int value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, unsigned int uValue);

//-----------------------------------------------------------------------------
/// generates XML with a node and an int value
///
/// \param strNode node name
/// \param nValue int value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, int nValue);

//-----------------------------------------------------------------------------
/// generates XML that contains the extents of a rectangle
///
/// \param rRect reference to a RECT structure to encode in XML
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLRect(WinRect& rRect);

//-----------------------------------------------------------------------------
/// Generates and XML header
///
/// \return string containing the XML header
//-----------------------------------------------------------------------------
gtASCIIString XMLHeader();

//-----------------------------------------------------------------------------
/// generates XML to describe a drawcall
///
/// \param ulIndex number of the draw call within the current frame
/// \param strDrawCallXML output from the other GetDrawCallXML function
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString GetDrawCallXML(unsigned long ulIndex, const char* strDrawCallXML);

//-----------------------------------------------------------------------------
/// generates XML to describe a drawcall
///
/// \param strDrawCallName string name of the drawcall (the function name)
/// \param strParams string describing the parameters of the draw call
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString GetDrawCallXML(const char* strDrawCallName, const char* strParams);

//-----------------------------------------------------------------------------
/// generates XML to encode X, Y, Z, and W integer values specified by the
/// input array
///
/// \param pIntConstants array of four floats values to encode as X, Y, Z, W
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString GetXMLIntegerConstant(int* pIntConstants);

//-----------------------------------------------------------------------------
/// generates XML to encode X, Y, Z, and W float values specified by the
/// input array
///
/// \param pFloatConstants array of four floats values to encode as X, Y, Z, W
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString GetXMLFloatConstant(float* pFloatConstants);

//-----------------------------------------------------------------------------
/// generates XML that contains two nodes: "name" and "value" and each have an
/// associated value. Particularly useful for counter data
///
/// \param strName string containing the name of the value
/// \param strValue string containing the value to encode
/// \return generated XML string
//-----------------------------------------------------------------------------
inline gtASCIIString XMLNameValue(const char* strName, const char* strValue);

/// texture type enumerations
enum XML_TEXTURE_TYPE
{
    XTT_UNKNOWN,
    XTT_BUFFER,
    XTT_1D,
    XTT_1DARRAY,
    XTT_2D,
    XTT_2DARRAY,
    XTT_2DMS,
    XTT_2DMSARRAY,
    XTT_3D,
    XTT_CUBEMAP,
    XTT_CUBEMAPARRAY,
    XTT_BUFFEREX,
};

//-----------------------------------------------------------------------------
/// Generates API-agnostic XML that describes a texture and provides a url
/// to the image and to the full API-specific description.
/// \param strStage The pipeline stage that the resource is in
/// \param strArrayName The name of the array
/// \param uSlot The texture slot index
/// \param eType The texture type
/// \param uWidth  The width
/// \param uHeight  The height
/// \param uDepth The depth
/// \param uMipLevels The mip map level
/// \param strUsage The usage string
/// \param strFormat The format
/// \param uSamples Sample count
/// \param uOffset  The offset
/// \param shaderInputTypeString Shader input type string
/// \param eRIT Resource image type
/// \param GpuIndex Index of the GPU to use
//-----------------------------------------------------------------------------
gtASCIIString TextureArrayElementXML(const char* strStage,
                                     const char* strArrayName,
                                     unsigned int uSlot,
                                     XML_TEXTURE_TYPE eType,
                                     unsigned int uWidth,
                                     unsigned int uHeight,
                                     unsigned int uDepth,
                                     unsigned int uMipLevels,
                                     const char* strUsage,
                                     const char* strFormat,
                                     unsigned int uSamples,
                                     unsigned int uOffset,
                                     const char* shaderInputTypeString,
                                     ResourceImageType eRIT,
                                     UINT GpuIndex);

//-----------------------------------------------------------------------------
/// Generates API-agnostic XML that describes an element of a constant buffer
/// \param pcszName the name of the constant
/// \param uNumRows the number of rows in the constant
/// \param uNumComponentsPerRow the number of elements in each row of the constant
/// \param pValues an array of values that must contain ( uNumRows * uNumComponentsPerRow ) values
/// \param pcszExtraXML optional parameter that allows additional API-specific XML to be added to the returned XML node
/// \return XML string containing the constant's name and values; one XML node is returned for each row of the constant
//-----------------------------------------------------------------------------
template <class T> gtASCIIString GetConstantBufferElementXML(const char* pcszName, unsigned int uNumRows, unsigned int uNumComponentsPerRow, T* pValues, const char* pcszExtraXML = "")
{
    gtASCIIString string;

    if (uNumComponentsPerRow < 1 || uNumComponentsPerRow > 4)
    {
        string += XMLAttrib("Constant", FormatText("Name='%s'", pcszName).asCharArray(), XML("X", "invalid").asCharArray());
        return string;
    }

    for (unsigned int i = 0; i < uNumRows; i++)
    {
        unsigned int uRowStart = i * uNumComponentsPerRow;

        gtASCIIString tmpString;

        switch (uNumComponentsPerRow)
        {
            case 1:
                tmpString = XML("X", pValues[uRowStart]);
                break;

            case 2:
                tmpString = XML("X", pValues[uRowStart]);
                tmpString += XML("Y", pValues[uRowStart + 1]);
                break;

            case 3:
                tmpString = XML("X", pValues[uRowStart]);
                tmpString += XML("Y", pValues[uRowStart + 1]);
                tmpString += XML("Z", pValues[uRowStart + 2]);
                break;

            case 4:
                tmpString = XML("X", pValues[uRowStart]);
                tmpString += XML("Y", pValues[uRowStart + 1]);
                tmpString += XML("Z", pValues[uRowStart + 2]);
                tmpString += XML("W", pValues[uRowStart + 3]);
                break;

            default:
                tmpString = "";
                break;
        }

        tmpString += pcszExtraXML;

        string += XMLAttrib("Constant",
                            (uNumRows == 1) ? FormatText("Name='%s'", pcszName).asCharArray() : FormatText("Name='%s[%d]'", pcszName, i).asCharArray(), tmpString.asCharArray());
    }

    return string;
}

#endif
