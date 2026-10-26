#include "pch.h"
#include "JunoInternal.h"
#include <new>

struct FormatSet
{
	DXGI_FORMAT mResource;
	DXGI_FORMAT mSrv;
	DXGI_FORMAT mDsv;
};

// Depth textures that are also sampled need a typeless resource format and typed views.
static FormatSet ResolveFormats(DXGI_FORMAT format, bool bDepth, bool bSrv)
{
	FormatSet set = { format, format, format };
	if (bDepth && bSrv)
	{
		switch (format)
		{
		case DXGI_FORMAT_D32_FLOAT:
			set.mResource = DXGI_FORMAT_R32_TYPELESS;
			set.mSrv = DXGI_FORMAT_R32_FLOAT;
			break;
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
			set.mResource = DXGI_FORMAT_R24G8_TYPELESS;
			set.mSrv = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;
		case DXGI_FORMAT_D16_UNORM:
			set.mResource = DXGI_FORMAT_R16_TYPELESS;
			set.mSrv = DXGI_FORMAT_R16_UNORM;
			break;
		default:
			break;
		}
	}
	return set;
}

JUNO_API JunoErrorCode JunoCreateTexture(JunoDevice* pDevice, const JunoTextureDesc* pDesc, const void* pInitialData, c_uint mRowPitch, JunoTexture** ppTexture)
{
	JUNO_REQUIRE(pDevice != nullptr);
	JUNO_REQUIRE(pDesc != nullptr);
	JUNO_REQUIRE(ppTexture != nullptr);
	*ppTexture = nullptr;
	JUNO_REQUIRE(pDesc->mWidth > 0 && pDesc->mHeight > 0);
	JUNO_REQUIRE(pDesc->mFormat != JunoPixelFormat_Unknown);
	JUNO_REQUIRE(pInitialData == nullptr || mRowPitch > 0);

	JunoTextureDesc desc = *pDesc;
	if (desc.mSampleCount == 0)
		desc.mSampleCount = 1;
	if (desc.bCubemap)
		desc.mArraySize = 6;
	else if (desc.mArraySize == 0)
		desc.mArraySize = 1;
	if (desc.mSampleCount > 1)
		desc.mMipLevels = 1;

	UINT bind = desc.mBindFlags;
	UINT misc = 0;
	if (desc.bGenerateMips)
	{
		JUNO_REQUIRE(desc.mUsage == JunoUsage_Default);
		JUNO_REQUIRE(desc.mSampleCount == 1);
		bind |= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		misc |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}
	if (desc.bCubemap)
		misc |= D3D11_RESOURCE_MISC_TEXTURECUBE;

	// Immutable textures must supply their data at creation: only a single mip / slice is supported there.
	const bool bImmutable = (desc.mUsage == JunoUsage_Immutable);
	if (bImmutable)
	{
		JUNO_REQUIRE(pInitialData != nullptr);
		JUNO_REQUIRE(desc.mMipLevels == 1 && desc.mArraySize == 1);
	}

	const bool bDepth = (bind & D3D11_BIND_DEPTH_STENCIL) != 0;
	const bool bSrv = (bind & D3D11_BIND_SHADER_RESOURCE) != 0;
	const FormatSet formats = ResolveFormats((DXGI_FORMAT)desc.mFormat, bDepth, bSrv);

	D3D11_TEXTURE2D_DESC td = {};
	td.Width = desc.mWidth;
	td.Height = desc.mHeight;
	td.MipLevels = desc.mMipLevels;
	td.ArraySize = desc.mArraySize;
	td.Format = formats.mResource;
	td.SampleDesc.Count = desc.mSampleCount;
	td.SampleDesc.Quality = 0;
	td.Usage = (D3D11_USAGE)desc.mUsage;
	td.BindFlags = bind;
	td.CPUAccessFlags = desc.mCpuAccessFlags;
	td.MiscFlags = misc;

	D3D11_SUBRESOURCE_DATA init = {};
	init.pSysMem = pInitialData;
	init.SysMemPitch = mRowPitch;

	JunoTexture* pTexture = new (std::nothrow) JunoTexture();
	JUNO_REQUIRE_ALLOC(pTexture);

	ID3D11Device* pNative = pDevice->mDevice.Get();
	ID3D11DeviceContext* pCtx = pDevice->mContext.Get();

	HRESULT hr = pNative->CreateTexture2D(&td, bImmutable ? &init : nullptr, &pTexture->mTexture);
	if (FAILED(hr))
	{
		delete pTexture;
		return JunoFailHResult(JunoErrorCode_TextureCreationFailed, hr, "ID3D11Device::CreateTexture2D");
	}

	// Read back what D3D actually created (mip count for 0 = full chain).
	pTexture->mTexture->GetDesc(&td);
	desc.mMipLevels = td.MipLevels;
	desc.mBindFlags = bind;
	pTexture->mDesc = desc;

	ID3D11Texture2D* pRes = pTexture->mTexture.Get();

	if (bind & D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC sv = {};
		sv.Format = formats.mSrv;
		if (desc.bCubemap)
		{
			sv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			sv.TextureCube.MostDetailedMip = 0;
			sv.TextureCube.MipLevels = (UINT)-1;
		}
		else if (desc.mSampleCount > 1)
		{
			sv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
		}
		else if (desc.mArraySize > 1)
		{
			sv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			sv.Texture2DArray.MostDetailedMip = 0;
			sv.Texture2DArray.MipLevels = (UINT)-1;
			sv.Texture2DArray.FirstArraySlice = 0;
			sv.Texture2DArray.ArraySize = desc.mArraySize;
		}
		else
		{
			sv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			sv.Texture2D.MostDetailedMip = 0;
			sv.Texture2D.MipLevels = (UINT)-1;
		}
		hr = pNative->CreateShaderResourceView(pRes, &sv, &pTexture->mSRV);
		if (FAILED(hr))
		{
			delete pTexture;
			return JunoFailHResult(JunoErrorCode_TextureCreationFailed, hr, "CreateShaderResourceView (texture)");
		}
	}

	if (bind & D3D11_BIND_RENDER_TARGET)
	{
		hr = pNative->CreateRenderTargetView(pRes, nullptr, &pTexture->mRTV);
		if (FAILED(hr))
		{
			delete pTexture;
			return JunoFailHResult(JunoErrorCode_TextureCreationFailed, hr, "CreateRenderTargetView (texture)");
		}
	}

	if (bind & D3D11_BIND_DEPTH_STENCIL)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dv = {};
		dv.Format = formats.mDsv;
		if (desc.mSampleCount > 1)
		{
			dv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		}
		else if (desc.mArraySize > 1)
		{
			dv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dv.Texture2DArray.MipSlice = 0;
			dv.Texture2DArray.FirstArraySlice = 0;
			dv.Texture2DArray.ArraySize = desc.mArraySize;
		}
		else
		{
			dv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			dv.Texture2D.MipSlice = 0;
		}
		hr = pNative->CreateDepthStencilView(pRes, &dv, &pTexture->mDSV);
		if (FAILED(hr))
		{
			delete pTexture;
			return JunoFailHResult(JunoErrorCode_TextureCreationFailed, hr, "CreateDepthStencilView (texture)");
		}
	}

	if (bind & D3D11_BIND_UNORDERED_ACCESS)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uv = {};
		uv.Format = formats.mDsv;
		if (desc.mArraySize > 1)
		{
			uv.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
			uv.Texture2DArray.MipSlice = 0;
			uv.Texture2DArray.FirstArraySlice = 0;
			uv.Texture2DArray.ArraySize = desc.mArraySize;
		}
		else
		{
			uv.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			uv.Texture2D.MipSlice = 0;
		}
		hr = pNative->CreateUnorderedAccessView(pRes, &uv, &pTexture->mUAV);
		if (FAILED(hr))
		{
			delete pTexture;
			return JunoFailHResult(JunoErrorCode_TextureCreationFailed, hr, "CreateUnorderedAccessView (texture)");
		}
	}

	if (pInitialData != nullptr && !bImmutable)
	{
		pCtx->UpdateSubresource(pRes, 0, nullptr, pInitialData, mRowPitch, 0);
		if (desc.bGenerateMips)
			pCtx->GenerateMips(pTexture->mSRV.Get());
	}

	*ppTexture = pTexture;
	return JunoErrorCode_Ok;
}

JUNO_API void JunoDestroyTexture(JunoTexture* pTexture)
{
	if (pTexture == nullptr)
		return;
	assert(!pTexture->bOwnedBySwapChain && "Textures returned by JunoGetBackBuffer/JunoGetDepthBuffer are owned by the swap chain");
	if (pTexture->bOwnedBySwapChain)
		return;
	delete pTexture;
}

JUNO_API JunoErrorCode JunoUpdateTexture(JunoDevice* pDevice, JunoTexture* pTexture, c_uint mArraySlice, c_uint mMipLevel, const JunoRegion* pRegion, const void* pData, c_uint mRowPitch)
{
	JUNO_REQUIRE(pDevice != nullptr);
	JUNO_REQUIRE(pTexture != nullptr);
	JUNO_REQUIRE(pData != nullptr);
	JUNO_REQUIRE(mRowPitch > 0);
	JUNO_REQUIRE(pTexture->mDesc.mUsage == JunoUsage_Default);
	JUNO_REQUIRE(mArraySlice < pTexture->mDesc.mArraySize);
	JUNO_REQUIRE(mMipLevel < pTexture->mDesc.mMipLevels);

	UINT subresource = D3D11CalcSubresource(mMipLevel, mArraySlice, pTexture->mDesc.mMipLevels);

	D3D11_BOX box = {};
	const D3D11_BOX* pBox = nullptr;
	if (pRegion != nullptr)
	{
		box.left = (UINT)pRegion->mLeft;
		box.top = (UINT)pRegion->mTop;
		box.right = (UINT)pRegion->mRight;
		box.bottom = (UINT)pRegion->mBottom;
		box.front = 0;
		box.back = 1;
		pBox = &box;
	}

	pDevice->mContext->UpdateSubresource(pTexture->mTexture.Get(), subresource, pBox, pData, mRowPitch, 0);
	return JunoErrorCode_Ok;
}

JUNO_API JunoErrorCode JunoMapTexture(JunoDevice* pDevice, JunoTexture* pTexture, c_uint mArraySlice, c_uint mMipLevel, JunoMapMode mMode, void** ppData, c_uint* pRowPitch)
{
	JUNO_REQUIRE(pDevice != nullptr);
	JUNO_REQUIRE(pTexture != nullptr);
	JUNO_REQUIRE(ppData != nullptr);
	JUNO_REQUIRE(pRowPitch != nullptr);
	*ppData = nullptr;
	*pRowPitch = 0;

	UINT subresource = D3D11CalcSubresource(mMipLevel, mArraySlice, pTexture->mDesc.mMipLevels);
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	HRESULT hr = pDevice->mContext->Map(pTexture->mTexture.Get(), subresource, (D3D11_MAP)mMode, 0, &mapped);
	if (FAILED(hr))
		return JunoFailHResult(JunoErrorCode_MapFailed, hr, "ID3D11DeviceContext::Map (texture)");

	*ppData = mapped.pData;
	*pRowPitch = mapped.RowPitch;
	return JunoErrorCode_Ok;
}

JUNO_API void JunoUnmapTexture(JunoDevice* pDevice, JunoTexture* pTexture, c_uint mArraySlice, c_uint mMipLevel)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(pTexture != nullptr);
	UINT subresource = D3D11CalcSubresource(mMipLevel, mArraySlice, pTexture->mDesc.mMipLevels);
	pDevice->mContext->Unmap(pTexture->mTexture.Get(), subresource);
}

JUNO_API void JunoGenerateMips(JunoDevice* pDevice, JunoTexture* pTexture)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(pTexture != nullptr);
	JUNO_REQUIRE_VOID(pTexture->mSRV != nullptr); // Needs bGenerateMips at creation.
	pDevice->mContext->GenerateMips(pTexture->mSRV.Get());
}

JUNO_API void JunoCopyTexture(JunoDevice* pDevice, JunoTexture* pDst, JunoTexture* pSrc)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(pDst != nullptr);
	JUNO_REQUIRE_VOID(pSrc != nullptr);
	pDevice->mContext->CopyResource(pDst->mTexture.Get(), pSrc->mTexture.Get());
}

JUNO_API void JunoResolveTexture(JunoDevice* pDevice, JunoTexture* pDst, JunoTexture* pSrc)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(pDst != nullptr);
	JUNO_REQUIRE_VOID(pSrc != nullptr);
	pDevice->mContext->ResolveSubresource(pDst->mTexture.Get(), 0, pSrc->mTexture.Get(), 0, (DXGI_FORMAT)pDst->mDesc.mFormat);
}

JUNO_API void JunoGetTextureSize(JunoTexture* pTexture, c_uint* pWidth, c_uint* pHeight)
{
	JUNO_REQUIRE_VOID(pTexture != nullptr);
	if (pWidth) *pWidth = pTexture->mDesc.mWidth;
	if (pHeight) *pHeight = pTexture->mDesc.mHeight;
}

JUNO_API void JunoClearRenderTarget(JunoDevice* pDevice, JunoTexture* pTexture, float mR, float mG, float mB, float mA)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(pTexture != nullptr);
	JUNO_REQUIRE_VOID(pTexture->mRTV != nullptr);
	const float color[4] = { mR, mG, mB, mA };
	pDevice->mContext->ClearRenderTargetView(pTexture->mRTV.Get(), color);
}

JUNO_API void JunoClearDepthStencil(JunoDevice* pDevice, JunoTexture* pTexture, c_uint mClearFlags, float mDepth, c_uchar mStencil)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(pTexture != nullptr);
	JUNO_REQUIRE_VOID(pTexture->mDSV != nullptr);
	pDevice->mContext->ClearDepthStencilView(pTexture->mDSV.Get(), mClearFlags, mDepth, mStencil);
}

JUNO_API void JunoSetTextures(JunoDevice* pDevice, JunoShaderStage mStage, c_uint mStartSlot, c_uint mCount, JunoTexture* const* ppTextures)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(mStartSlot + mCount <= JUNO_MAX_SRV_SLOTS);
	JUNO_REQUIRE_VOID(mCount == 0 || ppTextures != nullptr);

	ID3D11ShaderResourceView* views[JUNO_MAX_SRV_SLOTS] = {};
	for (c_uint i = 0; i < mCount; ++i)
		views[i] = ppTextures[i] ? ppTextures[i]->mSRV.Get() : nullptr;

	JunoBindShaderResources(pDevice->mContext.Get(), mStage, mStartSlot, mCount, views);
}

JUNO_API void JunoSetUnorderedAccessTextures(JunoDevice* pDevice, c_uint mStartSlot, c_uint mCount, JunoTexture* const* ppTextures)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(mStartSlot + mCount <= JUNO_MAX_UAV_SLOTS);
	JUNO_REQUIRE_VOID(mCount == 0 || ppTextures != nullptr);

	ID3D11UnorderedAccessView* views[JUNO_MAX_UAV_SLOTS] = {};
	for (c_uint i = 0; i < mCount; ++i)
		views[i] = ppTextures[i] ? ppTextures[i]->mUAV.Get() : nullptr;

	pDevice->mContext->CSSetUnorderedAccessViews(mStartSlot, mCount, views, nullptr);
}
