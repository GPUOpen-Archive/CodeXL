//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apExpression.h
///
//==================================================================================

//------------------------------ apExpression.h ------------------------------

#ifndef __APEXPRESSION_H
#define __APEXPRESSION_H

// Forward declarations:
class osRawMemoryStream;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

struct AP_API apExpression : public osTransferableObject
{
public:
    apExpression();
    apExpression(const apExpression& other);
    apExpression(apExpression&& other);
    apExpression& operator=(const apExpression& other);
    apExpression& operator=(apExpression&& other);
    ~apExpression();

    // Overrides via osTransferableObject
    virtual osTransferableObjectType type() const override;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const override;
    bool writeSelfIntoRawMemoryStream(osRawMemoryStream& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel) override;

    // Children access:
    apExpression* addChild();
    const gtVector<apExpression*>& children() const { return m_children; };

private:

public:
    gtString m_name;
    gtString m_value;
    gtString m_valueHex;
    gtString m_type;

private:
    gtPtrVector<apExpression*> m_children;
};

#endif //__APEXPRESSION_H

