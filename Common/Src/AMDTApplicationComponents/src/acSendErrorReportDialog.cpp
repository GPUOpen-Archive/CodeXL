//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSendErrorReportDialog.cpp
///
//==================================================================================

//------------------------------ acSendErrorReportDialog.cpp ------------------------------
#include <AMDTApplicationComponents/Include/acSendErrorReportDialog.h>

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osBugReporter.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acDefinitions.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>
#include <AMDTApplicationComponents/Include/acQMessageDialog.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osCallsStackReader.h>

// Defines for BugScouts:
#define CRASH_REPROT_SERVER "ausdevtoolsprd.amd.com"
#define CRASH_REPROT_SERVER_RELATIVE_URL "/cgi-bin/GRCrashHttpProxy.exe"
#define CRASH_REPROT_BUG_AREA "Crash Reports"
#define CRASH_REPROT_REPORTER_USER_NAME "Bugzscout"

// Graphics:
#define AC_DETAILS_MESSAGE_BOX_WIDTH 600
#define AC_DETAILS_MESSAGE_BOX_HEIGHT 400

#define AC_ADDITIONAL_INFO_TEXT_HEIGHT (14 * AC_DEFAULT_TEXT_CHAR_HEIGHT)
#define AF_TEXT_CONTROLS_WIDTH (400 + 20 * AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH)
#define AF_HTML_TEXT_HEIGHT_MARGIN 20


// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::acSendErrorReportDialog
// Description: Definition of the Help About Dialog.
// Author:      Avi Shapira
// Date:        5/11/2003
// ---------------------------------------------------------------------------
acSendErrorReportDialog::acSendErrorReportDialog(QWidget* pParent, const QString& productName, const QIcon& icon)
    : QDialog(pParent, Qt::Dialog | Qt::WindowCloseButtonHint),
      m_pAdditionalInformation(nullptr), m_pMainText(nullptr), m_pEmailAddress(nullptr),
      m_pSendButton(nullptr), m_pDontSendButton(nullptr), m_pPrivacyButton(nullptr),
      m_isRegisterForRecievingDebuggedProcessEvents(false)
{
    // Set the CodeXL project type title as string:
    m_productName = productName;
    QString strWindowTitle = m_productName;
    strWindowTitle.append(" "  AC_STR_SendErrorReportTitle);
    this->setWindowTitle(strWindowTitle);

    // Update CodeXL and OS versions strings:
    updateCodeXLVersionAsString();
    updateGeneralStrings();

    // Set the dialog icon:
    setWindowIcon(icon);


    // Create the dialog layout:
    setDialogLayout();
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::registerForRecievingDebuggedProcessEvents
// Description: Registers this class to receive debugged process events.
// Author:      Yaki Tebeka
// Date:        13/5/2009
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::registerForRecievingDebuggedProcessEvents()
{
    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
    m_isRegisterForRecievingDebuggedProcessEvents = true;
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::~acSendErrorReportDialog
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        16/5/2007
// ---------------------------------------------------------------------------
acSendErrorReportDialog::~acSendErrorReportDialog()
{
    if (m_isRegisterForRecievingDebuggedProcessEvents)
    {
        // Unregister myself from listening to debugged process events:
        apEventsHandler::instance().unregisterEventsObserver(*this);
    }
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::updateCodeXLVersionAsString
// Description: Retrieves CodeXL's version as string and set it into a class member.
// Author:      Avi Shapira
// Date:        27/11/2005
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::updateCodeXLVersionAsString()
{
    // Get the product version
    osProductVersion appVersion;
    osGetApplicationVersion(appVersion);
    m_CodeXLVersionString = acGTStringToQString(appVersion.toString());
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::updateGeneralStrings
// Description: Retrieves the Operating System and product strings and stores it in
//              the relevant members.
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/7/2007
// ---------------------------------------------------------------------------
bool acSendErrorReportDialog::updateGeneralStrings()
{
    bool retVal = false;

    // Get the product description string:
    gtString prodVersionString = osDebugLog::instance().productDescriptionString();
    m_productDescriptionString = acGTStringToQString(prodVersionString);

    // Get the OS description string:
    gtString osDescriptionString;
    bool rc1 = osGetOSShortDescriptionString(osDescriptionString);
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;
        m_osDescriptionString = acGTStringToQString(osDescriptionString);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::onEvent
// Description: Is called when a debugged process event occur.
//              Send the event to the appropriate event function for displaying
//              it to the user.
// Arguments:   event - The debugged process event.
// Author:      Avi Shapira
// Date:        27/11/2005
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(eve);
    GT_UNREFERENCED_PARAMETER(vetoEvent);
    // do nothing
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::onUnhandledException
// Description: Is called when an unhandled exception occur.
// Arguments: exceptionCode - The unhandled exception code.
//            exceptionCallStack - The unhandled exception associated call stack.
// Author:      Yaki Tebeka
// Date:        13/5/2009
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::onUnhandledException(osExceptionCode& exceptionCode, void* pExceptionContext, bool allowDifferentSystemPath)
{
    ///////////////////////////////////
    // Get the exception's associated call stack:
    osCallStack exceptionCallStack;
    osCallsStackReader callStackReader;

    bool gotExceptionCallStack = callStackReader.getCallStack(exceptionCallStack, pExceptionContext, false);
    GT_ASSERT(gotExceptionCallStack);
    ///////////////////////////////////

    // Translate the exception code to an exception reason:
    osExceptionReason exceptionReason = osExceptionCodeToExceptionReason(exceptionCode);

    // Display the error report dialog:
    displayErrorReportDialog(exceptionReason, exceptionCallStack, "", false, allowDifferentSystemPath);

}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::displayErrorReportDialog
// Description: Displays the send error report dialog to the user.
// Arguments: exceptionReason - The associated exception reason.
//            callStack - The call stack associated with the error report.
//            displayOnlySpyRelatedErrorReports - if true, only spy related error reports will be displayed.
// Author:      Yaki Tebeka
// Date:        13/5/2009
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::displayErrorReportDialog(osExceptionReason exceptionReason, const osCallStack& callStack, const QString& additionalInformation, bool displayOnlySpyRelatedErrorReports, bool allowDifferentSystemPath)
{
    // Translate the call stack into a string:
    QString callStackString;
    QString callStackTitleString;
    bool isSpyRelatedCallStack = false;

    gtString callStackStr, callStackTitleStr;
    callStack.asString(callStackTitleStr, callStackStr, isSpyRelatedCallStack, allowDifferentSystemPath);
    callStackString = acGTStringToQString(callStackStr);
    callStackTitleString = acGTStringToQString(callStackTitleStr);

    // Output the error call stack string into the log file:
    OS_OUTPUT_DEBUG_LOG(callStackStr.asCharArray(), OS_DEBUG_LOG_ERROR);

    // If the crash was in the spy or if we display all crash locations:
    if ((displayOnlySpyRelatedErrorReports && isSpyRelatedCallStack) || !displayOnlySpyRelatedErrorReports)
    {
        // If the crash did not happen when the spy was terminated:
        if (!isCrashInSpyTerminationFunc(callStackString))
        {
            // Set the OS and CodeXL versions (as string) into the title:
            m_errorReportTitle = m_productName;
            m_errorReportTitle += "-";
            m_errorReportTitle += m_productDescriptionString;
            m_errorReportTitle += "-";
            m_errorReportTitle += m_osDescriptionString;
            m_errorReportTitle += "-";
            m_errorReportTitle += m_CodeXLVersionString;
            m_errorReportTitle += " ";
            m_errorReportTitle.append(callStackTitleString);

            // Translate the exception reason to a string:
            gtString exceptionReasonAsStr;
            osExceptionReasonToString(exceptionReason, exceptionReasonAsStr);
            m_errorReportTitle += " (";
            m_errorReportTitle += acGTStringToQString(exceptionReasonAsStr);
            m_errorReportTitle += ")";

            // Set the call stack string into a class member:
            m_callStackString = callStackString;

            GT_IF_WITH_ASSERT(m_pSendButton != nullptr)
            {
                m_pSendButton->setDefault(true);
                m_pSendButton->setFocus();
            }
            // Set the call stack string into a class member:
            m_additionalInformation = additionalInformation;

            this->exec();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::isCrashInSpyTerminationFunc
// Description: Check if the crash triggered when the spy was terminated (Callstack contains "gaTerminateDebuggedProcessImpl").
// Arguments:   wxCommandEvent &eve
// Author:      Avi Shapira
// Date:        24/7/2006
// ---------------------------------------------------------------------------
bool acSendErrorReportDialog::isCrashInSpyTerminationFunc(QString& callStackString)
{
    bool retVal = false;

    int findLocation = callStackString.indexOf("gaTerminateDebuggedProcessImpl", 0);

    if (findLocation != -1)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::onSendErrorReportClick
// Description: Send AMD the error report.
// Arguments:   wxCommandEvent &eve
// Author:      Avi Shapira
// Date:        24/11/2005
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::onSendErrorReportClick()
{
    GT_IF_WITH_ASSERT(m_pEmailAddress != nullptr)
    {
        QString bugTitle;
        QString bugReportDetails;
        QString reporterEmailAddress;

        // Get the bug title:
        bugTitle = m_errorReportTitle;

        // Reporter email address:
        reporterEmailAddress = m_pEmailAddress->text().toLatin1().data();

        // Get the bug message:
        getBugReportDetails(bugReportDetails);

        gtASCIIString productName = m_productName.toLocal8Bit().data();
        osBugReporter bugReporter(CRASH_REPROT_SERVER, CRASH_REPROT_SERVER_RELATIVE_URL, productName, CRASH_REPROT_BUG_AREA, bugTitle.toLatin1().data(), bugReportDetails.toLatin1().data(), CRASH_REPROT_REPORTER_USER_NAME, reporterEmailAddress.toLatin1().data(), false);
        gtString bugsSystemReturnedMessage;

        bool isUsingProxy = false;
        osPortAddress proxyServer;

        bool rcProx = false; // NZ afGlobalVariablesManager::instance().getProxyInformation(isUsingProxy, proxyServer);

        if (!rcProx)
        {
            GT_ASSERT(rcProx);
            isUsingProxy = false;
        }

        // Report the bug to our CRM system:
        setCursor(Qt::WaitCursor);

        gtASCIIString bugsSystemReturnedMessageASCII;
        bool rc = bugReporter.reportBug(bugsSystemReturnedMessageASCII, isUsingProxy, proxyServer);
        setCursor(Qt::ArrowCursor);

        if (rc)
        {
            // The default scout message is '""', so after HTML decoding it becomes "  ", and we want to consider it as empty:
            if (bugsSystemReturnedMessageASCII == "\"\"")
            {
                bugsSystemReturnedMessageASCII.makeEmpty();
            }

            // Display a success message:
            acQMessageDialog detailsDialog(AC_STR_SendErrorReportDialogTitle, QString(AC_STR_SendErrorReportSucceededHeaderDescription).arg(m_productName), bugsSystemReturnedMessageASCII.asCharArray(), this, QSize(-1, -1));

            // Show the dialog:
            detailsDialog.exec();
        }
        else
        {
            QString failedMessage;

            /*if (bugsSystemReturnedMessageASCII != AF_STR_EmptyA)
            {
                // Add the failed reason to the output string:
                failedMessage.append(bugsSystemReturnedMessageASCII.asCharArray());
                failedMessage.append("\n\n");
            }*/

            failedMessage.append(bugReportDetails);

            // Display a failed message:
            QSize messageSize(acScalePixelSizeToDisplayDPI(AC_DETAILS_MESSAGE_BOX_WIDTH), acScalePixelSizeToDisplayDPI(AC_DETAILS_MESSAGE_BOX_HEIGHT));
            acQMessageDialog detailsDialog(AC_STR_SendErrorReportDialogTitle, AC_STR_SendErrorReportFailedHeaderDescription, failedMessage, this, messageSize);

            // Show the dialog:
            detailsDialog.exec();
        }

        this->close();
    }
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::onDontSendErrorReportClick
// Description: Close the dialog
// Author:      Sigal Algranaty
// Date:        13/5/2012
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::onDontSendErrorReportClick()
{
    this->close();
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::onPrivacyPolicyClick
// Description: Called when the user presses the privacy policy button
// Author:      Uri Shomroni
// Date:        20/6/2011
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::onPrivacyPolicyClick()
{
    osFileLauncher fileLauncher(AC_STR_PrivacyPolicyURL);
    bool rc = fileLauncher.launchFile();
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::getBugReportDetails
// Description: Build the bug report message details.
// Author:      Avi Shapira
// Date:        28/11/2005
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::getBugReportDetails(QString& bugReportDetails)
{
    bugReportDetails.clear();

    // Get the optional user scenario information:
    QString additionalInformationString(m_pAdditionalInformation->toPlainText().toLatin1().data());

    if (additionalInformationString.isEmpty())
    {

        additionalInformationString = AC_STR_NotAvailableA;
    }

    // Get the optional user email:
    QString userEmailAddressString(m_pEmailAddress->text().toLatin1().data());

    if (userEmailAddressString.isEmpty())
    {
        userEmailAddressString = AC_STR_NotAvailableA;
    }

    // Add the user information:
    bugReportDetails.append(AC_STR_NewLineA);
    bugReportDetails.append(AC_STR_SendErrorReportString5);
    bugReportDetails.append(AC_STR_NewLineA);
    bugReportDetails.append(additionalInformationString);
    bugReportDetails.append("\n\n");

    // Add the user email address:
    bugReportDetails.append(AC_STR_SendErrorReportString6);
    bugReportDetails.append(AC_STR_NewLineA);
    bugReportDetails.append(userEmailAddressString);
    bugReportDetails.append("\n\n");

    // Add the OS description string:
    bugReportDetails.append(AC_STR_SendErrorReportOperatingSystemTitle);
    bugReportDetails.append(AC_STR_NewLineA);
    bugReportDetails.append(m_osDescriptionString);
    bugReportDetails.append(AC_STR_NewLineA);
    bugReportDetails.append(AC_STR_NewLineA);

    // Add the Call stack data:
    bugReportDetails.append(AC_STR_SendErrorReportCallStackTitle);
    bugReportDetails.append(AC_STR_NewLineA);
    bugReportDetails.append(m_callStackString);

    // Get the system information string from the debug log:
    const gtString& systemInformationString = osDebugLog::instance().osDescriptionString();
    QString sysIntoStr = acGTStringToQString(systemInformationString);

    // Add the system information data:
    bugReportDetails.append(AC_STR_NewLineA);
    bugReportDetails.append(sysIntoStr);

    // Add the additional information:
    bugReportDetails.append(AC_STR_NewLineA);
    bugReportDetails.append(m_additionalInformation);
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::setDialogLayout
// Description: Create the dialog layout
// Author:      Sigal Algranaty
// Date:        13/5/2012
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::setDialogLayout()
{
    // Calculate sizes:
    unsigned int textControlsWidth = acScalePixelSizeToDisplayDPI(AF_TEXT_CONTROLS_WIDTH);
    unsigned int additionalInfoControlHeight = acScalePixelSizeToDisplayDPI(AC_ADDITIONAL_INFO_TEXT_HEIGHT);

    // Create the dialog main layout:
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->setSizeConstraint(QLayout::SetFixedSize);

    // Create the HTML main text control:
    m_pMainText = new acQHTMLWindow(nullptr);

    // Connect the link clicked signal:
    bool rc = connect(m_pMainText, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(onAnchorClicked(const QUrl&)));

    // Build the main string as HTML:
    QString mainStringAsHTML;
    buildHTMLMainText(mainStringAsHTML);

    QRect mainTextRect;
    QFontMetrics(m_pMainText->font()).boundingRect(mainTextRect, Qt::TextWordWrap, mainStringAsHTML);
    int textHeight = mainTextRect.height() + AF_HTML_TEXT_HEIGHT_MARGIN;

    // Set the main HTML text:
    m_pMainText->setFrameShape(QTextEdit::NoFrame);
    m_pMainText->setText(mainStringAsHTML);
    m_pMainText->horizontalScrollBar()->setVisible(false);
    m_pMainText->verticalScrollBar()->setVisible(false);
    m_pMainText->setMinimumHeight(textHeight);
    m_pMainText->setMaximumWidth(textControlsWidth);
    m_pMainText->setMinimumWidth(textControlsWidth);


    QLabel* pLabel2 = new QLabel;
    pLabel2->setText(AC_STR_SendErrorReportString5);

    // Create the additional information label:
    m_pAdditionalInformation = new QTextEdit;
    m_pEmailAddress = new QLineEdit;
    QLabel* pEmailLabel = new QLabel(AC_STR_SendErrorReportString6);

    // Create a dialog button box:
    QDialogButtonBox* pDialogButtonBox = new QDialogButtonBox(Qt::Horizontal);
    m_pPrivacyButton = new QPushButton(tr(AC_STR_SendErrorReportPrivacyPolicyButtonCaption));
    m_pPrivacyButton->setDefault(false);
    m_pSendButton = new QPushButton(tr(AC_STR_SendErrorReportSendButton));
    m_pSendButton->setDefault(true);
    m_pDontSendButton = new QPushButton(tr(AC_STR_SendErrorReportDontSendButton));
    m_pSendButton->setAutoDefault(true);
    m_pSendButton->setFocus();

    // Add the buttons to the button box:
    pDialogButtonBox->addButton(m_pPrivacyButton, QDialogButtonBox::ActionRole);
    pDialogButtonBox->addButton(m_pSendButton, QDialogButtonBox::ActionRole);
    pDialogButtonBox->addButton(m_pDontSendButton, QDialogButtonBox::ActionRole);

    m_pAdditionalInformation->setMaximumWidth(textControlsWidth);
    m_pAdditionalInformation->setMinimumWidth(textControlsWidth);
    m_pAdditionalInformation->setMaximumHeight(additionalInfoControlHeight);
    m_pAdditionalInformation->setMinimumHeight(additionalInfoControlHeight);

    QHBoxLayout* pMainTextLayout = new QHBoxLayout();
    pMainTextLayout->addWidget(m_pMainText, 1);
    pMainLayout->addLayout(pMainTextLayout);

    QHBoxLayout* pTextBoxLayout = new QHBoxLayout();
    pTextBoxLayout->addWidget(m_pAdditionalInformation, 1);

    pMainLayout->addWidget(pLabel2);
    pMainLayout->addLayout(pTextBoxLayout);

    QHBoxLayout* pEmailLayout = new QHBoxLayout();
    pEmailLayout->addWidget(pEmailLabel);
    pEmailLayout->addWidget(m_pEmailAddress, 1);
    pMainLayout->addLayout(pEmailLayout, Qt::AlignRight);
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(pDialogButtonBox, 0 , Qt::AlignRight);

    // Connect the buttons to slots:
    rc = connect(m_pPrivacyButton, SIGNAL(clicked()), this, SLOT(onPrivacyPolicyClick()));
    GT_ASSERT(rc);

    rc = connect(m_pSendButton, SIGNAL(clicked()), this, SLOT(onSendErrorReportClick()));
    GT_ASSERT(rc);

    rc = connect(m_pDontSendButton, SIGNAL(clicked()), this, SLOT(onDontSendErrorReportClick()));
    GT_ASSERT(rc);

    setLayout(pMainLayout);
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::buildHTMLMainText
// Description: Build the dialog main string as HTML
// Arguments:   QString& mainStringAsHTML
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        13/5/2012
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::buildHTMLMainText(QString& mainStringAsHTML)
{
    // Get system default background color:
    QString bgColor = acGTStringToQString(acGetSystemDefaultBackgroundColorAsHexString());

    // Create the dialog items:
    QString str1 = QString("<html><body style='font-size:120%;background-color:#%1;line-height:120%' >").arg(bgColor);
    mainStringAsHTML.append(str1);
    mainStringAsHTML.append(QString(AC_STR_SendErrorReportString1).arg(m_productName));
    mainStringAsHTML.append("<br/><br/>");
    mainStringAsHTML.append("<font color=blue>");
    mainStringAsHTML.append(QString(AC_STR_SendErrorReportString2).arg(m_productName));
    mainStringAsHTML.append("</font>");
    mainStringAsHTML.append("<br/><br/>");
    mainStringAsHTML.append(QString(AC_STR_SendErrorReportString3).arg(m_productName));
    mainStringAsHTML.append("<br/><br/>");
    mainStringAsHTML.append(AC_STR_SendErrorReportString4);
    mainStringAsHTML.append("<a href=");
    mainStringAsHTML.append(AC_STR_sendErrorReportClickHereLink);
    mainStringAsHTML.append(">");
    mainStringAsHTML.append(AC_STR_SendErrorReportClickHere);
    mainStringAsHTML.append("</a>");

    mainStringAsHTML.append("<br/><br/>");
    mainStringAsHTML.append(AC_STR_SendErrorReportStringProxy1);

    mainStringAsHTML.append("<a href=");
    mainStringAsHTML.append(AC_STR_SendErrorReportStringProxyLink);
    mainStringAsHTML.append(">");

    mainStringAsHTML.append(AC_STR_SendErrorReportStringProxy2);
    mainStringAsHTML.append("</a>");

    mainStringAsHTML.append(AC_STR_SendErrorReportStringProxy3);
    mainStringAsHTML.append("</body></html>");
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::onAnchorClicked
// Description: Handle URL clicked on main text object
// Arguments:   const QUrl & link
// Author:      Sigal Algranaty
// Date:        13/5/2012
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::onAnchorClicked(const QUrl& url)
{
    QString urlAsQString = url.toString();

    if (urlAsQString == AC_STR_sendErrorReportClickHereLink)
    {
        QString bugReportDetails;
        getBugReportDetails(bugReportDetails);

        // Open the more details Dialog:
        QString header;
        header.append(QString(AC_STR_SendErrorReportHeaderDescription).arg(m_productName));
        header.append(AC_STR_NewLineA AC_STR_NewLineA AC_STR_PrivacyPolicyUserInformationDisclaimer);
        QSize messageSize(acScalePixelSizeToDisplayDPI(AC_DETAILS_MESSAGE_BOX_WIDTH), acScalePixelSizeToDisplayDPI(AC_DETAILS_MESSAGE_BOX_HEIGHT));
        acQMessageDialog messageBox(QString(AC_STR_SendErrorReportDetailsDialogTitle).arg(m_productName), header, bugReportDetails, this, messageSize);
        messageBox.exec();
    }
    else if (urlAsQString == AC_STR_SendErrorReportStringProxyLink)
    {
        // NZ afGlobalSettingsDialog::instance().exec();
    }
}

// ---------------------------------------------------------------------------
// Name:        acSendErrorReportDialog::onMemoryAllocationFailure
// Description: Called when there is insufficient memory for allocation.
// Arguments:   const osCallStack& allocCallStack - the relevant call stack
// Author:      Amit Ben-Moshe
// Date:        30/01/2014
// ---------------------------------------------------------------------------
void acSendErrorReportDialog::onMemoryAllocationFailure(const osCallStack& allocCallStack, bool allowDifferentSystemPath)
{
    // Display the error report dialog.
    displayErrorReportDialog(OS_INSUFFICIENT_MEMORY, allocCallStack, "", false, allowDifferentSystemPath);
}
