# Is there a way to tell readfile not to use .xf files?

# Create a bunch of ply files in a subdirectory

# Usage: sd2plys4vrip <sd-file> <vrip-dir> <res> [-use_sub_sweeps]

if {[llength $argv] < 3} {
   puts ""
   puts "Usage: sd2plys4vrip <sd-file> <vrip-dir> <res> \[-use_sub_sweeps\]"
   puts ""
   exit
}

set file [lindex $argv 0]
set vripDir [lindex $argv 1]
set res [lindex $argv 2]

set useSubSweeps 0

if {[llength $argv] == 4} {
   if {[lindex $argv 3] == "-use_sub_sweeps"} {
      set useSubSweeps 1
   }
}

readfile $file

if {$useSubSweeps} {
   set sdConfLines [plv_write_sd_for_vrip $res $vripDir subsweeps]
} else {
   set sdConfLines [plv_write_sd_for_vrip $res $vripDir sweeps]
}

set CONF [open "${vripDir}/vrip.conf" a]
foreach bmesh $sdConfLines {
   puts $CONF $bmesh
}
close $CONF

exit

