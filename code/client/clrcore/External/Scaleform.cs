using System;
using System.Drawing;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core
{
	public sealed class ScaleformArgumentTXD
	{
		#region Fields
		internal string _txd;
		#endregion

		public ScaleformArgumentTXD(string s)
		{
			_txd = s;
		}
	}

	public sealed class Scaleform : INativeValue, IDisposable
    {
		public Scaleform(string scaleformID)
		{
			_handle = Function.Call<int>(Hash.REQUEST_SCALEFORM_MOVIE, scaleformID);
		}

		~Scaleform()
		{
			Dispose();
		}

        [SecuritySafeCritical]
		public void Dispose()
		{
			if (IsLoaded)
			{
				DeleteThat();
			}

			GC.SuppressFinalize(this);
		}

		[SecurityCritical]
		private void DeleteThat()
		{
			unsafe
			{
				fixed (int* handlePtr = &_handle)
				{
					Function.Call(Hash.SET_SCALEFORM_MOVIE_AS_NO_LONGER_NEEDED, handlePtr);
				}
			}
		}

		public int Handle
		{
			get { return _handle; }
		}

		private int _handle;

		public override ulong NativeValue
		{
			get
			{
				return (ulong)Handle;
			}
			set
			{
				_handle = unchecked((int)value);
			}
		}

		public bool IsValid
		{
			get
			{
				return Handle != 0;
			}
		}
		public bool IsLoaded
		{
			get
			{
				return Function.Call<bool>(Hash.HAS_SCALEFORM_MOVIE_LOADED, Handle);
			}
		}

		[SecuritySafeCritical]
		public void CallFunction(string function, params object[] arguments)
		{
			Function.Call(Hash._PUSH_SCALEFORM_MOVIE_FUNCTION, Handle, function);

			foreach (var argument in arguments)
			{
				if (argument is int)
				{
					Function.Call(Hash._PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_INT, (int)argument);
				}
				else if (argument is string)
				{
					Function.Call(Hash._BEGIN_TEXT_COMPONENT, MemoryAccess.StringPtr);
					Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, (string)argument);
					Function.Call(Hash._END_TEXT_COMPONENT);
				}
				else if (argument is char)
				{
					Function.Call(Hash._BEGIN_TEXT_COMPONENT, MemoryAccess.StringPtr);
					Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, argument.ToString());
					Function.Call(Hash._END_TEXT_COMPONENT);
				}
				else if (argument is float)
				{
					Function.Call(Hash._PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_FLOAT, (float)argument);
				}
				else if (argument is double)
				{
					Function.Call(Hash._PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_FLOAT, (float)(double)argument);
				}
				else if (argument is bool)
				{
					Function.Call(Hash._PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_BOOL, (bool)argument);
				}
				else if (argument is ScaleformArgumentTXD)
				{
					Function.Call(Hash._PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_STRING, ((ScaleformArgumentTXD)argument)._txd);
				}
				else
				{
					throw new ArgumentException(string.Format("Unknown argument type {0} passed to scaleform with handle {1}.", argument.GetType().Name, Handle), "arguments");
				}
			}

			Function.Call(Hash._POP_SCALEFORM_MOVIE_FUNCTION_VOID);
		}

		public void Render2D()
		{
			Function.Call(Hash.DRAW_SCALEFORM_MOVIE_FULLSCREEN, Handle, 255, 255, 255, 255, 0);
		}
		public void Render2DScreenSpace(PointF location, PointF size)
		{
			float x = location.X / UI.Screen.Width;
			float y = location.Y / UI.Screen.Height;
			float width = size.X / UI.Screen.Width;
			float height = size.Y / UI.Screen.Height;

			Function.Call(Hash.DRAW_SCALEFORM_MOVIE, Handle, x + (width / 2.0f), y + (height / 2.0f), width, height, 255, 255, 255, 255);
		}
		public void Render3D(Vector3 position, Vector3 rotation, Vector3 scale)
		{
			Function.Call(Hash._DRAW_SCALEFORM_MOVIE_3D_NON_ADDITIVE, Handle, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, 2.0f, 2.0f, 1.0f, scale.X, scale.Y, scale.Z, 2);
		}
		public void Render3DAdditive(Vector3 position, Vector3 rotation, Vector3 scale)
		{
			Function.Call(Hash.DRAW_SCALEFORM_MOVIE_3D, Handle, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, 2.0f, 2.0f, 1.0f, scale.X, scale.Y, scale.Z, 2);
		}
	}
}
