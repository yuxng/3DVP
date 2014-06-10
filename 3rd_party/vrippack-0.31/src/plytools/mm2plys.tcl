# Is there a way to tell readfile not to use .xf files?

# Create a bunch of ply files in a subdirectory

set file [lindex $argv 0]
set root [file root $file]
readfile $file
set dir ${root}_plys
set res 0
catch {exec mkdir $dir} msg; puts $msg
catch {exec chmod 775 $dir} msg; puts $msg
set mmConfLines [plv_write_mm_for_vrip $res $dir]
set CONF [open "$dir/vrip.conf" w]
foreach bmesh $mmConfLines {
   puts $CONF $bmesh
}
close $CONF

catch {chdir $dir} msg

# Vrip all the files at 7 resolutions

puts ""


puts -nonewline "\rMerging at 0.25mm...   "
flush stdout
exec vripnew temp.vri vrip.conf vrip.conf 0.25 -rampscale 625 -use_bigger_bbox
exec vripsurf temp.vri temp.ply
exec mv temp.ply 0.25mm.ply

puts -nonewline "\rMerging at 0.5mm...   "
flush stdout
exec vripnew temp.vri vrip.conf vrip.conf 0.5 -rampscale 1250 -use_bigger_bbox
exec vripsurf temp.vri temp.ply
exec mv temp.ply 0.5mm.ply

puts -nonewline "\rMerging at 1mm...   "
flush stdout
exec vripnew temp.vri vrip.conf vrip.conf 1 -rampscale 2500 -use_bigger_bbox
exec vripsurf temp.vri temp.ply
exec mv temp.ply 1mm.ply

puts -nonewline "\rMerging at 2mm...   "
flush stdout
exec vripnew temp.vri vrip.conf vrip.conf 2 -rampscale 5000 -use_bigger_bbox
exec vripsurf temp.vri temp.ply
exec mv temp.ply 2mm.ply

puts -nonewline "\rMerging at 4mm...   "
flush stdout
exec vripnew temp.vri vrip.conf vrip.conf 4 -rampscale 10000 -use_bigger_bbox
exec vripsurf temp.vri temp.ply
exec mv temp.ply 4mm.ply

puts -nonewline "\rMerging at 8mm...   "
flush stdout
exec vripnew temp.vri vrip.conf vrip.conf 8 -rampscale 20000 -use_bigger_bbox
exec vripsurf temp.vri temp.ply
exec mv temp.ply 8mm.ply

puts -nonewline "\rMerging at 16mm...   "
flush stdout
exec vripnew temp.vri vrip.conf vrip.conf 16 -rampscale 40000 -use_bigger_bbox
exec vripsurf temp.vri temp.ply
exec mv temp.ply 16mm.ply

puts ""

#Create the sets


set cmd "exec rm temp.vri vrip.conf [glob ${root}*ply]"
catch {eval $cmd} msg

catch {chdir ..} msg

set cmd "exec plys2set -no_dir_strip -no_xform ${root}.set [glob ${dir}/*mm.ply]"
puts $cmd
catch {eval $cmd} msg

exit

