namespace Juno;

using System;
using System.Interop;

// Opaque handle. Only ever used through pointers (Shader*).
[CRepr]
struct Shader
{
}
