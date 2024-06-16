using System;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;
using CitizenFX.Core.Native;

using System.Security;

#if MONO_V2
using CitizenFX.Core;
using PointF = CitizenFX.Core.Vector2;
using SizeF = CitizenFX.Core.Vector2;
using API = CitizenFX.FiveM.Native.Natives;
using String = CitizenFX.Core.CString;
using UI = CitizenFX.FiveM.GUI;

namespace CitizenFX.FiveM.GUI
#else
namespace CitizenFX.Core.UI
#endif
{
	public enum Alignment
	{
		Center = 0,
		Left = 1,
		Right = 2,
	}

	public class Text : IElement
	{
#if MONO_V2
		private String[] _captions;
#else
		private string _caption;
#endif

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
		/// <value>The caption.</value>
#if MONO_V2
		/// <remarks>This property is slow as it merges or splits the value from/to 99 chararacter length strings, use <see cref="SplitCaption"/> to access the split string directly.</remarks>
		public CString Caption
		{
			get => _captions != null ? CString.Concat(_captions) : CString.Empty;
			set => _captions = SplitString(value);
		}

		/// <value>The internally split captions</value>
		/// <inheritdoc cref="Caption"/>
		public CString[] SplitCaption
		{
			get => _captions;
			set => _captions = value;
		}
#else
		public string Caption
		{
			get { return _caption ?? String.Empty; }
			set
			{
				//const int maxStringLength = 99;

				//if (value.Length > maxStringLength)
				//{
				//	_caption = value.Substring(0, maxStringLength);
				//	return;
				//}
				_caption = value;
			}
		}
#endif

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
		/// Measures how many pixels on the horizontal axis this <see cref="Text"/> will use when drawn	against a 1280 pixel base
		/// </summary>
		public float Width
		{
			get
			{
				API.BeginTextCommandGetWidth("CELL_EMAIL_BCON");
#if MONO_V2
				for (int i = 0; i < _captions?.Length; ++i)
				{
					API.AddTextComponentSubstringPlayerName(_captions[i]);
				}
#else
				foreach (var s in Screen.StringToArray(Caption))
				{
					API.AddTextComponentSubstringPlayerName(s);
				}
#endif
				API.SetTextFont((int)Font);
				API.SetTextScale(Scale, Scale);

				return Screen.Width * API.EndTextCommandGetWidth(true);
			}
		}

		/// <summary>
		/// Measures how many pixels in the horizontal axis this <see cref="Text"/> will use when drawn against a <see cref="ScaledWidth"/> pixel base
		/// </summary>
		public float ScaledWidth
		{
			get
			{
				API.BeginTextCommandGetWidth("CELL_EMAIL_BCON");
#if MONO_V2
				for (int i = 0; i < _captions?.Length; ++i)
				{
					API.AddTextComponentSubstringPlayerName(_captions[i]);
				}
#else
				foreach (var s in Screen.StringToArray(Caption))
				{
					API.AddTextComponentSubstringPlayerName(s);
				}
#endif
				API.SetTextFont((int)Font);
				API.SetTextScale(Scale, Scale);

				return Screen.ScaledWidth * API.EndTextCommandGetWidth(true);
			}
		}

#if !MONO_V2
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
		/// <param name="alignment">Sets the <see cref="Alignment"/> used when drawing the text, <see cref="UI.Alignment.Left"/>,<see cref="UI.Alignment.Center"/> or <see cref="UI.Alignment.Right"/>.</param>
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
		/// <param name="alignment">Sets the <see cref="Alignment"/> used when drawing the text, <see cref="UI.Alignment.Left"/>,<see cref="UI.Alignment.Center"/> or <see cref="UI.Alignment.Right"/>.</param>
		/// <param name="shadow">Sets whether or not to draw the <see cref="Text"/> with a <see cref="Shadow"/> effect.</param>
		/// <param name="outline">Sets whether or not to draw the <see cref="Text"/> with an <see cref="Outline"/> around the letters.</param>
		public Text(string caption, PointF position, float scale, Color color, Font font, Alignment alignment, bool shadow, bool outline) : this(caption, position, scale, color, font, alignment, shadow, outline, 0.0f)
		{
		}
#endif

		/// <summary>
		/// Initializes a new instance of the <see cref="Text"/> class used for drawing text on the screen.
		/// </summary>
		/// <param name="caption">The <see cref="Text"/> to draw.</param>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Text"/>.</param>
		/// <param name="scale">Sets a <see cref="Scale"/> used to increase of decrease the size of the <see cref="Text"/>, for no scaling use 1.0f.</param>
		/// <param name="color">Set the <see cref="Color"/> used to draw the <see cref="Text"/>.</param>
		/// <param name="font">Sets the <see cref="Font"/> used when drawing the text.</param>
		/// <param name="alignment">Sets the <see cref="Alignment"/> used when drawing the text, <see cref="UI.Alignment.Left"/>,<see cref="UI.Alignment.Center"/> or <see cref="UI.Alignment.Right"/>.</param>
		/// <param name="shadow">Sets whether or not to draw the <see cref="Text"/> with a <see cref="Shadow"/> effect.</param>
		/// <param name="outline">Sets whether or not to draw the <see cref="Text"/> with an <see cref="Outline"/> around the letters.</param>
		/// <param name="wrapWidth">Sets how many horizontal pixel to draw before wrapping the <see cref="Text"/> on the next line down.</param>											 																	  
		/// <remarks><paramref name="caption"/> will be divided in multiple strings of 99 or less characters.<br />Any <paramref name="caption"/> that is 396+ characters long (5+ divisions) is known to <i>not</i> be rendered by GTA V.</remarks>

		public Text(string caption, PointF position, float scale, Color color, Font font = Font.ChaletLondon, Alignment alignment = Alignment.Left, bool shadow = false, bool outline = false, float wrapWidth = 0.0f)
#if !MONO_V2
		{
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
#else
			: this(SplitString(caption), position, scale, color, font, alignment, shadow, outline, wrapWidth)
		{ }

		/// <inheritdoc cref="Text(string, PointF, float, Color, Font, Alignment, bool, bool, float)"/>
		public Text(string caption, PointF position, float scale = 1.0f) : this(caption, position, scale, Color.White)
		{ }

		/// <remarks><paramref name="caption"/> needs to contain strings of 99 or less characters.<br />Any <paramref name="caption"/> that is 396+ characters long (5+ divisions) is known to <i>not</i> be rendered by GTA V.</remarks>
		/// <inheritdoc cref="Text(string, PointF, float, Color, Font, Alignment, bool, bool, float)"/>
		public Text(CString[] caption, PointF position, float scale, Color color, Font font = Font.ChaletLondon, Alignment alignment = Alignment.Left, bool shadow = false, bool outline = false, float wrapWidth = 0.0f)
		{
			Enabled = true;
			SplitCaption = caption;
			Position = position;
			Scale = scale;
			Color = color;
			Font = font;
			Alignment = alignment;
			Shadow = shadow;
			Outline = outline;
			WrapWidth = wrapWidth;
		}

		/// <inheritdoc cref="Text(CString[], PointF, float, Color, Font, Alignment, bool, bool, float)"/>
		public Text(CString[] caption, PointF position, float scale = 1.0f) : this(caption, position, scale, Color.White)
		{ }
#endif

		~Text()
		{
		}

		/// <summary>
		/// Measures how many pixels in the horizontal axis the string will use when drawn
		/// </summary>
		/// <param name="text">The string of text to measure.</param>
		/// <param name="font">The <see cref="UI.Font"/> to use when measuring the text's width.</param>
		/// <param name="scale">Sets a sclae value for increasing or decreasing the size of the text, default value 1.0f - no scaling.</param>
		/// <returns>
		/// The amount of pixels scaled on a 1280 pixel width base
		/// </returns>
		public static float GetStringWidth(String text, Font font = Font.ChaletLondon, float scale = 1.0f)
			=> GetStringWidth(SplitString(text), font, scale);

		/// <inheritdoc cref="GetStringWidth(String, Font, float)"/>
		/// <param name="texts">The series of strings of texts to measure, each string should not be bigger than 99 characters.</param>
		public static float GetStringWidth(String[] texts, Font font = Font.ChaletLondon, float scale = 1.0f)
		{
			API.BeginTextCommandGetWidth("CELL_EMAIL_BCON");

			for (var i = 0; i < texts?.Length; ++i)
			{
				API.AddTextComponentSubstringPlayerName(texts[i]);
			}

			API.SetTextFont((int)font);
			API.SetTextScale(scale, scale);

			return Screen.Width * API.EndTextCommandGetWidth(true);
		}


		/// <summary>
		/// Measures how many pixels in the horizontal axis the string will use when drawn
		/// </summary>
		/// <param name="text">The string of text to measure.</param>
		/// <param name="font">The <see cref="UI.Font"/> of the textu to measure.</param>
		/// <param name="scale">Sets a sclae value for increasing or decreasing the size of the text, default value 1.0f - no scaling.</param>
		/// <returns>
		/// The amount of pixels scaled by the pixel width base return in <see cref="Screen.ScaledWidth"/>
		/// </returns>
		public static float GetScaledStringWidth(String text, Font font = Font.ChaletLondon, float scale = 1.0f)
			=> GetScaledStringWidth(SplitString(text), font, scale);

		/// <inheritdoc cref="GetStringWidth(String, Font, float)"/>
		/// <param name="texts">The series of strings of texts to measure, each string should not be bigger than 99 characters.</param>
		public static float GetScaledStringWidth(String[] texts, Font font = Font.ChaletLondon, float scale = 1.0f)
		{
			API.BeginTextCommandGetWidth("CELL_EMAIL_BCON");

			for (var i = 0; i < texts?.Length; ++i)
			{
				API.AddTextComponentSubstringPlayerName(texts[i]);
			}

			API.SetTextFont((int)font);
			API.SetTextScale(scale, scale);

			return Screen.ScaledWidth * API.EndTextCommandGetWidth(true);
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
			InternalDraw(offset, 1.0f / Screen.Width, 1.0f / Screen.Height);
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
			InternalDraw(offset, 1.0f / Screen.ScaledWidth, 1.0f / Screen.Height);
		}

		void InternalDraw(SizeF offset, float inverseScreenWidth, float inverseScreenHeight)
		{
			if (!Enabled)
			{
				return;
			}

#if MONO_V2
			float x = (Position.X + offset.X) * inverseScreenWidth;
			float y = (Position.Y + offset.Y) * inverseScreenWidth;
#else
			float x = (Position.X + offset.Width) * inverseScreenWidth;
			float y = (Position.Y + offset.Height) * inverseScreenHeight;
#endif
			float w = WrapWidth * inverseScreenWidth;

			if (Shadow)
			{
				API.SetTextDropShadow();
			}

			if (Outline)
			{
				API.SetTextOutline();
			}

			API.SetTextFont((int)Font);
			API.SetTextScale(Scale, Scale);
			API.SetTextColour(Color.R, Color.G, Color.B, Color.A);
			API.SetTextJustification((int)Alignment);

			if (WrapWidth > 0.0f)
			{
				switch (Alignment)
				{
					case Alignment.Center:
						API.SetTextWrap(x - (w * 0.5f), x + (w * 0.5f));
						break;
					case Alignment.Left:
						API.SetTextWrap(x, x + w);
						break;
					case Alignment.Right:
						API.SetTextWrap(x - w, x);
						break;
				}
			}
			else if (Alignment == Alignment.Right)
			{
				API.SetTextWrap(0.0f, x);
			}

			API.BeginTextCommandDisplayText("CELL_EMAIL_BCON");

#if MONO_V2
			for (int i = 0; i < _captions?.Length; ++i)
			{
				API.AddTextComponentSubstringPlayerName(_captions[i]);
			}
#else
			foreach (var s in Screen.StringToArray(Caption))
			{
				API.AddTextComponentSubstringPlayerName(s);
			}
#endif

			API.EndTextCommandDisplayText(x, y);
		}

		/// <summary>
		/// Splits the <paramref name="inputString"/> into an array with each string having 99 characters or less<br />
		/// This is needed as characters beyond 99 aren't rendered by GTA V, e.g.: with <see cref="Text"/>
		/// </summary>
		/// <remarks>arrays of 5 and higher (396+ characters) are known to <i>not</i> being rendered by GTA V</remarks>
		/// <param name="inputString">The string to convert.</param>
		/// <returns>array containing strings each 99 characters or less.</returns>
		public static String[] SplitString(String inputString)
		{
			int stringsNeeded = (inputString.Length - 1) / 99 + 1; // division with round up

			String[] outputString = new String[stringsNeeded];
			for (int i = 0; i < stringsNeeded; i++)
			{
#if MONO_V2
				outputString[i] = inputString.Substring(i * 99, 99);
#else
				outputString[i] = inputString.Substring(i * 99, MathUtil.Clamp(inputString.Substring(i * 99).Length, 0, 99));
#endif
			}

			return outputString;
		}
	}
}
