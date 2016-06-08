# -*- Python -*-

Import('*')
from CXL_init import *

libName = "CXLBaseTools"

env = CXL_env.Clone()

env.Append( CPPPATH = [ 
	".",
	"./src/",
	env['CXL_commonproj_dir'],
	env['CXL_common_dir']
])

sources = \
[
	"src/gtIAllocationFailureObserver.cpp",
	"src/gtASCIIString.cpp",
	"src/gtASCIIStringTokenizer.cpp",
	"src/gtAssert.cpp", 
	"src/gtIAssertionFailureHandler.cpp",
	"src/gtErrorString.cpp",
	"src/gtList.cpp", 
	"src/gtMap.cpp", 
	"src/gtRedBlackTree.cpp",
	"src/gtSingeltonsDelete.cpp",
	"src/gtString.cpp", 
	"src/gtStringTokenizer.cpp", 
	"src/gtVector.cpp"
]
	
# Creating object files	
objFiles = env.SharedObject(sources)

# create static Library
statFiles = env.StaticLibrary(
	target = libName,
	source = objFiles)
install = statFiles

# Creating shared libraries
if (env['CXL_build_type'] != 'static'):
    soFiles = env.SharedLibrary(
        target = libName,
        source = objFiles)
    install += soFiles

# Installing libraries
libInstall = env.Install(
	dir = env['CXL_lib_dir'],
	source = (install))

Return('libInstall')
