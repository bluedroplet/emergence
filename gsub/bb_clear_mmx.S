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
.global bb_clear_mmx


.balign 16
bb_clear_mmx:

			push ebp
			push esi
			push edi
			push ebx
		
			mov edi, [esp+20]
			mov ebx, [esp+24]
			mov ecx, [esp+28]
		
			pxor mm0, mm0

.balign 16

x:			movntq [edi], mm0
			movntq [edi+8], mm0
			movntq [edi+16], mm0
			movntq [edi+24], mm0
			movntq [edi+32], mm0
			movntq [edi+40], mm0
			movntq [edi+48], mm0
			movntq [edi+56], mm0

			add edi, 64
		
			dec ebx
			jnz x
		
			sfence
			emms

			xor eax, eax

			rep stosd
		
			pop ebx
			pop edi
			pop esi
			pop ebp
			ret

/*
.balign 16
bb_clear_mmx:

			push ebp
			push esi
			push edi
			push ebx
		
			mov edi, [esp+20]
			mov ebx, [esp+24]
			mov ecx, [esp+28]
		
			pxor xmm0, xmm0

.balign 16

x:			movdqu [edi], xmm0
			movdqu [edi+16], xmm0
			movdqu [edi+32], xmm0
			movdqu [edi+48], xmm0

			add edi, 64
		
			dec ebx
			jnz x
		
			emms

			xor eax, eax

			rep stosd
		
			pop ebx
			pop edi
			pop esi
			pop ebp
			ret
*/
