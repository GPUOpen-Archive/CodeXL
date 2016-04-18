//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afInformationView.h
///
//==================================================================================

#ifndef __AFINFORMATIONVIEW_H
#define __AFINFORMATIONVIEW_H

// Qt:
#include <QtWidgets>

// Local:
#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTBaseTools/Include/gtMap.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

class QsciScintilla;

// ----------------------------------------------------------------------------------
// Class Name:          KA_API afInformationView : QWidget
// General Description: Dockable view to display the output of the Static analyzer
//
// Author:              Gilad Yarnitzky
// Creation Date:       21/8/2013
// ----------------------------------------------------------------------------------
class AF_API afInformationView : public QWidget, public apIEventsObserver, public afBaseView
{
    Q_OBJECT

public:
    afInformationView(QWidget* pParent);
    ~afInformationView();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"InformationOuputView"; };

    // Clear the view:
    void clear();

    // Add line:
    void appendToOutputWindow(const QString& infoLine);

    /// clear output window
    void clearOutputWindow();


    // Edit menu commands
    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);
    virtual void onUpdateEdit_Find(bool& isEnabled);
    virtual void onUpdateEdit_FindNext(bool& isEnabled);

    virtual void onEdit_Copy();
    virtual void onEdit_SelectAll();

public slots:
    virtual void textLineReceived(const QStringList& fullTextAsLines, const int clickedLinePosition);

private:
    /////// Helper methods for parsing OpenGl error output TODO move this functions to Kernel Analyzer and pass it as function pointer/////////////////////////////////////
    ///This method parses error line output for OPenGl programs and opens according to error line in the source code window
    void GLSLOutputLineParser(const int lineNum, const int clickedLinePosition, const QStringList& fullTextAsLines) const;
    ///This method looks for stage name, that error occurred in , within the output string of  Gl program
    QString GetFailedGLSLShaderStageName(const int clickedLinePosition, const QStringList& fullTextAsLines) const;
    ///This method used in GetFailedStageName to obtain stage in VK and GL shaders
    /// \param [in] outputLine - line from the output to search for stage name
    /// \param [out] failedStage - the name of stage shader where error occured
    /// \param [in] typeIndicator - the indicator of stage name line that is different in VK and GL shaders compilation output
    /// \param [in] secondWord - the indicator word following the stage name
    /// ]retval - true if stage name obtained
    bool ObtainStageName(const QString& outputLine, QString& failedStage, const QString& typeIndicator, const QString& secondWord)const;
    /// This method constructs  map of  stage name to correspondent source file paths
    gtMap<QString, QString> BuildStageToFileMap(const QStringList& fullTextAsLines) const;
    /////// End Helper methods for parsing OpenGl error output /////////////////////////////////////////////////////////

private:
    // Main view layout:
    QLayout* m_pMainLayout;

    // Text to be displayed:
    afSourceCodeView* m_pEditor;

};

#endif // __AFINFORMATIONVIEW_H
