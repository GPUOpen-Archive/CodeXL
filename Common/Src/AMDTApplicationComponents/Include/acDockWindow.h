//------------------------------ acDockWindow.h ------------------------------

#ifndef __ACDOCKWINDOW
#define __ACDOCKWINDOW

// wxDockIt
#include "wx/dockwindow.h"

Yaki - this class in not in use !!!

// ----------------------------------------------------------------------------------
// Class Name:           acDockWindow : public wxDockWindow
// General Description:
//   A wrapper to wxDockWindow that:
//   a. Makes it fit better into our windows infrastructure (pass key events to the
//      main application frame, etc).
//   b. Gives a simple anc clean API for common operations performed on dock windows.
//
// Author:               Yaki Tebeka
// Creation Date:        13/9/2005
// ----------------------------------------------------------------------------------
    class acDockWindow : public wxDockWindow
    {
    public:
        acDockWindow();
        acDockWindow(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, const wxString& name = wxT("frame"), unsigned int flags = wxDWC_DEFAULT);

    protected:
        void onKeyDown(wxKeyEvent& event);

    private:
        // wxWidgets event table:
        DECLARE_EVENT_TABLE()
    };

#endif  // __ACDOCKWINDOW
