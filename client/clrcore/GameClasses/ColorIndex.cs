using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public class ColorIndex
    {
        internal static int[] Colors =  
        {
            0x0A0A0A, 0x252527, 0x656A79, 0x58595A, 0x9CA1A3, 0x96918C, 0x515459, 0x3F3E45,
			0xA5A9A7, 0x979592, 0x767B7C, 0x5A5752, 0xADB0B0, 0x848988, 0x949D9F, 0xA4A7A5, 0x585853, 0xA4A096, 
			0xAFB1B1, 0x6D6C6E, 0x64686A, 0x525661, 0x8C929A, 0x5B5D5E, 0xBDBEC6, 0xB6B6B6, 0x646464, 0xE20606, 
			0x960800, 0x6B0000, 0x611009, 0x4A0A0A, 0x730B0B, 0x570707, 0x260306, 0x9E0000, 0x140002, 0x0F0404, 
			0x0F080A, 0x39191D, 0x552725, 0x4C2929, 0x741D28, 0x6D2837, 0x730A27, 0x640D1B, 0x620B1C, 0x731827, 
			0xAB988F, 0x20202C, 0x44624F, 0x2E5B20, 0x1E2E32, 0x304F45, 0x4D6268, 0x5E7072, 0x193826, 0x2D3A35, 
			0x335F3F, 0x47783C, 0x93A396, 0x9AA790, 0x263739, 0x4C75B7, 0x46597A, 0x5D7E8D, 0x3B4E78, 0x3D4A68, 
			0x6D7A88, 0x162248, 0x272F4B, 0x4E6881, 0x6A7A8C, 0x6F8297, 0x0E316D, 0x395A83, 0x204B6B, 0x2B3E57, 
			0x364155, 0x6C8495, 0x4D5D60, 0x406C8F, 0x134573, 0x105082, 0x385694, 0x001C32, 0x596E87, 0x223457, 
			0x20202C, 0xF5890F, 0x917347, 0x8E8C46, 0xAAAD8E, 0xAE9B7F, 0x96816C, 0x7A7560, 0x9D9872, 0x989586, 
			0x9C8D71, 0x691E3B, 0x722A3F, 0x7C1B44, 0x221918, 0x7F6956, 0x473532, 0x695853, 0x624428, 0x7D6256, 
			0xAA9D84, 0x7B715E, 0xAB9276, 0x635C5A, 0xC9C9C9, 0xD6DAD6, 0x9F9D94, 0x93A396, 0x9C9C98, 0xA7A28F, 
			0x0F6A89, 0xA19983, 0xA3ADC6, 0x9B8B80, 0x8494AB, 0x9EA4AB, 0x86446E, 0xE20606, 0x47783C, 0xD78E10, 
			0x2A77A1, 0x421F21, 0x6F675F, 0xFC2600, 0xFC6D00, 0xFFFFFF
        };

        internal static string[] ColorNames = 
        {
            "Black", "BlackPoly", "ConcordBluePoly", "PewterGrayPoly", "SilverStonePoly",
			"WinningSilverPoly", "SteelGrayPoly", "ShadowSilverPoly", "SilverStonePoly2", "PorcelainSilverPoly", "GrayPoly",
			"AnthraciteGrayPoly", "AstraSilverPoly", "AscotGray", "ClearCrystalBlueFrostPoly", "SilverPoly", "DarkTitaniumPoly", 
			"TitaniumFrostPoly", "PoliceWhite", "MediumGrayPoly", "MediumGrayPoly2", "SteelGrayPoly2", "SlateGray",
			"GunMetalPoly", "LightBlueGrey", "SecuricorLightGray", "ArcticWhite", "VeryRed", "TorinoRedPearl", "FormulaRed",
			"BlazeRed", "GracefulRedMica", "GarnetRedPoly", "DesertRed", "CabernetRedPoly", "TurismoRed", "DesertRed2",
			"CurrantRedSolid", "CurrantRedPoly", "ElectricCurrantRedPoly", "MediumCabernetSolid", "WildStrawberryPoly",
			"MediumRedSolid", "BrightRed", "BrightRed2", "MediumGarnetRedPoly", "BrilliantRedPoly", "BrilliantRedPoly2",
			"AlabasterSolid", "TwilightBluePoly", "TorchRed", "Green", "DeepJewelGreen", "AgateGreen", "PetrolBlueGreenPoly",
			"Hoods", "Green2", "DarkGreenPoly", "RioRed", "SecuricorDarkGreen", "SeafoamPoly", "PastelAlabasterSolid",
			"MidnightBlue", "StrikingBlue", "SaxonyBluePoly", "JasperGreenPoly", "MarinerBlue", "HarborBluePoly",
			"DiamondBluePoly", "SurfBlue", "NauticalBluePoly", "LightCrystalBluePoly", "MedRegattaBluePoly", "SpinnakerBlueSolid",
			"UltraBluePoly", "BrightBluePoly", "NassauBluePoly", "MediumSapphireBluePoly", "SteelBluePoly",
			"LightSapphireBluePoly", "MalachitePoly", "MediumMauiBluePoly", "BrightBluePoly2", "BrightBluePoly3", "Blue",
			"DarkSapphireBluePoly", "LightSapphireBluePoly2", "MediumSapphireBlueFiremist", "TwilightBluePoly2", "TaxiYellow",
			"RaceYellowSolid", "PastelAlabaster", "OxfordWhiteSolid", "Flax", "MediumFlax", "PuebloBeige", "LightIvory",
			"SmokeSilverPoly", "BisqueFrostPoly", "ClassicRed", "VermilionSolid", "VermillionSolid", "BistonBrownPoly",
			"LightBeechwoodPoly", "DarkBeechwoodPoly", "DarkSablePoly", "MediumBeechwoodPoly", "WoodrosePoly", "SandalwoodFrostPoly",
			"MediumSandalwoodPoly", "CopperBeige", "WarmGreyMica", "White", "FrostWhite", "HoneyBeigePoly", "SeafoamPoly2",
			"LightTitaniumPoly", "LightChampagnePoly", "ArcticPearl", "LightDriftwoodPoly", "WhiteDiamondPearl", "AntelopeBeige",
			"CurrantBluePoly", "CrystalBluePoly", "TempleCurtainPurple", "CherryRed", "SecuricorDarkGreen2", "TaxiYellow2",
			"PoliceCarBlue", "MellowBurgundy", "DesertTaupePoly", "LammyOrange", "LammyYellow", "VeryWhite"
        };

        public static ColorIndex Black { get { return new ColorIndex(0); } }
        public static ColorIndex BlackPoly { get { return new ColorIndex(1); } }
        public static ColorIndex ConcordBluePoly { get { return new ColorIndex(2); } }
        public static ColorIndex PewterGrayPoly { get { return new ColorIndex(3); } }
        public static ColorIndex SilverStonePoly { get { return new ColorIndex(4); } }
        public static ColorIndex WinningSilverPoly { get { return new ColorIndex(5); } }
        public static ColorIndex SteelGrayPoly { get { return new ColorIndex(6); } }
        public static ColorIndex ShadowSilverPoly { get { return new ColorIndex(7); } }
        public static ColorIndex SilverStonePoly2 { get { return new ColorIndex(8); } }
        public static ColorIndex PorcelainSilverPoly { get { return new ColorIndex(9); } }
        public static ColorIndex GrayPoly { get { return new ColorIndex(10); } }
        public static ColorIndex AnthraciteGrayPoly { get { return new ColorIndex(11); } }
        public static ColorIndex AstraSilverPoly { get { return new ColorIndex(12); } }
        public static ColorIndex AscotGray { get { return new ColorIndex(13); } }
        public static ColorIndex ClearCrystalBlueFrostPoly { get { return new ColorIndex(14); } }
        public static ColorIndex SilverPoly { get { return new ColorIndex(15); } }
        public static ColorIndex DarkTitaniumPoly { get { return new ColorIndex(16); } }
        public static ColorIndex TitaniumFrostPoly { get { return new ColorIndex(17); } }
        public static ColorIndex PoliceWhite { get { return new ColorIndex(18); } }
        public static ColorIndex MediumGrayPoly { get { return new ColorIndex(19); } }
        public static ColorIndex MediumGrayPoly2 { get { return new ColorIndex(20); } }
        public static ColorIndex SteelGrayPoly2 { get { return new ColorIndex(21); } }
        public static ColorIndex SlateGray { get { return new ColorIndex(22); } }
        public static ColorIndex GunMetalPoly { get { return new ColorIndex(23); } }
        public static ColorIndex LightBlueGrey { get { return new ColorIndex(24); } }
        public static ColorIndex SecuricorLightGray { get { return new ColorIndex(25); } }
        public static ColorIndex ArcticWhite { get { return new ColorIndex(26); } }
        public static ColorIndex VeryRed { get { return new ColorIndex(27); } }
        public static ColorIndex TorinoRedPearl { get { return new ColorIndex(28); } }
        public static ColorIndex FormulaRed { get { return new ColorIndex(29); } }
        public static ColorIndex BlazeRed { get { return new ColorIndex(30); } }
        public static ColorIndex GracefulRedMica { get { return new ColorIndex(31); } }
        public static ColorIndex GarnetRedPoly { get { return new ColorIndex(32); } }
        public static ColorIndex DesertRed { get { return new ColorIndex(33); } }
        public static ColorIndex CabernetRedPoly { get { return new ColorIndex(34); } }
        public static ColorIndex TurismoRed { get { return new ColorIndex(35); } }
        public static ColorIndex DesertRed2 { get { return new ColorIndex(36); } }
        public static ColorIndex CurrantRedSolid { get { return new ColorIndex(37); } }
        public static ColorIndex CurrantRedPoly { get { return new ColorIndex(38); } }
        public static ColorIndex ElectricCurrantRedPoly { get { return new ColorIndex(39); } }
        public static ColorIndex MediumCabernetSolid { get { return new ColorIndex(40); } }
        public static ColorIndex WildStrawberryPoly { get { return new ColorIndex(41); } }
        public static ColorIndex MediumRedSolid { get { return new ColorIndex(42); } }
        public static ColorIndex BrightRed { get { return new ColorIndex(43); } }
        public static ColorIndex BrightRed2 { get { return new ColorIndex(44); } }
        public static ColorIndex MediumGarnetRedPoly { get { return new ColorIndex(45); } }
        public static ColorIndex BrilliantRedPoly { get { return new ColorIndex(46); } }
        public static ColorIndex BrilliantRedPoly2 { get { return new ColorIndex(47); } }
        public static ColorIndex AlabasterSolid { get { return new ColorIndex(48); } }
        public static ColorIndex TwilightBluePoly { get { return new ColorIndex(49); } }
        public static ColorIndex TorchRed { get { return new ColorIndex(50); } }
        public static ColorIndex Green { get { return new ColorIndex(51); } }
        public static ColorIndex DeepJewelGreen { get { return new ColorIndex(52); } }
        public static ColorIndex AgateGreen { get { return new ColorIndex(53); } }
        public static ColorIndex PetrolBlueGreenPoly { get { return new ColorIndex(54); } }
        public static ColorIndex Hoods { get { return new ColorIndex(55); } }
        public static ColorIndex Green2 { get { return new ColorIndex(56); } }
        public static ColorIndex DarkGreenPoly { get { return new ColorIndex(57); } }
        public static ColorIndex RioRed { get { return new ColorIndex(58); } }
        public static ColorIndex SecuricorDarkGreen { get { return new ColorIndex(59); } }
        public static ColorIndex SeafoamPoly { get { return new ColorIndex(60); } }
        public static ColorIndex PastelAlabasterSolid { get { return new ColorIndex(61); } }
        public static ColorIndex MidnightBlue { get { return new ColorIndex(62); } }
        public static ColorIndex StrikingBlue { get { return new ColorIndex(63); } }
        public static ColorIndex SaxonyBluePoly { get { return new ColorIndex(64); } }
        public static ColorIndex JasperGreenPoly { get { return new ColorIndex(65); } }
        public static ColorIndex MarinerBlue { get { return new ColorIndex(66); } }
        public static ColorIndex HarborBluePoly { get { return new ColorIndex(67); } }
        public static ColorIndex DiamondBluePoly { get { return new ColorIndex(68); } }
        public static ColorIndex SurfBlue { get { return new ColorIndex(69); } }
        public static ColorIndex NauticalBluePoly { get { return new ColorIndex(70); } }
        public static ColorIndex LightCrystalBluePoly { get { return new ColorIndex(71); } }
        public static ColorIndex MedRegattaBluePoly { get { return new ColorIndex(72); } }
        public static ColorIndex SpinnakerBlueSolid { get { return new ColorIndex(73); } }
        public static ColorIndex UltraBluePoly { get { return new ColorIndex(74); } }
        public static ColorIndex BrightBluePoly { get { return new ColorIndex(75); } }
        public static ColorIndex NassauBluePoly { get { return new ColorIndex(76); } }
        public static ColorIndex MediumSapphireBluePoly { get { return new ColorIndex(77); } }
        public static ColorIndex SteelBluePoly { get { return new ColorIndex(78); } }
        public static ColorIndex LightSapphireBluePoly { get { return new ColorIndex(79); } }
        public static ColorIndex MalachitePoly { get { return new ColorIndex(80); } }
        public static ColorIndex MediumMauiBluePoly { get { return new ColorIndex(81); } }
        public static ColorIndex BrightBluePoly2 { get { return new ColorIndex(82); } }
        public static ColorIndex BrightBluePoly3 { get { return new ColorIndex(83); } }
        public static ColorIndex Blue { get { return new ColorIndex(84); } }
        public static ColorIndex DarkSapphireBluePoly { get { return new ColorIndex(85); } }
        public static ColorIndex LightSapphireBluePoly2 { get { return new ColorIndex(86); } }
        public static ColorIndex MediumSapphireBlueFiremist { get { return new ColorIndex(87); } }
        public static ColorIndex TwilightBluePoly2 { get { return new ColorIndex(88); } }
        public static ColorIndex TaxiYellow { get { return new ColorIndex(89); } }
        public static ColorIndex RaceYellowSolid { get { return new ColorIndex(90); } }
        public static ColorIndex PastelAlabaster { get { return new ColorIndex(91); } }
        public static ColorIndex OxfordWhiteSolid { get { return new ColorIndex(92); } }
        public static ColorIndex Flax { get { return new ColorIndex(93); } }
        public static ColorIndex MediumFlax { get { return new ColorIndex(94); } }
        public static ColorIndex PuebloBeige { get { return new ColorIndex(95); } }
        public static ColorIndex LightIvory { get { return new ColorIndex(96); } }
        public static ColorIndex SmokeSilverPoly { get { return new ColorIndex(97); } }
        public static ColorIndex BisqueFrostPoly { get { return new ColorIndex(98); } }
        public static ColorIndex ClassicRed { get { return new ColorIndex(99); } }
        public static ColorIndex VermilionSolid { get { return new ColorIndex(100); } }
        public static ColorIndex VermillionSolid { get { return new ColorIndex(101); } }
        public static ColorIndex BistonBrownPoly { get { return new ColorIndex(102); } }
        public static ColorIndex LightBeechwoodPoly { get { return new ColorIndex(103); } }
        public static ColorIndex DarkBeechwoodPoly { get { return new ColorIndex(104); } }
        public static ColorIndex DarkSablePoly { get { return new ColorIndex(105); } }
        public static ColorIndex MediumBeechwoodPoly { get { return new ColorIndex(106); } }
        public static ColorIndex WoodrosePoly { get { return new ColorIndex(107); } }
        public static ColorIndex SandalwoodFrostPoly { get { return new ColorIndex(108); } }
        public static ColorIndex MediumSandalwoodPoly { get { return new ColorIndex(109); } }
        public static ColorIndex CopperBeige { get { return new ColorIndex(110); } }
        public static ColorIndex WarmGreyMica { get { return new ColorIndex(111); } }
        public static ColorIndex White { get { return new ColorIndex(112); } }
        public static ColorIndex FrostWhite { get { return new ColorIndex(113); } }
        public static ColorIndex HoneyBeigePoly { get { return new ColorIndex(114); } }
        public static ColorIndex SeafoamPoly2 { get { return new ColorIndex(115); } }
        public static ColorIndex LightTitaniumPoly { get { return new ColorIndex(116); } }
        public static ColorIndex LightChampagnePoly { get { return new ColorIndex(117); } }
        public static ColorIndex ArcticPearl { get { return new ColorIndex(118); } }
        public static ColorIndex LightDriftwoodPoly { get { return new ColorIndex(119); } }
        public static ColorIndex WhiteDiamondPearl { get { return new ColorIndex(120); } }
        public static ColorIndex AntelopeBeige { get { return new ColorIndex(121); } }
        public static ColorIndex CurrantBluePoly { get { return new ColorIndex(122); } }
        public static ColorIndex CrystalBluePoly { get { return new ColorIndex(123); } }
        public static ColorIndex TempleCurtainPurple { get { return new ColorIndex(124); } }
        public static ColorIndex CherryRed { get { return new ColorIndex(125); } }
        public static ColorIndex SecuricorDarkGreen2 { get { return new ColorIndex(126); } }
        public static ColorIndex TaxiYellow2 { get { return new ColorIndex(127); } }
        public static ColorIndex PoliceCarBlue { get { return new ColorIndex(128); } }
        public static ColorIndex MellowBurgundy { get { return new ColorIndex(129); } }
        public static ColorIndex DesertTaupePoly { get { return new ColorIndex(130); } }
        public static ColorIndex LammyOrange { get { return new ColorIndex(131); } }
        public static ColorIndex LammyYellow { get { return new ColorIndex(132); } }
        public static ColorIndex VeryWhite { get { return new ColorIndex(133); } }

        private int m_index;

        public int Index
        {
            get
            {
                return m_index;
            }
        }

        public string Name
        {
            get
            {
                if (ColorNames.Length - 1 < m_index)
                    return "";
                else
                    return ColorNames[m_index];
            }
        }

        public ColorIndex(int index)
        {
            m_index = index;
        }

        public override string ToString()
        {
            return Name;
        }
    }
}
