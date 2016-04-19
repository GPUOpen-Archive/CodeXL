//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsFBOMonitor.cpp
///
//==================================================================================

//------------------------------ gsFBOMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/apDefaultTextureNames.h>
#include <AMDTAPIClasses/Include/ap2DRectangle.h>

// Spies Utilities:
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsExtensionsManager.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsTextureSerializer.h>
#include <src/gsFBOMonitor.h>


// ---------------------------------------------------------------------------
// Name:        gsFBOMonitor::gsFBOMonitor
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
gsFBOMonitor::gsFBOMonitor()
    : _amountOfFBOs(0)
{
}


// ---------------------------------------------------------------------------
// Name:        gsFBOMonitor::~gsFBOMonitor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
gsFBOMonitor::~gsFBOMonitor()
{
    // Delete the texture wrappers vector:
    int amountOfFBOs = (int)_fbos.size();

    for (int i = 0; i < amountOfFBOs; i++)
    {
        delete _fbos[i];
        _fbos[i] = NULL;
    }

    _fbos.clear();
}

// ---------------------------------------------------------------------------
// Name:        gsFBOMonitor::constructNewFbo
// Description: Construct a new apGLFBO object, and add it to the existing FBOs
// Arguments: GLuint fboName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
bool gsFBOMonitor::onGenFramebuffers(GLuint* fboNames, GLsizei count)
{
    bool retVal = true;

    GT_IF_WITH_ASSERT((NULL != fboNames) && (0 < count))
    {
        gtVector<apAllocatedObject*> fbosForAllocationMonitor;

        for (GLsizei i = 0; i < count; i++)
        {
            GLuint fboName = fboNames[i];

            // Construct new object:
            apGLFBO* pFBO = new apGLFBO(fboName);

            // Add the FBO object to the vector of FBOs:
            _fbos.push_back(pFBO);
            fbosForAllocationMonitor.push_back(pFBO);

            // Increase _amountOfFBOs:
            _amountOfFBOs++;
        }

        // Register this object in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObjects(fbosForAllocationMonitor);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsFBOMonitor::constructNewFbo
// Description: Destroys the apGLFBO object named by fboName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        18/6/2008
// ---------------------------------------------------------------------------
bool gsFBOMonitor::removeFbo(GLuint fboName)
{
    bool retVal = true;

    gtPtrVector<apGLFBO*>::iterator iter;
    gtPtrVector<apGLFBO*>::iterator iterEnd = _fbos.end();
    int i = 0;
    int foundIndex = -1;

    for (iter = _fbos.begin(); iter != iterEnd; iter++, i++)
    {
        apGLFBO* pCurrentFBO = *iter;
        GT_IF_WITH_ASSERT(pCurrentFBO != NULL)
        {
            if (pCurrentFBO->getFBOName() == fboName)
            {
                _fbos[i] = NULL;

                // Release memory:
                delete pCurrentFBO;

                // Decrease _amountOfFBOs:
                _amountOfFBOs--;

                foundIndex = i;
            }
        }
    }

    if (foundIndex > -1)
    {
        int n = (int)_fbos.size();

        // Shift the vector to one index back from the point of deletion, and remove the last item:
        for (int j = foundIndex; j < (n - 1); j++)
        {
            _fbos[j] = _fbos[j + 1];
        }

        _fbos.pop_back();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsFBOMonitor::getFBODetails
// Description: Returns an apGLFBO object according to GL frame buffer object name
// Arguments: GLuint fboName
// Return Val: apGLFBO*
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
apGLFBO* gsFBOMonitor::getFBODetails(GLuint fboName) const
{
    apGLFBO* retFBO = NULL;

    gtPtrVector<apGLFBO*>::const_iterator iter;
    gtPtrVector<apGLFBO*>::const_iterator iterEnd = _fbos.end();
    int i = 0;

    for (iter = _fbos.begin(); iter != iterEnd; iter++, i++)
    {
        apGLFBO* pCurrentFBO = *iter;
        GT_IF_WITH_ASSERT(pCurrentFBO != NULL)
        {
            if (pCurrentFBO->getFBOName() == fboName)
            {
                retFBO = pCurrentFBO;
                break;
            }
        }
    }

    return retFBO;
}

// ---------------------------------------------------------------------------
// Name:        gsFBOMonitor::getFBOName
// Description: Returns an FBO object name according to the FBO index in the list
// Arguments: int fboIndex
//            GLuint& fboName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/6/2008
// ---------------------------------------------------------------------------
bool gsFBOMonitor::getFBOName(int fboIndex, GLuint& fboName) const
{
    bool retVal = false;

    if (_fbos[fboIndex] != NULL)
    {
        // Get the FBO object from the list of FBOs:
        apGLFBO* pFbo = _fbos[fboIndex];

        // Get the FBO name:
        fboName = pFbo->getFBOName();

        retVal = true;
    }

    return retVal;
}
