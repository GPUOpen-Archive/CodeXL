//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscImageAndBuffersEditorDocument.h
///
//==================================================================================

//------------------------------ vspImageAndBuffersEditorDocument.h ------------------------------

#ifndef __VSCIMAGEANDBUFFERSEDITORDOCUMENT_H
#define __VSCIMAGEANDBUFFERSEDITORDOCUMENT_H

// Infra:
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

#include <Rpc.h>

// Local:
#include <Include/Public/vscEditorDocument.h>
#include "CodeXLVSPackageCoreDefs.h"

class gdImageAndBufferView;
class gdThumbnailView;

/// ----------------------------------------------------------------------------------
/// Class Name:    vscImageAndBuffersEditorDocument
///                This class is used to implement the core side of debugger MDI windows created by CodeXL VS package.
///                The class is implementing Kernel Analyzer specific MDI needs. The functions will be called when the VS
///                extension will call vspEditorDocument functionality, which will call the appropriate vsc_ function, which
///                will call this class
/// ----------------------------------------------------------------------------------
class vscImageAndBuffersEditorDocument : public vscEditorDocument
{
public:
    vscImageAndBuffersEditorDocument();

    ~vscImageAndBuffersEditorDocument() {}

    virtual void LoadDocData(const wchar_t* filePathStr);

    bool GetEditorCaption(const wchar_t* filePathStr, wchar_t*& pOutBuffer);
    virtual void SetEditorCaption(const wchar_t* filePathStr, wchar_t*& itemNameStrBuffer);

    void OnSize(int x, int y, int w, int h);

    void ClosePane();

    void OnShow();


    // A pointer to the created image buffer view:
    gdImageAndBufferView* ImageBufferView() const { return m_pImageBufferView; };

    // A pointer to the created thumbnail view:
    gdThumbnailView* ThumbnailView() const { return m_pThumbnailView; };

protected:
    ///  Create a CodeXL view according to the object type
    /// \param itemType the item type to be created
    /// \return the created window
    QWidget* CreateWindowByObjectType(afTreeItemType itemType);

    /// This function is called when the content of the window is created. It should be implemented by the specific classes:
    virtual QWidget* CreateView();

protected:

    // A pointer to the created image buffer view:
    gdImageAndBufferView* m_pImageBufferView;

    // A pointer to the created thumbnail view:
    gdThumbnailView* m_pThumbnailView;

};

void* vscImageAndBuffersEditorDocument_CreateInstance();

bool vscCodeXLImageAndBufferCommandIDFromVSCommandId(const GUID& cmdGuid, DWORD cmdId, long& cmdIdBuffer);

void  vscImageAndBuffersEditorDocument_OnImageAndBuffersAction(const GUID& cmdGuid, DWORD cmdId);

bool vscImageAndBuffersEditorDocument_OnQueryImageAndBufferAction_IsActionRequired(const GUID& cmdGuid, DWORD cmdId);

void vscImageAndBuffersEditorDocument_OnQueryImageAndBufferCheckedAction_IsActionRequired(const GUID& cmdGuid, DWORD cmdId, bool& shouldEnableBuffer, bool& shouldCheckBuffer);

void vscImageAndBuffersEditorDocument_OnQueryImageSizeChanged_IsActionRequired(bool& shouldEnableBuffer);

bool vscImageAndBuffersEditorDocument_GetAvailableZoomLevels(unsigned int*& pOutBuffer, size_t& sizeBuffer);

bool vscImageAndBuffersEditorDocument_ChangeZoomLevel(const wchar_t* pZoomText, int& currentZoomLevelBuffer);

#endif // __VSCIMAGEANDBUFFERSEDITORDOCUMENT_H
