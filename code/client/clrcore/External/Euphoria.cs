using System;
using System.Collections.Generic;
using CitizenFX.Core;

namespace CitizenFX.Core.NaturalMotion
{
	public sealed class Euphoria
	{
		#region Fields

		readonly Ped _ped;
		readonly Dictionary<string, CustomHelper> _helperCache;

		#endregion

		internal Euphoria(Ped ped)
		{
			_ped = ped;
			_helperCache = new Dictionary<string, CustomHelper>();
		}

		internal T GetHelper<T>(string message) where T : CustomHelper
		{
			CustomHelper h;

			if (!_helperCache.TryGetValue(message, out h))
			{
				h = (CustomHelper) Activator.CreateInstance(typeof(T), _ped);

				_helperCache.Add(message, h);
			}

			return (T) h;
		}

		/// <summary>
		/// Gets a ActivePose Helper class for sending ActivePose <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ActivePoseHelper ActivePose
		{
			get { return GetHelper<ActivePoseHelper>("activePose"); }
		}

		/// <summary>
		/// Gets a ApplyImpulse Helper class for sending ApplyImpulse <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ApplyImpulseHelper ApplyImpulse
		{
			get { return GetHelper<ApplyImpulseHelper>("applyImpulse"); }
		}

		/// <summary>
		/// Gets a ApplyBulletImpulse Helper class for sending ApplyBulletImpulse <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ApplyBulletImpulseHelper ApplyBulletImpulse
		{
			get { return GetHelper<ApplyBulletImpulseHelper>("applyBulletImpulse"); }
		}

		/// <summary>
		/// Gets a BodyRelax Helper class for sending BodyRelax <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Set the amount of relaxation across the whole body; Used to collapse the character into a rag-doll-like state.
		/// </remarks>
		public BodyRelaxHelper BodyRelax
		{
			get { return GetHelper<BodyRelaxHelper>("bodyRelax"); }
		}

		/// <summary>
		/// Gets a ConfigureBalance Helper class for sending ConfigureBalance <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// This single message allows you to configure various parameters used on any behaviour that uses the dynamic balance.
		/// </remarks>
		public ConfigureBalanceHelper ConfigureBalance
		{
			get { return GetHelper<ConfigureBalanceHelper>("configureBalance"); }
		}

		/// <summary>
		/// Gets a ConfigureBalanceReset Helper class for sending ConfigureBalanceReset <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// reset the values configurable by the Configure Balance message to their defaults.
		/// </remarks>
		public ConfigureBalanceResetHelper ConfigureBalanceReset
		{
			get { return GetHelper<ConfigureBalanceResetHelper>("configureBalanceReset"); }
		}

		/// <summary>
		/// Gets a ConfigureSelfAvoidance Helper class for sending ConfigureSelfAvoidance <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// this single message allows to configure self avoidance for the character.BBDD Self avoidance tech.
		/// </remarks>
		public ConfigureSelfAvoidanceHelper ConfigureSelfAvoidance
		{
			get { return GetHelper<ConfigureSelfAvoidanceHelper>("configureSelfAvoidance"); }
		}

		/// <summary>
		/// Gets a ConfigureBullets Helper class for sending ConfigureBullets <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ConfigureBulletsHelper ConfigureBullets
		{
			get { return GetHelper<ConfigureBulletsHelper>("configureBullets"); }
		}

		/// <summary>
		/// Gets a ConfigureBulletsExtra Helper class for sending ConfigureBulletsExtra <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ConfigureBulletsExtraHelper ConfigureBulletsExtra
		{
			get { return GetHelper<ConfigureBulletsExtraHelper>("configureBulletsExtra"); }
		}

		/// <summary>
		/// Gets a ConfigureLimits Helper class for sending ConfigureLimits <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Enable/disable/edit character limits in real time.  This adjusts limits in RAGE-native space and will *not* reorient the joint.
		/// </remarks>
		public ConfigureLimitsHelper ConfigureLimits
		{
			get { return GetHelper<ConfigureLimitsHelper>("configureLimits"); }
		}

		/// <summary>
		/// Gets a ConfigureSoftLimit Helper class for sending ConfigureSoftLimit <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ConfigureSoftLimitHelper ConfigureSoftLimit
		{
			get { return GetHelper<ConfigureSoftLimitHelper>("configureSoftLimit"); }
		}

		/// <summary>
		/// Gets a ConfigureShotInjuredArm Helper class for sending ConfigureShotInjuredArm <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// This single message allows you to configure the injured arm reaction during shot
		/// </remarks>
		public ConfigureShotInjuredArmHelper ConfigureShotInjuredArm
		{
			get { return GetHelper<ConfigureShotInjuredArmHelper>("configureShotInjuredArm"); }
		}

		/// <summary>
		/// Gets a ConfigureShotInjuredLeg Helper class for sending ConfigureShotInjuredLeg <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// This single message allows you to configure the injured leg reaction during shot
		/// </remarks>
		public ConfigureShotInjuredLegHelper ConfigureShotInjuredLeg
		{
			get { return GetHelper<ConfigureShotInjuredLegHelper>("configureShotInjuredLeg"); }
		}

		/// <summary>
		/// Gets a DefineAttachedObject Helper class for sending DefineAttachedObject <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public DefineAttachedObjectHelper DefineAttachedObject
		{
			get { return GetHelper<DefineAttachedObjectHelper>("defineAttachedObject"); }
		}

		/// <summary>
		/// Gets a ForceToBodyPart Helper class for sending ForceToBodyPart <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Apply an impulse to a named body part
		/// </remarks>
		public ForceToBodyPartHelper ForceToBodyPart
		{
			get { return GetHelper<ForceToBodyPartHelper>("forceToBodyPart"); }
		}

		/// <summary>
		/// Gets a LeanInDirection Helper class for sending LeanInDirection <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public LeanInDirectionHelper LeanInDirection
		{
			get { return GetHelper<LeanInDirectionHelper>("leanInDirection"); }
		}

		/// <summary>
		/// Gets a LeanRandom Helper class for sending LeanRandom <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public LeanRandomHelper LeanRandom
		{
			get { return GetHelper<LeanRandomHelper>("leanRandom"); }
		}

		/// <summary>
		/// Gets a LeanToPosition Helper class for sending LeanToPosition <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public LeanToPositionHelper LeanToPosition
		{
			get { return GetHelper<LeanToPositionHelper>("leanToPosition"); }
		}

		/// <summary>
		/// Gets a LeanTowardsObject Helper class for sending LeanTowardsObject <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public LeanTowardsObjectHelper LeanTowardsObject
		{
			get { return GetHelper<LeanTowardsObjectHelper>("leanTowardsObject"); }
		}

		/// <summary>
		/// Gets a HipsLeanInDirection Helper class for sending HipsLeanInDirection <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public HipsLeanInDirectionHelper HipsLeanInDirection
		{
			get { return GetHelper<HipsLeanInDirectionHelper>("hipsLeanInDirection"); }
		}

		/// <summary>
		/// Gets a HipsLeanRandom Helper class for sending HipsLeanRandom <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public HipsLeanRandomHelper HipsLeanRandom
		{
			get { return GetHelper<HipsLeanRandomHelper>("hipsLeanRandom"); }
		}

		/// <summary>
		/// Gets a HipsLeanToPosition Helper class for sending HipsLeanToPosition <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public HipsLeanToPositionHelper HipsLeanToPosition
		{
			get { return GetHelper<HipsLeanToPositionHelper>("hipsLeanToPosition"); }
		}

		/// <summary>
		/// Gets a HipsLeanTowardsObject Helper class for sending HipsLeanTowardsObject <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public HipsLeanTowardsObjectHelper HipsLeanTowardsObject
		{
			get { return GetHelper<HipsLeanTowardsObjectHelper>("hipsLeanTowardsObject"); }
		}

		/// <summary>
		/// Gets a ForceLeanInDirection Helper class for sending ForceLeanInDirection <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ForceLeanInDirectionHelper ForceLeanInDirection
		{
			get { return GetHelper<ForceLeanInDirectionHelper>("forceLeanInDirection"); }
		}

		/// <summary>
		/// Gets a ForceLeanRandom Helper class for sending ForceLeanRandom <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ForceLeanRandomHelper ForceLeanRandom
		{
			get { return GetHelper<ForceLeanRandomHelper>("forceLeanRandom"); }
		}

		/// <summary>
		/// Gets a ForceLeanToPosition Helper class for sending ForceLeanToPosition <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ForceLeanToPositionHelper ForceLeanToPosition
		{
			get { return GetHelper<ForceLeanToPositionHelper>("forceLeanToPosition"); }
		}

		/// <summary>
		/// Gets a ForceLeanTowardsObject Helper class for sending ForceLeanTowardsObject <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ForceLeanTowardsObjectHelper ForceLeanTowardsObject
		{
			get { return GetHelper<ForceLeanTowardsObjectHelper>("forceLeanTowardsObject"); }
		}

		/// <summary>
		/// Gets a SetStiffness Helper class for sending SetStiffness <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Use this message to manually set the body stiffness values -before using Active Pose to drive to an animated pose, for example.
		/// </remarks>
		public SetStiffnessHelper SetStiffness
		{
			get { return GetHelper<SetStiffnessHelper>("setStiffness"); }
		}

		/// <summary>
		/// Gets a SetMuscleStiffness Helper class for sending SetMuscleStiffness <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Use this message to manually set the muscle stiffness values -before using Active Pose to drive to an animated pose, for example.
		/// </remarks>
		public SetMuscleStiffnessHelper SetMuscleStiffness
		{
			get { return GetHelper<SetMuscleStiffnessHelper>("setMuscleStiffness"); }
		}

		/// <summary>
		/// Gets a SetWeaponMode Helper class for sending SetWeaponMode <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Use this message to set the character's weapon mode.  This is an alternativeto the setWeaponMode public function.
		/// </remarks>
		public SetWeaponModeHelper SetWeaponMode
		{
			get { return GetHelper<SetWeaponModeHelper>("setWeaponMode"); }
		}

		/// <summary>
		/// Gets a RegisterWeapon Helper class for sending RegisterWeapon <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Use this message to register weapon.  This is an alternativeto the registerWeapon public function.
		/// </remarks>
		public RegisterWeaponHelper RegisterWeapon
		{
			get { return GetHelper<RegisterWeaponHelper>("registerWeapon"); }
		}

		/// <summary>
		/// Gets a ShotRelax Helper class for sending ShotRelax <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ShotRelaxHelper ShotRelax
		{
			get { return GetHelper<ShotRelaxHelper>("shotRelax"); }
		}

		/// <summary>
		/// Gets a FireWeapon Helper class for sending FireWeapon <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// One shot message apply a force to the hand as we fire the gun that should be in this hand
		/// </remarks>
		public FireWeaponHelper FireWeapon
		{
			get { return GetHelper<FireWeaponHelper>("fireWeapon"); }
		}

		/// <summary>
		/// Gets a ConfigureConstraints Helper class for sending ConfigureConstraints <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// One shot to give state of constraints on character and response to constraints
		/// </remarks>
		public ConfigureConstraintsHelper ConfigureConstraints
		{
			get { return GetHelper<ConfigureConstraintsHelper>("configureConstraints"); }
		}

		/// <summary>
		/// Gets a StayUpright Helper class for sending StayUpright <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public StayUprightHelper StayUpright
		{
			get { return GetHelper<StayUprightHelper>("stayUpright"); }
		}

		/// <summary>
		/// Gets a StopAllBehaviours Helper class for sending StopAllBehaviours <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Send this message to immediately stop all behaviours from executing.
		/// </remarks>
		public StopAllBehavioursHelper StopAllBehaviours
		{
			get { return GetHelper<StopAllBehavioursHelper>("stopAllBehaviours"); }
		}

		/// <summary>
		/// Gets a SetCharacterStrength Helper class for sending SetCharacterStrength <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Sets character's strength on the dead-granny-to-healthy-terminator scale: [0..1]
		/// </remarks>
		public SetCharacterStrengthHelper SetCharacterStrength
		{
			get { return GetHelper<SetCharacterStrengthHelper>("setCharacterStrength"); }
		}

		/// <summary>
		/// Gets a SetCharacterHealth Helper class for sending SetCharacterHealth <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Sets character's health on the dead-to-alive scale: [0..1]
		/// </remarks>
		public SetCharacterHealthHelper SetCharacterHealth
		{
			get { return GetHelper<SetCharacterHealthHelper>("setCharacterHealth"); }
		}

		/// <summary>
		/// Gets a SetFallingReaction Helper class for sending SetFallingReaction <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Sets the type of reaction if catchFall is called
		/// </remarks>
		public SetFallingReactionHelper SetFallingReaction
		{
			get { return GetHelper<SetFallingReactionHelper>("setFallingReaction"); }
		}

		/// <summary>
		/// Gets a SetCharacterUnderwater Helper class for sending SetCharacterUnderwater <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Sets viscosity applied to damping limbs
		/// </remarks>
		public SetCharacterUnderwaterHelper SetCharacterUnderwater
		{
			get { return GetHelper<SetCharacterUnderwaterHelper>("setCharacterUnderwater"); }
		}

		/// <summary>
		/// Gets a SetCharacterCollisions Helper class for sending SetCharacterCollisions <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// setCharacterCollisions:
		/// </remarks>
		public SetCharacterCollisionsHelper SetCharacterCollisions
		{
			get { return GetHelper<SetCharacterCollisionsHelper>("setCharacterCollisions"); }
		}

		/// <summary>
		/// Gets a SetCharacterDamping Helper class for sending SetCharacterDamping <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Damp out cartwheeling and somersaulting above a certain threshold
		/// </remarks>
		public SetCharacterDampingHelper SetCharacterDamping
		{
			get { return GetHelper<SetCharacterDampingHelper>("setCharacterDamping"); }
		}

		/// <summary>
		/// Gets a SetFrictionScale Helper class for sending SetFrictionScale <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// setFrictionScale:
		/// </remarks>
		public SetFrictionScaleHelper SetFrictionScale
		{
			get { return GetHelper<SetFrictionScaleHelper>("setFrictionScale"); }
		}

		/// <summary>
		/// Gets a AnimPose Helper class for sending AnimPose <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public AnimPoseHelper AnimPose
		{
			get { return GetHelper<AnimPoseHelper>("animPose"); }
		}

		/// <summary>
		/// Gets a ArmsWindmill Helper class for sending ArmsWindmill <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ArmsWindmillHelper ArmsWindmill
		{
			get { return GetHelper<ArmsWindmillHelper>("armsWindmill"); }
		}

		/// <summary>
		/// Gets a ArmsWindmillAdaptive Helper class for sending ArmsWindmillAdaptive <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ArmsWindmillAdaptiveHelper ArmsWindmillAdaptive
		{
			get { return GetHelper<ArmsWindmillAdaptiveHelper>("armsWindmillAdaptive"); }
		}

		/// <summary>
		/// Gets a BalancerCollisionsReaction Helper class for sending BalancerCollisionsReaction <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public BalancerCollisionsReactionHelper BalancerCollisionsReaction
		{
			get { return GetHelper<BalancerCollisionsReactionHelper>("balancerCollisionsReaction"); }
		}

		/// <summary>
		/// Gets a BodyBalance Helper class for sending BodyBalance <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public BodyBalanceHelper BodyBalance
		{
			get { return GetHelper<BodyBalanceHelper>("bodyBalance"); }
		}

		/// <summary>
		/// Gets a BodyFoetal Helper class for sending BodyFoetal <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public BodyFoetalHelper BodyFoetal
		{
			get { return GetHelper<BodyFoetalHelper>("bodyFoetal"); }
		}

		/// <summary>
		/// Gets a BodyRollUp Helper class for sending BodyRollUp <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public BodyRollUpHelper BodyRollUp
		{
			get { return GetHelper<BodyRollUpHelper>("bodyRollUp"); }
		}

		/// <summary>
		/// Gets a BodyWrithe Helper class for sending BodyWrithe <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public BodyWritheHelper BodyWrithe
		{
			get { return GetHelper<BodyWritheHelper>("bodyWrithe"); }
		}

		/// <summary>
		/// Gets a BraceForImpact Helper class for sending BraceForImpact <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public BraceForImpactHelper BraceForImpact
		{
			get { return GetHelper<BraceForImpactHelper>("braceForImpact"); }
		}

		/// <summary>
		/// Gets a Buoyancy Helper class for sending Buoyancy <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Simple buoyancy model.  No character movement just fluid forces/torques added to parts.
		/// </remarks>
		public BuoyancyHelper Buoyancy
		{
			get { return GetHelper<BuoyancyHelper>("buoyancy"); }
		}

		/// <summary>
		/// Gets a CatchFall Helper class for sending CatchFall <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public CatchFallHelper CatchFall
		{
			get { return GetHelper<CatchFallHelper>("catchFall"); }
		}

		/// <summary>
		/// Gets a Electrocute Helper class for sending Electrocute <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ElectrocuteHelper Electrocute
		{
			get { return GetHelper<ElectrocuteHelper>("electrocute"); }
		}

		/// <summary>
		/// Gets a FallOverWall Helper class for sending FallOverWall <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public FallOverWallHelper FallOverWall
		{
			get { return GetHelper<FallOverWallHelper>("fallOverWall"); }
		}

		/// <summary>
		/// Gets a Grab Helper class for sending Grab <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public GrabHelper Grab
		{
			get { return GetHelper<GrabHelper>("grab"); }
		}

		/// <summary>
		/// Gets a HeadLook Helper class for sending HeadLook <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public HeadLookHelper HeadLook
		{
			get { return GetHelper<HeadLookHelper>("headLook"); }
		}

		/// <summary>
		/// Gets a HighFall Helper class for sending HighFall <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public HighFallHelper HighFall
		{
			get { return GetHelper<HighFallHelper>("highFall"); }
		}

		/// <summary>
		/// Gets a IncomingTransforms Helper class for sending IncomingTransforms <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public IncomingTransformsHelper IncomingTransforms
		{
			get { return GetHelper<IncomingTransformsHelper>("incomingTransforms"); }
		}

		/// <summary>
		/// Gets a InjuredOnGround Helper class for sending InjuredOnGround <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// InjuredOnGround
		/// </remarks>
		public InjuredOnGroundHelper InjuredOnGround
		{
			get { return GetHelper<InjuredOnGroundHelper>("injuredOnGround"); }
		}

		/// <summary>
		/// Gets a Carried Helper class for sending Carried <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Carried
		/// </remarks>
		public CarriedHelper Carried
		{
			get { return GetHelper<CarriedHelper>("carried"); }
		}

		/// <summary>
		/// Gets a Dangle Helper class for sending Dangle <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Dangle
		/// </remarks>
		public DangleHelper Dangle
		{
			get { return GetHelper<DangleHelper>("dangle"); }
		}

		/// <summary>
		/// Gets a OnFire Helper class for sending OnFire <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public OnFireHelper OnFire
		{
			get { return GetHelper<OnFireHelper>("onFire"); }
		}

		/// <summary>
		/// Gets a PedalLegs Helper class for sending PedalLegs <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public PedalLegsHelper PedalLegs
		{
			get { return GetHelper<PedalLegsHelper>("pedalLegs"); }
		}

		/// <summary>
		/// Gets a PointArm Helper class for sending PointArm <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// BEHAVIOURS REFERENCED: AnimPose - allows animPose to overridebodyParts: Arms (useLeftArm, useRightArm)
		/// </remarks>
		public PointArmHelper PointArm
		{
			get { return GetHelper<PointArmHelper>("pointArm"); }
		}

		/// <summary>
		/// Gets a PointGun Helper class for sending PointGun <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public PointGunHelper PointGun
		{
			get { return GetHelper<PointGunHelper>("pointGun"); }
		}

		/// <summary>
		/// Gets a PointGunExtra Helper class for sending PointGunExtra <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Seldom set parameters for pointGun - just to keep number of parameters in any message less than or equal to 64
		/// </remarks>
		public PointGunExtraHelper PointGunExtra
		{
			get { return GetHelper<PointGunExtraHelper>("pointGunExtra"); }
		}

		/// <summary>
		/// Gets a RollDownStairs Helper class for sending RollDownStairs <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public RollDownStairsHelper RollDownStairs
		{
			get { return GetHelper<RollDownStairsHelper>("rollDownStairs"); }
		}

		/// <summary>
		/// Gets a Shot Helper class for sending Shot <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ShotHelper Shot
		{
			get { return GetHelper<ShotHelper>("shot"); }
		}

		/// <summary>
		/// Gets a ShotNewBullet Helper class for sending ShotNewBullet <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Send new wound information to the shot.  Can cause shot to restart it's performance in part or in whole.
		/// </remarks>
		public ShotNewBulletHelper ShotNewBullet
		{
			get { return GetHelper<ShotNewBulletHelper>("shotNewBullet"); }
		}

		/// <summary>
		/// Gets a ShotSnap Helper class for sending ShotSnap <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ShotSnapHelper ShotSnap
		{
			get { return GetHelper<ShotSnapHelper>("shotSnap"); }
		}

		/// <summary>
		/// Gets a ShotShockSpin Helper class for sending ShotShockSpin <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// configure the shockSpin effect in shot.  Spin/Lift the character using cheat torques/forces
		/// </remarks>
		public ShotShockSpinHelper ShotShockSpin
		{
			get { return GetHelper<ShotShockSpinHelper>("shotShockSpin"); }
		}

		/// <summary>
		/// Gets a ShotFallToKnees Helper class for sending ShotFallToKnees <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// configure the fall to knees shot.
		/// </remarks>
		public ShotFallToKneesHelper ShotFallToKnees
		{
			get { return GetHelper<ShotFallToKneesHelper>("shotFallToKnees"); }
		}

		/// <summary>
		/// Gets a ShotFromBehind Helper class for sending ShotFromBehind <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// configure the shot from behind reaction
		/// </remarks>
		public ShotFromBehindHelper ShotFromBehind
		{
			get { return GetHelper<ShotFromBehindHelper>("shotFromBehind"); }
		}

		/// <summary>
		/// Gets a ShotInGuts Helper class for sending ShotInGuts <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// configure the shot in guts reaction
		/// </remarks>
		public ShotInGutsHelper ShotInGuts
		{
			get { return GetHelper<ShotInGutsHelper>("shotInGuts"); }
		}

		/// <summary>
		/// Gets a ShotHeadLook Helper class for sending ShotHeadLook <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public ShotHeadLookHelper ShotHeadLook
		{
			get { return GetHelper<ShotHeadLookHelper>("shotHeadLook"); }
		}

		/// <summary>
		/// Gets a ShotConfigureArms Helper class for sending ShotConfigureArms <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// configure the arm reactions in shot
		/// </remarks>
		public ShotConfigureArmsHelper ShotConfigureArms
		{
			get { return GetHelper<ShotConfigureArmsHelper>("shotConfigureArms"); }
		}

		/// <summary>
		/// Gets a SmartFall Helper class for sending SmartFall <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		/// <remarks>
		/// Clone of High Fall with a wider range of operating conditions.
		/// </remarks>
		public SmartFallHelper SmartFall
		{
			get { return GetHelper<SmartFallHelper>("smartFall"); }
		}

		/// <summary>
		/// Gets a StaggerFall Helper class for sending StaggerFall <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public StaggerFallHelper StaggerFall
		{
			get { return GetHelper<StaggerFallHelper>("staggerFall"); }
		}

		/// <summary>
		/// Gets a Teeter Helper class for sending Teeter <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public TeeterHelper Teeter
		{
			get { return GetHelper<TeeterHelper>("teeter"); }
		}

		/// <summary>
		/// Gets a UpperBodyFlinch Helper class for sending UpperBodyFlinch <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public UpperBodyFlinchHelper UpperBodyFlinch
		{
			get { return GetHelper<UpperBodyFlinchHelper>("upperBodyFlinch"); }
		}

		/// <summary>
		/// Gets a Yanked Helper class for sending Yanked <see cref="Message"/> to this <see cref="Ped"/>
		/// </summary>
		public YankedHelper Yanked
		{
			get { return GetHelper<YankedHelper>("yanked"); }
		}
	}
}
