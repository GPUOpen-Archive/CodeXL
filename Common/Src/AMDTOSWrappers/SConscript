# -*- Python -*-

Import('*')
from CXL_init import *

libName = "CXLOSWrappers"

env = CXL_env.Clone()

env_no_c11 = CXL_env.Clone()

# Remove the c++ flag for the env_no_c11 library
fullBuildFlags = env_no_c11['CPPFLAGS']
partBuildString = fullBuildFlags.replace('-std=c++11','')
env_no_c11.Replace( CPPFLAGS = partBuildString)

env.Append( CPPPATH = [ 
    ".",
    "./src/",
    "./Include/",
    env['CXL_commonproj_dir'],
	env['CXL_common_dir'] + '/Src/Miniz/',
    "/usr/include/gdk-pixbuf-2.0/", # [Suravee] Needed for Ubuntu-11.10 
])

env.Append(CPPFLAGS = '-fno-strict-aliasing')

# osMessageBox, osDesktop
env_no_c11.Append(CFLAGS = '-fno-strict-aliasing')

UseGtk(env)
UseTinyXml(env)
UseAPPSDK(env)
UseBoost(env)

sources = \
[
    "src/osProductVersion.cpp",
    "src/common/osASCIIInputFileImpl.cpp",
    "src/common/osApplication.cpp",
    "src/common/osBugReporter.cpp",
    "src/common/osChannel.cpp",
    "src/common/osChannelOperators.cpp",
    "src/common/osCommunicationDebugManager.cpp",
    "src/common/osCommunicationDebugThread.cpp",
    "src/common/osCGIInputDataReader.cpp",
    "src/common/osCallStack.cpp",
    "src/common/osCallStackFrame.cpp",
    "src/common/osCpuid.cpp",
    "src/common/osCriticalSectionLocker.cpp",
    "src/common/osCriticalSection.cpp",
    "src/common/osDebuggingFunctions.cpp",
    "src/common/osDebugLog.cpp",
    "src/common/osDirectory.cpp",
    "src/common/osDirectorySerializer.cpp",
    "src/common/osDNSQueryThread.cpp",
    "src/common/osEmailClient.cpp",
    "src/common/osExceptionReason.cpp",
    "src/common/osFileImpl.cpp",
    "src/common/osFileLauncher.cpp",
    "src/common/osFileLauncherThread.cpp",
    "src/common/osFilePath.cpp",
    "src/common/osFilePathByLastAccessDateCompareFunctor.cpp",
    "src/common/osFile.cpp",
    "src/common/osGeneralFunctions.cpp",
    "src/common/osHTTPClient.cpp",
    "src/common/osInputFileImpl.cpp",
    "src/common/osMessageBox.cpp",
    "src/common/osModule.cpp",
    "src/common/osMutexLocker.cpp",
    "src/common/osMutex.cpp",
    "src/common/osNetworkAdapter.cpp",
    "src/common/osNULLSocket.cpp",
    "src/common/osOutOfMemoryHandling.cpp",
    "src/common/osOutputFileImpl.cpp",
    "src/common/osPortAddress.cpp",
    "src/common/osProcess.cpp",
    "src/common/osRawMemoryBuffer.cpp",
    "src/common/osRawMemoryStream.cpp",
    "src/common/osSettingsFileHandler.cpp",
    "src/common/osSingeltonsDelete.cpp",
    "src/common/osSocket.cpp",
    "src/common/osStream.cpp",
#    "src/common/osSynchronizationObjectLocker.cpp",
#    "src/common/osSynchronizationObject.cpp",
    "src/common/osTCPSocketServerConnectionHandler.cpp",
    "src/common/osTCPSocketServer.cpp",
    "src/common/osTime.cpp",
    "src/common/osTimeInterval.cpp",
    "src/common/osTransferableObjectCreatorsBase.cpp",
    "src/common/osTransferableObjectCreatorsManager.cpp",
    "src/common/osTransferableObjectType.cpp",
    "src/common/osTransferableObject.cpp",
    "src/common/osWrappersInitFunc.cpp",
    "src/common/osEnvironmentVariable.cpp",
# linuxSources
    "src/linux/osCallsStackReader.cpp",
    "src/linux/osCondition.cpp",
    "src/linux/osConsole.cpp",
    "src/linux/osCriticalSectionImpl.cpp",
    "src/linux/osDaemon.cpp",
    "src/linux/osDirectory.cpp",
    "src/linux/osFile.cpp",
    "src/linux/osFileLauncher.cpp",
     "src/linux/osKeyboardListener.cpp",
    "src/linux/osLinuxProcFileSystemReader.cpp",
    "src/linux/osModule.cpp",
    "src/linux/osMutexImpl.cpp",
    "src/linux/osNetworkAdapter.cpp",
    "src/linux/osPipeExecutor.cpp",
    "src/linux/osPipeSocket.cpp",
    "src/linux/osPipeSocketClient.cpp",
    "src/linux/osPipeSocketServer.cpp",
    "src/linux/osPortAddress.cpp",
    "src/linux/osProcess.cpp",
    "src/linux/osProcessSharedFile.cpp",
    "src/linux/osReadWriteLock.cpp",
#    "src/linux/osSharedMemorySocketClient.cpp",
#    "src/linux/osSharedMemorySocketServer.cpp",
#    "src/linux/osSharedMemorySocket.cpp",
    "src/linux/osSingleApplicationInstance.cpp",
    "src/linux/osSocket.cpp",
    "src/linux/osStopWatch.cpp",
    "src/linux/osSystemError.cpp",
#    "src/linux/osSynchronizationObjectImpl.cpp",
    "src/linux/osTCPSocketClient.cpp",
    "src/linux/osTCPSocketServer.cpp",
    "src/linux/osTCPSocket.cpp",
    "src/linux/osThread.cpp",
    "src/linux/osThreadLocalData.cpp",
    "src/linux/osTime.cpp",    
    "src/linux/osToAndFromString.cpp",
    "src/linux/osTimer.cpp",
    "src/linux/osInputFileImpl.cpp",
    "src/linux/osUnhandledExceptionHandler.cpp",
    "src/linux/osUser.cpp",
# Generic Linux 
    "src/linux/osApplication.cpp",
    "src/linux/osDebuggingFunctions.cpp",
    "src/linux/osDesktop.cpp",
    "src/linux/osFilePath.cpp",
    "src/linux/osGeneralFunctions.cpp",
    "src/linux/osMachine.cpp",
]
    	
csources = \
[
	env['CXL_commonproj_dir'] + '/Miniz/miniz.c',
]


# Creating object files    
objFiles = env.SharedObject(sources)
objFilesC = env_no_c11.SharedObject(csources)

env.Append( LIBPATH = [
    "/usr/lib",
    env['CXL_common_dir'] + "/Lib/Ext/zlib/1.2.8/bin/x64/ZlibStatRelease/"
])

env.Append (LIBS = [
    "dl",
    "libCXLBaseTools",
    "rt",
    "pthread",
    "libz.a"
])

# create static library
statFiles = env.StaticLibrary(
	    target = libName,
	    source = objFiles + objFilesC)
install = statFiles

if (env['CXL_build_type'] != 'static'):
    # Creating shared libraries
    soFiles = env.SharedLibrary(
        target = libName,
        source = objFiles + objFilesC)
    install += soFiles

# Installing libraries
libInstall = env.Install(
    dir = env['CXL_lib_dir'],
    source = (install))

Return('libInstall')
