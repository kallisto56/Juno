namespace Juno;

using System;
using System.Interop;

static class Shell32
{
	[Import("SHELL32.lib"), CLink, CallingConvention(.Stdcall)]
	extern static public UINT ExtractIconExW(LPCWSTR lpszFile, int32 nIconIndex, HICON* phiconLarge, HICON* phiconSmall, UINT nIcons);
}
