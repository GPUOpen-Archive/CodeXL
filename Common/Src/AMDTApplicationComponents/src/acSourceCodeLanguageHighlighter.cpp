//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSourceCodeLanguageHighlighter.cpp
///
//==================================================================================

//------------------------------ acSourceCodeLanguageHighlighter.cpp ------------------------------

// QScintilla:
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercsharp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexerpython.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>

// Local:
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>
#include <AMDTApplicationComponents/Include/acSourceCodeLanguageHighlighter.h>
#include <AMDTApplicationComponents/Include/acSourceCodeLexers.h>


// Static members initializations:
acSourceCodeLanguageHighlighter* acSourceCodeLanguageHighlighter::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        acSourceCodeLanguageHighlighter::acSourceCodeLanguageHighlighter
// Description: Constructor.
// Arguments:   parent - My parent Qt widget
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
acSourceCodeLanguageHighlighter::acSourceCodeLanguageHighlighter()
{
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeLanguageHighlighter::~acSourceCodeLanguageHighlighter
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
acSourceCodeLanguageHighlighter::~acSourceCodeLanguageHighlighter()
{
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeLanguageHighlighter::instance
// Description: Returns the single instance of the gsExtensionsManager class
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
acSourceCodeLanguageHighlighter& acSourceCodeLanguageHighlighter::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new acSourceCodeLanguageHighlighter;
        GT_ASSERT(_pMySingleInstance);

        _pMySingleInstance->initLanguages();
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        acSourceCodeLanguageHighlighter::initLanguages
// Description: Initialize the view supported languages
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
void acSourceCodeLanguageHighlighter::initLanguages()
{
    // Add CPP language:
    QsciLexerCPP* pLexerCpp = new QsciLexerCPP;
    addLanguage(L"Cpp", L"cpp", pLexerCpp);

    // Add CPP language:
    QsciLexerCPP* pLexerHpp = new QsciLexerCPP;
    addLanguage(L"Hpp", L"hpp", pLexerHpp);

    // Add CL language:
    acQsciLexerCL* pLexerCL = new acQsciLexerCL;
    addLanguage(L"CL", L"cl", pLexerCL);

    // Add IL language:
    acQsciLexerIL* pLexerIL = new acQsciLexerIL;
    addLanguage(L"IL", L"cxlil", pLexerIL);

    // Add IL language:
    acQsciLexerISA* pLexerISA = new acQsciLexerISA;
    addLanguage(L"ISA", L"cxlisa", pLexerISA);

    // Add HLSL language:
    acQsciLexerHLSL* pLexerHLSL = new acQsciLexerHLSL;
    gtVector<gtString> langExtensionsVector;
    langExtensionsVector.push_back(L"vs");
    langExtensionsVector.push_back(L"hs");
    langExtensionsVector.push_back(L"ps");
    langExtensionsVector.push_back(L"gs");
    langExtensionsVector.push_back(L"ds");
    langExtensionsVector.push_back(L"cs");
    langExtensionsVector.push_back(L"hlsl");
    addLanguage(L"HLSL", langExtensionsVector, pLexerHLSL);


    // Add CS language:
    QsciLexerCSharp* pLexerCs = new QsciLexerCSharp;
    addLanguage(L"CS", L"cs", pLexerCs);

    // Add C language:
    QsciLexerCPP* pLexerC = new QsciLexerCPP;
    addLanguage(L"C", L"c", pLexerC);

    // Add CPP language:
    QsciLexerCPP* pLexerH = new QsciLexerCPP;
    addLanguage(L"H", L"h", pLexerH);

    // Add Css language:
    QsciLexerCSS* pLexerCss = new QsciLexerCSS;
    addLanguage(L"Css", L"css", pLexerCss);

    // Add Glsl language:
    QsciLexerCPP* pLexerGLSL = new QsciLexerCPP;
    gtVector<gtString> glslExtensionsVector;
    glslExtensionsVector.push_back(L"vert");
    glslExtensionsVector.push_back(L"tesc");
    glslExtensionsVector.push_back(L"tese");
    glslExtensionsVector.push_back(L"frag");
    glslExtensionsVector.push_back(L"comp");
    glslExtensionsVector.push_back(L"geom");
    glslExtensionsVector.push_back(L"glsl");
    glslExtensionsVector.push_back(L"vs");
    glslExtensionsVector.push_back(L"fs");
    glslExtensionsVector.push_back(L"cs");
    glslExtensionsVector.push_back(L"gs");
    addLanguage(L"GLSL", glslExtensionsVector, pLexerGLSL);

    // Add Java language:
    QsciLexerJava* pLexerJava = new QsciLexerJava;
    addLanguage(L"java", L"java", pLexerJava);

    // Add HTML language:
    QsciLexerHTML* pLexerHTML = new QsciLexerHTML;
    addLanguage(L"html", L"htm", pLexerHTML);

    gtVector<gtString> xmlExtensions;
    xmlExtensions.push_back(L"xml");
    xmlExtensions.push_back(L"gdb");
    xmlExtensions.push_back(L"gdbvs");
    xmlExtensions.push_back(L"bpt");
    addLanguage(L"xml", xmlExtensions, pLexerHTML);
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeLanguageHighlighter::addLanguage
// Description: Add a single language to the vector of supported languages
// Arguments:   const gtString& langName - the language display string
//              const gtString& langExtension - the language file extension
//              QsciLexer* pLexer - the lexer
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
void acSourceCodeLanguageHighlighter::addLanguage(const gtString& langName, const gtString& langExtension, QsciLexer* pLexer)
{
    gtVector<gtString> langExtensionsVector;
    langExtensionsVector.push_back(langExtension);
    addLanguage(langName, langExtensionsVector, pLexer);
}


// ---------------------------------------------------------------------------
// Name:        acSourceCodeLanguageHighlighter::addLanguage
// Description: Add a single language to the vector of supported languages
// Arguments:   const gtString& langName - the language display string
//              langExtensionsVector - the language file extensions vector
//              QsciLexer* pLexer - the lexer
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
void acSourceCodeLanguageHighlighter::addLanguage(const gtString& langName, const gtVector<gtString>& langExtensionsVector, QsciLexer* pLexer)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pLexer != NULL)
    {
        acSourceLanguage* pNewLang = new acSourceLanguage;

        // Set the language name:
        pNewLang->_languageName = langName;

        // Set the lexer:
        pNewLang->_pLanguageLexer = pLexer;

        // Set the file extension:
        for (int i = 0; i < (int)langExtensionsVector.size(); i++)
        {
            pNewLang->_languageFileExtensionsVector.push_back(langExtensionsVector[i]);
        }

        // Set the lexer font:
        QFont font;
        font.setFamily(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_FAMILY);
        font.setPointSize(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_SIZE);
        pLexer->setFont(font);
        pLexer->setDefaultFont(font);

        pLexer->setColor(QColor(0, 0, 255), QsciLexerCPP::PreProcessor);
        pLexer->setColor(QColor(0, 0, 255), QsciLexerCPP::Keyword);

        // Add the language to the vector:
        _languages.push_back(pNewLang);
    }
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeLanguageHighlighter::language
// Description: Return the language information for the requested index
// Arguments:   int index
// Return Val:  const acSourceLanguage*
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
const acSourceLanguage* acSourceCodeLanguageHighlighter::language(int index) const
{
    const acSourceLanguage* pRetVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT((index >= 0) && (index < (int)_languages.size()))
    {
        pRetVal = _languages[index];
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        acSourceCodeLanguageHighlighter::getLexerByExtension
// Description: Get the lexer handling a file with the requested extension
// Arguments:   const gtString& fileExtension
// Return Val:  QsciLexer*  -
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
QsciLexer* acSourceCodeLanguageHighlighter::getLexerByExtension(const gtString& fileExtension) const
{
    QsciLexer* pRetVal = NULL;

    // Go through the supported languages and search for the language with the requested extension:
    for (int i = 0 ; i < (int)_languages.size(); i++)
    {
        // Get the current language:
        acSourceLanguage* pLanguage = _languages[i];
        GT_IF_WITH_ASSERT(pLanguage != NULL)
        {
            gtVector<gtString>::const_iterator beginIter = pLanguage->_languageFileExtensionsVector.begin();
            gtVector<gtString>::const_iterator endIter = pLanguage->_languageFileExtensionsVector.end();

            if (gtFind(beginIter, endIter, fileExtension) != endIter)
            {
                // Return this lexer:
                pRetVal = pLanguage->_pLanguageLexer;
                break;
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        acSourceCodeLanguageHighlighter::getLanguageIndexByExtension
// Description: Get the language index  with the requested extension
// Arguments:   const gtString& fileExtension
// Return Val:  int  - index
// Author:      Bhattacharyya Koushik
// Date:        11/09/2012
// ---------------------------------------------------------------------------
int acSourceCodeLanguageHighlighter::getLanguageIndexByExtension(const gtString& fileExtension) const
{
    int pRetVal = -1;

    // Go through the supported languages and search for the language with the requested extension:
    for (int i = 0 ; i < (int)_languages.size(); i++)
    {
        // Get the current language:
        acSourceLanguage* pLanguage = _languages[i];
        GT_IF_WITH_ASSERT(pLanguage != NULL)
        {
            gtVector<gtString>::const_iterator beginIter = pLanguage->_languageFileExtensionsVector.begin();
            gtVector<gtString>::const_iterator endIter = pLanguage->_languageFileExtensionsVector.end();

            if (gtFind(beginIter, endIter, fileExtension) != endIter)
            {
                // Return this index:
                pRetVal = i;
                break;
            }
        }
    }

    return pRetVal;
}
