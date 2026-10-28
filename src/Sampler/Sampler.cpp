#include "pch.h"
#include "./../Common/JunoInternal.h"
#include <new>

JUNO_API JunoErrorCode JunoCreateSampler(const JunoSamplerCreateInfo* pCreateInfo, JunoSampler** ppSampler)
{
	JUNO_REQUIRE(pCreateInfo != nullptr);
	JUNO_REQUIRE(pCreateInfo->mDevice != nullptr);
	JUNO_REQUIRE(ppSampler != nullptr);
	*ppSampler = nullptr;

	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = (D3D11_FILTER)pCreateInfo->mFilter;
	sd.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)pCreateInfo->mAddressU;
	sd.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)pCreateInfo->mAddressV;
	sd.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)pCreateInfo->mAddressW;
	sd.MipLODBias = pCreateInfo->mMipLODBias;
	sd.MaxAnisotropy = pCreateInfo->mMaxAnisotropy;
	sd.ComparisonFunc = (D3D11_COMPARISON_FUNC)pCreateInfo->mComparisonFunc;
	sd.BorderColor[0] = pCreateInfo->mBorderColorR;
	sd.BorderColor[1] = pCreateInfo->mBorderColorG;
	sd.BorderColor[2] = pCreateInfo->mBorderColorB;
	sd.BorderColor[3] = pCreateInfo->mBorderColorA;
	sd.MinLOD = pCreateInfo->mMinLOD;
	sd.MaxLOD = pCreateInfo->mMaxLOD;

	JunoSampler* pSampler = new (std::nothrow) JunoSampler();
	JUNO_REQUIRE_ALLOC(pSampler);

	HRESULT hr = pCreateInfo->mDevice->mDevice->CreateSamplerState(&sd, &pSampler->mState);
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
