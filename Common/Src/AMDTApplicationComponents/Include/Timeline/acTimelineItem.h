//==================================================================================
// Copyright (c) 2011 - 2016  , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimelineItem.h
///
//==================================================================================

#ifndef _ACTIMELINEITEM_H_
#define _ACTIMELINEITEM_H_

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QMap>
#include <QList>
#include <QWidget>
#include <QPainter>

// forward declarations
class acTimeline;
class acTimelineBranch;
class acTimelineItemToolTip;

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

/// Timeline item class
class AC_API acTimelineItem : public QObject
{
    Q_OBJECT

public:

    enum ItemGraphicShape
    {
        AC_TIMELINE_RECTANGLE,
        AC_TIMELINE_LINE,
        AC_TIMELINE_DOT
    };

    /// Construct/initialize a new instance of the qcTimelineItem class.
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    acTimelineItem(quint64 startTime, quint64 endTime);

    /// Destroys an instance of the acTimeline class.
    virtual ~acTimelineItem() {}

    /// Gets the parent branch for this timeline item.
    /// \return the parent branch for this timeline item.
    acTimelineBranch* parentBranch() const { return m_pParentBranch; }

    /// Sets the parent branch for this timeline item. Should only be called by the branch.
    /// \param newParentBranch the new parent branch for this timeline item.
    void setParentBranch(acTimelineBranch* newParentBranch);

    /// Gets the background color for this timeline item.
    /// \return the background color for this timeline item.
    QColor backgroundColor() const { return m_backgroundColor; }

    /// Sets the background color for this timeline item.
    /// \param newColor the new background color for this timeline item.
    void setBackgroundColor(const QColor& newColor);

    /// Gets the lightened background color for this timeline item.  This color is used when doing gradient painting.
    /// Note: if not explicitly set, then a lightened version of backgroundColor is used.
    /// \return the lightened background color for this timeline item.
    QColor lightenedBackgroundColor();

    /// Sets the lightened background color for this timeline item.  This color is used when doing gradient painting.
    /// Note: if not explicitly set, then a lightened version of backgroundColor is used.
    /// \param newColor the lightened background color for this timeline item.
    void setLightenedBackgroundColor(const QColor& newColor);

    /// Gets the foreground color (font color) for this timeline item.
    /// \return the foreground color (font color) for this timeline item.
    QColor foregroundColor() const { return m_foregroundColor; }

    /// Sets the foreground color (font color) for this timeline item.
    /// \param newColor the foreground color (font color) for this timeline item.
    void setForegroundColor(const QColor& newColor) { m_foregroundColor = newColor; }

    /// Sets the outline color for this timeline item. The outline is shown when the mouse hovers over the item.
    /// \return the outline color for this timeline item.
    QColor outlineColor() const { return m_outlineColor; }

    /// Sets the outline color for this timeline item. By default the outline is shown when the mouse hovers over the item.
    /// \param newColor the outline color for this timeline item.
    void setOutlineColor(const QColor& newColor) { m_outlineColor = newColor; }

    /// Gets a flag indicating if this item was visible during the most recent paint pass (i.e not culled away, not scrolled out of view).
    /// \return true if the item is visible, false otherwise.
    bool isVisible() const { return m_bVisible; }

    /// Gets the draw rectangle for this item as calculated during the most recent paint pass.
    /// \return the draw rectangle for this item as calculated during the most recent paint pass.
    QRect drawRectangle() const { return m_rect; }

    /// Gets the text for this timeline item.
    /// \return the text for this timeline item.
    QString text() const { return m_strText; }

    /// Sets the text for this timeline item.
    /// \param newText the text for this timeline item.
    void setText(const QString newText) { m_strText = newText; }

    /// Gets the text shown in the tooltip when the mouse hovers over this timeline item.
    /// \return the text shown in the tooltip when the mouse hovers over this timeline item.
    QString tooltipText() const;

    /// Gets the start time for this timeline item.
    /// \return the start time for this timeline item.
    quint64 startTime() const { return m_nStartTime; }

    /// Sets the start time for this timeline item.
    /// \param newStartTime the new start time for this timeline item.
    void setStartTime(const quint64 newStartTime);

    /// Gets the end time for this timeline item.
    /// \return the end time for this timeline item.
    quint64 endTime() const { return m_nEndTime; }

    /// Sets the end time for this timeline item.
    /// \param newEndTime the new end time for this timeline item.
    void setEndTime(const quint64 newEndTime);

    /// Gets the index of this item within the owning branch.
    /// \return the index of this item within the owning branch.
    int index();

    /// Gets the fractional offset of this timeline item. This Should be a value between 0 and 1.
    /// This represents the vertical offset of the top of a timeline item within its parent branch.
    /// The fraction is multiplied by the branch height and then added to the branch top to get the top of the timeline item.
    /// A value of Zero indicates that the top of the timeline item will be equal to the top of the parent branch.
    /// \return the fractional offset of this timeline item.
    double fractionalOffset() const { return m_dFractionalOffset; }

    /// Gets the fractional offset of this timeline item. This Should be a value between 0 and 1.
    /// This represents the vertical offset of the top of a timeline item within its parent branch.
    /// The fraction is multiplied by the branch height and then added to the branch top to get the top of the timeline item.
    /// A value of Zero indicates that the top of the timeline item will be equal to the top of the parent branch.
    /// \param newFractionalOffset the fractional offset of this timeline item.
    void  setFractionalOffset(const double newFractionalOffset);

    /// Gets the fractional height of this timeline item. This Should be a value between 0 and 1.
    /// The fraction is multiplied by the branch height to get the height of this timeline item.
    /// A value of One indicates that the height of the item will be equal to the height of the branch.
    /// \return the fractional height of this timeline item.
    double fractionalHeight() const { return m_dFractionalHeight; }

    /// Sets the fractional height of this timeline item. This Should be a value between 0 and 1.
    /// The fraction is multiplied by the branch height to get the height of this timeline item.
    /// A value of One indicates that the height of the item will be equal to the height of the branch.
    /// \param newFractionalHeight the fractional height of this timeline item.
    void setFractionalHeight(const double newFractionalHeight);

    /// Draws this timeline item.
    /// \param painter the painter object to use when painting this item.
    /// \param branchRowTop the Y coordinate of the top of the owning branch.
    /// \param branchHeight the height of the owning branch.
    /// \param drawGradientBG should the background be painted with gradient
    /// \param shouldUpdateGeometry should the item geometry be updated? Sometimes the item is drawn twice (once in it's own branch, and once in it's parent branch),
    ///        when the item is painted in the parent branch the geomtery should not be updated
    virtual void draw(QPainter& painter, const int branchRowTop, const int branchHeight, bool drawGradientBG, bool shouldUpdateGeometry);

    /// Gets the tooltip items for this timeline item.
    /// \param [out] tooltip reference that gets populated with the tooltip contents for this timeline item.
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

    /// Set the item shape (rectangle[default], line or dot):
    /// \param shape the requested shape
    void SetItemShape(ItemGraphicShape shape) { m_itemShape = shape; };

    /// Is the item selected?
    /// \return true iff the item is selected
    bool IsHighlighted() const;

    /// Clear the item selection flag
    void ClearSelection() { m_isSelected = false; };

    /// Set the item selection flag
    void SetSelected(bool isSelected) { m_isSelected = isSelected; };

    /// Is the item selected?
    bool IsSelected() const { return m_isSelected; };

protected:

    /// Draw the item background
    /// \param painter the painter object to use when painting this item
    /// \param roundLeftCorners should the left corner be round?
    /// \param roundRightCorners should the left corner be round?
    /// \param drawGradientBG should the background be painted with gradient
    void DrawItemBackground(QPainter& painter, bool roundLeftCorners, bool roundRightCorners, bool drawGradientBG);

    /// Mark the item as highlighted in case it is selected
    /// \param painter the painter object to use when painting this item
    void DrawSelection(QPainter& painter);

    /// Draw the text
    void DrawItemText(acTimeline* pTimeline, QPainter& painter);

    // Checks the parent mask, and update m_rect accordingly
    void CheckParentBranchMask();

    /// Calculate the item rectangle before drawing it (the function sets m_rect and m_bVisible)
    /// \param branchHeight the parent branch height
    /// \param branchRowTop the parent branch top coordinate
    /// \param roundLeftCorners[out] should the left corner be round?
    /// \param roundRightCorners[out] should the right corner be round?
    void CalculateItemRect(const int branchHeight, const int branchRowTop, bool& roundLeftCorners, bool& roundRightCorners);

protected:

    /// Gets a string representation of the specified duration.
    /// \param duration the duration for which a string is needed
    /// \return a string representation of the specified duration.
    static QString getDurationString(quint64 duration);

    acTimelineBranch*       m_pParentBranch;                ///< The parent branch.

    QColor                  m_backgroundColor;              ///< The background color for this timeline item.
    QColor                  m_lightenedBackgroundColor;     ///< The lightened background color for this timeline item.  This color is used when doing gradient painting.
    QColor                  m_foregroundColor;              ///< The foreground color (font color) for this timeline item.
    QColor                  m_outlineColor;                 ///< The outline color for this timeline item. The outline is shown when the mouse hovers over the item.

    QString                 m_strText;                      ///< The text for this timeline item.

    int                     m_nIndex;                       ///< The index of this item within the owning branch.
    quint64                 m_nStartTime;                   ///< The start time for this timeline item.
    quint64                 m_nEndTime;                     ///< The end time for this timeline item.

    /// The fractional height of this timeline item. This Should be a value between 0 and 1.
    /// The fraction is multiplied by the branch height to get the height of this timeline item.
    /// A value of One indicates that the height of the item will be equal to the height of the branch.
    double                  m_dFractionalHeight;

    /// The fractional offset of this timeline item. This Should be a value between 0 and 1.
    /// This represents the vertical offset of the top of a timeline item within its parent branch.
    /// The fraction is multiplied by the branch height and then added to the branch top to get the top of the timeline item.
    /// A value of Zero indicates that the top of the timeline item will be equal to the top of the parent branch.
    double                  m_dFractionalOffset;

private:
    /// Gets a lightened version of the specified color
    /// \param color the color to lighten
    /// \param amount the amount to lighten the color
    /// \return the lightened color
    static QRgb lightenColor(QRgb color, int amount);
    /// Flag indicating whether or not the lightenedBackgroundColor has been explicitly set.
    bool m_bLightenedBGColorSet;

    /// Flag indicating whether or not the lightenedBackgroundColor has been calculated already.
    bool m_bLightenedBGColorCalculated;

    /// Flag indicating whether or not this item is currently visible (drawn).
    bool m_bVisible;

    /// Flag indicating whether or not the fractional offset has been set for this timeline item.
    bool m_bUseFractionalOffset;

    /// Represents the item graph shape (rectangle by default, dot or line):
    ItemGraphicShape m_itemShape;

    QRect m_rect;                         ///< Rectangle for this item (set while drawing).

    /// The path for the item frame
    QPainterPath m_itemFramePath;

    static QMap<QRgb, QRgb> m_lightenedColorMap;            ///< Map used to store the lightened version of given color.

    bool m_isSelected; /// <Is the item selected


};

/// Timeline item descendant which represents an API item.
class AC_API acAPITimelineItem : public acTimelineItem
{
    Q_OBJECT
public:
    /// Construct/initialize a new instance of the qcAPITimelineItem class.
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    /// \param apiIndex the index of the API represented by this timeline item.
    acAPITimelineItem(quint64 startTime, quint64 endTime, int apiIndex);

    /// Destroys an instance of the qcAPITimelineItem class.
    virtual ~acAPITimelineItem() {}

    /// Gets the index of the API represented by this timeline item.
    /// \return the index of the API represented by this timeline item.
    int apiIndex() const { return m_nApiIndex; }

    /// Gets the index of the API represented by this timeline item.
    /// \return the index of the API represented by this timeline item.
    void setApiIndex(int index) { m_nApiIndex = index; }

    /// Gets the tooltip items for this timeline item.
    /// \param [out] tooltip reference that gets populated with the tooltip contents for this timeline item.
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

protected:
    int m_nApiIndex; ///< The index of the API represented by this timeline item.
};

/// Class representing a tooltip for a qcTimelineItem.  The tooltip will display the specified name/value pairs and/or the specified additional text.
class AC_API acTimelineItemToolTip : public QObject
{
public:
    /// Construct/initialize a new instance of the acTimelineItemToolTip class.
    acTimelineItemToolTip();

    /// Destroys an instance of the acTimelineItemToolTip class.
    virtual ~acTimelineItemToolTip();

    /// Gets additional text for this tooltip.
    /// \return additional text for this tooltip.
    QString additionalText() const { return m_strAdditionalText; }

    /// Sets additional text for this tooltip.
    /// \param newText additional text for this tooltip.
    void setAddtionalText(const QString newText) { m_strAdditionalText = newText; }

    /// Gets the number of name/value pairs for this tooltip.
    /// \return the number of name/value pairs for this tooltip.
    int count() const;

    /// Gets a flag indicating whether or not there is valid data for this tooltip.
    /// \return true if there is at least one name/value pair or if the additional text has been set, false otherwise.
    bool isValid() const;

    /// Adds a name/value pair.
    /// \param name the name of for the tooltip item.
    /// \param value the value of for the tooltip item.
    /// \return true if the name/value pair could be added, false otehrwise.
    bool add(QString name, QString value);

    /// Removes the name/value pair at the given index.
    /// \param index the index of the name/value pair to remove.
    /// \return true if the item could be removed, false otherwise (index is out of bounds).
    bool remove(int index);

    /// Gets the name at the given index.
    /// \param index the index of the name to get.
    /// \return the name at the given index, NULL if the index is out of bounds.
    QString getName(int index) const;

    /// Gets the value at the given index.
    /// \param index the index of the value to get.
    /// \return the value at the given index, NULL if the index is out of bounds.
    QString getValue(int index) const;

    /// Clears all name/value pairs and sets the additional text to an empty string.
    void clear();

private:
    /// Nested structure to hold a name/value pair for a timeline tooltip item.
    struct ToolTipItem
    {
        /// Construct/initialize a new instance of the ToolTipItem struct.
        // \param name the name of the tooltip item.
        // \param value the value of the tooltip item.
        ToolTipItem(QString name, QString value) : m_strName(name), m_strValue(value) {};

        QString          m_strName;           ///< The name of the tooltip item.
        QString          m_strValue;          ///< The value of the tooltip item.
    };

    QList<ToolTipItem*> m_tooltipItems;      ///< The list of name/value pairs.
    QString             m_strAdditionalText; ///< Additional text.

};

#endif // _ACTIMELINEITEM_H_
