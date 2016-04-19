//------------------------------ kaSourceCodeView.h ------------------------------

#ifndef __KASOURCECODEVIEW__H
#define __KASOURCECODEVIEW__H

// Qt
#include <QtWidgets>

// Infra
#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>

// Local
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>

class kaSourceCodeView: public afSourceCodeView
{
    Q_OBJECT
public:
    /// Constructor
    kaSourceCodeView(QWidget* pParent, bool shouldShowLineNumbers, unsigned int contextMenuMask = (unsigned int)AC_ContextMenu_Default);

    /// Destructor
    ~kaSourceCodeView();

    /// Add an action to the menu that will be handled by an outside handler:
    /// \param pAction to be added to the menu
    /// \param inBeggining if true the action will be added in the menu beginning
    void addMenuAction(QAction* pAction, bool inBeginning = true);

    /// addSeparator at the beginning of the menu:
    /// \param inBeggining if true the separator will be added in the menu beginning
    void addSeparator(bool inBeginning = true);

    /// Update the view by reloading the file
    void updateView();

    /// Set text without updating caption
    void SetInternalText(const QString& text);

    /// Set text without updating caption
    void SetISAText(const osFilePath& isaFilePath);

    /// Sets the platform indicator
    void SetViewPlatform(kaPlatform platformIndicator) { m_platformIndicator = platformIndicator; }

    /// is the action is a part of this view context menu
    /// \param pAction - is the action to search
    /// \returns true if the action was found in this view actions list
    bool IsActionOfThisView(QAction* pAction) const;


private:
    /// Indicates if this source code view is used for OpenCL. (needed to know what type of info to display about in the captions and menu)
    kaPlatform m_platformIndicator;

    /// list of added context menu actions
    QVector<QAction*> m_addedMenuActions;

};

#endif  // __KASOURCECODEVIEW__H

