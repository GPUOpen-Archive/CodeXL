//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  CommandObject and CommandProcessor classes which are used for
///         registering and processing commands in a tree-like structure
//==============================================================================

#include "CommandProcessor.h"

#include "Logger.h"
#include "misc.h"
#include "xml.h"
#include <stdarg.h>
#include "CommandVisitor.h"
#include "SharedMemoryManager.h"
#include "LayerManager.h"

/// Length of the command buffer
#define COMMAND_MAX_LENGTH 4096

//=============================================================================
///            CommandObject Class
//=============================================================================
//--------------------------------------------------------------------------
/// constructors
//--------------------------------------------------------------------------
CommandObject::CommandObject()
{
    m_requestID = 0;

    m_pCommand = NULL;
    m_pChoppedCommand = NULL;

    m_eContentType = CONTENT_COUNT;
    m_eResponseState = RESPONSE_COUNT;
}

CommandObject::CommandObject(CommunicationID uRequestID, char* pCommand)
{
    SetCommand(pCommand);

    m_requestID = uRequestID;
    m_eContentType = CONTENT_REQUEST;
    m_eResponseState = NO_RESPONSE;
}

//--------------------------------------------------------------------------
/// destructor
//--------------------------------------------------------------------------
CommandObject::~CommandObject()
{
}

//--------------------------------------------------------------------------
/// Checks to the see if the next part of the command is the supplied token.
/// \param pInTok Should be either the ID of a CommandProcessor or the URL
///      of a CommandResponse
/// \return true if the token matches the current position in the command;
///      false otherwise
//--------------------------------------------------------------------------
bool CommandObject::IsCommand(const char* pInTok)
{
    if (m_pChoppedCommand == NULL)
    {
        return false;
    }

    size_t nInTokLen = strlen(pInTok);
    size_t nCurrentCommandLen = strlen(m_pChoppedCommand);

    if (_strnicmp(m_pChoppedCommand, pInTok, nInTokLen) == 0)
    {
        // Check to see if we are going to increment the pointer beyond the end of the array.
        if (nInTokLen >= nCurrentCommandLen)
        {
            if (nInTokLen == nCurrentCommandLen)
            {
                // the entire command was successfully parsed
                m_pChoppedCommand += nInTokLen;
                return true;
            }

            //Log( logERROR, "IsToken: buffer overrun. Str = %s, Tok = %s\n", *sIn, sTok );
            //PsAssert ( !"IsToken: buffer overrun." ) ;
            return false;
        }

        // make sure the next character is either a slash, a question mark, or an equal sign
        // we don't want to skip passed equal signs because they are needed in param parsing
        if (_strnicmp(m_pChoppedCommand + nInTokLen, "/", 1) != 0 &&
            _strnicmp(m_pChoppedCommand + nInTokLen, "?", 1) != 0)
        {
            if (_strnicmp(m_pChoppedCommand + nInTokLen, "=", 1) != 0)
            {
                return false;
            }
        }
        else
        {
            nInTokLen += 1;
        }

        if (nInTokLen > nCurrentCommandLen)
        {
            //Log( logERROR, "IsToken: buffer overrun. Str = %s, Tok = %s\n", *sIn, sTok );
            //PsAssert ( !"IsToken: buffer overrun." ) ;
            return false;
        }
        else
        {
            m_pChoppedCommand += nInTokLen;
        }

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------
/// Parses the params member variable for the specified parameter and, if
/// found sets the supplied variable to parameter value
/// \param pParamName the name of the parameter to search for
/// \param rValue the variable to store the value in
/// \return true if the parameter is found and the value can be extracted;
///      false otherwise
//--------------------------------------------------------------------------
bool CommandObject::GetParam(const char* pParamName, float& rValue)
{
    if (m_pChoppedCommand == NULL)
    {
        return false;
    }

    // interact with a pointer to the remainder of the command because the
    // specified ParamName may or may not be at the beginning of the
    // choppedCommand and we don't want to skip over a different param
    const char* pLoc = m_pChoppedCommand;

    if (pParamName != NULL)
    {
        // if a param name was provided, search for it from the beginning
        pLoc = strstr(m_pChoppedCommand, pParamName);

        if (pLoc == NULL)
        {
            // the param wasn't found, so return false
            return false;
        }

        pLoc += strlen(pParamName);
    }

    // try to extract the desired data type;
    // return true if it was extracted
    // return false it if was not
    if (sscanf_s(pLoc, "=%f", &rValue) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------
bool CommandObject::GetParam(const char* pParamName, bool&  rValue)
{
    if (m_pChoppedCommand == NULL)
    {
        return false;
    }

    const char* pLoc = m_pChoppedCommand;

    if (pParamName != NULL)
    {
        pLoc = strstr(m_pChoppedCommand, pParamName);

        if (pLoc == NULL)
        {
            return false;
        }

        pLoc += strlen(pParamName);
    }

    if (_strnicmp(pLoc, "=TRUE", 5) == 0)
    {
        rValue = true;
        return true;
    }
    else if (_strnicmp(pLoc, "=FALSE", 6) == 0)
    {
        rValue = false;
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------
bool CommandObject::GetParam(const char* pParamName, int&   rValue)
{
    if (m_pChoppedCommand == NULL)
    {
        return false;
    }

    const char* pLoc = m_pChoppedCommand;

    if (pParamName != NULL)
    {
        pLoc = strstr(m_pChoppedCommand, pParamName);

        if (pLoc == NULL)
        {
            return false;
        }

        pLoc += strlen(pParamName);
    }

    if (sscanf_s(pLoc, "=%d", &rValue) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------
bool CommandObject::GetParam(const char* pParamName, unsigned int& rValue)
{
    if (m_pChoppedCommand == NULL)
    {
        return false;
    }

    const char* pLoc = m_pChoppedCommand;

    if (pParamName != NULL)
    {
        pLoc = strstr(m_pChoppedCommand, pParamName);

        if (pLoc == NULL)
        {
            return false;
        }

        pLoc += strlen(pParamName);
    }

    if (sscanf_s(pLoc, "=%u", &rValue) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------
bool CommandObject::GetParam(const char* pParamName, unsigned long& rValue)
{
    if (m_pChoppedCommand == NULL)
    {
        return false;
    }

    const char* pLoc = m_pChoppedCommand;

    if (pParamName != NULL)
    {
        pLoc = strstr(m_pChoppedCommand, pParamName);

        if (pLoc == NULL)
        {
            return false;
        }

        pLoc += strlen(pParamName);
    }

    if (sscanf_s(pLoc, "=%lu", &rValue) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------
bool CommandObject::GetParam(const char* pParamName, std::string& rValue)
{
    if (m_pChoppedCommand == NULL)
    {
        return false;
    }

    const char* pLoc = m_pChoppedCommand;

    if (pParamName != NULL)
    {
        pLoc = strstr(m_pChoppedCommand, pParamName);

        if (pLoc == NULL)
        {
            return false;
        }

        pLoc += strlen(pParamName);
    }

    // Make sure we have an equal sign.
    if (*pLoc != '=')
    {
        return false;
    }

    /// Increment over the equal sign.
    pLoc++;

    // find the location of the next '&'
    const char* pToken = strstr(pLoc, "&");

    if (pToken == NULL)
    {
        // the found param is the last one in the list
        // so set the token to be the end of the string
        pToken = pLoc + strlen(pLoc);
    }

    rValue.assign(pLoc, pToken);

    return true;
}

//--------------------------------------------------------------------------
/// Moves the ChoppedCommand pointer past the parameters of the current cmd
/// to the start of the next command as delimited by the '&' character
/// \return true if there are more commands to process; false otherwise
//--------------------------------------------------------------------------
bool CommandObject::StepToNextCommand()
{
    if (m_pChoppedCommand == NULL)
    {
        return false;
    }

    char* pTok = strstr(m_pChoppedCommand, "&");

    if (pTok == NULL)
    {
        return false;
    }
    else
    {
        m_pChoppedCommand = pTok + 1;
        return true;
    }
}

//--------------------------------------------------------------------------
/// checks to see if there is another command in the current chopped command
/// based on whether or not there is a '&' between the current position and
/// the end of the string. If there are no more '&', then the chopped command
/// is set to NULL so that no further processing of the command string can happen.
/// \return true if there is another command; false otherwise
//--------------------------------------------------------------------------
bool CommandObject::HasAnotherCommand()
{
    if (m_pChoppedCommand == NULL)
    {
        return false;
    }

    char* pTok = strstr(m_pChoppedCommand, "&");

    if (pTok == NULL)
    {
        m_pChoppedCommand = NULL;
        return false;
    }
    else
    {
        return true;
    }
}

//--------------------------------------------------------------------------
/// Sets the current state of the response
/// \param eState new state that the response should have
//--------------------------------------------------------------------------
void CommandObject::SetState(ResponseState eState)
{
    m_eResponseState = eState;
}

//--------------------------------------------------------------------------
/// Accessor to the current response state of the command
/// \return the current response state of the command
//--------------------------------------------------------------------------
ResponseState CommandObject::GetResponseState()
{
    return m_eResponseState;
}

//--------------------------------------------------------------------------
/// Accessor to the CommunicationID
/// \return the CommunicationID
//--------------------------------------------------------------------------
CommunicationID CommandObject::GetCommunicationID() const
{
    return m_requestID;
}

//--------------------------------------------------------------------------
/// Accessor to the remaining command that has not been processed. This is
/// useful for CommandResponses to get any parameters that were passed with
/// the command
/// \return the remaining portion of the command that has not been processed.
//--------------------------------------------------------------------------
const char* CommandObject::GetRemainingCommand() const
{
    return m_pChoppedCommand;
}

//--------------------------------------------------------------------------
/// Sets the Command and ChoppedCommand pointers
/// \param pCommand the command that needs to be processed
//--------------------------------------------------------------------------
void CommandObject::SetCommand(char* pCommand)
{
    PsAssert(pCommand != NULL);
    m_pCommand = pCommand;
    m_pChoppedCommand = pCommand;
}

//=============================================================================
///              CommandResponse class
//=============================================================================

//--------------------------------------------------------------------------
/// constructors
//--------------------------------------------------------------------------
CommandResponse::CommandResponse()
    :   m_eTreeIncludeFlag(INCLUDE),    // Default to send to client.
        m_bNoHashData(false)
{
    m_eResponseState = NO_RESPONSE;
    m_eContentType = CONTENT_COUNT;
    m_pDisplayName = NULL;
    m_pURL = NULL;
    m_requestIDs.clear();
    m_bStreamingEnabled = false;
    m_bEditableContentAutoReply = true;
    m_uParsedParams = 0;

    SetEditableContent(NOT_EDITABLE);
}

//--------------------------------------------------------------------------
/// destructors
//--------------------------------------------------------------------------
CommandResponse::~CommandResponse()
{
}

//--------------------------------------------------------------------------
/// Handle interaction with a CommandVisitor object. The Command visitor is
/// recursing the command tree making copies of all the available commands.
/// \param rVisitor Implements a handler for this CommandResponse
//--------------------------------------------------------------------------
void CommandResponse::Accept(CommandVisitor& rVisitor)
{
    rVisitor.VisitCommandResponse(this);
}

//--------------------------------------------------------------------------
/// Allows the application to see if a request for this Command has been
/// received
/// \return true if the application should respond to this Command; false otherwise
//--------------------------------------------------------------------------
bool CommandResponse::IsActive()
{
    if (m_requestIDs.size() != 0)
    {
        bool bActive = false;

        for (std::list< CommunicationID >::const_iterator iRequestID = m_requestIDs.begin(); iRequestID != m_requestIDs.end(); ++iRequestID)
        {
            // if the response is currently rate limited, then report that it is not active
            // it's not rate limited, report that it is active
            bActive |= !IsResponseRateLimited(*iRequestID);
        }

        return bActive;
    }

    return false;
}

//void CommandResponse::SendZero()
//{
//   Send("0", 1);
//}

//--------------------------------------------------------------------------
/// Allows the application to send a response for this command.
/// If the server is streaming data, setting pData to NULL will closer the connection
/// \param pData the data to respond with,
/// \param uBytes if ContentType is JPEG, specifies the number of bytes in
///      the image; ignored otherwise
//--------------------------------------------------------------------------
void CommandResponse::Send(const char* pData, unsigned int uBytes)
{
    // send the data back to all received requests
    for (std::list< CommunicationID >::const_iterator iRequestID = m_requestIDs.begin(); iRequestID != m_requestIDs.end(); ++iRequestID)
    {
        if (m_bStreamingEnabled == true)
        {
            //server wants to close the connection?
            if (pData == NULL)
            {
                SendResponse(*iRequestID, "", NULL, 0, m_bStreamingEnabled);
                m_bStreamingEnabled = false;
                continue;
            }
        }

        // string which is used to add tags around XML and HTML responses
        std::string strResponse;

        bool bResult = false;

        switch (m_eContentType)
        {
            case CONTENT_XML:
            {
                strResponse += XMLHeader().asCharArray();
                strResponse += "<XML src='";
                strResponse += GetURL();
                strResponse += "'>";
                strResponse += (char*) pData;
                strResponse += "</XML>";
                bResult = SendResponse(*iRequestID, "text/xml", strResponse.c_str(), (unsigned int) strResponse.size(), m_bStreamingEnabled);
                m_eResponseState = SENT_RESPONSE;
                break;
            }

            case CONTENT_HTML:
            {
                strResponse += "<HTML>";
                strResponse += (char*) pData;
                strResponse += "</HTML>";
                bResult = SendResponse(*iRequestID, "text/html", strResponse.c_str(), (unsigned int) strResponse.size(), m_bStreamingEnabled);
                m_eResponseState = SENT_RESPONSE;
                break;
            }

            case CONTENT_TEXT:
            {
                bResult = SendResponse(*iRequestID, "text/plain", (char*) pData, (unsigned int) strlen((char*) pData), m_bStreamingEnabled);
                m_eResponseState = SENT_RESPONSE;
                break;
            }

            case CONTENT_PNG:
            {
                bResult = SendResponse(*iRequestID, "image/png", (char*) pData, uBytes, m_bStreamingEnabled);
                m_eResponseState = SENT_RESPONSE;
                break;
            }

            case CONTENT_JPG:
            {
                bResult = SendResponse(*iRequestID, "image/jpeg", (char*) pData, uBytes, m_bStreamingEnabled);
                m_eResponseState = SENT_RESPONSE;
                break;
            }

            case CONTENT_BMP:
            {
                bResult = SendResponse(*iRequestID, "image/bmp", (char*) pData, uBytes, m_bStreamingEnabled);
                m_eResponseState = SENT_RESPONSE;
                break;
            }

            case CONTENT_DDS:
            {
                bResult = SendResponse(*iRequestID, "application/dds", (char*) pData, uBytes, m_bStreamingEnabled);
                m_eResponseState = SENT_RESPONSE;
                break;
            }

            case CONTENT_PEF:
            {
                bResult = SendResponse(*iRequestID, "bytes/pef", (char*) pData, uBytes, m_bStreamingEnabled);
                m_eResponseState = SENT_RESPONSE;
                break;
            }

            case CONTENT_SCO:
            {
                bResult = SendResponse(*iRequestID, "bytes/sco", (char*) pData, uBytes, m_bStreamingEnabled);
                m_eResponseState = SENT_RESPONSE;
                break;
            }

            case CONTENT_REQUEST:
            default:
            {
                SendError("Attempted to send without setting ContentType");
                m_eResponseState = SENT_RESPONSE;
                break;
            }
        }

        if (bResult == false)
        {
            m_bStreamingEnabled = false;
            m_eResponseState = ERROR_SENDING_RESPONSE;
        }
    }

    if (m_bStreamingEnabled == false)
    {
        // we sent a response and the request was not for streaming data, so unset the request
        m_requestIDs.clear();
    }
}

//--------------------------------------------------------------------------
/// Allows the application to send an error response for this command.
/// These errors also get added to the log
/// \param pError Error message to send
//--------------------------------------------------------------------------
void CommandResponse::SendError(const char* pError, ...)
{
    PsAssert(pError != NULL);

    char errString[ COMMAND_MAX_LENGTH ];
    va_list arg_ptr;

    // we prepend "Error: to the string for the HTTP response - but not for the logfile
    // pLogString tracks the start of the formatted string to pass to the Logfile

    int nLen = sprintf_s(errString, COMMAND_MAX_LENGTH, "Error: ");

    if (nLen < 0)
    {
        Log(logERROR, "String length is less than 0\n");
        return;
    }

    char* pLogString = &errString[nLen];

    va_start(arg_ptr, pError);
    vsprintf_s(pLogString, COMMAND_MAX_LENGTH - nLen, pError, arg_ptr);
    va_end(arg_ptr);

    Log(logERROR, "%s\n", pLogString);

    // send the error back to all received requests
    for (std::list< CommunicationID >::const_iterator iRequestID = m_requestIDs.begin(); iRequestID != m_requestIDs.end(); ++iRequestID)
    {
        bool bResult = SendResponse(*iRequestID, "text/plain", errString, (unsigned int) strlen(errString), m_bStreamingEnabled);

        if (bResult == false)
        {
            Log(logERROR, "Failed to send error to request %u\n", *iRequestID);

            m_bStreamingEnabled = false;
            m_eResponseState = ERROR_SENDING_RESPONSE;
        }
    }

    if (m_bStreamingEnabled == false)
    {
        // we sent a response and the request was not for streaming data, so unset the request
        m_requestIDs.clear();
    }
}

///-------------------------------------------------------------------------
/// \param rCommObj the incoming request that this CommandObject should respond to
///-------------------------------------------------------------------------
void CommandResponse::SetActiveRequest(CommandObject& rCommObj)
{
    // this object knows about itself, but it doesn't know about the request
    // so only copy those pieces of information from the request that are needed
    m_requestIDs.push_back(rCommObj.GetCommunicationID());

    // set as DELAYED_RESPONSE since it is not being responded to immediately
    m_eResponseState = DELAYED_RESPONSE;
}

//--------------------------------------------------------------------------
/// Called by the CommandProcessor to give the CommandResponse a chance to
/// parse all of the command parameters before the app can call IsActive()
/// \param rCommObj incoming command which has the parameters that need to
///     be parsed
/// \return true if the params could be parsed; false otherwise
//--------------------------------------------------------------------------
bool CommandResponse::GetParams(CommandObject& rCommObj)
{
    PS_UNREFERENCED_PARAMETER(rCommObj);

    /// the default CommandResponse has no parameters,
    /// so there is nothing to do here.
    /// this should be overwritten when CommandResponse is inherited
    return true;
}

//--------------------------------------------------------------------------
/// Sets the response type
/// \param eType the response type for this command
//--------------------------------------------------------------------------
void CommandResponse::SetContentType(ContentType eType)
{
    m_eContentType = eType;
}

//--------------------------------------------------------------------------
/// Accessor to the URL.
/// \return returns the URL of this object.
//--------------------------------------------------------------------------
const char* CommandResponse::GetURL() const
{
    return m_pURL;
}

//--------------------------------------------------------------------------
/// Sets the URL to respond to.
/// \param pURL the URL of this object.
//--------------------------------------------------------------------------
void CommandResponse::SetURL(const char* pURL)
{
    PsAssert(pURL != NULL);
    m_pURL = pURL;
}

//--------------------------------------------------------------------------
/// Sets the UI display mode for the item.
/// \param eDisplayMode The display mode (see UIDisplayMode enum).
//--------------------------------------------------------------------------
void CommandResponse::SetUIDisplayMode(UIDisplayMode eDisplayMode)
{
    m_eDisplayMode = eDisplayMode;
}

//--------------------------------------------------------------------------
/// Gets the current display mode. Used to control if and how an item is to be
/// displayed in the ui.
/// \return The current display mode (see UIDisplayMode enum).
//--------------------------------------------------------------------------
UIDisplayMode CommandResponse::GetUIDisplayMode() const
{
    return m_eDisplayMode;
}

//--------------------------------------------------------------------------
/// \param pName the full name of this object
//--------------------------------------------------------------------------
void CommandResponse::SetDisplayName(const char* pName)
{
    PsAssert(pName != NULL);
    m_pDisplayName = pName;
}

//--------------------------------------------------------------------------
/// \return returns the name of this object
//--------------------------------------------------------------------------
const char* CommandResponse::GetDisplayName() const
{
    return m_pDisplayName;
}

//--------------------------------------------------------------------------
/// Sets the current tag name.
/// \param pTagName The tag name.
//--------------------------------------------------------------------------
void CommandResponse::SetTagName(const char* pTagName)
{
    PsAssert(pTagName != NULL);
    m_pTagName = pTagName;
}

//--------------------------------------------------------------------------
/// Gets the current XML tag name.
/// \return The name of the XML tag.
//--------------------------------------------------------------------------
const char* CommandResponse::GetTagName() const
{
    return m_pTagName;
}

//--------------------------------------------------------------------------
/// Sets the editable content type.
/// \param eType the full name of this object
//--------------------------------------------------------------------------
void CommandResponse::SetEditableContent(EditableContent eType)
{
    m_eEditableContent = eType;
}

//--------------------------------------------------------------------------
/// Returns the editable content type: see EditableContent enum
//--------------------------------------------------------------------------
EditableContent CommandResponse::GetEditableContent() const
{
    return m_eEditableContent;
}

//--------------------------------------------------------------------------
/// Returns the editable content value as a string
//--------------------------------------------------------------------------
std::string CommandResponse::GetEditableContentValue()
{
    return "Type Not Handled";
}

//--------------------------------------------------------------------------
/// Sets if the item is to be sent to the client in the command tree.
/// \param eTreeIncludeFlag The include option (see TreeInclude enum)
//--------------------------------------------------------------------------
void CommandResponse::SetTreeInclude(TreeInclude eTreeIncludeFlag)
{
    m_eTreeIncludeFlag = eTreeIncludeFlag ;
}

//--------------------------------------------------------------------------
/// Gets the current tree include flag. Controls if the item is to be sent to
/// the client as part of the command tree.
//--------------------------------------------------------------------------
bool CommandResponse::GetTreeInclude()
{
    if (m_eTreeIncludeFlag == INCLUDE)
    {
        return true;
    }

    return false;
}

//=============================================================================
///              CommandProcessor class
//=============================================================================
//--------------------------------------------------------------------------
/// constructors
//--------------------------------------------------------------------------
CommandProcessor::CommandProcessor()
    :  m_pParent(NULL),
       m_eDisplayMode(DISPLAY)    // Default to show in UI.
{
    m_strTagName = "";
    m_strID = "";
    m_strDisplayName = "";

    AddCommand(CONTENT_XML,  "CommandTree",    "Command Tree",    "CommandTree.xml", NO_DISPLAY, NO_INCLUDE, m_commandTreeResponse);
    AddCommand(CONTENT_XML,  "Settings",       "Settings",        "settings.xml",    NO_DISPLAY, NO_INCLUDE, m_xmlResponse);
    AddCommand(CONTENT_TEXT, "CommandList",    "Command List",    "commands",        NO_DISPLAY, NO_INCLUDE, m_commandListResponse);
}

//--------------------------------------------------------------------------
/// destructors
//--------------------------------------------------------------------------
CommandProcessor::~CommandProcessor()
{
    m_Processors.clear();
    m_Commands.clear();
}

//--------------------------------------------------------------------------
/// Handle interaction with a CommandVisitor object. The Command visitor is
/// recursing the command tree making copies of all the available commands.
/// \param rVisitor Implements a handler for this CommandProcessor
//--------------------------------------------------------------------------
void CommandProcessor::Accept(CommandVisitor& rVisitor)
{
    rVisitor.VisitCommandProcessor(this);
}

//--------------------------------------------------------------------------
/// Processes the specified CommandObject to see if it is targeting any of
/// the added Processors
/// \param rIncomingCommand the incoming command that should be handled
/// \return true if the command could be processed; false otherwise
//--------------------------------------------------------------------------
bool CommandProcessor::ProcessProcessors(CommandObject& rIncomingCommand)
{
    ProcessorList::const_iterator it;

    for (it = m_Processors.begin() ; it < m_Processors.end(); ++it)
    {
        CommandProcessor* pProc = *it;
        PsAssert(pProc != NULL);

        if (rIncomingCommand.IsCommand(pProc->GetID()))
        {
            pProc->Process(rIncomingCommand);
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------
/// Processes the specified CommandObject to see if it should activate any
/// of the added Commands
/// \param rIncomingCommand the incoming command that should be handled
/// \return true if the command could be processed; false otherwise
//--------------------------------------------------------------------------
bool CommandProcessor::ProcessCommands(CommandObject& rIncomingCommand)
{
    CommandList::const_iterator it;

    for (it = m_Commands.begin() ; it < m_Commands.end(); ++it)
    {
        CommandResponse* pComm = *it;
        PsAssert(pComm != NULL);

        if (rIncomingCommand.IsCommand(pComm->GetURL()))
        {
            float dummy;
            pComm->m_bStreamingEnabled = rIncomingCommand.GetParam("Stream", dummy);

            if (pComm->GetParams(rIncomingCommand))
            {
                // skip over the parsed parameters
                pComm->SkipParsedParams(rIncomingCommand);

                // only set the command to be active if
                // - the command is not editable (the client has requested data) or
                // - AutoReply is false (another part of the server wants to react / respond) or
                // - this is the last command and no response has been set yet
                if (pComm->GetEditableContent() == NOT_EDITABLE ||
                    pComm->GetEditableContentAutoReply() == false ||
                    (rIncomingCommand.HasAnotherCommand() == false &&
                     rIncomingCommand.GetResponseState() == NO_RESPONSE))
                {
                    pComm->SetActiveRequest(rIncomingCommand);
                }

                if (rIncomingCommand.HasAnotherCommand() == false)
                {
                    rIncomingCommand.SetState(DELAYED_RESPONSE);

                    if (pComm->GetEditableContent() != NOT_EDITABLE &&
                        rIncomingCommand.GetResponseState() != SENT_RESPONSE)
                    {
                        if (pComm->GetEditableContentAutoReply())
                        {
                            // if this is an editable value,
                            // send back a response
                            pComm->Send("OK");
                            rIncomingCommand.SetState(SENT_RESPONSE);
                        }
                    }
                }

                return true;
            }
            else
            {
                // return variable's value
                if (pComm->GetEditableContent() != NOT_EDITABLE &&
                    rIncomingCommand.GetResponseState() != SENT_RESPONSE &&
                    pComm->GetEditableContentAutoReply()
                   )
                {
                    pComm->SetActiveRequest(rIncomingCommand);
                    rIncomingCommand.SetState(DELAYED_RESPONSE);
                    pComm->Send(pComm->GetEditableContentValue().c_str());
                    rIncomingCommand.SetState(SENT_RESPONSE);
                    return true;
                }

                return false;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------
/// Processes the incoming request to see if it should activate any
/// of the added CommandObjects
/// \param rInCommObj CommandObject that needs to be handled
/// \return true if the CommandObject could be processed; false otherwise
//--------------------------------------------------------------------------
bool CommandProcessor::Process(CommandObject& rInCommObj)
{
    if (ProcessProcessors(rInCommObj))
    {
        return true;
    }

    // process as many of the commands in this processor as
    // can be found in the incomming command
    while (ProcessCommands(rInCommObj) == true)
    {
        // if there are more commands in the incomming command,
        // continue processing, otherwise handle this processors
        // commands and return
        if (rInCommObj.StepToNextCommand() == false)
        {
            // the last of the incomming commands was just processed
            HandleInternalCommands();
            return true;
        }
    }

    // The incomming command matched this processor, but couldn't match
    // and child-processors or any of the commands, so this must be a
    // bad command, report an error
    if (rInCommObj.GetResponseState() == NO_RESPONSE)
    {
        // there was an error activating a command
        CommandResponse response;
        response.SetContentType(CONTENT_TEXT);
        response.SetActiveRequest(rInCommObj);
        response.SendError("Command is invalid starting from: %s", rInCommObj.GetRemainingCommand());

        // need to return true to indicate that something has been sent back to the client
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
/// Handles any internal commands that a CommandProcessor must respond to
//-----------------------------------------------------------------------------
void CommandProcessor::HandleInternalCommands()
{
    if (m_commandTreeResponse.IsActive())
    {

#ifdef USE_C3_STREAMLOG
        StreamLog::Ref() << "\n CommandProcessor: HandleInternalCommands: Enter." << "--------------------------------------------" << "\n";
#endif

        std::string strTree;
        strTree += "<CommandTree>";

#ifdef USE_C3_STREAMLOG
        StreamLog::Ref() << "\n CommandProcessor: About to call GetCommandTree()." << "--------------------------------------------" << "\n";
#endif

        strTree += GetCommandTree();

#ifdef USE_C3_STREAMLOG
        StreamLog::Ref() << "\n CommandProcessor: Finished GetCommandTree()." << "--------------------------------------------" << "\n";
#endif

        strTree += "</CommandTree>";

#ifdef USE_C3_STREAMLOG
        StreamLog::Ref() << "\n CommandProcessor: About to send data back through command." << "--------------------------------------------" << "\n";
#endif

        m_commandTreeResponse.Send(strTree.c_str());
    }

    if (m_xmlResponse.IsActive())
    {
        m_xmlResponse.Send(GetEditableCommandValues().c_str());
    }

    if (m_commandListResponse.IsActive())
    {
        // Create a Visitor object
        CommandVisitor myVisitor;
        // Send the visitor off to fetch all of the available commands
        Accept(myVisitor);
        // Send the commands back to the client.
        m_commandListResponse.Send((char*)(myVisitor.GetCommandStrings()).asCharArray());
    }
}


//--------------------------------------------------------------------------
/// Registers the specified CommandObject
/// \param eType The type of content that this command should be responded with
/// \param pTagName The name to assign to nodes in XML in the CommandTree
/// \param pDisplayName The name to give the CommandResponse object
/// \param pURL The URL that should activate the CommandResponse object
/// \param eDisplayMode Indicates whether or not to display the command in the client
/// \param eIncludeFlag Indicates whether or not to include the command in the CommandTree
/// \param rComObj The CommandResponse object that will be activated when
///      the supplied URL is received as a command
//--------------------------------------------------------------------------
void CommandProcessor::AddCommand(ContentType       eType,
                                  const char*        pTagName,
                                  const char*        pDisplayName,
                                  const char*        pURL,
                                  UIDisplayMode     eDisplayMode,
                                  TreeInclude       eIncludeFlag,
                                  CommandResponse&   rComObj)
{
    PsAssert(pTagName != NULL);
    PsAssert(pDisplayName != NULL);
    PsAssert(pURL != NULL);

    rComObj.SetTagName(pTagName);
    rComObj.SetDisplayName(pDisplayName);
    rComObj.SetURL(pURL);
    rComObj.SetContentType(eType);
    rComObj.SetUIDisplayMode(eDisplayMode);
    rComObj.SetTreeInclude(eIncludeFlag);
    m_Commands.push_back(&rComObj);
}

//--------------------------------------------------------------------------
/// Registers the specified CommandProcessor
/// \param pTagName The XML tag name of the processor.
/// \param pDisplayName The name to display in the client for this command
/// \param pID The ID that should target a command to the CommandProcessor
/// \param pTitlePrefix A customizable prefix that is prepended to the display name
/// \param eDisplayMode Indicates whether or not to display the command in the client
/// \param rComProc The commandProcessor that should be targeted when a
///     command with the supplied ID is received
//--------------------------------------------------------------------------
void CommandProcessor::AddProcessor(const char* pTagName,
                                    const char* pDisplayName,
                                    const char* pID,
                                    const char* pTitlePrefix,
                                    UIDisplayMode eDisplayMode,
                                    CommandProcessor& rComProc)
{
    PsAssert(pTagName != NULL);
    PsAssert(pDisplayName != NULL);
    PsAssert(pID != NULL);

    rComProc.SetTagName(pTagName);
    rComProc.SetID(pID);
    rComProc.SetDisplayName(pDisplayName);
    rComProc.SetTitlePrefix(pTitlePrefix);
    rComProc.SetUIDisplayMode(eDisplayMode);
    rComProc.SetParent(this);
    m_Processors.push_back(&rComProc);
}

//--------------------------------------------------------------------------
/// Unregisters the specified CommandProcessor
/// \param rComProc The command processor object to unregister
//--------------------------------------------------------------------------
void CommandProcessor::RemoveProcessor(CommandProcessor& rComProc)
{
    // keep an interator to point to the one to remove
    ProcessorList::iterator removeIter = m_Processors.end();

    // search through the processors to find the specified one
    for (ProcessorList::iterator procIter = m_Processors.begin(); procIter != m_Processors.end(); ++procIter)
    {
        if (*(procIter) == &rComProc)
        {
            removeIter = procIter;
            break;
        }
    }

    // remove the processor if it was found
    if (removeIter != m_Processors.end())
    {
        m_Processors.erase(removeIter);
    }
}

//--------------------------------------------------------------------------
/// Returns the number of commands that are editable.
/// \return Number of editable commands (0-n).
//--------------------------------------------------------------------------
unsigned int CommandProcessor::GetEditableCount()
{
    unsigned int nCount = 0;
    CommandList::const_iterator objIter;

    for (objIter = m_Commands.begin(); objIter < m_Commands.end(); ++objIter)
    {
        CommandResponse* pObj = *objIter;
        PsAssert(pObj != NULL);

        if (pObj->GetEditableContent() != NOT_EDITABLE)
        {
            nCount++;
        }
    }

    return nCount;
}

//--------------------------------------------------------------------------
/// Allows derived classes to add additional attributes to the XML.
/// \return Empty string in the base class
//--------------------------------------------------------------------------
std::string CommandProcessor::GetDerivedAttributes()
{
    return "";
}

//--------------------------------------------------------------------------
/// Returns the CommandTree as XML based on the contents of this Processor
/// \return XML describing the added commands and Processors
//--------------------------------------------------------------------------
std::string CommandProcessor::GetCommandTree()
{
    std::stringstream strOut;

    // Add the link to the settings. Only add this link if there are editable items.
    if (GetEditableCount() > 0)
    {
        strOut << "<" << GetTagName() << "Settings name='Settings' url='" << GetFullPathString().asCharArray() << "/settings.xml' display='true'>";
        strOut << "</" << GetTagName() << "Settings>";
    }

    // Add the child nodes
    CommandList::const_iterator objIter;

    for (objIter = m_Commands.begin(); objIter < m_Commands.end(); ++objIter)
    {
        CommandResponse* pObj = *objIter;
        PsAssert(pObj != NULL);

        if (pObj->GetTreeInclude())
        {
            strOut << "<" << pObj->GetTagName() << " name='" << pObj->GetDisplayName() << "' url='";

            if (pObj->GetURL() != NULL)
            {
                strOut << GetFullPathString().asCharArray() << "/" << pObj->GetURL();
            }

            strOut << "' ";

            if (pObj->GetEditableContent() != NOT_EDITABLE)
            {
                strOut << "editable='TRUE' ";
            }

            strOut << "display='" << GetUIDisplayModeString(pObj->GetUIDisplayMode()) << "' prefix='";
            strOut << GetTitlePrefix() << "' ></" << pObj->GetTagName() << ">";
        }
    }

    // Add the sub-tree
    std::vector<CommandProcessor*>::const_iterator it;

    for (it = m_Processors.begin() ; it < m_Processors.end(); ++it)
    {
        CommandProcessor* pProc = *it;
        PsAssert(pProc != NULL);

        strOut << "<" << pProc->GetTagName() << " name='" << pProc->GetDisplayName() << "' id='";
        strOut << pProc->GetID() << "' display='" << GetUIDisplayModeString(pProc->GetUIDisplayMode());
        strOut << "' url='" << pProc->GetFullPathString().asCharArray() << "' prefix='" << GetTitlePrefix() << "'";

        std::string derivedAttrs = pProc->GetDerivedAttributes();

        if (!derivedAttrs.empty())
        {
            strOut << " " << derivedAttrs;
        }

        strOut << ">";

        strOut << pProc->GetCommandTree();

        strOut << "</" << pProc->GetTagName() << ">";
    }

    return strOut.str();
}


//--------------------------------------------------------------------------
/// Returns a string bool that can be used by the client to control if item
/// appears in the UI.
/// \param eDisplayMode The mode see UIDisplayMode enum
/// \return The string "TRUE" or "FALSE"
//--------------------------------------------------------------------------
std::string CommandProcessor::GetUIDisplayModeString(UIDisplayMode eDisplayMode)
{
    std::string strRet = "";

    switch (eDisplayMode)
    {
        case DISPLAY:
        {
            strRet = "TRUE";
            break;
        }

        case NO_DISPLAY:
        {
            strRet = "FALSE";
            break;
        }

        default:
            Log(logERROR, "Unknown UIDisplayMode\n");
            strRet = "False";
            break;
    }

    return strRet;
}

//--------------------------------------------------------------------------
/// Returns the values of editable commands
/// \return XML containing the values of editable commands
//--------------------------------------------------------------------------
std::string CommandProcessor::GetEditableCommandValues()
{
    std::stringstream strOut;

    // give derived classes a chance to update / add settings.
    strOut << GetDerivedSettings();

    // now add this processors editable commands (Settings).
    CommandList::const_iterator objIter;

    for (objIter = m_Commands.begin(); objIter < m_Commands.end(); ++objIter)
    {
        CommandResponse* pObj = *objIter;
        PsAssert(pObj != NULL);

        if (pObj->GetEditableContent() != NOT_EDITABLE)
        {
            strOut << "<";
            strOut << pObj->GetTagName();
            strOut << " name='";
            strOut << pObj->GetDisplayName();
            strOut << "' url='";
            strOut << GetFullPathString().asCharArray();
            strOut << "'>";
            strOut << pObj->GetEditableContentValue();
            strOut << "</";
            strOut << pObj->GetTagName();
            strOut << ">";
        }
    }

    return strOut.str();
}

//--------------------------------------------------------------------------
/// Sets the name of this object
/// \param pName The name of this object
//--------------------------------------------------------------------------
void CommandProcessor::SetTagName(const char* pName)
{
    PsAssert(pName != NULL);
    m_strTagName = pName;
}

//--------------------------------------------------------------------------
/// Sets the name of this object
/// \param pName The name of this object
//--------------------------------------------------------------------------
void CommandProcessor::SetDisplayName(const char* pName)
{
    PsAssert(pName != NULL);
    m_strDisplayName = pName;
}


//--------------------------------------------------------------------------
/// Sets the title prefix
/// \param pName The prefix
//--------------------------------------------------------------------------
void CommandProcessor::SetTitlePrefix(const char* pName)
{
    PsAssert(pName != NULL);
    m_strTitlePrefix = pName;
}

//--------------------------------------------------------------------------
/// Sets the UI display mode for the item.
/// \param eDisplayMode The display mode (see UIDisplayMode enum).
//--------------------------------------------------------------------------
void CommandProcessor::SetUIDisplayMode(UIDisplayMode eDisplayMode)
{
    m_eDisplayMode = eDisplayMode;
}

//--------------------------------------------------------------------------
/// Gets the current display mode.
/// \return The current display mode (see UIDisplayMode enum).
//--------------------------------------------------------------------------
UIDisplayMode CommandProcessor::GetUIDisplayMode() const
{
    return m_eDisplayMode;
}

//--------------------------------------------------------------------------
/// Accessor to the name of this Processor
/// \return the name of this processor
//--------------------------------------------------------------------------
const char* CommandProcessor::GetTagName() const
{
    return m_strTagName.asCharArray();
}

//--------------------------------------------------------------------------
/// Accessor to the name of this Processor.
/// \return the name of this processor.
//--------------------------------------------------------------------------
const char* CommandProcessor::GetDisplayName() const
{
    return m_strDisplayName.asCharArray();
}

//--------------------------------------------------------------------------
/// Accessor to the title prefix of this Processor.
/// \return the name of this processor.
//--------------------------------------------------------------------------
const char* CommandProcessor::GetTitlePrefix() const
{
    return m_strTitlePrefix.asCharArray();
}

//--------------------------------------------------------------------------
/// Set the ID that specifies when a command is targeting this processor
/// \param pID The id of this processor
//--------------------------------------------------------------------------
void CommandProcessor::SetID(const char* pID)
{
    PsAssert(pID != NULL);
    m_strID = pID;
}

//--------------------------------------------------------------------------
/// Accessor to the ID that specifies when a command is targeting this
/// processsor.
/// \return the ID of this processsor.
//--------------------------------------------------------------------------
const char* CommandProcessor::GetID() const
{
    return m_strID.asCharArray();
}

//--------------------------------------------------------------------------
/// Set the parent - each command processor knows who its parent it.
/// \param pComProc Pointer to the parent.
//--------------------------------------------------------------------------
void CommandProcessor::SetParent(CommandProcessor* pComProc)
{
    PsAssert(pComProc != NULL) ;
    m_pParent = pComProc;
}

//--------------------------------------------------------------------------
/// Recursive method that accumulates the parent IDs into a full command path.
/// \param strOut Accumulated output string.
//--------------------------------------------------------------------------
void CommandProcessor::AddParentPath(gtASCIIString& strOut)
{
    // If our parent is set.
    if (m_pParent != NULL)
    {
        // Go gets its command path.
        m_pParent->AddParentPath(strOut);
    }

    strOut += m_strID;
    strOut += "/";
}

//--------------------------------------------------------------------------
/// Returns the full path to this item in the command tree.
/// \return The full comamnd path to this item.
//--------------------------------------------------------------------------
gtASCIIString CommandProcessor::GetFullPathString() const
{
    gtASCIIString strPath = "";

    // If our parent is set.
    if (m_pParent != NULL)
    {
        // Go gets its command path.
        m_pParent->AddParentPath(strPath);
    }

    strPath +=  m_strID;

    return strPath;
}
