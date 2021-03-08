using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Simengine
{
	public class Debug
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void PrintInfo();

		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Log(float value);
	}
}