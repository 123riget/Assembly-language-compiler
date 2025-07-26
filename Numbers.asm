;/***************************
;TITLE: Adding n numbers													
;AUTHOR:   Rohit Roy
;Roll No:  2301AI54
;Declaration of Authorship
;***************************/
; outline of Adding n numbers program

ldc 0x1000
a2sp
adj -1
ldc 10
stl 0
ldc array
call addx
addx: adj -4
ldc 1
stl 0
ldc 1
stl 1
loop: ldc 1
ldl 1
add 
stl 1
ldl 0
ldl 1
add
stl 0
ldl 4
ldl 1
sub
brz done
br loop
done: ldl 0
HALT



