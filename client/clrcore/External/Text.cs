using System;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;
using CitizenFX.Core.Native;

using System.Security;

namespace CitizenFX.Core.UI
{
	public enum Alignment
	{
		Center = 0,
		Left = 1,
		Right = 2,
	}

	public class Text : IElement
	{
		private string _caption;
		private readonly List<IntPtr> _pinnedText;

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Text" /> will be drawn.
		/// </summary>
		/// <value>
		///   <c>true</c> if enabled; otherwise, <c>false</c>.
		/// </value>
		public bool Enabled { get; set; }
		/// <summary>
		/// Gets or sets the color of this <see cref="Text" />.
		/// </summary>
		/// <value>
		/// The color.
		/// </value>
		public Color Color { get; set; }
		/// <summary>
		/// Gets or sets the position of this <see cref="Text" />.
		/// </summary>
		/// <value>
		/// The position scaled on a 1280*720 pixel base.
		/// </value>
		/// <remarks>
		/// If ScaledDraw is called, the position will be scaled by the width returned in <see cref="Screen.ScaledWidth" />.
		/// </remarks>
		public PointF Position { get; set; }
		/// <summary>
		/// Gets or sets the scale of this <see cref="Text"/>.
		/// </summary>
		/// <value>
		/// The scale usually a value between ~0.5 and 3.0, Default = 1.0
		/// </value>
		public float Scale { get; set; }
		/// <summary>
		/// Gets or sets the font of this <see cref="Text"/>.
		/// </summary>
		/// <value>
		/// The GTA Font use when drawing.
		/// </value>
		public Font Font { get; set; }
		/// <summary>
		/// Gets or sets the text to draw in this <see cref="Text"/>.
		/// </summary>
		/// <value>
		/// The caption.
		/// </value>
		public string Caption
		{
			get { return _caption; }
			[SecuritySafeCritical]
			set
			{
				_caption = value;
				foreach (var ptr in _pinnedText)
				{
					Marshal.FreeCoTaskMem(ptr); //free any existing allocated text
				}
				_pinnedText.Clear();

				const int maxStringLength = 99;

				for (int i = 0; i < Caption.Length; i += maxStringLength)
				{
					byte[] data =
						Encoding.UTF8.GetBytes(Caption.Substring(i, System.Math.Min(maxStringLength, Caption.Length - i)) + "\0");
					IntPtr next = Marshal.AllocCoTaskMem(data.Length);
					Marshal.Copy(data, 0, next, data.Length);
					_pinnedText.Add(next);
				}
			}
		}

		/// <summary>
		/// Gets or sets the alignment of this <see cref="Text"/>.
		/// </summary>
		/// <value>
		/// The alignment:<c>Left</c>, <c>Center</c>, <c>Right</c> Justify
		/// </value>
		public Alignment Alignment { get; set; }
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Text"/> is drawn with a shadow effect.
		/// </summary>
		/// <value>
		///   <c>true</c> if shadow; otherwise, <c>false</c>.
		/// </value>
		public bool Shadow { get; set; }
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Text"/> is drawn with an outline.
		/// </summary>
		/// <value>
		///   <c>true</c> if outline; otherwise, <c>false</c>.
		/// </value>
		public bool Outline { get; set; }
		/// <summary>
		/// Gets or sets the maximun size of the <see cref="Text"/> before it wraps to a new line.
		/// </summary>
		/// <value>
		/// The width of the <see cref="Text"/>.
		/// </value>
		public float WrapWidth { get; set; }
		/// <summary>
		/// Gets or sets a value indicating whether the alignment of this <see cref="Text" /> is centered.
		/// See <see cref="Alignment"/>
		/// </summary>
		/// <value>
		///   <c>true</c> if centered; otherwise, <c>false</c>.
		/// </value>
		public bool Centered
		{
			get
			{
				return Alignment == Alignment.Center;
			}
			set
			{
				if (value)
				{
					Alignment = Alignment.Center;
				}
			}
		}

		/// <summary>
		/// Measures how many pixels in the horizontal axis this <see cref="Text"/> will use when drawn	against a 1280 pixel base
		/// </summary>
		public float Width
		{
			[SecuritySafeCritical]
			get
			{
				Function.Call(Hash._SET_TEXT_ENTRY_FOR_WIDTH, MemoryAccess.CellEmailBcon);

				foreach (IntPtr ptr in _pinnedText)
				{
					Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, ptr);
				}

				Function.Call(Hash.SET_TEXT_FONT, Font);
				Function.Call(Hash.SET_TEXT_SCALE, Scale, Scale);

				return Screen.Width*Function.Call<float>(Hash._GET_TEXT_SCREEN_WIDTH, 1);
			}
		}

		/// <summary>
		/// Measures how many pixels in the horizontal axis this <see cref="Text"/> will use when drawn against a <see cref="ScaledWidth"/> pixel base
		/// </summary>
		public float ScaledWidth
		{
			[SecuritySafeCritical]
			get
			{
				Function.Call(Hash._SET_TEXT_ENTRY_FOR_WIDTH, MemoryAccess.CellEmailBcon);

				foreach(IntPtr ptr in _pinnedText)
				{
					Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, ptr);
				}

				Function.Call(Hash.SET_TEXT_FONT, Font);
				Function.Call(Hash.SET_TEXT_SCALE, Scale, Scale);

				return Screen.ScaledWidth*Function.Call<float>(Hash._GET_TEXT_SCREEN_WIDTH, 1);
			}
		}

		/// <summary>
		/// Initializes a new instance of the <see cref="Text"/> class used for drawing text on the screen.
		/// </summary>
		/// <param name="caption">The <see cref="Text"/> to draw.</param>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Text"/>.</param>
		/// <param name="scale">Sets a <see cref="Scale"/> used to increase of decrease the size of the <see cref="Text"/>, for no scaling use 1.0f.</param>	
		public Text(string caption, PointF position, float scale) : this(caption, position, scale, Color.FromArgb(255, 255, 255, 255), Font.ChaletLondon, Alignment.Left, false, false, 0.0f)
		{
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Text"/> class used for drawing text on the screen.
		/// </summary>
		/// <param name="caption">The <see cref="Text"/> to draw.</param>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Text"/>.</param>
		/// <param name="scale">Sets a <see cref="Scale"/> used to increase of decrease the size of the <see cref="Text"/>, for no scaling use 1.0f.</param>
		/// <param name="color">Set the <see cref="Color"/> used to draw the <see cref="Text"/>.</param>							 	
		public Text(string caption, PointF position, float scale, Color color) : this(caption, position, scale, color, Font.ChaletLondon, Alignment.Left, false, false, 0.0f)
		{
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Text"/> class used for drawing text on the screen.
		/// </summary>
		/// <param name="caption">The <see cref="Text"/> to draw.</param>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Text"/>.</param>
		/// <param name="scale">Sets a <see cref="Scale"/> used to increase of decrease the size of the <see cref="Text"/>, for no scaling use 1.0f.</param>
		/// <param name="color">Set the <see cref="Color"/> used to draw the <see cref="Text"/>.</param>							 
		/// <param name="font">Sets the <see cref="Font"/> used when drawing the text.</param>	
		public Text(string caption, PointF position, float scale, Color color, Font font) : this(caption, position, scale, color, font, Alignment.Left, false, false, 0.0f)
		{
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Text"/> class used for drawing text on the screen.
		/// </summary>
		/// <param name="caption">The <see cref="Text"/> to draw.</param>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Text"/>.</param>
		/// <param name="scale">Sets a <see cref="Scale"/> used to increase of decrease the size of the <see cref="Text"/>, for no scaling use 1.0f.</param>
		/// <param name="color">Set the <see cref="Color"/> used to draw the <see cref="Text"/>.</param>							 
		/// <param name="font">Sets the <see cref="Font"/> used when drawing the text.</param>
		/// <param name="alignment">Sets the <see cref="Alignment"/> used when drawing the text, <see cref="GTA.UI.Alignment.Left"/>,<see cref="GTA.UI.Alignment.Center"/> or <see cref="GTA.UI.Alignment.Right"/>.</param>
		public Text(string caption, PointF position, float scale, Color color, Font font, Alignment alignment) : this(caption, position, scale, color, font, alignment, false, false, 0.0f)
		{
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Text"/> class used for drawing text on the screen.
		/// </summary>
		/// <param name="caption">The <see cref="Text"/> to draw.</param>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Text"/>.</param>
		/// <param name="scale">Sets a <see cref="Scale"/> used to increase of decrease the size of the <see cref="Text"/>, for no scaling use 1.0f.</param>
		/// <param name="color">Set the <see cref="Color"/> used to draw the <see cref="Text"/>.</param>							 
		/// <param name="font">Sets the <see cref="Font"/> used when drawing the text.</param>
		/// <param name="alignment">Sets the <see cref="Alignment"/> used when drawing the text, <see cref="GTA.UI.Alignment.Left"/>,<see cref="GTA.UI.Alignment.Center"/> or <see cref="GTA.UI.Alignment.Right"/>.</param>
		/// <param name="shadow">Sets whether or not to draw the <see cref="Text"/> with a <see cref="Shadow"/> effect.</param>
		/// <param name="outline">Sets whether or not to draw the <see cref="Text"/> with an <see cref="Outline"/> around the letters.</param>	
		public Text(string caption, PointF position, float scale, Color color, Font font, Alignment alignment, bool shadow, bool outline) : this(caption, position, scale, color, font, alignment, shadow, outline, 0.0f)
		{
		}

		/// <summary>
		/// Initializes a new instance of the <see cref="Text"/> class used for drawing text on the screen.
		/// </summary>
		/// <param name="caption">The <see cref="Text"/> to draw.</param>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Text"/>.</param>
		/// <param name="scale">Sets a <see cref="Scale"/> used to increase of decrease the size of the <see cref="Text"/>, for no scaling use 1.0f.</param>
		/// <param name="color">Set the <see cref="Color"/> used to draw the <see cref="Text"/>.</param>							 
		/// <param name="font">Sets the <see cref="Font"/> used when drawing the text.</param>
		/// <param name="alignment">Sets the <see cref="Alignment"/> used when drawing the text, <see cref="GTA.UI.Alignment.Left"/>,<see cref="GTA.UI.Alignment.Center"/> or <see cref="GTA.UI.Alignment.Right"/>.</param>
		/// <param name="shadow">Sets whether or not to draw the <see cref="Text"/> with a <see cref="Shadow"/> effect.</param>
		/// <param name="outline">Sets whether or not to draw the <see cref="Text"/> with an <see cref="Outline"/> around the letters.</param>
		/// <param name="wrapWidth">Sets how many horizontal pixel to draw before wrapping the <see cref="Text"/> on the next line down.</param>											 																	  
		public Text(string caption, PointF position, float scale, Color color, Font font, Alignment alignment, bool shadow, bool outline, float wrapWidth)
		{
			_pinnedText = new List<IntPtr>();
			Enabled = true;
			Caption = caption;
			Position = position;
			Scale = scale;
			Color = color;
			Font = font;
			Alignment = alignment;
			Shadow = shadow;
			Outline = outline;
			WrapWidth = wrapWidth;
		}

		~Text()
		{
			ClearText();
			_pinnedText.Clear();
		}

		[SecuritySafeCritical]
		private void ClearText()
		{
			foreach (var ptr in _pinnedText)
			{
				Marshal.FreeCoTaskMem(ptr); //free any existing allocated text
			}
		}

		/// <summary>
		/// Measures how many pixels in the horizontal axis the string will use when drawn
		/// </summary>
		/// <param name="text">The string of text to measure.</param>
		/// <param name="font">The <see cref="GTA.UI.Font"/> of the textu to measure.</param>
		/// <param name="scale">Sets a sclae value for increasing or decreasing the size of the text, default value 1.0f - no scaling.</param>
		/// <returns>
		/// The amount of pixels scaled on a 1280 pixel width base
		/// </returns>
		[SecuritySafeCritical]
		public static float GetStringWidth(string text, Font font = Font.ChaletLondon, float scale = 1.0f)
		{
			Function.Call(Hash._SET_TEXT_ENTRY_FOR_WIDTH, MemoryAccess.CellEmailBcon);

			const int maxStringLength = 99;

			for (int i = 0; i < text.Length; i += maxStringLength)
			{
				Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, text.Substring(i, System.Math.Min(maxStringLength, text.Length - i)));
			}

			Function.Call(Hash.SET_TEXT_FONT, font);
			Function.Call(Hash.SET_TEXT_SCALE, scale, scale);

			return Screen.Width * Function.Call<float>(Hash._GET_TEXT_SCREEN_WIDTH, 1);
		}
		/// <summary>
		/// Measures how many pixels in the horizontal axis the string will use when drawn
		/// </summary>
		/// <param name="text">The string of text to measure.</param>
		/// <param name="font">The <see cref="GTA.UI.Font"/> of the textu to measure.</param>
		/// <param name="scale">Sets a sclae value for increasing or decreasing the size of the text, default value 1.0f - no scaling.</param>
		/// <returns>
		/// The amount of pixels scaled by the pixel width base return in <see cref="Screen.ScaledWidth"/>
		/// </returns>
		[SecuritySafeCritical]
		public static float GetScaledStringWidth(string text, Font font = Font.ChaletLondon, float scale = 1.0f)
		{
			Function.Call(Hash._SET_TEXT_ENTRY_FOR_WIDTH, MemoryAccess.CellEmailBcon);

			const int maxStringLength = 99;

			for (int i = 0; i < text.Length; i += maxStringLength)
			{
				Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, text.Substring(i, System.Math.Min(maxStringLength, text.Length - i)));
			}

			Function.Call(Hash.SET_TEXT_FONT, font);
			Function.Call(Hash.SET_TEXT_SCALE, scale, scale);

			return Screen.ScaledWidth * Function.Call<float>(Hash._GET_TEXT_SCREEN_WIDTH, 1);
		}


		/// <summary>
		/// Draws the <see cref="Text" /> this frame.
		/// </summary>
		public virtual void Draw()
		{
			Draw(SizeF.Empty);
		}

		/// <summary>
		/// Draws the <see cref="Text" /> this frame at the specified offset.
		/// </summary>
		/// <param name="offset">The offset to shift the draw position of this <see cref="Text" /> using a 1280*720 pixel base.</param>
		public virtual void Draw(SizeF offset)
		{
			InternalDraw(offset, Screen.Width, Screen.Height);
		}

		/// <summary>
		/// Draws the <see cref="Text" /> this frame using the width returned in <see cref="Screen.ScaledWidth" />.
		/// </summary>
		public virtual void ScaledDraw()
		{
			ScaledDraw(SizeF.Empty);
		}

		/// <summary>
		/// Draws the <see cref="Text" /> this frame at the specified offset using the width returned in <see cref="Screen.ScaledWidth" />.
		/// </summary>
		/// <param name="offset">The offset to shift the draw position of this <see cref="Text" /> using a <see cref="Screen.ScaledWidth" />*720 pixel base.</param>
		public virtual void ScaledDraw(SizeF offset)
		{
			InternalDraw(offset, Screen.ScaledWidth, Screen.Height);
		}

		[SecuritySafeCritical]
		void InternalDraw(SizeF offset, float screenWidth, float screenHeight)
		{
			if (!Enabled)
			{
				return;
			}

			float x = (Position.X + offset.Width) / screenWidth;
			float y = (Position.Y + offset.Height) / screenHeight;
			float w = WrapWidth / screenWidth;

			if (Shadow)
			{
				Function.Call(Hash.SET_TEXT_DROP_SHADOW);
			}
			if (Outline)
			{
				Function.Call(Hash.SET_TEXT_OUTLINE);
			}

			Function.Call(Hash.SET_TEXT_FONT, Font);
			Function.Call(Hash.SET_TEXT_SCALE, Scale, Scale);
			Function.Call(Hash.SET_TEXT_COLOUR, Color.R, Color.G, Color.B, Color.A);
			Function.Call(Hash.SET_TEXT_JUSTIFICATION, Alignment);

			if (WrapWidth > 0.0f)
			{
				switch (Alignment)
				{
					case Alignment.Center:
						Function.Call(Hash.SET_TEXT_WRAP, x - (w / 2), x + (w / 2));
						break;
					case Alignment.Left:
						Function.Call(Hash.SET_TEXT_WRAP, x, x + w);
						break;
					case Alignment.Right:
						Function.Call(Hash.SET_TEXT_WRAP, x - w, x);
						break;
				}
			}
			else if (Alignment == Alignment.Right)
			{
				Function.Call(Hash.SET_TEXT_WRAP, 0.0f, x);
			}

			Function.Call(Hash._SET_TEXT_ENTRY, MemoryAccess.CellEmailBcon);

			foreach (IntPtr ptr in _pinnedText)
			{
				Function.Call(Hash.ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME, ptr);
			}

			Function.Call(Hash._DRAW_TEXT, x, y);
		}
	}
}
