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

public class EditorItem : Attribute
{
    public string editorName;

    public EditorItem(string editorName)
    {
        this.editorName = editorName;
    }
}

[Serializable]
[MenuItem("Window/Tools")]
[EditorItem("Editor")]
public class TestScript : BaseScript
{
    public int iteration = 0;
    public float speed = 1.0f;
    static int staticField = 12;
    private string testData = "testData";
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