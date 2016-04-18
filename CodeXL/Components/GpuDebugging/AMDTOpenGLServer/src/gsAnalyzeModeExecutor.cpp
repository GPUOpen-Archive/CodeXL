//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsAnalyzeModeExecutor.cpp
///
//==================================================================================

//------------------------------ gsAnalyzeModeExecutor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariablesManager.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Spy Utils:
#include <AMDTServerUtilities/Include/suBreakpointsManager.h>

// Local:
#include <src/gsAnalyzeModeExecutor.h>
#include <src/gsGlobalVariables.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsStateChangeExecutor.h>

// Static vector initialization:
bool* gsAnalyzeModeExecutor::_isFunctionSupportedInBeginEndBlock = NULL;


// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::gsAnalyzeModeExecutor
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        31/8/2008
// ---------------------------------------------------------------------------
gsAnalyzeModeExecutor::gsAnalyzeModeExecutor()
    : _pRenderContextMonitor(NULL),
      _pStateVaraiblesValues1(NULL),
      _pStateVaraiblesValues2(NULL),
      _lastComputedRedundancyStatus(AP_REDUNDANCY_UNKNOWN),
      _wasCalledBeforeFunction(false)
{
}


// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::~gsAnalyzeModeExecutor
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        31/8/2008
// ---------------------------------------------------------------------------
gsAnalyzeModeExecutor::~gsAnalyzeModeExecutor()
{
    // If previous state variables snapshot exist, clear it:
    if (_pStateVaraiblesValues1 != NULL)
    {
        delete _pStateVaraiblesValues1;
        _pStateVaraiblesValues1 = NULL;
    }

    // If previous state variables snapshot exist, clear it:
    if (_pStateVaraiblesValues2 != NULL)
    {
        delete _pStateVaraiblesValues2;
        _pStateVaraiblesValues2 = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::initialize
// Description: Initializes the render context monitor
//              Initializes the snapshot for begin end block tests with the functions
//              needed for support while in begin end block.
// Arguments: gsRenderContextMonitor* pRenderContextMonitor
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/8/2008
// ---------------------------------------------------------------------------
bool gsAnalyzeModeExecutor::initialize(gsRenderContextMonitor* pRenderContextMonitor)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
    {
        // Initializes static vectors for state change optimization;
        initStaticVectors();

        // Initialize the render context monitor pointer:
        _pRenderContextMonitor = pRenderContextMonitor;

        // Set state variables for color functions:
        _beginEndStateVariableIdsVec.push_back(apGL_CURRENT_COLOR);
        _beginEndStateVariableIdsVec.push_back(apGL_RGBA_MODE);

        // Set state variables for index functions:
        _beginEndStateVariableIdsVec.push_back(apGL_CURRENT_INDEX);

        // Set state variables for secondary color functions:
        _beginEndStateVariableIdsVec.push_back(apGL_CURRENT_SECONDARY_COLOR);
        _beginEndStateVariableIdsVec.push_back(apGL_RGBA_MODE);
        _beginEndStateVariableIdsVec.push_back(apGL_COLOR_SUM);

        // Set state variables for material functions:
        _beginEndStateVariableIdsVec.push_back(apGL_AMBIENT_back);
        _beginEndStateVariableIdsVec.push_back(apGL_AMBIENT_front);
        _beginEndStateVariableIdsVec.push_back(apGL_DIFFUSE_back);
        _beginEndStateVariableIdsVec.push_back(apGL_DIFFUSE_front);
        _beginEndStateVariableIdsVec.push_back(apGL_SPECULAR_back);
        _beginEndStateVariableIdsVec.push_back(apGL_SPECULAR_front);
        _beginEndStateVariableIdsVec.push_back(apGL_EMISSION_back);
        _beginEndStateVariableIdsVec.push_back(apGL_EMISSION_front);
        _beginEndStateVariableIdsVec.push_back(apGL_SHININESS_back);
        _beginEndStateVariableIdsVec.push_back(apGL_SHININESS_front);

        // Initialize glNormal state variable ids:
        _glNormalStateVariableIdsVec.push_back(apGL_CURRENT_NORMAL);
        _glNormalStateVariableIdsVec.push_back(apGL_NORMALIZE);
        _glNormalStateVariableIdsVec.push_back(apGL_RESCALE_NORMAL);

        // Initialize glPixel state variable ids:
        _glPixelStoreStateVariableIdsVec.push_back(apGL_PACK_ROW_LENGTH);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_PACK_IMAGE_HEIGHT_EXT);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_PACK_SKIP_ROWS);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_PACK_SKIP_PIXELS);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_PACK_SKIP_IMAGES_EXT);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_PACK_ALIGNMENT);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_UNPACK_SWAP_BYTES);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_UNPACK_LSB_FIRST);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_UNPACK_ROW_LENGTH);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_UNPACK_IMAGE_HEIGHT_EXT);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_UNPACK_SKIP_ROWS);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_UNPACK_SKIP_PIXELS);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_UNPACK_SKIP_IMAGES_EXT);
        _glPixelStoreStateVariableIdsVec.push_back(apGL_UNPACK_ALIGNMENT);

        // Initialize glViewport state variable ids:
        _glViewportStateVariableIdsVec.push_back(apGL_VIEWPORT);
        _glViewportStateVariableIdsVec.push_back(apGL_MAX_VIEWPORT_DIMS);

        // Initialize glEnable/Disable with alpha test state variable ids:
        _glAlphaTestStateVariableIdsVec.push_back(apGL_ALPHA_TEST_FUNC);
        _glAlphaTestStateVariableIdsVec.push_back(apGL_ALPHA_TEST_REF);
        _glAlphaTestStateVariableIdsVec.push_back(apGL_ALPHA_TEST);

        // Initialize glEnable/Disable with auto normal test state variable ids:
        _glAutoNormalStateVariableIdsVec.push_back(apGL_MAX_EVAL_ORDER);
        /*_glAutoNormalStateVariableIdsVec.push_back(apGL_MAP2_VERTEX_3);
        _glAutoNormalStateVariableIdsVec.push_back(apGL_MAP2_VERTEX_4);
        _glAutoNormalStateVariableIdsVec.push_back(apGL_MAP2_INDEX);
        _glAutoNormalStateVariableIdsVec.push_back(apGL_MAP2_COLOR_4);
        _glAutoNormalStateVariableIdsVec.push_back(apGL_MAP2_NORMAL);
        _glAutoNormalStateVariableIdsVec.push_back(apGL_MAP2_TEXTURE_COORD_1);
        _glAutoNormalStateVariableIdsVec.push_back(apGL_MAP2_TEXTURE_COORD_2);
        _glAutoNormalStateVariableIdsVec.push_back(apGL_MAP2_TEXTURE_COORD_3);
        _glAutoNormalStateVariableIdsVec.push_back(apGL_MAP2_TEXTURE_COORD_4);*/

        // Initialize glEnable/Disable with blend test state variable ids:
        _glBlendStateVariableIdsVec.push_back(apGL_BLEND_SRC);
        _glBlendStateVariableIdsVec.push_back(apGL_BLEND_DST);
        _glBlendStateVariableIdsVec.push_back(apGL_BLEND);

        // Initialize glEnable/Disable with texture state variable ids:
        _glTextureStateVariableIdsVec.push_back(apGL_TEXTURE_1D);
        _glTextureStateVariableIdsVec.push_back(apGL_TEXTURE_2D);
        _glTextureStateVariableIdsVec.push_back(apGL_TEXTURE_3D);

        // Initialize glEnable/Disable with texture state variable ids:
        _glCullFaceStateVariableIdsVec.push_back(apGL_CULL_FACE);
        _glCullFaceStateVariableIdsVec.push_back(apGL_CULL_FACE_MODE);

        // Initialize glEnable/Disable with texture state variable ids:
        _glLightingStateVariableIdsVec.push_back(apGL_LIGHTING);
        _glLightingStateVariableIdsVec.push_back(apGL_LIGHT_MODEL_AMBIENT);
        //_glLightingStateVariableIdsVec.push_back(apGL_LIGHT_MODEL_COLOR_CONTROL);
        _glLightingStateVariableIdsVec.push_back(apGL_LIGHT_MODEL_LOCAL_VIEWER);
        _glLightingStateVariableIdsVec.push_back(apGL_LIGHT_MODEL_TWO_SIDE);

        // Initialize matrix operation state variable ids:
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW_MATRIX);
        _glMatrixStateVariableIdsVec.push_back(apGL_PROJECTION_MATRIX);
        _glMatrixStateVariableIdsVec.push_back(apGL_TEXTURE_MATRIX);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW0_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW1_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW2_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW3_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW4_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW5_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW6_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW7_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW8_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW9_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW10_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW11_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW12_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW13_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW14_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW15_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW16_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW17_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW18_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW19_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW20_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW21_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW22_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW23_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW24_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW25_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW26_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW27_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW28_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW29_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW30_ARB);
        _glMatrixStateVariableIdsVec.push_back(apGL_MODELVIEW31_ARB);

        // Initialize glTexCoordPointer state variable ids:
        _glTexCoordPointerStateVariableIdsVec.push_back(apGL_TEXTURE_COORD_ARRAY);
        _glTexCoordPointerStateVariableIdsVec.push_back(apGL_TEXTURE_COORD_ARRAY_SIZE);
        _glTexCoordPointerStateVariableIdsVec.push_back(apGL_TEXTURE_COORD_ARRAY_TYPE);
        _glTexCoordPointerStateVariableIdsVec.push_back(apGL_TEXTURE_COORD_ARRAY_STRIDE);
        _glTexCoordPointerStateVariableIdsVec.push_back(apGL_TEXTURE_COORD_ARRAY_BUFFER_BINDING);
        _glTexCoordPointerStateVariableIdsVec.push_back(apGL_ARRAY_BUFFER_BINDING);
        _glTexCoordPointerStateVariableIdsVec.push_back(apGL_TEXTURE_COORD_ARRAY_POINTER);

        // Initialize glColorPointer state variable ids:
        _glColorPointerStateVariableIdsVec.push_back(apGL_COLOR_ARRAY);
        _glColorPointerStateVariableIdsVec.push_back(apGL_COLOR_ARRAY_SIZE);
        _glColorPointerStateVariableIdsVec.push_back(apGL_COLOR_ARRAY_TYPE);
        _glColorPointerStateVariableIdsVec.push_back(apGL_COLOR_ARRAY_STRIDE);
        _glColorPointerStateVariableIdsVec.push_back(apGL_COLOR_ARRAY_BUFFER_BINDING);
        _glColorPointerStateVariableIdsVec.push_back(apGL_ARRAY_BUFFER_BINDING);
        _glColorPointerStateVariableIdsVec.push_back(apGL_COLOR_ARRAY_POINTER);

        // Initialize _glVertexPointer state variable ids:
        _glVertexPointerStateVariableIdsVec.push_back(apGL_VERTEX_ARRAY);
        _glVertexPointerStateVariableIdsVec.push_back(apGL_VERTEX_ARRAY_SIZE);
        _glVertexPointerStateVariableIdsVec.push_back(apGL_VERTEX_ARRAY_TYPE);
        _glVertexPointerStateVariableIdsVec.push_back(apGL_VERTEX_ARRAY_STRIDE);
        _glVertexPointerStateVariableIdsVec.push_back(apGL_VERTEX_ARRAY_BUFFER_BINDING);
        _glVertexPointerStateVariableIdsVec.push_back(apGL_ARRAY_BUFFER_BINDING);
        _glVertexPointerStateVariableIdsVec.push_back(apGL_VERTEX_ARRAY_POINTER);

        // Initialize glDisable/Enable Client state with GL_NORMAL_ARRAY:
        _glNormalArrayStateVariableIds.push_back(apGL_NORMAL_ARRAY);
        _glNormalArrayStateVariableIds.push_back(apGL_NORMAL_ARRAY_TYPE);
        _glNormalArrayStateVariableIds.push_back(apGL_NORMAL_ARRAY_STRIDE);
        _glNormalArrayStateVariableIds.push_back(apGL_NORMAL_ARRAY_BUFFER_BINDING);
        _glNormalArrayStateVariableIds.push_back(apGL_ARRAY_BUFFER_BINDING);
        _glNormalArrayStateVariableIds.push_back(apGL_NORMAL_ARRAY_POINTER);

        // Get the state variable ids needed for comparison while in begin - end block:
        gtVector<apOpenGLStateVariableId>* pStateVaraiblesNeededForComparisonInBeginEndBlockVector = NULL;
        getAllBeginEndRelevantStateVariables(pStateVaraiblesNeededForComparisonInBeginEndBlockVector);

        // Set _glBeginStateVariableSnapShot state variable ids:
        _glBeginStateVariableSnapShot.supportOnlyFilteredStateVariableIds(pStateVaraiblesNeededForComparisonInBeginEndBlockVector);
        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::onFirstTimeContextMadeCurrent
// Description: Called the first time my context is made current
// Author:      Uri Shomroni
// Date:        14/7/2009
// ---------------------------------------------------------------------------
void gsAnalyzeModeExecutor::onFirstTimeContextMadeCurrent()
{
    // The context monitor now has the OpenGL version information, so we can trim the vectors according to this information:
    removeStateVariablesFromVectorByImplementation(_beginEndStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glNormalStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glPixelStoreStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glViewportStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glAlphaTestStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glAutoNormalStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glBlendStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glTextureStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glCullFaceStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glLightingStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glMatrixStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glTexCoordPointerStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glColorPointerStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glVertexPointerStateVariableIdsVec);
    removeStateVariablesFromVectorByImplementation(_glNormalArrayStateVariableIds);
}

// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::initStaticVectors
// Description: Initialize this class static vectors.
// Author:      Sigal Algranaty
// Date:        7/9/2008
// ---------------------------------------------------------------------------
void gsAnalyzeModeExecutor::initStaticVectors()
{
    // Contains true iff the static vectors were initialized:
    static bool stat_wereStaticVectorsInitialized = false;

    // A critical section that verifies that verifies that only one thread
    // can initialize this class vectors, and during initialization time no
    // other thread can exit this function:
    static osCriticalSection stat_vecInitializationCS;

    // Lock the access to the vectors initialization:
    osCriticalSectionLocker vecInitCSLocker(stat_vecInitializationCS);

    // If the vectors were not initialized:
    if (!stat_wereStaticVectorsInitialized)
    {
        stat_wereStaticVectorsInitialized = true;

        // Initialize the list of functions supported in begin - end block:
        _isFunctionSupportedInBeginEndBlock = new bool[apMonitoredFunctionsAmount];

        // Initialize each of the functions to false:
        for (int i = 0; i < apMonitoredFunctionsAmount; i++)
        {
            _isFunctionSupportedInBeginEndBlock[i] = false;
        }

        // glColor functions:
        _isFunctionSupportedInBeginEndBlock[ap_glColor3b] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3ubv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3bv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3d] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3dv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3f] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3fv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3i] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3iv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3s] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3sv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3ub] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3ubv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3ui] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3uiv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3us] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor3usv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4b] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4bv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4ub] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4ubv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4d] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4dv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4f] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4fv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4i] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4iv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4s] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4sv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4ub] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4ui] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4uiv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4us] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glColor4usv] = true;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // glColor4x is an OpenGL ES function
        _isFunctionSupportedInBeginEndBlock[ap_glColor4x] = true;
#endif

        // glIndex functions:
        _isFunctionSupportedInBeginEndBlock[ap_glIndexd] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glIndexdv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glIndexf] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glIndexfv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glIndexi] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glIndexiv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glIndexs] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glIndexsv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glIndexub] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glIndexubv] = true;

        // glSecondaryColor functions:
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3b] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3bv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3d] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3dv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3f] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3fv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3i] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3iv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3s] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3sv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3ub] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3ubv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3ui] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3uiv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3us] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glSecondaryColor3usv] = true;

        // glMaterial functions:
        _isFunctionSupportedInBeginEndBlock[ap_glMaterialf] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glMaterialfv] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glMateriali] = true;
        _isFunctionSupportedInBeginEndBlock[ap_glMaterialiv] = true;
    }

    vecInitCSLocker.leaveCriticalSection();
}

// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::takeStateVariablesSnapshotForStateChangeFunctionCall
// Description: Is taking a state variables snapshot.
// Arguments:   bool beforeFunctionCall - is the snapshot taken before or after a function call.
//              apMonitoredFunctionId calledFunctionId - the current function call id
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/7/2008
// ---------------------------------------------------------------------------
bool gsAnalyzeModeExecutor::takeStateVariablesSnapshotForStateChangeFunctionCall(apMonitoredFunctionId calledFunctionId, bool beforeFunctionCall)
{
    bool retVal = false;

    // MAke sure that the render context monitor is initialized:
    GT_IF_WITH_ASSERT(_pRenderContextMonitor != NULL)
    {
        // Take actions before snapshot update:
        _pRenderContextMonitor->beforeUpdatingContextDataSnapshot();

        // Initialize both the snapshot vectors if necessary:
        if (_pStateVaraiblesValues1 == NULL)
        {
            // Create and update the first state variables snapshot:
            _pStateVaraiblesValues1 = new gsStateVariablesSnapshot;


            // Initialize the new state variables snapshot:
            _pStateVaraiblesValues1->onFirstTimeContextMadeCurrent(*_pRenderContextMonitor);
        }

        if (_pStateVaraiblesValues2 == NULL)
        {
            // Create and update the first state variables snapshot:
            _pStateVaraiblesValues2 = new gsStateVariablesSnapshot;


            // Initialize the new state variables snapshot:
            _pStateVaraiblesValues2->onFirstTimeContextMadeCurrent(*_pRenderContextMonitor);
        }

        // Set the state variables snapshot to the second state variables snapshot:
        gsStateVariablesSnapshot* pStateVaraiblesValues = _pStateVaraiblesValues2;

        // If the snapshot is take before the function call, set the snapshot pointer to the first one:
        if (beforeFunctionCall)
        {
            pStateVaraiblesValues = _pStateVaraiblesValues1;

            // Get the last function call enum value:
            GLenum functionCallEnumValue;
            const suCallsStatisticsLogger& callsStatisticsLogger = _pRenderContextMonitor->callsStatisticsLogger();
            bool enumExtracted = callsStatisticsLogger.getCurrentFunctionCallEnumValue(calledFunctionId, functionCallEnumValue);

            // Check if this function has a vector with filtered state variable ids:
            gtVector<apOpenGLStateVariableId>* pVectorOfFilteredStateVariables = NULL;
            getFunctionCallStateVariablesVector(calledFunctionId, functionCallEnumValue, enumExtracted, pVectorOfFilteredStateVariables);

            // Filter the snapshots to this function call state variable ids:
            _pStateVaraiblesValues1->supportOnlyFilteredStateVariableIds(pVectorOfFilteredStateVariables);
            _pStateVaraiblesValues2->supportOnlyFilteredStateVariableIds(pVectorOfFilteredStateVariables);
        }

        // Update the context data snapshot:
        retVal = pStateVaraiblesValues->updateContextDataSnapshot();

        // Take actions after snapshot update:
        _pRenderContextMonitor->afterUpdatingContextDataSnapshot();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::addFunctionCall
// Description: Add a function call.
//              * Handles analyze mode function logging
// Arguments: apMonitoredFunctionId calledFuncitonId - the function call id
//            int argumentsAmount - argument amount
//            va_list& pArgumentList - the arguments list
// Return Val: void
// Author:      Sigal Algranaty
// Date:        6/8/2008
// ---------------------------------------------------------------------------
void gsAnalyzeModeExecutor::addFunctionCall(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList)
{
    // Check current execution mode:
    apExecutionMode executionMode = suDebuggedProcessExecutionMode();

    if (executionMode == AP_ANALYZE_MODE)
    {
        // Note that we were called before a function:
        _wasCalledBeforeFunction = true;

        // Make sure that we have a valid pointer to a render context monitor:
        GT_IF_WITH_ASSERT(_pRenderContextMonitor != NULL)
        {
            // Initialize analyze mode settings:
            startStateVariablesAnalyzeLogging(calledFunctionId);

            // Get a snapshot of state variables that are relevant for function within begin-end block:
            if (calledFunctionId == ap_glBegin)
            {
                // Take a state variables snapshot for the relevant state variables:
                _pRenderContextMonitor->beforeUpdatingContextDataSnapshot();
                _glBeginStateVariableSnapShot.onFirstTimeContextMadeCurrent(*_pRenderContextMonitor);
                _glBeginStateVariableSnapShot.updateContextDataSnapshot();
                _pRenderContextMonitor->afterUpdatingContextDataSnapshot();
            }
        }

        // For functions within a begin end block, check redundancy status:
        if (_pRenderContextMonitor->isInOpenGLBeginEndBlock())
        {
            getRedundancyStatusWithinBeginEndBlock(calledFunctionId, argumentsAmount, pArgumentList);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::startStateVariablesAnalyzeLogging
// Description: Initialize the current state variables snapshot
//              The state variables snapshot would be used later for comparison
//              (only in analyze mode)
// Arguments: int calledFunctionId - the called function id
// Return Val: void
// Author:      Sigal Algranaty
// Date:        6/7/2008
// ---------------------------------------------------------------------------
void gsAnalyzeModeExecutor::startStateVariablesAnalyzeLogging(apMonitoredFunctionId calledFunctionId)
{
    // Make sure that we have a valid pointer to a render context monitor:
    GT_IF_WITH_ASSERT(_pRenderContextMonitor != NULL)
    {
        // Take state variable snapshot for function out of begin end block:
        if (!_pRenderContextMonitor->isInOpenGLBeginEndBlock())
        {
            // Check if the function is a state change function:
            bool isFunctionStateChange = ((apMonitoredFunctionsManager::instance().monitoredFunctionType(calledFunctionId) & AP_STATE_CHANGE_FUNC) != 0);

            // If this function is a state change function, update the state variables snapshot:
            if (isFunctionStateChange)
            {
                bool rc = takeStateVariablesSnapshotForStateChangeFunctionCall(calledFunctionId, true);
                GT_ASSERT_EX(rc, L"Problem updating variables snapshot");
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::compareStateVariablesSnapshots
// Description: Compare the current state variable snapshot to the previous one
// Arguments:   bool& areValuesEqual - output - are the state variables values equal
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/7/2008
// ---------------------------------------------------------------------------
bool gsAnalyzeModeExecutor::comparePreviousToCurrentStateVariablesSnapshots(bool& areValuesEqual)
{
    bool retVal = false;

    // Check if we're in a begin end block:
    bool isInOpenGLBeginEndBlock = false;

    // Get call history logger:
    suCallsHistoryLogger* pCallsHistoryLogger = _pRenderContextMonitor->callsHistoryLogger();

    if (pCallsHistoryLogger != NULL)
    {
        isInOpenGLBeginEndBlock = pCallsHistoryLogger->isInOpenGLBeginEndBlock();
    }

    if (!isInOpenGLBeginEndBlock)
    {
        // Compare both the snapshots:
        GT_IF_WITH_ASSERT((_pStateVaraiblesValues1 != NULL) && (_pStateVaraiblesValues2 != NULL))
        {
            retVal = _pStateVaraiblesValues1->compareToOther(*_pStateVaraiblesValues2, areValuesEqual);
        }
    }
    else
    {
        retVal = true;
        areValuesEqual = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::setFunctionRedundancyStatus
// Description: The function sets a function redundancy status. The redundancy status is set
//              to default when the function call is added (since it is known only
//              at the function call end
// Arguments: int callIndex - the function call index
//            apMonitoredFunctionId calledFunctionID - the function call id
//            apFunctionRedundancyStatus redundancyStatus - the redundancy status
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/7/2008
// ---------------------------------------------------------------------------
bool gsAnalyzeModeExecutor::setFunctionRedundancyStatus(int callFunctionIndex, apMonitoredFunctionId calledFunctionID, apFunctionRedundancyStatus redundancyStatus)
{
    bool retVal = false;

    // Get monitored function calls logger, and calls statistics logger;
    suCallsHistoryLogger* pFuncsCallsLogger = _pRenderContextMonitor->callsHistoryLogger();
    suCallsStatisticsLogger& callsStatisticsLogger = _pRenderContextMonitor->callsStatisticsLogger();

    // Sanity check:
    GT_IF_WITH_ASSERT(pFuncsCallsLogger != NULL)
    {
        // Get function calls amount:
        int amountOfCalls = pFuncsCallsLogger->amountOfFunctionCalls();

        // callIndex range test:
        if ((0 <= callFunctionIndex) && (callFunctionIndex < amountOfCalls))
        {
            // Seek the raw memory read position to the beginning of the requested
            // function call raw memory log:
            pFuncsCallsLogger->writeFunctionRedundancyStatus(callFunctionIndex, redundancyStatus);

            // Set the redundancy status in statistics logger:
            callsStatisticsLogger.accumulateFunctionCallRedundancyStatus(calledFunctionID, redundancyStatus);

            retVal = true;

        }

    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::getRedundancyStatusWithinBeginEndBlock
// Description: This function is called for each function within begin-end block.
//              For the functions which are state change and supported within begin end block
//              we are checking the relevant state variable values. We compare the values to
//              the function arguments, and that's how we decide whether the function is a
//              redundant function or not.
// Arguments:   apMonitoredFunctionId calledFunctionId - the currently called function id
//              int argumentsAmount - the amount of arguments
//              va_list& pArgumentList - the list of arguments
// Return Val: void
// Author:      Sigal Algranaty
// Date:        18/8/2008
// ---------------------------------------------------------------------------
void gsAnalyzeModeExecutor::getRedundancyStatusWithinBeginEndBlock(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList)
{
    // Initialize computed redundancy status to unknown;
    _lastComputedRedundancyStatus = AP_REDUNDANCY_UNKNOWN;

    // Check if this function call requires state variables comparison:
    bool isSupportedFunction = false;

    // Get the calls statistics logger:
    (void) _pRenderContextMonitor->callsStatisticsLogger();

    // Check if this function is supported within begin end block, and if it is a state change:
    bool rc = isStateVariableComparisionNeededForFunctionInBeginEndBlock(calledFunctionId, isSupportedFunction);
    GT_IF_WITH_ASSERT(rc)
    {
        // If this function is supported while in begin end block:
        if (isSupportedFunction)
        {
            // Create a state change executor:
            gsStateChangeExecutor functionVirtualExecuter(&_glBeginStateVariableSnapShot);

            // Apply the function call state change, and get the function redundancy status:
            rc = functionVirtualExecuter.applyStateChange(calledFunctionId, argumentsAmount, pArgumentList, _lastComputedRedundancyStatus);
            GT_ASSERT_EX(rc, L"Problem apllying a state change");

            if (!rc)
            {
                // If computation of redundancy status had failed:
                _lastComputedRedundancyStatus = AP_REDUNDANCY_UNKNOWN;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::afterMonitoredFunctionExecutionActions
// Description:
// Arguments: apMonitoredFunctionId calledFunctionIndex
// Return Val: void
// Author:      Sigal Algranaty
// Date:        31/8/2008
// ---------------------------------------------------------------------------
void gsAnalyzeModeExecutor::afterMonitoredFunctionExecutionActions(apMonitoredFunctionId calledFunctionId)
{
    // Get application execution mode:
    apExecutionMode executionMode = suDebuggedProcessExecutionMode();

    if (executionMode == AP_ANALYZE_MODE)
    {
        // Get the OpenGL monitor:
        gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();

        // Check if the function is a state change function:
        bool isFunctionStateChange = ((apMonitoredFunctionsManager::instance().monitoredFunctionType(calledFunctionId) & AP_STATE_CHANGE_FUNC) != 0);

        // For state change functions, compare last state variables snapshot:
        if (isFunctionStateChange)
        {
            // Set the function redundancy status:
            apFunctionRedundancyStatus redundancyStatus = AP_REDUNDANCY_UNKNOWN;

            // If we're not in begin - end block, check the state variables values:
            suCallsHistoryLogger* pCallsHistoryLogger = _pRenderContextMonitor->callsHistoryLogger();
            GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
            {
                bool isInBeginEndBlock = false;

                if (pCallsHistoryLogger != NULL)
                {
                    isInBeginEndBlock = pCallsHistoryLogger->isInOpenGLBeginEndBlock();
                }

                if (!isInBeginEndBlock)
                {
                    // Take another snapshot after the function call;
                    bool rc = takeStateVariablesSnapshotForStateChangeFunctionCall(calledFunctionId, false);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        // If we just changed to Analyze Mode, we don't need to check since we don't have a "before" snapshot:
                        if (_wasCalledBeforeFunction)
                        {
                            // Compare both the snapshots taken:
                            bool areValuesEqual = false;
                            rc = comparePreviousToCurrentStateVariablesSnapshots(areValuesEqual);
                            GT_IF_WITH_ASSERT_EX(rc, L"Problem comparing state variables snapshot")
                            {
                                if (areValuesEqual)
                                {
                                    redundancyStatus = AP_REDUNDANCY_REDUNDANT;
                                }
                                else
                                {
                                    redundancyStatus = AP_REDUNDANCY_NOT_REDUNDANT;
                                }
                            }
                        }
                    }
                }
                else
                {
                    // If we're in begin-end block, the redundancy status was computed before (at addFunctionCall()):
                    redundancyStatus = _lastComputedRedundancyStatus;
                }

                // If we just switched to Analyze Mode, we don't need to mark this function as redundant, since we don't
                // know if it is:
                if (_wasCalledBeforeFunction)
                {
                    // Get the current function call id (the last function added):
                    int functionCallId = pCallsHistoryLogger->amountOfFunctionCalls() - 1;

                    // Set the function redundancy status:
                    setFunctionRedundancyStatus(functionCallId, calledFunctionId, redundancyStatus);
                }

                // If in "break on redundant state changes mode", break when relevant:
                if ((redundancyStatus == AP_REDUNDANCY_REDUNDANT) && (su_stat_theBreakpointsManager.breakOnGenericBreakpoint(AP_BREAK_ON_REDUNDANT_STATE_CHANGE)))
                {
                    theOpenGLMonitor.triggerBreakpointException(AP_REDUNDANT_STATE_CHANGE_BREAKPOINT_HIT, GL_NO_ERROR, false);
                }
            }
        }
    }

    // Mark that we were called after a function:
    _wasCalledBeforeFunction = false;
}


// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::getFunctionCallStateVariablesVector
// Description: Returns a list of state variables relevant for a function call (+enumeration)
// Arguments: apMonitoredFunctionId calledFunctionId
//            GLenum functionEnum
//            bool isEnumUsed
//            gtVector<apOpenGLStateVariableId> *pFilteredStateVariableIds
// Return Val: bool  - State variables list fount / not found.
// Author:      Sigal Algranaty
// Date:        7/9/2008
// ---------------------------------------------------------------------------
bool gsAnalyzeModeExecutor::getFunctionCallStateVariablesVector(apMonitoredFunctionId calledFunctionId, GLenum functionEnum, bool isEnumUsed , gtVector<apOpenGLStateVariableId>*& pFilteredStateVariableIds)
{
    (void)(isEnumUsed); // unused
    bool retVal = false;

    switch (calledFunctionId)
    {
        case ap_glRotatef:
        case ap_glRotated:
        case ap_glScalef:
        case ap_glScaled:
        case ap_glTranslatef:
        case ap_glTranslated:

            // OpenGL ES functions:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        case ap_glPopMatrix:
        case ap_glMultMatrixx:
        case ap_glRotatex:
        case ap_glTranslatex:
        case ap_glScalex:
#endif
            {
                pFilteredStateVariableIds = &_glMatrixStateVariableIdsVec;
                retVal = true;
                break;
            }

        case ap_glTexCoordPointer:
        {
            pFilteredStateVariableIds = &_glTexCoordPointerStateVariableIdsVec;
            retVal = true;
            break;
        }

        case ap_glColorPointer:
        {
            pFilteredStateVariableIds = &_glColorPointerStateVariableIdsVec;
            retVal = true;
            break;
        }

        case ap_glVertexPointer:
        {
            pFilteredStateVariableIds = &_glVertexPointerStateVariableIdsVec;
            retVal = true;
            break;
        }

        case ap_glNormal3b:
        case ap_glNormal3bv:
        case ap_glNormal3d:
        case ap_glNormal3dv:
        case ap_glNormal3f:
        case ap_glNormal3fv:
        case ap_glNormal3i:
        case ap_glNormal3iv:
        case ap_glNormal3s:
        case ap_glNormal3sv:
        {
            retVal = true;
            pFilteredStateVariableIds = &_glNormalStateVariableIdsVec;
            break;
        }

        case ap_glPixelStoref:
        case ap_glPixelStorei:
        {
            retVal = true;
            pFilteredStateVariableIds = &_glPixelStoreStateVariableIdsVec;
            break;
        }

        case ap_glViewport:
        {
            retVal = true;
            pFilteredStateVariableIds = &_glViewportStateVariableIdsVec;
            break;
        }

        case ap_glEnable:
        case ap_glDisable:
        {
            switch (functionEnum)
            {
                case GL_BLEND:
                {
                    pFilteredStateVariableIds = &_glBlendStateVariableIdsVec;
                    retVal = true;
                    break;
                }

                case GL_ALPHA_TEST:
                {
                    pFilteredStateVariableIds = &_glAlphaTestStateVariableIdsVec;
                    retVal = true;
                    break;
                }

                case GL_AUTO_NORMAL:
                {
                    pFilteredStateVariableIds = &_glAutoNormalStateVariableIdsVec;
                    retVal = true;
                    break;
                }

                case GL_TEXTURE_1D:
                case GL_TEXTURE_2D:
                case GL_TEXTURE_3D:
                {
                    pFilteredStateVariableIds = &_glTextureStateVariableIdsVec;
                    retVal = true;
                    break;
                }

                case GL_CULL_FACE:
                {
                    pFilteredStateVariableIds = &_glCullFaceStateVariableIdsVec;
                    retVal = true;
                    break;
                }

                case GL_LIGHTING:
                case GL_LIGHT0:
                case GL_LIGHT1:
                case GL_LIGHT2:
                case GL_LIGHT3:
                case GL_LIGHT4:
                case GL_LIGHT5:
                case GL_LIGHT6:
                case GL_LIGHT7:
                {
                    pFilteredStateVariableIds = &_glLightingStateVariableIdsVec;
                    retVal = true;
                    break;
                }

                default:
                {
                    retVal = false;
                    break;
                }
            }

            break;
        }

        case ap_glNormalPointer:
        {
            pFilteredStateVariableIds = &_glNormalArrayStateVariableIds;
            retVal = true;
            break;
        }

        case ap_glEnableClientState:
        case ap_glDisableClientState:
        {
            switch (functionEnum)
            {
                case GL_VERTEX_ARRAY:
                {
                    pFilteredStateVariableIds = &_glVertexPointerStateVariableIdsVec;
                    retVal = true;
                    break;
                }

                case GL_TEXTURE_COORD_ARRAY:
                {
                    pFilteredStateVariableIds = &_glTexCoordPointerStateVariableIdsVec;
                    retVal = true;
                    break;
                }

                case GL_NORMAL_ARRAY:
                {
                    pFilteredStateVariableIds = &_glNormalArrayStateVariableIds;
                    retVal = true;
                    break;
                }

                case GL_COLOR_ARRAY:
                {
                    pFilteredStateVariableIds = &_glColorPointerStateVariableIdsVec;
                    retVal = true;
                    break;
                }

                case GL_FOG_COORD_ARRAY:
                {
                    pFilteredStateVariableIds = &_glFogCoordStateVariableIdsVec;
                    retVal = true;
                    break;
                }

                default:
                {
                    retVal = false;
                    break;
                }
            }

            break;
        }

        default:
        {
            retVal = false;
            break;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::getAllBeginEndRelevantStateVariables
// Description: Returns all the variable ids needed for comparison in glBegin - glEnd block
// Arguments: gtVector<int>& pStateVariablesForBeginEndBlock - the state variable ids
// Return Val: void
// Author:      Sigal Algranaty
// Date:        18/8/2008
// ---------------------------------------------------------------------------
void gsAnalyzeModeExecutor::getAllBeginEndRelevantStateVariables(gtVector<apOpenGLStateVariableId>*& pStateVariablesForBeginEndBlock)
{
    pStateVariablesForBeginEndBlock = &_beginEndStateVariableIdsVec;
}


// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::isStateVariableComparisionNeededForFunctionInBeginEndBlock
// Description: Checks if for given function call id, state variable values should be compared. iff The function
//              is a state change function, and is supported within a begin end block, shouldBeCompared would be true
//              and relevantStateVariableIds would contain the state variable ids to compare.
// Arguments: int functionCallId - input - the function call id
//            int*& relevantStateVariableIds - state variable ids
//            int& numOfVariablesToCompare - number of item in relevantStateVariableIds
//            bool& shouldBeCompared - should the function be compared
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        18/8/2008
// ---------------------------------------------------------------------------
bool gsAnalyzeModeExecutor::isStateVariableComparisionNeededForFunctionInBeginEndBlock(int functionCallId, bool& shouldBeCompared)
{
    bool retVal = false;
    shouldBeCompared = false;

    // Make sure that the function is within the monitored function scope:
    GT_IF_WITH_ASSERT((functionCallId < apMonitoredFunctionsAmount) && (functionCallId > 0))
    {
        shouldBeCompared = _isFunctionSupportedInBeginEndBlock[functionCallId];
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsAnalyzeModeExecutor::removeStateVariablesFromVectorByImplementation
// Description: Removes from stateVariablesVector all state variables that do not belong
//              to this OpenGL implementation.
// Author:      Uri Shomroni
// Date:        14/7/2009
// ---------------------------------------------------------------------------
void gsAnalyzeModeExecutor::removeStateVariablesFromVectorByImplementation(gtVector<apOpenGLStateVariableId>& stateVariablesVector)
{
    size_t numberOfItems = stateVariablesVector.size();

    // Will advance only with items that pass the test:
    size_t newVectorIndex = 0;

    unsigned int allowedTypesMask = 0;
    allowedTypesMask = gsStateVariablesSnapshot::getValidStateVariableTypesMask(true, _pRenderContextMonitor);

    // Get the state variables manager:
    apOpenGLStateVariablesManager& theStateVariablesMgr = apOpenGLStateVariablesManager::instance();

    // Loop the original vector (note that always newVectorIndex <= i, and that we can overwrite items we checked
    // even if they are okay, since we duplicate them):
    for (size_t i = 0; i < numberOfItems; i++)
    {
        // Get the variable type:
        apOpenGLStateVariableId currentVariableId = stateVariablesVector[i];
        unsigned int currentVariableType = theStateVariablesMgr.stateVariableGlobalType(currentVariableId);

        // Only Add the item if it is allowed:
        if ((currentVariableType & allowedTypesMask) != 0)
        {
            // This variable is okay, move it to its new index:
            stateVariablesVector[newVectorIndex] = currentVariableId;

            // Advance the new indices counter
            newVectorIndex++;
        }
    }

    // The number of items we skipped is the rest of the vector
    for (size_t i = newVectorIndex; i < numberOfItems; i++)
    {
        stateVariablesVector.pop_back();
    }
}

