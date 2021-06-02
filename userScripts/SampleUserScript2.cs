using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Simengine;

public class SampleUserScript2 : MonoSystem
{
	public int counter = 5;
	public string componentName = "transform2";
	public int v = 7;
	
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
	
	public void Update()
	{
		float  dt = Time.Deltatime();
		for (int i = 0; i < v; i++)
		{
			Debug.Log(componentName);
		}
	}
}