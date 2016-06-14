import os

#####################################################
# sort command line options
def ParseCommandLine(env, buildInternal):
    env.archLinux = ""
    env.archWin = ""
    print "*************************"
    if env.buildConfig == 'Debug' or env.buildConfig == 'debug':
        print "This is a debug build"
        env.Append(CCFLAGS = '-g3')
        env.Append(CCFLAGS = '-D_DEBUG')
    else:
        print "This is a release build"
        env.Append(CCFLAGS = '-O3')
        env.Append(CCFLAGS = '-DNDEBUG')
        env.buildConfig = 'Release'

    if env.archConfig == 'x86':
        print "x86 (32 bit) build"
        env.Append(CCFLAGS = '-m32')
        env.Append(LINKFLAGS = '-m32')
        env.extConfig = 'x86'
        env.gnuConfig = 'i386-linux-gnu'
        env.libConfig = 'lib32'
        env.libGPA = 'libGPUPerfAPIGL32' + env.buildSuffix + '.so'
        env.libGPA_GLES = 'libGPUPerfAPIGLES32' + env.buildSuffix + '.so'
        env.archLinux = '32'
    else:
        print "x64 (64 bit) build"
        env.Append(CCFLAGS = '-m64')
        env.Append(CCFLAGS = '-DX64')
        env.Append(LINKFLAGS = '-m64')
        env.archConfig = 'x64'
        env.extConfig = 'x86_64'
        env.gnuConfig = 'x86-64-linux-gnu'
        env.libConfig = 'lib'
        env.libGPA = 'libGPUPerfAPIGL' + env.buildSuffix + '.so'
        env.libGPA_GLES = 'libGPUPerfAPIGLES' + env.buildSuffix + '.so'
        env.archWin = '-x64'

    if (buildInternal == True):
        print "Internal Build"
    print "*************************"

#####################################################
# initialize Gtk library
def initGtk(env):
    env.Append(BASE_PATH = [
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
   ])

##########################################################################################
# copy GPA shared library into target folder and change permissions so execute flag is set
def CopyGPA(env, fileName, platformTarget):
    command = "cp ../Common/Lib/AMD/GPUPerfAPI/"
    command = command + env.PerfAPIVersion
    command = command + "/Bin"
    command = command + env.buildSuffix
    command = command + "/Lin"
    command = command + env.archConfig
    command = command + "/"
    command = command + fileName
    command = command + " "
    command = command + platformTarget
    os.system(command)

    command = "chmod 775 "
    command = command + platformTarget
    command = command + "/"
    command = command + fileName
    os.system(command)

##########################################################################################
# Copy JSON files required by the Vulkan server
# Share the windows JSON files to minimize maintainance. Once the files are copied, they
# are modified to work with the Linux server. Uses the 'sed' utility to search and replace
# strings
def CopyJSON(env, pluginsTarget):
    source = "Server/VulkanServer/JSON/VulkanServer" + env.archWin + env.debugSuffix + env.buildSuffix + ".json"
    dest = pluginsTarget + "/VulkanServer" + env.archLinux + env.debugSuffix + env.buildSuffix + ".json"

    command = "cp " + source + " " + dest
    os.system(command)

    # make target JSON file read/write
    command = "chmod 664 " + dest
    os.system(command)

    # replace instances of '-x64' with ''
    command = "sed -i 's/-x64//g' " + dest
    os.system(command)

    # replace 2 backslashes with single forward slash
    command = "sed -i 's/\\\\\\\\/\//g' " + dest
    os.system(command)

    # replace 'dll' with 'so'
    command = "sed -i 's/dll/so/g' " + dest
    os.system(command)

    # append '32' to Server if 32-bit server
    if env.archConfig == 'x86':
        command = "sed -i 's/Server/Server" + env.archLinux + "/g' " + dest
        os.system(command)

#####################################################
# Function to copy support files to the target folder
def CopySupportFiles(env):

    configTarget = env.targetRoot
    pluginsTarget = configTarget + "/Plugins"
    imagesTarget = configTarget + "/Images"

    # create the directories if they don't exist
    if not os.path.exists(configTarget):
        os.makedirs(configTarget)

    if not os.path.exists(pluginsTarget):
        os.makedirs(pluginsTarget)

    if not os.path.exists(imagesTarget):
        os.makedirs(imagesTarget);

    # delete any old binaries in the bin folder. Use shell rather than
    # os.remove since it's easier to deal with wildcards
    rmcmd = "rm -f "
    rmcmd = rmcmd + configTarget
    command = rmcmd + "/libGPUPerfAPIGL" + env.platformSuffix + ".so"
    os.system(command)

    command = rmcmd + "/libGPUPerfAPIGLES" + env.platformSuffix + ".so"
    os.system(command)

    command = rmcmd + "/GPUPerfServer" + env.platformSuffix + env.debugSuffix
    os.system(command)

    command = rmcmd + "/Plugins/GLServer" + env.platformSuffix + env.debugSuffix + ".so"
    os.system(command)

    command = rmcmd + "/Plugins/GLESServer" + env.platformSuffix + env.debugSuffix + ".so"
    os.system(command)

    command + rmcmd + "/Plugins/VulkanServer" + env.platformSuffix + env.debugSuffix + ".so"
    os.system(command)

    # copy script files into bin folder
    command = "cp -n Server/Linux/*.sh "
    command = command + configTarget
    os.system(command)

    # copy README file into bin folder. Make it writable first
    destReadme = configTarget + "/README"
    if os.path.exists(destReadme):
        command = "chmod 664 "
        command = command + destReadme
        os.system(command)
    command = "cp Server/Linux/README "
    command = command + configTarget
    os.system(command)

    #copy thirdpartylicenses.txt file into bin folder
    destLicense = configTarget + "/thirdpartylicenses.txt"
    if os.path.exists(destLicense):
        command = "chmod 664 "
        command = command + destLicense
        os.system(command)
    command = "cp Server/thirdpartylicenses.txt "
    command = command + configTarget
    os.system(command)

    # copy media files into target folder
    command = "cp -n Server/media/*.png "
    command = command + imagesTarget
    os.system(command)

    # copy config file into target folder
    command = "cp -n Server/Linux/*.cfg "
    command = command + configTarget
    os.system(command)

    # copy GPA shared libraries into target folder
    CopyGPA(env, env.libGPA, configTarget)
    CopyGPA(env, env.libGPA_GLES, configTarget)

    # copy JSON files for Vulkan server
    CopyJSON(env, pluginsTarget)

