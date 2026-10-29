#include "pch.h"
#include "JunoInternal.h"

JUNO_API JunoErrorCode JunoCreateFence(JunoDevice* pDevice, JunoFence** ppFence)
{
	JUNO_REQUIRE(pDevice != nullptr);
	JUNO_REQUIRE(ppFence != nullptr);
	*ppFence = nullptr;

	D3D11_QUERY_DESC qd = {};
	qd.Query = D3D11_QUERY_EVENT;

	JunoFence* pFence = new (std::nothrow) JunoFence();
	JUNO_REQUIRE_ALLOC(pFence);

	HRESULT hr = pDevice->mDevice->CreateQuery(&qd, &pFence->mQuery);
	if (FAILED(hr))
	{
		delete pFence;
		return JunoFailHResult(JunoErrorCode_FenceCreationFailed, hr, "ID3D11Device::CreateQuery");
	}

	*ppFence = pFence;
	return JunoErrorCode_Ok;
}

JUNO_API void JunoDestroyFence(JunoFence* pFence)
{
	delete pFence;
}

// Signals when the GPU has finished all work submitted before this call.
JUNO_API void JunoInsertFence(JunoDevice* pDevice, JunoFence* pFence)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(pFence != nullptr);
	pDevice->mContext->End(pFence->mQuery.Get());
}

JUNO_API c_bool JunoIsFenceSignaled(JunoDevice* pDevice, JunoFence* pFence)
{
	JUNO_REQUIRE_RETURN(pDevice != nullptr, false);
	JUNO_REQUIRE_RETURN(pFence != nullptr, false);

	BOOL done = FALSE;
	HRESULT hr = pDevice->mContext->GetData(pFence->mQuery.Get(), &done, sizeof(done), 0);
	return hr == S_OK && done;
}

JUNO_API void JunoWaitFence(JunoDevice* pDevice, JunoFence* pFence)
{
	JUNO_REQUIRE_VOID(pDevice != nullptr);
	JUNO_REQUIRE_VOID(pFence != nullptr);

	BOOL done = FALSE;
	HRESULT hr;
	while ((hr = pDevice->mContext->GetData(pFence->mQuery.Get(), &done, sizeof(done), 0)) == S_FALSE)
		SwitchToThread();
}
