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
.global alpha_rect_draw_mmx

.balign 16
alpha_rect_draw_mmx:
		
			push ebp
			push esi
			push edi
			push ebx
		
		
			// align stack to 64 bit
		
			mov eax, esp
			and eax, 7
			sub esp, eax
			sub esp, 64
			mov [esp+56], eax
		
		
			// generate constants
		
			mov cx, blit_colour
			mov dl, blit_alpha
		
			mov ax, cx
			and ax, 0xf800
			shr ax, 3
			mov al, dl
		
			mov esi, vid_redalphalookup
			mov bx, [esi][eax*2]
		
			mov ax, cx
			and ax, 0x7e0
			shl ax, 3
			mov al, dl
		
			mov esi, vid_greenalphalookup
			or bx, [esi][eax*2]
		
			mov ax, cx
			and ax, 0x1f
			shl ax, 8
			mov al, dl
		
			mov esi, vid_bluealphalookup
			or bx, [esi][eax*2]
		
			mov ax, 0xff
		
			mov [esp], bx		// blendcolour
			mov [esp+2], bx
			mov [esp+4], bx
			mov [esp+6], bx
		
			mov dword ptr [esp+8], 0xf800f800	// redfilter
			mov dword ptr [esp+12], 0xf800f800
		
			sub al, dl
		
			mov dword ptr [esp+16], 0x7e007e0	// greenfilter
			mov dword ptr [esp+20], 0x7e007e0
		
			mov dword ptr [esp+24], 0x1f001f	// bluefilter
			mov dword ptr [esp+28], 0x1f001f
		
			mov [esp+32], ax	// negalpha
			mov [esp+34], ax
			mov [esp+36], ax
			mov [esp+38], ax
		
			mov edi, blit_desty
			mov eax, vid_pitch
			imul edi, vid_pitch
			sub eax, blit_width
			add edi, blit_destx
			shl eax, 1
			shl edi, 1
			mov [esp+40], eax	// addon
			add edi, vid_backbuffer
		
			mov ebx, blit_width
			mov esi, blit_height
			mov [esp+48], ebx	// xpos
		
			movq mm3, [esp]
			movq mm4, [esp+8]
			movq mm5, [esp+16]
			movq mm6, [esp+24]
			movq mm7, [esp+32]
		
		
			jmp start
		
		
.balign 16
y:			add edi, [esp+40]

		
			// see if we need to align
		
start:		mov ebx, blit_width	// blit_width
			mov [esp+48], ebx	// xpos

			mov edx, 8
			sub edx, edi
			and edx, 7
			shr edx, 1
		
			jz mmx

			sub ebx, edx
			mov [esp+48], ebx

		
prex:		mov ax, [edi]
			test ax, ax
		
			jnz prea
		
			mov ax, [esp]		// blendcolour
		
			mov [edi], ax
			add edi, 2
		
			dec edx
			jnz prex
		
			jmp mmx
		
		
.balign 16
prea:		mov bx, ax
			mov cx, ax
		
			and ax, 0xf800
			and bx, 0x7e0
			and cx, 0x1f
		
			shr ax, 11
			shr bx, 5
		
			imul ax, [esp+32]	// negalpha
			imul bx, [esp+32]	// negalpha
			imul cx, [esp+32]	// negalpha
		
			shl ax, 3
			shr bx, 3
			shr cx, 8
		
			and ax, 0xf800
			and bx, 0x7e0
		
			or ax, bx
			or ax, cx
		
			add ax, [esp]		// blendcolour
		
			mov [edi], ax
			add edi, 2
		
			dec edx
			jnz prex
		
		
mmx:		mov ecx, [esp+48]	// pixels left in this row
			shr ecx, 2
			jz post
		
		
x:			mov edx, [edi]
			cmp edx, 0
			jne a1
		
			mov ebx, [edi + 4]
			cmp ebx, 0
			jne a2
		
			movq [edi], mm3
			
			add edi, 8
			dec ecx
			jnz x
		
			jmp post
		
.balign 16
a1:			movd mm0, [edi+4]
			psllq mm0, 32
			movd mm1, edx
			por mm0, mm1
			jmp a
		
a2:			movd mm0, ebx
			psllq mm0, 32
			movd mm1, edx
			por mm0, mm1
		
a:			movq mm1, mm0
			movq mm2, mm0
		
			// seperate out colour components
			pand mm0, mm4		// redfilter
			pand mm1, mm5		// greenfilter
			pand mm2, mm6		// bluefilter
		
			// shift to far right
			psrlw mm0, 11
			psrlw mm1, 5
		
			// muliply by ~alpha
			pmullw mm0, mm7		// negalpha
			pmullw mm1, mm7		// negalpha
			pmullw mm2, mm7		// negalpha
		
			// shift back into position
			psllw mm0, 3
			psrlw mm1, 3
			psrlw mm2, 8
		
			// remove residue
			pand mm0, mm4		// redfilter
			pand mm1, mm5		// greenfilter
		
			// bring colour components back together
			por mm0, mm1
			por mm0, mm2
		
			// add new colour
			paddw mm0, mm3		// blendcolour
		
			movq [edi], mm0
		
			add edi, 8
			dec ecx
			jnz x
		
			
post:		mov edx, [esp+48]	// pixels left in this row
			and edx, 3
		
			jz checky
		
		
postx:		mov ax, [edi]
			test ax, ax
		
			jnz posta
		
			mov ax, [esp]		// blendcolour
		
			mov [edi], ax
			add edi, 2
		
			dec edx
			jnz postx
		
checky:		dec esi
			jnz y
		
			jmp end
		
		
.balign 16
posta:		mov bx, ax
			mov cx, ax
		
			and ax, 0xf800
			and bx, 0x7e0
			and cx, 0x1f
		
			shr ax, 11
			shr bx, 5
		
			imul ax, [esp+32]	// negalpha
			imul bx, [esp+32]	// negalpha
			imul cx, [esp+32]	// negalpha
		
			shl ax, 3
			shr bx, 3
			shr cx, 8
		
			and ax, 0xf800
			and bx, 0x7e0
		
			or ax, bx
			or ax, cx
		
			add ax, [esp]		// blendcolour
		
			mov [edi], ax
			add edi, 2
		
			dec edx
			jnz postx
		
			dec esi
		
			jnz y
		
end:		emms
		
			mov eax, [esp+56]
			add esp, 64
			add esp, eax
		
			pop ebx
			pop edi
			pop esi
			pop ebp
			ret
