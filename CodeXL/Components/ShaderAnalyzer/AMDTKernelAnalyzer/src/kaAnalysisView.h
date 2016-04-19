//------------------------------ kaAnalysisView.h ------------------------------

#ifndef __KAANALYSISVIEW_H
#define __KAANALYSISVIEW_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaTableView.h>

class kaAnalysisView : public kaTableView
{
    Q_OBJECT

public:
    kaAnalysisView(QWidget* pParent, const osFilePath& kernelFilePath);
    virtual ~kaAnalysisView();

private:
};

#endif // __KAANALYSISVIEW_H
