//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSourceCodeLexers.h
///
//==================================================================================

//------------------------------ acSourceCodeLexers.h ------------------------------

#ifndef __ACSOURCECODELEXERS_H
#define __ACSOURCECODELEXERS_H

// QScintilla:
#define QSCINTILLA_DLL
#include <Qsci/qscilexercpp.h>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class AC_API acQsciLexerCL : public QsciLexerCPP
{
public:
    acQsciLexerCL();
    virtual ~acQsciLexerCL();

    // Overrides QsciLexerCPP:
    virtual const char* language() const;
    virtual const char* keywords(int set) const;
    virtual QColor defaultColor(int style) const;

    // A cache of the keyword strings:
    gtVector<gtASCIIString> m_keywordStrings;
};

class AC_API acQsciLexerIL : public QsciLexerCPP
{
public:
    acQsciLexerIL();
    virtual ~acQsciLexerIL();

    // Overrides QsciLexerCPP:
    virtual const char* language() const;
    virtual const char* keywords(int set) const;
    virtual QColor defaultColor(int style) const;

    // A cache of the keyword strings:
    gtVector<gtASCIIString> m_keywordStrings;
};

class AC_API acQsciLexerISA : public QsciLexerCPP
{
public:
    acQsciLexerISA();
    virtual ~acQsciLexerISA();

    // Overrides QsciLexerCPP:
    virtual const char* language() const;
    virtual const char* keywords(int set) const;
    virtual QColor defaultColor(int style) const;

    // A cache of the keyword strings:
    gtVector<gtASCIIString> m_keywordStrings;
};

class AC_API acQsciLexerHLSL : public QsciLexerCPP
{
public:
    acQsciLexerHLSL();
    virtual ~acQsciLexerHLSL();

    // Overrides QsciLexerCPP:
    virtual const char* language() const;
    virtual const char* keywords(int set) const;
    virtual QColor defaultColor(int style) const;

    // A cache of the keyword strings:
    gtVector<gtASCIIString> m_keywordStrings;
};

#endif //__ACSOURCECODELEXERS_H

