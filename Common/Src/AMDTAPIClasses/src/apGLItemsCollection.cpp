//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLItemsCollection.cpp
///
//==================================================================================

//------------------------------ apGLItemsCollection.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTAPIClasses/Include/apGLItemsCollection.h>


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::apGLItemsCollection
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        28/5/2005
// ---------------------------------------------------------------------------
apGLItemsCollection::apGLItemsCollection()
{
}


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::apGLItemsCollection
// Description: Copy constructor.
// Arguments:   other - The other items collection from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        30/5/2005
// ---------------------------------------------------------------------------
apGLItemsCollection::apGLItemsCollection(const apGLItemsCollection& other)
{
    // Copy the other collection vectors:
    copyVectors(other);
}


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::~apGLItemsCollection
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        28/5/2005
// ---------------------------------------------------------------------------
apGLItemsCollection::~apGLItemsCollection()
{
    // Clear the class vectors:
    clearVectors();
}


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::operator=
// Description: Assignment operator.
// Arguments:   other - The other items collection from which I am copied.
// Return Val:  apGLItemsCollection& - Reference to self.
// Author:  AMD Developer Tools Team
// Date:        30/5/2005
// ---------------------------------------------------------------------------
apGLItemsCollection& apGLItemsCollection::operator=(const apGLItemsCollection& other)
{
    // Copy the other collection vectors:
    copyVectors(other);

    // Return a reference to self:
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::addItem
// Description: Adds an item into the collection.
// Arguments:   itemName - The item name.
//              itemType - The item OpenGL type.
//              itemSize - The item size (>1 if this is an array).
//              itemBlockIndex - the item block index (UBO)
// Return Val:  int - The item index in the collection.
// Author:  AMD Developer Tools Team
// Date:        29/5/2005
// ---------------------------------------------------------------------------
int apGLItemsCollection::addItem(const gtString& itemName, GLenum itemType, GLint itemSize, GLint itemLocation, GLint itemBlockIndex)
{
    // Get the item index:
    int retVal = amountOfItems();

    // Add the item into the collection:
    apGLCollectedItem* pNewItem = new apGLCollectedItem;
    pNewItem->m_name = itemName;
    pNewItem->m_type = itemType;
    pNewItem->m_size = itemSize;
    pNewItem->m_location = itemLocation;
    pNewItem->m_value = new apNotAvailableParameter;
    pNewItem->m_blockIndex = itemBlockIndex;

    m_items.push_back(pNewItem);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::setItemValue
// Description: Sets an item value
// Arguments:   itemIndex - The item index in the collection.
//              aptrValue - The item's new value.
// Author:  AMD Developer Tools Team
// Date:        29/5/2005
// ---------------------------------------------------------------------------
void apGLItemsCollection::setItemValue(int itemIndex, gtAutoPtr<apParameter>& aptrValue)
{
    // index range test:
    int itemsAmount = amountOfItems();

    if ((0 <= itemIndex) && (itemIndex < itemsAmount))
    {
        const apParameter* pParam = aptrValue.pointedObject();

        if (nullptr != pParam)
        {
            m_items[itemIndex]->m_value = aptrValue;
        }
        else
        {
            m_items[itemIndex]->m_value = new apNotAvailableParameter;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::itemIndex
// Description: Inputs an item name and returns its index in this class
//              (or -1 if it was not found)
// Author:  AMD Developer Tools Team
// Date:        29/11/2005
// ---------------------------------------------------------------------------
int apGLItemsCollection::itemIndex(const gtString& itemName) const
{
    int retVal = -1;

    // Iterate the available items:
    int itemsAmount = amountOfItems();

    for (int i = 0; i < itemsAmount; i++)
    {
        // If the current item is the item we are looking for:
        if (m_items[i]->m_name == itemName)
        {
            // Return the item index and exit the loop:
            retVal = i;
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::itemIndex
// Description: Inputs an item name and returns its index in this class
//              (or -1 if it was not found)
// Author:  AMD Developer Tools Team
// Date:        1/11/2009
// ---------------------------------------------------------------------------
int apGLItemsCollection::itemIndex(GLint itemBlockIndex) const
{
    int retVal = -1;

    // Iterate the available items:
    int itemsAmount = amountOfItems();

    for (int i = 0; i < itemsAmount; i++)
    {
        // If the current item is the item we are looking for:
        if (m_items[i]->m_blockIndex == itemBlockIndex)
        {
            // Return the item index and exit the loop:
            retVal = i;
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        28/5/2005
// ---------------------------------------------------------------------------
osTransferableObjectType apGLItemsCollection::type() const
{
    return OS_TOBJ_ID_GL_ITEMS_COLLECTION;
}


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::writeSelfIntoChannel
// Description: Writes self into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/5/2005
// ---------------------------------------------------------------------------
bool apGLItemsCollection::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the amount of items:
    gtInt32 itemsAmount = (gtInt32)amountOfItems();
    ipcChannel << itemsAmount;

    // Write the items:
    for (int i = 0; i < itemsAmount; i++)
    {
        const apGLCollectedItem* pCurrentItem = m_items[i];
        GT_ASSERT(nullptr != pCurrentItem);

        // Write the item name:
        ipcChannel << pCurrentItem->m_name;

        // Write the item type:
        ipcChannel << (gtInt32)pCurrentItem->m_type;

        // Write the item size:
        ipcChannel << (gtInt32)pCurrentItem->m_size;

        // Write the item location:
        ipcChannel << (gtInt32)pCurrentItem->m_location;

        // Write the item value:
        const apParameter* pValue = pCurrentItem->m_value.pointedObject();
        bool validValue = (nullptr != pValue);
        ipcChannel << validValue;

        if (validValue)
        {
            ipcChannel << *(pCurrentItem->m_value);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::readSelfFromChannel
// Description: Reads self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        28/5/2005
// ---------------------------------------------------------------------------
bool apGLItemsCollection::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Clear this class vectors:
    clearVectors();

    // Read the amount of items:
    gtInt32 itemsAmount = 0;
    ipcChannel >> itemsAmount;
    GT_ASSERT(itemsAmount >= 0);

    // Read the items:
    for (int i = 0; i < itemsAmount; i++)
    {
        apGLCollectedItem* pNewItem = new apGLCollectedItem;

        // Read the current item name:
        gtString curItemName;
        ipcChannel >> curItemName;
        pNewItem->m_name = curItemName;

        // Read the current item type:
        gtInt32 currentItemTypeAsInt32 = GL_INT;
        ipcChannel >> currentItemTypeAsInt32;
        GLenum currentItemType = (GLenum)currentItemTypeAsInt32;
        pNewItem->m_type = currentItemType;

        // Read the current item size:
        GLint itemSizeAsInt32 = 0;
        ipcChannel >> itemSizeAsInt32;
        GLint itemSize = (GLint)itemSizeAsInt32;
        pNewItem->m_size = itemSize;

        // Read the current item Location:
        GLint itemLocationAsInt32 = -2;
        ipcChannel >> itemLocationAsInt32;
        GLint itemLocation = (GLint)itemLocationAsInt32;
        pNewItem->m_location = itemLocation;

        bool validValue = false;
        ipcChannel >> validValue;

        if (validValue)
        {
            // Read the current item value (as transferable object):
            gtAutoPtr<osTransferableObject> aptrReadTransferableObj;
            ipcChannel >> aptrReadTransferableObj;

            // Verify that we read a parameter object:
            if (aptrReadTransferableObj->isParameterObject())
            {
                apParameter* pItemValue = (apParameter*)(aptrReadTransferableObj.releasePointedObjectOwnership());
                pNewItem->m_value = pItemValue;
            }
            else
            {
                pNewItem->m_value = nullptr;
                retVal = false;
            }
        }
        else
        {
            pNewItem->m_value = nullptr;
        }

        m_items.push_back(pNewItem);
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::clearVectors
// Description: Clears this class vectors.
// Author:  AMD Developer Tools Team
// Date:        30/5/2005
// ---------------------------------------------------------------------------
void apGLItemsCollection::clearVectors()
{
    m_items.deleteElementsAndClear();
}


// ---------------------------------------------------------------------------
// Name:        apGLItemsCollection::copyVectors
// Description: Copy this class vectors from another class instance.
// Arguments:   other - The other class instance.
// Author:  AMD Developer Tools Team
// Date:        30/5/2005
// ---------------------------------------------------------------------------
void apGLItemsCollection::copyVectors(const apGLItemsCollection& other)
{
    // Clear the class vectors:
    clearVectors();

    // Iterate the collection items:
    int amountOfOtherItems = other.amountOfItems();

    for (int i = 0; i < amountOfOtherItems; i++)
    {
        apGLCollectedItem* pNewItem = new apGLCollectedItem;

        // Copy the item name:
        const gtString& curItemName = other.itemName(i);
        pNewItem->m_name = curItemName;

        // Copy the item type:
        GLenum curItemType = other.itemType(i);
        pNewItem->m_type = curItemType;

        // Copy the item size:
        GLint curItemSize = other.itemSize(i);
        pNewItem->m_size = curItemSize;

        // Copy the item size:
        GLint curItemLocation = other.itemLocation(i);
        pNewItem->m_location = curItemLocation;

        // Copy the item value:
        const apParameter* pCurItemValue = other.itemValue(i);

        if (nullptr != pCurItemValue)
        {
            apParameter* pCurItemValueCopy = (apParameter*)(pCurItemValue->clone());
            pNewItem->m_value = pCurItemValueCopy;
        }
        else
        {
            pNewItem->m_value = nullptr;
        }

        // Copy the item block index:
        GLint curBlockIndex = other.itemBlockIndex(i);
        pNewItem->m_blockIndex = curBlockIndex;

        m_items.push_back(pNewItem);
    }
}

