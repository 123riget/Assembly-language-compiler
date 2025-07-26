;AUTHOR:   Rohit Roy
;Roll No:  2301AI54
;Declaration of Authorship 
;***************************/

ldc 1
ldc 10
adj 10
loop: shl
stl 30
adj -1
sp2a
brz exit
br loop
exit: HALT