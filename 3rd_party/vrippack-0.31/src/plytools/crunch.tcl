#!/usr/bin/tclsh

proc printUsage args {
    puts "Usage: crunch.tcl <in.ply>"
    exit
}

# Check usage -- one arg, doesn't start with -
if {[llength $argv] != 1 ||
    [string index [lindex $argv 0] 0] == "-"} {
    printUsage
}

set name [lindex $argv 0]

if {![file exists $name]} {
    puts "Error: cannot open input ply file: $name"
    printUsage
}

set root [file rootname $name]
set ext [file extension $name]


set last $name
set index 2
while {$index <= 4} {
    set next ${root}_res${index}${ext}
    puts "Generating $next..."
    catch [exec plycrunch < $last > $next] msg
    catch [exec chmod 644 $next] msg
    set last $next
    lappend filelist $last
    incr index
}

