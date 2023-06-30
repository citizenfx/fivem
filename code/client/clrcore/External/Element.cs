using System;
using System.Drawing;
using System.Collections.Generic;
using CitizenFX.Core.Native;

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
	public interface IElement
	{

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="IElement"/> will be drawn.
		/// </summary>
		/// <value>
		///   <c>true</c> if enabled; otherwise, <c>false</c>.
		/// </value>
		bool Enabled { get; set; }

		/// <summary>
		/// Gets or sets the color of this <see cref="IElement"/>.
		/// </summary>
		/// <value>
		/// The color.
		/// </value>
		Color Color { get; set; }

		/// <summary>
		/// Gets or sets the position of this <see cref="IElement"/>.
		/// </summary>
		/// <value>
		/// The position scaled on a 1280*720 pixel base.
		/// </value>
		/// <remarks>
		/// If ScaledDraw is called, the position will be scaled by the width returned in <see cref="Screen.ScaledWidth"/>.
		/// </remarks>
		PointF Position { get; set; }

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="IElement"/> should be positioned based on its center or top left corner
		/// </summary>
		/// <value>
		///   <c>true</c> if centered; otherwise, <c>false</c>.
		/// </value>
		bool Centered { get; set; }

		/// <summary>
		/// Draws this <see cref="IElement"/> this frame.
		/// </summary>
		void Draw();


		/// <summary>
		/// Draws this <see cref="IElement"/> this frame at the specified offset.
		/// </summary>
		/// <param name="offset">The offset to shift the draw position of this <see cref="IElement"/> using a 1280*720 pixel base.</param>
		void Draw(SizeF offset);


		/// <summary>
		/// Draws this <see cref="IElement"/> this frame using the width returned in <see cref="Screen.ScaledWidth"/>.
		/// </summary>
		void ScaledDraw();

		/// <summary>
		/// Draws this <see cref="IElement"/> this frame at the specified offset using the width returned in <see cref="Screen.ScaledWidth"/>.
		/// </summary>
		/// <param name="offset">The offset to shift the draw position of this <see cref="IElement"/> using a <see cref="Screen.ScaledWidth"/>*720 pixel base.</param>
		void ScaledDraw(SizeF offset);
	}

	
	public class Rectangle : IElement
	{
		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Rectangle"/> will be drawn.
		/// </summary>
		/// <value>
		///   <c>true</c> if enabled; otherwise, <c>false</c>.
		/// </value>
		public virtual bool Enabled { get; set; }

		/// <summary>
		/// Gets or sets the color of this <see cref="Rectangle"/>.
		/// </summary>
		/// <value>
		/// The color.
		/// </value>
		public virtual Color Color { get; set; }

		/// <summary>
		/// Gets or sets the position of this <see cref="Rectangle"/>.
		/// </summary>
		/// <value>
		/// The position scaled on a 1280*720 pixel base.
		/// </value>
		/// <remarks>
		/// If ScaledDraw is called, the position will be scaled by the width returned in <see cref="Screen.ScaledWidth"/>.
		/// </remarks>
		public virtual PointF Position { get; set; }

		/// <summary>
		/// Gets or sets a value indicating whether this <see cref="Rectangle"/> should be positioned based on its center or top left corner
		/// </summary>
		/// <value>
		///   <c>true</c> if centered; otherwise, <c>false</c>.
		/// </value>
		public virtual bool Centered { get; set; }
		/// <summary>
		/// Gets or sets the size to draw the <see cref="Rectangle"/>
		/// </summary>
		/// <value>
		/// The size on a 1280*720 pixel base
		/// </value>
		/// <remarks>
		/// If ScaledDraw is called, the size will be scaled by the width returned in <see cref="Screen.ScaledWidth"/>.
		/// </remarks>
		public SizeF Size { get; set; }

		/// <summary>
		/// Initializes a new instance of the <see cref="Rectangle"/> class used for grouping drawing Rectangles on screen.
		/// </summary>	 
		public Rectangle() : this(PointF.Empty, new SizeF(Screen.Width, Screen.Height), Color.Transparent, false)
		{
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Rectangle"/> class used for grouping drawing Rectangles on screen.
		/// </summary>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Rectangle"/>.</param>
		/// <param name="size">Set the <see cref="Size"/> of the <see cref="Container"/>.</param>						 							
		public Rectangle(PointF position, SizeF size) : this(position, size, Color.Transparent, false)
		{
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Rectangle"/> class used for grouping drawing Rectangles on screen.
		/// </summary>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Rectangle"/>.</param>
		/// <param name="size">Set the <see cref="Size"/> of the <see cref="Container"/>.</param>
		/// <param name="color">Set the <see cref="Color"/> used to draw the <see cref="Container"/>.</param>							 							
		public Rectangle(PointF position, SizeF size, Color color) : this(position, size, color, false)
		{
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Rectangle"/> class used for grouping drawing Rectangles on screen.
		/// </summary>
		/// <param name="position">Set the <see cref="Position"/> on screen where to draw the <see cref="Rectangle"/>.</param>
		/// <param name="size">Set the <see cref="Size"/> of the <see cref="Container"/>.</param>
		/// <param name="color">Set the <see cref="Color"/> used to draw the <see cref="Container"/>.</param>							 
		/// <param name="centered">Position the <see cref="Rectangle"/> based on its center instead of top left corner, see also <seealso cref="Centered"/>.</param>
		public Rectangle(PointF position, SizeF size, Color color, bool centered)
		{
			Enabled = true;
			Position = position;
			Size = size;
			Color = color;
			Centered = centered;
		}


		/// <summary>
		/// Draws this <see cref="Rectangle" /> this frame.
		/// </summary>
		public virtual void Draw()
		{
			Draw(SizeF.Empty);
		}


		/// <summary>
		/// Draws this <see cref="Rectangle" /> this frame at the specified offset.
		/// </summary>
		/// <param name="offset">The offset to shift the draw position of this <see cref="Rectangle" /> using a 1280*720 pixel base.</param>
		public virtual void Draw(SizeF offset)
		{
			InternalDraw(offset, 1.0f / Screen.Width, 1.0f / Screen.Height);
		}

		/// <summary>
		/// Draws this <see cref="Rectangle" /> this frame using the width returned in <see cref="Screen.ScaledWidth" />.
		/// </summary>
		public virtual void ScaledDraw()
		{
			ScaledDraw(SizeF.Empty);
		}

		/// <summary>
		/// Draws this <see cref="Rectangle" /> this frame at the specified offset using the width returned in <see cref="Screen.ScaledWidth" />.
		/// </summary>
		/// <param name="offset">The offset to shift the draw position of this <see cref="Rectangle" /> using a <see cref="Screen.ScaledWidth" />*720 pixel base.</param>
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
			float w = Size.X * inverseScreenWidth;
			float h = Size.Y * inverseScreenHeight;
			float x = (Position.X + offset.X) * inverseScreenWidth;
			float y = (Position.Y + offset.Y) * inverseScreenHeight;
#else
			float w = Size.Width * inverseScreenWidth;
			float h = Size.Height * inverseScreenHeight;
			float x = (Position.X + offset.Width) * inverseScreenWidth;
			float y = (Position.Y + offset.Height) * inverseScreenHeight;
#endif

			if (!Centered)
			{
				x += w * 0.5f;
				y += h * 0.5f;
			}

			API.DrawRect(x, y, w, h, Color.R, Color.G, Color.B, Color.A);
		}
	}
	public class Container : Rectangle
	{
		/// <summary>
		/// The <see cref="IElement"/>s Contained inside this <see cref="Container"/>
		/// </summary>
		public List<IElement> Items { get; private set; }

		/// <summary>
		/// Initializes a new instance of the <see cref="Container"/> class used for grouping <see cref="IElement"/>s together.
		/// </summary>																																						
		public Container()
		{
			Items = new List<IElement>();
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Container"/> class used for grouping <see cref="IElement"/>s together.
		/// </summary>
		/// <param name="position">Set the <see cref="Rectangle.Position"/> on screen where to draw the <see cref="Container"/>.</param>
		/// <param name="size">Set the <see cref="Rectangle.Size"/> of the <see cref="Container"/>.</param>																	   
		public Container(PointF position, SizeF size) : base(position, size)
		{
			Items = new List<IElement>();
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Container"/> class used for grouping <see cref="IElement"/>s together.
		/// </summary>
		/// <param name="position">Set the <see cref="Rectangle.Position"/> on screen where to draw the <see cref="Container"/>.</param>
		/// <param name="size">Set the <see cref="Rectangle.Size"/> of the <see cref="Container"/>.</param>
		/// <param name="color">Set the <see cref="Rectangle.Color"/> used to draw the <see cref="Container"/>.</param>							 							   
		public Container(PointF position, SizeF size, Color color) : base(position, size, color)
		{
			Items = new List<IElement>();
		}
		/// <summary>
		/// Initializes a new instance of the <see cref="Container"/> class used for grouping <see cref="IElement"/>s together.
		/// </summary>
		/// <param name="position">Set the <see cref="Rectangle.Position"/> on screen where to draw the <see cref="Container"/>.</param>
		/// <param name="size">Set the <see cref="Rectangle.Size"/> of the <see cref="Container"/>.</param>
		/// <param name="color">Set the <see cref="Rectangle.Color"/> used to draw the <see cref="Container"/>.</param>							 
		/// <param name="centered">Position the <see cref="Container"/> based on its center instead of top left corner, see also <seealso cref="Rectangle.Centered"/>.</param>
		public Container(PointF position, SizeF size, Color color, bool centered) : base(position, size, color, centered)
		{
			Items = new List<IElement>();
		}


		/// <summary>
		/// Draws this <see cref="Container" /> this frame and all its <see cref="IElement"/>s.
		/// </summary>
		public override void Draw()
		{
			Draw(SizeF.Empty);
		}

		/// <summary>
		/// Draws this <see cref="Container" /> this frame and all its <see cref="IElement"/>s at the specified offset.
		/// </summary>
		/// <param name="offset">The offset to shift the draw position of this <see cref="Rectangle" /> using a 1280*720 pixel base.</param>
		public override void Draw(SizeF offset)
		{
			if (!Enabled)
			{
				return;
			}

			base.Draw(offset);

			offset += new SizeF(Position);

			if (Centered)
			{
#if MONO_V2
				offset -= Size * 0.5f;
#else
				offset -= new SizeF(Size.Width * 0.5f, Size.Height * 0.5f);
#endif
			}

			foreach (var item in Items)
			{
				item.Draw(offset);
			}
		}

		/// <summary>
		/// Draws this <see cref="Container" /> this frame and all its <see cref="IElement"/>s using the width returned in <see cref="Screen.ScaledWidth" />.
		/// </summary>
		public override void ScaledDraw()
		{
			ScaledDraw(SizeF.Empty);
		}

		/// <summary>
		/// Draws this <see cref="Container" /> this frame and all its <see cref="IElement"/>s at the specified offset using the width returned in <see cref="Screen.ScaledWidth" />.
		/// </summary>
		/// <param name="offset">The offset to shift the draw position of this <see cref="Container" /> using a <see cref="Screen.ScaledWidth" />*720 pixel base.</param>
		public override void ScaledDraw(SizeF offset)
		{
			if (!Enabled)
			{
				return;
			}

			base.ScaledDraw(offset);

			offset += new SizeF(Position);

			if (Centered)
			{
#if MONO_V2
				offset -= Size * 0.5f;
#else
				offset -= new SizeF(Size.Width * 0.5f, Size.Height * 0.5f);
#endif
			}

			foreach (var item in Items)
			{
				item.ScaledDraw(offset);
			}
		}
	}
}
