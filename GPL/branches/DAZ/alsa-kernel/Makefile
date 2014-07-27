WMAKEOPTS=-h $(__MAKEOPTS__) KEE=$(KEE) DEBUG=$(DEBUG)

#===================================================================
#
#   Specific dependencies
#
#===================================================================
all: core drivers isa pci synth

core: .SYMBOLIC
  @echo BUILDING core
  @cd core
  @wmake $(WMAKEOPTS)
  @cd ..

drivers: .symbolic
  @echo BUILDING drivers
  @cd drivers
  @wmake $(WMAKEOPTS)
  @cd ..

isa: .symbolic
  @echo BUILDING isa
  @cd isa
  @wmake $(WMAKEOPTS)
  @cd ..

pci: .symbolic
  @echo BUILDING pci
  @cd pci
  @wmake $(WMAKEOPTS)
  @cd ..

synth: .symbolic
  @echo BUILDING synth
  @cd synth
  @wmake $(WMAKEOPTS)
  @cd ..

clean: .SYMBOLIC
  @echo CLEANING core
  @cd core
  @wmake $(WMAKEOPTS) clean
  @cd ..
  @echo CLEANING drivers
  @cd drivers
  @wmake $(WMAKEOPTS) clean
  @cd ..
  @echo CLEANING isa
  @cd isa
  @wmake $(WMAKEOPTS) clean
  @cd ..
  @echo CLEANING pci
  @cd pci
  @wmake $(WMAKEOPTS) clean
  @cd ..
  @echo CLEANING synth
  @cd synth
  @wmake $(WMAKEOPTS) clean
  @cd ..
