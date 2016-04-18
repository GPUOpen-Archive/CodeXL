//------------------------------ kaGlobalVariableManager.h ------------------------------

#ifndef __KAGLOBALVARIABLEMANAGER_H
#define __KAGLOBALVARIABLEMANAGER_H

// Qt:
#include <QtWidgets>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>

struct kaKernelExecutionDataStruct
{
public:
    QString m_kernelName;
    int m_globalWorkSize[3];
    int m_localWorkSize[3];
    int m_loopIterations;
};

class KA_API kaGlobalVariableManager
{
public:
    static kaGlobalVariableManager& instance();
    virtual ~kaGlobalVariableManager();

    const kaKernelExecutionDataStruct& defaultExecutionData() { return m_defaultExecutionData; }
    void setDefaultExecutionData(kaKernelExecutionDataStruct& executionData) { m_defaultExecutionData = executionData; }

    void setDefaultTreeList(QStringList& iDefaultTreeList);
    void setCurrentTreeList(QStringList& iCurrentTreeList) { m_currentTreeList = iCurrentTreeList; }
    QStringList& defaultTreeList() { return m_defaultTreeList; }
    QStringList& currentTreeList() { return m_currentTreeList; }

private:

    // Only my instance() function can create me:
    kaGlobalVariableManager();

    // Only kaSingletonsDelete should delete me:
    friend class kaSingletonsDelete;

    // The single instance of this class:
    static kaGlobalVariableManager* m_psMySingleInstance;

    kaKernelExecutionDataStruct m_defaultExecutionData;

    // Default tree list
    QStringList m_defaultTreeList;

    // current TreeList;
    QStringList m_currentTreeList;


};

#endif // __KADATAANALYZERFUNCTIONS_H

