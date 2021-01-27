!ifndef %ROOT
!error ROOT is not defined. Please check your SETENV.CMD.
!endif
ROOT=$(%ROOT)

NOOBJ=1
!include $(ROOT)\tools\header.mif

WMAKEOPTS=-h $(__MAKEOPTS__) DEBUG=$(DEBUG)

all: subdirs

subdirs: .SYMBOLIC
    @echo BUILDING opl3
    @cd opl3
    @wmake $(WMAKEOPTS)
    @cd ..
    @echo BUILDING mpu401
    @cd mpu401
    @wmake $(WMAKEOPTS)
    @cd ..

clean: .SYMBOLIC
    @echo CLEANING opl3
    @cd opl3
    @wmake $(WMAKEOPTS) clean
    @cd ..
    @echo CLEANING mpu401
    @cd mpu401
    @wmake $(WMAKEOPTS) clean
    @cd ..
    @if exist obj @rm.exe -r obj
    @if exist bin @rm.exe -r bin

