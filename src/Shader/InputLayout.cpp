#include "pch.h"
#include "./../Common/JunoInternal.h"

JUNO_API JunoErrorCode JunoCreateInputLayout(JunoDevice* pDevice, const JunoInputElementDescription* pElements, c_uint mElementCount, JunoShader* pVertexShader, JunoInputLayout** ppLayout)
{
	JUNO_REQUIRE(pDevice != nullptr);
	JUNO_REQUIRE(pElements != nullptr);
	JUNO_REQUIRE(mElementCount > 0);
	JUNO_REQUIRE(pVertexShader != nullptr);
	JUNO_REQUIRE(pVertexShader->mStage == JunoShaderStage_Vertex);
	JUNO_REQUIRE(ppLayout != nullptr);
	*ppLayout = nullptr;

	std::vector<D3D11_INPUT_ELEMENT_DESC> elements(mElementCount);
	for (c_uint i = 0; i < mElementCount; ++i)
	{
		JUNO_REQUIRE(pElements[i].mSemanticName != nullptr);

		D3D11_INPUT_ELEMENT_DESC& e = elements[i];
		e.SemanticName = pElements[i].mSemanticName;
		e.SemanticIndex = pElements[i].mSemanticIndex;
		e.Format = (DXGI_FORMAT)pElements[i].mFormat;
		e.InputSlot = pElements[i].mInputSlot;
		e.AlignedByteOffset = pElements[i].mAlignedByteOffset;
		e.InputSlotClass = (D3D11_INPUT_CLASSIFICATION)pElements[i].mInputSlotClass;
		e.InstanceDataStepRate = pElements[i].mInstanceDataStepRate;
	}

	JunoInputLayout* pLayout = new (std::nothrow) JunoInputLayout();
	JUNO_REQUIRE_ALLOC(pLayout);

	HRESULT hr = pDevice->mDevice->CreateInputLayout(elements.data(), mElementCount, pVertexShader->mBytecode.data(), pVertexShader->mBytecode.size(), &pLayout->mLayout);
	if (FAILED(hr))
	{
		delete pLayout;
		return JunoFailHResult(JunoErrorCode_InputLayoutCreationFailed, hr, "ID3D11Device::CreateInputLayout");
	}

	*ppLayout = pLayout;
	return JunoErrorCode_Ok;
}

JUNO_API void JunoDestroyInputLayout(JunoInputLayout* pLayout)
{
	delete pLayout;
}

JUNO_API void JunoSetInputLayout(JunoDevice* pDevice, JunoInputLayout* pLayout)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	pDevice->mContext->IASetInputLayout(pLayout ? pLayout->mLayout.Get() : nullptr);
}
