//------------------------------ acStaticURLText.h ------------------------------

Yaki 15 / 5 / 2007
This file is deprecated. It was replaced by wxHyperlinkCtrl.

/*
#ifndef __ACSTATICURLTEXT
#define __ACSTATICURLTEXT

// ----------------------------------------------------------------------------------
// Class Name:           acStaticURLText : public wxStaticText
// General Description:
//   A static text that displays a URL name.
//   When clicking on the URL text, the URL will be launched in the default
//   machine browser.
//
// Author:               Yaki Tebeka
// Creation Date:        29/7/2004
// ----------------------------------------------------------------------------------
class acStaticURLText : public wxStaticText
{
public:
    acStaticURLText(const wxString& label, const wxString& urlToBeLaunched,
                    wxWindow* pParent, wxWindowID id = wxID_ANY,
                    const wxPoint& position = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = 0);

protected:
    void onMouseEnterWindow(wxMouseEvent& event);
    void onMouseLeaveWindow(wxMouseEvent& event);
    void onLeftMouseUp(wxMouseEvent& event);

    DECLARE_EVENT_TABLE()

private:
    // The url to be launched when the user clicks this static text:
    wxString _urlToBeLaunched;
};

#endif  // __ACSTATICURLTEXT
*/