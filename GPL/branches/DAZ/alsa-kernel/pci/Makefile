!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

!include $(ROOT)\tools\header.mif

WMAKEOPTS=-h $(__MAKEOPTS__) KEE=$(KEE) DEBUG=$(DEBUG)

#===================================================================
#
#   List of source files
#
#===================================================================
FILES = ens1370.obj ens1371.obj cs4281.obj  es1938.obj &
  es1968.obj intel8x0.obj maestro3.obj via82xx.obj &
  fm801.obj atiixp.obj cmipci.obj als4000.obj bt87x.obj &
  ak4531_codec.obj cs5530.obj

#pci_iomap_compat.obj

TARGET = pci

LNKFILE = $(OBJDIR)\$(TARGET).lnk


#===================================================================
#
#   Specific dependencies
#
#===================================================================
all: $(OBJDIR)\$(TARGET).lib hda ac97 ali5451 cs46xx cs5535audio emu10k1 ca0106 nm256 trident ymfpci


$(LNKFILE): Makefile
    @%create $(LNKFILE)
    @for %f in ($(FILES)) do @%append $(LNKFILE) -+$(OBJDIR)\%f

$(OBJDIR)\$(TARGET).lib: $(LNKFILE) $(FILES)
    $(LIB) $(OBJDIR)\$(TARGET).lib @$(LNKFILE)
    copy $(OBJDIR)\$(TARGET).lib $(ALSA_LIB)


hda: .SYMBOLIC
    @echo BUILDING hda
    @cd hda
    @wmake $(WMAKEOPTS)
    @cd ..

ac97: .symbolic
    @echo BUILDING ac97
    @cd ac97
    @wmake $(WMAKEOPTS)
    @cd ..

ali5451: .symbolic
    @echo BUILDING ali5451
    @cd ali5451
    @wmake $(WMAKEOPTS)
    @cd ..

cs46xx: .symbolic
    @echo BUILDING cs46xx
    @cd cs46xx
    @wmake $(WMAKEOPTS)
    @cd ..

cs5535audio: .symbolic
    @echo BUILDING cs5535audio
    @cd cs5535audio
    @wmake $(WMAKEOPTS)
    @cd ..

emu10k1: .symbolic
    @echo BUILDING emu10k1
    @cd emu10k1
    @wmake $(WMAKEOPTS)
    @cd ..

ca0106: .symbolic
    @echo BUILDING ca0106
    @cd ca0106
    @wmake $(WMAKEOPTS)
    @cd ..

nm256: .symbolic
    @echo BUILDING nm256
    @cd nm256
    @wmake $(WMAKEOPTS)
    @cd ..

trident: .symbolic
    @echo BUILDING trident
    @cd trident
    @wmake $(WMAKEOPTS)
    @cd ..

ymfpci: .symbolic
    @echo BUILDING ymfpci
    @cd ymfpci
    @wmake $(WMAKEOPTS)
    @cd ..

#we don't build this
rme9653: .symbolic
    @echo BUILDING rme9652
    @cd rme9652
    @wmake $(WMAKEOPTS)
    @cd ..

#we don't build this
vortex: .symbolic
    @echo BUILDING vortex
    @cd au88x0
    @wmake $(WMAKEOPTS)
    @cd ..

#we don't build this
wavefront: .symbolic
    @echo BUILDING wavefront
    @cd wavefront
    @nmake $(WMAKEOPTS)
    @cd ..


clean: .SYMBOLIC
    @echo CLEANING hda
    @cd hda
    @wmake $(WMAKEOPTS) clean
    @cd ..
    @echo CLEANING ac97
    @cd ac97
    @wmake $(WMAKEOPTS) clean
    @cd ..
    @echo CLEANING ali5451
    @cd ali5451
    @wmake $(WMAKEOPTS) clean
    @cd ..
    @echo CLEANING cs46xx
    @cd cs46xx
    @wmake $(WMAKEOPTS) clean
    @cd ..
    @echo CLEANING cs5535audio
    @cd cs5535audio
    @wmake $(WMAKEOPTS) clean
    @cd ..
    @echo CLEANING emu10k1
    @cd emu10k1
    @wmake $(WMAKEOPTS) clean
    @cd ..
    @echo CLEANING ca0106
    @cd ca0106
    @wmake $(WMAKEOPTS) clean
    @cd ..
#    @echo CLEANING korg1212
#    @cd korg1212
#    @wmake $(WMAKEOPTS) clean
#    @cd ..
    @echo CLEANING nm256
    @cd nm256
    @wmake $(WMAKEOPTS) clean
    @cd ..
#    @echo CLEANING rme9652
#    @cd rme9652
#    @wmake $(WMAKEOPTS) clean
#    @cd ..
    @echo CLEANING trident
    @cd trident
    @wmake $(WMAKEOPTS) clean
    @cd ..
    @echo CLEANING ymfpci
    @cd ymfpci
    @wmake $(WMAKEOPTS) clean
    @cd ..
#    @echo CLEANING vortex
#    @cd au88x0
#    @wmake $(WMAKEOPTS) clean
#    @cd ..
#    @echo CLEANING wavefront
#    @cd wavefront
#    @nmake $(WMAKEOPTS) clean
#    @cd ..
    -@rm *.err
    @if exist obj @rm.exe -r obj
    @if exist bin @rm.exe -r bin

