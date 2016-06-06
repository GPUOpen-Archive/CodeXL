#ifndef _MODULEINFO_HPP_
#define _MODULEINFO_HPP_
#pragma once

#ifndef ALLOC_POOL_TAG
#   define ALLOC_POOL_TAG  'WktS'
#endif

#include <WinDriverUtils\Include\Common.h>

struct ModuleInfo : public NonPagedObject
{
#ifdef _X86_

    // The start address of the code (executable) section
    ULONG_PTR m_startAddr;

    // The size of the code
    ULONG m_codeSize;


    ULONG_PTR GetAddress() const { return m_startAddr; }
    ULONG GetSize() const { return m_codeSize; }

#else

    // The base address of the module
    ULONG_PTR m_baseAddr;

    // The virtual size of the module
    ULONG m_imageSize;

    // The number of entries in the runtime function table
    ULONG m_funcTableSize;

    // Entry to the runtime function table
    RUNTIME_FUNCTION* m_pFunctionTable;


    ULONG_PTR GetAddress() const { return m_baseAddr; }
    ULONG GetSize() const { return m_imageSize; }

#endif

    void Initialize(ULONG_PTR addr, ULONG size);
    void Initialize(ULONG_PTR addr, ULONG size, KIRQL irql);

#ifdef _AMD64_
    bool IsFunctionTableInitialized() const;
    void ClearFunctionTable();
    void InitializeFunctionTable(KIRQL irql);

    RUNTIME_FUNCTION* LookupFunctionEntry(ULONG_PTR addr, KIRQL irql);
#endif
};

class ModuleInfoList
{
private:
    enum { INITIAL_BUFFER_SIZE = 1024 * 16 };

    ModuleInfo* m_pHead;
    ULONG m_length;
    ULONG m_capacity;


    ModuleInfo* InsertSorted(const ModuleInfo* pInfo, ULONG pos);

public:
    ModuleInfoList();
    ~ModuleInfoList();

    bool IsEmpty() const { return 0UL == m_length; }
    ULONG GetLength() const { return m_length; }
    ULONG GetCapacity() const { return m_capacity; }

    void Clear();
    bool Grow(ULONG length = INITIAL_BUFFER_SIZE / sizeof(ModuleInfo));

    ModuleInfo* Insert(const ModuleInfo& info);
    ModuleInfo* Lookup(ULONG_PTR addr);

    const ModuleInfo* GetBegin() const { return m_pHead; }
    const ModuleInfo* GetEnd() const { return m_pHead + (m_length - 1UL); }
};

#endif // _MODULEINFO_HPP_
