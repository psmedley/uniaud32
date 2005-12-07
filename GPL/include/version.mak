# $Id: version.mak,v 1.1.1.1 2003/07/02 13:56:58 eleph Exp $ 

#

#       VERSION.MAK
#
#       Current build level for File Versioning
#
#       This file should reside in the same place as COMMON.MAK
#

_VENDOR = "OS/2 Universal Audio Driver"
_VERSION = 1.13.001

FILEVER = @^#$(_VENDOR):$(_VERSION)^#@

