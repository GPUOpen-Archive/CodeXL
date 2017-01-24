# Copyright (c) 2015 Advanced Micro Devices, Inc.  All Rights Reserved

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
 
# Module Name
MODULE_NAME=amdtPwrProf
# Module version
MODULE_VERSION=$(shell cat CodeXLPwrProfVersion)

# Module after compilation
MODULE_NAME_KO=$(MODULE_NAME).ko

# check is module inserted
MODPROBE_OUTPUT=$(shell lsmod | grep $(MODULE_NAME))

# check pcore dkms status
PCORE_DKMS_STATUS=$(shell dkms status | grep $(MODULE_NAME) | grep $(MODULE_VERSION))

# Compilation flags
EXTRA_CFLAGS=-I$(PWD)/inc -DKERNEL_MODULE -DLINUX -D_LINUX
# Compilation flags with debug enabled
# EXTRA_CFLAGS=-I$(PWD)/inc -DKERNEL_MODULE -g -DLINUX -D_LINUX -DDEBUG

# DKMS path
PCORE_DKMS_PATH=/usr/src/$(MODULE_NAME)-$(MODULE_VERSION)

# Kernel Version
ifeq ($(KERNEL_VERSION),)
KERNEL_VERSION=$(shell uname -r)
endif

# Module needs to be compiled
obj-m += $(MODULE_NAME).o

${MODULE_NAME}-objs :=  src/AMDTPwrProfHwaccess.o src/AMDTPwrProfHelper.o src/AMDTSmu7Interface.o src/AMDTSmu8Interface.o src/AMDTCounterAccessInterface.o src/AMDTRawDataFile.o src/AMDTCommonConfig.o src/AMDTAccessPmcData.o src/AMDTHwAccessInterface.o src/AMDTPwrProfTimer.o src/AMDTPwrProfEntry.o src/AMDTPwrProfCoreUtils.o src/AMDTPwrProfModule.o src/AMDTPwrProfTimerHelper.o  src/AMDTPwrProfSharedMemOps.o src/AMDTSmu9Interface.o src/AMDTPwrCommonHelper.o

# make
all:
	make -C /lib/modules/$(KERNEL_VERSION)/build M=$(PWD) $(MAKE_OPTS) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

# make clean
clean:
	make -C /lib/modules/$(KERNEL_VERSION)/build M=$(PWD) $(MAKE_OPTS) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" clean

# make install
install:
	@mkdir -p /lib/modules/`uname -r`/kernel/drivers/extra
	@rm  -f /lib/modules/`uname -r`/kernel/drivers/extra/$(MODULE_NAME_KO)
	@cp $(MODULE_NAME_KO) /lib/modules/`uname -r`/kernel/drivers/extra/
	@depmod -a
	@if [ ! -z "$(MODPROBE_OUTPUT)" ]; then \
		echo "Uninstalling AMD CodeXL Power Profiler Linux kernel module.";\
		rmmod $(MODULE_NAME);\
	fi
	@modprobe $(MODULE_NAME)

# make dkms
dkms:
	@echo "Installing AMD CodeXL Power Profiler Linux kernel module version $(MODULE_VERSION)."
	@sed -i -e '/^PACKAGE_VERSION=/ s/=.*/=\"$(MODULE_VERSION)\"/' dkms.conf
	@dkms add -m $(MODULE_NAME) -v $(MODULE_VERSION) -q
	@dkms build -m $(MODULE_NAME) -v $(MODULE_VERSION) -q
	@dkms install --force -m $(MODULE_NAME) -v $(MODULE_VERSION) -q
	@echo "Loading AMD CodeXL Power Profiler Linux kernel module."
	@modprobe $(MODULE_NAME)
	
# make cleandkms
cleandkms:
	@if [ ! -z "$(MODPROBE_OUTPUT)" ]; then \
		echo "Uninstalling AMD CodeXL Power Profiler Linux kernel module.";\
		rmmod $(MODULE_NAME);\
	fi
	@if [ ! -z "$(PCORE_DKMS_STATUS)" ]; then \
	    echo "Removing AMD CodeXL Power Profiler Linux kernel module.";\
	    dkms remove -m $(MODULE_NAME) -v $(MODULE_VERSION) --all -q;\
	fi

# make rebuilddkms
rebuilddkms:
	$(MAKE) cleandkms; \
	$(MAKE) dkms
