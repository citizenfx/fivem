#if MONO_V2
namespace CitizenFX.FiveM
#else
namespace CitizenFX.Core
#endif
{
	public enum Relationship
	{
		Hate = 5,
		Dislike = 4,
		Neutral = 3,
		Like = 2,
		Respect = 1,
		Companion = 0,
		Pedestrians = 255
	}
}
