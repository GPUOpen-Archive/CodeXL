//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afHTMLView.cpp
///
//==================================================================================

// Infra
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afHTMLView.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>


afHTMLView::afHTMLView(const osFilePath& filePath) : QWebView(), m_filePath(filePath)
{

    ReloadView();

    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    bool rc = connect(this, SIGNAL(linkClicked(const QUrl&)), this, SLOT(OnLinkClicked(const QUrl&)));
    GT_ASSERT(rc);
}

void afHTMLView::ReloadView()
{
    if (m_filePath.exists())
    {
        QString htmlText;
        QString htmlFilePath(acGTStringToQString(m_filePath.asString()));
        QFile file(htmlFilePath);
        bool rc = file.open(QIODevice::ReadOnly | QIODevice::Text);
        GT_IF_WITH_ASSERT(rc)
        {
            QTextStream in(&file);

            while (!in.atEnd())
            {
                QString line = in.readLine();
                htmlText.append(line);
            }
        }
        file.close();

        setHtml(htmlText);
    }
}

afHTMLView::~afHTMLView()
{

}

void afHTMLView::OnLinkClicked(const QUrl& url)
{

    QString urlAsStr = url.toString();

    if (urlAsStr.startsWith(AF_STR_HtmlFindSourceLink))
    {
        // find the line number
        QString numberSection = urlAsStr.right(urlAsStr.length() - QString(AF_STR_HtmlFindSourceLink).length() - 1); // the extra -1 is for the '-'
        int lineNumber = numberSection.toInt();
        FindSourceLink(lineNumber);
    }
    else
    {
        // if it is not one of the special links open the url
        QDesktopServices::openUrl(url);
    }
}

void afHTMLView::FindSourceLink(int lineNumber)
{
    // let the user select the file
    QString defaultFileLocation = acGTStringToQString(afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString());
    QString caption(acGTStringToQString(AF_STR_openFileStatusbarString));
    QString loadFileName = afApplicationCommands::instance()->ShowFileSelectionDialog(caption, defaultFileLocation, AF_STR_sourceFileDetails, nullptr);

    // if the file was selected ask the user if he wants to add it to the system pref dirs
    if (!loadFileName.isEmpty())
    {
        osFilePath filePath(acQStringToGTString(loadFileName));
        gtString fileDirAsStr = filePath.fileDirectoryAsString();

        QString questionStr = QString(AF_STR_sourceCodeQuestionInclude).arg(acGTStringToQString(fileDirAsStr));

        if (acMessageBox::instance().question(AF_STR_QuestionA, questionStr, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            // get the current source paths
            gtString sourceDirectories = afProjectManager::instance().currentProjectSettings().SourceFilesDirectories();
            QString srcDirs = acGTStringToQString(sourceDirectories);

            if ((!srcDirs.isEmpty()) && (!srcDirs.endsWith(";")))
            {
                srcDirs += ";";
            }

            // add the dir path of the selected file
            osDirectory dirPath;
            filePath.getFileDirectory(dirPath);
            srcDirs += acGTStringToQString(dirPath.directoryPath().asString());

            afProjectManager::instance().SetSourceFilesDirectories(acQStringToGTString(srcDirs));
        }

        // open the file
        afApplicationCommands::instance()->OpenFileAtLine(filePath, lineNumber);
    }
}