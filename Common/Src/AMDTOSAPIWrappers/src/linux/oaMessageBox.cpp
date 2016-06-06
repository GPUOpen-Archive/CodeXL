//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaMessageBox.cpp
///
//=====================================================================

//------------------------------ oaMessageBox.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTOSAPIWrappers/Include/oaMessageBox.h>

// GTK:
#include <gtk/gtk.h>
// cf http://man.chinaunix.net/develop/GTK+/2.6/gtk/GtkMessageDialog.html,

typedef GtkWidget* (*PFNGTKNEWDIALOG)(GtkWindow* parent, GtkDialogFlags flags, GtkMessageType type, GtkButtonsType buttons, const gchar* message_format, ...);
typedef void (*PFNGTKSECONDARYTEXT)(GtkMessageDialog* message_dialog, const gchar* message_format, ...);
typedef void (*PFNGTKRUNDIALOG)(GtkDialog* dialog);
typedef void (*PFNGTKDESTROYWIDGET)(GtkWidget* widget);
typedef gboolean(*PFNGTKINITCHECK)(int* argc, char** * argv);

// ---------------------------------------------------------------------------
// Name:        osMessageBox::display
// Description:
//   Displays the message box.
//   The message box will be displayed in a "Modal" way (blocks the application
//   GUI until the user close the message box).
// Author:      AMD Developer Tools Team
// Date:        6/10/2004
// ---------------------------------------------------------------------------
void oaMessageBoxDisplayCB(const gtString& title, const gtString& message, osMessageBox::osMessageBoxIcon icon, /*oaWindowHandle*/ void* hParentWindow)
{
    GT_UNREFERENCED_PARAMETER(hParentWindow);
    // Output the message to the console:
    gtString messageString = title;
    messageString += L": ";
    messageString += message;
    messageString += L"\n";
    wprintf(L"%ls", messageString.asCharArray());

    // Pop a message box using GTK:
    // Load the GTK library:
    osFilePath gtkSOFilePath = osFilePath(L"libgtk-x11-2.0.so");
    osModuleHandle gtkModule;
    bool rcModule = osLoadModule(gtkSOFilePath, gtkModule);

    GT_IF_WITH_ASSERT(rcModule)
    {
        // Get the procedure addresses for the functions that create, show and destroy the message box:
        PFNGTKNEWDIALOG messageDialogNewProcAddress = nullptr;
        PFNGTKSECONDARYTEXT secondaryTextProcAddress = nullptr;
        PFNGTKRUNDIALOG dialogRunProcAddress = nullptr;
        PFNGTKDESTROYWIDGET widgetDestroyProcAddress = nullptr;
        bool rcNew = osGetProcedureAddress(gtkModule, "gtk_message_dialog_new", (osProcedureAddress&)messageDialogNewProcAddress);
        bool rcSec = osGetProcedureAddress(gtkModule, "gtk_message_dialog_format_secondary_text", (osProcedureAddress&)secondaryTextProcAddress);
        bool rcRun = osGetProcedureAddress(gtkModule, "gtk_dialog_run", (osProcedureAddress&)dialogRunProcAddress);
        bool rcDes = osGetProcedureAddress(gtkModule, "gtk_widget_destroy", (osProcedureAddress&)widgetDestroyProcAddress);

        // Make sure GTK is initialized:
        static bool wasGTKInitialized = false;

        if (!wasGTKInitialized)
        {
            // Don't use gtk_init as it would exit the app in the case of failure:
            PFNGTKINITCHECK initCheck = nullptr;
            bool rcInit = osGetProcedureAddress(gtkModule, "gtk_init_check", (osProcedureAddress&)initCheck);
            GT_IF_WITH_ASSERT(rcInit && initCheck != nullptr)
            {
                // Try to initialize GTK:
                gboolean initSuccess = initCheck(nullptr, nullptr);
                wasGTKInitialized = (initSuccess == TRUE);
                GT_ASSERT(wasGTKInitialized);
            }
        }

        GT_IF_WITH_ASSERT(rcNew && rcSec && rcRun && rcDes && wasGTKInitialized)
        {
            // Get the message type (the icon to be displayed):
            GtkMessageType messageType = GTK_MESSAGE_WARNING;

            switch (icon)
            {
                case osMessageBox::OS_DISPLAYED_INFO_ICON:
                    messageType = GTK_MESSAGE_INFO;
                    break;

                case osMessageBox::OS_EXCLAMATION_POINT_ICON:
                    messageType = GTK_MESSAGE_WARNING;
                    break;

                case osMessageBox::OS_QUESTION_MARK_ICON:
                    messageType = GTK_MESSAGE_QUESTION;
                    break;

                case osMessageBox::OS_STOP_SIGN_ICON:
                    messageType = GTK_MESSAGE_ERROR;
                    break;

                default:
                    // do nothing
                    break;
            }

            GT_IF_WITH_ASSERT(messageDialogNewProcAddress != nullptr)
            {
                // Create the message pDialog and add the second line
                GtkWidget* pDialog = messageDialogNewProcAddress(nullptr, GTK_DIALOG_DESTROY_WITH_PARENT, messageType, GTK_BUTTONS_OK, title.asASCIICharArray());

                GT_IF_WITH_ASSERT(pDialog != nullptr                    &&
                                  secondaryTextProcAddress != nullptr   &&
                                  dialogRunProcAddress != nullptr       &&
                                  widgetDestroyProcAddress != nullptr)
                {
                    secondaryTextProcAddress((GtkMessageDialog*)pDialog, message.asASCIICharArray());
                    dialogRunProcAddress((GtkDialog*)pDialog);
                    widgetDestroyProcAddress(pDialog);
                }
            }
        }

        osReleaseModule(gtkModule);
    }
}

