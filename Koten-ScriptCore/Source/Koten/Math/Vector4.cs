namespace KTN
{
    public struct Vector4
    {
        public float X, Y, Z, W;

        public static Vector4 Zero => new Vector4(0.0f);

        public Vector4(float p_Scalar)
        {
            X = p_Scalar;
            Y = p_Scalar;
            Z = p_Scalar;
            W = p_Scalar;
        }

        public Vector4(float p_X, float p_Y, float p_Z, float p_W)
        {
            X = p_X;
            Y = p_Y;
            Z = p_Z;
            W = p_W;
        }

        public Vector4(Vector3 p_Vector, float p_W)
        {
            X = p_Vector.X;
            Y = p_Vector.Y;
            Z = p_Vector.Z;
            W = p_W;
        }

        public Vector4(Vector2 p_Vec1, Vector2 p_Vec2)
        {
            X = p_Vec1.X;
            Y = p_Vec1.Y;
            Z = p_Vec2.X;
            W = p_Vec2.Y;
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

        public Vector3 XYZ
        {
            get => new Vector3(X, Y, Z);
            set
            {
                X = value.X;
                Y = value.Y;
                Z = value.Z;
            }
        }

        public float Length()
        {
            return (float)System.Math.Sqrt(X * X + Y * Y + Z * Z + W * W);
        }

        public Vector4 Normalized()
        {
            float length = Length();
            if (length == 0.0f)
                return Zero;

            return new Vector4(X / length, Y / length, Z / length, W / length);
        }

        public static Vector4 operator +(Vector4 p_A, Vector4 p_B)
        {
            return new Vector4(p_A.X + p_B.X, p_A.Y + p_B.Y, p_A.Z + p_B.Z, p_A.W + p_B.W);
        }

        public static Vector4 operator *(Vector4 p_Vector, float p_Scalar)
        {
            return new Vector4(p_Vector.X * p_Scalar, p_Vector.Y * p_Scalar, p_Vector.Z * p_Scalar, p_Vector.W * p_Scalar);
        }

        public static Vector4 operator *(Vector4 p_A, Vector4 p_B)
        {
            return new Vector4(p_A.X * p_B.X, p_A.Y * p_B.Y, p_A.Z * p_B.Z, p_A.W * p_B.W);
        }
    }
} // namespace KTN