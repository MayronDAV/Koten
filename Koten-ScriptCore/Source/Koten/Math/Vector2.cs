namespace KTN
{
	public struct Vector2
	{
		public float X, Y;

		public static Vector2 Zero => new Vector2(0.0f);

		public Vector2(float p_Scalar)
		{
			X = p_Scalar;
			Y = p_Scalar;
		}

		public Vector2(float x, float y)
		{
			X = x;
			Y = y;
		}

		public float Length()
		{
			return (float)System.Math.Sqrt(X * X + Y * Y);
        }

        public Vector2 Normalized()
		{
			float length = Length();
			if (length == 0.0f)
				return Zero;

			return new Vector2(X / length, Y / length);
        }

        public static Vector2 operator +(Vector2 a, Vector2 b)
		{
			return new Vector2(a.X + b.X, a.Y + b.Y);
		}

		public static Vector2 operator *(Vector2 p_Vector, float p_Scalar)
		{
			return new Vector2(p_Vector.X * p_Scalar, p_Vector.Y * p_Scalar);
		}

        public static Vector2 operator *(Vector2 p_A, Vector2 p_B)
        {
            return new Vector2(p_A.X * p_B.X, p_A.Y * p_B.Y);
        }

    }
} // namespace KTN