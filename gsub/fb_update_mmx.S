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
.global fb_update_mmx

.balign 16
fb_update_mmx:

			push ebp
			push esi
			push edi
			push ebx
		
			mov edi, [esp+20]
			mov esi, [esp+24]
			mov ebx, [esp+28]
			mov eax, [esp+32]
			mov edx, [esp+36]
		
			pxor mm7, mm7

.balign 16
y:			mov ecx, ebx

x:			movq mm0, [esi]
			movq mm1, [esi+8]
			movq mm2, [esi+16]
			movq mm3, [esi+24]

			movq [edi], mm0
			movq [edi+8], mm1
			movq [edi+16], mm2
			movq [edi+24], mm3

			movq [esi], mm7
			movq [esi+8], mm7
			movq [esi+16], mm7
			movq [esi+24], mm7
		
			add esi, 32
			add edi, 32
		
			dec ecx
			jnz x
		
			add edi, eax
		
			dec edx
			jnz y
		
			emms
		
			pop ebx
			pop edi
			pop esi
			pop ebp
			ret

