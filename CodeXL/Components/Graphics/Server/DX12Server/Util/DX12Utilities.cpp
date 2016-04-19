//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12Utilities.cpp
/// \brief  Provides a set of utility functions for use in the DX12 server plugin.
//=============================================================================

#include "DX12Utilities.h"
#include "../../Common/xml.h"
#include "../Objects/DX12WrappedObjectDatabase.h"
#include "../Objects/IDX12InstanceBase.h"
#include "../Objects/DX12ObjectDatabaseProcessor.h"

namespace DX12Util
{
//-----------------------------------------------------------------------------
/// A custom GUID-printing implementation.
/// \param inGUID The GUID to append into a formatted string.
/// \param ioGUIDString A string containing the human-readable GUID.
//-----------------------------------------------------------------------------
void PrintGUID(REFGUID inGUID, gtASCIIString& ioGUIDString)
{
    ioGUIDString.appendFormattedString(
        "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
        inGUID.Data1, inGUID.Data2, inGUID.Data3,
        inGUID.Data4[0], inGUID.Data4[1], inGUID.Data4[2], inGUID.Data4[3],
        inGUID.Data4[4], inGUID.Data4[5], inGUID.Data4[6], inGUID.Data4[7]);

    ioGUIDString.toUpperCase();
}

//-----------------------------------------------------------------------------
/// Print a REFIID into the output string.
/// \param inREFIID The REFIID to print.
/// \param ioREFIIDString A string containing the human-readable REFIID.
//-----------------------------------------------------------------------------
void PrintREFIID(REFIID inREFIID, gtASCIIString& ioREFIIDString)
{
    ioREFIIDString.appendFormattedString(
        "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        inREFIID.Data1,
        inREFIID.Data2,
        inREFIID.Data3,
        inREFIID.Data4[0],
        inREFIID.Data4[1],
        inREFIID.Data4[2],
        inREFIID.Data4[3],
        inREFIID.Data4[4],
        inREFIID.Data4[5],
        inREFIID.Data4[6],
        inREFIID.Data4[7]);
    ioREFIIDString.toUpperCase();
}

//-----------------------------------------------------------------------------
/// Convert a DX12 bool into a string for display in client.
/// \param inBool The boolean variable to print.
/// \returns A string containing "true" or "false" based on the input.
//-----------------------------------------------------------------------------
const char* PrintBool(BOOL inBool)
{
    return (inBool == TRUE) ? "TRUE" : "FALSE";
}

//-----------------------------------------------------------------------------
/// Decompose the incoming packed flags into a pipe-separated string of enum strings.
/// \param inFlags A UINT instance where packed flags have been bitwise-OR'd into the variable.
/// \param ioFlagsString The string that the result will be packed into.
/// \param inWriteHook A pointer to the function responsible for writing the given enumeration into a string.
/// \param inMinFlag An enumeration member that controls which value to start decomposing flags from.
/// \param inMaxFlag An enumeration member that controls which value to stop decomposing flags from.
//-----------------------------------------------------------------------------
void DecomposeFlags(DWORD inFlags, gtASCIIString& ioFlagsString, WriteEnum_Hook inWriteHook, DWORD inMinFlag, DWORD inMaxFlag)
{
    // Initialize the flag to the minimum enum value.
    DWORD currentFlag = inMinFlag;
    bool bFlagWritten = false;

    do
    {
        // If there's an overlap between the input flags and the current flag bit, append it to the output string.
        if ((currentFlag & inFlags) == currentFlag)
        {
            // Append a spacer between the new and previous flag string (but only if this isn't the first flag).
            if ((currentFlag != inMinFlag) && bFlagWritten)
            {
                ioFlagsString.append(" | ");
            }

            bFlagWritten = true;
            ioFlagsString.appendFormattedString("%s", inWriteHook(currentFlag));
        }

        // If the min flag is zero, we're going to loop forever. Increment to "1" to check the next bit, and we'll be able to shift to check the remaining flags.
        if (currentFlag == 0)
        {
            currentFlag = 1;
        }
        else
        {
            // Shift the current flag to the left to check the for the presence of the next flag.
            currentFlag = (currentFlag << 1);
        }
    }
    while (currentFlag <= inMaxFlag);
}

//-----------------------------------------------------------------------------
/// Surround the given "inElementContents" XML with a root element that includes an optional 'name' attribute.
/// \param inStructureName The name of the type of structure being serialized.
/// \param inElementContents The XML contents that will be surrounded by a new parent element.
/// \param inOptionalNameAttribute An optional string to insert as the 'name' attribute for the new parent element.
/// \returns An XML string containing a new root element with name 'inStructureName', surrounding the original XML in 'inElementContents'.
//-----------------------------------------------------------------------------
gtASCIIString& SurroundWithNamedElement(const char* inStructureName, gtASCIIString& inElementContents, const char* inOptionalNameAttribute)
{
    // If the name argument is non-null, we can insert it as a name attribute.
    if (inOptionalNameAttribute != nullptr)
    {
        inElementContents = XMLAttrib(inStructureName, FormatText("name='%s'", inOptionalNameAttribute).asCharArray(), inElementContents.asCharArray());
    }
    else
    {
        inElementContents = XML(inStructureName, inElementContents.asCharArray());
    }

    return inElementContents;
}

//-----------------------------------------------------------------------------
/// Waits for a fence to complete.
/// \param fenceValueToWaitOn The fence value to wait on.
/// \param pFence The fence instance being waited on.
/// \param hEvent The OS event handle associated with the fence being waited on.
//-----------------------------------------------------------------------------
void WaitForFence(UINT64 fenceValueToWaitOn, ID3D12Fence* pFence, HANDLE hEvent)
{
    const UINT64 lastCompletedFence = pFence->GetCompletedValue();

    // If not completed, wait
    if (lastCompletedFence < fenceValueToWaitOn)
    {
        pFence->SetEventOnCompletion(fenceValueToWaitOn, hEvent);
        WaitForSingleObject(hEvent, INFINITE);
    }
}

//-----------------------------------------------------------------------------
/// Safely retrieve a device pointer for a device child.
/// \param pDeviceChild The DeviceChild to get the parent device for.
/// \returns The parent device for the incoming child interface.
//-----------------------------------------------------------------------------
ID3D12Device* SafeGetDevice(ID3D12DeviceChild* pDeviceChild)
{
    ID3D12Device* pDevice = nullptr;

    DX12WrappedObjectDatabase* pObjDatabase = (DX12WrappedObjectDatabase*)DX12ObjectDatabaseProcessor::Instance()->GetObjectDatabase();

    // Make sure DB is ok...
    if (pObjDatabase != nullptr)
    {
        IDX12InstanceBase* pObjMetadata = pObjDatabase->GetMetadataObject(pDeviceChild);

        // Only try to get device if this object hasn't been destroyed...
        if ((pObjMetadata != nullptr) && (pObjMetadata->IsDestroyed() == false))
        {
            HRESULT result = E_FAIL;
            result = pDeviceChild->GetDevice(__uuidof(ID3D12Device), (void**)&pDevice);
            PsAssert(result == S_OK);
        }
    }

    return pDevice;
}

//-----------------------------------------------------------------------------
/// Convert command list type to string.
/// \param inCmdListType The type of Command List to stringify.
/// \returns A string containing the type of Command List given as input.
//-----------------------------------------------------------------------------
std::string CommandListTypeToStr(D3D12_COMMAND_LIST_TYPE inCmdListType)
{
    std::string out = "";

    switch (inCmdListType)
    {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            out = "Direct";
            break;

        case D3D12_COMMAND_LIST_TYPE_BUNDLE:
            out = "Bundle";
            break;

        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            out = "Compute";
            break;

        case D3D12_COMMAND_LIST_TYPE_COPY:
            out = "Copy";
            break;
    }

    return out;
}

}
