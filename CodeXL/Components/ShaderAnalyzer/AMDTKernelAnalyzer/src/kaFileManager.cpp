//------------------------------ kaFileManager.cpp ------------------------------
//External
#include <tinyxml.h>
// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
//FrameWork
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaFileManager.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>


using namespace std;

kaFileManager::~kaFileManager()
{
    while (false == m_sourceFiles.empty())
    {
        auto itr = m_sourceFiles.begin();
        delete itr->second;
        m_sourceFiles.erase(itr);
    }

    m_changeRecipientsMap.clear();
}

void kaFileManager::Serialize(gtString& xmlString)
{
    xmlString.appendFormattedString(L"<%ls>", KA_STR_projectSettingSectionNode);

    for (auto itr : m_sourceFiles)
    {
        itr.second->Serialize(xmlString);
    }

    xmlString.appendFormattedString(L"</%ls>", KA_STR_projectSettingSectionNode);
}

void kaFileManager::DeSerialize(TiXmlElement* root)
{
    GT_ASSERT(root != nullptr);

    for (TiXmlElement* pNode = root->FirstChildElement(); pNode != nullptr; pNode = pNode->NextSiblingElement())
    {
        gtString kaFileTitle;
        kaFileTitle.fromASCIIString(pNode->Value());

        if (KA_STR_projectSettingFilesInfoNode == kaFileTitle)
        {
            kaSourceFile* sourcefile = new kaSourceFile();
            sourcefile->DeSerialize(pNode);

            Add(sourcefile);
            // Add listening to the file update:
            afDocUpdateManager::instance().RegisterDocumentActivate(sourcefile->filePath(),
                                                                    &KA_PROJECT_DATA_MGR_INSTANCE, false);
        }
    }
}

bool kaFileManager::Exists(const int id) const
{
    bool bRes = m_sourceFiles.find(id) != m_sourceFiles.end();
    return bRes;
}

bool kaFileManager::Add(kaSourceFile* pSourceFile)
{
    bool retVal = false;
    // Sanity check:
    GT_IF_WITH_ASSERT(pSourceFile != nullptr)
    {
        if (pSourceFile->id() < 0)
        {
            int nextFreeIndex = FindNextFreeIndex();
            pSourceFile->setId(nextFreeIndex);
        }

        retVal = m_sourceFiles.insert(pair<int, kaSourceFile*>(pSourceFile->id(), pSourceFile)).second;
    }

    return retVal;
}

bool kaFileManager::Remove(const int id)
{
    // check if a source file with such id exists in a map
    auto itr = m_sourceFiles.find(id);
    bool retVal = itr != m_sourceFiles.end();

    if (retVal)
    {
        // check if multimap is not empty
        if (!m_changeRecipientsMap.empty())
        {
            //remove that id from all recipients
            auto range = m_changeRecipientsMap.equal_range(id);

            for (auto rangeIterator = range.first; rangeIterator != range.second; ++rangeIterator)
            {
                kaFileManagerChangeRecipient* pRecipient = rangeIterator->second;

                if (pRecipient != nullptr)
                {
                    pRecipient->OnFileRemove(id);
                }
            }

            for (auto rangeIterator = range.first; rangeIterator != range.second;)
            {
                //remove this entry from the multimap updating iterator
                rangeIterator = m_changeRecipientsMap.erase(rangeIterator);
            }
        }

        //remove this entry from files map
        delete itr->second;
        m_sourceFiles.erase(itr);
    }

    return retVal;
}

bool kaFileManager::RemoveNext(osFilePath& removedPath)
{
    bool result = false;

    if (SourceFileCount() > 0)
    {
        removedPath.setFromOtherPath(m_sourceFiles.begin()->second->filePath());
        result = Remove(m_sourceFiles.begin()->first);
    }

    return result;
}

void kaFileManager::Rename(kaSourceFile* sourceFile, const gtString& newName)
{
    sourceFile->filePath().setFileName(newName);
}

void kaFileManager::Register(const int id, kaFileManagerChangeRecipient* recipient)
{
    m_changeRecipientsMap.insert(pair<int, kaFileManagerChangeRecipient*>(id, recipient));
}


void kaFileManager::Unregister(int fileId, kaFileManagerChangeRecipient* pRecipient)
{
    // check if multimap is not empty
    if (!m_changeRecipientsMap.empty())
    {
        //remove this entry
        auto range = m_changeRecipientsMap.equal_range(fileId);

        for (auto rangeIterator = range.first; rangeIterator != range.second; ++rangeIterator)
        {
            if (rangeIterator->second == pRecipient)
            {
                m_changeRecipientsMap.erase(rangeIterator);
                break;
            }
        }
    }
}

void kaFileManager::RemoveRecipient(kaFileManagerChangeRecipient* pRecipient)
{

    if (!m_changeRecipientsMap.empty())
    {
        //remove all entries with that recipient
        for (auto iter = m_changeRecipientsMap.begin(); iter != m_changeRecipientsMap.end();)
        {
            //remove this entry from the multimap updating iterator
            if (iter->second == pRecipient)
            {
                iter = m_changeRecipientsMap.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
}

int kaFileManager::GetFileId(const osFilePath& filePath) const
{
    int result = -1;

    for (auto itr : m_sourceFiles)
    {
        if (itr.second->filePath() == filePath)
        {
            result = itr.first;
            break;
        }
    }

    return result;
}

kaSourceFile* kaFileManager::GetFile(const osFilePath& filePath) const
{
    kaSourceFile* result = nullptr;

    int id = GetFileId(filePath);
    auto itr = m_sourceFiles.find(id);

    if (itr != m_sourceFiles.end())
    {
        result = itr->second;
    }

    return result;
}

void kaFileManager::GetFilePathByID(const int fileID, osFilePath& filePath) const
{
    auto itr = m_sourceFiles.find(fileID);

    if (itr != m_sourceFiles.end())
    {
        filePath = itr->second->filePath();
    }
}

gtList<kaSourceFile*> kaFileManager::GetSourceFilesByIds(const gtVector<int>& fileIds) const
{
    gtList<kaSourceFile*> result;

    for (auto id : fileIds)
    {
        auto itr = m_sourceFiles.find(id);

        if (itr != m_sourceFiles.end())
        {
            result.push_back(itr->second);
        }
    }

    return result;
}

int kaFileManager::SourceFileCount() const
{
    int result = m_sourceFiles.size();
    return result;
}

const gtMap<int, kaSourceFile*>& kaFileManager::GetSourceFilesMap() const
{
    return m_sourceFiles;
}

int kaFileManager::FindNextFreeIndex() const
{
    int retVal = 0;

    for (auto itr : m_sourceFiles)
    {
        if (retVal <= itr.first)
        {
            retVal = itr.first + 1;
        }
    }

    return retVal;
}
