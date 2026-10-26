namespace Juno;

using System;
using System.Interop;

// Opaque handle. Only ever used through pointers (GpuBuffer*).
[CRepr]
struct GpuBuffer
{
}
