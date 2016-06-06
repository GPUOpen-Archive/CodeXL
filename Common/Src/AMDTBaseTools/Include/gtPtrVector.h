//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtPtrVector.h
///
//=====================================================================

//------------------------------ gtPtrVector.h ------------------------------

#ifndef __GTPTRVECTOR
#define __GTPTRVECTOR

// Local:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtVector.h>


// ----------------------------------------------------------------------------------
// Class Name:           gtPtrVector : public gtVector
// General Description:
//   A class representing a vector that contains pointers to elements.
//   It inherits gtVector and adds "pointers specific" methods.
//
//   Deceleration example:
//   gtPtrVector<MyClass*> myPtrVector;
//
// Author:      AMD Developer Tools Team
// Creation Date:        4/4/2005
// ----------------------------------------------------------------------------------
template<class ElementType, class MemoryAllocator = allocator<ElementType> >
class gtPtrVector : public gtVector<ElementType, MemoryAllocator>
{
public:
    gtPtrVector() {};
    gtPtrVector(const gtPtrVector& other) : gtVector<ElementType, MemoryAllocator>(other) {};
    virtual ~gtPtrVector() { deleteElementsAndClear(); };

    // ---------------------------------------------------------------------------
    // Name:        gtPtrVector::deleteElementsAndClear
    // Description: Deletes all the elements pointed by this vector and clears the vector.
    // Author:      AMD Developer Tools Team
    // Date:        4/4/2005
    // ---------------------------------------------------------------------------
    void deleteElementsAndClear()
    {
        // Delete the elements pointed by this vector:
        typename gtVector<ElementType, MemoryAllocator>::iterator endIter = gtPtrVector::end();
        typename gtVector<ElementType, MemoryAllocator>::iterator iter = gtPtrVector::begin();

        while (iter != endIter)
        {
            delete *iter;
            iter++;
        }

        // Clear the vector:
        gtPtrVector::clear();
    };

    void removeItem(int index)
    {
        // Move the items above the removed item one place back:
        ElementType pItemData = (*this)[index];

        for (unsigned i = index; i < (*this).size() - 1; i++)
        {
            (*this)[i] = (*this)[i + 1];

        }

        // Release data of the removed item:
        delete pItemData;

        // Pop the top item:
        (*this).pop_back();
    }

    void push_front(const ElementType& el)
    {
        // Push the element back:
        push_back(el);

        // Move the item step by step until it gets to the front:
        for (unsigned i = this->size() - 1; i > 0 ; i--)
        {
            ElementType pTemp = (*this)[i];
            (*this)[i] = (*this)[i - 1];
            (*this)[i - 1] = pTemp;
        }
    }
};


#endif  // __GTPTRVECTOR
