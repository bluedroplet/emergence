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


.macro movsd
.att_syntax
movsl
.intel_syntax noprefix
.endm

.intel_syntax noprefix
.global surface_blit_mmx

.balign 16
surface_blit_mmx:

			push ebp
			push esi
			push edi
			push ebx
		
			mov esi, blit_sourcey
			imul esi, [esp+24]	// blit_source->width
			add esi, blit_sourcex
			shl esi, 1
			add esi, [esp+20]	// blit_source->buf
		
			mov edi, blit_desty
			imul edi, vid_pitch
			add edi, blit_destx
			shl edi, 1
			add edi, vid_backbuffer
		
			mov ebp, blit_height

blit_surface_core_y:
			mov ebx, blit_width
		
			mov eax, edi
			not eax		// BOLLOCKS BOLLOCKS BOLLOCKS
		
			test eax, 2
			jz align4_done
		
			movsw
			
			dec ebx
			jz sd

align4_done:
			test eax, 4
			jz align8_done
			cmp ebx, 2
			jb u3


			movsd

			sub ebx, 2
			jz sd

align8_done:
			mov ecx, eax
			and ecx, 24
			jz align32_done
		
			shr ecx, 3

q:			cmp ebx, 4
			jb unwind
		
			movq mm0, [esi]
			movq [edi], mm0
		
			add esi, 8
			add edi, 8
		
			sub ebx, 4
			dec ecx
			jnz q


align32_done:
			mov ecx, ebx
			and ecx, 0xfffffff0
			jz unwind
			shr ecx, 4
	
asp:		movq mm0, [esi]
			movq mm1, [esi+8]
			movq mm2, [esi+16]
			movq mm3, [esi+24]
		
			movq [edi], mm0
			movq [edi+8], mm1
			movq [edi+16], mm2
			movq [edi+24], mm3
		
			add esi, 32
			add edi, 32
		
			sub ebx, 16
			dec ecx
			jnz asp
		

unwind: 	mov ecx, ebx
			and ecx, 0xfffffffc
			jz u2
		
			shr ecx, 2

f:			movq mm0, [esi]
			movq [edi], mm0
		
			add esi, 8
			add edi, 8
		
			dec ecx
			jnz f


u2:			test ebx, 2
			jz u3

			movsd

u3:			test ebx, 1
			jz sd
			
			movsw

sd:			mov eax, [esp+24]	// blit_source->width
			sub eax, blit_width
			shl eax, 1
			add esi, eax
		
			mov eax, vid_pitch
			sub eax, blit_width
			shl eax, 1
			add edi, eax
		
			dec ebp
			jnz blit_surface_core_y
		
			emms
		
			pop ebx
			pop edi
			pop esi
			pop ebp
			ret
		
