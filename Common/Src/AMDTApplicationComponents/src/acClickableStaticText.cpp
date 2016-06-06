//------------------------------ acClickableStaticText.cpp ------------------------------

12 / 7 / 2006 - Yaki:
This class is deprecated.
    Please use wxHyperlinkCtrl instead!

/*
// wxWindows pre compiled header:
#include <AMDTApplicationComponents/Include/acWxWidgetsIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
// Local:
#include <AMDTApplicationComponents/Include/acClickableStaticText.h>

// acClickableStaticText event table:
    BEGIN_EVENT_TABLE(acClickableStaticText, wxStaticText)
        EVT_LEFT_UP(acClickableStaticText::onLeftMouseUp)
        EVT_ENTER_WINDOW(acClickableStaticText::onMouseEnterWindow)
        EVT_LEAVE_WINDOW(acClickableStaticText::onMouseLeaveWindow)
    END_EVENT_TABLE()



// ---------------------------------------------------------------------------
// Name:        acClickableStaticText::acClickableStaticText
// Description: Constructor
// Arguments:   label - The text that will be displayed to the user.
//              urlToBeLaunched - The URL to be launched when pressing the text label.
//              pParent - This static text parent window.
//              id - The id of this static text
//              position, size - The static text position and size.
//              style - The static text style.
// Author:      Yaki Tebeka
// Date:        29/7/2004
// ---------------------------------------------------------------------------
    acClickableStaticText::acClickableStaticText(const wxString& label,
                                                 wxWindow* pParent, wxWindowID id,
                                                 const wxPoint& position, const wxSize& size,
                                                 long style)
        : wxStaticText(pParent, id, label, position, size, style)
    {
    // Set the font color to blue:
    SetForegroundColour(*wxBLUE);
}


// ---------------------------------------------------------------------------
// Name:        acClickableStaticText::onMouseEnterWindow
// Description: Is called when the mouse enters this static text area.
// Arguments:   event - The mouse event.
// Author:      Yaki Tebeka
// Date:        29/7/2004
// ---------------------------------------------------------------------------
void acClickableStaticText::onMouseEnterWindow(wxMouseEvent& event)
    {
    SetCursor(wxCursor(wxCURSOR_HAND));
}


// ---------------------------------------------------------------------------
// Name:        acClickableStaticText::onMouseLeaveWindow
// Description: Is called when the mouse leaves this static text area.
// Arguments:   event - The mouse event.
// Author:      Yaki Tebeka
// Date:        29/7/2004
// ---------------------------------------------------------------------------
void acClickableStaticText::onMouseLeaveWindow(wxMouseEvent& event)
    {
    SetCursor(wxCursor(wxCURSOR_DEFAULT));
}


// ---------------------------------------------------------------------------
// Name:        acClickableStaticText::onLeftMouseUp
// Description: Is called when the left mouse button is released on top of this
//              static text area.
// Arguments:   event - The mouse event.
// Author:      Yaki Tebeka
// Date:        29/7/2004
// ---------------------------------------------------------------------------
void acClickableStaticText::onLeftMouseUp(wxMouseEvent& event)
    {
    wxMouseEvent mouseEvent(wxEVT_LEFT_UP);
    mouseEvent.SetId(GetId());

    // Send the event to my parent window:
    wxWindow* pMyParentWindow = wxWindow::GetParent();

    if (pMyParentWindow)
        {
        pMyParentWindow->ProcessEvent(mouseEvent);
    }
    else
        {
        // I don't have a parent window:
        GT_ASSERT(0);
    }
}
*/

