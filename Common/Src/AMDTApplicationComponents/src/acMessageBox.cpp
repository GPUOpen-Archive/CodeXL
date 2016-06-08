//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acMessageBox.cpp
///
//==================================================================================

//------------------------------ acMessageBox.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <inc/acStringConstants.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Forward declaration of window proc to be used in modal:
    LRESULT WINAPI mainOverrideQtWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    // Static members for modality
    static WNDPROC stOriginalWindowProc = NULL;
    static QDialog* stQtDialogToActivate = NULL;
#endif

// Static members initialization:
acMessageBox* acMessageBox::m_spMessageBoxSingleInstance = NULL;
QPixmap* acMessageBox::m_spApplicationIconPixmap = NULL;
QCheckBox* acMessageBox::m_spCheckboxButton = NULL;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // true before dialog is being loaded
    static bool sFirstActivation = true;
#endif

// ---------------------------------------------------------------------------
// Name:        acMessageBox::acMessageBox
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        19/3/2012
// ---------------------------------------------------------------------------
acMessageBox::acMessageBox() : m_pParent(NULL), m_hideDialogFlag(false)
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    m_parentHwnd = NULL;
#endif
}

// ---------------------------------------------------------------------------
// Name:        acMessageBox::~acMessageBox
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        19/3/2012
// ---------------------------------------------------------------------------
acMessageBox::~acMessageBox()
{

}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

// ---------------------------------------------------------------------------
// Name:        acMessageBox::setParentHwnd
// Description: Set the hwnd parent. used in the vsp package only
// Arguments:   HWND parentHwnd
// Return Val:  bool - true if setParent is no longer needed when parent is the same as before
// Author:      Gilad Yarnitzky
// Date:        28/5/2012
// ---------------------------------------------------------------------------
bool acMessageBox::setParentHwnd(HWND parentHwnd)
{
    bool retVal = false;

    if (m_parentHwnd != parentHwnd)
    {
        m_parentHwnd = parentHwnd;
    }
    else
    {
        // Stable parent reached, no more setting is needed:
        retVal = true;
    }

    return retVal;
}
#endif
// ---------------------------------------------------------------------------
// Name:        acMessageBox::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Return Val:  acMessageBox&
// Author:      Sigal Algranaty
// Date:        19/3/2012
// ---------------------------------------------------------------------------
acMessageBox& acMessageBox::instance()
{
    // If my single instance was not created yet - create it:
    if (m_spMessageBoxSingleInstance == NULL)
    {
        m_spMessageBoxSingleInstance = new acMessageBox;

    }

    if (NULL == m_spCheckboxButton)
    {
        m_spCheckboxButton = new QCheckBox;

        m_spCheckboxButton->setText(AC_STR_DoNotShowAgain);
        m_spCheckboxButton->blockSignals(true);
    }

    return *m_spMessageBoxSingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        acMessageBox::instance().critical
// Description: Wraps the Qt message box utility
// Arguments:   const QString& title
//              const QString& text
//              QMessageBox::StandardButtons buttons
//              QMessageBox::StandardButton defaultButton
// Return Val:  QMessageBox::StandardButton
// Author:      Sigal Algranaty
// Date:        19/3/2012
// ---------------------------------------------------------------------------
QMessageBox::StandardButton acMessageBox::critical(const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox::StandardButton retVal = QMessageBox::Cancel;

    QWidget* pValidParent = m_pParent;
    // Sanity check because of VS:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (m_pParent && !::IsWindow(reinterpret_cast<HWND>(m_pParent->winId())))
    {
        pValidParent = NULL;
    }

#endif

    retVal = showNewMessageBox(pValidParent, QMessageBox::Critical, title, text, buttons, defaultButton);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acMessageBox::information
// Description: Wraps the Qt message box utility
// Arguments:   const QString& title
//              const QString& text
//              QMessageBox::StandardButtons buttons
//              QMessageBox::StandardButton defaultButton
// Return Val:  QMessageBox::StandardButton
// Author:      Sigal Algranaty
// Date:        19/3/2012
// ---------------------------------------------------------------------------
QMessageBox::StandardButton acMessageBox::information(const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox::StandardButton retVal = QMessageBox::Cancel;

    QWidget* pValidParent = m_pParent;
    // Sanity check because of VS:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (m_pParent && !::IsWindow(reinterpret_cast<HWND>(m_pParent->winId())))
    {
        pValidParent = NULL;
    }

#endif

    retVal = showNewMessageBox(pValidParent, QMessageBox::Information, title, text, buttons, defaultButton);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acMessageBox::question
// Description: Wraps the Qt message box utility
// Arguments:   const QString& title
//              const QString& text
//              QMessageBox::StandardButtons buttons
//              QMessageBox::StandardButton defaultButton
// Return Val:  QMessageBox::StandardButton
// Author:      Sigal Algranaty
// Date:        19/3/2012
// ---------------------------------------------------------------------------
QMessageBox::StandardButton acMessageBox::question(const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox::StandardButton retVal = QMessageBox::Cancel;

    QWidget* pValidParent = m_pParent;
    // Sanity check because of VS:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (m_pParent && ::IsWindow(reinterpret_cast<HWND>(m_pParent->winId())))
    {
        pValidParent = NULL;
    }

#endif

    retVal = showNewMessageBox(pValidParent, QMessageBox::Question, title, text, buttons, defaultButton);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acMessageBox::warning
// Description: Wraps the Qt message box utility
// Arguments:   const QString& title
//              const QString& text
//              QMessageBox::StandardButtons buttons
//              QMessageBox::StandardButton defaultButton
// Return Val:  QMessageBox::StandardButton
// Author:      Sigal Algranaty
// Date:        19/3/2012
// ---------------------------------------------------------------------------
QMessageBox::StandardButton acMessageBox::warning(const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox::StandardButton retVal = QMessageBox::Cancel;

    QWidget* pValidParent = m_pParent;
    // Sanity check because of VS:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    if (m_pParent && !::IsWindow(reinterpret_cast<HWND>(m_pParent->winId())))
    {
        pValidParent = NULL;
    }

#endif

    retVal = showNewMessageBox(pValidParent, QMessageBox::Warning, title, text, buttons, defaultButton);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acMessageBox::showNewMessageBox
// Description:
// Arguments:   QWidget *parent
//              QMessageBox::Icon icon
//              const QString& title
//              const QString& text
//              QMessageBox::StandardButtons buttons
//              QMessageBox::StandardButton defaultButton
// Return Val:  QMessageBox::StandardButton
// Author:      Gilad Yarnitzky
// Date:        28/5/2012
// ---------------------------------------------------------------------------
QMessageBox::StandardButton acMessageBox::showNewMessageBox(QWidget* parent, QMessageBox::Icon icon, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    QMessageBox::StandardButton retVal =  QMessageBox::Cancel;
    QMessageBox msgBox(icon, title, text, QMessageBox::NoButton, parent);

    if (m_hideDialogFlag)
    {
        m_spCheckboxButton->setCheckState(Qt::Unchecked);
        msgBox.addButton(m_spCheckboxButton, QMessageBox::ResetRole);
    }

    if (m_spApplicationIconPixmap != NULL)
    {
        msgBox.setWindowIcon(QIcon(*m_spApplicationIconPixmap));
    }


    // This is deprecated in Qt 5.2, need to find some other code.
    //QDialogButtonBox* pButtonBox = qFindChild<QDialogButtonBox*>(&msgBox);
    QDialogButtonBox* pButtonBox = msgBox.findChild<QDialogButtonBox*>();
    GT_IF_WITH_ASSERT(NULL != pButtonBox)
    {
        uint mask = QMessageBox::FirstButton;

        while (mask <= QMessageBox::LastButton)
        {
            uint sb = buttons & mask;
            mask <<= 1;

            if (!sb)
            {
                continue;
            }

            QPushButton* pButton = msgBox.addButton((QMessageBox::StandardButton)sb);
            GT_IF_WITH_ASSERT(NULL != pButton)
            {
                // Choose the first accept role as the default
                if (msgBox.defaultButton())
                {
                    continue;
                }

                if ((defaultButton == QMessageBox::NoButton && pButtonBox->buttonRole(pButton) == QDialogButtonBox::AcceptRole)
                    || (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton)))
                {
                    msgBox.setDefaultButton(pButton);
                }
            }
        }


        if (doModal(&msgBox) == -1)
        {
            retVal = QMessageBox::Cancel;
        }
        else
        {
            retVal = msgBox.standardButton(msgBox.clickedButton());
        }
    }

    if (m_hideDialogFlag)
    {
        msgBox.removeButton(m_spCheckboxButton);
        m_hideDialogFlag = false;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        acMessageBox::doModal
// Description:
// Arguments:   QMessageBox* pMessageBox
// Return Val:  QMessageBox::StandardButton
// Author:      Gilad Yarnitzky
// Date:        28/5/2012
// ---------------------------------------------------------------------------
int acMessageBox::doModal(QDialog* pMessageBox)
{
    int retVal = -1;
    GT_IF_WITH_ASSERT(pMessageBox != NULL)
    {

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        // Pre modal actions:
        // actions are needed only if there is no parent and there is a HWND parent
        if (NULL == m_pParent && NULL != m_parentHwnd)
        {
            if (::IsWindow(m_parentHwnd))
            {
                ::EnableWindow(m_parentHwnd, false);
            }

            // replace the main window proc of the dialog window:
            sFirstActivation = true;
            stOriginalWindowProc = (WNDPROC)::SetWindowLongPtr((HWND)pMessageBox->winId(), GWLP_WNDPROC, (LONG_PTR)&mainOverrideQtWindowProc);
            GT_ASSERT(stOriginalWindowProc != NULL);
            stQtDialogToActivate = pMessageBox;
        }

#endif

        try
        {
            retVal = pMessageBox->exec();
        }
        catch (...)
        {
            // Just make sure everything is caught so main vs window
            // can be restored.
        }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        // Post modal actions
        // actions are needed only if there is no parent and there is a HWND parent
        if (NULL == m_pParent && NULL != m_parentHwnd)
        {
            // restore the original window proc of the visual studio window:
            ::SetWindowLongPtr((HWND)pMessageBox->winId(), GWLP_WNDPROC, (LONG_PTR)stOriginalWindowProc);
            stQtDialogToActivate = NULL;

            if (::IsWindow(m_parentHwnd))
            {
                ::EnableWindow(m_parentHwnd, true);

                // Force the visual window to be the fore window and active:
                ::SetForegroundWindow(m_parentHwnd);
            }
        }

#endif

    }
    return retVal;
}


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
// ---------------------------------------------------------------------------
// Name:        mainOverrideQtWindowProc
// Description: override window proc when doing a modal dialog
// Author:      Gilad Yarnitzky
// Date:        28/5/2012
// ---------------------------------------------------------------------------
LRESULT WINAPI mainOverrideQtWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = TRUE;

    // Force the first time brining to front
    if (Msg == WM_ACTIVATEAPP)
    {
        GT_IF_WITH_ASSERT(stQtDialogToActivate != NULL && !sFirstActivation)
        {
            // Bring the dialog to the front only we the application is getting focus
            if (wParam == TRUE)
            {
                ::SetForegroundWindow((HWND)stQtDialogToActivate->winId());
            }
        }
    }

    if (WM_ACTIVATE == Msg)
    {
        if (sFirstActivation)
        {
            ::SetForegroundWindow((HWND)stQtDialogToActivate->winId());
            sFirstActivation = false;
        }
    }

    lResult = stOriginalWindowProc(hWnd, Msg, wParam, lParam);

    return lResult;
}
#endif
