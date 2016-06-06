#include "..\Include\ModuleInfo.hpp"
#include <ntimage.h>


#ifdef _X86_
    #define m_addr   m_startAddr
    #define m_size   m_codeSize
#else
    #define m_addr   m_baseAddr
    #define m_size   m_imageSize
#endif


#ifdef _AMD64_
    #define UNINITIALIZED_FUNC_TABLE_SIZE ((ULONG)-1)
#endif


void ModuleInfo::Initialize(ULONG_PTR addr, ULONG size)
{
    m_addr = addr;
    m_size = size;

#ifdef _AMD64_
    ClearFunctionTable();
#endif
}


void ModuleInfo::Initialize(ULONG_PTR addr, ULONG size, KIRQL irql)
{
    m_addr = addr;
    m_size = size;

#ifdef _AMD64_
    InitializeFunctionTable(irql);
#else
    UNREFERENCED_PARAMETER(irql);
#endif
}


#ifdef _AMD64_

bool ModuleInfo::IsFunctionTableInitialized() const
{
    return !(NULL == m_pFunctionTable && 0UL != m_funcTableSize);
}


void ModuleInfo::ClearFunctionTable()
{
    m_pFunctionTable = NULL;
    m_funcTableSize = UNINITIALIZED_FUNC_TABLE_SIZE;
}


void ModuleInfo::InitializeFunctionTable(KIRQL irql)
{
    if (KxIsAddressValid(reinterpret_cast<PVOID>(m_baseAddr), irql))
    {
        m_pFunctionTable = reinterpret_cast<RUNTIME_FUNCTION*>(RtlImageDirectoryEntryToData(reinterpret_cast<PVOID>(m_baseAddr),
                                                               TRUE,
                                                               IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                                                               &m_funcTableSize));

        if (NULL != m_pFunctionTable)
        {
            m_funcTableSize /= sizeof(RUNTIME_FUNCTION);
        }
        else
        {
            m_funcTableSize = 0UL;
        }
    }
    else
    {
        ClearFunctionTable();
    }
}


// Return the master function table entry for a specified function table entry.
static FORCEINLINE RUNTIME_FUNCTION* ConvertFunctionEntry(RUNTIME_FUNCTION* pFunctionEntry, ULONG_PTR imageBase, KIRQL irql)
{
    //
    // If the specified function entry is not NULL and specifies indirection,
    // then compute the address of the master function table entry.
    //
    if (NULL != pFunctionEntry && 0 != (pFunctionEntry->UnwindData & RUNTIME_FUNCTION_INDIRECT))
    {
        pFunctionEntry = reinterpret_cast<RUNTIME_FUNCTION*>(imageBase + pFunctionEntry->UnwindData - 1);

        if (!KxIsAddressValid(pFunctionEntry, irql))
        {
            pFunctionEntry = NULL;
        }
    }

    return pFunctionEntry;
}


RUNTIME_FUNCTION* ModuleInfo::LookupFunctionEntry(ULONG_PTR addr, KIRQL irql)
{
    RUNTIME_FUNCTION* pFunctionEntry = NULL;

    if (NULL != m_pFunctionTable && 0UL != m_funcTableSize)
    {
        RUNTIME_FUNCTION* pFunctionTable = m_pFunctionTable;
        ULONG relativeAddr = static_cast<ULONG>(addr - m_baseAddr);
        ULONG middle;
        ULONG low = 0UL;
        ULONG high = m_funcTableSize - 1UL;

        while (high >= low)
        {
            //
            // Compute next probe index and test entry. If the specified
            // address is greater than of equal to the beginning address
            // and less than the ending address of the function table entry,
            // then return the address of the function table entry. Otherwise,
            // continue the search.
            //

            middle = (low + high) >> 1;
            pFunctionEntry = pFunctionTable + middle;

            if (!KxIsAddressValid(pFunctionEntry, irql))
            {
                pFunctionEntry = NULL;
                break;
            }

            if (relativeAddr < pFunctionEntry->BeginAddress)
            {
                high = middle - 1UL;
            }
            else if (relativeAddr >= pFunctionEntry->EndAddress)
            {
                low = middle + 1UL;
            }
            else
            {
                break;
            }
        }

        if (high < low)
        {
            pFunctionEntry = NULL;
        }
        else
        {
            pFunctionEntry = ConvertFunctionEntry(pFunctionEntry, m_baseAddr, irql);
        }
    }

    return pFunctionEntry;
}

#endif // _AMD64_



ModuleInfoList::ModuleInfoList() : m_pHead(NULL), m_length(0UL), m_capacity(0UL)
{
}


ModuleInfoList::~ModuleInfoList()
{
    if (NULL != m_pHead)
    {
        delete [] m_pHead;
    }
}


void ModuleInfoList::Clear()
{
    if (NULL != m_pHead)
    {
        delete [] m_pHead;
        m_pHead = NULL;
    }

    m_length = 0UL;
    m_capacity = 0UL;
}



bool ModuleInfoList::Grow(ULONG length)
{
    return (0UL < length) ? NULL != InsertSorted(NULL, m_length + length - 1UL) : true;
}


ModuleInfo* ModuleInfoList::InsertSorted(const ModuleInfo* pInfo, ULONG pos)
{
    ModuleInfo* pNewInfo;

    if (NULL != pInfo && pos < m_length && pInfo->m_addr == m_pHead[pos].m_addr)
    {
        pNewInfo = m_pHead + pos;
        RtlCopyMemory(pNewInfo, pInfo, sizeof(ModuleInfo));
    }
    else
    {
        ULONG newCapacity = pos + 1UL;

        if (m_capacity < newCapacity)
        {
            ULONG newSize = newCapacity * sizeof(ModuleInfo);
            newSize = ALIGN_UP_BY(newSize, INITIAL_BUFFER_SIZE);

            newCapacity = newSize / sizeof(ModuleInfo);

            // Re-allocate buffer size.
            ModuleInfo* pNewBuffer = new ModuleInfo[newCapacity];
            ASSERT(NULL != pNewBuffer);

            if (NULL != pNewBuffer)
            {
                if (NULL != m_pHead)
                {
                    if (0UL != m_length)
                    {
                        if (0UL != pos)
                        {
                            // Copy data from the previous buffer.
                            RtlCopyMemory(pNewBuffer, m_pHead, pos * sizeof(ModuleInfo));
                        }

                        if (pos < m_length)
                        {
                            // Copy data from the previous buffer and skip the insertion position.
                            RtlCopyMemory(pNewBuffer + pos + 1, m_pHead + pos, (m_length - pos) * sizeof(ModuleInfo));
                        }
                    }

                    ModuleInfo* pOldBuffer = m_pHead;
                    m_pHead = pNewBuffer;
                    _ReadWriteBarrier();

                    // Free the previous allocated buffer.
                    delete [] pOldBuffer;
                }
                else
                {
                    m_pHead = pNewBuffer;
                }

                m_capacity = newCapacity;

                pNewInfo = m_pHead + pos;
            }
            else
            {
                pNewInfo = NULL;
            }
        }
        else
        {
            pNewInfo = m_pHead + pos;

            if (pos < m_length)
            {
                // Copy data from the previous buffer and skip the insertion position.
                RtlMoveMemory(pNewInfo + 1, pNewInfo, (m_length - pos) * sizeof(ModuleInfo));
            }
        }

        if (NULL != pInfo && NULL != pNewInfo)
        {
            RtlCopyMemory(pNewInfo, pInfo, sizeof(ModuleInfo));
            m_length++;
        }
    }

    return pNewInfo;
}


ModuleInfo* ModuleInfoList::Insert(const ModuleInfo& info)
{
    ModuleInfo* pListEnd = m_pHead + m_length;
    ModuleInfo* pLowerBound = m_pHead;
    LONG count = static_cast<LONG>(m_length);
    ULONG_PTR addr = info.m_addr;

    ASSERT(0L == count || (0L < count && NULL != m_pHead));

    while (0L < count)
    {
        LONG middle = count / 2L;
        ModuleInfo* pMid = pLowerBound + middle;

        if (pMid->m_addr < addr)
        {
            pLowerBound = ++pMid;
            count -= middle + 1L;
        }
        else
        {
            count = middle;
        }
    }

    ULONG pos;

    if (pLowerBound != pListEnd)
    {
        pos = static_cast<ULONG>(pLowerBound - m_pHead);
    }
    else
    {
        pos = m_length;
    }

    return InsertSorted(&info, pos);
}


ModuleInfo* ModuleInfoList::Lookup(ULONG_PTR addr)
{
    ModuleInfo* pEntry = NULL;

    if (NULL != m_pHead && 0UL != m_length)
    {
        LONG middle;
        LONG low = 0L;
        LONG high = static_cast<LONG>(m_length) - 1L;

        while (high >= low)
        {
            //
            // Compute next probe index and test entry. If the specified
            // address is greater than of equal to the beginning address
            // and less than the ending address of the range entry,
            // then return the address of the range entry. Otherwise,
            // continue the search.
            //

            middle = (low + high) >> 1;
            pEntry = m_pHead + middle;

            if (addr < pEntry->m_addr)
            {
                high = middle - 1L;
            }
            else if (addr >= (pEntry->m_addr + pEntry->m_size))
            {
                low = middle + 1L;
            }
            else
            {
                break;
            }
        }

        if (high < low)
        {
            pEntry = NULL;
        }
    }

    return pEntry;
}
