//------------------------------ gpFrameView.h ------------------------------

#ifndef __GPFRAMEVIEW_H
#define __GPFRAMEVIEW_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedSessionWindow.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpBaseSessionView.h>
#include <AMDTGpuProfiling/gpUIManager.h>

class gpTraceDataModel;
class gpOverview;
class gpTraceView;

#ifdef GP_OBJECT_VIEW_ENABLE
    class gpObjectView;
#endif

/// This class is opened whenever a view related to a frame is opened.
/// The view contain a tab widget with all the inner data for frame:
/// Overview, timeline, profile tabs and image
class AMDT_GPU_PROF_API gpFrameView : public gpBaseSessionView
{
    Q_OBJECT

public:

    /// Constructor
    gpFrameView(QWidget* pParent);

    virtual ~gpFrameView();

    /// Display the frame file.
    /// \param sessionFilePath the file path for the requested session
    /// \param sessionInnerPage the item type describing the inner view to open, or AF_TREE_ITEM_ITEM_NONE when the root is supposed to open
    ///        we expect to get one of the following:
    ///        AF_TREE_ITEM_GP_FRAME (overview)
    ///        AF_TREE_ITEM_GP_FRAME_OVERVIEW
    ///        AF_TREE_ITEM_GP_FRAME_TIMELINE
    ///        AF_TREE_ITEM_GP_FRAME_PERFORMANCE_PROFILE
    ///        AF_TREE_ITEM_GP_FRAME_IMAGE
    ///        AF_TREE_ITEM_GP_FRAME_OBJECTINSPCTOR
    /// \param [out] errorMessage when the display fails, errorMessage should contain a message for the user
    virtual bool DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage);

    /// Set the data model
    virtual void SetProfileDataModel(gpTraceDataModel* pTraceDataModel);

    /// Display the overview
    void DisplayOverview(const osFilePath& sessionFilePath);

    /// Display the frame timeline view
    void DisplayTimeline();

    /// Display the frame profile view
    /// \param profileFilePath the trace file path
    void DisplayProfile(const osFilePath& profileFilePath);

#ifdef GP_OBJECT_VIEW_ENABLE
    /// Display the frame Object Inspector view
    void DisplayObjectInspector();
#endif   // GP_OBJECT_VIEW_ENABLE

private:
    /// Extract the frame index from the session file path
    void ExtractFrameIndex();
private:

    /// A tab widget with the inner views
    QTabWidget* m_pTabWidget;

    /// Contain the overview (always displayed, cannot be closed)
    gpOverview* m_pOverview;

    /// the trace/timeline view
    gpTraceView* m_pTraceView;

    /// the trace data model
    gpTraceDataModel* m_pTraceModel;

#ifdef GP_OBJECT_VIEW_ENABLE
    /// the object inspector view
    gpObjectView* m_pObjectView;

    /// the object data model
    gpObjectDataModel* m_pObjectModel;
#endif // GP_OBJECT_VIEW_ENABLE

    /// The frame index
    FrameIndex m_frameIndex;

};
#endif // __GPFRAMEVIEW_H
