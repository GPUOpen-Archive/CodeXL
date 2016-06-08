//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtFragmentedVector.h
///
//=====================================================================
#ifndef __GTFRAGMENTEDVECTOR
#define __GTFRAGMENTEDVECTOR

// Local:
#include <AMDTBaseTools/Include/gtAlgorithms.h>

template <class Ty, unsigned MaxBlockSize = 4096>
class gtFragmentedVectorTraits
{
    enum
    {
        MAX_BLOCK_SIZE = ((sizeof(Ty) * 2) < MaxBlockSize) ? MaxBlockSize : (sizeof(Ty) * 2)
    };

public:
    enum
    {
        BLOCK_SHIFT = Log2 < MAX_BLOCK_SIZE / sizeof(Ty) >::VALUE,
        BLOCK_LEN = 1 << BLOCK_SHIFT,
        BLOCK_MASK = BLOCK_LEN - 1
    };
};


template <class T, unsigned MaxBlockSize, bool isConst>
class gtFragmentedVector_iterator
{
public:
    enum
    {
        BLOCK_SHIFT = gtFragmentedVectorTraits<T, MaxBlockSize>::BLOCK_SHIFT,
        BLOCK_LEN   = gtFragmentedVectorTraits<T, MaxBlockSize>::BLOCK_LEN,
        BLOCK_MASK  = gtFragmentedVectorTraits<T, MaxBlockSize>::BLOCK_MASK
    };

    typedef std::forward_iterator_tag iterator_category;
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef typename ConditionalType<isConst, const T&, T&>::type reference;
    typedef typename ConditionalType<isConst, const T*, T*>::type pointer;

    gtFragmentedVector_iterator(pointer pData, unsigned index) : m_pData(pData), m_index(index) {}
    gtFragmentedVector_iterator(const gtFragmentedVector_iterator<T, MaxBlockSize, false>& i) : m_pData(i.m_pData), m_index(i.m_index) {}

    reference operator*() const
    {
        return m_pData[m_index & BLOCK_MASK];
    }

    pointer operator->() const
    {
        return &m_pData[m_index & BLOCK_MASK];
    }

    gtFragmentedVector_iterator& operator++()
    {
        if (0U == (++m_index & BLOCK_MASK))
        {
            m_pData = *reinterpret_cast<pointer*>(m_pData + BLOCK_LEN);
        }

        return *this;
    }

    gtFragmentedVector_iterator operator++(int)
    {
        gtFragmentedVector_iterator tmp(*this);
        ++*this;
        return tmp;
    }

    friend bool operator==(const gtFragmentedVector_iterator& itLeft, const gtFragmentedVector_iterator& itRight)
    {
        return itLeft.m_index == itRight.m_index;
    }

    friend bool operator!=(const gtFragmentedVector_iterator& itLeft, const gtFragmentedVector_iterator& itRight)
    {
        return itLeft.m_index != itRight.m_index;
    }

private:
    pointer m_pData;
    unsigned m_index;

    friend class gtFragmentedVector_iterator<T, MaxBlockSize, false>;
    friend class gtFragmentedVector_iterator<T, MaxBlockSize, true>;
};

template <class Ty, unsigned MaxBlockSize = 4096>
class gtFragmentedVector
{
public:
    enum
    {
        BLOCK_SHIFT = gtFragmentedVectorTraits<Ty, MaxBlockSize>::BLOCK_SHIFT,
        BLOCK_LEN   = gtFragmentedVectorTraits<Ty, MaxBlockSize>::BLOCK_LEN,
        BLOCK_MASK  = gtFragmentedVectorTraits<Ty, MaxBlockSize>::BLOCK_MASK
    };

    gtFragmentedVector() : m_size(0U)
    {
        m_data->~Ty();
        *reinterpret_cast<Ty**>(m_data + BLOCK_LEN) = NULL;
    }

    ~gtFragmentedVector()
    {
        clear();
    }

    void clear()
    {
        if (0U != m_size)
        {
            Ty* pData = m_data;

            while (NULL != *reinterpret_cast<Ty**>(pData + BLOCK_LEN))
            {
                Ty* pDataNext = *reinterpret_cast<Ty**>(pData + BLOCK_LEN);

                if (m_data != pData)
                {
                    for (unsigned i = 0U; i < BLOCK_LEN; ++i)
                    {
                        pData[i].~Ty();
                    }

                    free(pData);
                    m_size -= BLOCK_LEN;
                }

                pData = pDataNext;
            }

            if (m_data != pData)
            {
                m_size -= BLOCK_LEN;

                for (unsigned i = 0U; i < m_size; ++i)
                {
                    pData[i].~Ty();
                }

                free(pData);

                pData = m_data;

                for (unsigned i = 0U; i < BLOCK_LEN; ++i)
                {
                    pData[i].~Ty();
                }
            }
            else
            {
                for (unsigned i = 0U; i < m_size; ++i)
                {
                    pData[i].~Ty();
                }
            }

            *reinterpret_cast<Ty**>(m_data + BLOCK_LEN) = NULL;
            m_size = 0U;
        }
    }

    unsigned size() const { return m_size; }

    bool resize(unsigned sz)
    {
        bool ret = sz > m_size;

        if (ret)
        {
            unsigned index = m_size & BLOCK_MASK;
            m_size = sz;
            Ty* pData = m_data;

            for (unsigned block = (m_size - 1U) >> BLOCK_SHIFT; 0U != block; --block)
            {
                Ty* pDataPrev = pData;
                pData = *reinterpret_cast<Ty**>(pData + BLOCK_LEN);

                if (NULL == pData)
                {
                    for (; index < BLOCK_LEN; ++index)
                    {
                        new(&pDataPrev[index]) Ty();
                    }

                    index = 0U;

                    pData = static_cast<Ty*>(malloc((sizeof(Ty) * BLOCK_LEN) + sizeof(void*)));
                    *reinterpret_cast<Ty**>(pData + BLOCK_LEN) = NULL;
                    *reinterpret_cast<Ty**>(pDataPrev + BLOCK_LEN) = pData;
                }
            }

            for (sz &= BLOCK_MASK; index < sz; ++index)
            {
                new(&pData[index]) Ty();
            }
        }

        return ret;
    }

    const Ty& at(unsigned index) const
    {
        const Ty* pData = m_data;

        for (unsigned block = index >> BLOCK_SHIFT; 0U != block; --block)
        {
            pData = *reinterpret_cast<Ty* const*>(pData + BLOCK_LEN);

            if (NULL == pData)
            {
                break;
            }
        }

        if (NULL != pData)
        {
            pData = &pData[index & BLOCK_MASK];
        }

        return *pData;
    }

    Ty& at(unsigned index)
    {
        Ty* pData = m_data;

        for (unsigned block = index >> BLOCK_SHIFT; 0U != block; --block)
        {
            pData = *reinterpret_cast<Ty**>(pData + BLOCK_LEN);

            if (NULL == pData)
            {
                break;
            }
        }

        if (NULL != pData)
        {
            pData = &pData[index & BLOCK_MASK];
        }

        return *pData;
    }

    void push_back(const Ty& val)
    {
        const unsigned index = m_size++;
        Ty* pData = m_data;

        for (unsigned block = index >> BLOCK_SHIFT; 0U != block; --block)
        {
            Ty* pDataPrev = pData;
            pData = *reinterpret_cast<Ty**>(pData + BLOCK_LEN);

            if (NULL == pData)
            {
                pData = static_cast<Ty*>(malloc((sizeof(Ty) * BLOCK_LEN) + sizeof(void*)));
                *reinterpret_cast<Ty**>(pData + BLOCK_LEN) = NULL;
                *reinterpret_cast<Ty**>(pDataPrev + BLOCK_LEN) = pData;
                break;
            }
        }

        new(&pData[index & BLOCK_MASK]) Ty(val);
    }


    typedef gtFragmentedVector_iterator<Ty, MaxBlockSize, true>  const_iterator;
    typedef gtFragmentedVector_iterator<Ty, MaxBlockSize, false>       iterator;

    // *INDENT-OFF*
    const_iterator begin() const { return const_iterator(m_data, 0U); }
          iterator begin()       { return       iterator(m_data, 0U); }
    const_iterator end() const { return const_iterator(m_data, m_size); }
          iterator end()       { return       iterator(m_data, m_size); }
    // *INDENT-ON*

private:
    Ty m_data[1];
    gtByte m_buffer[(sizeof(Ty) * (BLOCK_LEN - 1)) + sizeof(void*)];
    unsigned m_size;

    gtFragmentedVector(const gtFragmentedVector&);
    gtFragmentedVector& operator=(const gtFragmentedVector&);
};

#endif // __GTFRAGMENTEDVECTOR
