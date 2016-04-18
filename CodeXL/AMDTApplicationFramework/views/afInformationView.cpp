//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afInformationView.cpp
///
//==================================================================================

// Qt:
#include <QtWidgets>

// Qscintilla:
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscidocument.h>
#include <Qsci/qsciscintillabase.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTAPIClasses/Include/Events/apOutputDebugStringEvent.h>
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// Local:
#include <AMDTApplicationFramework/Include/views/afInformationView.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>


// ---------------------------------------------------------------------------
// Name:        afInformationView::afInformationView
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
afInformationView::afInformationView(QWidget* pParent) : QWidget(pParent), afBaseView(&afProgressBarWrapper::instance()), m_pMainLayout(nullptr), m_pEditor(nullptr)
{
    m_pMainLayout = new QVBoxLayout(this);

    m_pEditor = new afSourceCodeView(this, false, 0x14);
    m_pEditor->setReadOnly(true);
    m_pEditor->setWrapMode(QsciScintilla::WrapWord);

    m_pEditor->setReadOnly(true);
    m_pEditor->setMarginWidth(0, 0);
    m_pEditor->setMarginWidth(1, 0);

    m_pEditor->SetMonoFont(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_SIZE);

    m_pMainLayout->addWidget(m_pEditor);
    m_pMainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_pMainLayout);

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    bool rc = connect(m_pEditor, SIGNAL(textLineDoubleClicked(const QStringList&, const int)), this, SLOT(textLineReceived(const QStringList&, const int)));
    GT_ASSERT(rc);
}


// ---------------------------------------------------------------------------
// Name:        afInformationView::~afInformationView
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
afInformationView::~afInformationView()
{

}

// ---------------------------------------------------------------------------
// Name:        afInformationView::clear
// Description: Clear the view:
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
void afInformationView::clear()
{
    GT_IF_WITH_ASSERT(nullptr != m_pEditor)
    {
        m_pEditor->clear();
        m_pEditor->setWrapMode(QsciScintilla::WrapNone);
    }
}

void afInformationView::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Build the string and add to the list view:
    gtString eventAsString;

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // Down cast the event according to its type, and call the appropriate
    // event handling function:
    switch (eventType)
    {
        case apEvent::AP_OUTPUT_DEBUG_STRING:
        {
            const apOutputDebugStringEvent& event = ((const apOutputDebugStringEvent&)eve);

            if (event.targetView() == apOutputDebugStringEvent::AP_GENERAL_OUTPUT_VIEW)
            {
                gtString message = event.debugString();
                QString qtMessage = acGTStringToQString(message);

                appendToOutputWindow(qtMessage);
            }
        }
        break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        afInformationView::addline
// Description: Add line
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
void afInformationView::appendToOutputWindow(const QString& infoLine)
{
    GT_IF_WITH_ASSERT(nullptr != m_pEditor)
    {
        QScrollBar* pVerticalBar =  m_pEditor->verticalScrollBar();
        int sliderPos   = pVerticalBar->sliderPosition();
        bool isAtBottom = (sliderPos == pVerticalBar->maximum());

        m_pEditor->append(infoLine);

        // Get the full text that is displayed and add it to the based object name of the editor:
        QString textString = m_pEditor->text();
        QString textName = "Output:" + textString;
        m_pEditor->setObjectName(textName);

        // Move the slider of the vertical scrollbar to the bottom to expose the
        // new text if the slider is at max position before appending.  This logic
        // will keep the slider where they were before appending.
        if (true == isAtBottom)
        {
            pVerticalBar->triggerAction(QAbstractSlider::SliderToMaximum);
        }
    }
}

// ---------------------------------------------------------------------------
void afInformationView::clearOutputWindow()
{
    clear();
}

void afInformationView::onUpdateEdit_Copy(bool& isEnabled)
{
    GT_IF_WITH_ASSERT(m_pEditor != nullptr)
    {
        isEnabled = m_pEditor->hasSelectedText();
    }
}

void afInformationView::onUpdateEdit_SelectAll(bool& isEnabled)
{
    isEnabled = true;
}

void afInformationView::onUpdateEdit_Find(bool& isEnabled)
{
    isEnabled = false;
}

void afInformationView::onUpdateEdit_FindNext(bool& isEnabled)
{
    isEnabled = false;
}

void afInformationView::onEdit_Copy()
{
    GT_IF_WITH_ASSERT(m_pEditor != nullptr)
    {
        m_pEditor->copy();
    }
}

void afInformationView::onEdit_SelectAll()
{
    GT_IF_WITH_ASSERT(m_pEditor != nullptr)
    {
        m_pEditor->selectAll();
    }
}

void afInformationView::textLineReceived(const QStringList& fullTextAsLines, const int clickedLinePosition)
{
    // These error lines should have fixed format forced in beProgramBuilderOpenCL::PostProcessOpenCLBuildLog()
    // This function handles textLine of the following format:
    // PATH_TO_FILE\file.cl, line 123:message
    // or if its a HLSL file PATH_TO_FILE\file.hlsl(123,1-2): message

    // looking for line 123: so the line number can be isolated
    QRegExp cllineNumExp(QString("[lL]ine [0-9]+:"));
    // for directX files, looking for (123, in order to isolate line number
    QRegExp hlsllineNumExp("(\\([\\d]+,)");
    // for openGL files, looking for :digits:
    QRegExp glsllineNumExp(":[\\d]+:");
    bool lineNumOk = false;
    QString lineNumText;
    QStringList matched;
    int lineNum = -1;
    QString textLine = fullTextAsLines.value(clickedLinePosition);

    // Check if the line has line number
    if (0 <= cllineNumExp.indexIn(textLine))
    {
        matched = cllineNumExp.capturedTexts();
        lineNumText = matched[0];
    }
    else if (0 <= hlsllineNumExp.indexIn(textLine))
    {
        matched = hlsllineNumExp.capturedTexts();
        lineNumText = matched[0];
    }
    else if (0 <= glsllineNumExp.indexIn(textLine))
    {
        matched = glsllineNumExp.capturedTexts();
        lineNumText = matched[0].mid(1, matched[0].size() - 2);
    }

    // Extract line number
    QRegExp numExp(QString("[0-9]+"));

    if (!lineNumText.isEmpty() && 0 <= numExp.indexIn(lineNumText))
    {
        matched = numExp.capturedTexts();
        lineNumText = matched[0];
        lineNum = lineNumText.toInt(&lineNumOk);
    }

    // if line number was found
    if (lineNumOk)
    {
        QRegExp clFilePathAndNamExp(QString(AF_STR_FILE_PATHEXTENSIONS_QREGEXP), Qt::CaseInsensitive);

        // Check if the line has file path and name.  This is usually a file name in
        // #include directive.
        if (0 <= clFilePathAndNamExp.indexIn(textLine))
        {
            matched = clFilePathAndNamExp.capturedTexts();
            QString filePathAndName = matched[0];
            osFilePath filePath(acQStringToGTString(filePathAndName));

            if (filePath.exists())
            {
                bool ok = afApplicationCommands::instance()->OpenFileAtLine(filePath, lineNum, 0);
                GT_ASSERT(ok);
            }
        }

        else
        {
            //Error line doesn't contain file name - could be Vulkan or OpenGL pipeline shader
            GLSLOutputLineParser(lineNum, clickedLinePosition, fullTextAsLines);
        }


    }
}


void afInformationView::GLSLOutputLineParser(const int lineNum, const int clickedLinePosition, const QStringList& fullTextAsLines) const
{
    //find shader type
    QString failedStage = GetFailedGLSLShaderStageName(clickedLinePosition, fullTextAsLines);

    if (failedStage.isEmpty() == false)
    {
        const auto stageToFilePathMap = BuildStageToFileMap(fullTextAsLines);

        for (const auto& itr : stageToFilePathMap)
        {
            //found file
            if (itr.first.contains(failedStage, Qt::CaseSensitivity::CaseInsensitive))
            {
                const osFilePath filePath(acQStringToGTString(itr.second));

                if (filePath.exists())
                {
                    bool ok = afApplicationCommands::instance()->OpenFileAtLine(filePath, lineNum, 0);
                    GT_ASSERT(ok);
                    break;
                }
            }
        }
    }
}
/// for GL program output find file paths in output header
gtMap<QString, QString> afInformationView::BuildStageToFileMap(const QStringList& fullTextAsLines) const
{
    gtMap<QString, QString> stageToFilePathMap;
    bool found = false;

    ///build  map stage to filename
    for (const QString& line : fullTextAsLines)
    {
        if (line.startsWith(AF_STR_BUILDING_STAGE))
        {
            const QStringList tokens = line.split(':');

            if (tokens.size() > 2)
            {
                QString  stageName = tokens[1].split(',')[0].trimmed();
                static const QString fileSeparator = AF_STR_FILENAME_SEPARATOR;
                int filePathIx = line.lastIndexOf(fileSeparator);
                QString filepath = line.mid(filePathIx + fileSeparator.size()).trimmed();
                stageToFilePathMap[stageName] = filepath;
                found = true;
            }
        }
        //we already found every line with filepath
        else if (found)
        {
            break;
        }
    }

    return stageToFilePathMap;
}

bool afInformationView::ObtainStageName(const QString& outputLine, QString& failedStage, const QString& typeIndicator, const QString& secondWord)const
{
    bool retVal = false;

    if (outputLine.contains(typeIndicator))
    {
        QStringList lineTokens = outputLine.split(' ', QString::SkipEmptyParts);

        if (lineTokens.size() > 2)
        {
            failedStage = lineTokens[1].trimmed();
            const QString stageSecondWord = lineTokens[2].trimmed();

            if (stageSecondWord.compare(secondWord, Qt::CaseInsensitive) != 0)
            {
                failedStage += " " + stageSecondWord;
            }

            retVal = true;
        }
    }

    return retVal;
}

QString afInformationView::GetFailedGLSLShaderStageName(const int clickedLinePosition, const QStringList& fullTextAsLines) const
{
    QString failedStage;

    for (int i = clickedLinePosition - 1; i >= 0; --i)
    {
        const QString line = fullTextAsLines[i];
        // try OpenGL
        bool stageNameObtained = ObtainStageName(line, failedStage, AF_STR_OpenGL_Shader_Failed, AF_STR_SHADER);

        if (!stageNameObtained)
        {
            //try Vulkan
            stageNameObtained = ObtainStageName(line, failedStage, AF_STR_COMPILING, AF_STR_STAGE);
        }

        if (stageNameObtained)
        {
            break;
        }
    }

    return failedStage;
}