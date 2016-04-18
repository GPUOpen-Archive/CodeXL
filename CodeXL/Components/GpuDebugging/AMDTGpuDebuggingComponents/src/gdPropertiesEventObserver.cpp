//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdPropertiesEventObserver.cpp
///
//==================================================================================

//------------------------------ gdPropertiesEventObserver.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apMemoryLeakEvent.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLSubBuffer.h>
#include <AMDTAPIClasses/Include/apCLCommandQueue.h>
#include <AMDTAPIClasses/Include/apCLContext.h>
#include <AMDTAPIClasses/Include/apCLEvent.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLPipe.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apCLSampler.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>
#include <AMDTAPIClasses/Include/apGLPipeline.h>
#include <AMDTAPIClasses/Include/apGLSampler.h>
#include <AMDTAPIClasses/Include/apGLRenderContextInfo.h>
#include <AMDTAPIClasses/Include/apGLRenderContextGraphicsInfo.h>
#include <AMDTAPIClasses/Include/apGLShaderObject.h>
#include <AMDTAPIClasses/Include/apGLSync.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTAPIClasses/Include/apPBuffer.h>
#include <AMDTAPIClasses/Include/apStaticBuffer.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeHandler.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>

// Static member initialization:
gdPropertiesEventObserver* gdPropertiesEventObserver::m_spMySingleInstance = NULL;

#define GD_HTML_BG_COLOR QColor(125, 125, 125)

// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::gdPropertiesEventObserver
// Description: Constructor.
// Arguments:   parent - My parent window.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
gdPropertiesEventObserver::gdPropertiesEventObserver()
    : _pLastMemoryLeakEvent(NULL), m_pApplicationCommands(NULL), m_pGDApplicationCommands(NULL),
      m_pPropertiesView(NULL), m_pProgressBar(NULL)
{
    // Get the application commands instance:
    m_pApplicationCommands = afApplicationCommands::instance();
    m_pGDApplicationCommands = gdApplicationCommands::gdInstance();

    m_pPropertiesView = m_pApplicationCommands->propertiesView();

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}

// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::instance
// Description: Returns the singleton instance of this class
// Return Val:  gdPropertiesEventObserver&
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
gdPropertiesEventObserver& gdPropertiesEventObserver::instance()
{
    if (NULL == m_spMySingleInstance)
    {
        m_spMySingleInstance = new gdPropertiesEventObserver;

    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::~gdPropertiesEventObserver
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
gdPropertiesEventObserver::~gdPropertiesEventObserver()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::clearView
// Description: Clears the properties view
// Author:      Eran Zinman
// Date:        15/5/2007
// ---------------------------------------------------------------------------
void gdPropertiesEventObserver::clearView()
{
    gtString startDebuggingPropertiesPage;
    afHTMLContent htmlContent;
    buildProcessStopString(htmlContent);
    htmlContent.toString(startDebuggingPropertiesPage);
    setPropertiesViewInfo();

    GT_IF_WITH_ASSERT(m_pPropertiesView != NULL)
    {
        m_pPropertiesView->setText(acGTStringToQString(startDebuggingPropertiesPage));
    }
}

// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   eve - The debugged process event.
// Author:      Yaki Tebeka
// Date:        4/4/2004
// ---------------------------------------------------------------------------
void gdPropertiesEventObserver::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            gtString viewsInfoPropertiesPage;
            afHTMLContent htmlContent;
            gdHTMLProperties htmlBuilder;
            htmlBuilder.buildProcessRunResumedMessage(htmlContent);

            htmlContent.toString(viewsInfoPropertiesPage);

            // When a new debugged process is created - clear the properties view:
            setPropertiesFromText(acGTStringToQString(viewsInfoPropertiesPage));
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            gtString startDebuggingPropertiesPage;
            afHTMLContent htmlContent;

            buildProcessStopString(htmlContent);
            htmlContent.toString(startDebuggingPropertiesPage);

            // When a new debugged process is created - clear the properties view:
            setPropertiesFromText(acGTStringToQString(startDebuggingPropertiesPage));
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            gtString processCreationFailurePropertiesString;
            afHTMLContent htmlContent;

            buildProcessStopString(htmlContent);
            htmlContent.toString(processCreationFailurePropertiesString);

            // When a new debugged process is created - clear the properties view:
            setPropertiesFromText(acGTStringToQString(processCreationFailurePropertiesString));
        }
        break;

        case apEvent::AP_MEMORY_LEAK:
        {
            // Save the last memory leak event:
            const apMemoryLeakEvent& memoryLeakEvent = (const apMemoryLeakEvent&)eve;
            _pLastMemoryLeakEvent = (apMemoryLeakEvent*)memoryLeakEvent.clone();
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        {
            // Display the properties of the breakpoint;
            onBreakpointHit(eve);
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT:
        {
            // Get the activation event:
            const apMonitoredObjectsTreeActivatedEvent& activationEvent = (const apMonitoredObjectsTreeActivatedEvent&)eve;

            // Get the item data;
            afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)activationEvent.selectedItemData();

            if (pItemData != NULL)
            {
                // Display the item:
                displayItemProperties(pItemData, false, true, true);
            }
        }
        break;

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            // If there is a new project, set the root name:
            const afGlobalVariableChangedEvent& variableChangedEvent = (const afGlobalVariableChangedEvent&)eve;

            if (variableChangedEvent.changedVariableId() == afGlobalVariableChangedEvent::CURRENT_PROJECT)
            {
                // Set the project loaded string:
                gtString htmlString;
                afHTMLContent htmlContent;
                buildProcessStopString(htmlContent);
                htmlContent.toString(htmlString);
                setPropertiesFromText(acGTStringToQString(htmlString));
            }
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT:
        {
            // Get the activation event:
            const apMonitoredObjectsTreeSelectedEvent& selectionEvent = (const apMonitoredObjectsTreeSelectedEvent&)eve;

            // Get the item data;
            afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)selectionEvent.selectedItemData();

            if (pItemData != NULL)
            {
                // Display the item:
                displayItemProperties(pItemData, false, true, true);
            }
        }
        break;

        default:
            // Do nothing...
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::handleURL
// Description: Overrides gdPropertiesEventObserver function
// Arguments:   QUrl& link
// Author:      Sigal Algranaty
// Date:        13/1/2011
// ---------------------------------------------------------------------------
void gdPropertiesEventObserver::handleURL(const QUrl& link)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pGDApplicationCommands != NULL)
    {
        // Check if the debugged process is suspended:
        bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

        if (!isDebuggedProcessSuspended)
        {
            m_pGDApplicationCommands->showMessageBox(AF_STR_InformationA, GD_STR_PropertiesViewLinkAvailableOnProcessSuspensionMessage);
        }

        // Get the pressed link url (which holds the object name and type):
        gtString linkHref;
        linkHref.fromASCIIString(link.toString().toLatin1());

        if (!linkHref.isEmpty() && isDebuggedProcessSuspended)
        {
            // Get the item identification as item data:
            afApplicationTreeItemData objectID;
            gdDebugApplicationTreeData* pGDData = new gdDebugApplicationTreeData;

            objectID.setExtendedData(pGDData);
            int additionalParameter = -1;
            bool rc = gdHTMLProperties::htmlLinkToObjectDetails(linkHref, objectID, additionalParameter);
            GT_IF_WITH_ASSERT(rc)
            {
                bool rcObjectDisplayed = false;

                if (objectID.m_itemType == AF_TREE_ITEM_CL_DEVICE ||
                    objectID.m_itemType == AF_TREE_ITEM_CL_PLATFORM)
                {
                    // Extract the current project's settings.
                    afProjectManager& theProjManager = afProjectManager::instance();
                    const apProjectSettings& theProjSettings = theProjManager.currentProjectSettings();

                    // Handle the request only in local sessions.
                    if (!theProjSettings.isRemoteTarget())
                    {
                        // Display the system information.
                        rcObjectDisplayed = displaySystemInformationItem(objectID);
                    }
                    else
                    {
                        // Notify the user that currently system information is only available on local sessions.
                        m_pGDApplicationCommands->showMessageBox(AF_STR_REMOTE_PLATFORM_INFO_UNAVAILABLE_TITLE,
                                                                 AF_STR_REMOTE_PLATFORM_INFO_UNAVAILABLE_BODY);
                    }
                }
                else
                {
                    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
                    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
                    {
                        // Get the monitored objects tree:
                        afApplicationTree* pObjectsTree = pApplicationCommands->applicationTree();
                        GT_IF_WITH_ASSERT(pObjectsTree != NULL)
                        {
                            // Get the matching item data for the requested object:
                            rcObjectDisplayed = pObjectsTree->selectItem(&objectID, true);
                        }
                    }
                }

                // Make sure that the item was displayed:
                GT_ASSERT(rcObjectDisplayed);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::displayItemProperties
// Description: Display a monitored item
// Arguments:   const gdDebugApplicationTreeData* pItemData
//              displayExtendedInformation - should the item extended information be
//              displayed? currently, only for texture parameters
//              displayThumbnail - should the object thumbnail be displayed?
// Author:      Sigal Algranaty
// Date:        14/10/2010
// ---------------------------------------------------------------------------
bool gdPropertiesEventObserver::displayItemProperties(const afApplicationTreeItemData* pItemData, bool displayItemChildrenAmount, bool displayThumbnail, bool displayExtendedInformation)
{
    bool retVal = true;

    afHTMLContent htmlContent;
    retVal = BuildItemHTMLProperties(pItemData, displayItemChildrenAmount, displayThumbnail, displayExtendedInformation, htmlContent);

    if (retVal && (htmlContent.size() > 0))
    {
        gtString propertiesHTMLString;

        // Get the HTML as string:
        htmlContent.toString(propertiesHTMLString);

        // Set the HTML string:
        if (!propertiesHTMLString.isEmpty())
        {
            setPropertiesFromText(acGTStringToQString(propertiesHTMLString));
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::BuildItemHTMLProperties
// Description: Display a monitored item
// Arguments:   const gdDebugApplicationTreeData* pItemData
//              displayExtendedInformation - should the item extended information be
//              displayed? currently, only for texture parameters
//              displayThumbnail - should the object thumbnail be displayed?
// Author:      Sigal Algranaty
// Date:        14/10/2010
// ---------------------------------------------------------------------------
bool gdPropertiesEventObserver::BuildItemHTMLProperties(const afApplicationTreeItemData* pItemData, bool displayItemChildrenAmount, bool displayThumbnail, bool displayExtendedInformation, afHTMLContent& htmlContent)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());

        if (pGDData != NULL)
        {
            retVal = true;

            setPropertiesViewInfo();

            // Get the item type:
            afTreeItemType propertiesItemType = pItemData->m_itemType;

            // Get the amount of item children:
            int amountOfItemChildren = (displayItemChildrenAmount) ? pItemData->m_objectCount : -1;

            switch (propertiesItemType)
            {
                case AF_TREE_ITEM_APP_ROOT:
                {
                    // Check if this is information to take:
                    gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
                    apDebugProjectSettings creationData = globalVarsManager.currentDebugProjectSettings();

                    // Add the debugged application's full path:
                    gtString appFullPath = creationData.executablePath().asString();

                    if (!appFullPath.isEmpty())
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildDebuggedApplicationHTMLPropertiesString(htmlContent);
                    }
                    else
                    {
                        buildProcessStopString(htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_RENDER_CONTEXT:
                {
                    // Get all the context information:
                    apGLRenderContextInfo contextInfo;
                    bool rcInfo = gaGetRenderContextDetails(pGDData->_contextId._contextId, contextInfo);

                    // Get the render context graphic info:
                    apGLRenderContextGraphicsInfo contextGraphicsInfo;
                    bool rcGraph = gaGetRenderContextGraphicDetails(pGDData->_contextId._contextId, contextGraphicsInfo);
                    GT_IF_WITH_ASSERT(rcInfo && rcGraph)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildOpenGLContextHTMLPropertiesString(pGDData->_contextId, contextInfo, contextGraphicsInfo, false, htmlContent);
                        retVal = true;
                    }
                }
                break;

                case AF_TREE_ITEM_CL_CONTEXT:
                {
                    apCLContext contextInfo;
                    bool rcCtxInfo = gaGetOpenCLContextDetails(pGDData->_contextId._contextId, contextInfo);
                    GT_IF_WITH_ASSERT(rcCtxInfo)
                    {
                        retVal = true;
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildOpenCLContextHTMLPropertiesString(pGDData->_contextId, contextInfo, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_TEXTURES_NODE:
                {
                    int amountOfTextureTypes = pGDData->_textureTypesAmount.size();

                    if (amountOfTextureTypes > 0)
                    {
                        // This is the "parent" item:
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildTextureTypesHTMLPropertiesString(pGDData->_contextId, pGDData->_textureTypesAmount, htmlContent);
                    }
                    else
                    {
                        // This is a child item (ie we are looking at the render context):
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildTexturesListHTMLPropertiesString(pGDData->_contextId, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_TEXTURE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    apGLTextureMipLevelID textureId;
                    textureId._textureName = pGDData->_objectOpenGLName;
                    textureId._textureMipLevel = 0;
                    htmlPropsCreator.buildTextureHTMLPropertiesString(pGDData->_contextId, textureId, htmlContent, displayExtendedInformation, displayThumbnail, m_pProgressBar);
                }
                break;

                case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildRenderBuffersListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_GL_RENDER_BUFFER:
                {
                    // Get the render buffer details:
                    apGLRenderBuffer renderBufferDetails(0);
                    bool rc = gaGetRenderBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, renderBufferDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildRenderBufferHTMLPropertiesString(pGDData->_contextId, renderBufferDetails, displayThumbnail, m_pProgressBar, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_PROGRAM_PIPELINES_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildPipelinesListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_GL_PROGRAM_PIPELINE:
                {
                    // Get the pipeline details:
                    apGLPipeline pipelineDetails;
                    bool rc = gaGetPipelineObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, pipelineDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildPipelinesHTMLPropertiesString(pGDData->_contextId, pipelineDetails, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_SAMPLERS_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildOpenGlSamplersListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_GL_SAMPLER:
                {
                    // Get the sampler's details:
                    apGLSampler samplerDetails;
                    bool rc = gaGetSamplerObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, samplerDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildOpenGlSamplersHTMLPropertiesString(samplerDetails, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_PBUFFERS_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildPBuffersListHTMLPropertiesString(htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_GL_PBUFFER_NODE:
                {
                    int bufferID = (int)pGDData->_objectOpenGLName;
                    apPBuffer pbufferDetails;
                    bool rcPBO = gaGetPBufferObjectDetails(bufferID, pbufferDetails);
                    GT_IF_WITH_ASSERT(rcPBO)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildPBufferHTMLPropertiesString(bufferID, pbufferDetails, htmlContent, false);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildStaticBuffersListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_GL_STATIC_BUFFER:
                {
                    // Get the render buffer details:
                    apStaticBuffer staticBufferDetails;
                    bool rc = gaGetStaticBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_bufferType, staticBufferDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildStaticBufferHTMLPropertiesString(pGDData->_contextId, staticBufferDetails, displayThumbnail, m_pProgressBar, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_VBO_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildVBOsListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_GL_VBO:
                {
                    // Get the render buffer details:
                    apGLVBO vboDetails;
                    bool rc = gaGetVBODetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, vboDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        GLenum vboLastAttachment = 0;
                        gtVector<GLenum> vboCurrentAttachments;
                        rc = gaGetVBOAttachment(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, vboLastAttachment, vboCurrentAttachments);
                        GT_ASSERT(rc);

                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildVBOHTMLPropertiesString(pGDData->_contextId, vboDetails, vboLastAttachment, vboCurrentAttachments, htmlContent, true);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_SYNC_OBJECTS_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildSyncListHTMLPropertiesString(amountOfItemChildren, htmlContent);
                }
                break;

                case AF_TREE_ITEM_GL_SYNC_OBJECT:
                {
                    // Get the sync object details:
                    apGLSync syncObjectDetails;
                    bool rc = gaGetSyncObjectDetails(pGDData->_syncIndex, syncObjectDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildSyncObjectHTMLPropertiesString(syncObjectDetails, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_PROGRAMS_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildProgramsListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_GL_PROGRAM:
                {
                    apGLProgram programDetails;
                    bool rc = gaGetProgramObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, programDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildProgramHTMLPropertiesString(pGDData->_contextId._contextId, programDetails, true, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_SHADERS_NODE:
                {
                    // Build the shaders root HTML string:
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildShadersListHTMLPropertiesString(pGDData->_contextId, htmlContent, pGDData->_shaderTypesAmount);
                }
                break;

                case AF_TREE_ITEM_GL_VERTEX_SHADER:
                case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
                case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
                case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
                case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
                case AF_TREE_ITEM_GL_COMPUTE_SHADER:
                case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
                {
                    gtAutoPtr<apGLShaderObject> aptrShaderDetails = NULL;
                    bool rc = gaGetShaderObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, aptrShaderDetails);
                    GT_IF_WITH_ASSERT(rc && (aptrShaderDetails.pointedObject() != NULL))
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildShaderHTMLPropertiesString(pGDData->_contextId, *aptrShaderDetails, false, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_DISPLAY_LISTS_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildDisplayListsListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_GL_DISPLAY_LIST:
                {
                    apGLDisplayList displayListDetails;
                    bool rc = gaGetDisplayListObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, displayListDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildDisplayListHTMLPropertiesString(pGDData->_contextId, displayListDetails, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_FBO_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildFBOsListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_GL_FBO:
                {
                    apGLFBO fboDetails;
                    bool rc = gaGetFBODetails(pGDData->_contextId._contextId, pGDData->_objectOpenGLName, fboDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildFBOHTMLPropertiesString(pGDData->_contextId, fboDetails, true, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_CL_IMAGE:
                {
                    // Build the texture HTML message:
                    gdHTMLProperties htmlPropsCreator;
                    bool isInteropObject = (pGDData->_objectOpenGLName > 0);
                    bool shouldCreateThumb = displayThumbnail && !isInteropObject;
                    htmlPropsCreator.buildOpenCLImageHTMLPropertiesString(pGDData->_contextId, pGDData->_objectOpenCLIndex, shouldCreateThumb, htmlContent, m_pProgressBar);
                }
                break;

                case AF_TREE_ITEM_CL_IMAGES_NODE:
                {
                    if (pGDData->_textureTypesAmount.size() > 0)
                    {
                        // This is the "parent" item:
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildTextureTypesHTMLPropertiesString(pGDData->_contextId, pGDData->_textureTypesAmount, htmlContent);
                    }
                    else
                    {
                        // This is a child item (ie we are looking at the render context:
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildTexturesListHTMLPropertiesString(pGDData->_contextId, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_CL_BUFFERS_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildOpenCLBuffersListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_CL_BUFFER:
                {
                    apCLBuffer bufferDetails;
                    bool rc = gaGetOpenCLBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, bufferDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildOpenCLBufferHTMLPropertiesString(pGDData->_contextId, bufferDetails, pGDData->_objectOpenCLIndex, true, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_CL_SUB_BUFFER:
                {
                    apCLSubBuffer subBufferDetails;
                    bool rc = gaGetOpenCLSubBufferObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, subBufferDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildOpenCLSubBufferHTMLPropertiesString(pGDData->_contextId, subBufferDetails, true, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_CL_PIPES_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildOpenCLPipesListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_CL_PIPE:
                {
                    apCLPipe pipeDetails;
                    bool rc = gaGetOpenCLPipeObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, pipeDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildOpenCLPipeHTMLPropertiesString(pGDData->_contextId, pipeDetails, pGDData->_objectOpenCLIndex, true, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_CL_COMMAND_QUEUE:
                {
                    apCLCommandQueue commandQueueDetails;
                    bool rc = gaGetCommandQueueDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, commandQueueDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildOpenCLCommandQueueHTMLPropertiesString(pGDData->_contextId, commandQueueDetails, pGDData->_objectOpenCLName, true, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_CL_COMMAND_QUEUES_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildOpenCLCommandQueuesListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_CL_PROGRAM:
                {
                    apCLProgram programDetails(0);
                    bool rc = gaGetOpenCLProgramObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, programDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildOpenCLProgramHTMLPropertiesString(pGDData->_contextId._contextId, programDetails, pGDData->_objectOpenCLName, true, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_CL_PROGRAMS_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildOpenCLProgramsListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_CL_KERNEL:
                {
                    apCLKernel kernelDetails(OA_CL_NULL_HANDLE, 0, OA_CL_NULL_HANDLE, L"");
                    bool rc = gaGetOpenCLKernelObjectDetails(pGDData->_contextId._contextId, pGDData->_clKernelHandle, kernelDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildOpenCLKernelHTMLPropertiesString(pGDData->_contextId._contextId, kernelDetails, pGDData->_objectOpenCLName, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_CL_SAMPLER:
                {
                    apCLSampler samplerDetails;
                    bool rc = gaGetOpenCLSamplerObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, samplerDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildOpenCLSamplerHTMLPropertiesString(pGDData->_contextId._contextId, samplerDetails, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_CL_SAMPLERS_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildOpenCLSamplersListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_CL_EVENT:
                {
                    apCLEvent eventDetails(OA_CL_NULL_HANDLE, OA_CL_NULL_HANDLE, false);
                    bool rc = gaGetOpenCLEventObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, eventDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        gdHTMLProperties htmlPropsCreator;
                        htmlPropsCreator.buildOpenCLEventHTMLPropertiesString(pGDData->_contextId._contextId, pGDData->_objectOpenCLName, eventDetails, htmlContent);
                    }
                }
                break;

                case AF_TREE_ITEM_CL_EVENTS_NODE:
                {
                    gdHTMLProperties htmlPropsCreator;
                    htmlPropsCreator.buildOpenCLEventsListHTMLPropertiesString(pGDData->_contextId, htmlContent, amountOfItemChildren);
                }
                break;

                case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
                case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
                {
                    // Get the monitored objects tree:
                    gdDebugApplicationTreeHandler* pMonitoredObjectsTree = gdDebugApplicationTreeHandler::instance();
                    GT_IF_WITH_ASSERT(pMonitoredObjectsTree != NULL)
                    {
                        // Get the original item data:
                        afApplicationTreeItemData* pOriginalItemData = (afApplicationTreeItemData*)(pMonitoredObjectsTree->getTreeItemData(pItemData->m_pOriginalItemTreeItem));
                        GT_IF_WITH_ASSERT(pOriginalItemData != NULL)
                        {
                            // Display the item properties:
                            retVal = displayItemProperties(pOriginalItemData, false, true, true);
                        }
                    }
                }
                break;

                default:
                {
                    // Do not assert for types that were not found:
                    retVal = false;
                }
                break;
            }

            if (htmlContent.size() > 0)
            {
                // Set the HTML string:
                gtString propertiesHTMLString;
                htmlContent.toString(propertiesHTMLString);

                if (!propertiesHTMLString.isEmpty())
                {
                    setPropertiesFromText(acGTStringToQString(propertiesHTMLString));
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::buildProjectNotLoadedString
// Description: Build a string stating that there is no project loaded
// Arguments:   gtString& propertiesInfo
// Author:      Sigal Algranaty
// Date:        26/3/2012
// ---------------------------------------------------------------------------
void gdPropertiesEventObserver::buildProjectNotLoadedString(gtString& propertiesInfo)
{
    afHTMLContent htmlContent(AF_STR_PropertiesProjectNotLoaded);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, AF_STR_PropertiesViewLoadProjectComment);
    htmlContent.toString(propertiesInfo);
}

// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::buildProcessStopString
// Description: create a string for stop process information
// Author:      Gilad Yarnitzky
// Date:        12/5/2011
// ---------------------------------------------------------------------------
void gdPropertiesEventObserver::buildProcessStopString(afHTMLContent& htmlContent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pGDApplicationCommands != NULL)
    {
        m_pGDApplicationCommands->buildProcessStopString(htmlContent);

    }
}

// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::displaySystemInformationItem
// Description: Display an item in the system information dialog
// Arguments:   const gdDebugApplicationTreeData& objectID
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/7/2011
// ---------------------------------------------------------------------------
bool gdPropertiesEventObserver::displaySystemInformationItem(const afApplicationTreeItemData& objectID)
{
    bool retVal = true;
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pApplicationCommands != NULL)
    {

        if (objectID.m_itemType == AF_TREE_ITEM_CL_DEVICE)
        {
            // Open the system information dialog with the requested device selected:
            m_pApplicationCommands->onToolsSystemInfo(afSystemInformationDialog::SYS_INFO_OPENCL_DEVICES);
        }
        else if (objectID.m_itemType == AF_TREE_ITEM_CL_PLATFORM)
        {
            // Open the system information dialog with the requested platform selected:
            m_pApplicationCommands->onToolsSystemInfo(afSystemInformationDialog::SYS_INFO_OPENCL_PLATFORMS);
        }
        else
        {
            // Unsupported object:
            retVal = false;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::onBreakpointHit
// Description: Display the breakpoint properties
// Arguments:   const apEvent& eve
// Author:      Sigal Algranaty
// Date:        14/3/2012
// ---------------------------------------------------------------------------
void gdPropertiesEventObserver::onBreakpointHit(const apEvent& eve)
{
    const apBreakpointHitEvent& breakpointEvent = (const apBreakpointHitEvent&)eve;

    if (breakpointEvent.breakReason() == AP_MEMORY_LEAK_BREAKPOINT_HIT)
    {
        GT_IF_WITH_ASSERT(_pLastMemoryLeakEvent != NULL)
        {
            // Display a properties message:
            gtString propertiesMessage;
            gdHTMLProperties htmlPropsCreator;
            afHTMLContent htmlContent;
            htmlPropsCreator.buildMemoryLeakEventHTMLPropertiesString(*_pLastMemoryLeakEvent, true, htmlContent);
            htmlContent.toString(propertiesMessage);
            setPropertiesFromText(acGTStringToQString(propertiesMessage));
        }
    }
    else
    {
        // Get the current breakpoint function details:
        gtString funcName, funcArgs;
        bool rc = gdGetCurrentBreakpointFunction(breakpointEvent.breakedOnFunctionCall(), funcName, funcArgs);
        apExecutionMode currentExecMode = AP_DEBUGGING_MODE;
        rc = rc && gaGetDebuggedProcessExecutionMode(currentExecMode);

        if (currentExecMode == AP_PROFILING_MODE)
        {
            // Profiling mode
            funcArgs.makeEmpty();
        }

        gdHTMLProperties htmlBuilder;
        gtString htmlPropertiesString;
        afHTMLContent htmlContent;

        htmlBuilder.buildBreakpointPropertiesString(funcName, funcArgs, breakpointEvent, htmlContent);
        htmlContent.toString(htmlPropertiesString);
        // Set the breakpoint string:
        setPropertiesFromText(acGTStringToQString(htmlPropertiesString));
    }
}


// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::setPropertiesFromText
// Description:
// Arguments:   const QString& htmlText
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
void gdPropertiesEventObserver::setPropertiesFromText(const QString& htmlText)
{
    setPropertiesViewInfo();

    GT_IF_WITH_ASSERT(NULL != m_pPropertiesView)
    {
        m_pPropertiesView->setHTMLText(htmlText, this);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdPropertiesEventObserver::setPropertiesViewInfo
// Description:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        13/9/2012
// ---------------------------------------------------------------------------
void gdPropertiesEventObserver::setPropertiesViewInfo()
{
    if (m_pPropertiesView == NULL)
    {
        GT_IF_WITH_ASSERT(NULL != m_pApplicationCommands)
        {
            m_pPropertiesView = m_pApplicationCommands->propertiesView();

            if (m_pPropertiesView != NULL)
            {
                m_pProgressBar = m_pPropertiesView->progressBar();
            }
        }
    }
}
