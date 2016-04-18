//------------------------------ gsEAGLWrappers.mm ------------------------------

// Objective-C runtime:
#import </usr/include/objc/runtime.h>

// EAGL:
#import <GROSWrappers/GLES/iPhone/EAGL.h>
#import <GROSWrappers/GLES/iPhone/EAGLDrawable.h>

// Infra:
#import <GRAPIClasses/apFrameTerminators.h>
#import <GROSWrappers/osOSDefinitions.h>

// Spies Utilities:
#import <GRSpiesUtilities/suGlobalVariables.h>

// Local:
#import <inc/gsEAGLWrappers.h>
#import <inc/gsOpenGLMonitor.h>
#import <inc/gsGlobalVariables.h>
#import <inc/gsMonitoredFunctionPointers.h>
#import <inc/gsWrappersCommon.h>

// Function predeclarations (these can't be in the header file since that file is included in some C++ files):
id gsEAGLContext_initWithAPI(id self, SEL, EAGLRenderingAPI api);
id gsEAGLContext_initWithAPI_sharegroup(id self, SEL, EAGLRenderingAPI api, EAGLSharegroup* sharegroup);
void gsEAGLContext_dealloc(id self, SEL);
BOOL gsEAGLContext_setCurrentContext(id, SEL, EAGLContext* context);
EAGLContext* gsEAGLContext_currentContext(id, SEL);
EAGLRenderingAPI gsEAGLContext_API(id self, SEL);
EAGLSharegroup* gsEAGLContext_sharegroup(id self, SEL);
BOOL gsEAGLContext_renderbufferStorage_fromDrawable(id self, SEL, NSUInteger target, id /*<EAGLDrawable>*/ drawable);
BOOL gsEAGLContext_presentRenderbuffer(id self, SEL, NSUInteger target);

// Global variables:
Class gsEAGLContextClass = Nil;

// ---------------------------------------------------------------------------
// Name:        gsInitializeEAGLWrappers
// Description: Obtains the function pointers for all the EAGL functions we
//				intercept.
// Author:      Uri Shomroni
// Date:        18/5/2009
// ---------------------------------------------------------------------------
bool gsInitializeEAGLWrappers()
{
	bool retVal = false;
	// TO_DO iPhone: Get gsEAGLContextClass to point to EAGLContext
	// maybe with _dyld_lookup_and_bind_objc with .objc_class_name_EAGLContext
	// then id objc_getClass(const char *name)
	gsEAGLContextClass = objc_getClass("EAGLContext");

	// Make sure we got the class description:
	GT_IF_WITH_ASSERT(gsEAGLContextClass != Nil)
	{
		retVal = true;
	}

	return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGetEAGLFunctionPointers
// Description: Obtains the function pointers for all the EAGL functions we
//				intercept.
// Author:      Uri Shomroni
// Date:        18/5/2009
// ---------------------------------------------------------------------------
void gsGetEAGLFunctionPointers(void*& pEAGLContext_initWithAPI, void*& pEAGLContext_initWithAPI_sharegroup, void*& pEAGLContext_dealloc,
							   void*& pEAGLContext_setCurrentContext, void*& pEAGLContext_currentContext, void*& pEAGLContext_API,
							   void*& pEAGLContext_sharegroup, void*& pEAGLContext_renderbufferStorage_fromDrawable, void*& pEAGLContext_presentRenderbuffer)
{
	// Get the function pointers and cast them from an IMP to a void*:
	pEAGLContext_initWithAPI = (void*)[[gsEAGLContextClass class] instanceMethodForSelector:@selector(initWithAPI:)];
	pEAGLContext_initWithAPI_sharegroup = (void*)[[gsEAGLContextClass class] instanceMethodForSelector:@selector(initWithAPI: sharegroup:)];
	pEAGLContext_dealloc = (void*)[[gsEAGLContextClass class] instanceMethodForSelector:@selector(dealloc)];
	pEAGLContext_setCurrentContext = (void*)[[gsEAGLContextClass class] methodForSelector:@selector(setCurrentContext:)];
	pEAGLContext_currentContext = (void*)[[gsEAGLContextClass class] methodForSelector:@selector(currentContext)];
	pEAGLContext_API = (void*)[[gsEAGLContextClass class] instanceMethodForSelector:@selector(API)];
	pEAGLContext_sharegroup = (void*)[[gsEAGLContextClass class] instanceMethodForSelector:@selector(sharegroup)];
	pEAGLContext_renderbufferStorage_fromDrawable = (void*)[[gsEAGLContextClass class] instanceMethodForSelector:@selector(renderbufferStorage: fromDrawable:)];
	pEAGLContext_presentRenderbuffer = (void*)[[gsEAGLContextClass class] instanceMethodForSelector:@selector(presentRenderbuffer:)];
}

// ---------------------------------------------------------------------------
// Name:        gsGetEAGLWrapperFunctionPointers
// Description: Obtains the wrapper function pointers for all the EAGL
//				functions we intercept.
// Author:      Uri Shomroni
// Date:        21/5/2009
// ---------------------------------------------------------------------------
void gsGetEAGLWrapperFunctionPointers(void*& pgsEAGLContext_initWithAPI, void*& pgsEAGLContext_initWithAPI_sharegroup, void*& pgsEAGLContext_dealloc, void*& pgsEAGLContext_setCurrentContext, void*& pgsEAGLContext_currentContext, void*& pgsEAGLContext_API, void*& pgsEAGLContext_sharegroup, void*& pgsEAGLContext_renderbufferStorage_fromDrawable, void*& pgsEAGLContext_presentRenderbuffer)
{
	// Get the wrapper function pointers and cast them to a void*:
	pgsEAGLContext_initWithAPI = (void*)gsEAGLContext_initWithAPI;
	pgsEAGLContext_initWithAPI_sharegroup = (void*)gsEAGLContext_initWithAPI_sharegroup;
	pgsEAGLContext_dealloc = (void*)gsEAGLContext_dealloc;
	pgsEAGLContext_setCurrentContext = (void*)gsEAGLContext_setCurrentContext;
	pgsEAGLContext_currentContext = (void*)gsEAGLContext_currentContext;
	pgsEAGLContext_API = (void*)gsEAGLContext_API;
	pgsEAGLContext_sharegroup = (void*)gsEAGLContext_sharegroup;
	pgsEAGLContext_renderbufferStorage_fromDrawable = (void*)gsEAGLContext_renderbufferStorage_fromDrawable;
	pgsEAGLContext_presentRenderbuffer = (void*)gsEAGLContext_presentRenderbuffer;
}

// ---------------------------------------------------------------------------
// Name:        gsMakeEAGLContextCurrent
// Description: Wraps +[EAGLContext setCurrentContext:] for internal usage
// Author:      Uri Shomroni
// Date:        16/7/2009
// ---------------------------------------------------------------------------
bool gsMakeEAGLContextCurrent(osOpenGLRenderContextHandle hRC)
{
	bool retVal = false;

	// Cast the context object to its type:
	EAGLContext* pContext = (EAGLContext*)hRC;

	// Make the context current
	SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_EAGLContext_setCurrentContext);
	BOOL result = gs_stat_realFunctionPointers.EAGLContext_setCurrentContext([gsEAGLContextClass class], @selector(presentRenderbuffer:), pContext);
	SU_AFTER_EXECUTING_REAL_FUNCTION(ap_EAGLContext_setCurrentContext);

	retVal = (result == YES);

	return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsSwapEAGLContextBuffers
// Description: Wraps -[EAGLContext presentRenderBuffer:] for internal usage
// Author:      Uri Shomroni
// Date:        16/7/2009
// ---------------------------------------------------------------------------
bool gsSwapEAGLContextBuffers(osOpenGLRenderContextHandle hRC)
{
	// Cast the context object to its type:
	EAGLContext* pContext = (EAGLContext*)hRC;

	// Get the OpenGL ES version:
	int oglesMajorVersion = 0;
	int oglesMinorVersion = 0;

	bool canSwapBuffers = false;

	// Get the OpenGL ES version to know which "GL_RENDERBUFFER" constant to use:
	int contextID = gs_stat_openGLMonitorInstance.renderContextSpyId(hRC);
	GT_IF_WITH_ASSERT(contextID > 0)
	{
		gsRenderContextMonitor* pRenderContextMonitor = gs_stat_openGLMonitorInstance.renderContextMonitor(contextID);
		GT_IF_WITH_ASSERT(pRenderContextMonitor != 0)
		{
			pRenderContextMonitor->getOpenGLVersion(oglesMajorVersion, oglesMinorVersion);

			// We can't call this function if no renderbuffer is present:
			GLuint boundRenderBuffer = pRenderContextMonitor->getActiveRenderBufferObject();
			if (boundRenderBuffer > 0)
			{
				gsRenderBuffersMonitor* pRenderBuffersMonitor = pRenderContextMonitor->renderBuffersMonitor();
				GT_IF_WITH_ASSERT(pRenderBuffersMonitor != NULL)
				{
					apGLRenderBuffer* pRenderBufferInfo = pRenderBuffersMonitor->getRenderBufferObjectDetails(boundRenderBuffer);
					GT_IF_WITH_ASSERT(pRenderBufferInfo != NULL)
					{
						GLint width = 0;
						GLint height = 0;
						pRenderBufferInfo->getBufferDimensions(width, height);

						// Only allow to swap buffers if the render buffer was initialized with -[EAGLContext renderBufferStorage: fromDrawable:] :
						canSwapBuffers = ((width > 0) && (height > 0) && (pRenderBufferInfo->getBufferType() == AP_COLOR_ATTACHMENT0_EXT));
					}
				}
			}
		}
	}

	BOOL result = NO;

	if (canSwapBuffers)
	{
		// Call the function to flush the renderbuffer into the screen:
		SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_EAGLContext_presentRenderbuffer);
		if (oglesMajorVersion == 2)
		{
			result = gs_stat_realFunctionPointers.EAGLContext_presentRenderbuffer(pContext, @selector(presentRenderbuffer:), GL_RENDERBUFFER);
		}
		else if (oglesMajorVersion == 1)
		{
			result = gs_stat_realFunctionPointers.EAGLContext_presentRenderbuffer(pContext, @selector(presentRenderbuffer:), GL_RENDERBUFFER_OES);
		}
		else
		{
			// Unknown OpenGL ES version
			GT_ASSERT(false);
		}
		SU_AFTER_EXECUTING_REAL_FUNCTION(ap_EAGLContext_presentRenderbuffer);
	}

	bool retVal = (result == YES);

	return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsUpdateActiveRenderBufferDimentionsOnStorgaeSet
// Description: Updates the given context's active render buffer's dimensions
//				from the viewport, to support calls of -[EAGLContext renderbufferStorage: fromDrawable:]
// Author:      Uri Shomroni
// Date:        27/4/2010
// ---------------------------------------------------------------------------
void gsUpdateActiveRenderBufferDimentionsOnStorgaeSet(gsRenderContextMonitor& contextMonitor)
{
	// Initialize to the default values for the iPhone:
	int renderBufferWidth = 320;
	int renderBufferHeight = 480;

	// Determine if this is OpenGL ES 1.1 or 2.0
	int oglesVersion[2] = {0,0};
	contextMonitor.getOpenGLVersion(oglesVersion[0], oglesVersion[1]);
	if (oglesVersion[0] == 1)
	{
		// Get the currently bound renderbuffer's dimensions:
		GLint renderBufferGLWidth = 0;
		GLint renderBufferGLHeight = 0;
		SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetRenderbufferParameterivOES);
		gs_stat_realFunctionPointers.glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &renderBufferGLWidth);
		gs_stat_realFunctionPointers.glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &renderBufferGLHeight);
		SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetRenderbufferParameterivOES);

		// If the values are valid:
		GT_IF_WITH_ASSERT((renderBufferGLWidth > 0) && (renderBufferGLHeight > 0))
		{
			// Set them:
			renderBufferWidth = (int)renderBufferGLWidth;
			renderBufferHeight = (int)renderBufferGLHeight;
		}
	}
	else if (oglesVersion[0] == 2)
	{
		// Get the currently bound renderbuffer's dimensions:
		GLint renderBufferGLWidth = 0;
		GLint renderBufferGLHeight = 0;
		SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetRenderbufferParameteriv);
		gs_stat_realFunctionPointers.glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &renderBufferGLWidth);
		gs_stat_realFunctionPointers.glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &renderBufferGLHeight);
		SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetRenderbufferParameteriv);

		// If the values are valid:
		GT_IF_WITH_ASSERT((renderBufferGLWidth > 0) && (renderBufferGLHeight > 0))
		{
			// Set them:
			renderBufferWidth = (int)renderBufferGLWidth;
			renderBufferHeight = (int)renderBufferGLHeight;
		}
	}
	else
	{
		// Unsupported OpenGL ES version:
		GT_ASSERT(false);
	}

	contextMonitor.setActiveRenderBufferObjectParameters(GL_RGBA, renderBufferWidth, renderBufferHeight);
}

// Wraps (id) [EAGLContext initWithAPI:] instance method
id gsEAGLContext_initWithAPI(id self, SEL _cmd, EAGLRenderingAPI api)
{
	SU_START_FUNCTION_WRAPPER(ap_EAGLContext_initWithAPI);

	// Log the call to this function:
	gs_stat_openGLMonitorInstance.addFunctionCall(ap_EAGLContext_initWithAPI, 1, OS_TOBJ_ID_EAGL_RENDERING_API_PARAMETER, api);

	// This function internally calls -[EAGLContext initWithAPI: sharegroup:]
	SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_EAGLContext_initWithAPI_sharegroup);
	id retVal = gs_stat_realFunctionPointers.EAGLContext_initWithAPI(self, _cmd, api);
	SU_AFTER_EXECUTING_REAL_FUNCTION(ap_EAGLContext_initWithAPI_sharegroup);

	if (retVal != nil)
	{
		// Register the created context in our OpenGL monitor:
		gs_stat_openGLMonitorInstance.onContextCreation(NULL, (osOpenGLRenderContextHandle)(retVal), ap_EAGLContext_initWithAPI);
	}

	SU_END_FUNCTION_WRAPPER(ap_EAGLContext_initWithAPI);

	return retVal;
}

// Wraps (id) [EAGLContext initWithAPI: sharegroup:] instance method
id gsEAGLContext_initWithAPI_sharegroup(id self, SEL _cmd, EAGLRenderingAPI api, EAGLSharegroup* sharegroup)
{
	// -[EAGLContext initWithAPI: api] is effectively a wrapper for -[EAGLContext initWithAPI: api sharegroup: nil].
	// If we got here while inside that function, just return the real value:
	if (su_stat_functionInterceptionInfo[ap_EAGLContext_initWithAPI]._isCurrentlyInsideWrapper)
	{
		return gs_stat_realFunctionPointers.EAGLContext_initWithAPI_sharegroup(self, _cmd, api, sharegroup);
	}

	SU_START_FUNCTION_WRAPPER(ap_EAGLContext_initWithAPI_sharegroup);

	// Log the call to this function:
	gs_stat_openGLMonitorInstance.addFunctionCall(ap_EAGLContext_initWithAPI_sharegroup, 2, OS_TOBJ_ID_EAGL_RENDERING_API_PARAMETER, api, OS_TOBJ_ID_POINTER_PARAMETER, (void*)sharegroup);

	id retVal = gs_stat_realFunctionPointers.EAGLContext_initWithAPI_sharegroup(self, _cmd, api, sharegroup);

	if (retVal != nil)
	{
		// Register the created context in our OpenGL monitor:
		gs_stat_openGLMonitorInstance.onContextCreation(NULL, (void*)(retVal), ap_EAGLContext_initWithAPI_sharegroup);
	}

	SU_END_FUNCTION_WRAPPER(ap_EAGLContext_initWithAPI_sharegroup);

	return retVal;
}

// Wraps (void) [EAGLContext dealloc] instance method
void gsEAGLContext_dealloc(id self, SEL _cmd)
{
	SU_START_FUNCTION_WRAPPER(ap_EAGLContext_dealloc);

	// Log the call to this function:
	gs_stat_openGLMonitorInstance.addFunctionCall(ap_EAGLContext_dealloc, 0);

	// Mark that the context was deleted:
	gs_stat_openGLMonitorInstance.beforeContextDeletion((void*)self);

	gs_stat_realFunctionPointers.EAGLContext_dealloc(self, _cmd);

	// Mark that the context was deleted:
	gs_stat_openGLMonitorInstance.afterContextDeletion((void*)self);

	SU_END_FUNCTION_WRAPPER(ap_EAGLContext_dealloc);
}

// Wraps (BOOL) [EAGLContext setCurrentContext:] class method
BOOL gsEAGLContext_setCurrentContext(id self, SEL _cmd, EAGLContext* context)
{
	SU_START_FUNCTION_WRAPPER(ap_EAGLContext_setCurrentContext);

	// Log the call to this function:
	gs_stat_openGLMonitorInstance.addFunctionCall(ap_EAGLContext_setCurrentContext, 1, OS_TOBJ_ID_POINTER_PARAMETER, (void*)context);

	// If we are defined as a frame terminator:
	unsigned int frameTerminatorsMask = suFrameTerminatorsMask();
	if (frameTerminatorsMask & AP_MAKE_CURRENT_TERMINATOR)
	{
		// Terminate the current frame:
		gs_stat_openGLMonitorInstance.onFrameTerminatorCall();
	}

	// Call the real function:
	BOOL retVal = gs_stat_realFunctionPointers.EAGLContext_setCurrentContext(self, _cmd, context);

	if (retVal == YES)
	{
		gs_stat_openGLMonitorInstance.onContextMadeCurrent(NULL, NULL, NULL, (void*)context);
	}

	SU_END_FUNCTION_WRAPPER(ap_EAGLContext_setCurrentContext);
	return retVal;
}

// Wraps (EAGLContext*) [EAGLContext currentContext] class method
EAGLContext* gsEAGLContext_currentContext(id self, SEL _cmd)
{
	SU_START_FUNCTION_WRAPPER(ap_EAGLContext_currentContext);

	// Log the call to this function:
	gs_stat_openGLMonitorInstance.addFunctionCall(ap_EAGLContext_currentContext, 0);

	// Call the real function:
	EAGLContext* retVal = gs_stat_realFunctionPointers.EAGLContext_currentContext(self, _cmd);

	SU_END_FUNCTION_WRAPPER(ap_EAGLContext_currentContext);
	return retVal;
}

// Wraps (EAGLRenderingAPI) [EAGLContext API] read only property getter
EAGLRenderingAPI gsEAGLContext_API(id self, SEL _cmd)
{
	SU_START_FUNCTION_WRAPPER(ap_EAGLContext_API);

	// Log the call to this function:
	gs_stat_openGLMonitorInstance.addFunctionCall(ap_EAGLContext_API, 0);

	EAGLRenderingAPI retVal = gs_stat_realFunctionPointers.EAGLContext_API(self, _cmd);

	SU_END_FUNCTION_WRAPPER(ap_EAGLContext_API);

	return retVal;
}

// Wraps (EAGLSharegroup*) [EAGLContext sharegroup] read only property getter
EAGLSharegroup* gsEAGLContext_sharegroup(id self, SEL _cmd)
{
	SU_START_FUNCTION_WRAPPER(ap_EAGLContext_sharegroup);

	// Log the call to this function:
	gs_stat_openGLMonitorInstance.addFunctionCall(ap_EAGLContext_sharegroup, 0);

	EAGLSharegroup* retVal = gs_stat_realFunctionPointers.EAGLContext_sharegroup(self, _cmd);

	SU_END_FUNCTION_WRAPPER(ap_EAGLContext_sharegroup);

	return retVal;
}

// Wraps (BOOL) [EAGLContext renderbufferStorage: fromDrawable:] instance method
BOOL gsEAGLContext_renderbufferStorage_fromDrawable(id self, SEL _cmd, NSUInteger target, id /*<EAGLDrawable>*/ drawable)
{
	SU_START_FUNCTION_WRAPPER(ap_EAGLContext_renderbufferStorage_fromDrawable);

	// Log the call to this function:
	gs_stat_openGLMonitorInstance.addFunctionCall(ap_EAGLContext_renderbufferStorage_fromDrawable, 2, OS_TOBJ_ID_UNSIGNED_INT_PARAMETER, target, OS_TOBJ_ID_POINTER_PARAMETER, (void*)drawable);

	BOOL retVal = gs_stat_realFunctionPointers.EAGLContext_renderbufferStorage_fromDrawable(self, _cmd, target, drawable);

	if ((retVal == YES) && ((target == GL_RENDERBUFFER_OES) || (target == GL_RENDERBUFFER)))
	{
		// TO_DO iPhone: get this from "self" instead of the current context.
		gsRenderContextMonitor* pRC = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
		GT_IF_WITH_ASSERT(pRC != NULL)
		{
			gsUpdateActiveRenderBufferDimentionsOnStorgaeSet(*pRC);
		}
	}

	SU_END_FUNCTION_WRAPPER(ap_EAGLContext_renderbufferStorage_fromDrawable);

	return retVal;
}

// Wraps (BOOL) [EAGLContext presentRenderbuffer:] instance method
BOOL gsEAGLContext_presentRenderbuffer(id self, SEL _cmd, NSUInteger target)
{
	SU_START_FUNCTION_WRAPPER(ap_EAGLContext_presentRenderbuffer);

	// Log the call to this function:
	gs_stat_openGLMonitorInstance.addFunctionCall(ap_EAGLContext_presentRenderbuffer, 1, OS_TOBJ_ID_UNSIGNED_INT_PARAMETER, target);

	// If we are defined as a frame terminator:
	unsigned int frameTerminatorsMask = suFrameTerminatorsMask();
	if (frameTerminatorsMask & AP_SWAP_BUFFERS_TERMINATOR)
	{
		// Terminate the current frame:
		gs_stat_openGLMonitorInstance.onFrameTerminatorCall();
	}

	BOOL retVal = nil;

	// Will get true iff we are in "Force front draw buffer" mode:
	bool isFrontDrawBuffForced = false;

	// Get the current thread render context monitor:
	gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
	if (pCurrentThreadRenderContextMonitor)
	{
		// Check if front draw buffer is forced:
		gsForcedModesManager& forcedModesMgr = pCurrentThreadRenderContextMonitor->forcedModesManager();
		isFrontDrawBuffForced = forcedModesMgr.isStubForced(AP_OPENGL_FORCED_FRONT_DRAW_BUFFER_MODE);
	}

	// If the OpenGL front draw buffer is forced:
	if (isFrontDrawBuffForced)
	{
		SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glFlush);
		// Since we are rendering into the front buffer - instead of calling [EAGL -presentRenderbuffer:]
		// we will call glFlush.
		// Note that glFlush doesn't return a value, so our wrapper just returns a success value.
		gs_stat_realFunctionPointers.glFlush();
		SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glFlush);

		retVal = YES;
	}
	else
	{
		retVal = gs_stat_realFunctionPointers.EAGLContext_presentRenderbuffer(self, _cmd, target);
	}

	SU_END_FUNCTION_WRAPPER(ap_EAGLContext_presentRenderbuffer);

	return retVal;
}

// Wraps EAGLGetVersion() function
void EAGLGetVersion(unsigned int* major, unsigned int* minor)
{
	SU_START_FUNCTION_WRAPPER(ap_EAGLGetVersion);

	// Log the call to this function:
	gs_stat_openGLMonitorInstance.addFunctionCall(ap_EAGLGetVersion, 2, OS_TOBJ_ID_P_UINT_PARAMETER, major, OS_TOBJ_ID_P_UINT_PARAMETER, minor);

	// Call the real function:
	gs_stat_realFunctionPointers.EAGLGetVersion(major, minor);

	SU_END_FUNCTION_WRAPPER(ap_EAGLGetVersion);
}


// TO_DO iPhone: refer to sharegroups?
// If we do, we need to wrap [EAGLSharegroup alloc].
// Refer to drawables?
// If we do, we need to wrap [CAEAGLLayer alloc] and [CAEAGLLayer drawableProperties]

