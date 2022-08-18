using Simengine;
using System;

public class TestScript : BaseScript
{
    public void OnUpdate()
    {
        Debug.Log(21.0f);
        Console.WriteLine("test");
        _ = new Config
        {
            modelName = "C:\\Users\\hankarun\\Desktop\\monoEmbeddedPlayground\\engine\\models\\bunny.obj",
            sampleCount = 50
        };
    }
}