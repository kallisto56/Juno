#include "pch.h"
#include "./../Common/JunoInternal.h"
#include <new>

static D3D11_DEPTH_STENCILOP_DESC ToD3D(const JunoStencilOpDesc& op)
{
	D3D11_DEPTH_STENCILOP_DESC d = {};
	d.StencilFailOp = (D3D11_STENCIL_OP)op.mStencilFailOp;
	d.StencilDepthFailOp = (D3D11_STENCIL_OP)op.mStencilDepthFailOp;
	d.StencilPassOp = (D3D11_STENCIL_OP)op.mStencilPassOp;
	d.StencilFunc = (D3D11_COMPARISON_FUNC)op.mStencilFunc;
	return d;
}

JUNO_API JunoErrorCode JunoCreateDepthStencilState(JunoDevice* pDevice, const JunoDepthStencilDesc* pDesc, JunoDepthStencilState** ppState)
{
	JUNO_REQUIRE(pDevice != nullptr);
	JUNO_REQUIRE(pDesc != nullptr);
	JUNO_REQUIRE(ppState != nullptr);
	*ppState = nullptr;

	D3D11_DEPTH_STENCIL_DESC dd = {};
	dd.DepthEnable = pDesc->bDepthEnable;
	dd.DepthWriteMask = pDesc->bDepthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	dd.DepthFunc = (D3D11_COMPARISON_FUNC)pDesc->mDepthFunc;
	dd.StencilEnable = pDesc->bStencilEnable;
	dd.StencilReadMask = pDesc->mStencilReadMask;
	dd.StencilWriteMask = pDesc->mStencilWriteMask;
	dd.FrontFace = ToD3D(pDesc->mFrontFace);
	dd.BackFace = ToD3D(pDesc->mBackFace);

	JunoDepthStencilState* pState = new (std::nothrow) JunoDepthStencilState();
	JUNO_REQUIRE_ALLOC(pState);

	HRESULT hr = pDevice->mDevice->CreateDepthStencilState(&dd, &pState->mState);
	if (FAILED(hr))
	{
		delete pState;
		return JunoFailHResult(JunoErrorCode_StateCreationFailed, hr, "ID3D11Device::CreateDepthStencilState");
	}

	*ppState = pState;
	return JunoErrorCode_Ok;
}

JUNO_API void JunoDestroyDepthStencilState(JunoDepthStencilState* pState)
{
	delete pState;
}

JUNO_API void JunoSetDepthStencilState(JunoDevice* pDevice, JunoDepthStencilState* pState, c_uint mStencilRef)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	pDevice->mContext->OMSetDepthStencilState(pState ? pState->mState.Get() : nullptr, mStencilRef);
}
