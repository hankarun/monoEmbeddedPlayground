using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Simengine
{
	public class Time
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern float Deltatime();
	}
}