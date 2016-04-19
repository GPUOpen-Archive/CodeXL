//------------------------------ kaOverviewView.h ------------------------------

#ifndef __KAOVERVIEWVIEW_H
#define __KAOVERVIEWVIEW_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Framework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>
#include <AMDTApplicationFramework/Include/afIDocUpdateHandler.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>

class acListCtrl;
class acQHTMLWindow;

class KA_API kaOverviewView : public QWidget, public afBaseView, public afIDocUpdateHandler
{
    Q_OBJECT

public:
    kaOverviewView(QWidget* pParent, const osFilePath& filePath);
    virtual ~kaOverviewView();

    /// Edit menu commands
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);

    virtual void onEdit_Copy();
    virtual void onEdit_SelectAll();

    // Handle afIDocUpdateHandler virtual function:
    virtual void UpdateDocument(const osFilePath& docToUpdate);

public slots:
    // Handle the change of cell data:
    void onCellChanged(int row, int column);
    // Handle the change of the text:
    void updateTable();
    // option url clicked
    void onOptionClicked(const QUrl urlLink);
    // update information
    void updateView();

private:
    // insert the kernels from the project data manager:
    void insertKernelsToTable();
    // internal function to clear table. clearContent does not work well.
    void clearTable();

private:
    // File path of the file that used to open the view:
    osFilePath m_filePath;

    // do not do validity check while changing values initiated by the view:
    bool m_doNotCheckVality;

    // Main Layout:
    QVBoxLayout* m_pAnalyzeInputLayout;

    /// Information caption
    acQHTMLWindow* m_pTopInformationCaption;

    /// Table information caption
    QGroupBox* m_pTableInformationGroupBox;

    // Data table showing the data:
    acListCtrl* m_pAnalyzeInputTable;

    // update table from source code:
    QPushButton*       m_pUpdateTableButton;

};

#endif // __KAOVERVIEWVIEW_H