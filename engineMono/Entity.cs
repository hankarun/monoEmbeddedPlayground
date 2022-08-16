
namespace Simengine
{
    public class Entity
    {
        public int value;

        public void addComponent<T>()
        {

        }

        public T getComponent<T>()  where T : Component, new()
        {
            return new T();
        }

        public void removeComponent<T>()
        {

        }

        public bool hasComponent<T>()
        {
            return false;
        }
    };
}