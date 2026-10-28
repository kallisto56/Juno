namespace Juno;

using System;
using System.Interop;

// Default vertex used by Graphics (immediate drawing, text, built-in meshes). 24 bytes.
//   POSITION  float3
//   TEXCOORD0 float2
//   COLOR0    R8G8B8A8_UNorm
[CRepr]
struct Vertex
{
	public float mX;
	public float mY;
	public float mZ;
	public float mU;
	public float mV;
	public uint32 mColor;       // R,G,B,A byte order (see Colors.PackRGBA)

	public const c_uint cSize = 24;

	public this(float x, float y, float z, float u, float v, uint32 color)
	{
		this.mX = x;
		this.mY = y;
		this.mZ = z;
		this.mU = u;
		this.mV = v;
		this.mColor = color;
	}

	public this(float x, float y, float z, float u, float v, Color color)
	{
		this.mX = x;
		this.mY = y;
		this.mZ = z;
		this.mU = u;
		this.mV = v;
		this.mColor = Colors.PackRGBA(color);
	}

	static public VertexLayout CreateLayout()
	{
		let layout = new VertexLayout(cSize);
		layout.Add("POSITION", .R32G32B32_Float, 0);
		layout.Add("TEXCOORD", .R32G32_Float, 12);
		layout.Add("COLOR", .R8G8B8A8_UNorm, 20);
		return layout;
	}
}
