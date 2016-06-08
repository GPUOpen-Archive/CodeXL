//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acFindParameters.cpp
///
//==================================================================================

//------------------------------ acFindParameters.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acFindParameters.h>

acFindParameters* acFindParameters::m_spMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
acFindParameters::acFindParameters()
    : m_findExpr(""),
      m_isCaseSensitive(false),
      m_isSearchUp(false),
      m_findFirstLine(-1),
      m_lastResult(false),
      m_findFromStart(false),
      m_shouldRespondToTextChange(true)
{

}

// ---------------------------------------------------------------------------
acFindParameters::acFindParameters(const acFindParameters& other)
    : m_findExpr(other.m_findExpr),
      m_isCaseSensitive(other.m_isCaseSensitive),
      m_isSearchUp(other.m_isSearchUp),
      // Notice: Do not  the find first line:
      // m_findFirstLine(other.m_findFirstLine),
      m_lastResult(other.m_lastResult),
      m_findFromStart(other.m_findFromStart),
      m_shouldRespondToTextChange(other.m_shouldRespondToTextChange)
{

}

// ---------------------------------------------------------------------------
acFindParameters& acFindParameters::operator=(const acFindParameters& other)
{
    m_findExpr = other.m_findExpr;
    m_isCaseSensitive = other.m_isCaseSensitive;
    m_isSearchUp = other.m_isSearchUp;
    // Notice: Do not  the find first line:
    // m_findFirstLine(other.m_findFirstLine),
    m_lastResult = other.m_lastResult;
    m_findFromStart = other.m_findFromStart;
    m_shouldRespondToTextChange = other.m_shouldRespondToTextChange;
    return *this;
}

acFindParameters& acFindParameters::Instance()
{
    if (nullptr == m_spMySingleInstance)
    {
        m_spMySingleInstance = new acFindParameters;
    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
void acFindParameters::Clear()
{
    m_findExpr = "";
    m_isCaseSensitive = false;
    m_isSearchUp = false;
    m_findFirstLine = -1;
    m_lastResult = false;
    m_findFromStart = false;
    m_shouldRespondToTextChange = true;
}

// ---------------------------------------------------------------------------
