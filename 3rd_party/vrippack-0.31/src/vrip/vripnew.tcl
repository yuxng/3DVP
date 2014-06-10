
proc usage args {
    puts ""
    puts "Usage: vripnew <vrip_file> <conf_file> <bound_mesh> <res_in_meters>"
    puts "       \[options\]"
    puts ""
    puts "    Options:"
    puts "        -carve_conservative"
    puts "        -carve_agressive"
    puts "        -carve_only"
    puts "        -show_render"
    puts "        -persp"
    puts "        -linepersp"
    puts "        -slicer"
    puts "        -use_bigger_bbox"
    puts "        -rampscale <scale factor>"
    puts "        -ramplength <length>"
    puts "      Note: options *must* be the last args."
    puts ""
    exit
}

# Print usage if necessary...
if {$argc == 0 || [lindex $argv 0] == "-h"} {
    usage
}


vrip_param -quiet 1

set gridName [lindex $argv 0]
set confFile [lindex $argv 1]
set boundMesh [lindex $argv 2]
set voxelSize [lindex $argv 3]
set doChangeScale 0
set haveRampLength 0

#set voxelSize [expr $voxelSize/1000.0]

# Sanity checks on the arguments to avoid obscure messages later
if {[llength $argv] < 4} {
    puts stderr "Error: Insufficient arguments"
    usage
}
if {![file isfile $confFile]} {
    puts stderr "Error:  Could not open conf file: $confFile"
    usage
}
if {![file isfile $boundMesh]} {
    puts stderr "Error:  Could not open bound file: $boundMesh"
    usage
}
if {$voxelSize <= 0} {
    puts stderr "Error: voxelSize must be greater than zero"
    usage
}


set bmeshCmd bmesh
set warp 0

if {$argc > 4} {
    for {set i 4} {$i < $argc} {incr i} {
        set arg [lindex $argv $i]
        if {$arg == "-carve_conservative"} {
	    vrip_param -use_tails 1
        } elseif {$arg == "-carve_agressive"} {
	    vrip_param -use_tails 1 -fill_gaps 1
        } elseif {$arg == "-rampscale"} {
	   incr i
	   set scaleFactor [lindex $argv $i]
	   set doChangeScale 1
        } elseif {$arg == "-ramplength"} {
	   incr i
	   set RampLength [lindex $argv $i]
	   set haveRampLength 1
        } elseif {$arg == "-use_bigger_bbox"} {
	    set conservativeBbox 1
        } elseif {$arg == "-carve_semi"} {
	    vrip_param -use_tails 1
	    param -maxedge 0.01
        } elseif {$arg == "-carve_only"} {
	    vrip_param -use_tails 1 -tails_only 1
	} elseif {$arg == "-show_render"} {
	    show_render
	} elseif {$arg == "-slicer"} {
	    slicer
	} elseif {$arg == "-linepersp"} {
	    set bmeshCmd bmeshlin
	} elseif {$arg == "-persp"} {
	    set bmeshCmd bmeshp
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

newFromConf $gridName $confFile $boundMesh $voxelSize

exit

