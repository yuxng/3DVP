
proc usage args {
    puts ""
    puts "Usage: vripslicer <vrip_file>"
    puts ""
    exit
}


# Print usage if necessary...
if {$argc == 0 || [lindex $argv 0] == "-h"} {
    usage
}

set gridFile [lindex $argv 0]

# Check for valid argument
if {![file isfile $gridFile]} {
    puts stderr "Error:  Could not open grid file: $gridFile"
    usage
}

puts -nonewline "Reading volume..."

if {[file extension $gridFile] == ".flat"} {
    vrip_readflatgrid $gridFile
} else {
    readgrid $gridFile
}
puts "done."

slicer
update
tkwait window $slicerWin

