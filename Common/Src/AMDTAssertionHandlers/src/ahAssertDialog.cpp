//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ahAssertDialog.cpp
///
//==================================================================================

// Qt
//#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <inc/ahCommandIDs.h>
#include <AMDTAssertionHandlers/Include/ahAssertDialog.h>

// Linux:
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    // We use xpm icons in Linux and Mac instead of packaged resources
    #include <AMDTApplicationFramework/Include/res/icons/ApplicationIcon_64_xpm.xpm>

#endif

// ---------------------------------------------------------------------------
// Name:        ahAssertDialog::ahAssertDialog
// Description: Constructor
// Arguments:   label - The text that will be displayed to the user.
//              urlToBeLaunched - The URL to be launched when pressing the text label.
//              pParent - This static text parent window.
//              id - The id of this static text
//              position, size - The static text position and size.
//              style - The static text style.
// Author:      Avi Shapira
// Date:        28/1/2007
// ---------------------------------------------------------------------------
ahAssertDialog::ahAssertDialog(const osFilePath& filePath, const int lineNumber, const gtASCIIString& assertReason, QWidget* pParentWidget)
    : _filePath(filePath), _lineNumber(lineNumber), _assertReason(assertReason), _pParentWidget(pParentWidget)

{
    // Add the Icon to the dialog
    loadIcon();

    // Add the dialog title:
    setDialogTitle();

    // Set the standard buttons:
    setStandardButtons(QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);

    // Override the buttons text for accelerators:
    QAbstractButton* pButton = button(QMessageBox::Retry);

    if (pButton != NULL)
    {
        pButton->setText("&Debug");
    }

    pButton = button(QMessageBox::Abort);

    if (pButton != NULL)
    {
        pButton->setText("E&xit");
    }

    pButton = button(QMessageBox::Ignore);

    if (pButton != NULL)
    {
        pButton->setText("&Ignore");
        pButton->setFocus();
    }

    // Add ignore all button:
    addButton("Ignore &All", ActionRole);

    // Fill the assertion message body:
    fillAssertionMessageBody();

    // Set the window modality:
    setWindowModality(Qt::ApplicationModal);

}



// ---------------------------------------------------------------------------
// Name:        ahAssertDialog::exec
// Description:
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        3/1/2012
// ---------------------------------------------------------------------------
int ahAssertDialog::exec()
{
    int retVal = 0;

    // Call the message box execution:
    retVal = QMessageBox::exec();

    // Convert the user decision to a handling id:
    switch (retVal)
    {

        case QMessageBox::Abort:
        {
            retVal = ID_AH_ASSERT_EXIT_BUTTON;
        }
        break;

        case QMessageBox::Ignore:
        {
            retVal = ID_AH_ASSERT_IGNORE_BUTTON;
        }
        break;

        case QMessageBox::Retry:
        {
            retVal = ID_AH_ASSERT_DEBUG_BUTTON;
        }
        break;

        default:
        {
            retVal = ID_AH_ASSERT_IGNORE_ALL_BUTTON;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        ahAssertDialog::~ahAssertDialog
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        9/7/2007
// ---------------------------------------------------------------------------
ahAssertDialog::~ahAssertDialog()
{
}



// ---------------------------------------------------------------------------
// Name:        ahAssertDialog::fillAssertionMessageBody
// Description: Fills the text control that contains the assertion message body
// Arguments: pMessageBody - The text control to be filled.
// Author:      Yaki Tebeka
// Date:        3/2/2009
// ---------------------------------------------------------------------------
void ahAssertDialog::fillAssertionMessageBody()
{
    // Build an HTML string for the text:
    gtASCIIString htmlText;

    // Calculate the file name string:
    gtString fileName;
    _filePath.getFileNameAndExtension(fileName);
    gtASCIIString fileNameString = fileName.asASCIICharArray();

    // Calculate the file path string:
    gtASCIIString filePathString = _filePath.asString().asASCIICharArray();

    // Calculate the line number string:
    gtASCIIString lineNumberAsString;
    lineNumberAsString.appendFormattedString("%d", _lineNumber);


    htmlText.append("<html><head></head><body>");
    htmlText.append("<font size=5 color=blue>Debug Assertion Failed!</font><br><br>");
    htmlText.append("<b><font size=4>File Name: </font></b>");
    htmlText.appendFormattedString("<font size=5>%s - %s</font><br><br>", fileNameString.asCharArray(), lineNumberAsString.asCharArray());

    htmlText.appendFormattedString("<font size=4><b>File Path: </b>%s</font><br><br>", filePathString.asCharArray());
    htmlText.append("<font size=5 color=blue>Assert Reason:</font><br><br>");
    htmlText.append(_assertReason);


    htmlText.append("</body></html>");

    setTextFormat(Qt::RichText);
    setText(htmlText.asCharArray());
}


// ---------------------------------------------------------------------------
// Name:        ahAssertDialog::setDialogTitle
// Description: Sets the dialog title.
// Author:      Avi Shapira
// Date:        28/1/2007
// ---------------------------------------------------------------------------
void ahAssertDialog::setDialogTitle()
{
    gtASCIIString title;
    gtString fileFullName;

    _filePath.getFileNameAndExtension(fileFullName);

    title = "Assert in: ";
    title.append(fileFullName.asASCIICharArray());
    title.append(" - ");
    title.append("Line: ");
    title.appendFormattedString("%d", _lineNumber);

    // Set the Qt dialog title:
    setWindowTitle(title.asCharArray());
}

// ---------------------------------------------------------------------------
// Name:        ahAssertDialog::loadIcon
// Description: Loads the frame icon (The CodeXL Application icon).
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        28/1/2007
// ---------------------------------------------------------------------------
bool ahAssertDialog::loadIcon()
{
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    QPixmap* pPixmap = new QPixmap(ApplicationIcon_64_xpm);
    setIconPixmap(*pPixmap);
#endif
    return true;
}
