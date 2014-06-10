
proc usage args {
    puts ""
    puts "Usage: vripsurf \[options\] <vrip_file> <ply_file>"
    puts " "
    puts "    Options:"
    puts "        -remove_slivers (default)"
    puts "        -no_remove_slivers"
    puts "        -fill_holes"
    puts "        -filter_holes"
    puts "        -crunch"
    puts " "
    exit
}

# Print usage if necessary...
if {$argc == 0 || [lindex $argv 0] == "-h"} {
    usage
}

vrip_param -quiet 1

set vripDir $env(VRIP_DIR)

# Set gridFile, plyFile to be empty, and fill them in
# with the first two non-"-" arguments
set gridFile ""
set plyFile ""

set fillHoles 0
set filterHoles 0
set removeSlivers 1
set useValueWeight 0
set generateNorm 0
set gradientForConfidence 0
set marchArgs ""
set crunch 0

# Parse arguments
for {set i 0} {$i < $argc} {incr i} {
    set arg [lindex $argv $i]
    if {[string index $arg 0] == "-"} {
	# Parse -flags

        if {$arg == "-h"} {
	    usage
        } elseif {$arg == "-fill_holes"} {
	    set fillHoles 1
        } elseif {$arg == "-remove_slivers"} {
	    set removeSlivers 1
        } elseif {$arg == "-no_remove_slivers"} {
	    set removeSlivers 0
        } elseif {$arg == "-filter_holes"} {
	    set filterHoles 1
        } elseif {$arg == "-crunch"} {
	    set crunch 1
        } elseif {$arg == "-thresh"} {
	    incr i
	    set arg [lindex $argv $i]
	    set marchArgs "$marchArgs -thresh $arg"
        } elseif {$arg == "-norm"} {
	    set generateNorm 1
	    set marchArgs "$marchArgs -n"
        } elseif {$arg == "-vw"} {
	    set useValueWeight 1
	    set marchArgs "$marchArgs -vw"
        } elseif {$arg == "-gc"} {
	    set gradientForConfidence 1
	    set marchArgs "$marchArgs -gc"
	} else {
	    puts "Invalid argument '$arg'."
	    usage
	    exit
	}

    } else {
	# Parse non--flag arguments
	if {$gridFile == ""} {
	    set gridFile $arg
	} elseif {$plyFile == ""} {
	    set plyFile $arg
	} else {
	    puts "Error: unhandled argument: $arg"
	    exit
	}
    }
}

# Check to make sure we got the required args...
if {$gridFile == "" || $plyFile == ""} {
    usage
}

#check 1st arg for validity
if {![file isfile $gridFile]} {
    puts stderr "Error:  Could not open grid file: $gridFile"
    exit
}


# Figure out where to save tmp files.  If /usr/tmp/openspace/
# exists, use that instead. (lucas mod)
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

if {[file extension $gridFile] == ".flat"} {
    vrip_readflatgrid $gridFile
} else {
    readgrid $gridFile
}

if {$fillHoles} {
    puts "Preparing for marching cubes"
    vrip_varfromconst

    set i 0

    while {1} {
	set tempFileName $tempDir/temp_$i.occ
	if {![file exists $tempFileName]} {
	    break
	}
	incr i
    }
    puts "Creating $tempFileName..."
    flush stdout
    writegrid $tempFileName
    exec chmod +w $tempFileName

    puts "Generating polygons with marching cubes..."
    flush stdout
    set cmd "exec vripmarch $marchArgs $tempFileName $plyFile -1 >@stdout"

    # set cmd "exec /u/leslie/ply/src/march/vripmarch $marchArgs $tempFileName $plyFile -1 >@stdout"
    eval $cmd

    puts "Removing $tempFileName..."
    flush stdout
    exec rm $tempFileName 2> /dev/null

    puts "Extracting largest connected component..."
    flush stdout
    exec plycomps -m 1 $plyFile -t 1 < $plyFile 2> /dev/null

} else {
   if {0} {
      puts "Generating polygons with marching cubes..."
      flush stdout
       
      set cmd "exec vripmarch $marchArgs $gridFile $plyFile >@stdout"

      # set cmd "exec /u/leslie/ply/src/march/vripmarch $marchArgs $gridFile $plyFile >@stdout"

      eval $cmd
   } else {
      puts "Preparing for marching cubes"
      vrip_varfromconst
      
      set i 0
      while {1} {
	 set tempFileName $tempDir/temp_$i.vri
	 if {![file exists $tempFileName]} {
	    break
	 }
	 incr i
      }
      puts "Creating $tempFileName..."
      flush stdout
      writegrid $tempFileName
      exec chmod +w $tempFileName
      
      puts "Generating polygons with marching cubes..."
      flush stdout

      set cmd "exec vripmarch $marchArgs $tempFileName $plyFile >@stdout" 
      # set cmd "exec /u/leslie/ply/src/march/vripmarch $marchArgs $tempFileName $plyFile >@stdout"
      
      eval $cmd
      puts "Removing $tempFileName..."
      flush stdout
      exec rm $tempFileName 2> /dev/null      
   }
}

if {$removeSlivers} {
    puts "Running plyclean to remove sliver triangles..."
    flush stdout
    catch {exec plyclean -q -defaults -o $plyFile $plyFile} msg
    # catch {exec /u/leslie/ply/src/plyclean/plyclean -q -defaults -o $plyFile $plyFile} msg
    puts $msg
}

if {$filterHoles} {
    exec trifilt $plyFile $plyFile \
	    -iters 50 -alpha 0.67 -lambda -0.63 -minconf 0.025 -confsteps 8
}

if {$crunch} {
    puts "Generating multi-res..."
    flush stdout
    exec ply2crunchset $plyFile 2>/dev/null
}


exit

