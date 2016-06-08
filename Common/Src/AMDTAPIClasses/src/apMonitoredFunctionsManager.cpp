//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMonitoredFunctionsManager.cpp
///
//==================================================================================

//------------------------------ apMonitoredFunctionsManager.cpp ------------------------------

// Standard C:
#include <string.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::MonitoredFunctionData::MonitoredFunctionData
// Description: Default constructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        28/3/2010
// ---------------------------------------------------------------------------
apMonitoredFunctionsManager::MonitoredFunctionData::MonitoredFunctionData()
    : _name(NULL), _apiType(AP_OPENGL_GENERIC_FUNC), _functionType(0), _deprecatedAtVersion(AP_GL_VERSION_NONE), _removedAtVersion(AP_GL_VERSION_NONE)
{
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::MonitoredFunctionData::MonitoredFunctionData
// Description: constructor
// Arguments:   const char* name
//              unsigned int apiType
//              unsigned int functionType
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        28/3/2010
// ---------------------------------------------------------------------------
apMonitoredFunctionsManager::MonitoredFunctionData::MonitoredFunctionData(const wchar_t* name, unsigned int apiType, unsigned int functionType)
    : _name(name), _apiType(apiType), _functionType(functionType) , _deprecatedAtVersion(AP_GL_VERSION_NONE), _removedAtVersion(AP_GL_VERSION_NONE)
{
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::MonitoredFunctionData::MonitoredFunctionData
// Description: Constructor
// Arguments:   const char* name
//              unsigned int apiType
//              unsigned int functionType
//              apAPIVersion deprecatedAtVersion
//              apAPIVersion removedAtVersion
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        28/3/2010
// ---------------------------------------------------------------------------
apMonitoredFunctionsManager::MonitoredFunctionData::MonitoredFunctionData(const wchar_t* name, unsigned int apiType, unsigned int functionType, apAPIVersion deprecatedAtVersion, apAPIVersion removedAtVersion)
    : _name(name), _apiType(apiType), _functionType(functionType) , _deprecatedAtVersion(deprecatedAtVersion), _removedAtVersion(removedAtVersion)
{
}


// Static members initializations:
apMonitoredFunctionsManager* apMonitoredFunctionsManager::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::instance
// Description: Returns the single instance of this class.
// Author:  AMD Developer Tools Team
// Date:        22/4/2004
// ---------------------------------------------------------------------------
apMonitoredFunctionsManager& apMonitoredFunctionsManager::instance()
{
    // If my single instance does not exist - create it:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new apMonitoredFunctionsManager;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::apMonitoredFunctionsManager
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        22/4/2004
// ---------------------------------------------------------------------------
apMonitoredFunctionsManager::apMonitoredFunctionsManager()
{
    // Initialize the _monitoredFunctionsData array:
    initializeMonitoredFunctionsData();

    // Initialize the monitored func name to id map:
    initializeMonitoredFunctionNameToIdMap();
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::~apMonitoredFunctionsManager
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        22/4/2004
// ---------------------------------------------------------------------------
apMonitoredFunctionsManager::~apMonitoredFunctionsManager()
{
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::amountOfMonitoredFunctions
// Description: Returns the amount of monitored functions.
// Author:  AMD Developer Tools Team
// Date:        22/4/2004
// ---------------------------------------------------------------------------
int apMonitoredFunctionsManager::amountOfMonitoredFunctions() const
{
    return apMonitoredFunctionsAmount;
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::monitoredFunctionName
// Description: Inputs a monitored function id and returns its name.
// Author:  AMD Developer Tools Team
// Date:        22/4/2004
// ---------------------------------------------------------------------------
const wchar_t* apMonitoredFunctionsManager::monitoredFunctionName(apMonitoredFunctionId functionId) const
{
    const wchar_t* retVal = NULL;

    // Verify that the input name is in the right range:
    if ((0 <= functionId) && (functionId < apMonitoredFunctionsAmount))
    {
        retVal = _monitoredFunctionsData[functionId]._name;
    }
    else
    {
        // Trigger an assertion failure:
        GT_ASSERT_EX(false , L"Calling an out of index function id");

        // Return an error function name:
        static const wchar_t* pUnknownFuncName = L"Unknown function name";
        retVal = pUnknownFuncName;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::monitoredFunctionId
// Description: Inputs a monitored function name and returns its id.
//              (Or -1 if it was not found)
// Author:  AMD Developer Tools Team
// Date:        8/9/2004
// ---------------------------------------------------------------------------
apMonitoredFunctionId apMonitoredFunctionsManager::monitoredFunctionId(const wchar_t* functionName) const
{
    apMonitoredFunctionId retVal = apMonitoredFunctionsAmount;

    gtString searchStr(functionName);

    gtMap<gtString, apMonitoredFunctionId>::const_iterator iter = _monitoredFunctionNameToId.find(searchStr);

    if (iter != _monitoredFunctionNameToId.end())
    {
        retVal = iter->second;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::monitoredFunctionType
// Description: Inputs a monitored function id and returns its type as a bitwise
//              mask of apFunctionType.
// Author:  AMD Developer Tools Team
// Date:        22/4/2004
// ---------------------------------------------------------------------------
unsigned int apMonitoredFunctionsManager::monitoredFunctionType(apMonitoredFunctionId functionIndex) const
{
    return _monitoredFunctionsData[functionIndex]._functionType;
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::monitoredFunctionAPIType
// Description: Inputs a monitored function id and returns its API type as a bitwise
//              mask of apFunctionType.
// Author:  AMD Developer Tools Team
// Date:        28/3/2010
// ---------------------------------------------------------------------------
unsigned int apMonitoredFunctionsManager::monitoredFunctionAPIType(apMonitoredFunctionId functionIndex) const
{
    return _monitoredFunctionsData[functionIndex]._apiType;
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::getMonitoredFunctionDeprecationVersions
// Description: Inputs a monitored function id and returns its deprecation version,
//              and deprecation remove version
// Arguments: int functionIndex
//            apAPIVersion& deprectedAtVersion
//            apAPIVersion& removedAtVersion
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        25/2/2009
// ---------------------------------------------------------------------------
void apMonitoredFunctionsManager::getMonitoredFunctionDeprecationVersions(apMonitoredFunctionId functionIndex, apAPIVersion& deprecatedAtVersion, apAPIVersion& removedAtVersion) const
{
    deprecatedAtVersion = _monitoredFunctionsData[functionIndex]._deprecatedAtVersion;
    removedAtVersion = _monitoredFunctionsData[functionIndex]._removedAtVersion;
}
// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::initializeMonitoredFunctionsData
// Description: Initialize _monitoredFunctionsData to contain the monitored functions
//              data (names, type, etc).
// Author:  AMD Developer Tools Team
// Date:        10/4/2004
// ---------------------------------------------------------------------------
void apMonitoredFunctionsManager::initializeMonitoredFunctionsData()
{
    // Initialize WGL functions data:
    initializeWGLFunctionsData();

    // Initialize GLX functions data:
    initializeGLXFunctionsData();

    // Initialize CGL functions data:
    initializeCGLFunctionsData();

    // Core OpenGL functions:
    initializeCoreOpenGLFunctionsData();

    // OpenGL 1.1
    // Supported with the base OpenGL implementation

    // OpenGL 1.2
    initializeOpenGL12FunctionsData();

    // OpenGL 1.3
    initializeOpenGL13FunctionsData();

    // OpenGL 1.4
    initializeOpenGL14FunctionsData();

    // OpenGL 1.5
    initializeOpenGL15FunctionsData();

    // OpenGL 2.0
    initializeOpenGL20FunctionsData();

    // OpenGL 2.1
    initializeOpenGL21FunctionsData();

    // OpenGL 3.0
    initializeOpenGL30FunctionsData();

    // OpenGL 3.1
    initializeOpenGL31FunctionsData();

    // OpenGL 3.2
    initializeOpenGL32FunctionsData();

    // OpenGL 3.3
    initializeOpenGL33FunctionsData();

    // OpenGL 4.0
    initializeOpenGL40FunctionsData();

    // OpenGL 4.1
    initializeOpenGL41FunctionsData();

    // OpenGL 4.2
    initializeOpenGL42FunctionsData();

    // OpenGL 4.3
    initializeOpenGL43FunctionsData();

    // OpenGL 4.4
    initializeOpenGL44FunctionsData();

    // OpenGL 4.5
    initializeOpenGL45FunctionsData();

    // OpenGL Extension functions:
    initializeOpenGLExtensionFunctionsData();

    // EGL functions:
    initializeEGLFunctionsData();

    // EGL functions:
    initializeEGLExtensionsFunctionsData();

    // OpenGL ES Core functions:
    initializeCoreOpenGLESFunctionsData();

    // OpenGL ES extension functions:
    initializeOpenGLESExtensionsFunctionsData();

    // OpenCL 1.0 functions:
    initializeOpenCL10FunctionsData();

    // OpenCL 1.1 functions:
    initializeOpenCL11FunctionsData();

    // OpenCL 1.2 functions:
    initializeOpenCL12FunctionsData();

    // OpenCL 2.0 functions:
    initializeOpenCL20FunctionsData();

    // OpenCL Extension functions:
    initializeOpenCLExtensionFunctionsData();
}


void apMonitoredFunctionsManager::initializeCoreOpenGLFunctionsData()
{
    _monitoredFunctionsData[ap_glAccum] = MonitoredFunctionData(L"glAccum", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glAlphaFunc] = MonitoredFunctionData(L"glAlphaFunc", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC , AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glAreTexturesResident] = MonitoredFunctionData(L"glAreTexturesResident", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glArrayElement] = MonitoredFunctionData(L"glArrayElement", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glBegin] = MonitoredFunctionData(L"glBegin", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glBindTexture] = MonitoredFunctionData(L"glBindTexture", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glBitmap] = MonitoredFunctionData(L"glBitmap", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glBlendFunc] = MonitoredFunctionData(L"glBlendFunc", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glCallList] = MonitoredFunctionData(L"glCallList", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glCallLists] = MonitoredFunctionData(L"glCallLists", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glClear] = MonitoredFunctionData(L"glClear", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glClearAccum] = MonitoredFunctionData(L"glClearAccum", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glClearColor] = MonitoredFunctionData(L"glClearColor", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glClearDepth] = MonitoredFunctionData(L"glClearDepth", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glClearIndex] = MonitoredFunctionData(L"glClearIndex", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glClearStencil] = MonitoredFunctionData(L"glClearStencil", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glClipPlane] = MonitoredFunctionData(L"glClipPlane", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glColor3b] = MonitoredFunctionData(L"glColor3b", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3bv] = MonitoredFunctionData(L"glColor3bv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3d] = MonitoredFunctionData(L"glColor3d", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3dv] = MonitoredFunctionData(L"glColor3dv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3f] = MonitoredFunctionData(L"glColor3f", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3fv] = MonitoredFunctionData(L"glColor3fv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3i] = MonitoredFunctionData(L"glColor3i", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3iv] = MonitoredFunctionData(L"glColor3iv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3s] = MonitoredFunctionData(L"glColor3s", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3sv] = MonitoredFunctionData(L"glColor3sv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3ub] = MonitoredFunctionData(L"glColor3ub", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3ubv] = MonitoredFunctionData(L"glColor3ubv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3ui] = MonitoredFunctionData(L"glColor3ui", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3uiv] = MonitoredFunctionData(L"glColor3uiv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3us] = MonitoredFunctionData(L"glColor3us", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor3usv] = MonitoredFunctionData(L"glColor3usv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4b] = MonitoredFunctionData(L"glColor4b", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4bv] = MonitoredFunctionData(L"glColor4bv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4d] = MonitoredFunctionData(L"glColor4d", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4dv] = MonitoredFunctionData(L"glColor4dv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4f] = MonitoredFunctionData(L"glColor4f", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4fv] = MonitoredFunctionData(L"glColor4fv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4i] = MonitoredFunctionData(L"glColor4i", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4iv] = MonitoredFunctionData(L"glColor4iv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4s] = MonitoredFunctionData(L"glColor4s", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4sv] = MonitoredFunctionData(L"glColor4sv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4ub] = MonitoredFunctionData(L"glColor4ub", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4ubv] = MonitoredFunctionData(L"glColor4ubv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4ui] = MonitoredFunctionData(L"glColor4ui", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4uiv] = MonitoredFunctionData(L"glColor4uiv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4us] = MonitoredFunctionData(L"glColor4us", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColor4usv] = MonitoredFunctionData(L"glColor4usv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColorMask] = MonitoredFunctionData(L"glColorMask", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glColorMaterial] = MonitoredFunctionData(L"glColorMaterial", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glColorPointer] = MonitoredFunctionData(L"glColorPointer", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glCopyPixels] = MonitoredFunctionData(L"glCopyPixels", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glCopyTexImage1D] = MonitoredFunctionData(L"glCopyTexImage1D", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTexImage2D] = MonitoredFunctionData(L"glCopyTexImage2D", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTexSubImage1D] = MonitoredFunctionData(L"glCopyTexSubImage1D", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTexSubImage2D] = MonitoredFunctionData(L"glCopyTexSubImage2D", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCullFace] = MonitoredFunctionData(L"glCullFace", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDeleteLists] = MonitoredFunctionData(L"glDeleteLists", AP_OPENGL_GENERIC_FUNC, AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glDeleteTextures] = MonitoredFunctionData(L"glDeleteTextures", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDepthFunc] = MonitoredFunctionData(L"glDepthFunc", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDepthMask] = MonitoredFunctionData(L"glDepthMask", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDepthRange] = MonitoredFunctionData(L"glDepthRange", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDisable] = MonitoredFunctionData(L"glDisable", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDisableClientState] = MonitoredFunctionData(L"glDisableClientState", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glDrawArrays] = MonitoredFunctionData(L"glDrawArrays", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glDrawBuffer] = MonitoredFunctionData(L"glDrawBuffer", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDrawElements] = MonitoredFunctionData(L"glDrawElements", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glDrawPixels] = MonitoredFunctionData(L"glDrawPixels", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEdgeFlag] = MonitoredFunctionData(L"glEdgeFlag", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEdgeFlagPointer] = MonitoredFunctionData(L"glEdgeFlagPointer", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEdgeFlagv] = MonitoredFunctionData(L"glEdgeFlagv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEnable] = MonitoredFunctionData(L"glEnable", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glEnableClientState] = MonitoredFunctionData(L"glEnableClientState", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEnd] = MonitoredFunctionData(L"glEnd", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEndList] = MonitoredFunctionData(L"glEndList", AP_OPENGL_GENERIC_FUNC, AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalCoord1d] = MonitoredFunctionData(L"glEvalCoord1d", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalCoord1dv] = MonitoredFunctionData(L"glEvalCoord1dv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalCoord1f] = MonitoredFunctionData(L"glEvalCoord1f", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalCoord1fv] = MonitoredFunctionData(L"glEvalCoord1fv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalCoord2d] = MonitoredFunctionData(L"glEvalCoord2d", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalCoord2dv] = MonitoredFunctionData(L"glEvalCoord2dv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalCoord2f] = MonitoredFunctionData(L"glEvalCoord2f", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalCoord2fv] = MonitoredFunctionData(L"glEvalCoord2fv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalMesh1] = MonitoredFunctionData(L"glEvalMesh1", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalMesh2] = MonitoredFunctionData(L"glEvalMesh2", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalPoint1] = MonitoredFunctionData(L"glEvalPoint1", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glEvalPoint2] = MonitoredFunctionData(L"glEvalPoint2", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glFeedbackBuffer] = MonitoredFunctionData(L"glFeedbackBuffer", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glFinish] = MonitoredFunctionData(L"glFinish", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_DRAW_FUNC  | AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glFlush] = MonitoredFunctionData(L"glFlush", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_DRAW_FUNC | AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glFogf] = MonitoredFunctionData(L"glFogf", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glFogfv] = MonitoredFunctionData(L"glFogfv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glFogi] = MonitoredFunctionData(L"glFogi", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glFogiv] = MonitoredFunctionData(L"glFogiv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glFrontFace] = MonitoredFunctionData(L"glFrontFace", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glFrustum] = MonitoredFunctionData(L"glFrustum", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_MATRIX_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glGenLists] = MonitoredFunctionData(L"glGenLists", AP_OPENGL_GENERIC_FUNC, AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glGenTextures] = MonitoredFunctionData(L"glGenTextures", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetBooleanv] = MonitoredFunctionData(L"glGetBooleanv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetClipPlane] = MonitoredFunctionData(L"glGetClipPlane", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetDoublev] = MonitoredFunctionData(L"glGetDoublev", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetError] = MonitoredFunctionData(L"glGetError", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetFloatv] = MonitoredFunctionData(L"glGetFloatv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetIntegerv] = MonitoredFunctionData(L"glGetIntegerv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetLightfv] = MonitoredFunctionData(L"glGetLightfv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glGetLightiv] = MonitoredFunctionData(L"glGetLightiv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glGetMapdv] = MonitoredFunctionData(L"glGetMapdv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetMapfv] = MonitoredFunctionData(L"glGetMapfv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetMapiv] = MonitoredFunctionData(L"glGetMapiv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetMaterialfv] = MonitoredFunctionData(L"glGetMaterialfv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetMaterialiv] = MonitoredFunctionData(L"glGetMaterialiv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetPixelMapfv] = MonitoredFunctionData(L"glGetPixelMapfv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetPixelMapuiv] = MonitoredFunctionData(L"glGetPixelMapuiv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetPixelMapusv] = MonitoredFunctionData(L"glGetPixelMapusv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetPointerv] = MonitoredFunctionData(L"glGetPointerv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetPolygonStipple] = MonitoredFunctionData(L"glGetPolygonStipple", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetString] = MonitoredFunctionData(L"glGetString", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetTexEnvfv] = MonitoredFunctionData(L"glGetTexEnvfv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexEnviv] = MonitoredFunctionData(L"glGetTexEnviv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexGendv] = MonitoredFunctionData(L"glGetTexGendv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexGenfv] = MonitoredFunctionData(L"glGetTexGenfv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexGeniv] = MonitoredFunctionData(L"glGetTexGeniv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexImage] = MonitoredFunctionData(L"glGetTexImage", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexLevelParameterfv] = MonitoredFunctionData(L"glGetTexLevelParameterfv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexLevelParameteriv] = MonitoredFunctionData(L"glGetTexLevelParameteriv", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexParameterfv] = MonitoredFunctionData(L"glGetTexParameterfv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexParameteriv] = MonitoredFunctionData(L"glGetTexParameteriv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glHint] = MonitoredFunctionData(L"glHint", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glIndexMask] = MonitoredFunctionData(L"glIndexMask", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIndexPointer] = MonitoredFunctionData(L"glIndexPointer", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIndexd] = MonitoredFunctionData(L"glIndexd", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIndexdv] = MonitoredFunctionData(L"glIndexdv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIndexf] = MonitoredFunctionData(L"glIndexf", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIndexfv] = MonitoredFunctionData(L"glIndexfv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIndexi] = MonitoredFunctionData(L"glIndexi", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIndexiv] = MonitoredFunctionData(L"glIndexiv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIndexs] = MonitoredFunctionData(L"glIndexs", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIndexsv] = MonitoredFunctionData(L"glIndexsv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIndexub] = MonitoredFunctionData(L"glIndexub", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIndexubv] = MonitoredFunctionData(L"glIndexubv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glInitNames] = MonitoredFunctionData(L"glInitNames", AP_OPENGL_GENERIC_FUNC, AP_NAME_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glInterleavedArrays] = MonitoredFunctionData(L"glInterleavedArrays", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glIsEnabled] = MonitoredFunctionData(L"glIsEnabled", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glIsList] = MonitoredFunctionData(L"glIsList", AP_OPENGL_GENERIC_FUNC, AP_GET_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glIsTexture] = MonitoredFunctionData(L"glIsTexture", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glLightModelf] = MonitoredFunctionData(L"glLightModelf", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLightModelfv] = MonitoredFunctionData(L"glLightModelfv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLightModeli] = MonitoredFunctionData(L"glLightModeli", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLightModeliv] = MonitoredFunctionData(L"glLightModeliv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLightf] = MonitoredFunctionData(L"glLightf", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLightfv] = MonitoredFunctionData(L"glLightfv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLighti] = MonitoredFunctionData(L"glLighti", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLightiv] = MonitoredFunctionData(L"glLightiv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLineStipple] = MonitoredFunctionData(L"glLineStipple", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLineWidth] = MonitoredFunctionData(L"glLineWidth", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glListBase] = MonitoredFunctionData(L"glListBase", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLoadIdentity] = MonitoredFunctionData(L"glLoadIdentity", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLoadMatrixd] = MonitoredFunctionData(L"glLoadMatrixd", AP_OPENGL_GENERIC_FUNC, AP_MATRIX_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLoadMatrixf] = MonitoredFunctionData(L"glLoadMatrixf", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLoadName] = MonitoredFunctionData(L"glLoadName", AP_OPENGL_GENERIC_FUNC, AP_NAME_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLogicOp] = MonitoredFunctionData(L"glLogicOp", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMap1d] = MonitoredFunctionData(L"glMap1d", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMap1f] = MonitoredFunctionData(L"glMap1f", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMap2d] = MonitoredFunctionData(L"glMap2d", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMap2f] = MonitoredFunctionData(L"glMap2f", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMapGrid1d] = MonitoredFunctionData(L"glMapGrid1d", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMapGrid1f] = MonitoredFunctionData(L"glMapGrid1f", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMapGrid2d] = MonitoredFunctionData(L"glMapGrid2d", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMapGrid2f] = MonitoredFunctionData(L"glMapGrid2f", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMaterialf] = MonitoredFunctionData(L"glMaterialf", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMaterialfv] = MonitoredFunctionData(L"glMaterialfv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMateriali] = MonitoredFunctionData(L"glMateriali", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMaterialiv] = MonitoredFunctionData(L"glMaterialiv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMatrixMode] = MonitoredFunctionData(L"glMatrixMode", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMultMatrixd] = MonitoredFunctionData(L"glMultMatrixd", AP_OPENGL_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMultMatrixf] = MonitoredFunctionData(L"glMultMatrixf", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNewList] = MonitoredFunctionData(L"glNewList", AP_OPENGL_GENERIC_FUNC, AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNormal3b] = MonitoredFunctionData(L"glNormal3b", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNormal3bv] = MonitoredFunctionData(L"glNormal3bv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNormal3d] = MonitoredFunctionData(L"glNormal3d", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNormal3dv] = MonitoredFunctionData(L"glNormal3dv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNormal3f] = MonitoredFunctionData(L"glNormal3f", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNormal3fv] = MonitoredFunctionData(L"glNormal3fv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNormal3i] = MonitoredFunctionData(L"glNormal3i", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNormal3iv] = MonitoredFunctionData(L"glNormal3iv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNormal3s] = MonitoredFunctionData(L"glNormal3s", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNormal3sv] = MonitoredFunctionData(L"glNormal3sv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glNormalPointer] = MonitoredFunctionData(L"glNormalPointer", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glOrtho] = MonitoredFunctionData(L"glOrtho", AP_OPENGL_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPassThrough] = MonitoredFunctionData(L"glPassThrough", AP_OPENGL_GENERIC_FUNC, AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPixelMapfv] = MonitoredFunctionData(L"glPixelMapfv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPixelMapuiv] = MonitoredFunctionData(L"glPixelMapuiv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPixelMapusv] = MonitoredFunctionData(L"glPixelMapusv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPixelStoref] = MonitoredFunctionData(L"glPixelStoref", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPixelStorei] = MonitoredFunctionData(L"glPixelStorei", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPixelTransferf] = MonitoredFunctionData(L"glPixelTransferf", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPixelTransferi] = MonitoredFunctionData(L"glPixelTransferi", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPixelZoom] = MonitoredFunctionData(L"glPixelZoom", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPointSize] = MonitoredFunctionData(L"glPointSize", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPolygonMode] = MonitoredFunctionData(L"glPolygonMode", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPolygonOffset] = MonitoredFunctionData(L"glPolygonOffset", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPolygonStipple] = MonitoredFunctionData(L"glPolygonStipple", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPopAttrib] = MonitoredFunctionData(L"glPopAttrib", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPopClientAttrib] = MonitoredFunctionData(L"glPopClientAttrib", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPopMatrix] = MonitoredFunctionData(L"glPopMatrix", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPopName] = MonitoredFunctionData(L"glPopName", AP_OPENGL_GENERIC_FUNC, AP_NAME_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPrioritizeTextures] = MonitoredFunctionData(L"glPrioritizeTextures", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPushAttrib] = MonitoredFunctionData(L"glPushAttrib", AP_OPENGL_GENERIC_FUNC, AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPushClientAttrib] = MonitoredFunctionData(L"glPushClientAttrib", AP_OPENGL_GENERIC_FUNC, AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPushMatrix] = MonitoredFunctionData(L"glPushMatrix", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glPushName] = MonitoredFunctionData(L"glPushName", AP_OPENGL_GENERIC_FUNC, AP_NAME_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos2d] = MonitoredFunctionData(L"glRasterPos2d", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos2dv] = MonitoredFunctionData(L"glRasterPos2dv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos2f] = MonitoredFunctionData(L"glRasterPos2f", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos2fv] = MonitoredFunctionData(L"glRasterPos2fv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos2i] = MonitoredFunctionData(L"glRasterPos2i", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos2iv] = MonitoredFunctionData(L"glRasterPos2iv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos2s] = MonitoredFunctionData(L"glRasterPos2s", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos2sv] = MonitoredFunctionData(L"glRasterPos2sv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos3d] = MonitoredFunctionData(L"glRasterPos3d", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos3dv] = MonitoredFunctionData(L"glRasterPos3dv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos3f] = MonitoredFunctionData(L"glRasterPos3f", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos3fv] = MonitoredFunctionData(L"glRasterPos3fv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos3i] = MonitoredFunctionData(L"glRasterPos3i", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos3iv] = MonitoredFunctionData(L"glRasterPos3iv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos3s] = MonitoredFunctionData(L"glRasterPos3s", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos3sv] = MonitoredFunctionData(L"glRasterPos3sv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos4d] = MonitoredFunctionData(L"glRasterPos4d", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos4dv] = MonitoredFunctionData(L"glRasterPos4dv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos4f] = MonitoredFunctionData(L"glRasterPos4f", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos4fv] = MonitoredFunctionData(L"glRasterPos4fv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos4i] = MonitoredFunctionData(L"glRasterPos4i", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos4iv] = MonitoredFunctionData(L"glRasterPos4iv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos4s] = MonitoredFunctionData(L"glRasterPos4s", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRasterPos4sv] = MonitoredFunctionData(L"glRasterPos4sv", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glReadBuffer] = MonitoredFunctionData(L"glReadBuffer", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glReadPixels] = MonitoredFunctionData(L"glReadPixels", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_RASTER_FUNC);
    _monitoredFunctionsData[ap_glRectd] = MonitoredFunctionData(L"glRectd", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRectdv] = MonitoredFunctionData(L"glRectdv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRectf] = MonitoredFunctionData(L"glRectf", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRectfv] = MonitoredFunctionData(L"glRectfv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRecti] = MonitoredFunctionData(L"glRecti", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRectiv] = MonitoredFunctionData(L"glRectiv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRects] = MonitoredFunctionData(L"glRects", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRectsv] = MonitoredFunctionData(L"glRectsv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRenderMode] = MonitoredFunctionData(L"glRenderMode", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRotated] = MonitoredFunctionData(L"glRotated", AP_OPENGL_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glRotatef] = MonitoredFunctionData(L"glRotatef", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glScaled] = MonitoredFunctionData(L"glScaled", AP_OPENGL_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glScalef] = MonitoredFunctionData(L"glScalef", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glScissor] = MonitoredFunctionData(L"glScissor", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glSelectBuffer] = MonitoredFunctionData(L"glSelectBuffer", AP_OPENGL_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_NAME_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glShadeModel] = MonitoredFunctionData(L"glShadeModel", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glStencilFunc] = MonitoredFunctionData(L"glStencilFunc", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glStencilMask] = MonitoredFunctionData(L"glStencilMask", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glStencilOp] = MonitoredFunctionData(L"glStencilOp", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glTexCoord1d] = MonitoredFunctionData(L"glTexCoord1d", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord1dv] = MonitoredFunctionData(L"glTexCoord1dv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord1f] = MonitoredFunctionData(L"glTexCoord1f", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord1fv] = MonitoredFunctionData(L"glTexCoord1fv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord1i] = MonitoredFunctionData(L"glTexCoord1i", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord1iv] = MonitoredFunctionData(L"glTexCoord1iv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord1s] = MonitoredFunctionData(L"glTexCoord1s", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord1sv] = MonitoredFunctionData(L"glTexCoord1sv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord2d] = MonitoredFunctionData(L"glTexCoord2d", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord2dv] = MonitoredFunctionData(L"glTexCoord2dv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord2f] = MonitoredFunctionData(L"glTexCoord2f", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord2fv] = MonitoredFunctionData(L"glTexCoord2fv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord2i] = MonitoredFunctionData(L"glTexCoord2i", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord2iv] = MonitoredFunctionData(L"glTexCoord2iv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord2s] = MonitoredFunctionData(L"glTexCoord2s", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord2sv] = MonitoredFunctionData(L"glTexCoord2sv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord3d] = MonitoredFunctionData(L"glTexCoord3d", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord3dv] = MonitoredFunctionData(L"glTexCoord3dv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord3f] = MonitoredFunctionData(L"glTexCoord3f", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord3fv] = MonitoredFunctionData(L"glTexCoord3fv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord3i] = MonitoredFunctionData(L"glTexCoord3i", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord3iv] = MonitoredFunctionData(L"glTexCoord3iv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord3s] = MonitoredFunctionData(L"glTexCoord3s", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord3sv] = MonitoredFunctionData(L"glTexCoord3sv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord4d] = MonitoredFunctionData(L"glTexCoord4d", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord4dv] = MonitoredFunctionData(L"glTexCoord4dv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord4f] = MonitoredFunctionData(L"glTexCoord4f", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord4fv] = MonitoredFunctionData(L"glTexCoord4fv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord4i] = MonitoredFunctionData(L"glTexCoord4i", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord4iv] = MonitoredFunctionData(L"glTexCoord4iv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord4s] = MonitoredFunctionData(L"glTexCoord4s", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoord4sv] = MonitoredFunctionData(L"glTexCoord4sv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexCoordPointer] = MonitoredFunctionData(L"glTexCoordPointer", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTexEnvf] = MonitoredFunctionData(L"glTexEnvf", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexEnvfv] = MonitoredFunctionData(L"glTexEnvfv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexEnvi] = MonitoredFunctionData(L"glTexEnvi", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexEnviv] = MonitoredFunctionData(L"glTexEnviv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexGend] = MonitoredFunctionData(L"glTexGend", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexGendv] = MonitoredFunctionData(L"glTexGendv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexGenf] = MonitoredFunctionData(L"glTexGenf", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexGenfv] = MonitoredFunctionData(L"glTexGenfv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexGeni] = MonitoredFunctionData(L"glTexGeni", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexGeniv] = MonitoredFunctionData(L"glTexGeniv", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexImage1D] = MonitoredFunctionData(L"glTexImage1D", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexImage2D] = MonitoredFunctionData(L"glTexImage2D", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexParameterf] = MonitoredFunctionData(L"glTexParameterf", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexParameterfv] = MonitoredFunctionData(L"glTexParameterfv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexParameteri] = MonitoredFunctionData(L"glTexParameteri", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexParameteriv] = MonitoredFunctionData(L"glTexParameteriv", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexSubImage1D] = MonitoredFunctionData(L"glTexSubImage1D", AP_OPENGL_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexSubImage2D] = MonitoredFunctionData(L"glTexSubImage2D", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTranslated] = MonitoredFunctionData(L"glTranslated", AP_OPENGL_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glTranslatef] = MonitoredFunctionData(L"glTranslatef", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex2d] = MonitoredFunctionData(L"glVertex2d", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex2dv] = MonitoredFunctionData(L"glVertex2dv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex2f] = MonitoredFunctionData(L"glVertex2f", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex2fv] = MonitoredFunctionData(L"glVertex2fv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex2i] = MonitoredFunctionData(L"glVertex2i", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex2iv] = MonitoredFunctionData(L"glVertex2iv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex2s] = MonitoredFunctionData(L"glVertex2s", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex2sv] = MonitoredFunctionData(L"glVertex2sv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex3d] = MonitoredFunctionData(L"glVertex3d", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex3dv] = MonitoredFunctionData(L"glVertex3dv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex3f] = MonitoredFunctionData(L"glVertex3f", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex3fv] = MonitoredFunctionData(L"glVertex3fv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex3i] = MonitoredFunctionData(L"glVertex3i", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex3iv] = MonitoredFunctionData(L"glVertex3iv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex3s] = MonitoredFunctionData(L"glVertex3s", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex3sv] = MonitoredFunctionData(L"glVertex3sv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex4d] = MonitoredFunctionData(L"glVertex4d", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex4dv] = MonitoredFunctionData(L"glVertex4dv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex4f] = MonitoredFunctionData(L"glVertex4f", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex4fv] = MonitoredFunctionData(L"glVertex4fv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex4i] = MonitoredFunctionData(L"glVertex4i", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex4iv] = MonitoredFunctionData(L"glVertex4iv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex4s] = MonitoredFunctionData(L"glVertex4s", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertex4sv] = MonitoredFunctionData(L"glVertex4sv", AP_OPENGL_GENERIC_FUNC, AP_DRAW_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glVertexPointer] = MonitoredFunctionData(L"glVertexPointer", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glViewport] = MonitoredFunctionData(L"glViewport", AP_OPENGL_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
}


void apMonitoredFunctionsManager::initializeCoreOpenGLESFunctionsData()
{
    // OpenGL ES is currently supported on Windows & Mac only:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    {
        _monitoredFunctionsData[ap_glAlphaFuncx] = MonitoredFunctionData(L"glAlphaFuncx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glClearColorx] = MonitoredFunctionData(L"glClearColorx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        // _monitoredFunctionsData[ap_glClearDepthf] = MonitoredFunctionData(L"glClearDepthf", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC); // Appears in OpenGL 4.1
        _monitoredFunctionsData[ap_glClearDepthx] = MonitoredFunctionData(L"glClearDepthx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glClipPlanef] = MonitoredFunctionData(L"glClipPlanef", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
        _monitoredFunctionsData[ap_glClipPlanex] = MonitoredFunctionData(L"glClipPlanex", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
        _monitoredFunctionsData[ap_glColor4x] = MonitoredFunctionData(L"glColor4x", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        // _monitoredFunctionsData[ap_glDepthRangef] = MonitoredFunctionData(L"glDepthRangef", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC); // Appears in OpenGL 4.1
        _monitoredFunctionsData[ap_glDepthRangex] = MonitoredFunctionData(L"glDepthRangex", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glFogx] = MonitoredFunctionData(L"glFogx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glFogxv] = MonitoredFunctionData(L"glFogxv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glFrustumf] = MonitoredFunctionData(L"glFrustumf", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC);
        _monitoredFunctionsData[ap_glFrustumx] = MonitoredFunctionData(L"glFrustumx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC);
        _monitoredFunctionsData[ap_glGetClipPlanef] = MonitoredFunctionData(L"glGetClipPlanef", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC);
        _monitoredFunctionsData[ap_glGetClipPlanex] = MonitoredFunctionData(L"glGetClipPlanex", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC);
        _monitoredFunctionsData[ap_glGetFixedv] = MonitoredFunctionData(L"glGetFixedv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC);
        _monitoredFunctionsData[ap_glGetLightxv] = MonitoredFunctionData(L"glGetLightxv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC);
        _monitoredFunctionsData[ap_glGetMaterialxv] = MonitoredFunctionData(L"glGetMaterialxv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC);
        _monitoredFunctionsData[ap_glGetTexEnvxv] = MonitoredFunctionData(L"glGetTexEnvxv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC);
        _monitoredFunctionsData[ap_glGetTexParameterxv] = MonitoredFunctionData(L"glGetTexParameterxv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
        _monitoredFunctionsData[ap_glLightModelx] = MonitoredFunctionData(L"glLightModelx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
        _monitoredFunctionsData[ap_glLightModelxv] = MonitoredFunctionData(L"glLightModelxv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
        _monitoredFunctionsData[ap_glLightx] = MonitoredFunctionData(L"glLightx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
        _monitoredFunctionsData[ap_glLightxv] = MonitoredFunctionData(L"glLightxv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
        _monitoredFunctionsData[ap_glLineWidthx] = MonitoredFunctionData(L"glLineWidthx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glLoadMatrixx] = MonitoredFunctionData(L"glLoadMatrixx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC);
        _monitoredFunctionsData[ap_glMaterialx] = MonitoredFunctionData(L"glMaterialx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glMaterialxv] = MonitoredFunctionData(L"glMaterialxv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glMultMatrixx] = MonitoredFunctionData(L"glMultMatrixx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glMultiTexCoord4x] = MonitoredFunctionData(L"glMultiTexCoord4x", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glNormal3x] = MonitoredFunctionData(L"glNormal3x", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glOrthof] = MonitoredFunctionData(L"glOrthof", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC);
        _monitoredFunctionsData[ap_glOrthox] = MonitoredFunctionData(L"glOrthox", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC);
        _monitoredFunctionsData[ap_glPointParameterx] = MonitoredFunctionData(L"glPointParameterx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glPointParameterxv] = MonitoredFunctionData(L"glPointParameterxv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glPointSizex] = MonitoredFunctionData(L"glPointSizex", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glPolygonOffsetx] = MonitoredFunctionData(L"glPolygonOffsetx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glRotatex] = MonitoredFunctionData(L"glRotatex", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC);
        _monitoredFunctionsData[ap_glSampleCoveragex] = MonitoredFunctionData(L"glSampleCoveragex", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glScalex] = MonitoredFunctionData(L"glScalex", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC);
        _monitoredFunctionsData[ap_glTexEnvx] = MonitoredFunctionData(L"glTexEnvx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glTexEnvxv] = MonitoredFunctionData(L"glTexEnvxv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
        _monitoredFunctionsData[ap_glTexParameterx] = MonitoredFunctionData(L"glTexParameterx", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
        _monitoredFunctionsData[ap_glTexParameterxv] = MonitoredFunctionData(L"glTexParameterxv", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
        _monitoredFunctionsData[ap_glTranslatex] = MonitoredFunctionData(L"glTranslatex", AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_MATRIX_FUNC);
    }
#endif // (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::initializeOpenGLESExtensionsFunctionsData
// Description: Initialize the EGL functions data.
// Author:  AMD Developer Tools Team
// Date:        23/2/2006
// ---------------------------------------------------------------------------
void apMonitoredFunctionsManager::initializeOpenGLESExtensionsFunctionsData()
{
    // OpenGL ES is currently supported on Windows & Mac only:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    _monitoredFunctionsData[ap_glDrawTexsOES] = MonitoredFunctionData(L"glDrawTexsOES", AP_OPENGL_ES_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexiOES] = MonitoredFunctionData(L"glDrawTexiOES", AP_OPENGL_ES_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexxOES] = MonitoredFunctionData(L"glDrawTexxOES", AP_OPENGL_ES_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexsvOES] = MonitoredFunctionData(L"glDrawTexsvOES", AP_OPENGL_ES_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexivOES] = MonitoredFunctionData(L"glDrawTexivOES", AP_OPENGL_ES_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexxvOES] = MonitoredFunctionData(L"glDrawTexxvOES", AP_OPENGL_ES_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexfOES] = MonitoredFunctionData(L"glDrawTexfOES", AP_OPENGL_ES_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexfvOES] = MonitoredFunctionData(L"glDrawTexfvOES", AP_OPENGL_ES_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);

#endif // (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    //////////////////////////////////////////////////////////////////////////
    // OES_draw_texture
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_glDrawTexsOES] = MonitoredFunctionData(L"glDrawTexsOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexiOES] = MonitoredFunctionData(L"glDrawTexiOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexxOES] = MonitoredFunctionData(L"glDrawTexxOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexsvOES] = MonitoredFunctionData(L"glDrawTexsvOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexivOES] = MonitoredFunctionData(L"glDrawTexivOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexxvOES] = MonitoredFunctionData(L"glDrawTexxvOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexfOES] = MonitoredFunctionData(L"glDrawTexfOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTexfvOES] = MonitoredFunctionData(L"glDrawTexfvOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_DRAW_FUNC | AP_RASTER_FUNC | AP_TEXTURE_FUNC);

    //////////////////////////////////////////////////////////////////////////
    // OES_blend_subtract
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_glBlendEquationOES] = MonitoredFunctionData(L"glBlendEquationOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, 0);

    //////////////////////////////////////////////////////////////////////////
    // OES_framebuffer_object
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_glIsRenderbufferOES] = MonitoredFunctionData(L"glIsRenderbufferOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glBindRenderbufferOES] = MonitoredFunctionData(L"glBindRenderbufferOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glDeleteRenderbuffersOES] = MonitoredFunctionData(L"glDeleteRenderbuffersOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGenRenderbuffersOES] = MonitoredFunctionData(L"glGenRenderbuffersOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glRenderbufferStorageOES] = MonitoredFunctionData(L"glRenderbufferStorageOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetRenderbufferParameterivOES] = MonitoredFunctionData(L"glGetRenderbufferParameterivOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glIsFramebufferOES] = MonitoredFunctionData(L"glIsFramebufferOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glBindFramebufferOES] = MonitoredFunctionData(L"glBindFramebufferOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glDeleteFramebuffersOES] = MonitoredFunctionData(L"glDeleteFramebuffersOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGenFramebuffersOES] = MonitoredFunctionData(L"glGenFramebuffersOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glCheckFramebufferStatusOES] = MonitoredFunctionData(L"glCheckFramebufferStatusOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferRenderbufferOES] = MonitoredFunctionData(L"glFramebufferRenderbufferOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTexture2DOES] = MonitoredFunctionData(L"glFramebufferTexture2DOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetFramebufferAttachmentParameterivOES] = MonitoredFunctionData(L"glGetFramebufferAttachmentParameterivOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGenerateMipmapOES] = MonitoredFunctionData(L"glGenerateMipmapOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_TEXTURE_FUNC);

    //////////////////////////////////////////////////////////////////////////
    // OES_mapbuffer
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_glGetBufferPointervOES] = MonitoredFunctionData(L"glGetBufferPointervOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glMapBufferOES] = MonitoredFunctionData(L"glMapBufferOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glUnmapBufferOES] = MonitoredFunctionData(L"glUnmapBufferOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_BUFFER_FUNC);

    //////////////////////////////////////////////////////////////////////////
    // OES_matrix_palette
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_glCurrentPaletteMatrixOES] = MonitoredFunctionData(L"glCurrentPaletteMatrixOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glLoadPaletteFromModelViewMatrixOES] = MonitoredFunctionData(L"glLoadPaletteFromModelViewMatrixOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixIndexPointerOES] = MonitoredFunctionData(L"glMatrixIndexPointerOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWeightPointerOES] = MonitoredFunctionData(L"glWeightPointerOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);

    //////////////////////////////////////////////////////////////////////////
    // OES_point_size_array
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_glPointSizePointerOES] = MonitoredFunctionData(L"glPointSizePointerOES", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    //////////////////////////////////////////////////////////////////////////
    // OpenGL ES 2.0
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_glGetShaderPrecisionFormat] = MonitoredFunctionData(L"glGetShaderPrecisionFormat", AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glShaderBinary] = MonitoredFunctionData(L"glShaderBinary", AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glReleaseShaderCompiler] = MonitoredFunctionData(L"glReleaseShaderCompiler", AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);

    //////////////////////////////////////////////////////////////////////////
    // EAGL Objective-C functions (these are marked as extension functions, since
    // they are implemented in Objective-C, so they have no external implementations
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_EAGLContext_initWithAPI] = MonitoredFunctionData(L"-[EAGLContext initWithAPI:]", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_EAGLContext_initWithAPI_sharegroup] = MonitoredFunctionData(L"-[EAGLContext initWithAPI: sharegroup:]", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_EAGLContext_dealloc] = MonitoredFunctionData(L"-[EAGLContext dealloc]", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_EAGLContext_setCurrentContext] = MonitoredFunctionData(L"+[EAGLContext setCurrentContext:]", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_EAGLContext_currentContext] = MonitoredFunctionData(L"+[EAGLContext currentContext]", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_EAGLContext_API] = MonitoredFunctionData(L"-[EAGLContext API]", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_EAGLContext_sharegroup] = MonitoredFunctionData(L"-[EAGLContext sharegroup]", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_EAGLContext_renderbufferStorage_fromDrawable] = MonitoredFunctionData(L"-[EAGLContext renderbufferStorage: fromDrawable:]", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_EAGLContext_presentRenderbuffer] = MonitoredFunctionData(L"-[EAGLContext presentRenderbuffer:]", AP_OPENGL_ES_MAC_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);

    //////////////////////////////////////////////////////////////////////////
    // EAGL C functions
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_EAGLGetVersion] = MonitoredFunctionData(L"EAGLGetVersion", AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_NULL_CONTEXT_FUNCTION);

#endif // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::initializeEGLFunctionsData
// Description: Initialize the EGL functions data.
// Author:  AMD Developer Tools Team
// Date:        23/2/2006
// ---------------------------------------------------------------------------
void apMonitoredFunctionsManager::initializeEGLFunctionsData()
{
    // OpenGL ES (EGL) is currently supported on Windows:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    _monitoredFunctionsData[ap_eglGetError] = MonitoredFunctionData(L"eglGetError", AP_EGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglGetDisplay] = MonitoredFunctionData(L"eglGetDisplay", AP_EGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglInitialize] = MonitoredFunctionData(L"eglInitialize", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglTerminate] = MonitoredFunctionData(L"eglTerminate", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglQueryString] = MonitoredFunctionData(L"eglQueryString", AP_EGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglGetProcAddress] = MonitoredFunctionData(L"eglGetProcAddress", AP_EGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglGetConfigs] = MonitoredFunctionData(L"eglGetConfigs", AP_EGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglChooseConfig] = MonitoredFunctionData(L"eglChooseConfig", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglGetConfigAttrib] = MonitoredFunctionData(L"eglGetConfigAttrib", AP_EGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglCreateWindowSurface] = MonitoredFunctionData(L"eglCreateWindowSurface", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglCreatePixmapSurface] = MonitoredFunctionData(L"eglCreatePixmapSurface", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglCreatePbufferSurface] = MonitoredFunctionData(L"eglCreatePbufferSurface", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglDestroySurface] = MonitoredFunctionData(L"eglDestroySurface", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglQuerySurface] = MonitoredFunctionData(L"eglQuerySurface", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglSurfaceAttrib] = MonitoredFunctionData(L"eglSurfaceAttrib", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglBindTexImage] = MonitoredFunctionData(L"eglBindTexImage", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglReleaseTexImage] = MonitoredFunctionData(L"eglReleaseTexImage", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglSwapInterval] = MonitoredFunctionData(L"eglSwapInterval", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglCreateContext] = MonitoredFunctionData(L"eglCreateContext", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglDestroyContext] = MonitoredFunctionData(L"eglDestroyContext", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglMakeCurrent] = MonitoredFunctionData(L"eglMakeCurrent", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglGetCurrentContext] = MonitoredFunctionData(L"eglGetCurrentContext", AP_EGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglGetCurrentSurface] = MonitoredFunctionData(L"eglGetCurrentSurface", AP_EGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglGetCurrentDisplay] = MonitoredFunctionData(L"eglGetCurrentDisplay", AP_EGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglQueryContext] = MonitoredFunctionData(L"eglQueryContext", AP_EGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglWaitGL] = MonitoredFunctionData(L"eglWaitGL", AP_EGL_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_eglWaitNative] = MonitoredFunctionData(L"eglWaitNative", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglSwapBuffers] = MonitoredFunctionData(L"eglSwapBuffers", AP_EGL_FUNC, AP_DRAW_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_eglCopyBuffers] = MonitoredFunctionData(L"eglCopyBuffers", AP_EGL_FUNC, AP_NULL_CONTEXT_FUNCTION);

#endif // (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::initializeEGLExtensionsFunctionsData
// Description: Initialize EGL extensions functions data.
// Author:  AMD Developer Tools Team
// Date:        14/5/2006
// ---------------------------------------------------------------------------
void apMonitoredFunctionsManager::initializeEGLExtensionsFunctionsData()
{
    // OpenGL ES (EGL) is currently supported on Windows only:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    _monitoredFunctionsData[ap_eglMakeWindowNV] = MonitoredFunctionData(L"eglMakeWindowNV", AP_EGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_eglDestroyWindowNV] = MonitoredFunctionData(L"eglDestroyWindowNV", AP_EGL_EXTENSION_FUNC, 0);

#endif
}


void apMonitoredFunctionsManager::initializeOpenGL12FunctionsData()
{
    // Get the function type by platform:
    apAPIType functionBaseType = AP_OPENGL_EXTENSION_FUNC;

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    // In Linux and Mac, OpenGL 1.2 is part of the standard and not an extension
    functionBaseType = AP_OPENGL_GENERIC_FUNC;
#endif

    _monitoredFunctionsData[ap_glTexImage3D] = MonitoredFunctionData(L"glTexImage3D", functionBaseType, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawRangeElements] = MonitoredFunctionData(L"glDrawRangeElements", functionBaseType, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glColorTable] = MonitoredFunctionData(L"glColorTable", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glColorTableParameterfv] = MonitoredFunctionData(L"glColorTableParameterfv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glColorTableParameteriv] = MonitoredFunctionData(L"glColorTableParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glCopyColorTable] = MonitoredFunctionData(L"glCopyColorTable", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glGetColorTable] = MonitoredFunctionData(L"glGetColorTable", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetColorTableParameterfv] = MonitoredFunctionData(L"glGetColorTableParameterfv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetColorTableParameteriv] = MonitoredFunctionData(L"glGetColorTableParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glTexSubImage3D] = MonitoredFunctionData(L"glTexSubImage3D", functionBaseType, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTexSubImage3D] = MonitoredFunctionData(L"glCopyTexSubImage3D", functionBaseType, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glColorSubTable] = MonitoredFunctionData(L"glColorSubTable", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glCopyColorSubTable] = MonitoredFunctionData(L"glCopyColorSubTable", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glConvolutionFilter1D] = MonitoredFunctionData(L"glConvolutionFilter1D", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glConvolutionFilter2D] = MonitoredFunctionData(L"glConvolutionFilter2D", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glConvolutionParameterf] = MonitoredFunctionData(L"glConvolutionParameterf", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glConvolutionParameterfv] = MonitoredFunctionData(L"glConvolutionParameterfv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glConvolutionParameteri] = MonitoredFunctionData(L"glConvolutionParameteri", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glConvolutionParameteriv] = MonitoredFunctionData(L"glConvolutionParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glCopyConvolutionFilter1D] = MonitoredFunctionData(L"glCopyConvolutionFilter1D", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glCopyConvolutionFilter2D] = MonitoredFunctionData(L"glCopyConvolutionFilter2D", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glGetConvolutionFilter] = MonitoredFunctionData(L"glGetConvolutionFilter", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetConvolutionParameterfv] = MonitoredFunctionData(L"glGetConvolutionParameterfv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetConvolutionParameteriv] = MonitoredFunctionData(L"glGetConvolutionParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetSeparableFilter] = MonitoredFunctionData(L"glGetSeparableFilter", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glSeparableFilter2D] = MonitoredFunctionData(L"glSeparableFilter2D", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glGetHistogram] = MonitoredFunctionData(L"glGetHistogram", functionBaseType, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetHistogramParameterfv] = MonitoredFunctionData(L"glGetHistogramParameterfv", functionBaseType, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetHistogramParameteriv] = MonitoredFunctionData(L"glGetHistogramParameteriv", functionBaseType, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetMinmax] = MonitoredFunctionData(L"glGetMinmax", functionBaseType, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetMinmaxParameterfv] = MonitoredFunctionData(L"glGetMinmaxParameterfv", functionBaseType, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetMinmaxParameteriv] = MonitoredFunctionData(L"glGetMinmaxParameteriv", functionBaseType, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glHistogram] = MonitoredFunctionData(L"glHistogram", AP_OPENGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_glMinmax] = MonitoredFunctionData(L"glMinmax", AP_OPENGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_glResetHistogram] = MonitoredFunctionData(L"glResetHistogram", AP_OPENGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_glResetMinmax] = MonitoredFunctionData(L"glResetMinmax", AP_OPENGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_glBlendColor] = MonitoredFunctionData(L"glBlendColor", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glBlendEquation] = MonitoredFunctionData(L"glBlendEquation", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
}


void apMonitoredFunctionsManager::initializeOpenGL13FunctionsData()
{
    // Get the function type by platform:
    apAPIType functionBaseType = AP_OPENGL_EXTENSION_FUNC;

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    // In Linux and Mac, OpenGL 1.3is part of the standard and not an extension
    functionBaseType = AP_OPENGL_GENERIC_FUNC;
#endif

    _monitoredFunctionsData[ap_glActiveTexture] = MonitoredFunctionData(L"glActiveTexture", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glClientActiveTexture] = MonitoredFunctionData(L"glClientActiveTexture", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMultiTexCoord1d] = MonitoredFunctionData(L"glMultiTexCoord1d", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1dv] = MonitoredFunctionData(L"glMultiTexCoord1dv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1f] = MonitoredFunctionData(L"glMultiTexCoord1f", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1fv] = MonitoredFunctionData(L"glMultiTexCoord1fv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1i] = MonitoredFunctionData(L"glMultiTexCoord1i", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1iv] = MonitoredFunctionData(L"glMultiTexCoord1iv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1s] = MonitoredFunctionData(L"glMultiTexCoord1s", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1sv] = MonitoredFunctionData(L"glMultiTexCoord1sv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2d] = MonitoredFunctionData(L"glMultiTexCoord2d", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2dv] = MonitoredFunctionData(L"glMultiTexCoord2dv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2f] = MonitoredFunctionData(L"glMultiTexCoord2f", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2fv] = MonitoredFunctionData(L"glMultiTexCoord2fv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2i] = MonitoredFunctionData(L"glMultiTexCoord2i", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2iv] = MonitoredFunctionData(L"glMultiTexCoord2iv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2s] = MonitoredFunctionData(L"glMultiTexCoord2s", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2sv] = MonitoredFunctionData(L"glMultiTexCoord2sv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3d] = MonitoredFunctionData(L"glMultiTexCoord3d", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3dv] = MonitoredFunctionData(L"glMultiTexCoord3dv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3f] = MonitoredFunctionData(L"glMultiTexCoord3f", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3fv] = MonitoredFunctionData(L"glMultiTexCoord3fv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3i] = MonitoredFunctionData(L"glMultiTexCoord3i", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3iv] = MonitoredFunctionData(L"glMultiTexCoord3iv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3s] = MonitoredFunctionData(L"glMultiTexCoord3s", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3sv] = MonitoredFunctionData(L"glMultiTexCoord3sv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4d] = MonitoredFunctionData(L"glMultiTexCoord4d", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4dv] = MonitoredFunctionData(L"glMultiTexCoord4dv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4f] = MonitoredFunctionData(L"glMultiTexCoord4f", functionBaseType | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4fv] = MonitoredFunctionData(L"glMultiTexCoord4fv", functionBaseType, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4i] = MonitoredFunctionData(L"glMultiTexCoord4i", functionBaseType, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4iv] = MonitoredFunctionData(L"glMultiTexCoord4iv", functionBaseType, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4s] = MonitoredFunctionData(L"glMultiTexCoord4s", functionBaseType, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4sv] = MonitoredFunctionData(L"glMultiTexCoord4sv", functionBaseType, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTexImage3D] = MonitoredFunctionData(L"glCompressedTexImage3D", functionBaseType, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTexImage2D] = MonitoredFunctionData(L"glCompressedTexImage2D", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTexImage1D] = MonitoredFunctionData(L"glCompressedTexImage1D", functionBaseType, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTexSubImage3D] = MonitoredFunctionData(L"glCompressedTexSubImage3D", functionBaseType, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTexSubImage2D] = MonitoredFunctionData(L"glCompressedTexSubImage2D", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTexSubImage1D] = MonitoredFunctionData(L"glCompressedTexSubImage1D", functionBaseType, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetCompressedTexImage] = MonitoredFunctionData(L"glGetCompressedTexImage", functionBaseType, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glSampleCoverage] = MonitoredFunctionData(L"glSampleCoverage", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glLoadTransposeMatrixf] = MonitoredFunctionData(L"glLoadTransposeMatrixf", functionBaseType, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glLoadTransposeMatrixd] = MonitoredFunctionData(L"glLoadTransposeMatrixd", functionBaseType, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMultTransposeMatrixf] = MonitoredFunctionData(L"glMultTransposeMatrixf", functionBaseType, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMultTransposeMatrixd] = MonitoredFunctionData(L"glMultTransposeMatrixd", functionBaseType, AP_TEXTURE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
}

void apMonitoredFunctionsManager::initializeOpenGL14FunctionsData()
{
    // Get the function type by platform:
    apAPIType functionBaseType = AP_OPENGL_EXTENSION_FUNC;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    functionBaseType = AP_OPENGL_GENERIC_FUNC;
#endif

    _monitoredFunctionsData[ap_glBlendFuncSeparate] = MonitoredFunctionData(L"glBlendFuncSeparate", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glFogCoordf] = MonitoredFunctionData(L"glFogCoordf", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glFogCoordfv] = MonitoredFunctionData(L"glFogCoordfv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glFogCoordd] = MonitoredFunctionData(L"glFogCoordd", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glFogCoorddv] = MonitoredFunctionData(L"glFogCoorddv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glFogCoordPointer] = MonitoredFunctionData(L"glFogCoordPointer", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glMultiDrawArrays] = MonitoredFunctionData(L"glMultiDrawArrays", functionBaseType, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glMultiDrawElements] = MonitoredFunctionData(L"glMultiDrawElements", functionBaseType, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glPointParameterf] = MonitoredFunctionData(L"glPointParameterf", functionBaseType | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPointParameterfv] = MonitoredFunctionData(L"glPointParameterfv", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPointParameteri] = MonitoredFunctionData(L"glPointParameteri", functionBaseType, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPointParameteriv] = MonitoredFunctionData(L"glPointParameteriv", functionBaseType, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glSecondaryColor3b] = MonitoredFunctionData(L"glSecondaryColor3b", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3bv] = MonitoredFunctionData(L"glSecondaryColor3bv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3d] = MonitoredFunctionData(L"glSecondaryColor3d", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3dv] = MonitoredFunctionData(L"glSecondaryColor3dv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3f] = MonitoredFunctionData(L"glSecondaryColor3f", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3fv] = MonitoredFunctionData(L"glSecondaryColor3fv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3i] = MonitoredFunctionData(L"glSecondaryColor3i", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3iv] = MonitoredFunctionData(L"glSecondaryColor3iv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3s] = MonitoredFunctionData(L"glSecondaryColor3s", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3sv] = MonitoredFunctionData(L"glSecondaryColor3sv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3ub] = MonitoredFunctionData(L"glSecondaryColor3ub", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3ubv] = MonitoredFunctionData(L"glSecondaryColor3ubv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3ui] = MonitoredFunctionData(L"glSecondaryColor3ui", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3uiv] = MonitoredFunctionData(L"glSecondaryColor3uiv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3us] = MonitoredFunctionData(L"glSecondaryColor3us", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColor3usv] = MonitoredFunctionData(L"glSecondaryColor3usv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glSecondaryColorPointer] = MonitoredFunctionData(L"glSecondaryColorPointer", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos2d] = MonitoredFunctionData(L"glWindowPos2d", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos2dv] = MonitoredFunctionData(L"glWindowPos2dv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos2f] = MonitoredFunctionData(L"glWindowPos2f", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos2fv] = MonitoredFunctionData(L"glWindowPos2fv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos2i] = MonitoredFunctionData(L"glWindowPos2i", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos2iv] = MonitoredFunctionData(L"glWindowPos2iv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos2s] = MonitoredFunctionData(L"glWindowPos2s", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos2sv] = MonitoredFunctionData(L"glWindowPos2sv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos3d] = MonitoredFunctionData(L"glWindowPos3d", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos3dv] = MonitoredFunctionData(L"glWindowPos3dv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos3f] = MonitoredFunctionData(L"glWindowPos3f", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos3fv] = MonitoredFunctionData(L"glWindowPos3fv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos3i] = MonitoredFunctionData(L"glWindowPos3i", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos3iv] = MonitoredFunctionData(L"glWindowPos3iv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos3s] = MonitoredFunctionData(L"glWindowPos3s", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
    _monitoredFunctionsData[ap_glWindowPos3sv] = MonitoredFunctionData(L"glWindowPos3sv", functionBaseType, AP_STATE_CHANGE_FUNC | AP_DEPRECATED_FUNC, AP_GL_VERSION_3_0, AP_GL_VERSION_3_1);
}

void apMonitoredFunctionsManager::initializeOpenGL15FunctionsData()
{
    // Get the function type by platform:
    apAPIType functionBaseType = AP_OPENGL_EXTENSION_FUNC;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    functionBaseType = AP_OPENGL_GENERIC_FUNC;
#endif
    _monitoredFunctionsData[ap_glGenQueries] = MonitoredFunctionData(L"glGenQueries", functionBaseType, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glDeleteQueries] = MonitoredFunctionData(L"glDeleteQueries", functionBaseType, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glIsQuery] = MonitoredFunctionData(L"glIsQuery", functionBaseType, AP_GET_FUNC | AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glBeginQuery] = MonitoredFunctionData(L"glBeginQuery", functionBaseType, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glEndQuery] = MonitoredFunctionData(L"glEndQuery", functionBaseType, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryiv] = MonitoredFunctionData(L"glGetQueryiv", functionBaseType, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryObjectiv] = MonitoredFunctionData(L"glGetQueryObjectiv", functionBaseType, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryObjectuiv] = MonitoredFunctionData(L"glGetQueryObjectuiv", functionBaseType, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glBindBuffer] = MonitoredFunctionData(L"glBindBuffer", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glDeleteBuffers] = MonitoredFunctionData(L"glDeleteBuffers", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGenBuffers] = MonitoredFunctionData(L"glGenBuffers", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glIsBuffer] = MonitoredFunctionData(L"glIsBuffer", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glBufferData] = MonitoredFunctionData(L"glBufferData", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glBufferSubData] = MonitoredFunctionData(L"glBufferSubData", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetBufferSubData] = MonitoredFunctionData(L"glGetBufferSubData", functionBaseType, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glMapBuffer] = MonitoredFunctionData(L"glMapBuffer", functionBaseType, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glUnmapBuffer] = MonitoredFunctionData(L"glUnmapBuffer", functionBaseType, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetBufferParameteriv] = MonitoredFunctionData(L"glGetBufferParameteriv", functionBaseType | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetBufferPointerv] = MonitoredFunctionData(L"glGetBufferPointerv", functionBaseType, AP_BUFFER_FUNC | AP_GET_FUNC);
}

void apMonitoredFunctionsManager::initializeOpenGL20FunctionsData()
{
    // Get the function type by platform:
    apAPIType functionBaseType = AP_OPENGL_EXTENSION_FUNC;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    functionBaseType = AP_OPENGL_GENERIC_FUNC;
#endif
    _monitoredFunctionsData[ap_glBlendEquationSeparate] = MonitoredFunctionData(L"glBlendEquationSeparate", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDrawBuffers] = MonitoredFunctionData(L"glDrawBuffers", functionBaseType, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glStencilOpSeparate] = MonitoredFunctionData(L"glStencilOpSeparate", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glStencilFuncSeparate] = MonitoredFunctionData(L"glStencilFuncSeparate", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glStencilMaskSeparate] = MonitoredFunctionData(L"glStencilMaskSeparate", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glAttachShader] = MonitoredFunctionData(L"glAttachShader", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindAttribLocation] = MonitoredFunctionData(L"glBindAttribLocation", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glCompileShader] = MonitoredFunctionData(L"glCompileShader", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glCreateProgram] = MonitoredFunctionData(L"glCreateProgram", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glCreateShader] = MonitoredFunctionData(L"glCreateShader", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDeleteProgram] = MonitoredFunctionData(L"glDeleteProgram", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDeleteShader] = MonitoredFunctionData(L"glDeleteShader", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDetachShader] = MonitoredFunctionData(L"glDetachShader", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDisableVertexAttribArray] = MonitoredFunctionData(L"glDisableVertexAttribArray", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glEnableVertexAttribArray] = MonitoredFunctionData(L"glEnableVertexAttribArray", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetActiveAttrib] = MonitoredFunctionData(L"glGetActiveAttrib", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetActiveUniform] = MonitoredFunctionData(L"glGetActiveUniform", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetAttachedShaders] = MonitoredFunctionData(L"glGetAttachedShaders", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetAttribLocation] = MonitoredFunctionData(L"glGetAttribLocation", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetProgramiv] = MonitoredFunctionData(L"glGetProgramiv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetProgramInfoLog] = MonitoredFunctionData(L"glGetProgramInfoLog", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetShaderiv] = MonitoredFunctionData(L"glGetShaderiv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetShaderInfoLog] = MonitoredFunctionData(L"glGetShaderInfoLog", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetShaderSource] = MonitoredFunctionData(L"glGetShaderSource", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetUniformLocation] = MonitoredFunctionData(L"glGetUniformLocation", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetUniformfv] = MonitoredFunctionData(L"glGetUniformfv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetUniformiv] = MonitoredFunctionData(L"glGetUniformiv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribdv] = MonitoredFunctionData(L"glGetVertexAttribdv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribfv] = MonitoredFunctionData(L"glGetVertexAttribfv", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribiv] = MonitoredFunctionData(L"glGetVertexAttribiv", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribPointerv] = MonitoredFunctionData(L"glGetVertexAttribPointerv", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glIsProgram] = MonitoredFunctionData(L"glIsProgram", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glIsShader] = MonitoredFunctionData(L"glIsShader", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glLinkProgram] = MonitoredFunctionData(L"glLinkProgram", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glShaderSource] = MonitoredFunctionData(L"glShaderSource", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUseProgram] = MonitoredFunctionData(L"glUseProgram", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform1f] = MonitoredFunctionData(L"glUniform1f", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2f] = MonitoredFunctionData(L"glUniform2f", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3f] = MonitoredFunctionData(L"glUniform3f", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4f] = MonitoredFunctionData(L"glUniform4f", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform1i] = MonitoredFunctionData(L"glUniform1i", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2i] = MonitoredFunctionData(L"glUniform2i", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3i] = MonitoredFunctionData(L"glUniform3i", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4i] = MonitoredFunctionData(L"glUniform4i", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform1fv] = MonitoredFunctionData(L"glUniform1fv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2fv] = MonitoredFunctionData(L"glUniform2fv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3fv] = MonitoredFunctionData(L"glUniform3fv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4fv] = MonitoredFunctionData(L"glUniform4fv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform1iv] = MonitoredFunctionData(L"glUniform1iv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2iv] = MonitoredFunctionData(L"glUniform2iv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3iv] = MonitoredFunctionData(L"glUniform3iv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4iv] = MonitoredFunctionData(L"glUniform4iv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix2fv] = MonitoredFunctionData(L"glUniformMatrix2fv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix3fv] = MonitoredFunctionData(L"glUniformMatrix3fv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix4fv] = MonitoredFunctionData(L"glUniformMatrix4fv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glValidateProgram] = MonitoredFunctionData(L"glValidateProgram", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1d] = MonitoredFunctionData(L"glVertexAttrib1d", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1dv] = MonitoredFunctionData(L"glVertexAttrib1dv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1f] = MonitoredFunctionData(L"glVertexAttrib1f", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1fv] = MonitoredFunctionData(L"glVertexAttrib1fv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1s] = MonitoredFunctionData(L"glVertexAttrib1s", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1sv] = MonitoredFunctionData(L"glVertexAttrib1sv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2d] = MonitoredFunctionData(L"glVertexAttrib2d", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2dv] = MonitoredFunctionData(L"glVertexAttrib2dv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2f] = MonitoredFunctionData(L"glVertexAttrib2f", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2fv] = MonitoredFunctionData(L"glVertexAttrib2fv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2s] = MonitoredFunctionData(L"glVertexAttrib2s", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2sv] = MonitoredFunctionData(L"glVertexAttrib2sv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3d] = MonitoredFunctionData(L"glVertexAttrib3d", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3dv] = MonitoredFunctionData(L"glVertexAttrib3dv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3f] = MonitoredFunctionData(L"glVertexAttrib3f", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3fv] = MonitoredFunctionData(L"glVertexAttrib3fv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3s] = MonitoredFunctionData(L"glVertexAttrib3s", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3sv] = MonitoredFunctionData(L"glVertexAttrib3sv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4Nbv] = MonitoredFunctionData(L"glVertexAttrib4Nbv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4Niv] = MonitoredFunctionData(L"glVertexAttrib4Niv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4Nsv] = MonitoredFunctionData(L"glVertexAttrib4Nsv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4Nub] = MonitoredFunctionData(L"glVertexAttrib4Nub", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4Nubv] = MonitoredFunctionData(L"glVertexAttrib4Nubv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4Nuiv] = MonitoredFunctionData(L"glVertexAttrib4Nuiv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4Nusv] = MonitoredFunctionData(L"glVertexAttrib4Nusv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4bv] = MonitoredFunctionData(L"glVertexAttrib4bv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4d] = MonitoredFunctionData(L"glVertexAttrib4d", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4dv] = MonitoredFunctionData(L"glVertexAttrib4dv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4f] = MonitoredFunctionData(L"glVertexAttrib4f", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4fv] = MonitoredFunctionData(L"glVertexAttrib4fv", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4iv] = MonitoredFunctionData(L"glVertexAttrib4iv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4s] = MonitoredFunctionData(L"glVertexAttrib4s", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4sv] = MonitoredFunctionData(L"glVertexAttrib4sv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4ubv] = MonitoredFunctionData(L"glVertexAttrib4ubv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4uiv] = MonitoredFunctionData(L"glVertexAttrib4uiv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4usv] = MonitoredFunctionData(L"glVertexAttrib4usv", functionBaseType, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribPointer] = MonitoredFunctionData(L"glVertexAttribPointer", functionBaseType | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_PROGRAM_SHADER_FUNC);
}

void apMonitoredFunctionsManager::initializeOpenGL21FunctionsData()
{
    _monitoredFunctionsData[ap_glUniformMatrix2x3fv] = MonitoredFunctionData(L"glUniformMatrix2x3fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix3x2fv] = MonitoredFunctionData(L"glUniformMatrix3x2fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix2x4fv] = MonitoredFunctionData(L"glUniformMatrix2x4fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix4x2fv] = MonitoredFunctionData(L"glUniformMatrix4x2fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix3x4fv] = MonitoredFunctionData(L"glUniformMatrix3x4fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix4x3fv] = MonitoredFunctionData(L"glUniformMatrix4x3fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
}



void apMonitoredFunctionsManager::initializeOpenGL30FunctionsData()
{
    // TO_DO: OpenGL3.0 - go through the functions, and check the types:
    _monitoredFunctionsData[ap_glColorMaski] = MonitoredFunctionData(L"glColorMaski", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glGetBooleani_v] = MonitoredFunctionData(L"glGetBooleani_v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetIntegeri_v] = MonitoredFunctionData(L"glGetIntegeri_v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glEnablei] = MonitoredFunctionData(L"glEnablei", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDisablei] = MonitoredFunctionData(L"glDisablei", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glIsEnabledi] = MonitoredFunctionData(L"glIsEnabledi", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glBeginTransformFeedback] = MonitoredFunctionData(L"glBeginTransformFeedback", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC | AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glEndTransformFeedback] = MonitoredFunctionData(L"glEndTransformFeedback", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC | AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glBindBufferRange] = MonitoredFunctionData(L"glBindBufferRange", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glBindBufferBase] = MonitoredFunctionData(L"glBindBufferBase", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glTransformFeedbackVaryings] = MonitoredFunctionData(L"glTransformFeedbackVaryings", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glGetTransformFeedbackVarying] = MonitoredFunctionData(L"glGetTransformFeedbackVarying", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glClampColor] = MonitoredFunctionData(L"glClampColor", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glBeginConditionalRender] = MonitoredFunctionData(L"glBeginConditionalRender", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glEndConditionalRender] = MonitoredFunctionData(L"glEndConditionalRender", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI1i] = MonitoredFunctionData(L"glVertexAttribI1i", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI2i] = MonitoredFunctionData(L"glVertexAttribI2i", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI3i] = MonitoredFunctionData(L"glVertexAttribI3i", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI4i] = MonitoredFunctionData(L"glVertexAttribI4i", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI1ui] = MonitoredFunctionData(L"glVertexAttribI1ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI2ui] = MonitoredFunctionData(L"glVertexAttribI2ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI3ui] = MonitoredFunctionData(L"glVertexAttribI3ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI4ui] = MonitoredFunctionData(L"glVertexAttribI4ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI1iv] = MonitoredFunctionData(L"glVertexAttribI1iv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI2iv] = MonitoredFunctionData(L"glVertexAttribI2iv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI3iv] = MonitoredFunctionData(L"glVertexAttribI3iv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI4iv] = MonitoredFunctionData(L"glVertexAttribI4iv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI1uiv] = MonitoredFunctionData(L"glVertexAttribI1uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI2uiv] = MonitoredFunctionData(L"glVertexAttribI2uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI3uiv] = MonitoredFunctionData(L"glVertexAttribI3uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI4uiv] = MonitoredFunctionData(L"glVertexAttribI4uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI4bv] = MonitoredFunctionData(L"glVertexAttribI4bv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI4sv] = MonitoredFunctionData(L"glVertexAttribI4sv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI4ubv] = MonitoredFunctionData(L"glVertexAttribI4ubv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribI4usv] = MonitoredFunctionData(L"glVertexAttribI4usv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribIPointer] = MonitoredFunctionData(L"glVertexAttribIPointer", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribIiv] = MonitoredFunctionData(L"glGetVertexAttribIiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribIuiv] = MonitoredFunctionData(L"glGetVertexAttribIuiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetUniformuiv] = MonitoredFunctionData(L"glGetUniformuiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glBindFragDataLocation] = MonitoredFunctionData(L"glBindFragDataLocation", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetFragDataLocation] = MonitoredFunctionData(L"glGetFragDataLocation", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glUniform1ui] = MonitoredFunctionData(L"glUniform1ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2ui] = MonitoredFunctionData(L"glUniform2ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3ui] = MonitoredFunctionData(L"glUniform3ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4ui] = MonitoredFunctionData(L"glUniform4ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform1uiv] = MonitoredFunctionData(L"glUniform1uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2uiv] = MonitoredFunctionData(L"glUniform2uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3uiv] = MonitoredFunctionData(L"glUniform3uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4uiv] = MonitoredFunctionData(L"glUniform4uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glTexParameterIiv] = MonitoredFunctionData(L"glTexParameterIiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexParameterIuiv] = MonitoredFunctionData(L"glTexParameterIuiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexParameterIiv] = MonitoredFunctionData(L"glGetTexParameterIiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexParameterIuiv] = MonitoredFunctionData(L"glGetTexParameterIuiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glClearBufferiv] = MonitoredFunctionData(L"glClearBufferiv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glClearBufferuiv] = MonitoredFunctionData(L"glClearBufferuiv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glClearBufferfv] = MonitoredFunctionData(L"glClearBufferfv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glClearBufferfi] = MonitoredFunctionData(L"glClearBufferfi", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glGetStringi] = MonitoredFunctionData(L"glGetStringi", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);

}


void apMonitoredFunctionsManager::initializeOpenGL31FunctionsData()
{
    _monitoredFunctionsData[ap_glDrawArraysInstanced] = MonitoredFunctionData(L"glDrawArraysInstanced", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glDrawElementsInstanced] = MonitoredFunctionData(L"glDrawElementsInstanced", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glTexBuffer] = MonitoredFunctionData(L"glTexBuffer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glPrimitiveRestartIndex] = MonitoredFunctionData(L"glPrimitiveRestartIndex", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
}

void apMonitoredFunctionsManager::initializeOpenGL32FunctionsData()
{
    // OpenGL3.1 - go through the functions, and check the types:
    _monitoredFunctionsData[ap_glGetInteger64i_v] = MonitoredFunctionData(L"glGetInteger64i_v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetBufferParameteri64v] = MonitoredFunctionData(L"glGetBufferParameteri64v", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTexture] = MonitoredFunctionData(L"glFramebufferTexture", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTextureFace] = MonitoredFunctionData(L"glFramebufferTextureFace", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
}

void apMonitoredFunctionsManager::initializeOpenGL33FunctionsData()
{
    _monitoredFunctionsData[ap_glBindFragDataLocationIndexed] = MonitoredFunctionData(L"glBindFragDataLocationIndexed", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetFragDataIndex] = MonitoredFunctionData(L"glGetFragDataIndex", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGenSamplers] = MonitoredFunctionData(L"glGenSamplers", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDeleteSamplers] = MonitoredFunctionData(L"glDeleteSamplers", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glIsSampler] = MonitoredFunctionData(L"glIsSampler", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glBindSampler] = MonitoredFunctionData(L"glBindSampler", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glSamplerParameteri] = MonitoredFunctionData(L"glSamplerParameteri", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glSamplerParameteriv] = MonitoredFunctionData(L"glSamplerParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glSamplerParameterf] = MonitoredFunctionData(L"glSamplerParameterf", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glSamplerParameterfv] = MonitoredFunctionData(L"glSamplerParameterfv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glSamplerParameterIiv] = MonitoredFunctionData(L"glSamplerParameterIiv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glSamplerParameterIuiv] = MonitoredFunctionData(L"glSamplerParameterIuiv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetSamplerParameteriv] = MonitoredFunctionData(L"glGetSamplerParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetSamplerParameterIiv] = MonitoredFunctionData(L"glGetSamplerParameterIiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetSamplerParameterfv] = MonitoredFunctionData(L"glGetSamplerParameterfv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetSamplerParameterIuiv] = MonitoredFunctionData(L"glGetSamplerParameterIuiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glQueryCounter] = MonitoredFunctionData(L"glQueryCounter", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryObjecti64v] = MonitoredFunctionData(L"glGetQueryObjecti64v", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryObjectui64v] = MonitoredFunctionData(L"glGetQueryObjectui64v", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribDivisor] = MonitoredFunctionData(L"glVertexAttribDivisor", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribP1ui] = MonitoredFunctionData(L"glVertexAttribP1ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribP1uiv] = MonitoredFunctionData(L"glVertexAttribP1uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribP2ui] = MonitoredFunctionData(L"glVertexAttribP2ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribP2uiv] = MonitoredFunctionData(L"glVertexAttribP2uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribP3ui] = MonitoredFunctionData(L"glVertexAttribP3ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribP3uiv] = MonitoredFunctionData(L"glVertexAttribP3uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribP4ui] = MonitoredFunctionData(L"glVertexAttribP4ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribP4uiv] = MonitoredFunctionData(L"glVertexAttribP4uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexP2ui] = MonitoredFunctionData(L"glVertexP2ui", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glVertexP2uiv] = MonitoredFunctionData(L"glVertexP2uiv", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glVertexP3ui] = MonitoredFunctionData(L"glVertexP3ui", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glVertexP3uiv] = MonitoredFunctionData(L"glVertexP3uiv", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glVertexP4ui] = MonitoredFunctionData(L"glVertexP4ui", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glVertexP4uiv] = MonitoredFunctionData(L"glVertexP4uiv", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glTexCoordP1ui] = MonitoredFunctionData(L"glTexCoordP1ui", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexCoordP1uiv] = MonitoredFunctionData(L"glTexCoordP1uiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexCoordP2ui] = MonitoredFunctionData(L"glTexCoordP2ui", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexCoordP2uiv] = MonitoredFunctionData(L"glTexCoordP2uiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexCoordP3ui] = MonitoredFunctionData(L"glTexCoordP3ui", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexCoordP3uiv] = MonitoredFunctionData(L"glTexCoordP3uiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexCoordP4ui] = MonitoredFunctionData(L"glTexCoordP4ui", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexCoordP4uiv] = MonitoredFunctionData(L"glTexCoordP4uiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoordP1ui] = MonitoredFunctionData(L"glMultiTexCoordP1ui", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoordP1uiv] = MonitoredFunctionData(L"glMultiTexCoordP1uiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoordP2ui] = MonitoredFunctionData(L"glMultiTexCoordP2ui", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoordP2uiv] = MonitoredFunctionData(L"glMultiTexCoordP2uiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoordP3ui] = MonitoredFunctionData(L"glMultiTexCoordP3ui", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoordP3uiv] = MonitoredFunctionData(L"glMultiTexCoordP3uiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoordP4ui] = MonitoredFunctionData(L"glMultiTexCoordP4ui", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoordP4uiv] = MonitoredFunctionData(L"glMultiTexCoordP4uiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glNormalP3ui] = MonitoredFunctionData(L"glNormalP3ui", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glNormalP3uiv] = MonitoredFunctionData(L"glNormalP3uiv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glColorP3ui] = MonitoredFunctionData(L"glColorP3ui", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glColorP3uiv] = MonitoredFunctionData(L"glColorP3uiv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glColorP4ui] = MonitoredFunctionData(L"glColorP4ui", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glColorP4uiv] = MonitoredFunctionData(L"glColorP4uiv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glSecondaryColorP3ui] = MonitoredFunctionData(L"glSecondaryColorP3ui", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glSecondaryColorP3uiv] = MonitoredFunctionData(L"glSecondaryColorP3uiv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
}

void apMonitoredFunctionsManager::initializeOpenGL40FunctionsData()
{
    _monitoredFunctionsData[ap_glMinSampleShading] = MonitoredFunctionData(L"glMinSampleShading", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glBlendEquationi] = MonitoredFunctionData(L"glBlendEquationi", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glBlendEquationSeparatei] = MonitoredFunctionData(L"glBlendEquationSeparatei", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glBlendFunci] = MonitoredFunctionData(L"glBlendFunci", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glBlendFuncSeparatei] = MonitoredFunctionData(L"glBlendFuncSeparatei", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDrawArraysIndirect] = MonitoredFunctionData(L"glDrawArraysIndirect", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glDrawElementsIndirect] = MonitoredFunctionData(L"glDrawElementsIndirect", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glUniform1d] = MonitoredFunctionData(L"glUniform1d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2d] = MonitoredFunctionData(L"glUniform2d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3d] = MonitoredFunctionData(L"glUniform3d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4d] = MonitoredFunctionData(L"glUniform4d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform1dv] = MonitoredFunctionData(L"glUniform1dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2dv] = MonitoredFunctionData(L"glUniform2dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3dv] = MonitoredFunctionData(L"glUniform3dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4dv] = MonitoredFunctionData(L"glUniform4dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix2dv] = MonitoredFunctionData(L"glUniformMatrix2dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix3dv] = MonitoredFunctionData(L"glUniformMatrix3dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix4dv] = MonitoredFunctionData(L"glUniformMatrix4dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix2x3dv] = MonitoredFunctionData(L"glUniformMatrix2x3dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix2x4dv] = MonitoredFunctionData(L"glUniformMatrix2x4dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix3x2dv] = MonitoredFunctionData(L"glUniformMatrix3x2dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix3x4dv] = MonitoredFunctionData(L"glUniformMatrix3x4dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix4x2dv] = MonitoredFunctionData(L"glUniformMatrix4x2dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix4x3dv] = MonitoredFunctionData(L"glUniformMatrix4x3dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetUniformdv] = MonitoredFunctionData(L"glGetUniformdv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetSubroutineUniformLocation] = MonitoredFunctionData(L"glGetSubroutineUniformLocation", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetSubroutineIndex] = MonitoredFunctionData(L"glGetSubroutineIndex", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetActiveSubroutineUniformiv] = MonitoredFunctionData(L"glGetActiveSubroutineUniformiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetActiveSubroutineUniformName] = MonitoredFunctionData(L"glGetActiveSubroutineUniformName", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetActiveSubroutineName] = MonitoredFunctionData(L"glGetActiveSubroutineName", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformSubroutinesuiv] = MonitoredFunctionData(L"glUniformSubroutinesuiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetUniformSubroutineuiv] = MonitoredFunctionData(L"glGetUniformSubroutineuiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetProgramStageiv] = MonitoredFunctionData(L"glGetProgramStageiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glPatchParameteri] = MonitoredFunctionData(L"glPatchParameteri", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glPatchParameterfv] = MonitoredFunctionData(L"glPatchParameterfv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindTransformFeedback] = MonitoredFunctionData(L"glBindTransformFeedback", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glDeleteTransformFeedbacks] = MonitoredFunctionData(L"glDeleteTransformFeedbacks", AP_OPENGL_EXTENSION_FUNC, AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glGenTransformFeedbacks] = MonitoredFunctionData(L"glGenTransformFeedbacks", AP_OPENGL_EXTENSION_FUNC, AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glIsTransformFeedback] = MonitoredFunctionData(L"glIsTransformFeedback", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glPauseTransformFeedback] = MonitoredFunctionData(L"glPauseTransformFeedback", AP_OPENGL_EXTENSION_FUNC, AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glResumeTransformFeedback] = MonitoredFunctionData(L"glResumeTransformFeedback", AP_OPENGL_EXTENSION_FUNC, AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glDrawTransformFeedback] = MonitoredFunctionData(L"glDrawTransformFeedback", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC | AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glDrawTransformFeedbackStream] = MonitoredFunctionData(L"glDrawTransformFeedbackStream", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC | AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glBeginQueryIndexed] = MonitoredFunctionData(L"glBeginQueryIndexed", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glEndQueryIndexed] = MonitoredFunctionData(L"glEndQueryIndexed", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryIndexediv] = MonitoredFunctionData(L"glGetQueryIndexediv", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
}

void apMonitoredFunctionsManager::initializeOpenGL41FunctionsData()
{
    _monitoredFunctionsData[ap_glReleaseShaderCompiler] = MonitoredFunctionData(L"glReleaseShaderCompiler", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glShaderBinary] = MonitoredFunctionData(L"glShaderBinary", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetShaderPrecisionFormat] = MonitoredFunctionData(L"glGetShaderPrecisionFormat", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDepthRangef] = MonitoredFunctionData(L"glDepthRangef", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glClearDepthf] = MonitoredFunctionData(L"glClearDepthf", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_GENERIC_FUNC | AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glGetProgramBinary] = MonitoredFunctionData(L"glGetProgramBinary", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramBinary] = MonitoredFunctionData(L"glProgramBinary", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramParameteri] = MonitoredFunctionData(L"glProgramParameteri", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUseProgramStages] = MonitoredFunctionData(L"glUseProgramStages", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glActiveShaderProgram] = MonitoredFunctionData(L"glActiveShaderProgram", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glCreateShaderProgramv] = MonitoredFunctionData(L"glCreateShaderProgramv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindProgramPipeline] = MonitoredFunctionData(L"glBindProgramPipeline", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDeleteProgramPipelines] = MonitoredFunctionData(L"glDeleteProgramPipelines", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGenProgramPipelines] = MonitoredFunctionData(L"glGenProgramPipelines", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glIsProgramPipeline] = MonitoredFunctionData(L"glIsProgramPipeline", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetProgramPipelineiv] = MonitoredFunctionData(L"glGetProgramPipelineiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1i] = MonitoredFunctionData(L"glProgramUniform1i", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1iv] = MonitoredFunctionData(L"glProgramUniform1iv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1f] = MonitoredFunctionData(L"glProgramUniform1f", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1fv] = MonitoredFunctionData(L"glProgramUniform1fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1d] = MonitoredFunctionData(L"glProgramUniform1d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1dv] = MonitoredFunctionData(L"glProgramUniform1dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1ui] = MonitoredFunctionData(L"glProgramUniform1ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1uiv] = MonitoredFunctionData(L"glProgramUniform1uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2i] = MonitoredFunctionData(L"glProgramUniform2i", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2iv] = MonitoredFunctionData(L"glProgramUniform2iv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2f] = MonitoredFunctionData(L"glProgramUniform2f", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2fv] = MonitoredFunctionData(L"glProgramUniform2fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2d] = MonitoredFunctionData(L"glProgramUniform2d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2dv] = MonitoredFunctionData(L"glProgramUniform2dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2ui] = MonitoredFunctionData(L"glProgramUniform2ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2uiv] = MonitoredFunctionData(L"glProgramUniform2uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3i] = MonitoredFunctionData(L"glProgramUniform3i", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3iv] = MonitoredFunctionData(L"glProgramUniform3iv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3f] = MonitoredFunctionData(L"glProgramUniform3f", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3fv] = MonitoredFunctionData(L"glProgramUniform3fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3d] = MonitoredFunctionData(L"glProgramUniform3d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3dv] = MonitoredFunctionData(L"glProgramUniform3dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3ui] = MonitoredFunctionData(L"glProgramUniform3ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3uiv] = MonitoredFunctionData(L"glProgramUniform3uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4i] = MonitoredFunctionData(L"glProgramUniform4i", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4iv] = MonitoredFunctionData(L"glProgramUniform4iv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4f] = MonitoredFunctionData(L"glProgramUniform4f", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4fv] = MonitoredFunctionData(L"glProgramUniform4fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4d] = MonitoredFunctionData(L"glProgramUniform4d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4dv] = MonitoredFunctionData(L"glProgramUniform4dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4ui] = MonitoredFunctionData(L"glProgramUniform4ui", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4uiv] = MonitoredFunctionData(L"glProgramUniform4uiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix2fv] = MonitoredFunctionData(L"glProgramUniformMatrix2fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix3fv] = MonitoredFunctionData(L"glProgramUniformMatrix3fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix4fv] = MonitoredFunctionData(L"glProgramUniformMatrix4fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix2dv] = MonitoredFunctionData(L"glProgramUniformMatrix2dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix3dv] = MonitoredFunctionData(L"glProgramUniformMatrix3dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix4dv] = MonitoredFunctionData(L"glProgramUniformMatrix4dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix2x3fv] = MonitoredFunctionData(L"glProgramUniformMatrix2x3fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix3x2fv] = MonitoredFunctionData(L"glProgramUniformMatrix3x2fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix2x4fv] = MonitoredFunctionData(L"glProgramUniformMatrix2x4fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix4x2fv] = MonitoredFunctionData(L"glProgramUniformMatrix4x2fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix3x4fv] = MonitoredFunctionData(L"glProgramUniformMatrix3x4fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix4x3fv] = MonitoredFunctionData(L"glProgramUniformMatrix4x3fv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix2x3dv] = MonitoredFunctionData(L"glProgramUniformMatrix2x3dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix3x2dv] = MonitoredFunctionData(L"glProgramUniformMatrix3x2dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix2x4dv] = MonitoredFunctionData(L"glProgramUniformMatrix2x4dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix4x2dv] = MonitoredFunctionData(L"glProgramUniformMatrix4x2dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix3x4dv] = MonitoredFunctionData(L"glProgramUniformMatrix3x4dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix4x3dv] = MonitoredFunctionData(L"glProgramUniformMatrix4x3dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glValidateProgramPipeline] = MonitoredFunctionData(L"glValidateProgramPipeline", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetProgramPipelineInfoLog] = MonitoredFunctionData(L"glGetProgramPipelineInfoLog", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribL1d] = MonitoredFunctionData(L"glVertexAttribL1d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribL2d] = MonitoredFunctionData(L"glVertexAttribL2d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribL3d] = MonitoredFunctionData(L"glVertexAttribL3d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribL4d] = MonitoredFunctionData(L"glVertexAttribL4d", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribL1dv] = MonitoredFunctionData(L"glVertexAttribL1dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribL2dv] = MonitoredFunctionData(L"glVertexAttribL2dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribL3dv] = MonitoredFunctionData(L"glVertexAttribL3dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribL4dv] = MonitoredFunctionData(L"glVertexAttribL4dv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribLPointer] = MonitoredFunctionData(L"glVertexAttribLPointer", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribLdv] = MonitoredFunctionData(L"glGetVertexAttribLdv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glViewportArrayv] = MonitoredFunctionData(L"glViewportArrayv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glViewportIndexedf] = MonitoredFunctionData(L"glViewportIndexedf", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glViewportIndexedfv] = MonitoredFunctionData(L"glViewportIndexedfv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glScissorArrayv] = MonitoredFunctionData(L"glScissorArrayv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glScissorIndexed] = MonitoredFunctionData(L"glScissorIndexed", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glScissorIndexedv] = MonitoredFunctionData(L"glScissorIndexedv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDepthRangeArrayv] = MonitoredFunctionData(L"glDepthRangeArrayv", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDepthRangeIndexed] = MonitoredFunctionData(L"glDepthRangeIndexed", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glGetFloati_v] = MonitoredFunctionData(L"glGetFloati_v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetDoublei_v] = MonitoredFunctionData(L"glGetDoublei_v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
}

void apMonitoredFunctionsManager::initializeOpenGL42FunctionsData()
{
    _monitoredFunctionsData[ap_glDrawArraysInstancedBaseInstance] = MonitoredFunctionData(L"glDrawArraysInstancedBaseInstance", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glDrawElementsInstancedBaseInstance] = MonitoredFunctionData(L"glDrawElementsInstancedBaseInstance", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glDrawElementsInstancedBaseVertexBaseInstance] = MonitoredFunctionData(L"glDrawElementsInstancedBaseVertexBaseInstance", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glGetInternalformativ] = MonitoredFunctionData(L"glGetInternalformativ", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetActiveAtomicCounterBufferiv] = MonitoredFunctionData(L"glGetActiveAtomicCounterBufferiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindImageTexture] = MonitoredFunctionData(L"glBindImageTexture", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMemoryBarrier] = MonitoredFunctionData(L"glMemoryBarrier", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glTexStorage1D] = MonitoredFunctionData(L"glTexStorage1D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexStorage2D] = MonitoredFunctionData(L"glTexStorage2D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexStorage3D] = MonitoredFunctionData(L"glTexStorage3D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glDrawTransformFeedbackInstanced] = MonitoredFunctionData(L"glDrawTransformFeedbackInstanced", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC | AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glDrawTransformFeedbackStreamInstanced] = MonitoredFunctionData(L"glDrawTransformFeedbackStreamInstanced", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC | AP_FEEDBACK_FUNC);
}

void apMonitoredFunctionsManager::initializeOpenGL43FunctionsData()
{
    _monitoredFunctionsData[ap_glClearBufferData] = MonitoredFunctionData(L"glClearBufferData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glClearBufferSubData] = MonitoredFunctionData(L"glClearBufferSubData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glDispatchCompute] = MonitoredFunctionData(L"glDispatchCompute", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDispatchComputeIndirect] = MonitoredFunctionData(L"glDispatchComputeIndirect", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glCopyImageSubData] = MonitoredFunctionData(L"glCopyImageSubData", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glFramebufferParameteri] = MonitoredFunctionData(L"glFramebufferParameteri", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetFramebufferParameteriv] = MonitoredFunctionData(L"glGetFramebufferParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetInternalformati64v] = MonitoredFunctionData(L"glGetInternalformati64v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glInvalidateTexSubImage] = MonitoredFunctionData(L"glInvalidateTexSubImage", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glInvalidateTexImage] = MonitoredFunctionData(L"glInvalidateTexImage", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glInvalidateBufferSubData] = MonitoredFunctionData(L"glInvalidateBufferSubData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glInvalidateBufferData] = MonitoredFunctionData(L"glInvalidateBufferData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glInvalidateFramebuffer] = MonitoredFunctionData(L"glInvalidateFramebuffer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glInvalidateSubFramebuffer] = MonitoredFunctionData(L"glInvalidateSubFramebuffer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glMultiDrawArraysIndirect] = MonitoredFunctionData(L"glMultiDrawArraysIndirect", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glMultiDrawElementsIndirect] = MonitoredFunctionData(L"glMultiDrawElementsIndirect", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glGetProgramInterfaceiv] = MonitoredFunctionData(L"glGetProgramInterfaceiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetProgramResourceIndex] = MonitoredFunctionData(L"glGetProgramResourceIndex", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetProgramResourceName] = MonitoredFunctionData(L"glGetProgramResourceName", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetProgramResourceiv] = MonitoredFunctionData(L"glGetProgramResourceiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetProgramResourceLocation] = MonitoredFunctionData(L"glGetProgramResourceLocation", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetProgramResourceLocationIndex] = MonitoredFunctionData(L"glGetProgramResourceLocationIndex", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glShaderStorageBlockBinding] = MonitoredFunctionData(L"glShaderStorageBlockBinding", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glTexBufferRange] = MonitoredFunctionData(L"glTexBufferRange", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glTexStorage2DMultisample] = MonitoredFunctionData(L"glTexStorage2DMultisample", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexStorage3DMultisample] = MonitoredFunctionData(L"glTexStorage3DMultisample", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureView] = MonitoredFunctionData(L"glTextureView", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glBindVertexBuffer] = MonitoredFunctionData(L"glBindVertexBuffer", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribFormat] = MonitoredFunctionData(L"glVertexAttribFormat", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribIFormat] = MonitoredFunctionData(L"glVertexAttribIFormat", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribLFormat] = MonitoredFunctionData(L"glVertexAttribLFormat", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribBinding] = MonitoredFunctionData(L"glVertexAttribBinding", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexBindingDivisor] = MonitoredFunctionData(L"glVertexBindingDivisor", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDebugMessageControl] = MonitoredFunctionData(L"glDebugMessageControl", AP_OPENGL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glDebugMessageInsert] = MonitoredFunctionData(L"glDebugMessageInsert", AP_OPENGL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glDebugMessageCallback] = MonitoredFunctionData(L"glDebugMessageCallback", AP_OPENGL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glGetDebugMessageLog] = MonitoredFunctionData(L"glGetDebugMessageLog", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glPushDebugGroup] = MonitoredFunctionData(L"glPushDebugGroup", AP_OPENGL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glPopDebugGroup] = MonitoredFunctionData(L"glPopDebugGroup", AP_OPENGL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glObjectLabel] = MonitoredFunctionData(L"glObjectLabel", AP_OPENGL_EXTENSION_FUNC, AP_NAME_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glGetObjectLabel] = MonitoredFunctionData(L"glGetObjectLabel", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_NAME_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glObjectPtrLabel] = MonitoredFunctionData(L"glObjectPtrLabel", AP_OPENGL_EXTENSION_FUNC, AP_NAME_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glGetObjectPtrLabel] = MonitoredFunctionData(L"glGetObjectPtrLabel", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_NAME_FUNC | AP_DEBUG_FUNC);
}

void apMonitoredFunctionsManager::initializeOpenGL44FunctionsData()
{
    _monitoredFunctionsData[ap_glBufferStorage] = MonitoredFunctionData(L"glBufferStorage", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glClearTexImage] = MonitoredFunctionData(L"glClearTexImage", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glClearTexSubImage] = MonitoredFunctionData(L"glClearTexSubImage", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glBindBuffersBase] = MonitoredFunctionData(L"glBindBuffersBase", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glBindBuffersRange] = MonitoredFunctionData(L"glBindBuffersRange", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glBindTextures] = MonitoredFunctionData(L"glBindTextures", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glBindSamplers] = MonitoredFunctionData(L"glBindSamplers", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glBindImageTextures] = MonitoredFunctionData(L"glBindImageTextures", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glBindVertexBuffers] = MonitoredFunctionData(L"glBindVertexBuffers", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
}

void apMonitoredFunctionsManager::initializeOpenGL45FunctionsData()
{
    _monitoredFunctionsData[ap_glClipControl] = MonitoredFunctionData(L"glClipControl", AP_OPENGL_EXTENSION_FUNC, AP_RASTER_FUNC);
    _monitoredFunctionsData[ap_glCreateTransformFeedbacks] = MonitoredFunctionData(L"glCreateTransformFeedbacks", AP_OPENGL_EXTENSION_FUNC, AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glTransformFeedbackBufferBase] = MonitoredFunctionData(L"glTransformFeedbackBufferBase", AP_OPENGL_EXTENSION_FUNC, AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glTransformFeedbackBufferRange] = MonitoredFunctionData(L"glTransformFeedbackBufferRange", AP_OPENGL_EXTENSION_FUNC, AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glGetTransformFeedbackiv] = MonitoredFunctionData(L"glGetTransformFeedbackiv", AP_OPENGL_EXTENSION_FUNC, AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glGetTransformFeedbacki_v] = MonitoredFunctionData(L"glGetTransformFeedbacki_v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glGetTransformFeedbacki64_v] = MonitoredFunctionData(L"glGetTransformFeedbacki64_v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_FEEDBACK_FUNC);
    _monitoredFunctionsData[ap_glCreateBuffers] = MonitoredFunctionData(L"glCreateBuffers", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedBufferStorage] = MonitoredFunctionData(L"glNamedBufferStorage", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedBufferData] = MonitoredFunctionData(L"glNamedBufferData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedBufferSubData] = MonitoredFunctionData(L"glNamedBufferSubData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glCopyNamedBufferSubData] = MonitoredFunctionData(L"glCopyNamedBufferSubData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glClearNamedBufferData] = MonitoredFunctionData(L"glClearNamedBufferData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glClearNamedBufferSubData] = MonitoredFunctionData(L"glClearNamedBufferSubData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glMapNamedBuffer] = MonitoredFunctionData(L"glMapNamedBuffer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glMapNamedBufferRange] = MonitoredFunctionData(L"glMapNamedBufferRange", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glUnmapNamedBuffer] = MonitoredFunctionData(L"glUnmapNamedBuffer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFlushMappedNamedBufferRange] = MonitoredFunctionData(L"glFlushMappedNamedBufferRange", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetNamedBufferParameteriv] = MonitoredFunctionData(L"glGetNamedBufferParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetNamedBufferParameteri64v] = MonitoredFunctionData(L"glGetNamedBufferParameteri64v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetNamedBufferPointerv] = MonitoredFunctionData(L"glGetNamedBufferPointerv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetNamedBufferSubData] = MonitoredFunctionData(L"glGetNamedBufferSubData", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glCreateFramebuffers] = MonitoredFunctionData(L"glCreateFramebuffers", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferRenderbuffer] = MonitoredFunctionData(L"glNamedFramebufferRenderbuffer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferParameteri] = MonitoredFunctionData(L"glNamedFramebufferParameteri", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferTexture] = MonitoredFunctionData(L"glNamedFramebufferTexture", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferTextureLayer] = MonitoredFunctionData(L"glNamedFramebufferTextureLayer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferDrawBuffer] = MonitoredFunctionData(L"glNamedFramebufferDrawBuffer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferDrawBuffers] = MonitoredFunctionData(L"glNamedFramebufferDrawBuffers", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferReadBuffer] = MonitoredFunctionData(L"glNamedFramebufferReadBuffer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glInvalidateNamedFramebufferData] = MonitoredFunctionData(L"glInvalidateNamedFramebufferData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glInvalidateNamedFramebufferSubData] = MonitoredFunctionData(L"glInvalidateNamedFramebufferSubData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glClearNamedFramebufferiv] = MonitoredFunctionData(L"glClearNamedFramebufferiv", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glClearNamedFramebufferuiv] = MonitoredFunctionData(L"glClearNamedFramebufferuiv", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glClearNamedFramebufferfv] = MonitoredFunctionData(L"glClearNamedFramebufferfv", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glClearNamedFramebufferfi] = MonitoredFunctionData(L"glClearNamedFramebufferfi", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glBlitNamedFramebuffer] = MonitoredFunctionData(L"glBlitNamedFramebuffer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glCheckNamedFramebufferStatus] = MonitoredFunctionData(L"glCheckNamedFramebufferStatus", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetNamedFramebufferParameteriv] = MonitoredFunctionData(L"glGetNamedFramebufferParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetNamedFramebufferAttachmentParameteriv] = MonitoredFunctionData(L"glGetNamedFramebufferAttachmentParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glCreateRenderbuffers] = MonitoredFunctionData(L"glCreateRenderbuffers", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedRenderbufferStorage] = MonitoredFunctionData(L"glNamedRenderbufferStorage", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedRenderbufferStorageMultisample] = MonitoredFunctionData(L"glNamedRenderbufferStorageMultisample", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetNamedRenderbufferParameteriv] = MonitoredFunctionData(L"glGetNamedRenderbufferParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glCreateTextures] = MonitoredFunctionData(L"glCreateTextures", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureBuffer] = MonitoredFunctionData(L"glTextureBuffer", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureBufferRange] = MonitoredFunctionData(L"glTextureBufferRange", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureStorage1D] = MonitoredFunctionData(L"glTextureStorage1D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureStorage2D] = MonitoredFunctionData(L"glTextureStorage2D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureStorage3D] = MonitoredFunctionData(L"glTextureStorage3D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureStorage2DMultisample] = MonitoredFunctionData(L"glTextureStorage2DMultisample", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureStorage3DMultisample] = MonitoredFunctionData(L"glTextureStorage3DMultisample", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureSubImage1D] = MonitoredFunctionData(L"glTextureSubImage1D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureSubImage2D] = MonitoredFunctionData(L"glTextureSubImage2D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureSubImage3D] = MonitoredFunctionData(L"glTextureSubImage3D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTextureSubImage1D] = MonitoredFunctionData(L"glCompressedTextureSubImage1D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTextureSubImage2D] = MonitoredFunctionData(L"glCompressedTextureSubImage2D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTextureSubImage3D] = MonitoredFunctionData(L"glCompressedTextureSubImage3D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTextureSubImage1D] = MonitoredFunctionData(L"glCopyTextureSubImage1D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTextureSubImage2D] = MonitoredFunctionData(L"glCopyTextureSubImage2D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTextureSubImage3D] = MonitoredFunctionData(L"glCopyTextureSubImage3D", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureParameterf] = MonitoredFunctionData(L"glTextureParameterf", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureParameterfv] = MonitoredFunctionData(L"glTextureParameterfv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureParameteri] = MonitoredFunctionData(L"glTextureParameteri", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureParameterIiv] = MonitoredFunctionData(L"glTextureParameterIiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureParameterIuiv] = MonitoredFunctionData(L"glTextureParameterIuiv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureParameteriv] = MonitoredFunctionData(L"glTextureParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGenerateTextureMipmap] = MonitoredFunctionData(L"glGenerateTextureMipmap", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glBindTextureUnit] = MonitoredFunctionData(L"glBindTextureUnit", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureImage] = MonitoredFunctionData(L"glGetTextureImage", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetCompressedTextureImage] = MonitoredFunctionData(L"glGetCompressedTextureImage", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureLevelParameterfv] = MonitoredFunctionData(L"glGetTextureLevelParameterfv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureLevelParameteriv] = MonitoredFunctionData(L"glGetTextureLevelParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureParameterfv] = MonitoredFunctionData(L"glGetTextureParameterfv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureParameterIiv] = MonitoredFunctionData(L"glGetTextureParameterIiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureParameterIuiv] = MonitoredFunctionData(L"glGetTextureParameterIuiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureParameteriv] = MonitoredFunctionData(L"glGetTextureParameteriv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCreateVertexArrays] = MonitoredFunctionData(L"glCreateVertexArrays", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glDisableVertexArrayAttrib] = MonitoredFunctionData(L"glDisableVertexArrayAttrib", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glEnableVertexArrayAttrib] = MonitoredFunctionData(L"glEnableVertexArrayAttrib", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glVertexArrayElementBuffer] = MonitoredFunctionData(L"glVertexArrayElementBuffer", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glVertexArrayVertexBuffer] = MonitoredFunctionData(L"glVertexArrayVertexBuffer", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glVertexArrayVertexBuffers] = MonitoredFunctionData(L"glVertexArrayVertexBuffers", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glVertexArrayAttribBinding] = MonitoredFunctionData(L"glVertexArrayAttribBinding", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glVertexArrayAttribFormat] = MonitoredFunctionData(L"glVertexArrayAttribFormat", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glVertexArrayAttribIFormat] = MonitoredFunctionData(L"glVertexArrayAttribIFormat", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glVertexArrayAttribLFormat] = MonitoredFunctionData(L"glVertexArrayAttribLFormat", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glVertexArrayBindingDivisor] = MonitoredFunctionData(L"glVertexArrayBindingDivisor", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glGetVertexArrayiv] = MonitoredFunctionData(L"glGetVertexArrayiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glGetVertexArrayIndexediv] = MonitoredFunctionData(L"glGetVertexArrayIndexediv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glGetVertexArrayIndexed64iv] = MonitoredFunctionData(L"glGetVertexArrayIndexed64iv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glCreateSamplers] = MonitoredFunctionData(L"glCreateSamplers", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCreateProgramPipelines] = MonitoredFunctionData(L"glCreateProgramPipelines", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glCreateQueries] = MonitoredFunctionData(L"glCreateQueries", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryBufferObjecti64v] = MonitoredFunctionData(L"glGetQueryBufferObjecti64v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryBufferObjectiv] = MonitoredFunctionData(L"glGetQueryBufferObjectiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryBufferObjectui64v] = MonitoredFunctionData(L"glGetQueryBufferObjectui64v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryBufferObjectuiv] = MonitoredFunctionData(L"glGetQueryBufferObjectuiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glMemoryBarrierByRegion] = MonitoredFunctionData(L"glMemoryBarrierByRegion", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glGetTextureSubImage] = MonitoredFunctionData(L"glGetTextureSubImage", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetCompressedTextureSubImage] = MonitoredFunctionData(L"glGetCompressedTextureSubImage", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetGraphicsResetStatus] = MonitoredFunctionData(L"glGetGraphicsResetStatus", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnCompressedTexImage] = MonitoredFunctionData(L"glGetnCompressedTexImage", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetnTexImage] = MonitoredFunctionData(L"glGetnTexImage", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetnUniformdv] = MonitoredFunctionData(L"glGetnUniformdv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetnUniformfv] = MonitoredFunctionData(L"glGetnUniformfv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetnUniformiv] = MonitoredFunctionData(L"glGetnUniformiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetnUniformuiv] = MonitoredFunctionData(L"glGetnUniformuiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glReadnPixels] = MonitoredFunctionData(L"glReadnPixels", AP_OPENGL_EXTENSION_FUNC, AP_RASTER_FUNC);
    _monitoredFunctionsData[ap_glGetnMapdv] = MonitoredFunctionData(L"glGetnMapdv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnMapfv] = MonitoredFunctionData(L"glGetnMapfv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnMapiv] = MonitoredFunctionData(L"glGetnMapiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnPixelMapfv] = MonitoredFunctionData(L"glGetnPixelMapfv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnPixelMapuiv] = MonitoredFunctionData(L"glGetnPixelMapuiv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnPixelMapusv] = MonitoredFunctionData(L"glGetnPixelMapusv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnPolygonStipple] = MonitoredFunctionData(L"glGetnPolygonStipple", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnColorTable] = MonitoredFunctionData(L"glGetnColorTable", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnConvolutionFilter] = MonitoredFunctionData(L"glGetnConvolutionFilter", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnSeparableFilter] = MonitoredFunctionData(L"glGetnSeparableFilter", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnHistogram] = MonitoredFunctionData(L"glGetnHistogram", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetnMinmax] = MonitoredFunctionData(L"glGetnMinmax", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glTextureBarrier] = MonitoredFunctionData(L"glTextureBarrier", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_SYNCHRONIZATION_FUNC);
}

void apMonitoredFunctionsManager::initializeWGLFunctionsData()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    _monitoredFunctionsData[ap_wglChoosePixelFormat] = MonitoredFunctionData(L"wglChoosePixelFormat", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglCopyContext] = MonitoredFunctionData(L"wglCopyContext", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglCreateContext] = MonitoredFunctionData(L"wglCreateContext", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglCreateLayerContext] = MonitoredFunctionData(L"wglCreateLayerContext", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglDeleteContext] = MonitoredFunctionData(L"wglDeleteContext", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglDescribeLayerPlane] = MonitoredFunctionData(L"wglDescribeLayerPlane", AP_WGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglDescribePixelFormat] = MonitoredFunctionData(L"wglDescribePixelFormat", AP_WGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglGetCurrentContext] = MonitoredFunctionData(L"wglGetCurrentContext", AP_WGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglGetCurrentDC] = MonitoredFunctionData(L"wglGetCurrentDC", AP_WGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglGetDefaultProcAddress] = MonitoredFunctionData(L"wglGetDefaultProcAddress", AP_WGL_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglGetLayerPaletteEntries] = MonitoredFunctionData(L"wglGetLayerPaletteEntries", AP_WGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglGetPixelFormat] = MonitoredFunctionData(L"wglGetPixelFormat", AP_WGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglGetProcAddress] = MonitoredFunctionData(L"wglGetProcAddress", AP_WGL_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglMakeCurrent] = MonitoredFunctionData(L"wglMakeCurrent", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglRealizeLayerPalette] = MonitoredFunctionData(L"wglRealizeLayerPalette", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglSetLayerPaletteEntries] = MonitoredFunctionData(L"wglSetLayerPaletteEntries", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglSetPixelFormat] = MonitoredFunctionData(L"wglSetPixelFormat", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglShareLists] = MonitoredFunctionData(L"wglShareLists", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglSwapBuffers] = MonitoredFunctionData(L"wglSwapBuffers", AP_WGL_FUNC, AP_DRAW_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglSwapLayerBuffers] = MonitoredFunctionData(L"wglSwapLayerBuffers", AP_WGL_FUNC, AP_DRAW_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglSwapMultipleBuffers] = MonitoredFunctionData(L"wglSwapMultipleBuffers", AP_WGL_FUNC, AP_DRAW_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglUseFontBitmapsA] = MonitoredFunctionData(L"wglUseFontBitmapsA", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglUseFontBitmapsW] = MonitoredFunctionData(L"wglUseFontBitmapsW", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglUseFontOutlinesA] = MonitoredFunctionData(L"wglUseFontOutlinesA", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglUseFontOutlinesW] = MonitoredFunctionData(L"wglUseFontOutlinesW", AP_WGL_FUNC, AP_NULL_CONTEXT_FUNCTION);

#endif // AMDT_BUILD_TARGET
}


void apMonitoredFunctionsManager::initializeGLXFunctionsData()
{
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))

    _monitoredFunctionsData[ap_glXChooseVisual] = MonitoredFunctionData(L"glXChooseVisual", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXCopyContext] = MonitoredFunctionData(L"glXCopyContext", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXCreateContext] = MonitoredFunctionData(L"glXCreateContext", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXCreateGLXPixmap] = MonitoredFunctionData(L"glXCreateGLXPixmap", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXDestroyContext] = MonitoredFunctionData(L"glXDestroyContext", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXDestroyGLXPixmap] = MonitoredFunctionData(L"glXDestroyGLXPixmap", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetConfig] = MonitoredFunctionData(L"glXGetConfig", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetCurrentContext] = MonitoredFunctionData(L"glXGetCurrentContext", AP_GLX_FUNC , AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetCurrentDrawable] = MonitoredFunctionData(L"glXGetCurrentDrawable", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXIsDirect] = MonitoredFunctionData(L"glXIsDirect", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXMakeCurrent] = MonitoredFunctionData(L"glXMakeCurrent", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXQueryExtension] = MonitoredFunctionData(L"glXQueryExtension", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXQueryVersion] = MonitoredFunctionData(L"glXQueryVersion", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXSwapBuffers] = MonitoredFunctionData(L"glXSwapBuffers", AP_GLX_FUNC, AP_DRAW_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXUseXFont] = MonitoredFunctionData(L"glXUseXFont", AP_GLX_FUNC, 0);
    _monitoredFunctionsData[ap_glXWaitGL] = MonitoredFunctionData(L"glXWaitGL", AP_GLX_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glXWaitX] = MonitoredFunctionData(L"glXWaitX", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetClientString] = MonitoredFunctionData(L"glXGetClientString", AP_GLX_FUNC , AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXQueryServerString] = MonitoredFunctionData(L"glXQueryServerString", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXQueryExtensionsString] = MonitoredFunctionData(L"glXQueryExtensionsString", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetFBConfigs] = MonitoredFunctionData(L"glXGetFBConfigs", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXChooseFBConfig] = MonitoredFunctionData(L"glXChooseFBConfig", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetFBConfigAttrib] = MonitoredFunctionData(L"glXGetFBConfigAttrib", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetVisualFromFBConfig] = MonitoredFunctionData(L"glXGetVisualFromFBConfig", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXCreateWindow] = MonitoredFunctionData(L"glXCreateWindow", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXDestroyWindow] = MonitoredFunctionData(L"glXDestroyWindow", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXCreatePixmap] = MonitoredFunctionData(L"glXCreatePixmap", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXDestroyPixmap] = MonitoredFunctionData(L"glXDestroyPixmap", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXCreatePbuffer] = MonitoredFunctionData(L"glXCreatePbuffer", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXDestroyPbuffer] = MonitoredFunctionData(L"glXDestroyPbuffer", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXQueryDrawable] = MonitoredFunctionData(L"glXQueryDrawable", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXCreateNewContext] = MonitoredFunctionData(L"glXCreateNewContext", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXMakeContextCurrent] = MonitoredFunctionData(L"glXMakeContextCurrent", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetCurrentReadDrawable] = MonitoredFunctionData(L"glXGetCurrentReadDrawable", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetCurrentDisplay] = MonitoredFunctionData(L"glXGetCurrentDisplay", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXQueryContext] = MonitoredFunctionData(L"glXQueryContext", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXSelectEvent] = MonitoredFunctionData(L"glXSelectEvent", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetSelectedEvent] = MonitoredFunctionData(L"glXGetSelectedEvent", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetProcAddress] = MonitoredFunctionData(L"glXGetProcAddress", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetProcAddressARB] = MonitoredFunctionData(L"glXGetProcAddressARB", AP_GLX_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);

    _monitoredFunctionsData[ap_glXCreateContextAttribsARB] = MonitoredFunctionData(L"glXCreateContextAttribsARB", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);

    /*
    _monitoredFunctionsData[ap_glXGetContextIDEXT] = MonitoredFunctionData(L"glXGetContextIDEXT", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetCurrentDrawableEXT] = MonitoredFunctionData(L"glXGetCurrentDrawableEXT", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXImportContextEXT] = MonitoredFunctionData(L"glXImportContextEXT", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXFreeContextEXT] = MonitoredFunctionData(L"glXFreeContextEXT", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXQueryContextInfoEXT] = MonitoredFunctionData(L"glXQueryContextInfoEXT", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetProcAddressARB] = MonitoredFunctionData(L"glXGetProcAddressARB", AP_GLX_FUNC, AP_NULL_CONTEXT_FUNCTION);
    */

    //////////////////////////////////////////////////////////////////////////
    // GLX Extensions
    //////////////////////////////////////////////////////////////////////////

    // GLX_SGIX_fbconfig:
    _monitoredFunctionsData[ap_glXGetFBConfigAttribSGIX] = MonitoredFunctionData(L"glXGetFBConfigAttribSGIX", AP_GLX_EXTENSION_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXChooseFBConfigSGIX] = MonitoredFunctionData(L"glXChooseFBConfigSGIX", AP_GLX_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXCreateGLXPixmapWithConfigSGIX] = MonitoredFunctionData(L"glXCreateGLXPixmapWithConfigSGIX", AP_GLX_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXCreateContextWithConfigSGIX] = MonitoredFunctionData(L"glXCreateContextWithConfigSGIX", AP_GLX_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetVisualFromFBConfigSGIX] = MonitoredFunctionData(L"glXGetVisualFromFBConfigSGIX", AP_GLX_EXTENSION_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetFBConfigFromVisualSGIX] = MonitoredFunctionData(L"glXGetFBConfigFromVisualSGIX", AP_GLX_EXTENSION_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);

    // GLX_SGI_video_sync
    _monitoredFunctionsData[ap_glXGetVideoSyncSGI] = MonitoredFunctionData(L"glXGetVideoSyncSGI", AP_GLX_EXTENSION_FUNC, AP_GET_FUNC | AP_SYNCHRONIZATION_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXWaitVideoSyncSGI] = MonitoredFunctionData(L"glXWaitVideoSyncSGI", AP_GLX_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC | AP_NULL_CONTEXT_FUNCTION);

    // GLX_ARB_get_proc_address
    // See above

    // GLX_ARB_multisample
    // See GL_ARB_multisample

    // GLX_ARB_fbconfig_float
    // See GL_ARB_color_buffer_float

    // GLX_ARB_framebuffer_sRGB
    // See GL_ARB_framebuffer_sRGB

    // GLX_SGIX_pbuffer
    _monitoredFunctionsData[ap_glXCreateGLXPbufferSGIX] = MonitoredFunctionData(L"glXCreateGLXPbufferSGIX", AP_GLX_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXDestroyGLXPbufferSGIX] = MonitoredFunctionData(L"glXDestroyGLXPbufferSGIX", AP_GLX_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXQueryGLXPbufferSGIX] = MonitoredFunctionData(L"glXQueryGLXPbufferSGIX", AP_GLX_EXTENSION_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXSelectEventSGIX] = MonitoredFunctionData(L"glXSelectEventSGIX", AP_GLX_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_glXGetSelectedEventSGIX] = MonitoredFunctionData(L"glXGetSelectedEventSGIX", AP_GLX_EXTENSION_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
#endif
}

void apMonitoredFunctionsManager::initializeCGLFunctionsData()
{
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    _monitoredFunctionsData[ap_CGLChoosePixelFormat] = MonitoredFunctionData(L"CGLChoosePixelFormat", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLDestroyPixelFormat] = MonitoredFunctionData(L"CGLDestroyPixelFormat", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLDescribePixelFormat] = MonitoredFunctionData(L"CGLDescribePixelFormat", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLCreateContext] = MonitoredFunctionData(L"CGLCreateContext", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLCopyContext] = MonitoredFunctionData(L"CGLCopyContext", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLDestroyContext] = MonitoredFunctionData(L"CGLDestroyContext", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLGetCurrentContext] = MonitoredFunctionData(L"CGLGetCurrentContext", AP_CGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLSetCurrentContext] = MonitoredFunctionData(L"CGLSetCurrentContext", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLEnable] = MonitoredFunctionData(L"CGLEnable", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLDisable] = MonitoredFunctionData(L"CGLDisable", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLIsEnabled] = MonitoredFunctionData(L"CGLIsEnabled", AP_CGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLSetParameter] = MonitoredFunctionData(L"CGLSetParameter", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLGetParameter] = MonitoredFunctionData(L"CGLGetParameter", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLLockContext] = MonitoredFunctionData(L"CGLLockContext", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLUnlockContext] = MonitoredFunctionData(L"CGLUnlockContext", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLSetOffScreen] = MonitoredFunctionData(L"CGLSetOffScreen", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLGetOffScreen] = MonitoredFunctionData(L"CGLGetOffScreen", AP_CGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLSetFullScreen] = MonitoredFunctionData(L"CGLSetFullScreen", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLClearDrawable] = MonitoredFunctionData(L"CGLClearDrawable", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLFlushDrawable] = MonitoredFunctionData(L"CGLFlushDrawable", AP_CGL_FUNC, AP_DRAW_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLCreatePBuffer] = MonitoredFunctionData(L"CGLCreatePBuffer", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLDescribePBuffer] = MonitoredFunctionData(L"CGLDescribePBuffer", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLDestroyPBuffer] = MonitoredFunctionData(L"CGLDestroyPBuffer", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLGetPBuffer] = MonitoredFunctionData(L"CGLGetPBuffer", AP_CGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLSetPBuffer] = MonitoredFunctionData(L"CGLSetPBuffer", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLTexImagePBuffer] = MonitoredFunctionData(L"CGLTexImagePBuffer", AP_CGL_FUNC, AP_TEXTURE_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLErrorString] = MonitoredFunctionData(L"CGLErrorString", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLSetOption] = MonitoredFunctionData(L"CGLSetOption", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLGetOption] = MonitoredFunctionData(L"CGLGetOption", AP_CGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLGetVersion] = MonitoredFunctionData(L"CGLGetVersion", AP_CGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLDescribeRenderer] = MonitoredFunctionData(L"CGLDescribeRenderer", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLDestroyRendererInfo] = MonitoredFunctionData(L"CGLDestroyRendererInfo", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLQueryRendererInfo] = MonitoredFunctionData(L"CGLQueryRendererInfo", AP_CGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLSetVirtualScreen] = MonitoredFunctionData(L"CGLSetVirtualScreen", AP_CGL_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_CGLGetVirtualScreen] = MonitoredFunctionData(L"CGLGetVirtualScreen", AP_CGL_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);

#endif // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
}


void apMonitoredFunctionsManager::initializeOpenGLExtensionFunctionsData()
{
    // GL_NV_primitive_restart
    _monitoredFunctionsData[ap_glPrimitiveRestartNV] = MonitoredFunctionData(L"glPrimitiveRestartNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPrimitiveRestartIndexNV] = MonitoredFunctionData(L"glPrimitiveRestartIndexNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_HP_occlusion_test extension
    // No functions

    // GL_NV_occlusion_query
    _monitoredFunctionsData[ap_glGenOcclusionQueriesNV] = MonitoredFunctionData(L"glGenOcclusionQueriesNV", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glDeleteOcclusionQueriesNV] = MonitoredFunctionData(L"glDeleteOcclusionQueriesNV", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glIsOcclusionQueryNV] = MonitoredFunctionData(L"glIsOcclusionQueryNV", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glBeginOcclusionQueryNV] = MonitoredFunctionData(L"glBeginOcclusionQueryNV", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glEndOcclusionQueryNV] = MonitoredFunctionData(L"glEndOcclusionQueryNV", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetOcclusionQueryivNV] = MonitoredFunctionData(L"glGetOcclusionQueryivNV", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetOcclusionQueryuivNV] = MonitoredFunctionData(L"glGetOcclusionQueryuivNV", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC | AP_GET_FUNC);

    // GL_ARB_occlusion_query extension
    _monitoredFunctionsData[ap_glGenQueriesARB] = MonitoredFunctionData(L"glGenQueriesARB", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glDeleteQueriesARB] = MonitoredFunctionData(L"glDeleteQueriesARB", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glIsQueryARB] = MonitoredFunctionData(L"glIsQueryARB", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glBeginQueryARB] = MonitoredFunctionData(L"glBeginQueryARB", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glEndQueryARB] = MonitoredFunctionData(L"glEndQueryARB", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryivARB] = MonitoredFunctionData(L"glGetQueryivARB", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC);
    _monitoredFunctionsData[ap_glGetQueryObjectivARB] = MonitoredFunctionData(L"glGetQueryObjectivARB", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetQueryObjectuivARB] = MonitoredFunctionData(L"glGetQueryObjectuivARB", AP_OPENGL_EXTENSION_FUNC, AP_QUERY_FUNC | AP_GET_FUNC);

    // GL_ARB_texture_cube_map extension
    // No functions

    // GL_ARB_texture_compression extension
    _monitoredFunctionsData[ap_glCompressedTexImage3DARB] = MonitoredFunctionData(L"glCompressedTexImage3DARB", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTexImage2DARB] = MonitoredFunctionData(L"glCompressedTexImage2DARB", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTexImage1DARB] = MonitoredFunctionData(L"glCompressedTexImage1DARB", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTexSubImage3DARB] = MonitoredFunctionData(L"glCompressedTexSubImage3DARB", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTexSubImage2DARB] = MonitoredFunctionData(L"glCompressedTexSubImage2DARB", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTexSubImage1DARB] = MonitoredFunctionData(L"glCompressedTexSubImage1DARB", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetCompressedTexImageARB] = MonitoredFunctionData(L"glGetCompressedTexImageARB", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_GET_FUNC);

    // GL_ARB_vertex_buffer_object extension
    _monitoredFunctionsData[ap_glBindBufferARB] = MonitoredFunctionData(L"glBindBufferARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glDeleteBuffersARB] = MonitoredFunctionData(L"glDeleteBuffersARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGenBuffersARB] = MonitoredFunctionData(L"glGenBuffersARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glIsBufferARB] = MonitoredFunctionData(L"glIsBufferARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glBufferDataARB] = MonitoredFunctionData(L"glBufferDataARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glBufferSubDataARB] = MonitoredFunctionData(L"glBufferSubDataARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetBufferSubDataARB] = MonitoredFunctionData(L"glGetBufferSubDataARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glMapBufferARB] = MonitoredFunctionData(L"glMapBufferARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glUnmapBufferARB] = MonitoredFunctionData(L"glUnmapBufferARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetBufferParameterivARB] = MonitoredFunctionData(L"glGetBufferParameterivARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetBufferPointervARB] = MonitoredFunctionData(L"glGetBufferPointervARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);

    // GL_ARB_vertex_blend extension
    _monitoredFunctionsData[ap_glWeightbvARB] = MonitoredFunctionData(L"glWeightbvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWeightsvARB] = MonitoredFunctionData(L"glWeightsvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWeightivARB] = MonitoredFunctionData(L"glWeightivARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWeightfvARB] = MonitoredFunctionData(L"glWeightfvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWeightdvARB] = MonitoredFunctionData(L"glWeightdvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWeightubvARB] = MonitoredFunctionData(L"glWeightubvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWeightusvARB] = MonitoredFunctionData(L"glWeightusvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWeightuivARB] = MonitoredFunctionData(L"glWeightuivARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWeightPointerARB] = MonitoredFunctionData(L"glWeightPointerARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glVertexBlendARB] = MonitoredFunctionData(L"glVertexBlendARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_EXT_texture3D extension:
    _monitoredFunctionsData[ap_glTexImage3DEXT] = MonitoredFunctionData(L"glTexImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexSubImage3DEXT] = MonitoredFunctionData(L"glTexSubImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);

    // GL_ARB_vertex_program extension:
    _monitoredFunctionsData[ap_glVertexAttrib1dARB] = MonitoredFunctionData(L"glVertexAttrib1dARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1dvARB] = MonitoredFunctionData(L"glVertexAttrib1dvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1fARB] = MonitoredFunctionData(L"glVertexAttrib1fARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1fvARB] = MonitoredFunctionData(L"glVertexAttrib1fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1sARB] = MonitoredFunctionData(L"glVertexAttrib1sARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1svARB] = MonitoredFunctionData(L"glVertexAttrib1svARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2dARB] = MonitoredFunctionData(L"glVertexAttrib2dARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2dvARB] = MonitoredFunctionData(L"glVertexAttrib2dvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2fARB] = MonitoredFunctionData(L"glVertexAttrib2fARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2fvARB] = MonitoredFunctionData(L"glVertexAttrib2fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2sARB] = MonitoredFunctionData(L"glVertexAttrib2sARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2svARB] = MonitoredFunctionData(L"glVertexAttrib2svARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3dARB] = MonitoredFunctionData(L"glVertexAttrib3dARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3dvARB] = MonitoredFunctionData(L"glVertexAttrib3dvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3fARB] = MonitoredFunctionData(L"glVertexAttrib3fARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3fvARB] = MonitoredFunctionData(L"glVertexAttrib3fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3sARB] = MonitoredFunctionData(L"glVertexAttrib3sARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3svARB] = MonitoredFunctionData(L"glVertexAttrib3svARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4NbvARB] = MonitoredFunctionData(L"glVertexAttrib4NbvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4NivARB] = MonitoredFunctionData(L"glVertexAttrib4NivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4NsvARB] = MonitoredFunctionData(L"glVertexAttrib4NsvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4NubARB] = MonitoredFunctionData(L"glVertexAttrib4NubARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4NubvARB] = MonitoredFunctionData(L"glVertexAttrib4NubvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4NuivARB] = MonitoredFunctionData(L"glVertexAttrib4NuivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4NusvARB] = MonitoredFunctionData(L"glVertexAttrib4NusvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4bvARB] = MonitoredFunctionData(L"glVertexAttrib4bvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4dARB] = MonitoredFunctionData(L"glVertexAttrib4dARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4dvARB] = MonitoredFunctionData(L"glVertexAttrib4dvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4fARB] = MonitoredFunctionData(L"glVertexAttrib4fARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4fvARB] = MonitoredFunctionData(L"glVertexAttrib4fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4ivARB] = MonitoredFunctionData(L"glVertexAttrib4ivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4sARB] = MonitoredFunctionData(L"glVertexAttrib4sARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4svARB] = MonitoredFunctionData(L"glVertexAttrib4svARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4ubvARB] = MonitoredFunctionData(L"glVertexAttrib4ubvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4uivARB] = MonitoredFunctionData(L"glVertexAttrib4uivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4usvARB] = MonitoredFunctionData(L"glVertexAttrib4usvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribPointerARB] = MonitoredFunctionData(L"glVertexAttribPointerARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glEnableVertexAttribArrayARB] = MonitoredFunctionData(L"glEnableVertexAttribArrayARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDisableVertexAttribArrayARB] = MonitoredFunctionData(L"glDisableVertexAttribArrayARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramStringARB] = MonitoredFunctionData(L"glProgramStringARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindProgramARB] = MonitoredFunctionData(L"glBindProgramARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDeleteProgramsARB] = MonitoredFunctionData(L"glDeleteProgramsARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGenProgramsARB] = MonitoredFunctionData(L"glGenProgramsARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramEnvParameter4dARB] = MonitoredFunctionData(L"glProgramEnvParameter4dARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramEnvParameter4dvARB] = MonitoredFunctionData(L"glProgramEnvParameter4dvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramEnvParameter4fARB] = MonitoredFunctionData(L"glProgramEnvParameter4fARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramEnvParameter4fvARB] = MonitoredFunctionData(L"glProgramEnvParameter4fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramLocalParameter4dARB] = MonitoredFunctionData(L"glProgramLocalParameter4dARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramLocalParameter4dvARB] = MonitoredFunctionData(L"glProgramLocalParameter4dvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramLocalParameter4fARB] = MonitoredFunctionData(L"glProgramLocalParameter4fARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramLocalParameter4fvARB] = MonitoredFunctionData(L"glProgramLocalParameter4fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetProgramEnvParameterdvARB] = MonitoredFunctionData(L"glGetProgramEnvParameterdvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetProgramEnvParameterfvARB] = MonitoredFunctionData(L"glGetProgramEnvParameterfvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetProgramLocalParameterdvARB] = MonitoredFunctionData(L"glGetProgramLocalParameterdvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetProgramLocalParameterfvARB] = MonitoredFunctionData(L"glGetProgramLocalParameterfvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetProgramivARB] = MonitoredFunctionData(L"glGetProgramivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetProgramStringARB] = MonitoredFunctionData(L"glGetProgramStringARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribdvARB] = MonitoredFunctionData(L"glGetVertexAttribdvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribfvARB] = MonitoredFunctionData(L"glGetVertexAttribfvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribivARB] = MonitoredFunctionData(L"glGetVertexAttribivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribPointervARB] = MonitoredFunctionData(L"glGetVertexAttribPointervARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glIsProgramARB] = MonitoredFunctionData(L"glIsProgramARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);

    // GL_ARB_vertex_shader extension:
    _monitoredFunctionsData[ap_glBindAttribLocationARB] = MonitoredFunctionData(L"glBindAttribLocationARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetActiveAttribARB] = MonitoredFunctionData(L"glGetActiveAttribARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetAttribLocationARB] = MonitoredFunctionData(L"glGetAttribLocationARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);

    // GL_ARB_shader_objects extension:
    _monitoredFunctionsData[ap_glDeleteObjectARB] = MonitoredFunctionData(L"glDeleteObjectARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetHandleARB] = MonitoredFunctionData(L"glGetHandleARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glDetachObjectARB] = MonitoredFunctionData(L"glDetachObjectARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glCreateShaderObjectARB] = MonitoredFunctionData(L"glCreateShaderObjectARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glShaderSourceARB] = MonitoredFunctionData(L"glShaderSourceARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glCompileShaderARB] = MonitoredFunctionData(L"glCompileShaderARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glCreateProgramObjectARB] = MonitoredFunctionData(L"glCreateProgramObjectARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glAttachObjectARB] = MonitoredFunctionData(L"glAttachObjectARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glLinkProgramARB] = MonitoredFunctionData(L"glLinkProgramARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUseProgramObjectARB] = MonitoredFunctionData(L"glUseProgramObjectARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glValidateProgramARB] = MonitoredFunctionData(L"glValidateProgramARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform1fARB] = MonitoredFunctionData(L"glUniform1fARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2fARB] = MonitoredFunctionData(L"glUniform2fARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3fARB] = MonitoredFunctionData(L"glUniform3fARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4fARB] = MonitoredFunctionData(L"glUniform4fARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform1iARB] = MonitoredFunctionData(L"glUniform1iARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2iARB] = MonitoredFunctionData(L"glUniform2iARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3iARB] = MonitoredFunctionData(L"glUniform3iARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4iARB] = MonitoredFunctionData(L"glUniform4iARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform1fvARB] = MonitoredFunctionData(L"glUniform1fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2fvARB] = MonitoredFunctionData(L"glUniform2fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3fvARB] = MonitoredFunctionData(L"glUniform3fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4fvARB] = MonitoredFunctionData(L"glUniform4fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform1ivARB] = MonitoredFunctionData(L"glUniform1ivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform2ivARB] = MonitoredFunctionData(L"glUniform2ivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform3ivARB] = MonitoredFunctionData(L"glUniform3ivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniform4ivARB] = MonitoredFunctionData(L"glUniform4ivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix2fvARB] = MonitoredFunctionData(L"glUniformMatrix2fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix3fvARB] = MonitoredFunctionData(L"glUniformMatrix3fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformMatrix4fvARB] = MonitoredFunctionData(L"glUniformMatrix4fvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetObjectParameterfvARB] = MonitoredFunctionData(L"glGetObjectParameterfvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetObjectParameterivARB] = MonitoredFunctionData(L"glGetObjectParameterivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetInfoLogARB] = MonitoredFunctionData(L"glGetInfoLogARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetAttachedObjectsARB] = MonitoredFunctionData(L"glGetAttachedObjectsARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetUniformLocationARB] = MonitoredFunctionData(L"glGetUniformLocationARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetActiveUniformARB] = MonitoredFunctionData(L"glGetActiveUniformARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetUniformfvARB] = MonitoredFunctionData(L"glGetUniformfvARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetUniformivARB] = MonitoredFunctionData(L"glGetUniformivARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetShaderSourceARB] = MonitoredFunctionData(L"glGetShaderSourceARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);

    // GL_NV_vertex_program extension:
    _monitoredFunctionsData[ap_glAreProgramsResidentNV] = MonitoredFunctionData(L"glAreProgramsResidentNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindProgramNV] = MonitoredFunctionData(L"glBindProgramNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDeleteProgramsNV] = MonitoredFunctionData(L"glDeleteProgramsNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glExecuteProgramNV] = MonitoredFunctionData(L"glExecuteProgramNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGenProgramsNV] = MonitoredFunctionData(L"glGenProgramsNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetProgramParameterdvNV] = MonitoredFunctionData(L"glGetProgramParameterdvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetProgramParameterfvNV] = MonitoredFunctionData(L"glGetProgramParameterfvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetProgramivNV] = MonitoredFunctionData(L"glGetProgramivNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetProgramStringNV] = MonitoredFunctionData(L"glGetProgramStringNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetTrackMatrixivNV] = MonitoredFunctionData(L"glGetTrackMatrixivNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribdvNV] = MonitoredFunctionData(L"glGetVertexAttribdvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribfvNV] = MonitoredFunctionData(L"glGetVertexAttribfvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribivNV] = MonitoredFunctionData(L"glGetVertexAttribivNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVertexAttribPointervNV] = MonitoredFunctionData(L"glGetVertexAttribPointervNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glIsProgramNV] = MonitoredFunctionData(L"glIsProgramNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glLoadProgramNV] = MonitoredFunctionData(L"glLoadProgramNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramParameter4dNV] = MonitoredFunctionData(L"glProgramParameter4dNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramParameter4dvNV] = MonitoredFunctionData(L"glProgramParameter4dvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramParameter4fNV] = MonitoredFunctionData(L"glProgramParameter4fNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramParameter4fvNV] = MonitoredFunctionData(L"glProgramParameter4fvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramParameters4dvNV] = MonitoredFunctionData(L"glProgramParameters4dvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramParameters4fvNV] = MonitoredFunctionData(L"glProgramParameters4fvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glRequestResidentProgramsNV] = MonitoredFunctionData(L"glRequestResidentProgramsNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glTrackMatrixNV] = MonitoredFunctionData(L"glTrackMatrixNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribPointerNV] = MonitoredFunctionData(L"glVertexAttribPointerNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1dNV] = MonitoredFunctionData(L"glVertexAttrib1dNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1dvNV] = MonitoredFunctionData(L"glVertexAttrib1dvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1fNV] = MonitoredFunctionData(L"glVertexAttrib1fNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1fvNV] = MonitoredFunctionData(L"glVertexAttrib1fvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1sNV] = MonitoredFunctionData(L"glVertexAttrib1sNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib1svNV] = MonitoredFunctionData(L"glVertexAttrib1svNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2dNV] = MonitoredFunctionData(L"glVertexAttrib2dNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2dvNV] = MonitoredFunctionData(L"glVertexAttrib2dvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2fNV] = MonitoredFunctionData(L"glVertexAttrib2fNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2fvNV] = MonitoredFunctionData(L"glVertexAttrib2fvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2sNV] = MonitoredFunctionData(L"glVertexAttrib2sNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib2svNV] = MonitoredFunctionData(L"glVertexAttrib2svNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3dNV] = MonitoredFunctionData(L"glVertexAttrib3dNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3dvNV] = MonitoredFunctionData(L"glVertexAttrib3dvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3fNV] = MonitoredFunctionData(L"glVertexAttrib3fNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3fvNV] = MonitoredFunctionData(L"glVertexAttrib3fvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3sNV] = MonitoredFunctionData(L"glVertexAttrib3sNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib3svNV] = MonitoredFunctionData(L"glVertexAttrib3svNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4dNV] = MonitoredFunctionData(L"glVertexAttrib4dNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4dvNV] = MonitoredFunctionData(L"glVertexAttrib4dvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4fNV] = MonitoredFunctionData(L"glVertexAttrib4fNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4fvNV] = MonitoredFunctionData(L"glVertexAttrib4fvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4sNV] = MonitoredFunctionData(L"glVertexAttrib4sNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4svNV] = MonitoredFunctionData(L"glVertexAttrib4svNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4ubNV] = MonitoredFunctionData(L"glVertexAttrib4ubNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttrib4ubvNV] = MonitoredFunctionData(L"glVertexAttrib4ubvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs1dvNV] = MonitoredFunctionData(L"glVertexAttribs1dvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs1fvNV] = MonitoredFunctionData(L"glVertexAttribs1fvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs1svNV] = MonitoredFunctionData(L"glVertexAttribs1svNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs2dvNV] = MonitoredFunctionData(L"glVertexAttribs2dvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs2fvNV] = MonitoredFunctionData(L"glVertexAttribs2fvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs2svNV] = MonitoredFunctionData(L"glVertexAttribs2svNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs3dvNV] = MonitoredFunctionData(L"glVertexAttribs3dvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs3fvNV] = MonitoredFunctionData(L"glVertexAttribs3fvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs3svNV] = MonitoredFunctionData(L"glVertexAttribs3svNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs4dvNV] = MonitoredFunctionData(L"glVertexAttribs4dvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs4fvNV] = MonitoredFunctionData(L"glVertexAttribs4fvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs4svNV] = MonitoredFunctionData(L"glVertexAttribs4svNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribs4ubvNV] = MonitoredFunctionData(L"glVertexAttribs4ubvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);

    // GL_ATI_fragment_shader:
    _monitoredFunctionsData[ap_glGenFragmentShadersATI] = MonitoredFunctionData(L"glGenFragmentShadersATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindFragmentShaderATI] = MonitoredFunctionData(L"glBindFragmentShaderATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDeleteFragmentShaderATI] = MonitoredFunctionData(L"glDeleteFragmentShaderATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBeginFragmentShaderATI] = MonitoredFunctionData(L"glBeginFragmentShaderATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glEndFragmentShaderATI] = MonitoredFunctionData(L"glEndFragmentShaderATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glPassTexCoordATI] = MonitoredFunctionData(L"glPassTexCoordATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glSampleMapATI] = MonitoredFunctionData(L"glSampleMapATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glColorFragmentOp1ATI] = MonitoredFunctionData(L"glColorFragmentOp1ATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glColorFragmentOp2ATI] = MonitoredFunctionData(L"glColorFragmentOp2ATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glColorFragmentOp3ATI] = MonitoredFunctionData(L"glColorFragmentOp3ATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glAlphaFragmentOp1ATI] = MonitoredFunctionData(L"glAlphaFragmentOp1ATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glAlphaFragmentOp2ATI] = MonitoredFunctionData(L"glAlphaFragmentOp2ATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glAlphaFragmentOp3ATI] = MonitoredFunctionData(L"glAlphaFragmentOp3ATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glSetFragmentShaderConstantATI] = MonitoredFunctionData(L"glSetFragmentShaderConstantATI", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);

    // GL_EXT_vertex_shader:
    _monitoredFunctionsData[ap_glBeginVertexShaderEXT] = MonitoredFunctionData(L"glBeginVertexShaderEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glEndVertexShaderEXT] = MonitoredFunctionData(L"glEndVertexShaderEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindVertexShaderEXT] = MonitoredFunctionData(L"glBindVertexShaderEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGenVertexShadersEXT] = MonitoredFunctionData(L"glGenVertexShadersEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDeleteVertexShaderEXT] = MonitoredFunctionData(L"glDeleteVertexShaderEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glShaderOp1EXT] = MonitoredFunctionData(L"glShaderOp1EXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glShaderOp2EXT] = MonitoredFunctionData(L"glShaderOp2EXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glShaderOp3EXT] = MonitoredFunctionData(L"glShaderOp3EXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glSwizzleEXT] = MonitoredFunctionData(L"glSwizzleEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glWriteMaskEXT] = MonitoredFunctionData(L"glWriteMaskEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glInsertComponentEXT] = MonitoredFunctionData(L"glInsertComponentEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glExtractComponentEXT] = MonitoredFunctionData(L"glExtractComponentEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGenSymbolsEXT] = MonitoredFunctionData(L"glGenSymbolsEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glSetInvariantEXT] = MonitoredFunctionData(L"glSetInvariantEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glSetLocalConstantEXT] = MonitoredFunctionData(L"glSetLocalConstantEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVariantbvEXT] = MonitoredFunctionData(L"glVariantbvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVariantsvEXT] = MonitoredFunctionData(L"glVariantsvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVariantivEXT] = MonitoredFunctionData(L"glVariantivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVariantfvEXT] = MonitoredFunctionData(L"glVariantfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVariantdvEXT] = MonitoredFunctionData(L"glVariantdvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVariantubvEXT] = MonitoredFunctionData(L"glVariantubvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVariantusvEXT] = MonitoredFunctionData(L"glVariantusvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVariantuivEXT] = MonitoredFunctionData(L"glVariantuivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glVariantPointerEXT] = MonitoredFunctionData(L"glVariantPointerEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glEnableVariantClientStateEXT] = MonitoredFunctionData(L"glEnableVariantClientStateEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDisableVariantClientStateEXT] = MonitoredFunctionData(L"glDisableVariantClientStateEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindLightParameterEXT] = MonitoredFunctionData(L"glBindLightParameterEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindMaterialParameterEXT] = MonitoredFunctionData(L"glBindMaterialParameterEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindTexGenParameterEXT] = MonitoredFunctionData(L"glBindTexGenParameterEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindTextureUnitParameterEXT] = MonitoredFunctionData(L"glBindTextureUnitParameterEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glBindParameterEXT] = MonitoredFunctionData(L"glBindParameterEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glIsVariantEnabledEXT] = MonitoredFunctionData(L"glIsVariantEnabledEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVariantBooleanvEXT] = MonitoredFunctionData(L"glGetVariantBooleanvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVariantIntegervEXT] = MonitoredFunctionData(L"glGetVariantIntegervEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVariantFloatvEXT] = MonitoredFunctionData(L"glGetVariantFloatvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetVariantPointervEXT] = MonitoredFunctionData(L"glGetVariantPointervEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetInvariantBooleanvEXT] = MonitoredFunctionData(L"glGetInvariantBooleanvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetInvariantIntegervEXT] = MonitoredFunctionData(L"glGetInvariantIntegervEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetInvariantFloatvEXT] = MonitoredFunctionData(L"glGetInvariantFloatvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetLocalConstantBooleanvEXT] = MonitoredFunctionData(L"glGetLocalConstantBooleanvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetLocalConstantIntegervEXT] = MonitoredFunctionData(L"glGetLocalConstantIntegervEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetLocalConstantFloatvEXT] = MonitoredFunctionData(L"glGetLocalConstantFloatvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);

    // GL_NV_fragment_program:
    _monitoredFunctionsData[ap_glProgramNamedParameter4fNV] = MonitoredFunctionData(L"glProgramNamedParameter4fNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramNamedParameter4dNV] = MonitoredFunctionData(L"glProgramNamedParameter4dNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramNamedParameter4fvNV] = MonitoredFunctionData(L"glProgramNamedParameter4fvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramNamedParameter4dvNV] = MonitoredFunctionData(L"glProgramNamedParameter4dvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetProgramNamedParameterfvNV] = MonitoredFunctionData(L"glGetProgramNamedParameterfvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetProgramNamedParameterdvNV] = MonitoredFunctionData(L"glGetProgramNamedParameterdvNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);

    // GL_NV_primitive_restart
    _monitoredFunctionsData[ap_glPrimitiveRestartNV] = MonitoredFunctionData(L"glPrimitiveRestartNV", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glPrimitiveRestartIndexNV] = MonitoredFunctionData(L"glPrimitiveRestartIndexNV", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);

    // GL_ARB_draw_buffers:
    _monitoredFunctionsData[ap_glDrawBuffersARB] = MonitoredFunctionData(L"glDrawBuffersARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);

    // GL_ATI_draw_buffers:
    _monitoredFunctionsData[ap_glDrawBuffersATI] = MonitoredFunctionData(L"glDrawBuffersATI", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);

    // GL_ARB_multitexture:
    _monitoredFunctionsData[ap_glActiveTextureARB] = MonitoredFunctionData(L"glActiveTextureARB", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glClientActiveTextureARB] = MonitoredFunctionData(L"glClientActiveTextureARB", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1dARB] = MonitoredFunctionData(L"glMultiTexCoord1dARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1dvARB] = MonitoredFunctionData(L"glMultiTexCoord1dvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1fARB] = MonitoredFunctionData(L"glMultiTexCoord1fARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1fvARB] = MonitoredFunctionData(L"glMultiTexCoord1fvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1iARB] = MonitoredFunctionData(L"glMultiTexCoord1iARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1ivARB] = MonitoredFunctionData(L"glMultiTexCoord1ivARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1sARB] = MonitoredFunctionData(L"glMultiTexCoord1sARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord1svARB] = MonitoredFunctionData(L"glMultiTexCoord1svARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2dARB] = MonitoredFunctionData(L"glMultiTexCoord2dARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2dvARB] = MonitoredFunctionData(L"glMultiTexCoord2dvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2fARB] = MonitoredFunctionData(L"glMultiTexCoord2fARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2fvARB] = MonitoredFunctionData(L"glMultiTexCoord2fvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2iARB] = MonitoredFunctionData(L"glMultiTexCoord2iARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2ivARB] = MonitoredFunctionData(L"glMultiTexCoord2ivARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2sARB] = MonitoredFunctionData(L"glMultiTexCoord2sARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord2svARB] = MonitoredFunctionData(L"glMultiTexCoord2svARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3dARB] = MonitoredFunctionData(L"glMultiTexCoord3dARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3dvARB] = MonitoredFunctionData(L"glMultiTexCoord3dvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3fARB] = MonitoredFunctionData(L"glMultiTexCoord3fARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3fvARB] = MonitoredFunctionData(L"glMultiTexCoord3fvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3iARB] = MonitoredFunctionData(L"glMultiTexCoord3iARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3ivARB] = MonitoredFunctionData(L"glMultiTexCoord3ivARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3sARB] = MonitoredFunctionData(L"glMultiTexCoord3sARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord3svARB] = MonitoredFunctionData(L"glMultiTexCoord3svARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4dARB] = MonitoredFunctionData(L"glMultiTexCoord4dARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4dvARB] = MonitoredFunctionData(L"glMultiTexCoord4dvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4fARB] = MonitoredFunctionData(L"glMultiTexCoord4fARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4fvARB] = MonitoredFunctionData(L"glMultiTexCoord4fvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4iARB] = MonitoredFunctionData(L"glMultiTexCoord4iARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4ivARB] = MonitoredFunctionData(L"glMultiTexCoord4ivARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4sARB] = MonitoredFunctionData(L"glMultiTexCoord4sARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoord4svARB] = MonitoredFunctionData(L"glMultiTexCoord4svARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_ARB_color_buffer_float
    _monitoredFunctionsData[ap_glClampColorARB] = MonitoredFunctionData(L"glClampColorARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_EXT_stencil_two_side
    _monitoredFunctionsData[ap_glActiveStencilFaceEXT] = MonitoredFunctionData(L"glActiveStencilFaceEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_EXT_framebuffer_object
    _monitoredFunctionsData[ap_glIsRenderbufferEXT] = MonitoredFunctionData(L"glIsRenderbufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glBindRenderbufferEXT] = MonitoredFunctionData(L"glBindRenderbufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glDeleteRenderbuffersEXT] = MonitoredFunctionData(L"glDeleteRenderbuffersEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGenRenderbuffersEXT] = MonitoredFunctionData(L"glGenRenderbuffersEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glRenderbufferStorageEXT] = MonitoredFunctionData(L"glRenderbufferStorageEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetRenderbufferParameterivEXT] = MonitoredFunctionData(L"glGetRenderbufferParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glIsFramebufferEXT] = MonitoredFunctionData(L"glIsFramebufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glBindFramebufferEXT] = MonitoredFunctionData(L"glBindFramebufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glDeleteFramebuffersEXT] = MonitoredFunctionData(L"glDeleteFramebuffersEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGenFramebuffersEXT] = MonitoredFunctionData(L"glGenFramebuffersEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glCheckFramebufferStatusEXT] = MonitoredFunctionData(L"glCheckFramebufferStatusEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTexture1DEXT] = MonitoredFunctionData(L"glFramebufferTexture1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTexture2DEXT] = MonitoredFunctionData(L"glFramebufferTexture2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTexture3DEXT] = MonitoredFunctionData(L"glFramebufferTexture3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferRenderbufferEXT] = MonitoredFunctionData(L"glFramebufferRenderbufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetFramebufferAttachmentParameterivEXT] = MonitoredFunctionData(L"glGetFramebufferAttachmentParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGenerateMipmapEXT] = MonitoredFunctionData(L"glGenerateMipmapEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);

    // GL_EXT_framebuffer_blit
    _monitoredFunctionsData[ap_glBlitFramebufferEXT] = MonitoredFunctionData(L"glBlitFramebufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);

    // GL_EXT_framebuffer_multisample
    _monitoredFunctionsData[ap_glRenderbufferStorageMultisampleEXT] = MonitoredFunctionData(L"glRenderbufferStorageMultisampleEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);

    // GL_ARB_framebuffer_object
    _monitoredFunctionsData[ap_glIsRenderbuffer] = MonitoredFunctionData(L"glIsRenderbuffer", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_GET_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glBindRenderbuffer] = MonitoredFunctionData(L"glBindRenderbuffer", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glDeleteRenderbuffers] = MonitoredFunctionData(L"glDeleteRenderbuffers", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGenRenderbuffers] = MonitoredFunctionData(L"glGenRenderbuffers", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glRenderbufferStorage] = MonitoredFunctionData(L"glRenderbufferStorage", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetRenderbufferParameteriv] = MonitoredFunctionData(L"glGetRenderbufferParameteriv", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glIsFramebuffer] = MonitoredFunctionData(L"glIsFramebuffer", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glBindFramebuffer] = MonitoredFunctionData(L"glBindFramebuffer", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glDeleteFramebuffers] = MonitoredFunctionData(L"glDeleteFramebuffers", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGenFramebuffers] = MonitoredFunctionData(L"glGenFramebuffers", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glCheckFramebufferStatus] = MonitoredFunctionData(L"glCheckFramebufferStatus", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTexture1D] = MonitoredFunctionData(L"glFramebufferTexture1D", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTexture2D] = MonitoredFunctionData(L"glFramebufferTexture2D", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTexture3D] = MonitoredFunctionData(L"glFramebufferTexture3D", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferRenderbuffer] = MonitoredFunctionData(L"glFramebufferRenderbuffer", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetFramebufferAttachmentParameteriv] = MonitoredFunctionData(L"glGetFramebufferAttachmentParameteriv", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGenerateMipmap] = MonitoredFunctionData(L"glGenerateMipmap", AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glBlitFramebuffer] = MonitoredFunctionData(L"glBlitFramebuffer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glRenderbufferStorageMultisample] = MonitoredFunctionData(L"glRenderbufferStorageMultisample", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTextureLayer] = MonitoredFunctionData(L"glFramebufferTextureLayer", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);

    // GL_EXT_direct_state_access
    _monitoredFunctionsData[ap_glClientAttribDefaultEXT] = MonitoredFunctionData(L"glClientAttribDefaultEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPushClientAttribDefaultEXT] = MonitoredFunctionData(L"glPushClientAttribDefaultEXT", AP_OPENGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_glMatrixLoadfEXT] = MonitoredFunctionData(L"glMatrixLoadfEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixLoaddEXT] = MonitoredFunctionData(L"glMatrixLoaddEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixMultfEXT] = MonitoredFunctionData(L"glMatrixMultfEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMatrixMultdEXT] = MonitoredFunctionData(L"glMatrixMultdEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMatrixLoadIdentityEXT] = MonitoredFunctionData(L"glMatrixLoadIdentityEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixRotatefEXT] = MonitoredFunctionData(L"glMatrixRotatefEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMatrixRotatedEXT] = MonitoredFunctionData(L"glMatrixRotatedEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMatrixScalefEXT] = MonitoredFunctionData(L"glMatrixScalefEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMatrixScaledEXT] = MonitoredFunctionData(L"glMatrixScaledEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMatrixTranslatefEXT] = MonitoredFunctionData(L"glMatrixTranslatefEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMatrixTranslatedEXT] = MonitoredFunctionData(L"glMatrixTranslatedEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMatrixFrustumEXT] = MonitoredFunctionData(L"glMatrixFrustumEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMatrixOrthoEXT] = MonitoredFunctionData(L"glMatrixOrthoEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMatrixPopEXT] = MonitoredFunctionData(L"glMatrixPopEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMatrixPushEXT] = MonitoredFunctionData(L"glMatrixPushEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixLoadTransposefEXT] = MonitoredFunctionData(L"glMatrixLoadTransposefEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixLoadTransposedEXT] = MonitoredFunctionData(L"glMatrixLoadTransposedEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixMultTransposefEXT] = MonitoredFunctionData(L"glMatrixMultTransposefEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixMultTransposedEXT] = MonitoredFunctionData(L"glMatrixMultTransposedEXT", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);

    _monitoredFunctionsData[ap_glTextureParameterfEXT] = MonitoredFunctionData(L"glTextureParameterfEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureParameterfvEXT] = MonitoredFunctionData(L"glTextureParameterfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureParameteriEXT] = MonitoredFunctionData(L"glTextureParameteriEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureParameterivEXT] = MonitoredFunctionData(L"glTextureParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureImage1DEXT] = MonitoredFunctionData(L"glTextureImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureImage2DEXT] = MonitoredFunctionData(L"glTextureImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureSubImage1DEXT] = MonitoredFunctionData(L"glTextureSubImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureSubImage2DEXT] = MonitoredFunctionData(L"glTextureSubImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTextureImage1DEXT] = MonitoredFunctionData(L"glCopyTextureImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTextureImage2DEXT] = MonitoredFunctionData(L"glCopyTextureImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTextureSubImage1DEXT] = MonitoredFunctionData(L"glCopyTextureSubImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTextureSubImage2DEXT] = MonitoredFunctionData(L"glCopyTextureSubImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureImageEXT] = MonitoredFunctionData(L"glGetTextureImageEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureParameterfvEXT] = MonitoredFunctionData(L"glGetTextureParameterfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureParameterivEXT] = MonitoredFunctionData(L"glGetTextureParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureLevelParameterfvEXT] = MonitoredFunctionData(L"glGetTextureLevelParameterfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureLevelParameterivEXT] = MonitoredFunctionData(L"glGetTextureLevelParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureImage3DEXT] = MonitoredFunctionData(L"glTextureImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureSubImage3DEXT] = MonitoredFunctionData(L"glTextureSubImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyTextureSubImage3DEXT] = MonitoredFunctionData(L"glCopyTextureSubImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexParameterfEXT] = MonitoredFunctionData(L"glMultiTexParameterfEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexParameterfvEXT] = MonitoredFunctionData(L"glMultiTexParameterfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexParameteriEXT] = MonitoredFunctionData(L"glMultiTexParameteriEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexParameterivEXT] = MonitoredFunctionData(L"glMultiTexParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexImage1DEXT] = MonitoredFunctionData(L"glMultiTexImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexImage2DEXT] = MonitoredFunctionData(L"glMultiTexImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexSubImage1DEXT] = MonitoredFunctionData(L"glMultiTexSubImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexSubImage2DEXT] = MonitoredFunctionData(L"glMultiTexSubImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyMultiTexImage1DEXT] = MonitoredFunctionData(L"glCopyMultiTexImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyMultiTexImage2DEXT] = MonitoredFunctionData(L"glCopyMultiTexImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyMultiTexSubImage1DEXT] = MonitoredFunctionData(L"glCopyMultiTexSubImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyMultiTexSubImage2DEXT] = MonitoredFunctionData(L"glCopyMultiTexSubImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexImageEXT] = MonitoredFunctionData(L"glGetMultiTexImageEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexParameterfvEXT] = MonitoredFunctionData(L"glGetMultiTexParameterfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexParameterivEXT] = MonitoredFunctionData(L"glGetMultiTexParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexLevelParameterfvEXT] = MonitoredFunctionData(L"glGetMultiTexLevelParameterfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexLevelParameterivEXT] = MonitoredFunctionData(L"glGetMultiTexLevelParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexImage3DEXT] = MonitoredFunctionData(L"glMultiTexImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexSubImage3DEXT] = MonitoredFunctionData(L"glMultiTexSubImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCopyMultiTexSubImage3DEXT] = MonitoredFunctionData(L"glCopyMultiTexSubImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glBindMultiTextureEXT] = MonitoredFunctionData(L"glBindMultiTextureEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glEnableClientStateIndexedEXT] = MonitoredFunctionData(L"glEnableClientStateIndexedEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDisableClientStateIndexedEXT] = MonitoredFunctionData(L"glDisableClientStateIndexedEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexCoordPointerEXT] = MonitoredFunctionData(L"glMultiTexCoordPointerEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexEnvfEXT] = MonitoredFunctionData(L"glMultiTexEnvfEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexEnvfvEXT] = MonitoredFunctionData(L"glMultiTexEnvfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexEnviEXT] = MonitoredFunctionData(L"glMultiTexEnviEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexEnvivEXT] = MonitoredFunctionData(L"glMultiTexEnvivEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexGendEXT] = MonitoredFunctionData(L"glMultiTexGendEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexGendvEXT] = MonitoredFunctionData(L"glMultiTexGendvEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexGenfEXT] = MonitoredFunctionData(L"glMultiTexGenfEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexGenfvEXT] = MonitoredFunctionData(L"glMultiTexGenfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexGeniEXT] = MonitoredFunctionData(L"glMultiTexGeniEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexGenivEXT] = MonitoredFunctionData(L"glMultiTexGenivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexEnvfvEXT] = MonitoredFunctionData(L"glGetMultiTexEnvfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexEnvivEXT] = MonitoredFunctionData(L"glGetMultiTexEnvivEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexGendvEXT] = MonitoredFunctionData(L"glGetMultiTexGendvEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexGenfvEXT] = MonitoredFunctionData(L"glGetMultiTexGenfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexGenivEXT] = MonitoredFunctionData(L"glGetMultiTexGenivEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetFloatIndexedvEXT] = MonitoredFunctionData(L"glGetFloatIndexedvEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetDoubleIndexedvEXT] = MonitoredFunctionData(L"glGetDoubleIndexedvEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetPointerIndexedvEXT] = MonitoredFunctionData(L"glGetPointerIndexedvEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);

    _monitoredFunctionsData[ap_glCompressedTextureImage3DEXT] = MonitoredFunctionData(L"glCompressedTextureImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTextureImage2DEXT] = MonitoredFunctionData(L"glCompressedTextureImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTextureImage1DEXT] = MonitoredFunctionData(L"glCompressedTextureImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTextureSubImage3DEXT] = MonitoredFunctionData(L"glCompressedTextureSubImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTextureSubImage2DEXT] = MonitoredFunctionData(L"glCompressedTextureSubImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedTextureSubImage1DEXT] = MonitoredFunctionData(L"glCompressedTextureSubImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetCompressedTextureImageEXT] = MonitoredFunctionData(L"glGetCompressedTextureImageEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glCompressedMultiTexImage3DEXT] = MonitoredFunctionData(L"glCompressedMultiTexImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedMultiTexImage2DEXT] = MonitoredFunctionData(L"glCompressedMultiTexImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedMultiTexImage1DEXT] = MonitoredFunctionData(L"glCompressedMultiTexImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedMultiTexSubImage3DEXT] = MonitoredFunctionData(L"glCompressedMultiTexSubImage3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedMultiTexSubImage2DEXT] = MonitoredFunctionData(L"glCompressedMultiTexSubImage2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glCompressedMultiTexSubImage1DEXT] = MonitoredFunctionData(L"glCompressedMultiTexSubImage1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetCompressedMultiTexImageEXT] = MonitoredFunctionData(L"glGetCompressedMultiTexImageEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_GET_FUNC);

    _monitoredFunctionsData[ap_glNamedProgramStringEXT] = MonitoredFunctionData(L"glNamedProgramStringEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glNamedProgramLocalParameter4dEXT] = MonitoredFunctionData(L"glNamedProgramLocalParameter4dEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glNamedProgramLocalParameter4dvEXT] = MonitoredFunctionData(L"glNamedProgramLocalParameter4dvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glNamedProgramLocalParameter4fEXT] = MonitoredFunctionData(L"glNamedProgramLocalParameter4fEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glNamedProgramLocalParameter4fvEXT] = MonitoredFunctionData(L"glNamedProgramLocalParameter4fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetNamedProgramLocalParameterdvEXT] = MonitoredFunctionData(L"glGetNamedProgramLocalParameterdvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetNamedProgramLocalParameterfvEXT] = MonitoredFunctionData(L"glGetNamedProgramLocalParameterfvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetNamedProgramivEXT] = MonitoredFunctionData(L"glGetNamedProgramivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetNamedProgramStringEXT] = MonitoredFunctionData(L"glGetNamedProgramStringEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glNamedProgramLocalParameters4fvEXT] = MonitoredFunctionData(L"glNamedProgramLocalParameters4fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glNamedProgramLocalParameterI4iEXT] = MonitoredFunctionData(L"glNamedProgramLocalParameterI4iEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glNamedProgramLocalParameterI4ivEXT] = MonitoredFunctionData(L"glNamedProgramLocalParameterI4ivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glNamedProgramLocalParametersI4ivEXT] = MonitoredFunctionData(L"glNamedProgramLocalParametersI4ivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glNamedProgramLocalParameterI4uiEXT] = MonitoredFunctionData(L"glNamedProgramLocalParameterI4uiEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glNamedProgramLocalParameterI4uivEXT] = MonitoredFunctionData(L"glNamedProgramLocalParameterI4uivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glNamedProgramLocalParametersI4uivEXT] = MonitoredFunctionData(L"glNamedProgramLocalParametersI4uivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetNamedProgramLocalParameterIivEXT] = MonitoredFunctionData(L"glGetNamedProgramLocalParameterIivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetNamedProgramLocalParameterIuivEXT] = MonitoredFunctionData(L"glGetNamedProgramLocalParameterIuivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);

    _monitoredFunctionsData[ap_glTextureParameterIivEXT] = MonitoredFunctionData(L"glTextureParameterIivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTextureParameterIuivEXT] = MonitoredFunctionData(L"glTextureParameterIuivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTextureParameterIivEXT] = MonitoredFunctionData(L"glGetTextureParameterIivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetTextureParameterIuivEXT] = MonitoredFunctionData(L"glGetTextureParameterIuivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glMultiTexParameterIivEXT] = MonitoredFunctionData(L"glMultiTexParameterIivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexParameterIuivEXT] = MonitoredFunctionData(L"glMultiTexParameterIuivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexParameterIivEXT] = MonitoredFunctionData(L"glGetMultiTexParameterIivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetMultiTexParameterIuivEXT] = MonitoredFunctionData(L"glGetMultiTexParameterIuivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_GET_FUNC);

    _monitoredFunctionsData[ap_glProgramUniform1fEXT] = MonitoredFunctionData(L"glProgramUniform1fEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2fEXT] = MonitoredFunctionData(L"glProgramUniform2fEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3fEXT] = MonitoredFunctionData(L"glProgramUniform3fEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4fEXT] = MonitoredFunctionData(L"glProgramUniform4fEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1iEXT] = MonitoredFunctionData(L"glProgramUniform1iEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2iEXT] = MonitoredFunctionData(L"glProgramUniform2iEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3iEXT] = MonitoredFunctionData(L"glProgramUniform3iEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4iEXT] = MonitoredFunctionData(L"glProgramUniform4iEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1fvEXT] = MonitoredFunctionData(L"glProgramUniform1fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2fvEXT] = MonitoredFunctionData(L"glProgramUniform2fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3fvEXT] = MonitoredFunctionData(L"glProgramUniform3fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4fvEXT] = MonitoredFunctionData(L"glProgramUniform4fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1ivEXT] = MonitoredFunctionData(L"glProgramUniform1ivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2ivEXT] = MonitoredFunctionData(L"glProgramUniform2ivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3ivEXT] = MonitoredFunctionData(L"glProgramUniform3ivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4ivEXT] = MonitoredFunctionData(L"glProgramUniform4ivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);

    _monitoredFunctionsData[ap_glProgramUniformMatrix2fvEXT] = MonitoredFunctionData(L"glProgramUniformMatrix2fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix3fvEXT] = MonitoredFunctionData(L"glProgramUniformMatrix3fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix4fvEXT] = MonitoredFunctionData(L"glProgramUniformMatrix4fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix2x3fvEXT] = MonitoredFunctionData(L"glProgramUniformMatrix2x3fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix3x2fvEXT] = MonitoredFunctionData(L"glProgramUniformMatrix3x2fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix2x4fvEXT] = MonitoredFunctionData(L"glProgramUniformMatrix2x4fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix4x2fvEXT] = MonitoredFunctionData(L"glProgramUniformMatrix4x2fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix3x4fvEXT] = MonitoredFunctionData(L"glProgramUniformMatrix3x4fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformMatrix4x3fvEXT] = MonitoredFunctionData(L"glProgramUniformMatrix4x3fvEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);

    _monitoredFunctionsData[ap_glProgramUniform1uiEXT] = MonitoredFunctionData(L"glProgramUniform1uiEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2uiEXT] = MonitoredFunctionData(L"glProgramUniform2uiEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3uiEXT] = MonitoredFunctionData(L"glProgramUniform3uiEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4uiEXT] = MonitoredFunctionData(L"glProgramUniform4uiEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform1uivEXT] = MonitoredFunctionData(L"glProgramUniform1uivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform2uivEXT] = MonitoredFunctionData(L"glProgramUniform2uivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform3uivEXT] = MonitoredFunctionData(L"glProgramUniform3uivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniform4uivEXT] = MonitoredFunctionData(L"glProgramUniform4uivEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);

    _monitoredFunctionsData[ap_glNamedBufferDataEXT] = MonitoredFunctionData(L"glNamedBufferDataEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedBufferSubDataEXT] = MonitoredFunctionData(L"glNamedBufferSubDataEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glMapNamedBufferEXT] = MonitoredFunctionData(L"glMapNamedBufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glUnmapNamedBufferEXT] = MonitoredFunctionData(L"glUnmapNamedBufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glMapNamedBufferRangeEXT] = MonitoredFunctionData(L"glMapNamedBufferRangeEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFlushMappedNamedBufferRangeEXT] = MonitoredFunctionData(L"glFlushMappedNamedBufferRangeEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedCopyBufferSubDataEXT] = MonitoredFunctionData(L"glNamedCopyBufferSubDataEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetNamedBufferParameterivEXT] = MonitoredFunctionData(L"glGetNamedBufferParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetNamedBufferPointervEXT] = MonitoredFunctionData(L"glGetNamedBufferPointervEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetNamedBufferSubDataEXT] = MonitoredFunctionData(L"glGetNamedBufferSubDataEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glTextureBufferEXT] = MonitoredFunctionData(L"glTextureBufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glMultiTexBufferEXT] = MonitoredFunctionData(L"glMultiTexBufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glNamedRenderbufferStorageEXT] = MonitoredFunctionData(L"glNamedRenderbufferStorageEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetNamedRenderbufferParameterivEXT] = MonitoredFunctionData(L"glGetNamedRenderbufferParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glCheckNamedFramebufferStatusEXT] = MonitoredFunctionData(L"glCheckNamedFramebufferStatusEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferTexture1DEXT] = MonitoredFunctionData(L"glNamedFramebufferTexture1DEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferTexture2DEXT] = MonitoredFunctionData(L"glNamedFramebufferTexture2DEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferTexture3DEXT] = MonitoredFunctionData(L"glNamedFramebufferTexture3DEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferRenderbufferEXT] = MonitoredFunctionData(L"glNamedFramebufferRenderbufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetNamedFramebufferAttachmentParameterivEXT] = MonitoredFunctionData(L"glGetNamedFramebufferAttachmentParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGenerateTextureMipmapEXT] = MonitoredFunctionData(L"glGenerateTextureMipmapEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGenerateMultiTexMipmapEXT] = MonitoredFunctionData(L"glGenerateMultiTexMipmapEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferDrawBufferEXT] = MonitoredFunctionData(L"glFramebufferDrawBufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferDrawBuffersEXT] = MonitoredFunctionData(L"glFramebufferDrawBuffersEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferReadBufferEXT] = MonitoredFunctionData(L"glFramebufferReadBufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetFramebufferParameterivEXT] = MonitoredFunctionData(L"glGetFramebufferParameterivEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glNamedRenderbufferStorageMultisampleEXT] = MonitoredFunctionData(L"glNamedRenderbufferStorageMultisampleEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedRenderbufferStorageMultisampleCoverageEXT] = MonitoredFunctionData(L"glNamedRenderbufferStorageMultisampleCoverageEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferTextureEXT] = MonitoredFunctionData(L"glNamedFramebufferTextureEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferTextureLayerEXT] = MonitoredFunctionData(L"glNamedFramebufferTextureLayerEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedFramebufferTextureFaceEXT] = MonitoredFunctionData(L"glNamedFramebufferTextureFaceEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glTextureRenderbufferEXT] = MonitoredFunctionData(L"glTextureRenderbufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glMultiTexRenderbufferEXT] = MonitoredFunctionData(L"glMultiTexRenderbufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_BUFFER_FUNC);

    // GL_EXT_bindable_uniform
    _monitoredFunctionsData[ap_glUniformBufferEXT] = MonitoredFunctionData(L"glUniformBufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_PROGRAM_SHADER_FUNC | AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glGetUniformBufferSizeEXT] = MonitoredFunctionData(L"glGetUniformBufferSizeEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetUniformOffsetEXT] = MonitoredFunctionData(L"glGetUniformOffsetEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);

    // GL_EXT_texture_integer
    _monitoredFunctionsData[ap_glTexParameterIivEXT] = MonitoredFunctionData(L"glTexParameterIivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexParameterIuivEXT] = MonitoredFunctionData(L"glTexParameterIuivEXT", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexParameterIivEXT] = MonitoredFunctionData(L"glGetTexParameterIivEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexParameterIuivEXT] = MonitoredFunctionData(L"glGetTexParameterIuivEXT", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glClearColorIiEXT] = MonitoredFunctionData(L"glClearColorIiEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glClearColorIuiEXT] = MonitoredFunctionData(L"glClearColorIuiEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_EXT_multi_draw_arrays
    _monitoredFunctionsData[ap_glMultiDrawArraysEXT] = MonitoredFunctionData(L"glMultiDrawArraysEXT", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glMultiDrawElementsEXT] = MonitoredFunctionData(L"glMultiDrawElementsEXT", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);

    // GL_ARB_multisample
    _monitoredFunctionsData[ap_glSampleCoverageARB] = MonitoredFunctionData(L"glSampleCoverageARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_EXT_blend_minmax
    _monitoredFunctionsData[ap_glBlendEquationEXT] = MonitoredFunctionData(L"glBlendEquationEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_EXT_geometry_shader4
    _monitoredFunctionsData[ap_glProgramParameteriEXT] = MonitoredFunctionData(L"glProgramParameteriEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTextureEXT] = MonitoredFunctionData(L"glFramebufferTextureEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTextureLayerEXT] = MonitoredFunctionData(L"glFramebufferTextureLayerEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTextureFaceEXT] = MonitoredFunctionData(L"glFramebufferTextureFaceEXT", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);

    // GL_NV_shader_buffer_load
    _monitoredFunctionsData[ap_glMakeBufferResidentNV] = MonitoredFunctionData(L"glMakeBufferResidentNV", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glMakeBufferNonResidentNV] = MonitoredFunctionData(L"glMakeBufferNonResidentNV", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glIsBufferResidentNV] = MonitoredFunctionData(L"glIsBufferResidentNV", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glMakeNamedBufferResidentNV] = MonitoredFunctionData(L"glMakeNamedBufferResidentNV", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glMakeNamedBufferNonResidentNV] = MonitoredFunctionData(L"glMakeNamedBufferNonResidentNV", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedMakeBufferResidentNV] = MonitoredFunctionData(L"glNamedMakeBufferResidentNV", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glNamedMakeBufferNonResidentNV] = MonitoredFunctionData(L"glNamedMakeBufferNonResidentNV", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glIsNamedBufferResidentNV] = MonitoredFunctionData(L"glIsNamedBufferResidentNV", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glGetBufferParameterui64vNV] = MonitoredFunctionData(L"glGetBufferParameterui64vNV", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetNamedBufferParameterui64vNV] = MonitoredFunctionData(L"glGetNamedBufferParameterui64vNV", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetIntegerui64vNV] = MonitoredFunctionData(L"glGetIntegerui64vNV", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glUniformui64NV] = MonitoredFunctionData(L"glUniformui64NV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glUniformui64vNV] = MonitoredFunctionData(L"glUniformui64vNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glGetUniformui64vNV] = MonitoredFunctionData(L"glGetUniformui64vNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformui64NV] = MonitoredFunctionData(L"glProgramUniformui64NV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glProgramUniformui64vNV] = MonitoredFunctionData(L"glProgramUniformui64vNV", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);

    //////////////////////////////////////////////////////////////////////////
    // GL_NV_vertex_buffer_unified_memory
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_glBufferAddressRangeNV] = MonitoredFunctionData(L"glBufferAddressRangeNV", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glVertexFormatNV] = MonitoredFunctionData(L"glVertexFormatNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glNormalFormatNV] = MonitoredFunctionData(L"glNormalFormatNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glColorFormatNV] = MonitoredFunctionData(L"glColorFormatNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glIndexFormatNV] = MonitoredFunctionData(L"glIndexFormatNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glTexCoordFormatNV] = MonitoredFunctionData(L"glTexCoordFormatNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glEdgeFlagFormatNV] = MonitoredFunctionData(L"glEdgeFlagFormatNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glSecondaryColorFormatNV] = MonitoredFunctionData(L"glSecondaryColorFormatNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glFogCoordFormatNV] = MonitoredFunctionData(L"glFogCoordFormatNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribFormatNV] = MonitoredFunctionData(L"glVertexAttribFormatNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glVertexAttribIFormatNV] = MonitoredFunctionData(L"glVertexAttribIFormatNV", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glGetIntegerui64i_vNV] = MonitoredFunctionData(L"glGetIntegerui64i_vNV", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);

    //////////////////////////////////////////////////////////////////////////
    // GL_AMD_debug_output
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_glDebugMessageEnableAMD] = MonitoredFunctionData(L"glDebugMessageEnableAMD", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glDebugMessageInsertAMD] = MonitoredFunctionData(L"glDebugMessageInsertAMD", AP_OPENGL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glDebugMessageCallbackAMD] = MonitoredFunctionData(L"glDebugMessageCallbackAMD", AP_OPENGL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glGetDebugMessageLogAMD] = MonitoredFunctionData(L"glGetDebugMessageLogAMD", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_DEBUG_FUNC);

    //////////////////////////////////////////////////////////////////////////
    // GL_AMDX_debug_output
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_glDebugMessageEnableAMDX] = MonitoredFunctionData(L"glDebugMessageEnableAMDX", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glDebugMessageInsertAMDX] = MonitoredFunctionData(L"glDebugMessageInsertAMDX", AP_OPENGL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glDebugMessageCallbackAMDX] = MonitoredFunctionData(L"glDebugMessageCallbackAMDX", AP_OPENGL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glGetDebugMessageLogAMDX] = MonitoredFunctionData(L"glGetDebugMessageLogAMDX", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_DEBUG_FUNC);

    //////////////////////////////////////////////////////////////////////////
    // GL_ARB_debug_output
    //////////////////////////////////////////////////////////////////////////
    _monitoredFunctionsData[ap_glDebugMessageControlARB] = MonitoredFunctionData(L"glDebugMessageControlARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glDebugMessageInsertARB] = MonitoredFunctionData(L"glDebugMessageInsertARB", AP_OPENGL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glDebugMessageCallbackARB] = MonitoredFunctionData(L"glDebugMessageCallbackARB", AP_OPENGL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_glGetDebugMessageLogARB] = MonitoredFunctionData(L"glGetDebugMessageLogARB", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_DEBUG_FUNC);

    // GL_EXT_draw_instanced
    _monitoredFunctionsData[ap_glDrawArraysInstancedEXT] = MonitoredFunctionData(L"glDrawArraysInstancedEXT", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glDrawElementsInstancedEXT] = MonitoredFunctionData(L"glDrawElementsInstancedEXT", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);

    // GL_EXT_texture_buffer_object
    _monitoredFunctionsData[ap_glTexBufferEXT] = MonitoredFunctionData(L"glTexBufferEXT", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_TEXTURE_FUNC);

    // GL_ARB_draw_instanced
    _monitoredFunctionsData[ap_glDrawArraysInstancedARB] = MonitoredFunctionData(L"glDrawArraysInstancedARB", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glDrawElementsInstancedARB] = MonitoredFunctionData(L"glDrawElementsInstancedARB", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);

    // GL_EXT_compiled_vertex_array
    _monitoredFunctionsData[ap_glLockArraysEXT] = MonitoredFunctionData(L"glLockArraysEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glUnlockArraysEXT] = MonitoredFunctionData(L"glUnlockArraysEXT", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_ARB_transpose_matrix
    _monitoredFunctionsData[ap_glLoadTransposeMatrixfARB] = MonitoredFunctionData(L"glLoadTransposeMatrixfARB", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glLoadTransposeMatrixdARB] = MonitoredFunctionData(L"glLoadTransposeMatrixdARB", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMultTransposeMatrixfARB] = MonitoredFunctionData(L"glMultTransposeMatrixfARB", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMultTransposeMatrixdARB] = MonitoredFunctionData(L"glMultTransposeMatrixdARB", AP_OPENGL_EXTENSION_FUNC, AP_MATRIX_FUNC);

    // GL_ARB_point_parameters
    _monitoredFunctionsData[ap_glPointParameterfARB] = MonitoredFunctionData(L"glPointParameterfARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glPointParameterfvARB] = MonitoredFunctionData(L"glPointParameterfvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_ARB_matrix_palette
    _monitoredFunctionsData[ap_glCurrentPaletteMatrixARB] = MonitoredFunctionData(L"glCurrentPaletteMatrixARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixIndexubvARB] = MonitoredFunctionData(L"glMatrixIndexubvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixIndexusvARB] = MonitoredFunctionData(L"glMatrixIndexusvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixIndexuivARB] = MonitoredFunctionData(L"glMatrixIndexuivARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_MATRIX_FUNC);
    _monitoredFunctionsData[ap_glMatrixIndexPointerARB] = MonitoredFunctionData(L"glMatrixIndexPointerARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_MATRIX_FUNC);

    // GL_ARB_window_pos
    _monitoredFunctionsData[ap_glWindowPos2dARB] = MonitoredFunctionData(L"glWindowPos2dARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos2fARB] = MonitoredFunctionData(L"glWindowPos2fARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos2iARB] = MonitoredFunctionData(L"glWindowPos2iARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos2sARB] = MonitoredFunctionData(L"glWindowPos2sARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos2dvARB] = MonitoredFunctionData(L"glWindowPos2dvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos2fvARB] = MonitoredFunctionData(L"glWindowPos2fvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos2ivARB] = MonitoredFunctionData(L"glWindowPos2ivARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos2svARB] = MonitoredFunctionData(L"glWindowPos2svARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos3dARB] = MonitoredFunctionData(L"glWindowPos3dARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos3fARB] = MonitoredFunctionData(L"glWindowPos3fARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos3iARB] = MonitoredFunctionData(L"glWindowPos3iARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos3sARB] = MonitoredFunctionData(L"glWindowPos3sARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos3dvARB] = MonitoredFunctionData(L"glWindowPos3dvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos3fvARB] = MonitoredFunctionData(L"glWindowPos3fvARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos3ivARB] = MonitoredFunctionData(L"glWindowPos3ivARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glWindowPos3svARB] = MonitoredFunctionData(L"glWindowPos3svARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_ARB_fragment_program_shadow
    // No new functions

    // GL_ARB_half_float_pixel
    // No new functions

    // GL_ARB_texture_float
    // No new functions

    // GL_ARB_pixel_buffer_object
    // No new functions

    // GL_ARB_depth_buffer_float
    // No new functions

    // GL_ARB_framebuffer_sRGB
    // No new functions

    // GL_ARB_geometry_shader4
    _monitoredFunctionsData[ap_glProgramParameteriARB] = MonitoredFunctionData(L"glProgramParameteriARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTextureARB] = MonitoredFunctionData(L"glFramebufferTextureARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTextureLayerARB] = MonitoredFunctionData(L"glFramebufferTextureLayerARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glFramebufferTextureFaceARB] = MonitoredFunctionData(L"glFramebufferTextureFaceARB", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);

    // GL_ARB_half_float_vertex
    // No new functions

    // GL_ARB_instanced_arrays
    _monitoredFunctionsData[ap_glVertexAttribDivisorARB] = MonitoredFunctionData(L"glVertexAttribDivisorARB", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_ARB_map_buffer_range
    _monitoredFunctionsData[ap_glMapBufferRange] = MonitoredFunctionData(L"glMapBufferRange", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFlushMappedBufferRange] = MonitoredFunctionData(L"glFlushMappedBufferRange", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_SYNCHRONIZATION_FUNC);

    // GL_ARB_texture_buffer_object
    _monitoredFunctionsData[ap_glTexBufferARB] = MonitoredFunctionData(L"glTexBufferARB", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC | AP_TEXTURE_FUNC);

    // GL_ARB_texture_compression_rgtc
    // No new functions

    // GL_ARB_texture_rg
    // No new functions

    // GL_ARB_vertex_array_object
    _monitoredFunctionsData[ap_glBindVertexArray] = MonitoredFunctionData(L"glBindVertexArray", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glDeleteVertexArrays] = MonitoredFunctionData(L"glDeleteVertexArrays", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glGenVertexArrays] = MonitoredFunctionData(L"glGenVertexArrays", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glIsVertexArray] = MonitoredFunctionData(L"glIsVertexArray", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_VERTEX_ARRAY_FUNC);

    // GL_ARB_uniform_buffer_object
    _monitoredFunctionsData[ap_glGetUniformIndices] = MonitoredFunctionData(L"glGetUniformIndices", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetActiveUniformsiv] = MonitoredFunctionData(L"glGetActiveUniformsiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetActiveUniformName] = MonitoredFunctionData(L"glGetActiveUniformName", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetUniformBlockIndex] = MonitoredFunctionData(L"glGetUniformBlockIndex", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetActiveUniformBlockiv] = MonitoredFunctionData(L"glGetActiveUniformBlockiv", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetActiveUniformBlockName] = MonitoredFunctionData(L"glGetActiveUniformBlockName", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glUniformBlockBinding] = MonitoredFunctionData(L"glUniformBlockBinding", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);

    // GL_ARB_copy_buffer
    _monitoredFunctionsData[ap_glCopyBufferSubData] = MonitoredFunctionData(L"glCopyBufferSubData", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);

    // GL_ARB_draw_elements_base_vertex
    _monitoredFunctionsData[ap_glDrawElementsBaseVertex] = MonitoredFunctionData(L"glDrawElementsBaseVertex", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glDrawRangeElementsBaseVertex] = MonitoredFunctionData(L"glDrawRangeElementsBaseVertex", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glDrawElementsInstancedBaseVertex] = MonitoredFunctionData(L"glDrawElementsInstancedBaseVertex", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glMultiDrawElementsBaseVertex] = MonitoredFunctionData(L"glMultiDrawElementsBaseVertex", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);

    // GL_ARB_provoking_vertex
    _monitoredFunctionsData[ap_glProvokingVertex] = MonitoredFunctionData(L"glProvokingVertex", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);

    // GL_ARB_sync
    // TO_DO: OpenGL3.2
    _monitoredFunctionsData[ap_glFenceSync] = MonitoredFunctionData(L"glFenceSync", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glIsSync] = MonitoredFunctionData(L"glIsSync", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glDeleteSync] = MonitoredFunctionData(L"glDeleteSync", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glClientWaitSync] = MonitoredFunctionData(L"glClientWaitSync", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glWaitSync] = MonitoredFunctionData(L"glWaitSync", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glGetInteger64v] = MonitoredFunctionData(L"glGetInteger64v", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_glGetSynciv] = MonitoredFunctionData(L"glGetSynciv", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_SYNCHRONIZATION_FUNC);

    // GL_ARB_texture_multisample
    _monitoredFunctionsData[ap_glTexImage2DMultisample] = MonitoredFunctionData(L"glTexImage2DMultisample", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glTexImage3DMultisample] = MonitoredFunctionData(L"glTexImage3DMultisample", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetMultisamplefv] = MonitoredFunctionData(L"glGetMultisamplefv", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC | AP_GET_FUNC);
    _monitoredFunctionsData[ap_glSampleMaski] = MonitoredFunctionData(L"glSampleMaski", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);

    // GL_ARB_cl_event
    _monitoredFunctionsData[ap_glCreateSyncFromCLeventARB] = MonitoredFunctionData(L"glCreateSyncFromCLeventARB", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);

    // GL_APPLE_aux_depth_stencil
    // No new functions

    // GL_APPLE_client_storage
    // No new functions

    // AP_GL_APPLE_element_array
    _monitoredFunctionsData[ap_glElementPointerAPPLE] = MonitoredFunctionData(L"glElementPointerAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC);
    _monitoredFunctionsData[ap_glDrawElementArrayAPPLE] = MonitoredFunctionData(L"glDrawElementArrayAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glDrawRangeElementArrayAPPLE] = MonitoredFunctionData(L"glDrawRangeElementArrayAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glMultiDrawElementArrayAPPLE] = MonitoredFunctionData(L"glMultiDrawElementArrayAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);
    _monitoredFunctionsData[ap_glMultiDrawRangeElementArrayAPPLE] = MonitoredFunctionData(L"glMultiDrawRangeElementArrayAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_DRAW_FUNC);

    // GL_APPLE_fence
    _monitoredFunctionsData[ap_glGenFencesAPPLE] = MonitoredFunctionData(L"glGenFencesAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glDeleteFencesAPPLE] = MonitoredFunctionData(L"glDeleteFencesAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glSetFenceAPPLE] = MonitoredFunctionData(L"glSetFenceAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glIsFenceAPPLE] = MonitoredFunctionData(L"glIsFenceAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glTestFenceAPPLE] = MonitoredFunctionData(L"glTestFenceAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glFinishFenceAPPLE] = MonitoredFunctionData(L"glFinishFenceAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glTestObjectAPPLE] = MonitoredFunctionData(L"glTestObjectAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_glFinishObjectAPPLE] = MonitoredFunctionData(L"glFinishObjectAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);

    // AP_GL_APPLE_float_pixels
    // No new functions

    // GL_APPLE_flush_buffer_range
    _monitoredFunctionsData[ap_glBufferParameteriAPPLE] = MonitoredFunctionData(L"glBufferParameteriAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_glFlushMappedBufferRangeAPPLE] = MonitoredFunctionData(L"glFlushMappedBufferRangeAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);

    // GL_APPLE_flush_render
    // No new functions
    //TO_DO: search for this functions (spec is unavailable on the net)
    //_monitoredFunctionsData[ap_glFlushRenderAPPLE] = MonitoredFunctionData(L"glFlushRenderAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    //_monitoredFunctionsData[ap_glFinishRenderAPPLE] = MonitoredFunctionData(L"glFinishRenderAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);

    // GL_APPLE_object_purgeable
    _monitoredFunctionsData[ap_glObjectPurgeableAPPLE] = MonitoredFunctionData(L"glObjectPurgeableAPPLE", AP_OPENGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_glObjectUnpurgeableAPPLE] = MonitoredFunctionData(L"glObjectUnpurgeableAPPLE", AP_OPENGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_glGetObjectParameterivAPPLE] = MonitoredFunctionData(L"glGetObjectParameterivAPPLE", AP_OPENGL_EXTENSION_FUNC, 0);

    // GL_APPLE_packed_pixels
    // No new functions

    // GL_APPLE_pixel_buffer
    // No new functions
    //TO_DO: search for this functions (spec is unavailable on the net)

    // GL_APPLE_specular_vector
    // No new functions

    // GL_APPLE_texture_range
    _monitoredFunctionsData[ap_glTextureRangeAPPLE] = MonitoredFunctionData(L"glTextureRangeAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_glGetTexParameterPointervAPPLE] = MonitoredFunctionData(L"glGetTexParameterPointervAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_TEXTURE_FUNC);

    // GL_APPLE_transform_hint
    // No new functions

    // GL_APPLE_vertex_array_object
    _monitoredFunctionsData[ap_glBindVertexArrayAPPLE] = MonitoredFunctionData(L"glBindVertexArrayAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glDeleteVertexArraysAPPLE] = MonitoredFunctionData(L"glDeleteVertexArraysAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glGenVertexArraysAPPLE] = MonitoredFunctionData(L"glGenVertexArraysAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glIsVertexArrayAPPLE] = MonitoredFunctionData(L"glIsVertexArrayAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);

    // GL_APPLE_vertex_array_range
    _monitoredFunctionsData[ap_glVertexArrayRangeAPPLE] = MonitoredFunctionData(L"glVertexArrayRangeAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glFlushVertexArrayRangeAPPLE] = MonitoredFunctionData(L"glFlushVertexArrayRangeAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC | AP_VERTEX_ARRAY_FUNC);
    _monitoredFunctionsData[ap_glVertexArrayParameteriAPPLE] = MonitoredFunctionData(L"glVertexArrayParameteriAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_VERTEX_ARRAY_FUNC);

    // GL_APPLE_vertex_program_evaluators
    _monitoredFunctionsData[ap_glEnableVertexAttribAPPLE] = MonitoredFunctionData(L"glEnableVertexAttribAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glDisableVertexAttribAPPLE] = MonitoredFunctionData(L"glDisableVertexAttribAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_STATE_CHANGE_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glIsVertexAttribEnabledAPPLE] = MonitoredFunctionData(L"glIsVertexAttribEnabledAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glMapVertexAttrib1dAPPLE] = MonitoredFunctionData(L"glMapVertexAttrib1dAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glMapVertexAttrib1fAPPLE] = MonitoredFunctionData(L"glMapVertexAttrib1fAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glMapVertexAttrib2dAPPLE] = MonitoredFunctionData(L"glMapVertexAttrib2dAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);
    _monitoredFunctionsData[ap_glMapVertexAttrib2fAPPLE] = MonitoredFunctionData(L"glMapVertexAttrib2fAPPLE", AP_OPENGL_EXTENSION_FUNC, AP_PROGRAM_SHADER_FUNC);

    // GL_APPLE_ycbcr_422
    // No new functions


    // Add new extensions here:

    //////////////////////////////////////////////////////////////////////////
    // WGL Extensions
    //////////////////////////////////////////////////////////////////////////

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    _monitoredFunctionsData[ap_wglGetExtensionsStringARB] = MonitoredFunctionData(L"wglGetExtensionsStringARB", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);

    // WGL_I3D_genlock Extension
    _monitoredFunctionsData[ap_wglEnableGenlockI3D] = MonitoredFunctionData(L"wglEnableGenlockI3D", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglDisableGenlockI3D] = MonitoredFunctionData(L"wglDisableGenlockI3D", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglIsEnabledGenlockI3D] = MonitoredFunctionData(L"wglIsEnabledGenlockI3D", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglGenlockSourceI3D] = MonitoredFunctionData(L"wglGenlockSourceI3D", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglGetGenlockSourceI3D] = MonitoredFunctionData(L"wglGetGenlockSourceI3D", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglGenlockSourceEdgeI3D] = MonitoredFunctionData(L"wglGenlockSourceEdgeI3D", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglGetGenlockSourceEdgeI3D] = MonitoredFunctionData(L"wglGetGenlockSourceEdgeI3D", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglGenlockSampleRateI3D] = MonitoredFunctionData(L"wglGenlockSampleRateI3D", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglGetGenlockSampleRateI3D] = MonitoredFunctionData(L"wglGetGenlockSampleRateI3D", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglGenlockSourceDelayI3D] = MonitoredFunctionData(L"wglGenlockSourceDelayI3D", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglGetGenlockSourceDelayI3D] = MonitoredFunctionData(L"wglGetGenlockSourceDelayI3D", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglQueryGenlockMaxSourceDelayI3D] = MonitoredFunctionData(L"wglQueryGenlockMaxSourceDelayI3D", AP_WGL_EXTENSION_FUNC, 0);

    // WGL_ARB_pbuffer
    _monitoredFunctionsData[ap_wglCreatePbufferARB] = MonitoredFunctionData(L"wglCreatePbufferARB", AP_WGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_wglGetPbufferDCARB] = MonitoredFunctionData(L"wglGetPbufferDCARB", AP_WGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_wglReleasePbufferDCARB] = MonitoredFunctionData(L"wglReleasePbufferDCARB", AP_WGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_wglDestroyPbufferARB] = MonitoredFunctionData(L"wglDestroyPbufferARB", AP_WGL_EXTENSION_FUNC, AP_BUFFER_FUNC);
    _monitoredFunctionsData[ap_wglQueryPbufferARB] = MonitoredFunctionData(L"wglQueryPbufferARB", AP_WGL_EXTENSION_FUNC, AP_BUFFER_FUNC);

    // WGL_ARB_pixel_format
    _monitoredFunctionsData[ap_wglGetPixelFormatAttribivARB] = MonitoredFunctionData(L"wglGetPixelFormatAttribivARB", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglGetPixelFormatAttribfvARB] = MonitoredFunctionData(L"wglGetPixelFormatAttribfvARB", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglChoosePixelFormatARB] = MonitoredFunctionData(L"wglChoosePixelFormatARB", AP_WGL_EXTENSION_FUNC, 0);

    // WGL_ARB_make_current_read
    _monitoredFunctionsData[ap_wglMakeContextCurrentARB] = MonitoredFunctionData(L"wglMakeContextCurrentARB", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglGetCurrentReadDCARB] = MonitoredFunctionData(L"wglGetCurrentReadDCARB", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);

    // WGL_ARB_render_texture
    _monitoredFunctionsData[ap_wglBindTexImageARB] = MonitoredFunctionData(L"wglBindTexImageARB", AP_WGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_wglReleaseTexImageARB] = MonitoredFunctionData(L"wglReleaseTexImageARB", AP_WGL_EXTENSION_FUNC, AP_TEXTURE_FUNC);
    _monitoredFunctionsData[ap_wglSetPbufferAttribARB] = MonitoredFunctionData(L"wglSetPbufferAttribARB", AP_WGL_EXTENSION_FUNC, AP_BUFFER_FUNC);

    // WGL_ARB_buffer_region
    _monitoredFunctionsData[ap_wglCreateBufferRegionARB] = MonitoredFunctionData(L"wglCreateBufferRegionARB", AP_WGL_EXTENSION_FUNC, AP_RASTER_FUNC);
    _monitoredFunctionsData[ap_wglDeleteBufferRegionARB] = MonitoredFunctionData(L"wglDeleteBufferRegionARB", AP_WGL_EXTENSION_FUNC, AP_RASTER_FUNC);
    _monitoredFunctionsData[ap_wglSaveBufferRegionARB] = MonitoredFunctionData(L"wglSaveBufferRegionARB", AP_WGL_EXTENSION_FUNC, AP_RASTER_FUNC);
    _monitoredFunctionsData[ap_wglRestoreBufferRegionARB] = MonitoredFunctionData(L"wglRestoreBufferRegionARB", AP_WGL_EXTENSION_FUNC, AP_RASTER_FUNC);

    // WGL_ARB_multisample
    // See GL_ARB_multisample

    // WGL_ARB_pixel_format_float
    // See GL_ARB_color_buffer_float

    // WGL_ARB_framebuffer_sRGB
    // No new functions

    // WGL_ARB_create_context
    _monitoredFunctionsData[ap_wglCreateContextAttribsARB] = MonitoredFunctionData(L"wglCreateContextAttribsARB", AP_WGL_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);

    // WGL_ARB_create_context
    _monitoredFunctionsData[ap_wglSwapIntervalEXT] = MonitoredFunctionData(L"wglSwapIntervalEXT", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglGetSwapIntervalEXT] = MonitoredFunctionData(L"wglGetSwapIntervalEXT", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);

    // WGL_NV_present_video
    _monitoredFunctionsData[ap_wglEnumerateVideoDevicesNV] = MonitoredFunctionData(L"wglEnumerateVideoDevicesNV", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglBindVideoDeviceNV] = MonitoredFunctionData(L"wglBindVideoDeviceNV", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglQueryCurrentContextNV] = MonitoredFunctionData(L"wglQueryCurrentContextNV", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);

    // WGL_NV_video_out
    _monitoredFunctionsData[ap_wglGetVideoDeviceNV] = MonitoredFunctionData(L"wglGetVideoDeviceNV", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglReleaseVideoDeviceNV] = MonitoredFunctionData(L"wglReleaseVideoDeviceNV", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglBindVideoImageNV] = MonitoredFunctionData(L"wglBindVideoImageNV", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglReleaseVideoImageNV] = MonitoredFunctionData(L"wglReleaseVideoImageNV", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglSendPbufferToVideoNV] = MonitoredFunctionData(L"wglSendPbufferToVideoNV", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglGetVideoInfoNV] = MonitoredFunctionData(L"wglGetVideoInfoNV", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);

    // WGL_NV_swap_group
    _monitoredFunctionsData[ap_wglJoinSwapGroupNV] = MonitoredFunctionData(L"wglJoinSwapGroupNV", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglBindSwapBarrierNV] = MonitoredFunctionData(L"wglBindSwapBarrierNV", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglQuerySwapGroupNV] = MonitoredFunctionData(L"wglQuerySwapGroupNV", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglQueryMaxSwapGroupsNV] = MonitoredFunctionData(L"wglQueryMaxSwapGroupsNV", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglQueryFrameCountNV] = MonitoredFunctionData(L"wglQueryFrameCountNV", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglResetFrameCountNV] = MonitoredFunctionData(L"wglResetFrameCountNV", AP_WGL_EXTENSION_FUNC, 0);

    // WGL_NV_gpu_affinity
    _monitoredFunctionsData[ap_wglEnumGpusNV] = MonitoredFunctionData(L"wglEnumGpusNV", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglEnumGpuDevicesNV] = MonitoredFunctionData(L"wglEnumGpuDevicesNV", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglCreateAffinityDCNV] = MonitoredFunctionData(L"wglCreateAffinityDCNV", AP_WGL_EXTENSION_FUNC, 0);
    _monitoredFunctionsData[ap_wglEnumGpusFromAffinityDCNV] = MonitoredFunctionData(L"wglEnumGpusFromAffinityDCNV", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglDeleteDCNV] = MonitoredFunctionData(L"wglDeleteDCNV", AP_WGL_EXTENSION_FUNC, 0);

    // WGL_AMD_gpu_association
    _monitoredFunctionsData[ap_wglGetGPUIDsAMD] = MonitoredFunctionData(L"wglGetGPUIDsAMD", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglGetGPUInfoAMD] = MonitoredFunctionData(L"wglGetGPUInfoAMD", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglGetContextGPUIDAMD] = MonitoredFunctionData(L"wglGetContextGPUIDAMD", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_wglCreateAssociatedContextAMD] = MonitoredFunctionData(L"wglCreateAssociatedContextAMD", AP_WGL_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglCreateAssociatedContextAttribsAMD] = MonitoredFunctionData(L"wglCreateAssociatedContextAttribsAMD", AP_WGL_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglDeleteAssociatedContextAMD] = MonitoredFunctionData(L"wglDeleteAssociatedContextAMD", AP_WGL_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglMakeAssociatedContextCurrentAMD] = MonitoredFunctionData(L"wglMakeAssociatedContextCurrentAMD", AP_WGL_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglGetCurrentAssociatedContextAMD] = MonitoredFunctionData(L"wglGetCurrentAssociatedContextAMD", AP_WGL_EXTENSION_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_wglBlitContextFramebufferAMD] = MonitoredFunctionData(L"wglBlitContextFramebufferAMD", AP_WGL_EXTENSION_FUNC, AP_BUFFER_FUNC);

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    //////////////////////////////////////////////////////////////////////////
    // Graphic Remedy Extensions:
    //////////////////////////////////////////////////////////////////////////

    // GL_GREMEDY_string_marker
    _monitoredFunctionsData[ap_glStringMarkerGREMEDY] = MonitoredFunctionData(L"glStringMarkerGREMEDY", AP_OPENGL_GREMEDY_EXTENSION_FUNC | AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_EXTENSION_FUNC, AP_DEBUG_FUNC);

    // GL_GREMEDY_frame_terminator
    _monitoredFunctionsData[ap_glFrameTerminatorGREMEDY] = MonitoredFunctionData(L"glFrameTerminatorGREMEDY", AP_OPENGL_GREMEDY_EXTENSION_FUNC | AP_OPENGL_EXTENSION_FUNC | AP_OPENGL_ES_EXTENSION_FUNC, AP_DEBUG_FUNC);
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::initializeOpenCL10FunctionsData
// Description: Initialize OpenCL 1.0 functions data.
// Author:  AMD Developer Tools Team
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void apMonitoredFunctionsManager::initializeOpenCL10FunctionsData()
{
    _monitoredFunctionsData[ap_clGetPlatformIDs] = MonitoredFunctionData(L"clGetPlatformIDs", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clGetPlatformInfo] = MonitoredFunctionData(L"clGetPlatformInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clGetDeviceIDs] = MonitoredFunctionData(L"clGetDeviceIDs", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clGetDeviceInfo] = MonitoredFunctionData(L"clGetDeviceInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clCreateContext] = MonitoredFunctionData(L"clCreateContext", AP_OPENCL_GENERIC_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clCreateContextFromType] = MonitoredFunctionData(L"clCreateContextFromType", AP_OPENCL_GENERIC_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clRetainContext] = MonitoredFunctionData(L"clRetainContext", AP_OPENCL_GENERIC_FUNC, 0);
    _monitoredFunctionsData[ap_clReleaseContext] = MonitoredFunctionData(L"clReleaseContext", AP_OPENCL_GENERIC_FUNC, 0);
    _monitoredFunctionsData[ap_clGetContextInfo] = MonitoredFunctionData(L"clGetContextInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC);
    _monitoredFunctionsData[ap_clCreateCommandQueue] = MonitoredFunctionData(L"clCreateCommandQueue", AP_OPENCL_GENERIC_FUNC, AP_QUEUE_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_2_0, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clRetainCommandQueue] = MonitoredFunctionData(L"clRetainCommandQueue", AP_OPENCL_GENERIC_FUNC, AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clReleaseCommandQueue] = MonitoredFunctionData(L"clReleaseCommandQueue", AP_OPENCL_GENERIC_FUNC, AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clGetCommandQueueInfo] = MonitoredFunctionData(L"clGetCommandQueueInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clSetCommandQueueProperty] = MonitoredFunctionData(L"clSetCommandQueueProperty", AP_OPENCL_GENERIC_FUNC, AP_QUEUE_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_1_1, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clCreateBuffer] = MonitoredFunctionData(L"clCreateBuffer", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clCreateImage2D] = MonitoredFunctionData(L"clCreateImage2D", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_1_2, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clCreateImage3D] = MonitoredFunctionData(L"clCreateImage3D", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_1_2, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clRetainMemObject] = MonitoredFunctionData(L"clRetainMemObject", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clReleaseMemObject] = MonitoredFunctionData(L"clReleaseMemObject", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clGetSupportedImageFormats] = MonitoredFunctionData(L"clGetSupportedImageFormats", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clGetMemObjectInfo] = MonitoredFunctionData(L"clGetMemObjectInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clGetImageInfo] = MonitoredFunctionData(L"clGetImageInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clCreateSampler] = MonitoredFunctionData(L"clCreateSampler", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_2_0, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clRetainSampler] = MonitoredFunctionData(L"clRetainSampler", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clReleaseSampler] = MonitoredFunctionData(L"clReleaseSampler", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clGetSamplerInfo] = MonitoredFunctionData(L"clGetSamplerInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clCreateProgramWithSource] = MonitoredFunctionData(L"clCreateProgramWithSource", AP_OPENCL_GENERIC_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clCreateProgramWithBinary] = MonitoredFunctionData(L"clCreateProgramWithBinary", AP_OPENCL_GENERIC_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clRetainProgram] = MonitoredFunctionData(L"clRetainProgram", AP_OPENCL_GENERIC_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clReleaseProgram] = MonitoredFunctionData(L"clReleaseProgram", AP_OPENCL_GENERIC_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clBuildProgram] = MonitoredFunctionData(L"clBuildProgram", AP_OPENCL_GENERIC_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clUnloadCompiler] = MonitoredFunctionData(L"clUnloadCompiler", AP_OPENCL_GENERIC_FUNC, AP_NULL_CONTEXT_FUNCTION | AP_PROGRAM_KERNEL_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_1_2, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clGetProgramInfo] = MonitoredFunctionData(L"clGetProgramInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clGetProgramBuildInfo] = MonitoredFunctionData(L"clGetProgramBuildInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clCreateKernel] = MonitoredFunctionData(L"clCreateKernel", AP_OPENCL_GENERIC_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clCreateKernelsInProgram] = MonitoredFunctionData(L"clCreateKernelsInProgram", AP_OPENCL_GENERIC_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clRetainKernel] = MonitoredFunctionData(L"clRetainKernel", AP_OPENCL_GENERIC_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clReleaseKernel] = MonitoredFunctionData(L"clReleaseKernel", AP_OPENCL_GENERIC_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clSetKernelArg] = MonitoredFunctionData(L"clSetKernelArg", AP_OPENCL_GENERIC_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clGetKernelInfo] = MonitoredFunctionData(L"clGetKernelInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clGetKernelWorkGroupInfo] = MonitoredFunctionData(L"clGetKernelWorkGroupInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clWaitForEvents] = MonitoredFunctionData(L"clWaitForEvents", AP_OPENCL_GENERIC_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clGetEventInfo] = MonitoredFunctionData(L"clGetEventInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clRetainEvent] = MonitoredFunctionData(L"clRetainEvent", AP_OPENCL_GENERIC_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clReleaseEvent] = MonitoredFunctionData(L"clReleaseEvent", AP_OPENCL_GENERIC_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clGetEventProfilingInfo] = MonitoredFunctionData(L"clGetEventProfilingInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clFlush] = MonitoredFunctionData(L"clFlush", AP_OPENCL_GENERIC_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clFinish] = MonitoredFunctionData(L"clFinish", AP_OPENCL_GENERIC_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clEnqueueReadBuffer] = MonitoredFunctionData(L"clEnqueueReadBuffer", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueWriteBuffer] = MonitoredFunctionData(L"clEnqueueWriteBuffer", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueCopyBuffer] = MonitoredFunctionData(L"clEnqueueCopyBuffer", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueReadImage] = MonitoredFunctionData(L"clEnqueueReadImage", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueWriteImage] = MonitoredFunctionData(L"clEnqueueWriteImage", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueCopyImage] = MonitoredFunctionData(L"clEnqueueCopyImage", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueCopyImageToBuffer] = MonitoredFunctionData(L"clEnqueueCopyImageToBuffer", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueCopyBufferToImage] = MonitoredFunctionData(L"clEnqueueCopyBufferToImage", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueMapBuffer] = MonitoredFunctionData(L"clEnqueueMapBuffer", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueMapImage] = MonitoredFunctionData(L"clEnqueueMapImage", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueUnmapMemObject] = MonitoredFunctionData(L"clEnqueueUnmapMemObject", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueNDRangeKernel] = MonitoredFunctionData(L"clEnqueueNDRangeKernel", AP_OPENCL_GENERIC_FUNC, AP_QUEUE_FUNC | AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clEnqueueTask] = MonitoredFunctionData(L"clEnqueueTask", AP_OPENCL_GENERIC_FUNC, AP_QUEUE_FUNC | AP_PROGRAM_KERNEL_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_2_0, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clEnqueueNativeKernel] = MonitoredFunctionData(L"clEnqueueNativeKernel", AP_OPENCL_GENERIC_FUNC, AP_QUEUE_FUNC | AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clEnqueueMarker] = MonitoredFunctionData(L"clEnqueueMarker", AP_OPENCL_GENERIC_FUNC, AP_QUEUE_FUNC | AP_SYNCHRONIZATION_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_1_2, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clEnqueueWaitForEvents] = MonitoredFunctionData(L"clEnqueueWaitForEvents", AP_OPENCL_GENERIC_FUNC, AP_QUEUE_FUNC | AP_SYNCHRONIZATION_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_1_2, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clEnqueueBarrier] = MonitoredFunctionData(L"clEnqueueBarrier", AP_OPENCL_GENERIC_FUNC, AP_QUEUE_FUNC | AP_SYNCHRONIZATION_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_1_2, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clGetExtensionFunctionAddress] = MonitoredFunctionData(L"clGetExtensionFunctionAddress", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION | AP_DEPRECATED_FUNC, AP_CL_VERSION_1_2, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clCreateFromGLBuffer] = MonitoredFunctionData(L"clCreateFromGLBuffer", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clCreateFromGLTexture2D] = MonitoredFunctionData(L"clCreateFromGLTexture2D", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_1_2, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clCreateFromGLTexture3D] = MonitoredFunctionData(L"clCreateFromGLTexture3D", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_DEPRECATED_FUNC, AP_CL_VERSION_1_2, AP_CL_VERSION_NONE);
    _monitoredFunctionsData[ap_clCreateFromGLRenderbuffer] = MonitoredFunctionData(L"clCreateFromGLRenderbuffer", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clGetGLObjectInfo] = MonitoredFunctionData(L"clGetGLObjectInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clGetGLTextureInfo] = MonitoredFunctionData(L"clGetGLTextureInfo", AP_OPENCL_GENERIC_FUNC, AP_GET_FUNC | AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueAcquireGLObjects] = MonitoredFunctionData(L"clEnqueueAcquireGLObjects", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueReleaseGLObjects] = MonitoredFunctionData(L"clEnqueueReleaseGLObjects", AP_OPENCL_GENERIC_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::initializeOpenCL11FunctionsData
// Description: Initialize OpenCL 1.1 functions data.
// Author:  AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
void apMonitoredFunctionsManager::initializeOpenCL11FunctionsData()
{
    _monitoredFunctionsData[ap_clCreateSubBuffer] = MonitoredFunctionData(L"clCreateSubBuffer", AP_OPENCL_EXTENSION_FUNC, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clSetMemObjectDestructorCallback] = MonitoredFunctionData(L"clSetMemObjectDestructorCallback", AP_OPENCL_EXTENSION_FUNC, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clCreateUserEvent] = MonitoredFunctionData(L"clCreateUserEvent", AP_OPENCL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clSetUserEventStatus] = MonitoredFunctionData(L"clSetUserEventStatus", AP_OPENCL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clSetEventCallback] = MonitoredFunctionData(L"clSetEventCallback", AP_OPENCL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clEnqueueReadBufferRect] = MonitoredFunctionData(L"clEnqueueReadBufferRect", AP_OPENCL_EXTENSION_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueWriteBufferRect] = MonitoredFunctionData(L"clEnqueueWriteBufferRect", AP_OPENCL_EXTENSION_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueCopyBufferRect] = MonitoredFunctionData(L"clEnqueueCopyBufferRect", AP_OPENCL_EXTENSION_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::initializeOpenCL12FunctionsData
// Description: Initialize OpenCL 1.2 functions data.
// Author:  AMD Developer Tools Team
// Date:        10/1/2012
// ---------------------------------------------------------------------------
void apMonitoredFunctionsManager::initializeOpenCL12FunctionsData()
{
    _monitoredFunctionsData[ap_clCreateSubDevices] = MonitoredFunctionData(L"clCreateSubDevices", AP_OPENCL_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clRetainDevice] = MonitoredFunctionData(L"clRetainDevice", AP_OPENCL_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clReleaseDevice] = MonitoredFunctionData(L"clReleaseDevice", AP_OPENCL_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clCreateImage] = MonitoredFunctionData(L"clCreateImage", AP_OPENCL_EXTENSION_FUNC, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clCreateProgramWithBuiltInKernels] = MonitoredFunctionData(L"clCreateProgramWithBuiltInKernels", AP_OPENCL_EXTENSION_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clCompileProgram] = MonitoredFunctionData(L"clCompileProgram", AP_OPENCL_EXTENSION_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clLinkProgram] = MonitoredFunctionData(L"clLinkProgram", AP_OPENCL_EXTENSION_FUNC, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clUnloadPlatformCompiler] = MonitoredFunctionData(L"clUnloadPlatformCompiler", AP_OPENCL_EXTENSION_FUNC, AP_NULL_CONTEXT_FUNCTION | AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clGetKernelArgInfo] = MonitoredFunctionData(L"clGetKernelArgInfo", AP_OPENCL_EXTENSION_FUNC, AP_GET_FUNC | AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clEnqueueFillBuffer] = MonitoredFunctionData(L"clEnqueueFillBuffer", AP_OPENCL_EXTENSION_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueFillImage] = MonitoredFunctionData(L"clEnqueueFillImage", AP_OPENCL_EXTENSION_FUNC, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueMarkerWithWaitList] = MonitoredFunctionData(L"clEnqueueMarkerWithWaitList", AP_OPENCL_EXTENSION_FUNC, AP_QUEUE_FUNC | AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clEnqueueMigrateMemObjects] = MonitoredFunctionData(L"clEnqueueMigrateMemObjects", AP_OPENCL_EXTENSION_FUNC, AP_QUEUE_FUNC | AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clEnqueueBarrierWithWaitList] = MonitoredFunctionData(L"clEnqueueBarrierWithWaitList", AP_OPENCL_EXTENSION_FUNC, AP_QUEUE_FUNC | AP_SYNCHRONIZATION_FUNC);
    _monitoredFunctionsData[ap_clGetExtensionFunctionAddressForPlatform] = MonitoredFunctionData(L"clGetExtensionFunctionAddressForPlatform", AP_OPENCL_EXTENSION_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clSetPrintfCallback] = MonitoredFunctionData(L"clSetPrintfCallback", AP_OPENCL_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_clCreateFromGLTexture] = MonitoredFunctionData(L"clCreateFromGLTexture", AP_OPENCL_EXTENSION_FUNC, AP_BUFFER_IMAGE_FUNC);
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::initializeOpenCL20FunctionsData
// Description: Initialize OpenCL 2.0 functions data.
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
void apMonitoredFunctionsManager::initializeOpenCL20FunctionsData()
{
    // Uri, 29/9/2014: We currently treat OpenCL 2.0 as extension functions, to avoid failing on the initialization if they're missing:
    apAPIType baseFuncType = AP_OPENCL_EXTENSION_FUNC;

    _monitoredFunctionsData[ap_clCreateCommandQueueWithProperties] = MonitoredFunctionData(L"clCreateCommandQueueWithProperties", baseFuncType, AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clCreatePipe] = MonitoredFunctionData(L"clCreatePipe", baseFuncType, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clGetPipeInfo] = MonitoredFunctionData(L"clGetPipeInfo", baseFuncType, AP_GET_FUNC | AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clSVMAlloc] = MonitoredFunctionData(L"clSVMAlloc", baseFuncType, 0);
    _monitoredFunctionsData[ap_clSVMFree] = MonitoredFunctionData(L"clSVMFree", baseFuncType, 0);
    _monitoredFunctionsData[ap_clCreateSamplerWithProperties] = MonitoredFunctionData(L"clCreateSamplerWithProperties", baseFuncType, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clSetKernelArgSVMPointer] = MonitoredFunctionData(L"clSetKernelArgSVMPointer", baseFuncType, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clSetKernelExecInfo] = MonitoredFunctionData(L"clSetKernelExecInfo", baseFuncType, AP_PROGRAM_KERNEL_FUNC);
    _monitoredFunctionsData[ap_clEnqueueSVMFree] = MonitoredFunctionData(L"clEnqueueSVMFree", baseFuncType, AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueSVMMemcpy] = MonitoredFunctionData(L"clEnqueueSVMMemcpy", baseFuncType, AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueSVMMemFill] = MonitoredFunctionData(L"clEnqueueSVMMemFill", baseFuncType, AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueSVMMap] = MonitoredFunctionData(L"clEnqueueSVMMap", baseFuncType, AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueSVMUnmap] = MonitoredFunctionData(L"clEnqueueSVMUnmap", baseFuncType, AP_QUEUE_FUNC);
}


// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::initializeOpenCLExtensionFunctionsData
// Description: Initialize the OpenCL extension functions data:
// Author:  AMD Developer Tools Team
// Date:        18/2/2010
// ---------------------------------------------------------------------------
void apMonitoredFunctionsManager::initializeOpenCLExtensionFunctionsData()
{
    // cl_khr_gl_sharing
    _monitoredFunctionsData[ap_clGetGLContextInfoKHR] = MonitoredFunctionData(L"clGetGLContextInfoKHR", AP_OPENCL_EXTENSION_FUNC, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);

    // cl_khr_gl_event:
    _monitoredFunctionsData[ap_clCreateEventFromGLsyncKHR] = MonitoredFunctionData(L"clCreateEventFromGLsyncKHR", AP_OPENCL_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC);

    apAPIType baseDXFuncType = (apAPIType)0;
    // DirectX integration supported only on Windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    baseDXFuncType = AP_OPENCL_EXTENSION_FUNC;
#endif

    // cl_khr_dx9_media_sharing:
    _monitoredFunctionsData[ap_clGetDeviceIDsFromDX9MediaAdapterKHR] = MonitoredFunctionData(L"clGetDeviceIDsFromDX9MediaAdapterKHR", baseDXFuncType, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clCreateFromDX9MediaSurfaceKHR] = MonitoredFunctionData(L"clCreateFromDX9MediaSurfaceKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueAcquireDX9MediaSurfacesKHR] = MonitoredFunctionData(L"clEnqueueAcquireDX9MediaSurfacesKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueReleaseDX9MediaSurfacesKHR] = MonitoredFunctionData(L"clEnqueueReleaseDX9MediaSurfacesKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);

    // cl_khr_d3d10_sharing:
    _monitoredFunctionsData[ap_clGetDeviceIDsFromD3D10KHR] = MonitoredFunctionData(L"clGetDeviceIDsFromD3D10KHR", baseDXFuncType, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clCreateFromD3D10BufferKHR] = MonitoredFunctionData(L"clCreateFromD3D10BufferKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clCreateFromD3D10Texture2DKHR] = MonitoredFunctionData(L"clCreateFromD3D10Texture2DKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clCreateFromD3D10Texture3DKHR] = MonitoredFunctionData(L"clCreateFromD3D10Texture3DKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueAcquireD3D10ObjectsKHR] = MonitoredFunctionData(L"clEnqueueAcquireD3D10ObjectsKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueReleaseD3D10ObjectsKHR] = MonitoredFunctionData(L"clEnqueueReleaseD3D10ObjectsKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);

    // cl_khr_d3d11_sharing:
    _monitoredFunctionsData[ap_clGetDeviceIDsFromD3D11KHR] = MonitoredFunctionData(L"clGetDeviceIDsFromD3D11KHR", baseDXFuncType, AP_GET_FUNC | AP_NULL_CONTEXT_FUNCTION);
    _monitoredFunctionsData[ap_clCreateFromD3D11BufferKHR] = MonitoredFunctionData(L"clCreateFromD3D11BufferKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clCreateFromD3D11Texture2DKHR] = MonitoredFunctionData(L"clCreateFromD3D11Texture2DKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clCreateFromD3D11Texture3DKHR] = MonitoredFunctionData(L"clCreateFromD3D11Texture3DKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueAcquireD3D11ObjectsKHR] = MonitoredFunctionData(L"clEnqueueAcquireD3D11ObjectsKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);
    _monitoredFunctionsData[ap_clEnqueueReleaseD3D11ObjectsKHR] = MonitoredFunctionData(L"clEnqueueReleaseD3D11ObjectsKHR", baseDXFuncType, AP_BUFFER_IMAGE_FUNC | AP_QUEUE_FUNC);

    //////////////////////////////////////////////////////////////////////////
    // Graphic Remedy Extensions:
    //////////////////////////////////////////////////////////////////////////

    // cl_amd_computation_frame
    _monitoredFunctionsData[ap_clBeginComputationFrameAMD] = MonitoredFunctionData(L"clBeginComputationFrameAMD", AP_OPENCL_AMD_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_clEndComputationFrameAMD] = MonitoredFunctionData(L"clEndComputationFrameAMD", AP_OPENCL_AMD_EXTENSION_FUNC, AP_DEBUG_FUNC);

    // cl_amd_object_naming
    _monitoredFunctionsData[ap_clNameContextAMD] = MonitoredFunctionData(L"clNameContextAMD", AP_OPENCL_AMD_EXTENSION_FUNC, AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_clNameCommandQueueAMD] = MonitoredFunctionData(L"clNameCommandQueueAMD", AP_OPENCL_AMD_EXTENSION_FUNC, AP_QUEUE_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_clNameMemObjectAMD] = MonitoredFunctionData(L"clNameMemObjectAMD", AP_OPENCL_AMD_EXTENSION_FUNC, AP_BUFFER_IMAGE_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_clNameSamplerAMD] = MonitoredFunctionData(L"clNameSamplerAMD", AP_OPENCL_AMD_EXTENSION_FUNC, AP_BUFFER_IMAGE_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_clNameProgramAMD] = MonitoredFunctionData(L"clNameProgramAMD", AP_OPENCL_AMD_EXTENSION_FUNC, AP_PROGRAM_KERNEL_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_clNameKernelAMD] = MonitoredFunctionData(L"clNameKernelAMD", AP_OPENCL_AMD_EXTENSION_FUNC, AP_PROGRAM_KERNEL_FUNC | AP_DEBUG_FUNC);
    _monitoredFunctionsData[ap_clNameEventAMD] = MonitoredFunctionData(L"clNameEventAMD", AP_OPENCL_AMD_EXTENSION_FUNC, AP_SYNCHRONIZATION_FUNC | AP_DEBUG_FUNC);
}

// ---------------------------------------------------------------------------
// Name:        apMonitoredFunctionsManager::initializeMonitoredFunctionNameToIdMap
// Description: Initialize the _monitoredFunctionNameToId map.
// Author:  AMD Developer Tools Team
// Date:        8/9/2004
// ---------------------------------------------------------------------------
void apMonitoredFunctionsManager::initializeMonitoredFunctionNameToIdMap()
{
    // Iterate the monitored function:
    int amountOfFuncs = amountOfMonitoredFunctions();

    for (int i = 0; i < amountOfFuncs; i++)
    {
        // Do not map OpenGL ES functions, as we do not currently support them:
        bool isOpenGLES = ((apFirstOpenGLESFunction <= i) && (apLastOpenGLESFunction >= i));

        if (!isOpenGLES)
        {
            // Push the current function name and id into the map:
            gtString currentFuncName = monitoredFunctionName((apMonitoredFunctionId)i);
            _monitoredFunctionNameToId[currentFuncName] = (apMonitoredFunctionId)i;
        }
    }
}


