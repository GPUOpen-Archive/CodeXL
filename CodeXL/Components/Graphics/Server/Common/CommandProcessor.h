//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  File contains the declaration of the CommandObject and CommandProcessor
///         classes which are used for registering and processing commands in a
///         tree-like structure.
//==============================================================================

#ifndef _COMMAND_PROCESSOR_H_
#define _COMMAND_PROCESSOR_H_

#include "ICommunication.h"

#include <stdio.h>
#include <string.h>

#include <list>
#include <vector>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include "Logger.h"
#include "misc.h"

class LayerManager;

using std::string;
using std::vector;

/// The possible response states of a CommandObject and a CommandResponse
enum ResponseState { NO_RESPONSE, DELAYED_RESPONSE, SENT_RESPONSE, ERROR_SENDING_RESPONSE, RESPONSE_COUNT  };

/// The type of content that a CommandResponse is expected to respond with
enum ContentType { CONTENT_HTML, CONTENT_XML, CONTENT_TEXT, CONTENT_PNG, CONTENT_JPG, CONTENT_BMP, CONTENT_DDS, CONTENT_PEF, CONTENT_SCO, CONTENT_REQUEST, CONTENT_COUNT };

/// Indicates what type of editable content (if any) a CommandResponse has
enum EditableContent { NOT_EDITABLE, EDITABLE_TEXT, EDITABLE_BOOL, EDITABLE_INT, EDITABLE_FLOAT, EDITABLE_ULONG, EDITABLE_COUNT };

/// Indicates whether or not to display the command in the client UI
enum UIDisplayMode { DISPLAY, NO_DISPLAY };

/// Indicates whether or not to include the command in the commandtree
enum TreeInclude { INCLUDE, NO_INCLUDE };


/// <summary>
/// Enums for each resource type. We OR them together to create a bitfield that we pass to the server to request teh thumbnail images we want to display.
/// </summary>
enum ResourceImageType
{
    /// <summary>
    /// Not an image
    /// </summary>
    RIT_None = 0x0,

    /// <summary>
    /// IA PreVS image
    /// </summary>
    IA_PreVS = 0x1,

    /// <summary>
    /// VS All images
    /// </summary>
    VS_All = 0x2,

    /// <summary>
    ///  VS Active images
    /// </summary>
    VS_Active = 0x4,

    /// <summary>
    /// HS All images
    /// </summary>
    HS_All = 0x8,

    /// <summary>
    /// HS Active images
    /// </summary>
    HS_Active = 0x10,

    /// <summary>
    /// DS All images
    /// </summary>
    DS_All = 0x20,

    /// <summary>
    /// DS Active images
    /// </summary>
    DS_Active = 0x40,

    /// <summary>
    /// GS All images
    /// </summary>
    GS_All = 0x80,

    /// <summary>
    /// GS Active images
    /// </summary>
    GS_Active = 0x100,

    /// <summary>
    /// PS All images
    /// </summary>
    PS_All = 0x200,

    /// <summary>
    /// PS Active images
    /// </summary>
    PS_Active = 0x400,

    /// <summary>
    /// OM RT images
    /// </summary>
    OM_RT = 0x800,

    /// <summary>
    /// OM Stencil buffer image
    /// </summary>
    OM_SB = 0x1000,

    /// <summary>
    /// OM Depth Buffer image
    /// </summary>
    OM_DB = 0x2000,

    /// <summary>
    /// OM Depth Buffer image
    /// </summary>
    OM_UAV = 0x4000,

    /// <summary>
    /// Swap Chain image
    /// </summary>
    SC_Image = 0x8000,

    /// <summary>
    /// CS All images
    /// </summary>
    CS_All = 0x10000,

    /// <summary>
    /// CS Active images
    /// </summary>
    CS_Active = 0x20000,

    /// <summary>
    /// CS UAV images
    /// </summary>
    CS_UAV = 0x40000
};

/// forward declarations
class CommandObject;
class CommandResponse;
class CommandProcessor;
class BoolCommandResponse;
class IntCommandResponse;
class FloatCommandResponse;
class ULongCommandResponse;
class TextCommandResponse;
class PictureCommandResponse;
class ResourceCommandResponse;
template <class T> class CommandProcessorArray;
class CommandProcessorArrayNode;
class CommandVisitor;

/// List of command responses
typedef vector < CommandResponse* >  CommandList;

/// List of command processors
typedef vector < CommandProcessor* > ProcessorList;

/**
* The CommandObject is responsible for tracking an incoming request as it is parsed by the CommandProcessors
*/
class CommandObject
{
public:

    /// Constructor
    CommandObject();

    /** Constructor
    * @param uRequestID The request ID
    * @param pCommand The command string
    */
    CommandObject(CommunicationID uRequestID, char* pCommand);

    /// Destructor
    virtual ~CommandObject();

private:

    // The following functions are only needed by the CommandProcessor and the CommandResponse (and some derived classes from CommandResponse).

    /// Friend class to provide access to private functions
    friend class CommandProcessor;

    /// Friend class to provide access to private functions
    friend class CommandResponse;

    /// Friend class to provide access to private functions
    friend class BoolCommandResponse;

    /// Friend class to provide access to private functions
    friend class IntCommandResponse;

    /// Friend class to provide access to private functions
    friend class FloatCommandResponse;

    /// Friend class to provide access to private functions
    friend class ULongCommandResponse;

    /// Friend class to provide access to private functions
    friend class TextCommandResponse;

    /// Friend class to provide access to private functions
    friend class PictureCommandResponse;

    /// Friend class to provide access to private functions
    friend class CaptureCommandResponse;

    /// Friend class to provide access to private functions
    friend class ResourceCommandResponse;

    /**
    * Checks to the see if the next part of the command is the supplied token.
    * @param pInTok Should be either the ID of a CommandProcessor or the URL of a CommandResponse
    * @return true if the token matches the current position in the command; false otherwise
    */
    bool IsCommand(const char* pInTok);

    //--------------------------------------------------------------------------
    /// Parses the params member variable for the specified parameter and, if
    /// found sets the supplied variable to parameter value
    /// \param pParamName the name of the parameter to search for
    /// \param rValue the variable to store the value in
    /// \return true if the parameter is found and the value can be extracted; false otherwise
    //--------------------------------------------------------------------------
    bool GetParam(const char* pParamName, float& rValue);

    //--------------------------------------------------------------------------
    /// Parses the params member variable for the specified parameter and, if
    /// found sets the supplied variable to parameter value
    /// \param pParamName the name of the parameter to search for
    /// \param rValue the variable to store the value in
    /// \return true if the parameter is found and the value can be extracted; false otherwise
    //--------------------------------------------------------------------------
    bool GetParam(const char* pParamName, bool&  rValue);

    //--------------------------------------------------------------------------
    /// Parses the params member variable for the specified parameter and, if
    /// found sets the supplied variable to parameter value
    /// \param pParamName the name of the parameter to search for
    /// \param rValue the variable to store the value in
    /// \return true if the parameter is found and the value can be extracted; false otherwise
    //--------------------------------------------------------------------------
    bool GetParam(const char* pParamName, int&   rValue);

    //--------------------------------------------------------------------------
    /// Parses the params member variable for the specified parameter and, if
    /// found sets the supplied variable to parameter value
    /// \param pParamName the name of the parameter to search for
    /// \param rValue the variable to store the value in
    /// \return true if the parameter is found and the value can be extracted; false otherwise
    //--------------------------------------------------------------------------
    bool GetParam(const char* pParamName, unsigned int& rValue);

    //--------------------------------------------------------------------------
    /// Parses the params member variable for the specified parameter and, if
    /// found sets the supplied variable to parameter value
    /// \param pParamName the name of the parameter to search for
    /// \param rValue the variable to store the value in
    /// \return true if the parameter is found and the value can be extracted; false otherwise
    //--------------------------------------------------------------------------
    bool GetParam(const char* pParamName, string& rValue);

    //--------------------------------------------------------------------------
    /// Parses the params member variable for the specified parameter and, if
    /// found sets the supplied variable to parameter value
    /// \param pParamName the name of the parameter to search for
    /// \param rValue the variable to store the value in
    /// \return true if the parameter is found and the value can be extracted; false otherwise
    //--------------------------------------------------------------------------
    bool GetParam(const char* pParamName, unsigned long& rValue);

    /**
    * Moves the ChoppedCommand pointer past the parameters of the current cmd
    * to the start of the next command as delimited by the '&' character.
    * @return true if there are more commands to process; false otherwise
    */
    bool StepToNextCommand();

    /**
    * checks to see if there is another command in the current chopped command
    * based on whether or not there is a '&' between the current position and
    * the end of the string.
    * @return true if there is another command; false otherwise
    */
    bool HasAnotherCommand();

    /**
    * Sets the current state of the response
    * @param eState New state that the response should have
    */
    void SetState(ResponseState eState);

    /**
    * Accessor to the ResponseState
    * @return The response state of this command
    */
    ResponseState GetResponseState();

    /**
    * Accessor to the CommunicationID
    * @return The CommunicationID
    */
    CommunicationID GetCommunicationID() const;

    /**
    * Accessor to the remaining command that has not been processed. This is
    * useful for CommandResponses to get any parameters that were passed with
    * the command
    * @return The remaining portion of the command that has not been processed.
    */
    const char* GetRemainingCommand() const;

    /**
    * Sets the Command and ChoppedCommand pointers
    * @param pCommand the command that needs to be processed
    */
    void SetCommand(char* pCommand);

protected :

private:

    /// The current state of the response to this command
    ResponseState m_eResponseState;

    /** Either the command to be processed (if this is a REQUEST) or
    * the filename / url (if this is a registered CommandObject) **/
    char* m_pCommand;

    /// Increments along m_pCommand as the CommandObject is processed
    char* m_pChoppedCommand;

    /// A requestID that identifies where to send the response to this command
    CommunicationID m_requestID;

    /// The type of content expected for this command
    ContentType m_eContentType;
};


/// Responsible for allowing the application to know when a command for it has been received by a CommandProcessor
/** The CommandResponse class is responsible for allowing the application to
*   know when a command for it has been received by a CommandProcessor and also
*   for allowing a way for the application to query parameters that we sent
*   the command and for providing a means of responding to the command **/
class CommandResponse
{
public:

    //--------------------------------------------------------------------------
    /// constructors
    //--------------------------------------------------------------------------
    CommandResponse();

    //--------------------------------------------------------------------------
    /// destructors
    //--------------------------------------------------------------------------
    virtual ~CommandResponse();

    //--------------------------------------------------------------------------
    /// Allows the application to see if a request for this Command has been
    /// received
    /// \return true if the application should respond to this Command; false otherwise
    //--------------------------------------------------------------------------
    virtual bool IsActive();

    //--------------------------------------------------------------------------
    /// Allows the application to send a response for this command
    /// \param pData the data to respond with
    /// \param uBytes if ContentType is JPEG, specifies the number of bytes in
    ///      the image; ignored otherwise
    //--------------------------------------------------------------------------
    void Send(const char* pData, unsigned int uBytes = 0);

    //--------------------------------------------------------------------------
    /// Sends a zero indicating a failure
    //--------------------------------------------------------------------------
    //void SendZero();

    //--------------------------------------------------------------------------
    /// Allows the application to send an error response for this command.
    /// These errors also get added to the log
    /// \param pError Error message to send
    //--------------------------------------------------------------------------
    void SendError(const char* pError, ...);

    //--------------------------------------------------------------------------
    /// Handle interaction with a CommandVisitor object.
    /// \param rVisitor the visitor object (busy collecting all the commands)
    //--------------------------------------------------------------------------
    void Accept(CommandVisitor& rVisitor);

    //--------------------------------------------------------------------------
    /// Accessor to the URL
    /// \return returns the URL of this object
    //--------------------------------------------------------------------------
    const char* GetURL() const;

    //--------------------------------------------------------------------------
    /// Accessor to the StreamingEnabled
    /// \return returns the StreamingEnabled value
    //--------------------------------------------------------------------------
    bool GetStreamingEnabled() { return m_bStreamingEnabled; }

    //--------------------------------------------------------------------------
    /// Accessor to the type of editable content this CommandResponse has
    /// \return returns the EditableContent type
    //--------------------------------------------------------------------------
    EditableContent GetEditableContent() const;

    //--------------------------------------------------------------------------
    /// Accessor to the value of the editable content as a string
    /// \return returns the value of the editable content as a string
    //--------------------------------------------------------------------------
    virtual std::string GetEditableContentValue();

    //--------------------------------------------------------------------------
    /// Sets Editable Content Auto Reply
    /// If true, OK is sent automatically when an editable command is received.
    /// Default is true.
    /// \param autoReply auto reply state
    //--------------------------------------------------------------------------
    void SetEditableContentAutoReply(bool autoReply)
    {
        m_bEditableContentAutoReply = autoReply;
    }

    //--------------------------------------------------------------------------
    /// Gets Editable Content Auto Reply
    /// If true, OK is sent automatically when an editable command is received.
    /// Default is true.
    /// \return returns the EditableContentAutoReply state
    //--------------------------------------------------------------------------
    bool GetEditableContentAutoReply()
    {
        return m_bEditableContentAutoReply;
    }

    //--------------------------------------------------------------------------
    /// Returns the display mode enum.
    //--------------------------------------------------------------------------
    UIDisplayMode GetUIDisplayMode() const;

    //--------------------------------------------------------------------------
    /// Gets the current XML tag name.
    //--------------------------------------------------------------------------
    const char* GetTagName() const;

    //--------------------------------------------------------------------------
    /// \return returns the name of this object
    //--------------------------------------------------------------------------
    const char* GetDisplayName() const;

    //--------------------------------------------------------------------------
    /// Gets the current tree include flag.
    //--------------------------------------------------------------------------
    bool GetTreeInclude();


    /// Gets the first request.
    /// \return The first request
    void* GetFirstRequest()
    {
        if (m_requestIDs.empty() == false)
        {
            return GetRequestBinary(m_requestIDs.front());
        }

        return NULL;
    }

    /// Gets a bool to indicate if the hash data should be sent or not
    /// \return True or False
    bool NoHashData()
    {
        return m_bNoHashData;
    }

    /// Sets a bool to indicate if the hash data should be sent or not
    /// \param val True or False
    void NoHashData(bool val)
    {
        m_bNoHashData = val;
    }

private:
    /// the following functions are only needed by the CommandProcessor.
    friend class CommandProcessor;

    //--------------------------------------------------------------------------
    /// Called by the CommandProcessor to give the CommandResponse a chance to
    /// parse all of the command parameters before the app can call IsActive()
    /// \param rCommObj incoming command which has the parameters that need to
    ///     be parsed
    /// \return true if the params could be parsed; false otherwise
    //--------------------------------------------------------------------------
    virtual bool GetParams(CommandObject& rCommObj);

    //--------------------------------------------------------------------------
    /// Skips of the params that were parsed with the GetParam(..) member function
    /// \param rCommObj the command object from which the commands were parsed
    //--------------------------------------------------------------------------
    void SkipParsedParams(CommandObject& rCommObj)
    {
        while (m_uParsedParams > 0)
        {
            if (rCommObj.StepToNextCommand() == false)
            {
                // there are no more commands
                return;
            }
        }
    }

    //--------------------------------------------------------------------------
    /// Sets the calling CommandObject as being active and needing to respond
    /// to the supplied request;
    /// \param rCommObj the incoming request that should be responded to
    //--------------------------------------------------------------------------
    void SetActiveRequest(CommandObject& rCommObj);

    //--------------------------------------------------------------------------
    /// Sets the internal state of this CommandResponse
    /// \param eState new internal state
    //--------------------------------------------------------------------------
    void SetState(ResponseState eState);

    //--------------------------------------------------------------------------
    /// Sets the URL to respond to
    /// \param pURL the URL of this object
    //--------------------------------------------------------------------------
    void SetURL(const char* pURL);

    //--------------------------------------------------------------------------
    /// \param pName the full name of this object
    //--------------------------------------------------------------------------
    void SetDisplayName(const char* pDisplayName);

    //--------------------------------------------------------------------------
    /// Sets the UI display mode for the item.
    //--------------------------------------------------------------------------
    void SetUIDisplayMode(UIDisplayMode bDisplayMode);

    //--------------------------------------------------------------------------
    /// Sets the current tag name.
    //--------------------------------------------------------------------------
    void SetTagName(const char* pTagName);

    //--------------------------------------------------------------------------
    /// Sets the response type
    /// \param eType the response type for this command
    //--------------------------------------------------------------------------
    void SetContentType(ContentType eType);

    //--------------------------------------------------------------------------
    /// Sets the current tree include flag.
    //--------------------------------------------------------------------------
    void SetTreeInclude(TreeInclude eTreeIncludeType);

protected:

    //--------------------------------------------------------------------------
    /// Sets the type of editable content that this CommandResponse has
    /// \param eType the response type for this command
    //--------------------------------------------------------------------------
    void SetEditableContent(EditableContent eType);

    //--------------------------------------------------------------------------
    /// Allows sub-classes to access the private GetParam function in the CommandObject
    /// \param rCommObj The command object to get the param from
    /// \param pParamName The name of the parameter that should be retrieved
    /// \param rValue The object to store the parameter value in
    //--------------------------------------------------------------------------
    template <class T> bool GetParam(CommandObject& rCommObj, const char* pParamName, T& rValue)
    {
        if (rCommObj.GetParam(pParamName, rValue))
        {
            m_uParsedParams++;
            return true;
        }

        return false;
    }

protected:

    /// The type of content expected for this command
    ContentType m_eContentType;


private:
    /// The current state of the response to this command
    ResponseState m_eResponseState;

    /// The name to be displayed in the UI
    const char* m_pDisplayName;

    /// The XML tag name for the command
    const char* m_pTagName;

    /// Command that this object is activated by
    const char* m_pURL;

    /// A list of requests that need to be responded to
    std::list< CommunicationID > m_requestIDs;

    /// Indicates if the response to this command should be streamed
    bool m_bStreamingEnabled;

    /// Indicates the type of editable content this CommandResponse has
    EditableContent m_eEditableContent;

    /// Indicates whether or not CommandResponses with editable content should auto reply
    /// when the commands are received
    bool m_bEditableContentAutoReply;

    /// Used to control if an item in the command tree is displayed in the UI.
    UIDisplayMode m_eDisplayMode;

    /// Controls if an item is even sent to the client as part of the command tree.
    TreeInclude m_eTreeIncludeFlag;

    /// Counts the number of params that were parsed so that they can be skipped over
    unsigned int m_uParsedParams;

    bool m_bNoHashData; ///< Flag to indicate if hash data is present.
};

/// A CommandProcessor is responsible for storing other CommandProcessors and CommandObjects
/// (its 'contents') which are listening for incoming requests.
/// The CommandProcessor is responsible for generating CommandTrees
/// based on its contents and for parsing the incoming requests which are
/// based on the CommandTrees.
class CommandProcessor
{
public:

    //--------------------------------------------------------------------------
    /// constructors
    //--------------------------------------------------------------------------
    CommandProcessor();

    //--------------------------------------------------------------------------
    /// destructors
    //--------------------------------------------------------------------------
    ~CommandProcessor();

    //--------------------------------------------------------------------------
    /// Registers the specified CommandObject
    /// \param eType The type of content that this command should be responded with.
    /// \param pTagName The XML tag name of the command.
    /// \param pDisplayName The name to be displayed in the command tree UI.
    /// \param pURL The full path to this command in the command tree.
    /// \param eDisplayMode An attribute to indicate to the client that this item does not need to be displayed in the command tree.
    /// \param eIncludeFlag Used to control if this command is to be included in the command tree or not.
    /// \param rComObj The input command response object.
    //--------------------------------------------------------------------------
    void AddCommand(ContentType eType,
                    const char* pTagName,
                    const char* pDisplayName,
                    const char* pURL,
                    UIDisplayMode eDisplayMode,
                    TreeInclude eIncludeFlag,
                    CommandResponse& rComObj);

    //--------------------------------------------------------------------------
    /// Registers the specified CommandProcessor
    /// \param pTagName The XML tag name of the processor.
    /// \param pDisplayName The name to be displayed in the command tree UI.
    /// \param pID The ID that should target a command to the CommandProcessor.
    /// \param eDisplayMode An attribute to indicate to the client that this item does not need to be displayed in the command tree.
    /// \param rComProc The input command processor object.
    //--------------------------------------------------------------------------
    void AddProcessor(const char* pTagName,
                      const char* pDisplayName,
                      const char* pID,
                      const char* pTitlePrefix,
                      UIDisplayMode eDisplayMode,
                      CommandProcessor& rComProc);

    //--------------------------------------------------------------------------
    /// Unregisters the specified CommandProcessor
    /// \param rComProc The command processor object to remove
    //--------------------------------------------------------------------------
    void RemoveProcessor(CommandProcessor& rComProc);

    //--------------------------------------------------------------------------
    /// Processes the incoming request to see if it should activate any
    /// of the added CommandObjects
    /// \param rInCommObj CommandObject that needs to be handled
    /// \return true if the CommandObject could be processed; false otherwise
    //--------------------------------------------------------------------------
    bool Process(CommandObject& rInCommObj);

    //--------------------------------------------------------------------------
    /// Accessor to the name of this Processor
    /// \return the name of this processor
    //--------------------------------------------------------------------------
    const char* GetTagName() const;

    //--------------------------------------------------------------------------
    /// Accessor to the display name of this Processor
    /// \return The display name of this processor
    //--------------------------------------------------------------------------
    const char* GetDisplayName() const;

    //--------------------------------------------------------------------------
    /// Accessor to the display the prefix that indicates which section of the
    /// pipeline the commands are from (used as a prefix in the client tab windows)
    /// \return The display name of this processor
    //--------------------------------------------------------------------------
    const char* GetTitlePrefix() const;

    //--------------------------------------------------------------------------
    /// Accessor to the DisplayMode
    /// \return the display mode of this processor
    //--------------------------------------------------------------------------
    UIDisplayMode GetUIDisplayMode() const;

    //--------------------------------------------------------------------------
    /// Accessor to the ID that specifies when a command is targeting this
    /// processsor
    /// \return the ID of this processsor
    //--------------------------------------------------------------------------
    const char* GetID() const;

    //--------------------------------------------------------------------------
    /// Handle interaction with a CommandVisitor object.
    /// \param rVisitor the visitor object (busy collecting all the commands)
    //--------------------------------------------------------------------------
    void Accept(CommandVisitor& rVisitor);

    //--------------------------------------------------------------------------
    /// Gets the number of CommandProcessors in this object.
    /// \return The size of the internal vector.
    //--------------------------------------------------------------------------
    size_t ProcessorCount()
    {
        return m_Processors.size();
    }

    //--------------------------------------------------------------------------
    /// Gets the number of Commands in this object.
    /// \return The size of the internal vector.
    //--------------------------------------------------------------------------
    size_t CommandCount()
    {
        return m_Commands.size();
    }

    //--------------------------------------------------------------------------
    /// Returns the CommandProcessor at nIndex
    /// \param nIndex Index of the CommandProcessor
    /// \return Pointer to the CommandProcessor or NULL
    //--------------------------------------------------------------------------
    CommandProcessor* GetCommandProcessor(size_t nIndex)
    {
        if (nIndex < m_Processors.size())
        {
            return m_Processors[nIndex];
        }
        else
        {
            return NULL;
        }
    }

    //--------------------------------------------------------------------------
    /// Returns the CommandResponse at nIndex
    /// \param nIndex Index of the CommandResponse
    /// \return Pointer to the CommandResponse or NULL
    //--------------------------------------------------------------------------
    CommandResponse* GetCommandResponse(size_t nIndex)
    {
        if (nIndex < m_Commands.size())
        {
            return m_Commands[nIndex];
        }
        else
        {
            return NULL;
        }
    }

    //--------------------------------------------------------------------------
    /// Returns the number of commands that are editable.
    //--------------------------------------------------------------------------
    unsigned int GetEditableCount();

    //--------------------------------------------------------------------------
    /// Returns the CommandTree as XML based on the contents of this Processor
    /// \return XML describing the added commands and Processors
    //--------------------------------------------------------------------------
    virtual string GetCommandTree();

protected:

    //--------------------------------------------------------------------------
    /// Returns the values of editable commands
    //--------------------------------------------------------------------------
    string GetEditableCommandValues();

    //--------------------------------------------------------------------------
    /// Returns a string of the full path to this point in the command tree.
    //--------------------------------------------------------------------------
    gtASCIIString GetFullPathString() const;

    //--------------------------------------------------------------------------
    /// Returns a string bool that can be used by the client to control if item
    /// appears in the UI.
    /// \param eDisplayMode The mode see UIDisplayMode enum
    /// \return The string "TRUE" or "FALSE"
    //--------------------------------------------------------------------------
    string GetUIDisplayModeString(UIDisplayMode eDisplayMode);

private:

    //--------------------------------------------------------------------------
    /// Allows derived classes to add additional settings.
    //--------------------------------------------------------------------------
    virtual string GetDerivedSettings() = 0;

    //--------------------------------------------------------------------------
    /// Allows derived classes to add additional attributes to the XML.  The
    /// string returned should be in the format:
    ///    name='value' (single-quotes needed)
    //--------------------------------------------------------------------------
    virtual string GetDerivedAttributes();

    //--------------------------------------------------------------------------
    /// Sets the name of this object
    /// \param pName The name of this object
    //--------------------------------------------------------------------------
    void SetTagName(const char* pName);

    //--------------------------------------------------------------------------
    /// Sets the display name of this object
    //--------------------------------------------------------------------------
    void SetDisplayName(const char* pName);

    //--------------------------------------------------------------------------
    /// Sets the title prefix
    //--------------------------------------------------------------------------
    void SetTitlePrefix(const char* pName);

    //--------------------------------------------------------------------------
    /// Sets of the command is to be displayed in the command tree.
    //--------------------------------------------------------------------------
    void SetUIDisplayMode(UIDisplayMode bDisplayMode);

    //--------------------------------------------------------------------------
    /// Set the ID that specifies when a command is targeting this processor
    /// \param pID The id of this processor
    //--------------------------------------------------------------------------
    void SetID(const char* pID);

    //--------------------------------------------------------------------------
    /// Processes the specified CommandObject to see if it is targeting any of
    /// the added Processors
    /// \param rIncomingCommand the incoming command that should be handled
    /// \return true if the command could be processed; false otherwise
    //--------------------------------------------------------------------------
    bool ProcessProcessors(CommandObject& rIncomingCommand);

    //--------------------------------------------------------------------------
    /// Processes the specified CommandObject to see if it should activate any
    /// of the added Commands
    /// \param rIncomingCommand the incoming command that should be handled
    /// \return true if the command could be processed; false otherwise
    //--------------------------------------------------------------------------
    bool ProcessCommands(CommandObject& rIncomingCommand);

    //--------------------------------------------------------------------------
    /// Handles any internal commands that a CommandProcessor must respond to
    //--------------------------------------------------------------------------
    void HandleInternalCommands();

    //--------------------------------------------------------------------------
    /// Set the parent of this processor. Used to track full path command URLs.
    //--------------------------------------------------------------------------
    void SetParent(CommandProcessor* pComProc);

    //--------------------------------------------------------------------------
    /// Recursive method that accumulates the parent IDs into a full command path.
    //--------------------------------------------------------------------------
    void AddParentPath(gtASCIIString& strOut);

protected:

    /// The list of added Commands
    CommandList m_Commands;

    /// The list of added Processors
    ProcessorList m_Processors;

private:

    /// The name of this object
    gtASCIIString m_strTagName;

    /// The display name of this object
    gtASCIIString m_strDisplayName;

    /// The string that indicates which section of the pipeline the command is from.
    gtASCIIString m_strTitlePrefix;

    /// The ID that indicates that a command is targetting this Processor
    gtASCIIString m_strID;

    /// Pointer to the parent node.
    CommandProcessor* m_pParent;

    /// Attribute to control if the command is to be displayed in the UI command tree.
    UIDisplayMode m_eDisplayMode;

    /// The Response that is activated when a command requests the commandTree
    CommandResponse m_commandTreeResponse;

    /// The Response that is activated when a command requests this processors xml
    CommandResponse m_xmlResponse;

    /// The Response that is activated when a command requests this processors xml
    CommandResponse m_commandListResponse;

};

//=============================================================================
// Special Types of CommandResponses
//=============================================================================

/// A CommandResponse class which sets a bool value (as editable content)
class BoolCommandResponse : public CommandResponse
{
public:
    /// Constructor
    BoolCommandResponse()
    {
        SetEditableContent(EDITABLE_BOOL);
        m_bool = false;
    }

    /// Constructor which sets the default value
    /// \param bValue default value
    BoolCommandResponse(bool bValue)
    {
        SetEditableContent(EDITABLE_BOOL);
        m_bool = bValue;
    }

    bool GetParams(CommandObject& rCommObj)
    {
        return rCommObj.GetParam(NULL, m_bool);
    }

    /// Accessor to the value
    /// \return the value
    bool GetValue()
    {
        return m_bool;
    }

    /// Sets the value
    /// \param bValue the value
    void SetValue(bool bValue)
    {
        m_bool = bValue;
    }

    /// Overloaded equal operator to similify setting the value of this commandResponse
    /// \param bValue the value to assign
    /// \return this object with an updated value
    BoolCommandResponse& operator = (bool bValue)
    {
        SetValue(bValue);
        return *this;
    }

    /// overloaded cast operator to bool
    operator bool()
    {
        return m_bool;
    }

    std::string GetEditableContentValue()
    {
        return ((GetValue() == true) ? "TRUE" : "FALSE");
    }

private:

    /// value of this BoolCommandResponse
    bool m_bool;
};

//=============================================================================
/// A CommandResponse class which sets an int value (as editable content)
class IntCommandResponse : public CommandResponse
{
public:
    /// Constructor
    IntCommandResponse()
    {
        SetEditableContent(EDITABLE_INT);
        m_int = 0;
    }

    /// Constructor which sets the default value
    /// \param nValue default value to assign
    IntCommandResponse(int nValue)
    {
        SetEditableContent(EDITABLE_INT);
        m_int = nValue;
    }

    bool GetParams(CommandObject& rCommObj)
    {
        return rCommObj.GetParam(NULL, m_int);
    }

    /// Accessor to the value of this IntCommandResponse
    /// \return the value
    int GetValue() const
    {
        return m_int;
    }

    /// Allows setting the value
    /// \param nValue the new value to assign
    void SetValue(int nValue)
    {
        m_int = nValue;
    }

    /// Overloaded equal operator to simplify setting the value of this IntCommandResponse
    /// \param nValue the value to assign
    /// \return this object with an updated value
    IntCommandResponse& operator = (int nValue)
    {
        SetValue(nValue);
        return *this;
    }

    /// Overloaded int cast operator
    operator int()
    {
        return m_int;
    }

    std::string GetEditableContentValue()
    {
        return FormatString("%d", GetValue());
    }

private:

    /// the value of this IntCommandResponse
    int m_int;
};

//=============================================================================
/// A CommandResponse class which sets a float value (as editable content)
class FloatCommandResponse : public CommandResponse
{
public:

    /// Constructor
    FloatCommandResponse()
    {
        SetEditableContent(EDITABLE_FLOAT);
        m_float = 0.0f;
    }

    /// Constructor which sets the default value
    /// \param fValue the default value
    FloatCommandResponse(float fValue)
    {
        SetEditableContent(EDITABLE_FLOAT);
        m_float = fValue;
    }

    bool GetParams(CommandObject& rCommObj)
    {
        return rCommObj.GetParam(NULL, m_float);
    }

    /// Accessor to the value of this FloatCommandResponse
    /// \return the value
    float GetValue()
    {
        return m_float;
    }

    /// Sets the value of this FloatCommandResponse
    /// \param fValue the value to assign
    void SetValue(float fValue)
    {
        m_float = fValue;
    }

    /// Overloaded equal operator to simplify setting the value
    /// \param fValue the value to assign
    /// \return this object with an updated value
    FloatCommandResponse& operator = (float fValue)
    {
        SetValue(fValue);
        return *this;
    }

    /// Overloaded float cast operator
    operator float()
    {
        return m_float;
    }

    std::string GetEditableContentValue()
    {
        return FormatString("%f", GetValue());
    }
private:

    /// The value of this FloatCommandResponse
    float m_float;
};

//=============================================================================
/// A CommandResponse class which sets a text value (as editable content)
class TextCommandResponse : public CommandResponse
{
public:
    /// Constructor
    TextCommandResponse()
    {
        SetEditableContent(EDITABLE_TEXT);
        m_string = "";
    }

    /// Constructor which sets the default value
    /// \param pszValue the default value
    TextCommandResponse(const char* pszValue)
    {
        SetEditableContent(EDITABLE_TEXT);
        m_string = pszValue;
    }

    bool GetParams(CommandObject& rCommObj)
    {
        return rCommObj.GetParam(NULL, m_string);
    }

    /// Accessor to the text value
    /// \return pointer to the contained text
    const char* GetValue() const
    {
        return m_string.c_str();
    }

    /// Sets the value of this TextCommandResponse
    /// \param pszValue the value to assign
    void SetValue(const char* pszValue)
    {
        m_string = pszValue;
    }

    /// Overloaded equal operator to simplify assigning this a new value
    /// \param pszValue the value to assign
    /// \return this object with an updated value
    TextCommandResponse& operator = (const char* pszValue)
    {
        SetValue(pszValue);
        return *this;
    }

    /// Overloaded string cast operator
    operator string()
    {
        return m_string;
    }

    std::string GetEditableContentValue()
    {
        return GetValue();
    }

private:

    /// The value of this TextCommandResponse
    string m_string;
};

//=============================================================================
/// A CommandResponse class which sets an unsigned long value (as editable content)
class ULongCommandResponse : public CommandResponse
{
public:
    /// Constructor
    ULongCommandResponse()
    {
        SetEditableContent(EDITABLE_ULONG);
        m_ulong = 0;
    }

    /// Constructor which sets the default value
    ULongCommandResponse(unsigned long ulValue)
    {
        SetEditableContent(EDITABLE_ULONG);
        m_ulong = ulValue;
    }

    bool GetParams(CommandObject& rCommObj)
    {
        return rCommObj.GetParam(NULL, m_ulong);
    }

    /// Accessor to the value
    /// \return the value
    unsigned long GetValue() const
    {
        return m_ulong;
    }

    /// Sets the value
    /// \param ulValue new value to assign
    void SetValue(unsigned long ulValue)
    {
        m_ulong = ulValue;
    }

    /// overloaded equal operator to allow easy setting of this value to an unsigned long
    /// \param ulValue the value to assign this object to
    /// \return reference to this class with an updated value
    ULongCommandResponse& operator = (unsigned long ulValue)
    {
        SetValue(ulValue);
        return *this;
    }

    /// overridden cast operator to an unsigned long
    /// \return this objects value as an unsigned long
    operator unsigned long()
    {
        return m_ulong;
    }

    std::string GetEditableContentValue()
    {
        return FormatString("%lu", GetValue());
    }

private:

    /// Value handled by this command.
    unsigned long m_ulong;
};
//=============================================================================
/// This class is responsible for handling tha sizing of textures that are sent to
/// the client.
class PictureCommandResponse : public CommandResponse
{
public:

    /// Constructor
    PictureCommandResponse()
    {
        m_uWidth = 0;
        m_uHeight = 0;
        m_GPUIndex = 0;
        m_strFormat = "";
    }

    /// Destructor
    virtual ~PictureCommandResponse()
    {
    }

    bool GetParams(CommandObject& rCommObj)
    {
        m_uWidth = 0;
        m_uHeight = 0;
        m_GPUIndex = 0;

        bool bGotWidth = rCommObj.GetParam("width", m_uWidth);
        bool bGotHeight = rCommObj.GetParam("height", m_uHeight);
        rCommObj.GetParam("GPU", m_GPUIndex);
        rCommObj.GetParam("format", m_strFormat);

        // If the user did not specifiy a format then default to png.
        if (m_strFormat.length() == 0)
        {
            m_strFormat = "PNG";
        }

        // Set the MIME content type based on the requested format.
        SetContentTypeByString(m_strFormat);

        if (bGotWidth == false &&
            bGotHeight == false)
        {
            // no params were sent, so the picture will return the
            // full size that is used by the app
            rCommObj.m_pChoppedCommand = NULL;
            return true;
        }

        if (bGotWidth == true &&
            bGotHeight == true)
        {
            // the client specified a width and a height
            rCommObj.m_pChoppedCommand = NULL;
            return true;
        }

        // the client only specified one size, this is invalid
        return false;
    }

    /// Get method for the current width.
    unsigned int GetWidth()
    {
        return m_uWidth;
    }

    /// Get method for the current height.
    unsigned int GetHeight()
    {
        return m_uHeight;
    }

    /// Gets the GPU Index number from the command
    /// \returns The index
    unsigned int GetGPUIndex()
    {
        return m_GPUIndex;
    }

    /// Get method for the current height.
    /// \return String version of the format
    std::string GetFormat()
    {
        return m_strFormat;
    }

    /// Set the string format
    /// \param strFormat String version of the image format e.g "PNG"
    void SetFormat(char* strFormat)
    {
        m_strFormat = strFormat;
    }

    //--------------------------------------------------------------------------
    /// Sets the response type
    /// \param strFormat the response type for this command
    //--------------------------------------------------------------------------
    void SetContentTypeByString(std::string strFormat)
    {
        if (strstr(strFormat.c_str(), "PNG") != NULL)
        {
            m_eContentType = CONTENT_PNG ;
        }
        else if (strstr(strFormat.c_str(), "JPG") != NULL)
        {
            m_eContentType = CONTENT_JPG ;
        }
        else if (strstr(strFormat.c_str(), "DDS") != NULL)
        {
            m_eContentType = CONTENT_DDS ;
        }
        else if (strstr(strFormat.c_str(), "BMP") != NULL)
        {
            m_eContentType = CONTENT_BMP ;
        }
        else if (strstr(strFormat.c_str(), "SCO") != NULL)
        {
            m_eContentType = CONTENT_SCO ;
        }
        else
        {
            // Default to PNG
            m_eContentType = CONTENT_PNG;
        }
    }

private:

    unsigned int m_uWidth;     ///< Requested width of the picture
    unsigned int m_uHeight;    ///< Requested Height of the picture
    std::string m_strFormat;   ///< Specify the return image format.
    unsigned int m_GPUIndex;   ///< The index of the GPU this picture comes from (0 is the first GPU)
};

//=============================================================================
/// This class is responsible for returning a specific resource as a DDS
/// It is used to return a vertex buffer by index or by pointer value
class ResourceCommandResponse : public CommandResponse
{
public:

    /// Constructor
    ResourceCommandResponse()
    {
        m_value = "" ;
        m_eContentType = CONTENT_DDS ;
    }

    /// Destructor
    virtual ~ResourceCommandResponse()
    {
    }

    bool GetParams(CommandObject& rCommObj)
    {
        m_eContentType = CONTENT_DDS ;

        bool bGotValue = rCommObj.GetParam("value", m_value);

        if (bGotValue == true)
        {
            // the client specified a width and a height
            rCommObj.m_pChoppedCommand = NULL;
            return true;
        }

        // the client only specified one size, this is invalid
        return false;
    }

    /// Accessor to the text value
    /// \return pointer to the contained text
    const char* GetValue() const
    {
        return m_value.c_str();
    }

private:

    std::string m_value;     ///< THe value requested by the client (index or pointer)
};


/// Helper CommandResponse for the Stream and NoHashData options to the profile command.
class ProfilerCommandResponse : public CommandResponse
{
public:

    /// Default constructor
    ProfilerCommandResponse():
        m_bStream(false)
    {
    }

    /// Default Destructor
    virtual ~ProfilerCommandResponse()
    {
    }

    /// Accessor the stream value
    /// \return true if the command is to use streaming
    bool Stream()
    {
        return m_bStream;
    }

private:

    /// Parse the Stream and NoHashData options to the profile command.
    /// \param rCommObj The input command
    /// \return True by default
    virtual bool GetParams(CommandObject& rCommObj)
    {
        int value;

        if (GetParam<int>(rCommObj, "Stream", value))
        {
            if (value == 1)
            {
                m_bStream = 1;
            }
        }

        return true;
    }

    /// Storage for the stream state of the command
    bool m_bStream;
};

/// Helper CommandResponse for handling automatic server frame capture
class CaptureCommandResponse : public CommandResponse
{
public:
    /// Default constructor
    CaptureCommandResponse()
        : m_timeOverrideMode(TIME_OVERRIDE_FREEZE)
        , m_filterDrawCalls(0)
        , m_handleMapsOnCPU(false)
        , m_flattenCommandLists(false)
        , m_autoCapture(false)
    {
    }

    /// Default Destructor
    virtual ~CaptureCommandResponse()
    {
    }

    /// Get the setup parameters needed for frame capture
    /// \param rCommObj Set the features based on the input command object.
    /// \return bool indicating if feature is on or off.
    bool GetParams(CommandObject& rCommObj)
    {
        GetParam(rCommObj, "TimeOverrideMode", m_timeOverrideMode);
        GetParam(rCommObj, "HandleMapsOnCPU", m_handleMapsOnCPU);
        GetParam(rCommObj, "FlattenCommandLists", m_flattenCommandLists);
        GetParam(rCommObj, "FilterDrawCalls", m_filterDrawCalls);
        GetParam(rCommObj, "AutoCapture", m_autoCapture);

        return true;
    }

    /// Get the time override method
    /// \return bool indicating if feature is on or off.
    int GetTimeOverrideMode()
    {
        return m_timeOverrideMode;
    }

    // Get method for filtering draw calls
    /// \return bool indicating if feature is on or off.
    int GetFilterDrawCalls()
    {
        return m_filterDrawCalls;
    }

    /// Get method for the handleMapsOnCPU setting
    /// \return bool indicating if feature is on or off.
    bool GetHandleMapsOnCPU()
    {
        return m_handleMapsOnCPU;
    }

    /// Get method for the handleMapsOnCPU setting
    /// \return bool indicating if feature is on or off.
    bool GetFlattenCommandLists()
    {
        return m_flattenCommandLists;
    }

    /// Get method for whether AutoCapture enabled
    /// \return bool indicating if feature is on or off.
    bool GetAutoCapture()
    {
        return m_autoCapture;
    }

private:

    int m_timeOverrideMode;         ///< Client Time Override Mode
    int m_filterDrawCalls;          ///< Client Filter non-Draw/Dispatch calls
    bool m_handleMapsOnCPU;         ///< Client Copy mapped buffers using CPU setting
    bool m_flattenCommandLists;     ///< Client Flatten CommandLists setting
    bool m_autoCapture;             ///< Autocapture mode enabled (pause from server without client attached)
};

/// Helper CommandResponse for handling automatic server frame capture
class StepFrameCommandResponse : public CommandResponse
{
public:
    /// Default constructor
    StepFrameCommandResponse()
        : m_stepCount(0)
        , m_timeOverrideMode(TIME_OVERRIDE_FREEZE)
        , m_filterDrawCalls(0)
        , m_handleMapsOnCPU(false)
        , m_flattenCommandLists(false)
    {
    }

    /// Default Destructor
    virtual ~StepFrameCommandResponse()
    {
    }

    /// get the setup parameters needed for frame capture
    bool GetParams(CommandObject& rCommObj)
    {
        GetParam(rCommObj, "FrameCount", m_stepCount);
        GetParam(rCommObj, "TimeOverrideMode", m_timeOverrideMode);
        GetParam(rCommObj, "HandleMapsOnCPU", m_handleMapsOnCPU);
        GetParam(rCommObj, "FlattenCommandLists", m_flattenCommandLists);
        GetParam(rCommObj, "FilterDrawCalls", m_filterDrawCalls);

        return true;
    }


    /// get method for the number of frames to skip
    int GetStepCount()
    {
        return m_stepCount;
    }

    /// Get method for the time override mode
    int GetTimeOverrideMode()
    {
        return m_timeOverrideMode;
    }

    /// Get method for filtering draw calls
    int GetFilterDrawCalls()
    {
        return m_filterDrawCalls;
    }

    /// Get method for the handleMapsOnCPU setting
    bool GetHandleMapsOnCPU()
    {
        return m_handleMapsOnCPU;
    }

    /// Get method for the handleMapsOnCPU setting
    bool GetFlattenCommandLists()
    {
        return m_flattenCommandLists;
    }

    /// Resets the step count
    void Reset()
    {
        m_stepCount = 0;
    }

private:
    int m_stepCount;                ///< Client Number of frames to advance before next capture
    int m_timeOverrideMode;         ///< Client Time Override Mode
    int m_filterDrawCalls;          ///< Client Filter non-Draw/Dispatch calls
    bool m_handleMapsOnCPU;         ///< Client Copy mapped buffers using CPU setting
    bool m_flattenCommandLists;     ///< Client Flatten CommandLists setting
};

/// Helper CommandResponse for handling captures with modern APIs
class ModernCaptureCommandResponse : public CommandResponse
{

public:

    /// Default constructor
    ModernCaptureCommandResponse()
        : m_captureType(3) // Default to linked trace
        , m_captureCount(1) // default to one frame to capture
    {
    }

    /// Default Destructor
    virtual ~ModernCaptureCommandResponse()
    {
    }

    /// get the setup parameters needed for frame capture
    bool GetParams(CommandObject& rCommObj)
    {
        GetParam(rCommObj, "CaptureCount", m_captureCount);
        GetParam(rCommObj, "CaptureType", m_captureType);
        return true;
    }

    /// Get method for the number frames to capture
    /// \return The number of frames to capture
    int GetCaptureCount()
    {
        return m_captureCount;
    }

    /// Get method for the capture type
    /// \return The capture type
    int GetCaptureType()
    {
        return m_captureType;
    }
 
private:

    int m_captureType;  ///< Capture type
    int m_captureCount; ///< The number of frames to capture
};

#endif //_COMMAND_PROCESSOR_H_
