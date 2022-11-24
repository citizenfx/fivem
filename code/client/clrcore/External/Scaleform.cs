using System;

#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;
using INativeValue = CitizenFX.Core.Native.Input.Primitive;
using PointF = CitizenFX.Core.Vector2;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;
using System.Drawing;

namespace CitizenFX.Core
#endif
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
			_handle = API.RequestScaleformMovie(scaleformID);
		}

		~Scaleform()
		{
			Dispose();
		}

		public void Dispose()
		{
			if (IsLoaded)
			{
				API.SetScaleformMovieAsNoLongerNeeded(ref _handle);
			}

			GC.SuppressFinalize(this);
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
				return API.HasScaleformMovieLoaded(Handle);
			}
		}

		public void CallFunction(string function, params object[] arguments)
		{
			API.BeginScaleformMovieMethod(Handle, function);
			foreach (var argument in arguments)
			{
				if (argument is int)
				{
					API.PushScaleformMovieMethodParameterInt((int)argument);
				}
				else if (argument is string)
				{
					API.PushScaleformMovieMethodParameterString((string)argument);
				}
				else if (argument is char)
				{
					API.PushScaleformMovieMethodParameterString(argument.ToString());
				}
				else if (argument is float)
				{
					API.PushScaleformMovieMethodParameterFloat((float)argument);
				}
				else if (argument is double)
				{
					API.PushScaleformMovieMethodParameterFloat((float)(double)argument);
				}
				else if (argument is bool)
				{
					API.PushScaleformMovieMethodParameterBool((bool)argument);
				}
				else if (argument is ScaleformArgumentTXD)
				{
					API.PushScaleformMovieMethodParameterString(((ScaleformArgumentTXD)argument)._txd);
				}
				else
				{
					throw new ArgumentException(string.Format("Unknown argument type {0} passed to scaleform with handle {1}.", argument.GetType().Name, Handle), "arguments");
				}
			}
			API.EndScaleformMovieMethod();
		}

		public void Render2D()
		{
			API.DrawScaleformMovieFullscreen(Handle, 255, 255, 255, 255, 0);
		}
		public void Render2DScreenSpace(PointF location, PointF size)
		{
			float x = location.X / UI.Screen.Width;
			float y = location.Y / UI.Screen.Height;
			float width = size.X / UI.Screen.Width;
			float height = size.Y / UI.Screen.Height;

			API.DrawScaleformMovie(Handle, x + (width / 2.0f), y + (height / 2.0f), width, height, 255, 255, 255, 255, 0);
		}
		public void Render3D(Vector3 position, Vector3 rotation, Vector3 scale)
		{
			API.DrawScaleformMovie_3dNonAdditive(Handle, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, 2.0f, 2.0f, 1.0f, scale.X, scale.Y, scale.Z, 2);
		}
		public void Render3DAdditive(Vector3 position, Vector3 rotation, Vector3 scale)
		{
			API.DrawScaleformMovie_3d(Handle, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, 2.0f, 2.0f, 1.0f, scale.X, scale.Y, scale.Z, 2);
		}
	}
}
