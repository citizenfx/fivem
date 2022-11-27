using System;
using System.Collections.Generic;
using System.Linq;
using System.Security;

#if MONO_V2
using CitizenFX.FiveM.Native;
using API = CitizenFX.FiveM.Native.Natives;
using Function = CitizenFX.FiveM.Native.Natives;

namespace CitizenFX.FiveM
#else
using CitizenFX.Core.Native;

namespace CitizenFX.Core
#endif
{
	public enum WeaponComponentHash : uint
	{
		PistolClip01 = 4275109233u,
		PistolClip02 = 3978713628u,
		AtPiFlsh = 899381934u,
		AtPiSupp02 = 1709866683u,
		PistolVarmodLuxe = 3610841222u,
		GunrunningMk2Upgrade = 1623028892u,
		CombatPistolClip01 = 119648377u,
		CombatPistolClip02 = 3598405421u,
		AtPiSupp = 3271853210u,
		CombatPistolVarmodLowrider = 3328527730u,
		APPistolClip01 = 834974250u,
		APPistolClip02 = 614078421u,
		APPistolVarmodLuxe = 2608252716u,
		APPistolVarmodSecurity = 1657753414u,
		Pistol50Clip01 = 580369945u,
		Pistol50Clip02 = 3654528146u,
		AtArSupp02 = 2805810788u,
		Pistol50VarmodLuxe = 2008591151u,
		MicroSMGClip01 = 3410538224u,
		MicroSMGClip02 = 283556395u,
		AtScopeMacro = 2637152041u,
		MicroSMGVarmodLuxe = 1215999497u,
		MicroSMGVarmodSecurity = 2012362801u,
		SMGClip01 = 643254679u,
		SMGClip02 = 889808635u,
		SMGClip03 = 2043113590u,
		AtArFlsh = 2076495324u,
		AtScopeMacro02 = 1019656791u,
		SMGVarmodLuxe = 663170192u,
		AssaultSMGClip01 = 2366834608u,
		AssaultSMGClip02 = 3141985303u,
		AssaultSMGVarmodLowrider = 663517359u,
		AssaultRifleClip01 = 3193891350u,
		AssaultRifleClip02 = 2971750299u,
		AssaultRifleClip03 = 3689981245u,
		AtArAfGrip = 202788691u,
		AssaultRifleVarmodLuxe = 1319990579u,
		CarbineRifleClip01 = 2680042476u,
		CarbineRifleClip02 = 2433783441u,
		CarbineRifleClip03 = 3127044405u,
		AtRailCover01 = 1967214384u,
		AtScopeMedium = 2698550338u,
		AtArSupp = 2205435306u,
		CarbineRifleVarmodLuxe = 3634075224u,
		AdvancedRifleClip01 = 4203716879u,
		AdvancedRifleClip02 = 2395064697u,
		AtScopeSmall = 2855028148u,
		AdvancedRifleVarmodLuxe = 930927479u,
		MGClip01 = 4097109892u,
		MGClip02 = 2182449991u,
		AtScopeSmall02 = 1006677997u,
		MGVarmodLowrider = 3604658878u,
		CombatMGClip01 = 3791631178u,
		CombatMGClip02 = 3603274966u,
		CombatMGVarmodLowrider = 2466172125u,
		PumpShotgunClip01 = 3513717816u,
		AtSrSupp = 3859329886u,
		PumpShotgunVarmodLowrider = 2732039643u,
		PumpShotgunVarmodSecurity = 4052644405u,
		SawnoffShotgunClip01 = 3352699429u,
		SawnoffShotgunVarmodLuxe = 2242268665u,
		AssaultShotgunClip01 = 2498239431u,
		AssaultShotgunClip02 = 2260565874u,
		BullpupShotgunClip01 = 3377353998u,
		SniperRifleClip01 = 2613461129u,
		AtScopeLarge = 3527687644u,
		AtScopeMax = 3159677559u,
		SniperRifleVarmodLuxe = 1077065191u,
		HeavySniperClip01 = 1198478068u,
		GrenadeLauncherClip01 = 296639639u,
		RPGClip01 = 1319465907u,
		MinigunClip01 = 3370020614u,
		PoliceTorchFlashlight = 3315797997u,
		SNSPistolClip01 = 4169150169u,
		SNSPistolClip02 = 2063610803u,
		SNSPistolVarmodLowrider = 2150886575u,
		BullpupRifleClip01 = 3315675008u,
		BullpupRifleClip02 = 3009973007u,
		BullpupRifleVarmodLow = 2824322168u,
		SpecialCarbineClip01 = 3334989185u,
		SpecialCarbineClip02 = 2089537806u,
		SpecialCarbineClip03 = 1801039530u,
		SpecialCarbineVarmodLowrider = 1929467122u,
		HeavyPistolClip01 = 222992026u,
		HeavyPistolClip02 = 1694090795u,
		HeavyPistolVarmodLuxe = 2053798779u,
		SNSPistolMk2ClipNormal = 21392614u,
		SNSPistolMk2ClipExtended = 3465283442u,
		SNSPistolMk2ClipTracer = 2418909806u,
		SNSPistolMk2ClipIncendiary = 3870121849u,
		SNSPistolMk2ClipHollowpoint = 2366665730u,
		SNSPistolMk2ClipFMJ = 3239176998u,
		AtPiFlsh03 = 1246324211u,
		AtPiRail02 = 1205768792u,
		AtPiComp02 = 2860680127u,
		SpecialCarbineMk2ClipNormal = 382112385u,
		SpecialCarbineMk2ClipExtended = 3726614828u,
		SpecialCarbineMk2ClipTracer = 2271594122u,
		SpecialCarbineMk2ClipIncendiary = 3724612230u,
		SpecialCarbineMk2ClipArmorPiercing = 1362433589u,
		SpecialCarbineMk2ClipFMJ = 1346235024u,
		AtSights = 1108334355u,
		AtScopeMacroMk2 = 77277509u,
		AtScopeMediumMk2 = 3328927042u,
		AtMuzzle1 = 3113485012u,
		AtMuzzle2 = 3362234491u,
		AtMuzzle3 = 3725708239u,
		AtMuzzle4 = 3968886988u,
		AtMuzzle5 = 48731514u,
		AtMuzzle6 = 880736428u,
		AtMuzzle7 = 1303784126u,
		AtArAfGrip2 = 2640679034u,
		SpecialCarbineMk2BarrelNormal = 3879097257u,
		SpecialCarbineMk2BarrelHeavy = 4185880635u,
		PumpShotgunMk2ClipNormal = 3449028929u,
		PumpShotgunMk2ClipArmorPiercing = 1315288101u,
		PumpShotgunMk2ClipExplosive = 1004815965u,
		PumpShotgunMk2ClipHollowpoint = 3914869031u,
		PumpShotgunMk2ClipIncendiary = 2676628469u,
		AtScopeSmallMk2 = 1060929921u,
		HeavySniperMk2Suppressor = 2890063729u,
		HeavySniperMk2Muzzle8 = 1602080333u,
		BullpupRifleMk2ClipNormal = 25766362u,
		BullpupRifleMk2ClipExtended = 4021290536u,
		BullpupRifleMk2ClipTracer = 2183159977u,
		BullpupRifleMk2ClipIncendiary = 2845636954u,
		BullpupRifleMk2ClipArmorPiercing = 4205311469u,
		BullpupRifleMk2ClipFMJ = 1130501904u,
		AtScopeMacro02Mk2 = 3350057221u,
		BullpupRifleMk2BarrelNormal = 1704640795u,
		BullpupRifleMk2BarrelHeavy = 1005743559u,
		MarksmanRifleMk2ClipNormal = 2497785294u,
		MarksmanRifleMk2ClipExtended = 3872379306u,
		MarksmanRifleMk2ClipArmorPiercing = 4100968569u,
		MarksmanRifleMk2ClipFMJ = 3779763923u,
		MarksmanRifleMk2ClipIncendiary = 1842849902u,
		MarksmanRifleMk2ClipTracer = 3615105746u,
		AtScopeLargeFixedZoomMk2 = 1528590652u,
		AtMrflBarrelNormal = 941317513u,
		AtMrflBarrelHeavy = 1748450780u,
		RayPistolVarmodXmas18 = 3621517063u,
		RevolverMk2ClipNormal = 3122911422u,
		RevolverMk2ClipFMJ = 231258687u,
		RevolverMk2ClipHollowpoint = 284438159u,
		RevolverMk2ClipIncendiary = 15712037u,
		RevolverMk2ClipTracer = 3336103030u,
		AtPiComp03 = 654802123u,
		DoubleActionClipNormal = 1328622785u,
		HomingLauncherClip01 = 4162006335u,
		GusenbergClip01 = 484812453u,
		GusenbergClip02 = 3939025520u,
		VintagePistolClip01 = 1168357051u,
		VintagePistolClip02 = 867832552u,
		FireworkClip01 = 3840197261u,
		MusketClip01 = 1322387263u,
		RailgunClip01 = 59044840u,
		HeavyShotgunClip01 = 844049759u,
		HeavyShotgunClip02 = 2535257853u,
		HeavyShotgunClip03 = 2294798931u,
		MarksmanRifleClip01 = 3627761985u,
		MarksmanRifleClip02 = 3439143621u,
		AtScopeLargeFixedZoom = 471997210u,
		MarksmanRifleVarmodLuxe = 371102273u,
		CeramicPistolClipNormal = 1423184737u,
		CeramicPistolClipExtended = 2172153001u,
		CeramicPistolSuppressor = 2466764538u,
		GadgetPistolClipNormal = 2871488073u,
		CombatShotgunClipNormal = 3323278933u,
		MilitaryRifleClipNormal = 759617595u,
		MilitaryRifleClipExtended = 1749732930u,
		MilitaryRifleSight01 = 1803744149u,
		FlareGunClip01 = 2481569177u,
		NavyRevolverClipNormal = 2556346983u,
		KnuckleVarmodBase = 4081463091u,
		KnuckleVarmodPimp = 3323197061u,
		KnuckleVarmodBallas = 4007263587u,
		KnuckleVarmodDollar = 1351683121u,
		KnuckleVarmodDiamond = 2539772380u,
		KnuckleVarmodHate = 2112683568u,
		KnuckleVarmodLove = 1062111910u,
		KnuckleVarmodPlayer = 146278587u,
		KnuckleVarmodKing = 3800804335u,
		KnuckleVarmodVagos = 2062808965u,
		MarksmanPistolClip01 = 3416146413u,
		CombatPDWClip01 = 1125642654u,
		CombatPDWClip02 = 860508675u,
		CombatPDWClip03 = 1857603803u,
		DBShotgunClip01 = 703231006u,
		MachinePistolClip01 = 1198425599u,
		MachinePistolClip02 = 3106695545u,
		MachinePistolClip03 = 2850671348u,
		CompactRifleClip01 = 1363085923u,
		CompactRifleClip02 = 1509923832u,
		CompactRifleClip03 = 3322377230u,
		FlashlightLight = 3719772431u,
		SwitchbladeVarmodBase = 2436343040u,
		SwitchbladeVarmodVar1 = 1530822070u,
		SwitchbladeVarmodVar2 = 3885209186u,
		RevolverClip01 = 3917905123u,
		RevolverVarmodBoss = 384708672u,
		RevolverVarmodGoon = 2492708877u,
		CompactLauncherClipNormal = 1235472140u,
		SweeperShotgunClipNormal = 169463950u,
		MiniSMGClipNormal = 2227745491u,
		MiniSMGClipExtended = 2474561719u,
		SMGMk2ClipNormal = 1277460590u,
		SMGMk2ClipExtended = 3112393518u,
		SMGMk2ClipFMJ = 190476639u,
		SMGMk2ClipHollowpoint = 974903034u,
		SMGMk2ClipIncendiary = 3650233061u,
		SMGMk2ClipTracer = 2146055916u,
		SMGMk2Sights = 2681951826u,
		SMGMk2ScopeMacro = 3842157419u,
		SMGMk2ScopeSmall = 1038927834u,
		SMGMk2BarrelNormal = 3641720545u,
		SMGMk2BarrelHeavy = 2774849419u,
		PistolMk2ClipNormal = 2499030370u,
		PistolMk2ClipExtended = 1591132456u,
		PistolMk2ClipFMJ = 1329061674u,
		PistolMk2ClipHollowpoint = 2248057097u,
		PistolMk2ClipIncendiary = 733837882u,
		PistolMk2ClipTracer = 634039983u,
		PistolMk2Scope = 2396306288u,
		PistolMk2Flash = 1140676955u,
		PistolMk2Compensator = 568543123u,
		CombatMGMk2ClipNormal = 1227564412u,
		CombatMGMk2ClipExtended = 400507625u,
		CombatMGMk2ClipArmorPiercing = 696788003u,
		CombatMGMk2ClipFMJ = 1475288264u,
		CombatMGMk2ClipIncendiary = 3274096058u,
		CombatMGMk2ClipTracer = 4133787461u,
		CombatMGMk2BarrelNormal = 3276730932u,
		CombatMGMk2BarrelHeavy = 3051509595u,
		CarbineRifleMk2ClipNormal = 1283078430u,
		CarbineRifleMk2ClipExtended = 1574296533u,
		CarbineRifleMk2ClipArmorPiercing = 626875735u,
		CarbineRifleMk2ClipFMJ = 1141059345u,
		CarbineRifleMk2ClipIncendiary = 1025884839u,
		CarbineRifleMk2ClipTracer = 391640422u,
		CarbineRifleMk2BarrelNormal = 2201368575u,
		CarbineRifleMk2BarrelHeavy = 2335983627u,
		AssaultRifleMk2ClipNormal = 2249208895u,
		AssaultRifleMk2ClipExtended = 3509242479u,
		AssaultRifleMk2ClipArmorPiercing = 2816286296u,
		AssaultRifleMk2ClipFMJ = 1675665560u,
		AssaultRifleMk2ClipIncendiary = 4218476627u,
		AssaultRifleMk2ClipTracer = 4012669121u,
		AssaultRifleMk2BarrelNormal = 1134861606u,
		AssaultRifleMk2BarrelHeavy = 1447477866u,
		HeavySniperMk2ClipNormal = 4196276776u,
		HeavySniperMk2ClipExtended = 752418717u,
		HeavySniperMk2ClipArmorPiercing = 4164277972u,
		HeavySniperMk2ClipExplosive = 2313935527u,
		HeavySniperMk2ClipFMJ = 1005144310u,
		HeavySniperMk2ClipIncendiary = 247526935u,
		HeavySniperMk2ScopeLarge = 2193687427u,
		HeavySniperMk2ScopeNightvision = 3061846192u,
		HeavySniperMk2ScopeThermal = 776198721u,
		HeavySniperMk2Muzzle9 = 1764221345u,
		HeavySniperMk2BarrelNormal = 2425761975u,
		HeavySniperMk2BarrelHeavy = 277524638u,
		TacticalRifleClipNormal = 927578299u,
		TacticalRifleClipExtended = 2241090895u,
		AtArFlshReh = 2645680163u,
		PrecisionRifleClipNormal = 4075474698u,
		HeavyRifleClipNormal = 1525977990u,
		HeavyRifleClipExtended = 1824470811u,
		HeavyRifleSight01 = 3017917522u,
		HeavyRifleCamoDigital = 3969903833u,
		EMPLauncherClipNormal = 3532609777u,

		// Gunrunning
		SNSPistolMk2CamoSlideDigital = 3891161322u,
		SNSPistolMk2CamoSlideBrushstroke = 691432737u,
		SNSPistolMk2CamoSlideWoodland = 987648331u,
		SNSPistolMk2CamoSlideSkull = 3863286761u,
		SNSPistolMk2CamoSlideSessanta = 3447384986u,
		SNSPistolMk2CamoSlidePerseus = 4202375078u,
		SNSPistolMk2CamoSlideLeopard = 3800418970u,
		SNSPistolMk2CamoSlideZebra = 730876697u,
		SNSPistolMk2CamoSlideGeometric = 583159708u,
		SNSPistolMk2CamoSlideBoom = 2366463693u,
		SNSPistolMk2CamoSlidePatriotic = 520557834u,
		SNSPistolMk2CamoDigital = 259780317u,
		SNSPistolMk2CamoBrushstroke = 2321624822u,
		SNSPistolMk2CamoWoodland = 1996130345u,
		SNSPistolMk2CamoSkull = 2839309484u,
		SNSPistolMk2CamoSessanta = 2626704212u,
		SNSPistolMk2CamoPerseus = 1308243489u,
		SNSPistolMk2CamoLeopard = 1122574335u,
		SNSPistolMk2CamoZebra = 1420313469u,
		SNSPistolMk2CamoGeometric = 109848390u,
		SNSPistolMk2CamoBoom = 593945703u,
		SNSPistolMk2CamoPatriotic = 1142457062u,
		SpecialCarbineMk2CamoDigital = 3557537083u,
		SpecialCarbineMk2CamoBrushstroke = 1125852043u,
		SpecialCarbineMk2CamoWoodland = 886015732u,
		SpecialCarbineMk2CamoSkull = 3032680157u,
		SpecialCarbineMk2CamoSessanta = 3999758885u,
		SpecialCarbineMk2CamoPerseus = 3750812792u,
		SpecialCarbineMk2CamoLeopard = 172765678u,
		SpecialCarbineMk2CamoZebra = 2312089847u,
		SpecialCarbineMk2CamoGeometric = 2072122460u,
		SpecialCarbineMk2CamoBoom = 2308747125u,
		SpecialCarbineMk2CamoPatriotic = 1377355801u,
		PumpShotgunMk2CamoDigital = 3820854852u,
		PumpShotgunMk2CamoBrushstroke = 387223451u,
		PumpShotgunMk2CamoWoodland = 617753366u,
		PumpShotgunMk2CamoSkull = 4072589040u,
		PumpShotgunMk2CamoSessanta = 8741501u,
		PumpShotgunMk2CamoPerseus = 3693681093u,
		PumpShotgunMk2CamoLeopard = 3783533691u,
		PumpShotgunMk2CamoZebra = 3639579478u,
		PumpShotgunMk2CamoGeometric = 4012490698u,
		PumpShotgunMk2CamoBoom = 1739501925u,
		PumpShotgunMk2CamoPatriotic = 1178671645u,
		BullpupRifleMk2CamoDigital = 2923451831u,
		BullpupRifleMk2CamoBrushstroke = 3104173419u,
		BullpupRifleMk2CamoWoodland = 2797881576u,
		BullpupRifleMk2CamoSkull = 2491819116u,
		BullpupRifleMk2CamoSessanta = 2318995410u,
		BullpupRifleMk2CamoPerseus = 36929477u,
		BullpupRifleMk2CamoLeopard = 4026522462u,
		BullpupRifleMk2CamoZebra = 3720197850u,
		BullpupRifleMk2CamoGeometric = 3412267557u,
		BullpupRifleMk2CamoBoom = 2826785822u,
		BullpupRifleMk2CamoPatriotic = 3320426066u,
		MarksmanRifleMk2CamoDigital = 2425682848u,
		MarksmanRifleMk2CamoBrushstroke = 1931539634u,
		MarksmanRifleMk2CamoWoodland = 1624199183u,
		MarksmanRifleMk2CamoSkull = 4268133183u,
		MarksmanRifleMk2CamoSessanta = 4084561241u,
		MarksmanRifleMk2CamoPerseus = 423313640u,
		MarksmanRifleMk2CamoLeopard = 276639596u,
		MarksmanRifleMk2CamoZebra = 3303610433u,
		MarksmanRifleMk2CamoGeometric = 2612118995u,
		MarksmanRifleMk2CamoBoom = 996213771u,
		MarksmanRifleMk2CamoPatriotic = 3080918746u,
		RevolverMk2CamoDigital = 3225415071u,
		RevolverMk2CamoBrushstroke = 11918884u,
		RevolverMk2CamoWoodland = 176157112u,
		RevolverMk2CamoSkull = 4074914441u,
		RevolverMk2CamoSessanta = 288456487u,
		RevolverMk2CamoPerseus = 398658626u,
		RevolverMk2CamoLeopard = 628697006u,
		RevolverMk2CamoZebra = 925911836u,
		RevolverMk2CamoGeometric = 1222307441u,
		RevolverMk2CamoBoom = 552442715u,
		RevolverMk2CamoPatriotic = 3646023783u,
		SMGMk2CamoDigital = 3298267239u,
		SMGMk2CamoBrushstroke = 940943685u,
		SMGMk2CamoWoodland = 1263226800u,
		SMGMk2CamoSkull = 3966931456u,
		SMGMk2CamoSessanta = 1224100642u,
		SMGMk2CamoPerseus = 899228776u,
		SMGMk2CamoLeopard = 616006309u,
		SMGMk2CamoZebra = 2733014785u,
		SMGMk2CamoGeometric = 572063080u,
		SMGMk2CamoBoom = 1170588613u,
		SMGMk2CamoPatriotic = 966612367u,
		PistolMk2CamoSlideDigital = 3036451504u,
		PistolMk2CamoSlideBrushstroke = 438243936u,
		PistolMk2CamoSlideWoodland = 3839888240u,
		PistolMk2CamoSlideSkull = 740920107u,
		PistolMk2CamoSlideSessanta = 3753350949u,
		PistolMk2CamoSlidePerseus = 1809261196u,
		PistolMk2CamoSlideLeopard = 2648428428u,
		PistolMk2CamoSlideZebra = 3004802348u,
		PistolMk2CamoSlideGeometric = 3330502162u,
		PistolMk2CamoSlideBoom = 1135718771u,
		PistolMk2CamoSlidePatriotic = 1253942266u,
		PistolMk2CamoDigital = 1550611612u,
		PistolMk2CamoBrushstroke = 368550800u,
		PistolMk2CamoWoodland = 2525897947u,
		PistolMk2CamoSkull = 24902297u,
		PistolMk2CamoSessanta = 4066925682u,
		PistolMk2CamoPerseus = 3710005734u,
		PistolMk2CamoLeopard = 3141791350u,
		PistolMk2CamoZebra = 1301287696u,
		PistolMk2CamoGeometric = 1597093459u,
		PistolMk2CamoBoom = 1769871776u,
		PistolMk2CamoPatriotic = 2467084625u,
		CombatMGMk2CamoDigital = 1249283253u,
		CombatMGMk2CamoBrushstroke = 3437259709u,
		CombatMGMk2CamoWoodland = 3197423398u,
		CombatMGMk2CamoSkull = 1980349969u,
		CombatMGMk2CamoSessanta = 1219453777u,
		CombatMGMk2CamoPerseus = 2441508106u,
		CombatMGMk2CamoLeopard = 2220186280u,
		CombatMGMk2CamoZebra = 457967755u,
		CombatMGMk2CamoGeometric = 235171324u,
		CombatMGMk2CamoBoom = 42685294u,
		CombatMGMk2CamoPatriotic = 3607349581u,
		CarbineRifleMk2CamoDigital = 1272803094u,
		CarbineRifleMk2CamoBrushstroke = 1080719624u,
		CarbineRifleMk2CamoWoodland = 792221348u,
		CarbineRifleMk2CamoSkull = 3842785869u,
		CarbineRifleMk2CamoSessanta = 3548192559u,
		CarbineRifleMk2CamoPerseus = 2250671235u,
		CarbineRifleMk2CamoLeopard = 4095795318u,
		CarbineRifleMk2CamoZebra = 2866892280u,
		CarbineRifleMk2CamoGeometric = 2559813981u,
		CarbineRifleMk2CamoBoom = 1796459838u,
		CarbineRifleMk2CamoPatriotic = 3663056191u,
		AssaultRifleMk2CamoDigital = 2434475183u,
		AssaultRifleMk2CamoBrushstroke = 937772107u,
		AssaultRifleMk2CamoWoodland = 1401650071u,
		AssaultRifleMk2CamoSkull = 628662130u,
		AssaultRifleMk2CamoSessanta = 3309920045u,
		AssaultRifleMk2CamoPerseus = 3482022833u,
		AssaultRifleMk2CamoLeopard = 2847614993u,
		AssaultRifleMk2CamoZebra = 4234628436u,
		AssaultRifleMk2CamoGeometric = 2088750491u,
		AssaultRifleMk2CamoBoom = 2781053842u,
		AssaultRifleMk2CamoPatriotic = 3115408816u,
		HeavySniperMk2CamoDigital = 4164123906u,
		HeavySniperMk2CamoBrushstroke = 3317620069u,
		HeavySniperMk2CamoWoodland = 3916506229u,
		HeavySniperMk2CamoSkull = 329939175u,
		HeavySniperMk2CamoSessanta = 643374672u,
		HeavySniperMk2CamoPerseus = 807875052u,
		HeavySniperMk2CamoLeopard = 2893163128u,
		HeavySniperMk2CamoZebra = 3198471901u,
		HeavySniperMk2CamoGeometric = 3447155842u,
		HeavySniperMk2CamoBoom = 2881858759u,
		HeavySniperMk2CamoPatriotic = 1815270123u,

		Invalid = 4294967295u
	}

	public class WeaponComponentCollection
	{
		#region Fields

		readonly Ped _owner;
		readonly Weapon _weapon;

		readonly Dictionary<WeaponComponentHash, WeaponComponent> _weaponComponents =
			new Dictionary<WeaponComponentHash, WeaponComponent>();

		readonly WeaponComponentHash[] _components;
		readonly static InvalidWeaponComponent _invalidComponent = new InvalidWeaponComponent();

		#endregion

		internal WeaponComponentCollection(Ped owner, Weapon weapon)
		{
			_owner = owner;
			_weapon = weapon;
			_components = GetComponentsFromHash(weapon.Hash);
		}

		public WeaponComponent this[WeaponComponentHash componentHash]
		{
			get
			{
				if (_components.Contains(componentHash))
				{
					WeaponComponent component = null;
					if (!_weaponComponents.TryGetValue(componentHash, out component))
					{
						component = new WeaponComponent(_owner, _weapon, componentHash);
						_weaponComponents.Add(componentHash, component);
					}

					return component;
				}
				else
				{
					return _invalidComponent;
				}
			}
		}

		public WeaponComponent this[int index]
		{
			get
			{
				WeaponComponent component = null;
				if (index >= 0 && index < Count)
				{
					WeaponComponentHash componentHash = _components[index];

					if (!_weaponComponents.TryGetValue(componentHash, out component))
					{
						component = new WeaponComponent(_owner, _weapon, componentHash);
						_weaponComponents.Add(componentHash, component);
					}
					return component;
				}
				else
				{
					return _invalidComponent;
				}
			}
		}

		public int Count
		{
			get { return _components.Length; }
		}

		public IEnumerator<WeaponComponent> GetEnumerator()
		{
			WeaponComponent[] AllComponents = new WeaponComponent[Count];
			for (int i = 0; i < Count; i++)
			{
				AllComponents[i] = this[_components[i]];
			}
			return (AllComponents as IEnumerable<WeaponComponent>).GetEnumerator();
		}

		public WeaponComponent GetClipComponent(int index)
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.Clip ||
					component.AttachmentPoint == ComponentAttachmentPoint.Clip2)
				{
					if (index-- == 0)
					{
						return component;
					}
				}
			}
			return _invalidComponent;
		}

		public int ClipVariationsCount
		{
			get
			{
				int count = 0;
				foreach (var component in this)
				{
					if (component.AttachmentPoint == ComponentAttachmentPoint.Clip ||
					component.AttachmentPoint == ComponentAttachmentPoint.Clip2)
					{
						count++;
					}
				}
				return count;
			}
		}

		public WeaponComponent GetScopeComponent(int index)
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.Scope ||
					component.AttachmentPoint == ComponentAttachmentPoint.Scope2 ||
					component.AttachmentPoint == ComponentAttachmentPoint.Scope3)
				{
					if (index-- == 0)
					{
						return component;
					}
				}
			}
			return _invalidComponent;
		}

		public int ScopeVariationsCount
		{
			get
			{
				int count = 0;
				foreach (var component in this)
				{
					if (component.AttachmentPoint == ComponentAttachmentPoint.Scope ||
					component.AttachmentPoint == ComponentAttachmentPoint.Scope2 ||
					component.AttachmentPoint == ComponentAttachmentPoint.Scope3)
					{
						count++;
					}
				}
				return count;
			}
		}

		public WeaponComponent GetSuppressorComponent()
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.Supp ||
					component.AttachmentPoint == ComponentAttachmentPoint.Supp2 ||
					component.AttachmentPoint == ComponentAttachmentPoint.Supp3)
				{
					return component;
				}
			}
			return _invalidComponent;
		}

		public WeaponComponent GetFlashLightComponent()
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.FlashLaser ||
					component.AttachmentPoint == ComponentAttachmentPoint.FlashLaser2 ||
					component.AttachmentPoint == ComponentAttachmentPoint.FlashLaser3)
				{
					return component;
				}
			}
			return _invalidComponent;
		}

		public WeaponComponent GetLuxuryFinishComponent()
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.GunRoot)
				{
					return component;
				}
			}
			return _invalidComponent;
		}
		public WeaponComponent GetMk2CamoComponent(int index)
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.GunRoot)
				{
					if (index-- == 0)
					{
						return component;
					}
				}
			}
			return _invalidComponent;
		}

		public WeaponComponent GetMk2BarrelComponent(int index)
		{
			foreach (var component in this)
			{
				if (component.AttachmentPoint == ComponentAttachmentPoint.Barrel)
				{
					if (index-- == 0)
					{
						return component;
					}
				}
			}
			return _invalidComponent;
		}

		[SecuritySafeCritical]
		public static WeaponComponentHash[] GetComponentsFromHash(WeaponHash hash)
		{
			return _GetComponentsFromHash(hash);
		}

		[SecurityCritical]
		private static WeaponComponentHash[] _GetComponentsFromHash(WeaponHash hash)
		{
			switch (hash)
			{
				case WeaponHash.Pistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.PistolClip01,
						WeaponComponentHash.PistolClip02,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtPiSupp02,
						WeaponComponentHash.PistolVarmodLuxe,
						WeaponComponentHash.GunrunningMk2Upgrade,
					};
				case WeaponHash.CombatPistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CombatPistolClip01,
						WeaponComponentHash.CombatPistolClip02,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtPiSupp,
						WeaponComponentHash.CombatPistolVarmodLowrider,
					};
				case WeaponHash.APPistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.APPistolClip01,
						WeaponComponentHash.APPistolClip02,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtPiSupp,
						WeaponComponentHash.APPistolVarmodLuxe,
						WeaponComponentHash.APPistolVarmodSecurity,
					};
				case WeaponHash.Pistol50:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.Pistol50Clip01,
						WeaponComponentHash.Pistol50Clip02,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.Pistol50VarmodLuxe,
					};
				case WeaponHash.MicroSMG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MicroSMGClip01,
						WeaponComponentHash.MicroSMGClip02,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtScopeMacro,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.MicroSMGVarmodLuxe,
						WeaponComponentHash.MicroSMGVarmodSecurity,
					};
				case WeaponHash.SMG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SMGClip01,
						WeaponComponentHash.SMGClip02,
						WeaponComponentHash.SMGClip03,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtPiSupp,
						WeaponComponentHash.AtScopeMacro02,
						WeaponComponentHash.SMGVarmodLuxe,
						WeaponComponentHash.GunrunningMk2Upgrade,
					};
				case WeaponHash.AssaultSMG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AssaultSMGClip01,
						WeaponComponentHash.AssaultSMGClip02,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeMacro,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AssaultSMGVarmodLowrider,
					};
				case WeaponHash.AssaultRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AssaultRifleClip01,
						WeaponComponentHash.AssaultRifleClip02,
						WeaponComponentHash.AssaultRifleClip03,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeMacro,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AssaultRifleVarmodLuxe,
						WeaponComponentHash.GunrunningMk2Upgrade,
					};
				case WeaponHash.CarbineRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CarbineRifleClip01,
						WeaponComponentHash.CarbineRifleClip02,
						WeaponComponentHash.CarbineRifleClip03,
						WeaponComponentHash.AtRailCover01,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeMedium,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.CarbineRifleVarmodLuxe,
						WeaponComponentHash.GunrunningMk2Upgrade,
					};
				case WeaponHash.AdvancedRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AdvancedRifleClip01,
						WeaponComponentHash.AdvancedRifleClip02,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeSmall,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.AdvancedRifleVarmodLuxe,
					};
				case WeaponHash.MG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MGClip01,
						WeaponComponentHash.MGClip02,
						WeaponComponentHash.AtScopeSmall02,
						WeaponComponentHash.MGVarmodLowrider,
					};
				case WeaponHash.CombatMG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CombatMGClip01,
						WeaponComponentHash.CombatMGClip02,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtScopeMedium,
						WeaponComponentHash.CombatMGVarmodLowrider,
						WeaponComponentHash.GunrunningMk2Upgrade,
					};
				case WeaponHash.PumpShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.PumpShotgunClip01,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtSrSupp,
						WeaponComponentHash.PumpShotgunVarmodLowrider,
						WeaponComponentHash.GunrunningMk2Upgrade,
						WeaponComponentHash.PumpShotgunVarmodSecurity,
					};
				case WeaponHash.SawnOffShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SawnoffShotgunClip01,
						WeaponComponentHash.SawnoffShotgunVarmodLuxe,
					};
				case WeaponHash.AssaultShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AssaultShotgunClip01,
						WeaponComponentHash.AssaultShotgunClip02,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.AtArAfGrip,
					};
				case WeaponHash.BullpupShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.BullpupShotgunClip01,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AtArAfGrip,
					};
				case WeaponHash.SniperRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SniperRifleClip01,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AtScopeLarge,
						WeaponComponentHash.AtScopeMax,
						WeaponComponentHash.SniperRifleVarmodLuxe,
					};
				case WeaponHash.HeavySniper:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.HeavySniperClip01,
						WeaponComponentHash.AtScopeLarge,
						WeaponComponentHash.AtScopeMax,
						WeaponComponentHash.GunrunningMk2Upgrade,
					};
				case WeaponHash.GrenadeLauncher:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.GrenadeLauncherClip01,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtScopeSmall,
					};
				case WeaponHash.GrenadeLauncherSmoke:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtScopeSmall,
					};
				case WeaponHash.RPG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.RPGClip01,
					};
				case WeaponHash.Minigun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MinigunClip01,
					};
				case WeaponHash.SNSPistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SNSPistolClip01,
						WeaponComponentHash.SNSPistolClip02,
						WeaponComponentHash.SNSPistolVarmodLowrider,
						WeaponComponentHash.GunrunningMk2Upgrade,
					};
				case WeaponHash.BullpupRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.BullpupRifleClip01,
						WeaponComponentHash.BullpupRifleClip02,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeSmall,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.BullpupRifleVarmodLow,
						WeaponComponentHash.GunrunningMk2Upgrade,
					};
				case WeaponHash.SpecialCarbine:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SpecialCarbineClip01,
						WeaponComponentHash.SpecialCarbineClip02,
						WeaponComponentHash.SpecialCarbineClip03,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeMedium,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.SpecialCarbineVarmodLowrider,
						WeaponComponentHash.GunrunningMk2Upgrade,
					};
				case WeaponHash.HeavyPistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.HeavyPistolClip01,
						WeaponComponentHash.HeavyPistolClip02,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtPiSupp,
						WeaponComponentHash.HeavyPistolVarmodLuxe,
					};
				case WeaponHash.RayPistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.RayPistolVarmodXmas18,
					};
				case WeaponHash.RayMinigun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MinigunClip01,
					};
				case WeaponHash.DoubleAction:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.DoubleActionClipNormal,
					};
				case WeaponHash.HomingLauncher:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.HomingLauncherClip01,
					};
				case WeaponHash.Gusenberg:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.GusenbergClip01,
						WeaponComponentHash.GusenbergClip02,
					};
				case WeaponHash.VintagePistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.VintagePistolClip01,
						WeaponComponentHash.VintagePistolClip02,
						WeaponComponentHash.AtPiSupp,
					};
				case WeaponHash.Firework:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.FireworkClip01,
					};
				case WeaponHash.Musket:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MusketClip01,
					};
				case WeaponHash.Railgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.RailgunClip01,
					};
				case WeaponHash.HeavyShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.HeavyShotgunClip01,
						WeaponComponentHash.HeavyShotgunClip02,
						WeaponComponentHash.HeavyShotgunClip03,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AtArAfGrip,
					};
				case WeaponHash.MarksmanRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MarksmanRifleClip01,
						WeaponComponentHash.MarksmanRifleClip02,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtScopeLargeFixedZoom,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.MarksmanRifleVarmodLuxe,
						WeaponComponentHash.GunrunningMk2Upgrade,
					};
				case WeaponHash.CeramicPistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CeramicPistolClipNormal,
						WeaponComponentHash.CeramicPistolClipExtended,
						WeaponComponentHash.CeramicPistolSuppressor,
					};
				case WeaponHash.GadgetPistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.GadgetPistolClipNormal,
					};
				case WeaponHash.CombatShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CombatShotgunClipNormal,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtArSupp,
					};
				case WeaponHash.MilitaryRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MilitaryRifleClipNormal,
						WeaponComponentHash.MilitaryRifleClipExtended,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.MilitaryRifleSight01,
						WeaponComponentHash.AtScopeSmall,
						WeaponComponentHash.AtArSupp,
					};
				case WeaponHash.FlareGun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.FlareGunClip01,
					};
				case WeaponHash.NavyRevolver:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.NavyRevolverClipNormal,
					};
				case WeaponHash.KnuckleDuster:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.KnuckleVarmodBase,
						WeaponComponentHash.KnuckleVarmodPimp,
						WeaponComponentHash.KnuckleVarmodBallas,
						WeaponComponentHash.KnuckleVarmodDollar,
						WeaponComponentHash.KnuckleVarmodDiamond,
						WeaponComponentHash.KnuckleVarmodHate,
						WeaponComponentHash.KnuckleVarmodLove,
						WeaponComponentHash.KnuckleVarmodPlayer,
						WeaponComponentHash.KnuckleVarmodKing,
						WeaponComponentHash.KnuckleVarmodVagos,
					};
				case WeaponHash.MarksmanPistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MarksmanPistolClip01,
					};
				case WeaponHash.CombatPDW:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CombatPDWClip01,
						WeaponComponentHash.CombatPDWClip02,
						WeaponComponentHash.CombatPDWClip03,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.AtScopeSmall,
					};
				case WeaponHash.DoubleBarrelShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.DBShotgunClip01,
					};
				case WeaponHash.MachinePistol:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MachinePistolClip01,
						WeaponComponentHash.MachinePistolClip02,
						WeaponComponentHash.MachinePistolClip03,
						WeaponComponentHash.AtPiSupp,
					};
				case WeaponHash.CompactRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CompactRifleClip01,
						WeaponComponentHash.CompactRifleClip02,
						WeaponComponentHash.CompactRifleClip03,
					};
				case WeaponHash.SwitchBlade:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SwitchbladeVarmodBase,
						WeaponComponentHash.SwitchbladeVarmodVar1,
						WeaponComponentHash.SwitchbladeVarmodVar2,
					};
				case WeaponHash.Revolver:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.RevolverClip01,
						WeaponComponentHash.RevolverVarmodBoss,
						WeaponComponentHash.RevolverVarmodGoon,
						WeaponComponentHash.GunrunningMk2Upgrade,
					};
				case WeaponHash.CompactGrenadeLauncher:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CompactLauncherClipNormal,
					};
				case WeaponHash.SweeperShotgun:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SweeperShotgunClipNormal,
					};
				case WeaponHash.MiniSMG:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MiniSMGClipNormal,
						WeaponComponentHash.MiniSMGClipExtended,
					};
				case WeaponHash.TacticalRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.TacticalRifleClipNormal,
						WeaponComponentHash.TacticalRifleClipExtended,
						WeaponComponentHash.AtArFlshReh,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AtArAfGrip,
					};
				case WeaponHash.PrecisionRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.PrecisionRifleClipNormal,
					};
				case WeaponHash.HeavyRifle:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.HeavyRifleClipNormal,
						WeaponComponentHash.HeavyRifleClipExtended,
						WeaponComponentHash.HeavyRifleSight01,
						WeaponComponentHash.AtScopeMedium,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.AtArAfGrip,
						WeaponComponentHash.HeavyRifleCamoDigital,
					};
				case WeaponHash.EMPLauncher:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.EMPLauncherClipNormal,
					};

				// Gunrunning
				case WeaponHash.SNSPistolMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SNSPistolMk2ClipNormal,
						WeaponComponentHash.SNSPistolMk2ClipExtended,
						WeaponComponentHash.SNSPistolMk2ClipTracer,
						WeaponComponentHash.SNSPistolMk2ClipIncendiary,
						WeaponComponentHash.SNSPistolMk2ClipHollowpoint,
						WeaponComponentHash.SNSPistolMk2ClipFMJ,
						WeaponComponentHash.AtPiFlsh03,
						WeaponComponentHash.AtPiRail02,
						WeaponComponentHash.AtPiSupp02,
						WeaponComponentHash.AtPiComp02,
						WeaponComponentHash.SNSPistolMk2CamoSlideDigital,
						WeaponComponentHash.SNSPistolMk2CamoSlideBrushstroke,
						WeaponComponentHash.SNSPistolMk2CamoSlideWoodland,
						WeaponComponentHash.SNSPistolMk2CamoSlideSkull,
						WeaponComponentHash.SNSPistolMk2CamoSlideSessanta,
						WeaponComponentHash.SNSPistolMk2CamoSlidePerseus,
						WeaponComponentHash.SNSPistolMk2CamoSlideLeopard,
						WeaponComponentHash.SNSPistolMk2CamoSlideZebra,
						WeaponComponentHash.SNSPistolMk2CamoSlideGeometric,
						WeaponComponentHash.SNSPistolMk2CamoSlideBoom,
						WeaponComponentHash.SNSPistolMk2CamoSlidePatriotic,
						WeaponComponentHash.SNSPistolMk2CamoDigital,
						WeaponComponentHash.SNSPistolMk2CamoBrushstroke,
						WeaponComponentHash.SNSPistolMk2CamoWoodland,
						WeaponComponentHash.SNSPistolMk2CamoSkull,
						WeaponComponentHash.SNSPistolMk2CamoSessanta,
						WeaponComponentHash.SNSPistolMk2CamoPerseus,
						WeaponComponentHash.SNSPistolMk2CamoLeopard,
						WeaponComponentHash.SNSPistolMk2CamoZebra,
						WeaponComponentHash.SNSPistolMk2CamoGeometric,
						WeaponComponentHash.SNSPistolMk2CamoBoom,
						WeaponComponentHash.SNSPistolMk2CamoPatriotic,
					};
				case WeaponHash.SpecialCarbineMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SpecialCarbineMk2ClipNormal,
						WeaponComponentHash.SpecialCarbineMk2ClipExtended,
						WeaponComponentHash.SpecialCarbineMk2ClipTracer,
						WeaponComponentHash.SpecialCarbineMk2ClipIncendiary,
						WeaponComponentHash.SpecialCarbineMk2ClipArmorPiercing,
						WeaponComponentHash.SpecialCarbineMk2ClipFMJ,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtSights,
						WeaponComponentHash.AtScopeMacroMk2,
						WeaponComponentHash.AtScopeMediumMk2,
						WeaponComponentHash.SpecialCarbineMk2BarrelNormal,
						WeaponComponentHash.SpecialCarbineMk2BarrelHeavy,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AtMuzzle1,
						WeaponComponentHash.AtMuzzle2,
						WeaponComponentHash.AtMuzzle3,
						WeaponComponentHash.AtMuzzle4,
						WeaponComponentHash.AtMuzzle5,
						WeaponComponentHash.AtMuzzle6,
						WeaponComponentHash.AtMuzzle7,
						WeaponComponentHash.AtArAfGrip2,
						WeaponComponentHash.SpecialCarbineMk2CamoDigital,
						WeaponComponentHash.SpecialCarbineMk2CamoBrushstroke,
						WeaponComponentHash.SpecialCarbineMk2CamoWoodland,
						WeaponComponentHash.SpecialCarbineMk2CamoSkull,
						WeaponComponentHash.SpecialCarbineMk2CamoSessanta,
						WeaponComponentHash.SpecialCarbineMk2CamoPerseus,
						WeaponComponentHash.SpecialCarbineMk2CamoLeopard,
						WeaponComponentHash.SpecialCarbineMk2CamoZebra,
						WeaponComponentHash.SpecialCarbineMk2CamoGeometric,
						WeaponComponentHash.SpecialCarbineMk2CamoBoom,
						WeaponComponentHash.SpecialCarbineMk2CamoPatriotic,
					};
				case WeaponHash.PumpShotgunMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.PumpShotgunMk2ClipNormal,
						WeaponComponentHash.PumpShotgunMk2ClipArmorPiercing,
						WeaponComponentHash.PumpShotgunMk2ClipExplosive,
						WeaponComponentHash.PumpShotgunMk2ClipHollowpoint,
						WeaponComponentHash.PumpShotgunMk2ClipIncendiary,
						WeaponComponentHash.AtSights,
						WeaponComponentHash.AtScopeMacroMk2,
						WeaponComponentHash.AtScopeSmallMk2,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.HeavySniperMk2Suppressor,
						WeaponComponentHash.HeavySniperMk2Muzzle8,
						WeaponComponentHash.PumpShotgunMk2CamoDigital,
						WeaponComponentHash.PumpShotgunMk2CamoBrushstroke,
						WeaponComponentHash.PumpShotgunMk2CamoWoodland,
						WeaponComponentHash.PumpShotgunMk2CamoSkull,
						WeaponComponentHash.PumpShotgunMk2CamoSessanta,
						WeaponComponentHash.PumpShotgunMk2CamoPerseus,
						WeaponComponentHash.PumpShotgunMk2CamoLeopard,
						WeaponComponentHash.PumpShotgunMk2CamoZebra,
						WeaponComponentHash.PumpShotgunMk2CamoGeometric,
						WeaponComponentHash.PumpShotgunMk2CamoBoom,
						WeaponComponentHash.PumpShotgunMk2CamoPatriotic,
					};
				case WeaponHash.BullpupRifleMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.BullpupRifleMk2ClipNormal,
						WeaponComponentHash.BullpupRifleMk2ClipExtended,
						WeaponComponentHash.BullpupRifleMk2ClipTracer,
						WeaponComponentHash.BullpupRifleMk2ClipIncendiary,
						WeaponComponentHash.BullpupRifleMk2ClipArmorPiercing,
						WeaponComponentHash.BullpupRifleMk2ClipFMJ,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtSights,
						WeaponComponentHash.AtScopeMacro02Mk2,
						WeaponComponentHash.AtScopeSmallMk2,
						WeaponComponentHash.BullpupRifleMk2BarrelNormal,
						WeaponComponentHash.BullpupRifleMk2BarrelHeavy,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.AtMuzzle1,
						WeaponComponentHash.AtMuzzle2,
						WeaponComponentHash.AtMuzzle3,
						WeaponComponentHash.AtMuzzle4,
						WeaponComponentHash.AtMuzzle5,
						WeaponComponentHash.AtMuzzle6,
						WeaponComponentHash.AtMuzzle7,
						WeaponComponentHash.AtArAfGrip2,
						WeaponComponentHash.BullpupRifleMk2CamoDigital,
						WeaponComponentHash.BullpupRifleMk2CamoBrushstroke,
						WeaponComponentHash.BullpupRifleMk2CamoWoodland,
						WeaponComponentHash.BullpupRifleMk2CamoSkull,
						WeaponComponentHash.BullpupRifleMk2CamoSessanta,
						WeaponComponentHash.BullpupRifleMk2CamoPerseus,
						WeaponComponentHash.BullpupRifleMk2CamoLeopard,
						WeaponComponentHash.BullpupRifleMk2CamoZebra,
						WeaponComponentHash.BullpupRifleMk2CamoGeometric,
						WeaponComponentHash.BullpupRifleMk2CamoBoom,
						WeaponComponentHash.BullpupRifleMk2CamoPatriotic,
					};
				case WeaponHash.MarksmanRifleMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.MarksmanRifleMk2ClipNormal,
						WeaponComponentHash.MarksmanRifleMk2ClipExtended,
						WeaponComponentHash.MarksmanRifleMk2ClipArmorPiercing,
						WeaponComponentHash.MarksmanRifleMk2ClipFMJ,
						WeaponComponentHash.MarksmanRifleMk2ClipIncendiary,
						WeaponComponentHash.MarksmanRifleMk2ClipTracer,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtSights,
						WeaponComponentHash.AtScopeMediumMk2,
						WeaponComponentHash.AtScopeLargeFixedZoomMk2,
						WeaponComponentHash.AtMrflBarrelNormal,
						WeaponComponentHash.AtMrflBarrelHeavy,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.AtMuzzle1,
						WeaponComponentHash.AtMuzzle2,
						WeaponComponentHash.AtMuzzle3,
						WeaponComponentHash.AtMuzzle4,
						WeaponComponentHash.AtMuzzle5,
						WeaponComponentHash.AtMuzzle6,
						WeaponComponentHash.AtMuzzle7,
						WeaponComponentHash.AtArAfGrip2,
						WeaponComponentHash.MarksmanRifleMk2CamoDigital,
						WeaponComponentHash.MarksmanRifleMk2CamoBrushstroke,
						WeaponComponentHash.MarksmanRifleMk2CamoWoodland,
						WeaponComponentHash.MarksmanRifleMk2CamoSkull,
						WeaponComponentHash.MarksmanRifleMk2CamoSessanta,
						WeaponComponentHash.MarksmanRifleMk2CamoPerseus,
						WeaponComponentHash.MarksmanRifleMk2CamoLeopard,
						WeaponComponentHash.MarksmanRifleMk2CamoZebra,
						WeaponComponentHash.MarksmanRifleMk2CamoGeometric,
						WeaponComponentHash.MarksmanRifleMk2CamoBoom,
						WeaponComponentHash.MarksmanRifleMk2CamoPatriotic,
					};
				case WeaponHash.RevolverMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.RevolverMk2ClipNormal,
						WeaponComponentHash.RevolverMk2ClipFMJ,
						WeaponComponentHash.RevolverMk2ClipHollowpoint,
						WeaponComponentHash.RevolverMk2ClipIncendiary,
						WeaponComponentHash.RevolverMk2ClipTracer,
						WeaponComponentHash.AtSights,
						WeaponComponentHash.AtScopeMacroMk2,
						WeaponComponentHash.AtPiFlsh,
						WeaponComponentHash.AtPiComp03,
						WeaponComponentHash.RevolverMk2CamoDigital,
						WeaponComponentHash.RevolverMk2CamoBrushstroke,
						WeaponComponentHash.RevolverMk2CamoWoodland,
						WeaponComponentHash.RevolverMk2CamoSkull,
						WeaponComponentHash.RevolverMk2CamoSessanta,
						WeaponComponentHash.RevolverMk2CamoPerseus,
						WeaponComponentHash.RevolverMk2CamoLeopard,
						WeaponComponentHash.RevolverMk2CamoZebra,
						WeaponComponentHash.RevolverMk2CamoGeometric,
						WeaponComponentHash.RevolverMk2CamoBoom,
						WeaponComponentHash.RevolverMk2CamoPatriotic,
					};
				case WeaponHash.SMGMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.SMGMk2ClipNormal,
						WeaponComponentHash.SMGMk2ClipExtended,
						WeaponComponentHash.SMGMk2ClipFMJ,
						WeaponComponentHash.SMGMk2ClipHollowpoint,
						WeaponComponentHash.SMGMk2ClipIncendiary,
						WeaponComponentHash.SMGMk2ClipTracer,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.SMGMk2Sights,
						WeaponComponentHash.SMGMk2ScopeMacro,
						WeaponComponentHash.SMGMk2ScopeSmall,
						WeaponComponentHash.AtPiSupp,
						WeaponComponentHash.SMGMk2BarrelNormal,
						WeaponComponentHash.SMGMk2BarrelHeavy,
						WeaponComponentHash.AtMuzzle1,
						WeaponComponentHash.AtMuzzle2,
						WeaponComponentHash.AtMuzzle3,
						WeaponComponentHash.AtMuzzle4,
						WeaponComponentHash.AtMuzzle5,
						WeaponComponentHash.AtMuzzle6,
						WeaponComponentHash.AtMuzzle7,
						WeaponComponentHash.SMGMk2CamoDigital,
						WeaponComponentHash.SMGMk2CamoBrushstroke,
						WeaponComponentHash.SMGMk2CamoWoodland,
						WeaponComponentHash.SMGMk2CamoSkull,
						WeaponComponentHash.SMGMk2CamoSessanta,
						WeaponComponentHash.SMGMk2CamoPerseus,
						WeaponComponentHash.SMGMk2CamoLeopard,
						WeaponComponentHash.SMGMk2CamoZebra,
						WeaponComponentHash.SMGMk2CamoGeometric,
						WeaponComponentHash.SMGMk2CamoBoom,
						WeaponComponentHash.SMGMk2CamoPatriotic,
					};
				case WeaponHash.PistolMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.PistolMk2ClipNormal,
						WeaponComponentHash.PistolMk2ClipExtended,
						WeaponComponentHash.PistolMk2ClipFMJ,
						WeaponComponentHash.PistolMk2ClipHollowpoint,
						WeaponComponentHash.PistolMk2ClipIncendiary,
						WeaponComponentHash.PistolMk2ClipTracer,
						WeaponComponentHash.PistolMk2Scope,
						WeaponComponentHash.PistolMk2Flash,
						WeaponComponentHash.AtPiSupp02,
						WeaponComponentHash.PistolMk2Compensator,
						WeaponComponentHash.PistolMk2CamoDigital,
						WeaponComponentHash.PistolMk2CamoBrushstroke,
						WeaponComponentHash.PistolMk2CamoWoodland,
						WeaponComponentHash.PistolMk2CamoSkull,
						WeaponComponentHash.PistolMk2CamoSessanta,
						WeaponComponentHash.PistolMk2CamoPerseus,
						WeaponComponentHash.PistolMk2CamoLeopard,
						WeaponComponentHash.PistolMk2CamoZebra,
						WeaponComponentHash.PistolMk2CamoGeometric,
						WeaponComponentHash.PistolMk2CamoBoom,
						WeaponComponentHash.PistolMk2CamoPatriotic,
						WeaponComponentHash.PistolMk2CamoSlideDigital,
						WeaponComponentHash.PistolMk2CamoSlideBrushstroke,
						WeaponComponentHash.PistolMk2CamoSlideWoodland,
						WeaponComponentHash.PistolMk2CamoSlideSkull,
						WeaponComponentHash.PistolMk2CamoSlideSessanta,
						WeaponComponentHash.PistolMk2CamoSlidePerseus,
						WeaponComponentHash.PistolMk2CamoSlideLeopard,
						WeaponComponentHash.PistolMk2CamoSlideZebra,
						WeaponComponentHash.PistolMk2CamoSlideGeometric,
						WeaponComponentHash.PistolMk2CamoSlideBoom,
						WeaponComponentHash.PistolMk2CamoSlidePatriotic,
					};
				case WeaponHash.CombatMGMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CombatMGMk2ClipNormal,
						WeaponComponentHash.CombatMGMk2ClipExtended,
						WeaponComponentHash.CombatMGMk2ClipArmorPiercing,
						WeaponComponentHash.CombatMGMk2ClipFMJ,
						WeaponComponentHash.CombatMGMk2ClipIncendiary,
						WeaponComponentHash.CombatMGMk2ClipTracer,
						WeaponComponentHash.AtSights,
						WeaponComponentHash.AtScopeSmallMk2,
						WeaponComponentHash.AtScopeMediumMk2,
						WeaponComponentHash.AtMuzzle1,
						WeaponComponentHash.AtMuzzle2,
						WeaponComponentHash.AtMuzzle3,
						WeaponComponentHash.AtMuzzle4,
						WeaponComponentHash.AtMuzzle5,
						WeaponComponentHash.AtMuzzle6,
						WeaponComponentHash.AtMuzzle7,
						WeaponComponentHash.AtArAfGrip2,
						WeaponComponentHash.CombatMGMk2BarrelNormal,
						WeaponComponentHash.CombatMGMk2BarrelHeavy,
						WeaponComponentHash.CombatMGMk2CamoDigital,
						WeaponComponentHash.CombatMGMk2CamoBrushstroke,
						WeaponComponentHash.CombatMGMk2CamoWoodland,
						WeaponComponentHash.CombatMGMk2CamoSkull,
						WeaponComponentHash.CombatMGMk2CamoSessanta,
						WeaponComponentHash.CombatMGMk2CamoPerseus,
						WeaponComponentHash.CombatMGMk2CamoLeopard,
						WeaponComponentHash.CombatMGMk2CamoZebra,
						WeaponComponentHash.CombatMGMk2CamoGeometric,
						WeaponComponentHash.CombatMGMk2CamoBoom,
						WeaponComponentHash.CombatMGMk2CamoPatriotic,
					};
				case WeaponHash.CarbineRifleMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.CarbineRifleMk2ClipNormal,
						WeaponComponentHash.CarbineRifleMk2ClipExtended,
						WeaponComponentHash.CarbineRifleMk2ClipArmorPiercing,
						WeaponComponentHash.CarbineRifleMk2ClipFMJ,
						WeaponComponentHash.CarbineRifleMk2ClipIncendiary,
						WeaponComponentHash.CarbineRifleMk2ClipTracer,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtSights,
						WeaponComponentHash.AtScopeMacroMk2,
						WeaponComponentHash.AtScopeMediumMk2,
						WeaponComponentHash.AtArAfGrip2,
						WeaponComponentHash.CarbineRifleMk2BarrelNormal,
						WeaponComponentHash.CarbineRifleMk2BarrelHeavy,
						WeaponComponentHash.AtArSupp,
						WeaponComponentHash.AtMuzzle1,
						WeaponComponentHash.AtMuzzle2,
						WeaponComponentHash.AtMuzzle3,
						WeaponComponentHash.AtMuzzle4,
						WeaponComponentHash.AtMuzzle5,
						WeaponComponentHash.AtMuzzle6,
						WeaponComponentHash.AtMuzzle7,
						WeaponComponentHash.CarbineRifleMk2CamoDigital,
						WeaponComponentHash.CarbineRifleMk2CamoBrushstroke,
						WeaponComponentHash.CarbineRifleMk2CamoWoodland,
						WeaponComponentHash.CarbineRifleMk2CamoSkull,
						WeaponComponentHash.CarbineRifleMk2CamoSessanta,
						WeaponComponentHash.CarbineRifleMk2CamoPerseus,
						WeaponComponentHash.CarbineRifleMk2CamoLeopard,
						WeaponComponentHash.CarbineRifleMk2CamoZebra,
						WeaponComponentHash.CarbineRifleMk2CamoGeometric,
						WeaponComponentHash.CarbineRifleMk2CamoBoom,
						WeaponComponentHash.CarbineRifleMk2CamoPatriotic,
					};
				case WeaponHash.AssaultRifleMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.AssaultRifleMk2ClipNormal,
						WeaponComponentHash.AssaultRifleMk2ClipExtended,
						WeaponComponentHash.AssaultRifleMk2ClipArmorPiercing,
						WeaponComponentHash.AssaultRifleMk2ClipFMJ,
						WeaponComponentHash.AssaultRifleMk2ClipIncendiary,
						WeaponComponentHash.AssaultRifleMk2ClipTracer,
						WeaponComponentHash.AtArFlsh,
						WeaponComponentHash.AtSights,
						WeaponComponentHash.AtScopeMacroMk2,
						WeaponComponentHash.AtScopeMediumMk2,
						WeaponComponentHash.AtArSupp02,
						WeaponComponentHash.AtMuzzle1,
						WeaponComponentHash.AtMuzzle2,
						WeaponComponentHash.AtMuzzle3,
						WeaponComponentHash.AtMuzzle4,
						WeaponComponentHash.AtMuzzle5,
						WeaponComponentHash.AtMuzzle6,
						WeaponComponentHash.AtMuzzle7,
						WeaponComponentHash.AtArAfGrip2,
						WeaponComponentHash.AssaultRifleMk2BarrelNormal,
						WeaponComponentHash.AssaultRifleMk2BarrelHeavy,
						WeaponComponentHash.AssaultRifleMk2CamoDigital,
						WeaponComponentHash.AssaultRifleMk2CamoBrushstroke,
						WeaponComponentHash.AssaultRifleMk2CamoWoodland,
						WeaponComponentHash.AssaultRifleMk2CamoSkull,
						WeaponComponentHash.AssaultRifleMk2CamoSessanta,
						WeaponComponentHash.AssaultRifleMk2CamoPerseus,
						WeaponComponentHash.AssaultRifleMk2CamoLeopard,
						WeaponComponentHash.AssaultRifleMk2CamoZebra,
						WeaponComponentHash.AssaultRifleMk2CamoGeometric,
						WeaponComponentHash.AssaultRifleMk2CamoBoom,
						WeaponComponentHash.AssaultRifleMk2CamoPatriotic,
					};
				case WeaponHash.HeavySniperMk2:
					return new WeaponComponentHash[]
					{
						WeaponComponentHash.HeavySniperMk2ClipNormal,
						WeaponComponentHash.HeavySniperMk2ClipExtended,
						WeaponComponentHash.HeavySniperMk2ClipArmorPiercing,
						WeaponComponentHash.HeavySniperMk2ClipExplosive,
						WeaponComponentHash.HeavySniperMk2ClipFMJ,
						WeaponComponentHash.HeavySniperMk2ClipIncendiary,
						WeaponComponentHash.HeavySniperMk2ScopeLarge,
						WeaponComponentHash.AtScopeMax,
						WeaponComponentHash.HeavySniperMk2ScopeNightvision,
						WeaponComponentHash.HeavySniperMk2ScopeThermal,
						WeaponComponentHash.HeavySniperMk2Suppressor,
						WeaponComponentHash.HeavySniperMk2Muzzle8,
						WeaponComponentHash.HeavySniperMk2Muzzle9,
						WeaponComponentHash.HeavySniperMk2BarrelNormal,
						WeaponComponentHash.HeavySniperMk2BarrelHeavy,
						WeaponComponentHash.HeavySniperMk2CamoDigital,
						WeaponComponentHash.HeavySniperMk2CamoBrushstroke,
						WeaponComponentHash.HeavySniperMk2CamoWoodland,
						WeaponComponentHash.HeavySniperMk2CamoSkull,
						WeaponComponentHash.HeavySniperMk2CamoSessanta,
						WeaponComponentHash.HeavySniperMk2CamoPerseus,
						WeaponComponentHash.HeavySniperMk2CamoLeopard,
						WeaponComponentHash.HeavySniperMk2CamoZebra,
						WeaponComponentHash.HeavySniperMk2CamoGeometric,
						WeaponComponentHash.HeavySniperMk2CamoBoom,
						WeaponComponentHash.HeavySniperMk2CamoPatriotic,
					};
			}

			WeaponComponentHash[] result = null;

			for (int i = 0, count = API.GetNumDlcWeapons(); i < count; i++)
			{
				unsafe
				{
					DlcWeaponData weaponData;
					if (Function.Call<bool>(Hash.GET_DLC_WEAPON_DATA, i, &weaponData))
					{
						if (weaponData.Hash == hash)
						{
							result = new WeaponComponentHash[API.GetNumDlcWeaponComponents(i)];

							for (int j = 0; j < result.Length; j++)
							{
								DlcWeaponComponentData componentData;
								if (Function.Call<bool>(Hash.GET_DLC_WEAPON_COMPONENT_DATA, i, j, &componentData))
								{
									result[j] = componentData.Hash;
								}
								else
								{
									result[j] = WeaponComponentHash.Invalid;
								}
							}
							return result;
						}
					}
				}
			}


			if (result == null)
			{
				result = new WeaponComponentHash[0];
			}

			return result;
		}
	}
}
