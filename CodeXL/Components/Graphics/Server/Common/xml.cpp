//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Functions to support the writing of XML
//==============================================================================

#include "xml.h"

//-----------------------------------------------------------------------------
// public functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// escapes special characters for formating into XML
///
/// \param strValue value that needs to have special characters escaped so that
///    the XML can be parsed properly
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLEscape(gtASCIIString strValue)
{
    // fix special characters
    strValue.replace("&", "&amp;");
    strValue.replace("<", "&lt;");
    strValue.replace(">", "&gt;");
    strValue.replace("'", "&apos;");
    strValue.replace("\"", "&quot;");

    return strValue;
}

//-----------------------------------------------------------------------------
/// generates XML with a node and an URL attribute
///
/// \param strNode node name
/// \param strURL string containing the URL
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLURL(const char* strNode, const char* strURL)
{
    gtASCIIString strXML;
    strXML.appendFormattedString("<%s url='%s'/>", strNode, strURL);
    return strXML;
}

//-----------------------------------------------------------------------------
/// generates XML with a node and an attribute
///
/// \param strNode node name
/// \param strAttrib string containing the name attribute
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLAttrib(const char* strNode, const char* strAttrib)
{
    gtASCIIString strXML;
    strXML.appendFormattedString("<%s %s/>", strNode, strAttrib);
    return strXML;
}

//-----------------------------------------------------------------------------
/// generates XML with a node, an attribute, and a value
///
/// \param strNode node name
/// \param strAttrib string containing the attribute names and values
/// \param strValue string containing the value enclosed within the start and
///    end tags of the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLAttrib(const char* strNode, const char* strAttrib, const char* strValue)
{
    gtASCIIString strXML;
    strXML.appendFormattedString("<%s %s>%s</%s>", strNode, strAttrib, strValue, strNode);
    return strXML;
}

//-----------------------------------------------------------------------------
/// generates XML with a node, an attribute, and a value
///
/// \param strNode node name
/// \param strAttrib string containing the attribute names and values
/// \param dwValue string containing the value enclosed within the start and
///    end tags of the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLAttrib(const char* strNode, const char* strAttrib, unsigned int dwValue)
{
    gtASCIIString strXML;
    strXML.appendFormattedString("<%s %s>%u</%s>", strNode, strAttrib, dwValue, strNode);
    return strXML;
}

//-----------------------------------------------------------------------------
/// generates XML with a node, an attribute, and a value
///
/// \param strNode node name
/// \param strAttrib string containing the attribute names and values
/// \param bValue string containing the value enclosed within the start and
///    end tags of the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLBoolAttrib(const char* strNode, const char* strAttrib, bool bValue)
{
    gtASCIIString strXML;
    strXML.appendFormattedString("<%s %s>%s</%s>", strNode, strAttrib, bValue ? "TRUE" : "FALSE" , strNode);
    return strXML;
}

//-----------------------------------------------------------------------------
/// generates XML with a node, an attribute, and a value
///
/// \param strNode node name
/// \param strAttrib string containing the attribute names and values
/// \param fValue string containing the value enclosed within the start and
///    end tags of the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLFloatAttrib(const char* strNode, const char* strAttrib, float fValue)
{
    gtASCIIString strXML;
    strXML.appendFormattedString("<%s %s>%f</%s>", strNode, strAttrib, fValue , strNode);
    return strXML;
}

//-----------------------------------------------------------------------------
/// generates XML with a node and a value
///
/// \param strNode node name
/// \param strValue string containing the value enclosed within the start and
///    end tags of the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, const char* strValue)
{
    gtASCIIString strXML;
    strXML.appendFormattedString("<%s>%s</%s>", strNode, strValue, strNode);
    return strXML;
}

//-----------------------------------------------------------------------------
/// generates XML with a node and an unsigned long value
///
/// \param strNode node name
/// \param ulValue unsigned long value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, unsigned long ulValue)
{
    return XML(strNode, FormatText("%u", ulValue).asCharArray());
}

//-----------------------------------------------------------------------------
/// generates XML with a node and a long value
///
/// \param strNode node name
/// \param lValue long value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, long lValue)
{
    return XML(strNode, FormatText("%u", lValue).asCharArray());
}

//-----------------------------------------------------------------------------
/// generates XML with a node and a UINT64 value
///
/// \param strNode node name
/// \param lValue long value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, UINT64 lValue)
{
    return XML(strNode, FormatText("%ld", lValue).asCharArray());
}
//-----------------------------------------------------------------------------
/// generates XML with a node and an unsigned int value
///
/// \param strNode node name
/// \param uValue unsigned int value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, unsigned int uValue)
{
    return XML(strNode, FormatText("%u", uValue).asCharArray());
}

//-----------------------------------------------------------------------------
/// generates XML with a node and an int value
///
/// \param strNode node name
/// \param nValue int value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, int nValue)
{
    return XML(strNode, FormatText("%d", nValue).asCharArray());
}

//-----------------------------------------------------------------------------
/// generates XML with a node and a float value
///
/// \param strNode node name
/// \param fValue float value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, float fValue)
{
    return XML(strNode, FormatText("%f", fValue).asCharArray());
}

//-----------------------------------------------------------------------------
/// generates XML with a node and a double value
///
/// \param strNode node name
/// \param dValue double value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XML(const char* strNode, double dValue)
{
    return XML(strNode, FormatText("%Lf", dValue).asCharArray());
}
//-----------------------------------------------------------------------------
/// generates XML with a node and a bool value
///
/// \param strNode node name
/// \param bValue bool value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLBool(const char* strNode, bool bValue)
{
    return XML(strNode, bValue ? "TRUE" : "FALSE");
}

//-----------------------------------------------------------------------------
/// generates XML with a node and a BOOL value
///
/// \param strNode node name
/// \param bValue BOOL value enclosed by the node
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLBOOL(const char* strNode, int bValue)
{
    return XML(strNode, bValue ? "TRUE" : "FALSE");
}

//-----------------------------------------------------------------------------
/// generates XML with a node and a hex value
///
/// \param strNode node name
/// \param ptr string containing the URL
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLHexPtr(const char* strNode, const void* ptr)
{
#ifdef _WIN32
    const char* text = "0x%p";
#else
    const char* text = "%p";

    // special case for NULL pointer; don't print (nil). Client expects
    // 32 or 64 bit hex value returned
    if (ptr == NULL)
    {
#ifdef X64
        text = "0x000000000000000%d";
#else
        text = "0x0000000%d";
#endif
    }

#endif
    return XML(strNode, FormatText(text, ptr).asCharArray());
}

//-----------------------------------------------------------------------------
/// generates XML that contains the extents of a rectangle
///
/// \param rRect reference to a RECT structure to encode in XML
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLRect(WinRect& rRect)
{
    gtASCIIString out;
    out += XML("left", rRect.left);
    out += XML("top", rRect.top);
    out += XML("right", rRect.right);
    out += XML("bottom", rRect.bottom);

    return XML("rect", out.asCharArray());
}

//-----------------------------------------------------------------------------
/// Generates and XML header
///
/// \return string containing the XML header
//-----------------------------------------------------------------------------
gtASCIIString XMLHeader()
{
    return "<?xml version='1.0' encoding='ISO-8859-1'?>";
}

//-----------------------------------------------------------------------------
/// generates XML to describe a drawcall
///
/// \param strDrawCallName string name of the drawcall (the function name)
/// \param strParams string describing the parameters of the draw call
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString GetDrawCallXML(const char* strDrawCallName, const char* strParams)
{
    gtASCIIString xml = XML("call", strDrawCallName);
    xml += XML("parameters", strParams);
    return xml;
}

gtASCIIString GetDrawCallXML(unsigned long ulIndex, const char* strDrawCallXML)
{
    gtASCIIString str = XML("index", FormatText("%lu", ulIndex).asCharArray());
    str += strDrawCallXML;
    return XML("drawcall", str.asCharArray());
}


//-----------------------------------------------------------------------------
/// generates XML to encode X, Y, Z, and W integer values specified by the
/// input array
///
/// \param pIntConstants array of four floats values to encode as X, Y, Z, W
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString GetXMLIntegerConstant(int* pIntConstants)
{
    return FormatText("<X>%i</X><Y>%i</Y><Z>%i</Z><W>%i</W>",
                      pIntConstants[0],
                      pIntConstants[1],
                      pIntConstants[2],
                      pIntConstants[3]);
}

//-----------------------------------------------------------------------------
/// generates XML to encode X, Y, Z, and W float values specified by the
/// input array
///
/// \param pFloatConstants array of four floats values to encode as X, Y, Z, W
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString GetXMLFloatConstant(float* pFloatConstants)
{
    return FormatText("<X>%f</X><Y>%f</Y><Z>%f</Z><W>%f</W>",
                      pFloatConstants[0],
                      pFloatConstants[1],
                      pFloatConstants[2],
                      pFloatConstants[3]);
}

//-----------------------------------------------------------------------------
/// generates XML that contains two nodes: "name" and "value" and each have an
/// associated value. Particularly useful for counter data
///
/// \param strName string containing the name of the value
/// \param strValue string containing the value to encode
/// \return generated XML string
//-----------------------------------------------------------------------------
gtASCIIString XMLNameValue(const char* strName, const char* strValue)
{
    gtASCIIString xml = XML("name", strName);
    xml += XML("value",  strValue);
    return xml;
}

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
                                     UINT GpuIndex)
{
    gtASCIIString strType = "Unknown";

    switch (eType)
    {
        case XTT_1D:
            strType = "1D";
            break;

        case XTT_1DARRAY:
            strType = "1DArray";
            break;

        case XTT_2D:
            strType = "2D";
            break;

        case XTT_2DARRAY:
            strType = "2DArray";
            break;

        case XTT_2DMS:
            strType = "2DMS";
            break;

        case XTT_2DMSARRAY:
            strType = "2DMSArray";
            break;

        case XTT_3D:
            strType = "3D";
            break;

        case XTT_CUBEMAP:
            strType = "CubeMap";
            break;

        case XTT_CUBEMAPARRAY:
            strType = "CubeMapArray";
            break;

        case XTT_BUFFER:
            strType = "Buffer";
            break;

        case XTT_BUFFEREX:
            strType = "BufferEx";
            break;

        case XTT_UNKNOWN:
            strType = "Unknown";
            break;
    }

    if (strlen(shaderInputTypeString) > 1)
    {
        // Include the shader input type in the XML
        gtASCIIString xmlString = XML("Type", strType.asCharArray());
        xmlString += XML("InputType", shaderInputTypeString);
        xmlString += XML("Width", uWidth);
        xmlString += XML("Height", uHeight);
        xmlString += XML("Depth", uDepth);
        xmlString += XML("MipLevels", uMipLevels);
        xmlString += XML("Usage", strUsage);
        xmlString += XML("Format", strFormat);
        xmlString += XML("Samples", uSamples);
        xmlString += XML("Offset", uOffset);
        xmlString += XML("ResourceImageType", eRIT);
        xmlString += XML("GPUIndex", GpuIndex);
        return XMLAttrib("Slot", FormatText("Stage='%s' Path='%s' Index='%u'", strStage, strArrayName, uSlot).asCharArray(), xmlString.asCharArray());
    }
    else
    {
        // Dont' include the shader input type in the XML
        gtASCIIString xmlString = XML("Type", strType.asCharArray());
        xmlString += XML("Width", uWidth);
        xmlString += XML("Height", uHeight);
        xmlString += XML("Depth", uDepth);
        xmlString += XML("MipLevels", uMipLevels);
        xmlString += XML("Usage", strUsage);
        xmlString += XML("Format", strFormat);
        xmlString += XML("Samples", uSamples);
        xmlString += XML("Offset", uOffset);
        xmlString += XML("ResourceImageType", eRIT);
        xmlString += XML("GPUIndex", GpuIndex);
        return XMLAttrib("Slot", FormatText("Stage='%s' Path='%s' Index='%u'", strStage, strArrayName, uSlot).asCharArray(), xmlString.asCharArray());
    }
}

