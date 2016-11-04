using System;
using CitizenFX.Core;

namespace CitizenFX.Core.NaturalMotion
{
	public enum ArmDirection
	{
		Backwards = -1,
		Adaptive,
		Forwards
	}

	public enum AnimSource
	{
		CurrentItems,
		PreviousItems,
		AnimItems
	}

	public enum FallType
	{
		RampDownStiffness,
		DontChangeStep,
		ForceBalance,
		Slump
	}

	public enum Synchroisation
	{
		NotSynced,
		AlwaysSynced,
		SyncedAtStart
	}

	public enum TurnType
	{
		DontTurn,
		ToTarget,
		AwayFromTarget
	}

	public enum TorqueMode
	{
		Disabled,
		Proportional,
		Additive
	}

	public enum TorqueSpinMode
	{
		FromImpulse,
		Random,
		Flipping
	}

	public enum TorqueFilterMode
	{
		ApplyEveryBullet,
		ApplyIfLastFinished,
		ApplyIfSpinDifferent
	}

	public enum RbTwistAxis
	{
		WorldUp,
		CharacterComUp
	}

	public enum WeaponMode
	{
		None = -1,
		Pistol,
		Dual,
		Rifle,
		SideArm,
		PistolLeft,
		PistolRight
	}

	public enum Hand
	{
		Left,
		Right
	}

	public enum MirrorMode
	{
		Independant,
		Mirrored,
		Parallel
	}

	public enum AdaptiveMode
	{
		NotAdaptive,
		OnlyDirection,
		DirectionAndSpeed,
		DirectionSpeedAndStrength
	}

	public sealed class ActivePoseHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ActivePoseHelper for sending a ActivePose <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ActivePose <see cref="Message"/> to.</param>
		public ActivePoseHelper(Ped ped) : base(ped, "activePose")
		{
		}

		/// <summary>
		/// Sets the Mask setting for this <see cref="ActivePoseHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see notes for explanation).
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string Mask
		{
			set { SetArgument("mask", value); }
		}

		/// <summary>
		/// Sets the UseGravityCompensation setting for this <see cref="ActivePoseHelper"/>.
		/// Apply gravity compensation as well?.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseGravityCompensation
		{
			set { SetArgument("useGravityCompensation", value); }
		}

		/// <summary>
		/// Sets the AnimSource setting for this <see cref="ActivePoseHelper"/>.
		/// </summary>
		public AnimSource AnimSource
		{
			set { SetArgument("animSource", (int) value); }
		}
	}

	public sealed class ApplyImpulseHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ApplyImpulseHelper for sending a ApplyImpulse <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ApplyImpulse <see cref="Message"/> to.</param>
		public ApplyImpulseHelper(Ped ped) : base(ped, "applyImpulse")
		{
		}

		/// <summary>
		/// Sets the EqualizeAmount setting for this <see cref="ApplyImpulseHelper"/>.
		/// 0 means straight impulse, 1 means multiply by the mass (change in velocity).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float EqualizeAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("equalizeAmount", value);
			}
		}

		/// <summary>
		/// Sets the PartIndex setting for this <see cref="ApplyImpulseHelper"/>.
		/// index of part being hit. -1 apply impulse to COM.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = -1.
		/// Max value = 28.
		/// </remarks>
		public int PartIndex
		{
			set
			{
				if (value > 28)
					value = 28;
				if (value < -1)
					value = -1;
				SetArgument("partIndex", value);
			}
		}

		/// <summary>
		/// Sets the Impulse setting for this <see cref="ApplyImpulseHelper"/>.
		/// impulse vector (impulse is change in momentum).
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -4500.0f.
		/// Max value = 4500.0f.
		/// </remarks>
		public Vector3 Impulse
		{
			set
			{
				SetArgument("impulse",
					Vector3.Clamp(value, new Vector3(-4500.0f, -4500.0f, -4500.0f), new Vector3(4500.0f, 4500.0f, 4500.0f)));
			}
		}

		/// <summary>
		/// Sets the HitPoint setting for this <see cref="ApplyImpulseHelper"/>.
		/// optional point on part where hit.  If not supplied then the impulse is applied at the part centre.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 HitPoint
		{
			set { SetArgument("hitPoint", value); }
		}

		/// <summary>
		/// Sets the LocalHitPointInfo setting for this <see cref="ApplyImpulseHelper"/>.
		/// hitPoint in local coordinates of bodyPart.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool LocalHitPointInfo
		{
			set { SetArgument("localHitPointInfo", value); }
		}

		/// <summary>
		/// Sets the LocalImpulseInfo setting for this <see cref="ApplyImpulseHelper"/>.
		/// impulse in local coordinates of bodyPart.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool LocalImpulseInfo
		{
			set { SetArgument("localImpulseInfo", value); }
		}

		/// <summary>
		/// Sets the AngularImpulse setting for this <see cref="ApplyImpulseHelper"/>.
		/// impulse should be considered an angular impulse.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AngularImpulse
		{
			set { SetArgument("angularImpulse", value); }
		}
	}

	public sealed class ApplyBulletImpulseHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ApplyBulletImpulseHelper for sending a ApplyBulletImpulse <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ApplyBulletImpulse <see cref="Message"/> to.</param>
		public ApplyBulletImpulseHelper(Ped ped) : base(ped, "applyBulletImpulse")
		{
		}

		/// <summary>
		/// Sets the EqualizeAmount setting for this <see cref="ApplyBulletImpulseHelper"/>.
		/// 0 means straight impulse, 1 means multiply by the mass (change in velocity).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float EqualizeAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("equalizeAmount", value);
			}
		}

		/// <summary>
		/// Sets the PartIndex setting for this <see cref="ApplyBulletImpulseHelper"/>.
		/// index of part being hit.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 28.
		/// </remarks>
		public int PartIndex
		{
			set
			{
				if (value > 28)
					value = 28;
				if (value < 0)
					value = 0;
				SetArgument("partIndex", value);
			}
		}

		/// <summary>
		/// Sets the Impulse setting for this <see cref="ApplyBulletImpulseHelper"/>.
		/// impulse vector (impulse is change in momentum).
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -1000.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public Vector3 Impulse
		{
			set
			{
				SetArgument("impulse",
					Vector3.Clamp(value, new Vector3(-1000.0f, -1000.0f, -1000.0f), new Vector3(1000.0f, 1000.0f, 1000.0f)));
			}
		}

		/// <summary>
		/// Sets the HitPoint setting for this <see cref="ApplyBulletImpulseHelper"/>.
		/// optional point on part where hit.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 HitPoint
		{
			set { SetArgument("hitPoint", value); }
		}

		/// <summary>
		/// Sets the LocalHitPointInfo setting for this <see cref="ApplyBulletImpulseHelper"/>.
		/// true = hitPoint is in local coordinates of bodyPart, false = hitpoint is in world coordinates.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool LocalHitPointInfo
		{
			set { SetArgument("localHitPointInfo", value); }
		}

		/// <summary>
		/// Sets the ExtraShare setting for this <see cref="ApplyBulletImpulseHelper"/>.
		/// if not 0.0 then have an extra bullet applied to spine0 (approximates the COM).  Uses setup from configureBulletsExtra.  0-1 shared 0.0 = no extra bullet, 0.5 = impulse split equally between extra and bullet,  1.0 only extra bullet.  LT 0.0 then bullet + scaled extra bullet.  Eg.-0.5 = bullet + 0.5 impulse extra bullet.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -2.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ExtraShare
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -2.0f)
					value = -2.0f;
				SetArgument("extraShare", value);
			}
		}
	}

	/// <summary>
	/// Set the amount of relaxation across the whole body; Used to collapse the character into a rag-doll-like state.
	/// </summary>
	public sealed class BodyRelaxHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the BodyRelaxHelper for sending a BodyRelax <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the BodyRelax <see cref="Message"/> to.</param>
		/// <remarks>
		/// Set the amount of relaxation across the whole body; Used to collapse the character into a rag-doll-like state.
		/// </remarks>
		public BodyRelaxHelper(Ped ped) : base(ped, "bodyRelax")
		{
		}

		/// <summary>
		/// Sets the Relaxation setting for this <see cref="BodyRelaxHelper"/>.
		/// How relaxed the body becomes, in percentage relaxed. 100 being totally rag-dolled, 0 being very stiff and rigid.
		/// </summary>
		/// <remarks>
		/// Default value = 50.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float Relaxation
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("relaxation", value);
			}
		}

		/// <summary>
		/// Sets the Damping setting for this <see cref="BodyRelaxHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float Damping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("damping", value);
			}
		}

		/// <summary>
		/// Sets the Mask setting for this <see cref="BodyRelaxHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see Active Pose notes for possible values).
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string Mask
		{
			set { SetArgument("mask", value); }
		}

		/// <summary>
		/// Sets the HoldPose setting for this <see cref="BodyRelaxHelper"/>.
		/// automatically hold the current pose as the character relaxes - can be used to avoid relaxing into a t-pose.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool HoldPose
		{
			set { SetArgument("holdPose", value); }
		}

		/// <summary>
		/// Sets the DisableJointDriving setting for this <see cref="BodyRelaxHelper"/>.
		/// sets the drive state to free - this reduces drifting on the ground.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool DisableJointDriving
		{
			set { SetArgument("disableJointDriving", value); }
		}
	}

	/// <summary>
	/// This single message allows you to configure various parameters used on any behaviour that uses the dynamic balance.
	/// </summary>
	public sealed class ConfigureBalanceHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ConfigureBalanceHelper for sending a ConfigureBalance <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ConfigureBalance <see cref="Message"/> to.</param>
		/// <remarks>
		/// This single message allows you to configure various parameters used on any behaviour that uses the dynamic balance.
		/// </remarks>
		public ConfigureBalanceHelper(Ped ped) : base(ped, "configureBalance")
		{
		}

		/// <summary>
		/// Sets the StepHeight setting for this <see cref="ConfigureBalanceHelper"/>.
		/// maximum height that character steps vertically (above 0.2 is high...but ok for say underwater).
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 0.4f.
		/// </remarks>
		public float StepHeight
		{
			set
			{
				if (value > 0.4f)
					value = 0.4f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stepHeight", value);
			}
		}

		/// <summary>
		/// Sets the StepHeightInc4Step setting for this <see cref="ConfigureBalanceHelper"/>.
		/// added to stepHeight if going up steps.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 0.4f.
		/// </remarks>
		public float StepHeightInc4Step
		{
			set
			{
				if (value > 0.4f)
					value = 0.4f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stepHeightInc4Step", value);
			}
		}

		/// <summary>
		/// Sets the LegsApartRestep setting for this <see cref="ConfigureBalanceHelper"/>.
		/// if the legs end up more than (legsApartRestep + hipwidth) apart even though balanced, take another step.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LegsApartRestep
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legsApartRestep", value);
			}
		}

		/// <summary>
		/// Sets the LegsTogetherRestep setting for this <see cref="ConfigureBalanceHelper"/>.
		/// mmmm0.1 for drunk if the legs end up less than (hipwidth - legsTogetherRestep) apart even though balanced, take another step.  A value of 1 will turn off this feature and the max value is hipWidth = 0.23f by default but is model dependent.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LegsTogetherRestep
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legsTogetherRestep", value);
			}
		}

		/// <summary>
		/// Sets the LegsApartMax setting for this <see cref="ConfigureBalanceHelper"/>.
		/// FRICTION WORKAROUND: if the legs end up more than (legsApartMax + hipwidth) apart when balanced, adjust the feet positions to slide back so they are legsApartMax + hipwidth apart.  Needs to be less than legsApartRestep to see any effect.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float LegsApartMax
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legsApartMax", value);
			}
		}

		/// <summary>
		/// Sets the TaperKneeStrength setting for this <see cref="ConfigureBalanceHelper"/>.
		/// does the knee strength reduce with angle.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool TaperKneeStrength
		{
			set { SetArgument("taperKneeStrength", value); }
		}

		/// <summary>
		/// Sets the LegStiffness setting for this <see cref="ConfigureBalanceHelper"/>.
		/// stiffness of legs.
		/// </summary>
		/// <remarks>
		/// Default value = 12.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float LegStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("legStiffness", value);
			}
		}

		/// <summary>
		/// Sets the LeftLegSwingDamping setting for this <see cref="ConfigureBalanceHelper"/>.
		/// damping of left leg during swing phase (mmmmDrunk used 1.25 to slow legs movement).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.2f.
		/// Max value = 4.0f.
		/// </remarks>
		public float LeftLegSwingDamping
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.2f)
					value = 0.2f;
				SetArgument("leftLegSwingDamping", value);
			}
		}

		/// <summary>
		/// Sets the RightLegSwingDamping setting for this <see cref="ConfigureBalanceHelper"/>.
		/// damping of right leg during swing phase (mmmmDrunk used 1.25 to slow legs movement).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.2f.
		/// Max value = 4.0f.
		/// </remarks>
		public float RightLegSwingDamping
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.2f)
					value = 0.2f;
				SetArgument("rightLegSwingDamping", value);
			}
		}

		/// <summary>
		/// Sets the OpposeGravityLegs setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Gravity opposition applied to hips and knees.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float OpposeGravityLegs
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("opposeGravityLegs", value);
			}
		}

		/// <summary>
		/// Sets the OpposeGravityAnkles setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Gravity opposition applied to ankles.  General balancer likes 1.0.  StaggerFall likes 0.1.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float OpposeGravityAnkles
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("opposeGravityAnkles", value);
			}
		}

		/// <summary>
		/// Sets the LeanAcc setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Multiplier on the floorAcceleration added to the lean.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAcc
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanAcc", value);
			}
		}

		/// <summary>
		/// Sets the HipLeanAcc setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Multiplier on the floorAcceleration added to the leanHips.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float HipLeanAcc
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("hipLeanAcc", value);
			}
		}

		/// <summary>
		/// Sets the LeanAccMax setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Max floorAcceleration allowed for lean and leanHips.
		/// </summary>
		/// <remarks>
		/// Default value = 5.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float LeanAccMax
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanAccMax", value);
			}
		}

		/// <summary>
		/// Sets the ResistAcc setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Level of cheat force added to character to resist the effect of floorAcceleration (anti-Acceleration) - added to upperbody.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ResistAcc
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("resistAcc", value);
			}
		}

		/// <summary>
		/// Sets the ResistAccMax setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Max floorAcceleration allowed for anti-Acceleration. If  GT 20.0 then it is probably in a crash.
		/// </summary>
		/// <remarks>
		/// Default value = 3.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float ResistAccMax
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("resistAccMax", value);
			}
		}

		/// <summary>
		/// Sets the FootSlipCompOnMovingFloor setting for this <see cref="ConfigureBalanceHelper"/>.
		/// This parameter will be removed when footSlipCompensation preserves the foot angle on a moving floor]. If the character detects a moving floor and footSlipCompOnMovingFloor is false then it will turn off footSlipCompensation - at footSlipCompensation preserves the global heading of the feet.  If footSlipCompensation is off then the character usually turns to the side in the end although when turning the vehicle turns it looks promising for a while.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool FootSlipCompOnMovingFloor
		{
			set { SetArgument("footSlipCompOnMovingFloor", value); }
		}

		/// <summary>
		/// Sets the AnkleEquilibrium setting for this <see cref="ConfigureBalanceHelper"/>.
		/// ankle equilibrium angle used when static balancing.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float AnkleEquilibrium
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("ankleEquilibrium", value);
			}
		}

		/// <summary>
		/// Sets the ExtraFeetApart setting for this <see cref="ConfigureBalanceHelper"/>.
		/// additional feet apart setting.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ExtraFeetApart
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("extraFeetApart", value);
			}
		}

		/// <summary>
		/// Sets the DontStepTime setting for this <see cref="ConfigureBalanceHelper"/>.
		/// amount of time at the start of a balance before the character is allowed to start stepping.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float DontStepTime
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dontStepTime", value);
			}
		}

		/// <summary>
		/// Sets the BalanceAbortThreshold setting for this <see cref="ConfigureBalanceHelper"/>.
		/// when the character gives up and goes into a fall.  Larger values mean that the balancer can lean more before failing.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float BalanceAbortThreshold
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("balanceAbortThreshold", value);
			}
		}

		/// <summary>
		/// Sets the GiveUpHeight setting for this <see cref="ConfigureBalanceHelper"/>.
		/// height between lowest foot and COM below which balancer will give up.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.5f.
		/// </remarks>
		public float GiveUpHeight
		{
			set
			{
				if (value > 1.5f)
					value = 1.5f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("giveUpHeight", value);
			}
		}

		/// <summary>
		/// Sets the StepClampScale setting for this <see cref="ConfigureBalanceHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float StepClampScale
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stepClampScale", value);
			}
		}

		/// <summary>
		/// Sets the StepClampScaleVariance setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Variance in clamp scale every step. if negative only takes away from clampScale.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float StepClampScaleVariance
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("stepClampScaleVariance", value);
			}
		}

		/// <summary>
		/// Sets the PredictionTimeHip setting for this <see cref="ConfigureBalanceHelper"/>.
		/// amount of time (seconds) into the future that the character tries to move hip to (kind of).  Will be controlled by balancer in future but can help recover spine quicker from bending forwards to much.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float PredictionTimeHip
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("predictionTimeHip", value);
			}
		}

		/// <summary>
		/// Sets the PredictionTime setting for this <see cref="ConfigureBalanceHelper"/>.
		/// amount of time (seconds) into the future that the character tries to step to. bigger values try to recover with fewer, bigger steps. smaller values recover with smaller steps, and generally recover less.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float PredictionTime
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("predictionTime", value);
			}
		}

		/// <summary>
		/// Sets the PredictionTimeVariance setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Variance in predictionTime every step. if negative only takes away from predictionTime.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float PredictionTimeVariance
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("predictionTimeVariance", value);
			}
		}

		/// <summary>
		/// Sets the MaxSteps setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Maximum number of steps that the balancer will take.
		/// </summary>
		/// <remarks>
		/// Default value = 100.
		/// Min value = 1.
		/// </remarks>
		public int MaxSteps
		{
			set
			{
				if (value < 1)
					value = 1;
				SetArgument("maxSteps", value);
			}
		}

		/// <summary>
		/// Sets the MaxBalanceTime setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Maximum time(seconds) that the balancer will balance for.
		/// </summary>
		/// <remarks>
		/// Default value = 50.0f.
		/// Min value = 1.0f.
		/// </remarks>
		public float MaxBalanceTime
		{
			set
			{
				if (value < 1.0f)
					value = 1.0f;
				SetArgument("maxBalanceTime", value);
			}
		}

		/// <summary>
		/// Sets the ExtraSteps setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Allow the balancer to take this many more steps before hitting maxSteps. If negative nothing happens(safe default).
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int ExtraSteps
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("extraSteps", value);
			}
		}

		/// <summary>
		/// Sets the ExtraTime setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Allow the balancer to balance for this many more seconds before hitting maxBalanceTime.  If negative nothing happens(safe default).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// </remarks>
		public float ExtraTime
		{
			set
			{
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("extraTime", value);
			}
		}

		/// <summary>
		/// Sets the FallType setting for this <see cref="ConfigureBalanceHelper"/>.
		/// How to fall after maxSteps or maxBalanceTime.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="FallType.RampDownStiffness"/>.
		/// If <see cref="FallType.Slump"/> BCR has to be active.
		/// </remarks>
		public FallType FallType
		{
			set { SetArgument("fallType", (int) value); }
		}

		/// <summary>
		/// Sets the FallMult setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Multiply the rampDown of stiffness on falling by this amount ( GT 1 fall quicker).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float FallMult
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("fallMult", value);
			}
		}

		/// <summary>
		/// Sets the FallReduceGravityComp setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Reduce gravity compensation as the legs weaken on falling.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FallReduceGravityComp
		{
			set { SetArgument("fallReduceGravityComp", value); }
		}

		/// <summary>
		/// Sets the RampHipPitchOnFail setting for this <see cref="ConfigureBalanceHelper"/>.
		/// bend over when falling after maxBalanceTime.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool RampHipPitchOnFail
		{
			set { SetArgument("rampHipPitchOnFail", value); }
		}

		/// <summary>
		/// Sets the StableLinSpeedThresh setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Linear speed threshold for successful balance.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float StableLinSpeedThresh
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stableLinSpeedThresh", value);
			}
		}

		/// <summary>
		/// Sets the StableRotSpeedThresh setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Rotational speed threshold for successful balance.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float StableRotSpeedThresh
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stableRotSpeedThresh", value);
			}
		}

		/// <summary>
		/// Sets the FailMustCollide setting for this <see cref="ConfigureBalanceHelper"/>.
		/// The upper body of the character must be colliding and other failure conditions met to fail.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FailMustCollide
		{
			set { SetArgument("failMustCollide", value); }
		}

		/// <summary>
		/// Sets the IgnoreFailure setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Ignore maxSteps and maxBalanceTime and try to balance forever.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool IgnoreFailure
		{
			set { SetArgument("ignoreFailure", value); }
		}

		/// <summary>
		/// Sets the ChangeStepTime setting for this <see cref="ConfigureBalanceHelper"/>.
		/// time not in contact (airborne) before step is changed. If -ve don't change step.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float ChangeStepTime
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("changeStepTime", value);
			}
		}

		/// <summary>
		/// Sets the BalanceIndefinitely setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Ignore maxSteps and maxBalanceTime and try to balance forever.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool BalanceIndefinitely
		{
			set { SetArgument("balanceIndefinitely", value); }
		}

		/// <summary>
		/// Sets the MovingFloor setting for this <see cref="ConfigureBalanceHelper"/>.
		/// temporary variable to ignore movingFloor code that generally causes the character to fall over if the feet probe a moving object e.g. treading on a gun.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool MovingFloor
		{
			set { SetArgument("movingFloor", value); }
		}

		/// <summary>
		/// Sets the AirborneStep setting for this <see cref="ConfigureBalanceHelper"/>.
		/// when airborne try to step.  Set to false for e.g. shotGun reaction.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool AirborneStep
		{
			set { SetArgument("airborneStep", value); }
		}

		/// <summary>
		/// Sets the UseComDirTurnVelThresh setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Velocity below which the balancer turns in the direction of the COM forward instead of the ComVel - for use with shot from running with high upright constraint use 1.9.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float UseComDirTurnVelThresh
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("useComDirTurnVelThresh", value);
			}
		}

		/// <summary>
		/// Sets the MinKneeAngle setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Minimum knee angle (-ve value will mean this functionality is not applied).  0.4 seems a good value.
		/// </summary>
		/// <remarks>
		/// Default value = -0.5f.
		/// Min value = -0.5f.
		/// Max value = 1.5f.
		/// </remarks>
		public float MinKneeAngle
		{
			set
			{
				if (value > 1.5f)
					value = 1.5f;
				if (value < -0.5f)
					value = -0.5f;
				SetArgument("minKneeAngle", value);
			}
		}

		/// <summary>
		/// Sets the FlatterSwingFeet setting for this <see cref="ConfigureBalanceHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FlatterSwingFeet
		{
			set { SetArgument("flatterSwingFeet", value); }
		}

		/// <summary>
		/// Sets the FlatterStaticFeet setting for this <see cref="ConfigureBalanceHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FlatterStaticFeet
		{
			set { SetArgument("flatterStaticFeet", value); }
		}

		/// <summary>
		/// Sets the AvoidLeg setting for this <see cref="ConfigureBalanceHelper"/>.
		/// If true then balancer tries to avoid leg2leg collisions/avoid crossing legs. Avoid tries to not step across a line of the inside of the stance leg's foot.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AvoidLeg
		{
			set { SetArgument("avoidLeg", value); }
		}

		/// <summary>
		/// Sets the AvoidFootWidth setting for this <see cref="ConfigureBalanceHelper"/>.
		/// NB. Very sensitive. Avoid tries to not step across a line of the inside of the stance leg's foot. avoidFootWidth = how much inwards from the ankle this line is in (m).
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float AvoidFootWidth
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("avoidFootWidth", value);
			}
		}

		/// <summary>
		/// Sets the AvoidFeedback setting for this <see cref="ConfigureBalanceHelper"/>.
		/// NB. Very sensitive. Avoid tries to not step across a line of the inside of the stance leg's foot. Avoid doesn't allow the desired stepping foot to cross the line.  avoidFeedback = how much of the actual crossing of that line is fedback as an error.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float AvoidFeedback
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("avoidFeedback", value);
			}
		}

		/// <summary>
		/// Sets the LeanAgainstVelocity setting for this <see cref="ConfigureBalanceHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAgainstVelocity
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanAgainstVelocity", value);
			}
		}

		/// <summary>
		/// Sets the StepDecisionThreshold setting for this <see cref="ConfigureBalanceHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float StepDecisionThreshold
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stepDecisionThreshold", value);
			}
		}

		/// <summary>
		/// Sets the StepIfInSupport setting for this <see cref="ConfigureBalanceHelper"/>.
		/// The balancer sometimes decides to step even if balanced.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool StepIfInSupport
		{
			set { SetArgument("stepIfInSupport", value); }
		}

		/// <summary>
		/// Sets the AlwaysStepWithFarthest setting for this <see cref="ConfigureBalanceHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AlwaysStepWithFarthest
		{
			set { SetArgument("alwaysStepWithFarthest", value); }
		}

		/// <summary>
		/// Sets the StandUp setting for this <see cref="ConfigureBalanceHelper"/>.
		/// standup more with increased velocity.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool StandUp
		{
			set { SetArgument("standUp", value); }
		}

		/// <summary>
		/// Sets the DepthFudge setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Supposed to increase foot friction: Impact depth of a collision with the foot is changed when the balancer is running - impact.SetDepth(impact.GetDepth() - depthFudge).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float DepthFudge
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("depthFudge", value);
			}
		}

		/// <summary>
		/// Sets the DepthFudgeStagger setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Supposed to increase foot friction: Impact depth of a collision with the foot is changed when staggerFall is running - impact.SetDepth(impact.GetDepth() - depthFudgeStagger).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float DepthFudgeStagger
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("depthFudgeStagger", value);
			}
		}

		/// <summary>
		/// Sets the FootFriction setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Foot friction multiplier is multiplied by this amount if balancer is running.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 40.0f.
		/// </remarks>
		public float FootFriction
		{
			set
			{
				if (value > 40.0f)
					value = 40.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("footFriction", value);
			}
		}

		/// <summary>
		/// Sets the FootFrictionStagger setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Foot friction multiplier is multiplied by this amount if staggerFall is running.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 40.0f.
		/// </remarks>
		public float FootFrictionStagger
		{
			set
			{
				if (value > 40.0f)
					value = 40.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("footFrictionStagger", value);
			}
		}

		/// <summary>
		/// Sets the BackwardsLeanCutoff setting for this <see cref="ConfigureBalanceHelper"/>.
		/// Backwards lean threshold to cut off stay upright forces. 0.0 Vertical - 1.0 horizontal.  0.6 is a sensible value.  NB: the balancer does not fail in order to give stagger that extra step as it falls.  A backwards lean of GT 0.6 will generally mean the balancer will soon fail without stayUpright forces.
		/// </summary>
		/// <remarks>
		/// Default value = 1.1f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float BackwardsLeanCutoff
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("backwardsLeanCutoff", value);
			}
		}

		/// <summary>
		/// Sets the GiveUpHeightEnd setting for this <see cref="ConfigureBalanceHelper"/>.
		/// if this value is different from giveUpHeight, actual giveUpHeight will be ramped toward this value.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.5f.
		/// </remarks>
		public float GiveUpHeightEnd
		{
			set
			{
				if (value > 1.5f)
					value = 1.5f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("giveUpHeightEnd", value);
			}
		}

		/// <summary>
		/// Sets the BalanceAbortThresholdEnd setting for this <see cref="ConfigureBalanceHelper"/>.
		/// if this value is different from balanceAbortThreshold, actual balanceAbortThreshold will be ramped toward this value.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float BalanceAbortThresholdEnd
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("balanceAbortThresholdEnd", value);
			}
		}

		/// <summary>
		/// Sets the GiveUpRampDuration setting for this <see cref="ConfigureBalanceHelper"/>.
		/// duration of ramp from start of behaviour for above two parameters. If smaller than 0, no ramp is applied.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GiveUpRampDuration
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("giveUpRampDuration", value);
			}
		}

		/// <summary>
		/// Sets the LeanToAbort setting for this <see cref="ConfigureBalanceHelper"/>.
		/// lean at which to send abort message when maxSteps or maxBalanceTime is reached.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanToAbort
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanToAbort", value);
			}
		}
	}

	/// <summary>
	/// reset the values configurable by the Configure Balance message to their defaults.
	/// </summary>
	public sealed class ConfigureBalanceResetHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ConfigureBalanceResetHelper for sending a ConfigureBalanceReset <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ConfigureBalanceReset <see cref="Message"/> to.</param>
		/// <remarks>
		/// reset the values configurable by the Configure Balance message to their defaults.
		/// </remarks>
		public ConfigureBalanceResetHelper(Ped ped) : base(ped, "configureBalanceReset")
		{
		}
	}

	/// <summary>
	/// this single message allows to configure self avoidance for the character.BBDD Self avoidance tech.
	/// </summary>
	public sealed class ConfigureSelfAvoidanceHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ConfigureSelfAvoidanceHelper for sending a ConfigureSelfAvoidance <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ConfigureSelfAvoidance <see cref="Message"/> to.</param>
		/// <remarks>
		/// this single message allows to configure self avoidance for the character.BBDD Self avoidance tech.
		/// </remarks>
		public ConfigureSelfAvoidanceHelper(Ped ped) : base(ped, "configureSelfAvoidance")
		{
		}

		/// <summary>
		/// Sets the UseSelfAvoidance setting for this <see cref="ConfigureSelfAvoidanceHelper"/>.
		/// Enable or disable self avoidance tech.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseSelfAvoidance
		{
			set { SetArgument("useSelfAvoidance", value); }
		}

		/// <summary>
		/// Sets the OverwriteDragReduction setting for this <see cref="ConfigureSelfAvoidanceHelper"/>.
		/// Specify whether self avoidance tech should use original IK input target or the target that has been already modified by getStabilisedPos() tech i.e. function that compensates for rotational and linear velocity of shoulder/thigh.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool OverwriteDragReduction
		{
			set { SetArgument("overwriteDragReduction", value); }
		}

		/// <summary>
		/// Sets the TorsoSwingFraction setting for this <see cref="ConfigureSelfAvoidanceHelper"/>.
		/// Place the adjusted target this much along the arc between effector (wrist) and target, value in range [0,1].
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TorsoSwingFraction
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torsoSwingFraction", value);
			}
		}

		/// <summary>
		/// Sets the MaxTorsoSwingAngleRad setting for this <see cref="ConfigureSelfAvoidanceHelper"/>.
		/// Max value on the effector (wrist) to adjusted target offset.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 1.6f.
		/// </remarks>
		public float MaxTorsoSwingAngleRad
		{
			set
			{
				if (value > 1.6f)
					value = 1.6f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxTorsoSwingAngleRad", value);
			}
		}

		/// <summary>
		/// Sets the SelfAvoidIfInSpineBoundsOnly setting for this <see cref="ConfigureSelfAvoidanceHelper"/>.
		/// Restrict self avoidance to operate on targets that are within character torso bounds only.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SelfAvoidIfInSpineBoundsOnly
		{
			set { SetArgument("selfAvoidIfInSpineBoundsOnly", value); }
		}

		/// <summary>
		/// Sets the SelfAvoidAmount setting for this <see cref="ConfigureSelfAvoidanceHelper"/>.
		/// Amount of self avoidance offset applied when angle from effector (wrist) to target is greater then right angle i.e. when total offset is a blend between where effector currently is to value that is a product of total arm length and selfAvoidAmount. SelfAvoidAmount is in a range between [0, 1].
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SelfAvoidAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("selfAvoidAmount", value);
			}
		}

		/// <summary>
		/// Sets the OverwriteTwist setting for this <see cref="ConfigureSelfAvoidanceHelper"/>.
		/// Overwrite desired IK twist with self avoidance procedural twist.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool OverwriteTwist
		{
			set { SetArgument("overwriteTwist", value); }
		}

		/// <summary>
		/// Sets the UsePolarPathAlgorithm setting for this <see cref="ConfigureSelfAvoidanceHelper"/>.
		/// Use the alternative self avoidance algorithm that is based on linear and polar target blending. WARNING: It only requires "radius" in terms of parametrization.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UsePolarPathAlgorithm
		{
			set { SetArgument("usePolarPathAlgorithm", value); }
		}

		/// <summary>
		/// Sets the Radius setting for this <see cref="ConfigureSelfAvoidanceHelper"/>.
		/// Self avoidance radius, measured out from the spine axis along the plane perpendicular to that axis. The closer is the proximity of reaching target to that radius, the more polar (curved) motion is used for offsetting the target. WARNING: Parameter only used by the alternative algorithm that is based on linear and polar target blending.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Radius
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("radius", value);
			}
		}
	}

	public sealed class ConfigureBulletsHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ConfigureBulletsHelper for sending a ConfigureBullets <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ConfigureBullets <see cref="Message"/> to.</param>
		public ConfigureBulletsHelper(Ped ped) : base(ped, "configureBullets")
		{
		}

		/// <summary>
		/// Sets the ImpulseSpreadOverParts setting for this <see cref="ConfigureBulletsHelper"/>.
		/// spreads impulse across parts. currently only for spine parts, not limbs.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ImpulseSpreadOverParts
		{
			set { SetArgument("impulseSpreadOverParts", value); }
		}

		/// <summary>
		/// Sets the ImpulseLeakageStrengthScaled setting for this <see cref="ConfigureBulletsHelper"/>.
		/// for weaker characters subsequent impulses remain strong.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ImpulseLeakageStrengthScaled
		{
			set { SetArgument("impulseLeakageStrengthScaled", value); }
		}

		/// <summary>
		/// Sets the ImpulsePeriod setting for this <see cref="ConfigureBulletsHelper"/>.
		/// duration that impulse is spread over (triangular shaped).
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulsePeriod
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulsePeriod", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseTorqueScale setting for this <see cref="ConfigureBulletsHelper"/>.
		/// An impulse applied at a point on a body equivalent to an impulse at the centre of the body and a torque.  This parameter scales the torque component. (The torque component seems to be excite the rage looseness bug which sends the character in a sometimes wildly different direction to an applied impulse).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseTorqueScale
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseTorqueScale", value);
			}
		}

		/// <summary>
		/// Sets the LoosenessFix setting for this <see cref="ConfigureBulletsHelper"/>.
		/// Fix the rage looseness bug by applying only the impulse at the centre of the body unless it is a spine part then apply the twist component only of the torque as well.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool LoosenessFix
		{
			set { SetArgument("loosenessFix", value); }
		}

		/// <summary>
		/// Sets the ImpulseDelay setting for this <see cref="ConfigureBulletsHelper"/>.
		/// time from hit before impulses are being applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseDelay
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseDelay", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseReductionPerShot setting for this <see cref="ConfigureBulletsHelper"/>.
		/// by how much are subsequent impulses reduced (e.g. 0.0: no reduction, 0.1: 10% reduction each new hit).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseReductionPerShot
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseReductionPerShot", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseRecovery setting for this <see cref="ConfigureBulletsHelper"/>.
		/// recovery rate of impulse strength per second (impulse strength from 0.0:1.0).  At 60fps a impulseRecovery=60.0 will recover in 1 frame.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 60.0f.
		/// </remarks>
		public float ImpulseRecovery
		{
			set
			{
				if (value > 60.0f)
					value = 60.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseRecovery", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseMinLeakage setting for this <see cref="ConfigureBulletsHelper"/>.
		/// the minimum amount of impulse leakage allowed.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseMinLeakage
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseMinLeakage", value);
			}
		}

		/// <summary>
		/// Sets the TorqueMode setting for this <see cref="ConfigureBulletsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="TorqueMode.Disabled"/>.
		/// If <see cref="TorqueMode.Proportional"/> - proportional to character strength, can reduce impulse amount.
		/// If <see cref="TorqueMode.Additive"/> - no reduction of impulse and not proportional to character strength.
		/// </remarks>
		public TorqueMode TorqueMode
		{
			set { SetArgument("torqueMode", (int) value); }
		}

		/// <summary>
		/// Sets the TorqueSpinMode setting for this <see cref="ConfigureBulletsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="TorqueSpinMode.FromImpulse"/>.
		/// If <see cref="TorqueSpinMode.Flipping"/> a burst effect is achieved.
		/// </remarks>
		public TorqueSpinMode TorqueSpinMode
		{
			set { SetArgument("torqueSpinMode", (int) value); }
		}

		/// <summary>
		/// Sets the TorqueFilterMode setting for this <see cref="ConfigureBulletsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="TorqueFilterMode.ApplyEveryBullet"/>.
		/// </remarks>
		public TorqueFilterMode TorqueFilterMode
		{
			set { SetArgument("torqueFilterMode", (int) value); }
		}

		/// <summary>
		/// Sets the TorqueAlwaysSpine3 setting for this <see cref="ConfigureBulletsHelper"/>.
		/// always apply torques to spine3 instead of actual part hit.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool TorqueAlwaysSpine3
		{
			set { SetArgument("torqueAlwaysSpine3", value); }
		}

		/// <summary>
		/// Sets the TorqueDelay setting for this <see cref="ConfigureBulletsHelper"/>.
		/// time from hit before torques are being applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TorqueDelay
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torqueDelay", value);
			}
		}

		/// <summary>
		/// Sets the TorquePeriod setting for this <see cref="ConfigureBulletsHelper"/>.
		/// duration of torque.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TorquePeriod
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torquePeriod", value);
			}
		}

		/// <summary>
		/// Sets the TorqueGain setting for this <see cref="ConfigureBulletsHelper"/>.
		/// multiplies impulse magnitude to arrive at torque that is applied.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float TorqueGain
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torqueGain", value);
			}
		}

		/// <summary>
		/// Sets the TorqueCutoff setting for this <see cref="ConfigureBulletsHelper"/>.
		/// minimum ratio of impulse that remains after converting to torque (if in strength-proportional mode).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TorqueCutoff
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torqueCutoff", value);
			}
		}

		/// <summary>
		/// Sets the TorqueReductionPerTick setting for this <see cref="ConfigureBulletsHelper"/>.
		/// ratio of torque for next tick (e.g. 1.0: not reducing over time, 0.9: each tick torque is reduced by 10%).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TorqueReductionPerTick
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torqueReductionPerTick", value);
			}
		}

		/// <summary>
		/// Sets the LiftGain setting for this <see cref="ConfigureBulletsHelper"/>.
		/// amount of lift (directly multiplies torque axis to give lift force).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LiftGain
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("liftGain", value);
			}
		}

		/// <summary>
		/// Sets the CounterImpulseDelay setting for this <see cref="ConfigureBulletsHelper"/>.
		/// time after impulse is applied that counter impulse is applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CounterImpulseDelay
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("counterImpulseDelay", value);
			}
		}

		/// <summary>
		/// Sets the CounterImpulseMag setting for this <see cref="ConfigureBulletsHelper"/>.
		/// amount of the original impulse that is countered.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CounterImpulseMag
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("counterImpulseMag", value);
			}
		}

		/// <summary>
		/// Sets the CounterAfterMagReached setting for this <see cref="ConfigureBulletsHelper"/>.
		/// applies the counter impulse counterImpulseDelay(secs) after counterImpulseMag of the Impulse has been applied.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool CounterAfterMagReached
		{
			set { SetArgument("counterAfterMagReached", value); }
		}

		/// <summary>
		/// Sets the DoCounterImpulse setting for this <see cref="ConfigureBulletsHelper"/>.
		/// add a counter impulse to the pelvis.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool DoCounterImpulse
		{
			set { SetArgument("doCounterImpulse", value); }
		}

		/// <summary>
		/// Sets the CounterImpulse2Hips setting for this <see cref="ConfigureBulletsHelper"/>.
		/// amount of the counter impulse applied to hips - the rest is applied to the part originally hit.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CounterImpulse2Hips
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("counterImpulse2Hips", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseNoBalMult setting for this <see cref="ConfigureBulletsHelper"/>.
		/// amount to scale impulse by if the dynamicBalance is not OK.  1.0 means this functionality is not applied.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseNoBalMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseNoBalMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseBalStabStart setting for this <see cref="ConfigureBulletsHelper"/>.
		/// 100% LE Start to impulseBalStabMult*100% GT End. NB: Start LT End.
		/// </summary>
		/// <remarks>
		/// Default value = 3.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float ImpulseBalStabStart
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseBalStabStart", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseBalStabEnd setting for this <see cref="ConfigureBulletsHelper"/>.
		/// 100% LE Start to impulseBalStabMult*100% GT End. NB: Start LT End.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float ImpulseBalStabEnd
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseBalStabEnd", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseBalStabMult setting for this <see cref="ConfigureBulletsHelper"/>.
		/// 100% LE Start to impulseBalStabMult*100% GT End. NB: leaving this as 1.0 means this functionality is not applied and Start and End have no effect.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseBalStabMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseBalStabMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseSpineAngStart setting for this <see cref="ConfigureBulletsHelper"/>.
		/// 100% GE Start to impulseSpineAngMult*100% LT End. NB: Start GT End.  This the dot of hip2Head with up.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseSpineAngStart
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("impulseSpineAngStart", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseSpineAngEnd setting for this <see cref="ConfigureBulletsHelper"/>.
		/// 100% GE Start to impulseSpineAngMult*100% LT End. NB: Start GT End.  This the dot of hip2Head with up.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseSpineAngEnd
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("impulseSpineAngEnd", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseSpineAngMult setting for this <see cref="ConfigureBulletsHelper"/>.
		/// 100% GE Start to impulseSpineAngMult*100% LT End. NB: leaving this as 1.0 means this functionality is not applied and Start and End have no effect.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseSpineAngMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseSpineAngMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseVelStart setting for this <see cref="ConfigureBulletsHelper"/>.
		/// 100% LE Start to impulseVelMult*100% GT End. NB: Start LT End.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float ImpulseVelStart
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseVelStart", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseVelEnd setting for this <see cref="ConfigureBulletsHelper"/>.
		/// 100% LE Start to impulseVelMult*100% GT End. NB: Start LT End.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float ImpulseVelEnd
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseVelEnd", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseVelMult setting for this <see cref="ConfigureBulletsHelper"/>.
		/// 100% LE Start to impulseVelMult*100% GT End. NB: leaving this as 1.0 means this functionality is not applied and Start and End have no effect.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseVelMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseVelMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseAirMult setting for this <see cref="ConfigureBulletsHelper"/>.
		/// amount to scale impulse by if the character is airborne and dynamicBalance is OK and impulse is above impulseAirMultStart.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseAirMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseAirMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseAirMultStart setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if impulse is above this value scale it by impulseAirMult.
		/// </summary>
		/// <remarks>
		/// Default value = 100.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseAirMultStart
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseAirMultStart", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseAirMax setting for this <see cref="ConfigureBulletsHelper"/>.
		/// amount to clamp impulse to if character is airborne  and dynamicBalance is OK.
		/// </summary>
		/// <remarks>
		/// Default value = 100.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseAirMax
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseAirMax", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseAirApplyAbove setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if impulse is above this amount then do not scale/clamp just let it through as is - it's a shotgun or cannon.
		/// </summary>
		/// <remarks>
		/// Default value = 399.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseAirApplyAbove
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseAirApplyAbove", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseAirOn setting for this <see cref="ConfigureBulletsHelper"/>.
		/// scale and/or clamp impulse if the character is airborne and dynamicBalance is OK.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ImpulseAirOn
		{
			set { SetArgument("impulseAirOn", value); }
		}

		/// <summary>
		/// Sets the ImpulseOneLegMult setting for this <see cref="ConfigureBulletsHelper"/>.
		/// amount to scale impulse by if the character is contacting with one foot only and dynamicBalance is OK and impulse is above impulseAirMultStart.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseOneLegMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseOneLegMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseOneLegMultStart setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if impulse is above this value scale it by impulseOneLegMult.
		/// </summary>
		/// <remarks>
		/// Default value = 100.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseOneLegMultStart
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseOneLegMultStart", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseOneLegMax setting for this <see cref="ConfigureBulletsHelper"/>.
		/// amount to clamp impulse to if character is contacting with one foot only  and dynamicBalance is OK.
		/// </summary>
		/// <remarks>
		/// Default value = 100.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseOneLegMax
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseOneLegMax", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseOneLegApplyAbove setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if impulse is above this amount then do not scale/clamp just let it through as is - it's a shotgun or cannon.
		/// </summary>
		/// <remarks>
		/// Default value = 399.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseOneLegApplyAbove
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseOneLegApplyAbove", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseOneLegOn setting for this <see cref="ConfigureBulletsHelper"/>.
		/// scale and/or clamp impulse if the character is contacting with one leg only and dynamicBalance is OK.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ImpulseOneLegOn
		{
			set { SetArgument("impulseOneLegOn", value); }
		}

		/// <summary>
		/// Sets the RbRatio setting for this <see cref="ConfigureBulletsHelper"/>.
		/// 0.0 no rigidBody response, 0.5 half partForce half rigidBody, 1.0 = no partForce full rigidBody.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbRatio
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbRatio", value);
			}
		}

		/// <summary>
		/// Sets the RbLowerShare setting for this <see cref="ConfigureBulletsHelper"/>.
		/// rigid body response is shared between the upper and lower body (rbUpperShare = 1-rbLowerShare). rbLowerShare=0.5 gives upper and lower share scaled by mass.  i.e. if 70% ub mass and 30% lower mass then rbLowerShare=0.5 gives actualrbShare of 0.7ub and 0.3lb. rbLowerShare GT 0.5 scales the ub share down from 0.7 and the lb up from 0.3.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbLowerShare
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbLowerShare", value);
			}
		}

		/// <summary>
		/// Sets the RbMoment setting for this <see cref="ConfigureBulletsHelper"/>.
		/// 0.0 only force, 0.5 = force and half the rigid body moment applied, 1.0 = force and full rigidBody moment.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbMoment
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMoment", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxTwistMomentArm setting for this <see cref="ConfigureBulletsHelper"/>.
		/// Maximum twist arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxTwistMomentArm
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxTwistMomentArm", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxBroomMomentArm setting for this <see cref="ConfigureBulletsHelper"/>.
		/// Maximum broom((everything but the twist) arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxBroomMomentArm
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxBroomMomentArm", value);
			}
		}

		/// <summary>
		/// Sets the RbRatioAirborne setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if Airborne: 0.0 no rigidBody response, 0.5 half partForce half rigidBody, 1.0 = no partForce full rigidBody.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbRatioAirborne
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbRatioAirborne", value);
			}
		}

		/// <summary>
		/// Sets the RbMomentAirborne setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if Airborne: 0.0 only force, 0.5 = force and half the rigid body moment applied, 1.0 = force and full rigidBody moment.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbMomentAirborne
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMomentAirborne", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxTwistMomentArmAirborne setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if Airborne: Maximum twist arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxTwistMomentArmAirborne
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxTwistMomentArmAirborne", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxBroomMomentArmAirborne setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if Airborne: Maximum broom((everything but the twist) arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxBroomMomentArmAirborne
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxBroomMomentArmAirborne", value);
			}
		}

		/// <summary>
		/// Sets the RbRatioOneLeg setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if only one leg in contact: 0.0 no rigidBody response, 0.5 half partForce half rigidBody, 1.0 = no partForce full rigidBody.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbRatioOneLeg
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbRatioOneLeg", value);
			}
		}

		/// <summary>
		/// Sets the RbMomentOneLeg setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if only one leg in contact: 0.0 only force, 0.5 = force and half the rigid body moment applied, 1.0 = force and full rigidBody moment.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbMomentOneLeg
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMomentOneLeg", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxTwistMomentArmOneLeg setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if only one leg in contact: Maximum twist arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxTwistMomentArmOneLeg
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxTwistMomentArmOneLeg", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxBroomMomentArmOneLeg setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if only one leg in contact: Maximum broom((everything but the twist) arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxBroomMomentArmOneLeg
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxBroomMomentArmOneLeg", value);
			}
		}

		/// <summary>
		/// Sets the RbTwistAxis setting for this <see cref="ConfigureBulletsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="RbTwistAxis.WorldUp"/>.
		/// </remarks>.
		public RbTwistAxis RbTwistAxis
		{
			set { SetArgument("rbTwistAxis", (int) value); }
		}

		/// <summary>
		/// Sets the RbPivot setting for this <see cref="ConfigureBulletsHelper"/>.
		/// if false pivot around COM always, if true change pivot depending on foot contact:  to feet centre if both feet in contact, or foot position if 1 foot in contact or COM position if no feet in contact.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool RbPivot
		{
			set { SetArgument("rbPivot", value); }
		}
	}

	public sealed class ConfigureBulletsExtraHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ConfigureBulletsExtraHelper for sending a ConfigureBulletsExtra <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ConfigureBulletsExtra <see cref="Message"/> to.</param>
		public ConfigureBulletsExtraHelper(Ped ped) : base(ped, "configureBulletsExtra")
		{
		}

		/// <summary>
		/// Sets the ImpulseSpreadOverParts setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// spreads impulse across parts. currently only for spine parts, not limbs.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ImpulseSpreadOverParts
		{
			set { SetArgument("impulseSpreadOverParts", value); }
		}

		/// <summary>
		/// Sets the ImpulsePeriod setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// duration that impulse is spread over (triangular shaped).
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulsePeriod
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulsePeriod", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseTorqueScale setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// An impulse applied at a point on a body equivalent to an impulse at the centre of the body and a torque.  This parameter scales the torque component. (The torque component seems to be excite the rage looseness bug which sends the character in a sometimes wildly different direction to an applied impulse).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseTorqueScale
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseTorqueScale", value);
			}
		}

		/// <summary>
		/// Sets the LoosenessFix setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// Fix the rage looseness bug by applying only the impulse at the centre of the body unless it is a spine part then apply the twist component only of the torque as well.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool LoosenessFix
		{
			set { SetArgument("loosenessFix", value); }
		}

		/// <summary>
		/// Sets the ImpulseDelay setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// time from hit before impulses are being applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseDelay
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseDelay", value);
			}
		}

		/// <summary>
		/// Sets the TorqueMode setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="TorqueMode.Disabled"/>.
		/// If <see cref="TorqueMode.Proportional"/> - proportional to character strength, can reduce impulse amount.
		/// If <see cref="TorqueMode.Additive"/> - no reduction of impulse and not proportional to character strength.
		/// </remarks>
		public TorqueMode TorqueMode
		{
			set { SetArgument("torqueMode", (int) value); }
		}

		/// <summary>
		/// Sets the TorqueSpinMode setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="TorqueSpinMode.FromImpulse"/>.
		/// If <see cref="TorqueSpinMode.Flipping"/> a burst effect is achieved.
		/// </remarks>
		public TorqueSpinMode TorqueSpinMode
		{
			set { SetArgument("torqueSpinMode", (int) value); }
		}

		/// <summary>
		/// Sets the TorqueFilterMode setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="TorqueFilterMode.ApplyEveryBullet"/>.
		/// </remarks>
		public TorqueFilterMode TorqueFilterMode
		{
			set { SetArgument("torqueFilterMode", (int) value); }
		}

		/// <summary>
		/// Sets the TorqueAlwaysSpine3 setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// always apply torques to spine3 instead of actual part hit.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool TorqueAlwaysSpine3
		{
			set { SetArgument("torqueAlwaysSpine3", value); }
		}

		/// <summary>
		/// Sets the TorqueDelay setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// time from hit before torques are being applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TorqueDelay
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torqueDelay", value);
			}
		}

		/// <summary>
		/// Sets the TorquePeriod setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// duration of torque.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TorquePeriod
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torquePeriod", value);
			}
		}

		/// <summary>
		/// Sets the TorqueGain setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// multiplies impulse magnitude to arrive at torque that is applied.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float TorqueGain
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torqueGain", value);
			}
		}

		/// <summary>
		/// Sets the TorqueCutoff setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// minimum ratio of impulse that remains after converting to torque (if in strength-proportional mode).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TorqueCutoff
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torqueCutoff", value);
			}
		}

		/// <summary>
		/// Sets the TorqueReductionPerTick setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// ratio of torque for next tick (e.g. 1.0: not reducing over time, 0.9: each tick torque is reduced by 10%).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TorqueReductionPerTick
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torqueReductionPerTick", value);
			}
		}

		/// <summary>
		/// Sets the LiftGain setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// amount of lift (directly multiplies torque axis to give lift force).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LiftGain
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("liftGain", value);
			}
		}

		/// <summary>
		/// Sets the CounterImpulseDelay setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// time after impulse is applied that counter impulse is applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CounterImpulseDelay
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("counterImpulseDelay", value);
			}
		}

		/// <summary>
		/// Sets the CounterImpulseMag setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// amount of the original impulse that is countered.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CounterImpulseMag
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("counterImpulseMag", value);
			}
		}

		/// <summary>
		/// Sets the CounterAfterMagReached setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// applies the counter impulse counterImpulseDelay(secs) after counterImpulseMag of the Impulse has been applied.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool CounterAfterMagReached
		{
			set { SetArgument("counterAfterMagReached", value); }
		}

		/// <summary>
		/// Sets the DoCounterImpulse setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// add a counter impulse to the pelvis.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool DoCounterImpulse
		{
			set { SetArgument("doCounterImpulse", value); }
		}

		/// <summary>
		/// Sets the CounterImpulse2Hips setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// amount of the counter impulse applied to hips - the rest is applied to the part originally hit.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CounterImpulse2Hips
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("counterImpulse2Hips", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseNoBalMult setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// amount to scale impulse by if the dynamicBalance is not OK.  1.0 means this functionality is not applied.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseNoBalMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseNoBalMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseBalStabStart setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// 100% LE Start to impulseBalStabMult*100% GT End. NB: Start LT End.
		/// </summary>
		/// <remarks>
		/// Default value = 3.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float ImpulseBalStabStart
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseBalStabStart", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseBalStabEnd setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// 100% LE Start to impulseBalStabMult*100% GT End. NB: Start LT End.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float ImpulseBalStabEnd
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseBalStabEnd", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseBalStabMult setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// 100% LE Start to impulseBalStabMult*100% GT End. NB: leaving this as 1.0 means this functionality is not applied and Start and End have no effect.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseBalStabMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseBalStabMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseSpineAngStart setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// 100% GE Start to impulseSpineAngMult*100% LT End. NB: Start GT End.  This the dot of hip2Head with up.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseSpineAngStart
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("impulseSpineAngStart", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseSpineAngEnd setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// 100% GE Start to impulseSpineAngMult*100% LT End. NB: Start GT End.  This the dot of hip2Head with up.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseSpineAngEnd
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("impulseSpineAngEnd", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseSpineAngMult setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// 100% GE Start to impulseSpineAngMult*100% LT End. NB: leaving this as 1.0 means this functionality is not applied and Start and End have no effect.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseSpineAngMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseSpineAngMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseVelStart setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// 100% LE Start to impulseVelMult*100% GT End. NB: Start LT End.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float ImpulseVelStart
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseVelStart", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseVelEnd setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// 100% LE Start to impulseVelMult*100% GT End. NB: Start LT End.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float ImpulseVelEnd
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseVelEnd", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseVelMult setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// 100% LE Start to impulseVelMult*100% GT End. NB: leaving this as 1.0 means this functionality is not applied and Start and End have no effect.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseVelMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseVelMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseAirMult setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// amount to scale impulse by if the character is airborne and dynamicBalance is OK and impulse is above impulseAirMultStart.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseAirMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseAirMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseAirMultStart setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if impulse is above this value scale it by impulseAirMult.
		/// </summary>
		/// <remarks>
		/// Default value = 100.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseAirMultStart
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseAirMultStart", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseAirMax setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// amount to clamp impulse to if character is airborne  and dynamicBalance is OK.
		/// </summary>
		/// <remarks>
		/// Default value = 100.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseAirMax
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseAirMax", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseAirApplyAbove setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if impulse is above this amount then do not scale/clamp just let it through as is - it's a shotgun or cannon.
		/// </summary>
		/// <remarks>
		/// Default value = 399.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseAirApplyAbove
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseAirApplyAbove", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseAirOn setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// scale and/or clamp impulse if the character is airborne and dynamicBalance is OK.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ImpulseAirOn
		{
			set { SetArgument("impulseAirOn", value); }
		}

		/// <summary>
		/// Sets the ImpulseOneLegMult setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// amount to scale impulse by if the character is contacting with one foot only and dynamicBalance is OK and impulse is above impulseAirMultStart.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpulseOneLegMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseOneLegMult", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseOneLegMultStart setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if impulse is above this value scale it by impulseOneLegMult.
		/// </summary>
		/// <remarks>
		/// Default value = 100.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseOneLegMultStart
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseOneLegMultStart", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseOneLegMax setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// amount to clamp impulse to if character is contacting with one foot only  and dynamicBalance is OK.
		/// </summary>
		/// <remarks>
		/// Default value = 100.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseOneLegMax
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseOneLegMax", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseOneLegApplyAbove setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if impulse is above this amount then do not scale/clamp just let it through as is - it's a shotgun or cannon.
		/// </summary>
		/// <remarks>
		/// Default value = 399.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ImpulseOneLegApplyAbove
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impulseOneLegApplyAbove", value);
			}
		}

		/// <summary>
		/// Sets the ImpulseOneLegOn setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// scale and/or clamp impulse if the character is contacting with one leg only and dynamicBalance is OK.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ImpulseOneLegOn
		{
			set { SetArgument("impulseOneLegOn", value); }
		}

		/// <summary>
		/// Sets the RbRatio setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// 0.0 no rigidBody response, 0.5 half partForce half rigidBody, 1.0 = no partForce full rigidBody.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbRatio
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbRatio", value);
			}
		}

		/// <summary>
		/// Sets the RbLowerShare setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// rigid body response is shared between the upper and lower body (rbUpperShare = 1-rbLowerShare). rbLowerShare=0.5 gives upper and lower share scaled by mass.  i.e. if 70% ub mass and 30% lower mass then rbLowerShare=0.5 gives actualrbShare of 0.7ub and 0.3lb. rbLowerShare GT 0.5 scales the ub share down from 0.7 and the lb up from 0.3.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbLowerShare
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbLowerShare", value);
			}
		}

		/// <summary>
		/// Sets the RbMoment setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// 0.0 only force, 0.5 = force and half the rigid body moment applied, 1.0 = force and full rigidBody moment.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbMoment
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMoment", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxTwistMomentArm setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// Maximum twist arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxTwistMomentArm
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxTwistMomentArm", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxBroomMomentArm setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// Maximum broom((everything but the twist) arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxBroomMomentArm
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxBroomMomentArm", value);
			}
		}

		/// <summary>
		/// Sets the RbRatioAirborne setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if Airborne: 0.0 no rigidBody response, 0.5 half partForce half rigidBody, 1.0 = no partForce full rigidBody.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbRatioAirborne
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbRatioAirborne", value);
			}
		}

		/// <summary>
		/// Sets the RbMomentAirborne setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if Airborne: 0.0 only force, 0.5 = force and half the rigid body moment applied, 1.0 = force and full rigidBody moment.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbMomentAirborne
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMomentAirborne", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxTwistMomentArmAirborne setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if Airborne: Maximum twist arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxTwistMomentArmAirborne
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxTwistMomentArmAirborne", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxBroomMomentArmAirborne setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if Airborne: Maximum broom((everything but the twist) arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxBroomMomentArmAirborne
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxBroomMomentArmAirborne", value);
			}
		}

		/// <summary>
		/// Sets the RbRatioOneLeg setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if only one leg in contact: 0.0 no rigidBody response, 0.5 half partForce half rigidBody, 1.0 = no partForce full rigidBody.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbRatioOneLeg
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbRatioOneLeg", value);
			}
		}

		/// <summary>
		/// Sets the RbMomentOneLeg setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if only one leg in contact: 0.0 only force, 0.5 = force and half the rigid body moment applied, 1.0 = force and full rigidBody moment.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RbMomentOneLeg
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMomentOneLeg", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxTwistMomentArmOneLeg setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if only one leg in contact: Maximum twist arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxTwistMomentArmOneLeg
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxTwistMomentArmOneLeg", value);
			}
		}

		/// <summary>
		/// Sets the RbMaxBroomMomentArmOneLeg setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if only one leg in contact: Maximum broom((everything but the twist) arm moment of bullet applied.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RbMaxBroomMomentArmOneLeg
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rbMaxBroomMomentArmOneLeg", value);
			}
		}

		/// <summary>
		/// Sets the RbTwistAxis setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="RbTwistAxis.WorldUp"/>.
		/// </remarks>.
		public RbTwistAxis RbTwistAxis
		{
			set { SetArgument("rbTwistAxis", (int) value); }
		}

		/// <summary>
		/// Sets the RbPivot setting for this <see cref="ConfigureBulletsExtraHelper"/>.
		/// if false pivot around COM always, if true change pivot depending on foot contact:  to feet centre if both feet in contact, or foot position if 1 foot in contact or COM position if no feet in contact.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool RbPivot
		{
			set { SetArgument("rbPivot", value); }
		}
	}

	/// <summary>
	/// Enable/disable/edit character limits in real time.  This adjusts limits in RAGE-native space and will *not* reorient the joint.
	/// </summary>
	public sealed class ConfigureLimitsHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ConfigureLimitsHelper for sending a ConfigureLimits <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ConfigureLimits <see cref="Message"/> to.</param>
		/// <remarks>
		/// Enable/disable/edit character limits in real time.  This adjusts limits in RAGE-native space and will *not* reorient the joint.
		/// </remarks>
		public ConfigureLimitsHelper(Ped ped) : base(ped, "configureLimits")
		{
		}

		/// <summary>
		/// Sets the Mask setting for this <see cref="ConfigureLimitsHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  for joint limits to configure. Ignored if index != -1.
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string Mask
		{
			set { SetArgument("mask", value); }
		}

		/// <summary>
		/// Sets the Enable setting for this <see cref="ConfigureLimitsHelper"/>.
		/// If false, disable (set all to PI, -PI) limits.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool Enable
		{
			set { SetArgument("enable", value); }
		}

		/// <summary>
		/// Sets the ToDesired setting for this <see cref="ConfigureLimitsHelper"/>.
		/// If true, set limits to accommodate current desired angles.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ToDesired
		{
			set { SetArgument("toDesired", value); }
		}

		/// <summary>
		/// Sets the Restore setting for this <see cref="ConfigureLimitsHelper"/>.
		/// Return to cached defaults?.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Restore
		{
			set { SetArgument("restore", value); }
		}

		/// <summary>
		/// Sets the ToCurAnimation setting for this <see cref="ConfigureLimitsHelper"/>.
		/// If true, set limits to the current animated limits.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ToCurAnimation
		{
			set { SetArgument("toCurAnimation", value); }
		}

		/// <summary>
		/// Sets the Index setting for this <see cref="ConfigureLimitsHelper"/>.
		/// Index of effector to configure.  Set to -1 to use mask.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int Index
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("index", value);
			}
		}

		/// <summary>
		/// Sets the Lean1 setting for this <see cref="ConfigureLimitsHelper"/>.
		/// Custom limit values to use if not setting limits to desired. Limits are RAGE-native, not NM-wrapper-native.
		/// </summary>
		/// <remarks>
		/// Default value = 1.6f.
		/// Min value = 0.0f.
		/// Max value = 3.1f.
		/// </remarks>
		public float Lean1
		{
			set
			{
				if (value > 3.1f)
					value = 3.1f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lean1", value);
			}
		}

		/// <summary>
		/// Sets the Lean2 setting for this <see cref="ConfigureLimitsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.6f.
		/// Min value = 0.0f.
		/// Max value = 3.1f.
		/// </remarks>
		public float Lean2
		{
			set
			{
				if (value > 3.1f)
					value = 3.1f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lean2", value);
			}
		}

		/// <summary>
		/// Sets the Twist setting for this <see cref="ConfigureLimitsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.6f.
		/// Min value = 0.0f.
		/// Max value = 3.1f.
		/// </remarks>
		public float Twist
		{
			set
			{
				if (value > 3.1f)
					value = 3.1f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("twist", value);
			}
		}

		/// <summary>
		/// Sets the Margin setting for this <see cref="ConfigureLimitsHelper"/>.
		/// Joint limit margin to add to current animation limits when using those to set runtime limits.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 3.1f.
		/// </remarks>
		public float Margin
		{
			set
			{
				if (value > 3.1f)
					value = 3.1f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("margin", value);
			}
		}
	}

	public sealed class ConfigureSoftLimitHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ConfigureSoftLimitHelper for sending a ConfigureSoftLimit <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ConfigureSoftLimit <see cref="Message"/> to.</param>
		public ConfigureSoftLimitHelper(Ped ped) : base(ped, "configureSoftLimit")
		{
		}

		/// <summary>
		/// Sets the Index setting for this <see cref="ConfigureSoftLimitHelper"/>.
		/// Select limb that the soft limit is going to be applied to.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 3.
		/// </remarks>
		public int Index
		{
			set
			{
				if (value > 3)
					value = 3;
				if (value < 0)
					value = 0;
				SetArgument("index", value);
			}
		}

		/// <summary>
		/// Sets the Stiffness setting for this <see cref="ConfigureSoftLimitHelper"/>.
		/// Stiffness of the soft limit. Parameter is used to calculate spring term that contributes to the desired acceleration.
		/// </summary>
		/// <remarks>
		/// Default value = 15.0f.
		/// Min value = 0.0f.
		/// Max value = 30.0f.
		/// </remarks>
		public float Stiffness
		{
			set
			{
				if (value > 30.0f)
					value = 30.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stiffness", value);
			}
		}

		/// <summary>
		/// Sets the Damping setting for this <see cref="ConfigureSoftLimitHelper"/>.
		/// Damping of the soft limit. Parameter is used to calculate damper term that contributes to the desired acceleration. To have the system critically dampened set it to 1.0.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.9f.
		/// Max value = 1.1f.
		/// </remarks>
		public float Damping
		{
			set
			{
				if (value > 1.1f)
					value = 1.1f;
				if (value < 0.9f)
					value = 0.9f;
				SetArgument("damping", value);
			}
		}

		/// <summary>
		/// Sets the LimitAngle setting for this <see cref="ConfigureSoftLimitHelper"/>.
		/// Soft limit angle. Positive angle in RAD, measured relatively either from hard limit maxAngle (approach direction = -1) or minAngle (approach direction = 1). This angle will be clamped if outside the joint hard limit range.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 6.3f.
		/// </remarks>
		public float LimitAngle
		{
			set
			{
				if (value > 6.3f)
					value = 6.3f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("limitAngle", value);
			}
		}

		/// <summary>
		/// Sets the ApproachDirection setting for this <see cref="ConfigureSoftLimitHelper"/>.
		/// Limit angle can be measured relatively to joints hard limit minAngle or maxAngle. Set approachDirection to +1 to measure soft limit angle relatively to hard limit minAngle that corresponds to the maximum stretch of the elbow. Set it to -1 to measure soft limit angle relatively to hard limit maxAngle that corresponds to the maximum stretch of the knee.
		/// </summary>
		/// <remarks>
		/// Default value = 1.
		/// Min value = -1.
		/// Max value = 1.
		/// </remarks>
		public int ApproachDirection
		{
			set
			{
				if (value > 1)
					value = 1;
				if (value < -1)
					value = -1;
				SetArgument("approachDirection", value);
			}
		}

		/// <summary>
		/// Sets the VelocityScaled setting for this <see cref="ConfigureSoftLimitHelper"/>.
		/// Scale stiffness based on character angular velocity.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool VelocityScaled
		{
			set { SetArgument("velocityScaled", value); }
		}
	}

	/// <summary>
	/// This single message allows you to configure the injured arm reaction during shot.
	/// </summary>
	public sealed class ConfigureShotInjuredArmHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ConfigureShotInjuredArmHelper for sending a ConfigureShotInjuredArm <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ConfigureShotInjuredArm <see cref="Message"/> to.</param>
		/// <remarks>
		/// This single message allows you to configure the injured arm reaction during shot.
		/// </remarks>
		public ConfigureShotInjuredArmHelper(Ped ped) : base(ped, "configureShotInjuredArm")
		{
		}

		/// <summary>
		/// Sets the InjuredArmTime setting for this <see cref="ConfigureShotInjuredArmHelper"/>.
		/// length of the reaction.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float InjuredArmTime
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("injuredArmTime", value);
			}
		}

		/// <summary>
		/// Sets the HipYaw setting for this <see cref="ConfigureShotInjuredArmHelper"/>.
		/// Amount of hip twist.  (Negative values twist into bullet direction - probably not what is wanted).
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = -2.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float HipYaw
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -2.0f)
					value = -2.0f;
				SetArgument("hipYaw", value);
			}
		}

		/// <summary>
		/// Sets the HipRoll setting for this <see cref="ConfigureShotInjuredArmHelper"/>.
		/// Amount of hip roll.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -2.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float HipRoll
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -2.0f)
					value = -2.0f;
				SetArgument("hipRoll", value);
			}
		}

		/// <summary>
		/// Sets the ForceStepExtraHeight setting for this <see cref="ConfigureShotInjuredArmHelper"/>.
		/// Additional height added to stepping foot.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 0.7f.
		/// </remarks>
		public float ForceStepExtraHeight
		{
			set
			{
				if (value > 0.7f)
					value = 0.7f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("forceStepExtraHeight", value);
			}
		}

		/// <summary>
		/// Sets the ForceStep setting for this <see cref="ConfigureShotInjuredArmHelper"/>.
		/// force a step to be taken whether pushed out of balance or not.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ForceStep
		{
			set { SetArgument("forceStep", value); }
		}

		/// <summary>
		/// Sets the StepTurn setting for this <see cref="ConfigureShotInjuredArmHelper"/>.
		/// turn the character using the balancer.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool StepTurn
		{
			set { SetArgument("stepTurn", value); }
		}

		/// <summary>
		/// Sets the VelMultiplierStart setting for this <see cref="ConfigureShotInjuredArmHelper"/>.
		/// Start velocity where parameters begin to be ramped down to zero linearly.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float VelMultiplierStart
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("velMultiplierStart", value);
			}
		}

		/// <summary>
		/// Sets the VelMultiplierEnd setting for this <see cref="ConfigureShotInjuredArmHelper"/>.
		/// End velocity of ramp where parameters are scaled to zero.
		/// </summary>
		/// <remarks>
		/// Default value = 5.0f.
		/// Min value = 1.0f.
		/// Max value = 40.0f.
		/// </remarks>
		public float VelMultiplierEnd
		{
			set
			{
				if (value > 40.0f)
					value = 40.0f;
				if (value < 1.0f)
					value = 1.0f;
				SetArgument("velMultiplierEnd", value);
			}
		}

		/// <summary>
		/// Sets the VelForceStep setting for this <see cref="ConfigureShotInjuredArmHelper"/>.
		/// Velocity above which a step is not forced.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float VelForceStep
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("velForceStep", value);
			}
		}

		/// <summary>
		/// Sets the VelStepTurn setting for this <see cref="ConfigureShotInjuredArmHelper"/>.
		/// Velocity above which a stepTurn is not asked for.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float VelStepTurn
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("velStepTurn", value);
			}
		}

		/// <summary>
		/// Sets the VelScales setting for this <see cref="ConfigureShotInjuredArmHelper"/>.
		/// Use the velocity scaling parameters.  Tune for standing still then use velocity scaling to make sure a running character stays balanced (the turning tends to make the character fall over more at speed).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool VelScales
		{
			set { SetArgument("velScales", value); }
		}
	}

	/// <summary>
	/// This single message allows you to configure the injured leg reaction during shot.
	/// </summary>
	public sealed class ConfigureShotInjuredLegHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ConfigureShotInjuredLegHelper for sending a ConfigureShotInjuredLeg <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ConfigureShotInjuredLeg <see cref="Message"/> to.</param>
		/// <remarks>
		/// This single message allows you to configure the injured leg reaction during shot.
		/// </remarks>
		public ConfigureShotInjuredLegHelper(Ped ped) : base(ped, "configureShotInjuredLeg")
		{
		}

		/// <summary>
		/// Sets the TimeBeforeCollapseWoundLeg setting for this <see cref="ConfigureShotInjuredLegHelper"/>.
		/// time before a wounded leg is set to be weak and cause the character to collapse.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float TimeBeforeCollapseWoundLeg
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("timeBeforeCollapseWoundLeg", value);
			}
		}

		/// <summary>
		/// Sets the LegInjuryTime setting for this <see cref="ConfigureShotInjuredLegHelper"/>.
		/// Leg inury duration (reaction to being shot in leg).
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float LegInjuryTime
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legInjuryTime", value);
			}
		}

		/// <summary>
		/// Sets the LegForceStep setting for this <see cref="ConfigureShotInjuredLegHelper"/>.
		/// force a step to be taken whether pushed out of balance or not.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool LegForceStep
		{
			set { SetArgument("legForceStep", value); }
		}

		/// <summary>
		/// Sets the LegLimpBend setting for this <see cref="ConfigureShotInjuredLegHelper"/>.
		/// Bend the legs via the balancer by this amount if stepping on the injured leg. 0.2 seems a good default.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LegLimpBend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legLimpBend", value);
			}
		}

		/// <summary>
		/// Sets the LegLiftTime setting for this <see cref="ConfigureShotInjuredLegHelper"/>.
		/// Leg lift duration (reaction to being shot in leg) (lifting happens when not stepping with other leg).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float LegLiftTime
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legLiftTime", value);
			}
		}

		/// <summary>
		/// Sets the LegInjury setting for this <see cref="ConfigureShotInjuredLegHelper"/>.
		/// Leg injury - leg strength is reduced.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LegInjury
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legInjury", value);
			}
		}

		/// <summary>
		/// Sets the LegInjuryHipPitch setting for this <see cref="ConfigureShotInjuredLegHelper"/>.
		/// Leg injury bend forwards amount when not lifting leg.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LegInjuryHipPitch
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("legInjuryHipPitch", value);
			}
		}

		/// <summary>
		/// Sets the LegInjuryLiftHipPitch setting for this <see cref="ConfigureShotInjuredLegHelper"/>.
		/// Leg injury bend forwards amount when lifting leg (lifting happens when not stepping with other leg).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LegInjuryLiftHipPitch
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("legInjuryLiftHipPitch", value);
			}
		}

		/// <summary>
		/// Sets the LegInjurySpineBend setting for this <see cref="ConfigureShotInjuredLegHelper"/>.
		/// Leg injury bend forwards amount when not lifting leg.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LegInjurySpineBend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("legInjurySpineBend", value);
			}
		}

		/// <summary>
		/// Sets the LegInjuryLiftSpineBend setting for this <see cref="ConfigureShotInjuredLegHelper"/>.
		/// Leg injury bend forwards amount when lifting leg (lifting happens when not stepping with other leg).
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LegInjuryLiftSpineBend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("legInjuryLiftSpineBend", value);
			}
		}
	}

	public sealed class DefineAttachedObjectHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the DefineAttachedObjectHelper for sending a DefineAttachedObject <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the DefineAttachedObject <see cref="Message"/> to.</param>
		public DefineAttachedObjectHelper(Ped ped) : base(ped, "defineAttachedObject")
		{
		}

		/// <summary>
		/// Sets the PartIndex setting for this <see cref="DefineAttachedObjectHelper"/>.
		/// index of part to attach to.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// Max value = 21.
		/// </remarks>
		public int PartIndex
		{
			set
			{
				if (value > 21)
					value = 21;
				if (value < -1)
					value = -1;
				SetArgument("partIndex", value);
			}
		}

		/// <summary>
		/// Sets the ObjectMass setting for this <see cref="DefineAttachedObjectHelper"/>.
		/// mass of the attached object.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ObjectMass
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("objectMass", value);
			}
		}

		/// <summary>
		/// Sets the WorldPos setting for this <see cref="DefineAttachedObjectHelper"/>.
		/// world position of attached object's centre of mass. must be updated each frame.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 WorldPos
		{
			set { SetArgument("worldPos", value); }
		}
	}

	/// <summary>
	/// Apply an impulse to a named body part.
	/// </summary>
	public sealed class ForceToBodyPartHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ForceToBodyPartHelper for sending a ForceToBodyPart <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ForceToBodyPart <see cref="Message"/> to.</param>
		/// <remarks>
		/// Apply an impulse to a named body part.
		/// </remarks>
		public ForceToBodyPartHelper(Ped ped) : base(ped, "forceToBodyPart")
		{
		}

		/// <summary>
		/// Sets the PartIndex setting for this <see cref="ForceToBodyPartHelper"/>.
		/// part or link or bound index.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 28.
		/// </remarks>
		public int PartIndex
		{
			set
			{
				if (value > 28)
					value = 28;
				if (value < 0)
					value = 0;
				SetArgument("partIndex", value);
			}
		}

		/// <summary>
		/// Sets the Force setting for this <see cref="ForceToBodyPartHelper"/>.
		/// force to apply.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, -50.0f, 0.0f).
		/// Min value = -100000.0f.
		/// Max value = 100000.0f.
		/// </remarks>
		public Vector3 Force
		{
			set
			{
				SetArgument("force",
					Vector3.Clamp(value, new Vector3(-100000.0f, -100000.0f, -100000.0f), new Vector3(100000.0f, 100000.0f, 100000.0f)));
			}
		}

		/// <summary>
		/// Sets the ForceDefinedInPartSpace setting for this <see cref="ForceToBodyPartHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ForceDefinedInPartSpace
		{
			set { SetArgument("forceDefinedInPartSpace", value); }
		}
	}

	public sealed class LeanInDirectionHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the LeanInDirectionHelper for sending a LeanInDirection <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the LeanInDirection <see cref="Message"/> to.</param>
		public LeanInDirectionHelper(Ped ped) : base(ped, "leanInDirection")
		{
		}

		/// <summary>
		/// Sets the LeanAmount setting for this <see cref="LeanInDirectionHelper"/>.
		/// amount of lean, 0 to about 0.5. -ve will move away from the target.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("leanAmount", value);
			}
		}

		/// <summary>
		/// Sets the Dir setting for this <see cref="LeanInDirectionHelper"/>.
		/// direction to lean in.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 1.0f).
		/// Min value = 0.0f.
		/// </remarks>
		public Vector3 Dir
		{
			set { SetArgument("dir", Vector3.Max(value, new Vector3(0.0f, 0.0f, 0.0f))); }
		}
	}

	public sealed class LeanRandomHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the LeanRandomHelper for sending a LeanRandom <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the LeanRandom <see cref="Message"/> to.</param>
		public LeanRandomHelper(Ped ped) : base(ped, "leanRandom")
		{
		}

		/// <summary>
		/// Sets the LeanAmountMin setting for this <see cref="LeanRandomHelper"/>.
		/// minimum amount of lean.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAmountMin
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanAmountMin", value);
			}
		}

		/// <summary>
		/// Sets the LeanAmountMax setting for this <see cref="LeanRandomHelper"/>.
		/// maximum amount of lean.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAmountMax
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanAmountMax", value);
			}
		}

		/// <summary>
		/// Sets the ChangeTimeMin setting for this <see cref="LeanRandomHelper"/>.
		/// min time until changing direction.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float ChangeTimeMin
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("changeTimeMin", value);
			}
		}

		/// <summary>
		/// Sets the ChangeTimeMax setting for this <see cref="LeanRandomHelper"/>.
		/// maximum time until changing direction.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float ChangeTimeMax
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("changeTimeMax", value);
			}
		}
	}

	public sealed class LeanToPositionHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the LeanToPositionHelper for sending a LeanToPosition <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the LeanToPosition <see cref="Message"/> to.</param>
		public LeanToPositionHelper(Ped ped) : base(ped, "leanToPosition")
		{
		}

		/// <summary>
		/// Sets the LeanAmount setting for this <see cref="LeanToPositionHelper"/>.
		/// amount of lean, 0 to about 0.5. -ve will move away from the target.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -0.5f.
		/// Max value = 0.5f.
		/// </remarks>
		public float LeanAmount
		{
			set
			{
				if (value > 0.5f)
					value = 0.5f;
				if (value < -0.5f)
					value = -0.5f;
				SetArgument("leanAmount", value);
			}
		}

		/// <summary>
		/// Sets the Pos setting for this <see cref="LeanToPositionHelper"/>.
		/// position to head towards.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Pos
		{
			set { SetArgument("pos", value); }
		}
	}

	public sealed class LeanTowardsObjectHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the LeanTowardsObjectHelper for sending a LeanTowardsObject <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the LeanTowardsObject <see cref="Message"/> to.</param>
		public LeanTowardsObjectHelper(Ped ped) : base(ped, "leanTowardsObject")
		{
		}

		/// <summary>
		/// Sets the LeanAmount setting for this <see cref="LeanTowardsObjectHelper"/>.
		/// amount of lean, 0 to about 0.5. -ve will move away from the target.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -0.5f.
		/// Max value = 0.5f.
		/// </remarks>
		public float LeanAmount
		{
			set
			{
				if (value > 0.5f)
					value = 0.5f;
				if (value < -0.5f)
					value = -0.5f;
				SetArgument("leanAmount", value);
			}
		}

		/// <summary>
		/// Sets the Offset setting for this <see cref="LeanTowardsObjectHelper"/>.
		/// offset from instance position added when calculating position to lean to.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -100.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public Vector3 Offset
		{
			set
			{
				SetArgument("offset",
					Vector3.Clamp(value, new Vector3(-100.0f, -100.0f, -100.0f), new Vector3(100.0f, 100.0f, 100.0f)));
			}
		}

		/// <summary>
		/// Sets the InstanceIndex setting for this <see cref="LeanTowardsObjectHelper"/>.
		/// levelIndex of object to lean towards.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int InstanceIndex
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("instanceIndex", value);
			}
		}

		/// <summary>
		/// Sets the BoundIndex setting for this <see cref="LeanTowardsObjectHelper"/>.
		/// boundIndex of object to lean towards (0 = just use instance coordinates).
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// </remarks>
		public int BoundIndex
		{
			set
			{
				if (value < 0)
					value = 0;
				SetArgument("boundIndex", value);
			}
		}
	}

	public sealed class HipsLeanInDirectionHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the HipsLeanInDirectionHelper for sending a HipsLeanInDirection <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the HipsLeanInDirection <see cref="Message"/> to.</param>
		public HipsLeanInDirectionHelper(Ped ped) : base(ped, "hipsLeanInDirection")
		{
		}

		/// <summary>
		/// Sets the LeanAmount setting for this <see cref="HipsLeanInDirectionHelper"/>.
		/// amount of lean, 0 to about 0.5. -ve will move away from the target.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("leanAmount", value);
			}
		}

		/// <summary>
		/// Sets the Dir setting for this <see cref="HipsLeanInDirectionHelper"/>.
		/// direction to lean in.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 1.0f).
		/// Min value = 0.0f.
		/// </remarks>
		public Vector3 Dir
		{
			set { SetArgument("dir", Vector3.Max(value, new Vector3(0.0f, 0.0f, 0.0f))); }
		}
	}

	public sealed class HipsLeanRandomHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the HipsLeanRandomHelper for sending a HipsLeanRandom <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the HipsLeanRandom <see cref="Message"/> to.</param>
		public HipsLeanRandomHelper(Ped ped) : base(ped, "hipsLeanRandom")
		{
		}

		/// <summary>
		/// Sets the LeanAmountMin setting for this <see cref="HipsLeanRandomHelper"/>.
		/// minimum amount of lean.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAmountMin
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanAmountMin", value);
			}
		}

		/// <summary>
		/// Sets the LeanAmountMax setting for this <see cref="HipsLeanRandomHelper"/>.
		/// maximum amount of lean.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAmountMax
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanAmountMax", value);
			}
		}

		/// <summary>
		/// Sets the ChangeTimeMin setting for this <see cref="HipsLeanRandomHelper"/>.
		/// min time until changing direction.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float ChangeTimeMin
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("changeTimeMin", value);
			}
		}

		/// <summary>
		/// Sets the ChangeTimeMax setting for this <see cref="HipsLeanRandomHelper"/>.
		/// maximum time until changing direction.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float ChangeTimeMax
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("changeTimeMax", value);
			}
		}
	}

	public sealed class HipsLeanToPositionHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the HipsLeanToPositionHelper for sending a HipsLeanToPosition <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the HipsLeanToPosition <see cref="Message"/> to.</param>
		public HipsLeanToPositionHelper(Ped ped) : base(ped, "hipsLeanToPosition")
		{
		}

		/// <summary>
		/// Sets the LeanAmount setting for this <see cref="HipsLeanToPositionHelper"/>.
		/// amount of lean, 0 to about 0.5. -ve will move away from the target.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -0.5f.
		/// Max value = 0.5f.
		/// </remarks>
		public float LeanAmount
		{
			set
			{
				if (value > 0.5f)
					value = 0.5f;
				if (value < -0.5f)
					value = -0.5f;
				SetArgument("leanAmount", value);
			}
		}

		/// <summary>
		/// Sets the Pos setting for this <see cref="HipsLeanToPositionHelper"/>.
		/// position to head towards.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Pos
		{
			set { SetArgument("pos", value); }
		}
	}

	public sealed class HipsLeanTowardsObjectHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the HipsLeanTowardsObjectHelper for sending a HipsLeanTowardsObject <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the HipsLeanTowardsObject <see cref="Message"/> to.</param>
		public HipsLeanTowardsObjectHelper(Ped ped) : base(ped, "hipsLeanTowardsObject")
		{
		}

		/// <summary>
		/// Sets the LeanAmount setting for this <see cref="HipsLeanTowardsObjectHelper"/>.
		/// amount of lean, 0 to about 0.5. -ve will move away from the target.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -0.5f.
		/// Max value = 0.5f.
		/// </remarks>
		public float LeanAmount
		{
			set
			{
				if (value > 0.5f)
					value = 0.5f;
				if (value < -0.5f)
					value = -0.5f;
				SetArgument("leanAmount", value);
			}
		}

		/// <summary>
		/// Sets the Offset setting for this <see cref="HipsLeanTowardsObjectHelper"/>.
		/// offset from instance position added when calculating position to lean to.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -100.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public Vector3 Offset
		{
			set
			{
				SetArgument("offset",
					Vector3.Clamp(value, new Vector3(-100.0f, -100.0f, -100.0f), new Vector3(100.0f, 100.0f, 100.0f)));
			}
		}

		/// <summary>
		/// Sets the InstanceIndex setting for this <see cref="HipsLeanTowardsObjectHelper"/>.
		/// levelIndex of object to lean hips towards.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int InstanceIndex
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("instanceIndex", value);
			}
		}

		/// <summary>
		/// Sets the BoundIndex setting for this <see cref="HipsLeanTowardsObjectHelper"/>.
		/// boundIndex of object to lean hips towards (0 = just use instance coordinates).
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// </remarks>
		public int BoundIndex
		{
			set
			{
				if (value < 0)
					value = 0;
				SetArgument("boundIndex", value);
			}
		}
	}

	public sealed class ForceLeanInDirectionHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ForceLeanInDirectionHelper for sending a ForceLeanInDirection <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ForceLeanInDirection <see cref="Message"/> to.</param>
		public ForceLeanInDirectionHelper(Ped ped) : base(ped, "forceLeanInDirection")
		{
		}

		/// <summary>
		/// Sets the LeanAmount setting for this <see cref="ForceLeanInDirectionHelper"/>.
		/// amount of lean, 0 to about 0.5. -ve will move away from the target.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("leanAmount", value);
			}
		}

		/// <summary>
		/// Sets the Dir setting for this <see cref="ForceLeanInDirectionHelper"/>.
		/// direction to lean in.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 1.0f).
		/// Min value = 0.0f.
		/// </remarks>
		public Vector3 Dir
		{
			set { SetArgument("dir", Vector3.Max(value, new Vector3(0.0f, 0.0f, 0.0f))); }
		}

		/// <summary>
		/// Sets the BodyPart setting for this <see cref="ForceLeanInDirectionHelper"/>.
		/// body part that the force is applied to.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 21.
		/// </remarks>
		public int BodyPart
		{
			set
			{
				if (value > 21)
					value = 21;
				if (value < 0)
					value = 0;
				SetArgument("bodyPart", value);
			}
		}
	}

	public sealed class ForceLeanRandomHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ForceLeanRandomHelper for sending a ForceLeanRandom <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ForceLeanRandom <see cref="Message"/> to.</param>
		public ForceLeanRandomHelper(Ped ped) : base(ped, "forceLeanRandom")
		{
		}

		/// <summary>
		/// Sets the LeanAmountMin setting for this <see cref="ForceLeanRandomHelper"/>.
		/// minimum amount of lean.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAmountMin
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanAmountMin", value);
			}
		}

		/// <summary>
		/// Sets the LeanAmountMax setting for this <see cref="ForceLeanRandomHelper"/>.
		/// maximum amount of lean.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAmountMax
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanAmountMax", value);
			}
		}

		/// <summary>
		/// Sets the ChangeTimeMin setting for this <see cref="ForceLeanRandomHelper"/>.
		/// min time until changing direction.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float ChangeTimeMin
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("changeTimeMin", value);
			}
		}

		/// <summary>
		/// Sets the ChangeTimeMax setting for this <see cref="ForceLeanRandomHelper"/>.
		/// maximum time until changing direction.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float ChangeTimeMax
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("changeTimeMax", value);
			}
		}

		/// <summary>
		/// Sets the BodyPart setting for this <see cref="ForceLeanRandomHelper"/>.
		/// body part that the force is applied to.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 21.
		/// </remarks>
		public int BodyPart
		{
			set
			{
				if (value > 21)
					value = 21;
				if (value < 0)
					value = 0;
				SetArgument("bodyPart", value);
			}
		}
	}

	public sealed class ForceLeanToPositionHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ForceLeanToPositionHelper for sending a ForceLeanToPosition <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ForceLeanToPosition <see cref="Message"/> to.</param>
		public ForceLeanToPositionHelper(Ped ped) : base(ped, "forceLeanToPosition")
		{
		}

		/// <summary>
		/// Sets the LeanAmount setting for this <see cref="ForceLeanToPositionHelper"/>.
		/// amount of lean, 0 to about 0.5. -ve will move away from the target.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -0.5f.
		/// Max value = 0.5f.
		/// </remarks>
		public float LeanAmount
		{
			set
			{
				if (value > 0.5f)
					value = 0.5f;
				if (value < -0.5f)
					value = -0.5f;
				SetArgument("leanAmount", value);
			}
		}

		/// <summary>
		/// Sets the Pos setting for this <see cref="ForceLeanToPositionHelper"/>.
		/// position to head towards.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Pos
		{
			set { SetArgument("pos", value); }
		}

		/// <summary>
		/// Sets the BodyPart setting for this <see cref="ForceLeanToPositionHelper"/>.
		/// body part that the force is applied to.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 21.
		/// </remarks>
		public int BodyPart
		{
			set
			{
				if (value > 21)
					value = 21;
				if (value < 0)
					value = 0;
				SetArgument("bodyPart", value);
			}
		}
	}

	public sealed class ForceLeanTowardsObjectHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ForceLeanTowardsObjectHelper for sending a ForceLeanTowardsObject <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ForceLeanTowardsObject <see cref="Message"/> to.</param>
		public ForceLeanTowardsObjectHelper(Ped ped) : base(ped, "forceLeanTowardsObject")
		{
		}

		/// <summary>
		/// Sets the LeanAmount setting for this <see cref="ForceLeanTowardsObjectHelper"/>.
		/// amount of lean, 0 to about 0.5. -ve will move away from the target.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -0.5f.
		/// Max value = 0.5f.
		/// </remarks>
		public float LeanAmount
		{
			set
			{
				if (value > 0.5f)
					value = 0.5f;
				if (value < -0.5f)
					value = -0.5f;
				SetArgument("leanAmount", value);
			}
		}

		/// <summary>
		/// Sets the Offset setting for this <see cref="ForceLeanTowardsObjectHelper"/>.
		/// offset from instance position added when calculating position to lean to.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -100.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public Vector3 Offset
		{
			set
			{
				SetArgument("offset",
					Vector3.Clamp(value, new Vector3(-100.0f, -100.0f, -100.0f), new Vector3(100.0f, 100.0f, 100.0f)));
			}
		}

		/// <summary>
		/// Sets the InstanceIndex setting for this <see cref="ForceLeanTowardsObjectHelper"/>.
		/// levelIndex of object to move towards.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int InstanceIndex
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("instanceIndex", value);
			}
		}

		/// <summary>
		/// Sets the BoundIndex setting for this <see cref="ForceLeanTowardsObjectHelper"/>.
		/// boundIndex of object to move towards (0 = just use instance coordinates).
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// </remarks>
		public int BoundIndex
		{
			set
			{
				if (value < 0)
					value = 0;
				SetArgument("boundIndex", value);
			}
		}

		/// <summary>
		/// Sets the BodyPart setting for this <see cref="ForceLeanTowardsObjectHelper"/>.
		/// body part that the force is applied to.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 21.
		/// </remarks>
		public int BodyPart
		{
			set
			{
				if (value > 21)
					value = 21;
				if (value < 0)
					value = 0;
				SetArgument("bodyPart", value);
			}
		}
	}

	/// <summary>
	/// Use this message to manually set the body stiffness values -before using Active Pose to drive to an animated pose, for example.
	/// </summary>
	public sealed class SetStiffnessHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the SetStiffnessHelper for sending a SetStiffness <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the SetStiffness <see cref="Message"/> to.</param>
		/// <remarks>
		/// Use this message to manually set the body stiffness values -before using Active Pose to drive to an animated pose, for example.
		/// </remarks>
		public SetStiffnessHelper(Ped ped) : base(ped, "setStiffness")
		{
		}

		/// <summary>
		/// Sets the BodyStiffness setting for this <see cref="SetStiffnessHelper"/>.
		/// stiffness of whole character.
		/// </summary>
		/// <remarks>
		/// Default value = 12.0f.
		/// Min value = 2.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float BodyStiffness
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 2.0f)
					value = 2.0f;
				SetArgument("bodyStiffness", value);
			}
		}

		/// <summary>
		/// Sets the Damping setting for this <see cref="SetStiffnessHelper"/>.
		/// damping amount, less is underdamped.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float Damping
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("damping", value);
			}
		}

		/// <summary>
		/// Sets the Mask setting for this <see cref="SetStiffnessHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see Active Pose notes for possible values).
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string Mask
		{
			set { SetArgument("mask", value); }
		}
	}

	/// <summary>
	/// Use this message to manually set the muscle stiffness values -before using Active Pose to drive to an animated pose, for example.
	/// </summary>
	public sealed class SetMuscleStiffnessHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the SetMuscleStiffnessHelper for sending a SetMuscleStiffness <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the SetMuscleStiffness <see cref="Message"/> to.</param>
		/// <remarks>
		/// Use this message to manually set the muscle stiffness values -before using Active Pose to drive to an animated pose, for example.
		/// </remarks>
		public SetMuscleStiffnessHelper(Ped ped) : base(ped, "setMuscleStiffness")
		{
		}

		/// <summary>
		/// Sets the MuscleStiffness setting for this <see cref="SetMuscleStiffnessHelper"/>.
		/// muscle stiffness of joint/s.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float MuscleStiffness
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("muscleStiffness", value);
			}
		}

		/// <summary>
		/// Sets the Mask setting for this <see cref="SetMuscleStiffnessHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see Active Pose notes for possible values).
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string Mask
		{
			set { SetArgument("mask", value); }
		}
	}

	/// <summary>
	/// Use this message to set the character's weapon mode.  This is an alternativeto the setWeaponMode public function.
	/// </summary>
	public sealed class SetWeaponModeHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the SetWeaponModeHelper for sending a SetWeaponMode <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the SetWeaponMode <see cref="Message"/> to.</param>
		/// <remarks>
		/// Use this message to set the character's weapon mode.  This is an alternativeto the setWeaponMode public function.
		/// </remarks>
		public SetWeaponModeHelper(Ped ped) : base(ped, "setWeaponMode")
		{
		}

		/// <summary>
		/// Sets the WeaponMode setting for this <see cref="SetWeaponModeHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="WeaponMode.PistolRight"/>.
		/// </remarks>.
		public WeaponMode WeaponMode
		{
			set { SetArgument("weaponMode", (int) value); }
		}
	}

	/// <summary>
	/// Use this message to register weapon.  This is an alternativeto the registerWeapon public function.
	/// </summary>
	public sealed class RegisterWeaponHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the RegisterWeaponHelper for sending a RegisterWeapon <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the RegisterWeapon <see cref="Message"/> to.</param>
		/// <remarks>
		/// Use this message to register weapon.  This is an alternativeto the registerWeapon public function.
		/// </remarks>
		public RegisterWeaponHelper(Ped ped) : base(ped, "registerWeapon")
		{
		}

		/// <summary>
		/// Sets the Hand setting for this <see cref="RegisterWeaponHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="Hand.Right"/>.
		/// </remarks>
		public Hand Hand
		{
			set { SetArgument("hand", (int) value); }
		}

		/// <summary>
		/// Sets the LevelIndex setting for this <see cref="RegisterWeaponHelper"/>.
		/// Level index of the weapon.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int LevelIndex
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("levelIndex", value);
			}
		}

		/// <summary>
		/// Sets the ConstraintHandle setting for this <see cref="RegisterWeaponHelper"/>.
		/// pointer to the hand-gun constraint handle.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int ConstraintHandle
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("constraintHandle", value);
			}
		}

		/// <summary>
		/// Sets the GunToHandA setting for this <see cref="RegisterWeaponHelper"/>.
		/// A vector of the gunToHand matrix.  The gunToHandMatrix is the desired gunToHandMatrix in the aimingPose. (The gunToHandMatrix when pointGun starts can be different so will be blended to this desired one).
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(1.0f, 0.0f, 0.0f).
		/// Min value = 0.0f.
		/// </remarks>
		public Vector3 GunToHandA
		{
			set { SetArgument("gunToHandA", Vector3.Max(value, new Vector3(0.0f, 0.0f, 0.0f))); }
		}

		/// <summary>
		/// Sets the GunToHandB setting for this <see cref="RegisterWeaponHelper"/>.
		/// B vector of the gunToHand matrix.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 1.0f, 0.0f).
		/// Min value = 0.0f.
		/// </remarks>
		public Vector3 GunToHandB
		{
			set { SetArgument("gunToHandB", Vector3.Max(value, new Vector3(0.0f, 0.0f, 0.0f))); }
		}

		/// <summary>
		/// Sets the GunToHandC setting for this <see cref="RegisterWeaponHelper"/>.
		/// C vector of the gunToHand matrix.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 1.0f).
		/// Min value = 0.0f.
		/// </remarks>
		public Vector3 GunToHandC
		{
			set { SetArgument("gunToHandC", Vector3.Max(value, new Vector3(0.0f, 0.0f, 0.0f))); }
		}

		/// <summary>
		/// Sets the GunToHandD setting for this <see cref="RegisterWeaponHelper"/>.
		/// D vector of the gunToHand matrix.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = 0.0f.
		/// </remarks>
		public Vector3 GunToHandD
		{
			set { SetArgument("gunToHandD", Vector3.Max(value, new Vector3(0.0f, 0.0f, 0.0f))); }
		}

		/// <summary>
		/// Sets the GunToMuzzleInGun setting for this <see cref="RegisterWeaponHelper"/>.
		/// Gun centre to muzzle expressed in gun co-ordinates.  To get the line of sight/barrel of the gun. Assumption: the muzzle direction is always along the same primary axis of the gun.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 GunToMuzzleInGun
		{
			set { SetArgument("gunToMuzzleInGun", value); }
		}

		/// <summary>
		/// Sets the GunToButtInGun setting for this <see cref="RegisterWeaponHelper"/>.
		/// Gun centre to butt expressed in gun co-ordinates.  The gun pivots around this point when aiming.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 GunToButtInGun
		{
			set { SetArgument("gunToButtInGun", value); }
		}
	}

	public sealed class ShotRelaxHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ShotRelaxHelper for sending a ShotRelax <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ShotRelax <see cref="Message"/> to.</param>
		public ShotRelaxHelper(Ped ped) : base(ped, "shotRelax")
		{
		}

		/// <summary>
		/// Sets the RelaxPeriodUpper setting for this <see cref="ShotRelaxHelper"/>.
		/// time over which to relax to full relaxation for upper body.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 40.0f.
		/// </remarks>
		public float RelaxPeriodUpper
		{
			set
			{
				if (value > 40.0f)
					value = 40.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("relaxPeriodUpper", value);
			}
		}

		/// <summary>
		/// Sets the RelaxPeriodLower setting for this <see cref="ShotRelaxHelper"/>.
		/// time over which to relax to full relaxation for lower body.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 40.0f.
		/// </remarks>
		public float RelaxPeriodLower
		{
			set
			{
				if (value > 40.0f)
					value = 40.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("relaxPeriodLower", value);
			}
		}
	}

	/// <summary>
	/// One shot message apply a force to the hand as we fire the gun that should be in this hand.
	/// </summary>
	public sealed class FireWeaponHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the FireWeaponHelper for sending a FireWeapon <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the FireWeapon <see cref="Message"/> to.</param>
		/// <remarks>
		/// One shot message apply a force to the hand as we fire the gun that should be in this hand.
		/// </remarks>
		public FireWeaponHelper(Ped ped) : base(ped, "fireWeapon")
		{
		}

		/// <summary>
		/// Sets the FiredWeaponStrength setting for this <see cref="FireWeaponHelper"/>.
		/// The force of the gun.
		/// </summary>
		/// <remarks>
		/// Default value = 1000.0f.
		/// Min value = 0.0f.
		/// Max value = 10000.0f.
		/// </remarks>
		public float FiredWeaponStrength
		{
			set
			{
				if (value > 10000.0f)
					value = 10000.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("firedWeaponStrength", value);
			}
		}

		/// <summary>
		/// Sets the GunHandEnum setting for this <see cref="FireWeaponHelper"/>.
		/// Which hand is the gun in.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="Hand.Left"/>.
		/// </remarks>
		public Hand GunHandEnum
		{
			set { SetArgument("gunHandEnum", (int) value); }
		}

		/// <summary>
		/// Sets the ApplyFireGunForceAtClavicle setting for this <see cref="FireWeaponHelper"/>.
		/// Should we apply some of the force at the shoulder. Force double handed weapons (Ak47 etc).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ApplyFireGunForceAtClavicle
		{
			set { SetArgument("applyFireGunForceAtClavicle", value); }
		}

		/// <summary>
		/// Sets the InhibitTime setting for this <see cref="FireWeaponHelper"/>.
		/// Minimum time before next fire impulse.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float InhibitTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("inhibitTime", value);
			}
		}

		/// <summary>
		/// Sets the Direction setting for this <see cref="FireWeaponHelper"/>.
		/// direction of impulse in gun frame.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Direction
		{
			set { SetArgument("direction", value); }
		}

		/// <summary>
		/// Sets the Split setting for this <see cref="FireWeaponHelper"/>.
		/// Split force between hand and clavicle when applyFireGunForceAtClavicle is true. 1 = all hand, 0 = all clavicle.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Split
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("split", value);
			}
		}
	}

	/// <summary>
	/// One shot to give state of constraints on character and response to constraints.
	/// </summary>
	public sealed class ConfigureConstraintsHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ConfigureConstraintsHelper for sending a ConfigureConstraints <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ConfigureConstraints <see cref="Message"/> to.</param>
		/// <remarks>
		/// One shot to give state of constraints on character and response to constraints.
		/// </remarks>
		public ConfigureConstraintsHelper(Ped ped) : base(ped, "configureConstraints")
		{
		}

		/// <summary>
		/// Sets the HandCuffs setting for this <see cref="ConfigureConstraintsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool HandCuffs
		{
			set { SetArgument("handCuffs", value); }
		}

		/// <summary>
		/// Sets the HandCuffsBehindBack setting for this <see cref="ConfigureConstraintsHelper"/>.
		/// not implemented.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool HandCuffsBehindBack
		{
			set { SetArgument("handCuffsBehindBack", value); }
		}

		/// <summary>
		/// Sets the LegCuffs setting for this <see cref="ConfigureConstraintsHelper"/>.
		/// not implemented.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool LegCuffs
		{
			set { SetArgument("legCuffs", value); }
		}

		/// <summary>
		/// Sets the RightDominant setting for this <see cref="ConfigureConstraintsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool RightDominant
		{
			set { SetArgument("rightDominant", value); }
		}

		/// <summary>
		/// Sets the PassiveMode setting for this <see cref="ConfigureConstraintsHelper"/>.
		/// 0 setCurrent, 1= IK to dominant, (2=pointGunLikeIK //not implemented).
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 5.
		/// </remarks>
		public int PassiveMode
		{
			set
			{
				if (value > 5)
					value = 5;
				if (value < 0)
					value = 0;
				SetArgument("passiveMode", value);
			}
		}

		/// <summary>
		/// Sets the BespokeBehaviour setting for this <see cref="ConfigureConstraintsHelper"/>.
		/// not implemented.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool BespokeBehaviour
		{
			set { SetArgument("bespokeBehaviour", value); }
		}

		/// <summary>
		/// Sets the Blend2ZeroPose setting for this <see cref="ConfigureConstraintsHelper"/>.
		/// Blend Arms to zero pose.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Blend2ZeroPose
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("blend2ZeroPose", value);
			}
		}
	}

	public sealed class StayUprightHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the StayUprightHelper for sending a StayUpright <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the StayUpright <see cref="Message"/> to.</param>
		public StayUprightHelper(Ped ped) : base(ped, "stayUpright")
		{
		}

		/// <summary>
		/// Sets the UseForces setting for this <see cref="StayUprightHelper"/>.
		/// enable force based constraint.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseForces
		{
			set { SetArgument("useForces", value); }
		}

		/// <summary>
		/// Sets the UseTorques setting for this <see cref="StayUprightHelper"/>.
		/// enable torque based constraint.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseTorques
		{
			set { SetArgument("useTorques", value); }
		}

		/// <summary>
		/// Sets the LastStandMode setting for this <see cref="StayUprightHelper"/>.
		/// Uses position/orientation control on the spine and drifts in the direction of bullets.  This ignores all other stayUpright settings.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool LastStandMode
		{
			set { SetArgument("lastStandMode", value); }
		}

		/// <summary>
		/// Sets the LastStandSinkRate setting for this <see cref="StayUprightHelper"/>.
		/// The sink rate (higher for a faster drop).
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LastStandSinkRate
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lastStandSinkRate", value);
			}
		}

		/// <summary>
		/// Sets the LastStandHorizDamping setting for this <see cref="StayUprightHelper"/>.
		/// Higher values for more damping.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LastStandHorizDamping
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lastStandHorizDamping", value);
			}
		}

		/// <summary>
		/// Sets the LastStandMaxTime setting for this <see cref="StayUprightHelper"/>.
		/// Max time allowed in last stand mode.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float LastStandMaxTime
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lastStandMaxTime", value);
			}
		}

		/// <summary>
		/// Sets the TurnTowardsBullets setting for this <see cref="StayUprightHelper"/>.
		/// Use cheat torques to face the direction of bullets if not facing too far away.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool TurnTowardsBullets
		{
			set { SetArgument("turnTowardsBullets", value); }
		}

		/// <summary>
		/// Sets the VelocityBased setting for this <see cref="StayUprightHelper"/>.
		/// make strength of constraint function of COM velocity.  Uses -1 for forceDamping if the damping is positive.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool VelocityBased
		{
			set { SetArgument("velocityBased", value); }
		}

		/// <summary>
		/// Sets the TorqueOnlyInAir setting for this <see cref="StayUprightHelper"/>.
		/// only apply torque based constraint when airBorne.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool TorqueOnlyInAir
		{
			set { SetArgument("torqueOnlyInAir", value); }
		}

		/// <summary>
		/// Sets the ForceStrength setting for this <see cref="StayUprightHelper"/>.
		/// strength of constraint.
		/// </summary>
		/// <remarks>
		/// Default value = 3.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ForceStrength
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("forceStrength", value);
			}
		}

		/// <summary>
		/// Sets the ForceDamping setting for this <see cref="StayUprightHelper"/>.
		/// damping in constraint: -1 makes it scale automagically with forceStrength.  Other negative values will scale this automagic damping.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 50.0f.
		/// </remarks>
		public float ForceDamping
		{
			set
			{
				if (value > 50.0f)
					value = 50.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("forceDamping", value);
			}
		}

		/// <summary>
		/// Sets the ForceFeetMult setting for this <see cref="StayUprightHelper"/>.
		/// multiplier to the force applied to the feet.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ForceFeetMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("forceFeetMult", value);
			}
		}

		/// <summary>
		/// Sets the ForceSpine3Share setting for this <see cref="StayUprightHelper"/>.
		/// share of pelvis force applied to spine3.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ForceSpine3Share
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("forceSpine3Share", value);
			}
		}

		/// <summary>
		/// Sets the ForceLeanReduction setting for this <see cref="StayUprightHelper"/>.
		/// how much the character lean is taken into account when reducing the force.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ForceLeanReduction
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("forceLeanReduction", value);
			}
		}

		/// <summary>
		/// Sets the ForceInAirShare setting for this <see cref="StayUprightHelper"/>.
		/// share of the feet force to the airborne foot.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ForceInAirShare
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("forceInAirShare", value);
			}
		}

		/// <summary>
		/// Sets the ForceMin setting for this <see cref="StayUprightHelper"/>.
		/// when min and max are greater than 0 the constraint strength is determined from character strength, scaled into the range given by min and max.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ForceMin
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("forceMin", value);
			}
		}

		/// <summary>
		/// Sets the ForceMax setting for this <see cref="StayUprightHelper"/>.
		/// see above.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ForceMax
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("forceMax", value);
			}
		}

		/// <summary>
		/// Sets the ForceSaturationVel setting for this <see cref="StayUprightHelper"/>.
		/// when in velocityBased mode, the COM velocity at which constraint reaches maximum strength (forceStrength).
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.1f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ForceSaturationVel
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("forceSaturationVel", value);
			}
		}

		/// <summary>
		/// Sets the ForceThresholdVel setting for this <see cref="StayUprightHelper"/>.
		/// when in velocityBased mode, the COM velocity above which constraint starts applying forces.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float ForceThresholdVel
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("forceThresholdVel", value);
			}
		}

		/// <summary>
		/// Sets the TorqueStrength setting for this <see cref="StayUprightHelper"/>.
		/// strength of torque based constraint.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float TorqueStrength
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torqueStrength", value);
			}
		}

		/// <summary>
		/// Sets the TorqueDamping setting for this <see cref="StayUprightHelper"/>.
		/// damping of torque based constraint.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float TorqueDamping
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torqueDamping", value);
			}
		}

		/// <summary>
		/// Sets the TorqueSaturationVel setting for this <see cref="StayUprightHelper"/>.
		/// when in velocityBased mode, the COM velocity at which constraint reaches maximum strength (torqueStrength).
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.1f.
		/// Max value = 10.0f.
		/// </remarks>
		public float TorqueSaturationVel
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("torqueSaturationVel", value);
			}
		}

		/// <summary>
		/// Sets the TorqueThresholdVel setting for this <see cref="StayUprightHelper"/>.
		/// when in velocityBased mode, the COM velocity above which constraint starts applying torques.
		/// </summary>
		/// <remarks>
		/// Default value = 2.5f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float TorqueThresholdVel
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("torqueThresholdVel", value);
			}
		}

		/// <summary>
		/// Sets the SupportPosition setting for this <see cref="StayUprightHelper"/>.
		/// distance the foot is behind Com projection that is still considered able to generate the support for the upright constraint.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = -2.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float SupportPosition
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -2.0f)
					value = -2.0f;
				SetArgument("supportPosition", value);
			}
		}

		/// <summary>
		/// Sets the NoSupportForceMult setting for this <see cref="StayUprightHelper"/>.
		/// still apply this fraction of the upright constaint force if the foot is not in a position (defined by supportPosition) to generate the support for the upright constraint.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float NoSupportForceMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("noSupportForceMult", value);
			}
		}

		/// <summary>
		/// Sets the StepUpHelp setting for this <see cref="StayUprightHelper"/>.
		/// strength of cheat force applied upwards to spine3 to help the character up steps/slopes.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float StepUpHelp
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stepUpHelp", value);
			}
		}

		/// <summary>
		/// Sets the StayUpAcc setting for this <see cref="StayUprightHelper"/>.
		/// How much the cheat force takes into account the acceleration of moving platforms.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float StayUpAcc
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stayUpAcc", value);
			}
		}

		/// <summary>
		/// Sets the StayUpAccMax setting for this <see cref="StayUprightHelper"/>.
		/// The maximum floorAcceleration (of a moving platform) that the cheat force takes into account.
		/// </summary>
		/// <remarks>
		/// Default value = 5.0f.
		/// Min value = 0.0f.
		/// Max value = 15.0f.
		/// </remarks>
		public float StayUpAccMax
		{
			set
			{
				if (value > 15.0f)
					value = 15.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stayUpAccMax", value);
			}
		}
	}

	/// <summary>
	/// Send this message to immediately stop all behaviours from executing.
	/// </summary>
	public sealed class StopAllBehavioursHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the StopAllBehavioursHelper for sending a StopAllBehaviours <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the StopAllBehaviours <see cref="Message"/> to.</param>
		/// <remarks>
		/// Send this message to immediately stop all behaviours from executing.
		/// </remarks>
		public StopAllBehavioursHelper(Ped ped) : base(ped, "stopAllBehaviours")
		{
		}
	}

	/// <summary>
	/// Sets character's strength on the dead-granny-to-healthy-terminator scale: [0..1].
	/// </summary>
	public sealed class SetCharacterStrengthHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the SetCharacterStrengthHelper for sending a SetCharacterStrength <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the SetCharacterStrength <see cref="Message"/> to.</param>
		/// <remarks>
		/// Sets character's strength on the dead-granny-to-healthy-terminator scale: [0..1].
		/// </remarks>
		public SetCharacterStrengthHelper(Ped ped) : base(ped, "setCharacterStrength")
		{
		}

		/// <summary>
		/// Sets the CharacterStrength setting for this <see cref="SetCharacterStrengthHelper"/>.
		/// strength of character.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CharacterStrength
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("characterStrength", value);
			}
		}
	}

	/// <summary>
	/// Sets character's health on the dead-to-alive scale: [0..1].
	/// </summary>
	public sealed class SetCharacterHealthHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the SetCharacterHealthHelper for sending a SetCharacterHealth <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the SetCharacterHealth <see cref="Message"/> to.</param>
		/// <remarks>
		/// Sets character's health on the dead-to-alive scale: [0..1].
		/// </remarks>
		public SetCharacterHealthHelper(Ped ped) : base(ped, "setCharacterHealth")
		{
		}

		/// <summary>
		/// Sets the CharacterHealth setting for this <see cref="SetCharacterHealthHelper"/>.
		/// health of character.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CharacterHealth
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("characterHealth", value);
			}
		}
	}

	/// <summary>
	/// Sets the type of reaction if catchFall is called.
	/// </summary>
	public sealed class SetFallingReactionHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the SetFallingReactionHelper for sending a SetFallingReaction <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the SetFallingReaction <see cref="Message"/> to.</param>
		/// <remarks>
		/// Sets the type of reaction if catchFall is called.
		/// </remarks>
		public SetFallingReactionHelper(Ped ped) : base(ped, "setFallingReaction")
		{
		}

		/// <summary>
		/// Sets the HandsAndKnees setting for this <see cref="SetFallingReactionHelper"/>.
		/// set to true to get handsAndKnees catchFall if catchFall called. If true allows the dynBalancer to stay on during the catchfall and modifies the catch fall to give a more alive looking performance (hands and knees for front landing or sitting up for back landing).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool HandsAndKnees
		{
			set { SetArgument("handsAndKnees", value); }
		}

		/// <summary>
		/// Sets the CallRDS setting for this <see cref="SetFallingReactionHelper"/>.
		/// If true catchFall will call rollDownstairs if comVel GT comVelRDSThresh - prevents excessive sliding in catchFall.  Was previously only true for handsAndKnees.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool CallRDS
		{
			set { SetArgument("callRDS", value); }
		}

		/// <summary>
		/// Sets the ComVelRDSThresh setting for this <see cref="SetFallingReactionHelper"/>.
		/// comVel above which rollDownstairs will start - prevents excessive sliding in catchFall.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float ComVelRDSThresh
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("comVelRDSThresh", value);
			}
		}

		/// <summary>
		/// Sets the ResistRolling setting for this <see cref="SetFallingReactionHelper"/>.
		/// For rds catchFall only: True to resist rolling motion (rolling motion is set off by ub contact and a sliding velocity), false to allow more of a continuous rolling  (rolling motion is set off at a sliding velocity).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ResistRolling
		{
			set { SetArgument("resistRolling", value); }
		}

		/// <summary>
		/// Sets the ArmReduceSpeed setting for this <see cref="SetFallingReactionHelper"/>.
		/// Strength is reduced in the catchFall when the arms contact the ground.  0.2 is good for handsAndKnees.  2.5 is good for normal catchFall, anything lower than 1.0 for normal catchFall may lead to bad catchFall poses.
		/// </summary>
		/// <remarks>
		/// Default value = 2.5f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ArmReduceSpeed
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armReduceSpeed", value);
			}
		}

		/// <summary>
		/// Sets the ReachLengthMultiplier setting for this <see cref="SetFallingReactionHelper"/>.
		/// Reach length multiplier that scales characters arm topological length, value in range from (0, 1 GT  where 1.0 means reach length is maximum.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.3f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ReachLengthMultiplier
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.3f)
					value = 0.3f;
				SetArgument("reachLengthMultiplier", value);
			}
		}

		/// <summary>
		/// Sets the InhibitRollingTime setting for this <see cref="SetFallingReactionHelper"/>.
		/// Time after hitting ground that the catchFall can call rds.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float InhibitRollingTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("inhibitRollingTime", value);
			}
		}

		/// <summary>
		/// Sets the ChangeFrictionTime setting for this <see cref="SetFallingReactionHelper"/>.
		/// Time after hitting ground that the catchFall can change the friction of parts to inhibit sliding.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ChangeFrictionTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("changeFrictionTime", value);
			}
		}

		/// <summary>
		/// Sets the GroundFriction setting for this <see cref="SetFallingReactionHelper"/>.
		/// 8.0 was used on yanked) Friction multiplier on bodyParts when on ground.  Character can look too slidy with groundFriction = 1.  Higher values give a more jerky reation but this seems timestep dependent especially for dragged by the feet.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GroundFriction
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("groundFriction", value);
			}
		}

		/// <summary>
		/// Sets the FrictionMin setting for this <see cref="SetFallingReactionHelper"/>.
		/// Min Friction of an impact with a body part (not head, hands or feet) - to increase friction of slippy environment to get character to roll better.  Applied in catchFall and rollUp(rollDownStairs).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float FrictionMin
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("frictionMin", value);
			}
		}

		/// <summary>
		/// Sets the FrictionMax setting for this <see cref="SetFallingReactionHelper"/>.
		/// Max Friction of an impact with a body part (not head, hands or feet) - to increase friction of slippy environment to get character to roll better.  Applied in catchFall and rollUp(rollDownStairs).
		/// </summary>
		/// <remarks>
		/// Default value = 9999.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float FrictionMax
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("frictionMax", value);
			}
		}

		/// <summary>
		/// Sets the StopOnSlopes setting for this <see cref="SetFallingReactionHelper"/>.
		/// Apply tactics to help stop on slopes.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool StopOnSlopes
		{
			set { SetArgument("stopOnSlopes", value); }
		}

		/// <summary>
		/// Sets the StopManual setting for this <see cref="SetFallingReactionHelper"/>.
		/// Override slope value to manually force stopping on flat ground.  Encourages character to come to rest face down or face up.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float StopManual
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stopManual", value);
			}
		}

		/// <summary>
		/// Sets the StoppedStrengthDecay setting for this <see cref="SetFallingReactionHelper"/>.
		/// Speed at which strength reduces when stopped.
		/// </summary>
		/// <remarks>
		/// Default value = 5.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float StoppedStrengthDecay
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stoppedStrengthDecay", value);
			}
		}

		/// <summary>
		/// Sets the SpineLean1Offset setting for this <see cref="SetFallingReactionHelper"/>.
		/// Bias spine post towards hunched (away from arched).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SpineLean1Offset
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineLean1Offset", value);
			}
		}

		/// <summary>
		/// Sets the RiflePose setting for this <see cref="SetFallingReactionHelper"/>.
		/// Hold rifle in a safe position to reduce complications with collision.  Only applied if holding a rifle.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool RiflePose
		{
			set { SetArgument("riflePose", value); }
		}

		/// <summary>
		/// Sets the HkHeadAvoid setting for this <see cref="SetFallingReactionHelper"/>.
		/// Enable head ground avoidance when handsAndKnees is true.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool HkHeadAvoid
		{
			set { SetArgument("hkHeadAvoid", value); }
		}

		/// <summary>
		/// Sets the AntiPropClav setting for this <see cref="SetFallingReactionHelper"/>.
		/// Discourage the character getting stuck propped up by elbows when falling backwards - by inhibiting backwards moving clavicles (keeps the arms slightly wider).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AntiPropClav
		{
			set { SetArgument("antiPropClav", value); }
		}

		/// <summary>
		/// Sets the AntiPropWeak setting for this <see cref="SetFallingReactionHelper"/>.
		/// Discourage the character getting stuck propped up by elbows when falling backwards - by weakening the arms as soon they hit the floor.  (Also stops the hands lifting up when flat on back).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AntiPropWeak
		{
			set { SetArgument("antiPropWeak", value); }
		}

		/// <summary>
		/// Sets the HeadAsWeakAsArms setting for this <see cref="SetFallingReactionHelper"/>.
		/// Head weakens as arms weaken. If false and antiPropWeak when falls onto back doesn't loosen neck so early (matches bodyStrength instead).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool HeadAsWeakAsArms
		{
			set { SetArgument("headAsWeakAsArms", value); }
		}

		/// <summary>
		/// Sets the SuccessStrength setting for this <see cref="SetFallingReactionHelper"/>.
		/// When bodyStrength is less than successStrength send a success feedback - DO NOT GO OUTSIDE MIN/MAX PARAMETER VALUES OTHERWISE NO SUCCESS FEEDBACK WILL BE SENT.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.3f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SuccessStrength
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.3f)
					value = 0.3f;
				SetArgument("successStrength", value);
			}
		}
	}

	/// <summary>
	/// Sets viscosity applied to damping limbs.
	/// </summary>
	public sealed class SetCharacterUnderwaterHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the SetCharacterUnderwaterHelper for sending a SetCharacterUnderwater <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the SetCharacterUnderwater <see cref="Message"/> to.</param>
		/// <remarks>
		/// Sets viscosity applied to damping limbs.
		/// </remarks>
		public SetCharacterUnderwaterHelper(Ped ped) : base(ped, "setCharacterUnderwater")
		{
		}

		/// <summary>
		/// Sets the Underwater setting for this <see cref="SetCharacterUnderwaterHelper"/>.
		/// is character underwater?.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Underwater
		{
			set { SetArgument("underwater", value); }
		}

		/// <summary>
		/// Sets the Viscosity setting for this <see cref="SetCharacterUnderwaterHelper"/>.
		/// viscosity applied to character's parts.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float Viscosity
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("viscosity", value);
			}
		}

		/// <summary>
		/// Sets the GravityFactor setting for this <see cref="SetCharacterUnderwaterHelper"/>.
		/// gravity factor applied to character.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = -10.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GravityFactor
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("gravityFactor", value);
			}
		}

		/// <summary>
		/// Sets the Stroke setting for this <see cref="SetCharacterUnderwaterHelper"/>.
		/// swimming force applied to character as a function of handVelocity and footVelocity.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1000.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float Stroke
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < -1000.0f)
					value = -1000.0f;
				SetArgument("stroke", value);
			}
		}

		/// <summary>
		/// Sets the LinearStroke setting for this <see cref="SetCharacterUnderwaterHelper"/>.
		/// swimming force (linearStroke=true,False) = (f(v),f(v*v)).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool LinearStroke
		{
			set { SetArgument("linearStroke", value); }
		}
	}

	/// <summary>
	/// setCharacterCollisions:.
	/// </summary>
	public sealed class SetCharacterCollisionsHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the SetCharacterCollisionsHelper for sending a SetCharacterCollisions <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the SetCharacterCollisions <see cref="Message"/> to.</param>
		/// <remarks>
		/// setCharacterCollisions:.
		/// </remarks>
		public SetCharacterCollisionsHelper(Ped ped) : base(ped, "setCharacterCollisions")
		{
		}

		/// <summary>
		/// Sets the Spin setting for this <see cref="SetCharacterCollisionsHelper"/>.
		/// sliding friction turned into spin 80.0 (used in demo videos) good for rest of default params below.  If 0.0 then no collision enhancement.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float Spin
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spin", value);
			}
		}

		/// <summary>
		/// Sets the MaxVelocity setting for this <see cref="SetCharacterCollisionsHelper"/>.
		/// torque = spin*(relative velocity) up to this maximum for relative velocity.
		/// </summary>
		/// <remarks>
		/// Default value = 8.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float MaxVelocity
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxVelocity", value);
			}
		}

		/// <summary>
		/// Sets the ApplyToAll setting for this <see cref="SetCharacterCollisionsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ApplyToAll
		{
			set { SetArgument("applyToAll", value); }
		}

		/// <summary>
		/// Sets the ApplyToSpine setting for this <see cref="SetCharacterCollisionsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ApplyToSpine
		{
			set { SetArgument("applyToSpine", value); }
		}

		/// <summary>
		/// Sets the ApplyToThighs setting for this <see cref="SetCharacterCollisionsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ApplyToThighs
		{
			set { SetArgument("applyToThighs", value); }
		}

		/// <summary>
		/// Sets the ApplyToClavicles setting for this <see cref="SetCharacterCollisionsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ApplyToClavicles
		{
			set { SetArgument("applyToClavicles", value); }
		}

		/// <summary>
		/// Sets the ApplyToUpperArms setting for this <see cref="SetCharacterCollisionsHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ApplyToUpperArms
		{
			set { SetArgument("applyToUpperArms", value); }
		}

		/// <summary>
		/// Sets the FootSlip setting for this <see cref="SetCharacterCollisionsHelper"/>.
		/// allow foot slipping if collided.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool FootSlip
		{
			set { SetArgument("footSlip", value); }
		}

		/// <summary>
		/// Sets the VehicleClass setting for this <see cref="SetCharacterCollisionsHelper"/>.
		/// ClassType of the object against which to enhance the collision.  All character vehicle interaction (e.g. braceForImpact glancing spins) relies on this value so EDIT WISELY. If it is used for things other than vehicles then NM should be informed.
		/// </summary>
		/// <remarks>
		/// Default value = 15.
		/// Min value = 0.
		/// Max value = 100.
		/// </remarks>
		public int VehicleClass
		{
			set
			{
				if (value > 100)
					value = 100;
				if (value < 0)
					value = 0;
				SetArgument("vehicleClass", value);
			}
		}
	}

	/// <summary>
	/// Damp out cartwheeling and somersaulting above a certain threshold.
	/// </summary>
	public sealed class SetCharacterDampingHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the SetCharacterDampingHelper for sending a SetCharacterDamping <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the SetCharacterDamping <see cref="Message"/> to.</param>
		/// <remarks>
		/// Damp out cartwheeling and somersaulting above a certain threshold.
		/// </remarks>
		public SetCharacterDampingHelper(Ped ped) : base(ped, "setCharacterDamping")
		{
		}

		/// <summary>
		/// Sets the SomersaultThresh setting for this <see cref="SetCharacterDampingHelper"/>.
		/// Somersault AngularMomentum measure above which we start damping - try 34.0.  Falling over straight backwards gives 54 on hitting ground.
		/// </summary>
		/// <remarks>
		/// Default value = 34.0f.
		/// Min value = 0.0f.
		/// Max value = 200.0f.
		/// </remarks>
		public float SomersaultThresh
		{
			set
			{
				if (value > 200.0f)
					value = 200.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("somersaultThresh", value);
			}
		}

		/// <summary>
		/// Sets the SomersaultDamp setting for this <see cref="SetCharacterDampingHelper"/>.
		/// Amount to damp somersaulting by (spinning around left/right axis) - try 0.45.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float SomersaultDamp
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("somersaultDamp", value);
			}
		}

		/// <summary>
		/// Sets the CartwheelThresh setting for this <see cref="SetCharacterDampingHelper"/>.
		/// Cartwheel AngularMomentum measure above which we start damping - try 27.0.
		/// </summary>
		/// <remarks>
		/// Default value = 27.0f.
		/// Min value = 0.0f.
		/// Max value = 200.0f.
		/// </remarks>
		public float CartwheelThresh
		{
			set
			{
				if (value > 200.0f)
					value = 200.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("cartwheelThresh", value);
			}
		}

		/// <summary>
		/// Sets the CartwheelDamp setting for this <see cref="SetCharacterDampingHelper"/>.
		/// Amount to damp somersaulting by (spinning around front/back axis) - try 0.8.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float CartwheelDamp
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("cartwheelDamp", value);
			}
		}

		/// <summary>
		/// Sets the VehicleCollisionTime setting for this <see cref="SetCharacterDampingHelper"/>.
		/// Time after impact with a vehicle to apply characterDamping. -ve values mean always apply whether collided with vehicle or not. =0.0 never apply. =timestep apply for only that frame.  A typical roll from being hit by a car lasts about 4secs.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float VehicleCollisionTime
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("vehicleCollisionTime", value);
			}
		}

		/// <summary>
		/// Sets the V2 setting for this <see cref="SetCharacterDampingHelper"/>.
		/// If true damping is proportional to Angular momentum squared.  If false proportional to Angular momentum.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool V2
		{
			set { SetArgument("v2", value); }
		}
	}

	/// <summary>
	/// setFrictionScale:.
	/// </summary>
	public sealed class SetFrictionScaleHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the SetFrictionScaleHelper for sending a SetFrictionScale <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the SetFrictionScale <see cref="Message"/> to.</param>
		/// <remarks>
		/// setFrictionScale:.
		/// </remarks>
		public SetFrictionScaleHelper(Ped ped) : base(ped, "setFrictionScale")
		{
		}

		/// <summary>
		/// Sets the Scale setting for this <see cref="SetFrictionScaleHelper"/>.
		/// Friction scale to be applied to parts in mask.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float Scale
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("scale", value);
			}
		}

		/// <summary>
		/// Sets the GlobalMin setting for this <see cref="SetFrictionScaleHelper"/>.
		/// Character-wide minimum impact friction. Affects all parts (not just those in mask).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1000000.0f.
		/// </remarks>
		public float GlobalMin
		{
			set
			{
				if (value > 1000000.0f)
					value = 1000000.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("globalMin", value);
			}
		}

		/// <summary>
		/// Sets the GlobalMax setting for this <see cref="SetFrictionScaleHelper"/>.
		/// Character-wide maximum impact friction. Affects all parts (not just those in mask).
		/// </summary>
		/// <remarks>
		/// Default value = 999999.0f.
		/// Min value = 0.0f.
		/// Max value = 1000000.0f.
		/// </remarks>
		public float GlobalMax
		{
			set
			{
				if (value > 1000000.0f)
					value = 1000000.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("globalMax", value);
			}
		}

		/// <summary>
		/// Sets the Mask setting for this <see cref="SetFrictionScaleHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see Active Pose notes for possible values).
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string Mask
		{
			set { SetArgument("mask", value); }
		}
	}

	public sealed class AnimPoseHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the AnimPoseHelper for sending a AnimPose <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the AnimPose <see cref="Message"/> to.</param>
		public AnimPoseHelper(Ped ped) : base(ped, "animPose")
		{
		}

		/// <summary>
		/// Sets the MuscleStiffness setting for this <see cref="AnimPoseHelper"/>.
		/// muscleStiffness of masked joints. -values mean don't apply (just use defaults or ones applied by behaviours - safer if you are going to return to a behaviour).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.1f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MuscleStiffness
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -1.1f)
					value = -1.1f;
				SetArgument("muscleStiffness", value);
			}
		}

		/// <summary>
		/// Sets the Stiffness setting for this <see cref="AnimPoseHelper"/>.
		/// stiffness of masked joints. -ve values mean don't apply stiffness or damping (just use defaults or ones applied by behaviours).  If you are using animpose fullbody on its own then this gives the opprtunity to use setStffness and setMuscleStiffness messages to set up the character's muscles. mmmmtodo get rid of this -ve.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.1f.
		/// Max value = 16.0f.
		/// </remarks>
		public float Stiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < -1.1f)
					value = -1.1f;
				SetArgument("stiffness", value);
			}
		}

		/// <summary>
		/// Sets the Damping setting for this <see cref="AnimPoseHelper"/>.
		/// damping of masked joints.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float Damping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("damping", value);
			}
		}

		/// <summary>
		/// Sets the EffectorMask setting for this <see cref="AnimPoseHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see notes for explanation).
		/// </summary>
		/// <remarks>
		/// Default value = ub.
		/// </remarks>
		public string EffectorMask
		{
			set { SetArgument("effectorMask", value); }
		}

		/// <summary>
		/// Sets the OverideHeadlook setting for this <see cref="AnimPoseHelper"/>.
		/// overide Headlook behaviour (if animPose includes the head).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool OverideHeadlook
		{
			set { SetArgument("overideHeadlook", value); }
		}

		/// <summary>
		/// Sets the OveridePointArm setting for this <see cref="AnimPoseHelper"/>.
		/// overide PointArm behaviour (if animPose includes the arm/arms).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool OveridePointArm
		{
			set { SetArgument("overidePointArm", value); }
		}

		/// <summary>
		/// Sets the OveridePointGun setting for this <see cref="AnimPoseHelper"/>.
		/// overide PointGun behaviour (if animPose includes the arm/arms)//mmmmtodo not used at moment.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool OveridePointGun
		{
			set { SetArgument("overidePointGun", value); }
		}

		/// <summary>
		/// Sets the UseZMPGravityCompensation setting for this <see cref="AnimPoseHelper"/>.
		/// If true then modify gravity compensation based on stance (can reduce gravity compensation to zero if cofm is outside of balance area).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseZMPGravityCompensation
		{
			set { SetArgument("useZMPGravityCompensation", value); }
		}

		/// <summary>
		/// Sets the GravityCompensation setting for this <see cref="AnimPoseHelper"/>.
		/// gravity compensation applied to joints in the effectorMask. If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 14.0f.
		/// </remarks>
		public float GravityCompensation
		{
			set
			{
				if (value > 14.0f)
					value = 14.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("gravityCompensation", value);
			}
		}

		/// <summary>
		/// Sets the MuscleStiffnessLeftArm setting for this <see cref="AnimPoseHelper"/>.
		/// muscle stiffness applied to left arm (applied after stiffness). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MuscleStiffnessLeftArm
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("muscleStiffnessLeftArm", value);
			}
		}

		/// <summary>
		/// Sets the MuscleStiffnessRightArm setting for this <see cref="AnimPoseHelper"/>.
		/// muscle stiffness applied to right arm (applied after stiffness). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MuscleStiffnessRightArm
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("muscleStiffnessRightArm", value);
			}
		}

		/// <summary>
		/// Sets the MuscleStiffnessSpine setting for this <see cref="AnimPoseHelper"/>.
		/// muscle stiffness applied to spine (applied after stiffness). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MuscleStiffnessSpine
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("muscleStiffnessSpine", value);
			}
		}

		/// <summary>
		/// Sets the MuscleStiffnessLeftLeg setting for this <see cref="AnimPoseHelper"/>.
		/// muscle stiffness applied to left leg (applied after stiffness). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MuscleStiffnessLeftLeg
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("muscleStiffnessLeftLeg", value);
			}
		}

		/// <summary>
		/// Sets the MuscleStiffnessRightLeg setting for this <see cref="AnimPoseHelper"/>.
		/// muscle stiffness applied to right leg (applied after stiffness). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MuscleStiffnessRightLeg
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("muscleStiffnessRightLeg", value);
			}
		}

		/// <summary>
		/// Sets the StiffnessLeftArm setting for this <see cref="AnimPoseHelper"/>.
		/// stiffness  applied to left arm (applied after stiffness). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float StiffnessLeftArm
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("stiffnessLeftArm", value);
			}
		}

		/// <summary>
		/// Sets the StiffnessRightArm setting for this <see cref="AnimPoseHelper"/>.
		/// stiffness applied to right arm (applied after stiffness). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float StiffnessRightArm
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("stiffnessRightArm", value);
			}
		}

		/// <summary>
		/// Sets the StiffnessSpine setting for this <see cref="AnimPoseHelper"/>.
		/// stiffness applied to spine (applied after stiffness). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float StiffnessSpine
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("stiffnessSpine", value);
			}
		}

		/// <summary>
		/// Sets the StiffnessLeftLeg setting for this <see cref="AnimPoseHelper"/>.
		/// stiffness applied to left leg (applied after stiffness). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float StiffnessLeftLeg
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("stiffnessLeftLeg", value);
			}
		}

		/// <summary>
		/// Sets the StiffnessRightLeg setting for this <see cref="AnimPoseHelper"/>.
		/// stiffness applied to right leg (applied after stiffness). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float StiffnessRightLeg
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("stiffnessRightLeg", value);
			}
		}

		/// <summary>
		/// Sets the DampingLeftArm setting for this <see cref="AnimPoseHelper"/>.
		/// damping applied to left arm (applied after stiffness). If stiffness -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float DampingLeftArm
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dampingLeftArm", value);
			}
		}

		/// <summary>
		/// Sets the DampingRightArm setting for this <see cref="AnimPoseHelper"/>.
		/// damping applied to right arm (applied after stiffness). If stiffness -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float DampingRightArm
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dampingRightArm", value);
			}
		}

		/// <summary>
		/// Sets the DampingSpine setting for this <see cref="AnimPoseHelper"/>.
		/// damping applied to spine (applied after stiffness). If stiffness-ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float DampingSpine
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dampingSpine", value);
			}
		}

		/// <summary>
		/// Sets the DampingLeftLeg setting for this <see cref="AnimPoseHelper"/>.
		/// damping applied to left leg (applied after stiffness). If stiffness-ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float DampingLeftLeg
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dampingLeftLeg", value);
			}
		}

		/// <summary>
		/// Sets the DampingRightLeg setting for this <see cref="AnimPoseHelper"/>.
		/// damping applied to right leg (applied after stiffness). If stiffness -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float DampingRightLeg
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dampingRightLeg", value);
			}
		}

		/// <summary>
		/// Sets the GravCompLeftArm setting for this <see cref="AnimPoseHelper"/>.
		/// gravity compensation applied to left arm (applied after gravityCompensation). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 14.0f.
		/// </remarks>
		public float GravCompLeftArm
		{
			set
			{
				if (value > 14.0f)
					value = 14.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("gravCompLeftArm", value);
			}
		}

		/// <summary>
		/// Sets the GravCompRightArm setting for this <see cref="AnimPoseHelper"/>.
		/// gravity compensation applied to right arm (applied after gravityCompensation). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 14.0f.
		/// </remarks>
		public float GravCompRightArm
		{
			set
			{
				if (value > 14.0f)
					value = 14.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("gravCompRightArm", value);
			}
		}

		/// <summary>
		/// Sets the GravCompSpine setting for this <see cref="AnimPoseHelper"/>.
		/// gravity compensation applied to spine (applied after gravityCompensation). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 14.0f.
		/// </remarks>
		public float GravCompSpine
		{
			set
			{
				if (value > 14.0f)
					value = 14.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("gravCompSpine", value);
			}
		}

		/// <summary>
		/// Sets the GravCompLeftLeg setting for this <see cref="AnimPoseHelper"/>.
		/// gravity compensation applied to left leg (applied after gravityCompensation). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 14.0f.
		/// </remarks>
		public float GravCompLeftLeg
		{
			set
			{
				if (value > 14.0f)
					value = 14.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("gravCompLeftLeg", value);
			}
		}

		/// <summary>
		/// Sets the GravCompRightLeg setting for this <see cref="AnimPoseHelper"/>.
		/// gravity compensation applied to right leg (applied after gravityCompensation). If -ve then not applied (use current setting).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 14.0f.
		/// </remarks>
		public float GravCompRightLeg
		{
			set
			{
				if (value > 14.0f)
					value = 14.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("gravCompRightLeg", value);
			}
		}

		/// <summary>
		/// Sets the ConnectedLeftHand setting for this <see cref="AnimPoseHelper"/>.
		/// Is the left hand constrained to the world/ an object: -1=auto decide by impact info, 0=no, 1=part fully constrained (not implemented:, 2=part point constraint, 3=line constraint).
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = -1.
		/// Max value = 2.
		/// </remarks>
		public int ConnectedLeftHand
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < -1)
					value = -1;
				SetArgument("connectedLeftHand", value);
			}
		}

		/// <summary>
		/// Sets the ConnectedRightHand setting for this <see cref="AnimPoseHelper"/>.
		/// Is the right hand constrained to the world/ an object: -1=auto decide by impact info, 0=no, 1=part fully constrained (not implemented:, 2=part point constraint, 3=line constraint).
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = -1.
		/// Max value = 2.
		/// </remarks>
		public int ConnectedRightHand
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < -1)
					value = -1;
				SetArgument("connectedRightHand", value);
			}
		}

		/// <summary>
		/// Sets the ConnectedLeftFoot setting for this <see cref="AnimPoseHelper"/>.
		/// Is the left foot constrained to the world/ an object: -2=do not set in animpose (e.g. let the balancer decide), -1=auto decide by impact info, 0=no, 1=part fully constrained (not implemented:, 2=part point constraint, 3=line constraint).
		/// </summary>
		/// <remarks>
		/// Default value = -2.
		/// Min value = -2.
		/// Max value = 2.
		/// </remarks>
		public int ConnectedLeftFoot
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < -2)
					value = -2;
				SetArgument("connectedLeftFoot", value);
			}
		}

		/// <summary>
		/// Sets the ConnectedRightFoot setting for this <see cref="AnimPoseHelper"/>.
		/// Is the right foot constrained to the world/ an object: -2=do not set in animpose (e.g. let the balancer decide),-1=auto decide by impact info, 0=no, 1=part fully constrained (not implemented:, 2=part point constraint, 3=line constraint).
		/// </summary>
		/// <remarks>
		/// Default value = -2.
		/// Min value = -2.
		/// Max value = 2.
		/// </remarks>
		public int ConnectedRightFoot
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < -2)
					value = -2;
				SetArgument("connectedRightFoot", value);
			}
		}

		/// <summary>
		/// Sets the AnimSource setting for this <see cref="AnimPoseHelper"/>.
		/// </summary>
		public AnimSource AnimSource
		{
			set { SetArgument("animSource", (int) value); }
		}

		/// <summary>
		/// Sets the DampenSideMotionInstanceIndex setting for this <see cref="AnimPoseHelper"/>.
		/// LevelIndex of object to dampen side motion relative to. -1 means not used.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int DampenSideMotionInstanceIndex
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("dampenSideMotionInstanceIndex", value);
			}
		}
	}

	public sealed class ArmsWindmillHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ArmsWindmillHelper for sending a ArmsWindmill <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ArmsWindmill <see cref="Message"/> to.</param>
		public ArmsWindmillHelper(Ped ped) : base(ped, "armsWindmill")
		{
		}

		/// <summary>
		/// Sets the LeftPartID setting for this <see cref="ArmsWindmillHelper"/>.
		/// ID of part that the circle uses as local space for positioning.
		/// </summary>
		/// <remarks>
		/// Default value = 10.
		/// Min value = 0.
		/// Max value = 21.
		/// </remarks>
		public int LeftPartID
		{
			set
			{
				if (value > 21)
					value = 21;
				if (value < 0)
					value = 0;
				SetArgument("leftPartID", value);
			}
		}

		/// <summary>
		/// Sets the LeftRadius1 setting for this <see cref="ArmsWindmillHelper"/>.
		/// radius for first axis of ellipse.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeftRadius1
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leftRadius1", value);
			}
		}

		/// <summary>
		/// Sets the LeftRadius2 setting for this <see cref="ArmsWindmillHelper"/>.
		/// radius for second axis of ellipse.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeftRadius2
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leftRadius2", value);
			}
		}

		/// <summary>
		/// Sets the LeftSpeed setting for this <see cref="ArmsWindmillHelper"/>.
		/// speed of target around the circle.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = -2.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float LeftSpeed
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -2.0f)
					value = -2.0f;
				SetArgument("leftSpeed", value);
			}
		}

		/// <summary>
		/// Sets the LeftNormal setting for this <see cref="ArmsWindmillHelper"/>.
		/// Euler Angles orientation of circle in space of part with part ID.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.2f, 0.2f).
		/// </remarks>
		public Vector3 LeftNormal
		{
			set { SetArgument("leftNormal", value); }
		}

		/// <summary>
		/// Sets the LeftCentre setting for this <see cref="ArmsWindmillHelper"/>.
		/// centre of circle in the space of partID.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.5f, -0.1f).
		/// </remarks>
		public Vector3 LeftCentre
		{
			set { SetArgument("leftCentre", value); }
		}

		/// <summary>
		/// Sets the RightPartID setting for this <see cref="ArmsWindmillHelper"/>.
		/// ID of part that the circle uses as local space for positioning.
		/// </summary>
		/// <remarks>
		/// Default value = 10.
		/// Min value = 0.
		/// Max value = 21.
		/// </remarks>
		public int RightPartID
		{
			set
			{
				if (value > 21)
					value = 21;
				if (value < 0)
					value = 0;
				SetArgument("rightPartID", value);
			}
		}

		/// <summary>
		/// Sets the RightRadius1 setting for this <see cref="ArmsWindmillHelper"/>.
		/// radius for first axis of ellipse.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RightRadius1
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rightRadius1", value);
			}
		}

		/// <summary>
		/// Sets the RightRadius2 setting for this <see cref="ArmsWindmillHelper"/>.
		/// radius for second axis of ellipse.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RightRadius2
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rightRadius2", value);
			}
		}

		/// <summary>
		/// Sets the RightSpeed setting for this <see cref="ArmsWindmillHelper"/>.
		/// speed of target around the circle.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = -2.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RightSpeed
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -2.0f)
					value = -2.0f;
				SetArgument("rightSpeed", value);
			}
		}

		/// <summary>
		/// Sets the RightNormal setting for this <see cref="ArmsWindmillHelper"/>.
		/// Euler Angles orientation of circle in space of part with part ID.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, -0.2f, -0.2f).
		/// </remarks>
		public Vector3 RightNormal
		{
			set { SetArgument("rightNormal", value); }
		}

		/// <summary>
		/// Sets the RightCentre setting for this <see cref="ArmsWindmillHelper"/>.
		/// centre of circle in the space of partID.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, -0.5f, -0.1f).
		/// </remarks>
		public Vector3 RightCentre
		{
			set { SetArgument("rightCentre", value); }
		}

		/// <summary>
		/// Sets the ShoulderStiffness setting for this <see cref="ArmsWindmillHelper"/>.
		/// Stiffness applied to the shoulders.
		/// </summary>
		/// <remarks>
		/// Default value = 12.0f.
		/// Min value = 1.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ShoulderStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 1.0f)
					value = 1.0f;
				SetArgument("shoulderStiffness", value);
			}
		}

		/// <summary>
		/// Sets the ShoulderDamping setting for this <see cref="ArmsWindmillHelper"/>.
		/// Damping applied to the shoulders.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ShoulderDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shoulderDamping", value);
			}
		}

		/// <summary>
		/// Sets the ElbowStiffness setting for this <see cref="ArmsWindmillHelper"/>.
		/// Stiffness applied to the elbows.
		/// </summary>
		/// <remarks>
		/// Default value = 12.0f.
		/// Min value = 1.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ElbowStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 1.0f)
					value = 1.0f;
				SetArgument("elbowStiffness", value);
			}
		}

		/// <summary>
		/// Sets the ElbowDamping setting for this <see cref="ArmsWindmillHelper"/>.
		/// Damping applied to the elbows.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ElbowDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("elbowDamping", value);
			}
		}

		/// <summary>
		/// Sets the LeftElbowMin setting for this <see cref="ArmsWindmillHelper"/>.
		/// Minimum left elbow bend.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.7f.
		/// </remarks>
		public float LeftElbowMin
		{
			set
			{
				if (value > 1.7f)
					value = 1.7f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leftElbowMin", value);
			}
		}

		/// <summary>
		/// Sets the RightElbowMin setting for this <see cref="ArmsWindmillHelper"/>.
		/// Minimum right elbow bend.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.7f.
		/// </remarks>
		public float RightElbowMin
		{
			set
			{
				if (value > 1.7f)
					value = 1.7f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rightElbowMin", value);
			}
		}

		/// <summary>
		/// Sets the PhaseOffset setting for this <see cref="ArmsWindmillHelper"/>.
		/// phase offset(degrees) when phase synchronization is turned on.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -360.0f.
		/// Max value = 360.0f.
		/// </remarks>
		public float PhaseOffset
		{
			set
			{
				if (value > 360.0f)
					value = 360.0f;
				if (value < -360.0f)
					value = -360.0f;
				SetArgument("phaseOffset", value);
			}
		}

		/// <summary>
		/// Sets the DragReduction setting for this <see cref="ArmsWindmillHelper"/>.
		/// how much to compensate for movement of character/target.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float DragReduction
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dragReduction", value);
			}
		}

		/// <summary>
		/// Sets the IKtwist setting for this <see cref="ArmsWindmillHelper"/>.
		/// angle of elbow around twist axis ?.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -3.1f.
		/// Max value = 3.1f.
		/// </remarks>
		public float IKtwist
		{
			set
			{
				if (value > 3.1f)
					value = 3.1f;
				if (value < -3.1f)
					value = -3.1f;
				SetArgument("IKtwist", value);
			}
		}

		/// <summary>
		/// Sets the AngVelThreshold setting for this <see cref="ArmsWindmillHelper"/>.
		/// value of character angular speed above which adaptive arm motion starts.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float AngVelThreshold
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("angVelThreshold", value);
			}
		}

		/// <summary>
		/// Sets the AngVelGain setting for this <see cref="ArmsWindmillHelper"/>.
		/// multiplies angular speed of character to get speed of arms.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float AngVelGain
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("angVelGain", value);
			}
		}

		/// <summary>
		/// Sets the MirrorMode setting for this <see cref="ArmsWindmillHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="MirrorMode.Mirrored"/>.
		/// If <see cref="MirrorMode.Parallel"/> leftArm parameters are used.
		/// </remarks>
		public MirrorMode MirrorMode
		{
			set { SetArgument("mirrorMode", (int) value); }
		}

		/// <summary>
		/// Sets the AdaptiveMode setting for this <see cref="ArmsWindmillHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="AdaptiveMode.NotAdaptive"/>.
		/// </remarks>
		public AdaptiveMode AdaptiveMode
		{
			set { SetArgument("adaptiveMode", (int) value); }
		}

		/// <summary>
		/// Sets the ForceSync setting for this <see cref="ArmsWindmillHelper"/>.
		/// toggles phase synchronization.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ForceSync
		{
			set { SetArgument("forceSync", value); }
		}

		/// <summary>
		/// Sets the UseLeft setting for this <see cref="ArmsWindmillHelper"/>.
		/// Use the left arm.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseLeft
		{
			set { SetArgument("useLeft", value); }
		}

		/// <summary>
		/// Sets the UseRight setting for this <see cref="ArmsWindmillHelper"/>.
		/// Use the right arm.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseRight
		{
			set { SetArgument("useRight", value); }
		}

		/// <summary>
		/// Sets the DisableOnImpact setting for this <see cref="ArmsWindmillHelper"/>.
		/// If true, each arm will stop windmilling if it hits the ground.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool DisableOnImpact
		{
			set { SetArgument("disableOnImpact", value); }
		}
	}

	public sealed class ArmsWindmillAdaptiveHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ArmsWindmillAdaptiveHelper for sending a ArmsWindmillAdaptive <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ArmsWindmillAdaptive <see cref="Message"/> to.</param>
		public ArmsWindmillAdaptiveHelper(Ped ped) : base(ped, "armsWindmillAdaptive")
		{
		}

		/// <summary>
		/// Sets the AngSpeed setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// Controls the speed of the windmilling.
		/// </summary>
		/// <remarks>
		/// Default value = 6.3f.
		/// Min value = 0.1f.
		/// Max value = 10.0f.
		/// </remarks>
		public float AngSpeed
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("angSpeed", value);
			}
		}

		/// <summary>
		/// Sets the BodyStiffness setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// Controls how stiff the rest of the body is.
		/// </summary>
		/// <remarks>
		/// Default value = 11.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float BodyStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("bodyStiffness", value);
			}
		}

		/// <summary>
		/// Sets the Amplitude setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// Controls how large the motion is, higher values means the character waves his arms in a massive arc.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float Amplitude
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("amplitude", value);
			}
		}

		/// <summary>
		/// Sets the Phase setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// Set to a non-zero value to desynchronise the left and right arms motion.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -4.0f.
		/// Max value = 8.0f.
		/// </remarks>
		public float Phase
		{
			set
			{
				if (value > 8.0f)
					value = 8.0f;
				if (value < -4.0f)
					value = -4.0f;
				SetArgument("phase", value);
			}
		}

		/// <summary>
		/// Sets the ArmStiffness setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// How stiff the arms are controls how pronounced the windmilling motion appears.
		/// </summary>
		/// <remarks>
		/// Default value = 14.1f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("armStiffness", value);
			}
		}

		/// <summary>
		/// Sets the LeftElbowAngle setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// If not negative then left arm will blend to this angle.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 6.0f.
		/// </remarks>
		public float LeftElbowAngle
		{
			set
			{
				if (value > 6.0f)
					value = 6.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("leftElbowAngle", value);
			}
		}

		/// <summary>
		/// Sets the RightElbowAngle setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// If not negative then right arm will blend to this angle.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 6.0f.
		/// </remarks>
		public float RightElbowAngle
		{
			set
			{
				if (value > 6.0f)
					value = 6.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("rightElbowAngle", value);
			}
		}

		/// <summary>
		/// Sets the Lean1mult setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// 0 arms go up and down at the side. 1 circles. 0..1 elipse.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float Lean1mult
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lean1mult", value);
			}
		}

		/// <summary>
		/// Sets the Lean1offset setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// 0.f centre of circle at side.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -6.0f.
		/// Max value = 6.0f.
		/// </remarks>
		public float Lean1offset
		{
			set
			{
				if (value > 6.0f)
					value = 6.0f;
				if (value < -6.0f)
					value = -6.0f;
				SetArgument("lean1offset", value);
			}
		}

		/// <summary>
		/// Sets the ElbowRate setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// rate at which elbow tries to match *ElbowAngle.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 6.0f.
		/// </remarks>
		public float ElbowRate
		{
			set
			{
				if (value > 6.0f)
					value = 6.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("elbowRate", value);
			}
		}

		/// <summary>
		/// Sets the ArmDirection setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="ArmDirection.Adaptive"/>.
		/// </remarks>
		public ArmDirection ArmDirection
		{
			set { SetArgument("armDirection", (int) value); }
		}

		/// <summary>
		/// Sets the DisableOnImpact setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// If true, each arm will stop windmilling if it hits the ground.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool DisableOnImpact
		{
			set { SetArgument("disableOnImpact", value); }
		}

		/// <summary>
		/// Sets the SetBackAngles setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// If true, back angles will be set to compliment arms windmill.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool SetBackAngles
		{
			set { SetArgument("setBackAngles", value); }
		}

		/// <summary>
		/// Sets the UseAngMom setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// If true, use angular momentum about com to choose arm circling direction. Otherwise use com angular velocity.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseAngMom
		{
			set { SetArgument("useAngMom", value); }
		}

		/// <summary>
		/// Sets the BendLeftElbow setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// If true, bend the left elbow to give a stuntman type scramble look.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool BendLeftElbow
		{
			set { SetArgument("bendLeftElbow", value); }
		}

		/// <summary>
		/// Sets the BendRightElbow setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// If true, bend the right elbow to give a stuntman type scramble look.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool BendRightElbow
		{
			set { SetArgument("bendRightElbow", value); }
		}

		/// <summary>
		/// Sets the Mask setting for this <see cref="ArmsWindmillAdaptiveHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see Active Pose notes for possible values).
		/// </summary>
		/// <remarks>
		/// Default value = ub.
		/// </remarks>
		public string Mask
		{
			set { SetArgument("mask", value); }
		}
	}

	public sealed class BalancerCollisionsReactionHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the BalancerCollisionsReactionHelper for sending a BalancerCollisionsReaction <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the BalancerCollisionsReaction <see cref="Message"/> to.</param>
		public BalancerCollisionsReactionHelper(Ped ped) : base(ped, "balancerCollisionsReaction")
		{
		}

		/// <summary>
		/// Sets the NumStepsTillSlump setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Begin slump and stop stepping after this many steps.
		/// </summary>
		/// <remarks>
		/// Default value = 4.
		/// Min value = 0.
		/// </remarks>
		public int NumStepsTillSlump
		{
			set
			{
				if (value < 0)
					value = 0;
				SetArgument("numStepsTillSlump", value);
			}
		}

		/// <summary>
		/// Sets the Stable2SlumpTime setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Time after becoming stable leaning against a wall that slump starts.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float Stable2SlumpTime
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stable2SlumpTime", value);
			}
		}

		/// <summary>
		/// Sets the ExclusionZone setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Steps are ihibited to not go closer to the wall than this (after impact).
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ExclusionZone
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("exclusionZone", value);
			}
		}

		/// <summary>
		/// Sets the FootFrictionMultStart setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Friction multiplier applied to feet when slump starts.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float FootFrictionMultStart
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("footFrictionMultStart", value);
			}
		}

		/// <summary>
		/// Sets the FootFrictionMultRate setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Friction multiplier reduced by this amount every second after slump starts (only if character is not slumping).
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 50.0f.
		/// </remarks>
		public float FootFrictionMultRate
		{
			set
			{
				if (value > 50.0f)
					value = 50.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("footFrictionMultRate", value);
			}
		}

		/// <summary>
		/// Sets the BackFrictionMultStart setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Friction multiplier applied to back when slump starts.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float BackFrictionMultStart
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("backFrictionMultStart", value);
			}
		}

		/// <summary>
		/// Sets the BackFrictionMultRate setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Friction multiplier reduced by this amount every second after slump starts (only if character is not slumping).
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 50.0f.
		/// </remarks>
		public float BackFrictionMultRate
		{
			set
			{
				if (value > 50.0f)
					value = 50.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("backFrictionMultRate", value);
			}
		}

		/// <summary>
		/// Sets the ImpactLegStiffReduction setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Reduce the stiffness of the legs by this much as soon as an impact is detected.
		/// </summary>
		/// <remarks>
		/// Default value = 3.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ImpactLegStiffReduction
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impactLegStiffReduction", value);
			}
		}

		/// <summary>
		/// Sets the SlumpLegStiffReduction setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Reduce the stiffness of the legs by this much as soon as slump starts.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float SlumpLegStiffReduction
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("slumpLegStiffReduction", value);
			}
		}

		/// <summary>
		/// Sets the SlumpLegStiffRate setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Rate at which the stiffness of the legs is reduced during slump.
		/// </summary>
		/// <remarks>
		/// Default value = 8.0f.
		/// Min value = 0.0f.
		/// Max value = 50.0f.
		/// </remarks>
		public float SlumpLegStiffRate
		{
			set
			{
				if (value > 50.0f)
					value = 50.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("slumpLegStiffRate", value);
			}
		}

		/// <summary>
		/// Sets the ReactTime setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Time that the character reacts to the impact with ub flinch and writhe.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ReactTime
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("reactTime", value);
			}
		}

		/// <summary>
		/// Sets the ImpactExagTime setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Time that the character exaggerates impact with spine.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ImpactExagTime
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impactExagTime", value);
			}
		}

		/// <summary>
		/// Sets the GlanceSpinTime setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Duration that the glance torque is applied for.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GlanceSpinTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("glanceSpinTime", value);
			}
		}

		/// <summary>
		/// Sets the GlanceSpinMag setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Magnitude of the glance torque.
		/// </summary>
		/// <remarks>
		/// Default value = 50.0f.
		/// Min value = 0.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float GlanceSpinMag
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("glanceSpinMag", value);
			}
		}

		/// <summary>
		/// Sets the GlanceSpinDecayMult setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// multiplier used when decaying torque spin over time.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GlanceSpinDecayMult
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("glanceSpinDecayMult", value);
			}
		}

		/// <summary>
		/// Sets the IgnoreColWithIndex setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// used so impact with the character that is pushing you over doesn't set off the behaviour.
		/// </summary>
		/// <remarks>
		/// Default value = -2.
		/// Min value = -2.
		/// </remarks>
		public int IgnoreColWithIndex
		{
			set
			{
				if (value < -2)
					value = -2;
				SetArgument("ignoreColWithIndex", value);
			}
		}

		/// <summary>
		/// Sets the SlumpMode setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// 0=Normal slump(less movement then slump and movement LT small), 1=fast slump, 2=less movement then slump.
		/// </summary>
		/// <remarks>
		/// Default value = 1.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int SlumpMode
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("slumpMode", value);
			}
		}

		/// <summary>
		/// Sets the ReboundMode setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// 0=fall2knees/slump if shot not running, 1=stumble, 2=slump, 3=restart.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 3.
		/// </remarks>
		public int ReboundMode
		{
			set
			{
				if (value > 3)
					value = 3;
				if (value < 0)
					value = 0;
				SetArgument("reboundMode", value);
			}
		}

		/// <summary>
		/// Sets the IgnoreColMassBelow setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// collisions with non-fixed objects with mass below this will not set this behaviour off (e.g. ignore guns).
		/// </summary>
		/// <remarks>
		/// Default value = 20.0f.
		/// Min value = -1.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float IgnoreColMassBelow
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("ignoreColMassBelow", value);
			}
		}

		/// <summary>
		/// Sets the ForwardMode setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// 0=slump, 1=fallToKnees if shot is running, otherwise slump.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 1.
		/// </remarks>
		public int ForwardMode
		{
			set
			{
				if (value > 1)
					value = 1;
				if (value < 0)
					value = 0;
				SetArgument("forwardMode", value);
			}
		}

		/// <summary>
		/// Sets the TimeToForward setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// time after a forwards impact before forwardMode is called (leave sometime for a rebound or brace - the min of 0.1 is to ensure fallOverWall can start although it probably needs only 1or2 frames for the probes to return).
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.1f.
		/// Max value = 2.0f.
		/// </remarks>
		public float TimeToForward
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("timeToForward", value);
			}
		}

		/// <summary>
		/// Sets the ReboundForce setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// if forwards impact only: cheat force to try to get the character away from the wall.  3 is a good value.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ReboundForce
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("reboundForce", value);
			}
		}

		/// <summary>
		/// Sets the BraceWall setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Brace against wall if forwards impact(at the moment only if bodyBalance is running/in charge of arms).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool BraceWall
		{
			set { SetArgument("braceWall", value); }
		}

		/// <summary>
		/// Sets the IgnoreColVolumeBelow setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// collisions with non-fixed objects with volume below this will not set this behaviour off.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = -1.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float IgnoreColVolumeBelow
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("ignoreColVolumeBelow", value);
			}
		}

		/// <summary>
		/// Sets the FallOverWallDrape setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// use fallOverWall as the main drape reaction.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool FallOverWallDrape
		{
			set { SetArgument("fallOverWallDrape", value); }
		}

		/// <summary>
		/// Sets the FallOverHighWalls setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// trigger fall over wall if hit up to spine2 else only if hit up to spine1.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FallOverHighWalls
		{
			set { SetArgument("fallOverHighWalls", value); }
		}

		/// <summary>
		/// Sets the Snap setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Add a Snap to when you hit a wall to emphasize the hit.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Snap
		{
			set { SetArgument("snap", value); }
		}

		/// <summary>
		/// Sets the SnapMag setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// The magnitude of the snap reaction.
		/// </summary>
		/// <remarks>
		/// Default value = -0.6f.
		/// Min value = -10.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SnapMag
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("snapMag", value);
			}
		}

		/// <summary>
		/// Sets the SnapDirectionRandomness setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// The character snaps in a prescribed way (decided by bullet direction) - Higher the value the more random this direction is.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SnapDirectionRandomness
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("snapDirectionRandomness", value);
			}
		}

		/// <summary>
		/// Sets the SnapLeftArm setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// snap the leftArm.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SnapLeftArm
		{
			set { SetArgument("snapLeftArm", value); }
		}

		/// <summary>
		/// Sets the SnapRightArm setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// snap the rightArm.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SnapRightArm
		{
			set { SetArgument("snapRightArm", value); }
		}

		/// <summary>
		/// Sets the SnapLeftLeg setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// snap the leftLeg.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SnapLeftLeg
		{
			set { SetArgument("snapLeftLeg", value); }
		}

		/// <summary>
		/// Sets the SnapRightLeg setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// snap the rightLeg.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SnapRightLeg
		{
			set { SetArgument("snapRightLeg", value); }
		}

		/// <summary>
		/// Sets the SnapSpine setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// snap the spine.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool SnapSpine
		{
			set { SetArgument("snapSpine", value); }
		}

		/// <summary>
		/// Sets the SnapNeck setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// snap the neck.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool SnapNeck
		{
			set { SetArgument("snapNeck", value); }
		}

		/// <summary>
		/// Sets the SnapPhasedLegs setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Legs are either in phase with each other or not.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool SnapPhasedLegs
		{
			set { SetArgument("snapPhasedLegs", value); }
		}

		/// <summary>
		/// Sets the SnapHipType setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// type of hip reaction 0=none, 1=side2side 2=steplike.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int SnapHipType
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("snapHipType", value);
			}
		}

		/// <summary>
		/// Sets the UnSnapInterval setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// Interval before applying reverse snap.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float UnSnapInterval
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("unSnapInterval", value);
			}
		}

		/// <summary>
		/// Sets the UnSnapRatio setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// The magnitude of the reverse snap.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float UnSnapRatio
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("unSnapRatio", value);
			}
		}

		/// <summary>
		/// Sets the SnapUseTorques setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// use torques to make the snap otherwise use a change in the parts angular velocity.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool SnapUseTorques
		{
			set { SetArgument("snapUseTorques", value); }
		}

		/// <summary>
		/// Sets the ImpactWeaknessZeroDuration setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// duration for which the character's upper body stays at minimum stiffness (not quite zero).
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ImpactWeaknessZeroDuration
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impactWeaknessZeroDuration", value);
			}
		}

		/// <summary>
		/// Sets the ImpactWeaknessRampDuration setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// duration of the ramp to bring the character's upper body stiffness back to normal levels.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ImpactWeaknessRampDuration
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impactWeaknessRampDuration", value);
			}
		}

		/// <summary>
		/// Sets the ImpactLoosenessAmount setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// how loose the character is on impact. between 0 and 1.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ImpactLoosenessAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("impactLoosenessAmount", value);
			}
		}

		/// <summary>
		/// Sets the ObjectBehindVictim setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// detected an object behind a shot victim in the direction of a bullet?.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ObjectBehindVictim
		{
			set { SetArgument("objectBehindVictim", value); }
		}

		/// <summary>
		/// Sets the ObjectBehindVictimPos setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// the intersection pos of a detected object behind a shot victim in the direction of a bullet.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 ObjectBehindVictimPos
		{
			set { SetArgument("objectBehindVictimPos", value); }
		}

		/// <summary>
		/// Sets the ObjectBehindVictimNormal setting for this <see cref="BalancerCollisionsReactionHelper"/>.
		/// the normal of a detected object behind a shot victim in the direction of a bullet.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public Vector3 ObjectBehindVictimNormal
		{
			set
			{
				SetArgument("objectBehindVictimNormal",
					Vector3.Clamp(value, new Vector3(-1.0f, -1.0f, -1.0f), new Vector3(1.0f, 1.0f, 1.0f)));
			}
		}
	}

	public sealed class BodyBalanceHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the BodyBalanceHelper for sending a BodyBalance <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the BodyBalance <see cref="Message"/> to.</param>
		public BodyBalanceHelper(Ped ped) : base(ped, "bodyBalance")
		{
		}

		/// <summary>
		/// Sets the ArmStiffness setting for this <see cref="BodyBalanceHelper"/>.
		/// NB. WAS m_bodyStiffness ClaviclesStiffness=9.0f.
		/// </summary>
		/// <remarks>
		/// Default value = 9.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("armStiffness", value);
			}
		}

		/// <summary>
		/// Sets the Elbow setting for this <see cref="BodyBalanceHelper"/>.
		/// How much the elbow swings based on the leg movement.
		/// </summary>
		/// <remarks>
		/// Default value = 0.9f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float Elbow
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("elbow", value);
			}
		}

		/// <summary>
		/// Sets the Shoulder setting for this <see cref="BodyBalanceHelper"/>.
		/// How much the shoulder(lean1) swings based on the leg movement.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float Shoulder
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shoulder", value);
			}
		}

		/// <summary>
		/// Sets the ArmDamping setting for this <see cref="BodyBalanceHelper"/>.
		/// NB. WAS m_damping NeckDamping=1 ClaviclesDamping=1.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armDamping", value);
			}
		}

		/// <summary>
		/// Sets the UseHeadLook setting for this <see cref="BodyBalanceHelper"/>.
		/// enable and provide a look-at target to make the character's head turn to face it while balancing.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseHeadLook
		{
			set { SetArgument("useHeadLook", value); }
		}

		/// <summary>
		/// Sets the HeadLookPos setting for this <see cref="BodyBalanceHelper"/>.
		/// position of thing to look at.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 HeadLookPos
		{
			set { SetArgument("headLookPos", value); }
		}

		/// <summary>
		/// Sets the HeadLookInstanceIndex setting for this <see cref="BodyBalanceHelper"/>.
		/// level index of thing to look at.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int HeadLookInstanceIndex
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("headLookInstanceIndex", value);
			}
		}

		/// <summary>
		/// Sets the SpineStiffness setting for this <see cref="BodyBalanceHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float SpineStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("spineStiffness", value);
			}
		}

		/// <summary>
		/// Sets the SomersaultAngle setting for this <see cref="BodyBalanceHelper"/>.
		/// multiplier of the somersault 'angle' (lean forward/back) for arms out (lean2).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float SomersaultAngle
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("somersaultAngle", value);
			}
		}

		/// <summary>
		/// Sets the SomersaultAngleThreshold setting for this <see cref="BodyBalanceHelper"/>.
		/// Amount of somersault 'angle' before m_somersaultAngle is used for ArmsOut. Unless drunk - DO NOT EXCEED 0.8.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SomersaultAngleThreshold
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("somersaultAngleThreshold", value);
			}
		}

		/// <summary>
		/// Sets the SideSomersaultAngle setting for this <see cref="BodyBalanceHelper"/>.
		/// Amount of side somersault 'angle' before sideSomersault is used for ArmsOut. Unless drunk - DO NOT EXCEED 0.8.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SideSomersaultAngle
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sideSomersaultAngle", value);
			}
		}

		/// <summary>
		/// Sets the SideSomersaultAngleThreshold setting for this <see cref="BodyBalanceHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SideSomersaultAngleThreshold
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sideSomersaultAngleThreshold", value);
			}
		}

		/// <summary>
		/// Sets the BackwardsAutoTurn setting for this <see cref="BodyBalanceHelper"/>.
		/// Automatically turn around if moving backwards.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool BackwardsAutoTurn
		{
			set { SetArgument("backwardsAutoTurn", value); }
		}

		/// <summary>
		/// Sets the TurnWithBumpRadius setting for this <see cref="BodyBalanceHelper"/>.
		/// 0.9 is a sensible value.  If pusher within this distance then turn to get out of the way of the pusher.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float TurnWithBumpRadius
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("turnWithBumpRadius", value);
			}
		}

		/// <summary>
		/// Sets the BackwardsArms setting for this <see cref="BodyBalanceHelper"/>.
		/// Bend elbows, relax shoulders and inhibit spine twist when moving backwards.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool BackwardsArms
		{
			set { SetArgument("backwardsArms", value); }
		}

		/// <summary>
		/// Sets the BlendToZeroPose setting for this <see cref="BodyBalanceHelper"/>.
		/// Blend upper body to zero pose as the character comes to rest. If false blend to a stored pose.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool BlendToZeroPose
		{
			set { SetArgument("blendToZeroPose", value); }
		}

		/// <summary>
		/// Sets the ArmsOutOnPush setting for this <see cref="BodyBalanceHelper"/>.
		/// Put arms out based on lean2 of legs, or angular velocity (lean or twist), or lean (front/back or side/side).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ArmsOutOnPush
		{
			set { SetArgument("armsOutOnPush", value); }
		}

		/// <summary>
		/// Sets the ArmsOutOnPushMultiplier setting for this <see cref="BodyBalanceHelper"/>.
		/// Arms out based on lean2 of the legs to simulate being pushed.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmsOutOnPushMultiplier
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armsOutOnPushMultiplier", value);
			}
		}

		/// <summary>
		/// Sets the ArmsOutOnPushTimeout setting for this <see cref="BodyBalanceHelper"/>.
		/// number of seconds before turning off the armsOutOnPush response only for Arms out based on lean2 of the legs (NOT for the angle or angular velocity).
		/// </summary>
		/// <remarks>
		/// Default value = 1.1f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmsOutOnPushTimeout
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armsOutOnPushTimeout", value);
			}
		}

		/// <summary>
		/// Sets the ReturningToBalanceArmsOut setting for this <see cref="BodyBalanceHelper"/>.
		/// range 0:1 0 = don't raise arms if returning to upright position, 0.x = 0.x*raise arms based on angvel and 'angle' settings, 1 = raise arms based on angvel and 'angle' settings.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ReturningToBalanceArmsOut
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("returningToBalanceArmsOut", value);
			}
		}

		/// <summary>
		/// Sets the ArmsOutStraightenElbows setting for this <see cref="BodyBalanceHelper"/>.
		/// multiplier for straightening the elbows based on the amount of arms out(lean2) 0 = dont straighten elbows. Otherwise straighten elbows proportionately to armsOut.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ArmsOutStraightenElbows
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armsOutStraightenElbows", value);
			}
		}

		/// <summary>
		/// Sets the ArmsOutMinLean2 setting for this <see cref="BodyBalanceHelper"/>.
		/// Minimum desiredLean2 applied to shoulder (to stop arms going above shoulder height or not).
		/// </summary>
		/// <remarks>
		/// Default value = -9.9f.
		/// Min value = -10.0f.
		/// Max value = 0.0f.
		/// </remarks>
		public float ArmsOutMinLean2
		{
			set
			{
				if (value > 0.0f)
					value = 0.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("armsOutMinLean2", value);
			}
		}

		/// <summary>
		/// Sets the SpineDamping setting for this <see cref="BodyBalanceHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float SpineDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineDamping", value);
			}
		}

		/// <summary>
		/// Sets the UseBodyTurn setting for this <see cref="BodyBalanceHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseBodyTurn
		{
			set { SetArgument("useBodyTurn", value); }
		}

		/// <summary>
		/// Sets the ElbowAngleOnContact setting for this <see cref="BodyBalanceHelper"/>.
		/// on contact with upperbody the desired elbow angle is set to at least this value.
		/// </summary>
		/// <remarks>
		/// Default value = 1.9f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float ElbowAngleOnContact
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("elbowAngleOnContact", value);
			}
		}

		/// <summary>
		/// Sets the BendElbowsTime setting for this <see cref="BodyBalanceHelper"/>.
		/// Time after contact (with Upper body) that the min m_elbowAngleOnContact is applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float BendElbowsTime
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("bendElbowsTime", value);
			}
		}

		/// <summary>
		/// Sets the BendElbowsGait setting for this <see cref="BodyBalanceHelper"/>.
		/// Minimum desired angle of elbow during non contact arm swing.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = -3.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float BendElbowsGait
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < -3.0f)
					value = -3.0f;
				SetArgument("bendElbowsGait", value);
			}
		}

		/// <summary>
		/// Sets the HipL2ArmL2 setting for this <see cref="BodyBalanceHelper"/>.
		/// mmmmdrunk = 0.2 multiplier of hip lean2 (star jump) to give shoulder lean2 (flapping).
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float HipL2ArmL2
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("hipL2ArmL2", value);
			}
		}

		/// <summary>
		/// Sets the ShoulderL2 setting for this <see cref="BodyBalanceHelper"/>.
		/// mmmmdrunk = 0.7 shoulder lean2 offset.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = -3.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float ShoulderL2
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < -3.0f)
					value = -3.0f;
				SetArgument("shoulderL2", value);
			}
		}

		/// <summary>
		/// Sets the ShoulderL1 setting for this <see cref="BodyBalanceHelper"/>.
		/// mmmmdrunk 1.1 shoulder lean1 offset (+ve frankenstein).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ShoulderL1
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("shoulderL1", value);
			}
		}

		/// <summary>
		/// Sets the ShoulderTwist setting for this <see cref="BodyBalanceHelper"/>.
		/// mmmmdrunk = 0.0 shoulder twist.
		/// </summary>
		/// <remarks>
		/// Default value = -0.4f.
		/// Min value = -3.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float ShoulderTwist
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < -3.0f)
					value = -3.0f;
				SetArgument("shoulderTwist", value);
			}
		}

		/// <summary>
		/// Sets the HeadLookAtVelProb setting for this <see cref="BodyBalanceHelper"/>.
		/// Probability [0-1] that headLook will be looking in the direction of velocity when stepping.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float HeadLookAtVelProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("headLookAtVelProb", value);
			}
		}

		/// <summary>
		/// Sets the TurnOffProb setting for this <see cref="BodyBalanceHelper"/>.
		/// Weighted Probability that turn will be off. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TurnOffProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turnOffProb", value);
			}
		}

		/// <summary>
		/// Sets the Turn2VelProb setting for this <see cref="BodyBalanceHelper"/>.
		/// Weighted Probability of turning towards velocity. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Turn2VelProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turn2VelProb", value);
			}
		}

		/// <summary>
		/// Sets the TurnAwayProb setting for this <see cref="BodyBalanceHelper"/>.
		/// Weighted Probability of turning away from headLook target. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TurnAwayProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turnAwayProb", value);
			}
		}

		/// <summary>
		/// Sets the TurnLeftProb setting for this <see cref="BodyBalanceHelper"/>.
		/// Weighted Probability of turning left. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TurnLeftProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turnLeftProb", value);
			}
		}

		/// <summary>
		/// Sets the TurnRightProb setting for this <see cref="BodyBalanceHelper"/>.
		/// Weighted Probability of turning right. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TurnRightProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turnRightProb", value);
			}
		}

		/// <summary>
		/// Sets the Turn2TargetProb setting for this <see cref="BodyBalanceHelper"/>.
		/// Weighted Probability of turning towards headLook target. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Turn2TargetProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turn2TargetProb", value);
			}
		}

		/// <summary>
		/// Sets the AngVelMultiplier setting for this <see cref="BodyBalanceHelper"/>.
		/// somersault, twist, sideSomersault) multiplier of the angular velocity  for arms out (lean2) (somersault, twist, sideSomersault).
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(4.0f, 1.0f, 4.0f).
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public Vector3 AngVelMultiplier
		{
			set
			{
				SetArgument("angVelMultiplier",
					Vector3.Clamp(value, new Vector3(0.0f, 0.0f, 0.0f), new Vector3(20.0f, 20.0f, 20.0f)));
			}
		}

		/// <summary>
		/// Sets the AngVelThreshold setting for this <see cref="BodyBalanceHelper"/>.
		/// somersault, twist, sideSomersault) threshold above which angVel is used for arms out (lean2) Unless drunk - DO NOT EXCEED 7.0 for each component.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(1.2f, 3.0f, 1.2f).
		/// Min value = 0.0f.
		/// Max value = 40.0f.
		/// </remarks>
		public Vector3 AngVelThreshold
		{
			set
			{
				SetArgument("angVelThreshold", Vector3.Clamp(value, new Vector3(0.0f, 0.0f, 0.0f), new Vector3(40.0f, 40.0f, 40.0f)));
			}
		}

		/// <summary>
		/// Sets the BraceDistance setting for this <see cref="BodyBalanceHelper"/>.
		/// if -ve then do not brace.  distance from object at which to raise hands to brace 0.5 good if newBrace=true - otherwise 0.65.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float BraceDistance
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("braceDistance", value);
			}
		}

		/// <summary>
		/// Sets the TargetPredictionTime setting for this <see cref="BodyBalanceHelper"/>.
		/// time expected to get arms up from idle.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TargetPredictionTime
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("targetPredictionTime", value);
			}
		}

		/// <summary>
		/// Sets the ReachAbsorbtionTime setting for this <see cref="BodyBalanceHelper"/>.
		/// larger values and he absorbs the impact more.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ReachAbsorbtionTime
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("reachAbsorbtionTime", value);
			}
		}

		/// <summary>
		/// Sets the BraceStiffness setting for this <see cref="BodyBalanceHelper"/>.
		/// stiffness of character. catch_fall stiffness scales with this too, with its defaults at this values default.
		/// </summary>
		/// <remarks>
		/// Default value = 12.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float BraceStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("braceStiffness", value);
			}
		}

		/// <summary>
		/// Sets the MinBraceTime setting for this <see cref="BodyBalanceHelper"/>.
		/// minimum bracing time so the character doesn't look twitchy.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float MinBraceTime
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("minBraceTime", value);
			}
		}

		/// <summary>
		/// Sets the TimeToBackwardsBrace setting for this <see cref="BodyBalanceHelper"/>.
		/// time before arm brace kicks in when hit from behind.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float TimeToBackwardsBrace
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("timeToBackwardsBrace", value);
			}
		}

		/// <summary>
		/// Sets the HandsDelayMin setting for this <see cref="BodyBalanceHelper"/>.
		/// If bracing with 2 hands delay one hand by at least this amount of time to introduce some asymmetry.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float HandsDelayMin
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("handsDelayMin", value);
			}
		}

		/// <summary>
		/// Sets the HandsDelayMax setting for this <see cref="BodyBalanceHelper"/>.
		/// If bracing with 2 hands delay one hand by at most this amount of time to introduce some asymmetry.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float HandsDelayMax
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("handsDelayMax", value);
			}
		}

		/// <summary>
		/// Sets the BraceOffset setting for this <see cref="BodyBalanceHelper"/>.
		/// braceTarget is global headLookPos plus braceOffset m in the up direction.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -2.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float BraceOffset
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -2.0f)
					value = -2.0f;
				SetArgument("braceOffset", value);
			}
		}

		/// <summary>
		/// Sets the MoveRadius setting for this <see cref="BodyBalanceHelper"/>.
		/// if -ve don't move away from pusher unless moveWhenBracing is true and braceDistance  GT  0.0f.  if the pusher is closer than moveRadius then move away from it.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float MoveRadius
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("moveRadius", value);
			}
		}

		/// <summary>
		/// Sets the MoveAmount setting for this <see cref="BodyBalanceHelper"/>.
		/// amount of leanForce applied away from pusher.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float MoveAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("moveAmount", value);
			}
		}

		/// <summary>
		/// Sets the MoveWhenBracing setting for this <see cref="BodyBalanceHelper"/>.
		/// Only move away from pusher when bracing against pusher.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool MoveWhenBracing
		{
			set { SetArgument("moveWhenBracing", value); }
		}
	}

	public sealed class BodyFoetalHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the BodyFoetalHelper for sending a BodyFoetal <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the BodyFoetal <see cref="Message"/> to.</param>
		public BodyFoetalHelper(Ped ped) : base(ped, "bodyFoetal")
		{
		}

		/// <summary>
		/// Sets the Stiffness setting for this <see cref="BodyFoetalHelper"/>.
		/// The stiffness of the body determines how fast the character moves into the position, and how well that they hold it.
		/// </summary>
		/// <remarks>
		/// Default value = 9.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float Stiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("stiffness", value);
			}
		}

		/// <summary>
		/// Sets the DampingFactor setting for this <see cref="BodyFoetalHelper"/>.
		/// Sets damping value for the character joints.
		/// </summary>
		/// <remarks>
		/// Default value = 1.4f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float DampingFactor
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dampingFactor", value);
			}
		}

		/// <summary>
		/// Sets the Asymmetry setting for this <see cref="BodyFoetalHelper"/>.
		/// A value between 0-1 that controls how asymmetric the results are by varying stiffness across the body.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Asymmetry
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("asymmetry", value);
			}
		}

		/// <summary>
		/// Sets the RandomSeed setting for this <see cref="BodyFoetalHelper"/>.
		/// Random seed used to generate asymmetry values.
		/// </summary>
		/// <remarks>
		/// Default value = 100.
		/// Min value = 0.
		/// </remarks>
		public int RandomSeed
		{
			set
			{
				if (value < 0)
					value = 0;
				SetArgument("randomSeed", value);
			}
		}

		/// <summary>
		/// Sets the BackTwist setting for this <see cref="BodyFoetalHelper"/>.
		/// Amount of random back twist to add.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float BackTwist
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("backTwist", value);
			}
		}

		/// <summary>
		/// Sets the Mask setting for this <see cref="BodyFoetalHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see Active Pose notes for possible values).
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string Mask
		{
			set { SetArgument("mask", value); }
		}
	}

	public sealed class BodyRollUpHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the BodyRollUpHelper for sending a BodyRollUp <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the BodyRollUp <see cref="Message"/> to.</param>
		public BodyRollUpHelper(Ped ped) : base(ped, "bodyRollUp")
		{
		}

		/// <summary>
		/// Sets the Stiffness setting for this <see cref="BodyRollUpHelper"/>.
		/// stiffness of whole body.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float Stiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("stiffness", value);
			}
		}

		/// <summary>
		/// Sets the UseArmToSlowDown setting for this <see cref="BodyRollUpHelper"/>.
		/// the degree to which the character will try to stop a barrel roll with his arms.
		/// </summary>
		/// <remarks>
		/// Default value = 1.3f.
		/// Min value = -2.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float UseArmToSlowDown
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < -2.0f)
					value = -2.0f;
				SetArgument("useArmToSlowDown", value);
			}
		}

		/// <summary>
		/// Sets the ArmReachAmount setting for this <see cref="BodyRollUpHelper"/>.
		/// the likeliness of the character reaching for the ground with its arms.
		/// </summary>
		/// <remarks>
		/// Default value = 1.4f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float ArmReachAmount
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armReachAmount", value);
			}
		}

		/// <summary>
		/// Sets the Mask setting for this <see cref="BodyRollUpHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see Active Pose notes for possible values).
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string Mask
		{
			set { SetArgument("mask", value); }
		}

		/// <summary>
		/// Sets the LegPush setting for this <see cref="BodyRollUpHelper"/>.
		/// used to keep rolling down slope, 1 is full (kicks legs out when pointing upwards).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float LegPush
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("legPush", value);
			}
		}

		/// <summary>
		/// Sets the AsymmetricalLegs setting for this <see cref="BodyRollUpHelper"/>.
		/// 0 is no leg asymmetry in 'foetal' position.  greater than 0 a asymmetricalLegs-rand(30%), added/minus each joint of the legs in radians.  Random number changes about once every roll.  0.4 gives a lot of asymmetry.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -2.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float AsymmetricalLegs
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -2.0f)
					value = -2.0f;
				SetArgument("asymmetricalLegs", value);
			}
		}

		/// <summary>
		/// Sets the NoRollTimeBeforeSuccess setting for this <see cref="BodyRollUpHelper"/>.
		/// time that roll velocity has to be lower than rollVelForSuccess, before success message is sent.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float NoRollTimeBeforeSuccess
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("noRollTimeBeforeSuccess", value);
			}
		}

		/// <summary>
		/// Sets the RollVelForSuccess setting for this <see cref="BodyRollUpHelper"/>.
		/// lower threshold for roll velocity at which success message can be sent.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RollVelForSuccess
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rollVelForSuccess", value);
			}
		}

		/// <summary>
		/// Sets the RollVelLinearContribution setting for this <see cref="BodyRollUpHelper"/>.
		/// contribution of linear COM velocity to roll Velocity (if 0, roll velocity equal to COM angular velocity).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RollVelLinearContribution
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rollVelLinearContribution", value);
			}
		}

		/// <summary>
		/// Sets the VelocityScale setting for this <see cref="BodyRollUpHelper"/>.
		/// Scales perceived body velocity.  The higher this value gets, the more quickly the velocity measure saturates, resulting in a tighter roll at slower speeds. (NB: Set to 1 to match earlier behaviour).
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float VelocityScale
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("velocityScale", value);
			}
		}

		/// <summary>
		/// Sets the VelocityOffset setting for this <see cref="BodyRollUpHelper"/>.
		/// Offsets perceived body velocity.  Increase to create larger "dead zone" around zero velocity where character will be less rolled. (NB: Reset to 0 to match earlier behaviour).
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float VelocityOffset
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("velocityOffset", value);
			}
		}

		/// <summary>
		/// Sets the ApplyMinMaxFriction setting for this <see cref="BodyRollUpHelper"/>.
		/// Controls whether or not behaviour enforces min/max friction.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ApplyMinMaxFriction
		{
			set { SetArgument("applyMinMaxFriction", value); }
		}
	}

	public sealed class BodyWritheHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the BodyWritheHelper for sending a BodyWrithe <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the BodyWrithe <see cref="Message"/> to.</param>
		public BodyWritheHelper(Ped ped) : base(ped, "bodyWrithe")
		{
		}

		/// <summary>
		/// Sets the ArmStiffness setting for this <see cref="BodyWritheHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 13.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("armStiffness", value);
			}
		}

		/// <summary>
		/// Sets the BackStiffness setting for this <see cref="BodyWritheHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 13.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float BackStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("backStiffness", value);
			}
		}

		/// <summary>
		/// Sets the LegStiffness setting for this <see cref="BodyWritheHelper"/>.
		/// The stiffness of the character will determine how 'determined' a writhe this is - high values will make him thrash about wildly.
		/// </summary>
		/// <remarks>
		/// Default value = 13.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float LegStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("legStiffness", value);
			}
		}

		/// <summary>
		/// Sets the ArmDamping setting for this <see cref="BodyWritheHelper"/>.
		/// damping amount, less is underdamped.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float ArmDamping
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armDamping", value);
			}
		}

		/// <summary>
		/// Sets the BackDamping setting for this <see cref="BodyWritheHelper"/>.
		/// damping amount, less is underdamped.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float BackDamping
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("backDamping", value);
			}
		}

		/// <summary>
		/// Sets the LegDamping setting for this <see cref="BodyWritheHelper"/>.
		/// damping amount, less is underdamped.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float LegDamping
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legDamping", value);
			}
		}

		/// <summary>
		/// Sets the ArmPeriod setting for this <see cref="BodyWritheHelper"/>.
		/// Controls how fast the writhe is executed, smaller values make faster motions.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float ArmPeriod
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armPeriod", value);
			}
		}

		/// <summary>
		/// Sets the BackPeriod setting for this <see cref="BodyWritheHelper"/>.
		/// Controls how fast the writhe is executed, smaller values make faster motions.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float BackPeriod
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("backPeriod", value);
			}
		}

		/// <summary>
		/// Sets the LegPeriod setting for this <see cref="BodyWritheHelper"/>.
		/// Controls how fast the writhe is executed, smaller values make faster motions.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float LegPeriod
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legPeriod", value);
			}
		}

		/// <summary>
		/// Sets the Mask setting for this <see cref="BodyWritheHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see Active Pose notes for possible values).
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string Mask
		{
			set { SetArgument("mask", value); }
		}

		/// <summary>
		/// Sets the ArmAmplitude setting for this <see cref="BodyWritheHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float ArmAmplitude
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armAmplitude", value);
			}
		}

		/// <summary>
		/// Sets the BackAmplitude setting for this <see cref="BodyWritheHelper"/>.
		/// scales the amount of writhe. 0 = no writhe.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float BackAmplitude
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("backAmplitude", value);
			}
		}

		/// <summary>
		/// Sets the LegAmplitude setting for this <see cref="BodyWritheHelper"/>.
		/// scales the amount of writhe. 0 = no writhe.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float LegAmplitude
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legAmplitude", value);
			}
		}

		/// <summary>
		/// Sets the ElbowAmplitude setting for this <see cref="BodyWritheHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float ElbowAmplitude
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("elbowAmplitude", value);
			}
		}

		/// <summary>
		/// Sets the KneeAmplitude setting for this <see cref="BodyWritheHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float KneeAmplitude
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("kneeAmplitude", value);
			}
		}

		/// <summary>
		/// Sets the RollOverFlag setting for this <see cref="BodyWritheHelper"/>.
		/// Flag to set trying to rollOver.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool RollOverFlag
		{
			set { SetArgument("rollOverFlag", value); }
		}

		/// <summary>
		/// Sets the BlendArms setting for this <see cref="BodyWritheHelper"/>.
		/// Blend the writhe arms with the current desired arms (0=don't apply any writhe, 1=only writhe).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float BlendArms
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("blendArms", value);
			}
		}

		/// <summary>
		/// Sets the BlendBack setting for this <see cref="BodyWritheHelper"/>.
		/// Blend the writhe spine and neck with the current desired (0=don't apply any writhe, 1=only writhe).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float BlendBack
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("blendBack", value);
			}
		}

		/// <summary>
		/// Sets the BlendLegs setting for this <see cref="BodyWritheHelper"/>.
		/// Blend the writhe legs with the current desired legs (0=don't apply any writhe, 1=only writhe).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float BlendLegs
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("blendLegs", value);
			}
		}

		/// <summary>
		/// Sets the ApplyStiffness setting for this <see cref="BodyWritheHelper"/>.
		/// Use writhe stiffnesses if true. If false don't set any stiffnesses.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ApplyStiffness
		{
			set { SetArgument("applyStiffness", value); }
		}

		/// <summary>
		/// Sets the OnFire setting for this <see cref="BodyWritheHelper"/>.
		/// Extra shoulderBlend. Rolling:one way only, maxRollOverTime, rollOverRadius, doesn't reduce arm stiffness to help rolling. No shoulder twist.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool OnFire
		{
			set { SetArgument("onFire", value); }
		}

		/// <summary>
		/// Sets the ShoulderLean1 setting for this <see cref="BodyWritheHelper"/>.
		/// Blend writhe shoulder desired lean1 with this angle in RAD. Note that onFire has to be set to true for this parameter to take any effect.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 6.3f.
		/// </remarks>
		public float ShoulderLean1
		{
			set
			{
				if (value > 6.3f)
					value = 6.3f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shoulderLean1", value);
			}
		}

		/// <summary>
		/// Sets the ShoulderLean2 setting for this <see cref="BodyWritheHelper"/>.
		/// Blend writhe shoulder desired lean2 with this angle in RAD. Note that onFire has to be set to true for this parameter to take any effect.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 6.3f.
		/// </remarks>
		public float ShoulderLean2
		{
			set
			{
				if (value > 6.3f)
					value = 6.3f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shoulderLean2", value);
			}
		}

		/// <summary>
		/// Sets the Lean1BlendFactor setting for this <see cref="BodyWritheHelper"/>.
		/// Shoulder desired lean1 with shoulderLean1 angle blend factor. Set it to 0 to use original shoulder withe desired lean1 angle for shoulders. Note that onFire has to be set to true for this parameter to take any effect.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Lean1BlendFactor
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lean1BlendFactor", value);
			}
		}

		/// <summary>
		/// Sets the Lean2BlendFactor setting for this <see cref="BodyWritheHelper"/>.
		/// Shoulder desired lean2 with shoulderLean2 angle blend factor. Set it to 0 to use original shoulder withe desired lean2 angle for shoulders. Note that onFire has to be set to true for this parameter to take any effect.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Lean2BlendFactor
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lean2BlendFactor", value);
			}
		}

		/// <summary>
		/// Sets the RollTorqueScale setting for this <see cref="BodyWritheHelper"/>.
		/// Scale rolling torque that is applied to character spine.
		/// </summary>
		/// <remarks>
		/// Default value = 150.0f.
		/// Min value = 0.0f.
		/// Max value = 300.0f.
		/// </remarks>
		public float RollTorqueScale
		{
			set
			{
				if (value > 300.0f)
					value = 300.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rollTorqueScale", value);
			}
		}

		/// <summary>
		/// Sets the MaxRollOverTime setting for this <see cref="BodyWritheHelper"/>.
		/// Rolling torque is ramped down over time. At this time in seconds torque value converges to zero. Use this parameter to restrict time the character is rolling. Note that onFire has to be set to true for this parameter to take any effect.
		/// </summary>
		/// <remarks>
		/// Default value = 8.0f.
		/// Min value = 0.0f.
		/// Max value = 60.0f.
		/// </remarks>
		public float MaxRollOverTime
		{
			set
			{
				if (value > 60.0f)
					value = 60.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxRollOverTime", value);
			}
		}

		/// <summary>
		/// Sets the RollOverRadius setting for this <see cref="BodyWritheHelper"/>.
		/// Rolling torque is ramped down with distance measured from position where character hit the ground and started rolling. At this distance in meters torque value converges to zero. Use this parameter to restrict distance the character travels due to rolling. Note that onFire has to be set to true for this parameter to take any effect.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float RollOverRadius
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rollOverRadius", value);
			}
		}
	}

	public sealed class BraceForImpactHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the BraceForImpactHelper for sending a BraceForImpact <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the BraceForImpact <see cref="Message"/> to.</param>
		public BraceForImpactHelper(Ped ped) : base(ped, "braceForImpact")
		{
		}

		/// <summary>
		/// Sets the BraceDistance setting for this <see cref="BraceForImpactHelper"/>.
		/// distance from object at which to raise hands to brace 0.5 good if newBrace=true - otherwise 0.65.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float BraceDistance
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("braceDistance", value);
			}
		}

		/// <summary>
		/// Sets the TargetPredictionTime setting for this <see cref="BraceForImpactHelper"/>.
		/// time epected to get arms up from idle.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TargetPredictionTime
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("targetPredictionTime", value);
			}
		}

		/// <summary>
		/// Sets the ReachAbsorbtionTime setting for this <see cref="BraceForImpactHelper"/>.
		/// larger values and he absorbs the impact more.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ReachAbsorbtionTime
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("reachAbsorbtionTime", value);
			}
		}

		/// <summary>
		/// Sets the InstanceIndex setting for this <see cref="BraceForImpactHelper"/>.
		/// levelIndex of object to brace.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int InstanceIndex
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("instanceIndex", value);
			}
		}

		/// <summary>
		/// Sets the BodyStiffness setting for this <see cref="BraceForImpactHelper"/>.
		/// stiffness of character. catch_fall stiffness scales with this too, with its defaults at this values default.
		/// </summary>
		/// <remarks>
		/// Default value = 12.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float BodyStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("bodyStiffness", value);
			}
		}

		/// <summary>
		/// Sets the GrabDontLetGo setting for this <see cref="BraceForImpactHelper"/>.
		/// Once a constraint is made, keep reaching with whatever hand is allowed.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool GrabDontLetGo
		{
			set { SetArgument("grabDontLetGo", value); }
		}

		/// <summary>
		/// Sets the GrabStrength setting for this <see cref="BraceForImpactHelper"/>.
		/// strength in hands for grabbing (kg m/s), -1 to ignore/disable.
		/// </summary>
		/// <remarks>
		/// Default value = 40.0f.
		/// Min value = -1.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float GrabStrength
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("grabStrength", value);
			}
		}

		/// <summary>
		/// Sets the GrabDistance setting for this <see cref="BraceForImpactHelper"/>.
		/// Relative distance at which the grab starts.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float GrabDistance
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("grabDistance", value);
			}
		}

		/// <summary>
		/// Sets the GrabReachAngle setting for this <see cref="BraceForImpactHelper"/>.
		/// Angle from front at which the grab activates. If the point is outside this angle from front will not try to grab.
		/// </summary>
		/// <remarks>
		/// Default value = 1.5f.
		/// Min value = 0.0f.
		/// Max value = 3.2f.
		/// </remarks>
		public float GrabReachAngle
		{
			set
			{
				if (value > 3.2f)
					value = 3.2f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("grabReachAngle", value);
			}
		}

		/// <summary>
		/// Sets the GrabHoldTimer setting for this <see cref="BraceForImpactHelper"/>.
		/// amount of time, in seconds, before grab automatically bails.
		/// </summary>
		/// <remarks>
		/// Default value = 2.5f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GrabHoldTimer
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("grabHoldTimer", value);
			}
		}

		/// <summary>
		/// Sets the MaxGrabCarVelocity setting for this <see cref="BraceForImpactHelper"/>.
		/// Don't try to grab a car moving above this speed mmmmtodo make this the relative velocity of car to character?.
		/// </summary>
		/// <remarks>
		/// Default value = 95.0f.
		/// Min value = 0.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float MaxGrabCarVelocity
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxGrabCarVelocity", value);
			}
		}

		/// <summary>
		/// Sets the LegStiffness setting for this <see cref="BraceForImpactHelper"/>.
		/// Balancer leg stiffness mmmmtodo remove this parameter and use configureBalance?.
		/// </summary>
		/// <remarks>
		/// Default value = 12.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float LegStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("legStiffness", value);
			}
		}

		/// <summary>
		/// Sets the TimeToBackwardsBrace setting for this <see cref="BraceForImpactHelper"/>.
		/// time before arm brace kicks in when hit from behind.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float TimeToBackwardsBrace
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("timeToBackwardsBrace", value);
			}
		}

		/// <summary>
		/// Sets the Look setting for this <see cref="BraceForImpactHelper"/>.
		/// position to look at, e.g. the driver.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Look
		{
			set { SetArgument("look", value); }
		}

		/// <summary>
		/// Sets the Pos setting for this <see cref="BraceForImpactHelper"/>.
		/// location of the front part of the object to brace against. This should be the centre of where his hands should meet the object.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Pos
		{
			set { SetArgument("pos", value); }
		}

		/// <summary>
		/// Sets the MinBraceTime setting for this <see cref="BraceForImpactHelper"/>.
		/// minimum bracing time so the character doesn't look twitchy.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float MinBraceTime
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("minBraceTime", value);
			}
		}

		/// <summary>
		/// Sets the HandsDelayMin setting for this <see cref="BraceForImpactHelper"/>.
		/// If bracing with 2 hands delay one hand by at least this amount of time to introduce some asymmetry.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float HandsDelayMin
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("handsDelayMin", value);
			}
		}

		/// <summary>
		/// Sets the HandsDelayMax setting for this <see cref="BraceForImpactHelper"/>.
		/// If bracing with 2 hands delay one hand by at most this amount of time to introduce some asymmetry.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float HandsDelayMax
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("handsDelayMax", value);
			}
		}

		/// <summary>
		/// Sets the MoveAway setting for this <see cref="BraceForImpactHelper"/>.
		/// move away from the car (if in reaching zone).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool MoveAway
		{
			set { SetArgument("moveAway", value); }
		}

		/// <summary>
		/// Sets the MoveAwayAmount setting for this <see cref="BraceForImpactHelper"/>.
		/// forceLean away amount (-ve is lean towards).
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float MoveAwayAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("moveAwayAmount", value);
			}
		}

		/// <summary>
		/// Sets the MoveAwayLean setting for this <see cref="BraceForImpactHelper"/>.
		/// Lean away amount (-ve is lean towards).
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = -0.5f.
		/// Max value = 0.5f.
		/// </remarks>
		public float MoveAwayLean
		{
			set
			{
				if (value > 0.5f)
					value = 0.5f;
				if (value < -0.5f)
					value = -0.5f;
				SetArgument("moveAwayLean", value);
			}
		}

		/// <summary>
		/// Sets the MoveSideways setting for this <see cref="BraceForImpactHelper"/>.
		/// Amount of sideways movement if at the front or back of the car to add to the move away from car.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MoveSideways
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("moveSideways", value);
			}
		}

		/// <summary>
		/// Sets the BbArms setting for this <see cref="BraceForImpactHelper"/>.
		/// Use bodyBalance arms for the default (non bracing) behaviour if bodyBalance is active.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool BbArms
		{
			set { SetArgument("bbArms", value); }
		}

		/// <summary>
		/// Sets the NewBrace setting for this <see cref="BraceForImpactHelper"/>.
		/// Use the new brace prediction code.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool NewBrace
		{
			set { SetArgument("newBrace", value); }
		}

		/// <summary>
		/// Sets the BraceOnImpact setting for this <see cref="BraceForImpactHelper"/>.
		/// If true then if a shin or thigh is in contact with the car then brace. NB: newBrace must be true.  For those situations where the car has pushed the ped backwards (at the same speed as the car) before the behaviour has been started and so doesn't predict an impact.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool BraceOnImpact
		{
			set { SetArgument("braceOnImpact", value); }
		}

		/// <summary>
		/// Sets the Roll2Velocity setting for this <see cref="BraceForImpactHelper"/>.
		/// When rollDownStairs is running use roll2Velocity to control the helper torques (this only attempts to roll to the chaarcter's velocity not some default linear velocity mag.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Roll2Velocity
		{
			set { SetArgument("roll2Velocity", value); }
		}

		/// <summary>
		/// Sets the RollType setting for this <see cref="BraceForImpactHelper"/>.
		/// 0 = original/roll off/stay on car:  Roll with character velocity, 1 = //Gentle: roll off/stay on car = use relative velocity of character to car to roll against, 2 = //roll over car:  Roll against character velocity.  i.e. roll against any velocity picked up by hitting car, 3 = //Gentle: roll over car:  use relative velocity of character to car to roll with.
		/// </summary>
		/// <remarks>
		/// Default value = 3.
		/// Min value = 0.
		/// Max value = 3.
		/// </remarks>
		public int RollType
		{
			set
			{
				if (value > 3)
					value = 3;
				if (value < 0)
					value = 0;
				SetArgument("rollType", value);
			}
		}

		/// <summary>
		/// Sets the SnapImpacts setting for this <see cref="BraceForImpactHelper"/>.
		/// Exaggerate impacts using snap.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SnapImpacts
		{
			set { SetArgument("snapImpacts", value); }
		}

		/// <summary>
		/// Sets the SnapImpact setting for this <see cref="BraceForImpactHelper"/>.
		/// Exaggeration amount of the initial impact (legs).  +ve fold with car impact (as if pushed at hips in the car velocity direction).  -ve fold away from car impact.
		/// </summary>
		/// <remarks>
		/// Default value = 7.0f.
		/// Min value = -20.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float SnapImpact
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < -20.0f)
					value = -20.0f;
				SetArgument("snapImpact", value);
			}
		}

		/// <summary>
		/// Sets the SnapBonnet setting for this <see cref="BraceForImpactHelper"/>.
		/// Exaggeration amount of the secondary (torso) impact with bonnet. +ve fold with car impact (as if pushed at hips by the impact normal).  -ve fold away from car impact.
		/// </summary>
		/// <remarks>
		/// Default value = -7.0f.
		/// Min value = -20.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float SnapBonnet
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < -20.0f)
					value = -20.0f;
				SetArgument("snapBonnet", value);
			}
		}

		/// <summary>
		/// Sets the SnapFloor setting for this <see cref="BraceForImpactHelper"/>.
		/// Exaggeration amount of the impact with the floor after falling off of car +ve fold with floor impact (as if pushed at hips in the impact normal direction).  -ve fold away from car impact.
		/// </summary>
		/// <remarks>
		/// Default value = 7.0f.
		/// Min value = -20.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float SnapFloor
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < -20.0f)
					value = -20.0f;
				SetArgument("snapFloor", value);
			}
		}

		/// <summary>
		/// Sets the DampVel setting for this <see cref="BraceForImpactHelper"/>.
		/// Damp out excessive spin and upward velocity when on car.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool DampVel
		{
			set { SetArgument("dampVel", value); }
		}

		/// <summary>
		/// Sets the DampSpin setting for this <see cref="BraceForImpactHelper"/>.
		/// Amount to damp spinning by (cartwheeling and somersaulting).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 40.0f.
		/// </remarks>
		public float DampSpin
		{
			set
			{
				if (value > 40.0f)
					value = 40.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dampSpin", value);
			}
		}

		/// <summary>
		/// Sets the DampUpVel setting for this <see cref="BraceForImpactHelper"/>.
		/// Amount to damp upward velocity by to limit the amount of air above the car the character can get.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 0.0f.
		/// Max value = 40.0f.
		/// </remarks>
		public float DampUpVel
		{
			set
			{
				if (value > 40.0f)
					value = 40.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dampUpVel", value);
			}
		}

		/// <summary>
		/// Sets the DampSpinThresh setting for this <see cref="BraceForImpactHelper"/>.
		/// Angular velocity above which we start damping.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float DampSpinThresh
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dampSpinThresh", value);
			}
		}

		/// <summary>
		/// Sets the DampUpVelThresh setting for this <see cref="BraceForImpactHelper"/>.
		/// Upward velocity above which we start damping.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float DampUpVelThresh
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dampUpVelThresh", value);
			}
		}

		/// <summary>
		/// Sets the GsHelp setting for this <see cref="BraceForImpactHelper"/>.
		/// Enhance a glancing spin with the side of the car by modulating body friction.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool GsHelp
		{
			set { SetArgument("gsHelp", value); }
		}

		/// <summary>
		/// Sets the GsEndMin setting for this <see cref="BraceForImpactHelper"/>.
		/// ID for glancing spin. min depth to be considered from either end (front/rear) of a car (-ve is inside the car area).
		/// </summary>
		/// <remarks>
		/// Default value = -0.1f.
		/// Min value = -10.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float GsEndMin
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("gsEndMin", value);
			}
		}

		/// <summary>
		/// Sets the GsSideMin setting for this <see cref="BraceForImpactHelper"/>.
		/// ID for glancing spin. min depth to be considered on the side of a car (-ve is inside the car area).
		/// </summary>
		/// <remarks>
		/// Default value = -0.2f.
		/// Min value = -10.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float GsSideMin
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("gsSideMin", value);
			}
		}

		/// <summary>
		/// Sets the GsSideMax setting for this <see cref="BraceForImpactHelper"/>.
		/// ID for glancing spin. max depth to be considered on the side of a car (+ve is outside the car area).
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = -10.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float GsSideMax
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("gsSideMax", value);
			}
		}

		/// <summary>
		/// Sets the GsUpness setting for this <see cref="BraceForImpactHelper"/>.
		/// ID for glancing spin. Character has to be more upright than this value for it to be considered on the side of a car. Fully upright = 1, upsideDown = -1.  Max Angle from upright is acos(gsUpness).
		/// </summary>
		/// <remarks>
		/// Default value = 0.9f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GsUpness
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("gsUpness", value);
			}
		}

		/// <summary>
		/// Sets the GsCarVelMin setting for this <see cref="BraceForImpactHelper"/>.
		/// ID for glancing spin. Minimum car velocity.
		/// </summary>
		/// <remarks>
		/// Default value = 3.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GsCarVelMin
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("gsCarVelMin", value);
			}
		}

		/// <summary>
		/// Sets the GsScale1Foot setting for this <see cref="BraceForImpactHelper"/>.
		/// Apply gsFricScale1 to the foot if colliding with car.  (Otherwise foot friction - with the ground - is determined by gsFricScale2 if it is in gsFricMask2).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool GsScale1Foot
		{
			set { SetArgument("gsScale1Foot", value); }
		}

		/// <summary>
		/// Sets the GsFricScale1 setting for this <see cref="BraceForImpactHelper"/>.
		/// Glancing spin help. Friction scale applied when to the side of the car.  e.g. make the character spin more by upping the friction against the car.
		/// </summary>
		/// <remarks>
		/// Default value = 8.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GsFricScale1
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("gsFricScale1", value);
			}
		}

		/// <summary>
		/// Sets the GsFricMask1 setting for this <see cref="BraceForImpactHelper"/>.
		/// Glancing spin help. Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see notes for explanation). Note gsFricMask1 and gsFricMask2 are made independent by the code so you can have fb for gsFricMask1 but gsFricScale1 will not be applied to any bodyParts in gsFricMask2.
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string GsFricMask1
		{
			set { SetArgument("gsFricMask1", value); }
		}

		/// <summary>
		/// Sets the GsFricScale2 setting for this <see cref="BraceForImpactHelper"/>.
		/// Glancing spin help. Friction scale applied when to the side of the car.  e.g. make the character spin more by lowering the feet friction. You could also lower the wrist friction here to stop the car pulling along the hands i.e. gsFricMask2 = la|uw.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GsFricScale2
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("gsFricScale2", value);
			}
		}

		/// <summary>
		/// Sets the GsFricMask2 setting for this <see cref="BraceForImpactHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see notes for explanation). Note gsFricMask1 and gsFricMask2 are made independent by the code so you can have fb for gsFricMask1 but gsFricScale1 will not be applied to any bodyParts in gsFricMask2.
		/// </summary>
		/// <remarks>
		/// Default value = la.
		/// </remarks>
		public string GsFricMask2
		{
			set { SetArgument("gsFricMask2", value); }
		}
	}

	/// <summary>
	/// Simple buoyancy model.  No character movement just fluid forces/torques added to parts.
	/// </summary>
	public sealed class BuoyancyHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the BuoyancyHelper for sending a Buoyancy <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the Buoyancy <see cref="Message"/> to.</param>
		/// <remarks>
		/// Simple buoyancy model.  No character movement just fluid forces/torques added to parts.
		/// </remarks>
		public BuoyancyHelper(Ped ped) : base(ped, "buoyancy")
		{
		}

		/// <summary>
		/// Sets the SurfacePoint setting for this <see cref="BuoyancyHelper"/>.
		/// Arbitrary point on surface of water.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 SurfacePoint
		{
			set { SetArgument("surfacePoint", value); }
		}

		/// <summary>
		/// Sets the SurfaceNormal setting for this <see cref="BuoyancyHelper"/>.
		/// Normal to surface of water.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 1.0f).
		/// Min value = 0.0f.
		/// </remarks>
		public Vector3 SurfaceNormal
		{
			set { SetArgument("surfaceNormal", Vector3.Max(value, new Vector3(0.0f, 0.0f, 0.0f))); }
		}

		/// <summary>
		/// Sets the Buoyancy setting for this <see cref="BuoyancyHelper"/>.
		/// Buoyancy multiplier.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float Buoyancy
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("buoyancy", value);
			}
		}

		/// <summary>
		/// Sets the ChestBuoyancy setting for this <see cref="BuoyancyHelper"/>.
		/// Buoyancy mulplier for spine2/3. Helps character float upright.
		/// </summary>
		/// <remarks>
		/// Default value = 8.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float ChestBuoyancy
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("chestBuoyancy", value);
			}
		}

		/// <summary>
		/// Sets the Damping setting for this <see cref="BuoyancyHelper"/>.
		/// Damping for submerged parts.
		/// </summary>
		/// <remarks>
		/// Default value = 40.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float Damping
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("damping", value);
			}
		}

		/// <summary>
		/// Sets the Righting setting for this <see cref="BuoyancyHelper"/>.
		/// Use righting torque to being character face-up in water?.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool Righting
		{
			set { SetArgument("righting", value); }
		}

		/// <summary>
		/// Sets the RightingStrength setting for this <see cref="BuoyancyHelper"/>.
		/// Strength of righting torque.
		/// </summary>
		/// <remarks>
		/// Default value = 25.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float RightingStrength
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rightingStrength", value);
			}
		}

		/// <summary>
		/// Sets the RightingTime setting for this <see cref="BuoyancyHelper"/>.
		/// How long to wait after chest hits water to begin righting torque.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// </remarks>
		public float RightingTime
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rightingTime", value);
			}
		}
	}

	public sealed class CatchFallHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the CatchFallHelper for sending a CatchFall <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the CatchFall <see cref="Message"/> to.</param>
		public CatchFallHelper(Ped ped) : base(ped, "catchFall")
		{
		}

		/// <summary>
		/// Sets the TorsoStiffness setting for this <see cref="CatchFallHelper"/>.
		/// stiffness of torso.
		/// </summary>
		/// <remarks>
		/// Default value = 9.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float TorsoStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("torsoStiffness", value);
			}
		}

		/// <summary>
		/// Sets the LegsStiffness setting for this <see cref="CatchFallHelper"/>.
		/// stiffness of legs.
		/// </summary>
		/// <remarks>
		/// Default value = 6.0f.
		/// Min value = 4.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float LegsStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 4.0f)
					value = 4.0f;
				SetArgument("legsStiffness", value);
			}
		}

		/// <summary>
		/// Sets the ArmsStiffness setting for this <see cref="CatchFallHelper"/>.
		/// stiffness of arms.
		/// </summary>
		/// <remarks>
		/// Default value = 15.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmsStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("armsStiffness", value);
			}
		}

		/// <summary>
		/// Sets the BackwardsMinArmOffset setting for this <see cref="CatchFallHelper"/>.
		/// 0 will prop arms up near his shoulders. -0.3 will place hands nearer his behind.
		/// </summary>
		/// <remarks>
		/// Default value = -0.3f.
		/// Min value = -1.0f.
		/// Max value = 0.0f.
		/// </remarks>
		public float BackwardsMinArmOffset
		{
			set
			{
				if (value > 0.0f)
					value = 0.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("backwardsMinArmOffset", value);
			}
		}

		/// <summary>
		/// Sets the ForwardMaxArmOffset setting for this <see cref="CatchFallHelper"/>.
		/// 0 will point arms down with angled body, 0.45 will point arms forward a bit to catch nearer the head.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ForwardMaxArmOffset
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("forwardMaxArmOffset", value);
			}
		}

		/// <summary>
		/// Sets the ZAxisSpinReduction setting for this <see cref="CatchFallHelper"/>.
		/// Tries to reduce the spin around the Z axis. Scale 0 - 1.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ZAxisSpinReduction
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("zAxisSpinReduction", value);
			}
		}

		/// <summary>
		/// Sets the ExtraSit setting for this <see cref="CatchFallHelper"/>.
		/// Scale extra-sit value 0..1. Setting to 0 helps with arched-back issues.  Set to 1 for a more alive-looking finish.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ExtraSit
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("extraSit", value);
			}
		}

		/// <summary>
		/// Sets the UseHeadLook setting for this <see cref="CatchFallHelper"/>.
		/// Toggle to use the head look in this behaviour.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseHeadLook
		{
			set { SetArgument("useHeadLook", value); }
		}

		/// <summary>
		/// Sets the Mask setting for this <see cref="CatchFallHelper"/>.
		/// Two character body-masking value, bitwise joint mask or bitwise logic string of two character body-masking value  (see Active Pose notes for possible values).
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string Mask
		{
			set { SetArgument("mask", value); }
		}
	}

	public sealed class ElectrocuteHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ElectrocuteHelper for sending a Electrocute <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the Electrocute <see cref="Message"/> to.</param>
		public ElectrocuteHelper(Ped ped) : base(ped, "electrocute")
		{
		}

		/// <summary>
		/// Sets the StunMag setting for this <see cref="ElectrocuteHelper"/>.
		/// The magnitude of the reaction.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float StunMag
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stunMag", value);
			}
		}

		/// <summary>
		/// Sets the InitialMult setting for this <see cref="ElectrocuteHelper"/>.
		/// initialMult*stunMag = The magnitude of the 1st snap reaction (other mults are applied after this).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float InitialMult
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("initialMult", value);
			}
		}

		/// <summary>
		/// Sets the LargeMult setting for this <see cref="ElectrocuteHelper"/>.
		/// largeMult*stunMag = The magnitude of a random large snap reaction (other mults are applied after this).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float LargeMult
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("largeMult", value);
			}
		}

		/// <summary>
		/// Sets the LargeMinTime setting for this <see cref="ElectrocuteHelper"/>.
		/// min time to next large random snap (about 14 snaps with stunInterval = 0.07s).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 200.0f.
		/// </remarks>
		public float LargeMinTime
		{
			set
			{
				if (value > 200.0f)
					value = 200.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("largeMinTime", value);
			}
		}

		/// <summary>
		/// Sets the LargeMaxTime setting for this <see cref="ElectrocuteHelper"/>.
		/// max time to next large random snap (about 28 snaps with stunInterval = 0.07s).
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 200.0f.
		/// </remarks>
		public float LargeMaxTime
		{
			set
			{
				if (value > 200.0f)
					value = 200.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("largeMaxTime", value);
			}
		}

		/// <summary>
		/// Sets the MovingMult setting for this <see cref="ElectrocuteHelper"/>.
		/// movingMult*stunMag = The magnitude of the reaction if moving(comVelMag) faster than movingThresh.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float MovingMult
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("movingMult", value);
			}
		}

		/// <summary>
		/// Sets the BalancingMult setting for this <see cref="ElectrocuteHelper"/>.
		/// balancingMult*stunMag = The magnitude of the reaction if balancing = (not lying on the floor/ not upper body not collided) and not airborne.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float BalancingMult
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("balancingMult", value);
			}
		}

		/// <summary>
		/// Sets the AirborneMult setting for this <see cref="ElectrocuteHelper"/>.
		/// airborneMult*stunMag = The magnitude of the reaction if airborne.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float AirborneMult
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("airborneMult", value);
			}
		}

		/// <summary>
		/// Sets the MovingThresh setting for this <see cref="ElectrocuteHelper"/>.
		/// If moving(comVelMag) faster than movingThresh then mvingMult applied to stunMag.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float MovingThresh
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("movingThresh", value);
			}
		}

		/// <summary>
		/// Sets the StunInterval setting for this <see cref="ElectrocuteHelper"/>.
		/// Direction flips every stunInterval.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float StunInterval
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stunInterval", value);
			}
		}

		/// <summary>
		/// Sets the DirectionRandomness setting for this <see cref="ElectrocuteHelper"/>.
		/// The character vibrates in a prescribed way - Higher the value the more random this direction is.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float DirectionRandomness
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("directionRandomness", value);
			}
		}

		/// <summary>
		/// Sets the LeftArm setting for this <see cref="ElectrocuteHelper"/>.
		/// vibrate the leftArm.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool LeftArm
		{
			set { SetArgument("leftArm", value); }
		}

		/// <summary>
		/// Sets the RightArm setting for this <see cref="ElectrocuteHelper"/>.
		/// vibrate the rightArm.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool RightArm
		{
			set { SetArgument("rightArm", value); }
		}

		/// <summary>
		/// Sets the LeftLeg setting for this <see cref="ElectrocuteHelper"/>.
		/// vibrate the leftLeg.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool LeftLeg
		{
			set { SetArgument("leftLeg", value); }
		}

		/// <summary>
		/// Sets the RightLeg setting for this <see cref="ElectrocuteHelper"/>.
		/// vibrate the rightLeg.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool RightLeg
		{
			set { SetArgument("rightLeg", value); }
		}

		/// <summary>
		/// Sets the Spine setting for this <see cref="ElectrocuteHelper"/>.
		/// vibrate the spine.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool Spine
		{
			set { SetArgument("spine", value); }
		}

		/// <summary>
		/// Sets the Neck setting for this <see cref="ElectrocuteHelper"/>.
		/// vibrate the neck.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool Neck
		{
			set { SetArgument("neck", value); }
		}

		/// <summary>
		/// Sets the PhasedLegs setting for this <see cref="ElectrocuteHelper"/>.
		/// Legs are either in phase with each other or not.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool PhasedLegs
		{
			set { SetArgument("phasedLegs", value); }
		}

		/// <summary>
		/// Sets the ApplyStiffness setting for this <see cref="ElectrocuteHelper"/>.
		/// let electrocute apply a (higher generally) stiffness to the character whilst being vibrated.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ApplyStiffness
		{
			set { SetArgument("applyStiffness", value); }
		}

		/// <summary>
		/// Sets the UseTorques setting for this <see cref="ElectrocuteHelper"/>.
		/// use torques to make vibration otherwise use a change in the parts angular velocity.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseTorques
		{
			set { SetArgument("useTorques", value); }
		}

		/// <summary>
		/// Sets the HipType setting for this <see cref="ElectrocuteHelper"/>.
		/// type of hip reaction 0=none, 1=side2side 2=steplike.
		/// </summary>
		/// <remarks>
		/// Default value = 2.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int HipType
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("hipType", value);
			}
		}
	}

	public sealed class FallOverWallHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the FallOverWallHelper for sending a FallOverWall <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the FallOverWall <see cref="Message"/> to.</param>
		public FallOverWallHelper(Ped ped) : base(ped, "fallOverWall")
		{
		}

		/// <summary>
		/// Sets the BodyStiffness setting for this <see cref="FallOverWallHelper"/>.
		/// stiffness of the body, roll up stiffness scales with this and defaults at this default value.
		/// </summary>
		/// <remarks>
		/// Default value = 9.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float BodyStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("bodyStiffness", value);
			}
		}

		/// <summary>
		/// Sets the Damping setting for this <see cref="FallOverWallHelper"/>.
		/// Damping in the effectors.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float Damping
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("damping", value);
			}
		}

		/// <summary>
		/// Sets the MagOfForce setting for this <see cref="FallOverWallHelper"/>.
		/// Magnitude of the falloverWall helper force.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float MagOfForce
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("magOfForce", value);
			}
		}

		/// <summary>
		/// Sets the MaxDistanceFromPelToHitPoint setting for this <see cref="FallOverWallHelper"/>.
		/// The maximum distance away from the pelvis that hit points will be registered.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float MaxDistanceFromPelToHitPoint
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxDistanceFromPelToHitPoint", value);
			}
		}

		/// <summary>
		/// Sets the MaxForceDist setting for this <see cref="FallOverWallHelper"/>.
		/// maximum distance between hitPoint and body part at which forces are applied to part.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float MaxForceDist
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxForceDist", value);
			}
		}

		/// <summary>
		/// Sets the StepExclusionZone setting for this <see cref="FallOverWallHelper"/>.
		/// Specifies extent of area in front of the wall in which balancer won't try to take another step.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float StepExclusionZone
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stepExclusionZone", value);
			}
		}

		/// <summary>
		/// Sets the MinLegHeight setting for this <see cref="FallOverWallHelper"/>.
		/// minimum height of pelvis above feet at which fallOverWall is attempted.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.1f.
		/// Max value = 2.0f.
		/// </remarks>
		public float MinLegHeight
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("minLegHeight", value);
			}
		}

		/// <summary>
		/// Sets the BodyTwist setting for this <see cref="FallOverWallHelper"/>.
		/// amount of twist to apply to the spine as the character tries to fling himself over the wall, provides more of a believable roll but increases the amount of lateral space the character needs to successfully flip.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float BodyTwist
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("bodyTwist", value);
			}
		}

		/// <summary>
		/// Sets the MaxTwist setting for this <see cref="FallOverWallHelper"/>.
		/// max angle the character can twist before twsit helper torques are turned off.
		/// </summary>
		/// <remarks>
		/// Default value = 3.1f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MaxTwist
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxTwist", value);
			}
		}

		/// <summary>
		/// Sets the FallOverWallEndA setting for this <see cref="FallOverWallHelper"/>.
		/// One end of the wall to try to fall over.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 FallOverWallEndA
		{
			set { SetArgument("fallOverWallEndA", value); }
		}

		/// <summary>
		/// Sets the FallOverWallEndB setting for this <see cref="FallOverWallHelper"/>.
		/// One end of the wall over which we are trying to fall over.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 FallOverWallEndB
		{
			set { SetArgument("fallOverWallEndB", value); }
		}

		/// <summary>
		/// Sets the ForceAngleAbort setting for this <see cref="FallOverWallHelper"/>.
		/// The angle abort threshold.
		/// </summary>
		/// <remarks>
		/// Default value = -0.2f.
		/// </remarks>
		public float ForceAngleAbort
		{
			set { SetArgument("forceAngleAbort", value); }
		}

		/// <summary>
		/// Sets the ForceTimeOut setting for this <see cref="FallOverWallHelper"/>.
		/// The force time out.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// </remarks>
		public float ForceTimeOut
		{
			set { SetArgument("forceTimeOut", value); }
		}

		/// <summary>
		/// Sets the MoveArms setting for this <see cref="FallOverWallHelper"/>.
		/// Lift the arms up if true.  Do nothing with the arms if false (eg when using catchfall arms or brace etc).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool MoveArms
		{
			set { SetArgument("moveArms", value); }
		}

		/// <summary>
		/// Sets the MoveLegs setting for this <see cref="FallOverWallHelper"/>.
		/// Move the legs if true.  Do nothing with the legs if false (eg when using dynamicBalancer etc).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool MoveLegs
		{
			set { SetArgument("moveLegs", value); }
		}

		/// <summary>
		/// Sets the BendSpine setting for this <see cref="FallOverWallHelper"/>.
		/// Bend spine to help falloverwall if true.  Do nothing with the spine if false.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool BendSpine
		{
			set { SetArgument("bendSpine", value); }
		}

		/// <summary>
		/// Sets the AngleDirWithWallNormal setting for this <see cref="FallOverWallHelper"/>.
		/// Maximum angle in degrees (between the direction of the velocity of the COM and the wall normal) to start to apply forces and torques to fall over the wall.
		/// </summary>
		/// <remarks>
		/// Default value = 180.0f.
		/// Min value = 0.0f.
		/// Max value = 180.0f.
		/// </remarks>
		public float AngleDirWithWallNormal
		{
			set
			{
				if (value > 180.0f)
					value = 180.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("angleDirWithWallNormal", value);
			}
		}

		/// <summary>
		/// Sets the LeaningAngleThreshold setting for this <see cref="FallOverWallHelper"/>.
		/// Maximum angle in degrees (between the vertical vector and a vector from pelvis to lower neck) to start to apply forces and torques to fall over the wall.
		/// </summary>
		/// <remarks>
		/// Default value = 180.0f.
		/// Min value = 0.0f.
		/// Max value = 180.0f.
		/// </remarks>
		public float LeaningAngleThreshold
		{
			set
			{
				if (value > 180.0f)
					value = 180.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leaningAngleThreshold", value);
			}
		}

		/// <summary>
		/// Sets the MaxAngVel setting for this <see cref="FallOverWallHelper"/>.
		/// if the angular velocity is higher than maxAngVel, the torques and forces are not applied.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = -1.0f.
		/// Max value = 30.0f.
		/// </remarks>
		public float MaxAngVel
		{
			set
			{
				if (value > 30.0f)
					value = 30.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("maxAngVel", value);
			}
		}

		/// <summary>
		/// Sets the AdaptForcesToLowWall setting for this <see cref="FallOverWallHelper"/>.
		/// Will reduce the magnitude of the forces applied to the character to help him to fall over wall.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AdaptForcesToLowWall
		{
			set { SetArgument("adaptForcesToLowWall", value); }
		}

		/// <summary>
		/// Sets the MaxWallHeight setting for this <see cref="FallOverWallHelper"/>.
		/// Maximum height (from the lowest foot) to start to apply forces and torques to fall over the wall.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float MaxWallHeight
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("maxWallHeight", value);
			}
		}

		/// <summary>
		/// Sets the DistanceToSendSuccessMessage setting for this <see cref="FallOverWallHelper"/>.
		/// Minimum distance between the pelvis and the wall to send the success message. If negative doesn't take this parameter into account when sending feedback.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float DistanceToSendSuccessMessage
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("distanceToSendSuccessMessage", value);
			}
		}

		/// <summary>
		/// Sets the RollingBackThr setting for this <see cref="FallOverWallHelper"/>.
		/// Value of the angular velocity about the wallEgde above which the character is considered as rolling backwards i.e. goes in to fow_RollingBack state.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float RollingBackThr
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rollingBackThr", value);
			}
		}

		/// <summary>
		/// Sets the RollingPotential setting for this <see cref="FallOverWallHelper"/>.
		/// On impact with the wall if the rollingPotential(calculated from the characters linear velocity w.r.t the wall) is greater than this value the character will try to go over the wall otherwise it won't try (fow_Aborted).
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = -1.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float RollingPotential
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("rollingPotential", value);
			}
		}

		/// <summary>
		/// Sets the UseArmIK setting for this <see cref="FallOverWallHelper"/>.
		/// Try to reach the wallEdge. To configure the IK : use limitAngleBack, limitAngleFront and limitAngleTotallyBack.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseArmIK
		{
			set { SetArgument("useArmIK", value); }
		}

		/// <summary>
		/// Sets the ReachDistanceFromHitPoint setting for this <see cref="FallOverWallHelper"/>.
		/// distance from predicted hitpoint where each hands will try to reach the wall.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ReachDistanceFromHitPoint
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("reachDistanceFromHitPoint", value);
			}
		}

		/// <summary>
		/// Sets the MinReachDistanceFromHitPoint setting for this <see cref="FallOverWallHelper"/>.
		/// minimal distance from predicted hitpoint where each hands will try to reach the wall. Used if the hand target is outside the wall Edge.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float MinReachDistanceFromHitPoint
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("minReachDistanceFromHitPoint", value);
			}
		}

		/// <summary>
		/// Sets the AngleTotallyBack setting for this <see cref="FallOverWallHelper"/>.
		/// max angle in degrees (between 1.the vector between two hips and 2. wallEdge) to try to reach the wall just behind his pelvis with his arms when the character is back to the wall.
		/// </summary>
		/// <remarks>
		/// Default value = 15.0f.
		/// Min value = 0.0f.
		/// Max value = 180.0f.
		/// </remarks>
		public float AngleTotallyBack
		{
			set
			{
				if (value > 180.0f)
					value = 180.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("angleTotallyBack", value);
			}
		}
	}

	public sealed class GrabHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the GrabHelper for sending a Grab <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the Grab <see cref="Message"/> to.</param>
		public GrabHelper(Ped ped) : base(ped, "grab")
		{
		}

		/// <summary>
		/// Sets the UseLeft setting for this <see cref="GrabHelper"/>.
		/// Flag to toggle use of left hand.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseLeft
		{
			set { SetArgument("useLeft", value); }
		}

		/// <summary>
		/// Sets the UseRight setting for this <see cref="GrabHelper"/>.
		/// Flag to toggle the use of the Right hand.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseRight
		{
			set { SetArgument("useRight", value); }
		}

		/// <summary>
		/// Sets the DropWeaponIfNecessary setting for this <see cref="GrabHelper"/>.
		/// if hasn't grabbed when weapon carrying hand is close to target, grab anyway.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool DropWeaponIfNecessary
		{
			set { SetArgument("dropWeaponIfNecessary", value); }
		}

		/// <summary>
		/// Sets the DropWeaponDistance setting for this <see cref="GrabHelper"/>.
		/// distance below which a weapon carrying hand will request weapon to be dropped.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float DropWeaponDistance
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dropWeaponDistance", value);
			}
		}

		/// <summary>
		/// Sets the GrabStrength setting for this <see cref="GrabHelper"/>.
		/// strength in hands for grabbing (kg m/s), -1 to ignore/disable.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 10000.0f.
		/// </remarks>
		public float GrabStrength
		{
			set
			{
				if (value > 10000.0f)
					value = 10000.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("grabStrength", value);
			}
		}

		/// <summary>
		/// Sets the StickyHands setting for this <see cref="GrabHelper"/>.
		/// strength of cheat force on hands to pull towards target and stick to target ("cleverHandIK" strength).
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float StickyHands
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stickyHands", value);
			}
		}

		/// <summary>
		/// Sets the TurnToTarget setting for this <see cref="GrabHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="TurnType.ToTarget"/>.
		/// </remarks>
		public TurnType TurnToTarget
		{
			set { SetArgument("turnToTarget", (int) value); }
		}

		/// <summary>
		/// Sets the GrabHoldMaxTimer setting for this <see cref="GrabHelper"/>.
		/// amount of time, in seconds, before grab automatically bails.
		/// </summary>
		/// <remarks>
		/// Default value = 100.0f.
		/// Min value = 0.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float GrabHoldMaxTimer
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("grabHoldMaxTimer", value);
			}
		}

		/// <summary>
		/// Sets the PullUpTime setting for this <see cref="GrabHelper"/>.
		/// Time to reach the full pullup strength.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float PullUpTime
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("pullUpTime", value);
			}
		}

		/// <summary>
		/// Sets the PullUpStrengthRight setting for this <see cref="GrabHelper"/>.
		/// Strength to pull up with the right arm. 0 = no pull up.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float PullUpStrengthRight
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("pullUpStrengthRight", value);
			}
		}

		/// <summary>
		/// Sets the PullUpStrengthLeft setting for this <see cref="GrabHelper"/>.
		/// Strength to pull up with the left arm. 0 = no pull up.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float PullUpStrengthLeft
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("pullUpStrengthLeft", value);
			}
		}

		/// <summary>
		/// Sets the Pos1 setting for this <see cref="GrabHelper"/>.
		/// Grab pos1, right hand if not using line or surface grab.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Pos1
		{
			set { SetArgument("pos1", value); }
		}

		/// <summary>
		/// Sets the Pos2 setting for this <see cref="GrabHelper"/>.
		/// Grab pos2, left hand if not using line or surface grab.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Pos2
		{
			set { SetArgument("pos2", value); }
		}

		/// <summary>
		/// Sets the Pos3 setting for this <see cref="GrabHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Pos3
		{
			set { SetArgument("pos3", value); }
		}

		/// <summary>
		/// Sets the Pos4 setting for this <see cref="GrabHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Pos4
		{
			set { SetArgument("pos4", value); }
		}

		/// <summary>
		/// Sets the NormalR setting for this <see cref="GrabHelper"/>.
		/// Normal for the right grab point.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public Vector3 NormalR
		{
			set
			{
				SetArgument("normalR", Vector3.Clamp(value, new Vector3(-1.0f, -1.0f, -1.0f), new Vector3(1.0f, 1.0f, 1.0f)));
			}
		}

		/// <summary>
		/// Sets the NormalL setting for this <see cref="GrabHelper"/>.
		/// Normal for the left grab point.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public Vector3 NormalL
		{
			set
			{
				SetArgument("normalL", Vector3.Clamp(value, new Vector3(-1.0f, -1.0f, -1.0f), new Vector3(1.0f, 1.0f, 1.0f)));
			}
		}

		/// <summary>
		/// Sets the NormalR2 setting for this <see cref="GrabHelper"/>.
		/// Normal for the 2nd right grab point (if pointsX4grab=true).
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public Vector3 NormalR2
		{
			set
			{
				SetArgument("normalR2", Vector3.Clamp(value, new Vector3(-1.0f, -1.0f, -1.0f), new Vector3(1.0f, 1.0f, 1.0f)));
			}
		}

		/// <summary>
		/// Sets the NormalL2 setting for this <see cref="GrabHelper"/>.
		/// Normal for the 3rd left grab point (if pointsX4grab=true).
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public Vector3 NormalL2
		{
			set
			{
				SetArgument("normalL2", Vector3.Clamp(value, new Vector3(-1.0f, -1.0f, -1.0f), new Vector3(1.0f, 1.0f, 1.0f)));
			}
		}

		/// <summary>
		/// Sets the HandsCollide setting for this <see cref="GrabHelper"/>.
		/// Hand collisions on when grabbing (false turns off hand collisions making grab more stable esp. to grab points slightly inside geometry).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool HandsCollide
		{
			set { SetArgument("handsCollide", value); }
		}

		/// <summary>
		/// Sets the JustBrace setting for this <see cref="GrabHelper"/>.
		/// Flag to toggle between grabbing and bracing.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool JustBrace
		{
			set { SetArgument("justBrace", value); }
		}

		/// <summary>
		/// Sets the UseLineGrab setting for this <see cref="GrabHelper"/>.
		/// use the line grab, Grab along the line (x-x2).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseLineGrab
		{
			set { SetArgument("useLineGrab", value); }
		}

		/// <summary>
		/// Sets the PointsX4grab setting for this <see cref="GrabHelper"/>.
		/// use 2 point.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool PointsX4grab
		{
			set { SetArgument("pointsX4grab", value); }
		}

		/// <summary>
		/// Sets the FromEA setting for this <see cref="GrabHelper"/>.
		/// use 2 point.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FromEA
		{
			set { SetArgument("fromEA", value); }
		}

		/// <summary>
		/// Sets the SurfaceGrab setting for this <see cref="GrabHelper"/>.
		/// Toggle surface grab on. Requires pos1,pos2,pos3 and pos4 to be specified.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SurfaceGrab
		{
			set { SetArgument("surfaceGrab", value); }
		}

		/// <summary>
		/// Sets the InstanceIndex setting for this <see cref="GrabHelper"/>.
		/// levelIndex of instance to grab (-1 = world coordinates).
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int InstanceIndex
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("instanceIndex", value);
			}
		}

		/// <summary>
		/// Sets the InstancePartIndex setting for this <see cref="GrabHelper"/>.
		/// boundIndex of part on instance to grab (0 = just use instance coordinates).
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// </remarks>
		public int InstancePartIndex
		{
			set
			{
				if (value < 0)
					value = 0;
				SetArgument("instancePartIndex", value);
			}
		}

		/// <summary>
		/// Sets the DontLetGo setting for this <see cref="GrabHelper"/>.
		/// Once a constraint is made, keep reaching with whatever hand is allowed - no matter what the angle/distance and whether or not the constraint has broken due to constraintForce  GT  grabStrength.  mmmtodo this is a badly named parameter.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool DontLetGo
		{
			set { SetArgument("dontLetGo", value); }
		}

		/// <summary>
		/// Sets the BodyStiffness setting for this <see cref="GrabHelper"/>.
		/// stiffness of upper body. Scales the arm grab such that the armStiffness is default when this is at default value.
		/// </summary>
		/// <remarks>
		/// Default value = 11.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float BodyStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("bodyStiffness", value);
			}
		}

		/// <summary>
		/// Sets the ReachAngle setting for this <see cref="GrabHelper"/>.
		/// Angle from front at which the grab activates. If the point is outside this angle from front will not try to grab.
		/// </summary>
		/// <remarks>
		/// Default value = 2.8f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float ReachAngle
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("reachAngle", value);
			}
		}

		/// <summary>
		/// Sets the OneSideReachAngle setting for this <see cref="GrabHelper"/>.
		/// Angle at which we will only reach with one hand.
		/// </summary>
		/// <remarks>
		/// Default value = 1.4f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float OneSideReachAngle
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("oneSideReachAngle", value);
			}
		}

		/// <summary>
		/// Sets the GrabDistance setting for this <see cref="GrabHelper"/>.
		/// Relative distance at which the grab starts.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float GrabDistance
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("grabDistance", value);
			}
		}

		/// <summary>
		/// Sets the Move2Radius setting for this <see cref="GrabHelper"/>.
		/// Relative distance (additional to grabDistance - doesn't try to move inside grabDistance)at which the grab tries to use the balancer to move to the grab point.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 14.0f.
		/// </remarks>
		public float Move2Radius
		{
			set
			{
				if (value > 14.0f)
					value = 14.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("move2Radius", value);
			}
		}

		/// <summary>
		/// Sets the ArmStiffness setting for this <see cref="GrabHelper"/>.
		/// Stiffness of the arm.
		/// </summary>
		/// <remarks>
		/// Default value = 14.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("armStiffness", value);
			}
		}

		/// <summary>
		/// Sets the MaxReachDistance setting for this <see cref="GrabHelper"/>.
		/// distance to reach out towards the grab point.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float MaxReachDistance
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxReachDistance", value);
			}
		}

		/// <summary>
		/// Sets the OrientationConstraintScale setting for this <see cref="GrabHelper"/>.
		/// scale torque used to rotate hands to face normals.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float OrientationConstraintScale
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("orientationConstraintScale", value);
			}
		}

		/// <summary>
		/// Sets the MaxWristAngle setting for this <see cref="GrabHelper"/>.
		/// When we are grabbing the max angle the wrist ccan be at before we break the grab.
		/// </summary>
		/// <remarks>
		/// Default value = 3.1f.
		/// Min value = 0.0f.
		/// Max value = 3.2f.
		/// </remarks>
		public float MaxWristAngle
		{
			set
			{
				if (value > 3.2f)
					value = 3.2f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxWristAngle", value);
			}
		}

		/// <summary>
		/// Sets the UseHeadLookToTarget setting for this <see cref="GrabHelper"/>.
		/// if true, the character will look at targetForHeadLook after a hand grabs until the end of the behavior. (Before grabbing it looks at the grab target).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseHeadLookToTarget
		{
			set { SetArgument("useHeadLookToTarget", value); }
		}

		/// <summary>
		/// Sets the LookAtGrab setting for this <see cref="GrabHelper"/>.
		/// if true, the character will look at the grab.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool LookAtGrab
		{
			set { SetArgument("lookAtGrab", value); }
		}

		/// <summary>
		/// Sets the TargetForHeadLook setting for this <see cref="GrabHelper"/>.
		/// Only used if useHeadLookToTarget is true, the target in world space to look at.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 TargetForHeadLook
		{
			set { SetArgument("targetForHeadLook", value); }
		}
	}

	public sealed class HeadLookHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the HeadLookHelper for sending a HeadLook <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the HeadLook <see cref="Message"/> to.</param>
		public HeadLookHelper(Ped ped) : base(ped, "headLook")
		{
		}

		/// <summary>
		/// Sets the Damping setting for this <see cref="HeadLookHelper"/>.
		/// Damping  of the muscles.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float Damping
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("damping", value);
			}
		}

		/// <summary>
		/// Sets the Stiffness setting for this <see cref="HeadLookHelper"/>.
		/// Stiffness of the muscles.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float Stiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("stiffness", value);
			}
		}

		/// <summary>
		/// Sets the InstanceIndex setting for this <see cref="HeadLookHelper"/>.
		/// levelIndex of object to be looked at. vel parameters are ignored if this is non -1.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int InstanceIndex
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("instanceIndex", value);
			}
		}

		/// <summary>
		/// Sets the Vel setting for this <see cref="HeadLookHelper"/>.
		/// The velocity of the point being looked at.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -100.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public Vector3 Vel
		{
			set
			{
				SetArgument("vel", Vector3.Clamp(value, new Vector3(-100.0f, -100.0f, -100.0f), new Vector3(100.0f, 100.0f, 100.0f)));
			}
		}

		/// <summary>
		/// Sets the Pos setting for this <see cref="HeadLookHelper"/>.
		/// The point being looked at.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Pos
		{
			set { SetArgument("pos", value); }
		}

		/// <summary>
		/// Sets the AlwaysLook setting for this <see cref="HeadLookHelper"/>.
		/// Flag to force always to look.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AlwaysLook
		{
			set { SetArgument("alwaysLook", value); }
		}

		/// <summary>
		/// Sets the EyesHorizontal setting for this <see cref="HeadLookHelper"/>.
		/// Keep the eyes horizontal.  Use true for impact with cars.  Use false if you want better look at target accuracy when the character is on the floor or leaned over alot.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool EyesHorizontal
		{
			set { SetArgument("eyesHorizontal", value); }
		}

		/// <summary>
		/// Sets the AlwaysEyesHorizontal setting for this <see cref="HeadLookHelper"/>.
		/// Keep the eyes horizontal.  Use true for impact with cars.  Use false if you want better look at target accuracy when the character is on the floor or leaned over (when not leaned over the eyes are still kept horizontal if eyesHorizontal=true ) alot.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool AlwaysEyesHorizontal
		{
			set { SetArgument("alwaysEyesHorizontal", value); }
		}

		/// <summary>
		/// Sets the KeepHeadAwayFromGround setting for this <see cref="HeadLookHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool KeepHeadAwayFromGround
		{
			set { SetArgument("keepHeadAwayFromGround", value); }
		}

		/// <summary>
		/// Sets the TwistSpine setting for this <see cref="HeadLookHelper"/>.
		/// Allow headlook to twist spine.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool TwistSpine
		{
			set { SetArgument("twistSpine", value); }
		}
	}

	public sealed class HighFallHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the HighFallHelper for sending a HighFall <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the HighFall <see cref="Message"/> to.</param>
		public HighFallHelper(Ped ped) : base(ped, "highFall")
		{
		}

		/// <summary>
		/// Sets the BodyStiffness setting for this <see cref="HighFallHelper"/>.
		/// stiffness of body. Value feeds through to bodyBalance (synched with defaults), to armsWindmill (14 for this value at default ), legs pedal, head look and roll down stairs directly.
		/// </summary>
		/// <remarks>
		/// Default value = 11.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float BodyStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("bodyStiffness", value);
			}
		}

		/// <summary>
		/// Sets the Bodydamping setting for this <see cref="HighFallHelper"/>.
		/// The damping of the joints.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float Bodydamping
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("bodydamping", value);
			}
		}

		/// <summary>
		/// Sets the Catchfalltime setting for this <see cref="HighFallHelper"/>.
		/// The length of time before the impact that the character transitions to the landing.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Catchfalltime
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("catchfalltime", value);
			}
		}

		/// <summary>
		/// Sets the CrashOrLandCutOff setting for this <see cref="HighFallHelper"/>.
		/// 0.52angle is 0.868 dot//A threshold for deciding how far away from upright the character needs to be before bailing out (going into a foetal) instead of trying to land (keeping stretched out).  NB: never does bailout if ignorWorldCollisions true.
		/// </summary>
		/// <remarks>
		/// Default value = 0.9f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CrashOrLandCutOff
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("crashOrLandCutOff", value);
			}
		}

		/// <summary>
		/// Sets the PdStrength setting for this <see cref="HighFallHelper"/>.
		/// Strength of the controller to keep the character at angle aimAngleBase from vertical.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float PdStrength
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("pdStrength", value);
			}
		}

		/// <summary>
		/// Sets the PdDamping setting for this <see cref="HighFallHelper"/>.
		/// Damping multiplier of the controller to keep the character at angle aimAngleBase from vertical.  The actual damping is pdDamping*pdStrength*constant*angVel.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float PdDamping
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("pdDamping", value);
			}
		}

		/// <summary>
		/// Sets the ArmAngSpeed setting for this <see cref="HighFallHelper"/>.
		/// arm circling speed in armWindMillAdaptive.
		/// </summary>
		/// <remarks>
		/// Default value = 7.9f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float ArmAngSpeed
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armAngSpeed", value);
			}
		}

		/// <summary>
		/// Sets the ArmAmplitude setting for this <see cref="HighFallHelper"/>.
		/// in armWindMillAdaptive.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ArmAmplitude
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armAmplitude", value);
			}
		}

		/// <summary>
		/// Sets the ArmPhase setting for this <see cref="HighFallHelper"/>.
		/// in armWindMillAdaptive 3.1 opposite for stuntman.  1.0 old default.  0.0 in phase.
		/// </summary>
		/// <remarks>
		/// Default value = 3.1f.
		/// Min value = 0.0f.
		/// Max value = 6.3f.
		/// </remarks>
		public float ArmPhase
		{
			set
			{
				if (value > 6.3f)
					value = 6.3f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armPhase", value);
			}
		}

		/// <summary>
		/// Sets the ArmBendElbows setting for this <see cref="HighFallHelper"/>.
		/// in armWindMillAdaptive bend the elbows as a function of armAngle.  For stuntman true otherwise false.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ArmBendElbows
		{
			set { SetArgument("armBendElbows", value); }
		}

		/// <summary>
		/// Sets the LegRadius setting for this <see cref="HighFallHelper"/>.
		/// radius of legs on pedal.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 0.5f.
		/// </remarks>
		public float LegRadius
		{
			set
			{
				if (value > 0.5f)
					value = 0.5f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legRadius", value);
			}
		}

		/// <summary>
		/// Sets the LegAngSpeed setting for this <see cref="HighFallHelper"/>.
		/// in pedal.
		/// </summary>
		/// <remarks>
		/// Default value = 7.9f.
		/// Min value = 0.0f.
		/// Max value = 15.0f.
		/// </remarks>
		public float LegAngSpeed
		{
			set
			{
				if (value > 15.0f)
					value = 15.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legAngSpeed", value);
			}
		}

		/// <summary>
		/// Sets the LegAsymmetry setting for this <see cref="HighFallHelper"/>.
		/// 0.0 for stuntman.  Random offset applied per leg to the angular speed to desynchronise the pedaling - set to 0 to disable, otherwise should be set to less than the angularSpeed value.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = -10.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float LegAsymmetry
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("legAsymmetry", value);
			}
		}

		/// <summary>
		/// Sets the Arms2LegsPhase setting for this <see cref="HighFallHelper"/>.
		/// phase angle between the arms and legs circling angle.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 6.5f.
		/// </remarks>
		public float Arms2LegsPhase
		{
			set
			{
				if (value > 6.5f)
					value = 6.5f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("arms2LegsPhase", value);
			}
		}

		/// <summary>
		/// Sets the Arms2LegsSync setting for this <see cref="HighFallHelper"/>.
		/// Syncs the arms angle to what the leg angle is.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="Synchroisation.AlwaysSynced"/>.
		/// All speed/direction parameters of armswindmill are overwritten if = <see cref="Synchroisation.AlwaysSynced"/>.
		/// If <see cref="Synchroisation.SyncedAtStart"/> and you want synced arms/legs then armAngSpeed=legAngSpeed, legAsymmetry = 0.0 (to stop randomizations of the leg cicle speed).
		/// </remarks>
		public Synchroisation Arms2LegsSync
		{
			set { SetArgument("arms2LegsSync", (int) value); }
		}

		/// <summary>
		/// Sets the ArmsUp setting for this <see cref="HighFallHelper"/>.
		/// Where to put the arms when preparing to land. Approx 1 = above head, 0 = head height, -1 = down.   LT -2.0 use catchFall arms,  LT -3.0 use prepare for landing pose if Agent is due to land vertically, feet first.
		/// </summary>
		/// <remarks>
		/// Default value = -3.1f.
		/// Min value = -4.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmsUp
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -4.0f)
					value = -4.0f;
				SetArgument("armsUp", value);
			}
		}

		/// <summary>
		/// Sets the OrientateBodyToFallDirection setting for this <see cref="HighFallHelper"/>.
		/// toggle to orientate to fall direction.  i.e. orientate so that the character faces the horizontal velocity direction.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool OrientateBodyToFallDirection
		{
			set { SetArgument("orientateBodyToFallDirection", value); }
		}

		/// <summary>
		/// Sets the OrientateTwist setting for this <see cref="HighFallHelper"/>.
		/// If false don't worry about the twist angle of the character when orientating the character.  If false this allows the twist axis of the character to be free (You can get a nice twisting highFall like the one in dieHard 4 when the car goes into the helicopter).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool OrientateTwist
		{
			set { SetArgument("orientateTwist", value); }
		}

		/// <summary>
		/// Sets the OrientateMax setting for this <see cref="HighFallHelper"/>.
		/// DEVEL parameter - suggest you don't edit it.  Maximum torque the orientation controller can apply.  If 0 then no helper torques will be used.  300 will orientate the character soflty for all but extreme angles away from aimAngleBase.  If abs (current -aimAngleBase) is getting near 3.0 then this can be reduced to give a softer feel.
		/// </summary>
		/// <remarks>
		/// Default value = 300.0f.
		/// Min value = 0.0f.
		/// Max value = 2000.0f.
		/// </remarks>
		public float OrientateMax
		{
			set
			{
				if (value > 2000.0f)
					value = 2000.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("orientateMax", value);
			}
		}

		/// <summary>
		/// Sets the AlanRickman setting for this <see cref="HighFallHelper"/>.
		/// If true then orientate the character to face the point from where it started falling.  HighFall like the one in dieHard with Alan Rickman.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AlanRickman
		{
			set { SetArgument("alanRickman", value); }
		}

		/// <summary>
		/// Sets the FowardRoll setting for this <see cref="HighFallHelper"/>.
		/// Try to execute a forward Roll on landing.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FowardRoll
		{
			set { SetArgument("fowardRoll", value); }
		}

		/// <summary>
		/// Sets the UseZeroPose_withFowardRoll setting for this <see cref="HighFallHelper"/>.
		/// Blend to a zero pose when forward roll is attempted.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseZeroPose_withFowardRoll
		{
			set { SetArgument("useZeroPose_withFowardRoll", value); }
		}

		/// <summary>
		/// Sets the AimAngleBase setting for this <see cref="HighFallHelper"/>.
		/// Angle from vertical the pdController is driving to ( positive = forwards).
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -3.1f.
		/// Max value = 3.1f.
		/// </remarks>
		public float AimAngleBase
		{
			set
			{
				if (value > 3.1f)
					value = 3.1f;
				if (value < -3.1f)
					value = -3.1f;
				SetArgument("aimAngleBase", value);
			}
		}

		/// <summary>
		/// Sets the FowardVelRotation setting for this <see cref="HighFallHelper"/>.
		/// scale to add/subtract from aimAngle based on forward speed (Internal).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FowardVelRotation
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("fowardVelRotation", value);
			}
		}

		/// <summary>
		/// Sets the FootVelCompScale setting for this <see cref="HighFallHelper"/>.
		/// Scale to change to amount of vel that is added to the foot ik from the velocity (Internal).
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FootVelCompScale
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("footVelCompScale", value);
			}
		}

		/// <summary>
		/// Sets the SideD setting for this <see cref="HighFallHelper"/>.
		/// sideoffset for the feet during prepareForLanding. +ve = right.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SideD
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("sideD", value);
			}
		}

		/// <summary>
		/// Sets the FowardOffsetOfLegIK setting for this <see cref="HighFallHelper"/>.
		/// Forward offset for the feet during prepareForLanding.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FowardOffsetOfLegIK
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("fowardOffsetOfLegIK", value);
			}
		}

		/// <summary>
		/// Sets the LegL setting for this <see cref="HighFallHelper"/>.
		/// Leg Length for ik (Internal)//unused.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float LegL
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legL", value);
			}
		}

		/// <summary>
		/// Sets the CatchFallCutOff setting for this <see cref="HighFallHelper"/>.
		/// 0.5angle is 0.878 dot. Cutoff to go to the catchFall ( internal) //mmmtodo do like crashOrLandCutOff.
		/// </summary>
		/// <remarks>
		/// Default value = 0.9f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CatchFallCutOff
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("catchFallCutOff", value);
			}
		}

		/// <summary>
		/// Sets the LegStrength setting for this <see cref="HighFallHelper"/>.
		/// Strength of the legs at landing.
		/// </summary>
		/// <remarks>
		/// Default value = 12.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float LegStrength
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("legStrength", value);
			}
		}

		/// <summary>
		/// Sets the Balance setting for this <see cref="HighFallHelper"/>.
		/// If true have enough strength to balance.  If false not enough strength in legs to balance (even though bodyBlance called).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool Balance
		{
			set { SetArgument("balance", value); }
		}

		/// <summary>
		/// Sets the IgnorWorldCollisions setting for this <see cref="HighFallHelper"/>.
		/// Never go into bailout (foetal).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool IgnorWorldCollisions
		{
			set { SetArgument("ignorWorldCollisions", value); }
		}

		/// <summary>
		/// Sets the AdaptiveCircling setting for this <see cref="HighFallHelper"/>.
		/// stuntman type fall.  Arm and legs circling direction controlled by angmom and orientation.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool AdaptiveCircling
		{
			set { SetArgument("adaptiveCircling", value); }
		}

		/// <summary>
		/// Sets the Hula setting for this <see cref="HighFallHelper"/>.
		/// With stuntman type fall.  Hula reaction if can't see floor and not rotating fast.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool Hula
		{
			set { SetArgument("hula", value); }
		}

		/// <summary>
		/// Sets the MaxSpeedForRecoverableFall setting for this <see cref="HighFallHelper"/>.
		/// Character needs to be moving less than this speed to consider fall as a recoverable one.
		/// </summary>
		/// <remarks>
		/// Default value = 15.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float MaxSpeedForRecoverableFall
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxSpeedForRecoverableFall", value);
			}
		}

		/// <summary>
		/// Sets the MinSpeedForBrace setting for this <see cref="HighFallHelper"/>.
		/// Character needs to be moving at least this fast horizontally to start bracing for impact if there is an object along its trajectory.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float MinSpeedForBrace
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("minSpeedForBrace", value);
			}
		}

		/// <summary>
		/// Sets the LandingNormal setting for this <see cref="HighFallHelper"/>.
		/// Ray-cast normal doted with up direction has to be greater than this number to consider object flat enough to land on it.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LandingNormal
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("landingNormal", value);
			}
		}
	}

	public sealed class IncomingTransformsHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the IncomingTransformsHelper for sending a IncomingTransforms <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the IncomingTransforms <see cref="Message"/> to.</param>
		public IncomingTransformsHelper(Ped ped) : base(ped, "incomingTransforms")
		{
		}
	}

	/// <summary>
	/// InjuredOnGround.
	/// </summary>
	public sealed class InjuredOnGroundHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the InjuredOnGroundHelper for sending a InjuredOnGround <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the InjuredOnGround <see cref="Message"/> to.</param>
		/// <remarks>
		/// InjuredOnGround.
		/// </remarks>
		public InjuredOnGroundHelper(Ped ped) : base(ped, "injuredOnGround")
		{
		}

		/// <summary>
		/// Sets the NumInjuries setting for this <see cref="InjuredOnGroundHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int NumInjuries
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("numInjuries", value);
			}
		}

		/// <summary>
		/// Sets the Injury1Component setting for this <see cref="InjuredOnGroundHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// </remarks>
		public int Injury1Component
		{
			set
			{
				if (value < 0)
					value = 0;
				SetArgument("injury1Component", value);
			}
		}

		/// <summary>
		/// Sets the Injury2Component setting for this <see cref="InjuredOnGroundHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// </remarks>
		public int Injury2Component
		{
			set
			{
				if (value < 0)
					value = 0;
				SetArgument("injury2Component", value);
			}
		}

		/// <summary>
		/// Sets the Injury1LocalPosition setting for this <see cref="InjuredOnGroundHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Injury1LocalPosition
		{
			set { SetArgument("injury1LocalPosition", value); }
		}

		/// <summary>
		/// Sets the Injury2LocalPosition setting for this <see cref="InjuredOnGroundHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Injury2LocalPosition
		{
			set { SetArgument("injury2LocalPosition", value); }
		}

		/// <summary>
		/// Sets the Injury1LocalNormal setting for this <see cref="InjuredOnGroundHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(1.0f, 0.0f, 0.0f).
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public Vector3 Injury1LocalNormal
		{
			set
			{
				SetArgument("injury1LocalNormal", Vector3.Clamp(value, new Vector3(0.0f, 0.0f, 0.0f), new Vector3(1.0f, 1.0f, 1.0f)));
			}
		}

		/// <summary>
		/// Sets the Injury2LocalNormal setting for this <see cref="InjuredOnGroundHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(1.0f, 0.0f, 0.0f).
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public Vector3 Injury2LocalNormal
		{
			set
			{
				SetArgument("injury2LocalNormal", Vector3.Clamp(value, new Vector3(0.0f, 0.0f, 0.0f), new Vector3(1.0f, 1.0f, 1.0f)));
			}
		}

		/// <summary>
		/// Sets the AttackerPos setting for this <see cref="InjuredOnGroundHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(1.0f, 0.0f, 0.0f).
		/// Min value = 0.0f.
		/// </remarks>
		public Vector3 AttackerPos
		{
			set { SetArgument("attackerPos", Vector3.Max(value, new Vector3(0.0f, 0.0f, 0.0f))); }
		}

		/// <summary>
		/// Sets the DontReachWithLeft setting for this <see cref="InjuredOnGroundHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool DontReachWithLeft
		{
			set { SetArgument("dontReachWithLeft", value); }
		}

		/// <summary>
		/// Sets the DontReachWithRight setting for this <see cref="InjuredOnGroundHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool DontReachWithRight
		{
			set { SetArgument("dontReachWithRight", value); }
		}

		/// <summary>
		/// Sets the StrongRollForce setting for this <see cref="InjuredOnGroundHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool StrongRollForce
		{
			set { SetArgument("strongRollForce", value); }
		}
	}

	/// <summary>
	/// Carried.
	/// </summary>
	public sealed class CarriedHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the CarriedHelper for sending a Carried <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the Carried <see cref="Message"/> to.</param>
		/// <remarks>
		/// Carried.
		/// </remarks>
		public CarriedHelper(Ped ped) : base(ped, "carried")
		{
		}
	}

	/// <summary>
	/// Dangle.
	/// </summary>
	public sealed class DangleHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the DangleHelper for sending a Dangle <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the Dangle <see cref="Message"/> to.</param>
		/// <remarks>
		/// Dangle.
		/// </remarks>
		public DangleHelper(Ped ped) : base(ped, "dangle")
		{
		}

		/// <summary>
		/// Sets the DoGrab setting for this <see cref="DangleHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool DoGrab
		{
			set { SetArgument("doGrab", value); }
		}

		/// <summary>
		/// Sets the GrabFrequency setting for this <see cref="DangleHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float GrabFrequency
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("grabFrequency", value);
			}
		}
	}

	public sealed class OnFireHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the OnFireHelper for sending a OnFire <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the OnFire <see cref="Message"/> to.</param>
		public OnFireHelper(Ped ped) : base(ped, "onFire")
		{
		}

		/// <summary>
		/// Sets the StaggerTime setting for this <see cref="OnFireHelper"/>.
		/// Max time for stumbling around before falling to ground.
		/// </summary>
		/// <remarks>
		/// Default value = 2.5f.
		/// Min value = 0.0f.
		/// Max value = 30.0f.
		/// </remarks>
		public float StaggerTime
		{
			set
			{
				if (value > 30.0f)
					value = 30.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("staggerTime", value);
			}
		}

		/// <summary>
		/// Sets the StaggerLeanRate setting for this <see cref="OnFireHelper"/>.
		/// How quickly the character leans hips when staggering.
		/// </summary>
		/// <remarks>
		/// Default value = 0.9f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float StaggerLeanRate
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("staggerLeanRate", value);
			}
		}

		/// <summary>
		/// Sets the StumbleMaxLeanBack setting for this <see cref="OnFireHelper"/>.
		/// max the character leans hips back when staggering.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 1.5f.
		/// </remarks>
		public float StumbleMaxLeanBack
		{
			set
			{
				if (value > 1.5f)
					value = 1.5f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stumbleMaxLeanBack", value);
			}
		}

		/// <summary>
		/// Sets the StumbleMaxLeanForward setting for this <see cref="OnFireHelper"/>.
		/// max the character leans hips forwards when staggering.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.5f.
		/// </remarks>
		public float StumbleMaxLeanForward
		{
			set
			{
				if (value > 1.5f)
					value = 1.5f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stumbleMaxLeanForward", value);
			}
		}

		/// <summary>
		/// Sets the ArmsWindmillWritheBlend setting for this <see cref="OnFireHelper"/>.
		/// Blend armsWindmill with the bodyWrithe arms when character is upright.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ArmsWindmillWritheBlend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armsWindmillWritheBlend", value);
			}
		}

		/// <summary>
		/// Sets the SpineStumbleWritheBlend setting for this <see cref="OnFireHelper"/>.
		/// Blend spine stumble with the bodyWrithe spine when character is upright.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SpineStumbleWritheBlend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineStumbleWritheBlend", value);
			}
		}

		/// <summary>
		/// Sets the LegsStumbleWritheBlend setting for this <see cref="OnFireHelper"/>.
		/// Blend legs stumble with the bodyWrithe legs when character is upright.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LegsStumbleWritheBlend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legsStumbleWritheBlend", value);
			}
		}

		/// <summary>
		/// Sets the ArmsPoseWritheBlend setting for this <see cref="OnFireHelper"/>.
		/// Blend the bodyWrithe arms with the current desired pose from on fire behaviour when character is on the floor.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ArmsPoseWritheBlend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armsPoseWritheBlend", value);
			}
		}

		/// <summary>
		/// Sets the SpinePoseWritheBlend setting for this <see cref="OnFireHelper"/>.
		/// Blend the bodyWrithe back with the current desired pose from on fire behaviour when character is on the floor.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SpinePoseWritheBlend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spinePoseWritheBlend", value);
			}
		}

		/// <summary>
		/// Sets the LegsPoseWritheBlend setting for this <see cref="OnFireHelper"/>.
		/// Blend the bodyWrithe legs with the current desired pose from on fire behaviour when character is on the floor.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LegsPoseWritheBlend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legsPoseWritheBlend", value);
			}
		}

		/// <summary>
		/// Sets the RollOverFlag setting for this <see cref="OnFireHelper"/>.
		/// Flag to set bodyWrithe trying to rollOver.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool RollOverFlag
		{
			set { SetArgument("rollOverFlag", value); }
		}

		/// <summary>
		/// Sets the RollTorqueScale setting for this <see cref="OnFireHelper"/>.
		/// Scale rolling torque that is applied to character spine by bodyWrithe. Torque magnitude is calculated with the following formula: m_rollOverDirection*rollOverPhase*rollTorqueScale.
		/// </summary>
		/// <remarks>
		/// Default value = 25.0f.
		/// Min value = 0.0f.
		/// Max value = 300.0f.
		/// </remarks>
		public float RollTorqueScale
		{
			set
			{
				if (value > 300.0f)
					value = 300.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rollTorqueScale", value);
			}
		}

		/// <summary>
		/// Sets the PredictTime setting for this <see cref="OnFireHelper"/>.
		/// Character pose depends on character facing direction that is evaluated from its COMTM orientation. Set this value to 0 to use no orientation prediction i.e. current character COMTM orientation will be used to determine character facing direction and finally the pose bodyWrithe is blending to. Set this value to  GT  0 to predict character COMTM orientation this amout of time in seconds to the future.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float PredictTime
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("predictTime", value);
			}
		}

		/// <summary>
		/// Sets the MaxRollOverTime setting for this <see cref="OnFireHelper"/>.
		/// Rolling torque is ramped down over time. At this time in seconds torque value converges to zero. Use this parameter to restrict time the character is rolling.
		/// </summary>
		/// <remarks>
		/// Default value = 8.0f.
		/// Min value = 0.0f.
		/// Max value = 60.0f.
		/// </remarks>
		public float MaxRollOverTime
		{
			set
			{
				if (value > 60.0f)
					value = 60.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxRollOverTime", value);
			}
		}

		/// <summary>
		/// Sets the RollOverRadius setting for this <see cref="OnFireHelper"/>.
		/// Rolling torque is ramped down with distance measured from position where character hit the ground and started rolling. At this distance in meters torque value converges to zero. Use this parameter to restrict distance the character travels due to rolling.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float RollOverRadius
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rollOverRadius", value);
			}
		}
	}

	public sealed class PedalLegsHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the PedalLegsHelper for sending a PedalLegs <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the PedalLegs <see cref="Message"/> to.</param>
		public PedalLegsHelper(Ped ped) : base(ped, "pedalLegs")
		{
		}

		/// <summary>
		/// Sets the PedalLeftLeg setting for this <see cref="PedalLegsHelper"/>.
		/// pedal with this leg or not.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool PedalLeftLeg
		{
			set { SetArgument("pedalLeftLeg", value); }
		}

		/// <summary>
		/// Sets the PedalRightLeg setting for this <see cref="PedalLegsHelper"/>.
		/// pedal with this leg or not.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool PedalRightLeg
		{
			set { SetArgument("pedalRightLeg", value); }
		}

		/// <summary>
		/// Sets the BackPedal setting for this <see cref="PedalLegsHelper"/>.
		/// pedal forwards or backwards.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool BackPedal
		{
			set { SetArgument("backPedal", value); }
		}

		/// <summary>
		/// Sets the Radius setting for this <see cref="PedalLegsHelper"/>.
		/// base radius of pedal action.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float Radius
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("radius", value);
			}
		}

		/// <summary>
		/// Sets the AngularSpeed setting for this <see cref="PedalLegsHelper"/>.
		/// rate of pedaling. If adaptivePedal4Dragging is true then the legsAngularSpeed calculated to match the linear speed of the character can have a maximum value of angularSpeed (this max used to be hard coded to 13.0).
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float AngularSpeed
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("angularSpeed", value);
			}
		}

		/// <summary>
		/// Sets the LegStiffness setting for this <see cref="PedalLegsHelper"/>.
		/// stiffness of legs.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float LegStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("legStiffness", value);
			}
		}

		/// <summary>
		/// Sets the PedalOffset setting for this <see cref="PedalLegsHelper"/>.
		/// Move the centre of the pedal for the left leg up by this amount, the right leg down by this amount.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float PedalOffset
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("pedalOffset", value);
			}
		}

		/// <summary>
		/// Sets the RandomSeed setting for this <see cref="PedalLegsHelper"/>.
		/// Random seed used to generate speed changes.
		/// </summary>
		/// <remarks>
		/// Default value = 100.
		/// Min value = 0.
		/// </remarks>
		public int RandomSeed
		{
			set
			{
				if (value < 0)
					value = 0;
				SetArgument("randomSeed", value);
			}
		}

		/// <summary>
		/// Sets the SpeedAsymmetry setting for this <see cref="PedalLegsHelper"/>.
		/// Random offset applied per leg to the angular speed to desynchronise the pedaling - set to 0 to disable, otherwise should be set to less than the angularSpeed value.
		/// </summary>
		/// <remarks>
		/// Default value = 8.0f.
		/// Min value = -10.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SpeedAsymmetry
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("speedAsymmetry", value);
			}
		}

		/// <summary>
		/// Sets the AdaptivePedal4Dragging setting for this <see cref="PedalLegsHelper"/>.
		/// Will pedal in the direction of travel (if backPedal = false, against travel if backPedal = true) and with an angular velocity relative to speed upto a maximum of 13(rads/sec).  Use when being dragged by a car.  Overrides angularSpeed.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AdaptivePedal4Dragging
		{
			set { SetArgument("adaptivePedal4Dragging", value); }
		}

		/// <summary>
		/// Sets the AngSpeedMultiplier4Dragging setting for this <see cref="PedalLegsHelper"/>.
		/// newAngularSpeed = Clamp(angSpeedMultiplier4Dragging * linear_speed/pedalRadius, 0.0, angularSpeed).
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float AngSpeedMultiplier4Dragging
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("angSpeedMultiplier4Dragging", value);
			}
		}

		/// <summary>
		/// Sets the RadiusVariance setting for this <see cref="PedalLegsHelper"/>.
		/// 0-1 value used to add variance to the radius value while pedalling, to desynchonize the legs' movement and provide some variety.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float RadiusVariance
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("radiusVariance", value);
			}
		}

		/// <summary>
		/// Sets the LegAngleVariance setting for this <see cref="PedalLegsHelper"/>.
		/// 0-1 value used to vary the angle of the legs from the hips during the pedal.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LegAngleVariance
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legAngleVariance", value);
			}
		}

		/// <summary>
		/// Sets the CentreSideways setting for this <see cref="PedalLegsHelper"/>.
		/// Move the centre of the pedal for both legs sideways (+ve = right).  NB: not applied to hula.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CentreSideways
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("centreSideways", value);
			}
		}

		/// <summary>
		/// Sets the CentreForwards setting for this <see cref="PedalLegsHelper"/>.
		/// Move the centre of the pedal for both legs forward (or backward -ve).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CentreForwards
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("centreForwards", value);
			}
		}

		/// <summary>
		/// Sets the CentreUp setting for this <see cref="PedalLegsHelper"/>.
		/// Move the centre of the pedal for both legs up (or down -ve).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CentreUp
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("centreUp", value);
			}
		}

		/// <summary>
		/// Sets the Ellipse setting for this <see cref="PedalLegsHelper"/>.
		/// Turn the circle into an ellipse.  Ellipse has horizontal radius a and vertical radius b.  If ellipse is +ve then a=radius*ellipse and b=radius.  If ellipse is -ve then a=radius and b = radius*ellipse.  0.0 = vertical line of length 2*radius, 0.0:1.0 circle squashed horizontally (vertical radius = radius), 1.0=circle.  -0.001 = horizontal line of length 2*radius, -0.0:-1.0 circle squashed vertically (horizontal radius = radius), -1.0 = circle.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Ellipse
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("ellipse", value);
			}
		}

		/// <summary>
		/// Sets the DragReduction setting for this <see cref="PedalLegsHelper"/>.
		/// how much to account for the target moving through space rather than being static.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float DragReduction
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("dragReduction", value);
			}
		}

		/// <summary>
		/// Sets the Spread setting for this <see cref="PedalLegsHelper"/>.
		/// Spread legs.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Spread
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("spread", value);
			}
		}

		/// <summary>
		/// Sets the Hula setting for this <see cref="PedalLegsHelper"/>.
		/// If true circle the legs in a hula motion.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Hula
		{
			set { SetArgument("hula", value); }
		}
	}

	/// <summary>
	/// BEHAVIOURS REFERENCED: AnimPose - allows animPose to overridebodyParts: Arms (useLeftArm, useRightArm).
	/// </summary>
	public sealed class PointArmHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the PointArmHelper for sending a PointArm <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the PointArm <see cref="Message"/> to.</param>
		/// <remarks>
		/// BEHAVIOURS REFERENCED: AnimPose - allows animPose to overridebodyParts: Arms (useLeftArm, useRightArm).
		/// </remarks>
		public PointArmHelper(Ped ped) : base(ped, "pointArm")
		{
		}

		/// <summary>
		/// Sets the TargetLeft setting for this <see cref="PointArmHelper"/>.
		/// point to point to (in world space).
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 TargetLeft
		{
			set { SetArgument("targetLeft", value); }
		}

		/// <summary>
		/// Sets the TwistLeft setting for this <see cref="PointArmHelper"/>.
		/// twist of the arm around point direction.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TwistLeft
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("twistLeft", value);
			}
		}

		/// <summary>
		/// Sets the ArmStraightnessLeft setting for this <see cref="PointArmHelper"/>.
		/// values less than 1 can give the arm a more bent look.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmStraightnessLeft
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armStraightnessLeft", value);
			}
		}

		/// <summary>
		/// Sets the UseLeftArm setting for this <see cref="PointArmHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseLeftArm
		{
			set { SetArgument("useLeftArm", value); }
		}

		/// <summary>
		/// Sets the ArmStiffnessLeft setting for this <see cref="PointArmHelper"/>.
		/// stiffness of arm.
		/// </summary>
		/// <remarks>
		/// Default value = 15.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmStiffnessLeft
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("armStiffnessLeft", value);
			}
		}

		/// <summary>
		/// Sets the ArmDampingLeft setting for this <see cref="PointArmHelper"/>.
		/// damping value for arm used to point.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmDampingLeft
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armDampingLeft", value);
			}
		}

		/// <summary>
		/// Sets the InstanceIndexLeft setting for this <see cref="PointArmHelper"/>.
		/// level index of thing to point at, or -1 for none. if -1, target is specified in world space, otherwise it is an offset from the object specified by this index.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int InstanceIndexLeft
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("instanceIndexLeft", value);
			}
		}

		/// <summary>
		/// Sets the PointSwingLimitLeft setting for this <see cref="PointArmHelper"/>.
		/// Swing limit.
		/// </summary>
		/// <remarks>
		/// Default value = 1.5f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float PointSwingLimitLeft
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("pointSwingLimitLeft", value);
			}
		}

		/// <summary>
		/// Sets the UseZeroPoseWhenNotPointingLeft setting for this <see cref="PointArmHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseZeroPoseWhenNotPointingLeft
		{
			set { SetArgument("useZeroPoseWhenNotPointingLeft", value); }
		}

		/// <summary>
		/// Sets the TargetRight setting for this <see cref="PointArmHelper"/>.
		/// point to point to (in world space).
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 TargetRight
		{
			set { SetArgument("targetRight", value); }
		}

		/// <summary>
		/// Sets the TwistRight setting for this <see cref="PointArmHelper"/>.
		/// twist of the arm around point direction.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TwistRight
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("twistRight", value);
			}
		}

		/// <summary>
		/// Sets the ArmStraightnessRight setting for this <see cref="PointArmHelper"/>.
		/// values less than 1 can give the arm a more bent look.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmStraightnessRight
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armStraightnessRight", value);
			}
		}

		/// <summary>
		/// Sets the UseRightArm setting for this <see cref="PointArmHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseRightArm
		{
			set { SetArgument("useRightArm", value); }
		}

		/// <summary>
		/// Sets the ArmStiffnessRight setting for this <see cref="PointArmHelper"/>.
		/// stiffness of arm.
		/// </summary>
		/// <remarks>
		/// Default value = 15.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmStiffnessRight
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("armStiffnessRight", value);
			}
		}

		/// <summary>
		/// Sets the ArmDampingRight setting for this <see cref="PointArmHelper"/>.
		/// damping value for arm used to point.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmDampingRight
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armDampingRight", value);
			}
		}

		/// <summary>
		/// Sets the InstanceIndexRight setting for this <see cref="PointArmHelper"/>.
		/// level index of thing to point at, or -1 for none. if -1, target is specified in world space, otherwise it is an offset from the object specified by this index.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int InstanceIndexRight
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("instanceIndexRight", value);
			}
		}

		/// <summary>
		/// Sets the PointSwingLimitRight setting for this <see cref="PointArmHelper"/>.
		/// Swing limit.
		/// </summary>
		/// <remarks>
		/// Default value = 1.5f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float PointSwingLimitRight
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("pointSwingLimitRight", value);
			}
		}

		/// <summary>
		/// Sets the UseZeroPoseWhenNotPointingRight setting for this <see cref="PointArmHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseZeroPoseWhenNotPointingRight
		{
			set { SetArgument("useZeroPoseWhenNotPointingRight", value); }
		}
	}

	public sealed class PointGunHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the PointGunHelper for sending a PointGun <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the PointGun <see cref="Message"/> to.</param>
		public PointGunHelper(Ped ped) : base(ped, "pointGun")
		{
		}

		/// <summary>
		/// Sets the EnableRight setting for this <see cref="PointGunHelper"/>.
		/// Allow right hand to point/support?.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool EnableRight
		{
			set { SetArgument("enableRight", value); }
		}

		/// <summary>
		/// Sets the EnableLeft setting for this <see cref="PointGunHelper"/>.
		/// Allow right hand to point/support?.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool EnableLeft
		{
			set { SetArgument("enableLeft", value); }
		}

		/// <summary>
		/// Sets the LeftHandTarget setting for this <see cref="PointGunHelper"/>.
		/// Target for the left Hand.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 LeftHandTarget
		{
			set { SetArgument("leftHandTarget", value); }
		}

		/// <summary>
		/// Sets the LeftHandTargetIndex setting for this <see cref="PointGunHelper"/>.
		/// Index of the object that the left hand target is specified in, -1 is world space.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// </remarks>
		public int LeftHandTargetIndex
		{
			set { SetArgument("leftHandTargetIndex", value); }
		}

		/// <summary>
		/// Sets the RightHandTarget setting for this <see cref="PointGunHelper"/>.
		/// Target for the right Hand.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 RightHandTarget
		{
			set { SetArgument("rightHandTarget", value); }
		}

		/// <summary>
		/// Sets the RightHandTargetIndex setting for this <see cref="PointGunHelper"/>.
		/// Index of the object that the right hand target is specified in, -1 is world space.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// </remarks>
		public int RightHandTargetIndex
		{
			set { SetArgument("rightHandTargetIndex", value); }
		}

		/// <summary>
		/// Sets the LeadTarget setting for this <see cref="PointGunHelper"/>.
		/// NB: Only Applied to single handed weapons (some more work is required to have this tech on two handed weapons). Amount to lead target based on target velocity relative to the chest.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float LeadTarget
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leadTarget", value);
			}
		}

		/// <summary>
		/// Sets the ArmStiffness setting for this <see cref="PointGunHelper"/>.
		/// Stiffness of the arm.
		/// </summary>
		/// <remarks>
		/// Default value = 14.0f.
		/// Min value = 2.0f.
		/// Max value = 15.0f.
		/// </remarks>
		public float ArmStiffness
		{
			set
			{
				if (value > 15.0f)
					value = 15.0f;
				if (value < 2.0f)
					value = 2.0f;
				SetArgument("armStiffness", value);
			}
		}

		/// <summary>
		/// Sets the ArmStiffnessDetSupport setting for this <see cref="PointGunHelper"/>.
		/// Stiffness of the arm on pointing arm when a support arm is detached from a two-handed weapon.
		/// </summary>
		/// <remarks>
		/// Default value = 8.0f.
		/// Min value = 2.0f.
		/// Max value = 15.0f.
		/// </remarks>
		public float ArmStiffnessDetSupport
		{
			set
			{
				if (value > 15.0f)
					value = 15.0f;
				if (value < 2.0f)
					value = 2.0f;
				SetArgument("armStiffnessDetSupport", value);
			}
		}

		/// <summary>
		/// Sets the ArmDamping setting for this <see cref="PointGunHelper"/>.
		/// Damping.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.1f.
		/// Max value = 5.0f.
		/// </remarks>
		public float ArmDamping
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("armDamping", value);
			}
		}

		/// <summary>
		/// Sets the GravityOpposition setting for this <see cref="PointGunHelper"/>.
		/// Amount of gravity opposition on pointing arm.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float GravityOpposition
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("gravityOpposition", value);
			}
		}

		/// <summary>
		/// Sets the GravOppDetachedSupport setting for this <see cref="PointGunHelper"/>.
		/// Amount of gravity opposition on pointing arm when a support arm is detached from a two-handed weapon.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float GravOppDetachedSupport
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("gravOppDetachedSupport", value);
			}
		}

		/// <summary>
		/// Sets the MassMultDetachedSupport setting for this <see cref="PointGunHelper"/>.
		/// Amount of mass of weapon taken into account by gravity opposition on pointing arm when a support arm is detached from a two-handed weapon.  The lower the value the more the character doesn't know about the weapon mass and therefore is more affected by it.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float MassMultDetachedSupport
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("massMultDetachedSupport", value);
			}
		}

		/// <summary>
		/// Sets the AllowShotLooseness setting for this <see cref="PointGunHelper"/>.
		/// Allow shot to set a lower arm muscleStiffness than pointGun normally would.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AllowShotLooseness
		{
			set { SetArgument("allowShotLooseness", value); }
		}

		/// <summary>
		/// Sets the ClavicleBlend setting for this <see cref="PointGunHelper"/>.
		/// How much of blend should come from incoming transforms 0(all IK) .. 1(all ITMs)   For pointing arms only.  (Support arm uses the IK solution as is for clavicles).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ClavicleBlend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("clavicleBlend", value);
			}
		}

		/// <summary>
		/// Sets the ElbowAttitude setting for this <see cref="PointGunHelper"/>.
		/// Controls arm twist. (except in pistolIK).
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ElbowAttitude
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("elbowAttitude", value);
			}
		}

		/// <summary>
		/// Sets the SupportConstraint setting for this <see cref="PointGunHelper"/>.
		/// Type of constraint between the support hand and gun.  0=no constraint, 1=hard distance constraint, 2=Force based constraint, 3=hard spherical constraint.
		/// </summary>
		/// <remarks>
		/// Default value = 1.
		/// Min value = 0.
		/// Max value = 3.
		/// </remarks>
		public int SupportConstraint
		{
			set
			{
				if (value > 3)
					value = 3;
				if (value < 0)
					value = 0;
				SetArgument("supportConstraint", value);
			}
		}

		/// <summary>
		/// Sets the ConstraintMinDistance setting for this <see cref="PointGunHelper"/>.
		/// For supportConstraint = 1: Support hand constraint distance will be slowly reduced until it hits this value.  This is for stability and also allows the pointing arm to lead a little.  Don't set lower than NM_MIN_STABLE_DISTANCECONSTRAINT_DISTANCE 0.001f.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 0.1f.
		/// </remarks>
		public float ConstraintMinDistance
		{
			set
			{
				if (value > 0.1f)
					value = 0.1f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("constraintMinDistance", value);
			}
		}

		/// <summary>
		/// Sets the MakeConstraintDistance setting for this <see cref="PointGunHelper"/>.
		/// For supportConstraint = 1:  Minimum distance within which support hand constraint will be made.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float MakeConstraintDistance
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("makeConstraintDistance", value);
			}
		}

		/// <summary>
		/// Sets the ReduceConstraintLengthVel setting for this <see cref="PointGunHelper"/>.
		/// For supportConstraint = 1:  Velocity at which to reduce the support hand constraint length.
		/// </summary>
		/// <remarks>
		/// Default value = 1.5f.
		/// Min value = 0.1f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ReduceConstraintLengthVel
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("reduceConstraintLengthVel", value);
			}
		}

		/// <summary>
		/// Sets the BreakingStrength setting for this <see cref="PointGunHelper"/>.
		/// For supportConstraint = 1: strength of the supporting hands constraint (kg m/s), -1 to ignore/disable.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float BreakingStrength
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("breakingStrength", value);
			}
		}

		/// <summary>
		/// Sets the BrokenSupportTime setting for this <see cref="PointGunHelper"/>.
		/// Once constraint is broken then do not try to reconnect/support for this amount of time.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float BrokenSupportTime
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("brokenSupportTime", value);
			}
		}

		/// <summary>
		/// Sets the BrokenToSideProb setting for this <see cref="PointGunHelper"/>.
		/// Probability that the when a constraint is broken that during brokenSupportTime a side pose will be selected.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float BrokenToSideProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("brokenToSideProb", value);
			}
		}

		/// <summary>
		/// Sets the ConnectAfter setting for this <see cref="PointGunHelper"/>.
		/// If gunArm has been controlled by other behaviours for this time when it could have been pointing but couldn't due to pointing only allowed if connected, change gunArm pose to something that could connect for connectFor seconds.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float ConnectAfter
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("connectAfter", value);
			}
		}

		/// <summary>
		/// Sets the ConnectFor setting for this <see cref="PointGunHelper"/>.
		/// Time to try to reconnect for.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float ConnectFor
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("connectFor", value);
			}
		}

		/// <summary>
		/// Sets the OneHandedPointing setting for this <see cref="PointGunHelper"/>.
		/// 0 = don't allow, 1= allow for kPistol(two handed pistol) only, 2 = allow for kRifle only, 3 = allow for kPistol and kRifle. Allow one handed pointing - no constraint if cant be supported .  If not allowed then gunHand does not try to point at target if it cannot be supported - the constraint will be controlled by always support.
		/// </summary>
		/// <remarks>
		/// Default value = 1.
		/// Min value = 0.
		/// Max value = 3.
		/// </remarks>
		public int OneHandedPointing
		{
			set
			{
				if (value > 3)
					value = 3;
				if (value < 0)
					value = 0;
				SetArgument("oneHandedPointing", value);
			}
		}

		/// <summary>
		/// Sets the AlwaysSupport setting for this <see cref="PointGunHelper"/>.
		/// Support a non pointing gunHand i.e. if in zero pose (constrain as well  if constraint possible).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AlwaysSupport
		{
			set { SetArgument("alwaysSupport", value); }
		}

		/// <summary>
		/// Sets the PoseUnusedGunArm setting for this <see cref="PointGunHelper"/>.
		/// Apply neutral pose when a gun arm isn't in use.  NB: at the moment Rifle hand is always controlled by pointGun.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool PoseUnusedGunArm
		{
			set { SetArgument("poseUnusedGunArm", value); }
		}

		/// <summary>
		/// Sets the PoseUnusedSupportArm setting for this <see cref="PointGunHelper"/>.
		/// Apply neutral pose when a support arm isn't in use.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool PoseUnusedSupportArm
		{
			set { SetArgument("poseUnusedSupportArm", value); }
		}

		/// <summary>
		/// Sets the PoseUnusedOtherArm setting for this <see cref="PointGunHelper"/>.
		/// Apply neutral pose to the non-gun arm (otherwise it is always under the control of other behaviours or not set). If the non-gun hand is a supporting hand it is not controlled by this parameter but by poseUnusedSupportArm.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool PoseUnusedOtherArm
		{
			set { SetArgument("poseUnusedOtherArm", value); }
		}

		/// <summary>
		/// Sets the MaxAngleAcross setting for this <see cref="PointGunHelper"/>.
		/// max aiming angle(deg) sideways across body midline measured from chest forward that the character will try to point.
		/// </summary>
		/// <remarks>
		/// Default value = 90.0f.
		/// Min value = 0.0f.
		/// Max value = 180.0f.
		/// </remarks>
		public float MaxAngleAcross
		{
			set
			{
				if (value > 180.0f)
					value = 180.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxAngleAcross", value);
			}
		}

		/// <summary>
		/// Sets the MaxAngleAway setting for this <see cref="PointGunHelper"/>.
		/// max aiming angle(deg) sideways away from body midline measured from chest forward that the character will try to point.
		/// </summary>
		/// <remarks>
		/// Default value = 90.0f.
		/// Min value = 0.0f.
		/// Max value = 180.0f.
		/// </remarks>
		public float MaxAngleAway
		{
			set
			{
				if (value > 180.0f)
					value = 180.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxAngleAway", value);
			}
		}

		/// <summary>
		/// Sets the FallingLimits setting for this <see cref="PointGunHelper"/>.
		/// 0= don't apply limits.  1=apply the limits below only when the character is falling.  2 =  always apply these limits (instead of applying maxAngleAcross and maxAngleAway which only limits the horizontal angle but implicity limits the updown (the limit shape is a vertical hinge).
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int FallingLimits
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("fallingLimits", value);
			}
		}

		/// <summary>
		/// Sets the AcrossLimit setting for this <see cref="PointGunHelper"/>.
		/// max aiming angle(deg) sideways across body midline measured from chest forward that the character will try to point.  i.e. for rightHanded gun this is the angle left of the midline.
		/// </summary>
		/// <remarks>
		/// Default value = 90.0f.
		/// Min value = 0.0f.
		/// Max value = 180.0f.
		/// </remarks>
		public float AcrossLimit
		{
			set
			{
				if (value > 180.0f)
					value = 180.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("acrossLimit", value);
			}
		}

		/// <summary>
		/// Sets the AwayLimit setting for this <see cref="PointGunHelper"/>.
		/// max aiming angle(deg) sideways away from body midline measured from chest forward that the character will try to point.  i.e. for rightHanded gun this is the angle right of the midline.
		/// </summary>
		/// <remarks>
		/// Default value = 90.0f.
		/// Min value = 0.0f.
		/// Max value = 180.0f.
		/// </remarks>
		public float AwayLimit
		{
			set
			{
				if (value > 180.0f)
					value = 180.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("awayLimit", value);
			}
		}

		/// <summary>
		/// Sets the UpLimit setting for this <see cref="PointGunHelper"/>.
		/// max aiming angle(deg) upwards from body midline measured from chest forward that the character will try to point.
		/// </summary>
		/// <remarks>
		/// Default value = 90.0f.
		/// Min value = 0.0f.
		/// Max value = 180.0f.
		/// </remarks>
		public float UpLimit
		{
			set
			{
				if (value > 180.0f)
					value = 180.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("upLimit", value);
			}
		}

		/// <summary>
		/// Sets the DownLimit setting for this <see cref="PointGunHelper"/>.
		/// max aiming angle(deg) downwards from body midline measured from chest forward that the character will try to point.
		/// </summary>
		/// <remarks>
		/// Default value = 45.0f.
		/// Min value = 0.0f.
		/// Max value = 180.0f.
		/// </remarks>
		public float DownLimit
		{
			set
			{
				if (value > 180.0f)
					value = 180.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("downLimit", value);
			}
		}

		/// <summary>
		/// Sets the RifleFall setting for this <see cref="PointGunHelper"/>.
		/// Pose the rifle hand to reduce complications with collisions. 0 = false, 1 = always when falling, 2 = when falling except if falling backwards.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int RifleFall
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("rifleFall", value);
			}
		}

		/// <summary>
		/// Sets the FallingSupport setting for this <see cref="PointGunHelper"/>.
		/// Allow supporting of a rifle(or two handed pistol) when falling. 0 = false, 1 = support if allowed, 2 = support until constraint not active (don't allow support to restart), 3 = support until constraint not effective (support hand to support distance must be less than 0.15 - don't allow support to restart).
		/// </summary>
		/// <remarks>
		/// Default value = 1.
		/// Min value = 0.
		/// Max value = 3.
		/// </remarks>
		public int FallingSupport
		{
			set
			{
				if (value > 3)
					value = 3;
				if (value < 0)
					value = 0;
				SetArgument("fallingSupport", value);
			}
		}

		/// <summary>
		/// Sets the FallingTypeSupport setting for this <see cref="PointGunHelper"/>.
		/// What is considered a fall by fallingSupport). Apply fallingSupport 0=never(will support if allowed), 1 = falling, 2 = falling except if falling backwards, 3 = falling and collided, 4 = falling and collided except if falling backwards, 5 = falling except if falling backwards until collided.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 5.
		/// </remarks>
		public int FallingTypeSupport
		{
			set
			{
				if (value > 5)
					value = 5;
				if (value < 0)
					value = 0;
				SetArgument("fallingTypeSupport", value);
			}
		}

		/// <summary>
		/// Sets the PistolNeutralType setting for this <see cref="PointGunHelper"/>.
		/// 0 = byFace, 1=acrossFront, 2=bySide.  NB: bySide is not connectible so be careful if combined with kPistol and oneHandedPointing = 0 or 2.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int PistolNeutralType
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("pistolNeutralType", value);
			}
		}

		/// <summary>
		/// Sets the NeutralPoint4Pistols setting for this <see cref="PointGunHelper"/>.
		/// NOT IMPLEMENTED YET KEEP=false - use pointing for neutral targets in pistol modes.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool NeutralPoint4Pistols
		{
			set { SetArgument("neutralPoint4Pistols", value); }
		}

		/// <summary>
		/// Sets the NeutralPoint4Rifle setting for this <see cref="PointGunHelper"/>.
		/// use pointing for neutral targets in rifle mode.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool NeutralPoint4Rifle
		{
			set { SetArgument("neutralPoint4Rifle", value); }
		}

		/// <summary>
		/// Sets the CheckNeutralPoint setting for this <see cref="PointGunHelper"/>.
		/// Check the neutral pointing is pointable, if it isn't then choose a neutral pose instead.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool CheckNeutralPoint
		{
			set { SetArgument("checkNeutralPoint", value); }
		}

		/// <summary>
		/// Sets the Point2Side setting for this <see cref="PointGunHelper"/>.
		/// side, up, back) side is left for left arm, right for right arm mmmmtodo.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(5.0f, -5.0f, -2.0f).
		/// </remarks>
		public Vector3 Point2Side
		{
			set { SetArgument("point2Side", value); }
		}

		/// <summary>
		/// Sets the Add2WeaponDistSide setting for this <see cref="PointGunHelper"/>.
		/// add to weaponDistance for point2Side neutral pointing (to straighten the arm).
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = -1.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float Add2WeaponDistSide
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("add2WeaponDistSide", value);
			}
		}

		/// <summary>
		/// Sets the Point2Connect setting for this <see cref="PointGunHelper"/>.
		/// side, up, back) side is left for left arm, right for rght arm mmmmtodo.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(-1.0f, -0.9f, -0.2f).
		/// </remarks>
		public Vector3 Point2Connect
		{
			set { SetArgument("point2Connect", value); }
		}

		/// <summary>
		/// Sets the Add2WeaponDistConnect setting for this <see cref="PointGunHelper"/>.
		/// add to weaponDistance for point2Connect neutral pointing (to straighten the arm).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float Add2WeaponDistConnect
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("add2WeaponDistConnect", value);
			}
		}

		/// <summary>
		/// Sets the UsePistolIK setting for this <see cref="PointGunHelper"/>.
		/// enable new ik for pistol pointing.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UsePistolIK
		{
			set { SetArgument("usePistolIK", value); }
		}

		/// <summary>
		/// Sets the UseSpineTwist setting for this <see cref="PointGunHelper"/>.
		/// Use spine twist to orient chest?.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseSpineTwist
		{
			set { SetArgument("useSpineTwist", value); }
		}

		/// <summary>
		/// Sets the UseTurnToTarget setting for this <see cref="PointGunHelper"/>.
		/// Turn balancer to help gun point at target.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseTurnToTarget
		{
			set { SetArgument("useTurnToTarget", value); }
		}

		/// <summary>
		/// Sets the UseHeadLook setting for this <see cref="PointGunHelper"/>.
		/// Use head look to drive head?.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseHeadLook
		{
			set { SetArgument("useHeadLook", value); }
		}

		/// <summary>
		/// Sets the ErrorThreshold setting for this <see cref="PointGunHelper"/>.
		/// angular difference between pointing direction and target direction above which feedback will be generated.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 3.1f.
		/// </remarks>
		public float ErrorThreshold
		{
			set
			{
				if (value > 3.1f)
					value = 3.1f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("errorThreshold", value);
			}
		}

		/// <summary>
		/// Sets the FireWeaponRelaxTime setting for this <see cref="PointGunHelper"/>.
		/// Duration of arms relax following firing weapon.  NB:This is clamped (0,5) in pointGun.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float FireWeaponRelaxTime
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("fireWeaponRelaxTime", value);
			}
		}

		/// <summary>
		/// Sets the FireWeaponRelaxAmount setting for this <see cref="PointGunHelper"/>.
		/// Relax multiplier following firing weapon. Recovers over relaxTime.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.1f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FireWeaponRelaxAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("fireWeaponRelaxAmount", value);
			}
		}

		/// <summary>
		/// Sets the FireWeaponRelaxDistance setting for this <see cref="PointGunHelper"/>.
		/// Range of motion for ik-based recoil.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 0.3f.
		/// </remarks>
		public float FireWeaponRelaxDistance
		{
			set
			{
				if (value > 0.3f)
					value = 0.3f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("fireWeaponRelaxDistance", value);
			}
		}

		/// <summary>
		/// Sets the UseIncomingTransforms setting for this <see cref="PointGunHelper"/>.
		/// Use the incoming transforms to inform the pointGun of the primaryWeaponDistance, poleVector for the arm.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseIncomingTransforms
		{
			set { SetArgument("useIncomingTransforms", value); }
		}

		/// <summary>
		/// Sets the MeasureParentOffset setting for this <see cref="PointGunHelper"/>.
		/// If useIncomingTransforms = true and measureParentOffset=true then measure the Pointing-from offset from parent effector, using itms - this should point the barrel of the gun to the target.  This is added to the rightHandParentOffset. NB NOT used if rightHandParentEffector LT 0.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool MeasureParentOffset
		{
			set { SetArgument("measureParentOffset", value); }
		}

		/// <summary>
		/// Sets the LeftHandParentOffset setting for this <see cref="PointGunHelper"/>.
		/// Pointing-from offset from parent effector, expressed in spine3's frame, x = back/forward, y = right/left, z = up/down.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 LeftHandParentOffset
		{
			set { SetArgument("leftHandParentOffset", value); }
		}

		/// <summary>
		/// Sets the LeftHandParentEffector setting for this <see cref="PointGunHelper"/>.
		/// 1 = Use leftShoulder. Effector from which the left hand pointing originates. ie, point from this part to the target. -1 causes default offset for active weapon mode to be applied.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// Max value = 21.
		/// </remarks>
		public int LeftHandParentEffector
		{
			set
			{
				if (value > 21)
					value = 21;
				if (value < -1)
					value = -1;
				SetArgument("leftHandParentEffector", value);
			}
		}

		/// <summary>
		/// Sets the RightHandParentOffset setting for this <see cref="PointGunHelper"/>.
		/// Pointing-from offset from parent effector, expressed in spine3's frame, x = back/forward, y = right/left, z = up/down. This is added to the measured one if useIncomingTransforms=true and measureParentOffset=true.  NB NOT used if rightHandParentEffector LT 0.  Pistol(0,0,0) Rifle(0.0032, 0.0, -0.0).
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 RightHandParentOffset
		{
			set { SetArgument("rightHandParentOffset", value); }
		}

		/// <summary>
		/// Sets the RightHandParentEffector setting for this <see cref="PointGunHelper"/>.
		/// 1 = Use rightShoulder.. Effector from which the right hand pointing originates. ie, point from this part to the target. -1 causes default offset for active weapon mode to be applied.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// Max value = 21.
		/// </remarks>
		public int RightHandParentEffector
		{
			set
			{
				if (value > 21)
					value = 21;
				if (value < -1)
					value = -1;
				SetArgument("rightHandParentEffector", value);
			}
		}

		/// <summary>
		/// Sets the PrimaryHandWeaponDistance setting for this <see cref="PointGunHelper"/>.
		/// Distance from the shoulder to hold the weapon. If -1 and useIncomingTransforms then weaponDistance is read from ITMs. weaponDistance=primaryHandWeaponDistance clamped [0.2f:m_maxArmReach=0.65] if useIncomingTransforms = false. pistol 0.60383, rifle 0.336.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float PrimaryHandWeaponDistance
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("primaryHandWeaponDistance", value);
			}
		}

		/// <summary>
		/// Sets the ConstrainRifle setting for this <see cref="PointGunHelper"/>.
		/// Use hard constraint to keep rifle stock against shoulder?.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ConstrainRifle
		{
			set { SetArgument("constrainRifle", value); }
		}

		/// <summary>
		/// Sets the RifleConstraintMinDistance setting for this <see cref="PointGunHelper"/>.
		/// Rifle constraint distance. Deliberately kept large to create a flat constraint surface where rifle meets the shoulder.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// </remarks>
		public float RifleConstraintMinDistance
		{
			set
			{
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rifleConstraintMinDistance", value);
			}
		}

		/// <summary>
		/// Sets the DisableArmCollisions setting for this <see cref="PointGunHelper"/>.
		/// Disable collisions between right hand/forearm and the torso/legs.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool DisableArmCollisions
		{
			set { SetArgument("disableArmCollisions", value); }
		}

		/// <summary>
		/// Sets the DisableRifleCollisions setting for this <see cref="PointGunHelper"/>.
		/// Disable collisions between right hand/forearm and spine3/spine2 if in rifle mode.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool DisableRifleCollisions
		{
			set { SetArgument("disableRifleCollisions", value); }
		}
	}

	/// <summary>
	/// Seldom set parameters for pointGun - just to keep number of parameters in any message less than or equal to 64.
	/// </summary>
	public sealed class PointGunExtraHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the PointGunExtraHelper for sending a PointGunExtra <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the PointGunExtra <see cref="Message"/> to.</param>
		/// <remarks>
		/// Seldom set parameters for pointGun - just to keep number of parameters in any message less than or equal to 64.
		/// </remarks>
		public PointGunExtraHelper(Ped ped) : base(ped, "pointGunExtra")
		{
		}

		/// <summary>
		/// Sets the ConstraintStrength setting for this <see cref="PointGunExtraHelper"/>.
		/// For supportConstraint = 2: force constraint strength of the supporting hands - it gets shaky at about 4.0.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float ConstraintStrength
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("constraintStrength", value);
			}
		}

		/// <summary>
		/// Sets the ConstraintThresh setting for this <see cref="PointGunExtraHelper"/>.
		/// For supportConstraint = 2:  Like makeConstraintDistance. Force starts acting when the hands are  LT  3.0*thresh apart but is maximum strength  LT  thresh. For comparison: 0.1 is used for reachForWound in shot, 0.25 is used in grab.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ConstraintThresh
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("constraintThresh", value);
			}
		}

		/// <summary>
		/// Sets the WeaponMask setting for this <see cref="PointGunExtraHelper"/>.
		/// Currently unused - no intoWorldTest. RAGE bit mask to exclude weapons from ray probe - currently defaults to MP3 weapon flag.
		/// </summary>
		/// <remarks>
		/// Default value = 1024.
		/// Min value = 0.
		/// </remarks>
		public int WeaponMask
		{
			set
			{
				if (value < 0)
					value = 0;
				SetArgument("weaponMask", value);
			}
		}

		/// <summary>
		/// Sets the TimeWarpActive setting for this <see cref="PointGunExtraHelper"/>.
		/// Is timeWarpActive enabled?.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool TimeWarpActive
		{
			set { SetArgument("timeWarpActive", value); }
		}

		/// <summary>
		/// Sets the TimeWarpStrengthScale setting for this <see cref="PointGunExtraHelper"/>.
		/// Scale for arm and helper strength when timewarp is enabled. 1 = normal compensation.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.1f.
		/// Max value = 2.0f.
		/// </remarks>
		public float TimeWarpStrengthScale
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("timeWarpStrengthScale", value);
			}
		}

		/// <summary>
		/// Sets the OriStiff setting for this <see cref="PointGunExtraHelper"/>.
		/// Hand stabilization controller stiffness.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float OriStiff
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("oriStiff", value);
			}
		}

		/// <summary>
		/// Sets the OriDamp setting for this <see cref="PointGunExtraHelper"/>.
		/// Hand stabilization controller damping.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float OriDamp
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("oriDamp", value);
			}
		}

		/// <summary>
		/// Sets the PosStiff setting for this <see cref="PointGunExtraHelper"/>.
		/// Hand stabilization controller stiffness.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float PosStiff
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("posStiff", value);
			}
		}

		/// <summary>
		/// Sets the PosDamp setting for this <see cref="PointGunExtraHelper"/>.
		/// Hand stabilization controller damping.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float PosDamp
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("posDamp", value);
			}
		}
	}

	public sealed class RollDownStairsHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the RollDownStairsHelper for sending a RollDownStairs <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the RollDownStairs <see cref="Message"/> to.</param>
		public RollDownStairsHelper(Ped ped) : base(ped, "rollDownStairs")
		{
		}

		/// <summary>
		/// Sets the Stiffness setting for this <see cref="RollDownStairsHelper"/>.
		/// Effector Stiffness. value feeds through to rollUp directly.
		/// </summary>
		/// <remarks>
		/// Default value = 11.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float Stiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("stiffness", value);
			}
		}

		/// <summary>
		/// Sets the Damping setting for this <see cref="RollDownStairsHelper"/>.
		/// Effector  Damping.
		/// </summary>
		/// <remarks>
		/// Default value = 1.4f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float Damping
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("damping", value);
			}
		}

		/// <summary>
		/// Sets the Forcemag setting for this <see cref="RollDownStairsHelper"/>.
		/// Helper force strength.  Do not go above 1 for a rollDownStairs/roll along ground reaction.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float Forcemag
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("forcemag", value);
			}
		}

		/// <summary>
		/// Sets the M_useArmToSlowDown setting for this <see cref="RollDownStairsHelper"/>.
		/// the degree to which the character will try to stop a barrel roll with his arms.
		/// </summary>
		/// <remarks>
		/// Default value = -1.9f.
		/// Min value = -3.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float M_useArmToSlowDown
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < -3.0f)
					value = -3.0f;
				SetArgument("m_useArmToSlowDown", value);
			}
		}

		/// <summary>
		/// Sets the UseZeroPose setting for this <see cref="RollDownStairsHelper"/>.
		/// Blends between a zeroPose and the Rollup, Faster the character is rotating the less the zeroPose.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseZeroPose
		{
			set { SetArgument("useZeroPose", value); }
		}

		/// <summary>
		/// Sets the SpinWhenInAir setting for this <see cref="RollDownStairsHelper"/>.
		/// Applied cheat forces to spin the character when in the air, the forces are 40% of the forces applied when touching the ground.  Be careful little bunny rabbits, the character could spin unnaturally in the air.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SpinWhenInAir
		{
			set { SetArgument("spinWhenInAir", value); }
		}

		/// <summary>
		/// Sets the M_armReachAmount setting for this <see cref="RollDownStairsHelper"/>.
		/// how much the character reaches with his arms to brace against the ground.
		/// </summary>
		/// <remarks>
		/// Default value = 1.4f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float M_armReachAmount
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("m_armReachAmount", value);
			}
		}

		/// <summary>
		/// Sets the M_legPush setting for this <see cref="RollDownStairsHelper"/>.
		/// amount that the legs push outwards when tumbling.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float M_legPush
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("m_legPush", value);
			}
		}

		/// <summary>
		/// Sets the TryToAvoidHeadButtingGround setting for this <see cref="RollDownStairsHelper"/>.
		/// Blends between a zeroPose and the Rollup, Faster the character is rotating the less the zeroPose.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool TryToAvoidHeadButtingGround
		{
			set { SetArgument("tryToAvoidHeadButtingGround", value); }
		}

		/// <summary>
		/// Sets the ArmReachLength setting for this <see cref="RollDownStairsHelper"/>.
		/// the length that the arm reaches and so how much it straightens.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ArmReachLength
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armReachLength", value);
			}
		}

		/// <summary>
		/// Sets the CustomRollDir setting for this <see cref="RollDownStairsHelper"/>.
		/// pass in a custom direction in to have the character try and roll in that direction.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 1.0f).
		/// Min value = 1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public Vector3 CustomRollDir
		{
			set
			{
				SetArgument("customRollDir", Vector3.Clamp(value, new Vector3(1.0f, 1.0f, 1.0f), new Vector3(1.0f, 1.0f, 1.0f)));
			}
		}

		/// <summary>
		/// Sets the UseCustomRollDir setting for this <see cref="RollDownStairsHelper"/>.
		/// pass in true to use the customRollDir parameter.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseCustomRollDir
		{
			set { SetArgument("useCustomRollDir", value); }
		}

		/// <summary>
		/// Sets the StiffnessDecayTarget setting for this <see cref="RollDownStairsHelper"/>.
		/// The target linear velocity used to start the rolling.
		/// </summary>
		/// <remarks>
		/// Default value = 9.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float StiffnessDecayTarget
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stiffnessDecayTarget", value);
			}
		}

		/// <summary>
		/// Sets the StiffnessDecayTime setting for this <see cref="RollDownStairsHelper"/>.
		/// time, in seconds, to decay stiffness down to the stiffnessDecayTarget value (or -1 to disable).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float StiffnessDecayTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("stiffnessDecayTime", value);
			}
		}

		/// <summary>
		/// Sets the AsymmetricalLegs setting for this <see cref="RollDownStairsHelper"/>.
		/// 0 is no leg asymmetry in 'foetal' position.  greater than 0 a asymmetricalLegs-rand(30%), added/minus each joint of the legs in radians.  Random number changes about once every roll.  0.4 gives a lot of asymmetry.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float AsymmetricalLegs
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("asymmetricalLegs", value);
			}
		}

		/// <summary>
		/// Sets the ZAxisSpinReduction setting for this <see cref="RollDownStairsHelper"/>.
		/// Tries to reduce the spin around the z axis. Scale 0 - 1.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ZAxisSpinReduction
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("zAxisSpinReduction", value);
			}
		}

		/// <summary>
		/// Sets the TargetLinearVelocityDecayTime setting for this <see cref="RollDownStairsHelper"/>.
		/// Time for the targetlinearVelocity to decay to zero.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float TargetLinearVelocityDecayTime
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("targetLinearVelocityDecayTime", value);
			}
		}

		/// <summary>
		/// Sets the TargetLinearVelocity setting for this <see cref="RollDownStairsHelper"/>.
		/// Helper torques are applied to match the spin of the character to the max of targetLinearVelocity and COMVelMag.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float TargetLinearVelocity
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("targetLinearVelocity", value);
			}
		}

		/// <summary>
		/// Sets the OnlyApplyHelperForces setting for this <see cref="RollDownStairsHelper"/>.
		/// Don't use rollup if true.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool OnlyApplyHelperForces
		{
			set { SetArgument("onlyApplyHelperForces", value); }
		}

		/// <summary>
		/// Sets the UseVelocityOfObjectBelow setting for this <see cref="RollDownStairsHelper"/>.
		/// scale applied cheat forces/torques to (zero) if object underneath character has velocity greater than 1.f.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseVelocityOfObjectBelow
		{
			set { SetArgument("useVelocityOfObjectBelow", value); }
		}

		/// <summary>
		/// Sets the UseRelativeVelocity setting for this <see cref="RollDownStairsHelper"/>.
		/// useVelocityOfObjectBelow uses a relative velocity of the character to the object underneath.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseRelativeVelocity
		{
			set { SetArgument("useRelativeVelocity", value); }
		}

		/// <summary>
		/// Sets the ApplyFoetalToLegs setting for this <see cref="RollDownStairsHelper"/>.
		/// if true, use rollup for upper body and a kind of foetal behavior for legs.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ApplyFoetalToLegs
		{
			set { SetArgument("applyFoetalToLegs", value); }
		}

		/// <summary>
		/// Sets the MovementLegsInFoetalPosition setting for this <see cref="RollDownStairsHelper"/>.
		/// Only used if applyFoetalToLegs = true : define the variation of angles for the joints of the legs.
		/// </summary>
		/// <remarks>
		/// Default value = 1.3f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MovementLegsInFoetalPosition
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("movementLegsInFoetalPosition", value);
			}
		}

		/// <summary>
		/// Sets the MaxAngVelAroundFrontwardAxis setting for this <see cref="RollDownStairsHelper"/>.
		/// Only used if applyNewRollingCheatingTorques or applyHelPerTorqueToAlign defined to true : maximal angular velocity around frontward axis of the pelvis to apply cheating torques.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = -1.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MaxAngVelAroundFrontwardAxis
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("maxAngVelAroundFrontwardAxis", value);
			}
		}

		/// <summary>
		/// Sets the MinAngVel setting for this <see cref="RollDownStairsHelper"/>.
		/// Only used if applyNewRollingCheatingTorques or applyHelPerTorqueToAlign defined to true : minimal angular velocity of the roll to apply cheating torques.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MinAngVel
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("minAngVel", value);
			}
		}

		/// <summary>
		/// Sets the ApplyNewRollingCheatingTorques setting for this <see cref="RollDownStairsHelper"/>.
		/// if true will use the new way to apply cheating torques (like in fallOverWall), otherwise will use the old way.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ApplyNewRollingCheatingTorques
		{
			set { SetArgument("applyNewRollingCheatingTorques", value); }
		}

		/// <summary>
		/// Sets the MaxAngVel setting for this <see cref="RollDownStairsHelper"/>.
		/// Only used if applyNewRollingCheatingTorques defined to true : maximal angular velocity of the roll to apply cheating torque.
		/// </summary>
		/// <remarks>
		/// Default value = 5.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float MaxAngVel
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxAngVel", value);
			}
		}

		/// <summary>
		/// Sets the MagOfTorqueToRoll setting for this <see cref="RollDownStairsHelper"/>.
		/// Only used if applyNewRollingCheatingTorques defined to true : magnitude of the torque to roll down the stairs.
		/// </summary>
		/// <remarks>
		/// Default value = 50.0f.
		/// Min value = 0.0f.
		/// Max value = 500.0f.
		/// </remarks>
		public float MagOfTorqueToRoll
		{
			set
			{
				if (value > 500.0f)
					value = 500.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("magOfTorqueToRoll", value);
			}
		}

		/// <summary>
		/// Sets the ApplyHelPerTorqueToAlign setting for this <see cref="RollDownStairsHelper"/>.
		/// apply torque to align the body orthogonally to the direction of the roll.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ApplyHelPerTorqueToAlign
		{
			set { SetArgument("applyHelPerTorqueToAlign", value); }
		}

		/// <summary>
		/// Sets the DelayToAlignBody setting for this <see cref="RollDownStairsHelper"/>.
		/// Only used if applyHelPerTorqueToAlign defined to true : delay to start to apply torques.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float DelayToAlignBody
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("delayToAlignBody", value);
			}
		}

		/// <summary>
		/// Sets the MagOfTorqueToAlign setting for this <see cref="RollDownStairsHelper"/>.
		/// Only used if applyHelPerTorqueToAlign defined to true : magnitude of the torque to align orthogonally the body.
		/// </summary>
		/// <remarks>
		/// Default value = 50.0f.
		/// Min value = 0.0f.
		/// Max value = 500.0f.
		/// </remarks>
		public float MagOfTorqueToAlign
		{
			set
			{
				if (value > 500.0f)
					value = 500.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("magOfTorqueToAlign", value);
			}
		}

		/// <summary>
		/// Sets the AirborneReduction setting for this <see cref="RollDownStairsHelper"/>.
		/// Ordinarily keep at 0.85.  Make this lower if you want spinning in the air.
		/// </summary>
		/// <remarks>
		/// Default value = 0.9f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float AirborneReduction
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("airborneReduction", value);
			}
		}

		/// <summary>
		/// Sets the ApplyMinMaxFriction setting for this <see cref="RollDownStairsHelper"/>.
		/// Pass-through to Roll Up. Controls whether or not behaviour enforces min/max friction.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ApplyMinMaxFriction
		{
			set { SetArgument("applyMinMaxFriction", value); }
		}

		/// <summary>
		/// Sets the LimitSpinReduction setting for this <see cref="RollDownStairsHelper"/>.
		/// Scale zAxisSpinReduction back when rotating end-over-end (somersault) to give the body a chance to align with the axis of rotation.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool LimitSpinReduction
		{
			set { SetArgument("limitSpinReduction", value); }
		}
	}

	public sealed class ShotHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ShotHelper for sending a Shot <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the Shot <see cref="Message"/> to.</param>
		public ShotHelper(Ped ped) : base(ped, "shot")
		{
		}

		/// <summary>
		/// Sets the BodyStiffness setting for this <see cref="ShotHelper"/>.
		/// stiffness of body. Feeds through to roll_up.
		/// </summary>
		/// <remarks>
		/// Default value = 11.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float BodyStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("bodyStiffness", value);
			}
		}

		/// <summary>
		/// Sets the SpineDamping setting for this <see cref="ShotHelper"/>.
		/// stiffness of body. Feeds through to roll_up.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.1f.
		/// Max value = 2.0f.
		/// </remarks>
		public float SpineDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("spineDamping", value);
			}
		}

		/// <summary>
		/// Sets the ArmStiffness setting for this <see cref="ShotHelper"/>.
		/// arm stiffness.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("armStiffness", value);
			}
		}

		/// <summary>
		/// Sets the InitialNeckStiffness setting for this <see cref="ShotHelper"/>.
		/// initial stiffness of neck after being shot.
		/// </summary>
		/// <remarks>
		/// Default value = 14.0f.
		/// Min value = 3.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float InitialNeckStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 3.0f)
					value = 3.0f;
				SetArgument("initialNeckStiffness", value);
			}
		}

		/// <summary>
		/// Sets the InitialNeckDamping setting for this <see cref="ShotHelper"/>.
		/// intial damping of neck after being shot.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.1f.
		/// Max value = 10.0f.
		/// </remarks>
		public float InitialNeckDamping
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("initialNeckDamping", value);
			}
		}

		/// <summary>
		/// Sets the NeckStiffness setting for this <see cref="ShotHelper"/>.
		/// stiffness of neck.
		/// </summary>
		/// <remarks>
		/// Default value = 14.0f.
		/// Min value = 3.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float NeckStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 3.0f)
					value = 3.0f;
				SetArgument("neckStiffness", value);
			}
		}

		/// <summary>
		/// Sets the NeckDamping setting for this <see cref="ShotHelper"/>.
		/// damping of neck.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.1f.
		/// Max value = 2.0f.
		/// </remarks>
		public float NeckDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("neckDamping", value);
			}
		}

		/// <summary>
		/// Sets the KMultOnLoose setting for this <see cref="ShotHelper"/>.
		/// how much to add to upperbody stiffness dependent on looseness.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float KMultOnLoose
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("kMultOnLoose", value);
			}
		}

		/// <summary>
		/// Sets the KMult4Legs setting for this <see cref="ShotHelper"/>.
		/// how much to add to leg stiffnesses dependent on looseness.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float KMult4Legs
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("kMult4Legs", value);
			}
		}

		/// <summary>
		/// Sets the LoosenessAmount setting for this <see cref="ShotHelper"/>.
		/// how loose the character is made by a newBullet. between 0 and 1.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LoosenessAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("loosenessAmount", value);
			}
		}

		/// <summary>
		/// Sets the Looseness4Fall setting for this <see cref="ShotHelper"/>.
		/// how loose the character is made by a newBullet if falling.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Looseness4Fall
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("looseness4Fall", value);
			}
		}

		/// <summary>
		/// Sets the Looseness4Stagger setting for this <see cref="ShotHelper"/>.
		/// how loose the upperBody of the character is made by a newBullet if staggerFall is running (and not falling).  Note atm the neck ramp values are ignored in staggerFall.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Looseness4Stagger
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("looseness4Stagger", value);
			}
		}

		/// <summary>
		/// Sets the MinArmsLooseness setting for this <see cref="ShotHelper"/>.
		/// minimum looseness to apply to the arms.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float MinArmsLooseness
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("minArmsLooseness", value);
			}
		}

		/// <summary>
		/// Sets the MinLegsLooseness setting for this <see cref="ShotHelper"/>.
		/// minimum looseness to apply to the Legs.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float MinLegsLooseness
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("minLegsLooseness", value);
			}
		}

		/// <summary>
		/// Sets the GrabHoldTime setting for this <see cref="ShotHelper"/>.
		/// how long to hold for before returning to relaxed arm position.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GrabHoldTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("grabHoldTime", value);
			}
		}

		/// <summary>
		/// Sets the SpineBlendExagCPain setting for this <see cref="ShotHelper"/>.
		/// true: spine is blended with zero pose, false: spine is blended with zero pose if not setting exag or cpain.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SpineBlendExagCPain
		{
			set { SetArgument("spineBlendExagCPain", value); }
		}

		/// <summary>
		/// Sets the SpineBlendZero setting for this <see cref="ShotHelper"/>.
		/// spine is always blended with zero pose this much and up to 1 as the character become stationary.  If negative no blend is ever applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = -0.1f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SpineBlendZero
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -0.1f)
					value = -0.1f;
				SetArgument("spineBlendZero", value);
			}
		}

		/// <summary>
		/// Sets the BulletProofVest setting for this <see cref="ShotHelper"/>.
		/// looseness applied to spine is different if bulletProofVest is true.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool BulletProofVest
		{
			set { SetArgument("bulletProofVest", value); }
		}

		/// <summary>
		/// Sets the AlwaysResetLooseness setting for this <see cref="ShotHelper"/>.
		/// looseness always reset on shotNewBullet even if previous looseness ramp still running.  Except for the neck which has it's own ramp.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool AlwaysResetLooseness
		{
			set { SetArgument("alwaysResetLooseness", value); }
		}

		/// <summary>
		/// Sets the AlwaysResetNeckLooseness setting for this <see cref="ShotHelper"/>.
		/// Neck looseness always reset on shotNewBullet even if previous looseness ramp still running.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool AlwaysResetNeckLooseness
		{
			set { SetArgument("alwaysResetNeckLooseness", value); }
		}

		/// <summary>
		/// Sets the AngVelScale setting for this <see cref="ShotHelper"/>.
		/// How much to scale the angular velocity coming in from animation of a part if it is in angVelScaleMask (otherwise scale by 1.0).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float AngVelScale
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("angVelScale", value);
			}
		}

		/// <summary>
		/// Sets the AngVelScaleMask setting for this <see cref="ShotHelper"/>.
		/// Parts to scale the initial angular velocity by angVelScale (otherwize scale by 1.0).
		/// </summary>
		/// <remarks>
		/// Default value = fb.
		/// </remarks>
		public string AngVelScaleMask
		{
			set { SetArgument("angVelScaleMask", value); }
		}

		/// <summary>
		/// Sets the FlingWidth setting for this <see cref="ShotHelper"/>.
		/// Width of the fling behaviour.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FlingWidth
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("flingWidth", value);
			}
		}

		/// <summary>
		/// Sets the FlingTime setting for this <see cref="ShotHelper"/>.
		/// Duration of the fling behaviour.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FlingTime
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("flingTime", value);
			}
		}

		/// <summary>
		/// Sets the TimeBeforeReachForWound setting for this <see cref="ShotHelper"/>.
		/// time, in seconds, before the character begins to grab for the wound on the first hit.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float TimeBeforeReachForWound
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("timeBeforeReachForWound", value);
			}
		}

		/// <summary>
		/// Sets the ExagDuration setting for this <see cref="ShotHelper"/>.
		/// exaggerate bullet duration (at exagMag/exagTwistMag).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ExagDuration
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("exagDuration", value);
			}
		}

		/// <summary>
		/// Sets the ExagMag setting for this <see cref="ShotHelper"/>.
		/// exaggerate bullet spine Lean magnitude.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ExagMag
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("exagMag", value);
			}
		}

		/// <summary>
		/// Sets the ExagTwistMag setting for this <see cref="ShotHelper"/>.
		/// exaggerate bullet spine Twist magnitude.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ExagTwistMag
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("exagTwistMag", value);
			}
		}

		/// <summary>
		/// Sets the ExagSmooth2Zero setting for this <see cref="ShotHelper"/>.
		/// exaggerate bullet duration ramping to zero after exagDuration.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ExagSmooth2Zero
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("exagSmooth2Zero", value);
			}
		}

		/// <summary>
		/// Sets the ExagZeroTime setting for this <see cref="ShotHelper"/>.
		/// exaggerate bullet time spent at 0 spine lean/twist after exagDuration + exagSmooth2Zero.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ExagZeroTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("exagZeroTime", value);
			}
		}

		/// <summary>
		/// Sets the CpainSmooth2Time setting for this <see cref="ShotHelper"/>.
		/// conscious pain duration ramping from zero to cpainMag/cpainTwistMag.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float CpainSmooth2Time
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("cpainSmooth2Time", value);
			}
		}

		/// <summary>
		/// Sets the CpainDuration setting for this <see cref="ShotHelper"/>.
		/// conscious pain duration at cpainMag/cpainTwistMag after cpainSmooth2Time.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float CpainDuration
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("cpainDuration", value);
			}
		}

		/// <summary>
		/// Sets the CpainMag setting for this <see cref="ShotHelper"/>.
		/// conscious pain spine Lean(back/Forward) magnitude (Replaces spinePainMultiplier).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float CpainMag
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("cpainMag", value);
			}
		}

		/// <summary>
		/// Sets the CpainTwistMag setting for this <see cref="ShotHelper"/>.
		/// conscious pain spine Twist/Lean2Side magnitude Replaces spinePainTwistMultiplier).
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float CpainTwistMag
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("cpainTwistMag", value);
			}
		}

		/// <summary>
		/// Sets the CpainSmooth2Zero setting for this <see cref="ShotHelper"/>.
		/// conscious pain ramping to zero after cpainSmooth2Time + cpainDuration (Replaces spinePainTime).
		/// </summary>
		/// <remarks>
		/// Default value = 1.5f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float CpainSmooth2Zero
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("cpainSmooth2Zero", value);
			}
		}

		/// <summary>
		/// Sets the Crouching setting for this <see cref="ShotHelper"/>.
		/// is the guy crouching or not.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Crouching
		{
			set { SetArgument("crouching", value); }
		}

		/// <summary>
		/// Sets the ChickenArms setting for this <see cref="ShotHelper"/>.
		/// Type of reaction.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ChickenArms
		{
			set { SetArgument("chickenArms", value); }
		}

		/// <summary>
		/// Sets the ReachForWound setting for this <see cref="ShotHelper"/>.
		/// Type of reaction.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ReachForWound
		{
			set { SetArgument("reachForWound", value); }
		}

		/// <summary>
		/// Sets the Fling setting for this <see cref="ShotHelper"/>.
		/// Type of reaction.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Fling
		{
			set { SetArgument("fling", value); }
		}

		/// <summary>
		/// Sets the AllowInjuredArm setting for this <see cref="ShotHelper"/>.
		/// injured arm code runs if arm hit (turns and steps and bends injured arm).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AllowInjuredArm
		{
			set { SetArgument("allowInjuredArm", value); }
		}

		/// <summary>
		/// Sets the AllowInjuredLeg setting for this <see cref="ShotHelper"/>.
		/// when false injured leg is not bent and character does not bend to reach it.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool AllowInjuredLeg
		{
			set { SetArgument("allowInjuredLeg", value); }
		}

		/// <summary>
		/// Sets the AllowInjuredLowerLegReach setting for this <see cref="ShotHelper"/>.
		/// when false don't try to reach for injured Lower Legs (shins/feet).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AllowInjuredLowerLegReach
		{
			set { SetArgument("allowInjuredLowerLegReach", value); }
		}

		/// <summary>
		/// Sets the AllowInjuredThighReach setting for this <see cref="ShotHelper"/>.
		/// when false don't try to reach for injured Thighs.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool AllowInjuredThighReach
		{
			set { SetArgument("allowInjuredThighReach", value); }
		}

		/// <summary>
		/// Sets the StableHandsAndNeck setting for this <see cref="ShotHelper"/>.
		/// additional stability for hands and neck (less loose).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool StableHandsAndNeck
		{
			set { SetArgument("stableHandsAndNeck", value); }
		}

		/// <summary>
		/// Sets the Melee setting for this <see cref="ShotHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Melee
		{
			set { SetArgument("melee", value); }
		}

		/// <summary>
		/// Sets the FallingReaction setting for this <see cref="ShotHelper"/>.
		/// 0=Rollup, 1=Catchfall, 2=rollDownStairs, 3=smartFall.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 3.
		/// </remarks>
		public int FallingReaction
		{
			set
			{
				if (value > 3)
					value = 3;
				if (value < 0)
					value = 0;
				SetArgument("fallingReaction", value);
			}
		}

		/// <summary>
		/// Sets the UseExtendedCatchFall setting for this <see cref="ShotHelper"/>.
		/// keep the character active instead of relaxing at the end of the catch fall.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseExtendedCatchFall
		{
			set { SetArgument("useExtendedCatchFall", value); }
		}

		/// <summary>
		/// Sets the InitialWeaknessZeroDuration setting for this <see cref="ShotHelper"/>.
		/// duration for which the character's upper body stays at minimum stiffness (not quite zero).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float InitialWeaknessZeroDuration
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("initialWeaknessZeroDuration", value);
			}
		}

		/// <summary>
		/// Sets the InitialWeaknessRampDuration setting for this <see cref="ShotHelper"/>.
		/// duration of the ramp to bring the character's upper body stiffness back to normal levels.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float InitialWeaknessRampDuration
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("initialWeaknessRampDuration", value);
			}
		}

		/// <summary>
		/// Sets the InitialNeckDuration setting for this <see cref="ShotHelper"/>.
		/// duration for which the neck stays at intial stiffness/damping.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float InitialNeckDuration
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("initialNeckDuration", value);
			}
		}

		/// <summary>
		/// Sets the InitialNeckRampDuration setting for this <see cref="ShotHelper"/>.
		/// duration of the ramp to bring the neck stiffness/damping back to normal levels.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float InitialNeckRampDuration
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("initialNeckRampDuration", value);
			}
		}

		/// <summary>
		/// Sets the UseCStrModulation setting for this <see cref="ShotHelper"/>.
		/// if enabled upper and lower body strength scales with character strength, using the range given by parameters below.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseCStrModulation
		{
			set { SetArgument("useCStrModulation", value); }
		}

		/// <summary>
		/// Sets the CStrUpperMin setting for this <see cref="ShotHelper"/>.
		/// proportions to what the strength would be normally.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.1f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CStrUpperMin
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("cStrUpperMin", value);
			}
		}

		/// <summary>
		/// Sets the CStrUpperMax setting for this <see cref="ShotHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.1f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CStrUpperMax
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("cStrUpperMax", value);
			}
		}

		/// <summary>
		/// Sets the CStrLowerMin setting for this <see cref="ShotHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.1f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CStrLowerMin
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("cStrLowerMin", value);
			}
		}

		/// <summary>
		/// Sets the CStrLowerMax setting for this <see cref="ShotHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.1f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CStrLowerMax
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.1f)
					value = 0.1f;
				SetArgument("cStrLowerMax", value);
			}
		}

		/// <summary>
		/// Sets the DeathTime setting for this <see cref="ShotHelper"/>.
		/// time to death (HACK for underwater). If -ve don't ever die.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float DeathTime
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("deathTime", value);
			}
		}
	}

	/// <summary>
	/// Send new wound information to the shot.  Can cause shot to restart it's performance in part or in whole.
	/// </summary>
	public sealed class ShotNewBulletHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ShotNewBulletHelper for sending a ShotNewBullet <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ShotNewBullet <see cref="Message"/> to.</param>
		/// <remarks>
		/// Send new wound information to the shot.  Can cause shot to restart it's performance in part or in whole.
		/// </remarks>
		public ShotNewBulletHelper(Ped ped) : base(ped, "shotNewBullet")
		{
		}

		/// <summary>
		/// Sets the BodyPart setting for this <see cref="ShotNewBulletHelper"/>.
		/// part ID on the body where the bullet hit.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 21.
		/// </remarks>
		public int BodyPart
		{
			set
			{
				if (value > 21)
					value = 21;
				if (value < 0)
					value = 0;
				SetArgument("bodyPart", value);
			}
		}

		/// <summary>
		/// Sets the LocalHitPointInfo setting for this <see cref="ShotNewBulletHelper"/>.
		/// if true then normal and hitPoint should be supplied in local coordinates of bodyPart.  If false then normal and hitPoint should be supplied in World coordinates.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool LocalHitPointInfo
		{
			set { SetArgument("localHitPointInfo", value); }
		}

		/// <summary>
		/// Sets the Normal setting for this <see cref="ShotNewBulletHelper"/>.
		/// Normal coming out of impact point on character.  Can be local or global depending on localHitPointInfo.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, -1.0f).
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public Vector3 Normal
		{
			set { SetArgument("normal", Vector3.Clamp(value, new Vector3(-1.0f, -1.0f, -1.0f), new Vector3(1.0f, 1.0f, 1.0f))); }
		}

		/// <summary>
		/// Sets the HitPoint setting for this <see cref="ShotNewBulletHelper"/>.
		/// position of impact on character. Can be local or global depending on localHitPointInfo.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 HitPoint
		{
			set { SetArgument("hitPoint", value); }
		}

		/// <summary>
		/// Sets the BulletVel setting for this <see cref="ShotNewBulletHelper"/>.
		/// bullet velocity in world coordinates.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// Min value = -2000.0f.
		/// Max value = 2000.0f.
		/// </remarks>
		public Vector3 BulletVel
		{
			set
			{
				SetArgument("bulletVel",
					Vector3.Clamp(value, new Vector3(-2000.0f, -2000.0f, -2000.0f), new Vector3(2000.0f, 2000.0f, 2000.0f)));
			}
		}
	}

	public sealed class ShotSnapHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ShotSnapHelper for sending a ShotSnap <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ShotSnap <see cref="Message"/> to.</param>
		public ShotSnapHelper(Ped ped) : base(ped, "shotSnap")
		{
		}

		/// <summary>
		/// Sets the Snap setting for this <see cref="ShotSnapHelper"/>.
		/// Add a Snap to shot.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Snap
		{
			set { SetArgument("snap", value); }
		}

		/// <summary>
		/// Sets the SnapMag setting for this <see cref="ShotSnapHelper"/>.
		/// The magnitude of the reaction.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = -10.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SnapMag
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("snapMag", value);
			}
		}

		/// <summary>
		/// Sets the SnapMovingMult setting for this <see cref="ShotSnapHelper"/>.
		/// movingMult*snapMag = The magnitude of the reaction if moving(comVelMag) faster than movingThresh.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float SnapMovingMult
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("snapMovingMult", value);
			}
		}

		/// <summary>
		/// Sets the SnapBalancingMult setting for this <see cref="ShotSnapHelper"/>.
		/// balancingMult*snapMag = The magnitude of the reaction if balancing = (not lying on the floor/ not upper body not collided) and not airborne.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float SnapBalancingMult
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("snapBalancingMult", value);
			}
		}

		/// <summary>
		/// Sets the SnapAirborneMult setting for this <see cref="ShotSnapHelper"/>.
		/// airborneMult*snapMag = The magnitude of the reaction if airborne.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float SnapAirborneMult
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("snapAirborneMult", value);
			}
		}

		/// <summary>
		/// Sets the SnapMovingThresh setting for this <see cref="ShotSnapHelper"/>.
		/// If moving(comVelMag) faster than movingThresh then mvingMult applied to stunMag.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float SnapMovingThresh
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("snapMovingThresh", value);
			}
		}

		/// <summary>
		/// Sets the SnapDirectionRandomness setting for this <see cref="ShotSnapHelper"/>.
		/// The character snaps in a prescribed way (decided by bullet direction) - Higher the value the more random this direction is.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SnapDirectionRandomness
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("snapDirectionRandomness", value);
			}
		}

		/// <summary>
		/// Sets the SnapLeftArm setting for this <see cref="ShotSnapHelper"/>.
		/// snap the leftArm.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SnapLeftArm
		{
			set { SetArgument("snapLeftArm", value); }
		}

		/// <summary>
		/// Sets the SnapRightArm setting for this <see cref="ShotSnapHelper"/>.
		/// snap the rightArm.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SnapRightArm
		{
			set { SetArgument("snapRightArm", value); }
		}

		/// <summary>
		/// Sets the SnapLeftLeg setting for this <see cref="ShotSnapHelper"/>.
		/// snap the leftLeg.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SnapLeftLeg
		{
			set { SetArgument("snapLeftLeg", value); }
		}

		/// <summary>
		/// Sets the SnapRightLeg setting for this <see cref="ShotSnapHelper"/>.
		/// snap the rightLeg.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SnapRightLeg
		{
			set { SetArgument("snapRightLeg", value); }
		}

		/// <summary>
		/// Sets the SnapSpine setting for this <see cref="ShotSnapHelper"/>.
		/// snap the spine.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool SnapSpine
		{
			set { SetArgument("snapSpine", value); }
		}

		/// <summary>
		/// Sets the SnapNeck setting for this <see cref="ShotSnapHelper"/>.
		/// snap the neck.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool SnapNeck
		{
			set { SetArgument("snapNeck", value); }
		}

		/// <summary>
		/// Sets the SnapPhasedLegs setting for this <see cref="ShotSnapHelper"/>.
		/// Legs are either in phase with each other or not.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool SnapPhasedLegs
		{
			set { SetArgument("snapPhasedLegs", value); }
		}

		/// <summary>
		/// Sets the SnapHipType setting for this <see cref="ShotSnapHelper"/>.
		/// type of hip reaction 0=none, 1=side2side 2=steplike.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int SnapHipType
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("snapHipType", value);
			}
		}

		/// <summary>
		/// Sets the SnapUseBulletDir setting for this <see cref="ShotSnapHelper"/>.
		/// Legs are either in phase with each other or not.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool SnapUseBulletDir
		{
			set { SetArgument("snapUseBulletDir", value); }
		}

		/// <summary>
		/// Sets the SnapHitPart setting for this <see cref="ShotSnapHelper"/>.
		/// Snap only around the wounded part//mmmmtodo check whether bodyPart doesn't have to be remembered for unSnap.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool SnapHitPart
		{
			set { SetArgument("snapHitPart", value); }
		}

		/// <summary>
		/// Sets the UnSnapInterval setting for this <see cref="ShotSnapHelper"/>.
		/// Interval before applying reverse snap.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float UnSnapInterval
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("unSnapInterval", value);
			}
		}

		/// <summary>
		/// Sets the UnSnapRatio setting for this <see cref="ShotSnapHelper"/>.
		/// The magnitude of the reverse snap.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float UnSnapRatio
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("unSnapRatio", value);
			}
		}

		/// <summary>
		/// Sets the SnapUseTorques setting for this <see cref="ShotSnapHelper"/>.
		/// use torques to make the snap otherwise use a change in the parts angular velocity.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool SnapUseTorques
		{
			set { SetArgument("snapUseTorques", value); }
		}
	}

	/// <summary>
	/// configure the shockSpin effect in shot.  Spin/Lift the character using cheat torques/forces.
	/// </summary>
	public sealed class ShotShockSpinHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ShotShockSpinHelper for sending a ShotShockSpin <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ShotShockSpin <see cref="Message"/> to.</param>
		/// <remarks>
		/// configure the shockSpin effect in shot.  Spin/Lift the character using cheat torques/forces.
		/// </remarks>
		public ShotShockSpinHelper(Ped ped) : base(ped, "shotShockSpin")
		{
		}

		/// <summary>
		/// Sets the AddShockSpin setting for this <see cref="ShotShockSpinHelper"/>.
		/// if enabled, add a short 'shock' of torque to the character's spine to exaggerate bullet impact.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AddShockSpin
		{
			set { SetArgument("addShockSpin", value); }
		}

		/// <summary>
		/// Sets the RandomizeShockSpinDirection setting for this <see cref="ShotShockSpinHelper"/>.
		/// for use with close-range shotgun blasts, or similar.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool RandomizeShockSpinDirection
		{
			set { SetArgument("randomizeShockSpinDirection", value); }
		}

		/// <summary>
		/// Sets the AlwaysAddShockSpin setting for this <see cref="ShotShockSpinHelper"/>.
		/// if true, apply the shock spin no matter which body component was hit. otherwise only apply if the spine or clavicles get hit.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AlwaysAddShockSpin
		{
			set { SetArgument("alwaysAddShockSpin", value); }
		}

		/// <summary>
		/// Sets the ShockSpinMin setting for this <see cref="ShotShockSpinHelper"/>.
		/// minimum amount of torque to add if using shock-spin feature.
		/// </summary>
		/// <remarks>
		/// Default value = 50.0f.
		/// Min value = 0.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float ShockSpinMin
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shockSpinMin", value);
			}
		}

		/// <summary>
		/// Sets the ShockSpinMax setting for this <see cref="ShotShockSpinHelper"/>.
		/// maxiumum amount of torque to add if using shock-spin feature.
		/// </summary>
		/// <remarks>
		/// Default value = 90.0f.
		/// Min value = 0.0f.
		/// Max value = 1000.0f.
		/// </remarks>
		public float ShockSpinMax
		{
			set
			{
				if (value > 1000.0f)
					value = 1000.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shockSpinMax", value);
			}
		}

		/// <summary>
		/// Sets the ShockSpinLiftForceMult setting for this <see cref="ShotShockSpinHelper"/>.
		/// if greater than 0, apply a force to lift the character up while the torque is applied, trying to produce a dramatic spun/twist shotgun-to-the-chest effect. this is a scale of the torque applied, so 8.0 or so would give a reasonable amount of lift.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ShockSpinLiftForceMult
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shockSpinLiftForceMult", value);
			}
		}

		/// <summary>
		/// Sets the ShockSpinDecayMult setting for this <see cref="ShotShockSpinHelper"/>.
		/// multiplier used when decaying torque spin over time.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ShockSpinDecayMult
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shockSpinDecayMult", value);
			}
		}

		/// <summary>
		/// Sets the ShockSpinScalePerComponent setting for this <see cref="ShotShockSpinHelper"/>.
		/// torque applied is scaled by this amount across the spine components - spine2 recieving the full amount, then 3 and 1 and finally 0. each time, this value is used to scale it down. 0.5 means half the torque each time.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ShockSpinScalePerComponent
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shockSpinScalePerComponent", value);
			}
		}

		/// <summary>
		/// Sets the ShockSpinMaxTwistVel setting for this <see cref="ShotShockSpinHelper"/>.
		/// shock spin ends when twist velocity is greater than this value (try 6.0).  If set to -1 does not stop.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 200.0f.
		/// </remarks>
		public float ShockSpinMaxTwistVel
		{
			set
			{
				if (value > 200.0f)
					value = 200.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("shockSpinMaxTwistVel", value);
			}
		}

		/// <summary>
		/// Sets the ShockSpinScaleByLeverArm setting for this <see cref="ShotShockSpinHelper"/>.
		/// shock spin scales by lever arm of bullet i.e. bullet impact point to centre line.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ShockSpinScaleByLeverArm
		{
			set { SetArgument("shockSpinScaleByLeverArm", value); }
		}

		/// <summary>
		/// Sets the ShockSpinAirMult setting for this <see cref="ShotShockSpinHelper"/>.
		/// shockSpin's torque is multipied by this value when both the character's feet are not in contact.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ShockSpinAirMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shockSpinAirMult", value);
			}
		}

		/// <summary>
		/// Sets the ShockSpin1FootMult setting for this <see cref="ShotShockSpinHelper"/>.
		/// shockSpin's torque is multipied by this value when the one of the character's feet are not in contact.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ShockSpin1FootMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shockSpin1FootMult", value);
			}
		}

		/// <summary>
		/// Sets the ShockSpinFootGripMult setting for this <see cref="ShotShockSpinHelper"/>.
		/// shockSpin scales the torques applied to the feet by footSlipCompensation.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float ShockSpinFootGripMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("shockSpinFootGripMult", value);
			}
		}

		/// <summary>
		/// Sets the BracedSideSpinMult setting for this <see cref="ShotShockSpinHelper"/>.
		/// If shot on a side with a forward foot and both feet are on the ground and balanced, increase the shockspin to compensate for the balancer naturally resisting spin to that side.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 1.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float BracedSideSpinMult
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 1.0f)
					value = 1.0f;
				SetArgument("bracedSideSpinMult", value);
			}
		}
	}

	/// <summary>
	/// configure the fall to knees shot.
	/// </summary>
	public sealed class ShotFallToKneesHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ShotFallToKneesHelper for sending a ShotFallToKnees <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ShotFallToKnees <see cref="Message"/> to.</param>
		/// <remarks>
		/// configure the fall to knees shot.
		/// </remarks>
		public ShotFallToKneesHelper(Ped ped) : base(ped, "shotFallToKnees")
		{
		}

		/// <summary>
		/// Sets the FallToKnees setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Type of reaction.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FallToKnees
		{
			set { SetArgument("fallToKnees", value); }
		}

		/// <summary>
		/// Sets the FtkAlwaysChangeFall setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Always change fall behaviour.  If false only change when falling forward.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FtkAlwaysChangeFall
		{
			set { SetArgument("ftkAlwaysChangeFall", value); }
		}

		/// <summary>
		/// Sets the FtkBalanceTime setting for this <see cref="ShotFallToKneesHelper"/>.
		/// How long the balancer runs for before fallToKnees starts.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float FtkBalanceTime
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("ftkBalanceTime", value);
			}
		}

		/// <summary>
		/// Sets the FtkHelperForce setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Hip helper force magnitude - to help character lean over balance point of line between toes.
		/// </summary>
		/// <remarks>
		/// Default value = 200.0f.
		/// Min value = 0.0f.
		/// Max value = 2000.0f.
		/// </remarks>
		public float FtkHelperForce
		{
			set
			{
				if (value > 2000.0f)
					value = 2000.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("ftkHelperForce", value);
			}
		}

		/// <summary>
		/// Sets the FtkHelperForceOnSpine setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Helper force applied to spine3 aswell.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool FtkHelperForceOnSpine
		{
			set { SetArgument("ftkHelperForceOnSpine", value); }
		}

		/// <summary>
		/// Sets the FtkLeanHelp setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Help balancer lean amount - to help character lean over balance point of line between toes.  Half of this is also applied as hipLean.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 0.3f.
		/// </remarks>
		public float FtkLeanHelp
		{
			set
			{
				if (value > 0.3f)
					value = 0.3f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("ftkLeanHelp", value);
			}
		}

		/// <summary>
		/// Sets the FtkSpineBend setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Bend applied to spine when falling from knees. (+ve forward - try -0.1) (only if rds called).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -0.2f.
		/// Max value = 0.3f.
		/// </remarks>
		public float FtkSpineBend
		{
			set
			{
				if (value > 0.3f)
					value = 0.3f;
				if (value < -0.2f)
					value = -0.2f;
				SetArgument("ftkSpineBend", value);
			}
		}

		/// <summary>
		/// Sets the FtkStiffSpine setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Stiffen spine when falling from knees (only if rds called).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FtkStiffSpine
		{
			set { SetArgument("ftkStiffSpine", value); }
		}

		/// <summary>
		/// Sets the FtkImpactLooseness setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Looseness (muscleStiffness = 1.01f - m_parameters.ftkImpactLooseness) applied to upperBody on knee impacts.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FtkImpactLooseness
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("ftkImpactLooseness", value);
			}
		}

		/// <summary>
		/// Sets the FtkImpactLoosenessTime setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Time that looseness is applied after knee impacts.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -0.1f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FtkImpactLoosenessTime
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -0.1f)
					value = -0.1f;
				SetArgument("ftkImpactLoosenessTime", value);
			}
		}

		/// <summary>
		/// Sets the FtkBendRate setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Rate at which the legs are bent to go from standing to on knees.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float FtkBendRate
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("ftkBendRate", value);
			}
		}

		/// <summary>
		/// Sets the FtkHipBlend setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Blend from current hip to balancing on knees hip angle.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FtkHipBlend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("ftkHipBlend", value);
			}
		}

		/// <summary>
		/// Sets the FtkLungeProb setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Probability that a lunge reaction will be allowed.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FtkLungeProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("ftkLungeProb", value);
			}
		}

		/// <summary>
		/// Sets the FtkKneeSpin setting for this <see cref="ShotFallToKneesHelper"/>.
		/// When on knees allow some spinning of the character.  If false then the balancers' footSlipCompensation remains on and tends to keep the character facing the same way as when it was balancing.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FtkKneeSpin
		{
			set { SetArgument("ftkKneeSpin", value); }
		}

		/// <summary>
		/// Sets the FtkFricMult setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Multiplier on the reduction of friction for the feet based on angle away from horizontal - helps the character fall to knees quicker.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float FtkFricMult
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("ftkFricMult", value);
			}
		}

		/// <summary>
		/// Sets the FtkHipAngleFall setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Apply this hip angle when the character starts to fall backwards when on knees.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FtkHipAngleFall
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("ftkHipAngleFall", value);
			}
		}

		/// <summary>
		/// Sets the FtkPitchForwards setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Hip pitch applied (+ve forward, -ve backwards) if character is falling forwards on way down to it's knees.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = -0.5f.
		/// Max value = 0.5f.
		/// </remarks>
		public float FtkPitchForwards
		{
			set
			{
				if (value > 0.5f)
					value = 0.5f;
				if (value < -0.5f)
					value = -0.5f;
				SetArgument("ftkPitchForwards", value);
			}
		}

		/// <summary>
		/// Sets the FtkPitchBackwards setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Hip pitch applied (+ve forward, -ve backwards) if character is falling backwards on way down to it's knees.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = -0.5f.
		/// Max value = 0.5f.
		/// </remarks>
		public float FtkPitchBackwards
		{
			set
			{
				if (value > 0.5f)
					value = 0.5f;
				if (value < -0.5f)
					value = -0.5f;
				SetArgument("ftkPitchBackwards", value);
			}
		}

		/// <summary>
		/// Sets the FtkFallBelowStab setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Balancer instability below which the character starts to bend legs even if it isn't going to fall on to it's knees (i.e. if going backwards). 0.3 almost ensures a fall to knees but means the character will keep stepping backward until it slows down enough.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 15.0f.
		/// </remarks>
		public float FtkFallBelowStab
		{
			set
			{
				if (value > 15.0f)
					value = 15.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("ftkFallBelowStab", value);
			}
		}

		/// <summary>
		/// Sets the FtkBalanceAbortThreshold setting for this <see cref="ShotFallToKneesHelper"/>.
		/// when the character gives up and goes into a fall.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float FtkBalanceAbortThreshold
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("ftkBalanceAbortThreshold", value);
			}
		}

		/// <summary>
		/// Sets the FtkOnKneesArmType setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Type of arm response when on knees falling forward 0=useFallArms (from RollDownstairs or catchFall), 1= armsIn, 2=armsOut.
		/// </summary>
		/// <remarks>
		/// Default value = 2.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int FtkOnKneesArmType
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("ftkOnKneesArmType", value);
			}
		}

		/// <summary>
		/// Sets the FtkReleaseReachForWound setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Release the reachForWound this amount of time after the knees have hit.  If  LT  0.0 then keep reaching for wound regardless of fall/onground state.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float FtkReleaseReachForWound
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("ftkReleaseReachForWound", value);
			}
		}

		/// <summary>
		/// Sets the FtkReachForWound setting for this <see cref="ShotFallToKneesHelper"/>.
		/// true = Keep reaching for wound regardless of fall/onground state.  false = respect the shotConfigureArms params: reachFalling, reachFallingWithOneHand, reachOnFloor.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool FtkReachForWound
		{
			set { SetArgument("ftkReachForWound", value); }
		}

		/// <summary>
		/// Sets the FtkReleasePointGun setting for this <see cref="ShotFallToKneesHelper"/>.
		/// Override the pointGun when knees hit.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FtkReleasePointGun
		{
			set { SetArgument("ftkReleasePointGun", value); }
		}

		/// <summary>
		/// Sets the FtkFailMustCollide setting for this <see cref="ShotFallToKneesHelper"/>.
		/// The upper body of the character must be colliding and other failure conditions met to fail.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool FtkFailMustCollide
		{
			set { SetArgument("ftkFailMustCollide", value); }
		}
	}

	/// <summary>
	/// configure the shot from behind reaction.
	/// </summary>
	public sealed class ShotFromBehindHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ShotFromBehindHelper for sending a ShotFromBehind <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ShotFromBehind <see cref="Message"/> to.</param>
		/// <remarks>
		/// configure the shot from behind reaction.
		/// </remarks>
		public ShotFromBehindHelper(Ped ped) : base(ped, "shotFromBehind")
		{
		}

		/// <summary>
		/// Sets the ShotFromBehind setting for this <see cref="ShotFromBehindHelper"/>.
		/// Type of reaction.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ShotFromBehind
		{
			set { SetArgument("shotFromBehind", value); }
		}

		/// <summary>
		/// Sets the SfbSpineAmount setting for this <see cref="ShotFromBehindHelper"/>.
		/// SpineBend.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SfbSpineAmount
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sfbSpineAmount", value);
			}
		}

		/// <summary>
		/// Sets the SfbNeckAmount setting for this <see cref="ShotFromBehindHelper"/>.
		/// Neck Bend.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SfbNeckAmount
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sfbNeckAmount", value);
			}
		}

		/// <summary>
		/// Sets the SfbHipAmount setting for this <see cref="ShotFromBehindHelper"/>.
		/// hip Pitch.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SfbHipAmount
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sfbHipAmount", value);
			}
		}

		/// <summary>
		/// Sets the SfbKneeAmount setting for this <see cref="ShotFromBehindHelper"/>.
		/// knee bend.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SfbKneeAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sfbKneeAmount", value);
			}
		}

		/// <summary>
		/// Sets the SfbPeriod setting for this <see cref="ShotFromBehindHelper"/>.
		/// shotFromBehind reaction period after being shot.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SfbPeriod
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sfbPeriod", value);
			}
		}

		/// <summary>
		/// Sets the SfbForceBalancePeriod setting for this <see cref="ShotFromBehindHelper"/>.
		/// amount of time not taking a step.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SfbForceBalancePeriod
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sfbForceBalancePeriod", value);
			}
		}

		/// <summary>
		/// Sets the SfbArmsOnset setting for this <see cref="ShotFromBehindHelper"/>.
		/// amount of time before applying spread out arms pose.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SfbArmsOnset
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sfbArmsOnset", value);
			}
		}

		/// <summary>
		/// Sets the SfbKneesOnset setting for this <see cref="ShotFromBehindHelper"/>.
		/// amount of time before bending knees a bit.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SfbKneesOnset
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sfbKneesOnset", value);
			}
		}

		/// <summary>
		/// Sets the SfbNoiseGain setting for this <see cref="ShotFromBehindHelper"/>.
		/// Controls additional independent randomized bending of left/right elbows.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float SfbNoiseGain
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sfbNoiseGain", value);
			}
		}

		/// <summary>
		/// Sets the SfbIgnoreFail setting for this <see cref="ShotFromBehindHelper"/>.
		/// 0=balancer fails as normal,  1= ignore backArchedBack and leanedTooFarBack balancer failures,  2= ignore backArchedBack balancer failure only,  3= ignore leanedTooFarBack balancer failure only.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 3.
		/// </remarks>
		public int SfbIgnoreFail
		{
			set
			{
				if (value > 3)
					value = 3;
				if (value < 0)
					value = 0;
				SetArgument("sfbIgnoreFail", value);
			}
		}
	}

	/// <summary>
	/// configure the shot in guts reaction.
	/// </summary>
	public sealed class ShotInGutsHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ShotInGutsHelper for sending a ShotInGuts <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ShotInGuts <see cref="Message"/> to.</param>
		/// <remarks>
		/// configure the shot in guts reaction.
		/// </remarks>
		public ShotInGutsHelper(Ped ped) : base(ped, "shotInGuts")
		{
		}

		/// <summary>
		/// Sets the ShotInGuts setting for this <see cref="ShotInGutsHelper"/>.
		/// Type of reaction.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ShotInGuts
		{
			set { SetArgument("shotInGuts", value); }
		}

		/// <summary>
		/// Sets the SigSpineAmount setting for this <see cref="ShotInGutsHelper"/>.
		/// SpineBend.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SigSpineAmount
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sigSpineAmount", value);
			}
		}

		/// <summary>
		/// Sets the SigNeckAmount setting for this <see cref="ShotInGutsHelper"/>.
		/// Neck Bend.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SigNeckAmount
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sigNeckAmount", value);
			}
		}

		/// <summary>
		/// Sets the SigHipAmount setting for this <see cref="ShotInGutsHelper"/>.
		/// hip Pitch.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SigHipAmount
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sigHipAmount", value);
			}
		}

		/// <summary>
		/// Sets the SigKneeAmount setting for this <see cref="ShotInGutsHelper"/>.
		/// knee bend.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SigKneeAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sigKneeAmount", value);
			}
		}

		/// <summary>
		/// Sets the SigPeriod setting for this <see cref="ShotInGutsHelper"/>.
		/// active time after being shot.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SigPeriod
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sigPeriod", value);
			}
		}

		/// <summary>
		/// Sets the SigForceBalancePeriod setting for this <see cref="ShotInGutsHelper"/>.
		/// amount of time not taking a step.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SigForceBalancePeriod
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sigForceBalancePeriod", value);
			}
		}

		/// <summary>
		/// Sets the SigKneesOnset setting for this <see cref="ShotInGutsHelper"/>.
		/// amount of time not taking a step.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SigKneesOnset
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("sigKneesOnset", value);
			}
		}
	}

	public sealed class ShotHeadLookHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ShotHeadLookHelper for sending a ShotHeadLook <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ShotHeadLook <see cref="Message"/> to.</param>
		public ShotHeadLookHelper(Ped ped) : base(ped, "shotHeadLook")
		{
		}

		/// <summary>
		/// Sets the UseHeadLook setting for this <see cref="ShotHeadLookHelper"/>.
		/// Use headLook.  Default: looks at provided target or if this is zero -  looks forward or in velocity direction. If reachForWound is enabled, switches between looking at the wound and at the default target.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseHeadLook
		{
			set { SetArgument("useHeadLook", value); }
		}

		/// <summary>
		/// Sets the HeadLook setting for this <see cref="ShotHeadLookHelper"/>.
		/// position to look at with headlook flag.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 HeadLook
		{
			set { SetArgument("headLook", value); }
		}

		/// <summary>
		/// Sets the HeadLookAtWoundMinTimer setting for this <see cref="ShotHeadLookHelper"/>.
		/// Min time to look at wound.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float HeadLookAtWoundMinTimer
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("headLookAtWoundMinTimer", value);
			}
		}

		/// <summary>
		/// Sets the HeadLookAtWoundMaxTimer setting for this <see cref="ShotHeadLookHelper"/>.
		/// Max time to look at wound.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float HeadLookAtWoundMaxTimer
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("headLookAtWoundMaxTimer", value);
			}
		}

		/// <summary>
		/// Sets the HeadLookAtHeadPosMaxTimer setting for this <see cref="ShotHeadLookHelper"/>.
		/// Min time to look headLook or if zero - forward or in velocity direction.
		/// </summary>
		/// <remarks>
		/// Default value = 1.7f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float HeadLookAtHeadPosMaxTimer
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("headLookAtHeadPosMaxTimer", value);
			}
		}

		/// <summary>
		/// Sets the HeadLookAtHeadPosMinTimer setting for this <see cref="ShotHeadLookHelper"/>.
		/// Max time to look headLook or if zero - forward or in velocity direction.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float HeadLookAtHeadPosMinTimer
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("headLookAtHeadPosMinTimer", value);
			}
		}
	}

	/// <summary>
	/// configure the arm reactions in shot.
	/// </summary>
	public sealed class ShotConfigureArmsHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the ShotConfigureArmsHelper for sending a ShotConfigureArms <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the ShotConfigureArms <see cref="Message"/> to.</param>
		/// <remarks>
		/// configure the arm reactions in shot.
		/// </remarks>
		public ShotConfigureArmsHelper(Ped ped) : base(ped, "shotConfigureArms")
		{
		}

		/// <summary>
		/// Sets the Brace setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// blind brace with arms if appropriate.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool Brace
		{
			set { SetArgument("brace", value); }
		}

		/// <summary>
		/// Sets the PointGun setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Point gun if appropriate.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool PointGun
		{
			set { SetArgument("pointGun", value); }
		}

		/// <summary>
		/// Sets the UseArmsWindmill setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// armsWindmill if going backwards fast enough.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseArmsWindmill
		{
			set { SetArgument("useArmsWindmill", value); }
		}

		/// <summary>
		/// Sets the ReleaseWound setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// release wound if going sideways/forward fast enough.  0 = don't. 1 = only if bracing. 2 = any default arm reaction.
		/// </summary>
		/// <remarks>
		/// Default value = 1.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int ReleaseWound
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("releaseWound", value);
			}
		}

		/// <summary>
		/// Sets the ReachFalling setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// reachForWound when falling 0 = false, 1 = true, 2 = once per shot performance.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int ReachFalling
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("reachFalling", value);
			}
		}

		/// <summary>
		/// Sets the ReachFallingWithOneHand setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Force character to reach for wound with only one hand when falling or fallen.  0= allow 2 handed reach, 1= left only if 2 handed possible, 2= right only if 2 handed possible, 3 = one handed but automatic (allows switching of hands).
		/// </summary>
		/// <remarks>
		/// Default value = 3.
		/// Min value = 0.
		/// Max value = 3.
		/// </remarks>
		public int ReachFallingWithOneHand
		{
			set
			{
				if (value > 3)
					value = 3;
				if (value < 0)
					value = 0;
				SetArgument("reachFallingWithOneHand", value);
			}
		}

		/// <summary>
		/// Sets the ReachOnFloor setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// reachForWound when on floor - 0 = false, 1 = true, 2 = once per shot performance.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int ReachOnFloor
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("reachOnFloor", value);
			}
		}

		/// <summary>
		/// Sets the AlwaysReachTime setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Inhibit arms brace for this amount of time after reachForWound has begun.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float AlwaysReachTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("alwaysReachTime", value);
			}
		}

		/// <summary>
		/// Sets the AWSpeedMult setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// For armsWindmill, multiplier on character speed - increase of speed of circling is proportional to character speed (max speed of circliing increase = 1.5). eg. lowering the value increases the range of velocity that the 0-1.5 is applied over.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float AWSpeedMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("AWSpeedMult", value);
			}
		}

		/// <summary>
		/// Sets the AWRadiusMult setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// For armsWindmill, multiplier on character speed - increase of radii is proportional to character speed (max radius increase = 0.45). eg. lowering the value increases the range of velocity that the 0-0.45 is applied over.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float AWRadiusMult
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("AWRadiusMult", value);
			}
		}

		/// <summary>
		/// Sets the AWStiffnessAdd setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// For armsWindmill, added arm stiffness ranges from 0 to AWStiffnessAdd.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float AWStiffnessAdd
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("AWStiffnessAdd", value);
			}
		}

		/// <summary>
		/// Sets the ReachWithOneHand setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Force character to reach for wound with only one hand.  0= allow 2 handed reach, 1= left only if 2 handed possible, 2= right only if 2 handed possible.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 2.
		/// </remarks>
		public int ReachWithOneHand
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < 0)
					value = 0;
				SetArgument("reachWithOneHand", value);
			}
		}

		/// <summary>
		/// Sets the AllowLeftPistolRFW setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Allow character to reach for wound with left hand if holding a pistol.  It never will for a rifle. If pointGun is running this will only happen if the hand cannot point and pointGun:poseUnusedGunArm = false.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool AllowLeftPistolRFW
		{
			set { SetArgument("allowLeftPistolRFW", value); }
		}

		/// <summary>
		/// Sets the AllowRightPistolRFW setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Allow character to reach for wound with right hand if holding a pistol. It never will for a rifle. If pointGun is running this will only happen if the hand cannot point and pointGun:poseUnusedGunArm = false.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AllowRightPistolRFW
		{
			set { SetArgument("allowRightPistolRFW", value); }
		}

		/// <summary>
		/// Sets the RfwWithPistol setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Override pointGun and reachForWound if desired if holding a pistol.  It never will for a rifle.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool RfwWithPistol
		{
			set { SetArgument("rfwWithPistol", value); }
		}

		/// <summary>
		/// Sets the Fling2 setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Type of reaction.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Fling2
		{
			set { SetArgument("fling2", value); }
		}

		/// <summary>
		/// Sets the Fling2Left setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Fling the left arm.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool Fling2Left
		{
			set { SetArgument("fling2Left", value); }
		}

		/// <summary>
		/// Sets the Fling2Right setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Fling the right arm.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool Fling2Right
		{
			set { SetArgument("fling2Right", value); }
		}

		/// <summary>
		/// Sets the Fling2OverrideStagger setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Override stagger arms even if staggerFall:m_upperBodyReaction = true.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Fling2OverrideStagger
		{
			set { SetArgument("fling2OverrideStagger", value); }
		}

		/// <summary>
		/// Sets the Fling2TimeBefore setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Time after hit that the fling will start (allows for a bit of loose arm movement from bullet impact.snap etc).
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Fling2TimeBefore
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("fling2TimeBefore", value);
			}
		}

		/// <summary>
		/// Sets the Fling2Time setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Duration of the fling behaviour.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Fling2Time
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("fling2Time", value);
			}
		}

		/// <summary>
		/// Sets the Fling2MStiffL setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// MuscleStiffness of the left arm.  If negative then uses the shots underlying muscle stiffness from controlStiffness (i.e. respects looseness).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = -1.0f.
		/// Max value = 1.5f.
		/// </remarks>
		public float Fling2MStiffL
		{
			set
			{
				if (value > 1.5f)
					value = 1.5f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("fling2MStiffL", value);
			}
		}

		/// <summary>
		/// Sets the Fling2MStiffR setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// MuscleStiffness of the right arm.  If negative then uses the shots underlying muscle stiffness from controlStiffness (i.e. respects looseness).
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 1.5f.
		/// </remarks>
		public float Fling2MStiffR
		{
			set
			{
				if (value > 1.5f)
					value = 1.5f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("fling2MStiffR", value);
			}
		}

		/// <summary>
		/// Sets the Fling2RelaxTimeL setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Maximum time before the left arm relaxes in the fling.  It will relax automatically when the arm has completed it's bent arm fling.  This is what causes the arm to straighten.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Fling2RelaxTimeL
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("fling2RelaxTimeL", value);
			}
		}

		/// <summary>
		/// Sets the Fling2RelaxTimeR setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Maximum time before the right arm relaxes in the fling.  It will relax automatically when the arm has completed it's bent arm fling.  This is what causes the arm to straighten.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Fling2RelaxTimeR
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("fling2RelaxTimeR", value);
			}
		}

		/// <summary>
		/// Sets the Fling2AngleMinL setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Min fling angle for left arm.  Fling angle is random in the range fling2AngleMin:fling2AngleMax. Angle of fling in radians measured from the body horizontal sideways from shoulder. positive is up, 0 shoulder level, negative down.
		/// </summary>
		/// <remarks>
		/// Default value = -1.5f.
		/// Min value = -1.5f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Fling2AngleMinL
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.5f)
					value = -1.5f;
				SetArgument("fling2AngleMinL", value);
			}
		}

		/// <summary>
		/// Sets the Fling2AngleMaxL setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Max fling angle for left arm.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = -1.5f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Fling2AngleMaxL
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.5f)
					value = -1.5f;
				SetArgument("fling2AngleMaxL", value);
			}
		}

		/// <summary>
		/// Sets the Fling2AngleMinR setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Min fling angle for right arm.
		/// </summary>
		/// <remarks>
		/// Default value = -1.5f.
		/// Min value = -1.5f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Fling2AngleMinR
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.5f)
					value = -1.5f;
				SetArgument("fling2AngleMinR", value);
			}
		}

		/// <summary>
		/// Sets the Fling2AngleMaxR setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Max fling angle for right arm.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = -1.5f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Fling2AngleMaxR
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.5f)
					value = -1.5f;
				SetArgument("fling2AngleMaxR", value);
			}
		}

		/// <summary>
		/// Sets the Fling2LengthMinL setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Min left arm length.  Armlength is random in the range fling2LengthMin:fling2LengthMax.  Armlength maps one to one with elbow angle.  (These values are scaled internally for the female character).
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.3f.
		/// Max value = 0.6f.
		/// </remarks>
		public float Fling2LengthMinL
		{
			set
			{
				if (value > 0.6f)
					value = 0.6f;
				if (value < 0.3f)
					value = 0.3f;
				SetArgument("fling2LengthMinL", value);
			}
		}

		/// <summary>
		/// Sets the Fling2LengthMaxL setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Max left arm length.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.3f.
		/// Max value = 0.6f.
		/// </remarks>
		public float Fling2LengthMaxL
		{
			set
			{
				if (value > 0.6f)
					value = 0.6f;
				if (value < 0.3f)
					value = 0.3f;
				SetArgument("fling2LengthMaxL", value);
			}
		}

		/// <summary>
		/// Sets the Fling2LengthMinR setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Min right arm length.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.3f.
		/// Max value = 0.6f.
		/// </remarks>
		public float Fling2LengthMinR
		{
			set
			{
				if (value > 0.6f)
					value = 0.6f;
				if (value < 0.3f)
					value = 0.3f;
				SetArgument("fling2LengthMinR", value);
			}
		}

		/// <summary>
		/// Sets the Fling2LengthMaxR setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Max right arm length.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.3f.
		/// Max value = 0.6f.
		/// </remarks>
		public float Fling2LengthMaxR
		{
			set
			{
				if (value > 0.6f)
					value = 0.6f;
				if (value < 0.3f)
					value = 0.3f;
				SetArgument("fling2LengthMaxR", value);
			}
		}

		/// <summary>
		/// Sets the Bust setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Has the character got a bust.  If so then cupBust (move bust reach targets below bust) or bustElbowLift and cupSize (stop upperArm penetrating bust and move bust targets to surface of bust) are implemented.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Bust
		{
			set { SetArgument("bust", value); }
		}

		/// <summary>
		/// Sets the BustElbowLift setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Lift the elbows up this much extra to avoid upper arm penetrating the bust (when target hits spine2 or spine3).
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float BustElbowLift
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("bustElbowLift", value);
			}
		}

		/// <summary>
		/// Sets the CupSize setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// Amount reach target to bust (spine2) will be offset forward by.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CupSize
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("cupSize", value);
			}
		}

		/// <summary>
		/// Sets the CupBust setting for this <see cref="ShotConfigureArmsHelper"/>.
		/// All reach targets above or on the bust will cause a reach below the bust. (specifically moves spine3 and spine2 targets to spine1). bustElbowLift and cupSize are ignored.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool CupBust
		{
			set { SetArgument("cupBust", value); }
		}
	}

	/// <summary>
	/// Clone of High Fall with a wider range of operating conditions.
	/// </summary>
	public sealed class SmartFallHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the SmartFallHelper for sending a SmartFall <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the SmartFall <see cref="Message"/> to.</param>
		/// <remarks>
		/// Clone of High Fall with a wider range of operating conditions.
		/// </remarks>
		public SmartFallHelper(Ped ped) : base(ped, "smartFall")
		{
		}

		/// <summary>
		/// Sets the BodyStiffness setting for this <see cref="SmartFallHelper"/>.
		/// stiffness of body. Value feeds through to bodyBalance (synched with defaults), to armsWindmill (14 for this value at default ), legs pedal, head look and roll down stairs directly.
		/// </summary>
		/// <remarks>
		/// Default value = 11.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float BodyStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("bodyStiffness", value);
			}
		}

		/// <summary>
		/// Sets the Bodydamping setting for this <see cref="SmartFallHelper"/>.
		/// The damping of the joints.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 3.0f.
		/// </remarks>
		public float Bodydamping
		{
			set
			{
				if (value > 3.0f)
					value = 3.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("bodydamping", value);
			}
		}

		/// <summary>
		/// Sets the Catchfalltime setting for this <see cref="SmartFallHelper"/>.
		/// The length of time before the impact that the character transitions to the landing.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Catchfalltime
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("catchfalltime", value);
			}
		}

		/// <summary>
		/// Sets the CrashOrLandCutOff setting for this <see cref="SmartFallHelper"/>.
		/// 0.52angle is 0.868 dot//A threshold for deciding how far away from upright the character needs to be before bailing out (going into a foetal) instead of trying to land (keeping stretched out).  NB: never does bailout if ignorWorldCollisions true.
		/// </summary>
		/// <remarks>
		/// Default value = 0.9f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CrashOrLandCutOff
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("crashOrLandCutOff", value);
			}
		}

		/// <summary>
		/// Sets the PdStrength setting for this <see cref="SmartFallHelper"/>.
		/// Strength of the controller to keep the character at angle aimAngleBase from vertical.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float PdStrength
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("pdStrength", value);
			}
		}

		/// <summary>
		/// Sets the PdDamping setting for this <see cref="SmartFallHelper"/>.
		/// Damping multiplier of the controller to keep the character at angle aimAngleBase from vertical.  The actual damping is pdDamping*pdStrength*constant*angVel.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float PdDamping
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("pdDamping", value);
			}
		}

		/// <summary>
		/// Sets the ArmAngSpeed setting for this <see cref="SmartFallHelper"/>.
		/// arm circling speed in armWindMillAdaptive.
		/// </summary>
		/// <remarks>
		/// Default value = 7.9f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float ArmAngSpeed
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armAngSpeed", value);
			}
		}

		/// <summary>
		/// Sets the ArmAmplitude setting for this <see cref="SmartFallHelper"/>.
		/// in armWindMillAdaptive.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float ArmAmplitude
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armAmplitude", value);
			}
		}

		/// <summary>
		/// Sets the ArmPhase setting for this <see cref="SmartFallHelper"/>.
		/// in armWindMillAdaptive 3.1 opposite for stuntman.  1.0 old default.  0.0 in phase.
		/// </summary>
		/// <remarks>
		/// Default value = 3.1f.
		/// Min value = 0.0f.
		/// Max value = 6.3f.
		/// </remarks>
		public float ArmPhase
		{
			set
			{
				if (value > 6.3f)
					value = 6.3f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armPhase", value);
			}
		}

		/// <summary>
		/// Sets the ArmBendElbows setting for this <see cref="SmartFallHelper"/>.
		/// in armWindMillAdaptive bend the elbows as a function of armAngle.  For stuntman true otherwise false.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ArmBendElbows
		{
			set { SetArgument("armBendElbows", value); }
		}

		/// <summary>
		/// Sets the LegRadius setting for this <see cref="SmartFallHelper"/>.
		/// radius of legs on pedal.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 0.5f.
		/// </remarks>
		public float LegRadius
		{
			set
			{
				if (value > 0.5f)
					value = 0.5f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legRadius", value);
			}
		}

		/// <summary>
		/// Sets the LegAngSpeed setting for this <see cref="SmartFallHelper"/>.
		/// in pedal.
		/// </summary>
		/// <remarks>
		/// Default value = 7.9f.
		/// Min value = 0.0f.
		/// Max value = 15.0f.
		/// </remarks>
		public float LegAngSpeed
		{
			set
			{
				if (value > 15.0f)
					value = 15.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legAngSpeed", value);
			}
		}

		/// <summary>
		/// Sets the LegAsymmetry setting for this <see cref="SmartFallHelper"/>.
		/// 0.0 for stuntman.  Random offset applied per leg to the angular speed to desynchronise the pedaling - set to 0 to disable, otherwise should be set to less than the angularSpeed value.
		/// </summary>
		/// <remarks>
		/// Default value = 4.0f.
		/// Min value = -10.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float LegAsymmetry
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("legAsymmetry", value);
			}
		}

		/// <summary>
		/// Sets the Arms2LegsPhase setting for this <see cref="SmartFallHelper"/>.
		/// phase angle between the arms and legs circling angle.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 6.5f.
		/// </remarks>
		public float Arms2LegsPhase
		{
			set
			{
				if (value > 6.5f)
					value = 6.5f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("arms2LegsPhase", value);
			}
		}

		/// <summary>
		/// Sets the Arms2LegsSync setting for this <see cref="SmartFallHelper"/>.
		/// Syncs the arms angle to what the leg angle is.
		/// </summary>
		/// <remarks>
		/// Default value = <see cref="Synchroisation.AlwaysSynced"/>.
		/// All speed/direction parameters of armswindmill are overwritten if = <see cref="Synchroisation.AlwaysSynced"/>.
		/// If <see cref="Synchroisation.SyncedAtStart"/> and you want synced arms/legs then armAngSpeed=legAngSpeed, legAsymmetry = 0.0 (to stop randomizations of the leg cicle speed).
		/// </remarks>
		public Synchroisation Arms2LegsSync
		{
			set { SetArgument("arms2LegsSync", (int) value); }
		}

		/// <summary>
		/// Sets the ArmsUp setting for this <see cref="SmartFallHelper"/>.
		/// Where to put the arms when preparing to land. Approx 1 = above head, 0 = head height, -1 = down.   LT -2.0 use catchFall arms,  LT -3.0 use prepare for landing pose if Agent is due to land vertically, feet first.
		/// </summary>
		/// <remarks>
		/// Default value = -3.1f.
		/// Min value = -4.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmsUp
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < -4.0f)
					value = -4.0f;
				SetArgument("armsUp", value);
			}
		}

		/// <summary>
		/// Sets the OrientateBodyToFallDirection setting for this <see cref="SmartFallHelper"/>.
		/// toggle to orientate to fall direction.  i.e. orientate so that the character faces the horizontal velocity direction.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool OrientateBodyToFallDirection
		{
			set { SetArgument("orientateBodyToFallDirection", value); }
		}

		/// <summary>
		/// Sets the OrientateTwist setting for this <see cref="SmartFallHelper"/>.
		/// If false don't worry about the twist angle of the character when orientating the character.  If false this allows the twist axis of the character to be free (You can get a nice twisting highFall like the one in dieHard 4 when the car goes into the helicopter).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool OrientateTwist
		{
			set { SetArgument("orientateTwist", value); }
		}

		/// <summary>
		/// Sets the OrientateMax setting for this <see cref="SmartFallHelper"/>.
		/// DEVEL parameter - suggest you don't edit it.  Maximum torque the orientation controller can apply.  If 0 then no helper torques will be used.  300 will orientate the character soflty for all but extreme angles away from aimAngleBase.  If abs (current -aimAngleBase) is getting near 3.0 then this can be reduced to give a softer feel.
		/// </summary>
		/// <remarks>
		/// Default value = 300.0f.
		/// Min value = 0.0f.
		/// Max value = 2000.0f.
		/// </remarks>
		public float OrientateMax
		{
			set
			{
				if (value > 2000.0f)
					value = 2000.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("orientateMax", value);
			}
		}

		/// <summary>
		/// Sets the AlanRickman setting for this <see cref="SmartFallHelper"/>.
		/// If true then orientate the character to face the point from where it started falling.  HighFall like the one in dieHard with Alan Rickman.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AlanRickman
		{
			set { SetArgument("alanRickman", value); }
		}

		/// <summary>
		/// Sets the FowardRoll setting for this <see cref="SmartFallHelper"/>.
		/// Try to execute a forward Roll on landing.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool FowardRoll
		{
			set { SetArgument("fowardRoll", value); }
		}

		/// <summary>
		/// Sets the UseZeroPose_withFowardRoll setting for this <see cref="SmartFallHelper"/>.
		/// Blend to a zero pose when forward roll is attempted.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseZeroPose_withFowardRoll
		{
			set { SetArgument("useZeroPose_withFowardRoll", value); }
		}

		/// <summary>
		/// Sets the AimAngleBase setting for this <see cref="SmartFallHelper"/>.
		/// Angle from vertical the pdController is driving to ( positive = forwards).
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -3.1f.
		/// Max value = 3.1f.
		/// </remarks>
		public float AimAngleBase
		{
			set
			{
				if (value > 3.1f)
					value = 3.1f;
				if (value < -3.1f)
					value = -3.1f;
				SetArgument("aimAngleBase", value);
			}
		}

		/// <summary>
		/// Sets the FowardVelRotation setting for this <see cref="SmartFallHelper"/>.
		/// scale to add/subtract from aimAngle based on forward speed (Internal).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FowardVelRotation
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("fowardVelRotation", value);
			}
		}

		/// <summary>
		/// Sets the FootVelCompScale setting for this <see cref="SmartFallHelper"/>.
		/// Scale to change to amount of vel that is added to the foot ik from the velocity (Internal).
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FootVelCompScale
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("footVelCompScale", value);
			}
		}

		/// <summary>
		/// Sets the SideD setting for this <see cref="SmartFallHelper"/>.
		/// sideoffset for the feet during prepareForLanding. +ve = right.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SideD
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("sideD", value);
			}
		}

		/// <summary>
		/// Sets the FowardOffsetOfLegIK setting for this <see cref="SmartFallHelper"/>.
		/// Forward offset for the feet during prepareForLanding.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float FowardOffsetOfLegIK
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("fowardOffsetOfLegIK", value);
			}
		}

		/// <summary>
		/// Sets the LegL setting for this <see cref="SmartFallHelper"/>.
		/// Leg Length for ik (Internal)//unused.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float LegL
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("legL", value);
			}
		}

		/// <summary>
		/// Sets the CatchFallCutOff setting for this <see cref="SmartFallHelper"/>.
		/// 0.5angle is 0.878 dot. Cutoff to go to the catchFall ( internal) //mmmtodo do like crashOrLandCutOff.
		/// </summary>
		/// <remarks>
		/// Default value = 0.9f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CatchFallCutOff
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("catchFallCutOff", value);
			}
		}

		/// <summary>
		/// Sets the LegStrength setting for this <see cref="SmartFallHelper"/>.
		/// Strength of the legs at landing.
		/// </summary>
		/// <remarks>
		/// Default value = 12.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float LegStrength
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("legStrength", value);
			}
		}

		/// <summary>
		/// Sets the Balance setting for this <see cref="SmartFallHelper"/>.
		/// If true have enough strength to balance.  If false not enough strength in legs to balance (even though bodyBlance called).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool Balance
		{
			set { SetArgument("balance", value); }
		}

		/// <summary>
		/// Sets the IgnorWorldCollisions setting for this <see cref="SmartFallHelper"/>.
		/// Never go into bailout (foetal).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool IgnorWorldCollisions
		{
			set { SetArgument("ignorWorldCollisions", value); }
		}

		/// <summary>
		/// Sets the AdaptiveCircling setting for this <see cref="SmartFallHelper"/>.
		/// stuntman type fall.  Arm and legs circling direction controlled by angmom and orientation.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool AdaptiveCircling
		{
			set { SetArgument("adaptiveCircling", value); }
		}

		/// <summary>
		/// Sets the Hula setting for this <see cref="SmartFallHelper"/>.
		/// With stuntman type fall.  Hula reaction if can't see floor and not rotating fast.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool Hula
		{
			set { SetArgument("hula", value); }
		}

		/// <summary>
		/// Sets the MaxSpeedForRecoverableFall setting for this <see cref="SmartFallHelper"/>.
		/// Character needs to be moving less than this speed to consider fall as a recoverable one.
		/// </summary>
		/// <remarks>
		/// Default value = 15.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float MaxSpeedForRecoverableFall
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("maxSpeedForRecoverableFall", value);
			}
		}

		/// <summary>
		/// Sets the MinSpeedForBrace setting for this <see cref="SmartFallHelper"/>.
		/// Character needs to be moving at least this fast horizontally to start bracing for impact if there is an object along its trajectory.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float MinSpeedForBrace
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("minSpeedForBrace", value);
			}
		}

		/// <summary>
		/// Sets the LandingNormal setting for this <see cref="SmartFallHelper"/>.
		/// Ray-cast normal doted with up direction has to be greater than this number to consider object flat enough to land on it.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LandingNormal
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("landingNormal", value);
			}
		}

		/// <summary>
		/// Sets the RdsForceMag setting for this <see cref="SmartFallHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 0.8f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float RdsForceMag
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rdsForceMag", value);
			}
		}

		/// <summary>
		/// Sets the RdsTargetLinVeDecayTime setting for this <see cref="SmartFallHelper"/>.
		/// RDS: Time for the targetlinearVelocity to decay to zero.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float RdsTargetLinVeDecayTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rdsTargetLinVeDecayTime", value);
			}
		}

		/// <summary>
		/// Sets the RdsTargetLinearVelocity setting for this <see cref="SmartFallHelper"/>.
		/// RDS: Helper torques are applied to match the spin of the character to the max of targetLinearVelocity and COMVelMag. -1 to use initial character velocity.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 30.0f.
		/// </remarks>
		public float RdsTargetLinearVelocity
		{
			set
			{
				if (value > 30.0f)
					value = 30.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rdsTargetLinearVelocity", value);
			}
		}

		/// <summary>
		/// Sets the RdsUseStartingFriction setting for this <see cref="SmartFallHelper"/>.
		/// Start Catch Fall/RDS state with specified friction. Catch fall will overwrite based on setFallingReaction.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool RdsUseStartingFriction
		{
			set { SetArgument("rdsUseStartingFriction", value); }
		}

		/// <summary>
		/// Sets the RdsStartingFriction setting for this <see cref="SmartFallHelper"/>.
		/// Catch Fall/RDS starting friction. Catch fall will overwrite based on setFallingReaction.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float RdsStartingFriction
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rdsStartingFriction", value);
			}
		}

		/// <summary>
		/// Sets the RdsStartingFrictionMin setting for this <see cref="SmartFallHelper"/>.
		/// Catch Fall/RDS starting friction minimum. Catch fall will overwrite based on setFallingReaction.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float RdsStartingFrictionMin
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rdsStartingFrictionMin", value);
			}
		}

		/// <summary>
		/// Sets the RdsForceVelThreshold setting for this <see cref="SmartFallHelper"/>.
		/// Velocity threshold under which RDS force mag will be applied.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float RdsForceVelThreshold
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rdsForceVelThreshold", value);
			}
		}

		/// <summary>
		/// Sets the InitialState setting for this <see cref="SmartFallHelper"/>.
		/// Force initial state (used in vehicle bail out to start SF_CatchFall (6) earlier.
		/// </summary>
		/// <remarks>
		/// Default value = 0.
		/// Min value = 0.
		/// Max value = 7.
		/// </remarks>
		public int InitialState
		{
			set
			{
				if (value > 7)
					value = 7;
				if (value < 0)
					value = 0;
				SetArgument("initialState", value);
			}
		}

		/// <summary>
		/// Sets the ChangeExtremityFriction setting for this <see cref="SmartFallHelper"/>.
		/// Allow friction changes to be applied to the hands and feet.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ChangeExtremityFriction
		{
			set { SetArgument("changeExtremityFriction", value); }
		}

		/// <summary>
		/// Sets the Teeter setting for this <see cref="SmartFallHelper"/>.
		/// Set up an immediate teeter in the direction of trave if initial state is SF_Balance.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool Teeter
		{
			set { SetArgument("teeter", value); }
		}

		/// <summary>
		/// Sets the TeeterOffset setting for this <see cref="SmartFallHelper"/>.
		/// Offset the default Teeter edge in the direction of travel. Will need to be tweaked depending on how close to the real edge AI tends to trigger the behaviour.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TeeterOffset
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("teeterOffset", value);
			}
		}

		/// <summary>
		/// Sets the StopRollingTime setting for this <see cref="SmartFallHelper"/>.
		/// Time in seconds before ped should start actively trying to stop rolling.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float StopRollingTime
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("stopRollingTime", value);
			}
		}

		/// <summary>
		/// Sets the ReboundScale setting for this <see cref="SmartFallHelper"/>.
		/// Scale for rebound assistance.  0=off, 1=very bouncy, 2=jbone crazy  Try 0.5?.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ReboundScale
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("reboundScale", value);
			}
		}

		/// <summary>
		/// Sets the ReboundMask setting for this <see cref="SmartFallHelper"/>.
		/// Part mask to apply rebound assistance.
		/// </summary>
		/// <remarks>
		/// Default value = uk.
		/// </remarks>
		public string ReboundMask
		{
			set { SetArgument("reboundMask", value); }
		}

		/// <summary>
		/// Sets the ForceHeadAvoid setting for this <see cref="SmartFallHelper"/>.
		/// Force head avoid to be active during Catch Fall even when character is not on the ground.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ForceHeadAvoid
		{
			set { SetArgument("forceHeadAvoid", value); }
		}

		/// <summary>
		/// Sets the CfZAxisSpinReduction setting for this <see cref="SmartFallHelper"/>.
		/// Pass-through parameter for Catch Fall spin reduction.  Increase to stop more spin. 0..1.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float CfZAxisSpinReduction
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("cfZAxisSpinReduction", value);
			}
		}

		/// <summary>
		/// Sets the SplatWhenStopped setting for this <see cref="SmartFallHelper"/>.
		/// Transition to splat state when com vel is below value, regardless of character health or fall velocity.  Set to zero to disable.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float SplatWhenStopped
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("splatWhenStopped", value);
			}
		}

		/// <summary>
		/// Sets the BlendHeadWhenStopped setting for this <see cref="SmartFallHelper"/>.
		/// Blend head to neutral pose com vel approaches zero.  Linear between zero and value.  Set to zero to disable.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float BlendHeadWhenStopped
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("blendHeadWhenStopped", value);
			}
		}

		/// <summary>
		/// Sets the SpreadLegs setting for this <see cref="SmartFallHelper"/>.
		/// Spread legs amount for Pedal during fall.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SpreadLegs
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("spreadLegs", value);
			}
		}
	}

	public sealed class StaggerFallHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the StaggerFallHelper for sending a StaggerFall <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the StaggerFall <see cref="Message"/> to.</param>
		public StaggerFallHelper(Ped ped) : base(ped, "staggerFall")
		{
		}

		/// <summary>
		/// Sets the ArmStiffness setting for this <see cref="StaggerFallHelper"/>.
		/// stiffness of arms. catch_fall's stiffness scales with this value, but has default values when this is default.
		/// </summary>
		/// <remarks>
		/// Default value = 12.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armStiffness", value);
			}
		}

		/// <summary>
		/// Sets the ArmDamping setting for this <see cref="StaggerFallHelper"/>.
		/// Sets damping value for the arms.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armDamping", value);
			}
		}

		/// <summary>
		/// Sets the SpineDamping setting for this <see cref="StaggerFallHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float SpineDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineDamping", value);
			}
		}

		/// <summary>
		/// Sets the SpineStiffness setting for this <see cref="StaggerFallHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float SpineStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineStiffness", value);
			}
		}

		/// <summary>
		/// Sets the ArmStiffnessStart setting for this <see cref="StaggerFallHelper"/>.
		/// armStiffness during the yanked timescale ie timeAtStartValues.
		/// </summary>
		/// <remarks>
		/// Default value = 3.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmStiffnessStart
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armStiffnessStart", value);
			}
		}

		/// <summary>
		/// Sets the ArmDampingStart setting for this <see cref="StaggerFallHelper"/>.
		/// armDamping during the yanked timescale ie timeAtStartValues.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmDampingStart
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armDampingStart", value);
			}
		}

		/// <summary>
		/// Sets the SpineDampingStart setting for this <see cref="StaggerFallHelper"/>.
		/// spineDamping during the yanked timescale ie timeAtStartValues.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float SpineDampingStart
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineDampingStart", value);
			}
		}

		/// <summary>
		/// Sets the SpineStiffnessStart setting for this <see cref="StaggerFallHelper"/>.
		/// spineStiffness during the yanked timescale ie timeAtStartValues.
		/// </summary>
		/// <remarks>
		/// Default value = 3.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float SpineStiffnessStart
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineStiffnessStart", value);
			}
		}

		/// <summary>
		/// Sets the TimeAtStartValues setting for this <see cref="StaggerFallHelper"/>.
		/// time spent with Start values for arms and spine stiffness and damping ie for whiplash efffect.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float TimeAtStartValues
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("timeAtStartValues", value);
			}
		}

		/// <summary>
		/// Sets the RampTimeFromStartValues setting for this <see cref="StaggerFallHelper"/>.
		/// time spent ramping from Start to end values for arms and spine stiffness and damping ie for whiplash efffect (occurs after timeAtStartValues).
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RampTimeFromStartValues
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rampTimeFromStartValues", value);
			}
		}

		/// <summary>
		/// Sets the StaggerStepProb setting for this <see cref="StaggerFallHelper"/>.
		/// Probability per step of time spent in a stagger step.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float StaggerStepProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("staggerStepProb", value);
			}
		}

		/// <summary>
		/// Sets the StepsTillStartEnd setting for this <see cref="StaggerFallHelper"/>.
		/// steps taken before lowerBodyStiffness starts ramping down by perStepReduction1.
		/// </summary>
		/// <remarks>
		/// Default value = 2.
		/// Min value = 0.
		/// Max value = 100.
		/// </remarks>
		public int StepsTillStartEnd
		{
			set
			{
				if (value > 100)
					value = 100;
				if (value < 0)
					value = 0;
				SetArgument("stepsTillStartEnd", value);
			}
		}

		/// <summary>
		/// Sets the TimeStartEnd setting for this <see cref="StaggerFallHelper"/>.
		/// time from start of behaviour before lowerBodyStiffness starts ramping down for rampTimeToEndValues to endValues.
		/// </summary>
		/// <remarks>
		/// Default value = 100.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float TimeStartEnd
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("timeStartEnd", value);
			}
		}

		/// <summary>
		/// Sets the RampTimeToEndValues setting for this <see cref="StaggerFallHelper"/>.
		/// time spent ramping from lowerBodyStiffness to lowerBodyStiffnessEnd.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float RampTimeToEndValues
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rampTimeToEndValues", value);
			}
		}

		/// <summary>
		/// Sets the LowerBodyStiffness setting for this <see cref="StaggerFallHelper"/>.
		/// lowerBodyStiffness should be 12.
		/// </summary>
		/// <remarks>
		/// Default value = 13.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float LowerBodyStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lowerBodyStiffness", value);
			}
		}

		/// <summary>
		/// Sets the LowerBodyStiffnessEnd setting for this <see cref="StaggerFallHelper"/>.
		/// lowerBodyStiffness at end.
		/// </summary>
		/// <remarks>
		/// Default value = 8.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float LowerBodyStiffnessEnd
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lowerBodyStiffnessEnd", value);
			}
		}

		/// <summary>
		/// Sets the PredictionTime setting for this <see cref="StaggerFallHelper"/>.
		/// amount of time (seconds) into the future that the character tries to step to. bigger values try to recover with fewer, bigger steps. smaller values recover with smaller steps, and generally recover less.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float PredictionTime
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("predictionTime", value);
			}
		}

		/// <summary>
		/// Sets the PerStepReduction1 setting for this <see cref="StaggerFallHelper"/>.
		/// LowerBody stiffness will be reduced every step to make the character fallover.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float PerStepReduction1
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("perStepReduction1", value);
			}
		}

		/// <summary>
		/// Sets the LeanInDirRate setting for this <see cref="StaggerFallHelper"/>.
		/// leanInDirection will be increased from 0 to leanInDirMax linearly at this rate.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float LeanInDirRate
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanInDirRate", value);
			}
		}

		/// <summary>
		/// Sets the LeanInDirMaxF setting for this <see cref="StaggerFallHelper"/>.
		/// Max of leanInDirection magnitude when going forwards.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanInDirMaxF
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanInDirMaxF", value);
			}
		}

		/// <summary>
		/// Sets the LeanInDirMaxB setting for this <see cref="StaggerFallHelper"/>.
		/// Max of leanInDirection magnitude when going backwards.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanInDirMaxB
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanInDirMaxB", value);
			}
		}

		/// <summary>
		/// Sets the LeanHipsMaxF setting for this <see cref="StaggerFallHelper"/>.
		/// Max of leanInDirectionHips magnitude when going forwards.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanHipsMaxF
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanHipsMaxF", value);
			}
		}

		/// <summary>
		/// Sets the LeanHipsMaxB setting for this <see cref="StaggerFallHelper"/>.
		/// Max of leanInDirectionHips magnitude when going backwards.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanHipsMaxB
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanHipsMaxB", value);
			}
		}

		/// <summary>
		/// Sets the Lean2multF setting for this <see cref="StaggerFallHelper"/>.
		/// Lean of spine to side in side velocity direction when going forwards.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -5.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float Lean2multF
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < -5.0f)
					value = -5.0f;
				SetArgument("lean2multF", value);
			}
		}

		/// <summary>
		/// Sets the Lean2multB setting for this <see cref="StaggerFallHelper"/>.
		/// Lean of spine to side in side velocity direction when going backwards.
		/// </summary>
		/// <remarks>
		/// Default value = -2.0f.
		/// Min value = -5.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float Lean2multB
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < -5.0f)
					value = -5.0f;
				SetArgument("lean2multB", value);
			}
		}

		/// <summary>
		/// Sets the PushOffDist setting for this <see cref="StaggerFallHelper"/>.
		/// amount stance foot is behind com in the direction of velocity before the leg tries to pushOff to increase momentum.  Increase to lower the probability of the pushOff making the character bouncy.
		/// </summary>
		/// <remarks>
		/// Default value = 0.2f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float PushOffDist
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("pushOffDist", value);
			}
		}

		/// <summary>
		/// Sets the MaxPushoffVel setting for this <see cref="StaggerFallHelper"/>.
		/// stance leg will only pushOff to increase momentum if the vertical hip velocity is less than this value. 0.4 seems like a good value.  The higher it is the the less this functionality is applied.  If it is very low or negative this can stop the pushOff altogether.
		/// </summary>
		/// <remarks>
		/// Default value = 20.0f.
		/// Min value = -20.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float MaxPushoffVel
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < -20.0f)
					value = -20.0f;
				SetArgument("maxPushoffVel", value);
			}
		}

		/// <summary>
		/// Sets the HipBendMult setting for this <see cref="StaggerFallHelper"/>.
		/// hipBend scaled with velocity.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = -10.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float HipBendMult
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("hipBendMult", value);
			}
		}

		/// <summary>
		/// Sets the AlwaysBendForwards setting for this <see cref="StaggerFallHelper"/>.
		/// bend forwards at the hip (hipBendMult) whether moving backwards or forwards.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool AlwaysBendForwards
		{
			set { SetArgument("alwaysBendForwards", value); }
		}

		/// <summary>
		/// Sets the SpineBendMult setting for this <see cref="StaggerFallHelper"/>.
		/// spine bend scaled with velocity.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = -10.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float SpineBendMult
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < -10.0f)
					value = -10.0f;
				SetArgument("spineBendMult", value);
			}
		}

		/// <summary>
		/// Sets the UseHeadLook setting for this <see cref="StaggerFallHelper"/>.
		/// enable and provide a look-at target to make the character's head turn to face it while balancing, balancer default is 0.2.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseHeadLook
		{
			set { SetArgument("useHeadLook", value); }
		}

		/// <summary>
		/// Sets the HeadLookPos setting for this <see cref="StaggerFallHelper"/>.
		/// position of thing to look at.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 HeadLookPos
		{
			set { SetArgument("headLookPos", value); }
		}

		/// <summary>
		/// Sets the HeadLookInstanceIndex setting for this <see cref="StaggerFallHelper"/>.
		/// level index of thing to look at.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int HeadLookInstanceIndex
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("headLookInstanceIndex", value);
			}
		}

		/// <summary>
		/// Sets the HeadLookAtVelProb setting for this <see cref="StaggerFallHelper"/>.
		/// Probability [0-1] that headLook will be looking in the direction of velocity when stepping.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float HeadLookAtVelProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("headLookAtVelProb", value);
			}
		}

		/// <summary>
		/// Sets the TurnOffProb setting for this <see cref="StaggerFallHelper"/>.
		/// Weighted Probability that turn will be off. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TurnOffProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turnOffProb", value);
			}
		}

		/// <summary>
		/// Sets the Turn2TargetProb setting for this <see cref="StaggerFallHelper"/>.
		/// Weighted Probability of turning towards headLook target. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Turn2TargetProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turn2TargetProb", value);
			}
		}

		/// <summary>
		/// Sets the Turn2VelProb setting for this <see cref="StaggerFallHelper"/>.
		/// Weighted Probability of turning towards velocity. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float Turn2VelProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turn2VelProb", value);
			}
		}

		/// <summary>
		/// Sets the TurnAwayProb setting for this <see cref="StaggerFallHelper"/>.
		/// Weighted Probability of turning away from headLook target. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TurnAwayProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turnAwayProb", value);
			}
		}

		/// <summary>
		/// Sets the TurnLeftProb setting for this <see cref="StaggerFallHelper"/>.
		/// Weighted Probability of turning left. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TurnLeftProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turnLeftProb", value);
			}
		}

		/// <summary>
		/// Sets the TurnRightProb setting for this <see cref="StaggerFallHelper"/>.
		/// Weighted Probability of turning right. This is one of six turn type weights.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TurnRightProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("turnRightProb", value);
			}
		}

		/// <summary>
		/// Sets the UseBodyTurn setting for this <see cref="StaggerFallHelper"/>.
		/// enable and provide a positive bodyTurnTimeout and provide a look-at target to make the character turn to face it while balancing.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseBodyTurn
		{
			set { SetArgument("useBodyTurn", value); }
		}

		/// <summary>
		/// Sets the UpperBodyReaction setting for this <see cref="StaggerFallHelper"/>.
		/// enable upper body reaction ie blindBrace and armswindmill.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UpperBodyReaction
		{
			set { SetArgument("upperBodyReaction", value); }
		}
	}

	public sealed class TeeterHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the TeeterHelper for sending a Teeter <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the Teeter <see cref="Message"/> to.</param>
		public TeeterHelper(Ped ped) : base(ped, "teeter")
		{
		}

		/// <summary>
		/// Sets the EdgeLeft setting for this <see cref="TeeterHelper"/>.
		/// Defines the left edge point (left of character facing edge).
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(39.5f, 38.9f, 21.1f).
		/// Min value = 0.0f.
		/// </remarks>
		public Vector3 EdgeLeft
		{
			set { SetArgument("edgeLeft", Vector3.Max(value, new Vector3(0.0f, 0.0f, 0.0f))); }
		}

		/// <summary>
		/// Sets the EdgeRight setting for this <see cref="TeeterHelper"/>.
		/// Defines the right edge point (right of character facing edge).
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(39.5f, 39.9f, 21.1f).
		/// Min value = 0.0f.
		/// </remarks>
		public Vector3 EdgeRight
		{
			set { SetArgument("edgeRight", Vector3.Max(value, new Vector3(0.0f, 0.0f, 0.0f))); }
		}

		/// <summary>
		/// Sets the UseExclusionZone setting for this <see cref="TeeterHelper"/>.
		/// stop stepping across the line defined by edgeLeft and edgeRight.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseExclusionZone
		{
			set { SetArgument("useExclusionZone", value); }
		}

		/// <summary>
		/// Sets the UseHeadLook setting for this <see cref="TeeterHelper"/>.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseHeadLook
		{
			set { SetArgument("useHeadLook", value); }
		}

		/// <summary>
		/// Sets the CallHighFall setting for this <see cref="TeeterHelper"/>.
		/// call highFall if fallen over the edge.  If false just call blended writhe (to go over the top of the fall behaviour of the underlying behaviour e.g. bodyBalance).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool CallHighFall
		{
			set { SetArgument("callHighFall", value); }
		}

		/// <summary>
		/// Sets the LeanAway setting for this <see cref="TeeterHelper"/>.
		/// lean away from the edge based on velocity towards the edge (if closer than 2m from edge).
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool LeanAway
		{
			set { SetArgument("leanAway", value); }
		}

		/// <summary>
		/// Sets the PreTeeterTime setting for this <see cref="TeeterHelper"/>.
		/// Time-to-edge threshold to start pre-teeter (windmilling, etc).
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float PreTeeterTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("preTeeterTime", value);
			}
		}

		/// <summary>
		/// Sets the LeanAwayTime setting for this <see cref="TeeterHelper"/>.
		/// Time-to-edge threshold to start leaning away from a potential fall.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float LeanAwayTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanAwayTime", value);
			}
		}

		/// <summary>
		/// Sets the LeanAwayScale setting for this <see cref="TeeterHelper"/>.
		/// Scales stay upright lean and hip pitch.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float LeanAwayScale
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("leanAwayScale", value);
			}
		}

		/// <summary>
		/// Sets the TeeterTime setting for this <see cref="TeeterHelper"/>.
		/// Time-to-edge threshold to start full-on teeter (more aggressive lean, drop-and-twist, etc).
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float TeeterTime
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("teeterTime", value);
			}
		}
	}

	public sealed class UpperBodyFlinchHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the UpperBodyFlinchHelper for sending a UpperBodyFlinch <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the UpperBodyFlinch <see cref="Message"/> to.</param>
		public UpperBodyFlinchHelper(Ped ped) : base(ped, "upperBodyFlinch")
		{
		}

		/// <summary>
		/// Sets the HandDistanceLeftRight setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Left-Right distance between the hands.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float HandDistanceLeftRight
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("handDistanceLeftRight", value);
			}
		}

		/// <summary>
		/// Sets the HandDistanceFrontBack setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Front-Back distance between the hands.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float HandDistanceFrontBack
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("handDistanceFrontBack", value);
			}
		}

		/// <summary>
		/// Sets the HandDistanceVertical setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Vertical distance between the hands.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float HandDistanceVertical
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("handDistanceVertical", value);
			}
		}

		/// <summary>
		/// Sets the BodyStiffness setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// stiffness of body. Value carries over to head look, spine twist.
		/// </summary>
		/// <remarks>
		/// Default value = 11.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float BodyStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("bodyStiffness", value);
			}
		}

		/// <summary>
		/// Sets the BodyDamping setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// damping value used for upper body.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float BodyDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("bodyDamping", value);
			}
		}

		/// <summary>
		/// Sets the BackBendAmount setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Amount to bend the back during the flinch.
		/// </summary>
		/// <remarks>
		/// Default value = -0.6f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float BackBendAmount
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("backBendAmount", value);
			}
		}

		/// <summary>
		/// Sets the UseRightArm setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Toggle to use the right arm.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseRightArm
		{
			set { SetArgument("useRightArm", value); }
		}

		/// <summary>
		/// Sets the UseLeftArm setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Toggle to Use the Left arm.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseLeftArm
		{
			set { SetArgument("useLeftArm", value); }
		}

		/// <summary>
		/// Sets the NoiseScale setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Amplitude of the perlin noise applied to the arms positions in the flicnh to the front part of the behaviour.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float NoiseScale
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("noiseScale", value);
			}
		}

		/// <summary>
		/// Sets the NewHit setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Relaxes the character for 1 frame if set.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool NewHit
		{
			set { SetArgument("newHit", value); }
		}

		/// <summary>
		/// Sets the ProtectHeadToggle setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Always protect head. Note if false then character flinches if target is in front, protects head if target is behind.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool ProtectHeadToggle
		{
			set { SetArgument("protectHeadToggle", value); }
		}

		/// <summary>
		/// Sets the DontBraceHead setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// don't protect head only brace from front. Turned on by bcr.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool DontBraceHead
		{
			set { SetArgument("dontBraceHead", value); }
		}

		/// <summary>
		/// Sets the ApplyStiffness setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Turned of by bcr.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool ApplyStiffness
		{
			set { SetArgument("applyStiffness", value); }
		}

		/// <summary>
		/// Sets the HeadLookAwayFromTarget setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Look away from target (unless protecting head then look between feet).
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool HeadLookAwayFromTarget
		{
			set { SetArgument("headLookAwayFromTarget", value); }
		}

		/// <summary>
		/// Sets the UseHeadLook setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// Use headlook.
		/// </summary>
		/// <remarks>
		/// Default value = True.
		/// </remarks>
		public bool UseHeadLook
		{
			set { SetArgument("useHeadLook", value); }
		}

		/// <summary>
		/// Sets the TurnTowards setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// ve balancer turn Towards, negative balancer turn Away, 0 balancer won't turn. NB.There is a 50% chance that the character will not turn even if this parameter is set to turn.
		/// </summary>
		/// <remarks>
		/// Default value = 1.
		/// Min value = -2.
		/// Max value = 2.
		/// </remarks>
		public int TurnTowards
		{
			set
			{
				if (value > 2)
					value = 2;
				if (value < -2)
					value = -2;
				SetArgument("turnTowards", value);
			}
		}

		/// <summary>
		/// Sets the Pos setting for this <see cref="UpperBodyFlinchHelper"/>.
		/// position in world-space of object to flinch from.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 Pos
		{
			set { SetArgument("pos", value); }
		}
	}

	public sealed class YankedHelper : CustomHelper
	{
		/// <summary>
		/// Creates a new Instance of the YankedHelper for sending a Yanked <see cref="Message"/> to a given <see cref="Ped"/>. 
		/// </summary>
		/// <param name="ped">The <see cref="Ped"/> to send the Yanked <see cref="Message"/> to.</param>
		public YankedHelper(Ped ped) : base(ped, "yanked")
		{
		}

		/// <summary>
		/// Sets the ArmStiffness setting for this <see cref="YankedHelper"/>.
		/// stiffness of arms when upright.
		/// </summary>
		/// <remarks>
		/// Default value = 11.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("armStiffness", value);
			}
		}

		/// <summary>
		/// Sets the ArmDamping setting for this <see cref="YankedHelper"/>.
		/// Sets damping value for the arms when upright.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armDamping", value);
			}
		}

		/// <summary>
		/// Sets the SpineDamping setting for this <see cref="YankedHelper"/>.
		/// Spine Damping when upright.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float SpineDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineDamping", value);
			}
		}

		/// <summary>
		/// Sets the SpineStiffness setting for this <see cref="YankedHelper"/>.
		/// Spine Stiffness  when upright...
		/// </summary>
		/// <remarks>
		/// Default value = 10.0f.
		/// Min value = 6.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float SpineStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 6.0f)
					value = 6.0f;
				SetArgument("spineStiffness", value);
			}
		}

		/// <summary>
		/// Sets the ArmStiffnessStart setting for this <see cref="YankedHelper"/>.
		/// armStiffness during the yanked timescale ie timeAtStartValues.
		/// </summary>
		/// <remarks>
		/// Default value = 3.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float ArmStiffnessStart
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armStiffnessStart", value);
			}
		}

		/// <summary>
		/// Sets the ArmDampingStart setting for this <see cref="YankedHelper"/>.
		/// armDamping during the yanked timescale ie timeAtStartValues.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float ArmDampingStart
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("armDampingStart", value);
			}
		}

		/// <summary>
		/// Sets the SpineDampingStart setting for this <see cref="YankedHelper"/>.
		/// spineDamping during the yanked timescale ie timeAtStartValues.
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float SpineDampingStart
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineDampingStart", value);
			}
		}

		/// <summary>
		/// Sets the SpineStiffnessStart setting for this <see cref="YankedHelper"/>.
		/// spineStiffness during the yanked timescale ie timeAtStartValues.
		/// </summary>
		/// <remarks>
		/// Default value = 3.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float SpineStiffnessStart
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineStiffnessStart", value);
			}
		}

		/// <summary>
		/// Sets the TimeAtStartValues setting for this <see cref="YankedHelper"/>.
		/// time spent with Start values for arms and spine stiffness and damping ie for whiplash efffect.
		/// </summary>
		/// <remarks>
		/// Default value = 0.4f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float TimeAtStartValues
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("timeAtStartValues", value);
			}
		}

		/// <summary>
		/// Sets the RampTimeFromStartValues setting for this <see cref="YankedHelper"/>.
		/// time spent ramping from Start to end values for arms and spine stiffness and damping ie for whiplash efffect (occurs after timeAtStartValues).
		/// </summary>
		/// <remarks>
		/// Default value = 0.1f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RampTimeFromStartValues
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rampTimeFromStartValues", value);
			}
		}

		/// <summary>
		/// Sets the StepsTillStartEnd setting for this <see cref="YankedHelper"/>.
		/// steps taken before lowerBodyStiffness starts ramping down.
		/// </summary>
		/// <remarks>
		/// Default value = 2.
		/// Min value = 0.
		/// Max value = 100.
		/// </remarks>
		public int StepsTillStartEnd
		{
			set
			{
				if (value > 100)
					value = 100;
				if (value < 0)
					value = 0;
				SetArgument("stepsTillStartEnd", value);
			}
		}

		/// <summary>
		/// Sets the TimeStartEnd setting for this <see cref="YankedHelper"/>.
		/// time from start of behaviour before lowerBodyStiffness starts ramping down by perStepReduction1.
		/// </summary>
		/// <remarks>
		/// Default value = 100.0f.
		/// Min value = 0.0f.
		/// Max value = 100.0f.
		/// </remarks>
		public float TimeStartEnd
		{
			set
			{
				if (value > 100.0f)
					value = 100.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("timeStartEnd", value);
			}
		}

		/// <summary>
		/// Sets the RampTimeToEndValues setting for this <see cref="YankedHelper"/>.
		/// time spent ramping from lowerBodyStiffness to lowerBodyStiffnessEnd.
		/// </summary>
		/// <remarks>
		/// Default value = 0.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float RampTimeToEndValues
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rampTimeToEndValues", value);
			}
		}

		/// <summary>
		/// Sets the LowerBodyStiffness setting for this <see cref="YankedHelper"/>.
		/// lowerBodyStiffness should be 12.
		/// </summary>
		/// <remarks>
		/// Default value = 12.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float LowerBodyStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lowerBodyStiffness", value);
			}
		}

		/// <summary>
		/// Sets the LowerBodyStiffnessEnd setting for this <see cref="YankedHelper"/>.
		/// lowerBodyStiffness at end.
		/// </summary>
		/// <remarks>
		/// Default value = 8.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float LowerBodyStiffnessEnd
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("lowerBodyStiffnessEnd", value);
			}
		}

		/// <summary>
		/// Sets the PerStepReduction setting for this <see cref="YankedHelper"/>.
		/// LowerBody stiffness will be reduced every step to make the character fallover.
		/// </summary>
		/// <remarks>
		/// Default value = 1.5f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float PerStepReduction
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("perStepReduction", value);
			}
		}

		/// <summary>
		/// Sets the HipPitchForward setting for this <see cref="YankedHelper"/>.
		/// Amount to bend forward at the hips (+ve forward, -ve backwards).  Behaviour switches between hipPitchForward and hipPitchBack.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = -1.3f.
		/// Max value = 1.3f.
		/// </remarks>
		public float HipPitchForward
		{
			set
			{
				if (value > 1.3f)
					value = 1.3f;
				if (value < -1.3f)
					value = -1.3f;
				SetArgument("hipPitchForward", value);
			}
		}

		/// <summary>
		/// Sets the HipPitchBack setting for this <see cref="YankedHelper"/>.
		/// Amount to bend backwards at the hips (+ve backwards, -ve forwards).  Behaviour switches between hipPitchForward and hipPitchBack.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = -1.3f.
		/// Max value = 1.3f.
		/// </remarks>
		public float HipPitchBack
		{
			set
			{
				if (value > 1.3f)
					value = 1.3f;
				if (value < -1.3f)
					value = -1.3f;
				SetArgument("hipPitchBack", value);
			}
		}

		/// <summary>
		/// Sets the SpineBend setting for this <see cref="YankedHelper"/>.
		/// Bend/Twist the spine amount.
		/// </summary>
		/// <remarks>
		/// Default value = 0.7f.
		/// Min value = 0.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float SpineBend
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineBend", value);
			}
		}

		/// <summary>
		/// Sets the FootFriction setting for this <see cref="YankedHelper"/>.
		/// Foot friction when standing/stepping.  0.5 gives a good slide sometimes.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float FootFriction
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("footFriction", value);
			}
		}

		/// <summary>
		/// Sets the TurnThresholdMin setting for this <see cref="YankedHelper"/>.
		/// min angle at which the turn with toggle to the other direction (actual toggle angle is chosen randomly in range min to max). If it is 1 then it will never toggle. If negative then no turn is applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = -0.1f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TurnThresholdMin
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -0.1f)
					value = -0.1f;
				SetArgument("turnThresholdMin", value);
			}
		}

		/// <summary>
		/// Sets the TurnThresholdMax setting for this <see cref="YankedHelper"/>.
		/// max angle at which the turn with toggle to the other direction (actual toggle angle is chosen randomly in range min to max). If it is 1 then it will never toggle. If negative then no turn is applied.
		/// </summary>
		/// <remarks>
		/// Default value = 0.6f.
		/// Min value = -0.1f.
		/// Max value = 1.0f.
		/// </remarks>
		public float TurnThresholdMax
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -0.1f)
					value = -0.1f;
				SetArgument("turnThresholdMax", value);
			}
		}

		/// <summary>
		/// Sets the UseHeadLook setting for this <see cref="YankedHelper"/>.
		/// enable and provide a look-at target to make the character's head turn to face it while balancing.
		/// </summary>
		/// <remarks>
		/// Default value = False.
		/// </remarks>
		public bool UseHeadLook
		{
			set { SetArgument("useHeadLook", value); }
		}

		/// <summary>
		/// Sets the HeadLookPos setting for this <see cref="YankedHelper"/>.
		/// position of thing to look at.
		/// </summary>
		/// <remarks>
		/// Default value = Vector3(0.0f, 0.0f, 0.0f).
		/// </remarks>
		public Vector3 HeadLookPos
		{
			set { SetArgument("headLookPos", value); }
		}

		/// <summary>
		/// Sets the HeadLookInstanceIndex setting for this <see cref="YankedHelper"/>.
		/// level index of thing to look at.
		/// </summary>
		/// <remarks>
		/// Default value = -1.
		/// Min value = -1.
		/// </remarks>
		public int HeadLookInstanceIndex
		{
			set
			{
				if (value < -1)
					value = -1;
				SetArgument("headLookInstanceIndex", value);
			}
		}

		/// <summary>
		/// Sets the HeadLookAtVelProb setting for this <see cref="YankedHelper"/>.
		/// Probability [0-1] that headLook will be looking in the direction of velocity when stepping.
		/// </summary>
		/// <remarks>
		/// Default value = -1.0f.
		/// Min value = -1.0f.
		/// Max value = 1.0f.
		/// </remarks>
		public float HeadLookAtVelProb
		{
			set
			{
				if (value > 1.0f)
					value = 1.0f;
				if (value < -1.0f)
					value = -1.0f;
				SetArgument("headLookAtVelProb", value);
			}
		}

		/// <summary>
		/// Sets the ComVelRDSThresh setting for this <see cref="YankedHelper"/>.
		/// for handsAndKnees catchfall ONLY: comVel above which rollDownstairs will start.
		/// </summary>
		/// <remarks>
		/// Default value = 2.0f.
		/// Min value = 0.0f.
		/// Max value = 20.0f.
		/// </remarks>
		public float ComVelRDSThresh
		{
			set
			{
				if (value > 20.0f)
					value = 20.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("comVelRDSThresh", value);
			}
		}

		/// <summary>
		/// Sets the HulaPeriod setting for this <see cref="YankedHelper"/>.
		/// 0.25 A complete wiggle will take 4*hulaPeriod.
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float HulaPeriod
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("hulaPeriod", value);
			}
		}

		/// <summary>
		/// Sets the HipAmplitude setting for this <see cref="YankedHelper"/>.
		/// Amount of hip movement.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float HipAmplitude
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("hipAmplitude", value);
			}
		}

		/// <summary>
		/// Sets the SpineAmplitude setting for this <see cref="YankedHelper"/>.
		/// Amount of spine movement.
		/// </summary>
		/// <remarks>
		/// Default value = 1.0f.
		/// Min value = 0.0f.
		/// Max value = 4.0f.
		/// </remarks>
		public float SpineAmplitude
		{
			set
			{
				if (value > 4.0f)
					value = 4.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("spineAmplitude", value);
			}
		}

		/// <summary>
		/// Sets the MinRelaxPeriod setting for this <see cref="YankedHelper"/>.
		/// wriggle relaxes for a minimum of minRelaxPeriod (if it is negative it is a multiplier on the time previously spent wriggling).
		/// </summary>
		/// <remarks>
		/// Default value = 0.3f.
		/// Min value = -5.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float MinRelaxPeriod
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < -5.0f)
					value = -5.0f;
				SetArgument("minRelaxPeriod", value);
			}
		}

		/// <summary>
		/// Sets the MaxRelaxPeriod setting for this <see cref="YankedHelper"/>.
		/// wriggle relaxes for a maximum of maxRelaxPeriod (if it is negative it is a multiplier on the time previously spent wriggling).
		/// </summary>
		/// <remarks>
		/// Default value = 1.5f.
		/// Min value = -5.0f.
		/// Max value = 5.0f.
		/// </remarks>
		public float MaxRelaxPeriod
		{
			set
			{
				if (value > 5.0f)
					value = 5.0f;
				if (value < -5.0f)
					value = -5.0f;
				SetArgument("maxRelaxPeriod", value);
			}
		}

		/// <summary>
		/// Sets the RollHelp setting for this <see cref="YankedHelper"/>.
		/// Amount of cheat torque applied to turn the character over.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float RollHelp
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("rollHelp", value);
			}
		}

		/// <summary>
		/// Sets the GroundLegStiffness setting for this <see cref="YankedHelper"/>.
		/// Leg Stiffness when on the ground.
		/// </summary>
		/// <remarks>
		/// Default value = 11.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float GroundLegStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("groundLegStiffness", value);
			}
		}

		/// <summary>
		/// Sets the GroundArmStiffness setting for this <see cref="YankedHelper"/>.
		/// Arm Stiffness when on the ground.
		/// </summary>
		/// <remarks>
		/// Default value = 11.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float GroundArmStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("groundArmStiffness", value);
			}
		}

		/// <summary>
		/// Sets the GroundSpineStiffness setting for this <see cref="YankedHelper"/>.
		/// Spine Stiffness when on the ground.
		/// </summary>
		/// <remarks>
		/// Default value = 14.0f.
		/// Min value = 0.0f.
		/// Max value = 16.0f.
		/// </remarks>
		public float GroundSpineStiffness
		{
			set
			{
				if (value > 16.0f)
					value = 16.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("groundSpineStiffness", value);
			}
		}

		/// <summary>
		/// Sets the GroundLegDamping setting for this <see cref="YankedHelper"/>.
		/// Leg Damping when on the ground.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float GroundLegDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("groundLegDamping", value);
			}
		}

		/// <summary>
		/// Sets the GroundArmDamping setting for this <see cref="YankedHelper"/>.
		/// Arm Damping when on the ground.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float GroundArmDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("groundArmDamping", value);
			}
		}

		/// <summary>
		/// Sets the GroundSpineDamping setting for this <see cref="YankedHelper"/>.
		/// Spine Damping when on the ground.
		/// </summary>
		/// <remarks>
		/// Default value = 0.5f.
		/// Min value = 0.0f.
		/// Max value = 2.0f.
		/// </remarks>
		public float GroundSpineDamping
		{
			set
			{
				if (value > 2.0f)
					value = 2.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("groundSpineDamping", value);
			}
		}

		/// <summary>
		/// Sets the GroundFriction setting for this <see cref="YankedHelper"/>.
		/// Friction multiplier on bodyParts when on ground.  Character can look too slidy with groundFriction = 1.  Higher values give a more jerky reation but this seems timestep dependent especially for dragged by the feet.
		/// </summary>
		/// <remarks>
		/// Default value = 8.0f.
		/// Min value = 0.0f.
		/// Max value = 10.0f.
		/// </remarks>
		public float GroundFriction
		{
			set
			{
				if (value > 10.0f)
					value = 10.0f;
				if (value < 0.0f)
					value = 0.0f;
				SetArgument("groundFriction", value);
			}
		}
	}
}
