import re 
import shutil
import os
import sys
import platform

#
# This file contains CXL specific initializations 
#

def initCXLVars (CXL_vars) :
    try:
        sys.path.append("/usr/lib/scons/SCons/Variables/")
        enum_mod = __import__("EnumVariable")
    except ImportError:
        print "Error: Could not import EnumVariable"
        exit(1)

    # CXL_build
    CXL_vars.AddVariables(enum_mod.EnumVariable(
        key = 'CXL_build',
        help = 'Choose a build type',
        default = 'release',
        allowed_values = ('release', 'debug')))

    # CXL_arch
    CXL_vars.AddVariables(enum_mod.EnumVariable(
        key = 'CXL_arch',
        help = 'Specify architecture',
        default = 'x86_64',
        allowed_values = ('x86', 'x64', 'x86_64')))

    # CXL_build_conf
    CXL_vars.AddVariables(enum_mod.EnumVariable(
        key = 'CXL_build_conf',
        help = 'Choose a build configuration type',
        default = 'PUBLIC',
        allowed_values = ('PUBLIC', 'NDA', 'INTERNAL')))

    # CXL_prefix
    CXL_vars.Add(
        key = 'CXL_prefix',
        help='Installation prefix',
        default = os.getcwd())

    # CXL_common_dir
    # Note: CXL_common_dir can also be specified by environment variables
    CXL_vars.Add(
        key = 'CXL_common_dir',
        help='Specify CXL common dir',
        default = '')

    # Note: MUST be specified on the command line if used
    CXL_vars.Add(
        key = 'CXL_commonproj_dir',
        help='Specify CXL CommonProjects dir',
        default = '')

    # CXL_build_verbose
    CXL_vars.Add(
        key = 'CXL_build_verbose',
        help='Specify CXL build output message verbose level ',
        default = 0)

    # CXL_build_verbose
    CXL_vars.Add(enum_mod.EnumVariable(
        key = 'CXL_build_type',
        help='Build Basetools/OSWrappers libraries as static ',
        default = '',
        allowed_values = ('', 'static')))

    # CxL support for hsa
    CXL_vars.Add(
        key = 'CXL_hsa',
        help = 'Support hsa',
        default = '',
        allowed_values = ('true', ''))


def initArch (env) :
    arch = env['CXL_arch']
    if (arch == 'x86'):
        bitness = '32'
    elif (arch == 'x64'):
        env['CXL_arch'] = 'x86_64'
        bitness = '64'
    elif (arch == 'x86_64'):
        bitness = '64'

    env.Append (CXL_bitness = bitness)


def initOs (env) :
    uname_tuple = platform.uname()
    if uname_tuple[0] == "Linux":
        env.Append (CXL_os = uname_tuple[0])
    else:
        env.Append (CXL_os = "Unknown")


def initDistro (env) :
    if (env["CXL_os"] == "Linux"):
        if os.path.exists("/etc/SuSE-release"):
            env.Append (CXL_distro = "SuSE")    
        elif os.path.exists("/etc/redhat-release"):
            env.Append (CXL_distro = "Redhat")    
        elif os.path.exists("/etc/lsb-release"):
            lsb_file = open("/etc/lsb-release")
            distro_id = lsb_file.readline().split("=")
            if (distro_id[1] == "Ubuntu"):
                env.Append (CXL_distro = "Ubuntu")    
        else:
            env.Append (CXL_distro = "Unknown")    


def initInstallDirs (env) :

    # Current CodeXL staging
    install_dir = env['CXL_prefix'] 
    install_dir += '/Output_' + env['CXL_arch']
    install_dir += '/' + env['CXL_build'] 
    install_dir += '/bin'
    env.Append (CXL_install_dir = install_dir)

    env.Append (CXL_bin_dir = install_dir)
    if not os.path.exists(env['CXL_bin_dir']):
        os.makedirs(env['CXL_bin_dir'])
    
    env.Append (CXL_lib_dir = install_dir)
    if not os.path.exists(env['CXL_lib_dir']):
        os.makedirs(env['CXL_lib_dir'])
    # Assign default linking path
    env.Append( LIBPATH = [env['CXL_lib_dir']])


    env.Append (CXL_include_dir = install_dir + '/include')
    if not os.path.exists(env['CXL_include_dir']):
        os.makedirs(env['CXL_include_dir'])

    env.Append (CXL_Images_dir = install_dir + '/Images')
    if not os.path.exists(env['CXL_Images_dir']):
        os.makedirs(env['CXL_Images_dir'])

    env.Append (CXL_HTML_dir = install_dir + '/HTML')
    if not os.path.exists(env['CXL_HTML_dir']):
        os.makedirs(env['CXL_HTML_dir'])

    env.Append (CXL_Legal_dir = install_dir + '/Legal')
    if not os.path.exists(env['CXL_Legal_dir']):
        os.makedirs(env['CXL_Legal_dir'])

    env.Append (CXL_spies_dir = install_dir + '/spies')
    if not os.path.exists(env['CXL_spies_dir']):
        os.makedirs(env['CXL_spies_dir'])

    env.Append (CXL_Data_dir = install_dir + '/Data')
    if not os.path.exists(env['CXL_Data_dir']):
        os.makedirs(env['CXL_Data_dir'])

    env.Append (CXL_Events_dir = install_dir + '/Data/Events')
    if not os.path.exists(env['CXL_Events_dir']):
        os.makedirs(env['CXL_Events_dir'])

    env.Append (CXL_Profiles_dir = install_dir + '/Data/Profiles')
    if not os.path.exists(env['CXL_Profiles_dir']):
        os.makedirs(env['CXL_Profiles_dir'])

    env.Append (CXL_Views_dir = install_dir + '/Data/Views')
    if not os.path.exists(env['CXL_Views_dir']):
        os.makedirs(env['CXL_Views_dir'])

    env.Append (CXL_Examples_dir = install_dir + '/examples/Teapot')
    env.Append (CXL_OriginalExamples_dir = env['CXL_common_dir'] + '/../CodeXL/Examples/AMDTTeaPot')
    if not os.path.exists(env['CXL_Examples_dir']):
        os.makedirs(env['CXL_Examples_dir'])

    # webhelp (generated by doxygen) - this environment variable must match that in
    # the Doxyfile.  So we need to export it to the os environment as well.
    env.Append (CXL_webhelp_dir = install_dir + '/webhelp')
    if not os.path.exists(env['CXL_webhelp_dir']):
        os.makedirs(env['CXL_webhelp_dir'])
    # env.AppendENVPath('CXL_webhelp_dir', env['CXL_webhelp_dir'])
    env['ENV']['CXL_webhelp_dir'] = env['CXL_webhelp_dir']

def initCompilerFlags (env) :
    compiler_base_flags = " -Wall -Werror -Wextra -g -fmessage-length=0 -Wno-unknown-pragmas -pthread -DGDT_BUILD_SUFFIX= -std=c++11"    
    linker_base_flags = ''
    
    if (env['CXL_build'] == 'debug'):
        compiler_base_flags += ' -D_DEBUG '
    else:
        compiler_base_flags += ' -O3 -DNDEBUG '

    if (env['CXL_bitness'] == '32'):
        compiler_base_flags += ' -m32 '
        linker_base_flags += ' -m32 '
    
    if (env['CXL_build_conf'] == 'PUBLIC'):
        compiler_base_flags += ' -DGDT_PUBLIC '
    elif (env['CXL_build_conf'] == 'NDA'):
        compiler_base_flags += ' -DGDT_NDA '
    elif (env['CXL_build_conf'] == 'INTERNAL'):
        compiler_base_flags += ' -DGDT_INTERNAL '
    else:
        compiler_base_flags += ' -DGDT_PUBLIC '

    # add the support for HSA
    if (env['CXL_hsa'] == 'true'):
        compiler_base_flags += ' -DCODEXL_HSA_SUPPORT '

    env.Prepend(CPPFLAGS = compiler_base_flags)
    env.Prepend(LINKFLAGS = linker_base_flags)

    
def initImages (env) :
    images = []
    imgSrcDir = env['CXL_common_dir'] + "/../CodeXL/Images"
    imgSrcFiles = os.listdir(imgSrcDir)
    for f in imgSrcFiles:
        images.append(imgSrcDir+ "/" + f)

    images_install = env.Install(
        dir = env['CXL_Images_dir'],
        source =  images)
    env.Append (CXL_Images_install = images_install)


def initLegal (env) :
    legals = []
    legalSrcDir = env['CXL_common_dir'] + "/../CodeXL/Setup/Legal"
    legalSrcFiles = os.listdir(legalSrcDir)
    for f in legalSrcFiles:
        legals.append(legalSrcDir+ "/" + f)

    legals_install = env.Install(
        dir = env['CXL_Legal_dir'],
        source =  legals)
    env.Append (CXL_Legal_install = legals_install)

def initHelp (env) :
    # The doxygen tool utilizes the environment variables:
    #   CXL_webhelp_dir
    #   CXL_doxygen_dir
    env.Append(CXL_doxygen_dir =  env['CXL_common_dir'] + "/DK/Doxygen/doxygen-1.8.4")
    env.Append(CXL_doxy_tool = env['CXL_doxygen_dir']  + "/bin/doxygen")
    env.AppendENVPath('CXL_doxygen_dir', env['CXL_doxygen_dir'])
    env['ENV']['CXL_doxygen_dir'] = env['CXL_doxygen_dir']

def initCpuPerfEventsData (env) :
    dataDstEnvs = [ 'CXL_Events_dir', 'CXL_Profiles_dir', 'CXL_Views_dir' ]
    dataTypes   = [ 'Events/Public' , 'Profiles'        , 'Views'         ]
    dataSrcDir = env['CXL_common_dir'] + "/Lib/AMD/AMDTCpuPerfEventUtils/1.0/Data/"

    cpuEventsData_install = []
    for idx in range(len(dataTypes)):
        eventFiles = []
        eventsSrcDir = dataSrcDir + dataTypes[idx]
        eventsSrcFiles = os.listdir(eventsSrcDir)
        for f in eventsSrcFiles:
            eventFiles.append(eventsSrcDir + "/" + f)

        cpuEventsData_install.append(env.Install(
            dir = env[ dataDstEnvs[idx] ],
            source = eventFiles))
    env.Append (CXL_CpuEventsData_install = cpuEventsData_install)

def initCXLBuild (env) :

    # Use CXL_common_dir from system environment variable 
    # if specified and not given as SConstruct variable.
    try:
        tmp = os.environ['CXL_common_dir']
    except KeyError:
        print ("Warning: CXL_common_dir not available. Using SConstruct variable.")
    else:
        if (tmp != "" and env['CXL_common_dir'] == ""):
            env['CXL_common_dir'] = tmp

    # Return with error if no CXL_common_dir specified
    if (env['CXL_common_dir'] == ""):
        print ("Error: Please specify CXL_common_dir" +
            " as env variable, or in SConstruct variable")
        exit(1)

    # Derive CXL_commonproj_dir from CXL_common_dir
    env.Append(CXL_commonproj_dir = env['CXL_common_dir'] + "/Src")

    # NOTE: Do not change order of these inits
    initArch(env)
    initOs(env)
    initDistro(env)
    initCompilerFlags(env)
    initInstallDirs(env)
    initImages (env)
    initLegal(env)
    initHelp(env)
    initCpuPerfEventsData(env)

    # This was an artifact of WxWidgets, and can go away
    # env.Append( CPPDEFINES = ["_FILE_OFFSET_BITS=64", "_LARGE_FILES"])


def copySharedLibrary ( env, sym, srcDir, destDir ):
    src = srcDir + "/" + sym 
    if os.path.islink(src):
        linkto = os.readlink(src)
        if env['CXL_build_verbose'] != 0 :
            print ("Symlinking : " + sym + '--->' + linkto)
        if not os.path.exists(destDir + "/" + linkto):
            shutil.copy2(srcDir + "/" + linkto, destDir + "/" + linkto)
        if not os.path.exists(destDir + "/" + sym):
            os.symlink(linkto, destDir + "/" + sym)
    else:
        if not os.path.exists(destDir + "/" + sym):
            shutil.copy2(src, destDir + "/" + sym)
            if env['CXL_build_verbose'] != 0 :
                print ("Copying    : " + sym)

def initQt4 (env) :
    # TODO: This will need to change for Ubuntu
    # Example: Common/Lib/Ext/Qt/4.7.4/Build/CentOS6.2/x86_64/debug
    cxl_qt_dir = env['CXL_common_dir'] + "/Lib/Ext/Qt/5.5"
    cxl_qt_dir += "/Build"
    cxl_qt_dir += "/CentOS66"
    cxl_qt_dir += "/" + env["CXL_arch"]
    cxl_qt_dir += "/release"

    # TODO: We should be able to specify these dirs
    qt_dir = cxl_qt_dir
    qt_inc_dir = qt_dir + "/include"
    qt_lib_dir = qt_dir + "/lib"
    qt_bin_dir = qt_dir + "/bin"
    qt_platforms_dir = qt_dir + "/plugins/platforms"

    # This is the base list of qt module needed for CodeXL
    # TODO: We should allow user to add to the list
    qt_base_module_list = ('Qt5Core', 'Qt5Gui', 'Qt5Xml', 'Qt5OpenGL', 'Qt5Network', 'Qt5WebKit','Qt5Widgets','Qt5WebKitWidgets','Qt5MultimediaWidgets','Qt5Positioning','Qt5PrintSupport','Qt5Multimedia','Qt5Sensors','Qt5Sql','Qt5Quick','Qt5Qml','Qt5DBus','Qt5WebChannel','Qt5XcbQpa')
    qt_module_list = qt_base_module_list    

    qt_inc_path  = [
        qt_inc_dir, 
        qt_inc_dir + "/Qt"]
    qt_libs =[] 
    dbgSuffix = ''
    for qtmod in qt_module_list:

        qtmodNo5 = qtmod.replace('Qt5','Qt')
        qt_inc_path += [qt_inc_dir + "/" + qtmodNo5 ]
        qt_libs.append(qtmod + dbgSuffix)

        files = os.listdir(qt_lib_dir)
        for file in files:
            tmp = re.match( "^lib" + qtmod + dbgSuffix + ".so", file)
            if tmp:
                copySharedLibrary(env, file, qt_lib_dir, env['CXL_lib_dir']) 
    
    # added libraries needed for Q5.3 25-may-2014
    files = os.listdir(qt_lib_dir)
    for file in files:
        tmp = re.match( "lib" + "ic*", file)
        if tmp:
            copySharedLibrary(env, file, qt_lib_dir, env['CXL_lib_dir'])
            #qt_extra_lib = file[0:file.find('.')]
            #lib_exist = qt_extra_lib in qt_libs
            #if (lib_exist):
            #    print "lib exists"
            #else:
            #    qt_libs.append(qt_extra_lib)
    qt_extra_libs = ('icui18n','icuuc','icudata')
    qt_libs.append(qt_extra_libs)

    # 13-Aug-2012 - update to match "classic" build
    qt_define_list = [
        'QT_DLL', 'QT_GUI_LIB', 'QT_CORE_LIB','QT_THREAD_SUPPORT',
    ]

    # 16-June-2014 Copy the xcd platform file that is needed in qt5
    qtxcd_file = "libqxcb.so"
    if not os.path.exists(env['CXL_lib_dir'] + "/platforms"):
        os.makedirs(env['CXL_lib_dir'] + "/platforms")

    copySharedLibrary(env, qtxcd_file, qt_platforms_dir, env['CXL_lib_dir'] + "/platforms")

    env.Append(CXL_qt_dir = qt_dir)
    env.Append(CXL_qt_def = qt_define_list)
    env.Append(CXL_uic_tool = qt_bin_dir + "/uic")
    env.Append(CXL_moc_tool = qt_bin_dir + "/moc")
    env.Append(CXL_rcc_tool = qt_bin_dir + "/rcc")

    env.Append(CXL_Qt4_define_list = qt_define_list)
    env.Append(CXL_Qt4_inc_path = qt_inc_path)
    env.Append(CXL_Qt4_libs = qt_libs)
    env.Append(CXL_Qt4_libdir = qt_lib_dir)

# Call the initQt4 routine to perform the setup, and 'install' of the Qt libraries
    
# and call UseQt4 to add it to the local environment
def UseQt4(env):
    env.Append( CPPDEFINES = env['CXL_Qt4_define_list'] )
    env.Append( CPPPATH = env['CXL_Qt4_inc_path'] )
    env.Append( LIBS = env['CXL_Qt4_libs'] )
    env.Append( LIBPATH = [env['CXL_Qt4_libdir']] )


def initGtk (env) :
    env.Append(CXL_Gtk_inc_path = [
        "/usr/include/cairo",
        "/usr/include/pango-1.0",
        "/usr/include/atk-1.0",
        "/usr/include/gtk-2.0",
        "/usr/include/gdk-pixbuf-2.0",
        "/usr/include/glib-2.0",
        "/usr/lib64/gtk-2.0/include",
        "/usr/lib64/glib-2.0/include/",
    "/usr/lib/x86_64-linux-gnu/glib-2.0/include",
    "/usr/lib/x86_64-linux-gnu/gtk-2.0/include",
    "/usr/include/x86_64-linux-gnu/",
    ])
    # No additional libraries needed for Gtk

def UseGtk(env):
    env.Append (CPPPATH = [env['CXL_Gtk_inc_path']])
from distutils.version import StrictVersion

def initStdc(env):
    stdclib_dir = env['CXL_common_dir'] + '/Lib/Ext/libstdc/6.0.16/CentOS64/' 
    if StrictVersion(env['CXXVERSION']) > StrictVersion('4.7.2'):
        stdclib_dir = env['CXL_common_dir'] + '/Lib/Ext/libstdc/6.0.20/CentOS64/'
    stdclib_lib = stdclib_dir + env['CXL_arch']
    for file in os.listdir(stdclib_lib):
        copySharedLibrary(env, file, stdclib_lib, env['CXL_lib_dir']) 
        
def initTinyXml (env) :
    dbgSuffix = '' 

    tinyxml_dir = env['CXL_common_dir'] + '/Lib/Ext/tinyxml/2.6.2'
    tinyxml_inc = tinyxml_dir
    tinyxml_lib = tinyxml_dir + '/Build/CentOS64/' + env['CXL_build'] + '/' + env['CXL_arch']
    libsrc = []
    tinyxml_libs = ['tinyXML']
    libsrc.append(tinyxml_libs)

    env.Append(CXL_TinyXML_inc = [tinyxml_dir, tinyxml_inc])
    env.Append(CXL_TinyXML_libs = tinyxml_libs)
    env.Append(CXL_TinyXML_libpath = tinyxml_lib)
    # We do not need to install anything - it is just a single archive

def UseTinyXml(env):
    env.Append(CPPPATH = [env['CXL_TinyXML_inc']])
    env.Append(LIBS = env['CXL_TinyXML_libs'])
    env.Append(LIBPATH = env['CXL_TinyXML_libpath'])

def initQScintilla (env) :
    qscintilla_dir = env['CXL_common_dir'] + '/Lib/Ext/QScintilla/2.8-GPL'
    qscintilla_inc = [qscintilla_dir + "/Qt4Qt5"]
    qscintilla_lib = qscintilla_dir + '/lib/linux/CentOS66' + '/' + env['CXL_arch'] + '/'
    libsrc = []
    
    for file in os.listdir(qscintilla_lib):
        libsrc.append(qscintilla_lib + "/" + file)
        # Copy the shared libs and symlinks to the install location
        tmp = re.match( "^libqscintilla" + ".so", file)
        if tmp:
            copySharedLibrary(env, file, qscintilla_lib, env['CXL_lib_dir']) 

    env.Append(CXL_QSci_inc = qscintilla_inc)
    env.Append(CXL_QSci_libs = 'qscintilla')
    env.Append(CXL_QSci_libpath = qscintilla_lib)

def UseQScintilla(env):
    env.Append(CPPPATH = env['CXL_QSci_inc'])
    env.Append(LIBS = env['CXL_QSci_libs'])
    env.Append(LIBPATH = [env['CXL_QSci_libpath']])

def initQCustomPlot (env) :
    if (env['CXL_build'] == 'debug'):
        dbgSuffix = 'd'
    else:
        dbgSuffix = ''
    libsrc = []

    qcustomplot_dir = env['CXL_common_dir'] + '/Lib/Ext/qcustomplot/1.3.1'
    qcustomplot_inc = qcustomplot_dir + '/include'
    qcustomplot_lib = qcustomplot_dir + '/lib/linux/' + env['CXL_build'] + '/'
    qcustomplot_libs = ['qcustomplot' + dbgSuffix] 

    for file in os.listdir(qcustomplot_lib):
        libsrc.append(qcustomplot_lib + "/" + file)
        # Copy the shared libs and symlinks to the install location
        tmp = re.match( "^libqcustomplot" + dbgSuffix + ".so", file)
        if tmp:
            copySharedLibrary(env, file, qcustomplot_lib, env['CXL_lib_dir']) 

    env.Append(CXL_QCustomPlot_inc = [qcustomplot_inc])
    env.Append(CXL_QCustomPlot_libs = qcustomplot_libs)
    env.Append(CXL_QCustomPlot_libpath = qcustomplot_lib)
    # We do not need to install anything - it is just a single archive

def UseQCustomPlot(env):
    env.Append(CPPPATH = [env['CXL_QCustomPlot_inc']])
    env.Append(LIBS = env['CXL_QCustomPlot_libs'])
    env.Append(LIBPATH = env['CXL_QCustomPlot_libpath'])

def initLibElf (env) :
    libElf_common  = env['CXL_common_dir'] + '/Lib/Ext/GRLibDWARF/common/'
    libElf_dir     = env['CXL_common_dir'] + '/Lib/Ext/GRLibDWARF/libelf/'
    libElf_inc     = libElf_dir
    libElf_libpath = libElf_dir + '/Build/CentOS6.2' + '/' + env['CXL_arch'] + '/' + env['CXL_build'] + "/lib"
    libElf_libs    = "libelf"

    env.Append(CPPPATH = [libElf_inc, libElf_common])
    env.Append(LIBPATH = [libElf_libpath])
    env.Append(LIBS = libElf_libs)


def initLibDwarf (env) :
    libDwarf_dir     = env['CXL_common_dir'] + '/Lib/Ext/GRLibDWARF/libdwarf/'
    libDwarf_inc     = libDwarf_dir
    libDwarf_libpath = libDwarf_dir + '/Build/CentOS6.2' + '/' + env['CXL_arch'] + '/' + env['CXL_build'] + "/lib"
    libDwarf_libs    = "libdwarf"

    env.Append(CPPPATH = [libDwarf_inc])
    env.Append(LIBPATH = [libDwarf_libpath])
    env.Append(LIBS = libDwarf_libs)

def initAMDOpenCL (env) :
    amdopencl_dir = env['CXL_common_dir'] + '/Lib/AMD/OpenCLDebugAPI/1.3/Lib'

    if (env['CXL_build'] == 'debug'):
        dbgSuffix = '-d'
    else:
        dbgSuffix = ''

    if (env['CXL_arch'] == 'x86_64'):
        amdopencl_lib = amdopencl_dir + '/x64'
        file = 'libAMDOpenCLDebugAPI64' + dbgSuffix + ".so"
    else:
        amdopencl_lib = amdopencl_dir + '/x86'
        file = 'libAMDOpenCLDebugAPI32' + dbgSuffix + ".so"

    copySharedLibrary(env, file, amdopencl_lib, env['CXL_lib_dir']) 

def initGLEW (env) :
    amdglew_dir =  env['CXL_common_dir'] + '/Lib/Ext/glew/1.9.0/Build/Ubuntu'

    if (env['CXL_arch'] == 'x86_64'):
        amdglew_dir = amdglew_dir + '/x86_64/'
    else:
        amdglew_dir = amdglew_dir + '/x86/'

    copySharedLibrary(env, "libGLEW.so.1.9.0", amdglew_dir, env['CXL_lib_dir']) 
    copySharedLibrary(env, "libGLEW.so.1.9", amdglew_dir, env['CXL_lib_dir']) 
    copySharedLibrary(env, "libGLEW.so", amdglew_dir, env['CXL_lib_dir']) 

def initBoost (env) :
    amdboost_dir =  env['CXL_common_dir'] + '/Lib/Ext/Boost/boost_1_59_0/lib/RHEL6'

    if (env['CXL_arch'] == 'x86_64'):
        amdboost_dir = amdboost_dir + '/x86_64/'
        shutil.copy2(amdboost_dir + "libboost_system.so.1.59.0", env['CXL_lib_dir'] + "/libboost_system.so.1.59.0")
        shutil.copy2(amdboost_dir + "libboost_filesystem.so.1.59.0", env['CXL_lib_dir'] + "/libboost_filesystem.so.1.59.0")
        shutil.copy2(amdboost_dir + "libboost_regex.so.1.59.0", env['CXL_lib_dir'] + "/libboost_regex.so.1.59.0")
        shutil.copy2(amdboost_dir + "libboost_program_options.so.1.59.0", env['CXL_lib_dir'] + "/libboost_program_options.so.1.59.0")
        shutil.copy2(amdboost_dir + "libboost_wave.so.1.59.0", env['CXL_lib_dir'] + "/libboost_wave.so.1.59.0")
        shutil.copy2(amdboost_dir + "libboost_thread.so.1.59.0", env['CXL_lib_dir'] + "/libboost_thread.so.1.59.0")
        shutil.copy2(amdboost_dir + "libboost_chrono.so.1.59.0", env['CXL_lib_dir'] + "/libboost_chrono.so.1.59.0")
        shutil.copy2(amdboost_dir + "libboost_date_time.so.1.59.0", env['CXL_lib_dir'] + "/libboost_date_time.so.1.59.0")
    else:
        amdboost_dir = amdboost_dir + '/x86/'

    env.Append(LIBPATH = [amdboost_dir])

def UseBoost (env):
    amdboost_dir =  env['CXL_common_dir'] + '/Lib/Ext/Boost/boost_1_59_0/'
    env.Append(CPPPATH = [amdboost_dir])

    if (env['CXL_arch'] == 'x86_64'):
        amdboost_dir = amdboost_dir + 'lib/RHEL6/x86_64/'
    else:
        amdboost_dir = amdboost_dir + 'lib/RHEL6/x86/'

    env.Append(LIBPATH = [amdboost_dir])
    
def UseFltk (env):
    amdfltk_dir =  env['CXL_common_dir'] + '/Lib/Ext/fltk/1.1.0/'
    amdfltk_dir_libs = amdfltk_dir + '/lib/x86_64/'
    amdfltk_dir_inc = amdfltk_dir + '/include/'
    env.Append(LIBPATH = [amdfltk_dir_libs])
    env.Append(CPPPATH = [amdfltk_dir_inc])

def initAMDTQTControls(env):
    amdtQtControls_dir = env['CXL_common_dir'] + '/Lib/AMD/AMDTQtControls/1.0/'
    amdtQtControls_inc     = amdtQtControls_dir + 'Include'
    amdtQtControls_libpath = amdtQtControls_dir + 'Build/CentOS64/' + env['CXL_arch'] + '/' + env['CXL_build']
    amdtQtControls_libs    = "libAMDTQtControls"
    copySharedLibrary(env, amdtQtControls_libs + '.so', amdtQtControls_libpath, env['CXL_lib_dir']) 
    env.Append(CXL_amdtQtControls_inc = amdtQtControls_inc)
    env.Append(CXL_amdtQtControls_libs = amdtQtControls_libs)
    env.Append(CXL_amdtQtControls_libpath = amdtQtControls_libpath)

def UseAMDTQTControls(env):
    env.Append(CPPPATH = env['CXL_amdtQtControls_inc'])
    env.Append(LIBS = env['CXL_amdtQtControls_libs'])
    env.Append(LIBPATH = env['CXL_amdtQtControls_libpath'])

def UseAPPSDK (env):
    amdAPPSDK_dir =  env['CXL_common_dir'] + '/Lib/AMD/APPSDK/3-0/include/'
    env.Append(CPPPATH = [amdAPPSDK_dir])

#####################################
# A component .py properties file takes the form
#   VER="<value>"   value is the version number (which forms a directory path)
#                   or live (case insensitive) which means build from the source
#                   code under CommonProjects
#   LIVE_INC_PATH   path to the include files under the live tree.  Only meaningful
#                   when using the 'live' version.  The location of the promoted
#                   include files may end up being completely different than the
#                   live versions, and may depend upon the complexity of the component
#                   For example, derived include files could contain content which
#                   is specific to debug vs. release/OS bit size/Win vs. Linux, etc.
#   LIBS            list of library names, without any "lib" prefix or ".so"/".dll" suffix
#                   Items in LIBS will be linked against, and then installed/copied to the
#                   relevant installation directory.
#   COPY_LIBS       list of library names to be copied to the installation directory.
#                   Currently only has meaning in the promotion model
#   ARCHIVES        list of archives (.ar or .lib)  These will be linked against, but not
#                   copied into the installation directory
#                   this is an OPTIONAL field.
#
#   INC_PATH        About to be phased out
#                       Use this if there is only a single set of includes in the promoted
#                       directory which are consumed.
#   LIB_PATH        About to be phased out.  Similar, but for the library location.
#
#       key/value lists
#       The key is constructed based upon debug/release and architecture
#       and takes the form debug|release-x86|x86_64
#
#   INCLUDE_TREE    a list of key/value pairs.  The value is the directory in which
#                   the include content for the key is located.  This can be a list
#                   of directories
#   LIBRARY_TREE    a list of key/value pairs.  The value is the directory in which
#                   the library or archive content for the key is located.  This is
#                   a single directory (not a list)
#
#####################################
def initCommonLibAmd (env, libNameList) :
    if (env['CXL_common_dir'] == ""):
        return

    common_inc_path = [] 
    common_lib_path = [] 
    common_libs     = [] 
    for libName in libNameList:
        mod_path = env['CXL_common_dir'] + "/Lib/AMD" + "/" +libName

        # TODO: Import environment variables from the file
        sys.path.append(mod_path)
        try :
            import_mod = __import__(libName)
        except ImportError:
            print "Error: Could not import module " + libName + "(" + mod_path + ")\n"
            exit (1)
        else:
            if (import_mod.VER.lower() == "live"):
                
                #########################################################################
                # LIVE MODEL :
                # If the versioning is "live" (case insensitive)
                # The module will be built from the CommonProject. 
                # It is the responsibility of the caller to deal with any exceptions and
                # the relevant library path.
                #########################################################################
                
                # Check is CommonProjects directory is specified
                liveProj_dir = env['CXL_commonproj_dir']
                if (liveProj_dir == ""):
                    print "Error: CXL_commonproj_dir not specified"
                    return

                if env['CXL_build_verbose'] != 0 :
                    print "Note: Using the 'live' code base for " + libName

                liveProj_dir += "/" + libName

                # We can provide one or more path (str or list)
                if ( type(import_mod.LIVE_INC_PATH).__name__ == 'list'):
                    for path in import_mod.LIVE_INC_PATH:
                        common_inc_path += [ liveProj_dir + "/" + path ]
                elif ( type(import_mod.LIVE_INC_PATH).__name__ == 'str'):
                    common_inc_path += [ liveProj_dir + "/" + import_mod.LIVE_INC_PATH ]

                # Import libs. In this case, we default to use installation directory
                # as default libpath and assuming that the module will be build and readily
                # available there.
                common_libs.append( import_mod.LIBS )
                if (hasattr (import_mod, 'ARCHIVES')):
                    common_libs.append( import_mod.ARCHIVES )

                if ( hasattr(import_mod, 'DATA_SRC') ) and ( hasattr(import_mod, 'DATA_DST') ) :
                    if import_mod.DATA_SRC != "":
                        src = liveProj_dir + "/" + import_mod.DATA_SRC 
                        dst = env['CXL_Data_dir'] + "/" + import_mod.DATA_DST 
                        if  os.path.exists(dst):
                            shutil.rmtree (dst)
                        shutil.copytree(src, dst, symlinks = True)
            else: 
                #########################################################################
                # PROMOTION MODEL :
                # Use the already built binaries in the Common directory specified by 
                # version number
                #########################################################################

                # Construct the relevant key
                tableKey = env['CXL_build'] + '-' + env['CXL_arch']

                if env['CXL_build_verbose'] != 0 :
                    print "Note: Using the version "+ import_mod.VER +" code base for " + libName
                    print "tableKey is: [" + tableKey + "]"

                ver_dir = mod_path + "/" + import_mod.VER

                # Look up the include tree
                if (hasattr(import_mod, 'INCLUDE_TREE')):
                    valueItem = import_mod.INCLUDE_TREE[tableKey]
                    if (type(valueItem).__name__ == 'list'):
                        for path in valueItem:
                            common_inc_path += [ ver_dir + "/" + path ]
                    elif (type(valueItem).__name__ == 'str'):
                            common_inc_path += [ ver_dir + "/" + valueItem ]

                # We can provide one or more path (str or list)
                # if ( type(import_mod.INC_PATH).__name__ == 'list'):
                #     for path in import_mod.INC_PATH:
                #         common_inc_path += [ ver_dir + "/" + path ]
                # elif ( type(import_mod.INC_PATH).__name__ == 'str'):
                #     common_inc_path += [ ver_dir + "/" + import_mod.INC_PATH ]

                # Import libs
                common_libs.append( import_mod.LIBS )
                if (hasattr (import_mod, 'ARCHIVES')):
                    common_libs.append( import_mod.ARCHIVES )

                # Import libpath
                lib_path =  ''
                # common_lib_path.append( lib_path )

                # Get the library path tree (cannot be a list)
                if (hasattr(import_mod, 'LIBRARY_TREE')):
                    valueItem = import_mod.LIBRARY_TREE[tableKey]
                    lib_path = ver_dir + "/" + valueItem
                    common_lib_path.append (lib_path)

                # Copy libraries (file/symlink) to the installation directory    
                # But not archives - leave them
                files = os.listdir(lib_path)
                if (hasattr(import_mod, 'COPY_LIBS')):
                    for libName in import_mod.COPY_LIBS:
                        for file in files:
                            tmp = re.match( libName, file )
                            if ( tmp != None ):
                                copySharedLibrary(env, file, lib_path, env['CXL_lib_dir'])
                else:
                    for file in files:
                        tmp = re.match( ".*\.a", file )
                        if ( tmp == None ):
                            copySharedLibrary(env, file, lib_path, env['CXL_lib_dir'])

                if ( hasattr(import_mod, 'DATA_SRC') ) and ( hasattr(import_mod, 'DATA_DST') ) :
                    src = ver_dir + "/" + import_mod.DATA_SRC
                    dst = env['CXL_Data_dir'] + "/" + import_mod.DATA_DST
                    if  os.path.exists(dst):
                        shutil.rmtree (dst)
                    if import_mod.DATA_SRC != "":
                        shutil.copytree( src, dst, symlinks = True)
                        if env['CXL_build_verbose'] != 0 :
                            print ("Note: Copy data :")
                            print ("from : " + src )
                            print ("to   : " + dst )

    env.Append( CPPPATH = common_inc_path )
    env.Append( LIBS = common_libs )
    env.Append( LIBPATH = common_lib_path )


# An init function for the "Live" components (non-promoted)
# If the caller passes one of these in, we definitely use it.
# It is the responsibility of the caller to ensure that a component is not both
# listed as a promotion and a common project; the LIBPATH (for # purposes of linking)
# is the responsibility of the caller
# This mechanism enables a project to override the promotion model for a component
def initCommonProjects (env, ProjNameList) :
    if (env['CXL_common_dir'] == ""):
        print "Error: CXL_common_dir not specified"
        return
    
    if (env['CXL_commonproj_dir'] == ""):
        print "Error: CXL_commonproj_dir not specified"
        return

    initCommonLibAmd (env, ProjNameList)
