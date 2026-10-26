namespace Juno;

using System;

struct WindowDesc
{
	public StringView mTitle = "Juno";
	public int32 mWidth = 1024;                 // Client area size.
	public int32 mHeight = 768;
	public int32 mX = int32.MinValue;           // int32.MinValue = CW_USEDEFAULT
	public int32 mY = int32.MinValue;
	public WINDOW_STYLE mStyle = .WS_OVERLAPPEDWINDOW;
	public WINDOW_EX_STYLE mExStyle = .WS_EX_LEFT;
	public bool bLayered = false;               // Adds WS_EX_LAYERED and applies mOpacity.
	public uint8 mOpacity = 255;
	public bool bVisible = true;
}
