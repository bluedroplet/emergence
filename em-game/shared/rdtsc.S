.intel_syntax noprefix
.global rdtsc

.balign 16
rdtsc:
		rdtsc
		ret
