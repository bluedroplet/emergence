__declspec(naked) double round(double)
{
	__asm
	{
		fld qword ptr [esp+4]
		frndint
		ret
	}
}


__declspec(naked) int lround(double)
{
	__asm
	{
		fld qword ptr [esp+4]
		fistp dword ptr [esp+4]
		mov eax, [esp+4]
		ret
	}
}


__declspec(naked) double cos(double)
{
	__asm
	{
		fld qword ptr [esp+4]
		fcos
		ret
	}
}


__declspec(naked) double sin(double)
{
	__asm
	{
		fld qword ptr [esp+4]
		fsin
		ret
	}
}


__declspec(naked) void sincos(double, double*, double*)
{
	__asm
	{
		fld qword ptr [esp+4]
		fsincos


		ret
	}
}


/*
__declspec(naked) float exp(double)
{
	__asm
	{
		fldl2e
		fmul qword ptr [esp+4]
		f2xm1
		fld1
		fadd
		ret
	}
}
*/