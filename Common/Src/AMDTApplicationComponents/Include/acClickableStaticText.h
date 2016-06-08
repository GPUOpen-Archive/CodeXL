//------------------------------ acClickableStaticText.h ------------------------------

#ifndef __ACCLICKABLESTATICTEXT
#define __ACCLICKABLESTATICTEXT

12 / 7 / 2006 - Yaki:
This class is deprecated.
    Please use wxHyperlinkCtrl instead!

/*
// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           acClickableStaticText : public wxStaticText
// General Description:
//   A static text that displays a clickable text.
//   When clicking on the text,
//
// Author:               Avi Shapira
// Creation Date:        27/11/2005
// ----------------------------------------------------------------------------------
    class AC_API acClickableStaticText : public wxStaticText
        {
    public:
        acClickableStaticText(const wxString& label,
                              wxWindow* pParent, wxWindowID id = wxID_ANY,
                              const wxPoint& position = wxDefaultPosition,
                              const wxSize& size = wxDefaultSize,
                              long style = 0);

    protected:
        void onMouseEnterWindow(wxMouseEvent& event);
        void onMouseLeaveWindow(wxMouseEvent& event);
        void onLeftMouseUp(wxMouseEvent& event);

        DECLARE_EVENT_TABLE()
    };
*/

#endif  // __ACCLICKABLESTATICTEXT
