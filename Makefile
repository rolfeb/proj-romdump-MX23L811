# vi: noexpandtab shiftwidth=8 softtabstop=8

AVR_ROOT	=	../avr-common

NAME		=	romdump

#
# Microcontroller definitions
#
MCU		=	atxmega16a4

#
# 10MHz clock
#
#CPU_FREQ	=	10000000
CPU_FREQ	=	1843200

#
# HFUSE = 0x99
#	JTAGEN		1	JTAG disabled
HFUSE		=	0xd9
#
# LFUSE = 0xe7
#	CKDIV8		1	no divide by 8
#	CKOUT		1	clock output off
#	SUT1:0		10	slow rising power
#	CKSEL3:0	0111	crystal oscillator
LFUSE		=	0xe7

#
# Required application components
#
MODULES		=

LDFLAGS_EXTRA	+=      -Wl,-u,vfprintf -lprintf_flt -lm

#
# Application code
#
CFILES		=	\
			serial.c \
			main.c

#
# Load standard rules
#
include $(AVR_ROOT)/build/avr-build.mk

AVRDUDE	= $(AVRDUDE_JTAGPDI)


# DO NOT DELETE

