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
.global alpha_surface_blit_mmx

.balign 16
alpha_surface_blit_mmx:

			push ebp
			push esi
			push edi
			push ebx
			
			// align stack to 64 bit
			
			mov eax, esp
			and eax, 7
			sub esp, eax
			sub esp, 128
			mov [esp+60], eax
			
			
			// copy parameters to known stack positions
			
			mov ebx, [esp+eax+148]
			mov [esp], ebx		// blit_alphasource->buf
			mov ebx, [esp+eax+152]
			mov [esp+4], ebx	// blit_alphasource->width
			
			mov ax, blit_colour
			mov bx, ax
			and ax, 0xf800
			shr ax, 11
			mov [esp+8], ax		// blendred
			mov [esp+10], ax
			mov [esp+12], ax
			mov [esp+14], ax
			
			mov ax, bx
			and ax, 0x7e0
			shr ax, 5
			mov [esp+16], ax	// blendgreen
			mov [esp+18], ax
			mov [esp+20], ax
			mov [esp+22], ax
			
			mov ax, bx
			and ax, 0x1f
			mov [esp+24], ax	// blendblue
			mov [esp+26], ax
			mov [esp+28], ax
			mov [esp+30], ax
			
			mov ax, bx
			mov [esp+106], ax	// blendcolour
			mov [esp+108], ax
			mov [esp+110], ax
			mov [esp+112], ax
			
			mov esi, blit_sourcey
			imul esi, [esp+4]	// blit_alphasource->width
			add esi, blit_sourcex
			add esi, [esp]		// blit_alphasource->buf
			
			mov edi, blit_desty
			imul edi, vid_pitch
			add edi, blit_destx
			shl edi, 1
			add edi, vid_backbuffer
			
			mov eax, [esp+4]	// blit_alphasource->width
			sub eax, blit_width
			mov [esp+40], eax	// srcaddon
			
			mov eax, vid_pitch
			sub eax, blit_width
			shl eax, 1
			mov [esp+44], eax	// dstaddon
			
			
			mov dword ptr [esp+48], 0xff00ff	// maxalpha
			mov dword ptr [esp+52], 0xff00ff
			mov dword ptr [esp+56], 0		// blank
			
			mov dword ptr [esp+72], 0xf800f800	// redfilter
			mov dword ptr [esp+76], 0xf800f800
			
			mov dword ptr [esp+80], 0x7e007e0	// greenfilter
			mov dword ptr [esp+84], 0x7e007e0
			
			mov dword ptr [esp+88], 0x1f001f	// bluefilter
			mov dword ptr [esp+92], 0x1f001f
			
			
			mov ebx, blit_height
			mov [esp+36], ebx
			
			jmp start
			
y:			add esi, [esp+40]
			add edi, [esp+44]
			
			
			// work out pixels until alignment
			
start:		mov eax, 8
			sub eax, edi
			and eax, 7
			shr eax, 1
			
			
			// if none, then jump to mmx
			
			mov ecx, blit_width
			
			test eax, eax
			jnz e
			
			mov [esp+32], ecx	// pixels left on this row
			
			jmp mmx
			
			
e:			sub ecx, eax
			
			jb r
			
			mov [esp+32], ecx
			mov [esp+64], eax
			
			jmp prex
			
r:			mov dword ptr [esp+32], 0
			mov [esp+64], ecx
			
prex:		mov al, [esi]
			
			test al, al
			jnz prego
			
			inc esi
			add edi, 2
			
			mov ecx, [esp+64]
			dec ecx
			mov [esp+64], ecx
			jnz prex
			
			jmp mmx
			
prego:		cmp al, 0xff
			jne preb
			
			mov ax, blit_colour
			mov [edi], ax
			
			inc esi
			add edi, 2
			
			mov ecx, [esp+64]
			dec ecx
			mov [esp+64], ecx
			jnz prex
			
			
			jmp mmx
			
			// load alpha value
preb:		movzx dx, [esi]
			
			// load blend colour components
			movzx ax, [esp+8]	// blendred
			movzx bx, [esp+16]	// blendgreen
			movzx cx, [esp+24]	// blendblue
			
			// multiply by alpha
			imul ax, dx
			imul bx, dx
			imul cx, dx
			
			// shift about
			shl ax, 3
			shr bx, 3
			shr cx, 8
			
			// remove residue
			and ax, 0xf800	// redfilter
			and bx, 0x7e0	// greenfilter
			
			// combine components
			or ax, bx
			or ax, cx
			
			// load the screen pixel
			mov bx, [edi]
			
			test bx, bx
			jz prew
			
			mov cx, bx
			mov bp, bx
			
			// calculate ~alpha
			not dl
			
			// separate colour components
			and bx, 0xf800	// redfilter
			and cx, 0x7e0	// greenfilter
			and bp, 0x1f	// bluefilter
			
			// shift to right
			shr bx, 11
			shr cx, 5
			
			// multiply by ~alpha
			imul bx, dx
			imul cx, dx
			imul bp, dx
			
			// shift about
			shl bx, 3
			shr cx, 3
			shr bp, 8
			
			// remove residue
			and bx, 0xf800	// redfilter
			and cx, 0x7e0	// greenfilter
			
			// combine components
			or bx, cx
			or bx, bp
			
			// add
			add ax, bx
			
			// write pixel back
prew:		mov [edi], ax
			
			inc esi
			add edi, 2
			
			mov ecx, [esp+64]
			dec ecx
			mov [esp+64], ecx
			jnz prex
			
mmx:		mov ecx, [esp+32]
			shr ecx, 2
			jz post
			
			mov [esp+64], ecx
			
x:			mov edx, [esi]
			test edx, edx
			jnz go
			
			// do nothing
			add esi, 4
			add edi, 8
			dec ecx
			jnz x
			
			jmp post
			
			// check for a row of four max alphas
go:			cmp edx, 0xffffffff
			jne blend
			
			// copy the blend colour unmodified
			movq mm0, [esp+106]
			movq [edi], mm0
			
			add esi, 4
			add edi, 8
			dec ecx
			jnz x
			
			jmp post
			
blend:		// load alpha values
			movd mm3, edx
			punpcklbw mm3, [esp+56]		// blank
			
			// load blend colour components
			movq mm0, [esp+8]		// blendred
			movq mm1, [esp+16]		// blendgreen
			movq mm2, [esp+24]		// blendblue
			
			// multiply by alpha
			pmullw mm0, mm3
			pmullw mm1, mm3
			pmullw mm2, mm3
			
			// shift about
			psllw mm0, 3
			psrlw mm1, 3
			psrlw mm2, 8
			
			// remove residue
			pand mm0, [esp+72]	// redfilter
			pand mm1, [esp+80]	// greenfilter
			
			// combine components
			por mm0, mm1
			por mm0, mm2
			
			// calculate ~alpha
			movq mm4, [esp+48]	// maxalpha
			psubb mm4, mm3




			// load four screen pixels

//			movq mm1, [edi]


			mov edx, [edi]
			test edx, edx
			jnz a1
		
			mov ebx, [edi + 4]
			test ebx, ebx
			jnz a2

			jmp write


a1:			movd mm1, [edi+4]
			psllq mm1, 32
			jmp a
		
a2:			movd mm1, ebx
			psllq mm1, 32

a:			movd mm2, edx
			por mm1, mm2
		
			movq mm2, mm1
			movq mm3, mm1
			
			// separate colour components
			pand mm1, [esp+72]	// redfilter
			pand mm2, [esp+80]	// greenfilter
			pand mm3, [esp+88]	// bluefilter
			
			// shift to right
			psrlw mm1, 11
			psrlw mm2, 5
			
			// multiply by ~alpha
			pmullw mm1, mm4
			pmullw mm2, mm4
			pmullw mm3, mm4
			
			// shift about
			psllw mm1, 3
			psrlw mm2, 3
			psrlw mm3, 8
			
			// remove residue
			pand mm1, [esp+72]	// redfilter
			pand mm2, [esp+80]	// greenfilter
			
			// combine components
			por mm1, mm2
			por mm1, mm3
			
			// add
			paddw mm0, mm1
			
			// write pixels back
write:		movq [edi], mm0
			
			add esi, 4
			add edi, 8
			dec ecx
			jnz x
			
post:		mov ecx, [esp+32]
			and ecx, 3
			jz end
			
			mov [esp+64], ecx
			
postx:		mov al, [esi]
			
			test al, al
			jnz postgo
			
			inc esi
			add edi, 2
			
			mov ecx, [esp+64]
			dec ecx
			mov [esp+64], ecx
			jnz postx
	
			jmp end
			
postgo:		cmp al, 0xff
			jne postb
			
			mov ax, blit_colour
			mov [edi], ax
			
			inc esi
			add edi, 2
			
			mov ecx, [esp+64]
			dec ecx
			mov [esp+64], ecx
			jnz postx
			
			jmp end
			
postb:		// load alpha value
			movzx dx, [esi]
			
			// load blend colour components
			movzx ax, [esp+8]	// blendred
			movzx bx, [esp+16]	// blendgreen
			movzx cx, [esp+24]	// blendblue
			
			// multiply by alpha
			imul ax, dx
			imul bx, dx
			imul cx, dx
			
			// shift about
			shl ax, 3
			shr bx, 3
			shr cx, 8
			
			// remove residue
			and ax, 0xf800	// redfilter
			and bx, 0x7e0	// greenfilter
			
			// combine components
			or ax, bx
			or ax, cx
			
			// load the screen pixel
			mov bx, [edi]
			
			test bx, bx
			jz postw
			
			mov cx, bx
			mov bp, bx
			
			// calculate ~alpha
			not dl
			
			// separate colour components
			and bx, 0xf800	// redfilter
			and cx, 0x7e0	// greenfilter
			and bp, 0x1f	// bluefilter
			
			// shift to right
			shr bx, 11
			shr cx, 5
			
			// multiply by ~alpha
			imul bx, dx
			imul cx, dx
			imul bp, dx
			
			// shift about
			shl bx, 3
			shr cx, 3
			shr bp, 8
			
			// remove residue
			and bx, 0xf800	// redfilter
			and cx, 0x7e0	// greenfilter
			
			// combine components
			or bx, cx
			or bx, bp
			
			// add
			add ax, bx
			
			// write pixel back
postw:		mov [edi], ax
			
			inc esi
			add edi, 2
			
			mov ecx, [esp+64]
			dec ecx
			mov [esp+64], ecx
			jnz postx
			
end:		mov edx, [esp+36]
			dec edx
			mov [esp+36], edx
			jnz y
			
			emms
			
			mov eax, [esp+60]
			add esp, 128
			add esp, eax
			
			pop ebx
			pop edi
			pop esi
			pop ebp
			
			ret
