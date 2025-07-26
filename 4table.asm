;AUTHOR:   Rohit Roy
;Roll No:  2301AI54
;Declaration of Authorship
;***************************/
size: data 10
adc 4
loop: stl 30
ldc 0
ldc 0
ldnl 0
adc -1
brz exit
ldc 0
stnl 0
ldl 30
adj 1
adc 4
br loop
exit: HALT

