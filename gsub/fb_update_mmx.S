/**
 * @file
 * @copyright 1998-2004 Jonathan Brown <jbrown@bluedroplet.com>
 * @license https://www.gnu.org/licenses/gpl-3.0.html
 * @homepage https://github.com/bluedroplet/emergence
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

