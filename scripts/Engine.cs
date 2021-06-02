using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Simengine
{
	public struct Vector3
	{
		double x;
		double y;
		double z;
	}
	
	public class Transform
	{
		private int entityid;
		
		public Transform(int entityid)
		{
		}
		
		public void setPosition(double x, double y, double z)
		{
			
		}
		public Vector3 getPosition()
		{
			return new Vector3();
		}
		
		public void setRotation(double x, double y, double z)
		{
		}
		
		public Vector3 getRotation()
		{
			return new Vector3();
		}
		
		public void setScale(double x, double y, double z)
		{
		}
		
		public Vector3 getScale()
		{
			return new Vector3();
		}
		
		public void setParent(int parentid, bool worldPositionStays = true)
		{
		}		
		
		static public Vector3 calculateLocalRotationFromLLA(double x, double y, Vector3 rotation)
		{
			return new Vector3();
		}
		
		static public Vector3 calculateLLARotationFromXYA(Vector3 position)
		{
			return new Vector3();
		}
		
		static public Vector3 calculateLLAPosition(double x, double y, double alt)
		{
			return new Vector3();
		}
		
		static public Vector3 calculateLLARotation(double x, double y, double yaw, double pitch, double roll)
		{
			return new Vector3();
		}
	}
	
    public class MonoSystem
    {
		private int entityId;
		
		public Transform getTransform()
		{		
			return new Transform(entityId);
		}
    }
}
