/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
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
