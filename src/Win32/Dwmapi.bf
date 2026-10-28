namespace Juno;

using System;
using System.Interop;

static class Dwmapi
{
	[Import("DWMAPI.lib"), CLink, CallingConvention(.Stdcall)]
	extern static public HRESULT DwmSetWindowAttribute(HWND hwnd, DWMWINDOWATTRIBUTE dwAttribute, void* pvAttribute, DWORD cbAttribute);

	[Import("DWMAPI.lib"), CLink, CallingConvention(.Stdcall)]
	extern static public HRESULT DwmExtendFrameIntoClientArea(HWND hWnd, MARGINS* pMarInset);
}
