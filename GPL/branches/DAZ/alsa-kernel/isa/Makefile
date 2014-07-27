!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

NOOBJ=1
!include $(ROOT)\tools\header.mif

WMAKEOPTS=-h $(__MAKEOPTS__) KEE=$(KEE) DEBUG=$(DEBUG)

#we only build sb
all: sb

ad1816a: .SYMBOLIC
    @echo BUILDING ad1816a
    @cd ad1816a
    @wmake $(WMAKEOPTS)
    @cd ..

ad1848: .symbolic
    @echo BUILDING ad1848
    @cd ad1848
    @wmake $(WMAKEOPTS)
    @cd ..

cs423x: .symbolic
    @echo BUILDING cs423x
    @cd cs423x
    @wmake $(WMAKEOPTS)
    @cd ..

es1688: .symbolic
    @echo BUILDING es1688
    @cd es1688
    @wmake $(WMAKEOPTS)
    @cd ..

gus: .symbolic
    @echo BUILDING gus
    @cd gus
    @wmake $(WMAKEOPTS)
    @cd ..

opti9xx: .symbolic
    @echo BUILDING opti9xx
    @cd opti9xx
    @wmake $(WMAKEOPTS)
    @cd ..

sb: .symbolic
    @echo BUILDING sb
    @cd sb
    @wmake $(WMAKEOPTS)
    @cd ..

wavefront: .symbolic
    @echo BUILDING wavefront
    @cd wavefront
    @nmake $(WMAKEOPTS)
    @cd ..


clean: .SYMBOLIC
    @echo CLEANING sb
    @cd sb
    @wmake $(WMAKEOPTS) clean
    @cd ..
    -@rm  *.err
    @if exist obj @rm.exe -r obj
    @if exist bin @rm.exe -r bin

