//------------------------------ kaTreeDataExtension.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>
// ---------------------------------------------------------------------------
// Name:        kaTreeDataExtension::kaTreeDataExtension
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
kaTreeDataExtension::kaTreeDataExtension() : m_pProgram(nullptr), m_lineNumber(0)
{

}

// ---------------------------------------------------------------------------
// Name:        kaTreeDataExtension::~kaTreeDataExtension
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
kaTreeDataExtension::~kaTreeDataExtension()
{

}

// ---------------------------------------------------------------------------
// Name:        kaTreeDataExtension::isSameObject
// Description: afTreeDataExtension override compares 2 items
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
bool kaTreeDataExtension::isSameObject(afTreeDataExtension* pOtherItemData) const
{
    bool retVal = false;

    if (pOtherItemData != NULL)
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pOtherItemData);

        if (pKAData != NULL)
        {
            retVal = true;
            retVal = retVal && (m_nodeStr == pKAData->m_nodeStr);
            retVal = retVal && (m_filePath == pKAData->m_filePath);
            retVal = retVal && (m_identifyFilePath == pKAData->m_identifyFilePath);
            retVal = retVal && (m_detailedFilePath == pKAData->m_detailedFilePath);
            retVal = retVal && (m_lineNumber == pKAData->m_lineNumber);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaTreeDataExtension::copyID
// Description: afTreeDataExtension copy one item data to other:
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
void kaTreeDataExtension::copyID(afTreeDataExtension*& pOtherItemData) const
{
    if (pOtherItemData == NULL)
    {
        pOtherItemData = new kaTreeDataExtension;

    }

    GT_IF_WITH_ASSERT(pOtherItemData != NULL)
    {
        kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pOtherItemData);
        GT_IF_WITH_ASSERT(pKAData != NULL)
        {
            pKAData->m_nodeStr = m_nodeStr;
            pKAData->m_filePath = m_filePath;
            pKAData->m_identifyFilePath = m_identifyFilePath;
            pKAData->m_detailedFilePath = m_detailedFilePath;
            pKAData->m_lineNumber = m_lineNumber;
        }
    }
}
