using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Simengine
{
    public class Component
    {
        Entity entity;
    };

  	public class Debug
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern void Log(float value);
    }

    public class Config
    {
        public string modelName
        {
            get { return modelName; }
            set
            {
                SetModelName(value);

            }
        }

        public int sampleCount 
        { 
            get { return sampleCount; }
            set 
            {
                SetSampleCount(value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern void SetModelName(string value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        static extern void SetSampleCount(int value);
    }
}
