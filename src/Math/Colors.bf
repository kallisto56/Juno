namespace Juno;

using System;

// NOTE: `Color` (Math/Color.bf) must live in `namespace Juno` for this file to resolve it.
// Named colors plus helpers. Always goes through the explicit uint8 constructor of Color,
// so it does not depend on Color's internal byte order.
static class Colors
{
	static public Color sWhite = Make(255, 255, 255, 255);
	static public Color sBlack = Make(0, 0, 0, 255);
	static public Color sTransparent = Make(0, 0, 0, 0);
	static public Color sRed = Make(255, 64, 64, 255);
	static public Color sGreen = Make(64, 220, 96, 255);
	static public Color sBlue = Make(64, 128, 255, 255);
	static public Color sYellow = Make(255, 220, 64, 255);
	static public Color sCyan = Make(64, 220, 255, 255);
	static public Color sMagenta = Make(255, 96, 220, 255);
	static public Color sOrange = Make(255, 150, 48, 255);
	static public Color sGray = Make(128, 128, 128, 255);
	static public Color sDarkGray = Make(40, 42, 50, 255);

	static public Color Make(uint8 r, uint8 g, uint8 b, uint8 a)
	{
		return Color(r, g, b, a);
	}

	static public Color WithAlpha(Color c, uint8 a)
	{
		return Color(c.mR, c.mG, c.mB, a);
	}

	// Component-wise multiply (like tinting).
	static public Color Multiply(Color a, Color b)
	{
		return Color(
			(uint8)(((int32)a.mR * (int32)b.mR) / 255),
			(uint8)(((int32)a.mG * (int32)b.mG) / 255),
			(uint8)(((int32)a.mB * (int32)b.mB) / 255),
			(uint8)(((int32)a.mA * (int32)b.mA) / 255));
	}

	// Packs to bytes R, G, B, A in memory order (little-endian uint32 = A<<24 | B<<16 | G<<8 | R),
	// which is what DXGI's R8G8B8A8_UNorm expects.
	static public uint32 PackRGBA(Color c)
	{
		return (uint32)c.mR | ((uint32)c.mG << 8) | ((uint32)c.mB << 16) | ((uint32)c.mA << 24);
	}
}
