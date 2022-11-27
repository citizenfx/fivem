using System;
using System.Drawing;
using CitizenFX.Core.Native;

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
				if (argument is int argI)
				{
					API.ScaleformMovieMethodAddParamInt(argI);
				}
				else if (argument is string argS)
				{
					API.ScaleformMovieMethodAddParamTextureNameString(argS);
				}
				else if (argument is char argC)
				{
					API.ScaleformMovieMethodAddParamTextureNameString(argC.ToString());
				}
				else if (argument is float argF)
				{
					API.ScaleformMovieMethodAddParamFloat((float)argF);
				}
				else if (argument is double argD)
				{
					API.ScaleformMovieMethodAddParamFloat((float)argD);
				}
				else if (argument is bool argB)
				{
					API.ScaleformMovieMethodAddParamBool(argB);
				}
				else if (argument is ScaleformArgumentTXD argTXD)
				{
					API.ScaleformMovieMethodAddParamTextureNameString(argTXD._txd);
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
			API.DrawScaleformMovie_3dSolid(Handle, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, 2.0f, 2.0f, 1.0f, scale.X, scale.Y, scale.Z, 2);
		}
		public void Render3DAdditive(Vector3 position, Vector3 rotation, Vector3 scale)
		{
			API.DrawScaleformMovie_3d(Handle, position.X, position.Y, position.Z, rotation.X, rotation.Y, rotation.Z, 2.0f, 2.0f, 1.0f, scale.X, scale.Y, scale.Z, 2);
		}
	}
}
