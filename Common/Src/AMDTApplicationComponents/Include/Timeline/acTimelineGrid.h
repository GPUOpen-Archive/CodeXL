//==================================================================================
// Copyright (c) 2011 - 2016  , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimelineGrid.h
///
//==================================================================================

#ifndef _ACTIMELINEGRID_H_
#define _ACTIMELINEGRID_H_

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QWidget>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


static const int ACTIMELINEGRID_DefaultGridLabelSpace = 100;

/// Widget for the timeline grid (ruler) that appears along the top of the timeline.
class AC_API acTimelineGrid : public QWidget
{
    Q_OBJECT

    /// Gets or sets the label text for the timeline grid.  Defaults to "Scale".
    Q_PROPERTY(QString gridLabel READ gridLabel WRITE setGridLabel)

    /// Gets or sets the number of large divisions in the timeline grid.  Auto-calculated by default.  Defaults to 10.
    Q_PROPERTY(unsigned int largeDivisionCount READ largeDivisionCount WRITE setLargeDivisionCount)

    /// Gets or sets the number of small divisions within each large division.  Defaults to 5.
    Q_PROPERTY(unsigned int smallDivisionCount READ smallDivisionCount WRITE setSmallDivisionCount)

    /// Gets or sets a flag indicating whether or not the number of large division is auto-calculated
    /// based on the length of the string representing the end time of the timeline.  Defaults to true.
    Q_PROPERTY(bool autoCalculateLargeDivisionCount READ autoCalculateLargeDivisionCount WRITE setAutoCalculateLargeDivisionCount)

    /// Gets or sets the timestamp for the start of the timeline grid.  Defaults to zero.
    Q_PROPERTY(quint64 startTime READ startTime WRITE setStartTime)

    /// Gets or sets the full range of the timeline grid.  Defaults to zero.
    Q_PROPERTY(quint64 fullRange READ fullRange WRITE setFullRange)

    /// Gets or sets the timestamp for the start of the visible portion of the timeline grid.  Defaults to zero.
    Q_PROPERTY(quint64 visibleStartTime READ visibleStartTime WRITE setVisibleStartTime)

    /// Gets or sets the range of the visible portion of the timeline grid.  Defaults to zero.
    Q_PROPERTY(quint64 visibleRange READ visibleRange WRITE setVisibleRange)

    /// Gets or sets the width (in pixels) of the space allocated for the grid label.  Defaults to 100.
    Q_PROPERTY(int gridLabelSpace READ gridLabelSpace WRITE setGridLabelSpace)

    /// Gets or sets a flag indicating whether or not hints should be shown in the time grid for
    /// the selected time (either a single timestamp or a range of time).  Defaults to true.
    Q_PROPERTY(bool showTimeHint READ showTimeHint WRITE setShowTimeHint)

    /// Gets or sets the selected timestamp.  Used to display a hint in the time grid.
    Q_PROPERTY(quint64 selectedTime READ selectedTime WRITE setSelectedTime)

    /// Gets or sets the timestamp corresponding to the end of a selection.  Used to display a hint in the time grid.
    /// If different than "selectedTime", then three hints are shown (start of selection, end of selection, and
    /// duration of selection).
    Q_PROPERTY(quint64 endSelectedTime READ endSelectedTime WRITE setEndSelectedTime)

    /// Gets or sets the format string to used when displaying the duration of the selection in
    /// the duration hint.  Defaults to "%1 units".
    Q_PROPERTY(QString durationHintLabel READ durationHintLabel WRITE setDurationHintLabel)

    /// Gets or sets the scaling factor to apply to timestamps when displaying them on the grid.
    Q_PROPERTY(quint64 scalingFactor READ scalingFactor WRITE setScalingFactor)

public:
    /// Construct/initialize a new instance of the qcTimelineGrid class.
    /// \param parent the parent widget
    /// \param flags the window flags
    acTimelineGrid(QWidget* parent, Qt::WindowFlags flags = 0);

    /// Destroys instance of the qcTimelineGrid class.
    virtual ~acTimelineGrid() {}

    /// Sets the label text for the timeline grid.
    /// \param newGridLabel new value for the grid label.
    void setGridLabel(const QString newGridLabel);

    /// Gets the label text for the timeline grid.
    /// \return the label text for the timeline grid.
    QString gridLabel() const { return m_strGridLabel; }

    /// Sets the number of large divisions in the timeline grid.
    /// \param newLargeDivisionCount the number of large divisions in the timeline grid.
    void setLargeDivisionCount(const unsigned int newLargeDivisionCount);

    /// Gets the number of large divisions in the timeline grid.
    /// \return the number of large divisions in the timeline grid.
    unsigned int largeDivisionCount() const { return m_nLargeDivisionCount; }

    /// Sets the number of small divisions in the timeline grid.
    /// \param newSmallDivisionCount the number of small divisions in the timeline grid.
    void setSmallDivisionCount(const unsigned int newSmallDivisionCount);

    /// Gets the number of small divisions in the timeline grid.
    /// \return the number of small divisions in the timeline grid.
    unsigned int smallDivisionCount() const { return m_nSmallDivisionCount; }

    /// Sets a flag indicating whether or not the number of large division is auto-calculated
    /// based on the length of the string representing the end time of the timeline.
    /// \param newAutoCalculateLargeDivisionCount flag indicating whether or not the number of large division is auto-calculated
    void setAutoCalculateLargeDivisionCount(const bool newAutoCalculateLargeDivisionCount) { m_bAutoCalculateLargeDivisionCount = newAutoCalculateLargeDivisionCount; }

    /// Gets a flag indicating whether or not the number of large division is auto-calculated
    /// based on the length of the string representing the end time of the timeline.
    /// \return a flag indicating whether or not the number of large division is auto-calculated
    bool autoCalculateLargeDivisionCount() const { return m_bAutoCalculateLargeDivisionCount; }

    /// Sets the timestamp for the start of the timeline grid.
    /// \param newStartTime the start of the timeline grid.
    void setStartTime(const quint64 newStartTime);

    /// Gets the timestamp for the start of the timeline grid.
    /// \return the start of the timeline grid.
    quint64 startTime() const { return m_nStartTime; }

    /// Sets the full range of the timeline grid.
    /// \param newFullRange the full range of the timeline grid.
    void setFullRange(const quint64 newFullRange);

    /// Gets the full range of the timeline grid.
    /// \return the full range of the timeline grid.
    quint64 fullRange() const { return m_nFullRange; }

    /// Sets the timestamp for the start of the visible portion of the timeline grid.
    /// \param newVisibleStartTime the timestamp for the start of the visible portion of the timeline grid.
    void setVisibleStartTime(const quint64 newVisibleStartTime);

    /// Gets the timestamp for the start of the visible portion of the timeline grid.
    /// \return the timestamp for the start of the visible portion of the timeline grid.
    quint64 visibleStartTime() const { return m_nVisibleStartTime; }

    /// Sets the range of the visible portion of the timeline grid.
    /// \param newVisibleRange the range of the visible portion of the timeline grid.
    void setVisibleRange(const quint64 newVisibleRange);

    /// Gets the range of the visible portion of the timeline grid.
    /// \return the range of the visible portion of the timeline grid.
    quint64 visibleRange() const { return m_nVisibleRange; }

    /// Sets the width (in pixels) of the space allocated for the grid label.
    /// \param newGridLabelSpace the width (in pixels) of the space allocated for the grid label.
    void setGridLabelSpace(const int newGridLabelSpace);

    /// Gets the width (in pixels) of the space allocated for the grid label.
    /// \return the width (in pixels) of the space allocated for the grid label.
    int gridLabelSpace() const { return m_nGridLabelSpace; }

    /// Sets a flag indicating whether or not hints should be shown in the time grid for
    /// the selected time (either a single timestamp or a range of time).
    /// \param newShowTimeHint a flag indicating whether or not hints should be shown in the time grid for the selected time
    void setShowTimeHint(const bool newShowTimeHint) { m_bShowTimeHint = newShowTimeHint; }

    /// Gets a flag indicating whether or not hints should be shown in the time grid for
    /// the selected time (either a single timestamp or a range of time).
    /// \return a flag indicating whether or not hints should be shown in the time grid for the selected time
    bool showTimeHint() const { return m_bShowTimeHint; }

    /// Sets the selected timestamp.
    /// \param newSelectedTime the selected timestamp.
    void setSelectedTime(const quint64 newSelectedTime);

    /// Gets the selected timestamp.
    /// \return the selected timestamp.
    quint64 selectedTime() const { return m_nSelectedTime; }

    /// Sets the timestamp corresponding to the end of a selection.  Used to display a hint in the time grid.
    /// If different than "selectedTime", then three hints are shown (start of selection, end of selection, and
    /// duration of selection).
    /// \param newEndSelectedTime the timestamp corresponding to the end of a selection
    void setEndSelectedTime(const quint64 newEndSelectedTime);

    /// Gets the timestamp corresponding to the end of a selection.  Used to display a hint in the time grid.
    /// If different than "selectedTime", then three hints are shown (start of selection, end of selection, and
    /// duration of selection).
    /// \return the timestamp corresponding to the end of a selection
    quint64 endSelectedTime() const { return m_nEndSelectedTime; }

    /// Sets the format string to used when displaying the duration of the selection in
    /// the duration hint.
    /// \param newDurationHintLabel the format string to used when displaying the duration of
    /// the selection in the duration hint
    void setDurationHintLabel(const QString newDurationHintLabel);

    /// Gets the format string to used when displaying the duration of the selection in
    /// the duration hint.
    /// \return the format string to used when displaying the duration of
    /// the selection in the duration hint
    QString durationHintLabel() const { return m_strDurationHintLabel; }

    /// Sets the scaling factor to apply to timestamps when displaying them on the grid.
    /// \param newScalingFactor the scaling factor to apply to timestamps when displaying them on the grid.
    void setScalingFactor(const quint64 newScalingFactor);

    /// Gets the scaling factor to apply to timestamps when displaying them on the grid.
    /// \return the scaling factor to apply to timestamps when displaying them on the grid.
    quint64 scalingFactor() const { return m_nScalingFactor; }

    /// Overridden QWidget method that returns the preferred size of this widget
    /// \return the preferred size of this widget
    QSize sizeHint() const;

    void GetGridPosition(int& xPos, int& yPos) const { xPos = m_nGridLabelSpace; yPos = m_nGridRightXPosition; }

    /// Sets the grid labels precision (amount of digits after the dot [3 by default])
    /// \param precision the new precision
    void SetGridLabelsPrecision(unsigned int precision) { m_precision = precision; }

    /// Set the right margin for the grid
    /// \param right margin
    void SetRightMargins(unsigned int rightMargins) { m_rightMargins = rightMargins; }

protected:
    /// Overridden QWidget method called when this widget needs to be painted
    /// \param event the event parameters
    void paintEvent(QPaintEvent* event);

    /// Overridden QWidget method called when this widget is resized
    /// \param event the event parameters
    void resizeEvent(QResizeEvent* event);

private:
    static const int ms_GRID_MARGIN = 5; ///< blank space at top and bottom of grid

    /// Calculates the number of large divisions to show
    void calcLargeDivisionCount();

    /// Calculates a rect to be used to display text of a specified width at the specified timestamp
    /// \param textWidth the width of the text to display
    /// \param time the timestamp at which the hint is to be displayed
    QRect calcHintRect(int textWidth, quint64 time);

    /// Method called by paintEvent to draw the hints along the grid
    /// \param painter the QPainter instance to use for painting
    void drawTimeHints(QPainter& painter);

    /// Adds the unicode-left-arrow or unicode-right-arrow character to the given string, if
    /// the given timestamp is not within the visible range of the time grid
    void addOutOfRangeCharacterToHintString(QString& hintString, quint64 timeStamp);

    /// Ensures that m_ullVisibleStartTime and m_ullVisibleRange remain within the bounds of
    /// m_ullStartTime and m_ullFullRange
    void ensureValidVisibleRange();

    QString      m_strGridLabel;                     ///< The label text for the timeline grid.  Defaults to "Scale".

    unsigned int m_nLargeDivisionCount;              ///< The number of large divisions in the timeline grid.  Auto-calculated by default.  Defaults to 10.
    unsigned int m_nSmallDivisionCount;              ///< The number of small divisions within each large division.  Defaults to 5.
    bool         m_bAutoCalculateLargeDivisionCount; ///< A flag indicating whether or not the number of large division is auto-calculated based on the length of the string representing the end time of the timeline.  Defaults to true.

    quint64      m_nStartTime;                       ///< The timestamp for the start of the timeline grid.  Defaults to zero.
    quint64      m_nFullRange;                       ///< The full range of the timeline grid.  Defaults to zero.
    quint64      m_nVisibleStartTime;                ///< The timestamp for the start of the visible portion of the timeline grid.  Defaults to zero.
    quint64      m_nVisibleRange;                    ///< The range of the visible portion of the timeline grid.  Defaults to zero.

    quint64      m_nSelectedTime;                    ///< The selected timestamp.  Used to display a hint in the time grid.
    quint64      m_nEndSelectedTime;                 ///< The timestamp corresponding to the end of a selection.  Used to display a hint in the time grid. If different than "selectedTime", then three hints are shown (start of selection, end of selection, and duration of selection).

    bool         m_bShowTimeHint;                    ///< A flag indicating whether or not hints should be shown in the time grid for the selected time (either a single timestamp or a range of time).  Defaults to true.

    QString      m_strDurationHintLabel;             ///< The format string to used when displaying the duration of the selection in the duration hint.  Defaults to "%1 units".

    int          m_nGridLabelSpace;                  ///< The width (in pixels) of the space allocated for the grid label.  Defaults to 100.
    int          m_nGridSpace;                       ///< The width (in pixels) of the space allocated for the grid itself.  Equal to width() - m_nGridLabelSpace.
    int          m_nGridRightXPosition;               ///< The x coordinate for the right point of the grid

    quint64      m_nScalingFactor;                   ///< The scaling factor to apply to timestamps when displaying them on the grid.
    double       m_dInverseScalingFactor;            ///< The inverse of m_ullScalingFactor.

    float        m_fMaxTextWidth;                    ///< The maximum text width of a number shown on the grid (based on the end timestamp of the grid)
    int          m_nGridShortLineHeight;             ///< The height (in pixels) of the short lines drawn on the grid
    int          m_nGridLongLineHeight;              ///< The height (in pixels) of the long lines drawn on the grid
    int          m_nGridNumTextHeight;               ///< The height (in pixels) of the numbers drawn on the grid

    bool         m_bPaintEndTime;                    ///< Flag indicating whether or not the grid is wide enough to paint the end time
    unsigned int m_precision;                        ///< Amount of digits after the dot in the grid labels (3 by default)
    int          m_rightMargins;                     ///< Right side margins for the drawn grid
};

#endif // _ACTIMELINEGRID_H_
