# -*- Python -*-

Import('*')
from CXL_init import *

libName = "CXLOSAPIWrappers"

env = CXL_env.Clone()

env.Append( CPPPATH = [ 
    ".",
    "./src/",
    "./Include/",
    env['CXL_commonproj_dir'],
    env['CXL_common_dir'] + '/Lib/AMD/HSA/STGFoundation/hsa/include',
    env['CXL_common_dir'] + '/Src/DynamicLibraryModule',
    env['CXL_common_dir'] + '/Src/HSAUtils',
	'/opt/rocm/hsa/include', # A dependency of /Src/HSAUtils
    env['CXL_common_dir'] + '/Src/TSingleton',
    "/usr/include/gdk-pixbuf-2.0/", # [Suravee] Needed for Ubuntu-11.10 
])

# osMessageBox, osDesktop
env.Append(CPPFLAGS = '-fno-strict-aliasing')

# oaDriver, HSAUtils, HSAModule
env.Append(CPPDEFINES=["AMD_INTERNAL_BUILD"])

UseGtk(env)
UseTinyXml(env)
UseAPPSDK(env)

sources = \
[
    "src/common/oaChannelOperators.cpp",
    "src/common/oaDataType.cpp",
    "src/common/oaDriver.cpp",
    "src/common/oaOpenGLRenderContext.cpp",
    "src/common/oaOpenGLFunctionPointers.cpp",
    "src/common/oaOpenGLUtilities.cpp",
    "src/common/oaRawFileSeralizer.cpp",
    "src/common/oaSingeltonsDelete.cpp",
    "src/common/oaTexelDataFormat.cpp",
# linuxSources
    "src/linux/oaDriver.cpp",
# Generic Linux
    "src/linux/oaDeviceContext.cpp",
    "src/linux/oaDisplay.cpp",
    "src/linux/oaHiddenWindow.cpp",
    "src/linux/oaMessageBox.cpp",
    "src/linux/oaOpenGLRenderContext.cpp",
    "src/linux/oaPixelFormat.cpp",
    "src/linux/oaPlatformSpecificFunctionPointers.cpp"
]

# Common/Src (HSA is not supported on 32-bit):
if (env['CXL_arch'] != 'x86' and env['CXL_hsa'] == 'true'):
    sources += \
    [
         env['CXL_common_dir'] + "/Src/HSAUtils/HSAUtils.cpp",
         env['CXL_common_dir'] + "/Src/DynamicLibraryModule/HSAModule.cpp",
         env['CXL_common_dir'] + "/Src/DynamicLibraryModule/DynamicLibraryModule.cpp"
    ]

# Creating object files    
objFiles = env.SharedObject(sources)

env.Append( LIBPATH = [
    "/usr/lib",
    "/usr/lib/x86_64-linux-gnu",
    env['CXL_common_dir'] + "/Lib/Ext/zlib/1.2.8/bin/x64/ZlibStatRelease/"
])

env.Append (LIBS = [
    "dl",
    "rt",
    "pthread",
    "libCXLBaseTools",
    "libCXLOSWrappers",
    "libz.a",
    "libX11"
])


if (env['CXL_build_type'] == 'static'):
	soFiles = env.StaticLibrary(
	target = libName,
	source = objFiles)
else:
	# Creating shared libraries
	soFiles = env.SharedLibrary(
	target = libName,
	source = objFiles)

# Installing libraries
libInstall = env.Install(
    dir = env['CXL_lib_dir'],
    source = (soFiles))

Return('libInstall')
