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

if {[file exists /usr/tmp/openspace]} {
    puts "Using /usr/tmp/openspace/"
    set tempDir "/usr/tmp/openspace"
} elseif {[file exists /usr/tmp]} {
    set tempDir "/usr/tmp"
} elseif {[file exists /var/tmp]} {
    set tempDir "/var/tmp"
} elseif {[file exists /tmp]} {
    set tempDir "/tmp"
}

set CywarpAngle ""
set CywarpSTZoom 1.0
set CywarpYZoom 1.0

set sliceNum 0
set weightScale 1
set sliceDirection "y"
set maxWeightScale 256
set useNorm 0
set useOffsetScale 0
set sliceOffset 32768
set sliceDivisor 256
set conservativeBbox 0
set conservativeBboxFactor 1.2

proc uiExists {} {
    global noui
    return [expr ! $noui]
}


proc newgrid args {
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
	set plyFile ${tempDir}/temp_$i.ply
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

proc bmeshp args {
    eval vrip_rangescanpersp $args
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
    global sliceOffset
    global sliceDivisor
    
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

    if {[uiExists]} {
	$slice_scale configure -to $size

	if {!$useNorm} {
	    vrip_setphotoslice $vripSlice $sliceNum \
		$sliceDirection $weightScale
	    vrip_photoslicerle $vripSlice $sliceNum \
		$sliceDirection $weightScale $sliceOffset $sliceDivisor
	} else {
	    vrip_photoslicenormrle $vripSlice $sliceNum \
		$sliceDirection $weightScale
	}
    }
}

# JED - just stole from above, so that I could use it elsewhere.
# didn't bother to really understand it.
proc get_slice_size {} {
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

    return $size
}

proc change_weight_scale {scale} {
    global weightScale
    set weightScale $scale
    update_slice
}

proc change_offset {offset} {
    global sliceOffset
    set sliceOffset $offset
    update_slice
}

proc change_divisor {divisor} {
    global sliceDivisor
    set sliceDivisor $divisor
    update_slice
}


proc change_slice {slice} {
    global sliceNum
    set sliceNum $slice
    update_slice
}

proc get_voxel_info {u v} {
    global sliceNum
    global sliceDirection
    global useNorm

    if {$useNorm} return ""
    if {$sliceDirection == "x"} {
	return [vrip_getvoxelrle $sliceNum $v $u]
    } elseif {$sliceDirection == "y"} {
	return [vrip_getvoxelrle $u $sliceNum $v]
    } elseif {$sliceDirection == "z"} {
	return [vrip_getvoxelrle $u $v $sliceNum]
    }
}


#################### UI stuff start ##########################

if {[uiExists]} {

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
    set sliceContainer [label $frame1.sliceContainer -image $vripSlice \
                        -cursor crosshair -borderwidth 0 -highlightthickness 0]
    
    set sliceFrame [frame $slicerWin.frame1.sliceFrame]
    set slice_scale [scale $sliceFrame.scale -from 0 \
			 -command "change_slice" \
			 -label "Slice number" -orient horizontal\
			 -length 150]
    pack append $sliceFrame $slice_scale {left padx 0}
    
    if {$useOffsetScale} {
       set offsetFrame [frame $slicerWin.frame1.offsetFrame]
       set offset_scale [scale $offsetFrame.scale -from 0 -to 65535 \
			     -command "change_offset" \
			     -label "Offset" \
			     -orient horizontal -length 150 ]
       $offset_scale set $sliceOffset			  
       pack append $offsetFrame $offset_scale {left padx 0}
       
       set divisorFrame [frame $slicerWin.frame1.divisorFrame]
       set divisor_scale [scale $divisorFrame.scale -from 1 -to 256 \
			      -command "change_divisor" \
			      -label "Divisor" \
			      -orient horizontal -length 150]
       $divisor_scale set $sliceDivisor
       pack append $divisorFrame $divisor_scale {left padx 0}
    }

    set weightFrame [frame $slicerWin.frame1.weightFrame]
    set weight_scale [scale $weightFrame.scale -from 1 -to $maxWeightScale \
			  -command "change_weight_scale" \
			  -label "Weight scale" \
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

    if {$useOffsetScale} {
       pack append $frame1 $sliceFrame {top pady 10} \
	   $offsetFrame {top pady 0} \
	   $divisorFrame {top pady 0} \
	   $weightFrame {top pady 0} \
	   [checkbutton $frame1.showWeights -text "Show weights" \
		-variable showWeights \
		-command {vrip_param -show_conf_slice
		   $showWeights; update_slice}] \
	   {top pady 0} \
	   $sliceContainer {top pady 0} \
	   [label $frame1.infoLabel -textvariable infoLabelStr -font 7x13] {top pady 0}
    } else {
       pack append $frame1 $sliceFrame {top pady 10} \
	   $weightFrame {top pady 0} \
	   [checkbutton $frame1.showWeights -text "Show weights" \
		-variable showWeights \
		-command {vrip_param -show_conf_slice
		   $showWeights; update_slice}] \
	   {top pady 0} \
	   $sliceContainer {top pady 0} \
	   [label $frame1.infoLabel -textvariable infoLabelStr -font 7x13] {top pady 0}
    }

    bind $sliceContainer <Motion>  {set infoLabelStr [get_voxel_info %x %y]}
    
    pack append $slicerWin $frame2 {top pady 0} $frame1 {top pady 0}

# end of ui-only stuff
}

proc write_slice {name im} {
    puts "Saving slice .. $name"
    $im write $name
}

proc set_slice {num} {
    global sliceNum
    set sliceNum $num
    
}

proc write_slice_movie {name slice} {
   
    set size [get_slice_size]
    puts "Size .. $size"
    exec mkdir -p $name

    for {set i 0} {$i < $size} {incr i} {
	set_slice $i
	set numname [expr $i + 10000]
	update_slice
	set filename "$name/$numname.ppm"
	puts $filename
	write_slice $filename $slice
    }
}

proc write_smooth_movie {name slice} {
    set size 15   
    puts "Size .. $size"
    exec mkdir -p $name

    for {set i 0} {$i < $size} {incr i} {
	set numname [expr $i + 10000]
	update_slice
	set filename "$name/$numname.ppm"
	puts $filename
	write_slice $filename $slice
	vrip_muckWithVol

    }

}

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
    global conservativeBbox
    global conservativeBboxFactor
    
    catch {exec plybbox < $plyfile} msg

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

    if {$conservativeBbox} {
       set xdim [expr int($xlen/$res*$conservativeBboxFactor)]
       set ydim [expr int($ylen/$res*$conservativeBboxFactor)]
       set zdim [expr int($zlen/$res*$conservativeBboxFactor)]       
    } else {
       set xdim [expr int($xlen/$res) + 10]
       set ydim [expr int($ylen/$res) + 10]
       set zdim [expr int($zlen/$res) + 10]
    }

    # We want to guarantee that the parity of the number of voxels
    # has not changed; this guarantees consistent alignment of the
    # grid w.r.t. the grid center -- David Koller (dk@cs.stanford.edu)
    if {[expr (round($xlen/$res))%2] != [expr $xdim%2]} {incr xdim}
    if {[expr (round($ylen/$res))%2] != [expr $ydim%2]} {incr ydim}
    if {[expr (round($zlen/$res))%2] != [expr $zdim%2]} {incr zdim}

    puts "newgrid $xdim $ydim $zdim $res $orgx $orgy $orgz"
    newgrid $xdim $ydim $zdim $res $orgx $orgy $orgz
}


proc newfromcyl {cylfile res} {
    global env
    global isLinux
    global conservativeBbox
    global conservativeBboxFactor

    set i 0
    while {1} {
	set tmpPlyFile ${tempDir}/temp_$i.ply
	if {![file exists $tmpPlyFile]} {
	    break
	}
	incr i
    }

    puts "Creating $tmpPlyFile for temporary usage..."
    if {$isLinux} {
	catch {exec cyb2ply $cylfile $tmpPlyFile} msg
	catch {exec plyxform -r 0 180 0 < $tmpPlyFile | plybbox} msg
    } else {
	catch {exec cyb2ply $cylfile $tmpPlyFile} msg
	catch {exec plyxform -r 0 180 0 < $tmpPlyFile | plybbox} msg
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

    if {$conservativeBbox} {
       set xdim [expr int($xlen/$res*$conservativeBboxFactor)]
       set ydim [expr int($ylen/$res*$conservativeBboxFactor)]
       set zdim [expr int($zlen/$res*$conservativeBboxFactor)]       
    } else {
       set xdim [expr int($xlen/$res) + 10]
       set ydim [expr int($ylen/$res) + 10]
       set zdim [expr int($zlen/$res) + 10]
    }

    # We want to guarantee that the parity of the number of voxels
    # has not changed; this guarantees consistent alignment of the
    # grid w.r.t. the grid center -- David Koller (dk@cs.stanford.edu)
    if {[expr (round($xlen/$res))%2] != [expr $xdim%2]} {incr xdim}
    if {[expr (round($ylen/$res))%2] != [expr $ydim%2]} {incr ydim}
    if {[expr (round($zlen/$res))%2] != [expr $zdim%2]} {incr zdim}

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
   global conservativeBbox
   global conservativeBboxFactor

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
	 set cmd "$cmd < [lindex $curline 1] | plybbox"
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

    if {$conservativeBbox} {
       set xdim [expr int($xlen/$res*$conservativeBboxFactor)]
       set ydim [expr int($ylen/$res*$conservativeBboxFactor)]
       set zdim [expr int($zlen/$res*$conservativeBboxFactor)]       
    } else {
       set xdim [expr int($xlen/$res) + 10]
       set ydim [expr int($ylen/$res) + 10]
       set zdim [expr int($zlen/$res) + 10]
    }

    # We want to guarantee that the parity of the number of voxels
    # has not changed; this guarantees consistent alignment of the
    # grid w.r.t. the grid center -- David Koller (dk@cs.stanford.edu)
    if {[expr (round($xlen/$res))%2] != [expr $xdim%2]} {incr xdim}
    if {[expr (round($ylen/$res))%2] != [expr $ydim%2]} {incr ydim}
    if {[expr (round($zlen/$res))%2] != [expr $zdim%2]} {incr zdim}

    puts "newgrid $xdim $ydim $zdim $res $orgx $orgy $orgz"
    newgrid $xdim $ydim $zdim $res $orgx $orgy $orgz
}


proc newfromlist {listfile res} {
   global env
   global conservativeBbox
   global conservativeBboxFactor

   set fileid [open $listfile "r"]
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
      set curmesh $line($index)
      set rootName [file root $curmesh]
      set xfFile "${rootName}.xf"
      if {[file exists $xfFile]} {
	 set cmd "exec plyxform -f $xfFile < $curmesh | plybbox"
      } else {
	 set cmd "exec plybbox < $curmesh"
      }

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

    if {$conservativeBbox} {
       set xdim [expr int($xlen/$res*$conservativeBboxFactor)]
       set ydim [expr int($ylen/$res*$conservativeBboxFactor)]
       set zdim [expr int($zlen/$res*$conservativeBboxFactor)]       
    } else {
       set xdim [expr int($xlen/$res) + 10]
       set ydim [expr int($ylen/$res) + 10]
       set zdim [expr int($zlen/$res) + 10]
    }

    # We want to guarantee that the parity of the number of voxels
    # has not changed; this guarantees consistent alignment of the
    # grid w.r.t. the grid center -- David Koller (dk@cs.stanford.edu)
    if {[expr (round($xlen/$res))%2] != [expr $xdim%2]} {incr xdim}
    if {[expr (round($ylen/$res))%2] != [expr $ydim%2]} {incr ydim}
    if {[expr (round($zlen/$res))%2] != [expr $zdim%2]} {incr zdim}

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
    } elseif {$ext == ".cyb"} {
	# this doesn't actually work, though it would be nice
        # to support cylindrical scans somehow...
	newfromcyl $boundMesh $voxelSize
    } else {
        newfromlist $boundMesh $voxelSize
    }

    fill 1 0

    set ext [file extension $confFile]
    if {$ext == ".conf"} {
        source $confFile
    } else {
        set fileid [open $confFile r]
	set numchars [gets $fileid filename]
	puts $numchars
	while {$numchars > 0} {
	    bmesh $filename
	    set numchars [gets $fileid filename]
	}
	close $fileid
    }

    puts "Writing to file ${gridFile}..."
    flush stdout
    writegrid $gridFile
}


proc updateFromConf {gridFile confFile} {

    readgrid $gridFile

    set ext [file extension $confFile]
    if {$ext == ".conf"} {
        source $confFile
    } else {
        set fileid [open $confFile r]
	set numchars [gets $fileid filename]
	puts $numchars
	while {$numchars > 0} {
	    bmesh $filename
	    set numchars [gets $fileid filename]
	}
	close $fileid
    }

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
	 exec oneband $image ${tempDir}/_temp.bw 1
	 exec iflip ${tempDir}/_temp.bw ${tempDir}/_temp2.bw xy
	 exec cybsil2ply ${tempDir}/_temp2.bw ${tempDir}/_temp.ply $lgincr
	 set newCmd [lreplace $curline 1 1 "${tempDir}/_temp.ply"]
	 eval $newCmd
	 exec rm ${tempDir}/_temp.bw ${tempDir}/_temp2.bw ${tempDir}/_temp.ply
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
set resToScaleMap(0.00025) 0.5

changeRampScale 1.5

#vrip_param -max_boundary_steps 3 -view_weight_exp 1

#vrip_param -min_view_dot 0.25

#vrip_param -max_edge_length [expr 0.003 * $scale]




newgrid 100 0.0005

if {[uiExists]} {
    bind all <Escape> {puts ""; exit}
}
