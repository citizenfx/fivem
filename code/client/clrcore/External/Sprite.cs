using System;
using System.IO;
using System.Drawing;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using CitizenFX.Core.Native;
using System.Security;

#if MONO_V2
using CitizenFX.Core;
using PointF = CitizenFX.Core.Vector2;
using SizeF = CitizenFX.Core.Vector2;
using API = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM.GUI
#else
namespace CitizenFX.Core.UI
#endif
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
#if MONO_V2
		private readonly CString _textureDict, _textureName;
		private static readonly Dictionary<CString, uint> _activeTextures = new Dictionary<CString, uint>();
#else
		private readonly string _textureDict, _textureName;
		private static readonly Dictionary<string, int> _activeTextures = new Dictionary<string, int>();
#endif
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
		public Sprite(string textureDict, string textureName, SizeF size, PointF position, Color color, float rotation, bool centered)
		{
			Enabled = true;
			Size = size;
			Position = position;
			Color = color;
			Rotation = rotation;
			Centered = centered;

			_textureDict = textureDict.ToLower();
			_textureName = textureName.ToLower();

			if (!API.HasStreamedTextureDictLoaded(_textureDict))
			{
				API.RequestStreamedTextureDict(_textureDict, false);
			}

			_activeTextures[_textureDict]++;
		}

		public void Dispose()
		{
			// TODO: give this some attention; not using it in this runtime, doesn't mean we're not using it in others
			if (_activeTextures.TryGetValue(_textureDict, out var current))
			{
				if (current == 1)
				{
					API.SetStreamedTextureDictAsNoLongerNeeded(_textureDict);
					_activeTextures.Remove(_textureDict);
				}
				else
				{
					_activeTextures[_textureDict] = current - 1;
				}
			}
			else
			{
				API.SetStreamedTextureDictAsNoLongerNeeded(_textureDict);
			}
			GC.SuppressFinalize(this);
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
			InternalDraw(offset, 1.0f / Screen.Width, 1.0f / Screen.Height);
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
			InternalDraw(offset, 1.0f / Screen.ScaledWidth, 1.0f / Screen.Height);
		}

		void InternalDraw(SizeF offset, float inverseScreenWidth, float inverseScreenHeight)
		{
			if (!Enabled || !API.HasStreamedTextureDictLoaded(_textureDict))
			{
				return;
			}

#if MONO_V2
			float scaleX = Size.X * inverseScreenWidth;
			float scaleY = Size.Y * inverseScreenHeight;
			float positionX = (Position.X + offset.X) * inverseScreenWidth;
			float positionY = (Position.Y + offset.Y) * inverseScreenHeight;
#else
			float scaleX = Size.Width * inverseScreenWidth;
			float scaleY = Size.Height * inverseScreenHeight;
			float positionX = (Position.X + offset.Width) * inverseScreenWidth;
			float positionY = (Position.Y + offset.Height) * inverseScreenHeight;
#endif

			if (!Centered)
			{
				positionX += scaleX * 0.5f;
				positionY += scaleY * 0.5f;
			}

			API.DrawSprite(_textureDict, _textureName, positionX, positionY, scaleX, scaleY, Rotation, Color.R, Color.G, Color.B, Color.A);
		}
	}
}
