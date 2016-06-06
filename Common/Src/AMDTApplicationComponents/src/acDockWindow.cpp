//------------------------------ acDockWindow.cpp ------------------------------

Yaki - this class in not in use !!!

// wxWidgets pre compiled header:
#include <AMDTApplicationComponents/Include/acWxWidgetsIncludes.h>

// Local:
#include <AMDTApplicationComponents/Include/acDockWindow.h>

// wxWidgets event table:
    BEGIN_EVENT_TABLE(acDockWindow, wxDockWindow)
        EVT_KEY_DOWN(onKeyDown)
    END_EVENT_TABLE()



// ---------------------------------------------------------------------------
// Name:        acDockWindow::acDockWindow
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        13/9/2005
// ---------------------------------------------------------------------------
    acDockWindow::acDockWindow()
{
}


// ---------------------------------------------------------------------------
// Name:        acDockWindow::acDockWindow
// Description: Constructor
// Arguments:   parent - My parent window.
//              id - My window Id.
//              title - The window title.
//              pos - The window initial position.
//              size - The window initial size.
//              name - The window name.
//              flags - Window flags.
// Author:      Yaki Tebeka
// Date:        13/9/2005
// ---------------------------------------------------------------------------
acDockWindow::acDockWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, const wxString& name, unsigned int flags)
    : wxDockWindow(parent, id, title, pos, size, name, flags)
{
}


// ---------------------------------------------------------------------------
// Name:        acDockWindow::onKeyDown
// Description:
//   Is called when a key is pressed.
//   Pass the key event into the main application frame.
//
// Arguments:   event - A class representing the key pressed event.
// Author:      Yaki Tebeka
// Date:        13/9/2005
// ---------------------------------------------------------------------------
void acDockWindow::onKeyDown(wxKeyEvent& event)
{
    // Get the application instance:
    wxApp* pTheApplicationInstance = (wxApp*)(wxApp::GetInstance());

    if (pTheApplicationInstance)
    {
        // Get the application top window (usually the main frame):
        wxWindow* pTopWindow = pTheApplicationInstance->GetTopWindow();

        if (pTopWindow)
        {
            // Send the event to the application top window:
            pTopWindow->ProcessEvent(event);
        }
    }
}
