namespace Juno;


using System;
using System.Diagnostics;
using System.Interop;


[CRepr]
struct Buffer
{
	public void* mHandle;
	public void* mSRV;
	public void* mUAV;
	public CreateInfo mCreateInfo;

	[CRepr]
	public struct CreateInfo
	{
		public Device* mDevice;
		public c_uint mSize;
		public c_uint mStride;
		public Usage mUsage;
		public BindFlags mBindFlags;
		public CpuAccessFlags mCpuAccessFlags;
		public c_bool bStructured;
		public void* mInitialData;
	}
}
