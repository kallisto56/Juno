namespace Juno;

using System;
using System.Interop;

[CRepr]
struct InputElementDesc
{
	public const c_uint cAppendAlignedElement = 0xFFFFFFFF;

	public c_char* mSemanticName = null;
	public c_uint mSemanticIndex = 0;
	public PixelFormat mFormat = .Unknown;
	public c_uint mInputSlot = 0;
	public c_uint mAlignedByteOffset = cAppendAlignedElement;
	public InputClassification mInputSlotClass = .PerVertexData;
	public c_uint mInstanceDataStepRate = 0;

	public this(c_char* semanticName, PixelFormat format, c_uint alignedByteOffset = cAppendAlignedElement)
	{
		mSemanticName = semanticName;
		mSemanticIndex = 0;
		mFormat = format;
		mInputSlot = 0;
		mAlignedByteOffset = alignedByteOffset;
		mInputSlotClass = .PerVertexData;
		mInstanceDataStepRate = 0;
	}
}
