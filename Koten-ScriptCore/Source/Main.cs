using System;
using System.Runtime.CompilerServices;

namespace KTN
{
	public struct Vector3
	{
		public float X, Y, Z;

		public Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}
	}

	public static class InternalCalls
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog(string p_Text, int p_Parameter);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog_Vector(ref Vector3 p_Parameter, out Vector3 p_Result);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static float NativeLog_VectorDot(ref Vector3 p_Parameter);
	}

	public class Entity
	{
		public float FloatVar { get; set; }

		public Entity()
		{
			Console.WriteLine("Main constructor!");
			Log("AAstroPhysiC", 8058);

			Vector3 pos = new Vector3(5, 2.5f, 1);
			Vector3 result = Log(pos);
			Console.WriteLine($"{result.X}, {result.Y}, {result.Z}");
			Console.WriteLine("{0}", InternalCalls.NativeLog_VectorDot(ref pos));
		}

		public void PrintMessage()
		{
			Console.WriteLine("Hello World from C#!");
		}

		public void PrintInt(int p_Value)
		{
			Console.WriteLine($"C# says: {p_Value}");
		}

		public void PrintInts(int p_Value1, int p_Value2)
		{
			Console.WriteLine($"C# says: {p_Value1} and {p_Value2}");
		}

		public void PrintCustomMessage(string p_Message)
		{
			Console.WriteLine($"C# says: {p_Message}");
		}

		private void Log(string p_Text, int p_Parameter)
		{
			InternalCalls.NativeLog(p_Text, p_Parameter);

		}
		private Vector3 Log(Vector3 p_Parameter)
		{
			InternalCalls.NativeLog_Vector(ref p_Parameter, out Vector3 p_Result);
			return p_Result;
		}

	}
} // namespace KTN