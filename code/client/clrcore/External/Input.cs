using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
	public enum PadCheck
	{
		Any = 0,
		Keyboard = 1,
		Controller = 2
	}

	[Flags]
	public enum ControlModifier
	{
		Any = -1,
		None = 0,
		Ctrl = 1 << 0,
		Alt = 1 << 1,
		Shift = 1 << 2
	}

	/// <summary>
	/// Reference page: https://docs.fivem.net/docs/game-references/controls/
	/// </summary>
	public static class Input
	{
		// da far diventare costanti
		const int defaultControlGroup = 0;
		const int controllerControlGroup = 2;
		public static Dictionary<ControlModifier, int> ModifierFlagToKeyCode = new Dictionary<ControlModifier, int>() { [ControlModifier.Ctrl] = 36, [ControlModifier.Alt] = 19, [ControlModifier.Shift] = 21 };

		/// <summary>
		/// Checks if the current input is by controller or keyboard
		/// </summary>
		/// <returns>True if player is using the controller.</returns>
		public static bool WasLastInputFromController() => !IsUsingKeyboard(controllerControlGroup);

		/// <summary>
		/// Checks if a control modifier (alt, ctrl, shift) has been pressed.
		/// </summary>
		/// <param name="modifier">You can specify one or more modifiers (with |)</param>
		/// <returns>True if one or more modifiers are pressed</returns>
		public static bool IsControlModifierPressed(ControlModifier modifier)
		{
			if (modifier == ControlModifier.Any)
				return true;
			else
			{
				ControlModifier BitMask = 0;
				ModifierFlagToKeyCode.ToList().ForEach(w =>
				{
					if (Game.IsControlPressed(defaultControlGroup, (Control)w.Value) && IsUsingKeyboard(2)) BitMask |= w.Key;
				});

				if (BitMask == modifier)
					return true;
				else
					return false;
			}
		}

		/// <summary>
		/// Gets whether a <see cref="Control"/> was just pressed this frame
		/// </summary>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <param name="keyboardOnly">Filter for Mouse and Keyboard, GamePad or any</param>
		/// <param name="modifier">Filter for any modifier (alt, ctrl, shift, none, any)</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsControlJustPressed(Control control, PadCheck keyboardOnly = PadCheck.Any, ControlModifier modifier = ControlModifier.None) 
		{ 
			return Game.IsControlJustPressed(0, control) && (keyboardOnly == PadCheck.Keyboard ? !WasLastInputFromController() : keyboardOnly == PadCheck.Controller ? WasLastInputFromController() : !WasLastInputFromController() || WasLastInputFromController()) && IsControlModifierPressed(modifier); 
		}

		/// <summary>
		/// Gets whether a <see cref="Control"/> is currently pressed
		/// </summary>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <param name="keyboardOnly">Filter for Mouse and Keyboard, GamePad or any</param>
		/// <param name="modifier">Filter for any modifier (alt, ctrl, shift, none, any)</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsControlPressed(Control control, PadCheck keyboardOnly = PadCheck.Any, ControlModifier modifier = ControlModifier.None) 
		{ 
			return Game.IsControlPressed(0, control) && (keyboardOnly == PadCheck.Keyboard ? !WasLastInputFromController() : keyboardOnly == PadCheck.Controller ? WasLastInputFromController() : !WasLastInputFromController() || WasLastInputFromController()) && IsControlModifierPressed(modifier); 
		}

		/// <summary>
		/// Gets whether a <see cref="Control"/> was just released this frame
		/// </summary>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <param name="keyboardOnly">Filter for Mouse and Keyboard, GamePad or any</param>
		/// <param name="modifier">Filter for any modifier (alt, ctrl, shift, none, any)</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsControlJustReleased(Control control, PadCheck keyboardOnly = PadCheck.Any, ControlModifier modifier = ControlModifier.None) 
		{ 
			return Game.IsControlJustReleased(0, control) && (keyboardOnly == PadCheck.Keyboard ? !WasLastInputFromController() : keyboardOnly == PadCheck.Controller ? WasLastInputFromController() : !WasLastInputFromController() || WasLastInputFromController()) && IsControlModifierPressed(modifier); 
		}

		/// <summary>
		/// Gets whether a <see cref="Control"/> is Disabled and was just pressed this frame
		/// </summary>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <param name="keyboardOnly">Filter for Mouse and Keyboard, GamePad or any</param>
		/// <param name="modifier">Filter for any modifier (alt, ctrl, shift, none, any)</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsDisabledControlJustPressed(Control control, PadCheck keyboardOnly = PadCheck.Any, ControlModifier modifier = ControlModifier.None) 
		{ 
			return Game.IsDisabledControlJustPressed(0, control) && (keyboardOnly == PadCheck.Keyboard ? !WasLastInputFromController() : keyboardOnly == PadCheck.Controller ? WasLastInputFromController() : !WasLastInputFromController() || WasLastInputFromController()) && IsControlModifierPressed(modifier); 
		}

		/// <summary>
		/// Gets whether a <see cref="Control"/> is Disabled and was just released this frame
		/// </summary>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <param name="keyboardOnly">Filter for Mouse and Keyboard, GamePad or any</param>
		/// <param name="modifier">Filter for any modifier (alt, ctrl, shift, none, any)</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsDisabledControlJustReleased(Control control, PadCheck keyboardOnly = PadCheck.Any, ControlModifier modifier = ControlModifier.None) 
		{ 
			return Game.IsDisabledControlJustReleased(0, control) && (keyboardOnly == PadCheck.Keyboard ? !WasLastInputFromController() : keyboardOnly == PadCheck.Controller ? WasLastInputFromController() : !WasLastInputFromController() || WasLastInputFromController()) && IsControlModifierPressed(modifier); 
		}

		/// <summary>
		/// Gets whether a <see cref="Control"/> is Disabled this frame and is currently pressed
		/// </summary>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <param name="keyboardOnly">Filter for Mouse and Keyboard, GamePad or any</param>
		/// <param name="modifier">Filter for any modifier (alt, ctrl, shift, none, any)</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsDisabledControlPressed(Control control, PadCheck keyboardOnly = PadCheck.Any, ControlModifier modifier = ControlModifier.None) 
		{ 
			return Game.IsDisabledControlPressed(0, control) && (keyboardOnly == PadCheck.Keyboard ? !WasLastInputFromController() : keyboardOnly == PadCheck.Controller ? WasLastInputFromController() : !WasLastInputFromController() || WasLastInputFromController()) && IsControlModifierPressed(modifier); 
		}

		/// <summary>
		/// Gets whether a <see cref="Control"/> is Enabled and was just pressed this frame
		/// </summary>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <param name="keyboardOnly">Filter for Mouse and Keyboard, GamePad or any</param>
		/// <param name="modifier">Filter for any modifier (alt, ctrl, shift, none, any)</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsEnabledControlJustPressed(Control control, PadCheck keyboardOnly = PadCheck.Any, ControlModifier modifier = ControlModifier.None) 
		{ 
			return Game.IsEnabledControlJustPressed(0, control) && (keyboardOnly == PadCheck.Keyboard ? !WasLastInputFromController() : keyboardOnly == PadCheck.Controller ? WasLastInputFromController() : !WasLastInputFromController() || WasLastInputFromController()) && IsControlModifierPressed(modifier); 
		}

		/// <summary>
		/// Gets whether a <see cref="Control"/> is Enabled and was just released this frame
		/// </summary>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <param name="keyboardOnly">Filter for Mouse and Keyboard, GamePad or any</param>
		/// <param name="modifier">Filter for any modifier (alt, ctrl, shift, none, any)</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>
		public static bool IsEnabledControlJustReleased(Control control, PadCheck keyboardOnly = PadCheck.Any, ControlModifier modifier = ControlModifier.None) 
		{ 
			return Game.IsEnabledControlJustPressed(0, control) && (keyboardOnly == PadCheck.Keyboard ? !WasLastInputFromController() : keyboardOnly == PadCheck.Controller ? WasLastInputFromController() : !WasLastInputFromController() || WasLastInputFromController()) && IsControlModifierPressed(modifier); 
		}

		/// <summary>
		/// Gets whether a <see cref="Control"/> is Enabled this frame and is currently pressed
		/// </summary>
		/// <param name="control">The <see cref="Control"/>.</param>
		/// <param name="keyboardOnly">Filter for Mouse and Keyboard, GamePad or any</param>
		/// <param name="modifier">Filter for any modifier (alt, ctrl, shift, none, any)</param>
		/// <returns><c>true</c> if the <see cref="Control"/> is pressed; otherwise, <c>false</c></returns>

		public static bool IsEnabledControlPressed(Control control, PadCheck keyboardOnly = PadCheck.Any, ControlModifier modifier = ControlModifier.None) 
		{ 
			return Game.IsEnabledControlPressed(0, control) && (keyboardOnly == PadCheck.Keyboard ? !WasLastInputFromController() : keyboardOnly == PadCheck.Controller ? WasLastInputFromController() : !WasLastInputFromController() || WasLastInputFromController()) && IsControlModifierPressed(modifier); 
		}

		/// <summary>
		/// Awaits while a key is pressed and returns true if the key is still pressed after a certain amount of time.
		/// </summary>
		/// <param name="control">The <see cref="Control"/> you want to await</param>
		/// <param name="keyboardOnly">Only keyboard or gamepad or both?</param>
		/// <param name="modifier">The <see cref="ControlModifier"/> additional</param>
		/// <param name="timeout">Time to wait before returning <see langword="true"/>or <see langword="false"/> (in milliseconds).</param>
		/// <returns>Ritorna se il player ha tenuto premuto più del tempo specificato</returns>
		public static async Task<bool> IsControlStillPressedAsync(Control control, PadCheck keyboardOnly = PadCheck.Any, ControlModifier modifier = ControlModifier.None, int timeout = 1000)
		{
			int currentTicks = Game.GameTime + 1;
			while (IsControlPressed(control, keyboardOnly, modifier) && Game.GameTime - currentTicks < timeout) await BaseScript.Delay(0);
			return Game.GameTime - currentTicks >= timeout;
		}
	}
}
