namespace Juno;

using System;
using System.Interop;

[CRepr]
struct TextureDesc
{
	public c_uint mWidth = 0;
	public c_uint mHeight = 0;
	public c_uint mMipLevels = 1;
	public c_uint mArraySize = 1;
	public c_uint mSampleCount = 1;
	public PixelFormat mFormat = .R8G8B8A8_UNorm;
	public Usage mUsage = .Default;
	public BindFlags mBindFlags = .ShaderResource;
	public CpuAccessFlags mCpuAccessFlags = default;
	public c_bool bGenerateMips = false;
	public c_bool bCubemap = false;
}
