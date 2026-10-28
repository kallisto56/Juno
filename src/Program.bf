namespace JunoDemo;

using System;
using System.Collections;
using Juno;

[CRepr]
struct PostConstants
{
	public float mAmount;       // 0 = color, 1 = grayscale
	public float mPad0;
	public float mPad1;
	public float mPad2;
}

// Showcase for the application layer:
//   - Application / window / input (WASD or arrows move the dot, typing is echoed, Esc quits, P toggles post-FX)
//   - 3D: a static Mesh drawn with a camera into an off-screen RenderTarget
//   - Shader switching: default shader for the scene, a post-processing shader for the full-screen composite
//   - Immediate 2D drawing: shapes, transform/clip stacks, textures, tint
//   - Text: FreeType + HarfBuzz (kerning, wrapping, alignment, Cyrillic, Arabic shaping)
//   - Widgets: Panel / Label / Button with anchors, hover, focus and click events
class Program : Application
{
	const String cPostShader = @"""
cbuffer FrameConstants : register(b0)
{
	row_major float4x4 gViewProjection;
	float gTime;
	float gDeltaTime;
	float2 gViewportSize;
};

cbuffer ObjectConstants : register(b1)
{
	row_major float4x4 gWorld;
	float4 gTint;
};

cbuffer PostConstants : register(b2)
{
	float gAmount;
	float3 gPad;
};

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

struct VSInput
{
	float3 position : POSITION;
	float2 uv       : TEXCOORD;
	float4 color    : COLOR;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv       : TEXCOORD;
	float4 color    : COLOR;
};

PSInput VSMain(VSInput input)
{
	PSInput output;
	output.position = mul(mul(float4(input.position, 1.0f), gWorld), gViewProjection);
	output.uv = input.uv;
	output.color = input.color * gTint;
	return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	float4 c = gTexture.Sample(gSampler, input.uv) * input.color;
	float gray = dot(c.rgb, float3(0.299f, 0.587f, 0.114f));
	c.rgb = lerp(c.rgb, gray.xxx, gAmount);

	float2 d = input.uv - 0.5f;
	c.rgb *= saturate(1.1f - dot(d, d) * 1.4f);
	return c;
}
""";

	Mesh mCube;
	Texture2D mChecker;
	RenderTarget mScene;
	ShaderProgram mPostShader;
	ConstantBuffer mPostConstants;
	Font mBigFont;

	Panel mPanel;
	Label mStatsLabel;

	bool bPostEffect = true;
	bool bWireframe = false;
	float mAngle;
	Vector2 mMarker = Vector2(120, 520);
	String mTyped = new .() ~ delete _;
	String mLastKey = new .("(none)") ~ delete _;

	public this()
	{
		mSettings.mTitle = "Juno Framework Demo";
		mSettings.mWidth = 1280;
		mSettings.mHeight = 800;
	}

	public override void OnInit()
	{
		if (CreateCubeMesh(1.6f) case .Ok(let mesh))
			mCube = mesh;

		// Procedural checkerboard texture
		let pixels = new uint8[64 * 64 * 4];
		defer delete pixels;
		for (int y < 64)
		{
			for (int x < 64)
			{
				let bLight = (((x / 8) + (y / 8)) & 1) == 0;
				let value = (uint8)(bLight ? 255 : 90);
				let o = (y * 64 + x) * 4;
				pixels[o] = value;
				pixels[o + 1] = value;
				pixels[o + 2] = value;
				pixels[o + 3] = 255;
			}
		}
		if (CreateTexture(64, 64, &pixels[0]) case .Ok(let texture))
			mChecker = texture;

		if (CreateRenderTarget(mWindowWidth, mWindowHeight) case .Ok(let target))
			mScene = target;
		if (CreateShader(cPostShader, "PostFx") case .Ok(let program))
			mPostShader = program;
		if (CreateConstantBuffer(sizeof(PostConstants)) case .Ok(let buffer))
			mPostConstants = buffer;
		if (LoadSystemFont(34) case .Ok(let font))
			mBigFont = font;

		BuildUI();
	}

	void BuildUI()
	{
		mPanel = mRoot.AddChild(new Panel((float)mWindowWidth - 280, 12, 268, 214));
		mPanel.mAnchor = .Right | .Top;

		mPanel.AddChild(new Label(12, 8, "Widgets"));
		mStatsLabel = mPanel.AddChild(new Label(12, 32, "..."));

		let postButton = mPanel.AddChild(new Button(12, 66, 244, 32, "Post-processing  (P)"));
		postButton.mOnClick.Add(new (widget) => { bPostEffect = !bPostEffect; });

		let wireButton = mPanel.AddChild(new Button(12, 106, 244, 32, "Wireframe cube"));
		wireButton.mOnClick.Add(new (widget) => { bWireframe = !bWireframe; });

		let quitButton = mPanel.AddChild(new Button(12, 146, 244, 32, "Quit  (Esc)"));
		quitButton.mOnClick.Add(new (widget) => { Stop(); });
	}

	public override void OnResize(int width, int height)
	{
		if (mScene != null)
			mScene.Resize(width, height);
	}

	public override void OnKeyDown(KeyCode key, bool isRepeat)
	{
		if (!isRepeat)
			mLastKey.Set(scope $"{key}");
	}

	public override void OnUpdate(float dt)
	{
		mAngle += dt;

		if (mInput.WasKeyPressed(.Escape))
			Stop();
		if (mInput.WasKeyPressed(.P))
			bPostEffect = !bPostEffect;

		let speed = 240.0f * dt;
		if (mInput.IsKeyDown(.W) || mInput.IsKeyDown(.Up)) mMarker.mY -= speed;
		if (mInput.IsKeyDown(.S) || mInput.IsKeyDown(.Down)) mMarker.mY += speed;
		if (mInput.IsKeyDown(.A) || mInput.IsKeyDown(.Left)) mMarker.mX -= speed;
		if (mInput.IsKeyDown(.D) || mInput.IsKeyDown(.Right)) mMarker.mX += speed;

		// Text input (arrives only when no widget has keyboard focus)
		if (mInput.mTextInput.Length > 0)
			mTyped.Append(mInput.mTextInput);
		if (mInput.WasKeyTyped(.Backspace))
		{
			while (mTyped.Length > 0)
			{
				let last = mTyped[mTyped.Length - 1];
				mTyped.RemoveFromEnd(1);
				if (((uint8)last & 0xC0) != 0x80)   // stop after removing the first byte of a UTF-8 sequence
					break;
			}
		}

		if (mStatsLabel != null)
			mStatsLabel.SetText(scope $"FPS {(int)mFPS}   draws {mGraphics.mDrawCalls}   verts {mGraphics.mVerticesPushed}");
	}

	public override void OnDraw(Graphics g)
	{
		let w = (float)mWindowWidth;
		let h = (float)mWindowHeight;

		// ---------------------------------------------------------------------------------------------------
		// 1) 3D scene into an off-screen target (static mesh + camera + depth)
		// ---------------------------------------------------------------------------------------------------
		if (mScene != null && mCube != null)
		{
			g.SetRenderTarget(mScene);
			g.Clear(Colors.Make(30, 36, 52, 255));
			g.ClearDepth();

			g.SetBlendMode(.Opaque);
			g.SetDepthMode(.ReadWrite);
			g.SetCullMode(.Back);
			g.SetWireframe(bWireframe);

			let view = Matrix4x4.CreateLookAt(Vector3(0, 1.8f, -4.2f), Vector3(0, 0, 0), Vector3(0, 1, 0));
			let projection = Matrix4x4.CreatePerspectiveFieldOfView(Math.PI_f / 4.0f, w / h, 0.1f, 100.0f);
			g.SetCamera(view, projection);

			let spin = Matrix4x4.CreateRotationY(mAngle) * Matrix4x4.CreateRotationX(mAngle * 0.6f);
			g.DrawMesh(mCube, spin, mChecker);
			g.DrawMesh(mCube, Matrix4x4.CreateScale(Vector3(0.45f, 0.45f, 0.45f)) * Matrix4x4.CreateTranslation(Vector3(Math.Sin(mAngle * 1.7f) * 2.2f, 0.2f, Math.Cos(mAngle * 1.7f) * 2.2f)), null, Colors.sOrange);

			// Back to the window: this resets state, camera and render target.
			g.SetRenderTarget(null);
			g.ResetState();

			// ---------------------------------------------------------------------------------------------------
			// 2) Full-screen composite. Switching to another shader is a single call.
			// ---------------------------------------------------------------------------------------------------
			g.SetBlendMode(.Opaque);
			if (bPostEffect && mPostShader != null && mPostConstants != null)
			{
				PostConstants constants = default;
				constants.mAmount = (Math.Sin(mAngle) * 0.5f + 0.5f) * 0.85f;
				mPostConstants.Update(constants);
				g.SetShader(mPostShader);
				g.SetConstantBuffer(0, mPostConstants);
			}
			g.DrawTexture(mScene.mColor, 0, 0, w, h);
			g.SetShader(null);
			g.SetConstantBuffer(0, null);
			g.SetBlendMode(.Alpha);
		}

		// ---------------------------------------------------------------------------------------------------
		// 3) Immediate 2D drawing
		// ---------------------------------------------------------------------------------------------------
		DrawShapes(g);
		DrawTextSamples(g);
		DrawInputInfo(g);

		// Movable dot (WASD / arrows)
		g.FillCircle(mMarker.mX, mMarker.mY, 14, Colors.sYellow);
		g.DrawCircle(mMarker.mX, mMarker.mY, 18, Colors.sWhite, 2);
	}

	void DrawShapes(Graphics g)
	{
		let top = 70.0f;

		g.FillRect(20, top, 90, 60, Colors.sRed);
		g.DrawRect(Rect(120, top, 90, 60), Colors.sGreen, 3);
		g.FillRectGradient(Rect(220, top, 90, 60), Colors.sRed, Colors.sYellow, Colors.sGreen, Colors.sBlue);
		g.FillCircle(355, top + 30, 30, Colors.sCyan);
		g.DrawCircle(430, top + 30, 30, Colors.sMagenta, 3);
		g.FillTriangle(470, top + 60, 515, top, 560, top + 60, Colors.sOrange);
		g.DrawLine(580, top, 640, top + 60, Colors.sWhite, 4);

		// Transform stack: translate, then rotate around the local origin.
		g.PushTranslate(690, top + 30);
		g.PushRotate(mAngle * 2.0f);
		g.FillRect(-28, -28, 56, 56, Colors.sBlue);
		g.PopTransform();
		g.PopTransform();

		// Clip stack: a big circle that is clipped to a small rectangle.
		let clip = Rect(750, top, 90, 60);
		g.DrawRect(clip.Inflate(2), Colors.sGray, 1);
		g.PushClip(clip);
		g.FillCircle(795 + Math.Sin(mAngle * 2.0f) * 40, top + 30, 40, Colors.sGreen);
		g.PopClip();

		// Textures: whole image, sub-rectangle, tinted + alpha
		if (mChecker != null)
		{
			g.DrawTexture(mChecker, 20, 150, 96, 96);
			g.DrawTexture(mChecker, Rect(130, 150, 96, 96), Rect(0, 0, 32, 32), Colors.sCyan);
			g.PushTint(Colors.Make(255, 255, 255, 120));
			g.DrawTexture(mChecker, 240, 150, 96, 96);
			g.PopTint();
		}
	}

	void DrawTextSamples(Graphics g)
	{
		let font = mDefaultFont;
		if (font == null)
			return;

		if (mBigFont != null)
			g.DrawString(mBigFont, "Juno Framework Demo", 20, 14, Colors.sWhite);

		let x = 20.0f;
		var y = 270.0f;
		let step = font.mLineHeight + 4;

		g.DrawString(font, "Kerning & ligatures: AVATAR  To  Wa  office affinity", x, y, Colors.sWhite); y += step;
		g.DrawString(font, "Latin extended: Ünïcödé  çàfé  Ångström  ß", x, y, Colors.sYellow); y += step;
		g.DrawString(font, "Cyrillic: Привет, мир!", x, y, Colors.sCyan); y += step;
		g.DrawString(font, "Arabic (HarfBuzz joining): مرحبا بالعالم", x, y, Colors.sGreen); y += step;

		// Wrapped, centered paragraph inside a measured box
		let paragraph = "Word wrapping works at spaces and respects the maximum width.\nA new line starts after a line feed.";
		let boxWidth = 330.0f;
		let size = g.MeasureString(font, paragraph, boxWidth);
		g.FillRect(x, y + 4, boxWidth, size.mY + 8, Colors.Make(255, 255, 255, 24));
		g.DrawString(font, paragraph, x, y + 8, Colors.sWhite, .Center, boxWidth);
	}

	void DrawInputInfo(Graphics g)
	{
		let font = mDefaultFont;
		if (font == null)
			return;

		let x = 20.0f;
		var y = (float)mWindowHeight - 140.0f;
		let step = font.mLineHeight + 2;

		let mouse = mInput.mMousePosition;
		g.DrawString(font, scope $"Mouse: {(int)mouse.mX}, {(int)mouse.mY}   L:{mInput.IsMouseDown(.Left)} R:{mInput.IsMouseDown(.Right)} M:{mInput.IsMouseDown(.Middle)}   wheel: {mInput.mMouseWheel.mY}", x, y, Colors.sGray); y += step;
		g.DrawString(font, scope $"Last key: {mLastKey}   Shift:{mInput.IsKeyDown(.Shift)}  Ctrl:{mInput.IsKeyDown(.Control)}  Alt:{mInput.IsKeyDown(.Alt)}", x, y, Colors.sGray); y += step;
		g.DrawString(font, "Type something (Backspace deletes). WASD / arrows move the yellow dot.", x, y, Colors.sGray); y += step;
		g.DrawString(font, scope $"> {mTyped}", x, y, Colors.sWhite);

		// Blinking caret after the typed text
		if (((int)(mTime * 2.0)) % 2 == 0)
		{
			let width = font.Measure(scope $"> {mTyped}").mX;
			g.FillRect(x + width + 2, y + 2, 2, font.mLineHeight - 4, Colors.sWhite);
		}
	}

	static int Main(String[] args)
	{
		Program app = scope .();
		return app.Run(args);
	}
}
