//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apExpression.cpp
///
//==================================================================================

//------------------------------ apExpression.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>
#include <AMDTOSWrappers/Include/osRawMemoryStream.h>

// Local:
#include <AMDTAPIClasses/Include/apExpression.h>

// ---------------------------------------------------------------------------
// Name:        apExpression::apExpression
// Description: Default Constructor.
// Author:      AMD Developer Tools Team
// Date:        9/6/2016
// ---------------------------------------------------------------------------
apExpression::apExpression()
{

}

// ---------------------------------------------------------------------------
// Name:        apExpression::apExpression
// Description: Copy Constructor.
// Author:      AMD Developer Tools Team
// Date:        9/6/2016
// ---------------------------------------------------------------------------
apExpression::apExpression(const apExpression& other)
{
    operator=(other);
}

// ---------------------------------------------------------------------------
// Name:        apExpression::apExpression
// Description: Move Constructor.
// Author:      AMD Developer Tools Team
// Date:        9/6/2016
// ---------------------------------------------------------------------------
apExpression::apExpression(apExpression&& other)
{
    operator=(other);
}

// ---------------------------------------------------------------------------
// Name:        apExpression::operator=
// Description: Copy Operator.
// Author:      AMD Developer Tools Team
// Date:        9/6/2016
// ---------------------------------------------------------------------------
apExpression& apExpression::operator=(const apExpression& other)
{
    // Copy scalar members:
    m_name = other.m_name;
    m_value = other.m_value;
    m_valueHex = other.m_valueHex;
    m_type = other.m_type;

    // Recursively copy children vector:
    m_children.deleteElementsAndClear();
    for (const apExpression* pChld : other.m_children)
    {
        apExpression* pNewChild = addChild();
        *pNewChild = (const apExpression&)(*pChld);
    }

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apExpression::operator=
// Description: Move Operator.
// Author:      AMD Developer Tools Team
// Date:        9/6/2016
// ---------------------------------------------------------------------------
apExpression& apExpression::operator=(apExpression&& other)
{
    // Copy scalar members:
    m_name = other.m_name;
    m_value = other.m_value;
    m_valueHex = other.m_valueHex;
    m_type = other.m_type;

    // Recursively move children vector:
    m_children.deleteElementsAndClear();
    for (apExpression*& pChld : other.m_children)
    {
        apExpression* pNewChild = addChild();
        *pNewChild = (apExpression&&)(*pChld);

        // Clear the member from the old vector:
        pChld = nullptr;
    }

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apExpression::~apExpression
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        9/6/2016
// ---------------------------------------------------------------------------
apExpression::~apExpression()
{

}

// ---------------------------------------------------------------------------
// Name:        apExpression::type
// Description: Returns my transferable object type.
// Author:      AMD Developer Tools Team
// Date:        9/6/2016
// ---------------------------------------------------------------------------
osTransferableObjectType apExpression::type() const
{
    return OS_TOBJ_ID_EXPRESSION;
}

// ---------------------------------------------------------------------------
// Name:        apExpression::writeSelfIntoChannel
// Description: Writes this object into an IPC Channel.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/6/2016
// ---------------------------------------------------------------------------
bool apExpression::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write scalar members:
    ipcChannel << m_name;
    ipcChannel << m_value;
    ipcChannel << m_valueHex;
    ipcChannel << m_type;

    // Recursively write children:
    gtUInt32 childrenCount = (gtUInt32)m_children.size();
    ipcChannel << childrenCount;
    for (gtUInt32 i = 0; i < childrenCount; ++i)
    {
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
        // This validation would slow down the operation, so only perform it in debug builds:
        GT_ASSERT(nullptr != m_children[i]);
#endif
        bool rcChld = m_children[i]->writeSelfIntoChannel(ipcChannel);
        GT_ASSERT(rcChld);
        retVal = retVal && rcChld;
    }

    return retVal;

/*
    // TO_DO: this optimization seems to cause some trouble...

    // Since this expression may have many descendants, we will first write it into
    // a raw memory stream, and write the stream into the channel.
    osRawMemoryStream expressionStream;

    bool retVal = writeSelfIntoRawMemoryStream(expressionStream);

    GT_IF_WITH_ASSERT(retVal)
    {
        ipcChannel << expressionStream;
    }

    return retVal;
    */
}

// ---------------------------------------------------------------------------
// Name:        apExpression::writeSelfIntoChannel
// Description: Writes this object into an osRawMemoryStream Channel.
//              This is used as an optimization to avoid creating 100s of transactions
//              When transferring expression data to the client.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/6/2016
// ---------------------------------------------------------------------------
bool apExpression::writeSelfIntoRawMemoryStream(osRawMemoryStream& ipcChannel) const
{
    bool retVal = true;

    // Write scalar members:
    ipcChannel << m_name;
    ipcChannel << m_value;
    ipcChannel << m_valueHex;
    ipcChannel << m_type;

    // Recursively write children:
    gtUInt32 childrenCount = (gtUInt32)m_children.size();
    ipcChannel << childrenCount;
    for (gtUInt32 i = 0; i < childrenCount; ++i)
    {
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
        // This validation would slow down the operation, so only perform it in debug builds:
        GT_ASSERT(nullptr != m_children[i]);
#endif
        bool rcChld = m_children[i]->writeSelfIntoRawMemoryStream(ipcChannel);
        GT_ASSERT(rcChld);
        retVal = retVal && rcChld;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apExpression::readSelfFromChannel
// Description: Reads this object from an IPC Channel.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/6/2016
// ---------------------------------------------------------------------------
bool apExpression::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Write scalar members:
    ipcChannel >> m_name;
    ipcChannel >> m_value;
    ipcChannel >> m_valueHex;
    ipcChannel >> m_type;

    // Recursively write children:
    gtUInt32 childrenCount = 0;
    ipcChannel >> childrenCount;
    m_children.reserve(childrenCount);

    for (gtUInt32 i = 0; i < childrenCount; ++i)
    {
        apExpression* pNewChild = addChild();
        GT_ASSERT(nullptr != pNewChild);

        bool rcChld = pNewChild->readSelfFromChannel(ipcChannel);
        GT_ASSERT(rcChld);
        retVal = retVal && rcChld;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apExpression::addChild
// Description: Adds a new child to this object and returns a pointer to it
// Author:      AMD Developer Tools Team
// Date:        9/6/2016
// ---------------------------------------------------------------------------
apExpression* apExpression::addChild()
{
    apExpression* retVal = new apExpression;

    GT_IF_WITH_ASSERT(nullptr != retVal)
    {
        m_children.push_back(retVal);
    }

    return retVal;
}

