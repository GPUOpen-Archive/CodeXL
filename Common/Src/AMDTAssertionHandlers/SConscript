# -*- Python -*-

Import('*')
from CXL_init import *

libName = "CXLAssertionHandlers"

env = CXL_env.Clone()
env.Append( CPPPATH = [ 
	".",
	"./inc/",
	env['CXL_commonproj_dir'],
        env['CXL_commonproj_dir'] + '/../../CodeXL',
])

UseQt4(env)

sources = \
[
	"src/ahAssertDialog.cpp",
	"src/ahDialogBasedAssertionFailureHandler.cpp",
]
	
# Creating object files	
objFiles = env.SharedObject(sources)

# Creating shared libraries
soFiles = env.SharedLibrary(
	target = libName, 
	source = objFiles)

# Installing libraries
libInstall = env.Install( 
	dir = env['CXL_lib_dir'], 
	source = (soFiles))

Return('libInstall')
