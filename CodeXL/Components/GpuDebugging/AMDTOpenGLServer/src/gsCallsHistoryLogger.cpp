//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsCallsHistoryLogger.cpp
///
//==================================================================================

//------------------------------ gsCallsHistoryLogger.cpp ------------------------------

// Infra:
// Infra:
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsCallsHistoryLogger.h>

// HTML texture thumbnail sizes:
#define GS_TEX_THUMBNAIL_WIDTH 50
#define GS_TEX_THUMBNAIL_HEIGHT 50
#define GS_TEX_THUMBNAIL_BORDER 1


// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::gsCallsHistoryLogger
// Description: Constructor
// Arguments: contextId - The id of the render context who's calls history is logged.
// Author:      Yaki Tebeka
// Date:        4/11/2009
// ---------------------------------------------------------------------------
gsCallsHistoryLogger::gsCallsHistoryLogger(int contextId, apMonitoredFunctionId contextCreationFunc, const gtVector<gtString>* pContextAttribs)
    : suCallsHistoryLogger(apContextID(AP_OPENGL_CONTEXT, contextId), contextCreationFunc, suMaxLoggedOpenGLCallsPerContext(), GS_STR_RenderContextCallsHistoryLoggerMessagesLabelFormat, false)
{
    if (nullptr != pContextAttribs)
    {
        parseContextAttribs(*pContextAttribs);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::~gsCallsHistoryLogger
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        4/11/2009
// ---------------------------------------------------------------------------
gsCallsHistoryLogger::~gsCallsHistoryLogger()
{
}


// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::onFrameTerminatorCall
// Description:
//  Is called when a monitored function, defined as a frame terminator is called.
//  Clears the calls history log.
//
// Author:      Yaki Tebeka
// Date:        8/11/2009
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::onFrameTerminatorCall()
{
    // Clear the log:
    clearLog();
}


// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::onContextDeletion
// Description: Is called when the context that I represent is deleted.
// Author:      Yaki Tebeka
// Date:        14/11/2005
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::onContextDeletion()
{
    // Output a "context deleted" message into the log file:
    outputContextDeletionMessage();

    // Close the physical log file:
    closeHTMLLogFile();
}

// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::calculateHTMLLogFilePath
// Description: Calculates and outputs the HTML log file path.
// Author:      Yaki Tebeka
// Date:        18/8/2004
// Implementation notes:
//   The log file path is:
//   <_logFilesDirectoryPath>\<Application name><Context><X><Log><Data and time>.txt
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::calculateHTMLLogFilePath(osFilePath& textLogFilePath) const
{
    // Build the log file name:
    gtString logFileName;
    logFileName.appendFormattedString(GS_STR_callsLogFilePath, _contextId._contextId);

    // Set the log file path:
    textLogFilePath = suCurrentSessionLogFilesDirectory();
    textLogFilePath.setFileName(logFileName);

    gtString fileExtension;
    apFileTypeToFileExtensionString(AP_HTML_FILE, fileExtension);
    textLogFilePath.setFileExtension(fileExtension);
}


// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::getHTMLLogFileHeader
// Description:
//   Outputs a string that will be used as the generated HTML log file's header
// Author:      Yaki Tebeka
// Date:        4/11/2009
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::getHTMLLogFileHeader(gtString& htmlLogFileHeader) const
{
    htmlLogFileHeader.makeEmpty();

    // Get the debugged application name:
    gtString applicationName;
    osGetCurrentApplicationName(applicationName);

    // Print the HTML header:
    htmlLogFileHeader += L"<HTML>\n";
    htmlLogFileHeader += L"<head>\n";
    htmlLogFileHeader += L"   <title>OpenGL calls log - ";
    htmlLogFileHeader += applicationName;
    htmlLogFileHeader += L" ";
    gtString contextIdText;
    contextIdText.appendFormattedString(L" - Context %d", _contextId._contextId);
    htmlLogFileHeader += contextIdText;
    htmlLogFileHeader += L" - generated by CodeXL</title>\n";
    htmlLogFileHeader += L"</head>\n\n";
    htmlLogFileHeader += L"<BODY style=\"font: 12px/16px Courier, Verdana, sans-serif; background-color: EFEFEF;\">\n";
    htmlLogFileHeader += L"<h3>\n";

    htmlLogFileHeader += L"////////////////////////////////////////////////////////////<br>\n";
    htmlLogFileHeader += L"// This File contain an OpenGL calls log<br>\n";

    htmlLogFileHeader += L"// Application: ";
    htmlLogFileHeader += applicationName;
    htmlLogFileHeader += L"<br>\n";

    gtString dateAsString;
    _logCreationTime.dateAsString(dateAsString, osTime::WINDOWS_STYLE, osTime::LOCAL);
    htmlLogFileHeader += L"// Generation date: ";
    htmlLogFileHeader += dateAsString;
    htmlLogFileHeader += L"<br>\n";

    gtString timeAsString;
    _logCreationTime.timeAsString(timeAsString, osTime::WINDOWS_STYLE, osTime::LOCAL);
    htmlLogFileHeader += L"// Generation time: ";
    htmlLogFileHeader += timeAsString;
    htmlLogFileHeader += L"<br>\n";

    gtString contextText = L"// OpenGL Context id:";
    contextText.appendFormattedString(L" %d<br>\n", _contextId._contextId);
    contextText.append(L"// Context created with ").append(apMonitoredFunctionsManager::instance().monitoredFunctionName(m_contextCreationFunc)).append(L"<br>\n");
    contextText.append(m_contextCreationAttribsString);
    htmlLogFileHeader += contextText;

    htmlLogFileHeader += L"//<br>\n";
    htmlLogFileHeader += L"// Generated by CodeXL - OpenGL and OpenCL Debugger, Profiler and Memory Analyzer<br>\n";
    htmlLogFileHeader += L"// <A HREF=\"http://gpuopen.com/\" TARGET=\"_blank\">http://gpuopen.com/</A><br>\n";
    htmlLogFileHeader += L"////////////////////////////////////////////////////////////<br>\n";
    htmlLogFileHeader += L"</h3>\n";
    htmlLogFileHeader += L"<br>\n\n";
}


// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::getHTMLLogFileFooter
// Description:
//   Outputs a string that will be used as the generated HTML log file's footer
// Author:      Yaki Tebeka
// Date:        4/11/2009
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::getHTMLLogFileFooter(gtString& htmlLogFileFooter) const
{
    htmlLogFileFooter = L"\n</BODY>\n";
    htmlLogFileFooter += L"</HTML>\n";
}


// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::outputContextDeletionMessage
// Description: Outputs a context deletion message into the text log file.
// Author:      Yaki Tebeka
// Date:        14/11/2005
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::outputContextDeletionMessage()
{
    // only print if the text log file is active.
    bool isHTMLFileOpen = isHTMLLogFileOpen();

    if (isHTMLFileOpen)
    {
        gtString contextDeletionMessage(GS_STR_contextWasDeletedHTMLLog);
        printToHTMLLogFile(contextDeletionMessage);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::getPseudoArgumentHTMLLogSection
// Description: Inputs a pseudo argument and output an HTML section that represents it.
// Arguments: pseudoParam - The pseudo argument to be logged.
//            htmlLogFileSection - The HTML section that represents the pseudo argument
// Author:      Yaki Tebeka
// Date:        26/1/2005
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::getPseudoArgumentHTMLLogSection(const apPseudoParameter& pseudoArgument, gtString& htmlLogFileSection)
{
    // Handle the pseudo param according to its type:
    osTransferableObjectType pseudoArgType = pseudoArgument.type();

    switch (pseudoArgType)
    {
        case OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER:
        {
            getAssociatedTextureHTMLLogSection((const apAssociatedTextureNamesPseudoParameter&)pseudoArgument, htmlLogFileSection);
            break;
        }

        case OS_TOBJ_ID_ASSOCIATED_PROGRAM_NAME_PSEUDO_PARAMETER:
        {
            getAssociatedProgramHTMLLogSection((const apAssociatedProgramNamePseudoParameter&)pseudoArgument, htmlLogFileSection);
            break;
        }

        case OS_TOBJ_ID_ASSOCIATED_SHADER_NAME_PSEUDO_PARAMETER:
        {
            getAssociatedShaderHTMLLogSection((const apAssociatedShaderNamePseudoParameter&)pseudoArgument, htmlLogFileSection);
            break;
        }

        default:
        {
            // We shouldn't reach here!
            GT_ASSERT(false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::reportMemoryAllocationFailure
// Description: Reports to my debugger about memory allocation failure.
// Author:      Yaki Tebeka
// Date:        31/1/2009
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::reportMemoryAllocationFailure()
{
    // If we are not in glBegin-glEnd block:
    bool isInGLBeginEndBlock = isInOpenGLBeginEndBlock();

    if (!isInGLBeginEndBlock)
    {
        // Get the amount of function calls:
        int functionCallsAmount = amountOfFunctionCalls();

        // Calculate the report string:
        gtString errorDescription = _loggerMessagesLabel;
        bool allocFailureOccur = allocationFailureOccur();

        if (allocFailureOccur)
        {
            errorDescription.appendFormattedString(SU_STR_notEnoughMemoryForLoggingFunctions, functionCallsAmount);
        }
        else
        {
            errorDescription.appendFormattedString(SU_STR_maxLoggedFunctionsAmountReached, functionCallsAmount);
        }

        // Report an error to the debugger:
        gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
        theOpenGLMonitor.reportDetectedError(AP_MAX_LOGGED_FUNCTIONS_EXCEEDED_ERROR, errorDescription, apMonitoredFunctionsAmount);
    }

    // Perform base class actions:
    suCallsHistoryLogger::reportMemoryAllocationFailure();
}


// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::getAssociatedTextureHTMLLogSection
// Description: Stores an associated texture pseudo argument into
//              htmlLogFileSection.
// Arguments:   assciatedTexture - The associated texture pseudo argument.
// Author:      Yaki Tebeka
// Date:        26/1/2005
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::getAssociatedTextureHTMLLogSection(const apAssociatedTextureNamesPseudoParameter& assciatedTexture,
                                                              gtString& htmlLogFileSection)
{
    // Get the names of textures associated with the current function call:
    const gtVector<GLuint>& associatedTextureNames = assciatedTexture.associatedTextureNames();
    int amountOfAssociatedTextures = (int)associatedTextureNames.size();

    // Get an instance of the openGL monitor
    const gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();

    // Get the appropriate render context monitor:
    const gsRenderContextMonitor* pRenderContextMon = theOpenGLMonitor.renderContextMonitor(_contextId._contextId);

    if (pRenderContextMon)
    {
        for (int i = 0; i < amountOfAssociatedTextures; i++)
        {
            // Get the current texture details:
            GLuint textureID = associatedTextureNames[i];
            const gsTexturesMonitor* texturesMon = pRenderContextMon->texturesMonitor();
            GT_IF_WITH_ASSERT(texturesMon != NULL)
            {
                gsGLTexture* pTextureDetails = texturesMon->getTextureObjectDetails(textureID);

                if (pTextureDetails)
                {
                    // Get the texture type
                    apTextureType textureType = pTextureDetails->textureType();

                    // Get current texture image saving format
                    apFileType fileType = suLoggedTexturesFileType();

                    // On 3D Textures we always use tiff format
                    if ((textureType == AP_3D_TEXTURE) || (textureType == AP_1D_ARRAY_TEXTURE) || (textureType == AP_2D_ARRAY_TEXTURE))
                    {
                        fileType = AP_TIFF_FILE;
                    }

                    // Get image format extension (as string)
                    gtString fileExtension;
                    bool rc1 = apFileTypeToFileExtensionString(fileType, fileExtension);
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Ask the texture monitor to generate the name for current texture object
                        bool rc2 = texturesMon->generateTextureElementsPaths(pTextureDetails);
                        GT_IF_WITH_ASSERT(rc2)
                        {
                            // Get the amount of texture data files:
                            int amountOfFiles = pTextureDetails->amountOfTextureDataFiles();

                            if (amountOfFiles == 0)
                            {
                                // Add a "no texture" printout to the print buffer:
                                htmlLogFileSection.appendFormattedString(GS_STR_noTextureImagePreviewAvailable, _contextId._contextId, textureID);
                            }
                            else
                            {
                                // Print the texture context and name:
                                htmlLogFileSection.appendFormattedString(GS_STR_beginTexturePreview, _contextId._contextId, textureID);

                                // Iterate the available texture data files:
                                for (int j = 0; j < amountOfFiles; j++)
                                {
                                    // Add comma separator between texture images:
                                    if (j != 0)
                                    {
                                        htmlLogFileSection += L", ";
                                    }

                                    // Get the current texture data file path:
                                    osFilePath textureDataFilePath;
                                    bool rc0 = pTextureDetails->getTextureDataFilePath(textureDataFilePath, j);
                                    GT_ASSERT(rc0);

                                    gtString textureFileName;
                                    bool rc3 = textureDataFilePath.getFileName(textureFileName);

                                    if (rc3)
                                    {
                                        // Translate the file path into a relative URL:
                                        gtString fileRelativeURL = L"./";
                                        fileRelativeURL += textureFileName;
                                        fileRelativeURL += osFilePath::osExtensionSeparator;
                                        fileRelativeURL += fileExtension;

                                        // Add the current texture image printout to the print buffer:
                                        htmlLogFileSection.appendFormattedString(GS_STR_texturePreview, fileRelativeURL.asCharArray(), fileRelativeURL.asCharArray(), _contextId._contextId, textureID, _contextId._contextId, textureID, GS_TEX_THUMBNAIL_WIDTH, GS_TEX_THUMBNAIL_HEIGHT, GS_TEX_THUMBNAIL_BORDER);
                                    }
                                    else
                                    {
                                        htmlLogFileSection += GS_STR_textureNoPreviewAvailable;
                                    }
                                }

                                // Close the texture printout block:
                                htmlLogFileSection += GS_STR_endTexturePreview;
                            }
                        }
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::getAssociatedProgramHTMLLogSection
// Description: Stores an associated program pseudo argument into
//              htmlLogFileSection.
// Arguments:   associatedProgram - The associated program pseudo argument.
// Author:      Yaki Tebeka
// Date:        22/5/2005
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::getAssociatedProgramHTMLLogSection(const apAssociatedProgramNamePseudoParameter& associatedProgram,
                                                              gtString& htmlLogFileSection)
{
    // Get the OpenGL name of the program associated with the current function call:
    GLuint associatedProgramName = associatedProgram.associatedProgramName();

    // Add an associated program printout to the print buffer:
    htmlLogFileSection.appendFormattedString(GS_STR_assiciatedProgramTextStart, _contextId._contextId, associatedProgramName);

    // Get the appropriate render context monitor:
    const gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
    const gsRenderContextMonitor* pRenderContextMon = theOpenGLMonitor.renderContextMonitor(_contextId._contextId);

    if (pRenderContextMon)
    {
        // Get the program object details:
        const gsProgramsAndShadersMonitor* programsAndShadersMon = pRenderContextMon->programsAndShadersMonitor();
        GT_IF_WITH_ASSERT(programsAndShadersMon != NULL)
        {
            const apGLProgram* pProgramObjDetails = programsAndShadersMon->programObjectDetails(associatedProgramName);

            if (pProgramObjDetails != NULL)
            {
                // Iterate the program attached shaders:
                const gtList<GLuint>& attachedShaders = pProgramObjDetails->shaderObjects();
                gtList<GLuint>::const_iterator iter = attachedShaders.begin();
                gtList<GLuint>::const_iterator endIter = attachedShaders.end();
                bool isFirstShader = true;

                while (iter != endIter)
                {
                    // Add comma and colon if needed:
                    if (isFirstShader)
                    {
                        htmlLogFileSection += L": ";
                        isFirstShader = false;
                    }
                    else
                    {
                        htmlLogFileSection += L", ";
                    }

                    // Get the shader object name:
                    GLuint currShaderName = *iter;

                    // Get the shader object details:
                    const apGLShaderObject* pShaderObjDetails = programsAndShadersMon->shaderObjectDetails(currShaderName);

                    if (pShaderObjDetails)
                    {
                        // Get the shader source code file:
                        const osFilePath& shaderSourceCodePath = pShaderObjDetails->sourceCodeFilePath();
                        const gtString& shaderSourceCodePathAsString = shaderSourceCodePath.asString();

                        // If we don't have the shader source code file:
                        if (shaderSourceCodePathAsString.isEmpty())
                        {
                            // Add a "source code is not available" printout to the print buffer:
                            htmlLogFileSection.appendFormattedString(GS_STR_assiciatedProgramTextShaderNoSourceCode, currShaderName);
                        }
                        else
                        {
                            // Will get the source code file relative path:
                            gtString fileRelativeURL = L"N/A";

                            // Get the shader file path name and extension:
                            gtString fileName;
                            gtString fileExtension;
                            bool rc1 = shaderSourceCodePath.getFileName(fileName);
                            bool rc2 = shaderSourceCodePath.getFileExtension(fileExtension);

                            if (rc1 && rc2)
                            {
                                // Translate the file path into a relative URL:
                                fileRelativeURL = L"./";
                                fileRelativeURL += fileName;
                                fileRelativeURL += osFilePath::osExtensionSeparator;
                                fileRelativeURL += fileExtension;
                            }

                            // Print the context and shader names:
                            htmlLogFileSection.appendFormattedString(GS_STR_assiciatedProgramTextShader,
                                                                     fileRelativeURL.asCharArray(),
                                                                     currShaderName);
                        }
                    }

                    // Next associated shader:
                    iter++;
                }
            }

        }
        // End the program printed string:
        htmlLogFileSection += GS_STR_assiciatedProgramTextEnd;
    }
}


// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::getAssociatedShaderHTMLLogSection
// Description: Stores an associated program pseudo argument into
//              htmlLogFileSection.
// Arguments:   associatedShader - The associated shader pseudo argument.
// Author:      Yaki Tebeka
// Date:        22/5/2005
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::getAssociatedShaderHTMLLogSection(const apAssociatedShaderNamePseudoParameter& associatedShader,
                                                             gtString& htmlLogFileSection)
{
    // Get the OpenGL name of the shader associated with the current function call:
    GLuint associatedShaderName = associatedShader.associatedShaderName();

    // Get the appropriate render context monitor:
    const gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
    const gsRenderContextMonitor* pRenderContextMon = theOpenGLMonitor.renderContextMonitor(_contextId._contextId);

    if (pRenderContextMon)
    {
        // Get the shader object details:
        const gsProgramsAndShadersMonitor* programsAndShadersMon = pRenderContextMon->programsAndShadersMonitor();
        GT_IF_WITH_ASSERT(programsAndShadersMon != NULL)
        {
            const apGLShaderObject* pShaderObjDetails = programsAndShadersMon->shaderObjectDetails(associatedShaderName);

            if (pShaderObjDetails)
            {
                // Get the shader source code file:
                const osFilePath& shaderSourceCodePath = pShaderObjDetails->sourceCodeFilePath();
                const gtString& shaderSourceCodePathAsString = shaderSourceCodePath.asString();

                // If we don't have the shader source code file:
                if (shaderSourceCodePathAsString.isEmpty())
                {
                    // Add a "source code is not available" printout to the print buffer:
                    htmlLogFileSection.appendFormattedString(GS_STR_noShaderSourceCodeAvailable, _contextId._contextId, associatedShaderName);
                }
                else
                {
                    // Will get the source code file relative path:
                    gtString fileRelativeURL = L"N/A";

                    // Get the shader file path name and extension:
                    gtString fileName;
                    gtString fileExtension;
                    bool rc1 = shaderSourceCodePath.getFileName(fileName);
                    bool rc2 = shaderSourceCodePath.getFileExtension(fileExtension);

                    if (rc1 && rc2)
                    {
                        // Translate the file path into a relative URL:
                        fileRelativeURL = L"./";
                        fileRelativeURL += fileName;
                        fileRelativeURL += osFilePath::osExtensionSeparator;
                        fileRelativeURL += fileExtension;
                    }

                    // Print the context and shader names:
                    htmlLogFileSection.appendFormattedString(GS_STR_shaderNameAndSourceCodeBegin, _contextId._contextId, fileRelativeURL.asCharArray(), associatedShaderName);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsCallsHistoryLogger::parseContextAttribs
// Description: Creates a string to be used in the log header from a list of attributes
// Author:      Uri Shomroni
// Date:        27/7/2015
// ---------------------------------------------------------------------------
void gsCallsHistoryLogger::parseContextAttribs(const gtVector<gtString>& contextAttribs)
{
    m_contextCreationAttribsString = L"// OpenGL Context attributes:<br>\n";

    int attribsCount = (int)contextAttribs.size();

    if (0 < attribsCount)
    {
        for (int i = 0; attribsCount > i; ++i)
        {
            const gtString& currentAttrib = contextAttribs[i];
            int attribLen = currentAttrib.length();

            if (60 > attribLen)
            {
                m_contextCreationAttribsString.append(L"//    ").append(currentAttrib).append(L"<br>\n");
            }
            else
            {
                gtString subStr;

                for (int j = 0; attribLen > j; j += 60)
                {
                    int line = (60 <= attribLen - j) ? 59 : attribLen - j - 1;
                    currentAttrib.getSubString(j, j + line, subStr);
                    m_contextCreationAttribsString.append(L"//    ").append(subStr).append(L"<br>\n");
                }
            }
        }
    }
}
