WMAKEOPTS=-h $(__MAKEOPTS__) DEBUG=$(DEBUG)

#===================================================================
#
#   Specific dependencies
#
#===================================================================
all: subdirs

subdirs: .SYMBOLIC
  @echo BUILDING core
  @cd core
  @wmake $(WMAKEOPTS)
  @cd ..
  @echo BUILDING drivers
  @cd drivers
  @wmake $(WMAKEOPTS)
  @cd ..
  @echo BUILDING isa
  @cd isa
  @wmake $(WMAKEOPTS)
  @cd ..
  @echo BUILDING hda
  @cd hda
  @wmake $(WMAKEOPTS)
  @cd ..
  @echo BUILDING pci
  @cd pci
  @wmake $(WMAKEOPTS)
  @cd ..
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
  @echo CLEANING hda
  @cd hda
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
