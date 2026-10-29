#include "pch.h"
#include "JunoInternal.h"

JUNO_API void JunoSetRenderTargets(JunoDevice* pDevice, c_uint mCount, JunoTexture* const* ppColorTargets, JunoTexture* pDepthTarget)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(mCount <= JUNO_MAX_RENDER_TARGETS);
	JUNO_REQUIRE_VOID(mCount == 0 || ppColorTargets != nullptr);

	ID3D11RenderTargetView* views[JUNO_MAX_RENDER_TARGETS] = {};
	for (c_uint i = 0; i < mCount; ++i)
	{
		JUNO_REQUIRE_VOID(ppColorTargets[i] != nullptr);
		JUNO_REQUIRE_VOID(ppColorTargets[i]->mRTV != nullptr); // Texture must be created with JunoBindFlags_RenderTarget.
		views[i] = ppColorTargets[i]->mRTV.Get();
	}

	ID3D11DepthStencilView* pDsv = nullptr;
	if (pDepthTarget != nullptr)
	{
		JUNO_REQUIRE_VOID(pDepthTarget->mDSV != nullptr); // Texture must be created with JunoBindFlags_DepthStencil.
		pDsv = pDepthTarget->mDSV.Get();
	}

	pDevice->mContext->OMSetRenderTargets(mCount, views, pDsv);
}

JUNO_API void JunoSetViewports(JunoDevice* pDevice, c_uint mCount, const JunoViewport* pViewports)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(mCount > 0 && mCount <= D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	JUNO_REQUIRE_VOID(pViewports != nullptr);
	pDevice->mContext->RSSetViewports(mCount, reinterpret_cast<const D3D11_VIEWPORT*>(pViewports));
}

JUNO_API void JunoSetScissorRects(JunoDevice* pDevice, c_uint mCount, const JunoRegion* pRects)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(mCount > 0 && mCount <= D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
	JUNO_REQUIRE_VOID(pRects != nullptr);
	pDevice->mContext->RSSetScissorRects(mCount, reinterpret_cast<const D3D11_RECT*>(pRects));
}

JUNO_API void JunoSetPrimitiveTopology(JunoDevice* pDevice, JunoPrimitiveTopology mTopology)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	pDevice->mContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)mTopology);
}
