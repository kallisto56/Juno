#include "pch.h"
#include "JunoInternal.h"

JUNO_API void JunoDraw(JunoDevice* pDevice, c_uint mVertexCount, c_uint mStartVertex)
{
	JUNO_ASSERT(pDevice != nullptr);
	pDevice->mContext->Draw(mVertexCount, mStartVertex);
}

JUNO_API void JunoDrawIndexed(JunoDevice* pDevice, c_uint mIndexCount, c_uint mStartIndex, c_int mBaseVertex)
{
	JUNO_ASSERT(pDevice != nullptr);
	pDevice->mContext->DrawIndexed(mIndexCount, mStartIndex, mBaseVertex);
}

JUNO_API void JunoDrawInstanced(JunoDevice* pDevice, c_uint mVertexCountPerInstance, c_uint mInstanceCount, c_uint mStartVertex, c_uint mStartInstance)
{
	JUNO_ASSERT(pDevice != nullptr);
	pDevice->mContext->DrawInstanced(mVertexCountPerInstance, mInstanceCount, mStartVertex, mStartInstance);
}

JUNO_API void JunoDrawIndexedInstanced(JunoDevice* pDevice, c_uint mIndexCountPerInstance, c_uint mInstanceCount, c_uint mStartIndex, c_int mBaseVertex, c_uint mStartInstance)
{
	JUNO_ASSERT(pDevice != nullptr);
	pDevice->mContext->DrawIndexedInstanced(mIndexCountPerInstance, mInstanceCount, mStartIndex, mBaseVertex, mStartInstance);
}

JUNO_API void JunoDispatch(JunoDevice* pDevice, c_uint mGroupsX, c_uint mGroupsY, c_uint mGroupsZ)
{
	JUNO_ASSERT(pDevice != nullptr);
	pDevice->mContext->Dispatch(mGroupsX, mGroupsY, mGroupsZ);
}
