//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtDenseIndexSet.h
///
//=====================================================================
#ifndef __GTDENSEINDEXSET
#define __GTDENSEINDEXSET

// Local:
#include <AMDTBaseTools/Include/gtAlgorithms.h>

template <unsigned BucketIndicesCount = 1024>
class gtDenseIndexSet
{
private:
    enum
    {
        STATE_EMPTY = 0,
        STATE_STATIC = 1,
        STATE_DYNAMIC_ANCHOR = 2
    };

public:
    enum
    {
        BUCKET_INDICES_COUNT = GT_ALIGN_UP((1 << Log2<BucketIndicesCount>::VALUE), sizeof(gtUInt32*)),

        HASH_BUCKET_COUNT = BUCKET_INDICES_COUNT / (sizeof(gtUInt32*) * 8),
        HASH_BUCKET_SHIFT = Log2<BUCKET_INDICES_COUNT>::VALUE,

        INDEX_BUCKET_COUNT = HASH_BUCKET_COUNT * sizeof(gtUInt32*) / sizeof(gtUInt32),
        INDEX_BUCKET_SHIFT = Log2<sizeof(gtUInt32) * 8>::VALUE,
        INDEX_BIT_MASK = (1 << INDEX_BUCKET_SHIFT) - 1
    };

    gtDenseIndexSet() : m_state(gtIntPtr(STATE_EMPTY)), m_baseIndex(0)
    {
        memset(m_indices, 0, sizeof(m_indices));
    }

    gtDenseIndexSet(const gtDenseIndexSet& other)
    {
        memcpy(this, &other, sizeof(*this));

        if (gtIntPtr(STATE_EMPTY) != m_state && gtIntPtr(STATE_STATIC) != m_state)
        {
            gtUInt32** ppIndices = m_hashTable;

            for (int i = 0; i < HASH_BUCKET_COUNT; ++i, ++ppIndices)
            {
                if (NULL != *ppIndices)
                {
                    gtUInt32* pIndices = *ppIndices;
                    *ppIndices = new gtUInt32[INDEX_BUCKET_COUNT];
                    memcpy(*ppIndices, pIndices, sizeof(gtUInt32) * INDEX_BUCKET_COUNT);
                }
            }

            if (gtIntPtr(STATE_DYNAMIC_ANCHOR) != m_state)
            {
                m_pNext = new gtDenseIndexSet(*m_pNext);
            }
        }
    }

#if AMDT_HAS_CPP0X
    gtDenseIndexSet(gtDenseIndexSet&& right)
    {
        memcpy(this, &right, sizeof(*this));

        if (gtIntPtr(STATE_EMPTY) != right.m_state)
        {
            right.m_state = gtIntPtr(STATE_EMPTY);
            memset(right.m_indices, 0, sizeof(right.m_indices));
        }
    }
#endif

    ~gtDenseIndexSet()
    {
        Destroy();
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Checks whether the set is empty.
    ///
    /// \return A boolean value indicating whether the set is empty.
    /// -----------------------------------------------------------------------------------------------
    bool IsEmpty() const { return gtIntPtr(STATE_EMPTY) == m_state; }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Adds the specified index number to the set.
    ///
    /// \param[in] index The index number to add to the set.
    /// -----------------------------------------------------------------------------------------------
    void Add(gtUInt32 index)
    {
        if (gtIntPtr(STATE_STATIC) == m_state)
        {
            if (m_baseIndex <= index && index < (m_baseIndex + BUCKET_INDICES_COUNT))
            {
                StaticAdd(index);
            }
            else
            {
                ConvertToDynamic();
                DynamicAdd(index);
            }
        }
        else if (gtIntPtr(STATE_EMPTY) != m_state)
        {
            DynamicAdd(index);
        }
        else
        {
            m_state = gtIntPtr(STATE_STATIC);
            m_baseIndex = gtAlignDown(index, BUCKET_INDICES_COUNT);
            StaticAdd(index);
        }
    }

    void Clear()
    {
        Destroy();
        m_state = gtIntPtr(STATE_EMPTY);
        m_baseIndex = 0;
        memset(m_indices, 0, sizeof(m_indices));
    }


    class const_iterator
    {
    public:
        const_iterator(gtUInt32 index, gtUInt32 bits, const gtDenseIndexSet* pIndexSet) : m_index(index),
            m_bits(bits),
            m_pIndexSet(pIndexSet)
        {
            m_baseIndex = 0;
            m_pIndices = NULL;
            m_pIndicesEnd = NULL;
            m_ppIndicesBegin = NULL;
        }

        gtUInt32 operator*() const
        {
            return m_index;
        }

        const_iterator& operator++()
        {
            for (;;)
            {
                if (0 != m_bits || StaticSearch())
                {
                    IncEncodedIndex();
                }
                else
                {
                    m_index = gtUInt32(-1);

                    if (gtIntPtr(STATE_STATIC) != m_pIndexSet->m_state)
                    {
                        if (!DynamicSearch())
                        {
                            if (gtIntPtr(STATE_DYNAMIC_ANCHOR) == m_pIndexSet->m_state)
                            {
                                break;
                            }
                            else
                            {
                                m_pIndexSet = m_pIndexSet->m_pNext;

                                if (!Initialize())
                                {
                                    break;
                                }
                            }
                        }

                        continue;
                    }
                }

                break;
            }

            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator tmp(*this);
            ++*this;
            return tmp;
        }

        bool operator==(const const_iterator& itRight) const
        {
            return m_index == itRight.m_index;
        }

        bool operator!=(const const_iterator& itRight) const
        {
            return m_index != itRight.m_index;
        }

    private:
        gtUInt32 m_index;
        gtUInt32 m_baseIndex;
        gtUInt32 m_bits;
        const gtUInt32* m_pIndices;
        const gtUInt32* m_pIndicesEnd;
        const gtUInt32* const* m_ppIndicesBegin;

        const gtDenseIndexSet* m_pIndexSet;

        friend class gtDenseIndexSet;

        bool Initialize()
        {
            bool ret = (gtIntPtr(STATE_EMPTY) != m_pIndexSet->m_state);

            if (ret)
            {
                if (gtIntPtr(STATE_STATIC) == m_pIndexSet->m_state)
                {
                    m_pIndices = m_pIndexSet->m_indices;
                    m_pIndicesEnd = m_pIndices + INDEX_BUCKET_COUNT;
                    m_baseIndex = m_pIndexSet->m_baseIndex;
                    m_bits = *m_pIndices;
                }
                else
                {
                    m_baseIndex = m_pIndexSet->m_baseIndex - BUCKET_INDICES_COUNT;
                    m_ppIndicesBegin = m_pIndexSet->m_hashTable - 1;
                    ret = DynamicSearch();
                }
            }

            return ret;
        }

        void IncEncodedIndex()
        {
            gtUInt32 shift = CountTrailingZeros(m_bits);
            m_bits ^= 1 << shift;
            m_index = m_baseIndex + shift;
        }

        bool StaticSearch()
        {
            const gtUInt32* pIndices = 1 + m_pIndices;

            while (pIndices < m_pIndicesEnd)
            {
                if (0 != *pIndices)
                {
                    m_baseIndex += (sizeof(gtUInt32) * 8) * (pIndices - m_pIndices);
                    m_bits = *pIndices;
                    m_pIndices = pIndices;
                    break;
                }

                pIndices++;
            }

            return pIndices < m_pIndicesEnd;
        }

        bool DynamicSearch()
        {
            const gtUInt32* const* const pHashTableBegin = m_pIndexSet->m_hashTable;
            const gtUInt32* const* const pHashTableEnd = pHashTableBegin + HASH_BUCKET_COUNT;

            while (++m_ppIndicesBegin < pHashTableEnd)
            {
                if (NULL != *m_ppIndicesBegin)
                {
                    m_baseIndex = (BUCKET_INDICES_COUNT * (m_ppIndicesBegin - pHashTableBegin)) + m_pIndexSet->m_baseIndex;
                    m_pIndices = *m_ppIndicesBegin;
                    m_pIndicesEnd = m_pIndices + INDEX_BUCKET_COUNT;
                    m_bits = *m_pIndices;
                    break;
                }
            }

            return m_ppIndicesBegin < pHashTableEnd;
        }
    };

    const_iterator begin() const
    {
        const_iterator itBegin(gtUInt32(-1), 0, this);

        if (gtIntPtr(STATE_EMPTY) != m_state)
        {
            itBegin.Initialize();
            ++itBegin;
        }

        return itBegin;
    }

    const_iterator end() const
    {
        return const_iterator(gtUInt32(-1), 0, this);
    }

private:
    union
    {
        gtIntPtr m_state;
        gtDenseIndexSet<BucketIndicesCount>* m_pNext;
    };
    gtUInt32 m_baseIndex;
    union
    {
        gtUInt32* m_hashTable[HASH_BUCKET_COUNT];
        gtUInt32  m_indices[INDEX_BUCKET_COUNT];
    };


    void Destroy()
    {
        if (gtIntPtr(STATE_EMPTY) != m_state && gtIntPtr(STATE_STATIC) != m_state)
        {
            gtUInt32** ppIndices = m_hashTable;

            for (int i = 0; i < HASH_BUCKET_COUNT; ++i, ++ppIndices)
            {
                if (NULL != *ppIndices)
                {
                    delete [] *ppIndices;
                }
            }

            if (gtIntPtr(STATE_DYNAMIC_ANCHOR) != m_state)
            {
                delete m_pNext;
            }
        }
    }

    inline void StaticAdd(gtUInt32 index)
    {
        index -= m_baseIndex;
        m_indices[index >> INDEX_BUCKET_SHIFT] |= 1 << (index & INDEX_BIT_MASK);
    }

    void DynamicAdd(gtUInt32 index)
    {
        if (m_baseIndex <= index && index < (m_baseIndex + (BUCKET_INDICES_COUNT * HASH_BUCKET_COUNT)))
        {
            index -= m_baseIndex;
            const gtUInt32 bucketIndex = index >> HASH_BUCKET_SHIFT;
            gtUInt32* pIndices = m_hashTable[bucketIndex];

            if (NULL == pIndices)
            {
                pIndices = new gtUInt32[INDEX_BUCKET_COUNT];
                memset(pIndices, 0, sizeof(gtUInt32) * INDEX_BUCKET_COUNT);
                m_hashTable[bucketIndex] = pIndices;
            }

            index -= BUCKET_INDICES_COUNT * bucketIndex;
            pIndices[index >> INDEX_BUCKET_SHIFT] |= 1 << (index & INDEX_BIT_MASK);
        }
        else
        {
            if (gtIntPtr(STATE_DYNAMIC_ANCHOR) == m_state)
            {
                m_pNext = new gtDenseIndexSet<BucketIndicesCount>();
            }

            m_pNext->Add(index);
        }
    }

    inline void ConvertToDynamic()
    {
        m_state = gtIntPtr(STATE_DYNAMIC_ANCHOR);
        gtUInt32* pIndices = new gtUInt32[INDEX_BUCKET_COUNT];
        memcpy(pIndices, m_indices, sizeof(m_indices));
        memset(m_hashTable, 0, sizeof(m_hashTable));
        gtUInt32 hashTableBaseIndex = gtAlignDown(m_baseIndex, BUCKET_INDICES_COUNT * HASH_BUCKET_COUNT);
        m_hashTable[((m_baseIndex - hashTableBaseIndex) >> HASH_BUCKET_SHIFT)] = pIndices;
        m_baseIndex = hashTableBaseIndex;
    }

    gtDenseIndexSet& operator=(const gtDenseIndexSet&);

    friend class const_iterator;
};

#endif // __GTDENSEINDEXSET
