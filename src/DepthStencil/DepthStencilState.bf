namespace Juno;


using System;
using System.Diagnostics;
using System.Interop;


[CRepr]
struct DepthStencilState
{
	public ComPtr<ID3D11DepthStencilState> mState;
}
