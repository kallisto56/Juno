namespace Juno;

using System;
using System.Diagnostics;
using System.Interop;


// Native Juno.dll entry points. Call as Juno.Foo(...) with `using Juno;`.
// "GpuBuffer" is the Beef name of JunoBuffer (Buffer would clash with System.Buffer).
// Out parameters are called like: Juno.CreateDevice(&desc, var device);
static class Juno
{
	[CallingConvention(.Cdecl), LinkName("JunoGetLastErrorMessage")]
	extern static public c_char* GetLastErrorMessage();

	[CallingConvention(.Cdecl), LinkName("JunoGetLastErrorCode")]
	extern static public ErrorCode GetLastErrorCode();

	[CallingConvention(.Cdecl), LinkName("JunoGetLastErrorHResult")]
	extern static public c_int GetLastErrorHResult();

	[CallingConvention(.Cdecl), LinkName("JunoClearLastError")]
	extern static public void ClearLastError();

	[CallingConvention(.Cdecl), LinkName("JunoCreateDevice")]
	extern static public ErrorCode CreateDevice(DeviceDesc* pDesc, out Device* ppDevice);

	[CallingConvention(.Cdecl), LinkName("JunoDestroyDevice")]
	extern static public void DestroyDevice(Device* pDevice);

	[CallingConvention(.Cdecl), LinkName("JunoGetFeatureLevel")]
	extern static public c_uint GetFeatureLevel(Device* pDevice);

	[CallingConvention(.Cdecl), LinkName("JunoIsTearingSupported")]
	extern static public c_bool IsTearingSupported(Device* pDevice);

	[CallingConvention(.Cdecl), LinkName("JunoGetNativeDevice")]
	extern static public void* GetNativeDevice(Device* pDevice);

	[CallingConvention(.Cdecl), LinkName("JunoGetNativeContext")]
	extern static public void* GetNativeContext(Device* pDevice);

	[CallingConvention(.Cdecl), LinkName("JunoFlush")]
	extern static public void Flush(Device* pDevice);

	[CallingConvention(.Cdecl), LinkName("JunoClearState")]
	extern static public void ClearState(Device* pDevice);

	[CallingConvention(.Cdecl), LinkName("JunoCreateSwapChain")]
	extern static public ErrorCode CreateSwapChain(Device* pDevice, SwapChainDesc* pDesc, out SwapChain* ppSwapChain);

	[CallingConvention(.Cdecl), LinkName("JunoDestroySwapChain")]
	extern static public void DestroySwapChain(SwapChain* pSwapChain);

	[CallingConvention(.Cdecl), LinkName("JunoResizeSwapChain")]
	extern static public ErrorCode ResizeSwapChain(SwapChain* pSwapChain, c_uint mWidth, c_uint mHeight);

	[CallingConvention(.Cdecl), LinkName("JunoPresent")]
	extern static public ErrorCode Present(SwapChain* pSwapChain, c_bool bVSync);

	[CallingConvention(.Cdecl), LinkName("JunoGetBackBuffer")]
	extern static public Texture* GetBackBuffer(SwapChain* pSwapChain);

	[CallingConvention(.Cdecl), LinkName("JunoGetDepthBuffer")]
	extern static public Texture* GetDepthBuffer(SwapChain* pSwapChain);

	[CallingConvention(.Cdecl), LinkName("JunoCreateBuffer")]
	extern static public ErrorCode CreateBuffer(Device* pDevice, BufferDesc* pDesc, void* pInitialData, out GpuBuffer* ppBuffer);

	[CallingConvention(.Cdecl), LinkName("JunoDestroyBuffer")]
	extern static public void DestroyBuffer(GpuBuffer* pBuffer);

	[CallingConvention(.Cdecl), LinkName("JunoUpdateBuffer")]
	extern static public ErrorCode UpdateBuffer(Device* pDevice, GpuBuffer* pBuffer, c_size mOffset, void* pData, c_size mSize);

	[CallingConvention(.Cdecl), LinkName("JunoMapBuffer")]
	extern static public ErrorCode MapBuffer(Device* pDevice, GpuBuffer* pBuffer, MapMode mMode, out void* ppData);

	[CallingConvention(.Cdecl), LinkName("JunoUnmapBuffer")]
	extern static public void UnmapBuffer(Device* pDevice, GpuBuffer* pBuffer);

	[CallingConvention(.Cdecl), LinkName("JunoCopyBuffer")]
	extern static public void CopyBuffer(Device* pDevice, GpuBuffer* pDst, GpuBuffer* pSrc);

	[CallingConvention(.Cdecl), LinkName("JunoSetVertexBuffers")]
	extern static public void SetVertexBuffers(Device* pDevice, c_uint mStartSlot, c_uint mCount, GpuBuffer** ppBuffers, c_uint* pStrides, c_uint* pOffsets);

	[CallingConvention(.Cdecl), LinkName("JunoSetIndexBuffer")]
	extern static public void SetIndexBuffer(Device* pDevice, GpuBuffer* pBuffer, IndexFormat mFormat, c_uint mOffset);

	[CallingConvention(.Cdecl), LinkName("JunoSetConstantBuffers")]
	extern static public void SetConstantBuffers(Device* pDevice, ShaderStage mStage, c_uint mStartSlot, c_uint mCount, GpuBuffer** ppBuffers);

	[CallingConvention(.Cdecl), LinkName("JunoSetBufferResources")]
	extern static public void SetBufferResources(Device* pDevice, ShaderStage mStage, c_uint mStartSlot, c_uint mCount, GpuBuffer** ppBuffers);

	[CallingConvention(.Cdecl), LinkName("JunoSetUnorderedAccessBuffers")]
	extern static public void SetUnorderedAccessBuffers(Device* pDevice, c_uint mStartSlot, c_uint mCount, GpuBuffer** ppBuffers);

	[CallingConvention(.Cdecl), LinkName("JunoCreateTexture")]
	extern static public ErrorCode CreateTexture(Device* pDevice, TextureDesc* pDesc, void* pInitialData, c_uint mRowPitch, out Texture* ppTexture);

	[CallingConvention(.Cdecl), LinkName("JunoDestroyTexture")]
	extern static public void DestroyTexture(Texture* pTexture);

	[CallingConvention(.Cdecl), LinkName("JunoUpdateTexture")]
	extern static public ErrorCode UpdateTexture(Device* pDevice, Texture* pTexture, c_uint mArraySlice, c_uint mMipLevel, Region* pRegion, void* pData, c_uint mRowPitch);

	[CallingConvention(.Cdecl), LinkName("JunoMapTexture")]
	extern static public ErrorCode MapTexture(Device* pDevice, Texture* pTexture, c_uint mArraySlice, c_uint mMipLevel, MapMode mMode, out void* ppData, c_uint* pRowPitch);

	[CallingConvention(.Cdecl), LinkName("JunoUnmapTexture")]
	extern static public void UnmapTexture(Device* pDevice, Texture* pTexture, c_uint mArraySlice, c_uint mMipLevel);

	[CallingConvention(.Cdecl), LinkName("JunoGenerateMips")]
	extern static public void GenerateMips(Device* pDevice, Texture* pTexture);

	[CallingConvention(.Cdecl), LinkName("JunoCopyTexture")]
	extern static public void CopyTexture(Device* pDevice, Texture* pDst, Texture* pSrc);

	[CallingConvention(.Cdecl), LinkName("JunoResolveTexture")]
	extern static public void ResolveTexture(Device* pDevice, Texture* pDst, Texture* pSrc);

	[CallingConvention(.Cdecl), LinkName("JunoGetTextureSize")]
	extern static public void GetTextureSize(Texture* pTexture, c_uint* pWidth, c_uint* pHeight);

	[CallingConvention(.Cdecl), LinkName("JunoClearRenderTarget")]
	extern static public void ClearRenderTarget(Device* pDevice, Texture* pTexture, float mR, float mG, float mB, float mA);

	[CallingConvention(.Cdecl), LinkName("JunoClearDepthStencil")]
	extern static public void ClearDepthStencil(Device* pDevice, Texture* pTexture, ClearFlags mClearFlags, float mDepth, c_uchar mStencil);

	[CallingConvention(.Cdecl), LinkName("JunoSetTextures")]
	extern static public void SetTextures(Device* pDevice, ShaderStage mStage, c_uint mStartSlot, c_uint mCount, Texture** ppTextures);

	[CallingConvention(.Cdecl), LinkName("JunoSetUnorderedAccessTextures")]
	extern static public void SetUnorderedAccessTextures(Device* pDevice, c_uint mStartSlot, c_uint mCount, Texture** ppTextures);

	[CallingConvention(.Cdecl), LinkName("JunoCreateSampler")]
	extern static public ErrorCode CreateSampler(Device* pDevice, SamplerDesc* pDesc, out Sampler* ppSampler);

	[CallingConvention(.Cdecl), LinkName("JunoDestroySampler")]
	extern static public void DestroySampler(Sampler* pSampler);

	[CallingConvention(.Cdecl), LinkName("JunoSetSamplers")]
	extern static public void SetSamplers(Device* pDevice, ShaderStage mStage, c_uint mStartSlot, c_uint mCount, Sampler** ppSamplers);

	[CallingConvention(.Cdecl), LinkName("JunoCompileShader")]
	extern static public ErrorCode CompileShader(c_char* pSource, c_size mSourceLength, c_char* pSourceName, c_char* pEntryPoint, c_char* pTarget, ShaderCompileFlags mCompileFlags, out Blob* ppBlob);

	[CallingConvention(.Cdecl), LinkName("JunoCompileShaderFromFile")]
	extern static public ErrorCode CompileShaderFromFile(char16* pPath, c_char* pEntryPoint, c_char* pTarget, ShaderCompileFlags mCompileFlags, out Blob* ppBlob);

	[CallingConvention(.Cdecl), LinkName("JunoGetBlobData")]
	extern static public void* GetBlobData(Blob* pBlob);

	[CallingConvention(.Cdecl), LinkName("JunoGetBlobSize")]
	extern static public c_size GetBlobSize(Blob* pBlob);

	[CallingConvention(.Cdecl), LinkName("JunoDestroyBlob")]
	extern static public void DestroyBlob(Blob* pBlob);

	[CallingConvention(.Cdecl), LinkName("JunoCreateShader")]
	extern static public ErrorCode CreateShader(Device* pDevice, ShaderStage mStage, void* pBytecode, c_size mBytecodeSize, out Shader* ppShader);

	[CallingConvention(.Cdecl), LinkName("JunoDestroyShader")]
	extern static public void DestroyShader(Shader* pShader);

	[CallingConvention(.Cdecl), LinkName("JunoSetShader")]
	extern static public void SetShader(Device* pDevice, ShaderStage mStage, Shader* pShader);

	[CallingConvention(.Cdecl), LinkName("JunoCreateInputLayout")]
	extern static public ErrorCode CreateInputLayout(Device* pDevice, InputElementDesc* pElements, c_uint mElementCount, Shader* pVertexShader, out InputLayout* ppLayout);

	[CallingConvention(.Cdecl), LinkName("JunoDestroyInputLayout")]
	extern static public void DestroyInputLayout(InputLayout* pLayout);

	[CallingConvention(.Cdecl), LinkName("JunoSetInputLayout")]
	extern static public void SetInputLayout(Device* pDevice, InputLayout* pLayout);

	[CallingConvention(.Cdecl), LinkName("JunoCreateBlendState")]
	extern static public ErrorCode CreateBlendState(Device* pDevice, BlendDesc* pDesc, out BlendState* ppState);

	[CallingConvention(.Cdecl), LinkName("JunoDestroyBlendState")]
	extern static public void DestroyBlendState(BlendState* pState);

	[CallingConvention(.Cdecl), LinkName("JunoSetBlendState")]
	extern static public void SetBlendState(Device* pDevice, BlendState* pState, float* pBlendFactor, c_uint mSampleMask);

	[CallingConvention(.Cdecl), LinkName("JunoCreateRasterizerState")]
	extern static public ErrorCode CreateRasterizerState(Device* pDevice, RasterizerDesc* pDesc, out RasterizerState* ppState);

	[CallingConvention(.Cdecl), LinkName("JunoDestroyRasterizerState")]
	extern static public void DestroyRasterizerState(RasterizerState* pState);

	[CallingConvention(.Cdecl), LinkName("JunoSetRasterizerState")]
	extern static public void SetRasterizerState(Device* pDevice, RasterizerState* pState);

	[CallingConvention(.Cdecl), LinkName("JunoCreateDepthStencilState")]
	extern static public ErrorCode CreateDepthStencilState(Device* pDevice, DepthStencilDesc* pDesc, out DepthStencilState* ppState);

	[CallingConvention(.Cdecl), LinkName("JunoDestroyDepthStencilState")]
	extern static public void DestroyDepthStencilState(DepthStencilState* pState);

	[CallingConvention(.Cdecl), LinkName("JunoSetDepthStencilState")]
	extern static public void SetDepthStencilState(Device* pDevice, DepthStencilState* pState, c_uint mStencilRef);

	[CallingConvention(.Cdecl), LinkName("JunoSetRenderTargets")]
	extern static public void SetRenderTargets(Device* pDevice, c_uint mCount, Texture** ppColorTargets, Texture* pDepthTarget);

	[CallingConvention(.Cdecl), LinkName("JunoSetViewports")]
	extern static public void SetViewports(Device* pDevice, c_uint mCount, Viewport* pViewports);

	[CallingConvention(.Cdecl), LinkName("JunoSetScissorRects")]
	extern static public void SetScissorRects(Device* pDevice, c_uint mCount, Region* pRects);

	[CallingConvention(.Cdecl), LinkName("JunoSetPrimitiveTopology")]
	extern static public void SetPrimitiveTopology(Device* pDevice, PrimitiveTopology mTopology);

	[CallingConvention(.Cdecl), LinkName("JunoDraw")]
	extern static public void Draw(Device* pDevice, c_uint mVertexCount, c_uint mStartVertex);

	[CallingConvention(.Cdecl), LinkName("JunoDrawIndexed")]
	extern static public void DrawIndexed(Device* pDevice, c_uint mIndexCount, c_uint mStartIndex, c_int mBaseVertex);

	[CallingConvention(.Cdecl), LinkName("JunoDrawInstanced")]
	extern static public void DrawInstanced(Device* pDevice, c_uint mVertexCountPerInstance, c_uint mInstanceCount, c_uint mStartVertex, c_uint mStartInstance);

	[CallingConvention(.Cdecl), LinkName("JunoDrawIndexedInstanced")]
	extern static public void DrawIndexedInstanced(Device* pDevice, c_uint mIndexCountPerInstance, c_uint mInstanceCount, c_uint mStartIndex, c_int mBaseVertex, c_uint mStartInstance);

	[CallingConvention(.Cdecl), LinkName("JunoDispatch")]
	extern static public void Dispatch(Device* pDevice, c_uint mGroupsX, c_uint mGroupsY, c_uint mGroupsZ);

	[CallingConvention(.Cdecl), LinkName("JunoCreateFence")]
	extern static public ErrorCode CreateFence(Device* pDevice, out Fence* ppFence);

	[CallingConvention(.Cdecl), LinkName("JunoDestroyFence")]
	extern static public void DestroyFence(Fence* pFence);

	[CallingConvention(.Cdecl), LinkName("JunoInsertFence")]
	extern static public void InsertFence(Device* pDevice, Fence* pFence);

	[CallingConvention(.Cdecl), LinkName("JunoIsFenceSignaled")]
	extern static public c_bool IsFenceSignaled(Device* pDevice, Fence* pFence);

	[CallingConvention(.Cdecl), LinkName("JunoWaitFence")]
	extern static public void WaitFence(Device* pDevice, Fence* pFence);
}
