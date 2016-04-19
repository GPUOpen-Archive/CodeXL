#ifndef _CPUPROF_FILEWRITER_HPP_
#define _CPUPROF_FILEWRITER_HPP_
#pragma once

#include "CpuProfCommon.hpp"

namespace CpuProf {

class FileWriter : ExplicitObject
{
private:
    HANDLE m_handle;

public:
    FileWriter();
    ~FileWriter();

    bool Open(const wchar_t* pFilePath, ULONG length);
    void Close();

    bool IsOpened() const { return NULL != m_handle; }

    bool Write(const void* pBuffer, ULONG length);
    bool Write(const void* pBuffer, ULONG length, ULONG64 offset);

    template<class Ty>
    bool Write(const Ty& val)
    {
        return Write(&val, sizeof(Ty));
    }

    ULONG GetPath(wchar_t* pBuffer, ULONG length) const;
};

} // namespace CpuProf

#endif // _CPUPROF_FILEWRITER_HPP_
