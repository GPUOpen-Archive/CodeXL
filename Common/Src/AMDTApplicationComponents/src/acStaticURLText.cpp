//------------------------------ acStaticURLText.cpp ------------------------------

Yaki 15 / 5 / 2007
This file is deprecated. It was replaced by wxHyperlinkCtrl.

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Avi - 27/12/2006: Replace the usage of acStaticURLText With wxHyperlinkCtrl
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/*
// wxWindows pre compiled header:
#include <AMDTApplicationComponents/Include/acWxWidgetsIncludes.h>

// Infra:
#include <AMDTOSWrappers/Include/osFileLauncher.h>

// Local:
#include <AMDTApplicationComponents/Include/acStaticURLText.h>

// acStaticURLText event table:
BEGIN_EVENT_TABLE(acStaticURLText, wxStaticText)
    EVT_LEFT_UP(acStaticURLText::onLeftMouseUp)
    EVT_ENTER_WINDOW(acStaticURLText::onMouseEnterWindow)
    EVT_LEAVE_WINDOW(acStaticURLText::onMouseLeaveWindow)
END_EVENT_TABLE()



// ---------------------------------------------------------------------------
// Name:        acStaticURLText::acStaticURLText
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
acStaticURLText::acStaticURLText(const wxString& label, const wxString& urlToBeLaunched,
                                 wxWindow* pParent, wxWindowID id,
                                 const wxPoint& position, const wxSize& size,
                                 long style)
: wxStaticText(pParent, id, label, position, size, style), _urlToBeLaunched(urlToBeLaunched)
{
    wxFont font;

    font.SetPointSize(9);
    font.SetUnderlined(true);
//  font.SetWeight(wxBOLD);

    SetForegroundColour(*wxBLUE);

    this->SetFont(font);
}


// ---------------------------------------------------------------------------
// Name:        acStaticURLText::onMouseEnterWindow
// Description: Is called when the mouse enters this static text area.
// Arguments:   event - The mouse event.
// Author:      Yaki Tebeka
// Date:        29/7/2004
// ---------------------------------------------------------------------------
void acStaticURLText::onMouseEnterWindow(wxMouseEvent& event)
{
    SetCursor(wxCursor(wxCURSOR_HAND));
}


// ---------------------------------------------------------------------------
// Name:        acStaticURLText::onMouseLeaveWindow
// Description: Is called when the mouse leaves this static text area.
// Arguments:   event - The mouse event.
// Author:      Yaki Tebeka
// Date:        29/7/2004
// ---------------------------------------------------------------------------
void acStaticURLText::onMouseLeaveWindow(wxMouseEvent& event)
{
    SetCursor(wxCursor(wxCURSOR_DEFAULT));
}


// ---------------------------------------------------------------------------
// Name:        acStaticURLText::onLeftMouseUp
// Description: Is called when the left mouse button is released on top of this
//              static text area.
// Arguments:   event - The mouse event.
// Author:      Yaki Tebeka
// Date:        29/7/2004
// ---------------------------------------------------------------------------
void acStaticURLText::onLeftMouseUp(wxMouseEvent& event)
{
    // Launch the URL:
    osFileLauncher fileLauncher(_urlToBeLaunched.c_str());
    fileLauncher.launchFile();
}

*/