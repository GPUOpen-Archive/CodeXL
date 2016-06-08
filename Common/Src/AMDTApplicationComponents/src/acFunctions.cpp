//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acFunctions.cpp
///
//==================================================================================

//------------------------------ acFunctions.cpp ------------------------------

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    // GTK:
    #include <gtk/gtk.h>
#endif // (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QWebElement>
#include <QWebFrame>
#include <QWebView>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osMessageBox.h>
#include <AMDTOSWrappers/Include/osPipeExecutor.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>

#define TOOLTIP_NEW_LINE "<br>"
#define TOOLTIP_MAX_CHARS_IN_LINE 60

#define AC_MICROSEC_IN_MSEC 1000
#define AC_NANOSEC_IN_MSEC 1000000
#define AC_MSEC_IN_SEC 1000
#define AC_MSEC_IN_MIN 60000
#define AC_SEC_IN_MIN 60
#define AC_MSEC_IN_HOUR 3600000
#define AC_HOURS_IN_DAY 24
#define AC_MSEC_TO_SEC_THRESHOLD_VAL 500


void acLineBreaker::FindPosForLineBreak(const QString& org, unsigned int readIndex, unsigned int& endOfLineIndex, TypeOfEndPosChar& endPosCharType)
{
    const int searchWindowsSize = 20;
    const int searchWindowsHalfSize = 10;
    unsigned int orgLength = org.length();
    bool isFoundEndPos = false;
    const unsigned int UNINITIALIZED_INDEX = 0xFFFF;
    unsigned int commaIndex = UNINITIALIZED_INDEX;
    unsigned int spaceIndex = UNINITIALIZED_INDEX;

    for (unsigned int searchIndex = readIndex + searchWindowsSize; searchIndex > readIndex; --searchIndex)
    {
        if (searchIndex < orgLength)
        {
            if ('.' == org[searchIndex])
            {
                // Found a period. No need to look for anything else
                endOfLineIndex = searchIndex;
                isFoundEndPos = true;
                endPosCharType = PUNCTUATION;
                break;
            }
            else if (',' == org[searchIndex])
            {
                commaIndex = searchIndex;
            }
            else if (' ' == org[searchIndex])
            {
                // Store this position if it is closer to the middle of the search window then the space position we stored previously
                if (abs((int)(searchWindowsHalfSize + spaceIndex - readIndex)) > abs((int)(searchWindowsHalfSize + searchIndex - readIndex)))
                {
                    spaceIndex = searchIndex;
                }
            }
        }
    }

    if (!isFoundEndPos)
    {
        if (commaIndex != UNINITIALIZED_INDEX)
        {
            endOfLineIndex = commaIndex;
            endPosCharType = PUNCTUATION;
            isFoundEndPos = true;
        }
        else if (spaceIndex != UNINITIALIZED_INDEX)
        {
            endOfLineIndex = spaceIndex;
            isFoundEndPos = true;
            endPosCharType = WHITESPACE;
        }
        else
        {
            endOfLineIndex = readIndex + searchWindowsHalfSize;

            if (endOfLineIndex + 1 > orgLength)
            {
                endOfLineIndex = orgLength - 1;
            }

            isFoundEndPos = true;
            endPosCharType = OTHER;
        }
    }
}

void acLineBreaker::AddLineBreaksToTooltipString(const QString& org, QString& dest)
{
    const unsigned int searchStartPosition = 90;

    dest.clear();

    // To be on the safe side, reserve 50% more space for the target text.
    unsigned int orgLength = org.length();
    unsigned int maxLength = (orgLength * 1.5);
    dest.resize(static_cast<unsigned int>(maxLength));

    unsigned int posInLine = 0;
    unsigned int writeIndex = 0;
    TypeOfEndPosChar endPosCharType;

    // Loop through the original text
    for (unsigned int readIndex = 0; readIndex < orgLength;)
    {
        // if the read position is near to where the line should end
        if (posInLine > searchStartPosition)
        {
            unsigned int endOfLineIndex;

            // Look for a period, comma or space to end the line
            FindPosForLineBreak(org, readIndex, endOfLineIndex, endPosCharType);

            // Copy the remainder of the line
            while (readIndex <= endOfLineIndex)
            {
                dest[writeIndex++] = org[readIndex++];
            }

            if (endPosCharType == PUNCTUATION)
            {
                // Do not copy whitespace after a comma or a period
                readIndex++;
            }
            else if (endPosCharType == WHITESPACE)
            {
                // Do not copy whitespace at the end of a line
                writeIndex--;
            }

            // Insert a newline
            dest[writeIndex++] = '<';
            dest[writeIndex++] = 'b';
            dest[writeIndex++] = 'r';
            dest[writeIndex++] = '>';
            posInLine = 0;
        }
        else
        {
            dest[writeIndex++] = org[readIndex++];
            posInLine++;
        }
    }

    dest.chop(maxLength - writeIndex);

    if (dest.endsWith("<br>"))
    {
        dest.chop(4);
    }
}


// ---------------------------------------------------------------------------
// Name:        acGetSystemDefaultBackgroundColor
// Description: Gets the default operation system background color.
// Return Val:  QColor
// Author:      Sigal Algranaty
// Date:        17/6/2012
// ---------------------------------------------------------------------------
QColor acGetSystemDefaultBackgroundColor()
{
    // Create a dummy dialog:
    QGroupBox dummyDialog;

    // Get the system default background color from the dummy dialog:
    QColor retVal = dummyDialog.palette().color(QPalette::Background);

    // Return default system background color:
    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        acGetSystemDefaultBackgroundColorAsHexString
// Description: Returns the system default bg color as a string of hexadecimal
//              numbers (RGB) eg orange would be FF3300
//              (Sigal - moved to AC - to use from components base classes)
// Author:      Uri Shomroni
// Date:        6/8/2008
// ---------------------------------------------------------------------------
gtString acGetSystemDefaultBackgroundColorAsHexString()
{
    QColor sysDefBgCol = acGetSystemDefaultBackgroundColor();

    gtString retVal;
    retVal.appendFormattedString(L"%.2X%.2X%.2X", sysDefBgCol.red(), sysDefBgCol.green(), sysDefBgCol.blue());

    // Check that none of the values are more than 255:
    GT_ASSERT(retVal.length() == 6);

    return retVal;
}

QString acGetSystemDefaultBackgroundColorAsHexQString()
{
    QColor sysDefBgCol = acGetSystemDefaultBackgroundColor();

    QString retVal;
    retVal.sprintf("%.2X%.2X%.2X", sysDefBgCol.red(), sysDefBgCol.green(), sysDefBgCol.blue());

    // Check that none of the values are more than 255:
    GT_ASSERT(retVal.length() == 6);

    return retVal;
}

////////////////////////////////////////////////////////////////////////////////
/// \brief No break line on HTML specific symbol
///
/// \details Some specific symbols treated as 'break-line' (see HTML rfc) and
///          to avoiding this have to around them with special "joining words" symbol
///
/// \param[In] str a refernce to processes string
/// \return processed string
///
/// \autor Vadim Entov
/// \date 01/09/2015
///
QString acSpecificsSymbolsJoining(const QString& str)
{
    QString localStr = str;
    localStr.replace("\\n", "<br>");
    static const QChar breakingSymbols [5] =  { '/', '\\', ':', '-', '+' } ;
    static const int JOINNG_WORDS_CONST = 8288;
    auto arrSize = sizeof(breakingSymbols) / sizeof(*breakingSymbols);

    for (size_t i = 0; i < arrSize; i++)
    {
        QString replaceOn = QChar(JOINNG_WORDS_CONST);
        replaceOn += breakingSymbols[i];
        replaceOn += QChar(JOINNG_WORDS_CONST);

        localStr.replace(breakingSymbols[i], replaceOn);
    }

    return localStr;
}

void acBuildFormattedTooltip(const QString& title, const QString& description, QString& tooltip)
{
    QString localTitle = acSpecificsSymbolsJoining(title);
    QString localDescription = acSpecificsSymbolsJoining(description);

    if (!title.isEmpty() || !description.isEmpty())
    {
        tooltip = "<p style='white-space:nowrap;'>";

        if (!localTitle.isEmpty())
        {
            tooltip += "<b>";
            tooltip += localTitle;
            tooltip.append(":");
            tooltip += "</b><br>";
        }

        if (!localDescription.isEmpty())
        {
            //tooltip += "<qt/>";
            tooltip += localDescription;
        }

        tooltip += "</p>";
    }

}

AC_API void acWrapAndBuildFormattedTooltip(const QString& title, const QString& description, QString& tooltip)
{

    if ((description.indexOf("\\n") < 0) && (description.indexOf("<br>") < 0))
    {
        // This is a single line string, convert it to a multiple line string before formatting it
        // Convert the
        QString descriptionWrapped;
        acLineBreaker::AddLineBreaksToTooltipString(description, descriptionWrapped);
        acBuildFormattedTooltip(title, descriptionWrapped, tooltip);

    }
    else
    {
        acBuildFormattedTooltip(title, description, tooltip);
    }
}

void acBuildFormattedStringByLength(const QString& title, const QString& description, QString& str)
{
    if (!title.isEmpty())
    {
        str = "<b>";
        str += title;

        if (!description.isEmpty())
        {
            str.append(":");
        }

        str += "</b>";
    }

    // Build sentences separated by TOOLTIP_NEW_LINE - max TOOLTIP_MAX_CHARS_IN_LINE characters in a sentence:
    int countSoFar = 0;
    QString line;
    QStringList lines = description.split(" ");

    foreach (QString currentWord, lines)
    {
        if ((countSoFar + currentWord.length()) > TOOLTIP_MAX_CHARS_IN_LINE)
        {
            if (!str.isEmpty())
            {
                str.append(TOOLTIP_NEW_LINE);
            }

            str.append("<nobr>");
            str.append(line);
            str.append("</nobr>");
            line.clear();
            countSoFar = 0;
        }

        if (!line.isEmpty())
        {
            line.append(" ");
        }

        line.append(currentWord);
        countSoFar += currentWord.length();
    }

    if (!line.isEmpty())
    {
        str.append(TOOLTIP_NEW_LINE);
        str.append("<span><nobr>");
        str.append(line);
        str.append("</nobr></span>");
    }
}


// ---------------------------------------------------------------------------
// Name:        acGetSystemDefaultBackgroundColorAsHexString
// Description: Returns the system default bg color as a string of hexadecimal
//              numbers (RGB) eg orange would be FF3300
//              (Sigal - moved to AC - to use from components base classes)
// Author:      Uri Shomroni
// Date:        6/8/2008
// ---------------------------------------------------------------------------
gtString acQColorAsHexString(const QColor& color)
{
    gtString retVal;
    retVal.appendFormattedString(L"%.2X%.2X%.2X", color.red(), color.green(), color.blue());

    // Check that none of the values are more than 255:
    GT_ASSERT(retVal.length() == 6);

    return retVal;
}

QString acQColorAsHexQString(const QColor& color)
{
    QString retVal;
    retVal.sprintf("%.2X%.2X%.2X", color.red(), color.green(), color.blue());

    // Check that none of the values are more than 255:
    GT_ASSERT(retVal.length() == 6);

    return retVal;
}


AC_API bool acExportTableViewToCSV(const QString& outputFilePath, const QTableWidget* pTableWidget)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pTableWidget != NULL)
    {
        // Define a file handle:
        QFile fileHandle(outputFilePath);

        // Open the file for write:
        if (fileHandle.open(QFile::WriteOnly | QFile::Truncate))
        {
            QTextStream data(&fileHandle);
            QStringList strList;

            // Add each of the columns a the list, with a quot for separation:
            for (int colIndex = 0; colIndex < pTableWidget->columnCount(); ++colIndex)
            {
                strList << AC_STR_QuotSpace + pTableWidget->horizontalHeaderItem(colIndex)->data(Qt::DisplayRole).toString() + AC_STR_QuotSpace;
            }

            // Write the string list (containing the headers) to the text stream:
            data << strList.join(";") << AC_STR_NewLineA;

            // Iterate the rows, and for each row, add the columns a the list, with a quot for separation:
            for (int rowIndex = 0; rowIndex < pTableWidget->rowCount(); ++rowIndex)
            {
                strList.clear();

                for (int colIndex = 0; colIndex < pTableWidget->columnCount(); ++colIndex)
                {
                    strList << AC_STR_QuotSpace + pTableWidget->item(rowIndex, colIndex)->text() + AC_STR_QuotSpace;
                }

                // Write the content of the current row to the text stream:
                data << strList.join(";") + AC_STR_NewLineA;
            }

            // Close the file:
            fileHandle.close();

            retVal = true;
        }
    }
    return retVal;
}


AC_API bool acExportHTMLTableToCSV(const QString& outputFilePath, const QWebView* pWebView)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((pWebView != NULL) && (pWebView->page() != NULL))
    {
        QWebFrame* pMainFrame = pWebView->page()->mainFrame();
        GT_IF_WITH_ASSERT(pMainFrame != NULL)
        {
            // Get the table from the web view as web element:
            QWebElement tableElement = pMainFrame->documentElement().findFirst(AC_STR_HTMLTable);

            // Define a file handle:
            QFile fileHandle(outputFilePath);

            // Open the file for write:
            if (fileHandle.open(QFile::WriteOnly | QFile::Truncate))
            {
                QTextStream data(&fileHandle);
                QStringList strList;

                // Add the table headers to the CSV string:
                QWebElementCollection tableHeaders = tableElement.findAll(AC_STR_HTMLTH);

                foreach (QWebElement headerElement, tableHeaders)
                {
                    // Get current row columns:
                    QString colText = headerElement.toPlainText();

                    strList << AC_STR_QuotSpace + colText + AC_STR_QuotSpace;
                }

                data << strList.join(AC_STR_Tab) << AC_STR_NewLineA;
                strList.clear();


                // Get the table rows:
                QWebElementCollection tableRows = tableElement.findAll(AC_STR_HTMLTR);

                foreach (QWebElement rowElement, tableRows)
                {
                    QWebElementCollection tableCols = rowElement.findAll(AC_STR_HTMLTD);

                    for (QWebElement colElement : tableCols)
                    {
                        // Get current row columns:
                        QString colText = colElement.toPlainText();

                        strList << AC_STR_QuotSpace + colText + AC_STR_QuotSpace;
                    }

                    data << strList.join(AC_STR_Tab) << AC_STR_NewLineA;
                    strList.clear();

                }

                // Close the file:
                fileHandle.close();

                retVal = true;
            }
        }
    }

    return retVal;
}

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
// ---------------------------------------------------------------------------
// Name:        acValidateGTKThemeIcons
// Description: Validates the GTK theme icons are installed correctly. displays
//              a message and exits on failure.
// Author:      Uri Shomroni
// Date:        19/3/2012
// ---------------------------------------------------------------------------
void acValidateGTKThemeIcons()
{
    gtString iconValidationCommandLine = L"gtk-update-icon-cache -v ";
    gtString themePath;

    // If we cannot find a theme, we do not wish to stop here, since Qt does the same validation:
    bool themeInitialized = true;

    // When there is no default screen present (sometimes occurs at startup), calling gtk_icon_theme_get_default()
    // can cause a hang:
    if (NULL != gdk_screen_get_default())
    {
        // Get the theme path:
        // NOTE: as GTK does not supply the icon path, we will try to go around this by searching for a specific icon.
        // If we cannot find the representative icon, we can consider it a failure.
        GtkIconTheme* pCurrentTheme = gtk_icon_theme_get_default();

        if (NULL != pCurrentTheme)
        {
            // If we got a theme, we need to make sure its icon cache is valid:
            themeInitialized = false;

            // Get the representative icon:
            GtkIconInfo* pIconInfo = NULL;
            char* pExampleIconName = gtk_icon_theme_get_example_icon_name(pCurrentTheme);

            if (NULL != pExampleIconName)
            {
                // Get the icon data:
                pIconInfo = gtk_icon_theme_lookup_icon(pCurrentTheme, pExampleIconName, -1, (GtkIconLookupFlags)0);
            }

            if (NULL == pIconInfo)
            {
                // If we couldn't get the icon info, try any other icon you can find:
                GList* pIconsList = gtk_icon_theme_list_icons(pCurrentTheme, NULL);

                if (NULL != pIconsList)
                {
                    while (NULL != pIconsList->prev) { pIconsList = pIconsList->prev; }

                    while (NULL != pIconsList)
                    {
                        char* pCurrentIconName = (char*)pIconsList->data;

                        if (NULL == pIconInfo)
                        {
                            pIconInfo = gtk_icon_theme_lookup_icon(pCurrentTheme, pCurrentIconName, -1, (GtkIconLookupFlags)0);
                        }

                        g_free(pIconsList->data);
                        pIconsList = pIconsList->next;
                    }
                }

                g_list_free(pIconsList);
            }

            // If we got an icon:
            if (NULL != pIconInfo)
            {
                // Get the icon path:
                const gchar* pIconPath = gtk_icon_info_get_filename(pIconInfo);

                if (NULL != pIconPath)
                {
                    // Tokenize the path to find the icon theme directory:
                    gtString iconPathAsString;
                    iconPathAsString.fromASCIIString(pIconPath);
                    gtStringTokenizer iconPathTokenizer(iconPathAsString, osFilePath::osPathSeparator);
                    gtString currentToken;
                    bool stopAfterNext = false;
                    static const gtString iconsDirName = L"icons";

                    // Continue until we find the icons directory subdir that is used:
                    while (iconPathTokenizer.getNextToken(currentToken))
                    {
                        // Build the path:
                        if (!currentToken.isEmpty())
                        {
                            // Get only the subpath up to /usr/share/icons/<theme name>/:
                            themePath.append(currentToken).append(osFilePath::osPathSeparator);

                            if (stopAfterNext)
                            {
                                break;
                            }
                            else if (iconsDirName == currentToken)
                            {
                                stopAfterNext = true;
                            }
                        }

                        // Verify the start of the string is valid:
                        if (themePath.isEmpty() || (themePath[0] != '~'))
                        {
                            themePath.prepend(osFilePath::osPathSeparator);
                        }
                    }
                }

                gtk_icon_info_free(pIconInfo);
            }

            g_free(pExampleIconName);
        }
    }

    if ((!themeInitialized) && (!themePath.isEmpty()))
    {
        osPipeExecutor pipeExec;
        gtString validationOutput;
        bool rcExec = pipeExec.executeCommand(iconValidationCommandLine, validationOutput);

        if (rcExec)
        {
            validationOutput.toLowerCase();

            if (-1 == validationOutput.find(L"not found"))
            {
                themeInitialized = true;
            }
        }
    }

    if (!themeInitialized)
    {
        // The cache is not found, we will crash as soon as Qt tries to open a dialog or preload icons:
        gtString errorMessage = L"GTK+ icon cache not found for current theme.\nPlease change the icon theme or run\ngtk-update-icon-cache <theme directory>\nas root and try again.";

        if (!themePath.isEmpty())
        {
            errorMessage.append(L"\n(current theme directory = ").append(themePath).append(')').append('\n');
        }
        else // Theme was not found, give the user a hint:
        {
            errorMessage.append(L"\nThe theme directory is usually /usr/share/icons/<theme name>").append('\n');
        }

        ::printf("%s", errorMessage.asASCIICharArray());
        osMessageBox(L"Error", errorMessage, osMessageBox::OS_STOP_SIGN_ICON);
        ::exit(1);
    }
}


#endif // (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)

void acWideQStringToGTString(const QString& org, gtString& dst)
{
    size_t orgLength = org.length();

    if (orgLength > 0)
    {
        // Resize the string, filling it with null terminating characters if the size has grown
        dst.resize(orgLength);

        // QString::toWCharArray does not place the terminating null character, but std::string does
        // not need it, so we don't need to add it ourselves.
        org.toWCharArray(&dst[0]);
    }
    else
    {
        dst.makeEmpty();
    }
}

void acQStringToOSFilePath(const QString& org, osFilePath& dst)
{
    size_t orgLength = org.length();

    gtString str;

    if (orgLength > 0)
    {
        // Resize the string, filling it with null terminating characters if the size has grown
        str.resize(orgLength);

        // QString::toWCharArray does not place the terminating null character, but std::string does
        // not need it, so we don't need to add it ourselves.
        org.toWCharArray(&str[0]);
    }
    else
    {
        str.makeEmpty();
    }

    // Convert to file path:
    dst.setFullPathFromString(str);
}

QString acGTStringToQString(const gtString& inputStr)
{
    return QString::fromWCharArray(inputStr.asCharArray(), inputStr.length());
}

QString acGTASCIIStringToQString(const gtASCIIString& inputStr)
{
    return QString::fromLatin1(inputStr.asCharArray(), inputStr.length());
}

gtString acQStringToGTString(const QString& inputStr)
{
    gtString str;
    str.resize(inputStr.length());

#if defined(_MSC_VER) && _MSC_VER >= 1400

    // VS2005 crashes if the string is empty
    if (!inputStr.length())
    {
        return str;
    }

#endif

    str.resize(inputStr.toWCharArray(&str[0]));
    return str;
}


void acQStringToWCharArray(const QString& inputStr, wchar_t*& pOutputStr)
{
    int charsAmount = inputStr.toStdWString().size() + 1;
    pOutputStr = new wchar_t[charsAmount];


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    wcscpy_s(pOutputStr, charsAmount, inputStr.toStdWString().c_str());
#else
    wcscpy(pOutputStr, inputStr.toStdWString().c_str());
#endif
}

gtString acDurationAsString(unsigned long duration)
{
    gtString retVal;
    unsigned long seconds = duration;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    minutes = minutes % 60;
    seconds = seconds % 60;

    if (hours > 0)
    {
        retVal.appendFormattedString(L"%d:%02d:%02d", hours, minutes, seconds);
    }
    else if (minutes > 0)
    {
        retVal.appendFormattedString(L"%d:%02d", minutes, seconds);
    }
    else
    {
        retVal.appendFormattedString(L"%d seconds", seconds);
    }

    return retVal;
}

QString acGetFileExtension(const QString& filePath, bool& isExtensionStructureValid)
{
    QString retVal;
    isExtensionStructureValid = true;

    // Split with a dot:
    QStringList extensions = filePath.split(AC_STR_Dot);

    if (extensions.size() > 1)
    {
        // Return the last string:
        retVal = extensions[extensions.length() - 1];

        // Check if there are multiple dots in the string:
        isExtensionStructureValid = (filePath.indexOf("..") < 0);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
QPoint acMapToGlobal(QWidget* pWidget, const QPoint& localPoint)
{
    QPoint globalPoint(localPoint);

    GT_IF_WITH_ASSERT(pWidget != nullptr)
    {
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        // Do normal Qt conversion
        globalPoint = pWidget->mapToGlobal(localPoint);
#else
        // use windows conversion even in SA application
        POINT localPos;
        localPos.x = localPoint.x();
        localPos.y = localPoint.y();

        // Get target HWND
        HWND widgetWin32;
        widgetWin32 = (HWND)pWidget->winId();

        // convert to local position
        ::ClientToScreen(widgetWin32, &localPos);
        QPoint  correctGlobalPos(localPos.x, localPos.y);
        globalPoint = correctGlobalPos;
#endif
    }

    return globalPoint;
}



// ---------------------------------------------------------------------------
// Name:        acGetVersionDetails
// Description: Parses the version number and output the numbers of the version
//              The information is parsed from STRPRODUCTVER, which its structure
//              should be: "[MAJOR],[MINOR],[BUILD],[UPDATE]"
// Arguments:   int& buildVersion
//              int& majorVersion
//              int& minorVersion
//              int& year
//              int& month
//              int& day
// Return Val:  GD_API bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        12/2/2012
// ---------------------------------------------------------------------------
bool acGetVersionDetails(int& buildVersion, int& majorVersion, int& minorVersion, int& year, int& month, int& day)
{
    bool retVal = true;
    osProductVersion appVersion;
    osGetApplicationVersion(appVersion);

    // Initialize return values:
    majorVersion = appVersion._majorVersion;
    minorVersion = appVersion._minorVersion;
    buildVersion = appVersion._patchNumber;
    year = -1;
    month = -1;
    day = -1;

    // Get the current date:
    osTime now;
    now.setFromCurrentTime();
    struct tm timeStruct;
    osTime::TimeZone timeZone = osTime::LOCAL;
    now.timeAsTmStruct(timeStruct, timeZone);
    day = timeStruct.tm_mday;
    month = timeStruct.tm_mon;
    year = timeStruct.tm_year + 1900;

    return retVal;
}


QString NanosecToTimeString(double nanoseconds, bool shouldShowSmallestUnits, bool shouldUseWords, bool millisecondsOnly)
{
    QString retVal = QString::number(0, 'f', 3);

    double msec = nanoseconds / AC_NANOSEC_IN_MSEC;

    if (msec > 0)
    {
        int nKey = (int)msec;
        int mseconds = nKey % AC_MSEC_IN_SEC;
        int seconds = (nKey / AC_MSEC_IN_SEC) % AC_SEC_IN_MIN;
        int minutes = (nKey / AC_MSEC_IN_MIN) % AC_SEC_IN_MIN;
        int hours = (nKey / AC_MSEC_IN_HOUR) % AC_HOURS_IN_DAY;
        int nano = (int)nanoseconds % AC_NANOSEC_IN_MSEC;

        // increment seconds if milliseconds should not be shown and and mseconds >500
        if (!shouldShowSmallestUnits && mseconds > 500)
        {
            ++seconds;
        }

        if (shouldUseWords)
        {
            if (!millisecondsOnly)
            {
                if (hours != 0)
                {
                    retVal = QString::number(hours);
                    retVal += AC_HOURS_POSTFIX;
                }

                if (minutes != 0)
                {
                    retVal += AC_STR_SpaceA;
                    retVal += QString::number(minutes);
                    retVal += AC_MINUTES_POSTFIX;
                }

                if (seconds != 0)
                {
                    retVal += AC_STR_SpaceA;
                    retVal += QString::number(seconds);
                    retVal += AC_SECONDS_POSTFIX;
                }

                if (mseconds != 0)
                {
                    retVal += AC_STR_SpaceA;
                    retVal += QString::number(mseconds);
                }
            }
            else
            {
                if (msec != 0)
                {
                    retVal = QString::number(msec, 'f', 3);
                    retVal += AC_STR_SpaceA;
                    retVal += AC_MSEC_POSTFIX;
                }
            }
        }
        else
        {
            // when mseconds shouldn't be shown
            // to prevent displaying tick step that differs from others ( e.g. 00:00  02:00  03:00  04:00...)
            // if mseconds > 500 set seconds to 1
            if (!shouldShowSmallestUnits && seconds == 0 && mseconds > AC_MSEC_TO_SEC_THRESHOLD_VAL)
            {
                seconds = 1;
            }

            if (!millisecondsOnly && ((seconds > 0) || (minutes > 0) || (hours > 0)))
            {
                retVal.sprintf(AC_NANOSEC_STR_TIME_STRUCTURE, minutes, seconds, mseconds, nano);

                if (minutes == 0)
                {
                    // discard 'mm:' of minutes
                    retVal = retVal.mid(3);
                }

                if (seconds == 0)
                {
                    // discard 'mm:' of seconds
                    retVal = retVal.mid(3);
                }

                if (!shouldShowSmallestUnits)
                {
                    // discard '.zzz' of nanoseconds
                    retVal = retVal.left(retVal.size() - 4);
                }
            }
            else
            {
                retVal = QString::number(msec, 'f', 3);
            }
        }
    }

    return retVal;

}
// ---------------------------------------------------------------------------
// Name:        acNavigationChart::MsecToTimeString
// Description: Returns formatted time string
// Parameters:
//              msec [in] - total amount of milliseconds
//              shouldShowSmallestUnits[in] - flag that shows weather to include or exclude milliseconds from output string
//              shouldUseWords[in] - flag that shows weather hrs min sec msc descriptions should be used after correspondent value
// Author:      Yuri Rshtunique
// Date:        November 2014
// ---------------------------------------------------------------------------
QString MsecToTimeString(double msec, bool shouldShowSmallestUnits, bool shouldUseWords)
{
    QString retVal;

    if (msec > 0)
    {
        int nKey = (int)msec;
        int mseconds = nKey % AC_MSEC_IN_SEC;
        int seconds = (nKey / AC_MSEC_IN_SEC) % AC_SEC_IN_MIN;

        // increment seconds if milliseconds should not be shown and and mseconds >500
        if (!shouldShowSmallestUnits && mseconds > 500)
        {
            ++seconds;
        }

        int minutes = (nKey / AC_MSEC_IN_MIN) % AC_SEC_IN_MIN;
        int hours = (nKey / AC_MSEC_IN_HOUR) % AC_HOURS_IN_DAY;


        if (shouldUseWords)
        {
            if (hours != 0)
            {
                retVal = QString::number(hours);
                retVal += AC_HOURS_POSTFIX;
            }

            if (minutes != 0)
            {
                retVal += AC_STR_SpaceA;
                retVal += QString::number(minutes);
                retVal += AC_MINUTES_POSTFIX;
            }

            if (seconds != 0)
            {
                retVal += AC_STR_SpaceA;
                retVal += QString::number(seconds);
                retVal += AC_SECONDS_POSTFIX;
            }

            if (shouldShowSmallestUnits)
            {
                if (mseconds != 0)
                {
                    retVal += AC_STR_SpaceA;
                    retVal += QString::number(mseconds);
                }
            }
        }
        else
        {
            // when mseconds shouldn't be shown
            // to prevent displaying tick step that differs from others ( e.g. 00:00  02:00  03:00  04:00...)
            // if mseconds > 500 set seconds to 1
            if (!shouldShowSmallestUnits && seconds == 0 && mseconds > AC_MSEC_TO_SEC_THRESHOLD_VAL)
            {
                seconds = 1;
            }

            retVal.sprintf(AC_MSEC_STR_TIME_STRUCTURE, hours, minutes, seconds, mseconds);

            if (hours == 0)
            {
                retVal = retVal.mid(3); // discard 'hh:' of hours
            }

            if (!shouldShowSmallestUnits)
            {
                retVal = retVal.left(retVal.size() - 4);// discard '.zzz' of mseconds
            }
        }
    }

    return retVal;
}

QString MicroSecToTimeString(double microsec, bool shouldShowSmallestUnits, bool shouldUseWords)
{

    QString retVal;
    int msec = microsec / AC_MICROSEC_IN_MSEC;
    int leftMicrosec = (int)microsec % AC_MICROSEC_IN_MSEC;

    if (!shouldShowSmallestUnits)
    {
        retVal = QString::number(msec, 'f', 0);
    }
    else
    {
        retVal.sprintf(AC_MICROSEC_STR_TIME_STRUCTURE, msec, leftMicrosec);
    }

    if (shouldUseWords)
    {
        retVal += AC_STR_SpaceA;
        retVal += AC_MICROSEC_POSTFIX;
    }

    return retVal;
}

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
// ---------------------------------------------------------------------------
// Name:        acGetPushButtonIcon
// Description: service function to load an icon for QPushButton
// Author:      Gilad Yarnitzky
// Date:        9/4/2012
// ---------------------------------------------------------------------------
QIcon acGetPushButtonIcon(QPushButton* pPushButton, const gtString& imagesPath)
{
    gtString iconPath = imagesPath;
    iconPath.append(osFilePath::osPathSeparator);

    if (pPushButton->text() == "&Open")
    {
        iconPath.append(AC_STR_image_open);
    }
    else if (pPushButton->text() == "&Save")
    {
        iconPath.append(AC_STR_image_save);
    }
    else
    {
        iconPath.append(AC_STR_image_cancel);
    }

    QString iconPathQStr(QString::fromWCharArray(iconPath.asCharArray()));

    QIcon returnIcon(iconPathQStr);

    return returnIcon;
};

// ---------------------------------------------------------------------------
// Name:        acGetToolButtonIcon
// Description: service function to load an icon for QToolButton
// Author:      Gilad Yarnitzky
// Date:        9/4/2012
// ---------------------------------------------------------------------------
QIcon acGetToolButtonIcon(QToolButton* pToolButton, const gtString& imagesPath)
{
    gtString iconPath = imagesPath;
    iconPath.append(osFilePath::osPathSeparator);

    if (pToolButton->objectName() == "backButton")
    {
        iconPath.append(AC_STR_image_back);
    }
    else if (pToolButton->objectName() == "forwardButton")
    {
        iconPath.append(AC_STR_image_forward);
    }
    else if (pToolButton->objectName() == "toParentButton")
    {
        iconPath.append(AC_STR_image_toParent);
    }
    else if (pToolButton->objectName() == "newFolderButton")
    {
        iconPath.append(AC_STR_image_newFolder);
    }
    else if (pToolButton->objectName() == "listModeButton")
    {
        iconPath.append(AC_STR_image_listMode);
    }
    else if (pToolButton->objectName() == "detailModeButton")
    {
        iconPath.append(AC_STR_image_detailMode);
    }

    QString iconPathQStr(QString::fromWCharArray(iconPath.asCharArray()));

    QIcon returnIcon(iconPathQStr);

    return returnIcon;
}

// ---------------------------------------------------------------------------
// Name:        acPrepareDialog
// Description: service function to prepare the dialog in linux
// Author:      Gilad Yarnitzky
// Date:        10/4/2012
// ---------------------------------------------------------------------------
void acPrepareDialog(QFileDialog& dialog, const gtString& imagesPath)
{
    // Specific handling of the qtoolbuttons:
    QList<QToolButton*> toolButtonList = dialog.findChildren<QToolButton*>();

    for (int nButton = 0; nButton < toolButtonList.count(); nButton++)
    {
        QToolButton* pCurrentButton = toolButtonList.at(nButton);

        if (pCurrentButton)
        {

            // Replace the toolbutton icon:
            pCurrentButton->setIcon(acGetToolButtonIcon(pCurrentButton, imagesPath));

            // hide the mode buttons:
            if ((pCurrentButton->objectName() == "listModeButton") || (pCurrentButton->objectName() == "detailModeButton"))
            {
                pCurrentButton->hide();
            }
        }
    }

    // Specific handling of the qpushbuttons:
    QList<QPushButton*> pushList = dialog.findChildren<QPushButton*>();

    for (int nPush = 0; nPush < pushList.count(); nPush++)
    {
        // Replace the pushbutton icon:
        pushList.at(nPush)->setIcon(acGetPushButtonIcon(pushList.at(nPush), imagesPath));
    }

    // Specific handling of the qtreeview:
    QList<QTreeView*> treeList = dialog.findChildren<QTreeView*>();

    for (int nTree = 0; nTree < treeList.count(); nTree++)
    {
        treeList.at(nTree)->setHeaderHidden(true);
    }

    //QFileIconProvider globalFileIconProvider;
    //dialog.setIconProvider(&globalFileIconProvider);
}

#endif

QString NanosecToTimeStringFormatted(double nanoseconds, bool shouldShowSmallestUnits, bool shouldUseWords, bool millisecondsOnly)
{
    QString retVal = QString::number(nanoseconds, 'f', 0);

    if (nanoseconds > AC_NANOSEC_IN_MSEC)
    {
        retVal = NanosecToTimeString(nanoseconds, shouldShowSmallestUnits, shouldUseWords, millisecondsOnly);
        retVal += (" ");
        retVal += AC_MSEC_POSTFIX;
    }
    else if (nanoseconds > AC_MICROSEC_IN_MSEC)
    {
        retVal = MicroSecToTimeString(nanoseconds, shouldShowSmallestUnits, true);
    }
    else
    {
        retVal += AC_STR_SpaceA;
        retVal += AC_NSEC_POSTFIX;
    }

    return retVal;
}



bool acIsChildOf(QObject* pChild, QObject* pParent)
{
    bool retVal = false;

    if (pChild != nullptr && pParent != nullptr)
    {
        if (pChild == pParent)
        {
            retVal = true;
        }
        else
        {
            QObject* pTempParent = pChild->parent();

            while (pTempParent != nullptr)
            {
                if (pTempParent == pParent)
                {
                    retVal = true;
                    break;
                }

                pTempParent = pTempParent->parent();
            }
        }
    }

    return retVal;
}