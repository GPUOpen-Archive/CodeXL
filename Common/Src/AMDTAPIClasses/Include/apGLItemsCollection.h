//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLItemsCollection.h
///
//==================================================================================

//------------------------------ apGLItemsCollection.h ------------------------------

#ifndef __APGLITEMSCOLLECTION
#define __APGLITEMSCOLLECTION

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTAPIClasses/Include/apParameters.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLItemsCollection : public osTransferableObject
// General Description:
//   Represents a collection of items. Each item has:
//   - Name - The item name.
//   - Type - The item type (an OpenGL base type).
//   - Size - The item size (>1 if this is an array).
//   - Value - The item value.
//
// Author:  AMD Developer Tools Team
// Creation Date:        04/04/2005
// ----------------------------------------------------------------------------------
class AP_API apGLItemsCollection : public osTransferableObject
{
public:
    apGLItemsCollection();
    apGLItemsCollection(const apGLItemsCollection& other);
    virtual ~apGLItemsCollection();

    void clear() { clearVectors(); };
    apGLItemsCollection& operator=(const apGLItemsCollection& other);

    // Set item methods:
    int addItem(const gtString& itemName, GLenum itemType, GLint itemSize, GLint itemLocation, GLint itemBlockIndex);
    void setItemValue(int itemIndex, gtAutoPtr<apParameter>& aptrValue);
    void setItemLocation(int index, GLint itemLocation) {m_items[index]->m_location = itemLocation;};

    // Get item methods:
    int amountOfItems() const { return (int)m_items.size(); };
    const gtString& itemName(int index) const { return m_items[index]->m_name; };
    GLenum itemType(int index) const { return m_items[index]->m_type; };
    GLint itemSize(int index) const { return m_items[index]->m_size; };
    GLint itemLocation(int index) const { return m_items[index]->m_location; };
    const apParameter* itemValue(int index) const { return m_items[index]->m_value.pointedObject(); };
    GLint itemBlockIndex(int index) const { return m_items[index]->m_blockIndex; };

    int itemIndex(const gtString& itemName) const;
    int itemIndex(GLint itemBlockIndex) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    void clearVectors();
    void copyVectors(const apGLItemsCollection& other);

private:
    struct apGLCollectedItem
    {
        apGLCollectedItem() :
            m_type(GL_NONE), m_size(0), m_location(-1), m_blockIndex(-1) {};
        apGLCollectedItem(const apGLCollectedItem&) = delete;
        apGLCollectedItem& operator=(const apGLCollectedItem&) = delete;

        gtString m_name;
        GLenum m_type;
        GLint m_size;
        GLint m_location;
        gtAutoPtr<apParameter> m_value;
        GLint m_blockIndex;
    };

    gtPtrVector<apGLCollectedItem*> m_items;
};


#endif  // __APGLITEMSCOLLECTION
