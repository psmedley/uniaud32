!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

CINCLUDES = ..\..\include;..\include;$(%WATCOM)\H
AINCLUDES = $(DDK)\base\H

CDEFINES = -D__KERNEL__ -DMODULE -dTARGET_OS2 -dALSA_BUILD -D__ISAPNP__ -DCONFIG_ISAPNP -D__i386_

!include $(ROOT)\tools\header.mif

WMAKEOPTS=-h $(__MAKEOPTS__) KEE=$(KEE) DEBUG=$(DEBUG)

#===================================================================
#
#   List of source files
#
#===================================================================
FILES = util_mem.obj

TARGET = synth

LNKFILE = $(OBJDIR)\$(TARGET).lnk


#===================================================================
#
#   Specific dependencies
#
#===================================================================
all: $(OBJDIR)\$(TARGET).lib subdirs


$(LNKFILE): Makefile
    @%create $(LNKFILE)
    @for %f in ($(FILES)) do @%append $(LNKFILE) -+$(OBJDIR)\%f

$(OBJDIR)\$(TARGET).lib: $(LNKFILE) $(FILES)
    $(LIB) $(OBJDIR)\$(TARGET).lib @$(LNKFILE)
    copy $(OBJDIR)\$(TARGET).lib $(ALSA_LIB)

subdirs: .SYMBOLIC
    @echo BUILDING emux
    @cd emux
    @wmake $(WMAKEOPTS)
    @cd ..

clean: .SYMBOLIC
    @echo CLEANING emux
    @cd emux
    @wmake $(WMAKEOPTS) clean
    @cd ..
    -@rm *.err
    @if exist obj @rm.exe -r obj
    @if exist bin @rm.exe -r bin

