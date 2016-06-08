//------------------------------ acGradientWindow.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Local:
#include <AMDTApplicationComponents/Include/acWxWidgetsIncludes.h>
#include <AMDTApplicationComponents/Include/acGradientWindow.h>

BEGIN_EVENT_TABLE(acGradientWindow, acCanvas)
    EVT_PAINT(acGradientWindow::onPaint)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// Name:        acGradientWindow::acGradientWindow
// Description: Constructor
// Author:      Uri Shomroni
// Date:        1/3/2011
// ---------------------------------------------------------------------------
acGradientWindow::acGradientWindow(wxWindow* pParent, const wxColour& firstColor, const wxColour& secondColor, int direction,
                                   wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : acCanvas(pParent, id, pos, size, style, name), _firstColor(firstColor), _secondColor(secondColor), _isHorizontal(direction != wxVERTICAL)
{
    GT_ASSERT((direction == wxHORIZONTAL) || (direction == wxVERTICAL));
}

// ---------------------------------------------------------------------------
// Name:        acGradientWindow::~acGradientWindow
// Description: Destructor
// Author:      Uri Shomroni
// Date:        1/3/2011
// ---------------------------------------------------------------------------
acGradientWindow::~acGradientWindow()
{
}

// ---------------------------------------------------------------------------
// Name:        acGradientWindow::onPaint
// Description: Called when the window is getting redrawn
// Author:      Uri Shomroni
// Date:        1/3/2011
// ---------------------------------------------------------------------------
void acGradientWindow::onPaint(wxPaintEvent& event)
{
    // Get the window size:
    wxSize currentWindowSize = GetSize();
    wxRect rectForFill(wxPoint(0, 0), currentWindowSize);

    // Prepare a device context:
    wxPaintDC canvasDC(this);
    PrepareDC(canvasDC);

    // Draw the gradient:
    canvasDC.GradientFillLinear(rectForFill, _firstColor, _secondColor, _isHorizontal ? wxEAST : wxSOUTH);

    // Call the base class's implementation, to allow any canvas items to be added:
    acCanvas::onPaint(event);
    event.Skip();
}

