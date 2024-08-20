
namespace CitizenFX.RedM
{
	public static partial class Game
	{
		/// <summary>
		/// Calculates a Jenkins One At A Time hash from the given <see cref="string"/> which can then be used by any native function that takes a hash
		/// </summary>
		/// <param name="input">The input <see cref="string"/> to hash.</param>
		/// <returns>The Jenkins hash of the <see cref="string"/></returns>
		public static uint GenerateHash(string input)
		{
			uint hash = 0;
			// If reorganization is needed then this encryption is better off in separate type, e.g.: `Encryption`
			if (!string.IsNullOrEmpty(input))
			{
				var len = input.Length;

				input = input.ToLowerInvariant();

				for (var i = 0; i < len; i++)
				{
					hash += input[i];
					hash += hash << 10;
					hash ^= hash >> 6;
				}

				hash += hash << 3;
				hash ^= hash >> 11;
				hash += hash << 15;

			}

			return hash;
		}
		
	}
}
