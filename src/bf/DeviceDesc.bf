namespace Juno;

using System;
using System.Interop;

[CRepr]
struct DeviceDesc
{
	public c_bool bDebug = false;
	public c_bool bAllowWarp = true;
	public c_int mAdapterIndex = 0;
}
