//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtSmallSList.h
///
//=====================================================================
#ifndef __GTSMALLSLIST
#define __GTSMALLSLIST

// Local:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>


template <typename T> struct gtSmallSList_node
{
    gtSmallSList_node* m_pNext;
    T m_val;

    gtSmallSList_node() {}
    gtSmallSList_node(const T& val) : m_val(val) {}
    bool IsAnchor() const;
};


template <typename T, bool isConst>
class gtSmallSList_iterator
{
public:
    typedef std::forward_iterator_tag iterator_category;
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef typename ConditionalType<isConst, const T&, T&>::type reference;
    typedef typename ConditionalType<isConst, const T*, T*>::type pointer;
    typedef typename ConditionalType<isConst, const gtSmallSList_node<T>*, gtSmallSList_node<T>*>::type nodeptr;

    gtSmallSList_iterator(nodeptr x = NULL) : m_pNode(x) {}
    gtSmallSList_iterator(const gtSmallSList_iterator<T, false>& i) : m_pNode(i.m_pNode) {}

    reference operator*() const
    {
        return m_pNode->m_val;
    }

    pointer operator->() const
    {
        return &(m_pNode->m_val);
    }

    gtSmallSList_iterator& operator++()
    {
        m_pNode = m_pNode->m_pNext;
        return *this;
    }

    gtSmallSList_iterator operator++(int)
    {
        gtSmallSList_iterator tmp(*this);
        ++*this;
        return tmp;
    }

    friend bool operator==(const gtSmallSList_iterator& itLeft, const gtSmallSList_iterator& itRight)
    {
        return itLeft.m_pNode == itRight.m_pNode;
    }

    friend bool operator!=(const gtSmallSList_iterator& itLeft, const gtSmallSList_iterator& itRight)
    {
        return itLeft.m_pNode != itRight.m_pNode;
    }

    nodeptr GetNode() { return m_pNode; }

private:
    nodeptr m_pNode;

    friend class gtSmallSList_iterator<T, false>;
    friend class gtSmallSList_iterator<T, true>;
};


template <typename T> class gtSmallSList
{
private:
    gtSmallSList_node<T> m_head;

    gtSmallSList& operator=(const gtSmallSList& other);

public:
    typedef gtSmallSList_iterator<T, true>  const_iterator;
    typedef gtSmallSList_iterator<T, false>       iterator;

    // *INDENT-OFF*
    const_iterator begin() const { return IsEmpty() ? end() : const_iterator(&m_head); }
          iterator begin()       { return IsEmpty() ? end() :       iterator(&m_head); }
    const_iterator end() const { return const_iterator(NULL); }
          iterator end()       { return       iterator(NULL); }
    // *INDENT-ON*


    gtSmallSList() : m_head(T())
    {
        m_head.m_pNext = NULL;
    }

    gtSmallSList(const gtSmallSList& other) : m_head(other.m_head.m_val)
    {
        const gtSmallSList_node<T>* pNode = &other.m_head;
        gtSmallSList_node<T>* pMyNode = &m_head;

        for (pNode = pNode->m_pNext; NULL != pNode; pNode = pNode->m_pNext)
        {
            pMyNode->m_pNext = new gtSmallSList_node<T>(pNode->m_val);
            pMyNode = pMyNode->m_pNext;
        }

        pMyNode->m_pNext = NULL;
    }

#if AMDT_HAS_CPP0X
    gtSmallSList(gtSmallSList&& right) : m_head(right.m_head)
    {
        right.m_head.m_pNext = NULL;
        right.m_head.m_val = T();
    }

    gtSmallSList& operator=(gtSmallSList&& right)
    {
        if (this != &right)
        {
            m_head = right.m_head;
            right.m_head.m_pNext = NULL;
            right.m_head.m_val = T();
        }

        return *this;
    }
#endif

    ~gtSmallSList()
    {
        gtSmallSList_node<T>* pNext;

        for (gtSmallSList_node<T>* pNode = m_head.m_pNext; NULL != pNode; pNode = pNext)
        {
            pNext = pNode->m_pNext;
            delete pNode;
        }
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Checks whether the list is empty.
    ///
    /// \return A boolean value indicating whether the list is empty.
    /// -----------------------------------------------------------------------------------------------
    bool IsEmpty() const
    {
        return m_head.IsAnchor();
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Inserts the specified element after the head of the list.
    ///
    /// \param[in] val The element value to insert.
    ///
    /// \note If the list is empty, the element is inserted as the new head.
    /// \remark This is a fast insertion.
    /// -----------------------------------------------------------------------------------------------
    void InsertAfterHead(const T& val)
    {
        if (IsEmpty())
        {
            m_head.m_val = val;
        }
        else
        {
            gtSmallSList_node<T>* pNode = new gtSmallSList_node<T>(val);
            pNode->m_pNext = m_head.m_pNext;
            m_head.m_pNext = pNode;
        }
    }

    /// -----------------------------------------------------------------------------------------------
    /// \brief Inserts the specified element after the specified position in the list.
    ///
    /// \param[in] pos The iterator to insert after. If this is the \a end iterator, the insertion will be at the head.
    /// \param[in] val The element value to insert.
    /// -----------------------------------------------------------------------------------------------
    void InsertAfter(iterator pos, const T& val)
    {
        gtSmallSList_node<T>* pNodePos = nullptr;
        gtSmallSList_node<T>* pNode = nullptr;

        if (end() == pos)
        {
            if (!IsEmpty())
            {
                pNodePos = &m_head;
                pNode = new gtSmallSList_node<T>(m_head.m_val);
            }

            m_head.m_val = val;
        }
        else
        {
            pNodePos = pos.GetNode();
            pNode = new gtSmallSList_node<T>(val);
        }

        if (NULL != pNode)
        {
            pNode->m_pNext = pNodePos->m_pNext;
            pNodePos->m_pNext = pNode;
        }
    }
};

#endif // __GTSMALLSLIST
