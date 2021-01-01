!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

!include $(ROOT)\tools\header.mif

AINCLUDES = $(DDK)\base\H

!if "$(IBM_BUILD)" == "1"
CDEFINES += -DIBM_BUILD
!endif

!ifdef %DRV32KIT
CDEFINES+= -DUSE_MSI
!endif

#===================================================================
#
#   List of source files
#
#===================================================================
FILES = spinlock.obj malloc.obj pci.obj irq.obj ioctl.obj devres.obj &
  memory.obj debug.obj sound.obj soundmixer.obj soundmidi.obj &
  ossidc.obj semaphore.obj waitqueue.obj misc.obj klist.obj kasprintf.obj &
  seq_file.obj strncpy.obj strscpy.obj task.obj rbtree.obj &
  strncmp.obj timer.obj kobject.obj driver.obj drivers_base.obj &
  instropl2.obj instropl3.obj vsprintf.obj bitmap.obj find_next_bit.obj &
  regmap.obj regcache.obj regcache-flat.obj regcache-rbtree.obj

  
!if $(KEE) == 0
FILES += stack.obj
!endif

TARGET = linuxlib

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

