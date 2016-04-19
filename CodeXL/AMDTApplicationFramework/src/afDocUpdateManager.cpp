//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afDocUpdateManager.cpp
///
//==================================================================================

//Qt
#include <QtGui>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>
#include <AMDTApplicationFramework/Include/afIDocUpdateHandler.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/afSourceCodeViewsManager.h>

// Static member initialization:
afDocUpdateManager* afDocUpdateManager::m_pMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
afDocUpdateManager::afDocUpdateManager()
{

}

// ---------------------------------------------------------------------------
afDocUpdateManager::~afDocUpdateManager()
{

}

// ---------------------------------------------------------------------------
afDocUpdateManager& afDocUpdateManager::instance()
{
    if (nullptr == m_pMySingleInstance)
    {
        m_pMySingleInstance = new afDocUpdateManager;

    }

    return *m_pMySingleInstance;
}

// ---------------------------------------------------------------------------
void afDocUpdateManager::RegisterDocument(QWidget* pOwnedWidget, const osFile& filePath, afIDocUpdateHandler* updateHandler, bool notifyUser)
{
    // and owned widget and an handler are needed:
    GT_IF_WITH_ASSERT(nullptr != pOwnedWidget && nullptr != updateHandler)
    {
        afDocUpdateData* pData = new afDocUpdateData;
        pData->m_pOwnedWidget = pOwnedWidget;
        pData->m_filePath = filePath;
        pData->m_pUpdateNotifier = updateHandler;
        pData->m_notifyUser = notifyUser;
        // in widget cases there can never be a cases of always update
        pData->m_alwaysUpdate = false;

        // Get the document last update time:
        gtString fileNameAsStr = filePath.path().asString();
        osStatStructure fileInfo;
        osWStat(fileNameAsStr, fileInfo);
        pData->m_lastUpdate = fileInfo.st_mtime;

        m_widgetDocUpdateDataMap[pOwnedWidget] = pData;
    }
}

// ---------------------------------------------------------------------------
void afDocUpdateManager::RegisterDocumentActivate(const osFile& filePath, afIDocUpdateHandler* updateHandler, bool notifyUser, bool alwaysUpdate)
{
    GT_IF_WITH_ASSERT(nullptr != updateHandler)
    {
        afDocUpdateData* pData = new afDocUpdateData;
        pData->m_pOwnedWidget = nullptr;
        pData->m_filePath = filePath;
        pData->m_pUpdateNotifier = updateHandler;

        // notifyUser and alwaysUpdate they can't be both true
        GT_IF_WITH_ASSERT(notifyUser == false || alwaysUpdate == false)
        {
            pData->m_notifyUser = notifyUser;
            pData->m_alwaysUpdate = alwaysUpdate;
        }

        // Get the document last update time:
        gtString fileNameAsStr = filePath.path().asString();
        osStatStructure fileInfo;
        osWStat(fileNameAsStr, fileInfo);
        pData->m_lastUpdate = fileInfo.st_mtime;

        list<afDocUpdateData*>::iterator docFound = m_activateDocUpdateDataList.end();

        list<afDocUpdateData*>::iterator listBegin = m_activateDocUpdateDataList.begin();
        list<afDocUpdateData*>::iterator listEnd = m_activateDocUpdateDataList.end();

        while (listBegin != listEnd)
        {
            afDocUpdateData* pCurrentData = *listBegin;

            if (pCurrentData->m_filePath.path().asString() == fileNameAsStr)
            {
                docFound = listBegin;
                break;
            }

            listBegin++;
        }

        if (m_activateDocUpdateDataList.end() == docFound)
        {
            m_activateDocUpdateDataList.push_back(pData);
        }
        else
        {
            afDocUpdateData* foundData = *docFound;

            if (foundData->m_lastUpdate < pData->m_lastUpdate)
            {
                foundData->m_lastUpdate = pData->m_lastUpdate;
            }
        }
    }
}

// ---------------------------------------------------------------------------
void afDocUpdateManager::UnregisterDocumentOfWidget(QWidget* pOwnedWidget)
{
    if ((nullptr != pOwnedWidget) && m_widgetDocUpdateDataMap.count(pOwnedWidget) != 0)
    {
        afDocUpdateData* pData = m_widgetDocUpdateDataMap[pOwnedWidget];
        delete pData;
        m_widgetDocUpdateDataMap.erase(pOwnedWidget);
    }
}

// ---------------------------------------------------------------------------
void afDocUpdateManager::UnregisterDocumentOfActivate(const osFile& filePath, afIDocUpdateHandler* updateHandler)
{
    gtString filePathAsStr = filePath.path().asString();
    list<afDocUpdateData*>::iterator listBegin = m_activateDocUpdateDataList.begin();
    list<afDocUpdateData*>::iterator listEnd = m_activateDocUpdateDataList.end();

    while (listBegin != listEnd)
    {
        afDocUpdateData* pData = *listBegin;

        if ((pData->m_filePath.path().asString() == filePathAsStr) && (pData->m_pUpdateNotifier == updateHandler))
        {
            delete pData;
            m_activateDocUpdateDataList.erase(listBegin);
            break;
        }

        listBegin++;
    }
}

// ---------------------------------------------------------------------------
void afDocUpdateManager::ActivateView(QMdiSubWindow* pSubWindow, bool isTimerCheck)
{
    // find the widget that is owned by the subwindow:
    QWidget* pOwnedWidget = OwnedWidget(pSubWindow);

    // get the associated document
    afQMdiSubWindow* pAFSubWindow = qobject_cast<afQMdiSubWindow*>(pSubWindow);
    osFilePath filePath;

    if (nullptr != pAFSubWindow)
    {
        filePath = pAFSubWindow->filePath();
    }

    if (nullptr != pOwnedWidget)
    {
        CheckWidget(pOwnedWidget, filePath, isTimerCheck);
    }
}

// ---------------------------------------------------------------------------
void afDocUpdateManager::GetActivateDocumentList(osFilePath& docToActivate, gtVector<afDocUpdateData*>& docToUpdateList, bool isTimerCheck, bool& notifyUser)
{
    notifyUser = false;

    gtString docToActivateStr = docToActivate.asString();

    list<afDocUpdateData*>::iterator listBegin = m_activateDocUpdateDataList.begin();
    list<afDocUpdateData*>::iterator listEnd = m_activateDocUpdateDataList.end();

    while (listBegin != listEnd)
    {
        afDocUpdateData* pData = *listBegin;

        if (nullptr != pData && pData->m_filePath.path().asString() == docToActivateStr)
        {
            if (!isTimerCheck || (isTimerCheck && !pData->m_notifyUser))
            {
                docToUpdateList.push_back(pData);
                notifyUser |= pData->m_notifyUser;
            }
        }

        listBegin++;
    }
}

// ---------------------------------------------------------------------------
void afDocUpdateManager::RemoveSubWindow(QMdiSubWindow* pSubWindow)
{
    // Find the sub window iterator and remove it from the map:
    QWidget* pOwnedWidget = OwnedWidget(pSubWindow);

    UnregisterDocumentOfWidget(pOwnedWidget);
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
// ---------------------------------------------------------------------------
void afDocUpdateManager::ActivateView(HWND subWindow, gtString& activeFilePath, bool isTimerCheck)
{
    // find the widget that is owned by the subwindow:
    QWidget* pOwnedWidget = OwnedWidget(subWindow);
    osFilePath filePath(activeFilePath);

    CheckWidget(pOwnedWidget, filePath, isTimerCheck);
}

// ---------------------------------------------------------------------------
void afDocUpdateManager::RemoveSubWindow(HWND subWindow)
{
    // Find the sub window iterator and remove it from the map:
    QWidget* pOwnedWidget = OwnedWidget(subWindow);

    UnregisterDocumentOfWidget(pOwnedWidget);
}

// ---------------------------------------------------------------------------
QWidget* afDocUpdateManager::OwnedWidget(HWND subWindow)
{
    QWidget* pOwnedWidget = nullptr;
    gtMap<QWidget*, afDocUpdateData*>::iterator widgetIterator;

    for (widgetIterator = m_widgetDocUpdateDataMap.begin() ; widgetIterator != m_widgetDocUpdateDataMap.end() && (nullptr == pOwnedWidget); widgetIterator++)
    {
        QWidget* pCurrentWidget = (*widgetIterator).first;
        GT_IF_WITH_ASSERT(pCurrentWidget != nullptr)
        {
            HWND qtHwnd = reinterpret_cast<HWND>(pCurrentWidget->winId());

            // Qt connection to VS does not enable us to find the real parent of the widget through the subWindow
            // since Qt will not connect to VS and the last Qt object will point to a nullptr parent. So we'll reach
            // The highest parent in Qt and look at all the children in the subWindow to see if the match that parent:

            // Find the highest parent in Qt:
            HWND windowParent = qtHwnd;

            while (::GetParent(windowParent) != nullptr)
            {
                windowParent = ::GetParent(windowParent);
            }

            HWND childWindow = ::GetWindow(subWindow, GW_CHILD);

            while (childWindow != windowParent && nullptr != childWindow)
            {
                childWindow = ::GetWindow(childWindow, GW_HWNDNEXT);
            }

            if (childWindow == windowParent)
            {
                pOwnedWidget = pCurrentWidget;
            }
        }
    }

    return pOwnedWidget;
}

#endif

// ---------------------------------------------------------------------------
QWidget* afDocUpdateManager::OwnedWidget(QMdiSubWindow* pSubWindow)
{
    QWidget* pOwnedWidget = nullptr;
    gtMap<QWidget*, afDocUpdateData*>::iterator widgetIterator;

    for (widgetIterator = m_widgetDocUpdateDataMap.begin() ; widgetIterator != m_widgetDocUpdateDataMap.end() && (nullptr == pOwnedWidget); widgetIterator++)
    {
        QWidget* pCurrentWidget = (*widgetIterator).first;

        if (pSubWindow->isAncestorOf(pCurrentWidget))
        {
            pOwnedWidget = pCurrentWidget;
        }
    }

    return pOwnedWidget;
}

// ---------------------------------------------------------------------------
void afDocUpdateManager::CheckWidget(QWidget* pOwnedWidget, osFilePath& filePath, bool isTimerCheck)
{
    GT_UNREFERENCED_PARAMETER(filePath);

    static bool inActiveViewEvent = false;

    // If the active window has a widget that needs handling (nullptr is allowed since an active window can have a document that does not need sync):
    if (!inActiveViewEvent)
    {
        // Check the data from the activate document list
        gtVector<afDocUpdateData*> documentsToUpdate;
        bool notifyUser = false;
        GetActivateDocumentList(filePath, documentsToUpdate, isTimerCheck, notifyUser);

        // if this is a timer check do not test the current view, since we do not want to update it only
        // listeners to the current view.
        if (!isTimerCheck)
        {
            // There can be a nullptr widget in VS in case the view was opened by VS and not by us (source editor)
            if (nullptr != pOwnedWidget && m_widgetDocUpdateDataMap.count(pOwnedWidget) != 0)
            {
                afDocUpdateData* pDataFromView = m_widgetDocUpdateDataMap[pOwnedWidget];
                GT_IF_WITH_ASSERT(nullptr != pDataFromView)
                {
                    documentsToUpdate.push_back(pDataFromView);
                    notifyUser |= pDataFromView->m_notifyUser;
                }
            }
        }

        if (documentsToUpdate.size() != 0 && nullptr != documentsToUpdate.at(0))
        {
            // take the file name from documentsToUpdate since filePath can be empty if this is an activation event with no
            // registered documents handler for activation only for widgets event
            filePath = documentsToUpdate.at(0)->m_filePath.path();
            inActiveViewEvent = true;

            // Check if the date changed:
            gtString fileNameAsStr = filePath.asString();
            osStatStructure fileInfo;
            osWStat(fileNameAsStr, fileInfo);

            // check if at least on of the files is not synced
            bool isFileInSync = true;
            int numDocs = documentsToUpdate.size();

            for (int nDoc = 0; nDoc < numDocs && isFileInSync; nDoc++)
            {
                afDocUpdateData* pCurrentDoc = documentsToUpdate.at(nDoc);

                if (nullptr != pCurrentDoc)
                {
                    if (pCurrentDoc->m_lastUpdate != fileInfo.st_mtime)
                    {
                        isFileInSync = false;
                    }
                }
            }

            // only do the update if the file is not at sync with at least of the listers
            if (!isFileInSync)
            {
                QString viewText;
                bool updateIsNeeded = true;

                if (notifyUser)
                {
                    osFile fileToCheck(filePath);

                    bool readRes = fileToCheck.open(filePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);
                    GT_IF_WITH_ASSERT(readRes)
                    {
                        unsigned long fileSize = 0;
                        bool rcSz = fileToCheck.getSize(fileSize);
                        GT_IF_WITH_ASSERT(rcSz)
                        {
                            // Get the file contents:
                            gtVector<gtByte> stringBuffer(fileSize + 1);
                            fileToCheck.read(&(stringBuffer[0]), fileSize);
                            fileToCheck.close();

                            stringBuffer[fileSize] = (char)0;

                            // Scintilla converts extended unicode characters (128-255) to question mark characters.
                            // QString converts them to signed short. This causes a mismatch in comparison (e.g. teapot
                            // kernels containing the copyright symbol).
                            for (char& c : stringBuffer)
                            {
                                if (0 > c)
                                {
                                    // Replace EOF character with null character, and all others with ?:
                                    c = (-1 == c) ? (char)0 : '?';
                                }
                            }

                            QString fileText(&(stringBuffer[0]));

                            // Get the view contents:
                            int currentView = 0;
                            afSourceCodeView* pSourceCodeView = afSourceCodeViewsManager::instance().getExistingView(filePath, currentView);

                            if (pSourceCodeView != nullptr)
                            {
                                viewText = pSourceCodeView->text();
                            }

                            // If they are not matching:
                            if (viewText != fileText)
                            {
                                QString updateMessage = QString(AF_STR_UpdateOutOfDate).arg(acGTStringToQString(filePath.asString()));
                                int userInput = acMessageBox::instance().question(AF_STR_QuestionA, updateMessage, QMessageBox::Yes | QMessageBox::No);
                                updateIsNeeded = (userInput == QMessageBox::Yes);
                            }
                        }
                    }
                }

                if (updateIsNeeded)
                {
                    // pass through all the registered listeners for this document:
                    for (int nDoc = 0; nDoc < numDocs; nDoc++)
                    {
                        afDocUpdateData* pCurrentDoc = documentsToUpdate.at(nDoc);

                        if (nullptr != pCurrentDoc && nullptr != pCurrentDoc->m_pUpdateNotifier)
                        {
                            // notify the document handler:
                            pCurrentDoc->m_pUpdateNotifier->UpdateDocument(filePath);
                            // Update the time to the new time:
                            pCurrentDoc->m_lastUpdate = fileInfo.st_mtime;
                        }
                    }
                }
            }

            inActiveViewEvent = false;
        }
    }
}

// ---------------------------------------------------------------------------
void afDocUpdateManager::ForceAlwasyUpdateDocuments()
{
    list<afDocUpdateData*>::iterator listBegin = m_activateDocUpdateDataList.begin();
    list<afDocUpdateData*>::iterator listEnd = m_activateDocUpdateDataList.end();

    while (listBegin != listEnd)
    {
        afDocUpdateData* pData = *listBegin;

        if (pData->m_alwaysUpdate)
        {
            osFilePath filePath = pData->m_filePath.path();
            gtString fileNameAsStr = filePath.asString();
            osStatStructure fileInfo;
            osWStat(fileNameAsStr, fileInfo);

            if (pData->m_lastUpdate != fileInfo.st_mtime)
            {
                pData->m_pUpdateNotifier->UpdateDocument(filePath);
            }
        }

        listBegin++;
    }
}

// ---------------------------------------------------------------------------
void afDocUpdateManager::UpdateDocument(QWidget* pOwnedWidget)
{
    GT_IF_WITH_ASSERT(nullptr != pOwnedWidget)
    {
        if (m_widgetDocUpdateDataMap.count(pOwnedWidget) != 0)
        {
            afDocUpdateData* pData = m_widgetDocUpdateDataMap[pOwnedWidget];
            GT_IF_WITH_ASSERT(nullptr != pData)
            {
                gtString fileNameAsStr = pData->m_filePath.path().asString();
                osStatStructure fileInfo;
                osWStat(fileNameAsStr, fileInfo);

                // Update the time to the new time:
                pData->m_lastUpdate = fileInfo.st_mtime;
            }
        }
    }
}

void afDocUpdateManager::RenameFile(const osFilePath& oldFilePath, const osFilePath& newFilePath)
{
    for (auto iter = m_activateDocUpdateDataList.begin(); iter != m_activateDocUpdateDataList.end(); iter++)
    {
        // Find the data containing the old file name and replace it with the new one:
        afDocUpdateData* pData = *iter;

        if (pData != nullptr)
        {
            if (pData->m_filePath.path() == oldFilePath)
            {
                pData->m_filePath = newFilePath;
                break;
            }
        }
    }

    for (auto iter = m_widgetDocUpdateDataMap.begin(); iter != m_widgetDocUpdateDataMap.end(); iter++)
    {
        // Find the data containing the old file name and replace it with the new one:
        afDocUpdateData* pData = iter->second;

        if (pData != nullptr)
        {
            if (pData->m_filePath.path() == oldFilePath)
            {
                pData->m_filePath = newFilePath;
                break;
            }
        }
    }
}
