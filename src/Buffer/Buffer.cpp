#include "pch.h"
#include "./../Common/JunoInternal.h"
#include <new>

JUNO_API JunoErrorCode JunoCreateBuffer(const JunoBufferCreateInfo* pCreateInfo, JunoBuffer** ppBuffer)
{
	JUNO_REQUIRE(pCreateInfo != nullptr);
	JUNO_REQUIRE(pCreateInfo->mDevice != nullptr);
	JUNO_REQUIRE(ppBuffer != nullptr);
	*ppBuffer = nullptr;
	JUNO_REQUIRE(pCreateInfo->mSize > 0);
	JUNO_REQUIRE(pCreateInfo->mUsage != JunoUsage_Immutable || pCreateInfo->mInitialData != nullptr);
	JUNO_REQUIRE((pCreateInfo->mBindFlags & JunoBindFlags_ConstantBuffer) == 0 || (pCreateInfo->mSize % 16) == 0);
	JUNO_REQUIRE(!pCreateInfo->bStructured || pCreateInfo->mStride > 0);

	D3D11_BUFFER_DESC bd = {};
	bd.ByteWidth = pCreateInfo->mSize;
	bd.Usage = (D3D11_USAGE)pCreateInfo->mUsage;
	bd.BindFlags = pCreateInfo->mBindFlags;
	bd.CPUAccessFlags = pCreateInfo->mCpuAccessFlags;
	bd.StructureByteStride = pCreateInfo->mStride;
	if (pCreateInfo->bStructured)
		bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA init = {};
	init.pSysMem = pCreateInfo->mInitialData;

	JunoBuffer* pBuffer = new (std::nothrow) JunoBuffer();
	JUNO_REQUIRE_ALLOC(pBuffer);
	pBuffer->mCreateInfo = *pCreateInfo;

	ID3D11Device* pNative = pCreateInfo->mDevice->mDevice.Get();
	HRESULT hr = pNative->CreateBuffer(&bd, pCreateInfo->mInitialData ? &init : nullptr, &pBuffer->mHandle);
	if (FAILED(hr))
	{
		delete pBuffer;
		return JunoFailHResult(JunoErrorCode_BufferCreationFailed, hr, "ID3D11Device::CreateBuffer");
	}

	if (pCreateInfo->bStructured && (pCreateInfo->mBindFlags & JunoBindFlags_ShaderResource))
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC sv = {};
		sv.Format = DXGI_FORMAT_UNKNOWN;
		sv.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		sv.Buffer.FirstElement = 0;
		sv.Buffer.NumElements = pCreateInfo->mSize / pCreateInfo->mStride;
		hr = pNative->CreateShaderResourceView(pBuffer->mHandle.Get(), &sv, &pBuffer->mSRV);
		if (FAILED(hr))
		{
			delete pBuffer;
			return JunoFailHResult(JunoErrorCode_BufferCreationFailed, hr, "CreateShaderResourceView (buffer)");
		}
	}

	if (pCreateInfo->bStructured && (pCreateInfo->mBindFlags & JunoBindFlags_UnorderedAccess))
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uv = {};
		uv.Format = DXGI_FORMAT_UNKNOWN;
		uv.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uv.Buffer.FirstElement = 0;
		uv.Buffer.NumElements = pCreateInfo->mSize / pCreateInfo->mStride;
		hr = pNative->CreateUnorderedAccessView(pBuffer->mHandle.Get(), &uv, &pBuffer->mUAV);
		if (FAILED(hr))
		{
			delete pBuffer;
			return JunoFailHResult(JunoErrorCode_BufferCreationFailed, hr, "CreateUnorderedAccessView (buffer)");
		}
	}

	*ppBuffer = pBuffer;
	return JunoErrorCode_Ok;
}

JUNO_API void JunoDestroyBuffer(JunoBuffer* pBuffer)
{
	delete pBuffer;
}

JUNO_API JunoErrorCode JunoUpdateBuffer(JunoDevice* pDevice, JunoBuffer* pBuffer, c_size mOffset, const void* pData, c_size mSize)
{
	JUNO_REQUIRE(pDevice != nullptr);
	JUNO_REQUIRE(pBuffer != nullptr);
	JUNO_REQUIRE(pData != nullptr);
	JUNO_REQUIRE(mSize > 0 && mOffset + mSize <= pBuffer->mCreateInfo.mSize);

	const JunoUsage usage = pBuffer->mCreateInfo.mUsage;
	JUNO_REQUIRE(usage != JunoUsage_Immutable);

	ID3D11DeviceContext* pCtx = pDevice->mContext.Get();
	ID3D11Buffer* pNative = pBuffer->mHandle.Get();

	if (usage == JunoUsage_Dynamic || usage == JunoUsage_Staging)
	{
		// Dynamic: offset 0 discards the old contents, other offsets append (vertex/index buffers only).
		D3D11_MAP mapType = (usage == JunoUsage_Staging) ? D3D11_MAP_WRITE : (mOffset == 0 ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE);
		D3D11_MAPPED_SUBRESOURCE mapped = {};
		HRESULT hr = pCtx->Map(pNative, 0, mapType, 0, &mapped);
		if (FAILED(hr))
			return JunoFailHResult(JunoErrorCode_MapFailed, hr, "ID3D11DeviceContext::Map");
		memcpy((unsigned char*)mapped.pData + mOffset, pData, mSize);
		pCtx->Unmap(pNative, 0);
		return JunoErrorCode_Ok;
	}

	const bool bWhole = (mOffset == 0 && mSize == pBuffer->mCreateInfo.mSize);
	const bool bConstant = (pBuffer->mCreateInfo.mBindFlags & JunoBindFlags_ConstantBuffer) != 0;
	JUNO_REQUIRE(!bConstant || bWhole); // Partial updates of constant buffers are not allowed.

	if (bWhole)
	{
		pCtx->UpdateSubresource(pNative, 0, nullptr, pData, 0, 0);
	}
	else
	{
		D3D11_BOX box = { mOffset, 0, 0, mOffset + mSize, 1, 1 };
		pCtx->UpdateSubresource(pNative, 0, &box, pData, 0, 0);
	}
	return JunoErrorCode_Ok;
}

JUNO_API JunoErrorCode JunoMapBuffer(JunoDevice* pDevice, JunoBuffer* pBuffer, JunoMapMode mMode, void** ppData)
{
	JUNO_REQUIRE(pDevice != nullptr);
	JUNO_REQUIRE(pBuffer != nullptr);
	JUNO_REQUIRE(ppData != nullptr);
	*ppData = nullptr;

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	HRESULT hr = pDevice->mContext->Map(pBuffer->mHandle.Get(), 0, (D3D11_MAP)mMode, 0, &mapped);
	if (FAILED(hr))
		return JunoFailHResult(JunoErrorCode_MapFailed, hr, "ID3D11DeviceContext::Map");

	*ppData = mapped.pData;
	return JunoErrorCode_Ok;
}

JUNO_API void JunoUnmapBuffer(JunoDevice* pDevice, JunoBuffer* pBuffer)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(pBuffer != nullptr);
	pDevice->mContext->Unmap(pBuffer->mHandle.Get(), 0);
}

JUNO_API void JunoCopyBuffer(JunoDevice* pDevice, JunoBuffer* pDst, JunoBuffer* pSrc)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(pDst != nullptr);
	JUNO_REQUIRE_VOID(pSrc != nullptr);
	pDevice->mContext->CopyResource(pDst->mHandle.Get(), pSrc->mHandle.Get());
}

JUNO_API void JunoSetVertexBuffers(JunoDevice* pDevice, c_uint mStartSlot, c_uint mCount, JunoBuffer* const* ppBuffers, const c_uint* pStrides, const c_uint* pOffsets)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(mCount <= JUNO_MAX_VB_SLOTS);
	JUNO_REQUIRE_VOID(mCount == 0 || ppBuffers != nullptr);
	JUNO_REQUIRE_VOID(mCount == 0 || pStrides != nullptr);

	ID3D11Buffer* buffers[JUNO_MAX_VB_SLOTS] = {};
	UINT zeroOffsets[JUNO_MAX_VB_SLOTS] = {};
	for (c_uint i = 0; i < mCount; ++i)
		buffers[i] = ppBuffers[i] ? ppBuffers[i]->mHandle.Get() : nullptr;

	pDevice->mContext->IASetVertexBuffers(mStartSlot, mCount, buffers, pStrides, pOffsets ? pOffsets : zeroOffsets);
}

JUNO_API void JunoSetIndexBuffer(JunoDevice* pDevice, JunoBuffer* pBuffer, JunoIndexFormat mFormat, c_uint mOffset)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	DXGI_FORMAT format = (mFormat == JunoIndexFormat_UInt16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	pDevice->mContext->IASetIndexBuffer(pBuffer ? pBuffer->mHandle.Get() : nullptr, format, mOffset);
}

JUNO_API void JunoSetConstantBuffers(JunoDevice* pDevice, JunoShaderStage mStage, c_uint mStartSlot, c_uint mCount, JunoBuffer* const* ppBuffers)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(mStartSlot + mCount <= JUNO_MAX_CB_SLOTS);
	JUNO_REQUIRE_VOID(mCount == 0 || ppBuffers != nullptr);

	ID3D11Buffer* buffers[JUNO_MAX_CB_SLOTS] = {};
	for (c_uint i = 0; i < mCount; ++i)
		buffers[i] = ppBuffers[i] ? ppBuffers[i]->mHandle.Get() : nullptr;

	JunoBindConstantBuffers(pDevice->mContext.Get(), mStage, mStartSlot, mCount, buffers);
}

JUNO_API void JunoSetBufferResources(JunoDevice* pDevice, JunoShaderStage mStage, c_uint mStartSlot, c_uint mCount, JunoBuffer* const* ppBuffers)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(mStartSlot + mCount <= JUNO_MAX_SRV_SLOTS);
	JUNO_REQUIRE_VOID(mCount == 0 || ppBuffers != nullptr);

	ID3D11ShaderResourceView* views[JUNO_MAX_SRV_SLOTS] = {};
	for (c_uint i = 0; i < mCount; ++i)
		views[i] = ppBuffers[i] ? ppBuffers[i]->mSRV.Get() : nullptr;

	JunoBindShaderResources(pDevice->mContext.Get(), mStage, mStartSlot, mCount, views);
}

JUNO_API void JunoSetUnorderedAccessBuffers(JunoDevice* pDevice, c_uint mStartSlot, c_uint mCount, JunoBuffer* const* ppBuffers)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(mStartSlot + mCount <= JUNO_MAX_UAV_SLOTS);
	JUNO_REQUIRE_VOID(mCount == 0 || ppBuffers != nullptr);

	ID3D11UnorderedAccessView* views[JUNO_MAX_UAV_SLOTS] = {};
	for (c_uint i = 0; i < mCount; ++i)
		views[i] = ppBuffers[i] ? ppBuffers[i]->mUAV.Get() : nullptr;

	pDevice->mContext->CSSetUnorderedAccessViews(mStartSlot, mCount, views, nullptr);
}
