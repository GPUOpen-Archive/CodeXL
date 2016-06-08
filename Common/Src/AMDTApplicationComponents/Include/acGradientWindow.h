//------------------------------ acGradientWindow.h ------------------------------

#ifndef __ACGRADIENTWINDOW_H
#define __ACGRADIENTWINDOW_H

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acCanvas.h>


// ----------------------------------------------------------------------------------
// Class Name:          AC_API acGradientWindow : public acCanvas
// General Description: A plain window with a gradient background
// Author:              Uri Shomroni
// Creation Date:       1/3/2011
// ----------------------------------------------------------------------------------
class AC_API acGradientWindow : public acCanvas
{
public:
    acGradientWindow(wxWindow* pParent, const wxColour& firstColor, const wxColour& secondColor, int direction = wxHORIZONTAL,
                     wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0,
                     const wxString& name = wxString("grad"));
    virtual ~acGradientWindow();

private:
    void onPaint(wxPaintEvent& event);

    DECLARE_EVENT_TABLE()

private:
    wxColour _firstColor;
    wxColour _secondColor;
    bool _isHorizontal;
};

#endif //__ACGRADIENTWINDOW_H

