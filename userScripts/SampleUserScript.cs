using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Simengine;

public class SampleUserScript : MonoSystem
{
	public int counter = 5;
	public string componentName = "transform2";
	public int v = 7;
	public bool enterLoop = true;
	public double angle = 0.231; 
	
	private void PrintAppData()
	{
		Debug.PrintInfo();
	}
	
	public void Start()
	{
		for(int i = 0; i < counter; i++)
		{
			PrintAppData();
		}
	}
	
	public void PrintComponentName()
	{
		Vector3 position = getTransform().getPosition();
		if (enterLoop)
		{
			for (int i = 0; i < v; i++)
			{
				Debug.Log(componentName);
			}
		}else
			Debug.Log(componentName);
	}
}