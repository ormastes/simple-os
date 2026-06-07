.macro AIC d,e,w
.global \d
\d:
A_DI
A_RT
.global \e
\e:
A_EI
A_RT
.global \w
\w:
A_WI
A_RT
.endm
