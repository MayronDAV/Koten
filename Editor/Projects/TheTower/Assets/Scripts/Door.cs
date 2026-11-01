using KTN;
using System;


namespace TW
{
    public class Door : ScriptBehavior
    {
        void OnCreate()
        {
        }

        void OnTriggerEnter(ulong p_EntityID)
        {
            // Console.WriteLine($"Door Triggered by Entity ID: {p_EntityID}");

            var gameObject = FindWithUUID(p_EntityID);
            if (gameObject == null || !gameObject.IsValid())
            {
                Console.WriteLine("Failed to find game object!");
                return;
            }

            var tc = gameObject.GetComponent<TagComponent>();
            var tag = tc.Tag;

            if (tag == "Player")
            {
                Console.WriteLine($"Door Triggered by Player: {p_EntityID}");
            }
        }

        void OnUpdate()
        {
        }
    }


} // namespace TW