.section .text
.align 2
.global D
.type D,@function
D:
csrci mstatus,0x8
ret
.size D,.-D
.global E
.type E,@function
E:
csrsi mstatus,0x8
ret
.size E,.-E
.global W
.type W,@function
W:
wfi
ret
.size W,.-W
