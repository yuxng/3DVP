set tcl_prompt1 {puts -nonewline "vrip> "}
set tcl_prompt2 {puts -nonewline "> "}

if {[info exists env(HOSTTYPE)]} {
    if {$env(HOSTTYPE) == "i386-linux"} {
	set isLinux 1
    } else {
	set isLinux 0
    }
} else {
    set isLinux 0
}

set CywarpAngle ""
set CywarpSTZoom 1.0
set CywarpYZoom 1.0

set sliceNum 0
set weightScale 1
set sliceDirection "y"
set maxWeightScale 256
set useNorm 0


proc newgrid args {
    global slice_scale
    global useNorm

    if {!$useNorm} {
	eval vrip_newgridrle $args
    } else {
	eval vrip_newgridnormrle $args
    }
    update_slice
}


proc readgrid args {
    eval vrip_readgrid $args
    update_slice
}


proc bmeshlinwarp args {
    global CywarpSTZoom
    global CywarpYZoom
    global CywarpAngle

    puts "Creating ply file..."
    set fileName [dowarp [lindex $args 0]]

    global useNorm

    set newArgs [lreplace $args 0 0 $fileName]

    eval vrip_rangescanlin $newArgs

    update_slice    

#    exec rm $fileName
    pcreate /bin/rm $fileName
}


proc bmeshwarp args {
    global CywarpSTZoom
    global CywarpYZoom
    global CywarpAngle

    puts "Creating ply file using ($CywarpSTZoom, $CywarpYZoom, $CywarpAngle)..."
    set fileName [dowarp [lindex $args 0]]

    global useNorm

    set newArgs [lreplace $args 0 0 $fileName]

    if {!$useNorm} {
	eval vrip_rangescanrle $newArgs
    } else {
	eval vrip_rangescannormrle $newArgs
    }
    update_slice    

#    exec rm $fileName
    pcreate /bin/rm $fileName
}


proc dowarp name {
    global CywarpSTZoom
    global CywarpYZoom
    global CywarpAngle

    set root [file rootname $name]
    set frameFile ${root}.cmp
    set i 0
    while {1} {
	set plyFile /usr/tmp/temp_$i.ply
	if {![file exists $plyFile]} {
	    break
	}
	incr i
    }

    if {$CywarpAngle == ""} {

#	catch {exec cywarp $frameFile $plyFile -stzoom $CywarpSTZoom -yzoom $CywarpYZoom} msg

	pcreate /u/curless/scripts/cywarp $frameFile $plyFile -stzoom $CywarpSTZoom -yzoom $CywarpYZoom

    } else {

#	catch {exec cywarp $frameFile $plyFile -stzoom $CywarpSTZoom -yzoom $CywarpYZoom -wangle $CywarpAngle} msg

	pcreate /u/curless/scripts/cywarp $frameFile $plyFile -stzoom $CywarpSTZoom -yzoom $CywarpYZoom -wangle $CywarpAngle

    }

    return $plyFile
}


proc bmesh args {
    global useNorm

    if {!$useNorm} {
	eval vrip_rangescanrle $args
    } else {
	eval vrip_rangescannormrle $args
    }
    update_slice
}

proc bmeshlin args {
    eval vrip_rangescanlin $args
    update_slice
}


proc vrip_slicer {} {
    global slicerWin
    wm geometry $slicerWin "+40+40"
    wm deiconify $slicerWin    
}


proc vrip_render_view {} {
    global renderWin
    wm geometry $renderWin "+40+40"
    wm deiconify $renderWin    
}


proc writeirisslice {name count} {
    global vripSlice

    # Set up frame count suffix
    if {$count < 10} {
	set suffix 000${count}
    } elseif {$count < 100} {
	set suffix 00${count}
    } elseif {$count < 1000} {
	set suffix 0${count}
    } else {
	set suffix ${count}
    }

    $vripSlice write ${name}${suffix}.ppm -format ppm
    exec fromppm ${name}${suffix}.ppm ${name}${suffix}.rgb
    exec /bin/rm ${name}${suffix}.ppm
}

proc update_slice {} {
    global vripSlice
    global sliceDirection
    global sliceNum
    global weightScale
    global slice_scale
    global useNorm
    
    if {$sliceDirection == "x"} {
	if {!$useNorm} {
	    set size [lindex [vrip_inforle] 1]
	} else {
	    set size [lindex [vrip_infonormrle] 1]
	}
    } elseif {$sliceDirection == "y"} {
	if {!$useNorm} {
	    set size [lindex [vrip_inforle] 3]
	} else {
	    set size [lindex [vrip_infonormrle] 3]
	}
    } elseif {$sliceDirection == "z"} {
	if {!$useNorm} {
	    set size [lindex [vrip_inforle] 5]
	} else {
	    set size [lindex [vrip_infonormrle] 5]
	}
    }

    $slice_scale configure -to $size
    
    if {!$useNorm} {
	vrip_setphotoslice $vripSlice $sliceNum $sliceDirection $weightScale
	vrip_photoslicerle $vripSlice $sliceNum $sliceDirection $weightScale
    } else {
	vrip_photoslicenormrle $vripSlice $sliceNum $sliceDirection $weightScale
    }
}


proc change_weight_scale {scale} {
    global weightScale
    set weightScale $scale
    update_slice
}

proc change_slice {slice} {
    global sliceNum
    set sliceNum $slice
    update_slice
}


if {$isLinux} {
    set renderWin [toplevel .renderWin]   
} else {
    set renderWin [toplevel .renderWin -visual {truecolor 24}]   
}

wm withdraw $renderWin
set renderPhoto [image create photo renderPhoto -palette 256]
set renderContainer [label $renderWin.renderContainer -image $renderPhoto]
pack $renderContainer
vrip_setrenderphoto $renderPhoto
$renderPhoto put black -to 0 0 199 199

wm withdraw .
if {$isLinux} {
    set slicerWin [toplevel .slicerWin]
} else {
    set slicerWin [toplevel .slicerWin -visual {truecolor 24}]
}
wm withdraw $slicerWin

set frame1 [frame $slicerWin.frame1]
if {$isLinux} {    
    set vripSlice [image create photo vripSlice -palette 256/0/256]
} else {
    set vripSlice [image create photo vripSlice]
}
set sliceContainer [label $frame1.sliceContainer -image $vripSlice]

set sliceFrame [frame $slicerWin.frame1.sliceFrame]
set slice_scale [scale $sliceFrame.scale -from 0 \
	-command "change_slice" -label "Slice number" -orient horizontal\
	-length 150]
pack append $sliceFrame $slice_scale {left padx 0}

set weightFrame [frame $slicerWin.frame1.weightFrame]
set weight_scale [scale $weightFrame.scale -from 1 -to $maxWeightScale \
	-command "change_weight_scale" -label "Weight scale" \
	-orient horizontal -length 150]
pack append $weightFrame $weight_scale {left padx 0}

set showWeights 0
set frame2 [frame $slicerWin.frame2]
pack append $frame2 \
	[label $frame2.sliceDirLabel -text "Slice direction"] {top pady 0} \
	[radiobutton $frame2.b1 -relief flat -text "X" \
	-variable sliceDirection -value x ]  {left padx 0}\
	[radiobutton $frame2.b2 -relief flat -text "Y" \
	-variable sliceDirection  -value y -state active]  {left padx 0}\
	[radiobutton $frame2.b3 -relief flat -text "Z" \
	-variable sliceDirection -value z]  {left padx 0}\

#pack append $frame1 $photo {left padx 0} $slice_scale {left padx 0}  
pack append $frame1 $sliceFrame {top pady 10} \
	$weightFrame {top pady 0} \
	[checkbutton $frame1.showWeights -text "Show weights" \
	-variable showWeights \
	-command {vrip_param -show_conf_slice $showWeights; update_slice}] \
	{top pady 0} \
	$sliceContainer {top pady 0} 

pack append $slicerWin $frame2 {top pady 0} $frame1 {top pady 0}


# Creates an alias for a command name
proc alias {newname oldname} {
    eval "proc $newname args \{eval \"$oldname \$args\"\}"
}

proc do_nothing args {}

alias show_render vrip_render_view
alias slicer vrip_slicer
alias cube vrip_cube
alias transpxz vrip_transpxzrle

if ($useNorm) {
    alias writegrid vrip_writegridnorm
} else {
    alias writegrid vrip_writegrid
}


alias fill vrip_fillrle
alias ellipse vrip_ellipserle
alias cylinder vrip_cylinderrle
alias extract vrip_extract
alias avgdown vrip_avgdownrle
alias blurvb vrip_blurvb

alias camera do_nothing
alias mesh do_nothing
alias bpolygon do_nothing
alias view do_nothing

alias param vrip_param
alias quit exit

proc newfromply {plyfile res} {
    global env
    
    catch {exec plybbox $plyfile} msg

    scan $msg "%f %f %f %f %f %f" minx miny minz maxx maxy maxz
    set xlen [expr $maxx - $minx]
    set ylen [expr $maxy - $miny]
    set zlen [expr $maxz - $minz]
    set orgx [expr $xlen/2 + $minx]
    set orgy [expr $ylen/2 + $miny]
    set orgz [expr $zlen/2 + $minz]

    #if {$xlen > $ylen} {set maxdim $xlen} else {set maxdim $ylen}
    #if {$maxdim < $zlen} {set maxdim $zlen}
    #set dim [expr int(1.2*$maxdim/$res)]
    #puts "$xlen $ylen $zlen"
    #puts "newgrid $dim $res $orgx $orgy $orgz"
    #newgrid $dim $res $orgx $orgy $orgz

    set xdim [expr int(1.2*$xlen/$res) + 10]
    set ydim [expr int(1.2*$ylen/$res) + 10]
    set zdim [expr int(1.2*$zlen/$res) + 10]
    puts "newgrid $xdim $ydim $zdim $res $orgx $orgy $orgz"
    newgrid $xdim $ydim $zdim $res $orgx $orgy $orgz
}


proc newfromcyl {cylfile res} {
    global env
    global isLinux

    set i 0
    while {1} {
	set tmpPlyFile /usr/tmp/temp_$i.ply
	if {![file exists $tmpPlyFile]} {
	    break
	}
	incr i
    }

    puts "Creating $tmpPlyFile for temporary usage..."
    if {$isLinux} {
	catch {exec cyfiletoply $cylfile $tmpPlyFile} msg
	catch {exec plyroty $tmpPlyFile 180} msg
	catch {exec plybbox $tmpPlyFile} msg
    } else {
	catch {exec cytoply $cylfile $tmpPlyFile} msg
	catch {exec plyroty $tmpPlyFile 180} msg
	catch {exec plybbox $tmpPlyFile} msg
    }

    puts "Removing $tmpPlyFile..."
    catch {exec /bin/rm $tmpPlyFile} tmp
    scan $msg "%f %f %f %f %f %f" minx miny minz maxx maxy maxz
    set xlen [expr $maxx - $minx]
    set ylen [expr $maxy - $miny]
    set zlen [expr $maxz - $minz]
    set orgx [expr $xlen/2 + $minx]
    set orgy [expr $ylen/2 + $miny]
    set orgz [expr $zlen/2 + $minz]

    #if {$xlen > $ylen} {set maxdim $xlen} else {set maxdim $ylen}
    #if {$maxdim < $zlen} {set maxdim $zlen}
    #set dim [expr int(1.2*$maxdim/$res)]
    #puts "newgrid $dim $res $orgx $orgy $orgz"
    #newgrid $dim $res $orgx $orgy $orgz

    set xdim [expr int(1.2*$xlen/$res) + 10]
    set ydim [expr int(1.2*$ylen/$res) + 10]
    set zdim [expr int(1.2*$zlen/$res) + 10]
    puts "newgrid $xdim $ydim $zdim $res $orgx $orgy $orgz"
    newgrid $xdim $ydim $zdim $res $orgx $orgy $orgz
}


proc max {a b} {
   if {$a > $b} {
      return $a
   } else {
      return $b
   }
}


proc min {a b} {
   if {$a < $b} {
      return $a
   } else {
      return $b
   }
}


proc newfromconf {confFile res} {
   global env

   set fileid [open $confFile "r"]
   set count 0
   set numchars [gets $fileid line($count)]
   incr count
   while {$numchars > 0} {
      set numchars [gets $fileid line($count)]
      incr count
   }

   close $fileid

   set count [expr $count -1]

   set minx 10000000
   set miny 10000000
   set minz 10000000
   set maxx -10000000
   set maxy -10000000
   set maxz -10000000

   for {set index 0} {$index < $count} {incr index} {
      set curline $line($index)
      if {("bmesh" == [lindex $curline 0])} {
	 set cmd "exec plyxform "
	 set cmd "$cmd -t [lindex $curline 2] [lindex $curline 3] [lindex $curline 4]"
	 set q3 [lindex $curline 8]
	 set q3 [expr -$q3]
	 set cmd "$cmd -q [lindex $curline 5] [lindex $curline 6] [lindex $curline 7] $q3"
	 set cmd "$cmd < [lindex $curline 1] | plybbox2"

	 catch {eval $cmd} msg

	 scan $msg "%f %f %f %f %f %f" newMinx newMiny newMinz \
	       newMaxx newMaxy newMaxz	 

	 set minx [min $minx $newMinx]
	 set miny [min $miny $newMiny]
	 set minz [min $minz $newMinz]

	 set maxx [max $maxx $newMaxx]
	 set maxy [max $maxy $newMaxy]
	 set maxz [max $maxz $newMaxz]

      }
   }
    


    set xlen [expr $maxx - $minx]
    set ylen [expr $maxy - $miny]
    set zlen [expr $maxz - $minz]
    set orgx [expr $xlen/2 + $minx]
    set orgy [expr $ylen/2 + $miny]
    set orgz [expr $zlen/2 + $minz]

    #if {$xlen > $ylen} {set maxdim $xlen} else {set maxdim $ylen}
    #if {$maxdim < $zlen} {set maxdim $zlen}
    #set dim [expr int(1.2*$maxdim/$res)]
    #puts "$xlen $ylen $zlen"
    #puts "newgrid $dim $res $orgx $orgy $orgz"
    #newgrid $dim $res $orgx $orgy $orgz

    set xdim [expr int(1.2*$xlen/$res) + 10]
    set ydim [expr int(1.2*$ylen/$res) + 10]
    set zdim [expr int(1.2*$zlen/$res) + 10]
    puts "newgrid $xdim $ydim $zdim $res $orgx $orgy $orgz"
    newgrid $xdim $ydim $zdim $res $orgx $orgy $orgz
}


proc fillprep {} {
    fill 1 0
    vrip_param -use_tails 1 -fill_gaps 1 -fill_bg 0
    alias bmesh bmeshlin
}


proc newFromConf {gridFile confFile boundMesh voxelSize} {

    set ext [file extension $boundMesh]
    if {$ext == ".ply"} {
	newfromply $boundMesh $voxelSize
    } elseif {$ext == ".conf"} { 
	newfromconf $boundMesh $voxelSize
    } else {
	newfromcyl $boundMesh $voxelSize
    }

    fill 1 0

    source $confFile

    puts "Writing to file ${gridFile}..."
    flush stdout
    writegrid $gridFile
}


proc updateFromConf {gridFile confFile} {

    readgrid $gridFile

    source $confFile

    puts ""
    puts "Writing to file ${gridFile}..."
    flush stdout
    writegrid $gridFile
}

proc updateSilFromConf {gridFile confFile} {

   readgrid $gridFile
   
   set fileid [open $confFile "r"]
   set count 0
   set numchars [gets $fileid line($count)]
   incr count
   while {$numchars > 0} {
      set numchars [gets $fileid line($count)]
      incr count
   }
   close $fileid

   for {set i 0} {$i < $count} {incr i} {
      set curline $line($i)
      if {("bmesh" == [lindex $curline 0]) } {
	 set mesh [lindex $curline 1]
	 catch {exec plyhead $mesh} msg
	 set x [string last echo_lgincr $msg]
	 set x [expr $x + [string length echo_lgincr] + 1]
	 set x [string range $msg $x end]
	 scan $x %f lgincr
	 set image "[file root $mesh].rgb"
	 exec oneband $image /usr/tmp/_temp.bw 1
	 exec iflip /usr/tmp/_temp.bw /usr/tmp/_temp2.bw xy
	 exec cysiltoply /usr/tmp/_temp2.bw /usr/tmp/_temp.ply $lgincr
	 set newCmd [lreplace $curline 1 1 "/usr/tmp/_temp.ply"]
	 eval $newCmd
	 exec rm /usr/tmp/_temp.bw /usr/tmp/_temp2.bw /usr/tmp/_temp.ply
      }
   }

   puts ""
   puts "Writing to file ${gridFile}..."
   flush stdout
   writegrid $gridFile
}


proc changeRampScale {scale} {
   vrip_param -ramp_width [expr 0.002 * $scale]
   vrip_param -w1 [expr 0.003 * $scale]
   vrip_param -w2 [expr 0.0025 * $scale]
   vrip_param -w3 [expr -0.001 * $scale]
   vrip_param -w4 [expr -0.0015 * $scale]
   vrip_param -w5 [expr -0.002 * $scale]
}

set resToScaleMap(0.002) 6
set resToScaleMap(0.001) 3
set resToScaleMap(0.0005) 1.5
set resToScaleMap(0.00035) 1
set resToScaleMap(0.00025) 1

changeRampScale 1.5

#vrip_param -max_boundary_steps 3 -view_weight_exp 1

#vrip_param -min_view_dot 0.25

#vrip_param -max_edge_length [expr 0.003 * $scale]




newgrid 100 0.0005

bind all <Escape> {puts ""; exit}

