//------------------------------ osDirectorySerializer.cpp ------------------------------

#define MINIZ_HEADER_FILE_ONLY
#include <miniz.c>
#include <AMDTOSWrappers/Include/osDirectorySerializer.h>


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>



// ---------------------------------------------------------------------------
// Name:        osDirectorySerializer::osDirectorySerializer
// Description: Ctor
// Author:
// Date:
// ---------------------------------------------------------------------------
osDirectorySerializer::osDirectorySerializer()
{
}


// ---------------------------------------------------------------------------
// Name:        osDirectorySerializer::~osDirectorySerializer
// Description: Dtor
// Author:
// Date:
// ---------------------------------------------------------------------------
osDirectorySerializer::~osDirectorySerializer()
{

}

bool osDirectorySerializer::CompressDir(const osDirectory& sessionDir, const gtString& outFileFullPath)
{
    m_outFileFullPath = outFileFullPath;
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    bool rc = true;

    if ((rand() % 100) >= 10)
    {
        zip.m_file_offset_alignment = 1 << (rand() & 15);
    }

    if (mz_zip_writer_init_file(&zip, m_outFileFullPath.asASCIICharArray(), 65537) == MZ_FALSE)
    {
        rc = false;
    }

    if (rc == true)
    {
        rc = AddDirToZip(sessionDir, L"", &zip);
        mz_bool finalized = mz_zip_writer_finalize_archive(&zip);
        mz_zip_writer_end(&zip);

        if (finalized == MZ_FALSE)
        {
            remove(m_outFileFullPath.asASCIICharArray());
        }
    }

    return rc;
}



bool osDirectorySerializer::AddDirToZip(const osDirectory& sessionDir, const gtString& relativePath, void* pZip)
{
    bool success = AddFilesToZip(sessionDir, relativePath, pZip);

    gtList<osFilePath> subDirectoriesPaths;
    success = sessionDir.getSubDirectoriesPaths(osDirectory::SORT_BY_DATE_ASCENDING, subDirectoriesPaths); // subdirectories order set to match session items order as done in (gpTreeHandler::BuildFrameAnalysisSessionTree)

    if (success)
    {
        gtList<osFilePath>::const_iterator iter = subDirectoriesPaths.begin();
        gtList<osFilePath>::const_iterator iterEnd = subDirectoriesPaths.end();

        for (; iter != iterEnd; iter++)
        {
            osDirectory osDir(*iter);
            gtString dirName = osDir.directoryPath().fileDirectoryAsString();
            int startPos = dirName.findLastOf(osFilePath::osPathSeparator) + 1;
            dirName.getSubString(startPos, (dirName.length() - startPos), dirName);
            gtString relPathAddition = relativePath;
            relPathAddition.append(dirName);
            AddDirToZip(*iter, relPathAddition, pZip);
        }
    }

    return success;
}

bool osDirectorySerializer::AddFilesToZip(const osDirectory& sessionDir, const gtString& relativePath, void* pZip)
{
    gtList<osFilePath> filePathsList;

    bool success = sessionDir.getContainedFilePaths(L"*.*", osDirectory::SORT_BY_NAME_ASCENDING, filePathsList);

    if (success)
    {
        gtList<osFilePath>::const_iterator iter = filePathsList.begin();
        gtList<osFilePath>::const_iterator iterEnd = filePathsList.end();

        for (; iter != iterEnd; iter++)
        {
            osFilePath osFile(*iter);
            gtString displayName;
            osFile.getFileNameAndExtension(displayName);


            gtString fullName = ((*iter).fileDirectoryAsString());
            fullName.append(osFilePath::osPathSeparator);
            fullName.append(displayName);

            if (relativePath.isEmpty() == false)
            {
                displayName.prepend(L"/");
                displayName.prepend(relativePath);
            }

            success &= (mz_zip_writer_add_file((mz_zip_archive*)pZip, displayName.asASCIICharArray(), fullName.asASCIICharArray(), NULL, 0, 9) == MZ_TRUE);
        }
    }

    return success;

}

bool osDirectorySerializer::DecompressDir(const gtString& compressedFile, const gtString& dstFolderPath, gtString& archiveRootDir, bool createRootDir)
{
    bool rc = false;
    m_outFileFullPath = dstFolderPath;
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    rc = (mz_zip_reader_init_file(&zip, compressedFile.asASCIICharArray(), 0) == MZ_TRUE);

    if (rc == true)
    {
        // users may chose to create a root dir INSIDE dstFolderPath
        if (createRootDir)
        {
            rc = CreateOutputDir(archiveRootDir, compressedFile);
        }
        else
        {
            archiveRootDir = dstFolderPath;
            osDirectory newRootDir(archiveRootDir);

            if (newRootDir.exists() == false)
            {
                rc = newRootDir.create();
            }
        }

        if (rc == true)
        {
            gtUInt32 numFiles = mz_zip_reader_get_num_files(&zip);

            for (gtUInt32 i = 0; i < numFiles && rc ; i++)
            {
                mz_zip_archive_file_stat stat;
                rc = (mz_zip_reader_file_stat(&zip, i, &stat) == MZ_TRUE);

                if (rc == true)
                {
                    gtString innerFileFullPath;
                    ComposeItemFullPath(archiveRootDir, stat.m_filename, innerFileFullPath);

                    if (mz_zip_reader_is_file_a_directory(&zip, i) == MZ_TRUE)
                    {
                        // written but not tested, wasn't relevant
                        osDirectory newDir(innerFileFullPath);
                        rc = newDir.create();
                    }
                    else
                    {
                        rc = ValidateFileDirectoryExists(innerFileFullPath);

                        if (rc)
                        {
                            rc = (mz_zip_reader_extract_to_file(&zip, i, innerFileFullPath.asASCIICharArray(), 0) == MZ_TRUE);
                        }
                    }
                }
            }
        }

        // finalize
        mz_zip_reader_end(&zip);
    }

    return rc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Decompression

bool osDirectorySerializer::CreateOutputDir(gtString& dstFileFullPath, const gtString& compressedFile)
{
    bool success = true;

    // create a root folder with the archive's filename output dir if needed
    // (i.e.: if the compressed file name is "archive.zip" create a folder named "archive" in m_outFileFullPath )
    dstFileFullPath = m_outFileFullPath;
    dstFileFullPath.replace(L'\\', L'/');
    int pos = compressedFile.reverseFind(L'/');

    if (-1 != pos)
    {
        gtString newDirName;
        compressedFile.getSubString(pos, compressedFile.length() - 1, newDirName);
        dstFileFullPath.append(newDirName);
    }

    TrimFileExtension(dstFileFullPath);

    osDirectory newRootDir(dstFileFullPath);

    if (newRootDir.exists() == false)
    {
        success = newRootDir.create();
    }

    return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compression

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper path manipulation methods
void osDirectorySerializer::TrimFileExtension(gtString& dstFileFullPath)
{
    int pos = dstFileFullPath.reverseFind(L'.');

    if (-1 != pos)
    {
        dstFileFullPath.truncate(0, pos - 1);
    }
}

bool osDirectorySerializer::ValidateFileDirectoryExists(const gtString& fileFullPath)
{
    bool rc = true;
    osFilePath filePath(fileFullPath);
    osDirectory newRootDir;
    filePath.getFileDirectory(newRootDir);

    if (newRootDir.exists() == false)
    {
        rc = newRootDir.create();
    }

    return rc;
}


void osDirectorySerializer::ComposeItemFullPath(gtString& archiveRootDir, const char* relFileName, gtString& outItemName)
{
    outItemName = archiveRootDir;
    outItemName.append(L'/');
    gtString tmpStr;
    tmpStr.fromASCIIString(relFileName);
    outItemName.append(tmpStr);
}

void osDirectorySerializer::ComposeItemRelPath(const osDirectory& osDir, const gtString& relativePath, gtString& relPathAddition)
{
    gtString dirName = osDir.directoryPath().fileDirectoryAsString();
    dirName.replace(L'\\', L'/');

    int startPos = dirName.findLastOf(L"/") + 1;
    dirName.getSubString(startPos, (dirName.length() - startPos), dirName);
    relPathAddition = relativePath;
    relPathAddition.append(dirName);
}

