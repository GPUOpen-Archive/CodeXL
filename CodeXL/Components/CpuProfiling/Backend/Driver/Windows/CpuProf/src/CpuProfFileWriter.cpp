#include "..\inc\CpuProfFileWriter.hpp"

namespace CpuProf
{

FileWriter::FileWriter()
{
    ;
}


FileWriter::~FileWriter()
{
    Close();
}


bool FileWriter::Open(const wchar_t* pFilePath, ULONG length)
{
    Close();

    if (NULL != pFilePath)
    {
        length *= sizeof(wchar_t);

        wchar_t fullName[_MAX_PATH + _MAX_DRIVE_NAME];
        fullName[0] = L'\\';
        fullName[1] = L'?';
        fullName[2] = L'?';
        fullName[3] = L'\\';
        RtlCopyMemory(&fullName[4], pFilePath, sizeof(wchar_t) + length);

        UNICODE_STRING unicodePath;
        OBJECT_ATTRIBUTES objAttrs;
        IO_STATUS_BLOCK ioStatus;

        unicodePath.Length = static_cast<USHORT>(length + (4 * sizeof(wchar_t)));
        unicodePath.MaximumLength = sizeof(fullName);
        unicodePath.Buffer = fullName;

        InitializeObjectAttributes(&objAttrs, &unicodePath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

        // If the file already exists, replace it with the given file.
        NTSTATUS status = ZwCreateFile(&m_handle,
                                       GENERIC_WRITE,
                                       &objAttrs,
                                       &ioStatus,
                                       NULL,
                                       FILE_ATTRIBUTE_NORMAL,
                                       0UL,
                                       FILE_SUPERSEDE,
                                       FILE_SYNCHRONOUS_IO_NONALERT,
                                       NULL,
                                       0UL);

        if (STATUS_SUCCESS != status || STATUS_SUCCESS != ioStatus.Status)
        {
            m_handle = NULL;
        }
    }

    return (NULL != m_handle);
}


void FileWriter::Close()
{
    if (NULL != m_handle)
    {
        ZwClose(m_handle);
        m_handle = NULL;
    }
}


bool FileWriter::Write(const void* pBuffer, ULONG length)
{
    bool ret;

    if (NULL != m_handle)
    {
        IO_STATUS_BLOCK ioStatus;
        LARGE_INTEGER offset;
        offset.HighPart = -1;
        offset.LowPart = FILE_WRITE_TO_END_OF_FILE;

        NTSTATUS status = ZwWriteFile(m_handle, NULL, NULL, NULL, &ioStatus, const_cast<void*>(pBuffer), length, &offset, NULL);

        ret = (STATUS_SUCCESS == status && STATUS_SUCCESS == ioStatus.Status);
    }
    else
    {
        ret = false;
    }

    return ret;
}


bool FileWriter::Write(const void* pBuffer, ULONG length, ULONG64 offset)
{
    bool ret;

    if (NULL != m_handle)
    {
        IO_STATUS_BLOCK ioStatus;

        NTSTATUS status = ZwWriteFile(m_handle, NULL, NULL, NULL, &ioStatus, const_cast<void*>(pBuffer), length,
                                      reinterpret_cast<LARGE_INTEGER*>(&offset), NULL);

        ret = (STATUS_SUCCESS == status && STATUS_SUCCESS == ioStatus.Status);
    }
    else
    {
        ret = false;
    }

    return ret;
}


ULONG FileWriter::GetPath(wchar_t* pBuffer, ULONG length) const
{
    ULONG nameLen = 0UL;

    if (NULL != m_handle)
    {
        char infoBuffer[sizeof(FILE_NAME_INFORMATION) + (sizeof(WCHAR) * (_MAX_PATH - 1))];
        FILE_NAME_INFORMATION* pFileNameInfo = reinterpret_cast<FILE_NAME_INFORMATION*>(infoBuffer);
        pFileNameInfo->FileNameLength = 0UL;
        pFileNameInfo->FileName[0] = L'\0';

        IO_STATUS_BLOCK ioStatus;
        NTSTATUS status = ZwQueryInformationFile(m_handle, &ioStatus, pFileNameInfo, sizeof(infoBuffer), FileNameInformation);

        if (STATUS_SUCCESS == status && STATUS_SUCCESS == ioStatus.Status && sizeof(WCHAR) < pFileNameInfo->FileNameLength)
        {
            length = length * sizeof(WCHAR);

            if (pFileNameInfo->FileNameLength < length)
            {
                length = pFileNameInfo->FileNameLength;
            }

            RtlCopyMemory(pBuffer, pFileNameInfo->FileName, length);

            nameLen = length / sizeof(WCHAR);
            pBuffer[nameLen] = L'\0';
        }
    }

    return nameLen;
}

} // namespace CpuProf
