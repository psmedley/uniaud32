#
# Makefile for the 32bit UNIAUD driver
#
# (C) 2000-2002 InnoTek Systemberatung GmbH
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the Free
# Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
# USA.
#
!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

#Default packing on byte boundary
PACK_1=1
!include $(ROOT)\tools\header.mif
!include $(ROOT)\include\version.mak

#.ERASE
#
#.SUFFIXES:
#.SUFFIXES: .sys .obj .asm .inc .def .lrf .ref .lst .sym .map .c .cpp .h .lib

# note: Watcom headers must be before DDK headers to avoid conflicts
# with C runtime headers
%INCLUDE = $(%INCLUDE)$(DDK)\base\h;
AINCLUDES = -I:$(DDK)\base\INC

TARGET   = uniaud32
LNKFILE   = $(OBJDIR)\$(TARGET).lrf
DEFFILE   = $(OBJDIR)\$(TARGET).def
MAKEFILE  = Makefile

!if "$(IBM_BUILD)" == "1"
CDEFINES = -DIBM_BUILD
!endif


#===================================================================
#
#   List of source files
#
#===================================================================

FILES = begin.obj startup.obj &
  strategy.obj read.obj &
  init.obj parse.obj &
  devhlp.obj &
  idc.obj dispatch.obj &
  rmhelp.obj irq.obj util.obj &
  impdos.lib
!if "$(KEE)" == "1"
FILES += impkee.lib
!endif
FILES += end.obj

LIBS     = $(ALSA_LIB)\linuxlib.lib $(ALSA_LIB)\core.lib $(ALSA_LIB)\ac97.lib $(ALSA_LIB)\pci.lib  $(ALSA_LIB)\ca0106.lib $(ALSA_LIB)\hda.lib $(ALSA_LIB)\sequencer.lib $(ALSA_LIB)\opl3.lib $(ALSA_LIB)\mpu401.lib $(ALSA_LIB)\synth.lib $(ALSA_LIB)\cs46xx.lib $(ALSA_LIB)\cs5535audio.lib $(ALSA_LIB)\emu10k1.lib $(ALSA_LIB)\ymfpci.lib $(ALSA_LIB)\ali5451.lib $(ALSA_LIB)\trident.lib $(ALSA_LIB)\nm256.lib $(ALSA_LIB)\sb.lib $(ALSA_LIB)\core_oss.lib
WMAPNAME = $(OBJDIR)\$(TARGET).wmap

# $(ALSA_LIB)\au88xx.lib
#===================================================================
#
#   Specific dependencies
#
#===================================================================
all: $(OBJDIR)\$(TARGET).sys $(OBJDIR)\$(TARGET).sym

!ifdef USE_WLINK
$(LNKFILE): $(MAKEFILE)
  @%write $^@ option quiet
  @%write $^@ option verbose
  @%write $^@ option caseexact
  @%write $^@ option cache
  @%write $^@ option alignment=16
  @%write $^@ option map=$(WMAPNAME)
  @$(ROOT)\tools\AddToFile.cmd $^@,$option description,BLDLEVEL,$(BLDLVL_VENDOR),$(BLDLVL_VERSION),Universal Audio Driver for OS/2 (c) 2016 $(BLDLVL_VENDOR),$(BLDLVL_FIXPACK)
  @%write $^@ name $(OBJDIR)\$(TARGET).sys
  @for %f in ($(FILES)) do @%append $^@ file $(OBJDIR)\%f
  @for %f in ($(LIBS)) do @%append $^@ library %f
  @%write $^@ library $(WATCOM)\lib386\os2\clib3r.lib
  @%write $^@ library $(WATCOM)\lib386\os2\os2386.lib
!else
$(LNKFILE): $(MAKEFILE)
  @%write $^@ /OUT:$(OBJDIR)\$(TARGET).sys
  @%write $^@ /MAP:$(OBJDIR)\$(TARGET).map
  @for %f in ($(FILES)) do @%append $^@ $(OBJDIR)\%f
  @for %f in ($(LIBS)) do @%append $^@ %f
  @%write $^@ $(WATCOM)\lib386\os2\clib3r.lib
  @%write $^@ $(WATCOM)\lib386\os2\os2386.lib
  @%write $^@ $(DEFFILE)
!endif

$(OBJDIR)\$(TARGET).sys: $(LNKFILE) $(FILES) $(LIBS)
  @%create $(DEFFILE)
  $(ROOT)\tools\AddToFile.cmd $(DEFFILE),description,BLDLEVEL,$(BLDLVL_VENDOR),$(BLDLVL_VERSION),Universal Audio Driver for OS/2 and eCS (c) 2014 $(BLDLVL_VENDOR),$(BLDLVL_FIXPACK)
  $(LINK) @$(LNKFILE)
  copy $(OBJDIR)\$(TARGET).sys $(ALSA_BIN)

$(OBJDIR)\$(TARGET).sym: $(LIBS)
!ifdef USE_WLINK
  $(WAT2MAP) $(WMAPNAME) $(OBJDIR)\$(TARGET).map
!endif
  cd $(OBJDIR)
  mapsym $(TARGET).MAP
  cd ..\..
  copy $(OBJDIR)\$(TARGET).sym $(ALSA_BIN)

clean: .SYMBOLIC
  -@rm *.err
  -@rm $(ALSA_BIN)\$(TARGET).sy*
  -@rm $(ALSA_LIB)\*.lib
  @if exist obj @rm.exe -r obj
  @if exist bin @rm.exe -r bin

