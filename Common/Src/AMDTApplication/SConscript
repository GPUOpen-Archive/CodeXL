# -*- Python -*-

import os
from CXL_init import *

Import('*')

libName = "CXLApplication"

env = CXL_env.Clone()

env.Append( CPPPATH = [ 
    "./",
    "./inc",
    env['CXL_commonproj_dir'],
    env['CXL_commonproj_dir'] + "/AMDTOSWrappers/Include",
    env['CXL_commonproj_dir'] + '/../../CodeXL',
    "/usr/include/gdk-pixbuf-2.0/", # [Suravee] Needed for Ubuntu-11.10 
])

UseGtk(env)
UseTinyXml(env)
UseQScintilla(env)
UseQt4(env)
UseQCustomPlot(env)

env.Append(CPPFLAGS = '-fPIC')

moc_files = Split(
            " inc/appMainAppWindow.h"
            + " inc/appMDIArea.h"
            + " src/appQtApplication.h"
            )

# Source files:
sources = \
[
# src:
    "src/appApplication.cpp",
    "src/appApplicationCommands.cpp",
    "src/appEventObserver.cpp",
    "src/appMainAppWindow.cpp",
    "src/appMDIArea.cpp",
    "src/appQtApplication.cpp",
]

commonLinkedLibraries = \
[
    "CXLBaseTools",
    "CXLOSWrappers",
    "CXLAPIClasses",
    "CXLApplicationComponents",
    "CXLAssertionHandlers",
    "CXLApplicationFramework",
    "CXLRemoteClient"
]


linkedLibraries = commonLinkedLibraries
env.Prepend (LIBS = linkedLibraries)


# Set the ELF hash generation mode:
# - When building on new systems, we would like to generate both sysv and gnu ELF hashes.
#   This enables running the executable also on old systems, that support only the sysv ELF hash.
# - When building on old systems, we need to set the GR_GENERATE_ONLY_DEFAULT_ELF_HASH environment
#   variable (preferably in the .bashrc file). Otherwise the link will fail when trying to
#   generate an ELF hash type that the old linker does not recognize.
# [Yaki 7/7/2009]
linkerFlags = [] 
shouldGenerateOnlyDefaultELFHash = os.environ.get('GR_GENERATE_ONLY_DEFAULT_ELF_HASH')
if shouldGenerateOnlyDefaultELFHash is None:
    linkerFlags += [ "-Wl,--hash-style=both" ]

MOC_Generated = []
for moc_file in moc_files:
    MOC_Generated += env.MocBld(moc_file)

# Creating shared libraries
soFiles = env.SharedLibrary(
    target = libName, 
    source = sources + MOC_Generated,
    LINKFLAGS = linkerFlags)

# Installing libraries
libInstall = env.Install( 
    dir = env['CXL_bin_dir'], 
    source = (soFiles))

Return('libInstall')
