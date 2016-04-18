//------------------------------ kaTreeDataExtension.h ------------------------------

#ifndef __KATREEDATAEXTENSION_H
#define __KATREEDATAEXTENSION_H

// infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Framework:
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>

#include <AMDTKernelAnalyzer/src/kaProgram.h>

class KA_API kaTreeDataExtension : public afTreeDataExtension
{
    Q_OBJECT
public:
    kaTreeDataExtension();
    virtual ~kaTreeDataExtension();

    // afTreeDataExtension override compares 2 items:
    virtual bool isSameObject(afTreeDataExtension* pOtherItemData) const;

    // afTreeDataExtension copy one item data to other:
    virtual void copyID(afTreeDataExtension*& pOtherItemData) const;

    void setFilePath(const osFilePath& ifilePath) { m_filePath = ifilePath; }
    osFilePath& filePath() { return m_filePath; }

    void setIdentifyFilePath(const osFilePath& ifilePath) { m_identifyFilePath = ifilePath; }
    osFilePath& identifyFilePath() { return m_identifyFilePath; }

    void setDetailedFilePath(const osFilePath& ifilePath) { m_detailedFilePath = ifilePath; }
    osFilePath& detailedFilePath() { return m_detailedFilePath; }

    int lineNumber() { return m_lineNumber; }
    void setLineNumber(int iLineNumber) { m_lineNumber = iLineNumber; }

    ///Program setter/getter
    void SetProgram(kaProgram* pProgram) { m_pProgram = pProgram; }

    kaProgram* GetProgram() { return m_pProgram; }

private:

    QString m_nodeStr;
    osFilePath m_filePath;
    osFilePath m_identifyFilePath;
    osFilePath m_detailedFilePath;
    kaProgram* m_pProgram;

    int m_lineNumber;

};

#endif // __KATREEDATAEXTENSION_H