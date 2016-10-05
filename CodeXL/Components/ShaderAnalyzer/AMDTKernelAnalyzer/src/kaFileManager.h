//------------------------------ kaFileManager.h ------------------------------

#ifndef __KAFILEMANAGER_H
#define __KAFILEMANAGER_H

//C++ STD
#include <map>

// Infra:
#include <AMDTAPIClasses/Include/apSerializable.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtList.h>

#include <AMDTOSWrappers/Include/osFilePath.h>
// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>


class kaSourceFile;
// ----------------------------------------------------------------------------------
// Class Name:              kaFileManager
// General Description:     Holds data with shaders source file
// Author:               Roman Bober
// Creation Date:        27/12/2015
// ----------------------------------------------------------------------------------
class kaFileManagerChangeRecipient
{
public:
    virtual void OnFileRemove(const int id) = 0;
};

class KA_API kaFileManager  : public apXmlSerializable
{
public:
    typedef gtMap<int, kaSourceFile*>::const_iterator SourceFileMapCItr;
public:
    kaFileManager() {}
    virtual ~kaFileManager();


    virtual void Serialize(gtString& xmlLString) override;
    virtual void DeSerialize(TiXmlElement*) override;

    bool Exists(const int id) const;
    bool Add(kaSourceFile* sourceFile);
    bool Remove(const int id);


    //removes first source file, and returns its filepath
    bool RemoveNext(osFilePath&);

    void Rename(kaSourceFile* sourceFile, const gtString& newName);

    /// registers id with recipient
    /// \param [in] id - file id
    /// \param [in] recipient
    void Register(const int id, kaFileManagerChangeRecipient* recipient);

    /// unregisters id from recipient
    /// \param [in] id - file id
    /// \param [in] recipient
    void Unregister(const int id, kaFileManagerChangeRecipient* recipient);

    /// removes entry with passed recipient
    /// \param [in] id - file id
    /// \param [in] recipient
    void RemoveRecipient(kaFileManagerChangeRecipient* recipient);

    int GetFileId(const osFilePath& filePath) const;
    kaSourceFile* GetFile(const osFilePath& filePath) const;

    /// returns file path by file id
    /// \param [in] fileID
    /// \param [out] filePath
    void GetFilePathByID(const int fileID, osFilePath& filePath) const;

    gtList<kaSourceFile*> GetSourceFilesByIds(const gtVector<int>& fileIds) const;

    int SourceFileCount() const;

    const gtMap<int, kaSourceFile*>& GetSourceFilesMap() const;

    /// Find the next available index in the map
    int FindNextFreeIndex() const;

private:
    //Map from unique id to source file pointers
    gtMap<int, kaSourceFile*> m_sourceFiles;
    std::multimap <int, kaFileManagerChangeRecipient*> m_changeRecipientsMap;
};
#endif  // __KAFILEMANAGER_H