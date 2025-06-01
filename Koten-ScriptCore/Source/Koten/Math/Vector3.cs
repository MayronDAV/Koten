namespace KTN
{
	public struct Vector3
	{
		public float X, Y, Z;

		public static Vector3 Zero => new Vector3(0.0f);

		public Vector3(Vector2 p_XY, float p_Z)
		{
			X = p_XY.X;
			Y = p_XY.Y;
			Z = p_Z;
		}

		public Vector3(float p_Scalar)
		{
			X = p_Scalar;
			Y = p_Scalar;
			Z = p_Scalar;
		}

		public Vector3(float p_X, float p_Y, float p_Z)
		{
			X = p_X;
			Y = p_Y;
			Z = p_Z;
		}

		public Vector2 XY
		{
			get => new Vector2(X, Y);
			set
			{
				X = value.X;
				Y = value.Y;
			}
		}

		public static Vector3 operator +(Vector3 p_A, Vector3 p_B)
		{
			return new Vector3(p_A.X + p_B.X, p_A.Y + p_B.Y, p_A.Z + p_B.Z);
		}

		public static Vector3 operator *(Vector3 p_Vector, float p_Scalar)
		{
			return new Vector3(p_Vector.X * p_Scalar, p_Vector.Y * p_Scalar, p_Vector.Z * p_Scalar);
		}
	}
} // namespace KTN