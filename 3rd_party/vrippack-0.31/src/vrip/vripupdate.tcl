
proc usage args {
    puts ""
    puts "vripupdate <vrip_file> <conf_file> \[options\]"
    puts ""
    puts "    Options:"
    puts "        -carve_conservative"
    puts "        -carve_agressive"
    puts "        -carve_only"
    puts "        -show_render"
    puts "        -persp"
    puts "        -linepersp"
    puts "        -slicer"
    puts "        -rampscale <scale factor>"
    puts "        -ramplength <length>"
    puts ""
    exit
}

# Print usage if necessary...
if {$argc == 0 || [lindex $argv 0] == "-h"} {
    usage
}


vrip_param -quiet 1

set doChangeScale 0
set haveRampLength 0

set gridFile [lindex $argv 0]
set confFile [lindex $argv 1]

# Check arguments, to avoid obscure error messages later.
if {![file isfile $gridFile]} {
    puts stderr "Error:  Could not open grid file: $gridFile"
    usage
}
if {![file isfile $confFile]} {
    puts stderr "Error:  Could not open conf file: $confFile"
    usage
}

set bmeshCmd bmesh
set warp 0
set doSil 0

if {$argc > 2} {
    for {set i 2} {$i < $argc} {incr i} {
        set arg [lindex $argv $i]
        if {$arg == "-carve_conservative"} {
	    set bmeshCmd bmeshlin
	    vrip_param -use_tails 1
        } elseif {$arg == "-carve_semi"} {
	    set bmeshCmd bmeshlin
	    vrip_param -use_tails 1
	    param -maxedge 0.01
        } elseif {$arg == "-carve_agressive"} {
	    set bmeshCmd bmeshlin
	    vrip_param -use_tails 1 -fill_gaps 1
        } elseif {$arg == "-carve_only"} {
	    set bmeshCmd bmeshlin
	    vrip_param -use_tails 1 -tails_only 1
	} elseif {$arg == "-show_render"} {
	    show_render
	} elseif {$arg == "-slicer"} {
	    slicer
	} elseif {$arg == "-linepersp"} {
	    set bmeshCmd bmeshlin
	} elseif {$arg == "-persp"} {
	    set bmeshCmd bmeshp
        } elseif {$arg == "-rampscale"} {
	   incr i
	   set scaleFactor [lindex $argv $i]
	   set doChangeScale 1
        } elseif {$arg == "-ramplength"} {
	   incr i
	   set RampLength [lindex $argv $i]
	   set haveRampLength 1
	} elseif {$arg == "-stzoom"} {
	    incr i
	    set arg [lindex $argv $i]
	    set CywarpSTZoom $arg
	    set warp 1
	} elseif {$arg == "-yzoom"} {
	    incr i
	    set arg [lindex $argv $i]
	    set CywarpYZoom $arg
	    set warp 1
	} elseif {$arg == "-wangle"} {
	    incr i
	    set arg [lindex $argv $i]
	    set CywarpAngle $arg
	    set warp 1
	} elseif {$arg == "-sil"} {
	    set doSil 1
	    set bmeshCmd bmeshlin
	}
    }
}

if {$warp} {
    set bmeshCmd "${bmeshCmd}warp"
}

if {$bmeshCmd != "bmesh"} {
    alias bmesh $bmeshCmd
}

update

catch {exec vriphead $gridFile} msg
set x [string last Resolution: $msg]
set x [expr $x + [string length Resolution:] + 1]
set voxelSize [string range $msg $x end]
set voxelSize [format "%g" $voxelSize]

if {[info exists resToScaleMap($voxelSize)]} {
   changeRampScale $resToScaleMap($voxelSize)
}

if {$doChangeScale} {
   changeRampScale $scaleFactor
}

if {$haveRampLength} {
   vrip_param -ramp_width $RampLength
   vrip_param -w1 [expr $RampLength * 1.5]
   vrip_param -w2 [expr $RampLength * 1.25]
   vrip_param -w3 [expr $RampLength * -0.5]
   vrip_param -w4 [expr $RampLength * -0.75]
   vrip_param -w5 [expr $RampLength * -1.0]
   set boundary [expr int(3 * ($RampLength / $voxelSize + 1))]
   vrip_param -max_boundary_steps $boundary
   puts "Using ramps of length $RampLength, with $boundary boundary steps"
}

if {$doSil} {
   param -use_length 0
   updateSilFromConf $gridFile $confFile
} else {
   updateFromConf $gridFile $confFile
}

exit

