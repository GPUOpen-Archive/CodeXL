#include "Include/beD3DIncludeManager.h"
#include <fstream>

// Infra.
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtAssert.h>


D3DIncludeManager::D3DIncludeManager(const std::string& shaderDir, const std::vector<std::string>& includeSearchDirs) : m_shaderDir(shaderDir), m_includeSearchDirs(includeSearchDirs)
{
}


D3DIncludeManager::~D3DIncludeManager()
{
}

static bool OpenIncludeFile(const std::string& includeFileFullPath, char*& pFileBuffer, unsigned& fileSizeInBytes)
{
    bool ret = false;

    // Open the file.
    std::ifstream includeFile(includeFileFullPath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

    if (includeFile.is_open())
    {
        // Get the file size.
        fileSizeInBytes = (unsigned int)includeFile.tellg();

        if (fileSizeInBytes > 0)
        {
            // Allocate the buffer.
            pFileBuffer = new char[fileSizeInBytes];

            // Read the file's contents into the buffer.
            includeFile.seekg(0, std::ios::beg);
            includeFile.read(pFileBuffer, fileSizeInBytes);

            // Close the file.
            includeFile.close();

            // We are done.
            ret = true;
        }
    }

    return ret;
}

// Returns true if fullString starts with substring.
static bool IsBeginsWith(std::string const& fullString, std::string const& substring)
{
    bool ret = (fullString.size() >= substring.size());
    ret = ret && (fullString.compare(0, substring.length(), substring) == 0);
    return ret;
}

// Returns true if fullString ends with substring.
static bool IsEndsWith(std::string const& fullString, std::string const& substring)
{
    bool ret = false;

    if (fullString.length() >= substring.length())
    {
        ret = (fullString.compare(fullString.length() - substring.length(), substring.length(), substring) == 0);
    }

    return ret;
}

// Adds a directory separator character to the path, if required.
static void AdjustIncludePath(std::string& includePath)
{
    const std::string DIR_SEPARATOR = "\\";

    if (!IsEndsWith(includePath, DIR_SEPARATOR))
    {
        includePath += DIR_SEPARATOR;
    }
}


STDMETHODIMP D3DIncludeManager::Open(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
{
    GT_UNREFERENCED_PARAMETER(pParentData);
    bool isDone = false;

    if (pFileName != nullptr)
    {
        std::string includeFileFullPath;

        switch (IncludeType)
        {
            case D3D_INCLUDE_LOCAL:
            {
                // First, try the shader's directory.
                includeFileFullPath = m_shaderDir;

                // Is it a relative path to the shader's directory.
                bool isRelative = IsBeginsWith(pFileName, "\\");

                if (!isRelative)
                {
                    AdjustIncludePath(includeFileFullPath);
                }

                includeFileFullPath += pFileName;
                isDone = OpenIncludeFile(includeFileFullPath, (char*&) * ppData, *pBytes);

                if (!isDone)
                {
                    // Search in the user-defined directories.
                    for (const std::string& includeDir : m_includeSearchDirs)
                    {
                        includeFileFullPath = includeDir;

                        if (!isRelative)
                        {
                            AdjustIncludePath(includeFileFullPath);
                        }

                        includeFileFullPath += pFileName;
                        isDone = OpenIncludeFile(includeFileFullPath, (char*&) * ppData, *pBytes);

                        if (isDone)
                        {
                            break;
                        }
                    }
                }

                break;
            }

            case D3D_INCLUDE_SYSTEM:
            {
                // First, try the shader's directory.
                includeFileFullPath = m_shaderDir;
                AdjustIncludePath(includeFileFullPath);
                includeFileFullPath += pFileName;
                isDone = OpenIncludeFile(includeFileFullPath, (char*&) * ppData, *pBytes);

                if (!isDone)
                {
                    // Go through the directories which the user specified.
                    for (const std::string& includeDir : m_includeSearchDirs)
                    {
                        includeFileFullPath = includeDir;
                        AdjustIncludePath(includeFileFullPath);
                        includeFileFullPath += pFileName;
                        isDone = OpenIncludeFile(includeFileFullPath, (char*&) * ppData, *pBytes);

                        if (isDone)
                        {
                            break;
                        }
                    }
                }

                break;
            }

            default:
                GT_ASSERT_EX(false, L"Unknown D3D include type.");
                break;
        }
    }

    // Must return S_OK according to the documentation.
    return (isDone ? S_OK : E_FAIL);
}

STDMETHODIMP D3DIncludeManager::Close(THIS_ LPCVOID pData)
{
    char* buf = (char*)pData;
    delete[] buf;
    return S_OK;
}
