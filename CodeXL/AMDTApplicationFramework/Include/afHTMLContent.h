//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afHTMLContent.h
///
//==================================================================================

#ifndef __afHTMLContent
#define __afHTMLContent

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


// Enumeration used for icon type of items:
enum afIconType
{
    AF_ICON_NONE,
    AF_ICON_INFO,
    AF_ICON_WARNING1,
    AF_ICON_WARNING2,
    AF_ICON_WARNING3,
    AF_ICON_LIGHT_BULB
};
// ----------------------------------------------------------------------------------
// Class Name:           afHTMLContent
// General Description: Contain an HTML properties content
// Author:               Sigal Algranaty
// Creation Date:        16/12/2009
// ----------------------------------------------------------------------------------
class AF_API afHTMLContent
{
public:

    friend class gdHTMLProperties;
    afHTMLContent(const gtString& title = L"", const gtString& thumbnail = L"", int colspan = 2);
    ~afHTMLContent();

    // Get the HTML content as string:
    void toString(gtString& str);

    void setColspan(int colSpan) {_colspan = colSpan;}
    // Title:
    void setTitle(const gtString& title, const gtString& thumbnail = L"", int colspan = 2);

    void setImageTitle(const gtString& title, const gtString& iconPath);

    // Clear:
    void clear() {_items.deleteElementsAndClear();}

    enum afHTMLContentItemType
    {
        AP_HTML_TITLE,
        AP_HTML_IMAGE_TITLE,
        AP_HTML_SUB_TITLE,
        AP_HTML_ITEM_LINK,
        AP_HTML_LINE,
        AP_HTML_LINE_NO_PADDING,
        AP_HTML_NO_BG_LINE,
        AP_HTML_BOLD_LINE,
        AP_HTML_HEADING,
        AP_HTML_COLOR_SUBTITLE,
        AP_HTML_TOTAL_LINE,
        AP_HTML_SPACE
    };

    // Add item functions:
    void addHTMLItem(afHTMLContentItemType itemType, const gtString& name, const gtString& value = L"", const gtString& thumbnail = L"");
    void addHTMLItem(afHTMLContentItemType itemType, const gtString& name, const gtString& value, int valueRowSpan);
    void addHTMLItem(afHTMLContentItemType itemType, const gtString& name, const gtVector<gtString>& values);
    void addHTMLItemWithColSpan(afHTMLContentItemType itemType, const gtString& name, const gtString& value, int nameColSpan);
    void addHTMLItemWithColor(afHTMLContentItemType itemType, const gtString& name, const gtString& value, const gtString& namecolor);
    void addHTMLTotalLine(int totalValue);
    void addSpaceLine();
    void addSpaceLines(unsigned int count);

    static bool addIconPath(afIconType iconType, const gtString& title, bool isSmall, gtString& htmlWindowString);
    static bool iconImagePathByType(afIconType iconType, bool isSmall, gtString& iconImagePath);

    // Static members:
    void initializeStaticMembers();
    static gtString defaultBackgroundColourAsString() {return _defaultBackgroundColourAsString;};
    static gtString defaultQTFont() {return _defaultQTFont;};

    const static gtString& emptyHTML() {return _static_EmptyHTMLString;}

    static void buildHTMLHeader(gtString& propertiesHTMLMessage);
    static void endHTML(gtString& propertiesHTMLMessage);
    static void classNameByType(afHTMLContentItemType type, gtString& cssClassName);
    class afHTMLContentItem
    {
        friend class afHTMLContent;
    public:
        afHTMLContentItem():
            _name(L""), _thumbnail(L""), _namecolor(L""), _nameColspan(1), _valueColspan(2), _rowspan(1), _useLargeFont(false), _isNameBold(false), _isValueBold(false),
            _totalValue(0), _type(AP_HTML_LINE) {}
        void appendToString(gtString& str);
        void setItemProperties();

    private:

        // Help functions for printing an HTML item:
        void appendTotalLine(gtString& str);
        void appendOneCellRow(gtString& str);
        void appendImageTitleLine(gtString& str);
        void appendDoubleCellRow(gtString& str);
        void appendMultiValuesRow(gtString& str);
        void appendSpaceLine(gtString& str);

    private:
        gtString _name;
        gtVector<gtString> _values;
        gtString _thumbnail;
        gtString _namecolor;
        int _nameColspan;
        int _valueColspan;
        int _rowspan;
        bool _useLargeFont;
        bool _isNameBold;
        bool _isValueBold;
        int _totalValue;
        afHTMLContentItemType _type;
    };

    int size() const {return _items.size();}

private:
    // Static members:
    static bool _static_areStaticInitialized;
    static gtString _defaultBackgroundColourAsString;
    static gtString _defaultQTFont;

    // Empty string:
    static gtString _static_EmptyHTMLString;

    // The HTML items:
    gtPtrVector<afHTMLContentItem*> _items;

    // Amount of cells (default is 2, use this member for non default structure):
    int _colspan;
};


#endif  // __afHTMLContent
