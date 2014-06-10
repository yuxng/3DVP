#!/bin/csh -f

if (! $?VRIP_DIR) then
    setenv VRIP_DIR /usr/graphics/project/dmich/ply/src/vrip
endif

if (! $?VRIP_TCL_LIBRARY) then
    if (-f /usr/common/tcl8.0/lib/tcl8.0) then
	setenv VRIP_TCL_LIBRARY /usr/common/tcl8.0/lib/tcl8.0
    else
	setenv VRIP_TCL_LIBRARY ${VRIP_DIR}/lib/tcl
    endif
endif

if (! $?VRIP_TK_LIBRARY) then
    if (-f /usr/common/tcl8.0/lib/tk8.0) then
	setenv VRIP_TK_LIBRARY /usr/common/tcl8.0/lib/tk8.0
    else
	setenv VRIP_TK_LIBRARY ${VRIP_DIR}/lib/tk
    endif
endif

setenv TCL_LIBRARY ${VRIP_TCL_LIBRARY}
setenv TK_LIBRARY ${VRIP_TK_LIBRARY}

# Just call vripslicer.tcl -- it handles usage
${VRIP_DIR}/vrip ${VRIP_DIR}/vripslicer.tcl $argv[*]
