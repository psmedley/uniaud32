!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

!include $(ROOT)\tools\header.mif

WMAKEOPTS=-h $(__MAKEOPTS__)

!if "$(DEBUG)" == "1"
WMAKEOPTS += DEBUG=1
!endif

!if "$(KEE)" == "0"
WMAKEOPTS += KEE=0
KEE=0
!else
WMAKEOPTS += KEE=1
KEE=1
!endif

#===================================================================
#
#   List of source files
#
#===================================================================
FILES    = pcm_oss.obj

TARGET   = core_oss

LNKFILE  = $(OBJDIR)\$(TARGET).lnk


#===================================================================
#
#   Specific dependencies
#
#===================================================================
all: $(OBJDIR)\$(TARGET).lib


$(LNKFILE): Makefile
    @%create $(LNKFILE)
    @for %f in ($(FILES)) do @%append $(LNKFILE) -+$(OBJDIR)\%f

$(OBJDIR)\$(TARGET).lib: $(LNKFILE) $(FILES)
    $(LIB) $(OBJDIR)\$(TARGET).lib @$(LNKFILE)
    copy $(OBJDIR)\$(TARGET).lib $(ALSA_LIB)

# we don't build this
instr: .SYMBOLIC
    @echo BUILDING instr
    @cd instr
    @wmake $(WMAKEOPTS)
    @cd ..

clean: .SYMBOLIC
#    @echo BUILDING instr
#    @cd instr
#    @wmake $(WMAKEOPTS) clean
#    @cd ..
    -@rm *.err
    @if exist obj @rm.exe -r obj
    @if exist bin @rm.exe -r bin

