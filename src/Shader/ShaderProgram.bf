namespace Juno;

using System;
using System.IO;
using System.Interop;

// A vertex + pixel shader pair together with the input layout. Switch between programs with
// Graphics.SetShader / PushShader. Source files contain `VSMain` and `PSMain`.
//
// Constant buffer slots used by Graphics (declare them in HLSL to use them):
//   b0 = FrameConstants  { row_major float4x4 gViewProjection; float gTime; float gDeltaTime; float2 gViewportSize; }
//   b1 = ObjectConstants { row_major float4x4 gWorld; float4 gTint; }
//   b2..b7 = user buffers (Graphics.SetConstantBuffer)
// Texture slots t0..t3, sampler s0.
class ShaderProgram
{
	public Shader* mVertexShader;
	public Shader* mPixelShader;
	public InputLayout* mInputLayout;
	public String mName = new .() ~ delete _;

	public ~this()
	{
		if (mInputLayout != null) Juno.DestroyInputLayout(mInputLayout);
		if (mVertexShader != null) Juno.DestroyShader(mVertexShader);
		if (mPixelShader != null) Juno.DestroyShader(mPixelShader);
	}

	static public Result<ShaderProgram> Create(Device* device, StringView source, VertexLayout layout, StringView name = "shader")
	{
		let program = new ShaderProgram();
		program.mName.Set(name);
		if (!program.Build(device, source, layout))
		{
			delete program;
			return .Err;
		}
		return .Ok(program);
	}

	static public Result<ShaderProgram> FromFile(Device* device, StringView path, VertexLayout layout)
	{
		let text = scope String();
		if (File.ReadAllText(path, text) case .Err)
		{
			Console.WriteLine("[Juno] Could not read shader file '{}'", path);
			return .Err;
		}
		return Create(device, text, layout, path);
	}

	bool Build(Device* device, StringView source, VertexLayout layout)
	{
		let nameC = scope String(mName);

		var result = Juno.CompileShader(source.Ptr, (c_size)source.Length, nameC.CStr(), "VSMain", "vs_5_0", .None, var vsBlob);
		if (!JunoLog.Check(result, scope $"Compile {mName} (VSMain)"))
			return false;

		result = Juno.CompileShader(source.Ptr, (c_size)source.Length, nameC.CStr(), "PSMain", "ps_5_0", .None, var psBlob);
		if (!JunoLog.Check(result, scope $"Compile {mName} (PSMain)"))
		{
			Juno.DestroyBlob(vsBlob);
			return false;
		}

		result = Juno.CreateShader(device, .Vertex, Juno.GetBlobData(vsBlob), Juno.GetBlobSize(vsBlob), var vertexShader);
		bool bOk = JunoLog.Check(result, "CreateShader (vertex)");
		if (bOk)
		{
			mVertexShader = vertexShader;
			result = Juno.CreateShader(device, .Pixel, Juno.GetBlobData(psBlob), Juno.GetBlobSize(psBlob), var pixelShader);
			bOk = JunoLog.Check(result, "CreateShader (pixel)");
			if (bOk)
				mPixelShader = pixelShader;
		}
		Juno.DestroyBlob(vsBlob);
		Juno.DestroyBlob(psBlob);
		if (!bOk)
			return false;

		result = Juno.CreateInputLayout(device, &layout.mElements[0], (c_uint)layout.mElements.Count, mVertexShader, var inputLayout);
		if (!JunoLog.Check(result, "CreateInputLayout"))
			return false;
		mInputLayout = inputLayout;
		return true;
	}
}
