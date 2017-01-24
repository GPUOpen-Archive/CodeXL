#! /bin/bash

check_exe_permission() {
    # script can be executed only with root permission.
    if [ "$(id -u)" != "0" ]; then
	echo "Please execute this script with root permission."
	exit 1
    fi
}

module_in_use() {
    # check if proc entry exists for state
    if [ -f /proc/amdtPwrProf/state ] 
    then
        # if state is set then module is in use
        MOD_IN_USE=`cat /proc/amdtPwrProf/state`
        if [ $MOD_IN_USE -eq "1" ]
        then
	    echo "Unable to $1 CodeXL Power Profiler, module is in use."
            echo "cat /proc/amdtPwrProf/state will show status of the module, 1 if module is in use."
            echo "Please run the CodeXLPwrProfDriver.sh script to $1, when the power profiler is not in use."
            exit 1
        fi
    fi

    # compatiblity with prior versions, as /proc/amdtPwrProf/state file does not exist
    count=`lsmod | grep -e amdtPwrProf -e pcore  | awk '{print $NF}'`
    if [ -n "$count" ]
    then 
        if [ "$count" -ne "0" ]
        then
            echo "Unable to $1 CodeXL Power Profiler, module is in use."
            echo "Please run the CodeXLPwrProfDriver.sh script to $1, when the power profiler is not in use."
            exit 1
        fi
    fi
}

set_module_name () {
    # Module name.
    MODULE_NAME=$1
}

set_version_name() {
    # Module version.
    MODULE_VERSION=$(cat CodeXLPwrProfVersion)
}

set_src_path() {
    # Source path.
    MODULE_SOURCE_PATH=/usr/src/$MODULE_NAME-$MODULE_VERSION
}


prompt_if_suse_unsupported () {
    # for SuSE, allow_unsupported_modules flag should be set,
    # to install 3rd Party driver/module. 
    if [ -f /etc/SuSE-release ]; then
	count=`cat /etc/modprobe.d/unsupported-modules | grep "^allow_unsupported_modules 0" | wc -l`
	# if allow_unsupported_modules == 0, set it to 1.
	if [ $count -gt 0 ]
	then
	    echo 'Cannot install CodeXL Power Profiler.'
	    echo 'Use --allow-unsupported or set allow_unsupported_modules to 1 in /etc/modprobe.d/unsupported-modules for installation.'
	    exit 1
	fi
    fi
}

add_src () {
    # untar the file and install it in /usr/src/...
    tar -zxf CodeXLPwrProfDriverSource.tar.gz -C /usr/src/ 
}

install_dkms () {
    # DKMS is installed.
    MODULE_SOURCE=`pwd`

    cd $MODULE_SOURCE_PATH 

    # uninstall dkms module for pcore
    make cleandkms
    if [ $? -ne 0 ] ; then
	echo "ERROR: CodeXL Power Profiler driver installation failed while uninstalling the previously installed driver." 
	exit 1
    fi

    # install dkms module for pcore
    make dkms
    if [ $? -ne 0 ] ; then
	echo "ERROR: CodeXL Power Profiler driver installation failed." 
	exit 1
    fi

    cd $MODULE_SOURCE 
}

install_non_dkms () {
    # DKMS not installed 
    cd $MODULE_SOURCE_PATH 

    # cleanup
    make clean
    if [ $? -ne 0 ] ; then
	echo "ERROR: CodeXL Power Profiler driver build failed while uninstalling the previously installed driver." 
	exit 1
    fi

    make 
    if [ $? -ne 0 ] ; then
	echo "ERROR: CodeXL Power Profiler driver build failed." 
	exit 1
    fi

    make install
    if [ $? -ne 0 ] ; then
	echo "ERROR: CodeXL Power Profiler driver installation failed." 
	exit 1
    fi

    cd -
}

install_mod() {
    DKMS=`which dkms 2>/dev/null`
    if [ -n "$DKMS" ]
    then
	install_dkms
    else
	install_non_dkms
    fi
}

create_proc_entry () {
    #check if module is loaded 
    if [ -f /proc/${MODULE_NAME}/device ] 
    then
	#Module loaded, Create the device file
	VER=`cat /proc/${MODULE_NAME}/device`
	if [[ ! -a /dev/${MODULE_NAME} ]]
	then
	    mknod /dev/${MODULE_NAME} -m 666 c $VER 0
	    if [ $? -ne 0 ] ; then
		echo "ERROR: CodeXL Power Profiler driver installation failed while creating device file." 
		exit 1
	    fi
	fi
    else
	echo "ERROR: CodeXL Power Profiler driver installation failed while loading driver." 
	exit 1 
    fi
}

add_module_entry () {
    # Make the module to be loaded on reboot
    if [ -f /etc/redhat-release ]
    then
	# RHEL/CENTOS system 
	# 1. Add the modprobe command to /etc/rc.modules
	count=`grep ${MODULE_NAME} /etc/rc.modules | wc -l`
	if [ ! $count -gt 0 ]
	then
	    echo modprobe ${MODULE_NAME} >> /etc/rc.modules
	    chmod +x /etc/rc.modules
	fi

	count=`grep ${MODULE_NAME} /etc/rc.local | wc -l`
	if [ ! $count -gt 0 ]
	then
	    # Add a command in the rc.local file to create the device file on reboot
	    echo mknod /dev/${MODULE_NAME} -m 666 c \`cat /proc/${MODULE_NAME}/device\` 0 >> /etc/rc.local

	    # For RHEL 7
	    rhelVer=$(awk -F" " '{print $7}' /etc/redhat-release | grep 7.0 | wc -l)
	    if [  $rhelVer -gt 0 ]
	    then
		chmod +x /etc/rc.d/rc.local
	    fi
	fi
    elif [ -f /etc/SuSE-release ]
    then
	# for SuSE, 
	# modules to be loaded once the main filesystem is active
	count=`grep ${MODULE_NAME} /etc/sysconfig/kernel | wc -l`
	if [ ! $count -gt 0 ]
	then
	    sed -i "s/.*MODULES_LOADED_ON_BOOT.*/&\nMODULES_LOADED_ON_BOOT=\"${MODULE_NAME}\"/" /etc/sysconfig/kernel
	fi

	# commands to be executed from init on system startup
	count=`grep ${MODULE_NAME} /etc/rc.d/boot.local | wc -l`
	if [ ! $count -gt 0 ]
	then
	    echo mknod /dev/${MODULE_NAME} -m 666 c \`cat /proc/${MODULE_NAME}/device\` 0 >> /etc/rc.d/boot.local 
	fi
    else
	# Ubuntu OS
	# modules to be loaded once the main filesystem is active
	count=`grep ${MODULE_NAME} /etc/modules | wc -l`
	if [ ! $count -gt 0 ]
	then
	    echo ${MODULE_NAME} >> /etc/modules
	fi

	# commands to be executed from init on system startup
	count=`grep ${MODULE_NAME} /etc/rc.local | wc -l`
	if [ ! $count -gt 0 ]
	then
	    sed -i '/exit 0/d' /etc/rc.local
	    echo mknod /dev/${MODULE_NAME} -m 666 c \`cat /proc/${MODULE_NAME}/device\` 0 >> /etc/rc.local
	    echo exit 0 >> /etc/rc.local 
	fi
    fi
}

uninstall_dkms() {
    # DKMS is installed
    MODULE_SOURCE=`pwd`
    if [[ -d $MODULE_SOURCE_PATH ]]; then
        cd $MODULE_SOURCE_PATH 

        # uninstall dkms entry 
        make cleandkms
        if [ $? -ne 0 ] ; then
	    echo "ERROR: Uninstalling the CodeXL Power Profiler driver failed." 
	    exit 1
        fi
        cd $MODULE_SOURCE 
    else
	rmmod ${MODULE_NAME}
    fi
}

uninstall_module() {

    # remove module.
    KO="/lib/modules/`uname -r`/kernel/drivers/extra/${MODULE_NAME}.ko"
    if [ -f ${KO} ]
    then
	rm  -f ${KO}
    fi

    # remove module from dkms path 
    DKMS_KO="/lib/modules/`uname -r`/updates/dkms/${MODULE_NAME}.ko"
    if [ -f ${DKMS_KO} ]
    then
	rm  -f ${DKMS_KO}
    fi

    DEV="/dev/${MODULE_NAME}"
    count=`ls /dev | grep ${MODULE_NAME} | wc -l`
    if [ $count -gt 0 ]
    then
	rm  -f ${DEV}
    fi

    #load the existing kernel module if exists.
    count=`lsmod | grep ${MODULE_NAME} | wc -l`
    # if module installed.
    if [ $count -gt 0 ]
    then 

	DKMS=`which dkms 2>/dev/null`

	if [ -n "$DKMS" ]
	then
	    uninstall_dkms
	else
	    rmmod ${MODULE_NAME} 
	fi

    fi
}

delete_mod_entry() {
    # delete entry from REDHAT and Ubuntu
    if [ ! -f /etc/SuSE-release ]
    then
	count=`grep ${MODULE_NAME} /etc/rc.local | wc -l`
	if [ $count -gt 0 ]
	then
	    sed -i "/${MODULE_NAME}/d" /etc/rc.local
	fi
    fi

    if [ -f /etc/redhat-release ]
    then
	# RHEL/CENTOS system 
	# remove the modprobe command to /etc/rc.modules
	count=`grep ${MODULE_NAME} /etc/rc.modules | wc -l`
	if [ $count -gt 0 ]
	then
	    sed -i "/${MODULE_NAME}/d" /etc/rc.modules
	fi
    elif [ -f /etc/SuSE-release ]
    then
	# SuSE system
	# remove modules from /etc/sysconfig/kernel
	mod_found=`grep ${MODULE_NAME} /etc/sysconfig/kernel | wc -l`
	# module found
	if [ $mod_found -gt 0 ]
	then
	    sed -i "/MODULES_LOADED_ON_BOOT=\"${MODULE_NAME}\"/d" /etc/sysconfig/kernel
	fi

	# remove the modprobe command from /etc/rc.d/boot.local
	mod_found=`grep ${MODULE_NAME} /etc/rc.d/boot.local | wc -l`
	if [ $mod_found -gt 0 ]
	then
	    sed -i "/${MODULE_NAME}/d" /etc/rc.d/boot.local
	fi
    else
	# Ubuntu OS
	count=`grep ${MODULE_NAME} /etc/modules | wc -l`
	if [ $count -gt 0 ]
	then
	    sed -i "/${MODULE_NAME}/d" /etc/modules
	fi
    fi
}

delete_src() {
    # delete the PP module source folder
    rm -rf /usr/src/${MODULE_NAME}-*  &> /dev/null

    # in case of DKMS non existence module entried 
    # must be cleared from dkms source tree
    DKMS=`which dkms 2>/dev/null`
    if [ -n "$DKMS" ]
    then
        rm -rf sudo rm -rvf /var/lib/dkms/${MODULE_NAME}  &> /dev/null
    fi
}

common_uninstaller() {
    set_version_name
    set_src_path
    uninstall_module
    delete_mod_entry
    delete_src
}

uninstall_pcore() {
    set_module_name pcore
    common_uninstaller
}

uninstall_amdtPwrProf() {
    set_module_name amdtPwrProf 
    common_uninstaller
}

uninstall() {

    # earlier Power Profiler driver named pcore
    # uninstall if old module exists
    uninstall_pcore
    # uninstall power profiler current module
    uninstall_amdtPwrProf
}

install() {

    # set module name
    set_module_name amdtPwrProf 
    # set driver version number
    set_version_name
    # set driver sorce file path 
    set_src_path
    # prompt for SuSE version
    prompt_if_suse_unsupported
    # add source file in the source folder
    add_src
    # install module
    install_mod
    # create mknod entry
    create_proc_entry
    # add module entry, required after system reboot
    add_module_entry
}

verify_kernel_header() {
 
    if [ -f /etc/redhat-release ]
    then
        HEADER_SRC="/usr/src/kernels/`uname -r`"
        if [ ! -d ${HEADER_SRC} ]
        then
            echo "ERROR: Linux headers is required for installing CodeXL Power Profiler driver."
            echo "       Please install the sources using"
            tput bold    # put the text in bold
            echo "       sudo yum install kernel-devel"
            tput sgr0    #Reset text attributes to normal without clear.
            echo "       and then start the installation again."
            exit 1
        fi
    elif [ -f /etc/SuSE-release ]
    then
        # TODO:
        echo  1 > /dev/null
    else
        HEADER_SRC="/usr/src/linux-headers-`uname -r`"

        if [ ! -d ${HEADER_SRC} ]
        then
            echo "ERROR: Linux headers is required for installing CodeXL Power Profiler driver."
            echo "       Please install the sources using"
            tput bold    # put the text in bold
            echo "       sudo apt-get install linux-headers-$(uname -r)"
            tput sgr0    #Reset text attributes to normal without clear.
            echo "       and then start the installation again."
            exit 1
        fi
    fi
}

installer() {
 
    # verify if the linux-header source exists
    verify_kernel_header 
 
    # uninstall all the previous version installed
    uninstall

    # install the latest driver
    echo "Installing CodeXL Power Profiler driver."
    install
    echo "CodeXL Power Profiler driver installation completed successfully."
}

un_installer() {
    
    # uninstall all the previous version installed
    echo "Uninstalling the CodeXL Power Profiler driver."
    uninstall
    echo "CodeXL Power Profiler driver uninstallation completed successfully."
}

usage()  {
    echo "Usage:"
    echo "Install CodeXL Power Profiler:    sudo ./CodeXLPwrProfDriver.sh install"
    echo "Uninstall CodeXL Power Profiler:  sudo ./CodeXLPwrProfDriver.sh uninstall"
}

# check for root permission
check_exe_permission

# check if module in use
module_in_use $1

# number of arguments can not be greater then 2
if [ "$#" -ne 1 ] ; then
    echo "Invalid number of arguments"
    echo ""
    usage 
    exit 1
fi 

case $1 in
    install)
        # install driver
        installer
    ;;

    uninstall)
        # un install driver
        un_installer
    ;;

    *)
        # in appropiate input
        usage 
    ;;
esac 
