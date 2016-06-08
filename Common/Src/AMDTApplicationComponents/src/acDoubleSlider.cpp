//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDoubleSlider.cpp
///
//==================================================================================

//------------------------------ acDoubleSlider.cpp ------------------------------


// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acDefinitions.h>
#include <AMDTApplicationComponents/Include/acDoubleSlider.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// Icons:
//#include <AMDTApplicationComponents/Include/res/icons/double_slider_tickbox.xpm>
//#include <AMDTApplicationComponents/Include/res/icons/double_slider_background_pattern.xpm>
//#include <AMDTApplicationComponents/Include/res/icons/double_slider_selected_region_pattern.xpm>
//#include <AMDTApplicationComponents/Include/res/icons/double_slider_above_region_pattern.xpm>
//#include <AMDTApplicationComponents/Include/res/icons/double_slider_below_region_pattern.xpm>

// Defines the size of the text boxes showing the values
#define AC_DOUBLE_SLIDER_TEXT_BOXES_SIZE QSize(76, 20)


// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::acQtDoubleSlider
// Description: Constructor
// Arguments:   QWidget* parent
// Author:      Sigal Algranaty
// Date:        27/6/2012
// ---------------------------------------------------------------------------
acQtDoubleSlider::acQtDoubleSlider(QWidget* parent) : QSlider(Qt::Horizontal, parent),
    m_sliderMinPossibleValue(0), m_sliderMaxPossibleValue(1), m_currentSliderLeftValue(0), m_currentSliderRightValue(1),
    m_offset(0), m_position(0), m_blockTracking(false), m_firstMovement(false), m_lastPressed(NoHandle), m_mainControl(LeftHandle),
    m_leftPressed(QStyle::SC_None), m_rightPressed(QStyle::SC_None), m_movement(acQtDoubleSlider::FreeMovement), m_ignoreInvalidText(false)
{
    // Connect the range change signal and slot:
    bool rc = connect(this, SIGNAL(rangeChanged(int, int)), this, SLOT(resetSliderRange(int, int)));
    GT_ASSERT(rc);

    // Set the initial range:
    emit spanChanged(m_sliderMinPossibleValue, m_sliderMaxPossibleValue);
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::~acQtDoubleSlider
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        28/6/2012
// ---------------------------------------------------------------------------
acQtDoubleSlider::~acQtDoubleSlider()
{
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::initStyleOption
// Description: Init a style option object
// Arguments:   QStyleOptionSlider* option
//              SpanHandle handle
// Author:      Sigal Algranaty
// Date:        2/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::initStyleOption(QStyleOptionSlider* pStyleOption, SpanHandle handle) const
{
    if (pStyleOption != NULL)
    {

        // Initialize from myself:
        pStyleOption->initFrom(this);

        //
        pStyleOption->subControls = QStyle::SC_None;
        pStyleOption->activeSubControls = QStyle::SC_None;
        pStyleOption->orientation = orientation();
        pStyleOption->maximum = maximum();
        pStyleOption->minimum = minimum();
        pStyleOption->tickPosition = tickPosition();
        pStyleOption->tickInterval = tickInterval();
        pStyleOption->upsideDown = false;
        pStyleOption->direction = Qt::LeftToRight;

        // Set the slider positions:
        pStyleOption->sliderPosition = (handle == LeftHandle ? leftPosition() : rightPosition());
        pStyleOption->sliderValue = (handle == LeftHandle ? leftMinValuePosition() : rightMaxValuePosition());
        pStyleOption->singleStep = singleStep();
        pStyleOption->pageStep = pageStep();
        pStyleOption->state |= QStyle::State_Horizontal;
    }
}


// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::pixelPosToRangeValue
// Description: Translates a pixel position to range value (pixel on the widget to
//              a range within the slider)
// Arguments:   int pos
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        1/7/2012
// ---------------------------------------------------------------------------
int acQtDoubleSlider::pixelPosToRangeValue(int pos) const
{
    int retVal = 0;

    // Define a style option object:
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    // Get the slider and groove rectangles:
    int sliderMin = 0;
    int sliderMax = 0;
    int sliderLength = 0;
    const QRect grooveRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
    const QRect sliderRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    // Get the slider length, min and max:
    sliderLength = sliderRect.width();
    sliderMin = grooveRect.x();
    sliderMax = grooveRect.right() - sliderLength + 1;

    // Get the slider value from position:
    retVal = QStyle::sliderValueFromPosition(minimum(), maximum(), pos - sliderMin, sliderMax - sliderMin, opt.upsideDown);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::setupPainter
// Description: Set the pPainter
// Arguments:   QPainter* pPainter
//              qreal x1
//              qreal y1
//              qreal x2
//              qreal y2
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        1/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::setupPainter(QPainter* pPainter, qreal x1, qreal y1, qreal x2, qreal y2, SpanType spanType) const
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pPainter != NULL)
    {
        // Get the highlight color:
        QColor bgColor = palette().color(QPalette::Highlight);

        if (spanType == LeftSpan)
        {
            bgColor = acQRAW_FILE_BELOW_RANGE_COLOR;
        }
        else if (spanType == RightSpan)
        {
            bgColor = acQRAW_FILE_ABOVE_RANGE_COLOR;
        }

        // Set the gradient for the brush:
        QLinearGradient gradient(x1, y1, x2, y2);
        gradient.setColorAt(0, bgColor.dark(120));
        gradient.setColorAt(1, bgColor.light(108));

        // Set the pPainter brush:
        pPainter->setBrush(gradient);

        // Set the pPainter pen:
        pPainter->setPen(QPen(bgColor.dark(130), 0));
    }
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::drawSpan
// Description: Draw the slider span
// Arguments:   QStylePainter* pPainter
//              const QRect& rect
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::drawSpan(QStylePainter* pPainter, const QRect& rect, SpanType spanType) const
{
    GT_IF_WITH_ASSERT(pPainter != NULL)
    {
        // Initialize style option class:
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        const QSlider* p = this;

        // Get the groove area:
        QRect groove = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
        groove.adjust(0, 0, -1, 0);

        // Set the pen & brush:
        pPainter->setPen(QPen(p->palette().color(QPalette::Dark).light(110), 0));

        // Set the painter:
        setupPainter(pPainter, groove.center().x(), groove.top(), groove.center().x(), groove.bottom(), spanType);

        // Draw the groove rect:
        pPainter->drawRect(rect.intersected(groove));
    }
}


// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::drawHandle
// Description: Draw the slider handle
// Arguments:   QStylePainter* painter
//              SpanHandle handle
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::drawHandle(QStylePainter* pPainter, SpanHandle handle) const
{
    GT_IF_WITH_ASSERT(pPainter != NULL)
    {
        // Initialize style option object:
        QStyleOptionSlider opt;
        initStyleOption(&opt, handle);
        opt.subControls = QStyle::SC_SliderHandle;

        // Set the handle state (pressed / not pressed):
        QStyle::SubControl pressed = (handle == LeftHandle ? m_leftPressed : m_rightPressed);

        if (pressed == QStyle::SC_SliderHandle)
        {
            opt.activeSubControls = pressed;
            opt.state |= QStyle::State_Sunken;
        }

        // Set the position according to the right / left handle:
        // If left value == right value set left handle to leftmost position,
        // right handle to rightmost position and disable dragging

        if (handle == acQtDoubleSlider::LeftHandle)
        {
            if (m_currentSliderLeftValue == m_currentSliderRightValue)
            {
                opt.sliderPosition = minimum();
            }
            else
            {
                opt.sliderPosition = sliderPositionFromValue(m_currentSliderLeftValue);
            }
        }

        else if (handle == acQtDoubleSlider::RightHandle)
        {
            if (m_currentSliderLeftValue == m_currentSliderRightValue)
            {
                opt.sliderPosition = maximum();
            }
            else
            {
                opt.sliderPosition = sliderPositionFromValue(m_currentSliderRightValue);
            }
        }

        pPainter->drawComplexControl(QStyle::CC_Slider, opt);
    }
}

void acQtDoubleSlider::triggerAction(QAbstractSlider::SliderAction action, bool main)
{
    float value = 0;
    bool noAction = false;
    bool up = false;
    const float min = m_sliderMinPossibleValue;
    const float max = m_sliderMaxPossibleValue;
    const SpanHandle altControl = (m_mainControl == LeftHandle ? RightHandle : LeftHandle);

    m_blockTracking = true;

    switch (action)
    {
        case QAbstractSlider::SliderSingleStepAdd:
            if ((main && m_mainControl == RightHandle) || (!main && altControl == RightHandle))
            {
                value = qBound(min, m_sliderMaxPossibleValue + singleStepValue(), max);
                up = true;
                break;
            }

            value = qBound(min, m_sliderMinPossibleValue + singleStepValue(), max);
            break;

        case QAbstractSlider::SliderSingleStepSub:
            if ((main && m_mainControl == RightHandle) || (!main && altControl == RightHandle))
            {
                value = qBound(min, m_sliderMaxPossibleValue - singleStepValue(), max);
                up = true;
                break;
            }

            value = qBound(min, m_sliderMinPossibleValue - singleStepValue(), max);
            break;

        case QAbstractSlider::SliderToMinimum:
            value = min;

            if ((main && m_mainControl == RightHandle) || (!main && altControl == RightHandle))
            {
                up = true;
            }

            break;

        case QAbstractSlider::SliderToMaximum:
            value = max;

            if ((main && m_mainControl == RightHandle) || (!main && altControl == RightHandle))
            {
                up = true;
            }

            break;

        case QAbstractSlider::SliderMove:
            if ((main && m_mainControl == RightHandle) || (!main && altControl == RightHandle))
            {
                up = true;
            }

            break;

        case QAbstractSlider::SliderNoAction:
            noAction = true;
            break;

        default:
            qWarning("acQtDoubleSlider::triggerAction: Unknown action");
            break;
    }

    if (!noAction && !up)
    {
        if (m_movement == acQtDoubleSlider::NoCrossing)
        {
            value = qMin(value, m_sliderMaxPossibleValue);
        }
        else if (m_movement == acQtDoubleSlider::NoOverlapping)
        {
            value = qMin(value, m_sliderMaxPossibleValue - 1);
        }

        if (m_movement == acQtDoubleSlider::FreeMovement && value > m_sliderMaxPossibleValue)
        {
            swapSliderHandles();
            setRightPosition(value);
        }
        else
        {
            setLeftPosition(value);
        }
    }
    else if (!noAction)
    {
        bool shouldSetValue = true;

        if (m_movement == acQtDoubleSlider::NoCrossing)
        {
            value = qMax(value, m_sliderMinPossibleValue);
        }
        else if (m_movement == acQtDoubleSlider::NoOverlapping)
        {
            value = qMax(value, m_sliderMinPossibleValue + 1);
        }
        else
        {
            shouldSetValue = false;
        }

        if (shouldSetValue)
        {
            if (m_movement == acQtDoubleSlider::FreeMovement && value < m_sliderMinPossibleValue)
            {
                swapSliderHandles();
                setLeftPosition(value);
            }
            else
            {
                setRightPosition(value);
            }
        }
    }

    m_blockTracking = false;

    setSpan(m_currentSliderLeftValue, m_currentSliderRightValue);
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::swapSliderHandles
// Description: Swap right and left handles
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::swapSliderHandles()
{
    qSwap(m_currentSliderRightValue, m_currentSliderLeftValue);
    qSwap(m_leftPressed, m_rightPressed);
    m_lastPressed = (m_lastPressed == LeftHandle ? RightHandle : LeftHandle);
    m_mainControl = (m_mainControl == LeftHandle ? RightHandle : LeftHandle);
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::resetSliderRange
// Description: Reset the range
// Arguments:   int min
//              int max
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::resetSliderRange(int min, int max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
    // setSpan() takes care of keeping span in range
    setSpan(m_sliderMinPossibleValue, m_sliderMaxPossibleValue);
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::leftValue
// Description: This property holds the m_sliderMinPossibleValue value of the span
// Return Val:  float
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
float acQtDoubleSlider::leftValue() const
{
    return qMin(m_sliderMinPossibleValue, m_sliderMaxPossibleValue);
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::rightValue
// Description: This property holds the m_sliderMaxPossibleValue value of the span
// Return Val:  float
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
float acQtDoubleSlider::rightValue() const
{
    return qMax(m_sliderMinPossibleValue, m_sliderMaxPossibleValue);
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::setSpan
// Description: Set the slider span
// Arguments:   float leftValue
//              float rightValue
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::setSpan(float leftValue, float rightValue)
{
    // Check if the new values are in the span:
    const float left = qBound(m_sliderMinPossibleValue, qMin(leftValue, m_currentSliderRightValue), m_sliderMaxPossibleValue);
    const float right = qBound(m_sliderMinPossibleValue, qMax(m_currentSliderLeftValue, rightValue), m_sliderMaxPossibleValue);

    if (left != m_currentSliderLeftValue || right != m_currentSliderRightValue)
    {
        // Set the new values:
        m_currentSliderLeftValue = left;
        m_currentSliderRightValue = right;

        // gtString dbg;
        // dbg.appendFormattedString(L"setSpan: %f, %f\n", m_currentSliderLeftValue, m_currentSliderRightValue);
        // osOutputDebugString(dbg);

        // Emit a span changed signal:
        emit spanChanged(m_currentSliderLeftValue, m_currentSliderRightValue);

        // Update the control:
        update();
    }
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::leftPosition
// Description: Return the slider left position
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
int acQtDoubleSlider::leftPosition() const
{
    int retVal = 0;

    retVal = sliderPositionFromValue(m_currentSliderLeftValue);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::leftMinValuePosition
// Description: Left minimum position
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
int acQtDoubleSlider::leftMinValuePosition() const
{
    int retVal = 0;

    retVal = sliderPositionFromValue(m_sliderMinPossibleValue);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::rightMaxValuePosition
// Description: Right maximum position
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
int acQtDoubleSlider::rightMaxValuePosition() const
{
    int retVal = 0;

    retVal = sliderPositionFromValue(m_sliderMaxPossibleValue);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::setLeftPosition
// Description: Set the slider left position
// Arguments:   int leftPosition
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::setLeftPosition(int leftPosition)
{
    int leftPos = sliderPositionFromValue(m_currentSliderLeftValue);

    if (leftPos != leftPosition)
    {

        float newLeftValue = sliderValueFromPosition(leftPosition);

        // Set the new span:
        setSpan(newLeftValue, m_currentSliderRightValue);

        // Update the slider:
        if (!hasTracking())
        {
            update();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::rightPosition
// Description: Get the slider right position
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
int acQtDoubleSlider::rightPosition() const
{
    int retVal = 0;

    retVal = sliderPositionFromValue(m_currentSliderRightValue);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::setRightPosition
// Description: Set the slider right position
// Arguments:   int rightPosition
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::setRightPosition(int rightPosition)
{
    int rightPos = sliderPositionFromValue(m_currentSliderRightValue);

    if (rightPos != rightPosition)
    {
        QStyleOptionSlider opt;
        initStyleOption(&opt);

        float newRightValue = sliderValueFromPosition(rightPosition);

        // Set the new span:
        setSpan(m_currentSliderLeftValue, newRightValue);

        // Update the control:
        if (!hasTracking())
        {
            update();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::keyPressEvent
// Description: Re-implements the key press event
// Arguments:   QKeyEvent* event
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::keyPressEvent(QKeyEvent* pEvent)
{
    // Call the base class implementation:
    QSlider::keyPressEvent(pEvent);

    // Sanity check
    GT_IF_WITH_ASSERT(pEvent != NULL)
    {
        bool main = true;
        SliderAction action = SliderNoAction;

        switch (pEvent->key())
        {
            case Qt::Key_Left:
                main   = true;
                action = !invertedAppearance() ? SliderSingleStepSub : SliderSingleStepAdd;
                break;

            case Qt::Key_Right:
                main   = true;
                action = !invertedAppearance() ? SliderSingleStepAdd : SliderSingleStepSub;
                break;

            case Qt::Key_Up:
                main   = false;
                action = invertedControls() ? SliderSingleStepSub : SliderSingleStepAdd;
                break;

            case Qt::Key_Down:
                main   = false;
                action = invertedControls() ? SliderSingleStepAdd : SliderSingleStepSub;
                break;

            case Qt::Key_Home:
                main   = (m_mainControl == acQtDoubleSlider::LeftHandle);
                action = SliderToMinimum;
                break;

            case Qt::Key_End:
                main   = (m_mainControl == acQtDoubleSlider::RightHandle);
                action = SliderToMaximum;
                break;

            default:
                pEvent->ignore();
                break;
        }

        // If an action is required, trigger action:
        if (action)
        {
            triggerAction(action, main);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::mousePressEvent
// Description: Is handling the mouse press event
// Arguments:   QMouseEvent* pEvent
// Author:      Sigal Algranaty
// Date:        2/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::mousePressEvent(QMouseEvent* pEvent)
{
    GT_IF_WITH_ASSERT(pEvent != NULL)
    {
        // Check if this event should be ignored:
        bool shouldIgnore = ((minimum() == maximum()) || (pEvent->buttons() ^ pEvent->button()));

        if (shouldIgnore)
        {
            pEvent->ignore();
        }
        else
        {
            // Get the mouse click position:
            QPoint mouseClickPosition = pEvent->pos();

            // Check if the clicked handle is the right handle:
            QStyleOptionSlider optRightHandle;
            initStyleOption(&optRightHandle, acQtDoubleSlider::RightHandle);

            // Save the old right handle state:
            QStyle::SubControl oldControl = m_rightPressed;

            // Get the value related for the right value:
            int value = sliderPositionFromValue(m_sliderMaxPossibleValue);

            // Perform a hit test for the right handle:
            m_rightPressed = style()->hitTestComplexControl(QStyle::CC_Slider, &optRightHandle, mouseClickPosition, this);

            QRect sliderRect = style()->subControlRect(QStyle::CC_Slider, &optRightHandle, QStyle::SC_SliderHandle, this);

            bool shouldUpdate = false;

            if (m_rightPressed == QStyle::SC_SliderHandle)
            {
                // gtString debugStr;
                // debugStr.appendFormattedString(L"m_rightPressed hit test was passed! value=%d, position=(%d, %d)\n", value, mouseClickPosition.x(), mouseClickPosition.y());
                // osOutputDebugString(debugStr);

                // Check if the widget should be updated:
                shouldUpdate = m_rightPressed != oldControl;
                m_position = value;
                m_offset = (mouseClickPosition - sliderRect.topLeft()).x();
                m_lastPressed = acQtDoubleSlider::RightHandle;
                setSliderDown(true);
            }
            else
            {
                oldControl = m_leftPressed;

                // Check if the clicked handle is the right handle:
                QStyleOptionSlider optLeftHandle;
                initStyleOption(&optLeftHandle, acQtDoubleSlider::LeftHandle);

                sliderRect = style()->subControlRect(QStyle::CC_Slider, &optLeftHandle, QStyle::SC_SliderHandle, this);
                // Perform a hit test for the right handle:
                m_leftPressed = style()->hitTestComplexControl(QStyle::CC_Slider, &optLeftHandle, mouseClickPosition, this);

                // Get the value related for the right value:
                value = sliderPositionFromValue(m_sliderMinPossibleValue);

                if (m_leftPressed == QStyle::SC_SliderHandle)
                {
                    // gtString debugStr;
                    // debugStr.appendFormattedString(L"m_leftPressed hit test was passed! value=%d, position=(%d, %d)\n", value, mouseClickPosition.x(), mouseClickPosition.y());
                    // osOutputDebugString(debugStr);

                    // Check if the widget should be updated:
                    shouldUpdate = m_leftPressed != oldControl;

                    m_position = value;
                    m_offset = (mouseClickPosition - sliderRect.topLeft()).x();
                    m_lastPressed = acQtDoubleSlider::LeftHandle;
                    setSliderDown(true);
                }
            }

            if (shouldUpdate)
            {
                update(sliderRect);
            }

            m_firstMovement = true;
            pEvent->accept();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::mouseMoveEvent
// Description: Re implements the mouse move event
// Arguments:   QMouseEvent* pEvent
// Author:      Sigal Algranaty
// Date:        1/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::mouseMoveEvent(QMouseEvent* pEvent)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pEvent != NULL)
    {
        if (m_leftPressed != QStyle::SC_SliderHandle && m_rightPressed != QStyle::SC_SliderHandle)
        {
            // Ignore the event:
            pEvent->ignore();
        }
        else
        {
            QStyleOptionSlider opt;
            initStyleOption(&opt);
            const int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);
            int newPosition = pixelPosToRangeValue(pEvent->pos().x() - m_offset);

            if (m >= 0)
            {
                const QRect r = rect().adjusted(-m, -m, m, m);

                if (!r.contains(pEvent->pos()))
                {
                    newPosition = m_position;
                }
            }

            // pick the preferred handle on the first movement
            if (m_firstMovement)
            {
                if (m_sliderMinPossibleValue == m_sliderMaxPossibleValue)
                {
                    if (newPosition < rightPosition())
                    {
                        swapSliderHandles();
                        m_firstMovement = false;
                    }
                }
                else
                {
                    m_firstMovement = false;
                }
            }

            if (m_leftPressed == QStyle::SC_SliderHandle)
            {
                if (m_movement == NoCrossing)
                {
                    newPosition = qMin(newPosition, rightPosition());
                }
                else if (m_movement == NoOverlapping)
                {
                    newPosition = qMin(newPosition, rightPosition() - 1);
                }

                if (m_movement == FreeMovement && newPosition > rightMaxValuePosition())
                {
                    swapSliderHandles();
                    setRightPosition(newPosition);
                }
                else
                {
                    setLeftPosition(newPosition);
                }
            }
            else if (m_rightPressed == QStyle::SC_SliderHandle)
            {
                if (m_movement == NoCrossing)
                {
                    newPosition = qMax(newPosition, rightPosition());
                }
                else if (m_movement == NoOverlapping)
                {
                    newPosition = qMax(newPosition, rightPosition() + 1);
                }

                if (m_movement == FreeMovement && newPosition < leftMinValuePosition())
                {
                    swapSliderHandles();
                    debugOutput(L"setLeftPosition: ", newPosition);
                    setLeftPosition(newPosition);
                }
                else
                {
                    debugOutput(L"setRightPosition: ", newPosition);
                    setRightPosition(newPosition);
                }
            }

            pEvent->accept();
        }
    }
}

/*!
\reimp
*/
void acQtDoubleSlider::mouseReleaseEvent(QMouseEvent* event)
{
    QSlider::mouseReleaseEvent(event);
    setSliderDown(false);
    m_leftPressed = QStyle::SC_None;
    m_rightPressed = QStyle::SC_None;
    update();
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::paintEvent
// Description: Re-implementation of the slider class paint event
// Arguments:   QPaintEvent* pEvent
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
void acQtDoubleSlider::paintEvent(QPaintEvent* pEvent)
{
    // Create a pPainter:
    Q_UNUSED(pEvent);
    QStylePainter painter(this);

    // Draw the slider ticks:
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    opt.subControls = QStyle::SC_SliderTickmarks;
    painter.drawComplexControl(QStyle::CC_Slider, opt);

    // Draw the slider groove:
    opt.sliderValue = 0;
    opt.sliderPosition = 0;
    opt.subControls = QStyle::SC_SliderGroove;
    painter.drawComplexControl(QStyle::CC_Slider, opt);

    // Get the slider handles rects:

    // Left handle:
    opt.sliderPosition = sliderPositionFromValue(m_currentSliderLeftValue);
    const QRect leftHandleRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    const int leftHandleRectX  = leftHandleRect.center().x();

    // Right handle:
    opt.sliderPosition = sliderPositionFromValue(m_currentSliderRightValue);
    const QRect rightHandleRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    const int rightHandleRectX  = rightHandleRect.center().x();

    // span
    const int minv = qMin(leftHandleRectX, rightHandleRectX);
    const int maxv = qMax(leftHandleRectX, rightHandleRectX);
    const QPoint grooveCenter = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this).center();

    // Draw the left span (red):
    QRect leftSpanRect = QRect(QPoint(0, grooveCenter.y() - 2), QPoint(minv, grooveCenter.y() + 1));
    drawSpan(&painter, leftSpanRect, LeftSpan);

    // Get the span rectangle:
    QRect spanRect = QRect(QPoint(minv, grooveCenter.y() - 2), QPoint(maxv, grooveCenter.y() + 1));
    drawSpan(&painter, spanRect, MiddleSpan);

    // Draw the right span (purple):
    int grooveRightEnd = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this).right();
    QRect rightSpanRect = QRect(QPoint(maxv, grooveCenter.y() - 2), QPoint(grooveRightEnd, grooveCenter.y() + 1));
    drawSpan(&painter, rightSpanRect, RightSpan);

    // Draw the slider handles:
    switch (m_lastPressed)
    {
        case acQtDoubleSlider::LeftHandle:
            drawHandle(&painter, acQtDoubleSlider::RightHandle);
            drawHandle(&painter, acQtDoubleSlider::LeftHandle);
            break;

        case acQtDoubleSlider::RightHandle:
        default:
            drawHandle(&painter, acQtDoubleSlider::LeftHandle);
            drawHandle(&painter, acQtDoubleSlider::RightHandle);
            break;
    }

}


// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::sliderPositionFromValue
// Description: Return the slider requested position for a requested slider float
//              value
// Arguments:   float sliderValue
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        2/7/2012
// ---------------------------------------------------------------------------
int acQtDoubleSlider::sliderPositionFromValue(float sliderValue) const
{
    int retVal = 0;

    // Get the slider range:
    int sliderRange = maximum() - minimum();

    // Get the slider full range:
    float sliderFullRange = m_sliderMaxPossibleValue - m_sliderMinPossibleValue;

    // ND check
    if (sliderFullRange != 0)
    {
        // Get the relative position:
        float relativePosition = (sliderValue - m_sliderMinPossibleValue) / sliderFullRange;
        retVal = sliderRange * relativePosition;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acQtDoubleSlider::sliderValueFromPosition
// Description: Return the slider value given the slider position in integer numbers
// Arguments:   int sliderPosition
// Return Val:  float
// Author:      Sigal Algranaty
// Date:        2/7/2012
// ---------------------------------------------------------------------------
float acQtDoubleSlider::sliderValueFromPosition(int sliderPosition) const
{
    float retVal = 0;

    // Get the slider range:
    int sliderRange = maximum() - minimum();

    if (sliderRange != 0)
    {
        float relativePosition = (float)(sliderPosition - minimum()) / (float)sliderRange;

        // Get the slider full range:
        retVal = m_sliderMinPossibleValue + relativePosition * (m_sliderMaxPossibleValue - m_sliderMinPossibleValue);
    }

    return retVal;
}

float acQtDoubleSlider::singleStepValue() const
{
    int step = singleStep();
    float retVal = sliderValueFromPosition(step);
    return retVal;
}

void acQtDoubleSlider::debugOutput(const gtString& prefix, int number)
{
#if 0
    gtString dbg;
    dbg.appendFormattedString(L"%ls = %d\n", prefix.asCharArray(), number);
    osOutputDebugString(dbg);
#else
    GT_UNREFERENCED_PARAMETER(prefix);
    GT_UNREFERENCED_PARAMETER(number);
#endif
}

// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::acDoubleSlider
// Description: Constructor
// Arguments:   itemId - The canvas item id.
//              position - The slider position inside the canvas.
//              width - The slider width (in pixels).
//              sliderMinPossibleVal - The slider minimum possible value
//              sliderMaxPossibleVal - The slider maximum possible value
// Author:      Eran Zinman
// Date:        2/11/2007
// ---------------------------------------------------------------------------
acDoubleSlider::acDoubleSlider(QWidget* pParent)
    : QWidget(pParent),
      m_sliderDataType(OA_FLOAT),
      m_floatAmountOfDigits(2),
      m_pDoubleSlider(NULL),
      m_pLeftValueTextCtrl(NULL),
      m_pRightValueTextCtrl(NULL),
      m_pMinValueText(NULL),
      m_pMaxValueText(NULL),
      m_needValidation(false)
{

    // Initialize the slider graphics
    initLayout();
}

// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::~acDobuleSlider
// Description: Destructor
// Author:      Eran Zinman
// Date:        2/11/2007
// ---------------------------------------------------------------------------
acDoubleSlider::~acDoubleSlider()
{
}

// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::displayTextError
// Description:
//   Displays a message box containing an error message of the format:
//   "Must be a numeric value between X and Y."
// Author:      Eran Zinman
// Date:        7/11/2007
// ---------------------------------------------------------------------------
void acDoubleSlider::displayTextError()
{
    GT_IF_WITH_ASSERT(m_pDoubleSlider != NULL)
    {
        // Build the error message:
        QString spinCtrlErrorMessage;
        spinCtrlErrorMessage.sprintf("Must be a numeric value between %.2f and %.2f", m_pDoubleSlider->m_sliderMinPossibleValue, m_pDoubleSlider->m_sliderMaxPossibleValue);

        // Display it to the user:
        acMessageBox::instance().critical("Error", spinCtrlErrorMessage);
    }
}

// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::setSliderPositions
// Description: Updates the slider left and right positions
// Arguments:   newSliderLeftPosition - New slider left position
//              newSliderRightPosition - New slider right position
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        7/11/2007
// ---------------------------------------------------------------------------
bool acDoubleSlider::setSliderPositions(double newSliderLeftPosition, double newSliderRightPosition)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pDoubleSlider != NULL)
    {
        // Check that we are inside the range
        if ((newSliderLeftPosition >= m_pDoubleSlider->m_sliderMinPossibleValue) && (newSliderLeftPosition <= m_pDoubleSlider->m_sliderMaxPossibleValue))
        {
            if ((newSliderRightPosition >= m_pDoubleSlider->m_sliderMinPossibleValue) && (newSliderRightPosition <= m_pDoubleSlider->m_sliderMaxPossibleValue))
            {
                // Update the slider left and right values
                m_pDoubleSlider->m_currentSliderLeftValue = newSliderLeftPosition;
                m_pDoubleSlider->m_currentSliderRightValue = newSliderRightPosition;

                // Check if we need to readjust the range:
                if (m_pDoubleSlider->m_currentSliderLeftValue > m_pDoubleSlider->m_currentSliderRightValue)
                {
                    m_pDoubleSlider->m_currentSliderRightValue = m_pDoubleSlider->m_currentSliderLeftValue;
                }
                // Check if we need to re range:
                else if (m_pDoubleSlider->m_currentSliderRightValue < m_pDoubleSlider->m_currentSliderLeftValue)
                {
                    m_pDoubleSlider->m_currentSliderLeftValue = m_pDoubleSlider->m_currentSliderRightValue;
                }

                // Update the tick boxes positions:
                updateTextControls();

                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::onDoubleSliderValuesChanged
// Description: Emit double slider position changed signal
// Author:      Sigal Algranaty
// Date:        28/6/2012
// ---------------------------------------------------------------------------
void acDoubleSlider::onDoubleSliderValuesChanged(double leftValue, double rightValue)
{
    // Notify my parent to my sliders position was changed:
    emit doubleSliderPositionChanged(leftValue, rightValue);


    // Update the text controls
    updateTextControls();
}


// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::onTextChange
// Description: Is called when the text is updated inside one of the range
//              text boxes. It enables validation
// Author:      Bhattacharyya Koushik
// Date:        25/09/2012
// ---------------------------------------------------------------------------
void acDoubleSlider::onTextChange()
{
    m_needValidation = true;
}

// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::onTextUpdate
// Description: Is called when the text is updated inside one of the range
//              text boxes. It checks if the input text is ok
// Arguments:   pEvent - The pEvent details
// Author:      Eran Zinman
// Date:        7/11/2007
// ---------------------------------------------------------------------------
void acDoubleSlider::onTextUpdate()
{
    if (!m_needValidation)
    {
        return;
    }

    m_needValidation = false;

    QLineEdit* pLineEditControl = qobject_cast<QLineEdit*>(sender());
    GT_IF_WITH_ASSERT(pLineEditControl != NULL)
    {
        // Get the new updated text, and trim it from left and right
        QString  updatedText = pLineEditControl->text().trimmed();

        // If the text box is not empty
        if (!updatedText.isEmpty())
        {
            // In order to make sure that the digits entered are building double number,
            // convert the string to a "double": and ignore the value
            bool isOK = false;
            double value = updatedText.toDouble(&isOK);

            // Sanity check
            GT_IF_WITH_ASSERT(m_pDoubleSlider != NULL)
            {
                if (((pLineEditControl == m_pLeftValueTextCtrl) && (value < m_pDoubleSlider->m_sliderMinPossibleValue)) ||
                    ((pLineEditControl == m_pRightValueTextCtrl) && (value > m_pDoubleSlider->m_sliderMaxPossibleValue)))
                {
                    isOK = false;
                }

            }

            if (!isOK)
            {
                // Notify the user that an error has occurred - value is incorrect
                displayTextError();

                // Revert the changes in the text controls:
                updateTextControls();

                // Set the focus back to the text control who triggered this error
                pLineEditControl->setFocus();
            }
        }// End of if (!updatedText.isEmpty())
    }
}

// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::updatePossibleValuesLabels
// Description: Updates the possible values labels content
// Author:      Eran Zinman
// Date:        29/12/2007
// ---------------------------------------------------------------------------
void acDoubleSlider::updatePossibleValuesLabels()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pMinValueText != NULL) && (m_pMaxValueText != NULL))
    {
        // Create the minimum and maximum values strings:
        QString minValue;
        QString maxValue;

        QString formatString;
        bool rcFormat = FormatStrByDataType(formatString);
        GT_ASSERT(rcFormat);

        GT_IF_WITH_ASSERT(!formatString.isEmpty())
        {
            minValue.sprintf(formatString.toLatin1().data(), m_pDoubleSlider->m_sliderMinPossibleValue);
            maxValue.sprintf(formatString.toLatin1().data(), m_pDoubleSlider->m_sliderMaxPossibleValue);
        }

        // Add a "min" and "max" text to the values:
        minValue.append(" min");
        QString maxValueLabel = "max ";
        maxValueLabel.append(maxValue);
        maxValue = maxValueLabel;

        // Update the labels
        m_pMinValueText->setText(minValue);
        m_pMaxValueText->setText(maxValue);
        updateGeometry();
    }
}

// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::initLayout
// Description: Initialize the slider layout
// Author:      Sigal Algranaty
// Date:        24/6/2012
// ---------------------------------------------------------------------------
void acDoubleSlider::initLayout()
{
    // Create the values text controls:
    m_pLeftValueTextCtrl = new acLineEdit(NULL);

    m_pLeftValueTextCtrl->resize(AC_DOUBLE_SLIDER_TEXT_BOXES_SIZE);

    m_pRightValueTextCtrl = new acLineEdit(NULL);

    m_pRightValueTextCtrl->resize(AC_DOUBLE_SLIDER_TEXT_BOXES_SIZE);

    // Reduce the text size inside the text controls
    QFont font = m_pLeftValueTextCtrl->font();
    font.setPointSize(font.pointSize() - 1);

    // Set the new font size and apply to both text controls
    m_pLeftValueTextCtrl->setFont(font);
    m_pRightValueTextCtrl->setFont(font);

    // Connect the text ctrl events:
    bool rc;
    rc = connect(m_pLeftValueTextCtrl, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChange()));
    GT_ASSERT(rc);

    rc = connect(m_pRightValueTextCtrl, SIGNAL(textChanged(const QString&)), this, SLOT(onTextChange()));
    GT_ASSERT(rc);

    rc = connect(m_pLeftValueTextCtrl, SIGNAL(editingFinished()), this, SLOT(onTextUpdate()));
    GT_ASSERT(rc);
    rc = connect(m_pRightValueTextCtrl, SIGNAL(editingFinished()), this, SLOT(onTextUpdate()));
    GT_ASSERT(rc);

    // Create the grid layout for the slider:
    QGridLayout* pGridLayout = new QGridLayout;


    // Add the right and left text controls:
    pGridLayout->addWidget(m_pLeftValueTextCtrl, 0, 0, Qt::AlignLeft);
    pGridLayout->addWidget(m_pRightValueTextCtrl, 0, 1, Qt::AlignRight);

    // Create the double slider:
    m_pDoubleSlider = new acQtDoubleSlider;

    pGridLayout->addWidget(m_pDoubleSlider , 1, 0, 1, 2);

    rc = connect(m_pDoubleSlider, SIGNAL(spanChanged(double, double)), this, SLOT(onDoubleSliderValuesChanged(double, double)));
    GT_ASSERT(rc);


    // Updates the text controls values
    updateTextControls();

    // Now create them minimum and maximum values indicators:
    m_pMinValueText = new QLabel;


    m_pMaxValueText = new QLabel;


    // Updates the possible values labels content
    updatePossibleValuesLabels();

    pGridLayout->addWidget(m_pMinValueText, 2, 0, Qt::AlignLeft);
    pGridLayout->addWidget(m_pMaxValueText, 2, 1, Qt::AlignRight);

    // Set my layout:
    setLayout(pGridLayout);

    layout();
}


// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::setSliderMinMaxPossibleValues
// Description: Sets the slider min / max values
// Arguments:   double minPossibleValue
//              double maxPossibleValue
// Author:      Sigal Algranaty
// Date:        3/7/2012
// ---------------------------------------------------------------------------
void acDoubleSlider::setSliderMinMaxPossibleValues(double minPossibleValue, double maxPossibleValue)
{
    float normalizedMaxPossibleValue = maxPossibleValue;
    float normalizedMinPossibleValue = minPossibleValue;

    // Calculate the requested digits for float display:
    double diff = fabs(maxPossibleValue - minPossibleValue);

    if (diff < 0.01)
    {
        m_floatAmountOfDigits = 3;
    }

    if (diff < 0.001)
    {
        m_floatAmountOfDigits = 4;
    }

    if (diff < 0.0001)
    {
        m_floatAmountOfDigits = 4;

        // Cut the float numbers to contain only 4 digits:
        QString number;
        number.sprintf("%2.4f", (float)maxPossibleValue);
        normalizedMaxPossibleValue = number.toDouble();
        number = "";
        number.sprintf("%2.4f", (float)minPossibleValue);
        normalizedMinPossibleValue = number.toDouble();
    }

    // Save the new minimum and maximum possible values
    m_pDoubleSlider->m_sliderMinPossibleValue = normalizedMinPossibleValue;
    m_pDoubleSlider->m_sliderMaxPossibleValue = normalizedMaxPossibleValue;

    m_pDoubleSlider->m_currentSliderLeftValue = m_pDoubleSlider->m_sliderMinPossibleValue;
    m_pDoubleSlider->m_currentSliderRightValue = m_pDoubleSlider->m_sliderMaxPossibleValue;

    // Updates the possible values labels content
    updatePossibleValuesLabels();
    updateTextControls();
}

// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::setSliderDataType
// Description: Defines the slider data type and set the minimum and maximum
//              possible values according to the data type
// Arguments:   dataType - The slider data type
//              bool isOpenGL - is the data type displayed in OpenGL (for OpenGL
//.             the float values are in the range of 0 and 1)
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        4/11/2007
// ---------------------------------------------------------------------------
bool acDoubleSlider::setSliderDataType(oaDataType dataType, bool isOpenGL)
{
    bool retVal = true;

    // Save the new data type
    m_sliderDataType = dataType;

    // Change the possible min and max values according to the data type
    switch (dataType)
    {
        case OA_UNSIGNED_BYTE:
        {
            m_pDoubleSlider->m_sliderMinPossibleValue = 0;
            m_pDoubleSlider->m_sliderMaxPossibleValue = UCHAR_MAX;
        }
        break;

        case OA_BYTE:
        {
            m_pDoubleSlider->m_sliderMinPossibleValue = CHAR_MIN;
            m_pDoubleSlider->m_sliderMaxPossibleValue = CHAR_MAX;
        }
        break;

        case OA_UNSIGNED_SHORT:
        {
            m_pDoubleSlider->m_sliderMinPossibleValue = 0;
            m_pDoubleSlider->m_sliderMaxPossibleValue = USHRT_MAX;
        }
        break;

        case OA_SHORT:
        {
            m_pDoubleSlider->m_sliderMinPossibleValue = SHRT_MIN;
            m_pDoubleSlider->m_sliderMaxPossibleValue = SHRT_MAX;
        }
        break;

        case OA_UNSIGNED_INT:
        {
            m_pDoubleSlider->m_sliderMinPossibleValue = (float)0;
            m_pDoubleSlider->m_sliderMaxPossibleValue = (float)UINT_MAX;
        }
        break;

        case OA_INT:
        {
            m_pDoubleSlider->m_sliderMinPossibleValue = (float)INT_MIN;
            m_pDoubleSlider->m_sliderMaxPossibleValue = (float)INT_MAX;
        }
        break;

        case OA_UNSIGNED_LONG:
        {
            m_pDoubleSlider->m_sliderMinPossibleValue = (float)0;
            m_pDoubleSlider->m_sliderMaxPossibleValue = (float)ULLONG_MAX;
        }
        break;

        case OA_LONG:
        {
            m_pDoubleSlider->m_sliderMinPossibleValue = (float)LLONG_MIN;
            m_pDoubleSlider->m_sliderMaxPossibleValue = (float)LLONG_MAX;
        }
        break;

        case OA_FLOAT:
        {
            if (isOpenGL)
            {
                // In openGL min and max for float are [0..1]
                m_pDoubleSlider->m_sliderMinPossibleValue = 0.0f;
                m_pDoubleSlider->m_sliderMaxPossibleValue = 1.0f;
            }
            else
            {
                m_pDoubleSlider->m_sliderMinPossibleValue = FLT_MAX * -1;
                m_pDoubleSlider->m_sliderMaxPossibleValue = FLT_MAX;
            }
        }
        break;

        case OA_DOUBLE:
        {
            if (isOpenGL)
            {
                // In openGL min and max for float are [0..1]
                m_pDoubleSlider->m_sliderMinPossibleValue = 0.0;
                m_pDoubleSlider->m_sliderMaxPossibleValue = 1.0;
            }
            else
            {
                m_pDoubleSlider->m_sliderMinPossibleValue = FLT_MAX * -1;
                m_pDoubleSlider->m_sliderMaxPossibleValue = FLT_MAX;
            }
        }
        break;

        default:
        {
            GT_ASSERT_EX(false, L"Unsupported Data Type!");
            retVal = false;
        }
        break;
    }

    GT_IF_WITH_ASSERT(retVal)
    {
        m_pDoubleSlider->m_currentSliderLeftValue = m_pDoubleSlider->m_sliderMinPossibleValue;
        m_pDoubleSlider->m_currentSliderRightValue = m_pDoubleSlider->m_sliderMaxPossibleValue;

        // Update min and max labels and tick positions:
        updatePossibleValuesLabels();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::updateTextControls
// Description: Updates the text controls values
// Author:      Eran Zinman
// Date:        4/11/2007
// ---------------------------------------------------------------------------
void acDoubleSlider::updateTextControls()
{
    GT_IF_WITH_ASSERT(m_pLeftValueTextCtrl && m_pRightValueTextCtrl)
    {
        // Create the strings:
        QString leftTextCtrlValue;
        QString rightTextCtrlValue;

        QString formatString;
        bool rcFormat = FormatStrByDataType(formatString);
        GT_ASSERT(rcFormat);

        GT_IF_WITH_ASSERT(!formatString.isEmpty())
        {
            // Show float values:
            leftTextCtrlValue.sprintf(formatString.toLatin1().data(), m_pDoubleSlider->m_currentSliderLeftValue);
            rightTextCtrlValue.sprintf(formatString.toLatin1().data(), m_pDoubleSlider->m_currentSliderRightValue);
        }

        // Update the text controls
        m_pLeftValueTextCtrl->setText(leftTextCtrlValue);
        m_pRightValueTextCtrl->setText(rightTextCtrlValue);

        // Check if the text controls should be enabled:
        bool isEnabled = (m_pDoubleSlider->m_sliderMaxPossibleValue > m_pDoubleSlider->m_sliderMinPossibleValue);

        // Enable / Disable the text controls:
        m_pLeftValueTextCtrl->setEnabled(isEnabled);
        m_pRightValueTextCtrl->setEnabled(isEnabled);
    }
}


// ---------------------------------------------------------------------------
// Name:        acDoubleSlider::FormatStrByDataType
// Description: Return number format string according to the data type
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/4/2011
// ---------------------------------------------------------------------------
bool acDoubleSlider::FormatStrByDataType(QString& formatStr)
{
    bool retVal = true;

    // Should we show floating point values?
    switch (m_sliderDataType)
    {
        case OA_FLOAT:
        case OA_DOUBLE:
        {
            // Show float values:
            // Notice: By default the float precision is 2 digits.
            // The precision can be changed if the values entered differences are smaller then 0.01:
            formatStr.clear();
            formatStr.sprintf("%%2.%df", m_floatAmountOfDigits);
        }
        break;

        // If normal numeric value (without floating point) just show round number:
        case OA_UNSIGNED_BYTE:
        case OA_BYTE:
        case OA_UNSIGNED_SHORT:
        case OA_SHORT:
        case OA_UNSIGNED_INT:
        case OA_INT:
        case OA_UNSIGNED_LONG:
        case OA_LONG:
        {
            formatStr = "%2.0f";
        }
        break;

        default:
        {
            formatStr = "%2.0f";
            GT_ASSERT_EX(false, L"Unsupported Data Type!");
            retVal = false;
        }
        break;
    }

    return retVal;
}
