#
# Target directories.
# Both bin and lib directories are compiler dependent.
#
!if "$(KEE)" == "0"
DIREXT = W4
KEE    = 0
!else
DIREXT = KEE
KEE    = 1
!endif

!ifndef ALSA_BIN
!  if "$(DEBUG)" == "1"
ALSA_BIN  = $(ALSA_BIN_)\Debug.$(DIREXT)
ALSA_BIN__= $(ALSA_BIN_)\Debug.$(DIREXT)
!  else
ALSA_BIN  = $(ALSA_BIN_)\Release.$(DIREXT)
ALSA_BIN__= $(ALSA_BIN_)\Release.$(DIREXT)
!  endif
!endif

!ifndef ALSA_LIB
!  if "$(DEBUG)" == "1"
ALSA_LIB  = $(ALSA_LIB_)\Debug.$(DIREXT)
ALSA_LIB__= $(ALSA_LIB_)\Debug.$(DIREXT)
!  else
ALSA_LIB  = $(ALSA_LIB_)\Release.$(DIREXT)
ALSA_LIB__= $(ALSA_LIB_)\Release.$(DIREXT)
!  endif
!endif

!ifndef OBJDIR
!  if "$(DEBUG)" == "1"
OBJDIR   = bin\Debug.$(DIREXT)
!    else
OBJDIR   = bin\Release.$(DIREXT)
!  endif
!endif
