//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSourceCodeLanguageHighlighter.h
///
//==================================================================================

//------------------------------ acSourceCodeLanguageHighlighter.h ------------------------------

#ifndef __ACSOURCECODELANGUAGEHIGHLIGHTER__H
#define __ACSOURCECODELANGUAGEHIGHLIGHTER__H

// QScintilla:
#define QSCINTILLA_DLL
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexercpp.h>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class acSourceLanguage
{
public:

    acSourceLanguage(): _pLanguageLexer(NULL), _languageName(L"") {}
    QsciLexer* _pLanguageLexer;
    gtString _languageName;
    gtVector<gtString> _languageFileExtensionsVector;

};
// ----------------------------------------------------------------------------------
// Class Name:          AC_API acSourceCodeLanguageHighlighter: public QsciScintilla
// General Description: A single instance class that is responsible for language highlighting
//                      service for our source code view
// Author:              Sigal Algranaty
// Creation Date:       9/8/2011
// ----------------------------------------------------------------------------------
class AC_API acSourceCodeLanguageHighlighter
{
    friend class acSingeltonsDelete;

public:

    static acSourceCodeLanguageHighlighter& instance();
    virtual ~acSourceCodeLanguageHighlighter();

    // Languages initialization utilities:
    void initLanguages();
    void addLanguage(const gtString& langName, const gtString& langExtension, QsciLexer* pLexer);
    void addLanguage(const gtString& langName, const gtVector<gtString>& langExtensionsVector, QsciLexer* pLexer);

    // Return the supported languages:
    const gtVector<acSourceLanguage*>& languages() const {return _languages;} ;
    const acSourceLanguage* language(int index) const ;
    QsciLexer* getLexerByExtension(const gtString& fileExtension) const;
    int getLanguageIndexByExtension(const gtString& fileExtension) const;

private:

    // Do not allow the use of my default constructor:
    acSourceCodeLanguageHighlighter();
private:

    static acSourceCodeLanguageHighlighter* _pMySingleInstance;

    // Languages:
    gtVector<acSourceLanguage*> _languages;

};


#endif  // __ACSOURCECODELANGUAGEHIGHLIGHTER__H
