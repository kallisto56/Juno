namespace Juno;

using System;
using System.Interop;

[CRepr]
struct SwapChainDesc
{
	public void* mWindow = null;
	public c_uint mWidth = 0;
	public c_uint mHeight = 0;
	public PixelFormat mFormat = .B8G8R8A8_UNorm;
	public PixelFormat mDepthFormat = .Unknown;
	public c_uint mBufferCount = 2;
	public c_bool bAllowTearing = false;
	public c_bool bComposition = false;
}
