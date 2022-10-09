using Simengine;
using System;

public class MenuItem : Attribute
{
    public string Name;
    public MenuItem(string name)
    {
        Name = name;
    }
}

[MenuItem("Window/Tools")]
public class TestScript : BaseScript
{
    public void OnUpdate()
    {
        Debug.Log(21.0f);
        Console.WriteLine("On Update - test");
        _ = new Config
        {
            modelName = "C:\\Users\\hankarun\\Desktop\\monoEmbeddedPlayground\\engine\\models\\bunny.obj",
            sampleCount = 50
        };
    }
}