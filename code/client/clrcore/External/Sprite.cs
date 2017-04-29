using System;
using System.IO;
using System.Drawing;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using CitizenFX.Core.Native;
using System.Security;

namespace CitizenFX.Core.UI
{
	public interface ISprite : IElement
	{
		/// <summary>
		/// Gets or sets the size to draw the <see cref="ISprite"/>
		/// </summary>
		/// <value>
		/// The size on a 1280*720 pixel base
		/// </value>
		/// <remarks>
		/// If ScaledDraw is called, the size will be scaled by the width returned in <see cref="Screen.ScaledWidth"/>.
		/// </remarks>					 
		SizeF Size { get; set; }
		/// <summary>
		/// Gets or sets the rotation to draw thie <see cref="ISprite"/>.
		/// </summary>
		/// <value>
		/// The rotation measured in degrees, clockwise increasing, 0.0 at vertical
		/// </value>
		float Rotation { get; set; }
	}

	public class Sprite : ISprite, IDisposable
	{
		#region Fields
		private readonly string _textureDict, _textureName;
		private static readonly Dictionary<string, int> _activeTextures = new Dictionary<string, int>();
		private readonly IntPtr _pinnedDict, _pinnedName;
		#endregion

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Sprite" /> will be drawn.
		/// </summary>
		/// <value>
		///   <c>true</c> if enabled; otherwise, <c>false</c>.
		/// </value>
		public bool Enabled { get; set; }
		/// <summary>
		/// Gets or sets the color of this <see cref="Sprite" />.
		/// </summary>
		/// <value>
		/// The color.
		/// </value>
		public Color Color { get; set; }
		/// <summary>
		/// Gets or sets the position of this <see cref="Sprite" />.
		/// </summary>
		/// <value>
		/// The position scaled on a 1280*720 pixel base.
		/// </value>
		/// <remarks>
		/// If ScaledDraw is called, the position will be scaled by the width returned in <see cref="Screen.ScaledWidth" />.
		/// </remarks>
		public PointF Position { get; set; }
		/// <summary>
		/// Gets or sets the size to draw the <see cref="Sprite" />
		/// </summary>
		/// <value>
		/// The size on a 1280*720 pixel base
		/// </value>
		/// <remarks>
		/// If ScaledDraw is called, the size will be scaled by the width returned in <see cref="Screen.ScaledWidth" />.
		/// </remarks>
		public SizeF Size { get; set; }
		/// <summary>
		/// Gets or sets the rotation to draw thie <see cref="Sprite" />.
		/// </summary>
		/// <value>
		/// The rotation measured in degrees, clockwise increasing, 0.0 at vertical
		/// </value>
		public float Rotation { get; set; }
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Sprite"/> should be positioned based on its center or top left corner
		/// </summary>
		/// <value>
		///   <c>true</c> if centered; otherwise, <c>false</c>.
		/// </value>
		public bool Centered { get; set; }

		/// <summary>
		/// Initializes a new instance of the <see cref="Sprite"/> class used for drawing in game textures on the screen.
		/// </summary>
		/// <param name="textureDict">The Texture dictionary where the <see cref="Sprite"/> is stored (the *.ytd file).</param>
		/// <param name="textureName">Name of the <see cref="Sprite"/> inside the Texture dictionary.</param>
		/// <param name="size">Set the <see cref="Size"/> of the <see cref="Sprite"/>.</param>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Sprite"/>.</param>											   
		public Sprite(string textureDict, string textureName, SizeF size, PointF position) : this(textureDict, textureName, size, position, Color.FromArgb(255, 255, 255, 255), 0f, false)
		{
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Sprite"/> class used for drawing in game textures on the screen.
		/// </summary>
		/// <param name="textureDict">The Texture dictionary where the <see cref="Sprite"/> is stored (the *.ytd file).</param>
		/// <param name="textureName">Name of the <see cref="Sprite"/> inside the Texture dictionary.</param>
		/// <param name="size">Set the <see cref="Size"/> of the <see cref="Sprite"/>.</param>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Sprite"/>.</param>
		/// <param name="color">Set the <see cref="Color"/> used to draw the <see cref="Sprite"/>.</param>														 
		public Sprite(string textureDict, string textureName, SizeF size, PointF position, Color color) : this(textureDict, textureName, size, position, color, 0f, false)
		{
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Sprite"/> class used for drawing in game textures on the screen.
		/// </summary>
		/// <param name="textureDict">The Texture dictionary where the <see cref="Sprite"/> is stored (the *.ytd file).</param>
		/// <param name="textureName">Name of the <see cref="Sprite"/> inside the Texture dictionary.</param>
		/// <param name="size">Set the <see cref="Size"/> of the <see cref="Sprite"/>.</param>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Sprite"/>.</param>
		/// <param name="color">Set the <see cref="Color"/> used to draw the <see cref="Sprite"/>.</param>
		/// <param name="rotation">Set the rotation to draw the sprite, measured in degrees, see also <seealso cref="Rotation"/>.</param>						
		public Sprite(string textureDict, string textureName, SizeF size, PointF position, Color color, float rotation) : this(textureDict, textureName, size, position, color, rotation, false)
		{
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Sprite"/> class used for drawing in game textures on the screen.
		/// </summary>
		/// <param name="textureDict">The Texture dictionary where the <see cref="Sprite"/> is stored (the *.ytd file).</param>
		/// <param name="textureName">Name of the <see cref="Sprite"/> inside the Texture dictionary.</param>
		/// <param name="size">Set the <see cref="Size"/> of the <see cref="Sprite"/>.</param>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Sprite"/>.</param>
		/// <param name="color">Set the <see cref="Color"/> used to draw the <see cref="Sprite"/>.</param>
		/// <param name="rotation">Set the rotation to draw the sprite, measured in degrees, see also <seealso cref="Rotation"/>.</param>
		/// <param name="centered">Position the <see cref="Sprite"/> based on its center instead of top left corner, see also <seealso cref="Centered"/>.</param>
		[SecuritySafeCritical]
		public Sprite(string textureDict, string textureName, SizeF size, PointF position, Color color, float rotation, bool centered)
		{
			byte[] data = Encoding.UTF8.GetBytes(textureDict + "\0");
			_pinnedDict = Marshal.AllocCoTaskMem(data.Length);
			Marshal.Copy(data, 0, _pinnedDict, data.Length);
			data = Encoding.UTF8.GetBytes(textureName + "\0");
			_pinnedName = Marshal.AllocCoTaskMem(data.Length);
			Marshal.Copy(data, 0, _pinnedName, data.Length);

			_textureDict = textureDict;
			_textureName = textureName;

			Enabled = true;
			Size = size;
			Position = position;
			Color = color;
			Rotation = rotation;
			Centered = centered;

			Function.Call(Hash.REQUEST_STREAMED_TEXTURE_DICT, _pinnedDict);

			if (_activeTextures.ContainsKey(textureDict.ToLower()))
			{
				_activeTextures[textureDict.ToLower()] += 1;
			}
			else
			{
				_activeTextures.Add(textureDict.ToLower(), 1);
			}
		}

		public void Dispose()
		{
			Dispose(true);
			GC.SuppressFinalize(this);
		}
		[SecuritySafeCritical]
		protected virtual void Dispose(bool disposing)
		{
			if (disposing)
			{
				if (_activeTextures.ContainsKey(_textureDict.ToLower()))
				{
					int current = _activeTextures[_textureDict.ToLower()];
					if (current == 1)
					{
						Function.Call(Hash.SET_STREAMED_TEXTURE_DICT_AS_NO_LONGER_NEEDED, _pinnedDict);
						_activeTextures.Remove(_textureDict.ToLower());
					}
					else
					{
						_activeTextures[_textureDict.ToLower()] = current - 1;
					}
				}
				else
				{
					//In practice this should never get executed
					Function.Call(Hash.SET_STREAMED_TEXTURE_DICT_AS_NO_LONGER_NEEDED, _pinnedDict);
				}
				Marshal.FreeCoTaskMem(_pinnedDict);
				Marshal.FreeCoTaskMem(_pinnedName);
			}
		}

		/// <summary>
		/// Draws this <see cref="Sprite" />.
		/// </summary>
		public virtual void Draw()
		{
			Draw(SizeF.Empty);
		}
		/// <summary>
		/// Draws the <see cref="Sprite" /> at the specified offset.
		/// </summary>
		/// <param name="offset">The offset.</param>
		public virtual void Draw(SizeF offset)
		{
			InternalDraw(offset, Screen.Width, Screen.Height);
		}
		/// <summary>
		/// Draws this <see cref="Sprite" /> using the width returned in <see cref="Screen.ScaledWidth" />.
		/// </summary>
		public virtual void ScaledDraw()
		{
			ScaledDraw(SizeF.Empty);
		}
		/// <summary>
		/// Draws the <see cref="Sprite" /> at the specified offset using the width returned in <see cref="Screen.ScaledWidth" />.
		/// </summary>
		/// <param name="offset">The offset.</param>
		public virtual void ScaledDraw(SizeF offset)
		{
			InternalDraw(offset, Screen.ScaledWidth, Screen.Height);
		}

		void InternalDraw(SizeF offset, float screenWidth, float screenHeight)
		{
			if (!Enabled || !Function.Call<bool>(Hash.HAS_STREAMED_TEXTURE_DICT_LOADED, _textureDict))
			{
				return;
			}

			float scaleX = Size.Width / screenWidth;
			float scaleY = Size.Height / screenHeight;
			float positionX = (Position.X + offset.Width) / screenWidth;
			float positionY = (Position.Y + offset.Height) / screenHeight;

			if (!Centered)
			{
				positionX += scaleX * 0.5f;
				positionY += scaleY * 0.5f;
			}

			Function.Call(Hash.DRAW_SPRITE, _pinnedDict, _pinnedName, positionX, positionY, scaleX, scaleY, Rotation, Color.R, Color.G, Color.B, Color.A);
		}
	}
}
