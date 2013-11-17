#ifndef _ASM_DELAY_
#define _ASM_DELAY_ 1

#ifdef __ASSEMBLER__

.macro delay_ms t
	push R28
	push R29
	ldi	R28,lo8(\t)
	ldi	R29,hi8(\t)
2:
	_usdel = (1000-4/F_CPU_MHZ)
	delay_us 1000
	sbiw	R28,1
	brne	2b
	pop R29
	pop R28
.endm
// Cyles = 4N+1
.macro delayd t
	_del = (F_CPU_MHZ*(\t)-1)/4
	.if _del > 65535
	.warning "Delay out of Range, using reduced accuracy delay_ms"
	_del = \t/1000
	delay_ms _del
	.else
	ldi	R26,lo8(_del)
	ldi	R27,hi8(_del)
1:
	sbiw	R26,1
	brne	1b
	.endif
.endm

// Cyles = 3N
.macro delay_us t
	_del = (F_CPU_MHZ*(\t))/3
	.if (_del > 255)
	delayd \t
	.else
	ldi	r18,_del
1:
	dec	r18
	brne	1b	
	.endif
.endm

#endif /* __ASSEMBLER__ */

#endif
