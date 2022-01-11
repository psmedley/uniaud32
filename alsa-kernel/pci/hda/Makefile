!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

!include $(ROOT)\tools\header.mif


#===================================================================
#
#   List of source files
#
#===================================================================
FILES = hda_intel.obj hda_codec.obj hda_controller.obj hda_auto_parser.obj hda_jack.obj &
  hda_proc.obj hda_generic.obj hda_hwdep.obj hda_sysfs.obj hda_bind.obj hda_eld.obj &
  patch_analog.obj patch_cmedia.obj patch_conexant.obj patch_ca0110.obj patch_ca0132.obj patch_cirrus.obj &
  patch_hdmi.obj patch_realtek.obj patch_si3054.obj patch_sigmatel.obj patch_via.obj

TARGET = hda

LNKFILE = $(OBJDIR)\$(TARGET).lnk


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

clean: .SYMBOLIC
    -@rm *.err
    @if exist obj @rm.exe -r obj
    @if exist bin @rm.exe -r bin

