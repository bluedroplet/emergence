/*
	Copyright (C) 1998-2002 Jonathan Brown
	
    This file is part of the gsub graphics library.
	
	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.
	
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	
	1.	The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.
	2.	Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.
	3.	This notice may not be removed or altered from any source distribution.
	
	Jonathan Brown
	jbrown@emergence.uk.net
*/


.intel_syntax noprefix
.global convert_16bit_to_32bit_mmx

red:
.byte 0x1f, 0, 0, 0, 0x1f, 0, 0, 0

green:
.byte 0xe0, 0x7, 0, 0, 0xe0, 0x7, 0, 0

blue:
.byte 0, 0xf8, 0, 0, 0, 0xf8, 0, 0


.balign 16
convert_16bit_to_32bit_mmx:

			push ebp
			push esi
			push edi
			push ebx
		
			mov edi, [esp+20]
			mov esi, [esp+24]
			mov ecx, [esp+28]

			prefetchnta [esi]
			prefetchnta [esi+32]
			prefetchnta [esi+64]
			prefetchnta [esi+96]
			prefetchnta [esi+128]
			prefetchnta [esi+160]
			prefetchnta [esi+192]
			prefetchnta [esi+224]
			prefetchnta [esi+256]
			prefetchnta [esi+288]

			pxor mm3, mm3
			movq mm4, red
			movq mm5, green
			movq mm6, blue
			
			sub ecx, 5

.balign 16
x:			
			prefetchnta [esi+320]
			prefetchnta [esi+352]
			
			mov eax, 16
			
xx:		movd mm0, [esi]
			punpcklwd mm0, mm3

			movq mm1, mm0
			movq mm2, mm0

			pand mm0, mm4
			pand mm1, mm5
			pand mm2, mm6

			psllq mm0, 3
			psllq mm1, 5
			psllq mm2, 8

			por mm0, mm1
			por mm0, mm2

			movntq [edi], mm0

			add esi, 4
			add edi, 8
			
			dec eax
			jnz xx
			
			dec ecx
			jnz x


			
			mov eax, 80
			
xxx:		movd mm0, [esi]
			punpcklwd mm0, mm3

			movq mm1, mm0
			movq mm2, mm0

			pand mm0, mm4
			pand mm1, mm5
			pand mm2, mm6

			psllq mm0, 3
			psllq mm1, 5
			psllq mm2, 8

			por mm0, mm1
			por mm0, mm2

			movntq [edi], mm0

			add esi, 4
			add edi, 8
			
			dec eax
			jnz xxx
			
			sfence
			emms

			pop ebx
			pop edi
			pop esi
			pop ebp
			ret
