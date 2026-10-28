#include "pch.h"
#include "./../Common/JunoInternal.h"
#include <new>

JUNO_API JunoErrorCode JunoCreateSampler(JunoDevice* pDevice, const JunoSamplerDesc* pDesc, JunoSampler** ppSampler)
{
	JUNO_REQUIRE(pDevice != nullptr);
	JUNO_REQUIRE(pDesc != nullptr);
	JUNO_REQUIRE(ppSampler != nullptr);
	*ppSampler = nullptr;

	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = (D3D11_FILTER)pDesc->mFilter;
	sd.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)pDesc->mAddressU;
	sd.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)pDesc->mAddressV;
	sd.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)pDesc->mAddressW;
	sd.MipLODBias = pDesc->mMipLODBias;
	sd.MaxAnisotropy = pDesc->mMaxAnisotropy;
	sd.ComparisonFunc = (D3D11_COMPARISON_FUNC)pDesc->mComparisonFunc;
	sd.BorderColor[0] = pDesc->mBorderColorR;
	sd.BorderColor[1] = pDesc->mBorderColorG;
	sd.BorderColor[2] = pDesc->mBorderColorB;
	sd.BorderColor[3] = pDesc->mBorderColorA;
	sd.MinLOD = pDesc->mMinLOD;
	sd.MaxLOD = pDesc->mMaxLOD;

	JunoSampler* pSampler = new (std::nothrow) JunoSampler();
	JUNO_REQUIRE_ALLOC(pSampler);

	HRESULT hr = pDevice->mDevice->CreateSamplerState(&sd, &pSampler->mState);
	if (FAILED(hr))
	{
		delete pSampler;
		return JunoFailHResult(JunoErrorCode_StateCreationFailed, hr, "ID3D11Device::CreateSamplerState");
	}

	*ppSampler = pSampler;
	return JunoErrorCode_Ok;
}

JUNO_API void JunoDestroySampler(JunoSampler* pSampler)
{
	delete pSampler;
}

JUNO_API void JunoSetSamplers(JunoDevice* pDevice, JunoShaderStage mStage, c_uint mStartSlot, c_uint mCount, JunoSampler* const* ppSamplers)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(mStartSlot + mCount <= JUNO_MAX_SAMPLER_SLOTS);
	JUNO_REQUIRE_VOID(mCount == 0 || ppSamplers != nullptr);

	ID3D11SamplerState* samplers[JUNO_MAX_SAMPLER_SLOTS] = {};
	for (c_uint i = 0; i < mCount; ++i)
		samplers[i] = ppSamplers[i] ? ppSamplers[i]->mState.Get() : nullptr;

	JunoBindSamplers(pDevice->mContext.Get(), mStage, mStartSlot, mCount, samplers);
}
