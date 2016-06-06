//==================================================================================
// Copyright (c) 2011 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTimelineBranch.h
///
//==================================================================================

#ifndef _ACTIMELINEBRANCH_H_
#define _ACTIMELINEBRANCH_H_

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QObject>
#include <QPainter>
#include <QMap>

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

class acTimeline;
class acTimelineItem;
class acTimelineMaskBuffer;

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

/// Timeline branch class -- takes ownership (memory-wise) of contained timeline items
class AC_API acTimelineBranch : public QObject
{
    Q_OBJECT

public:
    /// Construct/Initialize a new instance of the qcTimelinebranch class.
    acTimelineBranch();

    /// Destroys instance of the the acTimelineBranch class.
    virtual ~acTimelineBranch();

    /// Sets the acTimeline instance that contains this branch.
    /// \param newParentTimeline the timeline instance.
    void setParentTimeline(acTimeline* newParentTimeline);

    /// Gets the acTimeline instance that contains this branch.
    /// \return the acTimeline instance that contains this branch.
    acTimeline* parentTimeline() const { return m_pParentTimeline; }

    /// Gets the acTimelineBranch instance that is the parent of this branch.
    /// \return the acTimelineBranch instance that is the parent of this branch (NULL if this branch has no parent branch).
    acTimelineBranch* parentBranch() const { return m_pParentBranch; }

    /// Gets the text for the timeline branch.
    /// \return the text for the timeline branch.
    QString text() const { return m_strText; }

    /// Sets the text for this timeline branch.
    /// \param newText new value for the branch text.
    void setText(const QString newText);

    /// Gets the required width of the title section of this branch.
    /// \return the required width of the title section of this branch.
    int titleWidth() const { return m_nTitleWidth; }

    /// Sets the start time for the branch.  Note that the branch's start time can also be adjusted by addTimelineItem, addSubBranch, and subBranchRangeChanged.
    /// \param newStartTime the start time for the branch.
    void setStartTime(const quint64 newStartTime);

    /// Gets the start time of the branch.
    /// \return the start time of the branch.
    quint64 startTime() const { return m_nStartTime; }

    /// Sets the end time for the branch.  Note that the branch's end time can also be adjusted by addTimelineItem, addSubBranch, and subBranchRangeChanged.
    /// \param newEndTime the end time for the branch.
    void setEndTime(const quint64 newEndTime);

    /// Gets the end time of the branch.
    /// \return the end time of the branch.
    quint64 endTime() const { return m_nEndTime; }

    /// Sets the folded state of the branch.
    /// \param newFolded the folded state of the branch.
    void setFolded(const bool newFolded);

    /// Gets the folded state of the branch.
    /// \return the folded state of the branch.
    bool isFolded() const { return m_bFolded; }

    /// Sets the selected state of the branch.
    /// \param newSelected the selected state of the branch.
    void setSelected(const bool newSelected);

    /// Gets the selected state of the branch.
    /// \return the selected state of the branch.
    bool isSelected() const { return m_bSelected; }

    /// Sets a flag indicating whether or not this branch uses a mask test when painting items.
    /// \param newEnableMask the flag indicating whether or not this branch uses a mask test when painting items.
    void setMaskEnabled(const bool newEnableMask) { m_bMaskEnabled = newEnableMask; }

    /// Gets a flag indicating whether or not this branch uses a mask test when painting items.
    /// \return the flag indicating whether or not this branch uses a mask test when painting items.
    bool isMaskEnabled() const { return m_bMaskEnabled; }

    /// Gets the mask buffer used by this branch when painting items.
    /// \return the mask buffer used by this branch when painting items.
    acTimelineMaskBuffer* getMaskBuffer() const { return m_pMask; }

    /// Gets a flag indicating whether or not all this branch's parents are unfolded.
    /// \return true if all parents are unfolded, false otherwise.
    bool allParentsUnfolded() const;

    /// Gets the depth of this branch.  Depth indicates the number of levels between this branch and the root (think treeview).
    /// \return the depth of this branch.
    unsigned int depth() const { return m_nDepth; }

    /// Gets the total row count for this branch (1 for this branch, and then sum of all sub branches).
    /// \return the total row count for this branch.
    unsigned int rowCount() const;

    /// Gets the row index of this branch.  The row index is the position of the branch within the timeline if you started at the top with zero and counted as you moved down through the timeline.
    /// \return the row index of this branch.
    unsigned int rowIndex() const {return m_nRowIndex; }

    /// Resets the row index of this branch and all sub branches.
    /// \param rowIndex reference to the row index to assign to this branch.  It is incremented in the body so that subsequent calls to resetRowIndex for other branch's will get the correct index.
    void resetRowIndex(unsigned int& rowIndex);

    /// Gets the top (y coordinate) of this branch
    /// \return the top (y coordinate) of this branch
    int top() const { return m_nTop; }

    /// Gets the height of this branch.
    /// \return the height of this branch.
    int height() const { return m_nHeight; }

    /// Sets the height of this branch.
    /// \param newHeight the height of this branch.
    void setHeight(const int newHeight);

    /// Sets extra data to be attached to this branch.
    /// \param newTag an untyped pointer containing extra data attached to this branch.
    void setTag(void* newTag) { m_pTag = newTag; }

    /// Gets extra data to be attached to this branch.
    /// \return an untyped pointer containing extra data attached to this branch.
    void* tag() const { return m_pTag; }

    /// Gets the cumulative height of this branch and all visible sub branches.
    /// \return the cumulative height of this branch and all visible sub branches.
    int cumulativeHeight() const;

    /// Adds the specified timeline item to this branch.
    /// Note: timeline painting performance is best when timeline items are added in order
    /// according to their start time.  Also, performance of the add operation is better
    /// when timeline items are added to a branch before the branch is added to the timeline.
    /// \param item the timeline item to be added.
    /// \return true if the item was added, false otherwise.
    bool addTimelineItem(acTimelineItem* item);

    /// Adds the specified branch as a sub branch of this branch.
    /// \param subBranch the branch to be added.
    /// \return true if the sub branch was added, false otherwise.
    bool addSubBranch(acTimelineBranch* subBranch);

    /// Gets the number of sub branches contained by this branch.
    /// \return the number of sub branches contained by this branch.
    int subBranchCount() const;

    /// Gets the sub branch at the specified index.
    /// \param index the index of the sub branch to return.
    /// \return the sub branch at the specified index, or NULL if index is out of range.
    acTimelineBranch* getSubBranch(const int index) const;

    /// Gets the sub branch with the specified row index.
    /// \param rowIndex the row index whose branch is needed.
    /// \return the sub branch with the specified row index, or NULL if no sub branch has the specified index.
    acTimelineBranch* getSubBranchFromRowIndex(const unsigned int rowIndex) const;

    /// Gets the first sub branch found with the specified text.
    /// \param branchText the branch text to find
    /// \param partialMatch flag indicating whether to perform a partial match or complete match
    /// \return the first sub branch found with the specified text.  Returns NULL if branch can not be found
    acTimelineBranch* getSubBranchFromText(const QString& branchText, bool partialMatch) const;

    /// Gets the sub branch at the specified Y coordinate.
    /// \param y the Y coordinate of the sub branch which is needed.
    /// \return the sub branch at the specified Y coordinate, or NULL if no sub branch is located at the specified Y coordinate.
    acTimelineBranch* getSubBranchFromY(const int y) const;

    /// Gets the number of timeline items contained by this branch.
    /// \return the number of timeline items contained by this branch.
    int itemCount() const;

    /// Gets the timeline item at the specified index.
    /// \param index the index of the timeline item to return.
    /// \return the timeline item at the specified index, or NULL if index is out of range.
    acTimelineItem* getTimelineItem(const int index) const;

    /// Gets the timeline item at the specified location.
    /// \param x the X coordinate of the item which is needed.
    /// \param y the Y coordinate of the item which is needed.
    /// \return the item at the specified location, or NULL if no item is located at the specified location.
    acTimelineItem* getTimelineItem(const int x, const int y) const;

    /// Gets the index of the specified item within the m_timelineItems list
    /// \param item the item whose index is needed
    /// \return the index of the specified item within the m_timelineItems list
    int indexOfItem(acTimelineItem* item) const;

    /// Gets the rectangle of the tree mark of this branch. The tree mark is the plus/minus glyph used for expanding/collapsing branches.
    /// \return the rectangle of the tree mark of this branch.
    QRect treeMarkRect() { return m_treeMarkRect; }

    /// Draws this branch.
    /// \param painter the painter object to use when painting this branch.
    /// \param yOffset the Y coordinate at which to draw this branch.
    void draw(QPainter& painter, int& yOffset);

    /// Draws all subbranch items on this branch when this branch is folded (collapsed).
    /// \param painter the painter object to use when painting the items.
    /// \param yOffset the Y coordinate of the branch on which to draw the items.
    /// \param branchHeight the height of the branch on which to draw the items.
    /// \param shouldUpdateChildGeometry when the branch items are drawn, should it's geometry be updated?
    void DrawSubBranchItems(QPainter& painter, const int yOffset, const int branchHeight, bool shouldUpdateChildGeometry);

    /// Clears the draw cache which contains the list of items to draw.
    /// \param clearSubBranchCache flag indicating if the draw cache for all sub branches should be cleared as well.
    void clearDrawCache(bool clearSubBranchCache);

    /// Sets the visibility flag of the specified item
    void setVisibility(bool isVisible);

    /// Querries the visibility flag of the specified item
    bool IsVisible()const;

    /// Should the children be painted on parent?
    void SetDrawChildren(bool shouldDraw) { m_shouldDrawChildren = shouldDraw; }

    /// Set the branch background color
    void SetBGColor(const QColor& bgColor) { m_bgColor = bgColor; }

signals:
    /// Signal emitted when the branch start time or end time is changed.
    void branchRangeChanged();

    /// Signal emitted when a sub branch is added.
    /// \param subBranch the timeline branch added.
    void branchAdded(acTimelineBranch* subBranch);

    /// Signal emitted when a timeline item is added.
    /// \param item the timeline item added.
    void branchItemAdded(acTimelineItem* item);

    /// Signal emitted when a timeline item's text is changed.
    void branchTextChanged();

    /// Signal emitted when a timeline item is folded/unfolded.
    void branchFoldedChanged();

    /// Signal emitted when a timeline item is selected/unselected.
    void branchSelectedChanged();

private slots:
    /// Called when a sub branch's range is changed.
    void subBranchRangeChanged();

    /// Called when a sub branch is added.
    /// \param subBranch the timeline branch added.
    void subBranchAdded(acTimelineBranch* subBranch);

    /// Called when a sub branch's text is changed.
    void subBranchTextChanged();

    /// Called when the zoom factor of offset of the parent timeline is changed.
    void zoomFactorOrOffsetChanged();

private:
    /// Sets the depth of this branch and all sub branches
    /// \param newDepth the depth of this branch
    void setDepth(const unsigned int newDepth);

    /// Gets the timeline item at the specified location, recursively checking sub branches. This overload is used when a branch is folded.
    /// \param branch the branch whose items should be checked.
    /// \param x the X coordinate of the item which is needed.
    /// \param y the Y coordinate of the item which is needed.
    /// \return the item at the specified location, or NULL if no item is located at the specified location.
    acTimelineItem* getTimelineItem(const acTimelineBranch* branch, const int x, const int y) const;

    /// Recalculates the title width of this branch, taking into account the title width of sub branches.
    void recalcTitleWidth();

    QString                 m_strText;          ///< The title text of this branch.

    acTimeline*             m_pParentTimeline;  ///< The parent timeline.
    acTimelineBranch*       m_pParentBranch;    ///< The parent branch.

    quint64                 m_nStartTime;       ///< The start time of this branch.
    quint64                 m_nEndTime;         ///< The end time of this branch.

    bool                    m_bFolded;          ///< Flag indicating whether or not this branch is folded.
    bool                    m_bSelected;        ///< Flag indicating whether or not this branch is selected.
    bool                    m_bSorted;          ///< Flag indicating whether or not this branch is sorted.

    int                     m_nTitleWidth;      ///< The width of the title section of this branch.

    unsigned int            m_nDepth;           ///< The depth of this branch.
    unsigned int            m_nRowIndex;        ///< The row index of this branch.

    int                     m_nTop;             ///< The top (y coordinate) of this branch.
    bool                    m_bHeightSet;       ///< Flag indicating whether or not the height has been set.  False indicates that the parent timeline's defaultBranchHeight is being used.
    int                     m_nHeight;          ///< The height of this branch.

    QRect                   m_treeMarkRect;     ///< The rectangle where the tree mark (plus/minus glyph) is drawn. Rectangle is invalid if this branch has no sub branches.

    void*                   m_pTag;             ///< Extra data to be attached to this branch.

    bool                    m_bMaskEnabled;     ///< Flag indicating whether or not this branch uses a mask test when painting items.
    bool                    m_bVisible;

    acTimelineMaskBuffer*    m_pMask;           ///< The mask buffer used by this branch when painting items.
    QList<acTimelineBranch*> m_subBranches;     ///< The list of sub branches for this branch
    QList<acTimelineItem*>   m_timelineItems;   ///< The list of timeline items contained in this branch.
    QList<acTimelineItem*>   m_drawCache;       ///< A list of items actually painted.  Used as an optimization when repainting this branch.

    bool                     m_shouldDrawChildren; /// Should the branch children be drawn on parent?

    QColor                   m_bgColor;         /// The branch background color
};

/// Mask buffer class used to prevent painting items that would not actually appear on the
/// screen because another timeline item has already been drawn there (i.e. multiple items
/// with the same height that occupy the same area on the timeline).  Maintains a separate
/// mask for each height (since branches can contain items of differing heights)
class acTimelineMaskBuffer
{
public:
    /// Construct/Initialize a new instance of the MaskBuffer class.
    acTimelineMaskBuffer() : m_nBufferWidth(0) {}

    /// Destroys instance of the the MaskBuffer class.
    virtual ~acTimelineMaskBuffer();

    /// Sets the mask for a timeline item.
    /// \param index the index within the branch of the first pixel of the timeline item.
    /// \param rect the rectangle of the timeline item.
    /// \return true if the mask was set, false otherwise.
    bool setMask(int index, QRect rect);

    /// Checks the mask for a timeline item.
    /// \param index the index within the branch of the first pixel of the timeline item.
    /// \param rect the rectangle of the timeline item.
    /// \return true if the mask is set, false otherwise.
    bool checkMask(int index, QRect rect);

    /// Resets the mask buffer to the specified width.
    /// \param rowWidth the width of the row.
    void resetMask(int rowWidth);

private:
    /// Disable copy constructor.
    acTimelineMaskBuffer(const acTimelineMaskBuffer& obj);

    /// Disable assignment operator.
    acTimelineMaskBuffer& operator= (const acTimelineMaskBuffer& obj);

    int m_nBufferWidth;           ///< the width of the buffer.
    QMap<int, QMap<int, int*> > m_maskBuffer; ///< the mask buffer -- maps from a height to a map that maps from an offset to a list of pixels.

};

#endif // _ACTIMELINEBRANCH_H_
