//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afHTMLContent.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QFontInfo>
#include <QTableWidget>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>


// Static members
gtString afHTMLContent::_static_EmptyHTMLString = AF_STR_Empty;
gtString afHTMLContent::_defaultBackgroundColourAsString = AF_STR_Empty;
gtString afHTMLContent::_defaultQTFont = AF_STR_Empty;
bool afHTMLContent::_static_areStaticInitialized = false;

// Colors definitions:
#define AF_Color_TitleBG L"BBDDFF"
#define AF_Color_SecondaryTitleBG L"CCCCCC"

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    #define AF_Color_SimpleLineBG L"FFFFFF"
#else
    #define AF_Color_SimpleLineBG L"F5F5F5"
#endif


// HTML String definitions. These string are only supposed to be used at this class:
#define AF_STR_HtmlHtmlTagStart L"<html><head><style type='text/css'>%ls</style></head><body>"
#define AF_STR_HtmlHeaderStyleCssDefaultFont L"body {background-color: #%ls; font-size:medium; } table td { margin: 2px;}"
#define AF_STR_HtmlHeaderStyleCss\
    L"body {background-color: #%ls; font-size:medium; font-family:%ls;} table tr {} table td { padding: 4px 0px 4px 0px; } "\
    L"table td.tdLeft{width:'*';} table td.tdRight{} table td.tdTitle{background-color: #BCC8D8;} table td.TdTotal{background-color: #f5f5f5;}\n"\
    L"table td.TdImage{} table td.TdSubTitle{} table td.TdLink{} table td.TdLine{background-color: #f5f5f5;}\n"\
    L"table td.TdLineNoPadding{} table td.TdLineNoBG{} table td.TdLineBold{} table td.TdHeading{}\n"\
    L"table td.TdColorSubtitle{} table td.TdSpace{}\n"

#define AF_STR_HtmlHtmlTagEnd L"</body></html>"
#define AF_STR_HtmlEmptyLineWithColspan L"<tr><td colspan='%u'></td></tr>"
#define AF_STR_HtmlTrTdStartClassName L"<tr><td align=right class=%ls>"
#define AF_STR_HtmlTrTdStartClassNameColSpan L"<tr><td colspan='%d' class=%ls>"
#define AF_STR_HtmlTableNoPadding L"<table width=100% cellspacing=0>"

// CSS class names:
#define AF_STR_HtmlTdTitleClassName L"tdTitle"
#define AF_STR_HtmlTdImageClassName L"TdImage"
#define AF_STR_HtmlTdSubTitleClassName L"TdSubTitle"
#define AF_STR_HtmlTdLinkClassName L"TdLink"
#define AF_STR_HtmlTdLineClassName L"TdLine"
#define AF_STR_HtmlTdLineNoPaddingClassName L"TdLineNoPadding"
#define AF_STR_HtmlTdLineNoBGClassName L"TdLineNoBG"
#define AF_STR_HtmlTdLineBoldClassName L"TdLineBold"
#define AF_STR_HtmlTdHeadingClassName L"TdHeading"
#define AF_STR_HtmlTdColorSubtitleClassName L"TdColorSubtitle"
#define AF_STR_HtmlTdTotalClassName L"TdTotal"
#define AF_STR_HtmlTdSpaceClassName L"TdSpace"

// ---------------------------------------------------------------------------
// Name:        afHTMLContent::afHTMLContent
// Description:
// Arguments: const gtString& title - the properties table title
//            const gtString& thumbnail - the thumbnail string
//            int amountOfCells - amount of cells in the table
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
afHTMLContent::afHTMLContent(const gtString& title, const gtString& thumbnail, int colspan)
{
    // Initialize static items:
    if (!_static_areStaticInitialized)
    {
        initializeStaticMembers();
    }

    // Set the amount of cells:
    _colspan = colspan;

    // Add the title:
    if (!title.isEmpty())
    {
        addHTMLItem(AP_HTML_TITLE, title, AF_STR_Empty, thumbnail);
    }
}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::~afHTMLContent
// Description: Clear the HTML item
// Return Val:
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
afHTMLContent::~afHTMLContent()
{
    // Clear the HTML items:
    _items.deleteElementsAndClear();

}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::setTitle
// Description: Sets the HTML title
// Arguments: const gtString& title
//            , afIconType iconType - the icon type for the title
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::setTitle(const gtString& title, const gtString& thumbnail, int colspan)
{
    // Initialize static items:
    if (!_static_areStaticInitialized)
    {
        initializeStaticMembers();
    }

    // Set the amount of cells:
    _colspan = colspan;

    // Add the title:
    if (!title.isEmpty())
    {
        addHTMLItem(AP_HTML_TITLE, title, AF_STR_Empty, thumbnail);
    }
}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::setImageTitle
// Description: Set an image title to the HTML table
// Arguments: const gtString& title
//            const& gtString iconPath
// Return Val: void
// Author:      Sigal Algranaty
// Date:        7/1/2010
// ---------------------------------------------------------------------------
void afHTMLContent::setImageTitle(const gtString& title, const gtString& iconPath)
{
    // Add the title:
    addHTMLItem(AP_HTML_IMAGE_TITLE, title, AF_STR_Empty, iconPath);
}
// ---------------------------------------------------------------------------
// Name:        afHTMLContent::initializeStaticMembers
// Description: Initializes static members
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::initializeStaticMembers()
{
    // Get the system default background color (to be used in the HTML ctrl).
    _defaultBackgroundColourAsString = acGetSystemDefaultBackgroundColorAsHexString();

    // Create a text editor, in order to get Qt default font family:
    QTableWidgetItem widgetItem;
    QFontInfo fontInfo(widgetItem.font());
    _defaultQTFont.fromASCIIString(fontInfo.family().toLatin1());

    // Remove the leading '#' from the string if present:
    if (_defaultBackgroundColourAsString[0] == '#')
    {
        _defaultBackgroundColourAsString.truncate(1, _defaultBackgroundColourAsString.length() - 1);
    }

    // Initialize empty HTML string:
    gtString style;
    style.appendFormattedString(AF_STR_HtmlHeaderStyleCss, _defaultBackgroundColourAsString.asCharArray(), _defaultQTFont.asCharArray());
    _static_EmptyHTMLString.appendFormattedString(AF_STR_HtmlHtmlTagStart AF_STR_HtmlHtmlTagEnd, style.asCharArray());

    _static_areStaticInitialized = true;
}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::addHTMLItem
// Description: Adds an HTML item
// Arguments: afHTMLContentItemType itemType
//            const gtString& name
//            const gtString& value
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::addHTMLItem(afHTMLContentItemType itemType, const gtString& name, const gtString& value, const gtString& thumbnail)
{
    // Add space before before subtitle item:
    if (itemType == AP_HTML_COLOR_SUBTITLE)
    {
        addSpaceLine();
    }

    // Allocate the new content item:
    afHTMLContentItem* pNewContentItem = new afHTMLContentItem;


    // Set the item attributes:
    pNewContentItem->_type = itemType;
    pNewContentItem->_name = name;

    if (!value.isEmpty())
    {
        pNewContentItem->_values.push_back(value);
    }

    pNewContentItem->_thumbnail = thumbnail;
    pNewContentItem->_valueColspan = _colspan;

    // Add the item:
    _items.push_back(pNewContentItem);
}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::addHTMLItemWithColor
// Description: Adds an HTML item
// Arguments: afHTMLContentItemType itemType
//            const gtString& name
//            const gtString& value
//            const gtString& namecolor
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::addHTMLItemWithColor(afHTMLContentItemType itemType, const gtString& name, const gtString& value, const gtString& namecolor)
{
    // Allocate the new content item:
    afHTMLContentItem* pNewContentItem = new afHTMLContentItem;


    // Set the item attributes:
    pNewContentItem->_type = itemType;
    pNewContentItem->_name = name;

    if (!value.isEmpty())
    {
        pNewContentItem->_values.push_back(value);
    }

    pNewContentItem->_namecolor = namecolor;
    pNewContentItem->_valueColspan = _colspan;

    // Add the item:
    _items.push_back(pNewContentItem);
}

// ---------------------------------------------------------------------------
// Name:        afHTMLContent::addHTMLItem
// Description: Adds an HTML item
// Arguments: afHTMLContentItemType itemType
//            const gtString& name
//            const gtString& value
//            const gtString& namecolor
//            const gtString& valuecolor
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::addHTMLItem(afHTMLContentItemType itemType, const gtString& name, const gtVector<gtString>& values)
{
    // Add space before before subtitle item:
    if (itemType == AP_HTML_COLOR_SUBTITLE)
    {
        addSpaceLine();
    }

    // Allocate the new content item:
    afHTMLContentItem* pNewContentItem = new afHTMLContentItem;


    // Set the item attributes:
    pNewContentItem->_type = itemType;
    pNewContentItem->_name = name;
    GT_IF_WITH_ASSERT((int)values.size() == _colspan - 1)
    {
        for (int i = 0; i < (int)values.size(); i++)
        {
            pNewContentItem->_values.push_back(values[i]);
        }
    }

    pNewContentItem->_valueColspan = _colspan;

    // Add the item:
    _items.push_back(pNewContentItem);

}
void afHTMLContent::addHTMLItemWithColSpan(afHTMLContentItemType itemType, const gtString& name, const gtString& value, int nameColSpan)
{
    // Add space before before subtitle item:
    if (itemType == AP_HTML_COLOR_SUBTITLE)
    {
        addSpaceLine();
    }

    // Allocate the new content item:
    afHTMLContentItem* pNewContentItem = new afHTMLContentItem;


    // Set the item attributes:
    pNewContentItem->_type = itemType;
    pNewContentItem->_name = name;

    if (!value.isEmpty())
    {
        pNewContentItem->_values.push_back(value);
    }

    // We add 1 since the value requires 1 col span:
    pNewContentItem->_nameColspan = nameColSpan;

    // Add the item:
    _items.push_back(pNewContentItem);

}

void afHTMLContent::addHTMLItem(afHTMLContentItemType itemType, const gtString& name, const gtString& value, int valueRowSpan)
{
    // Add space before before subtitle item:
    if (itemType == AP_HTML_COLOR_SUBTITLE)
    {
        addSpaceLine();
    }

    // Allocate the new content item:
    afHTMLContentItem* pNewContentItem = new afHTMLContentItem;


    // Set the item attributes:
    pNewContentItem->_type = itemType;
    pNewContentItem->_name = name;

    if (!value.isEmpty())
    {
        pNewContentItem->_values.push_back(value);
    }

    pNewContentItem->_valueColspan = _colspan;
    pNewContentItem->_rowspan = valueRowSpan;

    // Add the item:
    _items.push_back(pNewContentItem);

}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::addHTMLTotalLine
// Description: Adds a total line to an HTML properties table
// Arguments: int totalValue
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::addHTMLTotalLine(int totalValue)
{
    // Allocate the new content item:
    afHTMLContentItem* pNewContentItem = new afHTMLContentItem;


    // Set the item attributes:
    pNewContentItem->_type = AP_HTML_TOTAL_LINE;
    pNewContentItem->_totalValue = totalValue;

    // Add the item:
    _items.push_back(pNewContentItem);

}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::addSpaceLine
// Description: Add an HTML space line
// Arguments: int height
// Return Val: void
// Author:      Sigal Algranaty
// Date:        7/1/2010
// ---------------------------------------------------------------------------
void afHTMLContent::addSpaceLine()
{
    // Allocate the new content item:
    afHTMLContentItem* pSpaceItem = new afHTMLContentItem;

    pSpaceItem->_type = AP_HTML_SPACE;
    pSpaceItem->_valueColspan = _colspan;
    _items.push_back(pSpaceItem);
}

// ---------------------------------------------------------------------------
// Name:        afHTMLContent::addSpaceLines
// Description: Add multiple HTML space line
// Arguments: unsigned int count
// Author:      Uri Shomroni
// Date:        19/7/2015
// ---------------------------------------------------------------------------
void afHTMLContent::addSpaceLines(unsigned int count)
{
    for (unsigned int i = 0; count > i; ++i)
    {
        addSpaceLine();
    }
}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::toString
// Description: Get the HTML content as string
// Return Val: const gtString&
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::toString(gtString& str)
{
    // Open an HTML tag:
    str.makeEmpty();

    // Initialize empty HTML string:
    gtString style;
    style.appendFormattedString(AF_STR_HtmlHeaderStyleCss, _defaultBackgroundColourAsString.asCharArray(), _defaultQTFont.asCharArray());
    str.appendFormattedString(AF_STR_HtmlHtmlTagStart, style.asCharArray());

    // Open an HTML table:
    str.append(AF_STR_HtmlTableNoPadding);

    // Go through the item and add them to the string:
    for (int i = 0; i < (int)_items.size(); i++)
    {
        // Get current item:
        afHTMLContentItem* pItem = _items[i];
        GT_IF_WITH_ASSERT(pItem != nullptr)
        {
            // Add the item to the string:
            pItem->appendToString(str);
        }
    }

    // Close the table and the HTML tag:
    str.append(AF_STR_HtmlTableTagEnd AF_STR_HtmlHtmlTagEnd);

}


// ---------------------------------------------------------------------------
// Name:        afHTMLContentItem::appendToString
// Description: Append the item to a string
// Arguments: gtString& str
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::afHTMLContentItem::appendToString(gtString& str)
{
    // Set the item background color by its type:
    setItemProperties();

    if (_type == AP_HTML_TOTAL_LINE)
    {
        appendTotalLine(str);
    }
    else if (_type == AP_HTML_IMAGE_TITLE)
    {
        appendImageTitleLine(str);
    }
    else if (_type == AP_HTML_SPACE)
    {
        appendSpaceLine(str);
    }

    // Simple table row (no value):
    else if (_values.size() == 0)
    {
        appendOneCellRow(str);
    }
    // One value table row:
    else if (_values.size() == 1)
    {
        appendDoubleCellRow(str);
    }
    // More then one value table row:
    else if (_values.size() > 1)
    {
        appendMultiValuesRow(str);
    }
}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::classByType
// Description: Return a class name according to a line type
// Arguments:   afHTMLContentItemType type
//              gtString& cssClassName
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        11/12/2011
// ---------------------------------------------------------------------------
void afHTMLContent::classNameByType(afHTMLContentItemType type, gtString& cssClassName)
{
    switch (type)
    {
        case AP_HTML_TITLE:
            cssClassName = AF_STR_HtmlTdTitleClassName;
            break;

        case AP_HTML_IMAGE_TITLE:
            cssClassName = AF_STR_HtmlTdImageClassName;
            break;

        case AP_HTML_SUB_TITLE:
            cssClassName = AF_STR_HtmlTdSubTitleClassName;
            break;

        case AP_HTML_ITEM_LINK:
            cssClassName = AF_STR_HtmlTdLinkClassName;
            break;

        case AP_HTML_LINE:
            cssClassName = AF_STR_HtmlTdLineNoBGClassName;
            break;

        case AP_HTML_LINE_NO_PADDING:
            cssClassName = AF_STR_HtmlTdLineNoPaddingClassName;
            break;

        case AP_HTML_NO_BG_LINE:
            cssClassName = AF_STR_HtmlTdLineNoBGClassName;
            break;

        case AP_HTML_BOLD_LINE:
            cssClassName = AF_STR_HtmlTdLineBoldClassName;
            break;

        case AP_HTML_HEADING:
            cssClassName = AF_STR_HtmlTdHeadingClassName;
            break;

        case AP_HTML_COLOR_SUBTITLE:
            cssClassName = AF_STR_HtmlTdColorSubtitleClassName;
            break;

        case AP_HTML_TOTAL_LINE:
            cssClassName = AF_STR_HtmlTdTotalClassName;
            break;

        case AP_HTML_SPACE:
            cssClassName = AF_STR_HtmlTdSpaceClassName;
            break;

        default:
            GT_ASSERT(false);
            break;
    }
}
// ---------------------------------------------------------------------------
// Name:        afHTMLContent::afHTMLContentItem::setItemProperties
// Description: Sets the item properties according to its type
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::afHTMLContentItem::setItemProperties()
{
    // If the item background color, do nothing:
    switch (_type)
    {
        case AP_HTML_IMAGE_TITLE:
            _isNameBold = true;
            _useLargeFont = true;
            break;

        case AP_HTML_TITLE:
            _isNameBold = true;
            _useLargeFont = true;
            break;

        case AP_HTML_COLOR_SUBTITLE:
            _isNameBold = true;
            _useLargeFont = false;
            break;

        case AP_HTML_SUB_TITLE:
            _isNameBold = true;
            _isValueBold = true;
            _useLargeFont = false;
            break;

        case AP_HTML_ITEM_LINK:
            _isNameBold = false;
            _isValueBold = false;
            _useLargeFont = false;
            break;

        case AP_HTML_HEADING:
            _isNameBold = true;
            _isValueBold = true;
            _useLargeFont = false;
            break;

        case AP_HTML_LINE_NO_PADDING:
        case AP_HTML_LINE:
            _isNameBold = false;
            break;

        case AP_HTML_NO_BG_LINE:
            _isNameBold = false;
            break;

        case AP_HTML_BOLD_LINE:
            _isNameBold = true;
            break;

        case AP_HTML_TOTAL_LINE:
            break;

        case AP_HTML_SPACE:
            break;

        default:
            GT_ASSERT_EX(false, L"unsupported HTML item type");
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        afHTMLContent::appendTotalLine
// Description: Builds an HTML properties total line string
// Arguments: gtString& str
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::afHTMLContentItem::appendTotalLine(gtString& str)
{
    // Build the total number as string:
    gtString totalAmountStr;
    totalAmountStr.appendFormattedString(L"%d", _totalValue);

    // Get the CSS class name for this line:
    gtString className;
    afHTMLContent::classNameByType(_type, className);

    // Open a cell with class name according to type:
    str.appendFormattedString(AF_STR_HtmlTrTdStartClassName, className.asCharArray());

    // Add the total fixed string:
    str += AF_STR_HtmlPropertiesTotal;
    str += AF_STR_HtmlPropertiesTableColEndHighlightedCell;
    str += AF_STR_HtmlPropertiesLeftTagStart;
    str += totalAmountStr;
    str += AF_STR_HtmlPropertiesLeftTagEnd;
    str += AF_STR_HtmlPropertiesTableRowEnd;
}

// ---------------------------------------------------------------------------
// Name:        afHTMLContent::afHTMLContentItem::appendImageTitleLine
// Description: Append an image title line to the HTML string
// Arguments: gtString& str
// Return Val: void
// Author:      Sigal Algranaty
// Date:        7/1/2010
// ---------------------------------------------------------------------------
void afHTMLContent::afHTMLContentItem::appendImageTitleLine(gtString& str)
{
    // Add the icon:
    str.appendFormattedString(L"<tr><td colspan=%d><img src=\"%ls\" /><font size=+2><b>&nbsp;&nbsp;%ls</b></font><br></td></tr>", _valueColspan, _thumbnail.asCharArray(), _name.asCharArray());
}
// ---------------------------------------------------------------------------
// Name:        afHTMLContent::afHTMLContentItem::appendOneCellRow
// Description: Build a one cell HTML table row
// Arguments: gtString& str
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::afHTMLContentItem::appendOneCellRow(gtString& str)
{
    // Get class name by type:
    gtString className;
    classNameByType(_type, className);

    // Open a table cell:
    str.appendFormattedString(AF_STR_HtmlTrTdStartClassNameColSpan, _valueColspan, className.asCharArray());

    // Add a font tag with the name:
    if (_useLargeFont)
    {
        str.append(AF_STR_HtmlFontTagStart);
    }

    if (_isNameBold)
    {
        str.append(AF_STR_HtmlBoldTagStart);
    }

    // If the value should be printed with color, print the value color:
    if (!_namecolor.isEmpty())
    {
        _name.prependFormattedString(L"<FONT COLOR=%ls><B>", _namecolor.asCharArray());
        _name.append(L"</b></font>");
    }

    str.append(_name);

    if (_isNameBold)
    {
        str.append(AF_STR_HtmlBoldTagEnd);
    }

    if (_useLargeFont)
    {
        str.append(AF_STR_HtmlFontTagEnd);
    }

    // Close the table cell:
    str += AF_STR_HtmlTDEnd;

    // For thumbnail, open an HTML table that will contain both the properties table in the left cell,
    // and the thumbnail in the right cell:
    if (!_thumbnail.isEmpty())
    {
        str += L"<td rowspan='10' valign='top' align='center' cellpadding='5' width='1%%'>";
        str += _thumbnail;
        str += AF_STR_HtmlTDEnd;
    }

    // Close the table row:
    str += AF_STR_HtmlTREnd;
}

// ---------------------------------------------------------------------------
// Name:        afHTMLContent::afHTMLContentItem::appendDoubleCellRow
// Description: Build a double cell HTML table row
// Arguments: gtString& str
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::afHTMLContentItem::appendDoubleCellRow(gtString& str)
{
    // If the value is empty, give it a non breaking space value:
    gtString valueStr;

    if (_values.size() > 0)
    {
        valueStr = _values[0];
    }
    else
    {
        valueStr = AF_STR_HtmlPropertiesNonbreakingSpace;
    }

    if (_name.isEmpty())
    {
        _name = AF_STR_HtmlPropertiesNonbreakingSpace;
    }

    int colspan2 = _valueColspan - 1;

    gtString className = (_type == AP_HTML_TITLE) ? L"tdTitle" : L"tdLeft";

    // Add the property and value to the table:
    str.appendFormattedString(L"<tr valign=top><td class=%ls valign=top colspan=%d>", className.asCharArray(), _nameColspan);

    if (_isNameBold)
    {
        str.append(AF_STR_HtmlBoldTagStart);
    }

    str.append(_name);

    if (_type == AP_HTML_TITLE)
    {
        // Both name and value should be with bg:
        _isValueBold = true;
    }

    className = (_type == AP_HTML_TITLE) ? L"tdTitle" : L"tdRight";
    str.appendFormattedString(L"</td><td valign=top class=%ls colspan=%d rowspan=%d>", className.asCharArray(), colspan2, _rowspan);

    if (_isNameBold)
    {
        str.append(AF_STR_HtmlBoldTagEnd);
    }

    if (_isValueBold)
    {
        str.append(AF_STR_HtmlBoldTagStart);
    }

    str.append(valueStr);

    if (_isValueBold)
    {
        str.append(AF_STR_HtmlBoldTagEnd);
    }

    str.append(AF_STR_HtmlPropertiesTableRowEnd);
}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::afHTMLContentItem::appendMultiValuesRow
// Description: Build a row with multi values cell
// Arguments: gtString& str
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::afHTMLContentItem::appendMultiValuesRow(gtString& str)
{
    // If the value is empty, give it a non breaking space value:
    gtString valueStr;
    GT_IF_WITH_ASSERT(_values.size() > 0)
    {
        // If the value should be printed with color, print the value color:
        if (!_namecolor.isEmpty())
        {
            _name.prependFormattedString(L"<FONT COLOR=%ls><B>", _namecolor.asCharArray());
            _name.append(L"</b></font>");
        }

        if (_name.isEmpty())
        {
            _name = AF_STR_HtmlPropertiesNonbreakingSpace;
        }

        // Open a table row:
        // Add the property and value to the table:
        str.append(L"<tr valign=top><td width='1%%'>");

        // Add the name to the table row:
        if (_isNameBold)
        {
            str.append(AF_STR_HtmlBoldTagStart);
        }

        str.append(_name);

        if (_isNameBold)
        {
            str.append(AF_STR_HtmlBoldTagEnd);
        }

        // Close the name cell:
        str.append(AF_STR_HtmlTDEnd);

        for (int i = 0; i < (int)_values.size(); i++)
        {
            // Set the value string:
            valueStr = _values[i];

            // Open current table cell for this value:
            str.append(L"<td width='*'>");

            if (_isValueBold)
            {
                str.append(AF_STR_HtmlBoldTagStart);
            }

            str.append(valueStr);

            if (_isValueBold)
            {
                str.append(AF_STR_HtmlBoldTagEnd);
            }

            // Close the current value cell:
            str.append(AF_STR_HtmlTDEnd);
        }

        // Close the table row tag:
        str.append(AF_STR_HtmlTREnd);
    }
}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::addSpaceLine
// Description: Add a space line
// Arguments: gtString& str
// Return Val: void
// Author:      Sigal Algranaty
// Date:        30/12/2009
// ---------------------------------------------------------------------------
void afHTMLContent::afHTMLContentItem::appendSpaceLine(gtString& str)
{
    str.appendFormattedString(AF_STR_HtmlEmptyLine, _valueColspan);
}


// ---------------------------------------------------------------------------
// Name:        afHTMLContent::addIconPath
// Description: Build an image path according to icon id
// Arguments: unsigned int iconId
//            const gtString& title - the image tooltip
//            bool isSmall - should the icon be added as small icon
//            gtString& htmlWindowString - the HTML string to add the icon to
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/3/2009
// ---------------------------------------------------------------------------
bool afHTMLContent::addIconPath(afIconType iconType, const gtString& title, bool isSmall, gtString& htmlWindowString)
{
    bool retVal = false;

    // Get the icon path according to the icon type:
    gtString iconPath;
    retVal = iconImagePathByType(iconType, isSmall, iconPath);
    GT_IF_WITH_ASSERT(retVal)
    {
        // Append the icon to the HTML window string:
        htmlWindowString.appendFormattedString(AF_STR_HtmlImage2, iconPath.asCharArray(), title.asCharArray());

        // Mark our success:
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afHTMLContent::iconImagePathByType
// Description: Return an icon image path according to icon type
// Arguments:   afIconType iconType
//              bool isSmall - should the icon be small
//              gtString& iconImagePath
// Return Val:  gtString
// Author:      Sigal Algranaty
// Date:        14/7/2010
// ---------------------------------------------------------------------------
bool afHTMLContent::iconImagePathByType(afIconType iconType, bool isSmall, gtString& iconImagePath)
{
    // Get application images path:
    bool retVal = afGetApplicationImagesPath(iconImagePath);
    iconImagePath.append(osFilePath::osPathSeparator);

    // Append icon file name according to icon index:
    switch (iconType)
    {
        case AF_ICON_INFO:
        {
            // Information icon:
            if (isSmall)
            {
                iconImagePath.append(AF_STR_InformationIconSmallFileName);
            }
            else
            {
                iconImagePath.append(AF_STR_InformationIconFileName);
            }
        }
        break;

        case AF_ICON_WARNING1:
        {
            // Warning level 1 icon:
            if (isSmall)
            {
                iconImagePath.append(AF_STR_WarningYellowIconSmallFileName);
            }
            else
            {
                iconImagePath.append(AF_STR_WarningYellowIconFileName);
            }
        }
        break;

        case AF_ICON_WARNING2:
        {
            // Warning level 2 icon:
            if (isSmall)
            {
                iconImagePath.append(AF_STR_WarningOrangeIconSmallFileName);
            }
            else
            {
                iconImagePath.append(AF_STR_WarningOrangeIconFileName);
            }
        }
        break;

        case AF_ICON_WARNING3:
        {
            // Warning level 4 icon:
            if (isSmall)
            {
                iconImagePath.append(AF_STR_WarningRedIconSmallFileName);
            }
            else
            {
                iconImagePath.append(AF_STR_WarningRedIconFileName);
            }
        }
        break;

        case AF_ICON_LIGHT_BULB:
        {
            // Light bulb icon:
            if (isSmall)
            {
                iconImagePath.append(AF_STR_LightBulbSmallFileName);
            }
            else
            {
                GT_ASSERT_EX(0, L"No big icon for light bulb!")
            }
        }
        break;

        case AF_ICON_NONE:
            // No icon:
            break;

        default:
        {
            // passed the wrong argument to the function
            GT_ASSERT(false);
            retVal = false;
        }
    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        afHTMLContent::buildHTMLHeader
// Description: Build an HTML string header
// Arguments:   gtString& htmlStr
// Author:      Sigal Algranaty
// Date:        11/12/2011
// ---------------------------------------------------------------------------
void afHTMLContent::buildHTMLHeader(gtString& htmlStr)
{
    // Build HTML style string with background:
    gtString style;
    style.appendFormattedString(AF_STR_HtmlHeaderStyleCss, afHTMLContent::defaultBackgroundColourAsString().asCharArray(), afHTMLContent::defaultQTFont().asCharArray());

    // Initialize HTML header string:
    htmlStr.appendFormattedString(AF_STR_HtmlHtmlTagStart, style.asCharArray());
}

// ---------------------------------------------------------------------------
// Name:        afHTMLContent::endHTML
// Description: End an HTML tag
// Arguments:   gtString& htmlStr
// Author:      Sigal Algranaty
// Date:        11/12/2011
// ---------------------------------------------------------------------------
void afHTMLContent::endHTML(gtString& htmlStr)
{
    htmlStr.append(AF_STR_HtmlHtmlTagEnd);
}


