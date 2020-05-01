#
# libn64/Makefile: Makefile for libn64.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

ifdef SystemRoot
FIXPATH = $(subst /,\,$1)
RM = del /Q
CP = copy
else
FIXPATH = $1
RM = rm -f
CP = cp
endif

AS = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-as)
AR = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-gcc-ar)
CC = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-gcc)
CPP = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-cpp)
LD = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-ld)
OC = $(call FIXPATH,$(CURDIR)/../tools/bin/mips64-elf-objcopy)

RSPASM = $(call FIXPATH,$(CURDIR)/../tools/bin/rspasm)

CFLAGS = -Wall -Wextra -pedantic -std=c99 -I. -Iinclude  -save-temps
OPTFLAGS = -Os -march=vr4300 -mtune=vr4300 -mabi=eabi -mgp32 -mlong32 \
	-flto -ffat-lto-objects -ffunction-sections -fdata-sections \
	-G4 -mno-extern-sdata -mgpopt -mfix4300 -mbranch-likely \
	-mno-check-zero-division

ASMFILES = $(call FIXPATH,\
)

CFILES = $(call FIXPATH,\
	src/joy64.c \
	src/joy64_port.c \
	src/joy64_auto.c \
)

UCODES = $(call FIXPATH,\
)

OBJFILES = \
	$(CFILES:.c=.o) \
	$(ASMFILES:.s=.o) \
	$(UCODES:.rsp=.o)

DEPFILES = $(OBJFILES:.o=.d)

#
# Primary targets.
#
libjoy64.a: $(OBJFILES)
	@echo $(call FIXPATH,"Building: libjoy64/$@")
	@$(AR) rcs $@ $^
	@$(CP) $(call FIXPATH, include/joy64.h) $(call FIXPATH, ../include)
	@$(CP) $(call FIXPATH, include/joy_keyboard64.h) $(call FIXPATH, ../include)
	@$(CP) $(call FIXPATH, libjoy64.a) $(call FIXPATH, ../lib)

#
# Generic compilation/assembly targets.
#




%.o: %.s
	@echo $(call FIXPATH,"Assembling: libjoy64/$<")
	@$(CC) -x assembler-with-cpp $(CFLAGS) $(OPTFLAGS) -MMD -c $< -o $@

%.o: %.c 
	@echo $(call FIXPATH,"Compiling: libjoy64/$<")
	@$(CC) $(CFLAGS) $(OPTFLAGS) -MMD -c $< -o $@
	

%.o: %.rsp %.rsps
	@echo $(call FIXPATH,"Assembling: $(ROM_NAME)/$@")
	@$(CPP) -E -Iucodes $< > $(<:.rsp=.rsppch)
	@$(RSPASM) $(<:.rsp=.rsppch) -o $(<:.rsp=.bin)
	@$(CC) -x assembler-with-cpp $(CFLAGS) $(OPTFLAGS) -MMD -c $(<:.rsp=.rsps) -o $@

#
# Clean project target.
#
.PHONY: clean
clean:
	@echo "Cleaning libjoy64..."
	@$(RM) libjoy64.a $(DEPFILES) $(OBJFILES)
	@$(RM) *.d $(DEPFILES) $(OBJFILES)
	@$(RM) *.o $(DEPFILES) $(OBJFILES)