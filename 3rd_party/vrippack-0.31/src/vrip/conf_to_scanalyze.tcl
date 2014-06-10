#!/usr/common/bin/tclsh

set confFile [lindex $argv 0]
set fileid [open $confFile "r"]

set numchars [gets $fileid line]
set scanalyzeString "readfile [lindex $line 1]"

while {$numchars > 0} {
   set numchars [gets $fileid line]
   set scanalyzeString "$scanalyzeString [lindex $line 1]"
}

puts $scanalyzeString

