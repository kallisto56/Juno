namespace Juno;

using System;
using System.Interop;

[CRepr, Ordered]
struct BufferDesc
{
	public c_uint mSize = 0;
	public c_uint mStride = 0;
	public Usage mUsage = .Default;
	public BindFlags mBindFlags = default;
	public CpuAccessFlags mCpuAccessFlags = default;
	public c_bool bStructured = false;
}
