#########################################################
# These envrionment variable should be set in the system
# Note:
#	COMMON_DIR = 
#	COMMONPROJECTS_DIR = 
# The default is to use -m64 if a generic version of GCC is used by a makefile

# put common definitions in here
INC		= -I$(COMMON_DIR)/Src/CommonProjectsHdr
OUTDIR		= ../Output/Release/bin

CC		= gcc
CXXFLAGS	= -fPIC -c

LD32		= ld
LD64		= ld
LD		= ld
LDFLAGS		=

GPP32		= g++ -m32
GPP64		= g++ -m64
GPP		= g++ -m64

GCC32		= gcc -m32
GCC64		= gcc -m64
GCC		= gcc -m64
CFLAGS		=

DEBUG		= -g
