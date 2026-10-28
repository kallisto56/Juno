namespace Juno;

using System;
using System.Interop;

// Opaque handle. Only ever used through pointers (Fence*).
[CRepr]
struct Fence
{
}
