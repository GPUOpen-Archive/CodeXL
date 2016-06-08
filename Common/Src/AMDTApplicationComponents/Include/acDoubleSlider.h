//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDoubleSlider.h
///
//==================================================================================

//------------------------------ acDoubleSlider.h ------------------------------

#ifndef __ACDOUBLESLIDER
#define __ACDOUBLESLIDER

// Qt:
#include <QtWidgets>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QStyle>
#include <QStylePainter>

// Infra:
#include <AMDTOSAPIWrappers/Include/oaDataType.h>

// Local:
#include <AMDTApplicationComponents/Include/acImageItem.h>
#include <AMDTApplicationComponents/Include/acLineEdit.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          AC_API acQtDoubleSlider : public QSlider
// General Description: Implement the Qt control for the double slider
// Author:              Sigal Algranaty
// Creation Date:       3/7/2012
// ----------------------------------------------------------------------------------
class AC_API acQtDoubleSlider : public QSlider
{
    Q_OBJECT

public:

    friend class acDoubleSlider;
    acQtDoubleSlider(QWidget* parent = 0);
    virtual ~acQtDoubleSlider();

    // Get slider left and right values
    double sliderLeftValue() const { return m_currentSliderLeftValue; };
    double sliderRightValue() const { return m_currentSliderRightValue; };


    enum HandleMovementMode
    {
        FreeMovement,
        NoCrossing,
        NoOverlapping
    };

    enum SpanHandle
    {
        NoHandle,
        LeftHandle,
        RightHandle
    };

    enum SpanType
    {
        LeftSpan,
        MiddleSpan,
        RightSpan
    };

    float leftValue() const;
    float rightValue() const;

    int rightPosition() const;
    int leftPosition() const;

public slots:

    void setSpan(float leftValue, float rightValue);

    void setLeftPosition(int leftPosition);
    void setRightPosition(int rightPosition);
    void resetSliderRange(int min, int max);

signals:
    void spanChanged(double leftValue, double rightValue);

protected:

    virtual void keyPressEvent(QKeyEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void paintEvent(QPaintEvent* event);

private:

    void initStyleOption(QStyleOptionSlider* option, SpanHandle handle = RightHandle) const;
    int pixelPosToRangeValue(int pos) const;
    void drawHandle(QStylePainter* painter, SpanHandle handle) const;
    void setupPainter(QPainter* painter, qreal x1, qreal y1, qreal x2, qreal y2, SpanType spanType) const;
    void drawSpan(QStylePainter* painter, const QRect& rect, SpanType spanType) const;
    void triggerAction(QAbstractSlider::SliderAction action, bool main);
    void swapSliderHandles();
    int sliderPositionFromValue(float sliderValue) const;
    float sliderValueFromPosition(int sliderPosition) const;

    int leftMinValuePosition() const;
    int rightMaxValuePosition() const;

    // Debugging utility:
    void debugOutput(const gtString& prefix, int number);

    float singleStepValue() const ;
private:

    // The slider min and max possible values:
    float m_sliderMinPossibleValue;
    float m_sliderMaxPossibleValue;

    // The slider position (measured in virtual units):
    float m_currentSliderLeftValue;
    float m_currentSliderRightValue;

    int m_offset;
    int m_position;

    bool m_blockTracking;
    bool m_firstMovement;

    SpanHandle m_lastPressed;
    SpanHandle m_mainControl;
    QStyle::SubControl m_leftPressed;
    QStyle::SubControl m_rightPressed;
    HandleMovementMode m_handleMovementMode;
    HandleMovementMode m_movement;

    // Avoid displaying invalid text message twice:
    bool m_ignoreInvalidText;


};

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acDoubleSlider : public QWidget
// General Description:  A slider that have two edges and defines a certain range.
// Author:              Sigal Algranaty
// Creation Date:       24/6/2012
// ----------------------------------------------------------------------------------
class AC_API acDoubleSlider : public QWidget
{
    Q_OBJECT

public:
    // Constructor:
    acDoubleSlider(QWidget* pParent = NULL);

    // Destructor:
    ~acDoubleSlider();

public:

    // Public access functions:
    bool setSliderPositions(double newSliderLeftPosition, double newSliderRightPosition);

    // Defines the slider data type
    bool setSliderDataType(oaDataType dataType, bool isOpenGL);

    // Sets the double slider minimum and maximum possible values
    void setSliderMinMaxPossibleValues(double minPossibleValue, double maxPossibleValue);

    // Get slider left and right values
    double sliderLeftValue() const { return m_pDoubleSlider->m_currentSliderLeftValue; };
    double sliderRightValue() const { return m_pDoubleSlider->m_currentSliderRightValue; };

signals:

    // Pixel position changed signal:
    void doubleSliderPositionChanged(double leftValue, double rightValue);

protected:

    // Initialize the slider graphics
    void initLayout();

    // Updates the text controls values
    void updateTextControls();

    // Displays a message box containing an error message when numeric range is invalid
    void displayTextError();

    // Updates the possible values labels content
    void updatePossibleValuesLabels();

    // Return number format string according to the data type:
    bool FormatStrByDataType(QString& formatStr);

protected slots:

    void onTextChange();
    void onTextUpdate();
    void onDoubleSliderValuesChanged(double leftPos, double rightPos);

protected:

    // The double slider data type
    oaDataType m_sliderDataType;

    // Float amount of digits:
    int m_floatAmountOfDigits;

    // Double slider:
    acQtDoubleSlider* m_pDoubleSlider;

    // The text controls holding the left and right tick box values:
    acLineEdit* m_pLeftValueTextCtrl;
    acLineEdit* m_pRightValueTextCtrl;

    // The label for the min / max values:
    QLabel* m_pMinValueText;
    QLabel* m_pMaxValueText;

    // To avoid duplicate calls
    bool m_needValidation;
};

#endif  // __ACDOUBLESLIDER
