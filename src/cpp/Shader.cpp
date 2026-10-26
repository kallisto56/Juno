#include "pch.h"
#include "JunoInternal.h"
#include <new>

static JunoErrorCode CompileFailed(HRESULT hr, ID3DBlob* pErrors)
{
	const char* pText = pErrors ? (const char*)pErrors->GetBufferPointer() : "No compiler output.";
	return JunoSetError(JunoErrorCode_ShaderCompileFailed, hr, "Shader compilation failed:\n%s", pText);
}

JUNO_API JunoErrorCode JunoCompileShader(const c_char* pSource, c_size mSourceLength, const c_char* pSourceName, const c_char* pEntryPoint, const c_char* pTarget, c_uint mCompileFlags, JunoBlob** ppBlob)
{
	JUNO_REQUIRE(pSource != nullptr);
	JUNO_REQUIRE(mSourceLength > 0);
	JUNO_REQUIRE(pEntryPoint != nullptr);
	JUNO_REQUIRE(pTarget != nullptr);
	JUNO_REQUIRE(ppBlob != nullptr);
	*ppBlob = nullptr;

	ComPtr<ID3DBlob> code;
	ComPtr<ID3DBlob> errors;
	HRESULT hr = D3DCompile(pSource, mSourceLength, pSourceName, nullptr, nullptr, pEntryPoint, pTarget, mCompileFlags, 0, &code, &errors);
	if (FAILED(hr))
		return CompileFailed(hr, errors.Get());

	JunoBlob* pBlob = new (std::nothrow) JunoBlob();
	JUNO_REQUIRE_ALLOC(pBlob);
	pBlob->mBlob = code;
	*ppBlob = pBlob;
	return JunoErrorCode_Ok;
}

JUNO_API JunoErrorCode JunoCompileShaderFromFile(const c_wchar* pPath, const c_char* pEntryPoint, const c_char* pTarget, c_uint mCompileFlags, JunoBlob** ppBlob)
{
	JUNO_REQUIRE(pPath != nullptr);
	JUNO_REQUIRE(pEntryPoint != nullptr);
	JUNO_REQUIRE(pTarget != nullptr);
	JUNO_REQUIRE(ppBlob != nullptr);
	*ppBlob = nullptr;

	ComPtr<ID3DBlob> code;
	ComPtr<ID3DBlob> errors;
	HRESULT hr = D3DCompileFromFile(pPath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, pEntryPoint, pTarget, mCompileFlags, 0, &code, &errors);
	if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) || hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
		return JunoSetError(JunoErrorCode_FileNotFound, hr, "Shader file not found");
	if (FAILED(hr))
		return CompileFailed(hr, errors.Get());

	JunoBlob* pBlob = new (std::nothrow) JunoBlob();
	JUNO_REQUIRE_ALLOC(pBlob);
	pBlob->mBlob = code;
	*ppBlob = pBlob;
	return JunoErrorCode_Ok;
}

JUNO_API const void* JunoGetBlobData(JunoBlob* pBlob)
{
	JUNO_REQUIRE_RETURN(pBlob != nullptr, nullptr);
	return pBlob->mBlob->GetBufferPointer();
}

JUNO_API c_size JunoGetBlobSize(JunoBlob* pBlob)
{
	JUNO_REQUIRE_RETURN(pBlob != nullptr, 0);
	return (c_size)pBlob->mBlob->GetBufferSize();
}

JUNO_API void JunoDestroyBlob(JunoBlob* pBlob)
{
	delete pBlob;
}

JUNO_API JunoErrorCode JunoCreateShader(JunoDevice* pDevice, JunoShaderStage mStage, const void* pBytecode, c_size mBytecodeSize, JunoShader** ppShader)
{
	JUNO_REQUIRE(pDevice != nullptr);
	JUNO_REQUIRE(pBytecode != nullptr);
	JUNO_REQUIRE(mBytecodeSize > 0);
	JUNO_REQUIRE(ppShader != nullptr);
	*ppShader = nullptr;

	JunoShader* pShader = new (std::nothrow) JunoShader();
	JUNO_REQUIRE_ALLOC(pShader);
	pShader->mStage = mStage;

	ID3D11Device* pNative = pDevice->mDevice.Get();
	HRESULT hr = E_INVALIDARG;

	switch (mStage)
	{
	case JunoShaderStage_Vertex:
	{
		ComPtr<ID3D11VertexShader> shader;
		hr = pNative->CreateVertexShader(pBytecode, mBytecodeSize, nullptr, &shader);
		pShader->mShader = shader;
		pShader->mBytecode.assign((const unsigned char*)pBytecode, (const unsigned char*)pBytecode + mBytecodeSize);
		break;
	}
	case JunoShaderStage_Pixel:
	{
		ComPtr<ID3D11PixelShader> shader;
		hr = pNative->CreatePixelShader(pBytecode, mBytecodeSize, nullptr, &shader);
		pShader->mShader = shader;
		break;
	}
	case JunoShaderStage_Geometry:
	{
		ComPtr<ID3D11GeometryShader> shader;
		hr = pNative->CreateGeometryShader(pBytecode, mBytecodeSize, nullptr, &shader);
		pShader->mShader = shader;
		break;
	}
	case JunoShaderStage_Compute:
	{
		ComPtr<ID3D11ComputeShader> shader;
		hr = pNative->CreateComputeShader(pBytecode, mBytecodeSize, nullptr, &shader);
		pShader->mShader = shader;
		break;
	}
	default:
		assert(false && "Unknown shader stage");
		break;
	}

	if (FAILED(hr))
	{
		delete pShader;
		return JunoFailHResult(JunoErrorCode_ShaderCreationFailed, hr, "ID3D11Device::Create*Shader");
	}

	*ppShader = pShader;
	return JunoErrorCode_Ok;
}

JUNO_API void JunoDestroyShader(JunoShader* pShader)
{
	delete pShader;
}

JUNO_API void JunoSetShader(JunoDevice* pDevice, JunoShaderStage mStage, JunoShader* pShader)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(pShader == nullptr || pShader->mStage == mStage);

	ID3D11DeviceChild* pNative = pShader ? pShader->mShader.Get() : nullptr;
	ID3D11DeviceContext* pCtx = pDevice->mContext.Get();

	switch (mStage)
	{
	case JunoShaderStage_Vertex: pCtx->VSSetShader(static_cast<ID3D11VertexShader*>(pNative), nullptr, 0); break;
	case JunoShaderStage_Pixel: pCtx->PSSetShader(static_cast<ID3D11PixelShader*>(pNative), nullptr, 0); break;
	case JunoShaderStage_Geometry: pCtx->GSSetShader(static_cast<ID3D11GeometryShader*>(pNative), nullptr, 0); break;
	case JunoShaderStage_Compute: pCtx->CSSetShader(static_cast<ID3D11ComputeShader*>(pNative), nullptr, 0); break;
	default: assert(false); break;
	}
}
