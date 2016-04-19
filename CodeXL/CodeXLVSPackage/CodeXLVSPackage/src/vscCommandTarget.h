//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscCommandTarget.h
///
//==================================================================================

/**********************************************************************************
This code is based on the VSDCommandTarget.handler
There was a need to duplicate this file and resolve some issues:
IOleCommandTarget implements two interface functions:
STDMETHOD(QueryStatus)(_In_ const GUID* pCmdGroupGuid,_In_ ULONG cCmds, _Inout_cap_(cCmds) OLECMD pCmds[], _Inout_opt_ OLECMDTEXT* pCmdText)
STDMETHOD(Exec)(_In_ const GUID* pCmdGroupGuid,_In_ DWORD nCmdID,_In_ DWORD nCmdexecopt, _In_opt_ VARIANT* pIn,_Inout_opt_ VARIANT* pOut)
Both functions return HRESULT.
Instead of implementing those two functions in a class that implements
IOleCommandTarget and creating a big
switch(nCmdID) { case...} to handle all commands handled by the command handler this VS implementation
created a set of macros with a map that helps the user create a specific function for
each cmdId both for the QueryStatus & Exec function.
However the problem is that those newly created function do not return HRESULT
This reduces the control the programmer has and leads to a specific unique problem:
If the user defines in the macro a specific handling function for the QueryStatus
for updating the UI. In specific cases but dose not want to handle the Exec
at all he cannot. In the new copied and modified class all function must
return an HRESULT value that is forwarded as needed. This means that if a
programmer wants only to implement one function of the two he needs to
implement both but in the one that don't need to do anything he just needs
to return OLECMDERR_E_NOTSUPPORTED
**********************************************************************************/

#ifndef _VSPCOMMANDTARGET_H_
#define _VSPCOMMANDTARGET_H_

#if _MSC_VER > 1000
    #pragma once
#endif

#include <VSL.h>
#include <VSLErrorHandlers.h>
#include <VSLExceptionHandlers.h>
#include <VsShellInterfaces.h>
#include <limits>
#include <AtlColl.h>
#include <atlstr.h>

// A command is identified by VS using a GUID/DWORD pair where the GUID is the command set
// and the DWORD is a unique identifier for the command inside the command set.
class vscCommandId
{
private:

    // No default construction or assignment
    vscCommandId();
    const vscCommandId& operator=(const vscCommandId& rToCopy);

public:
    vscCommandId(const GUID& rGuid, DWORD id) :
        m_CommandGuid(rGuid),
        m_Id(id)
    {
    }

    vscCommandId(const vscCommandId& rToCopy) :
        m_CommandGuid(rToCopy.m_CommandGuid),
        m_Id(rToCopy.m_Id)
    {
    }

    const GUID& GetGuid() const
    {
        return m_CommandGuid;
    }

    const DWORD GetId() const
    {
        return m_Id;
    }

    bool operator==(const vscCommandId& rvscCommandId) const
    {
        return (m_CommandGuid == rvscCommandId.m_CommandGuid) && (m_Id == rvscCommandId.m_Id);
    }

    bool operator!=(const vscCommandId& rvscCommandId) const
    {
        return !operator==(rvscCommandId);
    }

    // This operator is used by the ATL map to get a hash code for the object.
    operator ULONG_PTR() const
    {
        return m_CommandGuid.Data1 ^ (((int)m_CommandGuid.Data2 << 16) | (int)m_CommandGuid.Data3) ^
               (((int)m_CommandGuid.Data4[3] << 24) | m_CommandGuid.Data4[7]) ^ m_Id;
    }

private:

    GUID m_CommandGuid;
    DWORD m_Id;
};

// This class handles a single command. It provides a default implementation for
// the QueryStatus and the Exec methods.
template<class Target_T>
class vspCommandHandlerBase
{

    VSL_DECLARE_NOT_COPYABLE(vspCommandHandlerBase)

public:
    // Define the types for the functions that handle the QueryStatus and Exec methods.
    typedef HRESULT(Target_T::*QueryStatusHandler)(const typename Target_T::vspCommandHandler& handler, OLECMD*, OLECMDTEXT*);
    typedef HRESULT(Target_T::*ExecHandler)(typename Target_T::vspCommandHandler* handler, DWORD, VARIANT*, VARIANT*);

    vspCommandHandlerBase(const vscCommandId& vscCommandId, QueryStatusHandler statusHandler = NULL, ExecHandler execHandler = NULL, DWORD dwStatus = OLECMDF_SUPPORTED | OLECMDF_ENABLED, const wchar_t* szText = NULL) :
        m_vscCommandId(vscCommandId),
        m_OleCommandFlags(dwStatus),
        m_StatusHandler(DefaultStatusHandlerOnNull(statusHandler)),
        m_ExecHandler(execHandler),
        m_strText(szText)
    {
    }

    vspCommandHandlerBase(const GUID& rGuid, DWORD id, QueryStatusHandler statusHandler = NULL, ExecHandler execHandler = NULL, DWORD dwStatus = OLECMDF_SUPPORTED | OLECMDF_ENABLED, const wchar_t* szText = NULL) :
        m_vscCommandId(rGuid, id),
        m_OleCommandFlags(dwStatus),
        m_StatusHandler(DefaultStatusHandlerOnNull(statusHandler)),
        m_ExecHandler(execHandler),
        m_strText(szText)
    {
    }

    virtual ~vspCommandHandlerBase() {}

    // QueryStatus: this method is called when the shell needs to get the status of the commands,
    // e.g. when a menu is show.
    virtual HRESULT QueryStatus(Target_T* pTarget, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
    {
        VSL_CHECKBOOLEAN(m_StatusHandler != NULL, E_POINTER);
        VSL_CHECKPOINTER_DEFAULT(pTarget);
        VSL_CHECKPOINTER_DEFAULT(pOleCmd);
        return (pTarget->*m_StatusHandler)(*this, pOleCmd, pOleText);
    }

    static HRESULT QueryStatusDefault(const vspCommandHandlerBase<Target_T>& rSender, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
    {
        pOleCmd->cmdf = rSender.GetFlags();

        // Check if the shell is asking for the text of this command.
        if ((NULL != pOleText) && (0 != (pOleText->cmdtextf & OLECMDTEXTF_NAME)) && (rSender.GetText()))
        {
            ULONG charsToWrite = rSender.GetText().GetLength();

            if (charsToWrite > pOleText->cwBuf - 1)
            {
                charsToWrite = pOleText->cwBuf - 1;
            }

            ::wcsncpy_s(pOleText->rgwz, pOleText->cwBuf, rSender.GetText(), charsToWrite);
            pOleText->cwActual = charsToWrite;
        }

        return S_OK;
    }

    // Exec: This method is called when the user selects a command clicking on a menu item or on a
    // toolbar button.
    virtual HRESULT Exec(Target_T* pTarget, DWORD dwFlags, VARIANT* pIn, VARIANT* pOut)
    {
        if ((NULL != m_ExecHandler))
        {
            VSL_CHECKPOINTER_DEFAULT(pTarget);
            return (pTarget->*m_ExecHandler)(this, dwFlags, pIn, pOut);
        }

        return OLECMDERR_E_NOTSUPPORTED;
    }

    // Get the ID of the command handled by this object.
    const vscCommandId& GetId() const
    {
        return m_vscCommandId;
    }

    // Command Text
    const CStringW& GetText() const
    {
        return m_strText;
    }

    CStringW& GetText()
    {
        return m_strText;
    }

    DWORD GetFlags() const
    {
        return m_OleCommandFlags;
    }

    // Command Checked
    bool GetChecked() const
    {
        return (0 != (m_OleCommandFlags & OLECMDF_LATCHED));
    }

    void SetChecked(bool bChecked)
    {
        SetFlag(OLECMDF_LATCHED, bChecked);
    }

    // Command Enabled
    bool GetEnabled() const
    {
        return (0 != (m_OleCommandFlags & OLECMDF_ENABLED));
    }

    void SetEnabled(bool bEnabled)
    {
        SetFlag(OLECMDF_ENABLED, bEnabled);
    }

    // Command Supported
    bool GetSupported() const
    {
        return (0 != (m_OleCommandFlags & OLECMDF_SUPPORTED));
    }

    void SetSupported(bool bSupported)
    {
        SetFlag(OLECMDF_SUPPORTED, bSupported);
    }

    // Command Visible
    bool GetVisible() const
    {
        return (0 == (m_OleCommandFlags & OLECMDF_INVISIBLE));
    }

    void SetVisible(bool bVisible)
    {
        SetFlag(OLECMDF_INVISIBLE, !bVisible);
    }

private:

    void SetFlag(DWORD flag, bool bSet)
    {
        if (bSet)
        {
            m_OleCommandFlags |= flag;
        }
        else
        {
            m_OleCommandFlags &= ~flag;
        }
    }

    QueryStatusHandler DefaultStatusHandlerOnNull(QueryStatusHandler statusHandler)
    {
        return statusHandler != NULL ? statusHandler : &Target_T::QueryStatusDefault;
    }

    vscCommandId m_vscCommandId; // The Id of this command.
    DWORD m_OleCommandFlags; // The dwFlags for the command.
    QueryStatusHandler m_StatusHandler; // The function to call for the QueryStatus.
    ExecHandler m_ExecHandler; // The function to call for the Exec.
    CStringW m_strText;

};

template <class Derived_T>
class IOleCommandTargetVSPImpl :
    public IOleCommandTarget
{

    VSL_DECLARE_NONINSTANTIABLE_BASE_CLASS(IOleCommandTargetVSPImpl)

public:
    typedef vspCommandHandlerBase<Derived_T> vspCommandHandler;

protected:
    vspCommandHandler& GetCommand(const vscCommandId& rId)
    {
        vspCommandHandler* handler = Base_T::GetCommand(rId);
        VSL_CHECKBOOLEAN(NULL != handler, OLECMDERR_E_NOTSUPPORTED);
#pragma warning(push) // compiler doesn't get that the above line will throw if pair is NULL
#pragma warning(disable : 6011) // Dereferencing NULL pointer 'pair'
        return *handler;
#pragma warning(pop)
    }

    vspCommandHandler* TryToGetCommand(const vscCommandId& rId)
    {
        return Derived_T::GetCommand(rId);
    }

public:

    STDMETHOD(QueryStatus)(
        _In_ const GUID* pCmdGroupGuid,
        _In_ ULONG cCmds,
        _Inout_cap_(cCmds) OLECMD pCmds[],
        _Inout_opt_ OLECMDTEXT* pCmdText)
    {
        VSL_STDMETHODTRY
        {

            VSL_CHECKPOINTER_DEFAULT(pCmdGroupGuid);
            VSL_CHECKPOINTER_DEFAULT(pCmds);
            VSL_CHECKBOOLEAN(1 == cCmds, E_INVALIDARG);

            // It is expected and common that the command is not found, so in this case we want to use
            // the non throwing version of GetCommand.
            vspCommandHandler* pCommandHandler = TryToGetCommand(vscCommandId(*pCmdGroupGuid, pCmds[0].cmdID));

            if (NULL == pCommandHandler)
            {
                return OLECMDERR_E_NOTSUPPORTED;
            }

            VSL_SET_STDMETHOD_HRESULT(pCommandHandler->QueryStatus(static_cast<Derived_T*>(this), pCmds, pCmdText));

        } VSL_STDMETHODCATCH()

        return VSL_GET_STDMETHOD_HRESULT();
    }

    STDMETHOD(Exec)(
        _In_ const GUID* pCmdGroupGuid,
        _In_ DWORD nCmdID,
        _In_ DWORD nCmdexecopt,
        _In_opt_ VARIANT* pIn,
        _Inout_opt_ VARIANT* pOut)
    {
        VSL_STDMETHODTRY
        {

            VSL_CHECKPOINTER_DEFAULT(pCmdGroupGuid);

            // It is expected and common that the command is not found, so in this case we want to use
            // the non throwing version of GetCommand.
            vspCommandHandler* pCommandHandler = TryToGetCommand(vscCommandId(*pCmdGroupGuid, nCmdID));

            if (NULL == pCommandHandler)
            {
                return OLECMDERR_E_NOTSUPPORTED;
            }

            VSL_SET_STDMETHOD_HRESULT(pCommandHandler->Exec(static_cast<Derived_T*>(this), nCmdexecopt, pIn, pOut));

        } VSL_STDMETHODCATCH()

        return VSL_GET_STDMETHOD_HRESULT();
    }

    HRESULT QueryStatusDefault(const vspCommandHandler& rSender, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
    {
        return vspCommandHandler::QueryStatusDefault(rSender, pOleCmd, pOleText);
    }
};

#define VSP_BEGIN_COMMAND_MAP(className) \
    className::vspCommandHandler* className::GetCommand(const vscCommandId& rId) \
    { \
        /* Default, unfortunately ATL doesn't supply a const for it. */ \
        UINT iNumberOfBins = 17; \
        __if_exists(CAtlMapNumberOfBins) \
        { \
            iNumberOfBins = CAtlMapNumberOfBins; \
        } \
        \
        typedef CAtlMap<const vscCommandId, vspCommandHandler*> CommandMap; \
        \
        static CommandMap commands; \
        static bool bInitialized = false; \
        if(!bInitialized) \
        { \
            commands.InitHashTable(iNumberOfBins, false);

#define VSP_COMMAND_MAP_ENTRY(guid, id, qsHandler, execHandler) \
    static vspCommandHandler guid##id##vspCommandHandler(guid, id, static_cast<vspCommandHandler::QueryStatusHandler>(qsHandler), static_cast<vspCommandHandler::ExecHandler>(execHandler)); \
    commands[guid##id##vspCommandHandler.GetId()] = &guid##id##vspCommandHandler;

#define VSP_COMMAND_MAP_ENTRY_WITH_FLAGS(guid, id, qsHandler, execHandler, dwFlags) \
    static vspCommandHandler guid##id##vspCommandHandler(guid, id, static_cast<vspCommandHandler::QueryStatusHandler>(qsHandler), static_cast<vspCommandHandler::ExecHandler>(execHandler), dwFlags); \
    commands[guid##id##vspCommandHandler.GetId()] = &guid##id##CommandHandler;

#define VSP_COMMAND_MAP_CLASS_ENTRY(type, parameters) \
    { \
        static type handler parameters; \
        commands[handler.GetId()] = &handler; \
    }

#define VSP_END_VSCOMMAND_MAP() \
    bInitialized = true; \
    }; \
    \
    CommandMap::CPair* pair = commands.Lookup(rId); \
    if(NULL == pair) \
    { \
        return NULL; \
    } \
    return pair->m_value; \
    }


#endif // _VSPCOMMANDTARGET_H_
