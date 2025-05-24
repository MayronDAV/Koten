using System;

namespace KTN
{
	public class Main
	{
		public float FloatVar { get; set; }

		public Main()
		{
			Console.WriteLine("Main constructor called");
		}

		public void PrintMessage()
		{
			Console.WriteLine("Hello World from C#!");
		}

		public void PrintIntMessage(int p_Value)
		{
			Console.WriteLine($"C# Says: {p_Value}");
		}

		public void PrintCustomMessage(string p_Message)
		{
			Console.WriteLine($"C# Says: {p_Message}");
		}
	}

} // namespace KTN
