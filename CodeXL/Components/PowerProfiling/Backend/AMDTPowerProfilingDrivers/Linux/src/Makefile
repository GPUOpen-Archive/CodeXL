TARGET = amdtPwrProf
DEBUG = 0
ifeq (${DEBUG}, 1)
EXTRA_CFLAGS=-I$(CURDIR)/../inc  -I$(CURDIR)/../inc/Smu8Header  -I$(CURDIR)/../../inc -I$(CURDIR)/../../common -I$(CURDIR)/../../Non-OpenSource -DKERNEL_MODULE -g -DLINUX -D_LINUX -DDEBUG
else
EXTRA_CFLAGS=-I$(CURDIR)/../inc -I$(CURDIR)/../../inc -I$(CURDIR)/../../common -I$(CURDIR)/../../Non-OpenSource -DKERNEL_MODULE -DLINUX -D_LINUX -g 
endif

obj-m += $(TARGET).o

${TARGET}-objs :=  AMDTPwrProfHwaccess.o AMDTPwrProfHelper.o ../../common/AMDTSmu7Interface.o ../../common/AMDTSmu8Interface.o ../../common/AMDTAccessPmcData.o ../../common/AMDTCounterAccessInterface.o ../../common/AMDTRawDataFile.o ../../common/AMDTCommonConfig.o ../../common/AMDTHwAccessInterface.o ../../common/AMDTPwrCommonHelper.o AMDTPwrProfTimer.o AMDTPwrProfEntry.o AMDTPwrProfCoreUtils.o AMDTPwrProfModule.o AMDTPwrProfTimerHelper.o AMDTPwrProfSharedMemOps.o ../../Non-OpenSource/AMDTSmu9Interface.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) $(MAKE_OPTS) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(CURDIR) $(MAKE_OPTS) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" clean
