-- Lua design suggestion
-- Suggestion from NTAuthority from #fourdeltaone

--[[
Remarks about this design:
- Based on Lua
- Drops namespaces completely to allow direct natives access
- All natives are expected to have respectively friendlier names
- Native names are pascal-case
- Custom native names don't have _ prefix but would need to maintain name history for compatibility
- This design aims to allow for friendly wrappers which open up easier possibilities for usage in (for example) object-oriented programming languages
	o includes "property" and "method" definitions that directly call natives
- type will be redefined for the runtime of this script
]]

-- Type definitions
-- !! THIS IS NOT IMPLEMENTED YET AND BELOW CODE IS JUST AN EXAMPLE OF HOW IT COULD LOOK LIKE. NOTHING OF THIS SECTION IS GENERATED. !!

--[[!
<summary>
	Represents an entity in the game.
</summary>
<example>
	// Just some example code here...
</example>
<remarks>blahblah</remarks>
]]
type "Entity"
	nativeType "int"

--[[!
<summary>
	Represents a vehicle.
</summary>
]]
type "Vehicle"
	extends "Entity"

--[[!
<summary>
	Represents a player.
</summary>
]]
type "Player"
	extends "Entity"

	property "position" { "GET_ENTITY_COORDS" }
	method "getPosition" { "GET_ENTITY_COORDS" }

-- Native definitions

--[[!
<summary>
	Returns ped for a player
</summary>
]]--
native "GET_PLAYER_PED"
	hash "0x43A66C31C68491C0"
	jhash (0x6E31E993)
	arguments {
		float "player",
	}
	returns	"Ped"

native "GET_PLAYER_PED_SCRIPT_INDEX"
	hash "0x50FAC3A3E030A6E1"
	jhash (0x6AC64990)
	arguments {
		Player "player",
	}
	returns	"Ped"

native "SET_PLAYER_MODEL"
	hash "0x00A1CADD00108836"
	jhash (0x774A4C54)
	arguments {
		Player "player",

		Hash "model",
	}
	returns	"void"

native "CHANGE_PLAYER_PED"
	hash "0x048189FAC643DEEE"
	jhash (0xBE515485)
	arguments {
		Player "player",

		Ped "ped",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "GET_PLAYER_RGB_COLOUR"
	hash "0xE902EF951DCE178F"
	jhash (0x6EF43BBB)
	arguments {
		Player "player",

		intPtr "r",

		intPtr "g",

		intPtr "b",
	}
	returns	"void"

native "GET_NUMBER_OF_PLAYERS"
	hash "0x407C7F91DDB46C16"
	jhash (0x4C1B8867)
	returns	"int"

native "GET_PLAYER_TEAM"
	hash "0x37039302F4E0A008"
	jhash (0x9873E404)
	arguments {
		Player "player",
	}
	returns	"int"

--[[!
<summary>
	Set player team on deathmatch and last team standing..

	teams +1 +2 +3
</summary>
]]--
native "SET_PLAYER_TEAM"
	hash "0x0299FA38396A4940"
	jhash (0x725ADCF2)
	arguments {
		Player "player",

		int "team",
	}
	returns	"void"

native "GET_PLAYER_NAME"
	hash "0x6D0DE6A7B5DA71F8"
	jhash (0x406B4B20)
	arguments {
		Player "player",
	}
	returns	"charPtr"

native "GET_WANTED_LEVEL_RADIUS"
	hash "0x085DEB493BE80812"
	jhash (0x1CF7D7DA)
	arguments {
		Player "player",
	}
	returns	"float"

native "GET_PLAYER_WANTED_CENTRE_POSITION"
	hash "0x0C92BA89F1AF26F8"
	jhash (0x821F2D2C)
	arguments {
		Player "player",
	}
	returns	"Vector3"

--[[!
<summary>
	# Predominant call signatures
	PLAYER::SET_PLAYER_WANTED_CENTRE_POSITION(PLAYER::PLAYER_ID(), ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1));

	# Parameter value ranges
	P0: PLAYER::PLAYER_ID()
	P1: ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 1)
	P2: Not set by any call
</summary>
]]--
native "SET_PLAYER_WANTED_CENTRE_POSITION"
	hash "0x520E541A97A13354"
	jhash (0xF261633A)
	arguments {
		Player "player",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "GET_WANTED_LEVEL_THRESHOLD"
	hash "0xFDD179EAF45B556C"
	jhash (0xD9783F6B)
	arguments {
		int "wantedLevel",
	}
	returns	"int"

--[[!
<summary>
	Call SET_PLAYER_WANTED_LEVEL_NOW for immediate effect

	p2 appears to always be false
</summary>
]]--
native "SET_PLAYER_WANTED_LEVEL"
	hash "0x39FF19C64EF7DA5B"
	jhash (0xB7A0914B)
	arguments {
		Player "player",

		int "wantedLevel",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	p2 is always false in R* scripts
</summary>
]]--
native "SET_PLAYER_WANTED_LEVEL_NO_DROP"
	hash "0x340E61DE7F471565"
	jhash (0xED6F44F5)
	arguments {
		Player "player",

		int "wantedLevel",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	Forces any pending wanted level to be applied to the specified player immediately.

	Call SET_PLAYER_WANTED_LEVEL with the desired wanted level, followed by SET_PLAYER_WANTED_LEVEL_NOW.

	Second parameter is unknown (always false).
</summary>
]]--
native "SET_PLAYER_WANTED_LEVEL_NOW"
	hash "0xE0A7D1E497FFCD6F"
	jhash (0xAF3AFD83)
	arguments {
		Player "player",

		BOOL "p1",
	}
	returns	"void"

native "ARE_PLAYER_FLASHING_STARS_ABOUT_TO_DROP"
	hash "0xAFAF86043E5874E9"
	jhash (0xE13A71C7)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "ARE_PLAYER_STARS_GREYED_OUT"
	hash "0x0A6EB355EE14A2DB"
	jhash (0x5E72AB72)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "SET_DISPATCH_COPS_FOR_PLAYER"
	hash "0xDB172424876553F4"
	jhash (0x48A18913)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

native "IS_PLAYER_WANTED_LEVEL_GREATER"
	hash "0x238DB2A2C23EE9EF"
	jhash (0x589A2661)
	arguments {
		Player "player",

		int "wantedLevel",
	}
	returns	"BOOL"

native "CLEAR_PLAYER_WANTED_LEVEL"
	hash "0xB302540597885499"
	jhash (0x54EA5BCC)
	arguments {
		Player "player",
	}
	returns	"void"

native "IS_PLAYER_DEAD"
	hash "0x424D4687FA1E5652"
	jhash (0x140CA5A8)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "IS_PLAYER_PRESSING_HORN"
	hash "0xFA1E2BF8B10598F9"
	jhash (0xED1D1662)
	arguments {
		Player "player",
	}
	returns	"BOOL"

--[[!
<summary>
	IS_PLAYER_PRESSING_TOGGLE

</summary>
]]--
native "SET_PLAYER_CONTROL"
	hash "0x8D32347D6D4C40A2"
	jhash (0xD17AFCD8)
	arguments {
		Player "player",

		BOOL "toggle",

		int "possiblyFlags",
	}
	returns	"void"

native "GET_PLAYER_WANTED_LEVEL"
	hash "0xE28E54788CE8F12D"
	jhash (0xBDCDD163)
	arguments {
		Player "player",
	}
	returns	"int"

native "SET_MAX_WANTED_LEVEL"
	hash "0xAA5F02DB48D704B9"
	jhash (0x665A06F5)
	arguments {
		int "maxWantedLevel",
	}
	returns	"void"

--[[!
<summary>
	H
	B
	H
	H
	O
	H
	M
	N
</summary>
]]--
native "SET_POLICE_RADAR_BLIPS"
	hash "0x43286D561B72B8BF"
	jhash (0x8E114B10)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "SET_POLICE_IGNORE_PLAYER"
	hash "0x32C62AA929C2DA6A"
	jhash (0xE6DE71B7)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Checks if player is wasted or busted
</summary>
]]--
native "IS_PLAYER_PLAYING"
	hash "0x5E9564D8246B909A"
	jhash (0xE15D777F)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "SET_EVERYONE_IGNORE_PLAYER"
	hash "0x8EEDA153AD141BA4"
	jhash (0xC915285E)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ALL_RANDOM_PEDS_FLEE"
	hash "0x056E0FE8534C2949"
	jhash (0x49EAE968)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ALL_RANDOM_PEDS_FLEE_THIS_FRAME"
	hash "0x471D2FF42A94B4F2"
	jhash (0xBF974891)
	arguments {
		Player "player",
	}
	returns	"void"

--[[!
<summary>
	hash collision?
</summary>
]]--
native "SET_HUD_ANIM_STOP_LEVEL"
	hash "0xDE45D1A1EF45EE61"
	jhash (0x274631FE)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	- Changed Any p0 to Player player based on use within c4 scripts
</summary>
]]--
native "SET_AREAS_GENERATOR_ORIENTATION"
	hash "0xC3376F42B1FACCC6"
	jhash (0x02DF7AF4)
	arguments {
		Player "player",
	}
	returns	"void"

native "SET_IGNORE_LOW_PRIORITY_SHOCKING_EVENTS"
	hash "0x596976B02B6B5700"
	jhash (0xA3D675ED)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

native "SET_WANTED_LEVEL_MULTIPLIER"
	hash "0x020E5F00CDA207BA"
	jhash (0x1359292F)
	arguments {
		float "multiplier",
	}
	returns	"void"

native "SET_WANTED_LEVEL_DIFFICULTY"
	hash "0x9B0BB33B04405E7A"
	jhash (0xB552626C)
	arguments {
		Player "player",

		float "difficulty",
	}
	returns	"void"

native "RESET_WANTED_LEVEL_DIFFICULTY"
	hash "0xB9D0DD990DC141DD"
	jhash (0xA64C378D)
	arguments {
		Player "player",
	}
	returns	"void"

native "START_FIRING_AMNESTY"
	hash "0xBF9BD71691857E48"
	jhash (0x5F8A22A6)
	arguments {
		int "duration",
	}
	returns	"void"

--[[!
<summary>
	PLAYER::REPORT_CRIME(PLAYER::PLAYER_ID(), 37, PLAYER::GET_WANTED_LEVEL_THRESHOLD(1));
	This was found in the decompiled files, but I can't figure out what '37' stands for, or what the threshold is used for. Any help?

	From am_armybase.ysc.c4:

	PLAYER::REPORT_CRIME(PLAYER::PLAYER_ID(4), 36, PLAYER::GET_WANTED_LEVEL_THRESHOLD(4));

	I believe that '36' in this case is an ID. The second parameter seems to be the wanted level. I'll leave the 2nd parameter as 'p0' as I can't really confirm it.

	Thought I'd chime in here on this riveting conversation that apparently I'm the only fucking person on Earth that knows how to use IDA.
	  switch ( crimeType )
	  {
	    case 37:
	      v8 = v4 * 55.0;
	      goto LABEL_38;
	  }

	This was taken from the GTAV.exe v1.334. The function is called sub_140592CE8. For a full decompilation of the function, see here: http://pastebin.com/09qSMsN7 
</summary>
]]--
native "REPORT_CRIME"
	hash "0xE9B09589827545E7"
	jhash (0xD8EB3A44)
	arguments {
		Player "player",

		int "crimeType",

		int "wantedLvlThresh",
	}
	returns	"void"

--[[!
<summary>
	Seems to only appear in scripts used in Singleplayer. p1 ranges from 2 - 46.
</summary>
]]--
native "0x9A987297ED8BD838"
	hash "0x9A987297ED8BD838"
	jhash (0x59B5C2A2)
	arguments {
		Player "player",

		int "p1",
	}
	returns	"void"

--[[!
<summary>
	Seems to only appear in scripts used in Singleplayer.
</summary>
]]--
native "0xBC9490CA15AEA8FB"
	hash "0xBC9490CA15AEA8FB"
	jhash (0x6B34A160)
	arguments {
		Player "player",
	}
	returns	"void"

--[[!
<summary>
	This has been found in use in the decompiled files.
</summary>
]]--
native "0x4669B3ED80F24B4E"
	hash "0x4669B3ED80F24B4E"
	jhash (0xB9FB142F)
	arguments {
		Player "player",
	}
	returns	"Any"

--[[!
<summary>
	This has been found in use in the decompiled files.
</summary>
]]--
native "0xAD73CE5A09E42D12"
	hash "0xAD73CE5A09E42D12"
	jhash (0x85725848)
	arguments {
		Player "player",
	}
	returns	"Any"

native "0x36F1B38855F2A8DF"
	hash "0x36F1B38855F2A8DF"
	jhash (0x3A7E5FB6)
	arguments {
		Player "player",
	}
	returns	"void"

--[[!
<summary>
	This has been found in use in the decompiled files.
</summary>
]]--
native "0xDC64D2C53493ED12"
	hash "0xDC64D2C53493ED12"
	jhash (0xD15C4B1C)
	arguments {
		Player "player",
	}
	returns	"Any"

--[[!
<summary>
	PLAYER::0xBF6993C7(rPtr((&amp;l_122) + 71)); // Found in decompilation

	***

	In "am_hold_up.ysc.c4" used once:

	l_8d._f47 = GAMEPLAY::GET_RANDOM_FLOAT_IN_RANGE(18.0, 28.0);
	PLAYER::_B45EFF719D8427A6((l_8d._f47));
</summary>
]]--
native "0xB45EFF719D8427A6"
	hash "0xB45EFF719D8427A6"
	jhash (0xBF6993C7)
	arguments {
		float "p0",
	}
	returns	"void"

--[[!
<summary>
	2 matches in 1 script 
</summary>
]]--
native "0x0032A6DBA562C518"
	hash "0x0032A6DBA562C518"
	jhash (0x47CAB814)
	returns	"void"

native "CAN_PLAYER_START_MISSION"
	hash "0xDE7465A27D403C06"
	jhash (0x39E3CB3F)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "IS_PLAYER_READY_FOR_CUTSCENE"
	hash "0x908CBECC2CAA3690"
	jhash (0xBB77E9CD)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "IS_PLAYER_TARGETTING_ENTITY"
	hash "0x7912F7FC4F6264B6"
	jhash (0xF3240B77)
	arguments {
		Player "player",

		Entity "entity",
	}
	returns	"BOOL"

--[[!
<summary>
	Assigns the handle of locked-on melee target to *entity that you pass it; returns false if no entity found. Player must be PLAYER::PLAYER_ID and not PLAYER::PLAYER_PED_ID
</summary>
]]--
native "GET_PLAYER_TARGET_ENTITY"
	hash "0x13EDE1A5DBF797C9"
	jhash (0xF6AAA2D7)
	arguments {
		Player "player",

		EntityPtr "entity",
	}
	returns	"BOOL"

--[[!
<summary>
	Gets a value indicating whether the specified player is currently aiming freely.
</summary>
]]--
native "IS_PLAYER_FREE_AIMING"
	hash "0x2E397FD2ECD37C87"
	jhash (0x1DEC67B7)
	arguments {
		Player "player",
	}
	returns	"BOOL"

--[[!
<summary>
	Gets a value indicating whether the specified player is currently aiming freely at the specified entity.
</summary>
]]--
native "IS_PLAYER_FREE_AIMING_AT_ENTITY"
	hash "0x3C06B5C839B38F7B"
	jhash (0x7D80EEAA)
	arguments {
		Player "character",

		Entity "aimedEntity",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns TRUE if it found an entity in your crosshair within range of your weapon. Assigns the handle of the target to the *entity that you pass it; returns false if no entity found. Player must be PLAYER::PLAYER_ID() and not PLAYER::PLAYER_PED_ID()

	Also known as _GET_AIMED_ENTITY.
</summary>
]]--
native "GET_ENTITY_PLAYER_IS_FREE_AIMING_AT"
	hash "0x2975C866E6713290"
	jhash (0x8866D9D0)
	arguments {
		Player "player",

		EntityPtr "entity",
	}
	returns	"BOOL"

--[[!
<summary>
	Affects the range of auto aim target.
</summary>
]]--
native "SET_PLAYER_LOCKON_RANGE_OVERRIDE"
	hash "0x29961D490E5814FD"
	jhash (0x74D42C03)
	arguments {
		Player "player",

		float "range",
	}
	returns	"void"

--[[!
<summary>
	Set whether this player should be able to do drive-bys.

	// Needs clarification, what is technically a "drive-by" in game code?
</summary>
]]--
native "SET_PLAYER_CAN_DO_DRIVE_BY"
	hash "0x6E8834B52EC20C77"
	jhash (0xF4D99685)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Sets whether this player can be hassled by gangs.
</summary>
]]--
native "SET_PLAYER_CAN_BE_HASSLED_BY_GANGS"
	hash "0xD5E460AD7020A246"
	jhash (0x71B305BB)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Sets whether this player can take cover.
</summary>
]]--
native "SET_PLAYER_CAN_USE_COVER"
	hash "0xD465A8599DFF6814"
	jhash (0x13CAFAFA)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"Any"

--[[!
<summary>
	Gets the maximum wanted level the player can get.
	Ranges from 0 to 5.
</summary>
]]--
native "GET_MAX_WANTED_LEVEL"
	hash "0x462E0DB9B137DC5F"
	jhash (0x457F1E44)
	returns	"int"

native "IS_PLAYER_TARGETTING_ANYTHING"
	hash "0x78CFE51896B6B8A4"
	jhash (0x456DB50D)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "SET_PLAYER_SPRINT"
	hash "0xA01B8075D8B92DF4"
	jhash (0x7DD7900C)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

native "RESET_PLAYER_STAMINA"
	hash "0xA6F312FCCE9C1DFE"
	jhash (0xC0445A9C)
	arguments {
		Player "player",
	}
	returns	"void"

native "RESTORE_PLAYER_STAMINA"
	hash "0xA352C1B864CAFD33"
	jhash (0x62A93608)
	arguments {
		Player "player",

		float "p1",
	}
	returns	"void"

native "GET_PLAYER_SPRINT_STAMINA_REMAINING"
	hash "0x3F9F16F8E65A7ED7"
	jhash (0x47017C90)
	arguments {
		Player "player",
	}
	returns	"float"

native "GET_PLAYER_SPRINT_TIME_REMAINING"
	hash "0x1885BC9B108B4C99"
	jhash (0x40E80543)
	arguments {
		Player "player",
	}
	returns	"float"

native "GET_PLAYER_UNDERWATER_TIME_REMAINING"
	hash "0xA1FCF8E6AF40B731"
	jhash (0x1317125A)
	arguments {
		Player "player",
	}
	returns	"float"

--[[!
<summary>
	Returns the group ID the player is member of.
</summary>
]]--
native "GET_PLAYER_GROUP"
	hash "0x0D127585F77030AF"
	jhash (0xA5EDCDE8)
	arguments {
		Player "player",
	}
	returns	"int"

native "GET_PLAYER_MAX_ARMOUR"
	hash "0x92659B4CE1863CB3"
	jhash (0x02A50657)
	arguments {
		Player "player",
	}
	returns	"int"

--[[!
<summary>
	Can the player control himself, used to disable controls for player for things like a cutscene
</summary>
]]--
native "IS_PLAYER_CONTROL_ON"
	hash "0x49C32D60007AFA47"
	jhash (0x618857F2)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "0x7C814D2FB49F40C0"
	hash "0x7C814D2FB49F40C0"
	returns	"Any"

native "IS_PLAYER_SCRIPT_CONTROL_ON"
	hash "0x8A876A65283DD7D7"
	jhash (0x61B00A84)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "IS_PLAYER_CLIMBING"
	hash "0x95E8F73DC65EFB9C"
	jhash (0x4A9E9AE0)
	arguments {
		Player "player",
	}
	returns	"BOOL"

--[[!
<summary>
	Return true while player is being arrested / busted.

	If atArresting is set to 1, this function will return 1 when player is being arrested (while player is putting his hand up, but still have control)

	If atArresting is set to 0, this function will return 1 only when the busted screen is shown.
</summary>
]]--
native "IS_PLAYER_BEING_ARRESTED"
	hash "0x388A47C51ABDAC8E"
	jhash (0x7F6A60D3)
	arguments {
		Player "player",

		BOOL "atArresting",
	}
	returns	"BOOL"

native "RESET_PLAYER_ARREST_STATE"
	hash "0x2D03E13C460760D6"
	jhash (0x453C7CAB)
	arguments {
		Player "player",
	}
	returns	"void"

native "GET_PLAYERS_LAST_VEHICLE"
	hash "0xB6997A7EB3F5C8C0"
	jhash (0xE2757AC1)
	returns	"Vehicle"

--[[!
<summary>
	Returns the same as PLAYER_ID
</summary>
]]--
native "GET_PLAYER_INDEX"
	hash "0xA5EDC40EF369B48D"
	jhash (0x309BBDC1)
	returns	"Player"

native "INT_TO_PLAYERINDEX"
	hash "0x41BD2A6B006AF756"
	jhash (0x98DD98F1)
	arguments {
		int "value",
	}
	returns	"Player"

--[[!
<summary>
	if (NETWORK::NETWORK_IS_PARTICIPANT_ACTIVE(PLAYER::INT_TO_PARTICIPANTINDEX(i)))

</summary>
]]--
native "INT_TO_PARTICIPANTINDEX"
	hash "0x9EC6603812C24710"
	jhash (0x98F3B274)
	arguments {
		int "value",
	}
	returns	"int"

native "GET_TIME_SINCE_PLAYER_HIT_VEHICLE"
	hash "0x5D35ECF3A81A0EE0"
	jhash (0x6E9B8B9E)
	arguments {
		Player "player",
	}
	returns	"int"

native "GET_TIME_SINCE_PLAYER_HIT_PED"
	hash "0xE36A25322DC35F42"
	jhash (0xB6209195)
	arguments {
		Player "player",
	}
	returns	"int"

native "GET_TIME_SINCE_PLAYER_DROVE_ON_PAVEMENT"
	hash "0xD559D2BE9E37853B"
	jhash (0x8836E732)
	arguments {
		Player "player",
	}
	returns	"int"

native "GET_TIME_SINCE_PLAYER_DROVE_AGAINST_TRAFFIC"
	hash "0xDB89591E290D9182"
	jhash (0x9F27D00E)
	arguments {
		Player "player",
	}
	returns	"int"

native "IS_PLAYER_FREE_FOR_AMBIENT_TASK"
	hash "0xDCCFD3F106C36AB4"
	jhash (0x85C7E232)
	arguments {
		Player "player",
	}
	returns	"BOOL"

--[[!
<summary>
	The players 'identity' as a Player type
</summary>
]]--
native "PLAYER_ID"
	hash "0x4F8644AF03D0E0D6"
	jhash (0x8AEA886C)
	returns	"Player"

--[[!
<summary>
	returns the playing Ped
</summary>
]]--
native "PLAYER_PED_ID"
	hash "0xD80958FC74E988A6"
	jhash (0xFA92E226)
	returns	"Ped"

--[[!
<summary>
	Does exactly the same thing as player_id
</summary>
]]--
native "NETWORK_PLAYER_ID_TO_INT"
	hash "0xEE68096F9F37341E"
	jhash (0x8DD5B838)
	returns	"Player"

native "HAS_FORCE_CLEANUP_OCCURRED"
	hash "0xC968670BFACE42D9"
	jhash (0x4B37333C)
	arguments {
		int "cleanupFlags",
	}
	returns	"BOOL"

--[[!
<summary>
	used with 1,2,8,64,128 in the scripts
</summary>
]]--
native "FORCE_CLEANUP"
	hash "0xBC8983F38F78ED51"
	jhash (0xFDAAEA2B)
	arguments {
		int "cleanupType",
	}
	returns	"void"

--[[!
<summary>
	PLAYER::FORCE_CLEANUP_FOR_ALL_THREADS_WITH_THIS_NAME("pb_prostitute", 1); // Found in decompilation
</summary>
]]--
native "FORCE_CLEANUP_FOR_ALL_THREADS_WITH_THIS_NAME"
	hash "0x4C68DDDDF0097317"
	jhash (0x04256C73)
	arguments {
		charPtr "name",

		int "cleanupFlags",
	}
	returns	"void"

native "FORCE_CLEANUP_FOR_THREAD_WITH_THIS_ID"
	hash "0xF745B37630DF176B"
	jhash (0x882D3EB3)
	arguments {
		int "id",

		int "cleanupFlags",
	}
	returns	"void"

native "GET_CAUSE_OF_MOST_RECENT_FORCE_CLEANUP"
	hash "0x9A41CF4674A12272"
	jhash (0x39AA9FC8)
	returns	"int"

native "SET_PLAYER_MAY_ONLY_ENTER_THIS_VEHICLE"
	hash "0x8026FF78F208978A"
	jhash (0xA454DD29)
	arguments {
		Player "player",

		Vehicle "vehicle",
	}
	returns	"void"

native "SET_PLAYER_MAY_NOT_ENTER_ANY_VEHICLE"
	hash "0x1DE37BBF9E9CC14A"
	jhash (0xAF7AFCC4)
	arguments {
		Player "player",
	}
	returns	"void"

--[[!
<summary>
	Achievements from 0-57
	-unknown

	more achievements came with update 1.29 (freemode events update), I'd say that they now go to 60, but I'll need to check.
</summary>
]]--
native "GIVE_ACHIEVEMENT_TO_PLAYER"
	hash "0xBEC7076D64130195"
	jhash (0x822BC992)
	arguments {
		int "achievement",
	}
	returns	"BOOL"

--[[!
<summary>
	This seems to be related to Steam achievements.
</summary>
]]--
native "0xC2AFFFDABBDC2C5C"
	hash "0xC2AFFFDABBDC2C5C"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x1C186837D0619335"
	hash "0x1C186837D0619335"
	arguments {
		Any "p0",
	}
	returns	"Cam"

native "HAS_ACHIEVEMENT_BEEN_PASSED"
	hash "0x867365E111A3B6EB"
	jhash (0x136A5BE9)
	arguments {
		int "achievement",
	}
	returns	"BOOL"

--[[!
<summary>
	This is an alias for NETWORK_IS_SIGNED_ONLINE.
</summary>
]]--
native "IS_PLAYER_ONLINE"
	hash "0xF25D331DC2627BBC"
	jhash (0x9FAB6729)
	returns	"Player"

native "IS_PLAYER_LOGGING_IN_NP"
	hash "0x74556E1420867ECA"
	jhash (0x8F72FAD0)
	returns	"BOOL"

native "DISPLAY_SYSTEM_SIGNIN_UI"
	hash "0x94DD7888C10A979E"
	jhash (0x4264CED2)
	arguments {
		ScrHandle "scrHandle",
	}
	returns	"void"

native "IS_SYSTEM_UI_BEING_DISPLAYED"
	hash "0x5D511E3867C87139"
	jhash (0xE495B6DA)
	returns	"BOOL"

--[[!
<summary>
	Simply sets you as invincible (Health will not deplete).
</summary>
]]--
native "SET_PLAYER_INVINCIBLE"
	hash "0x239528EACDC3E7DE"
	jhash (0xDFB9A2A2)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Returns the Player's Invincible status.
</summary>
]]--
native "GET_PLAYER_INVINCIBLE"
	hash "0xB721981B2B939E07"
	jhash (0x680C90EE)
	arguments {
		Player "player",
	}
	returns	"BOOL"

--[[!
<summary>
	p1 apears to always be one

	Also, this native is only called in the fm_(something)_controller files
</summary>
]]--
native "0xCAC57395B151135F"
	hash "0xCAC57395B151135F"
	jhash (0x00563E0D)
	arguments {
		Player "player",

		Any "p1",
	}
	returns	"void"

native "REMOVE_PLAYER_HELMET"
	hash "0xF3AC26D3CC576528"
	jhash (0x6255F3B4)
	arguments {
		Player "player",

		BOOL "p2",
	}
	returns	"Any"

native "GIVE_PLAYER_RAGDOLL_CONTROL"
	hash "0x3C49C870E66F0A28"
	jhash (0xC7B4D7AC)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Example from fm_mission_controler.ysc.c4:

	PLAYER::SET_PLAYER_LOCKON(PLAYER::PLAYER_ID(), 1);

	All other decompiled scripts using this seem to be using the player id as the first parameter, so I feel the need to confirm it as so.
</summary>
]]--
native "SET_PLAYER_LOCKON"
	hash "0x5C8B2F450EE4328E"
	jhash (0x0B270E0F)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PLAYER_TARGETING_MODE"
	hash "0xB1906895227793F3"
	jhash (0x61CAE253)
	arguments {
		Player "player",
	}
	returns	"void"

--[[!
<summary>
	short hash: 0x772DA539
</summary>
]]--
native "0x5702B917B99DB1CD"
	hash "0x5702B917B99DB1CD"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xB9CF1F793A9F1BF1"
	hash "0xB9CF1F793A9F1BF1"
	returns	"Any"

native "CLEAR_PLAYER_HAS_DAMAGED_AT_LEAST_ONE_PED"
	hash "0xF0B67A4DE6AB5F98"
	jhash (0x1D31CBBD)
	arguments {
		Player "player",
	}
	returns	"void"

native "HAS_PLAYER_DAMAGED_AT_LEAST_ONE_PED"
	hash "0x20CE80B0C2BF4ACC"
	jhash (0x14F52453)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "CLEAR_PLAYER_HAS_DAMAGED_AT_LEAST_ONE_NON_ANIMAL_PED"
	hash "0x4AACB96203D11A31"
	jhash (0x7E3BFBC5)
	arguments {
		Player "player",
	}
	returns	"void"

native "HAS_PLAYER_DAMAGED_AT_LEAST_ONE_NON_ANIMAL_PED"
	hash "0xE4B90F367BD81752"
	jhash (0xA3707DFC)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "SET_AIR_DRAG_MULTIPLIER_FOR_PLAYERS_VEHICLE"
	hash "0xCA7DC8329F0A1E9E"
	jhash (0xF20F72E5)
	arguments {
		Player "player",

		float "multiplier",
	}
	returns	"void"

--[[!
<summary>
	Swim speed multiplier.
	Multiplier goes up to 1.49
</summary>
]]--
native "SET_SWIM_MULTIPLIER_FOR_PLAYER"
	hash "0xA91C6F0FF7D16A13"
	jhash (0xB986FF47)
	arguments {
		Player "player",

		float "multiplier",
	}
	returns	"void"

--[[!
<summary>
	Multiplier can go up to 1.49
</summary>
]]--
native "SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER"
	hash "0x6DB47AA77FD94E09"
	jhash (0x825423C2)
	arguments {
		Player "player",

		float "multiplier",
	}
	returns	"void"

native "GET_TIME_SINCE_LAST_ARREST"
	hash "0x5063F92F07C2A316"
	jhash (0x62824EF4)
	returns	"Any"

native "GET_TIME_SINCE_LAST_DEATH"
	hash "0xC7034807558DDFCA"
	jhash (0x24BC5AC0)
	returns	"Any"

native "ASSISTED_MOVEMENT_CLOSE_ROUTE"
	hash "0xAEBF081FFC0A0E5E"
	jhash (0xF23277F3)
	returns	"void"

native "ASSISTED_MOVEMENT_FLUSH_ROUTE"
	hash "0x8621390F0CDCFE1F"
	jhash (0xD04568B9)
	returns	"void"

native "SET_PLAYER_FORCED_AIM"
	hash "0x0FEE4F80AC44A726"
	jhash (0x94E42E2E)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PLAYER_FORCED_ZOOM"
	hash "0x75E7D505F2B15902"
	jhash (0xB0C576CB)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PLAYER_FORCE_SKIP_AIM_INTRO"
	hash "0x7651BC64AE59E128"
	jhash (0x374F42F0)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Inhibits the player from using any method of combat including melee and firearms.

	NOTE: Only disables the firing for one frame

	---
	DISABLE_PLAYER_FIRING(0,0) works perfectly (for one frame); before, I was having problems with it not toggling off at all
</summary>
]]--
native "DISABLE_PLAYER_FIRING"
	hash "0x5E6CC07646BBEAB8"
	jhash (0x30CB28CB)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Old Gen: 0x47D6004E
</summary>
]]--
native "0xB885852C39CC265D"
	hash "0xB885852C39CC265D"
	returns	"void"

native "SET_DISABLE_AMBIENT_MELEE_MOVE"
	hash "0x2E8AABFA40A84F8C"
	jhash (0xCCD937E7)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Default is 100. Use player id and not ped id. For instance: PLAYER::SET_PLAYER_MAX_ARMOUR(PLAYER::PLAYER_ID(), 100); // main_persistent.ct4
</summary>
]]--
native "SET_PLAYER_MAX_ARMOUR"
	hash "0x77DFCCF5948B8C71"
	jhash (0xC6C3C53B)
	arguments {
		Player "player",

		int "value",
	}
	returns	"void"

native "SPECIAL_ABILITY_DEACTIVATE"
	hash "0xD6A953C6D1492057"
	jhash (0x80C2AB09)
	arguments {
		Player "player",
	}
	returns	"void"

native "SPECIAL_ABILITY_DEACTIVATE_FAST"
	hash "0x9CB5CE07A3968D5A"
	jhash (0x0751908A)
	arguments {
		Player "player",
	}
	returns	"void"

native "SPECIAL_ABILITY_RESET"
	hash "0x375F0E738F861A94"
	jhash (0xA7D8BCD3)
	arguments {
		Player "player",
	}
	returns	"void"

native "0xC9A763D8FE87436A"
	hash "0xC9A763D8FE87436A"
	jhash (0x4136829A)
	arguments {
		Player "player",
	}
	returns	"void"

--[[!
<summary>
	Every occurrence of p1 &amp; p2 were both true. 
</summary>
]]--
native "SPECIAL_ABILITY_CHARGE_SMALL"
	hash "0x2E7B9B683481687D"
	jhash (0x6F463F56)
	arguments {
		Player "player",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	Only 1 match. Both p1 &amp; p2 were true. 
</summary>
]]--
native "SPECIAL_ABILITY_CHARGE_MEDIUM"
	hash "0xF113E3AA9BC54613"
	jhash (0xAB55D8F3)
	arguments {
		Player "player",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	2 matches. p1 was always true. 
</summary>
]]--
native "SPECIAL_ABILITY_CHARGE_LARGE"
	hash "0xF733F45FA4497D93"
	jhash (0xF440C04D)
	arguments {
		Player "player",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	p1 appears to always be 1 (only comes up twice)
</summary>
]]--
native "SPECIAL_ABILITY_CHARGE_CONTINUOUS"
	hash "0xED481732DFF7E997"
	jhash (0x5FEE98A2)
	arguments {
		Player "player",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	p1 appears as 5, 10, 15, 25, or 30. p2 is always true. 
</summary>
]]--
native "SPECIAL_ABILITY_CHARGE_ABSOLUTE"
	hash "0xB7B0870EB531D08D"
	jhash (0x72429998)
	arguments {
		Player "player",

		int "p1",

		BOOL "p2",
	}
	returns	"void"

native "RESET_SPECIAL_ABILITY_CONTROLS_CINEMATIC"
	hash "0xA0696A65F009EE18"
	jhash (0x8C7E68C1)
	arguments {
		Player "player",

		float "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	Also known as _RECHARGE_SPECIAL_ABILITY
</summary>
]]--
native "SPECIAL_ABILITY_FILL_METER"
	hash "0x3DACA8DDC6FD4980"
	jhash (0xB71589DA)
	arguments {
		Player "player",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	p1 was always true. 
</summary>
]]--
native "SPECIAL_ABILITY_DEPLETE_METER"
	hash "0x1D506DBBBC51E64B"
	jhash (0x9F80F6DF)
	arguments {
		Player "player",

		BOOL "p1",
	}
	returns	"void"

native "SPECIAL_ABILITY_LOCK"
	hash "0x6A09D0D590A47D13"
	jhash (0x1B7BB388)
	arguments {
		Hash "playerModel",
	}
	returns	"void"

native "SPECIAL_ABILITY_UNLOCK"
	hash "0xF145F3BE2EFA9A3B"
	jhash (0x1FDB2919)
	arguments {
		Hash "playerModel",
	}
	returns	"void"

native "IS_SPECIAL_ABILITY_UNLOCKED"
	hash "0xC6017F6A6CDFA694"
	jhash (0xC9C75E82)
	arguments {
		Hash "modelHash",
	}
	returns	"BOOL"

native "IS_SPECIAL_ABILITY_ACTIVE"
	hash "0x3E5F7FC85D854E15"
	jhash (0x1B17E334)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "IS_SPECIAL_ABILITY_METER_FULL"
	hash "0x05A1FE504B7F2587"
	jhash (0x2E19D7F6)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "ENABLE_SPECIAL_ABILITY"
	hash "0x181EC197DAEFE121"
	jhash (0xC86C1B4E)
	arguments {
		Player "player",

		BOOL "enabled",
	}
	returns	"void"

native "IS_SPECIAL_ABILITY_ENABLED"
	hash "0xB1D200FE26AEF3CB"
	jhash (0xC01238CC)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "SET_SPECIAL_ABILITY_MULTIPLIER"
	hash "0xA49C426ED0CA4AB7"
	jhash (0xFF1BC556)
	arguments {
		float "multiplier",
	}
	returns	"void"

native "0xFFEE8FA29AB9A18E"
	hash "0xFFEE8FA29AB9A18E"
	jhash (0x5D0FE25B)
	arguments {
		Player "player",
	}
	returns	"void"

native "0x5FC472C501CCADB3"
	hash "0x5FC472C501CCADB3"
	jhash (0x46E7E31D)
	arguments {
		Player "player",
	}
	returns	"BOOL"

--[[!
<summary>
	Only 1 occurrence. p1 was 2.
</summary>
]]--
native "0xF10B44FD479D69F3"
	hash "0xF10B44FD479D69F3"
	jhash (0x1E359CC8)
	arguments {
		Player "player",

		int "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	2 occurrences in agency_heist3a. p1 was 0.7f then 0.4f.
</summary>
]]--
native "0xDD2620B7B9D16FF1"
	hash "0xDD2620B7B9D16FF1"
	jhash (0x8CB53C9F)
	arguments {
		Player "player",

		float "p1",
	}
	returns	"BOOL"

native "START_PLAYER_TELEPORT"
	hash "0xAD15F075A4DA0FDE"
	jhash (0xC552E06C)
	arguments {
		Player "player",

		float "x",

		float "y",

		float "z",

		float "heading",

		BOOL "p5",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"void"

native "0xE23D5873C2394C61"
	hash "0xE23D5873C2394C61"
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "STOP_PLAYER_TELEPORT"
	hash "0xC449EDED9D73009C"
	jhash (0x86AB8DBB)
	returns	"void"

native "IS_PLAYER_TELEPORT_ACTIVE"
	hash "0x02B15662D7F8886F"
	jhash (0x3A11D118)
	returns	"BOOL"

native "GET_PLAYER_CURRENT_STEALTH_NOISE"
	hash "0x2F395D61F3A1F877"
	jhash (0xC3B02362)
	arguments {
		Player "player",
	}
	returns	"float"

native "SET_PLAYER_HEALTH_RECHARGE_MULTIPLIER"
	hash "0x5DB660B38DD98A31"
	jhash (0x45514731)
	arguments {
		Player "player",

		float "regenRate",
	}
	returns	"void"

--[[!
<summary>
	This modifies the damage value of your weapon. Whether it is a multiplier or base damage is unknown. 

	Based on tests, it is unlikely to be a multiplier.
</summary>
]]--
native "SET_PLAYER_WEAPON_DAMAGE_MODIFIER"
	hash "0xCE07B9F7817AADA3"
	jhash (0xB02C2F39)
	arguments {
		Player "player",

		float "damageAmount",
	}
	returns	"void"

native "SET_PLAYER_WEAPON_DEFENSE_MODIFIER"
	hash "0x2D83BC011CA14A3C"
	jhash (0xAE446344)
	arguments {
		Player "player",

		float "modifier",
	}
	returns	"void"

native "SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER"
	hash "0x4A3DC7ECCC321032"
	jhash (0x362E69AD)
	arguments {
		Player "player",

		float "modifier",
	}
	returns	"void"

native "SET_PLAYER_MELEE_WEAPON_DEFENSE_MODIFIER"
	hash "0xAE540335B4ABC4E2"
	jhash (0x9F3D577F)
	arguments {
		Player "player",

		float "modifier",
	}
	returns	"void"

native "SET_PLAYER_VEHICLE_DAMAGE_MODIFIER"
	hash "0xA50E117CDDF82F0C"
	jhash (0x823ECA63)
	arguments {
		Player "player",

		float "damageAmount",
	}
	returns	"void"

native "SET_PLAYER_VEHICLE_DEFENSE_MODIFIER"
	hash "0x4C60E6EFDAFF2462"
	jhash (0xA16626C7)
	arguments {
		Player "player",

		float "modifier",
	}
	returns	"void"

native "SET_PLAYER_PARACHUTE_TINT_INDEX"
	hash "0xA3D0E54541D9A5E5"
	jhash (0x8EA12EDB)
	arguments {
		Player "player",

		int "tintIndex",
	}
	returns	"void"

native "GET_PLAYER_PARACHUTE_TINT_INDEX"
	hash "0x75D3F7A1B0D9B145"
	jhash (0x432B0509)
	arguments {
		Player "player",

		intPtr "tintIndex",
	}
	returns	"void"

native "SET_PLAYER_RESERVE_PARACHUTE_TINT_INDEX"
	hash "0xAF04C87F5DC1DF38"
	jhash (0x70689638)
	arguments {
		Player "player",

		int "index",
	}
	returns	"void"

native "GET_PLAYER_RESERVE_PARACHUTE_TINT_INDEX"
	hash "0xD5A016BC3C09CF40"
	jhash (0x77B8EF01)
	arguments {
		Player "player",

		intPtr "index",
	}
	returns	"void"

--[[!
<summary>
	tints 0

	1 
	2 
	3 
	4 
</summary>
]]--
native "SET_PLAYER_PARACHUTE_PACK_TINT_INDEX"
	hash "0x93B0FB27C9A04060"
	jhash (0xD79D5D1B)
	arguments {
		Player "player",

		int "tintIndex",
	}
	returns	"void"

native "GET_PLAYER_PARACHUTE_PACK_TINT_INDEX"
	hash "0x6E9C742F340CE5A2"
	jhash (0x4E418E13)
	arguments {
		Player "player",

		intPtr "tintIndex",
	}
	returns	"void"

native "SET_PLAYER_HAS_RESERVE_PARACHUTE"
	hash "0x7DDAB28D31FAC363"
	jhash (0xA3E4798E)
	arguments {
		Player "player",
	}
	returns	"void"

native "GET_PLAYER_HAS_RESERVE_PARACHUTE"
	hash "0x5DDFE2FF727F3CA3"
	jhash (0x30DA1DA1)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "SET_PLAYER_CAN_LEAVE_PARACHUTE_SMOKE_TRAIL"
	hash "0xF401B182DBA8AF53"
	jhash (0x832DEB7A)
	arguments {
		Player "player",

		BOOL "enabled",
	}
	returns	"void"

native "SET_PLAYER_PARACHUTE_SMOKE_TRAIL_COLOR"
	hash "0x8217FD371A4625CF"
	jhash (0x14FE9264)
	arguments {
		Player "player",

		int "r",

		int "g",

		int "b",
	}
	returns	"void"

native "GET_PLAYER_PARACHUTE_SMOKE_TRAIL_COLOR"
	hash "0xEF56DBABD3CD4887"
	jhash (0xF66E5CDD)
	arguments {
		Player "player",

		intPtr "r",

		intPtr "g",

		intPtr "b",
	}
	returns	"void"

--[[!
<summary>
	example:

	flags: 0-6

	PLAYER::SET_PLAYER_RESET_FLAG_PREFER_REAR_SEATS(PLAYER::PLAYER_ID(), 6);
</summary>
]]--
native "SET_PLAYER_RESET_FLAG_PREFER_REAR_SEATS"
	hash "0x11D5F725F0E780E0"
	jhash (0x725C6174)
	arguments {
		Player "player",

		int "flags",
	}
	returns	"void"

native "SET_PLAYER_NOISE_MULTIPLIER"
	hash "0xDB89EF50FF25FCE9"
	jhash (0x15786DD1)
	arguments {
		Player "player",

		float "multiplier",
	}
	returns	"void"

--[[!
<summary>
	Low floats around 1.0f to 2.0f appeared.
</summary>
]]--
native "SET_PLAYER_SNEAKING_NOISE_MULTIPLIER"
	hash "0xB2C1A29588A9F47C"
	jhash (0x8D2D89C4)
	arguments {
		Player "player",

		float "multiplier",
	}
	returns	"void"

native "CAN_PED_HEAR_PLAYER"
	hash "0xF297383AA91DCA29"
	jhash (0x1C70B2EB)
	arguments {
		Player "player",

		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	This is to make the player walk without accepting input from INPUT.

	gaitType is in increments of 100s. 2000, 500, 300, 200, etc.

	p4 is always 1 and p5 is always 0.
</summary>
]]--
native "SIMULATE_PLAYER_INPUT_GAIT"
	hash "0x477D5D63E63ECA5D"
	jhash (0x0D77CC34)
	arguments {
		Player "player",

		float "amount",

		int "gaitType",

		float "speed",

		BOOL "p4",

		BOOL "p5",
	}
	returns	"void"

native "RESET_PLAYER_INPUT_GAIT"
	hash "0x19531C47A2ABD691"
	jhash (0x4A701EE1)
	arguments {
		Player "player",
	}
	returns	"void"

native "SET_AUTO_GIVE_PARACHUTE_WHEN_ENTER_PLANE"
	hash "0x9F343285A00B4BB6"
	jhash (0xA97C2059)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

native "0xD2B315B6689D537D"
	hash "0xD2B315B6689D537D"
	jhash (0xA25D767E)
	arguments {
		Player "player",

		BOOL "p1",
	}
	returns	"void"

native "SET_PLAYER_STEALTH_PERCEPTION_MODIFIER"
	hash "0x4E9021C1FCDD507A"
	jhash (0x3D26105F)
	arguments {
		Player "player",

		float "value",
	}
	returns	"void"

native "0x690A61A6D13583F6"
	hash "0x690A61A6D13583F6"
	jhash (0x1D371529)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x9EDD76E87D5D51BA"
	hash "0x9EDD76E87D5D51BA"
	jhash (0xE30A64DC)
	arguments {
		Player "player",
	}
	returns	"void"

native "SET_PLAYER_SIMULATE_AIMING"
	hash "0xC54C95DA968EC5B5"
	jhash (0xF1E0CAFC)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Every occurrence of p1 I found was true.
</summary>
]]--
native "SET_PLAYER_CLOTH_PIN_FRAMES"
	hash "0x749FADDF97DFE930"
	jhash (0xF7A0F00F)
	arguments {
		Player "player",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Every occurrence was either 0 or 2.
</summary>
]]--
native "SET_PLAYER_CLOTH_PACKAGE_INDEX"
	hash "0x9F7BBA2EA6372500"
	jhash (0xB8209F16)
	arguments {
		int "index",
	}
	returns	"void"

--[[!
<summary>
	6 matches across 4 scripts. 5 occurrences were 240. The other was 255.
</summary>
]]--
native "SET_PLAYER_CLOTH_LOCK_COUNTER"
	hash "0x14D913B777DFF5DA"
	jhash (0x8D9FD4D1)
	arguments {
		int "value",
	}
	returns	"void"

--[[!
<summary>
	Only 1 match. ob_sofa_michael.

	PLAYER::PLAYER_ATTACH_VIRTUAL_BOUND(-804.5928f, 173.1801f, 71.68436f, 0f, 0f, 0.590625f, 1f, 0.7f);
</summary>
]]--
native "PLAYER_ATTACH_VIRTUAL_BOUND"
	hash "0xED51733DC73AED51"
	jhash (0xECD12E60)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",
	}
	returns	"void"

native "PLAYER_DETACH_VIRTUAL_BOUND"
	hash "0x1DD5897E2FA6E7C9"
	jhash (0x96100EA4)
	returns	"void"

native "HAS_PLAYER_BEEN_SPOTTED_IN_STOLEN_VEHICLE"
	hash "0xD705740BB0A1CF4C"
	jhash (0x4A01B76A)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "0x38D28DA81E4E9BF9"
	hash "0x38D28DA81E4E9BF9"
	jhash (0x013B4F72)
	arguments {
		Player "player",
	}
	returns	"BOOL"

--[[!
<summary>
	Assuming p0 is Player, not sure.
	p1 was always 100.
	p2 was always false.

</summary>
]]--
native "0xBC0753C9CA14B506"
	hash "0xBC0753C9CA14B506"
	jhash (0x9DF75B2A)
	arguments {
		Any "p0",

		int "p1",

		BOOL "p2",
	}
	returns	"BOOL"

native "0x5006D96C995A5827"
	hash "0x5006D96C995A5827"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

--[[!
<summary>
	Returns true if the player is riding a train.
</summary>
]]--
native "IS_PLAYER_RIDING_TRAIN"
	hash "0x4EC12697209F2196"
	jhash (0x9765E71D)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "HAS_PLAYER_LEFT_THE_WORLD"
	hash "0xD55DDFB47991A294"
	jhash (0xFEA40B6C)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "0xFF300C7649724A0B"
	hash "0xFF300C7649724A0B"
	jhash (0xAD8383FA)
	arguments {
		Player "player",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	p1 was always 5.
	p4 was always false.

</summary>
]]--
native "SET_PLAYER_PARACHUTE_VARIATION_OVERRIDE"
	hash "0xD9284A8C0D48352C"
	jhash (0x9254249D)
	arguments {
		Player "player",

		int "p1",

		Any "p2",

		Any "p3",

		BOOL "p4",
	}
	returns	"void"

native "CLEAR_PLAYER_PARACHUTE_VARIATION_OVERRIDE"
	hash "0x0F4CC924CF8C7B21"
	jhash (0xFD60F5AB)
	arguments {
		Player "player",
	}
	returns	"void"

--[[!
<summary>
	example:

	PLAYER::SET_PLAYER_PARACHUTE_MODEL_OVERRIDE(PLAYER::PLAYER_ID(), 0x73268708);
</summary>
]]--
native "SET_PLAYER_PARACHUTE_MODEL_OVERRIDE"
	hash "0x977DB4641F6FC3DB"
	jhash (0x5D382498)
	arguments {
		Player "player",

		Hash "model",
	}
	returns	"void"

native "CLEAR_PLAYER_PARACHUTE_MODEL_OVERRIDE"
	hash "0x8753997EB5F6EE3F"
	jhash (0x6FF034BB)
	arguments {
		Player "player",
	}
	returns	"void"

native "SET_PLAYER_PARACHUTE_PACK_MODEL_OVERRIDE"
	hash "0xDC80A4C2F18A2B64"
	jhash (0xA877FF5E)
	arguments {
		Player "player",

		Hash "model",
	}
	returns	"void"

native "CLEAR_PLAYER_PARACHUTE_PACK_MODEL_OVERRIDE"
	hash "0x10C54E4389C12B42"
	jhash (0xBB62AAC5)
	arguments {
		Player "player",
	}
	returns	"void"

native "DISABLE_PLAYER_VEHICLE_REWARDS"
	hash "0xC142BE3BB9CE125F"
	jhash (0x8C6E611D)
	arguments {
		Player "player",
	}
	returns	"void"

--[[!
<summary>
	Used with radios:

	void sub_cf383(auto _a0) {
	    if ((a_0)==1) {
	        if (GAMEPLAY::IS_BIT_SET((g_240005._f1), 3)) {
	            PLAYER::_2F7CEB6520288061(0);
	            AUDIO::SET_AUDIO_FLAG("AllowRadioDuringSwitch", 0);
	            AUDIO::SET_MOBILE_PHONE_RADIO_STATE(0);
	            AUDIO::SET_AUDIO_FLAG("MobileRadioInGame", 0);
	        }
	        sub_cf3f6(1);
	    } else { 
	        if (GAMEPLAY::IS_BIT_SET((g_240005._f1), 3)) {
	            PLAYER::_2F7CEB6520288061(1);
	            AUDIO::SET_AUDIO_FLAG("AllowRadioDuringSwitch", 1);
	            AUDIO::SET_MOBILE_PHONE_RADIO_STATE(1);
	            AUDIO::SET_AUDIO_FLAG("MobileRadioInGame", 1);
	        }
	        sub_cf3f6(0);
	    }
	}
</summary>
]]--
native "0x2F7CEB6520288061"
	hash "0x2F7CEB6520288061"
	jhash (0x2849D4B2)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x5DC40A8869C22141"
	hash "0x5DC40A8869C22141"
	arguments {
		BOOL "p0",

		ScrHandle "p1",
	}
	returns	"void"

native "0x65FAEE425DE637B0"
	hash "0x65FAEE425DE637B0"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x5501B7A5CDB79D37"
	hash "0x5501B7A5CDB79D37"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x56105E599CAB0EFA"
	hash "0x56105E599CAB0EFA"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "DOES_ENTITY_EXIST"
	hash "0x7239B21A38F536BA"
	jhash (0x3AC90869)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "DOES_ENTITY_BELONG_TO_THIS_SCRIPT"
	hash "0xDDE6DF5AE89981D2"
	jhash (0xACFEB3F9)
	arguments {
		Entity "entity",

		BOOL "p1",
	}
	returns	"BOOL"

native "DOES_ENTITY_HAVE_DRAWABLE"
	hash "0x060D6E96F8B8E48D"
	jhash (0xA5B33300)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "DOES_ENTITY_HAVE_PHYSICS"
	hash "0xDA95EA3317CC5064"
	jhash (0x9BCD2979)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

--[[!
<summary>
	P3 is always 3 as far as i cant tell 
</summary>
]]--
native "HAS_ENTITY_ANIM_FINISHED"
	hash "0x20B711662962B472"
	jhash (0x1D9CAB92)
	arguments {
		Entity "entity",

		charPtr "Animdict",

		charPtr "Anim",

		int "p3",
	}
	returns	"BOOL"

native "HAS_ENTITY_BEEN_DAMAGED_BY_ANY_OBJECT"
	hash "0x95EB9964FF5C5C65"
	jhash (0x6B74582E)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "HAS_ENTITY_BEEN_DAMAGED_BY_ANY_PED"
	hash "0x605F5A140F202491"
	jhash (0x53FD4A25)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "HAS_ENTITY_BEEN_DAMAGED_BY_ANY_VEHICLE"
	hash "0xDFD5033FDBA0A9C8"
	jhash (0x878C2CE0)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "HAS_ENTITY_BEEN_DAMAGED_BY_ENTITY"
	hash "0xC86D67D52A707CF8"
	jhash (0x07FC77E0)
	arguments {
		Entity "entity1",

		Entity "entity2",

		BOOL "p2",
	}
	returns	"BOOL"

--[[!
<summary>
	traceType is always 17 in the scripts.
</summary>
]]--
native "HAS_ENTITY_CLEAR_LOS_TO_ENTITY"
	hash "0xFCDFF7B72D23A1AC"
	jhash (0x53576FA7)
	arguments {
		Entity "entity1",

		Entity "entity2",

		Any "traceType",
	}
	returns	"BOOL"

--[[!
<summary>
	Has the entity1 got a clear line of sight to the other entity2 from the direction entity1 is facing.
	This is one of the most CPU demanding BOOL natives in the game; avoid calling this in things like nested for-loops
</summary>
]]--
native "HAS_ENTITY_CLEAR_LOS_TO_ENTITY_IN_FRONT"
	hash "0x0267D00AF114F17A"
	jhash (0x210D87C8)
	arguments {
		Entity "entity1",

		Entity "entity2",
	}
	returns	"BOOL"

native "HAS_ENTITY_COLLIDED_WITH_ANYTHING"
	hash "0x8BAD02F0368D9E14"
	jhash (0x662A2F41)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "GET_LAST_MATERIAL_HIT_BY_ENTITY"
	hash "0x5C3D0A935F535C4C"
	jhash (0xC0E3AA47)
	arguments {
		Entity "entity",
	}
	returns	"Hash"

native "GET_COLLISION_NORMAL_OF_LAST_HIT_FOR_ENTITY"
	hash "0xE465D4AB7CA6AE72"
	jhash (0xAB415C07)
	arguments {
		Entity "entity",
	}
	returns	"Vector3"

--[[!
<summary>
	Based on carmod_shop script decompile this takes a vehicle parameter. It is called when repair is done on initial enter.
</summary>
]]--
native "FORCE_ENTITY_AI_AND_ANIMATION_UPDATE"
	hash "0x40FDEDB72F8293B2"
	jhash (0x58D9775F)
	arguments {
		Entity "entity",
	}
	returns	"void"

--[[!
<summary>
	Return a float value representing animation's current playtime with respect to its total playtime. This value increasing in a range from [0 to 1] and wrap back to 0 when it reach 1.

	Example:
	0.000000 - mark the starting of animation.
	0.500000 - mark the midpoint of the animation.
	1.000000 - mark the end of animation.
</summary>
]]--
native "GET_ENTITY_ANIM_CURRENT_TIME"
	hash "0x346D81500D088F42"
	jhash (0x83943F41)
	arguments {
		Entity "entity",

		charPtr "animDict",

		charPtr "animation",
	}
	returns	"float"

--[[!
<summary>
	Return a float value representing animation's total playtime in milliseconds.

	Example:
	GET_ENTITY_ANIM_TOTAL_TIME(PLAYER_ID(),"amb@world_human_yoga@female@base","base_b") 
	return 20800.000000
</summary>
]]--
native "GET_ENTITY_ANIM_TOTAL_TIME"
	hash "0x50BD2730B191E360"
	jhash (0x433A9D18)
	arguments {
		Entity "entity",

		charPtr "animDict",

		charPtr "animation",
	}
	returns	"float"

--[[!
<summary>
	TODO: Copy hash from x360
</summary>
]]--
native "_GET_ANIM_DURATION"
	hash "0xFEDDF04D62B8D790"
	arguments {
		charPtr "animDict",

		charPtr "animation",
	}
	returns	"float"

native "GET_ENTITY_ATTACHED_TO"
	hash "0x48C2BED9180FE123"
	jhash (0xFE1589F9)
	arguments {
		Entity "entity",
	}
	returns	"Entity"

--[[!
<summary>
	p1 tends to be TRUE, however this is by no means a rule.


	if IS_ENTITY_DEAD then p1 set to 0 else p1 set to 1
</summary>
]]--
native "GET_ENTITY_COORDS"
	hash "0x3FEF770D40960D5A"
	jhash (0x1647F1CB)
	arguments {
		Entity "entity",

		BOOL "p1",
	}
	returns	"Vector3"

native "GET_ENTITY_FORWARD_VECTOR"
	hash "0x0A794A5A57F8DF91"
	jhash (0x84DCECBF)
	arguments {
		Entity "entity",
	}
	returns	"Any"

native "GET_ENTITY_FORWARD_X"
	hash "0x8BB4EF4214E0E6D5"
	jhash (0x49FAE914)
	arguments {
		Entity "entity",
	}
	returns	"float"

native "GET_ENTITY_FORWARD_Y"
	hash "0x866A4A5FAE349510"
	jhash (0x9E2F917C)
	arguments {
		Entity "entity",
	}
	returns	"float"

--[[!
<summary>
	Returns the heading of the entity in degrees.  
</summary>
]]--
native "GET_ENTITY_HEADING"
	hash "0xE83D4F9BA2A38914"
	jhash (0x972CC383)
	arguments {
		Entity "entity",
	}
	returns	"float"

native "0x846BF6291198A71E"
	hash "0x846BF6291198A71E"
	arguments {
		Entity "entity",
	}
	returns	"float"

--[[!
<summary>
	Return an integer value of entity's current health.

	Example of range for ped:
	- Player [0 to 200]
</summary>
]]--
native "GET_ENTITY_HEALTH"
	hash "0xEEF059FAD016D209"
	jhash (0x8E3222B7)
	arguments {
		Entity "entity",
	}
	returns	"int"

--[[!
<summary>
	Return an integer value of entity's maximum health.

	Example:
	- Player = 200
</summary>
]]--
native "GET_ENTITY_MAX_HEALTH"
	hash "0x15D757606D170C3C"
	jhash (0xC7AE6AA1)
	arguments {
		Entity "entity",
	}
	returns	"int"

--[[!
<summary>
	For instance: ENTITY::SET_ENTITY_MAX_HEALTH(PLAYER::PLAYER_PED_ID(), 200); // director_mode.c4: 67849
</summary>
]]--
native "SET_ENTITY_MAX_HEALTH"
	hash "0x166E7CF68597D8B5"
	jhash (0x96F84DF8)
	arguments {
		Entity "entity",

		int "value",
	}
	returns	"void"

native "GET_ENTITY_HEIGHT"
	hash "0x5A504562485944DD"
	jhash (0xEE443481)
	arguments {
		Entity "entity",

		float "X",

		float "Y",

		float "Z",

		BOOL "atTop",

		BOOL "inWorldCoords",
	}
	returns	"float"

--[[!
<summary>
	Return height (z-dimension) above ground. 
	Example: The pilot in a titan plane is 1.844176 above ground.
</summary>
]]--
native "GET_ENTITY_HEIGHT_ABOVE_GROUND"
	hash "0x1DD55701034110E5"
	jhash (0x57F56A4D)
	arguments {
		Entity "entity",
	}
	returns	"float"

--[[!
<summary>
	p1 - p4 are pointers to structs.
</summary>
]]--
native "GET_ENTITY_MATRIX"
	hash "0xECB2FC7235A7D137"
	jhash (0xEB9EB001)
	arguments {
		Entity "entity",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",

		AnyPtr "p4",
	}
	returns	"void"

--[[!
<summary>
	Returns the hash from the entity
</summary>
]]--
native "GET_ENTITY_MODEL"
	hash "0x9F47B058362C84B5"
	jhash (0xDAFCB3EC)
	arguments {
		Entity "entity",
	}
	returns	"Hash"

--[[!
<summary>
	Converts world coords to coords relative to the entity
</summary>
]]--
native "GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS"
	hash "0x2274BC1C4885E333"
	jhash (0x6477EC9E)
	arguments {
		Entity "entity",

		float "X",

		float "Y",

		float "Z",
	}
	returns	"Vector3"

--[[!
<summary>
	Offset values are relative to the entity.

	x = left/right
	y = forward/backward
	z = up/down
</summary>
]]--
native "GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS"
	hash "0x1899F328B0E12848"
	jhash (0xABCF043A)
	arguments {
		Entity "entity",

		float "xOffset",

		float "yOffset",

		float "zOffset",
	}
	returns	"Vector3"

native "GET_ENTITY_PITCH"
	hash "0xD45DC2893621E1FE"
	jhash (0xFCE6ECE5)
	arguments {
		Entity "entity",
	}
	returns	"float"

--[[!
<summary>
	w is the correct parameter name!
</summary>
]]--
native "GET_ENTITY_QUATERNION"
	hash "0x7B3703D2D32DFA18"
	jhash (0x5154EC90)
	arguments {
		Entity "entity",

		floatPtr "x",

		floatPtr "y",

		floatPtr "z",

		floatPtr "w",
	}
	returns	"void"

--[[!
<summary>
	Displays the current ROLL axis of the entity [-180.0000/180.0000+]
	(Sideways Roll) such as a vehicle tipped on its side
</summary>
]]--
native "GET_ENTITY_ROLL"
	hash "0x831E0242595560DF"
	jhash (0x36610842)
	arguments {
		Entity "entity",
	}
	returns	"float"

--[[!
<summary>
	p1 is usually 2 in scripts
	------
	ENTITY::GET_ENTITY_ROTATION(Any p0, false or true);
	if false than return from -180 to 180
	if true than return from -90 to 90

	---

	As said above, the value of p1 affects the outcome. R* uses 1 and 2 instead of 0 and 1, so I marked it as an int.

	What it returns is the yaw on the z part of the vector, which makes sense considering R* considers z as vertical. Here's a picture for those of you who don't understand pitch, yaw, and roll:

	http://www.allstar.fiu.edu/aero/images/pic5-1.gif

	I don't know why it returns a Vec3, but sometimes the values x and y go negative, yet they're always zero. Just use GET_ENTITY_PITCH and GET_ENTITY_ROLL for pitch and roll.
</summary>
]]--
native "GET_ENTITY_ROTATION"
	hash "0xAFBD61CC738D9EB9"
	jhash (0x8FF45B04)
	arguments {
		Entity "entity",

		int "p1",
	}
	returns	"Vector3"

native "GET_ENTITY_ROTATION_VELOCITY"
	hash "0x213B91045D09B983"
	jhash (0x9BF8A73F)
	arguments {
		Entity "entity",
	}
	returns	"Vector3"

--[[!
<summary>
	all ambient entities in-world seem to have the same value for the second argument (Any *script), depending on when the scripthook was activated/re-activated. I've seen numbers from ~5 to almost 70 when the value was translated with to_string. The function return value seems to always be 0
</summary>
]]--
native "GET_ENTITY_SCRIPT"
	hash "0xA6E9C38DB51D7748"
	jhash (0xB7F70784)
	arguments {
		Entity "entity",

		AnyPtr "script",
	}
	returns	"Any"

--[[!
<summary>
	result is in meters per second

	------------------------------------------------------------
	So would the conversion to MPH, be along the lines of this.

	float speed = get_entity_speed(veh);
	float MetersPerHour= (speed * 3600) //  corrected it said / 3600
	float MilesPerHour = (MetersPerHour / 1609.344);
</summary>
]]--
native "GET_ENTITY_SPEED"
	hash "0xD5037BA82E12416F"
	jhash (0x9E1E4798)
	arguments {
		Entity "entity",
	}
	returns	"float"

native "GET_ENTITY_SPEED_VECTOR"
	hash "0x9A8D700A51CB7B0D"
	jhash (0x3ED2B997)
	arguments {
		Entity "entity",

		BOOL "relative",
	}
	returns	"Vector3"

native "GET_ENTITY_UPRIGHT_VALUE"
	hash "0x95EED5A694951F9F"
	jhash (0xF4268190)
	arguments {
		Any "p0",
	}
	returns	"float"

native "GET_ENTITY_VELOCITY"
	hash "0x4805D2B1D8CF94A9"
	jhash (0xC14C9B6B)
	arguments {
		Entity "entity",
	}
	returns	"Vector3"

native "GET_OBJECT_INDEX_FROM_ENTITY_INDEX"
	hash "0xD7E3B9735C0F89D6"
	jhash (0xBC5A9C58)
	arguments {
		Entity "entity",
	}
	returns	"Object"

--[[!
<summary>
	source: mission_stat_watcher.c4
</summary>
]]--
native "GET_PED_INDEX_FROM_ENTITY_INDEX"
	hash "0x04A2A40C73395041"
	jhash (0xC46F74AC)
	arguments {
		Entity "entity",
	}
	returns	"Ped"

native "GET_VEHICLE_INDEX_FROM_ENTITY_INDEX"
	hash "0x4B53F92932ADFAC0"
	jhash (0xC69CF43D)
	arguments {
		Entity "entity",
	}
	returns	"Vehicle"

--[[!
<summary>
	Returns the coordinates of an entity-bone.
	Also known as "_GET_ENTITY_BONE_INDEX"
</summary>
]]--
native "GET_WORLD_POSITION_OF_ENTITY_BONE"
	hash "0x44A8FCB8ED227738"
	jhash (0x7C6339DF)
	arguments {
		Entity "entity",

		int "boneIndex",
	}
	returns	"Vector3"

native "GET_NEAREST_PLAYER_TO_ENTITY"
	hash "0x7196842CB375CDB3"
	jhash (0xCE17FDEC)
	arguments {
		Entity "entity",
	}
	returns	"Player"

native "GET_NEAREST_PLAYER_TO_ENTITY_ON_TEAM"
	hash "0x4DC9A62F844D9337"
	jhash (0xB1808F56)
	arguments {
		Entity "entity",

		int "team",
	}
	returns	"Player"

--[[!
<summary>
	Return:
	1 = peds
	2 
	3 = objects
</summary>
]]--
native "GET_ENTITY_TYPE"
	hash "0x8ACD366038D14505"
	jhash (0x0B1BD08D)
	arguments {
		Entity "entity",
	}
	returns	"int"

--[[!
<summary>
	TODO: copy hash from x360
</summary>
]]--
native "_GET_ENTITY_POPULATION_TYPE"
	hash "0xF6F5161F4534EDFF"
	arguments {
		Entity "entity",
	}
	returns	"int"

native "IS_AN_ENTITY"
	hash "0x731EC8A916BD11A1"
	jhash (0xD4B9715A)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_A_PED"
	hash "0x524AC5ECEA15343E"
	jhash (0x55D33EAB)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_A_MISSION_ENTITY"
	hash "0x0A7B270912999B3C"
	jhash (0x2632E124)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_A_VEHICLE"
	hash "0x6AC7003FA6E5575E"
	jhash (0xBE800B01)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_AN_OBJECT"
	hash "0x8D68C8FD0FACA94E"
	jhash (0x3F52E561)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

--[[!
<summary>
	Checks if entity is within x/y/zSize distance of x/y/z. 

	Last three are unknown ints, almost always p7 = 0, p8 = 1, p9 = 0
</summary>
]]--
native "IS_ENTITY_AT_COORD"
	hash "0x20B60995556D004F"
	jhash (0xD749B606)
	arguments {
		Entity "entity",

		float "x",

		float "y",

		float "z",

		float "xSize",

		float "ySize",

		float "zSize",

		BOOL "p7",

		BOOL "p8",

		int "p9",
	}
	returns	"BOOL"

--[[!
<summary>
	Checks if entity1 is within the box defined by x/y/zSize of entity2.

	Last three parameters are almost alwasy p5 = 0, p6 = 1, p7 = 0
</summary>
]]--
native "IS_ENTITY_AT_ENTITY"
	hash "0x751B70C3D034E187"
	jhash (0xDABDCB52)
	arguments {
		Entity "entity1",

		Entity "entity2",

		float "xSize",

		float "ySize",

		float "zSize",

		BOOL "p5",

		BOOL "p6",

		int "p7",
	}
	returns	"BOOL"

native "IS_ENTITY_ATTACHED"
	hash "0xB346476EF1A64897"
	jhash (0xEC1479D5)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_ATTACHED_TO_ANY_OBJECT"
	hash "0xCF511840CEEDE0CC"
	jhash (0x0B5DE340)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_ATTACHED_TO_ANY_PED"
	hash "0xB1632E9A5F988D11"
	jhash (0x9D7A609C)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_ATTACHED_TO_ANY_VEHICLE"
	hash "0x26AA915AD89BFB4B"
	jhash (0xDE5C995E)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_ATTACHED_TO_ENTITY"
	hash "0xEFBE71898A993728"
	jhash (0xB0ABFEA8)
	arguments {
		Entity "from",

		Entity "to",
	}
	returns	"BOOL"

native "IS_ENTITY_DEAD"
	hash "0x5F9532F3B5CC2551"
	jhash (0xB6F7CBAC)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_IN_AIR"
	hash "0x886E37EC497200B6"
	jhash (0xA4157987)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

--[[!
<summary>
	angle is in radians
</summary>
]]--
native "IS_ENTITY_IN_ANGLED_AREA"
	hash "0x51210CED3DA1C78A"
	jhash (0x883622FA)
	arguments {
		Entity "entity",

		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		float "angle",

		BOOL "p8",

		BOOL "p9",

		Any "p10",
	}
	returns	"BOOL"

native "IS_ENTITY_IN_AREA"
	hash "0x54736AA40E271165"
	jhash (0x8C2DFA9D)
	arguments {
		Entity "entity",

		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		BOOL "p7",

		BOOL "p8",

		Any "p9",
	}
	returns	"BOOL"

native "IS_ENTITY_IN_ZONE"
	hash "0xB6463CF6AF527071"
	jhash (0x45C82B21)
	arguments {
		Entity "entity",

		charPtr "zone",
	}
	returns	"BOOL"

native "IS_ENTITY_IN_WATER"
	hash "0xCFB0A0D8EDD145A3"
	jhash (0x4C3C2508)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

--[[!
<summary>
	Get how much of the entity is submerged.  1.0f is whole entity.
</summary>
]]--
native "GET_ENTITY_SUBMERGED_LEVEL"
	hash "0xE81AFC1BC4CC41CE"
	jhash (0x0170F68C)
	arguments {
		Entity "entity",
	}
	returns	"float"

native "0x694E00132F2823ED"
	hash "0x694E00132F2823ED"
	jhash (0x40C84A74)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_ENTITY_ON_SCREEN"
	hash "0xE659E47AF827484B"
	jhash (0xC1FEC5ED)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

--[[!
<summary>
	See also PED::_IS_PED_PERFORMING_ANIM 0x6EC47A344923E1ED 0x3C30B447

	Taken from ENTITY::IS_ENTITY_PLAYING_ANIM(PLAYER::PLAYER_PED_ID(), "creatures@shark@move", "attack_player", 3)

	p4 is always 3 in the scripts.

	Animations list - http://www.gta5-mystery-busters.onet.domains/tools/anims.php

</summary>
]]--
native "IS_ENTITY_PLAYING_ANIM"
	hash "0x1F0B79228E461EC9"
	jhash (0x0D130D34)
	arguments {
		Entity "entity",

		charPtr "animGroup",

		charPtr "animation",

		int "p4",
	}
	returns	"BOOL"

--[[!
<summary>
	a static ped will not react to natives like "APPLY_FORCE_TO_ENTITY" or "SET_ENTITY_VELOCITY" and oftentimes will not react to task-natives like "AI::TASK_COMBAT_PED". The only way I know of to make one of these peds react is to ragdoll them (or sometimes to use CLEAR_PED_TASKS_IMMEDIATELY(). Static peds include almost all far-away peds, beach-combers, peds in certain scenarios, peds crossing a crosswalk, peds walking to get back into their cars, and others. If anyone knows how to make a ped non-static without ragdolling them, please edit this with the solution.
</summary>
]]--
native "IS_ENTITY_STATIC"
	hash "0x1218E6886D3D8327"
	jhash (0x928E12E9)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_TOUCHING_ENTITY"
	hash "0x17FFC1B2BA35A494"
	jhash (0x6B931477)
	arguments {
		Entity "from",

		Entity "to",
	}
	returns	"BOOL"

native "IS_ENTITY_TOUCHING_MODEL"
	hash "0x0F42323798A58C8C"
	jhash (0x307E7611)
	arguments {
		Entity "entity",

		Hash "modelHash",
	}
	returns	"BOOL"

native "IS_ENTITY_UPRIGHT"
	hash "0x5333F526F6AB19AA"
	jhash (0x3BCDF4E1)
	arguments {
		Entity "entity",

		float "angle",
	}
	returns	"BOOL"

native "IS_ENTITY_UPSIDEDOWN"
	hash "0x1DBD58820FA61D71"
	jhash (0x5ACAA48F)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_VISIBLE"
	hash "0x47D6F43D77935C75"
	jhash (0x120B4ED5)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_VISIBLE_TO_SCRIPT"
	hash "0xD796CB5BA8F20E32"
	jhash (0x5D240E9D)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "IS_ENTITY_OCCLUDED"
	hash "0xE31C2C72B8692B64"
	jhash (0x46BC5B40)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "WOULD_ENTITY_BE_OCCLUDED"
	hash "0xEE5D2A122E09EC42"
	jhash (0xEA127CBC)
	arguments {
		Entity "entity",

		float "x",

		float "y",

		float "z",

		BOOL "p4",
	}
	returns	"BOOL"

native "IS_ENTITY_WAITING_FOR_WORLD_COLLISION"
	hash "0xD05BFF0C0A12C68F"
	jhash (0x00AB7A4A)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS"
	hash "0x18FF00FC7EFF559E"
	jhash (0x28924E98)
	arguments {
		Entity "entity",

		int "forceType",

		float "x",

		float "y",

		float "z",

		BOOL "p5",

		BOOL "p6",

		BOOL "p7",

		BOOL "p8",
	}
	returns	"void"

--[[!
<summary>
	isRel - specifies if the force direction is relative to direction entity is facing (true), or static world direction (false).

	---(from someone else:)
	From the scripts, here:
	ENTITY::APPLY_FORCE_TO_ENTITY(a_1, 1, a_0._f1D * ((vector)(4.0)), v_4 - ENTITY::GET_ENTITY_COORDS(a_1, 1), PED::GET_PED_RAGDOLL_BONE_INDEX(a_1, 0), 0, 0, 1, 0, 1);
	Why is there a native to retrieve the bone index in the place of the isRel BOOL?

	It's actually being used for the p8 variable.

	Well just change the p8 from bool to int as in the script, epsilon1.c extracted from xbox by XBL Toothpik they use a "3" so it's not a bool.
</summary>
]]--
native "APPLY_FORCE_TO_ENTITY"
	hash "0xC5F68BE9613E2D18"
	jhash (0xC1C0855A)
	arguments {
		Entity "entity",

		int "forceType",

		float "x",

		float "y",

		float "z",

		float "xRot",

		float "yRot",

		float "zRot",

		int "p8",

		BOOL "isRel",

		BOOL "ignoreUpVec",

		BOOL "p11",

		BOOL "p12",

		BOOL "p13",
	}
	returns	"void"

--[[!
<summary>
	"int unk" mainly set as 2 in scripts
	"BOOL p14" mainly set as true in scripts
</summary>
]]--
native "ATTACH_ENTITY_TO_ENTITY"
	hash "0x6B9BBD38AB0796DF"
	jhash (0xEC024237)
	arguments {
		Entity "entity1",

		Entity "entity2",

		int "boneIndex",

		float "x",

		float "y",

		float "z",

		float "xRot",

		float "yRot",

		float "zRot",

		BOOL "p9",

		BOOL "isRel",

		BOOL "ignoreUpVec",

		BOOL "allowRotation",

		int "unk",

		BOOL "p14",
	}
	returns	"void"

--[[!
<summary>
	forceToBreak is the amount of force required to break the bond.
	p14 - is always 1 in scripts
	p15 - is 1 or 0 in scripts - unknown what it does
	p16 - controls collision between entities (FALSE disables collision).
	p17 - is 1 or 0 in scripts - unknown what it does
	p18 - is always 2 in scripts.
</summary>
]]--
native "ATTACH_ENTITY_TO_ENTITY_PHYSICALLY"
	hash "0xC3675780C92F90F9"
	jhash (0x0547417F)
	arguments {
		Entity "entity1",

		Entity "entity2",

		int "boneIndex",

		int "boneIndex2",

		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		float "xRot",

		float "yRot",

		float "zRot",

		float "forceToBreak",

		BOOL "p14",

		BOOL "p15",

		BOOL "p16",

		BOOL "p17",

		int "p18",
	}
	returns	"void"

--[[!
<summary>
	Called to update entity attachments.
</summary>
]]--
native "PROCESS_ENTITY_ATTACHMENTS"
	hash "0xF4080490ADC51C6F"
	jhash (0x6909BA59)
	arguments {
		Entity "entity",
	}
	returns	"void"

--[[!
<summary>
	Returns the index of the bone. If the bone was not found, the return value is -1.

	BoneNames:
		chassis,
		windscreen,
		seat_pside_r,
		seat_dside_r,
		bodyshell,
		suspension_lm,
		suspension_lr,
		platelight,

	I found other names in the decompiled scripts:
		attach_female,
		attach_male,
		bonnet,
		boot,
		chassis_dummy,	//Center of the dummy
		chassis_Control,	//Not found yet
		door_dside_f,	//Door left, front
		door_dside_r,	//Door left, back
		door_pside_f,	//Door right, front
		door_pside_r,	//Door right, back
		Gun_GripR,
		windscreen_f,
		platelight,	//Position where the light above the numberplate is located
		VFX_Emitter,
		window_lf,	//Window left, front
		window_lr,	//Window left, back
		window_rf,	//Window right, front
		window_rr,	//Window right, back
		engine,	//Position of the engine
		gun_ammo,
		ROPE_ATTATCH,	//Not misspelled. In script "finale_heist2b.c4".
		wheel_lf,	//Wheel left, front
		wheel_lr,	//Wheel left, back
		wheel_rf,	//Wheel right, front
		wheel_rr,	//Wheel right, back
		exhaust,	//Exhaust. shows only the position of the stock-exhaust
		overheat,	//A position on the engine(not exactly sure, how to name it)
		misc_e,	//Not a car-bone.
		seat_dside_f,	//Driver-seat
		seat_pside_f,	//Seat next to driver
		Gun_Nuzzle,
		seat_r

	I doubt that the function is case-sensitive, since I found a "Chassis" and a "chassis". - Just tested: Definitely not case-sensitive.

	Also known as "_GET_ENTITY_BONE_INDEX".

	Use this function in combination of "GET_WORLD_POSITION_OF_ENTITY_BONE":
	GET_WORLD_POSITION_OF_ENTITY_BONE(entity, GET_ENTITY_BONE_INDEX_BY_NAME(entity, index))
</summary>
]]--
native "GET_ENTITY_BONE_INDEX_BY_NAME"
	hash "0xFB71170B7E76ACBA"
	jhash (0xE4ECAC22)
	arguments {
		Entity "entity",

		charPtr "boneName",
	}
	returns	"int"

native "CLEAR_ENTITY_LAST_DAMAGE_ENTITY"
	hash "0xA72CD9CA74A5ECBA"
	jhash (0x2B83F43B)
	arguments {
		Entity "entity",
	}
	returns	"Any"

native "DELETE_ENTITY"
	hash "0xAE3CBE5BF394C9C9"
	jhash (0xFAA3D236)
	arguments {
		EntityPtr "entity",
	}
	returns	"void"

--[[!
<summary>
	bools usually both true
</summary>
]]--
native "DETACH_ENTITY"
	hash "0x961AC54BF0613F5D"
	jhash (0xC8EFCB41)
	arguments {
		Entity "entity",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "FREEZE_ENTITY_POSITION"
	hash "0x428CA6DBD1094446"
	jhash (0x65C16D57)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

native "0x3910051CCECDB00C"
	hash "0x3910051CCECDB00C"
	jhash (0xD3850671)
	arguments {
		Entity "entity",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	delta and bitset are guessed fields. They are based on the fact that most of the calls have 0 or nil field types passed in.

	The only time bitset has a value is 0x4000 and the only time delta has a value is during stealth with usually &lt;1.0f values.

	Animations list - http://www.gta5-mystery-busters.onet.domains/tools/anims.php
</summary>
]]--
native "PLAY_ENTITY_ANIM"
	hash "0x7FB218262B810701"
	jhash (0x878753D5)
	arguments {
		Entity "entity",

		charPtr "animation",

		charPtr "propName",

		float "p3",

		BOOL "p4",

		BOOL "p5",

		BOOL "p6",

		float "delta",

		Any "bitset",
	}
	returns	"BOOL"

--[[!
<summary>
	p4 and p7 are usually 1000.0f.
</summary>
]]--
native "PLAY_SYNCHRONIZED_ENTITY_ANIM"
	hash "0xC77720A12FE14A86"
	jhash (0x012760AA)
	arguments {
		Entity "entity",

		Entity "syncedScene",

		charPtr "animation",

		charPtr "propName",

		float "p4",

		float "p5",

		Any "p6",

		float "p7",
	}
	returns	"BOOL"

native "PLAY_SYNCHRONIZED_MAP_ENTITY_ANIM"
	hash "0xB9C54555ED30FBC4"
	jhash (0xEB4CBA74)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		Any "p5",

		AnyPtr "p6",

		AnyPtr "p7",

		float "p8",

		float "p9",

		Any "p10",

		float "p11",
	}
	returns	"BOOL"

native "STOP_SYNCHRONIZED_MAP_ENTITY_ANIM"
	hash "0x11E79CAB7183B6F5"
	jhash (0x7253D5B2)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		float "p5",
	}
	returns	"BOOL"

--[[!
<summary>
	Animations list - http://www.gta5-mystery-busters.onet.domains/tools/anims.php
</summary>
]]--
native "STOP_ENTITY_ANIM"
	hash "0x28004F88151E03E0"
	jhash (0xC4769830)
	arguments {
		Entity "entity",

		charPtr "animation",

		charPtr "animGroup",

		float "p3",
	}
	returns	"Any"

native "STOP_SYNCHRONIZED_ENTITY_ANIM"
	hash "0x43D3807C077261E3"
	jhash (0xE27D2FC1)
	arguments {
		Entity "entity",

		float "p1",

		BOOL "p2",
	}
	returns	"BOOL"

--[[!
<summary>
	if (ENTITY::HAS_ANIM_EVENT_FIRED(PLAYER::PLAYER_PED_ID(), GAMEPLAY::GET_HASH_KEY("CreateObject")))
</summary>
]]--
native "HAS_ANIM_EVENT_FIRED"
	hash "0xEAF4CD9EA3E7E922"
	jhash (0x66571CA0)
	arguments {
		Entity "entity",

		Hash "actionHash",
	}
	returns	"BOOL"

--[[!
<summary>
	In the script "player_scene_t_bbfight.c4":
	"if (ENTITY::FIND_ANIM_EVENT_PHASE(&amp;l_16E, &amp;l_19F[v_4/*16*/], v_9, &amp;v_A, &amp;v_B))"
	-- &amp;l_16E (p0) is requested as an anim dictionary earlier in the script.
	-- &amp;l_19F[v_4/*16*/] (p1) is used in other natives in the script as the "animation" param.
	-- v_9 (p2) is instantiated as "victim_fall"; I'm guessing that's another anim
	--v_A and v_B (p3 &amp; p4) are both set as -1.0, but v_A is used immediately after this native for: 
	"if (v_A &lt; ENTITY::GET_ENTITY_ANIM_CURRENT_TIME(...))"
	Both v_A and v_B are seemingly used to contain both Vector3's and floats, so I can't say what either really is other than that they are both output parameters. p4 looks more like a *Vector3 though
</summary>
]]--
native "FIND_ANIM_EVENT_PHASE"
	hash "0x07F1BE2BCCAA27A7"
	jhash (0xC41DDA62)
	arguments {
		charPtr "animDict",

		charPtr "animation",

		charPtr "p2",

		AnyPtr "p3",

		AnyPtr "p4",
	}
	returns	"BOOL"

native "SET_ENTITY_ANIM_CURRENT_TIME"
	hash "0x4487C259F0F70977"
	jhash (0x99D90735)
	arguments {
		Ped "ped",

		charPtr "animdict",

		charPtr "animName",

		float "time",
	}
	returns	"void"

native "SET_ENTITY_ANIM_SPEED"
	hash "0x28D1A16553C51776"
	jhash (0x3990C90A)
	arguments {
		Entity "entityHandle",

		charPtr "animationDiction",

		charPtr "animationName",

		float "speedMultiplier",
	}
	returns	"void"

--[[!
<summary>
	Makes the specified entity (ped, vehicle or object) persistent. Persistent entities will not automatically be removed by the engine.

	Always pass true for p2 until its use is known.
</summary>
]]--
native "SET_ENTITY_AS_MISSION_ENTITY"
	hash "0xAD738C3085FE7E11"
	jhash (0x5D1F9E0F)
	arguments {
		Entity "entity",

		BOOL "value",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	Marks the specified entity (ped, vehicle or object) as no longer needed.
	Entities marked as no longer needed, will be deleted as the engine sees fit.
</summary>
]]--
native "SET_ENTITY_AS_NO_LONGER_NEEDED"
	hash "0xB736A491E64A32CF"
	jhash (0xADF2267C)
	arguments {
		EntityPtr "entity",
	}
	returns	"void"

--[[!
<summary>
	This is an alias of SET_ENTITY_AS_NO_LONGER_NEEDED.
</summary>
]]--
native "SET_PED_AS_NO_LONGER_NEEDED"
	hash "0x2595DD4236549CE3"
	jhash (0x9A388380)
	arguments {
		PedPtr "ped",
	}
	returns	"void"

--[[!
<summary>
	This is an alias of SET_ENTITY_AS_NO_LONGER_NEEDED.
</summary>
]]--
native "SET_VEHICLE_AS_NO_LONGER_NEEDED"
	hash "0x629BFA74418D6239"
	jhash (0x9B0E10BE)
	arguments {
		VehiclePtr "vehicle",
	}
	returns	"void"

--[[!
<summary>
	This is an alias of SET_ENTITY_AS_NO_LONGER_NEEDED.
</summary>
]]--
native "SET_OBJECT_AS_NO_LONGER_NEEDED"
	hash "0x3AE22DEB5BA5A3E6"
	jhash (0x3F6B949F)
	arguments {
		ObjectPtr "object",
	}
	returns	"void"

native "SET_ENTITY_CAN_BE_DAMAGED"
	hash "0x1760FFA8AB074D66"
	jhash (0x60B6E744)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ENTITY_CAN_BE_DAMAGED_BY_RELATIONSHIP_GROUP"
	hash "0xE22D8FDE858B8119"
	jhash (0x34165B5D)
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",
	}
	returns	"void"

native "SET_ENTITY_CAN_BE_TARGETED_WITHOUT_LOS"
	hash "0xD3997889736FD899"
	jhash (0x3B13797C)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Collision on  when toggle -&gt; 1 
	Collision off when toggle -&gt; 0

	p2 is always 0 in the decompiled scripts (gtav.ysc.decompiled)
</summary>
]]--
native "SET_ENTITY_COLLISION"
	hash "0x1A9205C1B9EE827F"
	jhash (0x139FD37D)
	arguments {
		Entity "entity",

		BOOL "toggle",

		BOOL "p2",
	}
	returns	"void"

native "0xCCF1E97BEFDAE480"
	hash "0xCCF1E97BEFDAE480"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x9EBC85ED0FFFE51C"
	hash "0x9EBC85ED0FFFE51C"
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	p7 is always 1 in the scripts.
</summary>
]]--
native "SET_ENTITY_COORDS"
	hash "0x06843DA7060A026B"
	jhash (0xDF70B41B)
	arguments {
		Entity "entity",

		float "X",

		float "Y",

		float "Z",

		BOOL "xAxis",

		BOOL "yAxis",

		BOOL "zAxis",

		BOOL "p7",
	}
	returns	"void"

--[[!
<summary>
	does the same as SET_ENTITY_COORDS.

	Nice comment, but for anyone who just decides to use it. 
	Console Hash: 0x749B282E
</summary>
]]--
native "_SET_ENTITY_COORDS_2"
	hash "0x621873ECE1178967"
	arguments {
		Entity "entity",

		float "X",

		float "Y",

		float "Z",

		BOOL "xAxis",

		BOOL "yAxis",

		BOOL "zAxis",

		BOOL "p7",
	}
	returns	"void"

native "SET_ENTITY_COORDS_NO_OFFSET"
	hash "0x239A3351AC1DA385"
	jhash (0x4C83DE8D)
	arguments {
		Entity "entity",

		float "X",

		float "Y",

		float "Z",

		BOOL "p4",

		BOOL "p5",

		BOOL "p6",
	}
	returns	"void"

native "SET_ENTITY_DYNAMIC"
	hash "0x1718DE8E3F2823CA"
	jhash (0x236F525B)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ENTITY_HEADING"
	hash "0x8E2530AA8ADA980E"
	jhash (0xE0FF064D)
	arguments {
		Entity "entity",

		float "heading",
	}
	returns	"void"

--[[!
<summary>
	health &gt;= 0
</summary>
]]--
native "SET_ENTITY_HEALTH"
	hash "0x6B76DC1F3AE6E6A3"
	jhash (0xFBCD1831)
	arguments {
		Entity "entity",

		int "health",
	}
	returns	"void"

native "SET_ENTITY_INVINCIBLE"
	hash "0x3882114BDE571AD4"
	jhash (0xC1213A21)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ENTITY_IS_TARGET_PRIORITY"
	hash "0xEA02E132F5C68722"
	jhash (0x9729EE32)
	arguments {
		Any "p0",

		BOOL "p1",

		float "p2",
	}
	returns	"void"

native "SET_ENTITY_LIGHTS"
	hash "0x7CFBA6A80BDF3874"
	jhash (0xE8FC85AF)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ENTITY_LOAD_COLLISION_FLAG"
	hash "0x0DC7CABAB1E9B67E"
	jhash (0xC52F295B)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

native "HAS_COLLISION_LOADED_AROUND_ENTITY"
	hash "0xE9676F61BC0B3321"
	jhash (0x851687F9)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "SET_ENTITY_MAX_SPEED"
	hash "0x0E46A3FCBDE2A1B1"
	jhash (0x46AFFED3)
	arguments {
		Entity "entity",

		float "speed",
	}
	returns	"void"

native "SET_ENTITY_ONLY_DAMAGED_BY_PLAYER"
	hash "0x79F020FF9EDC0748"
	jhash (0x4B707F50)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_ENTITY_ONLY_DAMAGED_BY_RELATIONSHIP_GROUP"
	hash "0x7022BD828FA0B082"
	jhash (0x202237E2)
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",
	}
	returns	"void"

--[[!
<summary>
	Enable / disable each type of damage.
</summary>
]]--
native "SET_ENTITY_PROOFS"
	hash "0xFAEE099C6F890BB8"
	jhash (0x7E9EAB66)
	arguments {
		Entity "entity",

		BOOL "bulletProof",

		BOOL "fireProof",

		BOOL "explosionProof",

		BOOL "collisionProof",

		BOOL "meleeProof",

		BOOL "p6",

		BOOL "p7",

		BOOL "drownProof",
	}
	returns	"void"

--[[!
<summary>
	w is the correct parameter name!
</summary>
]]--
native "SET_ENTITY_QUATERNION"
	hash "0x77B21BE7AC540F07"
	jhash (0x83B6046F)
	arguments {
		Entity "entity",

		float "x",

		float "y",

		float "z",

		float "w",
	}
	returns	"void"

native "SET_ENTITY_RECORDS_COLLISIONS"
	hash "0x0A50A1EEDAD01E65"
	jhash (0x6B189A1A)
	arguments {
		Entity "entity",

		BOOL "record",
	}
	returns	"void"

--[[!
<summary>
	p4 is usually 2 in scripts and p5 is usually 1
</summary>
]]--
native "SET_ENTITY_ROTATION"
	hash "0x8524A8B0171D5E07"
	jhash (0x0A345EFE)
	arguments {
		Entity "entity",

		float "pitch",

		float "roll",

		float "yaw",

		int "p4",

		BOOL "p5",
	}
	returns	"void"

native "SET_ENTITY_VISIBLE"
	hash "0xEA1C610A04DB6BBB"
	jhash (0xD043E8E1)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Note that the third parameter(denoted as z) is "up and down" with positive numbers encouraging upwards movement.
</summary>
]]--
native "SET_ENTITY_VELOCITY"
	hash "0x1C99BB7B6E96D16F"
	jhash (0xFF5A1988)
	arguments {
		Entity "entity",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "SET_ENTITY_HAS_GRAVITY"
	hash "0x4A4722448F18EEF5"
	jhash (0xE2F262BF)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ENTITY_LOD_DIST"
	hash "0x5927F96A78577363"
	jhash (0xD7ACC7AD)
	arguments {
		Entity "entity",

		int "distance",
	}
	returns	"void"

--[[!
<summary>
	Returns the LOD distance of an entity.
</summary>
]]--
native "GET_ENTITY_LOD_DIST"
	hash "0x4159C2762B5791D6"
	jhash (0x4DA3D51F)
	arguments {
		Entity "entity",
	}
	returns	"int"

--[[!
<summary>
	Set entity alpha level. Ranging from 0 to 255 but chnages occur after every 20 percent (after every 51).
</summary>
]]--
native "SET_ENTITY_ALPHA"
	hash "0x44A0870B7E92D7C0"
	jhash (0xAE667CB0)
	arguments {
		Entity "entity",

		int "alphaLevel",

		BOOL "p2",
	}
	returns	"void"

native "GET_ENTITY_ALPHA"
	hash "0x5A47B3B5E63E94C6"
	jhash (0x1560B017)
	arguments {
		Entity "entity",
	}
	returns	"int"

native "RESET_ENTITY_ALPHA"
	hash "0x9B1E824FFBB7027A"
	jhash (0x8A30761C)
	arguments {
		Entity "entity",
	}
	returns	"Any"

native "0x5C3B791D580E0BC2"
	hash "0x5C3B791D580E0BC2"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "SET_ENTITY_ALWAYS_PRERENDER"
	hash "0xACAD101E1FB66689"
	jhash (0xD8FF798A)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ENTITY_RENDER_SCORCHED"
	hash "0x730F5F8D3F0F2050"
	jhash (0xAAC9317B)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	State can be 0 through 3.
	(The function ignores anything but the 3 least significant bits, so 4 would be interpreted as 0, 5 as 1, etc.)
</summary>
]]--
native "SET_ENTITY_TRAFFICLIGHT_OVERRIDE"
	hash "0x57C5DB656185EAC4"
	jhash (0xC47F5B91)
	arguments {
		Entity "entity",

		int "state",
	}
	returns	"void"

native "0x78E8E3A640178255"
	hash "0x78E8E3A640178255"
	arguments {
		Any "p0",
	}
	returns	"void"

native "CREATE_MODEL_SWAP"
	hash "0x92C47782FDA8B2A3"
	jhash (0x0BC12F9E)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		Any "p5",

		BOOL "p6",
	}
	returns	"void"

native "REMOVE_MODEL_SWAP"
	hash "0x033C0F9A64E229AE"
	jhash (0xCE0AA8BC)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		Any "p5",

		BOOL "p6",
	}
	returns	"void"

native "CREATE_MODEL_HIDE"
	hash "0x8A97BCA30A0CE478"
	jhash (0x7BD5CF2F)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		BOOL "p5",
	}
	returns	"void"

native "CREATE_MODEL_HIDE_EXCLUDING_SCRIPT_OBJECTS"
	hash "0x3A52AE588830BF7F"
	jhash (0x07AAF22C)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		BOOL "p5",
	}
	returns	"void"

native "REMOVE_MODEL_HIDE"
	hash "0xD9E3006FB3CBD765"
	jhash (0x993DBC10)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"void"

native "CREATE_FORCED_OBJECT"
	hash "0x150E808B375A385A"
	jhash (0x335190A2)
	arguments {
		float "x",

		float "y",

		float "z",

		Any "p3",

		Hash "modelHash",

		BOOL "p5",
	}
	returns	"void"

native "REMOVE_FORCED_OBJECT"
	hash "0x61B6775E83C0DB6F"
	jhash (0xAED73ADD)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

--[[!
<summary>
	Useful for creating a Singleplayer passive mode.
</summary>
]]--
native "SET_ENTITY_NO_COLLISION_ENTITY"
	hash "0xA53ED5520C07654A"
	jhash (0x1E11BFE9)
	arguments {
		Entity "entity1",

		Entity "entity2",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ENTITY_MOTION_BLUR"
	hash "0x295D82A8559F9150"
	jhash (0xE90005B8)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	p1 always false. 
</summary>
]]--
native "0xE12ABE5E3A389A6C"
	hash "0xE12ABE5E3A389A6C"
	jhash (0x44767B31)
	arguments {
		Entity "entity",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	p1 always false. 
</summary>
]]--
native "0xA80AE305E0A3044F"
	hash "0xA80AE305E0A3044F"
	jhash (0xE224A6A5)
	arguments {
		Entity "entity",

		BOOL "p1",
	}
	returns	"void"

native "0xDC6F8601FAF2E893"
	hash "0xDC6F8601FAF2E893"
	arguments {
		Entity "entity",

		BOOL "p1",
	}
	returns	"void"

native "0x2C2E3DC128F44309"
	hash "0x2C2E3DC128F44309"
	arguments {
		Entity "entity",

		BOOL "p1",
	}
	returns	"void"

native "0x1A092BB0C3808B96"
	hash "0x1A092BB0C3808B96"
	arguments {
		Entity "entity",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	vb.net
	Dim p As Vector3 = Game.Player.Character.Position
	Native.Function.Call(Hash.CREATE_PED,1,New Model(PedHash.Abigail),
	 p.x,p.y,p.z,0.0,false,true)

	*Heading*: 0.0
	*Heading* is the Z axis spawn rotation of the ped 0-&gt;5th parameter.

	Ped Types:
	[Player,1|Male,4|Female,5|Cop,6|Human,26|SWAT,27|Animal,28|Army,29]
	You can also use GET_PED_TYPE
</summary>
]]--
native "CREATE_PED"
	hash "0xD49F9B0955C367DE"
	jhash (0x0389EF71)
	arguments {
		int "pedType",

		Hash "modelHash",

		float "x",

		float "y",

		float "z",

		float "heading",

		BOOL "networkHandle",

		BOOL "pedHandle",
	}
	returns	"Ped"

--[[!
<summary>
	Deletes the specified ped, then sets the handle pointed to by the pointer to NULL.
</summary>
]]--
native "DELETE_PED"
	hash "0x9614299DCB53E54B"
	jhash (0x13EFB9A0)
	arguments {
		Player "ped",
	}
	returns	"void"

--[[!
<summary>
	Clone a Ped of your choice.

	Example of Cloning Your Player:
	CLONE_PED(PLAYER_PED_ID(), GET_ENTITY_HEADING(PLAYER_PED_ID()), 0, 1);
</summary>
]]--
native "CLONE_PED"
	hash "0xEF29A16337FACADB"
	jhash (0x8C8A8D6E)
	arguments {
		Ped "ped",

		float "heading",

		BOOL "networkHandle",

		BOOL "pedHandle",
	}
	returns	"Ped"

native "_ASSIGN_PLAYER_TO_PED"
	hash "0xE952D6431689AD9A"
	jhash (0xFC70EEC7)
	arguments {
		Player "player",

		Ped "ped",
	}
	returns	"void"

--[[!
<summary>
	Gets a value indicating whether the specified ped is in the specified vehicle.

	If 'atGetIn' is false, the function will not return true until the ped is sitting in the vehicle and is about to close the door. If it's true, the function returns true the moment the ped starts to get onto the seat (after opening the door). Eg. if false, and the ped is getting into a submersible, the function will not return true until the ped has descended down into the submersible and gotten into the seat, while if it's true, it'll return true the moment the hatch has been opened and the ped is about to descend into the submersible.
</summary>
]]--
native "IS_PED_IN_VEHICLE"
	hash "0xA3EE4A07279BB9DB"
	jhash (0x7DA6BC83)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		BOOL "atGetIn",
	}
	returns	"BOOL"

native "IS_PED_IN_MODEL"
	hash "0x796D90EFB19AA332"
	jhash (0xA6438D4B)
	arguments {
		Ped "ped",

		Hash "modelHash",
	}
	returns	"BOOL"

--[[!
<summary>
	Gets a value indicating whether the specified ped is in any vehicle.

	If 'atGetIn' is false, the function will not return true until the ped is sitting in the vehicle and is about to close the door. If it's true, the function returns true the moment the ped starts to get onto the seat (after opening the door). Eg. if false, and the ped is getting into a submersible, the function will not return true until the ped has descended down into the submersible and gotten into the seat, while if it's true, it'll return true the moment the hatch has been opened and the ped is about to descend into the submersible.
</summary>
]]--
native "IS_PED_IN_ANY_VEHICLE"
	hash "0x997ABD671D25CA0B"
	jhash (0x3B0171EE)
	arguments {
		Ped "ped",

		BOOL "atGetIn",
	}
	returns	"BOOL"

--[[!
<summary>
	xyz - relative to the world origin.
</summary>
]]--
native "IS_COP_PED_IN_AREA_3D"
	hash "0x16EC4839969F9F5E"
	jhash (0xB98DB96B)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns whether the specified ped is injured.
</summary>
]]--
native "IS_PED_INJURED"
	hash "0x84A2DD9AC37C35C1"
	jhash (0x2530A087)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns whether the specified ped is hurt.
</summary>
]]--
native "IS_PED_HURT"
	hash "0x5983BB449D7FDB12"
	jhash (0x69DFA0AF)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	dzzqd
</summary>
]]--
native "IS_PED_FATALLY_INJURED"
	hash "0xD839450756ED5A80"
	jhash (0xBADA0093)
	arguments {
		Vehicle "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	Seems to consistently return true if the ped is dead.

	p1 is always passed 1 in the scripts.

	I suggest to remove "OR_DYING" part, because it does not detect dying phase.   // gtaVmod

	That's what the devs call it, cry about it. // gir489
</summary>
]]--
native "IS_PED_DEAD_OR_DYING"
	hash "0x3317DEDB88C95038"
	jhash (0xCBDB7739)
	arguments {
		Ped "ped",

		BOOL "p1",
	}
	returns	"BOOL"

native "IS_CONVERSATION_PED_DEAD"
	hash "0xE0A0AEC214B1FABA"
	jhash (0x1FA39EFE)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_AIMING_FROM_COVER"
	hash "0x3998B1276A3300E5"
	jhash (0xDEBAB2AF)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns whether the specified ped is reloading.
</summary>
]]--
native "IS_PED_RELOADING"
	hash "0x24B100C68C645951"
	jhash (0x961E1745)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_A_PLAYER"
	hash "0x12534C348C6CB68B"
	jhash (0x404794CA)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	Ped Types:
	Player = 1
	Male = 4 
	Female = 5 
	Cop = 6
	Human = 26
	SWAT = 27 
	Animal = 28
	Army = 29
</summary>
]]--
native "CREATE_PED_INSIDE_VEHICLE"
	hash "0x7DD959874C1FD534"
	jhash (0x3000F092)
	arguments {
		Vehicle "vehicle",

		int "pedType",

		Hash "modelHash",

		int "seat",

		BOOL "NetworkHandle",

		BOOL "PedHandle",
	}
	returns	"Ped"

native "SET_PED_DESIRED_HEADING"
	hash "0xAA5A7ECE2AA8FE70"
	jhash (0x961458F9)
	arguments {
		Ped "ped",

		float "heading",
	}
	returns	"void"

native "0xFF287323B0E2C69A"
	hash "0xFF287323B0E2C69A"
	jhash (0x290421BE)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	angle is ped1's view cone
</summary>
]]--
native "IS_PED_FACING_PED"
	hash "0xD71649DB0A545AA3"
	jhash (0x0B775838)
	arguments {
		Ped "ped1",

		Player "ped2",

		float "angle",
	}
	returns	"BOOL"

--[[!
<summary>
	Notes: The function only returns true while the ped is: 
	A.) Swinging a random melee attack (including pistol-whipping)

	B.) Reacting to being hit by a melee attack (including pistol-whipping)

	C.) Is locked-on to an enemy (arms up, strafing/skipping in the default fighting-stance, ready to dodge+counter). 

	You don't have to be holding the melee-targetting button to be in this stance; you stay in it by default for a few seconds after swinging at someone. If you do a sprinting punch, it returns true for the duration of the punch animation and then returns false again, even if you've punched and made-angry many peds
</summary>
]]--
native "IS_PED_IN_MELEE_COMBAT"
	hash "0x4E209B2C1EAD5159"
	jhash (0xFD7814A5)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	This could possibly represent whether the specified ped stands still.
</summary>
]]--
native "IS_PED_STOPPED"
	hash "0x530944F6F4B8A214"
	jhash (0xA0DC0B87)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_SHOOTING_IN_AREA"
	hash "0x7E9DFE24AC1E58EF"
	jhash (0x741BF04F)
	arguments {
		Ped "ped",

		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		BOOL "p7",

		BOOL "p8",
	}
	returns	"BOOL"

native "IS_ANY_PED_SHOOTING_IN_AREA"
	hash "0xA0D3D71EA1086C55"
	jhash (0x91833867)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns whether the specified ped is shooting.
</summary>
]]--
native "IS_PED_SHOOTING"
	hash "0x34616828CD07F1A1"
	jhash (0xE7C3405E)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	accuracy = 0-100, 100 being perfectly accurate
</summary>
]]--
native "SET_PED_ACCURACY"
	hash "0x7AEFB85C1D49DEB6"
	jhash (0x6C17122E)
	arguments {
		Ped "ped",

		int "accuracy",
	}
	returns	"Any"

native "GET_PED_ACCURACY"
	hash "0x37F4AD56ECBC0CD6"
	jhash (0x0A2A0AA0)
	arguments {
		Ped "ped",
	}
	returns	"int"

native "IS_PED_MODEL"
	hash "0xC9D55B1A358A5BF7"
	jhash (0x5F1DDFCB)
	arguments {
		Ped "ped",

		Hash "modelHash",
	}
	returns	"BOOL"

--[[!
<summary>
	Forces the ped to fall back and kills it.
</summary>
]]--
native "EXPLODE_PED_HEAD"
	hash "0x2D05CED3A38D0F3A"
	jhash (0x05CC1380)
	arguments {
		Ped "ped",

		Hash "weaponHash",
	}
	returns	"void"

--[[!
<summary>
	I thought this would delete a ped when off-screen or something but it doesn't seem to do anything?
</summary>
]]--
native "REMOVE_PED_ELEGANTLY"
	hash "0xAC6D445B994DF95E"
	jhash (0x4FFB8C6C)
	arguments {
		PedPtr "ped",
	}
	returns	"void"

native "ADD_ARMOUR_TO_PED"
	hash "0x5BA652A0CD14DF2F"
	jhash (0xF686B26E)
	arguments {
		Ped "ped",

		int "amount",
	}
	returns	"void"

native "SET_PED_ARMOUR"
	hash "0xCEA04D83135264CC"
	jhash (0x4E3A0CC4)
	arguments {
		Ped "ped",

		int "amount",
	}
	returns	"void"

--[[!
<summary>
	Ped: The ped to warp.
	vehicle: The vehicle to warp the ped into.
	Seat_Index: [-1 is driver seat, -2 first free passenger seat]
</summary>
]]--
native "SET_PED_INTO_VEHICLE"
	hash "0xF75B0D629E1C063D"
	jhash (0x07500C79)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		int "seatIndex",
	}
	returns	"void"

native "SET_PED_ALLOW_VEHICLES_OVERRIDE"
	hash "0x3C028C636A414ED9"
	jhash (0x58A80BD5)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "CAN_CREATE_RANDOM_PED"
	hash "0x3E8349C08E4B82E4"
	jhash (0xF9ABE88F)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	vb.net
	Dim ped_handle As Integer
	                    With Game.Player.Character
	                        Dim pos As Vector3 = .Position + .ForwardVector * 3
	                        ped_handle = Native.Function.Call(Of Integer)(Hash.CREATE_RANDOM_PED, pos.X, pos.Y, pos.Z)
	                    End With

	Creates a Ped at the specified location, returns the Ped Handle.  
	Ped will not act until SET_PED_AS_NO_LONGER_NEEDED is called.
</summary>
]]--
native "CREATE_RANDOM_PED"
	hash "0xB4AC7D0CF06BFE8F"
	jhash (0x5A949543)
	arguments {
		float "posX",

		float "posY",

		float "posZ",
	}
	returns	"Ped"

native "CREATE_RANDOM_PED_AS_DRIVER"
	hash "0x9B62392B474F44A0"
	jhash (0xB927CE9A)
	arguments {
		Vehicle "vehicle",

		BOOL "returnHandle",
	}
	returns	"Ped"

native "CAN_CREATE_RANDOM_DRIVER"
	hash "0xB8EB95E5B4E56978"
	jhash (0x99861609)
	returns	"BOOL"

native "CAN_CREATE_RANDOM_BIKE_RIDER"
	hash "0xEACEEDA81751915C"
	jhash (0x7018BE31)
	returns	"BOOL"

native "SET_PED_MOVE_ANIMS_BLEND_OUT"
	hash "0x9E8C908F41584ECD"
	jhash (0x20E01957)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "SET_PED_CAN_BE_DRAGGED_OUT"
	hash "0xC1670E958EEE24E5"
	jhash (0xAA7F1131)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "0xF2BEBCDFAFDAA19E"
	hash "0xF2BEBCDFAFDAA19E"
	jhash (0x6CD58238)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Returns true/false if the ped is/isn't male.
</summary>
]]--
native "IS_PED_MALE"
	hash "0x6D9F5FAA7488BA46"
	jhash (0x90950455)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns true/false if the ped is/isn't humanoid.
</summary>
]]--
native "IS_PED_HUMAN"
	hash "0xB980061DA992779D"
	jhash (0x194BB7B0)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	Gets the vehicle the specified Ped is/was in depending on bool value.

	[False = CurrentVehicle, True = LastVehicle]
</summary>
]]--
native "GET_VEHICLE_PED_IS_IN"
	hash "0x9A9112A0FE9A4713"
	jhash (0xAFE92319)
	arguments {
		Ped "ped",

		BOOL "getLastVehicle",
	}
	returns	"Vehicle"

--[[!
<summary>
	Resets the value for the last vehicle driven by the Ped.
</summary>
]]--
native "RESET_PED_LAST_VEHICLE"
	hash "0xBB8DE8CF6A8DD8BB"
	jhash (0x5E3B5942)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "SET_PED_DENSITY_MULTIPLIER_THIS_FRAME"
	hash "0x95E3D6257B166CF2"
	jhash (0x039C82BB)
	arguments {
		float "multiplier",
	}
	returns	"void"

native "SET_SCENARIO_PED_DENSITY_MULTIPLIER_THIS_FRAME"
	hash "0x7A556143A1C03898"
	jhash (0x2909ABF0)
	arguments {
		float "p0",

		float "p1",
	}
	returns	"void"

native "0x5A7F62FDA59759BD"
	hash "0x5A7F62FDA59759BD"
	jhash (0xB48C0C04)
	returns	"void"

native "SET_SCRIPTED_CONVERSION_COORD_THIS_FRAME"
	hash "0x5086C7843552CF85"
	jhash (0x25EA2AA5)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

--[[!
<summary>
	The distance between these points, is the diagonal of a box (remember it's 3D).
</summary>
]]--
native "SET_PED_NON_CREATION_AREA"
	hash "0xEE01041D559983EA"
	jhash (0x7A97283F)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",
	}
	returns	"void"

native "CLEAR_PED_NON_CREATION_AREA"
	hash "0x2E05208086BA0651"
	jhash (0x6F7043A3)
	returns	"void"

native "0x4759CC730F947C81"
	hash "0x4759CC730F947C81"
	jhash (0x8C555ADD)
	returns	"void"

native "IS_PED_ON_MOUNT"
	hash "0x460BC76A0E10655E"
	jhash (0x43103006)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "GET_MOUNT"
	hash "0xE7E11B8DCBED1058"
	jhash (0xDD31EC4E)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	Gets a value indicating whether the specified ped is on top of any vehicle.

	Return 1 when ped is on vehicle.
	Return 0 when ped is not on a vehicle.

</summary>
]]--
native "IS_PED_ON_VEHICLE"
	hash "0x67722AEB798E5FAB"
	jhash (0xA1AE7CC7)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_ON_SPECIFIC_VEHICLE"
	hash "0xEC5F66E459AF3BB2"
	jhash (0x63CB4603)
	arguments {
		Ped "ped",

		Vehicle "vehicle",
	}
	returns	"BOOL"

native "SET_PED_MONEY"
	hash "0xA9C8960E8684C1B5"
	jhash (0x40D90BF2)
	arguments {
		Ped "ped",

		int "amount",
	}
	returns	"void"

native "GET_PED_MONEY"
	hash "0x3F69145BBA87BAE7"
	jhash (0xEB3C4C7E)
	arguments {
		Ped "ped",
	}
	returns	"int"

native "0xFF4803BC019852D9"
	hash "0xFF4803BC019852D9"
	jhash (0xD41C9AED)
	arguments {
		float "p0",

		Any "p1",
	}
	returns	"void"

native "0x6B0E6172C9A4D902"
	hash "0x6B0E6172C9A4D902"
	jhash (0x30B98369)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x9911F4A24485F653"
	hash "0x9911F4A24485F653"
	jhash (0x02A080C8)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	ped p0 cannot be headshot if this is set to false
</summary>
]]--
native "SET_PED_SUFFERS_CRITICAL_HITS"
	hash "0xEBD76F2359F190AC"
	jhash (0x6F6FC7E6)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "0xAFC976FD0580C7B3"
	hash "0xAFC976FD0580C7B3"
	jhash (0x1572022A)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Detect if ped is sitting in the specified vehicle
	[True/False]
</summary>
]]--
native "IS_PED_SITTING_IN_VEHICLE"
	hash "0xA808AA1D79230FC2"
	jhash (0xDDDE26FA)
	arguments {
		Ped "ped",

		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	Detect if ped is in any vehicle
	[True/False]
</summary>
]]--
native "IS_PED_SITTING_IN_ANY_VEHICLE"
	hash "0x826AA586EDB9FEF8"
	jhash (0x0EA9CA03)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_ON_FOOT"
	hash "0x01FEE67DB37F59B2"
	jhash (0xC60D0785)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_ON_ANY_BIKE"
	hash "0x94495889E22C6479"
	jhash (0x4D885B2E)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_PLANTING_BOMB"
	hash "0xC70B5FAE151982D8"
	jhash (0x0EDAC574)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "GET_DEAD_PED_PICKUP_COORDS"
	hash "0xCD5003B097200F36"
	jhash (0x129F9DC1)
	arguments {
		Any "p0",

		float "p1",

		float "p2",
	}
	returns	"int"

native "IS_PED_IN_ANY_BOAT"
	hash "0x2E0E1C2B4F6CB339"
	jhash (0x1118A947)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_IN_ANY_SUB"
	hash "0xFBFC01CCFB35D99E"
	jhash (0xE65F8059)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_IN_ANY_HELI"
	hash "0x298B91AE825E5705"
	jhash (0x7AB5523B)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_IN_ANY_PLANE"
	hash "0x5FFF4CFC74D8FB80"
	jhash (0x51BBCE7E)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_IN_FLYING_VEHICLE"
	hash "0x9134873537FA419C"
	jhash (0xCA072485)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "SET_PED_DIES_IN_WATER"
	hash "0x56CEF0AC79073BDE"
	jhash (0x604C872B)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_DIES_IN_SINKING_VEHICLE"
	hash "0xD718A22995E2B4BC"
	jhash (0x8D4D9ABB)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "GET_PED_ARMOUR"
	hash "0x9483AF821605B1D8"
	jhash (0x2CE311A7)
	arguments {
		Ped "ped",
	}
	returns	"int"

native "SET_PED_STAY_IN_VEHICLE_WHEN_JACKED"
	hash "0xEDF4079F9D54C9A1"
	jhash (0xB014A09C)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_BE_SHOT_IN_VEHICLE"
	hash "0xC7EF1BA83230BA07"
	jhash (0x5DB7B3A9)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "GET_PED_LAST_DAMAGE_BONE"
	hash "0xD75960F6BD9EA49C"
	jhash (0xAB933841)
	arguments {
		Ped "ped",

		AnyPtr "outBone",
	}
	returns	"BOOL"

native "CLEAR_PED_LAST_DAMAGE_BONE"
	hash "0x8EF6B7AC68E2F01B"
	jhash (0x56CB715E)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "SET_AI_WEAPON_DAMAGE_MODIFIER"
	hash "0x1B1E2A40A65B8521"
	jhash (0x516E30EE)
	arguments {
		float "value",
	}
	returns	"void"

native "RESET_AI_WEAPON_DAMAGE_MODIFIER"
	hash "0xEA16670E7BA4743C"
	jhash (0x6E965420)
	returns	"void"

native "SET_AI_MELEE_WEAPON_DAMAGE_MODIFIER"
	hash "0x66460DEDDD417254"
	jhash (0x0F9A401F)
	arguments {
		float "modifier",
	}
	returns	"void"

native "RESET_AI_MELEE_WEAPON_DAMAGE_MODIFIER"
	hash "0x46E56A7CD1D63C3F"
	jhash (0x97886238)
	returns	"void"

native "0x2F3C3D9F50681DE4"
	hash "0x2F3C3D9F50681DE4"
	jhash (0xCC9D7F1A)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_PED_CAN_BE_TARGETTED"
	hash "0x63F58F7C80513AAD"
	jhash (0x75C49F74)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_BE_TARGETTED_BY_TEAM"
	hash "0xBF1CA77833E58F2C"
	jhash (0xB103A8E1)
	arguments {
		Ped "ped",

		Any "team",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_BE_TARGETTED_BY_PLAYER"
	hash "0x66B57B72E0836A76"
	jhash (0xD050F490)
	arguments {
		Ped "ped",

		Player "player",

		BOOL "toggle",
	}
	returns	"void"

native "0x061CB768363D6424"
	hash "0x061CB768363D6424"
	jhash (0x7DA12905)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_TIME_EXCLUSIVE_DISPLAY_TEXTURE"
	hash "0xFD325494792302D7"
	jhash (0x7F67671D)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_PED_IN_ANY_POLICE_VEHICLE"
	hash "0x0BD04E29640C9C12"
	jhash (0x84FA790D)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "FORCE_PED_TO_OPEN_PARACHUTE"
	hash "0x16E42E800B472221"
	jhash (0xA819680B)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "IS_PED_IN_PARACHUTE_FREE_FALL"
	hash "0x7DCE8BDA0F1C1200"
	jhash (0xCD71F11B)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_FALLING"
	hash "0xFB92A102F1C4DFA3"
	jhash (0xABF77334)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_JUMPING"
	hash "0xCEDABC5900A0BF97"
	jhash (0x07E5BC0E)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_CLIMBING"
	hash "0x53E8CB4F48BFE623"
	jhash (0xBCE03D35)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_VAULTING"
	hash "0x117C70D1F5730B5E"
	jhash (0xC3169BDA)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_DIVING"
	hash "0x5527B8246FEF9B11"
	jhash (0x7BC5BF3C)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_JUMPING_OUT_OF_VEHICLE"
	hash "0x433DDFFE2044B636"
	jhash (0xB19215F6)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "0x26AF0E8E30BD2A2C"
	hash "0x26AF0E8E30BD2A2C"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns:

	-1: Normal
	0: Wearing parachute on back
	1: Parachute opening
	2: Parachute open
	3: Falling to doom (e.g. after exiting parachute)
</summary>
]]--
native "GET_PED_PARACHUTE_STATE"
	hash "0x79CFD9827CC979B6"
	jhash (0x7D4BC475)
	arguments {
		Ped "ped",
	}
	returns	"int"

native "GET_PED_PARACHUTE_LANDING_TYPE"
	hash "0x8B9F1FC6AE8166C0"
	jhash (0x01F3B035)
	arguments {
		Ped "ped",
	}
	returns	"int"

native "SET_PED_PARACHUTE_TINT_INDEX"
	hash "0x333FC8DB079B7186"
	jhash (0x5AEFEC3A)
	arguments {
		Ped "ped",

		Any "tintIndex",
	}
	returns	"void"

native "GET_PED_PARACHUTE_TINT_INDEX"
	hash "0xEAF5F7E5AE7C6C9D"
	jhash (0xE9E7FAC5)
	arguments {
		Ped "ped",

		AnyPtr "outTintIndex",
	}
	returns	"void"

native "SET_PED_RESERVE_PARACHUTE_TINT_INDEX"
	hash "0xE88DA0751C22A2AD"
	jhash (0x177EFC79)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x8C4F3BF23B6237DB"
	hash "0x8C4F3BF23B6237DB"
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"Any"

native "SET_PED_DUCKING"
	hash "0x030983CA930B692D"
	jhash (0xB90353D7)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "IS_PED_DUCKING"
	hash "0xD125AE748725C6BC"
	jhash (0x9199C77D)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_IN_ANY_TAXI"
	hash "0x6E575D6A898AB852"
	jhash (0x16FD386C)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "SET_PED_ID_RANGE"
	hash "0xF107E836A70DCE05"
	jhash (0xEF3B4ED9)
	arguments {
		Ped "ped",

		float "value",
	}
	returns	"void"

native "0x52D59AB61DDC05DD"
	hash "0x52D59AB61DDC05DD"
	jhash (0x9A2180FF)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xEC4B4B3B9908052A"
	hash "0xEC4B4B3B9908052A"
	jhash (0xF30658D2)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x733C87D4CE22BEA2"
	hash "0x733C87D4CE22BEA2"
	jhash (0x43709044)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_PED_SEEING_RANGE"
	hash "0xF29CF591C4BF6CEE"
	jhash (0x4BD72FE8)
	arguments {
		Ped "ped",

		float "value",
	}
	returns	"void"

native "SET_PED_HEARING_RANGE"
	hash "0x33A8F7F7D5F7F33C"
	jhash (0xB32087E0)
	arguments {
		Ped "ped",

		float "value",
	}
	returns	"void"

native "SET_PED_VISUAL_FIELD_MIN_ANGLE"
	hash "0x2DB492222FB21E26"
	jhash (0x72E2E18B)
	arguments {
		Ped "ped",

		float "value",
	}
	returns	"void"

native "SET_PED_VISUAL_FIELD_MAX_ANGLE"
	hash "0x70793BDCA1E854D4"
	jhash (0x0CEA0F9A)
	arguments {
		Ped "ped",

		float "value",
	}
	returns	"void"

--[[!
<summary>
	This native refers to the field of vision the ped has below them, starting at 0 degrees. The angle value should be negative.
</summary>
]]--
native "SET_PED_VISUAL_FIELD_MIN_ELEVATION_ANGLE"
	hash "0x7A276EB2C224D70F"
	jhash (0x5CC2F1B8)
	arguments {
		Ped "ped",

		float "angle",
	}
	returns	"void"

--[[!
<summary>
	This native refers to the field of vision the ped has above them, starting at 0 degrees. 90f would let the ped see enemies directly above of them.
</summary>
]]--
native "SET_PED_VISUAL_FIELD_MAX_ELEVATION_ANGLE"
	hash "0x78D0B67629D75856"
	jhash (0x39D9102F)
	arguments {
		Ped "ped",

		float "angle",
	}
	returns	"void"

native "SET_PED_VISUAL_FIELD_PERIPHERAL_RANGE"
	hash "0x9C74B0BC831B753A"
	jhash (0xFDF2F7C2)
	arguments {
		Ped "ped",

		float "range",
	}
	returns	"void"

native "SET_PED_VISUAL_FIELD_CENTER_ANGLE"
	hash "0x3B6405E8AB34A907"
	jhash (0xE57202A1)
	arguments {
		Ped "ped",

		float "angle",
	}
	returns	"void"

--[[!
<summary>
	p1 is usually 0 in the scripts. action is either 0 or a pointer to "DEFAULT_ACTION".
</summary>
]]--
native "SET_PED_STEALTH_MOVEMENT"
	hash "0x88CBB5CEB96B7BD2"
	jhash (0x67E28E1D)
	arguments {
		Ped "ped",

		BOOL "p1",

		charPtr "action",
	}
	returns	"void"

native "GET_PED_STEALTH_MOVEMENT"
	hash "0x7C2AC9CA66575FBF"
	jhash (0x40321B83)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	Creates a new ped group.
	Groups can contain up to 8 peds.

	The parameter is unused.

	Returns a handle to the created group, or 0 if a group couldn't be created.
</summary>
]]--
native "CREATE_GROUP"
	hash "0x90370EBE0FEE1A3D"
	jhash (0x8DC0368D)
	arguments {
		int "unused",
	}
	returns	"int"

native "SET_PED_AS_GROUP_LEADER"
	hash "0x2A7819605465FBCE"
	jhash (0x7265BEA2)
	arguments {
		Ped "ped",

		int "groupId",
	}
	returns	"void"

native "SET_PED_AS_GROUP_MEMBER"
	hash "0x9F3480FE65DB31B5"
	jhash (0x0EE13F92)
	arguments {
		Ped "ped",

		int "groupId",
	}
	returns	"void"

native "SET_PED_CAN_TELEPORT_TO_GROUP_LEADER"
	hash "0x2E2F4240B3F24647"
	jhash (0xD0D8BDBC)
	arguments {
		Ped "pedHandle",

		int "groupHandle",

		BOOL "p2",
	}
	returns	"void"

native "REMOVE_GROUP"
	hash "0x8EB2F69076AF7053"
	jhash (0x48D72B88)
	arguments {
		int "groupId",
	}
	returns	"void"

native "REMOVE_PED_FROM_GROUP"
	hash "0xED74007FFB146BC2"
	jhash (0x82697713)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "IS_PED_GROUP_MEMBER"
	hash "0x9BB01E3834671191"
	jhash (0x876D5363)
	arguments {
		Ped "ped",

		int "groupId",
	}
	returns	"BOOL"

native "IS_PED_HANGING_ON_TO_VEHICLE"
	hash "0x1C86D8AEF8254B78"
	jhash (0x9678D4FF)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	Sets the range at which members will automatically leave the group.
</summary>
]]--
native "SET_GROUP_SEPARATION_RANGE"
	hash "0x4102C7858CFEE4E4"
	jhash (0x7B820CD5)
	arguments {
		int "groupHandle",

		float "separationRange",
	}
	returns	"void"

native "SET_PED_MIN_GROUND_TIME_FOR_STUNGUN"
	hash "0xFA0675AB151073FA"
	jhash (0x2F0D0973)
	arguments {
		Ped "ped",

		int "ms",
	}
	returns	"void"

native "IS_PED_PRONE"
	hash "0xD6A86331A537A7B9"
	jhash (0x02C2A6C3)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	Checks to see if ped and target are in combat with eachother. Only goes one-way: if target is engaged in combat with ped but ped has not yet reacted, the function will return false until ped starts fighting back.

	p1 is usually 0 in the scripts because it gets the ped id during the task sequence. For instance: PED::IS_PED_IN_COMBAT(l_42E[4/*14*/], PLAYER::PLAYER_PED_ID()) // armenian2.ct4: 43794
</summary>
]]--
native "IS_PED_IN_COMBAT"
	hash "0x4859F1FC66A6278E"
	jhash (0xFE027CB5)
	arguments {
		Ped "ped",

		Ped "target",
	}
	returns	"BOOL"

native "CAN_PED_IN_COMBAT_SEE_TARGET"
	hash "0xEAD42DE3610D0721"
	jhash (0xCCD525E1)
	arguments {
		Ped "ped",

		Ped "target",
	}
	returns	"BOOL"

native "IS_PED_DOING_DRIVEBY"
	hash "0xB2C086CC1BF8F2BF"
	jhash (0xAC3CEB9C)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_JACKING"
	hash "0x4AE4FF911DFB61DA"
	jhash (0x3B321816)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_BEING_JACKED"
	hash "0x9A497FE2DF198913"
	jhash (0xD45D605C)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	p1 is always 0
</summary>
]]--
native "IS_PED_BEING_STUNNED"
	hash "0x4FBACCE3B4138EE8"
	jhash (0x0A66CE30)
	arguments {
		Ped "ped",

		int "p1",
	}
	returns	"BOOL"

native "GET_PEDS_JACKER"
	hash "0x9B128DC36C1E04CF"
	jhash (0xDE1DBB59)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_JACK_TARGET"
	hash "0x5486A79D9FBD342D"
	jhash (0x1D196361)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "IS_PED_FLEEING"
	hash "0xBBCCE00B381F8482"
	jhash (0x85D813C6)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_IN_COVER"
	hash "0x60DFD0691A170B88"
	jhash (0x972C5A8B)
	arguments {
		Ped "ped",

		BOOL "p1",
	}
	returns	"BOOL"

native "IS_PED_IN_COVER_FACING_LEFT"
	hash "0x845333B3150583AB"
	jhash (0xB89DBB80)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "0x6A03BF943D767C93"
	hash "0x6A03BF943D767C93"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "IS_PED_GOING_INTO_COVER"
	hash "0x9F65DBC537E59AD5"
	jhash (0xA3589628)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "SET_PED_PINNED_DOWN"
	hash "0xAAD6D1ACF08F4612"
	jhash (0xCC78999D)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "GET_SEAT_PED_IS_TRYING_TO_ENTER"
	hash "0x6F4C85ACD641BCD2"
	jhash (0xACF162E0)
	arguments {
		Ped "ped",
	}
	returns	"int"

native "GET_VEHICLE_PED_IS_TRYING_TO_ENTER"
	hash "0x814FA8BE5449445D"
	jhash (0x99968B37)
	arguments {
		Ped "ped",
	}
	returns	"Vehicle"

--[[!
<summary>
	Returns the Entity (Ped, Vehicle, or ?Object?) that killed the 'ped'

	Is best to check if the Ped is dead before asking for it's killer.
</summary>
]]--
native "_GET_PED_KILLER"
	hash "0x93C8B64DEB84728C"
	jhash (0x84ADF9EB)
	arguments {
		Ped "ped",
	}
	returns	"Entity"

--[[!
<summary>
	Returns the hash of the weapon/model/object that killed the ped.
</summary>
]]--
native "GET_PED_CAUSE_OF_DEATH"
	hash "0x16FFE42AB2D2DC59"
	jhash (0x63458C27)
	arguments {
		Ped "ped",
	}
	returns	"Hash"

--[[!
<summary>
	TODO: add hash from x360
</summary>
]]--
native "_GET_PED_TIME_OF_DEATH"
	hash "0x1E98817B311AE98A"
	arguments {
		Ped "ped",
	}
	returns	"int"

native "0x5407B7288D0478B7"
	hash "0x5407B7288D0478B7"
	jhash (0xEF0B78E6)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x336B3D200AB007CB"
	hash "0x336B3D200AB007CB"
	jhash (0xFB18CB19)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"Any"

native "SET_PED_RELATIONSHIP_GROUP_DEFAULT_HASH"
	hash "0xADB3F206518799E8"
	jhash (0x423B7BA2)
	arguments {
		Ped "ped",

		Hash "hash",
	}
	returns	"void"

--[[!
<summary>
	SET_PED_RELATIONSHIP_GROUP_HASH(Ped peds, Hash hash)
</summary>
]]--
native "SET_PED_RELATIONSHIP_GROUP_HASH"
	hash "0xC80A74AC829DDD92"
	jhash (0x79F8C18C)
	arguments {
		Ped "ped",

		Hash "hash",
	}
	returns	"void"

--[[!
<summary>
	Sets the relationship between two groups. This should be called twice (once for each group).

	Relationship types:
	0 = Companion
	1 = Respect
	2 = Like
	3 = Neutral
	4 = Dislike
	5 = Hate
	255 = Pedestrians

	Example:
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(2, l_1017, 0xA49E591C);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(2, 0xA49E591C, l_1017);
</summary>
]]--
native "SET_RELATIONSHIP_BETWEEN_GROUPS"
	hash "0xBF25EB89375A37AD"
	jhash (0xD4A215BA)
	arguments {
		int "relationship",

		Hash "group1",

		Hash "group2",
	}
	returns	"void"

--[[!
<summary>
	Clears the relationship between two groups. This should be called twice (once for each group).

	Relationship types:
	0 = Companion
	1 = Respect
	2 = Like
	3 = Neutral
	4 = Dislike
	5 = Hate
	255 = Pedestrians
	(Credits: Inco)

	Example:
	PED::CLEAR_RELATIONSHIP_BETWEEN_GROUPS(2, l_1017, 0xA49E591C);
	PED::CLEAR_RELATIONSHIP_BETWEEN_GROUPS(2, 0xA49E591C, l_1017);
</summary>
]]--
native "CLEAR_RELATIONSHIP_BETWEEN_GROUPS"
	hash "0x5E29243FB56FC6D4"
	jhash (0x994B8C2D)
	arguments {
		int "relationship",

		Hash "group1",

		Hash "group2",
	}
	returns	"void"

--[[!
<summary>
	Can't select void. This function returns nothing. The hash of the created relationship group is output in the second parameter.
</summary>
]]--
native "ADD_RELATIONSHIP_GROUP"
	hash "0xF372BC22FCB88606"
	jhash (0x8B635546)
	arguments {
		charPtr "name",

		HashPtr "groupHash",
	}
	returns	"Any"

native "REMOVE_RELATIONSHIP_GROUP"
	hash "0xB6BA2444AB393DA2"
	jhash (0x4A1DC59A)
	arguments {
		Hash "groupHash",
	}
	returns	"void"

--[[!
<summary>
	Gets the relationship between two peds. This should be called twice (once for each ped).

	Relationship types:
	0 = Companion
	1 = Respect
	2 = Like
	3 = Neutral
	4 = Dislike
	5 = Hate
	255 = Pedestrians
	(Credits: Inco)

	Example:
	PED::GET_RELATIONSHIP_BETWEEN_PEDS(2, l_1017, 0xA49E591C);
	PED::GET_RELATIONSHIP_BETWEEN_PEDS(2, 0xA49E591C, l_1017);
</summary>
]]--
native "GET_RELATIONSHIP_BETWEEN_PEDS"
	hash "0xEBA5AD3A0EAF7121"
	jhash (0xE254C39C)
	arguments {
		Ped "ped1",

		Ped "ped2",
	}
	returns	"int"

native "GET_PED_RELATIONSHIP_GROUP_DEFAULT_HASH"
	hash "0x42FDD0F017B1E38E"
	jhash (0x714BD6E4)
	arguments {
		Ped "ped",
	}
	returns	"Hash"

native "GET_PED_RELATIONSHIP_GROUP_HASH"
	hash "0x7DBDD04862D95F04"
	jhash (0x354F283C)
	arguments {
		Ped "ped",
	}
	returns	"Hash"

--[[!
<summary>
	Gets the relationship between two groups. This should be called twice (once for each group).

	Relationship types:
	0 = Companion
	1 = Respect
	2 = Like
	3 = Neutral
	4 = Dislike
	5 = Hate
	255 = Pedestrians
	(Credits: Inco)

	Example:
	PED::GET_RELATIONSHIP_BETWEEN_GROUPS(l_1017, 0xA49E591C);
	PED::GET_RELATIONSHIP_BETWEEN_GROUPS(0xA49E591C, l_1017);
</summary>
]]--
native "GET_RELATIONSHIP_BETWEEN_GROUPS"
	hash "0x9E6B70061662AE5C"
	jhash (0x4E372FE2)
	arguments {
		Hash "group1",

		Hash "group2",
	}
	returns	"int"

native "SET_PED_CAN_BE_TARGETED_WITHOUT_LOS"
	hash "0x4328652AE5769C71"
	jhash (0x7FDDC0A6)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	p1 could be radius
</summary>
]]--
native "SET_PED_TO_INFORM_RESPECTED_FRIENDS"
	hash "0x112942C6E708F70B"
	jhash (0xD78AC46C)
	arguments {
		Ped "ped",

		float "p1",

		Any "p2",
	}
	returns	"void"

native "IS_PED_RESPONDING_TO_EVENT"
	hash "0x625B774D75C87068"
	jhash (0x7A877554)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	FIRING_PATTERN_BURST_FIRE = 0xD6FF6D61 ( 1073727030 )
	FIRING_PATTERN_BURST_FIRE_IN_COVER = 0x026321F1 ( 40051185 )
	FIRING_PATTERN_BURST_FIRE_DRIVEBY = 0xD31265F2 ( -753768974 )
	FIRING_PATTERN_FROM_GROUND = 0x2264E5D6 ( 577037782 )
	FIRING_PATTERN_DELAY_FIRE_BY_ONE_SEC = 0x7A845691 ( 2055493265 )
	FIRING_PATTERN_FULL_AUTO = 0xC6EE6B4C ( -957453492 )
	FIRING_PATTERN_SINGLE_SHOT = 0x5D60E4E0 ( 1566631136 )
	FIRING_PATTERN_BURST_FIRE_PISTOL = 0xA018DB8A ( -1608983670 )
	FIRING_PATTERN_BURST_FIRE_SMG = 0xD10DADEE ( 1863348768 )
	FIRING_PATTERN_BURST_FIRE_RIFLE = 0x9C74B406 ( -1670073338 )
	FIRING_PATTERN_BURST_FIRE_MG = 0xB573C5B4 ( -1250703948 )
	FIRING_PATTERN_BURST_FIRE_PUMPSHOTGUN = 0x00BAC39B ( 12239771 )
	FIRING_PATTERN_BURST_FIRE_HELI = 0x914E786F ( -1857128337 )
	FIRING_PATTERN_BURST_FIRE_MICRO = 0x42EF03FD ( 1122960381 )
	FIRING_PATTERN_SHORT_BURSTS = 0x1A92D7DF ( 445831135 )
	FIRING_PATTERN_SLOW_FIRE_TANK = 0xE2CA3A71 ( -490063247 )
</summary>
]]--
native "SET_PED_FIRING_PATTERN"
	hash "0x9AC577F5A12AD8A9"
	jhash (0xB4629D66)
	arguments {
		Ped "ped",

		Hash "patternHash",
	}
	returns	"Any"

--[[!
<summary>
	shootRate 0-1000
</summary>
]]--
native "SET_PED_SHOOT_RATE"
	hash "0x614DA022990752DC"
	jhash (0xFB301746)
	arguments {
		Ped "ped",

		int "shootRate",
	}
	returns	"void"

--[[!
<summary>
	combatType can be between 0-14
</summary>
]]--
native "SET_COMBAT_FLOAT"
	hash "0xFF41B4B141ED981C"
	jhash (0xD8B7637C)
	arguments {
		Ped "ped",

		int "combatType",

		float "p2",
	}
	returns	"void"

--[[!
<summary>
	p0: Ped Handle
	p1: int i | 0 &lt;= i &lt;= 27
</summary>
]]--
native "GET_COMBAT_FLOAT"
	hash "0x52DFF8A10508090A"
	jhash (0x511D7EF8)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"float"

--[[!
<summary>
	p1 may be a BOOL representing whether or not the group even exists
</summary>
]]--
native "GET_GROUP_SIZE"
	hash "0x8DE69FE35CA09A45"
	jhash (0xF7E1A691)
	arguments {
		int "groupID",

		AnyPtr "unknown",

		intPtr "sizeInMembers",
	}
	returns	"void"

native "DOES_GROUP_EXIST"
	hash "0x7C6B0C22F9F40BBE"
	jhash (0x935C978D)
	arguments {
		int "groupId",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns the group id of which the specified ped is a member of.
</summary>
]]--
native "GET_PED_GROUP_INDEX"
	hash "0xF162E133B4E7A675"
	jhash (0x134E0785)
	arguments {
		Ped "ped",
	}
	returns	"int"

native "IS_PED_IN_GROUP"
	hash "0x5891CAC5D4ACFF74"
	jhash (0x836D9795)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "GET_PLAYER_PED_IS_FOLLOWING"
	hash "0x6A3975DEA89F9A17"
	jhash (0xDE7442EE)
	arguments {
		Ped "ped",
	}
	returns	"Player"

--[[!
<summary>
	0: Default
	1: Circle Around Leader
	2: Alternative Circle Around Leader
	3: Line, with Leader at center
</summary>
]]--
native "SET_GROUP_FORMATION"
	hash "0xCE2F5FC3AF7E8C1E"
	jhash (0x08FAC739)
	arguments {
		int "groupId",

		int "formationType",
	}
	returns	"void"

native "SET_GROUP_FORMATION_SPACING"
	hash "0x1D9D45004C28C916"
	jhash (0xB1E086FF)
	arguments {
		int "groupId",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "RESET_GROUP_FORMATION_DEFAULT_SPACING"
	hash "0x63DAB4CCB3273205"
	jhash (0x267FCEAD)
	arguments {
		int "groupHandle",
	}
	returns	"void"

--[[!
<summary>
	Gets ID of vehicle player using. It means it can get ID at any interaction with vehicle. Enter\exit for example. And that means it is faster then GET_VEHICLE_PED_IS_IN()
</summary>
]]--
native "GET_VEHICLE_PED_IS_USING"
	hash "0x6094AD011A2EA87D"
	jhash (0x6DE3AADA)
	arguments {
		Ped "ped",
	}
	returns	"Vehicle"

native "SET_EXCLUSIVE_PHONE_RELATIONSHIPS"
	hash "0xF92691AED837A5FC"
	jhash (0x56E0C163)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "SET_PED_GRAVITY"
	hash "0x9FF447B6B6AD960A"
	jhash (0x3CA16652)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "APPLY_DAMAGE_TO_PED"
	hash "0x697157CED63F18D4"
	jhash (0x4DC27FCF)
	arguments {
		Ped "ped",

		Any "damageAmount",

		BOOL "p2",
	}
	returns	"void"

native "0x36B77BB84687C318"
	hash "0x36B77BB84687C318"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "SET_PED_ALLOWED_TO_DUCK"
	hash "0xDA1F1B7BE1A8766F"
	jhash (0xC4D122F8)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_NEVER_LEAVES_GROUP"
	hash "0x3DBFC55D5C9BB447"
	jhash (0x0E038813)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Ped Types: (ordered by return priority)

	Michael = 0
	Franklin = 1
	Trevor = 2

	Army = 29
	Animal = 28
	SWAT = 27
	LSFD = 21
	Paramedic = 20

	Cop = 6

	Male = 4
	Female = 5 

	Human = 26

	Note/Exception
	hc_gunman : 4 // Mix male and female
	hc_hacker : 4 // Mix male and female
	mp_f_misty_01 : 4 // Female character
	s_f_y_ranger_01 : 5 // Ranger
	s_m_y_ranger_01 : 4 // Ranger
	s_m_y_uscg_01 : 6 // US Coast Guard
</summary>
]]--
native "GET_PED_TYPE"
	hash "0xFF059E1E4C01E63C"
	jhash (0xB1460D43)
	arguments {
		Hash "model",
	}
	returns	"int"

--[[!
<summary>
	Turns the desired ped into a cop. If you use this on the player ped, you will become almost invisible to cops dispatched for you. You will also report your own crimes, get a generic cop voice, get a cop-vision-cone on the radar, and you will be unable to shoot at other cops. SWAT and Army will still shoot at you. Toggling ped as "false" has no effect; you must change p0's ped model to disable the effect.
</summary>
]]--
native "SET_PED_AS_COP"
	hash "0xBB03C38DD3FB7FFD"
	jhash (0x84E7DE9F)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_MAX_HEALTH"
	hash "0xF5F6378C4F3419D3"
	jhash (0x5533F60B)
	arguments {
		Ped "ped",

		Any "value",
	}
	returns	"void"

native "GET_PED_MAX_HEALTH"
	hash "0x4700A416E8324EF3"
	jhash (0xA45B6C8D)
	arguments {
		Ped "ped",
	}
	returns	"Any"

native "SET_PED_MAX_TIME_IN_WATER"
	hash "0x43C851690662113D"
	jhash (0xFE0A106B)
	arguments {
		Ped "ped",

		float "value",
	}
	returns	"void"

native "SET_PED_MAX_TIME_UNDERWATER"
	hash "0x6BA428C528D9E522"
	jhash (0x082EF240)
	arguments {
		Ped "ped",

		float "value",
	}
	returns	"void"

native "0x2735233A786B1BEF"
	hash "0x2735233A786B1BEF"
	jhash (0x373CC405)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x952F06BEECD775CC"
	hash "0x952F06BEECD775CC"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0xE6CA85E7259CE16B"
	hash "0xE6CA85E7259CE16B"
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	0 = can (bike)
	1 = can't (bike)
	2 = unk 
	3 = unk
</summary>
]]--
native "SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE"
	hash "0x7A6535691B477C48"
	jhash (0x8A251612)
	arguments {
		Ped "ped",

		int "state",
	}
	returns	"void"

native "CAN_KNOCK_PED_OFF_VEHICLE"
	hash "0x51AC07A44D4F5B8A"
	jhash (0xC9D098B3)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "KNOCK_PED_OFF_VEHICLE"
	hash "0x45BBCBA77C29A841"
	jhash (0xACDD0674)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "SET_PED_COORDS_NO_GANG"
	hash "0x87052FE446E07247"
	jhash (0x9561AD98)
	arguments {
		Ped "ped",

		float "posX",

		float "posY",

		float "posZ",
	}
	returns	"void"

--[[!
<summary>
	from fm_mission_controller.c4 (variable names changed for clarity):

	int groupID = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
	PED::GET_GROUP_SIZE(group, &amp;unused, &amp;groupSize);
	if (groupSize &gt;= 1) {
	. . . . for (int memberNumber = 0; memberNumber &lt; groupSize; memberNumber++) {
	. . . . . . . . Ped ped1 = PED::GET_PED_AS_GROUP_MEMBER(groupID, memberNumber);
	. . . . . . . . //and so on
</summary>
]]--
native "GET_PED_AS_GROUP_MEMBER"
	hash "0x51455483CF23ED97"
	jhash (0x9AA3CC8C)
	arguments {
		int "groupID",

		int "memberNumber",
	}
	returns	"Ped"

native "_GET_PED_AS_GROUP_LEADER"
	hash "0x5CCE68DBD5FE93EC"
	arguments {
		int "groupID",
	}
	returns	"Ped"

native "SET_PED_KEEP_TASK"
	hash "0x971D38760FBC02EF"
	jhash (0xA7EC79CE)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "0x49E50BDB8BA4DAB2"
	hash "0x49E50BDB8BA4DAB2"
	jhash (0x397F06E3)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_PED_SWIMMING"
	hash "0x9DE327631295B4C2"
	jhash (0x7AB43DB8)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_SWIMMING_UNDER_WATER"
	hash "0xC024869A53992F34"
	jhash (0x0E8D524F)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	teleports ped to coords along with the vehicle ped is in
</summary>
]]--
native "SET_PED_COORDS_KEEP_VEHICLE"
	hash "0x9AFEFF481A85AB2E"
	jhash (0xD66AE1D3)
	arguments {
		Ped "ped",

		float "posX",

		float "posY",

		float "posZ",
	}
	returns	"void"

native "SET_PED_DIES_IN_VEHICLE"
	hash "0x2A30922C90C9B42C"
	jhash (0x6FE1E440)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_CREATE_RANDOM_COPS"
	hash "0x102E68B2024D536D"
	jhash (0x23441648)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "SET_CREATE_RANDOM_COPS_NOT_ON_SCENARIOS"
	hash "0x8A4986851C4EF6E7"
	jhash (0x82E548CC)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "SET_CREATE_RANDOM_COPS_ON_SCENARIOS"
	hash "0x444CB7D7DBE6973D"
	jhash (0xEDC31475)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "CAN_CREATE_RANDOM_COPS"
	hash "0x5EE2CAFF7F17770D"
	jhash (0xAA73DAD9)
	returns	"BOOL"

native "SET_PED_AS_ENEMY"
	hash "0x02A0C9720B854BFA"
	jhash (0xAE620A1B)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_SMASH_GLASS"
	hash "0x1CCE141467FF42A2"
	jhash (0x149C60A8)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "IS_PED_IN_ANY_TRAIN"
	hash "0x6F972C1AB75A1ED0"
	jhash (0x759EF63A)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_GETTING_INTO_A_VEHICLE"
	hash "0xBB062B2B5722478E"
	jhash (0x90E805AC)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_TRYING_TO_ENTER_A_LOCKED_VEHICLE"
	hash "0x44D28D5DDFE5F68C"
	jhash (0x46828B4E)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "SET_ENABLE_HANDCUFFS"
	hash "0xDF1AF8B5D56542FA"
	jhash (0xAC9BBA23)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ENABLE_BOUND_ANKLES"
	hash "0xC52E0F855C58FC2E"
	jhash (0x9208D689)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ENABLE_SCUBA"
	hash "0xF99F62004024D506"
	jhash (0x7BF61471)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Setting ped to true allows the ped to shoot "friendlies".
</summary>
]]--
native "SET_CAN_ATTACK_FRIENDLY"
	hash "0xB3B1CB349FF9C75D"
	jhash (0x47C60963)
	arguments {
		Ped "ped",

		BOOL "toggle",

		BOOL "p2",
	}
	returns	"void"

native "GET_PED_ALERTNESS"
	hash "0xF6AA118530443FD2"
	jhash (0xF83E4DAF)
	arguments {
		Ped "ped",
	}
	returns	"Any"

native "SET_PED_ALERTNESS"
	hash "0xDBA71115ED9941A6"
	jhash (0x2C32D9AE)
	arguments {
		Ped "ped",

		Any "value",
	}
	returns	"void"

native "SET_PED_GET_OUT_UPSIDE_DOWN_VEHICLE"
	hash "0xBC0ED94165A48BC2"
	jhash (0x89AD49FF)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	p2 is usually 1.0f  hwatucnt
</summary>
]]--
native "SET_PED_MOVEMENT_CLIPSET"
	hash "0xAF8A94EDE7712BEF"
	jhash (0xA817CDEB)
	arguments {
		Ped "ped",

		charPtr "clipSet",

		float "p2",
	}
	returns	"void"

--[[!
<summary>
	If p1 is 0.0, I believe you are back to normal. 
	If p1 is 1.0, it looks like you can only rotate the ped, not walk.

	Using the following code to reset back to normal
	PED::RESET_PED_MOVEMENT_CLIPSET(PLAYER::PLAYER_PED_ID(), 0.0);
</summary>
]]--
native "RESET_PED_MOVEMENT_CLIPSET"
	hash "0xAA74EC0CB0AAEA2C"
	jhash (0xB83CEE93)
	arguments {
		Ped "ped",

		float "p1",
	}
	returns	"void"

native "SET_PED_STRAFE_CLIPSET"
	hash "0x29A28F3F8CF6D854"
	jhash (0x0BACF010)
	arguments {
		Ped "ped",

		charPtr "clipSet",
	}
	returns	"void"

native "RESET_PED_STRAFE_CLIPSET"
	hash "0x20510814175EA477"
	jhash (0xF1967A12)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "SET_PED_WEAPON_MOVEMENT_CLIPSET"
	hash "0x2622E35B77D3ACA2"
	jhash (0xF8BE54DC)
	arguments {
		Ped "ped",

		charPtr "clipSet",
	}
	returns	"void"

native "RESET_PED_WEAPON_MOVEMENT_CLIPSET"
	hash "0x97B0DB5B4AA74E77"
	jhash (0xC60C9ACD)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "SET_PED_DRIVE_BY_CLIPSET_OVERRIDE"
	hash "0xED34AB6C5CB36520"
	jhash (0xD4C73595)
	arguments {
		Ped "ped",

		charPtr "clipset",
	}
	returns	"void"

native "CLEAR_PED_DRIVE_BY_CLIPSET_OVERRIDE"
	hash "0x4AFE3690D7E0B5AC"
	jhash (0xAEC9163B)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "0x9DBA107B4937F809"
	hash "0x9DBA107B4937F809"
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0xC79196DCB36F6121"
	hash "0xC79196DCB36F6121"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x80054D7FCC70EEC6"
	hash "0x80054D7FCC70EEC6"
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	PED::SET_PED_IN_VEHICLE_CONTEXT(l_128, GAMEPLAY::GET_HASH_KEY("MINI_PROSTITUTE_LOW_PASSENGER"));
	PED::SET_PED_IN_VEHICLE_CONTEXT(l_128, GAMEPLAY::GET_HASH_KEY("MINI_PROSTITUTE_LOW_RESTRICTED_PASSENGER"));
	PED::SET_PED_IN_VEHICLE_CONTEXT(l_3212, GAMEPLAY::GET_HASH_KEY("MISS_FAMILY1_JIMMY_SIT"));
	PED::SET_PED_IN_VEHICLE_CONTEXT(l_3212, GAMEPLAY::GET_HASH_KEY("MISS_FAMILY1_JIMMY_SIT_REAR"));
	PED::SET_PED_IN_VEHICLE_CONTEXT(l_95, GAMEPLAY::GET_HASH_KEY("MISS_FAMILY2_JIMMY_BICYCLE"));
	PED::SET_PED_IN_VEHICLE_CONTEXT(num3, GAMEPLAY::GET_HASH_KEY("MISSFBI2_MICHAEL_DRIVEBY"));
	PED::SET_PED_IN_VEHICLE_CONTEXT(PLAYER::PLAYER_PED_ID(), GAMEPLAY::GET_HASH_KEY("MISS_ARMENIAN3_FRANKLIN_TENSE"));
	PED::SET_PED_IN_VEHICLE_CONTEXT(PLAYER::PLAYER_PED_ID(), GAMEPLAY::GET_HASH_KEY("MISSFBI5_TREVOR_DRIVING"));


</summary>
]]--
native "SET_PED_IN_VEHICLE_CONTEXT"
	hash "0x530071295899A8C6"
	jhash (0x27F25C0E)
	arguments {
		Ped "ped",

		Hash "context",
	}
	returns	"void"

native "RESET_PED_IN_VEHICLE_CONTEXT"
	hash "0x22EF8FF8778030EB"
	jhash (0x3C94D88A)
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_SCRIPTED_SCENARIO_PED_USING_CONDITIONAL_ANIM"
	hash "0x6EC47A344923E1ED"
	jhash (0x3C30B447)
	arguments {
		Ped "ped",

		charPtr "animDict",

		charPtr "anim",
	}
	returns	"BOOL"

native "SET_PED_ALTERNATE_WALK_ANIM"
	hash "0x6C60394CB4F75E9A"
	jhash (0x895E1D67)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		float "p3",

		BOOL "p4",
	}
	returns	"void"

native "CLEAR_PED_ALTERNATE_WALK_ANIM"
	hash "0x8844BBFCE30AA9E9"
	jhash (0x5736FB23)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	stance:
	0 = idle
	1 = walk
	2 = running

	p5 = usually set to true
</summary>
]]--
native "SET_PED_ALTERNATE_MOVEMENT_ANIM"
	hash "0x90A43CC281FFAB46"
	jhash (0xBA84FD8C)
	arguments {
		Ped "ped",

		int "stance",

		charPtr "animDictionary",

		charPtr "animationName",

		float "p4",

		BOOL "p5",
	}
	returns	"void"

native "CLEAR_PED_ALTERNATE_MOVEMENT_ANIM"
	hash "0xD8D19675ED5FBDCE"
	jhash (0x7A7F5BC3)
	arguments {
		Ped "ped",

		int "stance",

		float "p2",
	}
	returns	"void"

native "SET_PED_GESTURE_GROUP"
	hash "0xDDF803377F94AAA8"
	jhash (0x170DA109)
	arguments {
		Ped "ped",

		AnyPtr "p1",
	}
	returns	"void"

native "GET_ANIM_INITIAL_OFFSET_POSITION"
	hash "0xBE22B26DD764C040"
	jhash (0xC59D4268)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		Any "p9",
	}
	returns	"Vector3"

native "GET_ANIM_INITIAL_OFFSET_ROTATION"
	hash "0x4B805E6046EE9E47"
	jhash (0x5F7789E6)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		Any "p9",
	}
	returns	"Vector3"

native "GET_PED_DRAWABLE_VARIATION"
	hash "0x67F3780DD425D4FC"
	jhash (0x29850FE2)
	arguments {
		Ped "ped",

		int "componentId",
	}
	returns	"int"

native "GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS"
	hash "0x27561561732A7842"
	jhash (0x9754C27D)
	arguments {
		Ped "ped",

		int "componentId",
	}
	returns	"int"

native "GET_PED_TEXTURE_VARIATION"
	hash "0x04A355E041E004E6"
	jhash (0xC0A8590A)
	arguments {
		Ped "ped",

		int "componentId",
	}
	returns	"int"

native "GET_NUMBER_OF_PED_TEXTURE_VARIATIONS"
	hash "0x8F7156A3142A6BAD"
	jhash (0x83D9FBE7)
	arguments {
		Ped "ped",

		int "componentId",

		int "drawableId",
	}
	returns	"int"

native "GET_NUMBER_OF_PED_PROP_DRAWABLE_VARIATIONS"
	hash "0x5FAF9754E789FB47"
	jhash (0xC9780B95)
	arguments {
		Ped "ped",

		int "propId",
	}
	returns	"int"

--[[!
<summary>
	Need to check behavior when drawableId = -1.
</summary>
]]--
native "GET_NUMBER_OF_PED_PROP_TEXTURE_VARIATIONS"
	hash "0xA6E7F1CEB523E171"
	jhash (0x4892B882)
	arguments {
		Ped "ped",

		int "propId",

		int "drawableId",
	}
	returns	"int"

native "GET_PED_PALETTE_VARIATION"
	hash "0xE3DD5F2A84B42281"
	jhash (0xEF1BC082)
	arguments {
		Ped "ped",

		int "componentId",
	}
	returns	"int"

native "0x9E30E91FB03A2CAF"
	hash "0x9E30E91FB03A2CAF"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x1E77FA7A62EE6C4C"
	hash "0x1E77FA7A62EE6C4C"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xF033419D1B81FAE8"
	hash "0xF033419D1B81FAE8"
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	Checks if the component variation is valid, this works great for randomizing components using loops.


</summary>
]]--
native "IS_PED_COMPONENT_VARIATION_VALID"
	hash "0xE825F6B6CEA7671D"
	jhash (0x952ABD9A)
	arguments {
		Ped "ped",

		int "componentId",

		int "drawableId",

		int "textureId",
	}
	returns	"BOOL"

--[[!
<summary>
	Component IDS
	0 FACE
	1 MASKS (Previous: BEARDS, but I believe they're overlays)
	2 HAIRCUT
	3 SHIRT
	4 PANTS
	5 Hands / Gloves
	6 SHOES
	7 Eyes
	8 Accessories
	9 Mission Items/Tasks
	10 Decals
	11 Collars and Inner Shirts

	PALETTEID is mostly 2.
</summary>
]]--
native "SET_PED_COMPONENT_VARIATION"
	hash "0x262B14F48D29DE80"
	jhash (0xD4F7B05C)
	arguments {
		Ped "ped",

		int "componentId",

		int "drawableId",

		int "textureId",

		int "paletteId",
	}
	returns	"void"

--[[!
<summary>
	p1 is always false in R* scripts.
</summary>
]]--
native "SET_PED_RANDOM_COMPONENT_VARIATION"
	hash "0xC8A9481A01E63C28"
	jhash (0x4111BA46)
	arguments {
		Ped "ped",

		BOOL "p1",
	}
	returns	"void"

native "SET_PED_RANDOM_PROPS"
	hash "0xC44AA05345C992C6"
	jhash (0xE3318E0E)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "SET_PED_DEFAULT_COMPONENT_VARIATION"
	hash "0x45EEE61580806D63"
	jhash (0xC866A984)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "SET_PED_BLEND_FROM_PARENTS"
	hash "0x137BBD05230DB22D"
	jhash (0x837BD370)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		float "p3",

		float "p4",
	}
	returns	"void"

--[[!
<summary>
	The "shape" parameters control the shape of the ped's face. The "skin" parameters control the skin tone. ShapeMix and skinMix control how much the first and second IDs contribute,(typically mother and father.) ThirdMix overrides the others in favor of the third IDs. IsParent is set for "children" of the player character's grandparents during old-gen character creation. It has unknown effect otherwise.

	The IDs start at zero and go Male Non-DLC, Female Non-DLC, Male DLC, and Female DLC.
</summary>
]]--
native "SET_PED_HEAD_BLEND_DATA"
	hash "0x9414E18B9434C2FE"
	jhash (0x60746B88)
	arguments {
		Ped "ped",

		int "shapeFirstID",

		int "shapeSecondID",

		int "shapeThirdID",

		int "skinFirstID",

		int "skinSecondID",

		int "skinThirdID",

		float "shapeMix",

		float "skinMix",

		float "thirdMix",

		BOOL "isParent",
	}
	returns	"void"

--[[!
<summary>
	The pointer is to a padded struct that matches the arguments to SET_PED_HEAD_BLEND_DATA(...). There are 4 bytes of padding after each field.
</summary>
]]--
native "_GET_PED_HEAD_BLEND_DATA"
	hash "0x2746BD9D88C5C5D0"
	arguments {
		Ped "ped",

		AnyPtr "headBlendData",
	}
	returns	"BOOL"

--[[!
<summary>
	See SET_PED_HEAD_BLEND_DATA().
</summary>
]]--
native "UPDATE_PED_HEAD_BLEND_DATA"
	hash "0x723538F61C647C5A"
	jhash (0x5CB76219)
	arguments {
		Ped "ped",

		float "shapeMix",

		float "skinMix",

		float "thirdMix",
	}
	returns	"void"

--[[!
<summary>
	Used for freemode (online) characters.

	For some reason, the scripts use a rounded float for the index.
</summary>
]]--
native "_SET_PED_EYE_COLOR"
	hash "0x50B56988B170AFDF"
	arguments {
		Ped "ped",

		int "index",
	}
	returns	"void"

--[[!
<summary>
	OverlayID ranges from 0 to 12, index from 0 to _GET_NUM_OVERLAY_VALUES(overlayID)-1, and opacity from 0.0 to 1.0. 
</summary>
]]--
native "SET_PED_HEAD_OVERLAY"
	hash "0x48F44967FA05CC1E"
	jhash (0xD28DBA90)
	arguments {
		Ped "ped",

		int "overlayID",

		int "index",

		float "opacity",
	}
	returns	"void"

--[[!
<summary>
	Likely a char, if that overlay is not set, e.i. "None" option, returns 255;

</summary>
]]--
native "_GET_PED_HEAD_OVERLAY_VALUE"
	hash "0xA60EF3B6461A4D43"
	arguments {
		Ped "ped",

		int "overlayID",
	}
	returns	"int"

--[[!
<summary>
	Used with freemode (online) characters.
</summary>
]]--
native "_GET_NUM_HEAD_OVERLAY_VALUES"
	hash "0xCF1CE768BB43480E"
	jhash (0xFF43C18D)
	arguments {
		int "overlayID",
	}
	returns	"int"

--[[!
<summary>
	Used for freemode (online) characters.

	ColorType is 1 for eyebrows, beards, and chest hair; 2 for blush and lipstick; and 0 otherwise, though not called in those cases.

	Called after SET_PED_HEAD_OVERLAY().
</summary>
]]--
native "_SET_PED_HEAD_OVERLAY_COLOR"
	hash "0x497BF74A7B9CB952"
	arguments {
		Ped "ped",

		int "overlayID",

		int "colorType",

		int "colorID",

		int "secondColorID",
	}
	returns	"void"

--[[!
<summary>
	Used for freemode (online) characters.
</summary>
]]--
native "_SET_PED_HAIR_COLOR"
	hash "0x4CFFC65454C93A49"
	arguments {
		Ped "ped",

		int "colorID",

		int "highlightColorID",
	}
	returns	"void"

--[[!
<summary>
	Used for freemode (online) characters.
</summary>
]]--
native "_GET_NUM_HAIR_COLORS"
	hash "0xE5C0CF872C2AD150"
	returns	"int"

native "0xD1F7CA1535D22818"
	hash "0xD1F7CA1535D22818"
	returns	"int"

native "0x4852FC386E2E1BB5"
	hash "0x4852FC386E2E1BB5"
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"void"

native "0x013E5CFC38CD5387"
	hash "0x013E5CFC38CD5387"
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"void"

native "0xED6D8E27A43B8CDE"
	hash "0xED6D8E27A43B8CDE"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xEA9960D07DADCF10"
	hash "0xEA9960D07DADCF10"
	arguments {
		Any "p0",
	}
	returns	"int"

native "0x3E802F11FBE27674"
	hash "0x3E802F11FBE27674"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xF41B5D290C99A3D6"
	hash "0xF41B5D290C99A3D6"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xE0D36E5D9E99CC21"
	hash "0xE0D36E5D9E99CC21"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xAAA6A3698A69E048"
	hash "0xAAA6A3698A69E048"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x0525A2C2562F3CD4"
	hash "0x0525A2C2562F3CD4"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x604E810189EE3A59"
	hash "0x604E810189EE3A59"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xC56FBF2F228E1DAC"
	hash "0xC56FBF2F228E1DAC"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

--[[!
<summary>
	Sets the various freemode face features, e.g. nose length, chin shape. Scale ranges from -1.0 to 1.0.

	index appears to be from 0-20
</summary>
]]--
native "_SET_PED_FACE_FEATURE"
	hash "0x71A5C1DBA060049E"
	arguments {
		Ped "ped",

		int "index",

		float "scale",
	}
	returns	"void"

native "HAS_PED_HEAD_BLEND_FINISHED"
	hash "0x654CD0A825161131"
	jhash (0x2B1BD9C5)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "0x4668D80430D6C299"
	hash "0x4668D80430D6C299"
	jhash (0x894314A4)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "0xCC9682B8951C5229"
	hash "0xCC9682B8951C5229"
	jhash (0x57E5B3F9)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "0xA21C118553BBDF02"
	hash "0xA21C118553BBDF02"
	jhash (0xC6F36292)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Type equals 0 for male non-dlc, 1 for female non-dlc, 2 for male dlc, and 3 for female dlc.

	Used when calling SET_PED_HEAD_BLEND_DATA.
</summary>
]]--
native "_GET_FIRST_PARENT_ID_FOR_PED_TYPE"
	hash "0x68D353AB88B97E0C"
	jhash (0x211DEFEC)
	arguments {
		int "type",
	}
	returns	"int"

--[[!
<summary>
	Type equals 0 for male non-dlc, 1 for female non-dlc, 2 for male dlc, and 3 for female dlc.
</summary>
]]--
native "_GET_NUM_PARENT_PEDS_OF_TYPE"
	hash "0x5EF37013A6539C9D"
	jhash (0x095D3BD8)
	arguments {
		int "type",
	}
	returns	"int"

--[[!
<summary>
	from extreme3.c4
	PED::_39D55A620FCB6A3A(PLAYER::PLAYER_PED_ID(), 8, PED::GET_PED_DRAWABLE_VARIATION(PLAYER::PLAYER_PED_ID(), 8), PED::GET_PED_TEXTURE_VARIATION(PLAYER::PLAYER_PED_ID(), 8));

	p1 is probably componentId
</summary>
]]--
native "0x39D55A620FCB6A3A"
	hash "0x39D55A620FCB6A3A"
	jhash (0x45F3BDFB)
	arguments {
		Ped "ped",

		int "p1",

		int "drawableId",

		int "textureId",
	}
	returns	"Any"

--[[!
<summary>
	Normally returns true. Returns false briefly whilst getting into a plane. This is probably a check to see if the ped model and all its components/drawables are properly loaded yet.
</summary>
]]--
native "0x66680A92700F43DF"
	hash "0x66680A92700F43DF"
	jhash (0xC6517D52)
	arguments {
		Ped "p0",
	}
	returns	"BOOL"

native "0x5AAB586FFEC0FD96"
	hash "0x5AAB586FFEC0FD96"
	jhash (0x6435F67F)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x2B16A3BFF1FBCE49"
	hash "0x2B16A3BFF1FBCE49"
	jhash (0xC0E23671)
	arguments {
		Ped "ped",

		int "componentId",

		int "drawableId",

		int "TextureId",
	}
	returns	"Any"

--[[!
<summary>
	Normally returns true. Returns false briefly whilst putting on a helmet after getting onto a motorbike. Not sure what that's about.
</summary>
]]--
native "0x784002A632822099"
	hash "0x784002A632822099"
	jhash (0x3B0CA391)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "0xF79F9DEF0AADE61A"
	hash "0xF79F9DEF0AADE61A"
	jhash (0xFD103BA7)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "GET_PED_PROP_INDEX"
	hash "0x898CC20EA75BACD8"
	jhash (0x746DDAC0)
	arguments {
		Ped "ped",

		int "componentId",
	}
	returns	"int"

--[[!
<summary>
	ComponentId can be set to various things based on what category you're wanting to set

	1 
	They are more but those 2 id's are definite

</summary>
]]--
native "SET_PED_PROP_INDEX"
	hash "0x93376B65A266EB5F"
	jhash (0x0829F2E2)
	arguments {
		Ped "ped",

		int "componentId",

		int "drawableId",

		int "TextureId",

		BOOL "attach",
	}
	returns	"void"

native "KNOCK_OFF_PED_PROP"
	hash "0x6FD7816A36615F48"
	jhash (0x08D8B180)
	arguments {
		int "propIndex",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"void"

native "CLEAR_PED_PROP"
	hash "0x0943E5B8E078E76E"
	jhash (0x2D23D743)
	arguments {
		Ped "ped",

		int "propId",
	}
	returns	"void"

native "CLEAR_ALL_PED_PROPS"
	hash "0xCD8A7537A9B52F06"
	jhash (0x81DF8B43)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "0xAFF4710E2A0A6C12"
	hash "0xAFF4710E2A0A6C12"
	arguments {
		Any "p0",
	}
	returns	"void"

native "GET_PED_PROP_TEXTURE_INDEX"
	hash "0xE131A28626F81AB2"
	jhash (0x922A6653)
	arguments {
		Ped "ped",

		int "componentId",
	}
	returns	"int"

native "0x1280804F7CFD2D6C"
	hash "0x1280804F7CFD2D6C"
	jhash (0x7BCD8991)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x36C6984C3ED0C911"
	hash "0x36C6984C3ED0C911"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xB50EB4CCB29704AC"
	hash "0xB50EB4CCB29704AC"
	jhash (0x080275EE)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xFEC9A3B1820F3331"
	hash "0xFEC9A3B1820F3331"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	works with AI::TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS to make a ped completely oblivious to all events going on around him
</summary>
]]--
native "SET_BLOCKING_OF_NON_TEMPORARY_EVENTS"
	hash "0x9F8AA94D6D97DBF4"
	jhash (0xDFE34E4A)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_BOUNDS_ORIENTATION"
	hash "0x4F5F651ACCC9C4CF"
	jhash (0xCFA20D68)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"void"

--[[!
<summary>
	PED::REGISTER_TARGET(l_216, PLAYER::PLAYER_PED_ID()); from re_prisonbreak.txt.

	l_216 = RECSBRobber1
</summary>
]]--
native "REGISTER_TARGET"
	hash "0x2F25D9AEFA34FBA2"
	jhash (0x50A95442)
	arguments {
		Ped "ped",

		Ped "target",
	}
	returns	"void"

--[[!
<summary>
	Based on TASK_COMBAT_HATED_TARGETS_AROUND_PED, the parameters are likely similar (PedHandle, and area to attack in).
</summary>
]]--
native "REGISTER_HATED_TARGETS_AROUND_PED"
	hash "0x9222F300BF8354FE"
	jhash (0x7F87559E)
	arguments {
		Ped "ped",

		float "areaToAttack",
	}
	returns	"void"

--[[!
<summary>
	Gets a random ped in the x/y/zRadius near the x/y/z coordinates passed. 

	Ped Types:
	Any = -1
	Player = 1
	Male = 4 
	Female = 5 
	Cop = 6
	Human = 26
	SWAT = 27 
	Animal = 28
	Army = 29
</summary>
]]--
native "GET_RANDOM_PED_AT_COORD"
	hash "0x876046A8E3A4B71C"
	jhash (0xDC8239EB)
	arguments {
		float "x",

		float "y",

		float "z",

		float "xRadius",

		float "yRadius",

		float "zRadius",

		int "pedType",
	}
	returns	"Ped"

--[[!
<summary>
	Gets the closest ped in a radius.

	Ped Types:
	Any ped = -1
	Player = 1
	Male = 4 
	Female = 5 
	Cop = 6
	Human = 26
	SWAT = 27 
	Animal = 28
	Army = 29

	------------------
	P4 P5 P7 P8
	1  0  x  x  = return nearest walking Ped
	1  x  0  x  = return nearest walking Ped
	x  1  1  x  = return Ped you are using
	0  0  x  x  = no effect
	0  x  0  x  = no effect

	x = can be 1 or 0. Does not have any obvious changes.

	This function does not return ped who is:
	1. Standing still
	2. Driving
	3. Fleeing
	4. Attacking

	This function only work if the ped is:
	1. walking normally.
	2. waiting to cross a road.

	Note: PED::GET_PED_NEARBY_PEDS works for more peds.
</summary>
]]--
native "GET_CLOSEST_PED"
	hash "0xC33AB876A77F8164"
	jhash (0x8F6C1F55)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		BOOL "p4",

		BOOL "p5",

		PedPtr "outPed",

		BOOL "p7",

		BOOL "p8",

		int "pedType",
	}
	returns	"BOOL"

--[[!
<summary>
	Sets a value indicating whether scenario peds should be returned by the next call to a command that returns peds. Eg. GET_CLOSEST_PED.
</summary>
]]--
native "SET_SCENARIO_PEDS_TO_BE_RETURNED_BY_NEXT_COMMAND"
	hash "0x14F19A8782C8071E"
	jhash (0x85615FD0)
	arguments {
		BOOL "value",
	}
	returns	"void"

native "0x03EA03AF85A85CB7"
	hash "0x03EA03AF85A85CB7"
	jhash (0x18DD76A1)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",

		BOOL "p4",

		BOOL "p5",

		BOOL "p6",

		BOOL "p7",

		Any "p8",
	}
	returns	"BOOL"

native "0xDED5AF5A0EA4B297"
	hash "0xDED5AF5A0EA4B297"
	jhash (0x6D55B3B3)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	Decompiled scripts seem to use a range between 0.0 and 1.0, but tests confirm that 1.0 is not the limit.
	Both 10.0 and 100.0 allow the driver to avoid other vehicles better and have faster reaction times, but I don't know if 10.0 is the limit or is it 100.0. Didn't really notice differences.
</summary>
]]--
native "SET_DRIVER_ABILITY"
	hash "0xB195FFA8042FC5C3"
	jhash (0xAAD4012C)
	arguments {
		Ped "driver",

		float "ability",
	}
	returns	"void"

--[[!
<summary>
	range 0.0f - 1.0f
</summary>
]]--
native "SET_DRIVER_AGGRESSIVENESS"
	hash "0xA731F608CA104E3C"
	jhash (0x8B02A8FB)
	arguments {
		Ped "driver",

		float "aggressiveness",
	}
	returns	"void"

--[[!
<summary>
	Prevents the ped from going limp.

	[Example: Can prevent peds from falling when standing on moving vehicles.]
</summary>
]]--
native "CAN_PED_RAGDOLL"
	hash "0x128F79EDCECE4FD5"
	jhash (0xC0EFB7A3)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	time1- Time Ped is in ragdoll mode(ms)

	time2- time2 same as time1 but in mircoseconds (us)

	ragdollType-
	0 : Normal ragdoll
	1 : Falls with stiff legs/body
	2 : Narrow leg stumble(may not fall)
	3 : Wide leg stumble(may not fall)

	p4, p5, p6- No idea

</summary>
]]--
native "SET_PED_TO_RAGDOLL"
	hash "0xAE99FB955581844A"
	jhash (0x83CB5052)
	arguments {
		Ped "ped",

		int "time1",

		int "time2",

		int "ragdollType",

		BOOL "p4",

		BOOL "p5",

		BOOL "p6",
	}
	returns	"BOOL"

native "SET_PED_TO_RAGDOLL_WITH_FALL"
	hash "0xD76632D99E4966C8"
	jhash (0xFA12E286)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",

		Any "p9",

		Any "p10",

		Any "p11",

		Any "p12",

		Any "p13",
	}
	returns	"Any"

--[[!
<summary>
	Causes Ped to ragdoll on collision with any object (e.g Running into trashcan). If applied to player you will sometimes trip on the sidewalk.
</summary>
]]--
native "SET_PED_RAGDOLL_ON_COLLISION"
	hash "0xF0A4F1BBF4FA7497"
	jhash (0x2654A0F4)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "IS_PED_RAGDOLL"
	hash "0x47E4E977581C5B55"
	jhash (0xC833BBE1)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_RUNNING_RAGDOLL_TASK"
	hash "0xE3B6097CC25AA69E"
	jhash (0x44A153F2)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "SET_PED_RAGDOLL_FORCE_FALL"
	hash "0x01F6594B923B9251"
	jhash (0x20A5BDE0)
	arguments {
		Ped "ped",
	}
	returns	"Any"

native "RESET_PED_RAGDOLL_TIMER"
	hash "0x9FA4664CF62E47E8"
	jhash (0xF2865370)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "SET_PED_CAN_RAGDOLL"
	hash "0xB128377056A54E2A"
	jhash (0xCF1384C4)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "0xD1871251F3B5ACD7"
	hash "0xD1871251F3B5ACD7"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "IS_PED_RUNNING_MOBILE_PHONE_TASK"
	hash "0x2AFE52F782F25775"
	jhash (0xFB2AFED1)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "0xA3F3564A5B3646C0"
	hash "0xA3F3564A5B3646C0"
	jhash (0x97353375)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x26695EC767728D84"
	hash "0x26695EC767728D84"
	jhash (0x9C8F830D)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xD86D101FCFD00A4B"
	hash "0xD86D101FCFD00A4B"
	jhash (0x77CBA290)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "SET_PED_ANGLED_DEFENSIVE_AREA"
	hash "0xC7F76DF27A5045A1"
	jhash (0x3EFBDD9B)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		BOOL "p8",

		BOOL "p9",
	}
	returns	"void"

native "SET_PED_SPHERE_DEFENSIVE_AREA"
	hash "0x9D3151A373974804"
	jhash (0xBD96D8E8)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		float "radius",

		BOOL "p5",

		BOOL "p6",
	}
	returns	"void"

native "0xF9B8F91AAD3B953E"
	hash "0xF9B8F91AAD3B953E"
	jhash (0x40638BDC)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",
	}
	returns	"void"

native "0xE4723DB6E736CCFF"
	hash "0xE4723DB6E736CCFF"
	jhash (0x4763B2C6)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",
	}
	returns	"void"

native "SET_PED_DEFENSIVE_AREA_ATTACHED_TO_PED"
	hash "0x4EF47FE21698A8B6"
	jhash (0x74BDA7CE)
	arguments {
		Ped "ped",

		Ped "attachPed",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		BOOL "p9",

		BOOL "p10",
	}
	returns	"void"

native "SET_PED_DEFENSIVE_AREA_DIRECTION"
	hash "0x413C6C763A4AFFAD"
	jhash (0xB66B0C9A)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",
	}
	returns	"void"

native "REMOVE_PED_DEFENSIVE_AREA"
	hash "0x74D4E028107450A9"
	jhash (0x34AAAFA5)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "GET_PED_DEFENSIVE_AREA_POSITION"
	hash "0x3C06B8786DD94CD1"
	jhash (0xCB65198D)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"int"

native "0xBA63D9FE45412247"
	hash "0xBA63D9FE45412247"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"BOOL"

native "0x8421EB4DA7E391B9"
	hash "0x8421EB4DA7E391B9"
	jhash (0xF3B7EFBF)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xFDDB234CF74073D9"
	hash "0xFDDB234CF74073D9"
	jhash (0xA0134498)
	arguments {
		Any "p0",
	}
	returns	"void"

native "REVIVE_INJURED_PED"
	hash "0x8D8ACD8388CD99CE"
	jhash (0x14D3E6E3)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "RESURRECT_PED"
	hash "0x71BC8E838B9C6035"
	jhash (0xA4B82097)
	arguments {
		Ped "ped",
	}
	returns	"void"

--[[!
<summary>
	NOTE: Debugging functions are not present in the retail version of the game.

	*untested but char *name could also be a hash for a localized string
</summary>
]]--
native "SET_PED_NAME_DEBUG"
	hash "0x98EFA132A4117BE1"
	jhash (0x20D6273E)
	arguments {
		Ped "ped",

		charPtr "name",
	}
	returns	"void"

native "GET_PED_EXTRACTED_DISPLACEMENT"
	hash "0xE0AF41401ADF87E3"
	jhash (0x5231F901)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"int"

native "SET_PED_DIES_WHEN_INJURED"
	hash "0x5BA7919BED300023"
	jhash (0xE94E24D4)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"Any"

native "SET_PED_ENABLE_WEAPON_BLOCKING"
	hash "0x97A790315D3831FD"
	jhash (0x4CAD1A4A)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"Any"

native "0xF9ACF4A08098EA25"
	hash "0xF9ACF4A08098EA25"
	jhash (0x141CC936)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "RESET_PED_VISIBLE_DAMAGE"
	hash "0x3AC1F7B898F30C05"
	jhash (0xC4BC4841)
	arguments {
		Ped "ped",
	}
	returns	"Any"

native "0x816F6981C60BF53B"
	hash "0x816F6981C60BF53B"
	jhash (0x1E54DB12)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		Any "p4",
	}
	returns	"void"

native "APPLY_PED_BLOOD"
	hash "0x83F7E01C7B769A26"
	jhash (0x376CE3C0)
	arguments {
		Ped "ped",

		int "boneIndex",

		float "xRot",

		float "yRot",

		float "zRot",

		charPtr "woundType",
	}
	returns	"void"

native "0x3311E47B91EDCBBC"
	hash "0x3311E47B91EDCBBC"
	jhash (0x8F3F3A9C)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		AnyPtr "p4",
	}
	returns	"void"

native "0xEF0D582CBF2D9B0F"
	hash "0xEF0D582CBF2D9B0F"
	jhash (0xFC13CE80)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		Any "p6",

		float "p7",

		AnyPtr "p8",
	}
	returns	"void"

native "APPLY_PED_DAMAGE_DECAL"
	hash "0x397C38AA7B4A5F83"
	jhash (0x8A13A41F)
	arguments {
		Ped "p0",

		int "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		int "p7",

		BOOL "p8",

		charPtr "p9",
	}
	returns	"Any"

--[[!
<summary>
	vb.net
	Dim closest_ped as Integer = World.GetNearestPed(Game.Player.Character.Position,100).Handle
	Native.function.call(Hash.APPLY_PED_DAMAGE_PACK,closest_ped, "BigHitByVehicle", 0, 1)

	Edited by: Enumerator


</summary>
]]--
native "APPLY_PED_DAMAGE_PACK"
	hash "0x46DF918788CB093F"
	jhash (0x208D0CB8)
	arguments {
		Ped "ped",

		charPtr "damagePack",

		float "damage",

		float "mult",
	}
	returns	"void"

native "CLEAR_PED_BLOOD_DAMAGE"
	hash "0x8FE22675A5A45817"
	jhash (0xF7ADC960)
	arguments {
		Ped "ped",
	}
	returns	"void"

--[[!
<summary>
	Somehow related to changing ped's clothes.
</summary>
]]--
native "0x56E3B78C5408D9F4"
	hash "0x56E3B78C5408D9F4"
	jhash (0xF210BE69)
	arguments {
		Ped "p0",

		int "p1",
	}
	returns	"Any"

native "0x62AB793144DE75DC"
	hash "0x62AB793144DE75DC"
	jhash (0x0CB6C4ED)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x523C79AEEFCC4A2A"
	hash "0x523C79AEEFCC4A2A"
	jhash (0x70AA5B7D)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0x71EAB450D86954A1"
	hash "0x71EAB450D86954A1"
	jhash (0x47187F7F)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x2B694AFCF64E6994"
	hash "0x2B694AFCF64E6994"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "CLEAR_PED_WETNESS"
	hash "0x9C720776DAA43E7E"
	jhash (0x629F15BD)
	arguments {
		Ped "ped",
	}
	returns	"Any"

native "SET_PED_WETNESS_HEIGHT"
	hash "0x44CB6447D2571AA0"
	jhash (0x7B33289A)
	arguments {
		Ped "ped",

		float "height",
	}
	returns	"Any"

--[[!
<summary>
	combined with PED::SET_PED_WETNESS_HEIGHT(), this native makes the ped drenched in water up to the height specified in the other function
</summary>
]]--
native "SET_PED_WETNESS_ENABLED_THIS_FRAME"
	hash "0xB5485E4907B53019"
	jhash (0xBDE749F7)
	arguments {
		Ped "ped",
	}
	returns	"void"

--[[!
<summary>
	Something related to clearing the ped because always used with CLEAR_PED_WETNESS, CLEAR_PED_BLOOD_DAMAGE and RESET_PED_VISIBLE_DAMAGE.
</summary>
]]--
native "0x6585D955A68452A5"
	hash "0x6585D955A68452A5"
	jhash (0xA993915F)
	arguments {
		Ped "ped",
	}
	returns	"Any"

--[[!
<summary>
	Sweat is set to 100.0 or 0.0 in the decompiled scripts.
</summary>
]]--
native "SET_PED_SWEAT"
	hash "0x27B0405F59637D1F"
	jhash (0x76A1DB9F)
	arguments {
		Ped "ped",

		float "sweat",
	}
	returns	"void"

--[[!
<summary>
	Applies an Item from a PedDecorationCollection to a ped. These include tattoos and shirt decals.

	collection - PedDecorationCollection filename hash
	overlay - Item name hash

	Example:
	Entry inside "mpbeach_overlays.xml" -
	&lt;Item&gt;
	  &lt;uvPos x="0.500000" y="0.500000" /&gt;
	  &lt;scale x="0.600000" y="0.500000" /&gt;
	  &lt;rotation value="0.000000" /&gt;
	  &lt;nameHash&gt;FM_Hair_Fuzz&lt;/nameHash&gt;
	  &lt;txdHash&gt;mp_hair_fuzz&lt;/txdHash&gt;
	  &lt;txtHash&gt;mp_hair_fuzz&lt;/txtHash&gt;
	  &lt;zone&gt;ZONE_HEAD&lt;/zone&gt;
	  &lt;type&gt;TYPE_TATTOO&lt;/type&gt;
	  &lt;faction&gt;FM&lt;/faction&gt;
	  &lt;garment&gt;All&lt;/garment&gt;
	  &lt;gender&gt;GENDER_DONTCARE&lt;/gender&gt;
	  &lt;award /&gt;
	  &lt;awardLevel /&gt;
	&lt;/Item&gt;

	Code:
	PED::_0x5F5D1665E352A839(PLAYER::PLAYER_PED_ID(), GAMEPLAY::GET_HASH_KEY("mpbeach_overlays"), GAMEPLAY::GET_HASH_KEY("fm_hair_fuzz"))
</summary>
]]--
native "_APPLY_PED_OVERLAY"
	hash "0x5F5D1665E352A839"
	jhash (0x70559AC7)
	arguments {
		Ped "ped",

		Hash "collection",

		Hash "overlay",
	}
	returns	"void"

--[[!
<summary>
	Console Hash: 0x8CD3E487

</summary>
]]--
native "0x5619BFA07CFD7833"
	hash "0x5619BFA07CFD7833"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

--[[!
<summary>
	Returns the zoneID for the overlay if it is a member of collection.
	0 = torso
	1 = head
	2 = left arm
	3 = right arm
	4 = left leg
	5 = right leg
	6 = unknown
	7 = none / not in collection
</summary>
]]--
native "_GET_TATTOO_ZONE"
	hash "0x9FD452BFBE7A7A8B"
	jhash (0x3543019E)
	arguments {
		Hash "collection",

		Hash "overlay",
	}
	returns	"int"

native "CLEAR_PED_DECORATIONS"
	hash "0x0E5173C163976E38"
	jhash (0xD4496BF3)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "0xE3B27E70CEAB9F0C"
	hash "0xE3B27E70CEAB9F0C"
	jhash (0xEFD58EB9)
	arguments {
		Any "p0",
	}
	returns	"void"

native "WAS_PED_SKELETON_UPDATED"
	hash "0x11B499C1E0FF8559"
	jhash (0xF7E2FBAD)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	Gets the position of the specified bone of the specified ped.

	ped: The ped to get the position of a bone from.
	boneId: The ID of the bone to get the position from. This is NOT the index.
	offsetX: The X-component of the offset to add to the position relative to the bone's rotation.
	offsetY: The Y-component of the offset to add to the position relative to the bone's rotation.
	offsetZ: The Z-component of the offset to add to the position relative to the bone's rotation.
</summary>
]]--
native "GET_PED_BONE_COORDS"
	hash "0x17C07FC640E86B4E"
	jhash (0x4579CAB1)
	arguments {
		Ped "ped",

		int "boneId",

		float "offsetX",

		float "offsetY",

		float "offsetZ",
	}
	returns	"Vector3"

--[[!
<summary>
	Creates a new NaturalMotion message.

	startImmediately: If set to true, the character will perform the message the moment it receives it by GIVE_PED_NM_MESSAGE. If false, the Ped will get the message but won't perform it yet. While it's a boolean value, if negative, the message will not be initialized.
	messageId: The ID of the NaturalMotion message.

	If a message already exists, this function does nothing. A message exists until the point it has been successfully dispatched by GIVE_PED_NM_MESSAGE.
</summary>
]]--
native "CREATE_NM_MESSAGE"
	hash "0x418EF2A1BCE56685"
	jhash (0x1CFBFD4B)
	arguments {
		BOOL "startImmediately",

		int "messageId",
	}
	returns	"void"

--[[!
<summary>
	Sends the message that was created by a call to CREATE_NM_MESSAGE to the specified Ped.

	If a message hasn't been created already, this function does nothing.
	If the Ped is not ragdolled with Euphoria enabled, this function does nothing.
	The following call can be used to ragdoll the Ped with Euphoria enabled: SET_PED_TO_RAGDOLL(ped, 4000, 5000, 1, 1, 1, 0);

	Call order:
	SET_PED_TO_RAGDOLL
	CREATE_NM_MESSAGE
	GIVE_PED_NM_MESSAGE

	Multiple messages can be chained. Eg. to make the ped stagger and swing his arms around, the following calls can be made:
	SET_PED_TO_RAGDOLL(ped, 4000, 5000, 1, 1, 1, 0);
	CREATE_NM_MESSAGE(true, 0); // stopAllBehaviours - Stop all other behaviours, in case the Ped is already doing some Euphoria stuff.
	GIVE_PED_NM_MESSAGE(ped); // Dispatch message to Ped.
	CREATE_NM_MESSAGE(true, 1151); // staggerFall - Attempt to walk while falling.
	GIVE_PED_NM_MESSAGE(ped); // Dispatch message to Ped.
	CREATE_NM_MESSAGE(true, 372); // armsWindmill - Swing arms around.
	GIVE_PED_NM_MESSAGE(ped); // Dispatch message to Ped.
</summary>
]]--
native "GIVE_PED_NM_MESSAGE"
	hash "0xB158DFCCC56E5C5B"
	jhash (0x737C3689)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "ADD_SCENARIO_BLOCKING_AREA"
	hash "0x1B5C85C612E5256E"
	jhash (0xA38C0234)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",

		BOOL "p7",

		BOOL "p8",

		BOOL "p9",
	}
	returns	"Any"

native "REMOVE_SCENARIO_BLOCKING_AREAS"
	hash "0xD37401D78A929A49"
	jhash (0x4DDF845F)
	returns	"void"

native "REMOVE_SCENARIO_BLOCKING_AREA"
	hash "0x31D16B74C6E29D66"
	jhash (0x4483EF06)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_SCENARIO_PEDS_SPAWN_IN_SPHERE_AREA"
	hash "0x28157D43CF600981"
	jhash (0x80EAD297)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "IS_PED_USING_SCENARIO"
	hash "0x1BF094736DD62C2E"
	jhash (0x0F65B0D4)
	arguments {
		Ped "ped",

		charPtr "scenario",
	}
	returns	"BOOL"

native "IS_PED_USING_ANY_SCENARIO"
	hash "0x57AB4A3080F85143"
	jhash (0x195EF5B7)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "0xFE07FF6495D52E2A"
	hash "0xFE07FF6495D52E2A"
	jhash (0x59DE73AC)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"Any"

native "0x9A77DFD295E29B09"
	hash "0x9A77DFD295E29B09"
	jhash (0xC08FE5F6)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x25361A96E0F7E419"
	hash "0x25361A96E0F7E419"
	jhash (0x58C0F6CF)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"Any"

native "0xEC6935EBE0847B90"
	hash "0xEC6935EBE0847B90"
	jhash (0x761F8F48)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"Any"

native "0xA3A9299C4F2ADB98"
	hash "0xA3A9299C4F2ADB98"
	jhash (0x033F43FA)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xF1C03A5352243A30"
	hash "0xF1C03A5352243A30"
	jhash (0x4C684C81)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xEEED8FAFEC331A70"
	hash "0xEEED8FAFEC331A70"
	jhash (0x7B4C3E6F)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"Any"

native "0x425AECF167663F48"
	hash "0x425AECF167663F48"
	jhash (0x5BC276AE)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x5B6010B3CBC29095"
	hash "0x5B6010B3CBC29095"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xCEDA60A74219D064"
	hash "0xCEDA60A74219D064"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "PLAY_FACIAL_ANIM"
	hash "0xE1E65CA8AC9C00ED"
	jhash (0x1F6CCDDE)
	arguments {
		Ped "ped",

		charPtr "animName",

		charPtr "animDict",
	}
	returns	"void"

native "SET_FACIAL_IDLE_ANIM_OVERRIDE"
	hash "0xFFC24B988B938B38"
	jhash (0x9BA19C13)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "CLEAR_FACIAL_IDLE_ANIM_OVERRIDE"
	hash "0x726256CC1EEB182F"
	jhash (0x5244F4E2)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_PED_CAN_PLAY_GESTURE_ANIMS"
	hash "0xBAF20C5432058024"
	jhash (0xE131E3B3)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_PLAY_VISEME_ANIMS"
	hash "0xF833DDBA3B104D43"
	jhash (0xA2FDAF27)
	arguments {
		Ped "ped",

		BOOL "p1",

		Any "p2",
	}
	returns	"void"

native "0x33A60D8BDD6E508C"
	hash "0x33A60D8BDD6E508C"
	jhash (0xADB2511A)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_PED_CAN_PLAY_AMBIENT_ANIMS"
	hash "0x6373D1349925A70E"
	jhash (0xF8053081)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_PLAY_AMBIENT_BASE_ANIMS"
	hash "0x0EB0585D15254740"
	jhash (0x5720A5DD)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "0xC2EE020F5FB4DB53"
	hash "0xC2EE020F5FB4DB53"
	jhash (0xB7CD0A49)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_PED_CAN_ARM_IK"
	hash "0x6C3B4D6D13B4C841"
	jhash (0x343B4DE0)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_HEAD_IK"
	hash "0xC11C18092C5530DC"
	jhash (0xD3B04476)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_LEG_IK"
	hash "0x73518ECE2485412B"
	jhash (0x9955BC6F)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_TORSO_IK"
	hash "0xF2B7106D37947CE0"
	jhash (0x8E5D4EAB)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "0xF5846EDB26A98A24"
	hash "0xF5846EDB26A98A24"
	jhash (0x7B0040A8)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x6647C5F6F5792496"
	hash "0x6647C5F6F5792496"
	jhash (0x0FDA62DE)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_PED_CAN_USE_AUTO_CONVERSATION_LOOKAT"
	hash "0xEC4686EC06434678"
	jhash (0x584C5178)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "IS_PED_HEADTRACKING_PED"
	hash "0x5CD3CB88A7F8850D"
	jhash (0x2A5DF721)
	arguments {
		Ped "ped1",

		Ped "ped2",
	}
	returns	"BOOL"

native "IS_PED_HEADTRACKING_ENTITY"
	hash "0x813A0A7C9D2E831F"
	jhash (0x233C9ACF)
	arguments {
		Ped "ped",

		Entity "entity",
	}
	returns	"BOOL"

--[[!
<summary>
	This is only called once in the scripts.

	sub_1CD9(&amp;l_49, 0, getElem(3, &amp;l_34, 4), "MICHAEL", 0, 1);
	                    sub_1CA8("WORLD_HUMAN_SMOKING", 2);
	                    PED::SET_PED_PRIMARY_LOOKAT(getElem(3, &amp;l_34, 4), PLAYER::PLAYER_PED_ID());
</summary>
]]--
native "SET_PED_PRIMARY_LOOKAT"
	hash "0xCD17B554996A8D9E"
	jhash (0x6DEF6F1C)
	arguments {
		Ped "ped",

		Ped "lookAt",
	}
	returns	"void"

native "0x78C4E9961DB3EB5B"
	hash "0x78C4E9961DB3EB5B"
	jhash (0xFC942D7C)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x82A3D6D9CC2CB8E3"
	hash "0x82A3D6D9CC2CB8E3"
	jhash (0x89EEE07B)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xA660FAF550EB37E5"
	hash "0xA660FAF550EB37E5"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
		Flags List : pastebin.com/DenXRR8c
		Research Help : pastebin.com/fPL1cSwB

		(*) When flagId is set to 33 and the bool value to true, peds will die by starting ragdoll, so you should set this flag to false when you resurrect a ped.
		When flagId is set to 62 and the boolvalue to false this happens: Ped is taken out of vehicle and can't get back in when jacking their empty vehicle. If in a plane it falls from the sky and crashes. Sometimes peds vehicle continue to drive the route without its driver who's running after. 

		(*)
		JUMPING CHANGES  60,61,104 TO FALSE
		BEING ON WATER CHANGES 60,61 TO FALSE AND 65,66,168 TO TRUE
		FALLING CHANGES 60,61,104,276 TO FALSE AND TO 76 TRUE
		DYING CHANGES 60,61,104,276* TO FALSE AND (NONE) TO TRUE
		DYING MAKES 60,61,104 TO FALSE
		BEING IN A CAR CHANGES 60,79,104 TO FALSE AND 62 TO TRUE

		(*)Maximum value for flagId is 0x1CA (458) in b944.
		ID 0xF0 (240) appears to be a special flag which is handled different compared to the others IDs.
</summary>
]]--
native "SET_PED_CONFIG_FLAG"
	hash "0x1913FE4CBF41C463"
	jhash (0x9CFBE10D)
	arguments {
		Ped "ped",

		int "flagId",

		BOOL "value",
	}
	returns	"void"

--[[!
<summary>
	PED::SET_PED_RESET_FLAG(PLAYER::PLAYER_PED_ID(), 240, 1);
</summary>
]]--
native "SET_PED_RESET_FLAG"
	hash "0xC1E8A365BF3B29F2"
	jhash (0xCFF6FF66)
	arguments {
		Ped "ped",

		int "flagId",

		BOOL "doReset",
	}
	returns	"void"

--[[!
<summary>
	p2 is always 1 in the scripts.

	if (GET_PED_CONFIG_FLAG(ped, 78, 1))
	= returns true if ped is aiming/shooting a gun
</summary>
]]--
native "GET_PED_CONFIG_FLAG"
	hash "0x7EE53118C892B513"
	jhash (0xABE98267)
	arguments {
		Ped "ped",

		int "flagId",

		BOOL "p2",
	}
	returns	"BOOL"

native "GET_PED_RESET_FLAG"
	hash "0xAF9E59B1B1FBF2A0"
	jhash (0x2FC10D11)
	arguments {
		Ped "ped",

		int "flagId",
	}
	returns	"BOOL"

native "SET_PED_GROUP_MEMBER_PASSENGER_INDEX"
	hash "0x0BDDB8D9EC6BCF3C"
	jhash (0x2AB3670B)
	arguments {
		Ped "ped",

		int "index",
	}
	returns	"void"

native "SET_PED_CAN_EVASIVE_DIVE"
	hash "0x6B7A646C242A7059"
	jhash (0x542FEB4D)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Presumably returns the Entity that the Ped is currently diving out of the way of.

	var num3;
	    if (PED::IS_PED_EVASIVE_DIVING(A_0, &amp;num3) != 0)
	        if (ENTITY::IS_ENTITY_A_VEHICLE(num3) != 0)
</summary>
]]--
native "IS_PED_EVASIVE_DIVING"
	hash "0x414641C26E105898"
	jhash (0xD82829DC)
	arguments {
		Ped "ped",

		EntityPtr "evadingEntity",
	}
	returns	"BOOL"

native "SET_PED_SHOOTS_AT_COORD"
	hash "0x96A05E4FB321B1BA"
	jhash (0xFD64EAE5)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_MODEL_IS_SUPPRESSED"
	hash "0xE163A4BCE4DE6F11"
	jhash (0x7820CA43)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "STOP_ANY_PED_MODEL_BEING_SUPPRESSED"
	hash "0xB47BD05FA66B40CF"
	jhash (0x5AD7DC55)
	returns	"void"

native "SET_PED_CAN_BE_TARGETED_WHEN_INJURED"
	hash "0x638C03B0F9878F57"
	jhash (0x6FD9A7CD)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_GENERATES_DEAD_BODY_EVENTS"
	hash "0x7FB17BA2E7DECA5B"
	jhash (0xE9B97A2B)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "0xE43A13C9E4CCCBCF"
	hash "0xE43A13C9E4CCCBCF"
	jhash (0xFF1F6AEB)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT"
	hash "0xDF993EE5E90ABA25"
	jhash (0xE9BD733A)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	PoliceMotorcycleHelmet	1024	
	RegularMotorcycleHelmet	4096	
	FiremanHelmet	16384	
	PilotHeadset	32768	
	PilotHelmet	65536
	--
	p2 is generally 4096 or 16384 in the scripts. p3 is always -1, and p1 varies between 1 and 0.
</summary>
]]--
native "GIVE_PED_HELMET"
	hash "0x54C7C4A94367717E"
	jhash (0x1862A461)
	arguments {
		Ped "ped",

		BOOL "p1",

		int "helmetFlag",

		Any "p3",
	}
	returns	"void"

native "REMOVE_PED_HELMET"
	hash "0xA7B2458D0AD6DED8"
	jhash (0x2086B1F0)
	arguments {
		Ped "ped",

		BOOL "p1",
	}
	returns	"void"

native "0x14590DDBEDB1EC85"
	hash "0x14590DDBEDB1EC85"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SET_PED_HELMET"
	hash "0x560A43136EB58105"
	jhash (0xED366E53)
	arguments {
		Ped "ped",

		BOOL "p1",
	}
	returns	"void"

native "SET_PED_HELMET_FLAG"
	hash "0xC0E78D5C2CE3EB25"
	jhash (0x12677780)
	arguments {
		Ped "ped",

		int "helmetFlag",
	}
	returns	"void"

native "SET_PED_HELMET_PROP_INDEX"
	hash "0x26D83693ED99291C"
	jhash (0xA316D13F)
	arguments {
		Ped "ped",

		int "propIndex",
	}
	returns	"void"

native "SET_PED_HELMET_TEXTURE_INDEX"
	hash "0xF1550C4BD22582E2"
	jhash (0x5F6C3328)
	arguments {
		Ped "ped",

		int "textureIndex",
	}
	returns	"void"

native "IS_PED_WEARING_HELMET"
	hash "0xF33BDFE19B309B19"
	jhash (0x0D680D49)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "0x687C0B594907D2E8"
	hash "0x687C0B594907D2E8"
	jhash (0x24A1284E)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x451294E859ECC018"
	hash "0x451294E859ECC018"
	jhash (0x8A3A3116)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x9D728C1E12BF5518"
	hash "0x9D728C1E12BF5518"
	jhash (0x74EB662D)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xF2385935BFFD4D92"
	hash "0xF2385935BFFD4D92"
	jhash (0xFFF149FE)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SET_PED_TO_LOAD_COVER"
	hash "0x332B562EEDA62399"
	jhash (0xCF94BA97)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_COWER_IN_COVER"
	hash "0xCB7553CDCEF4A735"
	jhash (0x5194658B)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_PEEK_IN_COVER"
	hash "0xC514825C507E3736"
	jhash (0xC1DAE216)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_PLAYS_HEAD_ON_HORN_ANIM_WHEN_DIES_IN_VEHICLE"
	hash "0x94D94BF1A75AED3D"
	jhash (0x7C563CD2)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	"IK" stands for "Inverse kinematics." I assume this has something to do with how the ped uses his legs to balance. In the scripts, the second parameter is always an int with a value of 2, 0, or sometimes 1
</summary>
]]--
native "SET_PED_LEG_IK_MODE"
	hash "0xC396F5B86FF9FEBD"
	jhash (0xFDDB042E)
	arguments {
		Ped "ped",

		int "mode",
	}
	returns	"void"

native "SET_PED_MOTION_BLUR"
	hash "0x0A986918B102B448"
	jhash (0xA211A128)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_CAN_SWITCH_WEAPON"
	hash "0xED7F7EFE9FABF340"
	jhash (0xB5F8BA28)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_DIES_INSTANTLY_IN_WATER"
	hash "0xEEB64139BA29A7CF"
	jhash (0xFE2554FC)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "0x1A330D297AAC6BC1"
	hash "0x1A330D297AAC6BC1"
	jhash (0x77BB7CB8)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "STOP_PED_WEAPON_FIRING_WHEN_DROPPED"
	hash "0xC158D28142A34608"
	jhash (0x4AC3421E)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_SCRIPTED_ANIM_SEAT_OFFSET"
	hash "0x5917BBA32D06C230"
	jhash (0x7CEFFA45)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	0 - Stationary (Will just stand in place)
	1 - Defensive (Will try to find cover and very likely to blind fire)
	2 - Offensive (Will attempt to charge at enemy but take cover as well)
	3 - Suicidal Offensive (Will try to flank enemy in a suicidal attack)
</summary>
]]--
native "SET_PED_COMBAT_MOVEMENT"
	hash "0x4D9CA1009AFBD057"
	jhash (0x12E62F9E)
	arguments {
		Ped "ped",

		int "combatMovement",
	}
	returns	"void"

native "GET_PED_COMBAT_MOVEMENT"
	hash "0xDEA92412FCAEB3F5"
	jhash (0xF3E7730E)
	arguments {
		Ped "ped",
	}
	returns	"Any"

--[[!
<summary>
	100 would equal attack
	less then 50ish would mean run away

	Only the values 0, 1 and 2 occur in the decompiled scripts. Most likely refers directly to the values also described in combatbehaviour.meta:
	0: CA_Poor
	1: CA_Average
	2: CA_Professional
</summary>
]]--
native "SET_PED_COMBAT_ABILITY"
	hash "0xC7622C0D36B2FDA8"
	jhash (0x6C23D329)
	arguments {
		Ped "ped",

		int "p1",
	}
	returns	"void"

--[[!
<summary>
	Only the values 0, 1 and 2 occur in the decompiled scripts. Most likely refers directly to the values also described as AttackRange in combatbehaviour.meta:
	0: CR_Near
	1: CR_Medium
	2: CR_Far
</summary>
]]--
native "SET_PED_COMBAT_RANGE"
	hash "0x3C606747B23E497B"
	jhash (0x8818A959)
	arguments {
		Ped "ped",

		int "p1",
	}
	returns	"void"

native "GET_PED_COMBAT_RANGE"
	hash "0xF9D9F7F2DB8E2FA0"
	jhash (0x9B9B7163)
	arguments {
		Ped "ped",
	}
	returns	"Any"

--[[!
<summary>
	attributeIndex = 46 : fight to death
	attributeIndex = 3 : get out of the car while in combat
</summary>
]]--
native "SET_PED_COMBAT_ATTRIBUTES"
	hash "0x9F7794730795E019"
	jhash (0x81D64248)
	arguments {
		Ped "ped",

		int "attributeIndex",

		BOOL "p2",
	}
	returns	"void"

native "SET_PED_TARGET_LOSS_RESPONSE"
	hash "0x0703B9079823DA4A"
	jhash (0xCFA613FF)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xDCCA191DF9980FD7"
	hash "0xDCCA191DF9980FD7"
	jhash (0x139C0875)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "IS_PED_PERFORMING_STEALTH_KILL"
	hash "0xFD4CCDBCC59941B7"
	jhash (0x9ADD7B21)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "0xEBD0EDBA5BE957CF"
	hash "0xEBD0EDBA5BE957CF"
	jhash (0x9BE7C860)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_BEING_STEALTH_KILLED"
	hash "0x863B23EFDE9C5DF2"
	jhash (0xD044C8AF)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "GET_MELEE_TARGET_FOR_PED"
	hash "0x18A3E9EE1297FD39"
	jhash (0xAFEC26A4)
	arguments {
		Ped "ped",
	}
	returns	"Ped"

native "WAS_PED_KILLED_BY_STEALTH"
	hash "0xF9800AA1A771B000"
	jhash (0x2EA4B54E)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "WAS_PED_KILLED_BY_TAKEDOWN"
	hash "0x7F08E26039C7347C"
	jhash (0xBDD3CE69)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "0x61767F73EACEED21"
	hash "0x61767F73EACEED21"
	jhash (0x3993092B)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SET_PED_FLEE_ATTRIBUTES"
	hash "0x70A2D1137C8ED7C9"
	jhash (0xA717A875)
	arguments {
		Ped "ped",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "SET_PED_COWER_HASH"
	hash "0xA549131166868ED3"
	jhash (0x16F30DF4)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x2016C603D6B8987C"
	hash "0x2016C603D6B8987C"
	jhash (0xA6F2C057)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_PED_STEERS_AROUND_PEDS"
	hash "0x46F2193B3AD1D891"
	jhash (0x797CAE4F)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_STEERS_AROUND_OBJECTS"
	hash "0x1509C089ADC208BF"
	jhash (0x3BD9B0A6)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_STEERS_AROUND_VEHICLES"
	hash "0xEB6FB9D48DDE23EC"
	jhash (0x533C0651)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "0xA9B61A329BFDCBEA"
	hash "0xA9B61A329BFDCBEA"
	jhash (0x2276DE0D)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x570389D1C3DE3C6B"
	hash "0x570389D1C3DE3C6B"
	jhash (0x59C52BE6)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x576594E8D64375E2"
	hash "0x576594E8D64375E2"
	jhash (0x1D87DDC1)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xA52D5247A4227E14"
	hash "0xA52D5247A4227E14"
	jhash (0xB52BA5F5)
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_ANY_PED_NEAR_POINT"
	hash "0x083961498679DC9F"
	jhash (0xFBD9B050)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",
	}
	returns	"BOOL"

native "0x2208438012482A1A"
	hash "0x2208438012482A1A"
	jhash (0x187B9070)
	arguments {
		Ped "ped",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xFCF37A457CB96DC0"
	hash "0xFCF37A457CB96DC0"
	jhash (0x45037B9B)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"BOOL"

native "0x7D7A2E43E74E2EB8"
	hash "0x7D7A2E43E74E2EB8"
	jhash (0x840D24D3)
	arguments {
		Any "p0",
	}
	returns	"void"

native "GET_PED_FLOOD_INVINCIBILITY"
	hash "0x2BC338A7B21F4608"
	jhash (0x31C31DAA)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xCD018C591F94CB43"
	hash "0xCD018C591F94CB43"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x75BA1CB3B7D40CAF"
	hash "0x75BA1CB3B7D40CAF"
	jhash (0x9194DB71)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	returns whether or not a ped is visible within your FOV, not this check auto's to false after a certain distance.

	~ Lynxaa
</summary>
]]--
native "IS_TRACKED_PED_VISIBLE"
	hash "0x91C8E617F64188AC"
	jhash (0x33248CC1)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "0x511F1A683387C7E2"
	hash "0x511F1A683387C7E2"
	jhash (0x5B1B70AA)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "IS_PED_TRACKED"
	hash "0x4C5E1F087CD10BB7"
	jhash (0x7EB613D9)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "HAS_PED_RECEIVED_EVENT"
	hash "0x8507BCB710FA6DC0"
	jhash (0xECD73DB0)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x6CD5A433374D4CFB"
	hash "0x6CD5A433374D4CFB"
	jhash (0x74A0F291)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x9C6A6C19B6C0C496"
	hash "0x9C6A6C19B6C0C496"
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "GET_PED_BONE_INDEX"
	hash "0x3F428D08BE5AAE31"
	jhash (0x259C6BA2)
	arguments {
		Ped "ped",

		int "boneId",
	}
	returns	"int"

native "GET_PED_RAGDOLL_BONE_INDEX"
	hash "0x2057EF813397A772"
	jhash (0x849F0716)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "SET_PED_ENVEFF_SCALE"
	hash "0xBF29516833893561"
	jhash (0xFC1CFC27)
	arguments {
		Ped "ped",

		float "value",
	}
	returns	"void"

native "GET_PED_ENVEFF_SCALE"
	hash "0x9C14D30395A51A3C"
	jhash (0xA3421E39)
	arguments {
		Ped "ped",
	}
	returns	"float"

native "SET_ENABLE_PED_ENVEFF_SCALE"
	hash "0xD2C5AA0C0E8D0F1E"
	jhash (0xC70F4A84)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	In agency_heist3b.c4, its like this 90% of the time:

	PED::_110F526AB784111F(ped, 0.099);
	PED::SET_PED_ENVEFF_SCALE(ped, 1.0);
	PED::_D69411AA0CEBF9E9(ped, 87, 81, 68);
	PED::SET_ENABLE_PED_ENVEFF_SCALE(ped, 1);

	and its like this 10% of the time:

	PED::_110F526AB784111F(ped, 0.2);
	PED::SET_PED_ENVEFF_SCALE(ped, 0.65);
	PED::_D69411AA0CEBF9E9(ped, 74, 69, 60);
	PED::SET_ENABLE_PED_ENVEFF_SCALE(ped, 1);
</summary>
]]--
native "0x110F526AB784111F"
	hash "0x110F526AB784111F"
	jhash (0x3B882533)
	arguments {
		Ped "ped",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	Something related to the environmental effects natives.
	In the "agency_heist3b" script, p1 - p3 are always under 100 - usually they are {87, 81, 68}. If SET_PED_ENVEFF_SCALE is set to 0.65 (instead of the usual 1.0), they use {74, 69, 60}
</summary>
]]--
native "0xD69411AA0CEBF9E9"
	hash "0xD69411AA0CEBF9E9"
	jhash (0x87A0C174)
	arguments {
		Ped "ped",

		int "p1",

		int "p2",

		int "p3",
	}
	returns	"void"

native "0x1216E0BFA72CC703"
	hash "0x1216E0BFA72CC703"
	jhash (0x7BD26837)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x2B5AA717A181FB4C"
	hash "0x2B5AA717A181FB4C"
	jhash (0x98E29ED0)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	if (!$B8B52E498014F5B0(PLAYER::PLAYER_PED_ID())) {
</summary>
]]--
native "0xB8B52E498014F5B0"
	hash "0xB8B52E498014F5B0"
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "CREATE_SYNCHRONIZED_SCENE"
	hash "0x8C18E0F9080ADD73"
	jhash (0xFFDDF8FA)
	arguments {
		float "x",

		float "y",

		float "z",

		float "p3",

		float "p4",

		float "p5",

		Any "p6",
	}
	returns	"Any"

--[[!
<summary>
	Also creates a synchronized scene. Difference from this and CREATE_SYNCHRONIZED_SCENE is currently unknown.
</summary>
]]--
native "0x62EC273D00187DCA"
	hash "0x62EC273D00187DCA"
	jhash (0xF3876894)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",
	}
	returns	"Any"

native "IS_SYNCHRONIZED_SCENE_RUNNING"
	hash "0x25D39B935A038A26"
	jhash (0x57A282F1)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SET_SYNCHRONIZED_SCENE_ORIGIN"
	hash "0x6ACF6B7225801CD7"
	jhash (0x2EC2A0B2)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		Any "p7",
	}
	returns	"void"

native "SET_SYNCHRONIZED_SCENE_PHASE"
	hash "0x734292F4F0ABF6D0"
	jhash (0xF5AB0D98)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "GET_SYNCHRONIZED_SCENE_PHASE"
	hash "0xE4A310B1D7FA73CC"
	jhash (0xB0B2C852)
	arguments {
		Any "scene",
	}
	returns	"float"

native "SET_SYNCHRONIZED_SCENE_RATE"
	hash "0xB6C49F8A5E295A5D"
	jhash (0xF10112FD)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "GET_SYNCHRONIZED_SCENE_RATE"
	hash "0xD80932D577274D40"
	jhash (0x89365F0D)
	arguments {
		Any "p0",
	}
	returns	"float"

native "SET_SYNCHRONIZED_SCENE_LOOPED"
	hash "0xD9A897A4C6C2974F"
	jhash (0x32ED9F82)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_SYNCHRONIZED_SCENE_LOOPED"
	hash "0x62522002E0C391BA"
	jhash (0x47D87A84)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x394B9CD12435C981"
	hash "0x394B9CD12435C981"
	jhash (0x2DE48DA1)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x7F2F4F13AC5257EF"
	hash "0x7F2F4F13AC5257EF"
	jhash (0x72CF2514)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "ATTACH_SYNCHRONIZED_SCENE_TO_ENTITY"
	hash "0x272E4723B56A3B96"
	jhash (0xE9BA6189)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "DETACH_SYNCHRONIZED_SCENE"
	hash "0x6D38F1F04CBB37EA"
	jhash (0x52A1CAB2)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xCD9CC7E200A52A6F"
	hash "0xCD9CC7E200A52A6F"
	jhash (0xBF7F9035)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Some motionstate hashes are

	0xec17e58, 0xbac0f10b, 0x3f67c6af, 0x422d7a25, 0xbd8817db, 0x916e828c

	and those for the strings

	"motionstate_idle", "motionstate_walk", "motionstate_run", "motionstate_actionmode_idle", and "motionstate_actionmode_walk". 
</summary>
]]--
native "FORCE_PED_MOTION_STATE"
	hash "0xF28965D04F570DCA"
	jhash (0x164DDEFF)
	arguments {
		Ped "ped",

		Hash "motionStateHash",

		BOOL "p2",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"BOOL"

native "0xF60165E1D2C5370B"
	hash "0xF60165E1D2C5370B"
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "SET_PED_MAX_MOVE_BLEND_RATIO"
	hash "0x433083750C5E064A"
	jhash (0xEAD0269A)
	arguments {
		Ped "ped",

		float "value",
	}
	returns	"void"

native "SET_PED_MIN_MOVE_BLEND_RATIO"
	hash "0x01A898D26E2333DD"
	jhash (0x383EC364)
	arguments {
		Ped "ped",

		float "value",
	}
	returns	"void"

native "SET_PED_MOVE_RATE_OVERRIDE"
	hash "0x085BF80FA50A39D1"
	jhash (0x900008C6)
	arguments {
		Ped "ped",

		float "value",
	}
	returns	"void"

native "0x46B05BCAE43856B0"
	hash "0x46B05BCAE43856B0"
	jhash (0x79543043)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns size of array, passed into the second variable.

	See below for usage information.

	&gt;moment0
	This function actually requires a struct, where the first value is the maximum number of elements to return.  Here is a sample of how I was able to get it to work correctly, without yet knowing the struct format.

	//Setup the array
		const int numElements = 10;
		const int arrSize = numElements * 2 + 2;
		Any veh[arrSize];
		//0 index is the size of the array
		veh[0] = numElements;

		int count = PED::GET_PED_NEARBY_VEHICLES(PLAYER::PLAYER_PED_ID(), veh);

		if (veh != NULL)
		{
			//Simple loop to go through results
			for (int i = 0; i &lt; count; i++)
			{
				int offsettedID = i * 2 + 2;
				//Make sure it exists
				if (veh[offsettedID] != NULL &amp;&amp; ENTITY::DOES_ENTITY_EXIST(veh[offsettedID]))
				{
					//Do something
				}
			}
		}  
</summary>
]]--
native "GET_PED_NEARBY_VEHICLES"
	hash "0xCFF869CBFA210D82"
	jhash (0xCB716F68)
	arguments {
		Ped "ped",

		intPtr "sizeAndVehs",
	}
	returns	"int"

--[[!
<summary>
	sizeAndPeds is a pointer to an array. The array is filled with peds found nearby the ped supplied to the first argument.

	p2 tend to be -1 (could be pedType)

	Return value is the number of peds found and added to the array passed.

	-----------------------------------

	Example of returned array:
	0: 4 (This value need to be set before passing the array to this function)
	1: -858993460
	2: 31274
	3: -858993460
	4: 7211
	5: -858993460
	6: 50732
	7: -858993460
	8: -858993460

	Return : 3

	-----------------------------------

	Example: http://gtaforums.com/topic/789788-function-args-to-pedget-ped-nearby-peds/?p=1067386687
</summary>
]]--
native "GET_PED_NEARBY_PEDS"
	hash "0x23F8F5FC7E8C4A6B"
	jhash (0x4D3325F4)
	arguments {
		Ped "ped",

		intPtr "sizeAndPeds",

		int "p2",
	}
	returns	"int"

native "0x7350823473013C02"
	hash "0x7350823473013C02"
	jhash (0xF9FB4B71)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "IS_PED_USING_ACTION_MODE"
	hash "0x00E73468D085F745"
	jhash (0x5AE7EDA2)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	p2 is usually -1 in the scripts. action is either 0 or "DEFAULT_ACTION".
</summary>
]]--
native "SET_PED_USING_ACTION_MODE"
	hash "0xD75ACCF5E0FB5367"
	jhash (0x8802F696)
	arguments {
		Ped "ped",

		BOOL "p1",

		Any "p2",

		charPtr "action",
	}
	returns	"void"

native "0x781DE8FA214E87D2"
	hash "0x781DE8FA214E87D2"
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

--[[!
<summary>
	p1 is a float between 0 and 1.
</summary>
]]--
native "SET_PED_CAPSULE"
	hash "0x364DF566EC833DE2"
	jhash (0xB153E1B9)
	arguments {
		Ped "ped",

		float "p1",
	}
	returns	"void"

native "REGISTER_PEDHEADSHOT"
	hash "0x4462658788425076"
	jhash (0xFFE2667B)
	arguments {
		Ped "ped",
	}
	returns	"Any"

native "0x953563CE563143AF"
	hash "0x953563CE563143AF"
	jhash (0x4DD03628)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "UNREGISTER_PEDHEADSHOT"
	hash "0x96B1361D9B24C2FF"
	jhash (0x0879AE45)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "IS_PEDHEADSHOT_VALID"
	hash "0xA0A9668F158129A2"
	jhash (0x0B1080C4)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PEDHEADSHOT_READY"
	hash "0x7085228842B13A67"
	jhash (0x761CD02E)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "GET_PEDHEADSHOT_TXD_STRING"
	hash "0xDB4EACD4AD0A5D6B"
	jhash (0x76D28E96)
	arguments {
		Ped "ped",
	}
	returns	"charPtr"

native "0xF0DAEF2F545BEE25"
	hash "0xF0DAEF2F545BEE25"
	jhash (0x10F2C023)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x5D517B27CF6ECD04"
	hash "0x5D517B27CF6ECD04"
	jhash (0x0DBB2FA7)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xEBB376779A760AA8"
	hash "0xEBB376779A760AA8"
	jhash (0x810158F8)
	returns	"Any"

native "0x876928DDDFCCC9CD"
	hash "0x876928DDDFCCC9CD"
	jhash (0x05023F8F)
	returns	"Any"

native "0xE8A169E666CBC541"
	hash "0xE8A169E666CBC541"
	jhash (0xAA39FD6C)
	returns	"Any"

native "0xC1F6EBF9A3D55538"
	hash "0xC1F6EBF9A3D55538"
	jhash (0xEF9142DB)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x600048C60D5C2C51"
	hash "0x600048C60D5C2C51"
	jhash (0x0688DE64)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x2DF9038C90AD5264"
	hash "0x2DF9038C90AD5264"
	jhash (0x909A1D76)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",

		float "p6",

		Any "p7",
	}
	returns	"void"

native "0xB2AFF10216DEFA2F"
	hash "0xB2AFF10216DEFA2F"
	jhash (0x4AAD0ECB)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		Any "p7",

		float "p8",

		Any "p9",
	}
	returns	"void"

native "0xFEE4A5459472A9F8"
	hash "0xFEE4A5459472A9F8"
	jhash (0x492C9E46)
	returns	"void"

native "0x3C67506996001F5E"
	hash "0x3C67506996001F5E"
	jhash (0x814A28F4)
	returns	"Any"

native "0xA586FBEB32A53DBB"
	hash "0xA586FBEB32A53DBB"
	jhash (0x0B60D2BA)
	returns	"Any"

native "0xF445DE8DA80A1792"
	hash "0xF445DE8DA80A1792"
	jhash (0x6B83ABDF)
	returns	"Any"

native "0xA635C11B8C44AFC2"
	hash "0xA635C11B8C44AFC2"
	jhash (0xF46B4DC8)
	returns	"Any"

native "0x280C7E3AC7F56E90"
	hash "0x280C7E3AC7F56E90"
	jhash (0x36A4AC65)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"void"

native "0xB782F8238512BAD5"
	hash "0xB782F8238512BAD5"
	jhash (0xBA699DDF)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "SET_IK_TARGET"
	hash "0xC32779C16FCEECD9"
	jhash (0x6FE5218C)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		float "p4",

		float "p5",

		float "p6",

		Any "p7",

		Any "p8",

		Any "p9",
	}
	returns	"void"

native "0xED3C76ADFA6D07C4"
	hash "0xED3C76ADFA6D07C4"
	jhash (0xFB4000DC)
	arguments {
		Any "p0",
	}
	returns	"void"

native "REQUEST_ACTION_MODE_ASSET"
	hash "0x290E2780BB7AA598"
	jhash (0x572BA553)
	arguments {
		charPtr "asset",
	}
	returns	"void"

native "HAS_ACTION_MODE_ASSET_LOADED"
	hash "0xE4B5F4BF2CB24E65"
	jhash (0xF7EB2BF1)
	arguments {
		charPtr "asset",
	}
	returns	"BOOL"

native "REMOVE_ACTION_MODE_ASSET"
	hash "0x13E940F88470FA51"
	jhash (0x3F480F92)
	arguments {
		charPtr "asset",
	}
	returns	"void"

native "REQUEST_STEALTH_MODE_ASSET"
	hash "0x2A0A62FCDEE16D4F"
	jhash (0x280A004A)
	arguments {
		charPtr "asset",
	}
	returns	"void"

native "HAS_STEALTH_MODE_ASSET_LOADED"
	hash "0xE977FC5B08AF3441"
	jhash (0x39245667)
	arguments {
		charPtr "asset",
	}
	returns	"BOOL"

native "REMOVE_STEALTH_MODE_ASSET"
	hash "0x9219857D21F0E842"
	jhash (0x8C0B243A)
	arguments {
		charPtr "asset",
	}
	returns	"void"

native "SET_PED_LOD_MULTIPLIER"
	hash "0xDC2C5C242AAC342B"
	jhash (0x1D2B5C70)
	arguments {
		Ped "ped",

		float "multiplier",
	}
	returns	"void"

native "0xE861D0B05C7662B8"
	hash "0xE861D0B05C7662B8"
	jhash (0x2F9550C2)
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",
	}
	returns	"void"

native "0x129466ED55140F8D"
	hash "0x129466ED55140F8D"
	jhash (0x37DBC2AD)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xCB968B53FC7F916D"
	hash "0xCB968B53FC7F916D"
	jhash (0xC0F1BC91)
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x68772DB2B2526F9F"
	hash "0x68772DB2B2526F9F"
	jhash (0x1A464167)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"BOOL"

native "0x06087579E7AA85A9"
	hash "0x06087579E7AA85A9"
	jhash (0xD0567D41)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"BOOL"

native "0xD8C3BE3EE94CAF2D"
	hash "0xD8C3BE3EE94CAF2D"
	jhash (0x4BBE5E2C)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "0xD33DAA36272177C4"
	hash "0xD33DAA36272177C4"
	jhash (0xA89A53F2)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x83A169EABCDB10A2"
	hash "0x83A169EABCDB10A2"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x288DF530C92DAD6F"
	hash "0x288DF530C92DAD6F"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	vb.net
	Public Function Create_Vehicle(vh As VehicleHash, x As Single, y As Single, z As Single, h As Single, retNetHandle As Boolean, retHandle As Boolean) As Vehicle
	        Return Native.Function.Call(Of Vehicle)(Hash.CREATE_VEHICLE, vh, x, y, z, h, retNetHandle, retHandle)
	    End Function
</summary>
]]--
native "CREATE_VEHICLE"
	hash "0xAF35D0D2583051B0"
	jhash (0xDD75460A)
	arguments {
		Hash "modelHash",

		float "x",

		float "y",

		float "z",

		float "heading",

		BOOL "networkHandle",

		BOOL "vehiclehandle",
	}
	returns	"Vehicle"

--[[!
<summary>
	Deletes a vehicle.
	the vehicle cant be a mission entity to delete, so call this before deleting SET_ENTITY_AS_MISSION_ENTITY(vehicle, false, true);

	eg how to use:
	ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, true, false);
	VEHICLE::DELETE_VEHICLE(&amp;veh);

	Check http://gtaforums.com/topic/789907-vrel-community-script-hook-v-net/?p=1067490925
</summary>
]]--
native "DELETE_VEHICLE"
	hash "0xEA386986E786A54F"
	jhash (0x9803AF60)
	arguments {
		VehiclePtr "vehicle",
	}
	returns	"void"

native "0x7D6F9A3EF26136A0"
	hash "0x7D6F9A3EF26136A0"
	jhash (0xBB54ECCA)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Makes the vehicle accept no passengers.
</summary>
]]--
native "SET_VEHICLE_ALLOW_NO_PASSENGERS_LOCKON"
	hash "0x5D14D4154BFE7B2C"
	jhash (0x8BAAC437)
	arguments {
		Vehicle "veh",

		BOOL "p1",
	}
	returns	"void"

native "0xE6B0E8CFC3633BF0"
	hash "0xE6B0E8CFC3633BF0"
	jhash (0xFBDE9FD8)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

--[[!
<summary>
	c#
	if (VEHICLE::IS_VEHICLE_MODEL(rPtr(A_1), 0xC703DB5F) != 0)

	vb.net
	Public Function Is_Model_Vehicle(vh As Vehicle, hash As Hash) As Boolean
	        Return Native.Function.Call(Of Boolean)(Native.Hash.IS_VEHICLE_MODEL, vh, hash)
	    End Function
</summary>
]]--
native "IS_VEHICLE_MODEL"
	hash "0x423E8DE37D934D89"
	jhash (0x013B10B6)
	arguments {
		Vehicle "vehicle",

		Hash "hash",
	}
	returns	"BOOL"

native "DOES_SCRIPT_VEHICLE_GENERATOR_EXIST"
	hash "0xF6086BC836400876"
	jhash (0xF6BDDA30)
	arguments {
		Any "vehicleGenerator",
	}
	returns	"BOOL"

--[[!
<summary>
	Creates a script vehicle generator at the given coordinates. Most parameters after the model hash are unknown.

	Parameters:
	x/y/z - Generator position
	heading - Generator heading
	p4 - Unknown (always 5.0)
	p5 - Unknown (always 3.0)
	modelHash - Vehicle model hash
	p7/8/9/10 - Unknown (always -1)
	p11 - Unknown (usually TRUE, only one instance of FALSE)
	p12/13 - Unknown (always FALSE)
	p14 - Unknown (usally FALSE, only two instances of TRUE)
	p15 - Unknown (always TRUE)
	p16 - Unknown (always -1)
	--------Not Sure You FOund This--------------

	------------Lachie4145--------------------
		Vector3 coords = GET_ENTITY_COORDS(PLAYER_PED_ID(), 0);
		CREATE_SCRIPT_VEHICLE_GENERATOR(coords.x, coords.y, coords.z, 1.0f, 5.0f, 3.0f, GET_HASH_KEY("adder"), -1. -1, -1, -1, -1, true, false, false, false, true, -1);
</summary>
]]--
native "CREATE_SCRIPT_VEHICLE_GENERATOR"
	hash "0x9DEF883114668116"
	jhash (0x25A9A261)
	arguments {
		float "x",

		float "y",

		float "z",

		float "heading",

		float "p4",

		float "p5",

		Hash "modelHash",

		int "p7",

		int "p8",

		int "p9",

		int "p10",

		BOOL "p11",

		BOOL "p12",

		BOOL "p13",

		BOOL "p14",

		BOOL "p15",

		int "p16",
	}
	returns	"Any"

native "DELETE_SCRIPT_VEHICLE_GENERATOR"
	hash "0x22102C9ABFCF125D"
	jhash (0xE4328E3F)
	arguments {
		Any "vehicleGenerator",
	}
	returns	"void"

--[[!
<summary>
	Only called once in the decompiled scripts. Presumably activates the specified generator.
</summary>
]]--
native "SET_SCRIPT_VEHICLE_GENERATOR"
	hash "0xD9D620E0AC6DC4B0"
	jhash (0x40D73747)
	arguments {
		Any "vehicleGenerator",

		BOOL "enabled",
	}
	returns	"void"

native "SET_ALL_VEHICLE_GENERATORS_ACTIVE_IN_AREA"
	hash "0xC12321827687FE4D"
	jhash (0xB4E0E69A)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"void"

native "SET_ALL_VEHICLE_GENERATORS_ACTIVE"
	hash "0x34AD89078831A4BC"
	jhash (0xAB1FDD76)
	returns	"void"

native "SET_ALL_LOW_PRIORITY_VEHICLE_GENERATORS_ACTIVE"
	hash "0x608207E7A8FB787C"
	jhash (0x87F767F2)
	arguments {
		BOOL "active",
	}
	returns	"void"

native "0x9A75585FB2E54FAD"
	hash "0x9A75585FB2E54FAD"
	jhash (0x935A95DA)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "0x0A436B8643716D14"
	hash "0x0A436B8643716D14"
	jhash (0x6C73E45A)
	returns	"void"

--[[!
<summary>
	Sets a vehicle on the ground on all wheels.  Returns whether or not the operation was successful.
</summary>
]]--
native "SET_VEHICLE_ON_GROUND_PROPERLY"
	hash "0x49733E92263139D1"
	jhash (0xE14FDBA6)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	returns Void. Behaviour unknown.   // gtaVmod
</summary>
]]--
native "SET_ALL_VEHICLES_SPAWN"
	hash "0xE023E8AC4EF7C117"
	jhash (0xA0909ADB)
	arguments {
		Vehicle "p0",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"Any"

native "IS_VEHICLE_STUCK_ON_ROOF"
	hash "0xB497F06B288DCFDF"
	jhash (0x18D07C6C)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "ADD_VEHICLE_UPSIDEDOWN_CHECK"
	hash "0xB72E26D81006005B"
	jhash (0x3A13D384)
	arguments {
		Vehicle "vehicle",
	}
	returns	"void"

native "REMOVE_VEHICLE_UPSIDEDOWN_CHECK"
	hash "0xC53EB42A499A7E90"
	jhash (0xF390BA1B)
	arguments {
		Vehicle "vehicle",
	}
	returns	"void"

--[[!
<summary>
	Returns true if the vehicle's current speed is less than, or equal to 0.0025f.
</summary>
]]--
native "IS_VEHICLE_STOPPED"
	hash "0x5721B434AD84D57A"
	jhash (0x655F072C)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	Gets the number of passengers, NOT including the driver. Use IS_VEHICLE_SEAT_FREE(Vehicle, -1) to also check for the driver
</summary>
]]--
native "GET_VEHICLE_NUMBER_OF_PASSENGERS"
	hash "0x24CB2137731FFE89"
	jhash (0x1EF20849)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

native "GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS"
	hash "0xA7C4F2C6E744A550"
	jhash (0x0A2FC08C)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

native "_GET_VEHICLE_MODEL_MAX_NUMBER_OF_PASSENGERS"
	hash "0x2AD93716F184EDA4"
	jhash (0x838F7BF7)
	arguments {
		Hash "VehicleModel",
	}
	returns	"int"

native "0xF7F203E31F96F6A1"
	hash "0xF7F203E31F96F6A1"
	jhash (0x769E5CF2)
	arguments {
		Vehicle "vehicle",

		BOOL "flag",
	}
	returns	"BOOL"

native "0xE33FFA906CE74880"
	hash "0xE33FFA906CE74880"
	arguments {
		Vehicle "vehicle",

		Any "p1",
	}
	returns	"BOOL"

native "SET_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME"
	hash "0x245A6883D966D537"
	jhash (0xF4187E51)
	arguments {
		float "multiplier",
	}
	returns	"void"

native "SET_RANDOM_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME"
	hash "0xB3B3359379FE77D3"
	jhash (0x543F712B)
	arguments {
		float "multiplier",
	}
	returns	"void"

native "SET_PARKED_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME"
	hash "0xEAE6DCC7EEE3DB1D"
	jhash (0xDD46CEBE)
	arguments {
		float "multiplier",
	}
	returns	"void"

native "0xD4B8E3D1917BC86B"
	hash "0xD4B8E3D1917BC86B"
	jhash (0x09462665)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x90B6DA738A9A25DA"
	hash "0x90B6DA738A9A25DA"
	jhash (0xDAE2A2BE)
	arguments {
		float "p0",
	}
	returns	"void"

native "SET_FAR_DRAW_VEHICLES"
	hash "0x26324F33423F3CC3"
	jhash (0x9F019C49)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "SET_NUMBER_OF_PARKED_VEHICLES"
	hash "0xCAA15F13EBD417FF"
	jhash (0x206A58E8)
	arguments {
		int "value",
	}
	returns	"Any"

native "SET_VEHICLE_DOORS_LOCKED"
	hash "0xB664292EAECF7FA6"
	jhash (0x4CDD35D0)
	arguments {
		Vehicle "vehicle",

		int "doorLockStatus",
	}
	returns	"void"

--[[!
<summary>
	destroyType is 1 for opens on damage, 2 for breaks on damage.
</summary>
]]--
native "SET_PED_TARGETTABLE_VEHICLE_DESTROY"
	hash "0xBE70724027F85BCD"
	jhash (0xD61D182D)
	arguments {
		Vehicle "vehicle",

		int "vehicleComponent",

		int "destroyType",
	}
	returns	"void"

--[[!
<summary>
	siren has no sound if set to TRUE

	also looks like hash collision
</summary>
]]--
native "DISABLE_VEHICLE_IMPACT_EXPLOSION_ACTIVATION"
	hash "0xD8050E0EB60CF274"
	jhash (0xC54156A9)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "SET_VEHICLE_DOORS_LOCKED_FOR_PLAYER"
	hash "0x517AAF684BB50CD1"
	jhash (0x49829236)
	arguments {
		Vehicle "vehicle",

		Player "player",

		BOOL "toggle",
	}
	returns	"void"

native "GET_VEHICLE_DOORS_LOCKED_FOR_PLAYER"
	hash "0xF6AF6CB341349015"
	jhash (0x1DC50247)
	arguments {
		Vehicle "vehicle",

		Player "player",
	}
	returns	"BOOL"

--[[!
<summary>
	After some analysis, I've decided that these are what the parameters are.

	We can see this being used in R* scripts such as "am_mp_property_int.ysc.c4":
	l_11A1 = PED::GET_VEHICLE_PED_IS_IN(PLAYER::PLAYER_PED_ID(), 1);
	...
	VEHICLE::SET_VEHICLE_DOORS_LOCKED_FOR_ALL_PLAYERS(l_11A1, 1);


</summary>
]]--
native "SET_VEHICLE_DOORS_LOCKED_FOR_ALL_PLAYERS"
	hash "0xA2F80B8D040727CC"
	jhash (0x891BA8A4)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "0x9737A37136F07E75"
	hash "0x9737A37136F07E75"
	jhash (0xE4EF6514)
	arguments {
		Vehicle "veh",

		BOOL "p1",
	}
	returns	"void"

native "SET_VEHICLE_DOORS_LOCKED_FOR_TEAM"
	hash "0xB81F6D4A8F5EEBA8"
	jhash (0x4F85E783)
	arguments {
		Vehicle "vehicle",

		int "team",

		BOOL "toggle",
	}
	returns	"void"

native "EXPLODE_VEHICLE"
	hash "0xBA71116ADF5B514C"
	jhash (0xBEDEACEB)
	arguments {
		Vehicle "vehicle",

		BOOL "isAudible",

		BOOL "isInvisible",
	}
	returns	"void"

--[[!
<summary>
	Tested on the player's current vehicle. Unless you kill the driver, the vehicle doesn't lose control, however, if enabled, explodeOnImpact is still active. The moment you crash, boom.
</summary>
]]--
native "SET_VEHICLE_OUT_OF_CONTROL"
	hash "0xF19D095E42D430CC"
	jhash (0x3764D734)
	arguments {
		Vehicle "vehicle",

		BOOL "killDriver",

		BOOL "explodeOnImpact",
	}
	returns	"void"

--[[!
<summary>
	VEHICLE::SET_VEHICLE_TIMED_EXPLOSION(num4, PLAYER::GET_PLAYER_PED(num6), 1);

	GTA:Online only.
</summary>
]]--
native "SET_VEHICLE_TIMED_EXPLOSION"
	hash "0x2E0A74E1002380B1"
	jhash (0xDB8CB8E2)
	arguments {
		Vehicle "vehicle",

		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "0x99AD4CCCB128CBC9"
	hash "0x99AD4CCCB128CBC9"
	jhash (0x811373DE)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x6ADAABD3068C5235"
	hash "0x6ADAABD3068C5235"
	jhash (0xA4E69134)
	returns	"Any"

native "0xEF49CF0270307CBE"
	hash "0xEF49CF0270307CBE"
	jhash (0x65255524)
	returns	"void"

native "0xAE3FEE8709B39DCB"
	hash "0xAE3FEE8709B39DCB"
	jhash (0xE39DAF36)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	This is not tested - it's just an assumption.


	Doesn't seem to work.  I'll try with an int instead. --JT

	Read the scripts, dumpass. 

	                            if (!VEHICLE::IS_TAXI_LIGHT_ON(l_115)) {
	                                VEHICLE::SET_TAXI_LIGHTS(l_115, 1);
	                            }
</summary>
]]--
native "SET_TAXI_LIGHTS"
	hash "0x598803E85E8448D9"
	jhash (0x68639D85)
	arguments {
		Vehicle "vehicle",

		BOOL "state",
	}
	returns	"void"

native "IS_TAXI_LIGHT_ON"
	hash "0x7504C0F113AB50FC"
	jhash (0x6FC4924A)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	garageName example "Michael - Beverly Hills"

	Check garages.meta for more names.
</summary>
]]--
native "IS_VEHICLE_IN_GARAGE_AREA"
	hash "0xCEE4490CD57BB3C2"
	jhash (0xA90EC257)
	arguments {
		charPtr "garageName",

		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	colorPrimary &amp; colorSecondary are the paint index for the vehicle.
	For a list of valid paint indexes, view: http://pastebin.com/pwHci0xK
</summary>
]]--
native "SET_VEHICLE_COLOURS"
	hash "0x4F1D4BE3A7F24601"
	jhash (0x57F24253)
	arguments {
		Vehicle "vehicle",

		int "colorPrimary",

		int "colorSecondary",
	}
	returns	"void"

--[[!
<summary>
	This is not tested - it's just an assumption.


	seems to always switch to lowbeam, true or false.
	 - Houri 

	@Nac you crazy or what? you can't know if somebody hasn't tested it yet.. you doing so much idiot comments ^^
</summary>
]]--
native "SET_VEHICLE_FULLBEAM"
	hash "0x8B7FD87F0DDB421E"
	jhash (0x9C49CC15)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "STEER_UNLOCK_BIAS"
	hash "0x07116E24E9D1929D"
	jhash (0xA59E3DCD)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	p1, p2, p3 are RGB values for color (255,0,0 for Red, ect)
</summary>
]]--
native "SET_VEHICLE_CUSTOM_PRIMARY_COLOUR"
	hash "0x7141766F91D15BEA"
	jhash (0x8DF9F9BC)
	arguments {
		Vehicle "vehicle",

		int "r",

		int "g",

		int "b",
	}
	returns	"void"

native "GET_VEHICLE_CUSTOM_PRIMARY_COLOUR"
	hash "0xB64CF2CCA9D95F52"
	jhash (0x1C2B9FEF)
	arguments {
		Vehicle "vehicle",

		intPtr "r",

		intPtr "g",

		intPtr "b",
	}
	returns	"void"

native "CLEAR_VEHICLE_CUSTOM_PRIMARY_COLOUR"
	hash "0x55E1D2758F34E437"
	jhash (0x51E1E33D)
	arguments {
		Vehicle "vehicle",
	}
	returns	"Any"

native "GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM"
	hash "0xF095C0405307B21B"
	jhash (0xD7EC8760)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	p1, p2, p3 are RGB values for color (255,0,0 for Red, ect)
</summary>
]]--
native "SET_VEHICLE_CUSTOM_SECONDARY_COLOUR"
	hash "0x36CED73BFED89754"
	jhash (0x9D77259E)
	arguments {
		Vehicle "vehicle",

		int "r",

		int "g",

		int "b",
	}
	returns	"void"

native "GET_VEHICLE_CUSTOM_SECONDARY_COLOUR"
	hash "0x8389CD56CA8072DC"
	jhash (0x3FF247A2)
	arguments {
		Vehicle "vehicle",

		intPtr "r",

		intPtr "g",

		intPtr "b",
	}
	returns	"void"

native "CLEAR_VEHICLE_CUSTOM_SECONDARY_COLOUR"
	hash "0x5FFBDEEC3E8E2009"
	jhash (0x7CE00B29)
	arguments {
		Vehicle "vehicle",
	}
	returns	"Any"

--[[!
<summary>
	Check if Vehicle Secondary is avaliable for customize
</summary>
]]--
native "GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM"
	hash "0x910A32E7AAD2656C"
	jhash (0x288AD228)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	The parameter fade is a value from 0-1, where 0 is fresh paint.
</summary>
]]--
native "_SET_VEHICLE_PAINT_FADE"
	hash "0x3AFDC536C3D01674"
	jhash (0x8332730C)
	arguments {
		Vehicle "veh",

		float "fade",
	}
	returns	"void"

--[[!
<summary>
	The result is a value from 0-1, where 0 is fresh paint.
</summary>
]]--
native "_GET_VEHICLE_PAINT_FADE"
	hash "0xA82819CAC9C4C403"
	jhash (0xD5F1EEE1)
	arguments {
		Vehicle "veh",
	}
	returns	"float"

--[[!
<summary>
	This is not tested - it's just an assumption.


	Nac is a completly idiot.. this native sets your current vehicle if can enter in respray or not.. 1 or 0
</summary>
]]--
native "SET_CAN_RESPRAY_VEHICLE"
	hash "0x52BBA29D5EC69356"
	jhash (0x37677590)
	arguments {
		Vehicle "vehicle",

		BOOL "state",
	}
	returns	"void"

native "0x33506883545AC0DF"
	hash "0x33506883545AC0DF"
	jhash (0x54E9EE75)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	When I called this with what the script was doing, which was -190f for yaw pitch and roll, all my car did was jitter a little. I also tried 0 and 190f. I altered the p1 variable between TRUE and FALSE and didn't see a difference.

	This might have something to do with the physbox of the vehicle, but I'm not sure.
</summary>
]]--
native "_JITTER_VEHICLE"
	hash "0xC59872A5134879C7"
	jhash (0x4A46E814)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",

		float "yaw",

		float "pitch",

		float "roll",
	}
	returns	"void"

native "SET_BOAT_ANCHOR"
	hash "0x75DBEC174AEEAD10"
	jhash (0xA3906284)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "_GET_BOAT_ANCHOR"
	hash "0x26C10ECBDA5D043B"
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	No observed effect.
</summary>
]]--
native "0xE3EBAAE484798530"
	hash "0xE3EBAAE484798530"
	jhash (0x0ED84792)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	No observed effect.
</summary>
]]--
native "0xB28B1FE5BFADD7F5"
	hash "0xB28B1FE5BFADD7F5"
	jhash (0xA739012A)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "0xE842A9398079BD82"
	hash "0xE842A9398079BD82"
	jhash (0x66FA450C)
	arguments {
		Vehicle "vehicle",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	No observed effect.
</summary>
]]--
native "0x8F719973E1445BA2"
	hash "0x8F719973E1445BA2"
	jhash (0x35614622)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Activate siren on vehicle (Only works if the vehicle has a siren).
</summary>
]]--
native "SET_VEHICLE_SIREN"
	hash "0xF4924635A19EB37D"
	jhash (0x4AC1EFC7)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "IS_VEHICLE_SIREN_ON"
	hash "0x4C9BF537BE2634B2"
	jhash (0x25EB5873)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "0xB5CC40FBCB586380"
	hash "0xB5CC40FBCB586380"
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	If set to true, vehicle will not take crash damage, but is still susceptible to damage from bullets and explosives
</summary>
]]--
native "SET_VEHICLE_STRONG"
	hash "0x3E8C8727991A8A0B"
	jhash (0xC758D19F)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "REMOVE_VEHICLE_STUCK_CHECK"
	hash "0x8386BFB614D06749"
	jhash (0x81594917)
	arguments {
		Any "p0",
	}
	returns	"void"

native "GET_VEHICLE_COLOURS"
	hash "0xA19435F193E081AC"
	jhash (0x40D82D88)
	arguments {
		Vehicle "vehicle",

		intPtr "colorPrimary",

		intPtr "colorSecondary",
	}
	returns	"void"

--[[!
<summary>
	Check if a vehicle seat is free.
	-1 being the driver seat.
	Use GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(vehicle) - 1 for last seat index.
</summary>
]]--
native "IS_VEHICLE_SEAT_FREE"
	hash "0x22AC59A870E6A669"
	jhash (0xDAF42B02)
	arguments {
		Vehicle "vehicle",

		int "seatIndex",
	}
	returns	"BOOL"

--[[!
<summary>
	-1 (driver) &lt;= index &lt; GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(vehicle)
</summary>
]]--
native "GET_PED_IN_VEHICLE_SEAT"
	hash "0xBB40DD2270B65366"
	jhash (0x388FDE9A)
	arguments {
		Vehicle "vehicle",

		int "index",
	}
	returns	"Ped"

native "GET_LAST_PED_IN_VEHICLE_SEAT"
	hash "0x83F969AA1EE2A664"
	jhash (0xF7C6792D)
	arguments {
		Vehicle "vehicle",

		int "seatIndex",
	}
	returns	"Ped"

native "GET_VEHICLE_LIGHTS_STATE"
	hash "0xB91B4C20085BD12F"
	jhash (0x7C278621)
	arguments {
		Vehicle "vehicle",

		intPtr "lightsOn",

		intPtr "highbeamsOn",
	}
	returns	"BOOL"

--[[!
<summary>
	wheelID used for 4 wheelers seem to be (0, 1, 4, 5)
	completely - is to check if tire completely gone from rim.
</summary>
]]--
native "IS_VEHICLE_TYRE_BURST"
	hash "0xBA291848A0815CA9"
	jhash (0x48C80210)
	arguments {
		Vehicle "vehicle",

		int "wheelID",

		BOOL "completely",
	}
	returns	"BOOL"

--[[!
<summary>
	SCALE: Setting the speed to 30 would result in a speed of roughly 60mph, according to speedometer.

	Speed is in meters per second
	You can convert meters/s to mph here:
	http://www.calculateme.com/Speed/MetersperSecond/ToMilesperHour.htm
</summary>
]]--
native "SET_VEHICLE_FORWARD_SPEED"
	hash "0xAB54A438726D25D5"
	jhash (0x69880D14)
	arguments {
		Vehicle "vehicle",

		float "speed",
	}
	returns	"void"

native "0x260BE8F09E326A20"
	hash "0x260BE8F09E326A20"
	jhash (0xCBC7D3C8)
	arguments {
		Vehicle "vehicle",

		float "p1",

		Any "p2",

		BOOL "p3",
	}
	returns	"void"

native "0x37EBBF3117BD6A25"
	hash "0x37EBBF3117BD6A25"
	jhash (0x943A6CFC)
	arguments {
		Vehicle "vehicle",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	In the scripts, ringtone is 4.
</summary>
]]--
native "SET_PED_ENABLED_BIKE_RINGTONE"
	hash "0x57715966069157AD"
	jhash (0x7FB25568)
	arguments {
		Vehicle "vehicle",

		int "ringtone",
	}
	returns	"BOOL"

native "0x62CA17B74C435651"
	hash "0x62CA17B74C435651"
	jhash (0x593143B9)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x375E7FC44F21C8AB"
	hash "0x375E7FC44F21C8AB"
	jhash (0x70DD5E25)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x89D630CF5EA96D23"
	hash "0x89D630CF5EA96D23"
	jhash (0xFBF5536A)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x6A98C2ECF57FA5D4"
	hash "0x6A98C2ECF57FA5D4"
	jhash (0x20AB5783)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x7C0043FDFF6436BC"
	hash "0x7C0043FDFF6436BC"
	jhash (0x0F11D01F)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x8AA9180DE2FEDD45"
	hash "0x8AA9180DE2FEDD45"
	jhash (0xAE040377)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x0A6A279F3AA4FD70"
	hash "0x0A6A279F3AA4FD70"
	jhash (0x4C0E4031)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x634148744F385576"
	hash "0x634148744F385576"
	jhash (0x6346B7CC)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xE6F13851780394DA"
	hash "0xE6F13851780394DA"
	jhash (0xCCB41A55)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	"To burst tyres VEHICLE::SET_VEHICLE_TYRE_BURST(vehicle, 0, true, 1000.0)
	to burst all tyres type it 8 times where p1 = 0 to 7. Also, in gtao, it only works if you are the driver, despite creating the sparks fx on your screen"
</summary>
]]--
native "SET_VEHICLE_TYRE_BURST"
	hash "0xEC6A202EE4960385"
	jhash (0x89D28068)
	arguments {
		Vehicle "p0",

		BOOL "p1",

		BOOL "p2",

		float "p3",
	}
	returns	"void"

--[[!
<summary>
	Closes all doors of vehicle (Tested in Metro)
</summary>
]]--
native "SET_VEHICLE_DOORS_SHUT"
	hash "0x781B3D62BB013EF5"
	jhash (0xBB1FF6E7)
	arguments {
		Vehicle "vehicle",

		BOOL "closeInstantly",
	}
	returns	"void"

--[[!
<summary>
	Allows you to toggle bulletproof tires.
</summary>
]]--
native "SET_VEHICLE_TYRES_CAN_BURST"
	hash "0xEB9DC3C7D8596C46"
	jhash (0xA198DB54)
	arguments {
		Vehicle "vehicle",

		BOOL "value",
	}
	returns	"Any"

--[[!
<summary>
	This is not tested - it's just an assumption.
</summary>
]]--
native "GET_VEHICLE_TYRES_CAN_BURST"
	hash "0x678B9BB8C3F58FEB"
	jhash (0x4D76CD2F)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "SET_VEHICLE_WHEELS_CAN_BREAK"
	hash "0x29B18B4FD460CA8F"
	jhash (0x829ED654)
	arguments {
		Vehicle "vehicle",

		BOOL "enabled",
	}
	returns	"Any"

--[[!
<summary>
	doorIndex:
	0 = Front Left Door
	1 = Front Right Door
	2 = Back Left Door
	3 = Back Right Door
	4 = Hood
	5 = Trunk
	6 = Back
	7 = Back2
</summary>
]]--
native "SET_VEHICLE_DOOR_OPEN"
	hash "0x7C65DAC73C35C862"
	jhash (0xBB75D38B)
	arguments {
		Vehicle "vehicle",

		int "doorIndex",

		BOOL "loose",

		BOOL "openInstantly",
	}
	returns	"void"

--[[!
<summary>
	windowIndex:
	0 = Front Right Window
	1 = Front Left Window
	2 = Back Right Window
	3 = Back Left Window
</summary>
]]--
native "REMOVE_VEHICLE_WINDOW"
	hash "0xA711568EEDB43069"
	jhash (0xBB8104A3)
	arguments {
		Vehicle "vehicle",

		int "windowIndex",
	}
	returns	"void"

native "ROLL_DOWN_WINDOWS"
	hash "0x85796B0549DDE156"
	jhash (0x51A16DC6)
	arguments {
		Vehicle "vehicle",
	}
	returns	"Any"

--[[!
<summary>
	windowIndex:
	0 = Front Right Window
	1 = Front Left Window
	2 = Back Right Window
	3 = Back Left Window
</summary>
]]--
native "ROLL_DOWN_WINDOW"
	hash "0x7AD9E6CE657D69E3"
	jhash (0xF840134C)
	arguments {
		Vehicle "vehicle",

		int "windowIndex",
	}
	returns	"Any"

--[[!
<summary>
	0 = Front Right Window
	1 = Front Left Window
	2 = Back Right Window
	3 = Back Left Window
</summary>
]]--
native "ROLL_UP_WINDOW"
	hash "0x602E548F46E24D59"
	jhash (0x83B7E06A)
	arguments {
		Vehicle "vehicle",

		int "windowIndex",
	}
	returns	"Any"

native "SMASH_VEHICLE_WINDOW"
	hash "0x9E5B5E4D2CCD2259"
	jhash (0xDDD9A8C2)
	arguments {
		Vehicle "vehicle",

		int "index",
	}
	returns	"Any"

native "FIX_VEHICLE_WINDOW"
	hash "0x772282EBEB95E682"
	jhash (0x6B8E990D)
	arguments {
		Vehicle "vehicle",

		int "index",
	}
	returns	"Any"

--[[!
<summary>
	Detaches the vehicle's windscreen.
</summary>
]]--
native "_DETACH_VEHICLE_WINDSCREEN"
	hash "0x6D645D59FB5F5AD3"
	jhash (0xCC95C96B)
	arguments {
		Vehicle "vehicleHandle",
	}
	returns	"Any"

native "0xE38CB9D7D39FDBCC"
	hash "0xE38CB9D7D39FDBCC"
	jhash (0xFDA7B6CA)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

--[[!
<summary>
	set's if the vehicle has lights or not.
	not an on off toggle.
	p1 = 0 ;vehicle normal lights, off then lowbeams, then highbeams
	p1 = 1 ;vehicle doesn't have lights, always off
	p1 = 2 ;vehicle has always on lights
	p1 = 3 ;or even larger like 4,5,... normal lights like =1
	note1: when using =2 on day it's lowbeam,highbeam
	but at night it's lowbeam,lowbeam,highbeam
	note2: when using =0 it's affected by day or night for highbeams don't exist in daytime.


	~ Houri ..:)  
</summary>
]]--
native "SET_VEHICLE_LIGHTS"
	hash "0x34E710FF01247C5A"
	jhash (0xE8930226)
	arguments {
		Vehicle "vehicle",

		int "p1",
	}
	returns	"void"

native "0xC45C27EF50F36ADC"
	hash "0xC45C27EF50F36ADC"
	jhash (0x4221E435)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x1FD09E7390A74D54"
	hash "0x1FD09E7390A74D54"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	This is not tested - it's just an assumption.


	Confirm. It sets an alarm.

</summary>
]]--
native "SET_VEHICLE_ALARM"
	hash "0xCDE5E70C1DDB954C"
	jhash (0x24877D84)
	arguments {
		Vehicle "vehicle",

		BOOL "state",
	}
	returns	"void"

native "START_VEHICLE_ALARM"
	hash "0xB8FF7AB45305C345"
	jhash (0x5B451FF7)
	arguments {
		Vehicle "vehicle",
	}
	returns	"Any"

native "IS_VEHICLE_ALARM_ACTIVATED"
	hash "0x4319E335B71FFF34"
	jhash (0xF2630A4C)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "SET_VEHICLE_INTERIORLIGHT"
	hash "0xBC2042F090AF6AD3"
	jhash (0x9AD1FE1E)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	multiplier = brightness of head lights.
	this value isn't capped afaik.

	~ Lynxaa

	multiplier = 0.0 no lights
	multiplier = 1.0 default game value

	~ Houri
</summary>
]]--
native "SET_VEHICLE_LIGHT_MULTIPLIER"
	hash "0xB385454F8791F57C"
	jhash (0x48039D6A)
	arguments {
		Vehicle "vehicle",

		float "multiplier",
	}
	returns	"void"

--[[!
<summary>
	vb.net
	 Public Sub attachTrailer(vh1 As Vehicle, vh2 As Vehicle, ra As Single)
	        Native.Function.Call(Hash.ATTACH_VEHICLE_TO_TRAILER, vh1, vh2, ra)
	    End Sub
</summary>
]]--
native "ATTACH_VEHICLE_TO_TRAILER"
	hash "0x3C7D42D58F770B54"
	jhash (0x2133977F)
	arguments {
		Any "vehicle",

		Vehicle "trailer",

		float "radius",
	}
	returns	"void"

--[[!
<summary>
	only documented to be continued...
</summary>
]]--
native "0x16B5E274BDE402F8"
	hash "0x16B5E274BDE402F8"
	jhash (0x12AC1A16)
	arguments {
		Vehicle "vehicle",

		Vehicle "trailer",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",

		float "p11",
	}
	returns	"void"

native "0x374706271354CB18"
	hash "0x374706271354CB18"
	jhash (0x40C4763F)
	arguments {
		VehiclePtr "p0",

		PlayerPtr "p1",

		ObjectPtr "p2",
	}
	returns	"void"

--[[!
<summary>
	Public Sub detatchTrailer(vh1 As Vehicle)
	        Native.Function.Call(Hash.DETACH_VEHICLE_FROM_TRAILER, vh1)
	    End Sub
</summary>
]]--
native "DETACH_VEHICLE_FROM_TRAILER"
	hash "0x90532EDF0D2BDD86"
	jhash (0xB5DBF91D)
	arguments {
		Vehicle "vehicle",
	}
	returns	"void"

--[[!
<summary>
	Public Function isVehicleAttachedToTrailer(vh As Vehicle) As Boolean
	        Return Native.Function.Call(Of Boolean)(Hash.IS_VEHICLE_ATTACHED_TO_TRAILER, vh)
	    End Function
</summary>
]]--
native "IS_VEHICLE_ATTACHED_TO_TRAILER"
	hash "0xE7CF3C4F9F489F0C"
	jhash (0xE142BBCC)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "0x2A8F319B392E7B3F"
	hash "0x2A8F319B392E7B3F"
	jhash (0xE74E85CE)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x95CF53B3D687F9FA"
	hash "0x95CF53B3D687F9FA"
	jhash (0x06C47A6F)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	tyreIndex = 0 to 4 on normal vehicles
</summary>
]]--
native "SET_VEHICLE_TYRE_FIXED"
	hash "0x6E13FC662B882D1D"
	jhash (0xA42EFA6B)
	arguments {
		Vehicle "vehicle",

		int "tyreIndex",
	}
	returns	"Any"

--[[!
<summary>
	Sets a vehicle's license plate text.  8 chars maximum.

	Example:
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	Vehicle veh = PED::GET_VEHICLE_PED_IS_USING(playerPed);
	char* plateText = "KING";
	VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(veh, (plateText));
</summary>
]]--
native "SET_VEHICLE_NUMBER_PLATE_TEXT"
	hash "0x95A88F0B409CDA47"
	jhash (0x400F9556)
	arguments {
		Vehicle "vehicle",

		charPtr "plateText",
	}
	returns	"void"

--[[!
<summary>
	Returns the license plate text from a vehicle.  8 chars maximum.
</summary>
]]--
native "GET_VEHICLE_NUMBER_PLATE_TEXT"
	hash "0x7CE1CCB9B293020E"
	jhash (0xE8522D58)
	arguments {
		Vehicle "vehicle",
	}
	returns	"charPtr"

--[[!
<summary>
	Returns ant int. For every vehicle I tested, the int was "6" --unknown

	This is the number of *types* of licence plates, enumerated below in SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX. --JT
</summary>
]]--
native "GET_NUMBER_OF_VEHICLE_NUMBER_PLATES"
	hash "0x4C4D6B2644F458CB"
	jhash (0xD24BC1AE)
	returns	"Any"

--[[!
<summary>
	Plates:
	Blue/White 
	Yellow/black 
	Yellow/Blue 
	Blue/White2 
	Blue/White3 
	Yankton 
</summary>
]]--
native "SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX"
	hash "0x9088EB5A43FFB0A1"
	jhash (0xA1A1890E)
	arguments {
		Vehicle "vehicle",

		int "plateIndex",
	}
	returns	"void"

--[[!
<summary>
	Returns the PlateType of a vehicle
			Blue_on_White_1 = 3,
			Blue_on_White_2 = 0,
			Blue_on_White_3 = 4,
			Yellow_on_Blue = 2,
			Yellow_on_Black = 1,
			North_Yankton = 5,
</summary>
]]--
native "GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX"
	hash "0xF11BC2DD9A3E7195"
	jhash (0x499747B6)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

native "SET_RANDOM_TRAINS"
	hash "0x80D9F74197EA47D9"
	jhash (0xD461CA7F)
	arguments {
		BOOL "unk",
	}
	returns	"Any"

native "CREATE_MISSION_TRAIN"
	hash "0x63C6CCA8E68AE8C8"
	jhash (0xD4C2EAFD)
	arguments {
		int "variation",

		float "x",

		float "y",

		float "z",

		BOOL "direction",
	}
	returns	"Vehicle"

native "SWITCH_TRAIN_TRACK"
	hash "0xFD813BB7DB977F20"
	jhash (0x68BFDD61)
	arguments {
		int "intersectionId",

		BOOL "state",
	}
	returns	"Any"

native "0x21973BBF8D17EDFA"
	hash "0x21973BBF8D17EDFA"
	jhash (0xD5774FB7)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "DELETE_ALL_TRAINS"
	hash "0x736A718577F39C7D"
	jhash (0x83DE7ABF)
	returns	"void"

native "SET_TRAIN_SPEED"
	hash "0xAA0BC91BE0B796E3"
	jhash (0xDFC35E4D)
	arguments {
		Vehicle "train",

		float "speed",
	}
	returns	"Any"

native "SET_TRAIN_CRUISE_SPEED"
	hash "0x16469284DB8C62B5"
	jhash (0xB507F51D)
	arguments {
		Vehicle "train",

		float "speed",
	}
	returns	"Any"

native "SET_RANDOM_BOATS"
	hash "0x84436EC293B1415F"
	jhash (0xB505BD89)
	arguments {
		BOOL "toggle",
	}
	returns	"Any"

native "SET_GARBAGE_TRUCKS"
	hash "0x2AFD795EEAC8D30D"
	jhash (0xD9ABB0FF)
	arguments {
		BOOL "toggle",
	}
	returns	"Any"

native "DOES_VEHICLE_HAVE_STUCK_VEHICLE_CHECK"
	hash "0x57E4C39DE5EE8470"
	jhash (0x5D91D9AC)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "GET_VEHICLE_RECORDING_ID"
	hash "0x21543C612379DB3C"
	jhash (0x328D601D)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"Any"

native "REQUEST_VEHICLE_RECORDING"
	hash "0xAF514CABE74CBF15"
	jhash (0x91AFEFD9)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "HAS_VEHICLE_RECORDING_BEEN_LOADED"
	hash "0x300D614A4C785FC4"
	jhash (0xF52CD7F5)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "REMOVE_VEHICLE_RECORDING"
	hash "0xF1160ACCF98A3FC8"
	jhash (0xD3C05B00)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x92523B76657A517D"
	hash "0x92523B76657A517D"
	jhash (0xF31973BB)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"int"

native "GET_POSITION_OF_VEHICLE_RECORDING_AT_TIME"
	hash "0xD242728AA6F0FBA2"
	jhash (0x7178558D)
	arguments {
		Any "p0",

		float "p1",

		AnyPtr "p2",
	}
	returns	"int"

native "0xF0F2103EFAF8CBA7"
	hash "0xF0F2103EFAF8CBA7"
	jhash (0x4D1C15C2)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"Vector3"

native "GET_ROTATION_OF_VEHICLE_RECORDING_AT_TIME"
	hash "0x2058206FBE79A8AD"
	jhash (0xD96DEC68)
	arguments {
		Any "p0",

		float "p1",

		AnyPtr "p2",
	}
	returns	"int"

native "GET_TOTAL_DURATION_OF_VEHICLE_RECORDING_ID"
	hash "0x102D125411A7B6E6"
	jhash (0x7116785E)
	arguments {
		Any "p0",
	}
	returns	"float"

native "GET_TOTAL_DURATION_OF_VEHICLE_RECORDING"
	hash "0x0E48D1C262390950"
	jhash (0x5B35EEB7)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "GET_POSITION_IN_RECORDING"
	hash "0x2DACD605FC681475"
	jhash (0x7DCD644C)
	arguments {
		Any "p0",
	}
	returns	"float"

native "GET_TIME_POSITION_IN_RECORDING"
	hash "0x5746F3A7AB7FE544"
	jhash (0xF8C3E4A2)
	arguments {
		Any "p0",
	}
	returns	"float"

native "START_PLAYBACK_RECORDED_VEHICLE"
	hash "0x3F878F92B3A7A071"
	jhash (0xCF614CA8)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		BOOL "p3",
	}
	returns	"void"

native "START_PLAYBACK_RECORDED_VEHICLE_WITH_FLAGS"
	hash "0x7D80FD645D4DA346"
	jhash (0x4E721AD2)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"void"

native "0x1F2E4E06DEA8992B"
	hash "0x1F2E4E06DEA8992B"
	jhash (0x01B91CD0)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "STOP_PLAYBACK_RECORDED_VEHICLE"
	hash "0x54833611C17ABDEA"
	jhash (0xAE99C57C)
	arguments {
		Any "p0",
	}
	returns	"void"

native "PAUSE_PLAYBACK_RECORDED_VEHICLE"
	hash "0x632A689BF42301B1"
	jhash (0xCCF54912)
	arguments {
		Any "p0",
	}
	returns	"void"

native "UNPAUSE_PLAYBACK_RECORDED_VEHICLE"
	hash "0x8879EE09268305D5"
	jhash (0x59060F75)
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_PLAYBACK_GOING_ON_FOR_VEHICLE"
	hash "0x1C8A4C2C19E68EEC"
	jhash (0x61F7650D)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "IS_PLAYBACK_USING_AI_GOING_ON_FOR_VEHICLE"
	hash "0xAEA8FD591FAD4106"
	jhash (0x63022C58)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "GET_CURRENT_PLAYBACK_FOR_VEHICLE"
	hash "0x42BC05C27A946054"
	jhash (0xA3F44390)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "SKIP_TO_END_AND_STOP_PLAYBACK_RECORDED_VEHICLE"
	hash "0xAB8E2EDA0C0A5883"
	jhash (0x8DEA18C8)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_PLAYBACK_SPEED"
	hash "0x6683AB880E427778"
	jhash (0x684E26E4)
	arguments {
		Any "p0",

		float "speed",
	}
	returns	"Any"

native "START_PLAYBACK_RECORDED_VEHICLE_USING_AI"
	hash "0x29DE5FA52D00428C"
	jhash (0x8DE8E24E)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		float "p3",

		Any "p4",
	}
	returns	"void"

native "SKIP_TIME_IN_PLAYBACK_RECORDED_VEHICLE"
	hash "0x9438F7AD68771A20"
	jhash (0xCF3EFA4B)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "SET_PLAYBACK_TO_USE_AI"
	hash "0xA549C3B37EA28131"
	jhash (0xB536CCD7)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "SET_PLAYBACK_TO_USE_AI_TRY_TO_REVERT_BACK_LATER"
	hash "0x6E63860BBB190730"
	jhash (0x0C8ABAA4)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		BOOL "p3",
	}
	returns	"void"

native "0x5845066D8A1EA7F7"
	hash "0x5845066D8A1EA7F7"
	jhash (0x943A58EB)
	arguments {
		Vehicle "vehicle",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",
	}
	returns	"void"

native "0x796A877E459B99EA"
	hash "0x796A877E459B99EA"
	jhash (0x5C9F477C)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "0xFAF2A78061FD9EF4"
	hash "0xFAF2A78061FD9EF4"
	jhash (0xCD83C393)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "0x063AE2B2CC273588"
	hash "0x063AE2B2CC273588"
	jhash (0x2EF8435C)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "EXPLODE_VEHICLE_IN_CUTSCENE"
	hash "0x786A4EB67B01BF0B"
	jhash (0xA85207B5)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "ADD_VEHICLE_STUCK_CHECK_WITH_WARP"
	hash "0x2FA9923062DD396C"
	jhash (0xC8B789AD)
	arguments {
		Any "p0",

		float "p1",

		Any "p2",

		BOOL "p3",

		BOOL "p4",

		BOOL "p5",

		Any "p6",
	}
	returns	"void"

--[[!
<summary>
	seems to make the vehicle stop spawning naturally in traffic. Here's an essential example:

	VEHICLE::SET_VEHICLE_MODEL_IS_SUPPRESSED(GAMEPLAY::GET_HASH_KEY("taco"), true);

	god I hate taco vans
</summary>
]]--
native "SET_VEHICLE_MODEL_IS_SUPPRESSED"
	hash "0x0FC2D89AC25A5814"
	jhash (0x42A08C9B)
	arguments {
		Hash "model",

		BOOL "suppressed",
	}
	returns	"void"

--[[!
<summary>
	Gets a random vehicle in a sphere at the specified position, of the specified radius.

	x: The X-component of the position of the sphere.
	y: The Y-component of the position of the sphere.
	z: The Z-component of the position of the sphere.
	radius: The radius of the sphere. Max is 9999.9004.
	modelHash: The vehicle model to limit the selection to. Pass 0 for any model.
	flags: The bitwise flags that modifies the behaviour of this function.
</summary>
]]--
native "GET_RANDOM_VEHICLE_IN_SPHERE"
	hash "0x386F6CE5BAF6091C"
	jhash (0x57216D03)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		Hash "modelHash",

		int "flags",
	}
	returns	"Vehicle"

native "GET_RANDOM_VEHICLE_FRONT_BUMPER_IN_SPHERE"
	hash "0xC5574E0AEB86BA68"
	jhash (0xDCADEB66)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		int "p4",

		int "p5",

		int "p6",
	}
	returns	"Vehicle"

native "GET_RANDOM_VEHICLE_BACK_BUMPER_IN_SPHERE"
	hash "0xB50807EABE20A8DC"
	jhash (0xD6343F6B)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		int "p4",

		int "p5",

		int "p6",
	}
	returns	"Vehicle"

--[[!
<summary>
	Example usage
	VEHICLE::GET_CLOSEST_VEHICLE(x, y, z, radius, hash, unknown leave at 70) 

	x, y, z: Position to get closest vehicle to.
	radius: Max radius to get a vehicle.
	modelHash: Limit to vehicles with this model. 0 for any.
	flags: The bitwise flags altering the function's behaviour.

	Does not return police cars.

	Does not return helicopters.

	It seems to return police cars for me, does not seem to return helicopters, planes or boats for some reason
</summary>
]]--
native "GET_CLOSEST_VEHICLE"
	hash "0xF73EB622C4F1689B"
	jhash (0xD7E26B2C)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		Hash "modelHash",

		int "flags",
	}
	returns	"Vehicle"

native "GET_TRAIN_CARRIAGE"
	hash "0x08AAFD0814722BC3"
	jhash (0x2544E7A6)
	arguments {
		Vehicle "train",

		int "carriage",
	}
	returns	"Entity"

native "DELETE_MISSION_TRAIN"
	hash "0x5B76B14AE875C795"
	jhash (0x86C9497D)
	arguments {
		VehiclePtr "train",
	}
	returns	"void"

native "SET_MISSION_TRAIN_AS_NO_LONGER_NEEDED"
	hash "0xBBE7648349B49BE8"
	jhash (0x19808560)
	arguments {
		VehiclePtr "train",

		BOOL "unk",
	}
	returns	"void"

native "SET_MISSION_TRAIN_COORDS"
	hash "0x591CA673AA6AB736"
	jhash (0xD6D70803)
	arguments {
		Vehicle "train",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

--[[!
<summary>
	It's pretty safe to say it's just checking what type of vehicle it is. But as I have mentioned many times before, it's just an assumption.

	Edit: Now that I think about it, it could also be a model hash parameter.

</summary>
]]--
native "IS_THIS_MODEL_A_BOAT"
	hash "0x45A9187928F4B9E3"
	jhash (0x10F6085C)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

native "_IS_THIS_MODEL_A_SUBMERSIBLE"
	hash "0x9537097412CF75FE"
	arguments {
		Hash "model",
	}
	returns	"BOOL"

--[[!
<summary>
	It's pretty safe to say it's just checking what type of vehicle it is. But as I have mentioned many times before, it's just an assumption.

	Edit: Now that I think about it, it could also be a model hash parameter.

</summary>
]]--
native "IS_THIS_MODEL_A_PLANE"
	hash "0xA0948AB42D7BA0DE"
	jhash (0x3B3907BB)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

--[[!
<summary>
	It's pretty safe to say it's just checking what type of vehicle it is. But as I have mentioned many times before, it's just an assumption.

	Edit: Now that I think about it, it could also be a model hash parameter.

</summary>
]]--
native "IS_THIS_MODEL_A_HELI"
	hash "0xDCE4334788AF94EA"
	jhash (0x8AF7F568)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

--[[!
<summary>
	It's pretty safe to say it's just checking what type of vehicle it is. But as I have mentioned many times before, it's just an assumption.

	Edit: Now that I think about it, it could also be a model hash parameter.

</summary>
]]--
native "IS_THIS_MODEL_A_CAR"
	hash "0x7F6DB52EEFC96DF8"
	jhash (0x60E4C22F)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

--[[!
<summary>
	It's pretty safe to say it's just checking what type of vehicle it is. But as I have mentioned many times before, it's just an assumption.

	Edit: Now that I think about it, it could also be a model hash parameter.

</summary>
]]--
native "IS_THIS_MODEL_A_TRAIN"
	hash "0xAB935175B22E822B"
	jhash (0xF87DCFFD)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

--[[!
<summary>
	It's pretty safe to say it's just checking what type of vehicle it is. But as I have mentioned many times before, it's just an assumption.

	Edit: Now that I think about it, it could also be a model hash parameter.

</summary>
]]--
native "IS_THIS_MODEL_A_BIKE"
	hash "0xB50C0B0CEDC6CE84"
	jhash (0x7E702CDD)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

--[[!
<summary>
	It's pretty safe to say it's just checking what type of vehicle it is. But as I have mentioned many times before, it's just an assumption.

	Edit: Now that I think about it, it could also be a model hash parameter.

</summary>
]]--
native "IS_THIS_MODEL_A_BICYCLE"
	hash "0xBF94DD42F63BDED2"
	jhash (0x328E6FF5)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

--[[!
<summary>
	It's pretty safe to say it's just checking what type of vehicle it is. But as I have mentioned many times before, it's just an assumption.

	Edit: Now that I think about it, it could also be a model hash parameter.

</summary>
]]--
native "IS_THIS_MODEL_A_QUADBIKE"
	hash "0x39DAC362EE65FA28"
	jhash (0xC1625277)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

--[[!
<summary>
	Equivalent of SET_HELI_BLADES_SPEED(vehicleHandle, 1.0f);
</summary>
]]--
native "SET_HELI_BLADES_FULL_SPEED"
	hash "0xA178472EBB8AE60D"
	jhash (0x033A9408)
	arguments {
		Vehicle "vehicle",
	}
	returns	"void"

--[[!
<summary>
	Sets the speed of the helicopter blades in percentage of the full speed.

	vehicleHandle: The helicopter.
	speed: The speed in percentage, 0.0f being 0% and 1.0f being 100%.
</summary>
]]--
native "SET_HELI_BLADES_SPEED"
	hash "0xFD280B4D7F3ABC4D"
	jhash (0x5C7D4EA9)
	arguments {
		Vehicle "vehicle",

		float "speed",
	}
	returns	"void"

native "0x99CAD8E7AFDB60FA"
	hash "0x99CAD8E7AFDB60FA"
	jhash (0x1128A45B)
	arguments {
		Any "p0",

		float "p1",

		float "p2",
	}
	returns	"void"

--[[!
<summary>
	This has not yet been tested - it's just an assumption of what the types could be.
</summary>
]]--
native "SET_VEHICLE_CAN_BE_TARGETTED"
	hash "0x3750146A28097A82"
	jhash (0x64B70B1D)
	arguments {
		Vehicle "vehicle",

		BOOL "state",
	}
	returns	"void"

native "0xDBC631F109350B8C"
	hash "0xDBC631F109350B8C"
	jhash (0x486C1280)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	This has not yet been tested - it's just an assumption of what the types could be.
</summary>
]]--
native "SET_VEHICLE_CAN_BE_VISIBLY_DAMAGED"
	hash "0x4C7028F78FFD3681"
	jhash (0xC5D94017)
	arguments {
		Vehicle "vehicle",

		BOOL "state",
	}
	returns	"void"

native "0x1AA8A837D2169D94"
	hash "0x1AA8A837D2169D94"
	jhash (0x009AB49E)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x2311DD7159F00582"
	hash "0x2311DD7159F00582"
	jhash (0x758C5E2E)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Dirt level 0..15
</summary>
]]--
native "GET_VEHICLE_DIRT_LEVEL"
	hash "0x8F17BC8BA08DA62B"
	jhash (0xFD15C065)
	arguments {
		Vehicle "vehicle",
	}
	returns	"float"

--[[!
<summary>
	dirtLevel = 0 (clean) to 15 (dirty)
</summary>
]]--
native "SET_VEHICLE_DIRT_LEVEL"
	hash "0x79D3B596FE44EE8B"
	jhash (0x2B39128B)
	arguments {
		Vehicle "vehicle",

		float "dirtLevel",
	}
	returns	"void"

--[[!
<summary>
	Appears to return true if the vehicle has any damage, including cosmetically.
</summary>
]]--
native "_IS_VEHICLE_DAMAGED"
	hash "0xBCDC5017D3CE1E9E"
	jhash (0xDAC523BC)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	doorIndex:
	0 = Front Left Door
	1 = Front Right Door
	2 = Back Left Door
	3 = Back Right Door
	4 = Hood
	5 = Trunk
	6 = Trunk2
</summary>
]]--
native "IS_VEHICLE_DOOR_FULLY_OPEN"
	hash "0x3E933CFF7B111C22"
	jhash (0xC2385B6F)
	arguments {
		Vehicle "vehicle",

		int "doorIndex",
	}
	returns	"BOOL"

--[[!
<summary>
	Starts or stops the engine on the specified vehicle.

	vehicle: The vehicle to start or stop the engine on.
	value: true to turn the vehicle on; false to turn it off.
	instantly: if true, the vehicle will be set to the state immediately; otherwise, the current driver will physically turn on or off the engine.
</summary>
]]--
native "SET_VEHICLE_ENGINE_ON"
	hash "0x2497C4717C8B881E"
	jhash (0x7FBC86F1)
	arguments {
		Vehicle "vehicle",

		BOOL "value",

		BOOL "instantly",
	}
	returns	"void"

--[[!
<summary>
	This has not yet been tested - it's just an assumption of what the types could be.
</summary>
]]--
native "SET_VEHICLE_UNDRIVEABLE"
	hash "0x8ABA6AF54B942B95"
	jhash (0x48D02A4E)
	arguments {
		Vehicle "vehicle",

		BOOL "state",
	}
	returns	"void"

native "SET_VEHICLE_PROVIDES_COVER"
	hash "0x5AFEEDD9BB2899D7"
	jhash (0xEFC01CA9)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	doorIndex:
	0 = Front Left Door (driver door)
	1 = Front Right Door
	2 = Back Left Door
	3 = Back Right Door
	4 = Hood
	5 = Trunk
	6 = Trunk2

	p3:
	It seems it is an angle
</summary>
]]--
native "SET_VEHICLE_DOOR_CONTROL"
	hash "0xF2BFA0430F0A0FCB"
	jhash (0x572DD360)
	arguments {
		Vehicle "vehicle",

		int "doorIndex",

		Any "p2",

		float "p3",
	}
	returns	"void"

native "SET_VEHICLE_DOOR_LATCHED"
	hash "0xA5A9653A8D2CAF48"
	jhash (0x4EB7BBFC)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"void"

native "GET_VEHICLE_DOOR_ANGLE_RATIO"
	hash "0xFE3F9C29F7B32BD5"
	jhash (0x0E399C26)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"float"

native "0x218297BF0CFD853B"
	hash "0x218297BF0CFD853B"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

--[[!
<summary>
	doorIndex:
	0 = Front Left Door
	1 = Front Right Door
	2 = Back Left Door
	3 = Back Right Door
	4 = Hood
	5 = Trunk
	6 = Trunk2
</summary>
]]--
native "SET_VEHICLE_DOOR_SHUT"
	hash "0x93D9BD300D7789E5"
	jhash (0x142606BD)
	arguments {
		Vehicle "vehicle",

		int "doorIndex",

		BOOL "closeInstantly",
	}
	returns	"void"

--[[!
<summary>
	doorIndex:
	0 = Front Right Door
	1 = Front Left Door
	2 = Back Right Door
	3 = Back Left Door
	4 = Hood
	5 = Trunk
	6 = Trunk2
</summary>
]]--
native "SET_VEHICLE_DOOR_BROKEN"
	hash "0xD4D4F6A4AB575A33"
	jhash (0x8147FEA7)
	arguments {
		Vehicle "vehicle",

		int "doorIndex",

		BOOL "toggle",
	}
	returns	"void"

native "SET_VEHICLE_CAN_BREAK"
	hash "0x59BF8C3D52C92F66"
	jhash (0x90A810D1)
	arguments {
		Vehicle "vehicle",

		BOOL "Toggle",
	}
	returns	"Any"

native "DOES_VEHICLE_HAVE_ROOF"
	hash "0x8AC862B0B32C5B80"
	jhash (0xDB817403)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "IS_BIG_VEHICLE"
	hash "0x9F243D3919F442FE"
	jhash (0x9CDBA8DE)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "GET_NUMBER_OF_VEHICLE_COLOURS"
	hash "0x3B963160CD65D41E"
	jhash (0xF2442EE2)
	arguments {
		Vehicle "vehicle",
	}
	returns	"Any"

--[[!
<summary>
	numCombos can be anywhere from 0 to 9 according to the scripts.
</summary>
]]--
native "SET_VEHICLE_COLOUR_COMBINATION"
	hash "0x33E8CD3322E2FE31"
	jhash (0xA557AEAD)
	arguments {
		Vehicle "vehicle",

		int "numCombos",
	}
	returns	"void"

native "GET_VEHICLE_COLOUR_COMBINATION"
	hash "0x6A842D197F845D56"
	jhash (0x77AC1B4C)
	arguments {
		Vehicle "vehicle",
	}
	returns	"Any"

native "SET_VEHICLE_IS_CONSIDERED_BY_PLAYER"
	hash "0x31B927BBC44156CD"
	jhash (0x14413319)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "0xBE5C1255A1830FF5"
	hash "0xBE5C1255A1830FF5"
	jhash (0xA6D8D7A5)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x9BECD4B9FEF3F8A6"
	hash "0x9BECD4B9FEF3F8A6"
	jhash (0xACAB8FF3)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x88BC673CA9E0AE99"
	hash "0x88BC673CA9E0AE99"
	jhash (0xF0E5C41D)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xE851E480B814D4BA"
	hash "0xE851E480B814D4BA"
	jhash (0x2F98B4B7)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	p0 always true (except in one case)
	p1 returns a random vehicle hash loaded in memory
	p2 unsure, maybe returns a different model
</summary>
]]--
native "GET_RANDOM_VEHICLE_MODEL_IN_MEMORY"
	hash "0x055BF0AC0C34F4FD"
	jhash (0xE2C45631)
	arguments {
		BOOL "p0",

		HashPtr "modelHash",

		intPtr "p2",
	}
	returns	"void"

native "GET_VEHICLE_DOOR_LOCK_STATUS"
	hash "0x25BC98A59C2EA962"
	jhash (0x0D72CEF2)
	arguments {
		int "doorIndex",
	}
	returns	"Any"

--[[!
<summary>
	doorID starts at 0, not seeming to skip any numbers. Four door vehicles intuitively range from 0 to 3.
</summary>
]]--
native "IS_VEHICLE_DOOR_DAMAGED"
	hash "0xB8E181E559464527"
	jhash (0x4999E3C3)
	arguments {
		Vehicle "veh",

		int "doorID",
	}
	returns	"BOOL"

--[[!
<summary>
	Keeps Vehicle Doors/Hood/Trunk from breaking off
</summary>
]]--
native "_SET_VEHICLE_DOOR_BREAKABLE"
	hash "0x2FA133A4A9D37ED8"
	jhash (0x065B92B3)
	arguments {
		Vehicle "vehicle",

		int "doorIndex",

		BOOL "isBreakable",
	}
	returns	"void"

native "0x27B926779DEB502D"
	hash "0x27B926779DEB502D"
	jhash (0xB3A2CC4F)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	Edited by: Enumerator


</summary>
]]--
native "IS_VEHICLE_BUMPER_BROKEN_OFF"
	hash "0x468056A6BB6F3846"
	jhash (0xAF25C027)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	Usage:

	public bool isCopInRange(Vector3 Location, float Range)
	        {
	            return Function.Call&lt;bool&gt;(Hash.IS_COP_PED_IN_AREA_3D, Location.X - Range, Location.Y - Range, Location.Z - Range, Location.X + Range, Location.Y + Range, Location.Z + Range);
	        }
</summary>
]]--
native "IS_COP_VEHICLE_IN_AREA_3D"
	hash "0x7EEF65D5F153E26A"
	jhash (0xFB16C6D1)
	arguments {
		float "x1",

		float "x2",

		float "y1",

		float "y2",

		float "z1",

		float "z2",
	}
	returns	"BOOL"

--[[!
<summary>
	 Public Function isVehicleOnAllWheels(vh As Vehicle) As Boolean
	        Return Native.Function.Call(Of Boolean)(Hash.IS_VEHICLE_ON_ALL_WHEELS, vh)
	    End Function
</summary>
]]--
native "IS_VEHICLE_ON_ALL_WHEELS"
	hash "0xB104CD1BABF302E2"
	jhash (0x10089F8E)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "GET_VEHICLE_LAYOUT_HASH"
	hash "0x28D37D4F71AC5C58"
	jhash (0xE0B35187)
	arguments {
		Vehicle "vehicle",
	}
	returns	"Hash"

native "0xA01BC64DD4BFBBAC"
	hash "0xA01BC64DD4BFBBAC"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

--[[!
<summary>
	makes the train all jumbled up and derailed as it moves on the tracks (though that wont stop it from its normal operations)
</summary>
]]--
native "SET_RENDER_TRAIN_AS_DERAILED"
	hash "0x317B11A312DF5534"
	jhash (0x899D9092)
	arguments {
		Vehicle "train",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	They use the same color indexs as SET_VEHICLE_COLOURS.
</summary>
]]--
native "SET_VEHICLE_EXTRA_COLOURS"
	hash "0x2036F561ADD12E33"
	jhash (0x515DB2A0)
	arguments {
		Vehicle "vehicle",

		int "pearlescentColor",

		int "wheelColor",
	}
	returns	"void"

native "GET_VEHICLE_EXTRA_COLOURS"
	hash "0x3BC4245933A166F7"
	jhash (0x80E4659B)
	arguments {
		Vehicle "vehicle",

		intPtr "pearlescentColor",

		intPtr "wheelColor",
	}
	returns	"void"

native "STOP_ALL_GARAGE_ACTIVITY"
	hash "0x0F87E938BDF29D66"
	jhash (0x17A0BCE5)
	returns	"void"

native "SET_VEHICLE_FIXED"
	hash "0x115722B1B9C14C1C"
	jhash (0x17469AA1)
	arguments {
		Vehicle "vehicle",
	}
	returns	"void"

native "SET_VEHICLE_DEFORMATION_FIXED"
	hash "0x953DA1E1B12C0491"
	jhash (0xDD2920C8)
	arguments {
		Vehicle "vehicle",
	}
	returns	"void"

native "0x206BC5DC9D1AC70A"
	hash "0x206BC5DC9D1AC70A"
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "0x51BB2D88D31A914B"
	hash "0x51BB2D88D31A914B"
	jhash (0x88F0F7E7)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "0x192547247864DFDD"
	hash "0x192547247864DFDD"
	jhash (0x90D6EE57)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "0x465BF26AB9684352"
	hash "0x465BF26AB9684352"
	jhash (0xC40192B5)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "SET_DISABLE_VEHICLE_PETROL_TANK_DAMAGE"
	hash "0x37C8252A7C92D017"
	jhash (0xAD3E05F2)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "0x91A0BD635321F145"
	hash "0x91A0BD635321F145"
	jhash (0x1784BA1A)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "0xC50CE861B55EAB8B"
	hash "0xC50CE861B55EAB8B"
	jhash (0x40C323AE)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "0x6EBFB22D646FFC18"
	hash "0x6EBFB22D646FFC18"
	jhash (0x847F1304)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "0x25367DE49D64CF16"
	hash "0x25367DE49D64CF16"
	jhash (0xCBD98BA1)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "REMOVE_VEHICLES_FROM_GENERATORS_IN_AREA"
	hash "0x46A1E1A299EC4BBA"
	jhash (0x42CC15E0)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",
	}
	returns	"void"

--[[!
<summary>
	Locks the vehicle's steering to the desired angle, explained below.

	Requires to be called onTick. Steering is unlocked the moment the function stops being called on the vehicle.

	Steer bias:
	-1.0 = full right
	0.0 = centered steering
	1.0 = full left
</summary>
]]--
native "SET_VEHICLE_STEER_BIAS"
	hash "0x42A8EC77D5150CBE"
	jhash (0x7357C1EB)
	arguments {
		Vehicle "vehicle",

		float "value",
	}
	returns	"void"

native "IS_VEHICLE_EXTRA_TURNED_ON"
	hash "0xD2E6822DBFD6C8BD"
	jhash (0x042098B5)
	arguments {
		Vehicle "vehicle",

		int "extraId",
	}
	returns	"BOOL"

--[[!
<summary>
	how to toggle:
	0 for ON
	-1 for OFF

	Note: only some vehicle have extras
	extra ids are from 1 - 9 depending on the vehicle
</summary>
]]--
native "SET_VEHICLE_EXTRA"
	hash "0x7EE3A3C5E4A40CC9"
	jhash (0x642D065C)
	arguments {
		Vehicle "vehicle",

		int "extraId",

		int "toggle",
	}
	returns	"void"

native "DOES_EXTRA_EXIST"
	hash "0x1262D55792428154"
	jhash (0x409411CC)
	arguments {
		Vehicle "vehicle",

		int "extraId",
	}
	returns	"BOOL"

native "SET_CONVERTIBLE_ROOF"
	hash "0xF39C4F538B5124C2"
	jhash (0xC87B6A51)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "LOWER_CONVERTIBLE_ROOF"
	hash "0xDED51F703D0FA83D"
	jhash (0xC5F72EAE)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "RAISE_CONVERTIBLE_ROOF"
	hash "0x8F5FB35D7E88FC70"
	jhash (0xA4E4CBA3)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "GET_CONVERTIBLE_ROOF_STATE"
	hash "0xF8C397922FC03F41"
	jhash (0x1B09714D)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

native "IS_VEHICLE_A_CONVERTIBLE"
	hash "0x52F357A30698BCCE"
	jhash (0x6EF54490)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"BOOL"

native "IS_VEHICLE_STOPPED_AT_TRAFFIC_LIGHTS"
	hash "0x2959F696AE390A99"
	jhash (0x69200FA4)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	Apply damage to vehicle at a location. Location is relative to vehicle model (not world).

	Radius of effect damage applied in a sphere at impact location
</summary>
]]--
native "SET_VEHICLE_DAMAGE"
	hash "0xA1DD317EA8FD4F29"
	jhash (0x21B458B2)
	arguments {
		Vehicle "vehicle",

		float "xOffset",

		float "yOffset",

		float "zOffset",

		float "damage",

		float "radius",

		BOOL "p6",
	}
	returns	"void"

--[[!
<summary>
	1000 is max health, -4000 is minimum health
	Engine begins smoking and losing functionality around 300
	Engine catches fire and beings rapidly losing health if health drops below 0

	Vehicle does not explode when the engine reaches -4000 health
</summary>
]]--
native "GET_VEHICLE_ENGINE_HEALTH"
	hash "0xC45D23BAF168AAB8"
	jhash (0x8880038A)
	arguments {
		Vehicle "vehicle",
	}
	returns	"float"

native "SET_VEHICLE_ENGINE_HEALTH"
	hash "0x45F6D8EEF34ABEF1"
	jhash (0x1B760FB5)
	arguments {
		Vehicle "vehicle",

		float "health",
	}
	returns	"void"

--[[!
<summary>
	1000 is max health
	Begins leaking gas at around 650 health
</summary>
]]--
native "GET_VEHICLE_PETROL_TANK_HEALTH"
	hash "0x7D5DABE888D2D074"
	jhash (0xE41595CE)
	arguments {
		Vehicle "vehicle",
	}
	returns	"float"

native "SET_VEHICLE_PETROL_TANK_HEALTH"
	hash "0x70DB57649FA8D0D8"
	jhash (0x660A3692)
	arguments {
		Vehicle "vehicle",

		float "health",
	}
	returns	"Any"

--[[!
<summary>
	p1 can be anywhere from 0 to 3 in the scripts. p2 is generally somewhere in the 1000 to 10000 range.
</summary>
]]--
native "IS_VEHICLE_STUCK_TIMER_UP"
	hash "0x679BE1DAF71DA874"
	jhash (0x2FCF58C1)
	arguments {
		Vehicle "vehicle",

		int "p1",

		int "p2",
	}
	returns	"BOOL"

--[[!
<summary>
	nullAttributes is always 0 in the scripts. In IDA it calls a second function that's entirely dependent on the 2nd parameter being true, and all it does is set a bunch of offsets to 0.


	MulleDK19: That's not true at all. The second parameter is an int, not a bool. The inner function has a switch on the second parameter. It's the stuck timer index.

	Here's some pseudo code I wrote for the inner function:
	void __fastcall NATIVE_RESET_VEHICLE_STUCK_TIMER_INNER(CUnknown* unknownClassInVehicle, int timerIndex)
	{
		switch (timerIndex)
		{
		case 0:
			unknownClassInVehicle-&gt;FirstStuckTimer = (WORD)0u;
		case 1:
			unknownClassInVehicle-&gt;SecondStuckTimer = (WORD)0u;
		case 2:
			unknownClassInVehicle-&gt;ThirdStuckTimer = (WORD)0u;
		case 3:
			unknownClassInVehicle-&gt;FourthStuckTimer = (WORD)0u;
		case 4:
			unknownClassInVehicle-&gt;FirstStuckTimer = (WORD)0u;
			unknownClassInVehicle-&gt;SecondStuckTimer = (WORD)0u;
			unknownClassInVehicle-&gt;ThirdStuckTimer = (WORD)0u;
			unknownClassInVehicle-&gt;FourthStuckTimer = (WORD)0u;
			break;
		};
	}
</summary>
]]--
native "RESET_VEHICLE_STUCK_TIMER"
	hash "0xD7591B0065AFAA7A"
	jhash (0xEF2A6016)
	arguments {
		Vehicle "vehicle",

		BOOL "nullAttributes",
	}
	returns	"void"

--[[!
<summary>
	p1 is always 0 in the scripts.
</summary>
]]--
native "IS_VEHICLE_DRIVEABLE"
	hash "0x4C241E39B23DF959"
	jhash (0x41A7267A)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"BOOL"

native "SET_VEHICLE_HAS_BEEN_OWNED_BY_PLAYER"
	hash "0x2B5F9D2AF1F1722D"
	jhash (0xB4D3DBFB)
	arguments {
		Vehicle "vehicle",

		BOOL "owned",
	}
	returns	"void"

native "SET_VEHICLE_NEEDS_TO_BE_HOTWIRED"
	hash "0xFBA550EA44404EE6"
	jhash (0xD8260751)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "0x9F3F689B814F2599"
	hash "0x9F3F689B814F2599"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x4E74E62E0A97E901"
	hash "0x4E74E62E0A97E901"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Sounds the horn for the specified vehicle.

	vehicle: The vehicle to activate the horn for.
	mode: The hash of "NORMAL" or "HELDDOWN". Can be 0.
	duration: The duration to sound the horn, in milliseconds.
	p3: unknown.

	Note: If a player is in the vehicle, it will only sound briefly.
</summary>
]]--
native "START_VEHICLE_HORN"
	hash "0x9C8C6504B5B63D2C"
	jhash (0x0DF5ADB3)
	arguments {
		Vehicle "vehicle",

		int "duration",

		Hash "mode",

		BOOL "p3",
	}
	returns	"void"

--[[!
<summary>
	If set to TRUE, it seems to suppress door noises and doesn't allow the horn to be continuous.
</summary>
]]--
native "0x9D44FCCE98450843"
	hash "0x9D44FCCE98450843"
	jhash (0x968E5770)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	if true, axles won't bend.
</summary>
]]--
native "SET_VEHICLE_HAS_STRONG_AXLES"
	hash "0x92F0CF722BC4202F"
	jhash (0x0D1CBC65)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Returns model name of vehicle in all caps. Needs to be displayed through localizing text natives to get proper display name.
	-----------------------------------------------------------------------------------------------------------------------------------------
	Example: 

	char *name = GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(0x1F52A43F);
	char text[32];
	sprintf_s(text, "%s", name);
	set_status_text(text);

	Result: "MOONBEAM"
</summary>
]]--
native "GET_DISPLAY_NAME_FROM_VEHICLE_MODEL"
	hash "0xB215AAC32D25D019"
	jhash (0xEC86DF39)
	arguments {
		Hash "modelHash",
	}
	returns	"charPtr"

--[[!
<summary>
	The only example I can find of this function in the scripts, is this:

	struct _s = VEHICLE::GET_VEHICLE_DEFORMATION_AT_POS(rPtr((A_0) + 4), 1.21f, 6.15f, 0.3f);
</summary>
]]--
native "GET_VEHICLE_DEFORMATION_AT_POS"
	hash "0x4EC6CFBC7B2E9536"
	jhash (0xABF02075)
	arguments {
		Vehicle "vehicle",

		float "position1",

		float "position2",

		float "position3",
	}
	returns	"Vector3"

native "SET_VEHICLE_LIVERY"
	hash "0x60BF608F1B8CD1B6"
	jhash (0x7AD87059)
	arguments {
		Vehicle "vehicle",

		int "livery",
	}
	returns	"void"

--[[!
<summary>
	-1 = no livery
</summary>
]]--
native "GET_VEHICLE_LIVERY"
	hash "0x2BB9230590DA5E8A"
	jhash (0xEC82A51D)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

--[[!
<summary>
	Returns -1 if the vehicle has no livery
</summary>
]]--
native "GET_VEHICLE_LIVERY_COUNT"
	hash "0x87B63E25A529D526"
	jhash (0xFB0CA947)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

native "IS_VEHICLE_WINDOW_INTACT"
	hash "0x46E571A0E20D01F1"
	jhash (0xAC4EF23D)
	arguments {
		Vehicle "vehicle",

		int "windowIndex",
	}
	returns	"BOOL"

--[[!
<summary>
	Appears to return false if any window is broken.
</summary>
]]--
native "_ARE_ALL_VEHICLE_WINDOWS_INTACT"
	hash "0x11D862A3E977A9EF"
	jhash (0xBB619744)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns false if every seat is occupied.
</summary>
]]--
native "_IS_ANY_VEHICLE_SEAT_EMPTY"
	hash "0x2D34FC3BC4ADB780"
	jhash (0x648E685A)
	arguments {
		Vehicle "veh",
	}
	returns	"BOOL"

native "RESET_VEHICLE_WHEELS"
	hash "0x21D2E5662C1F6FED"
	jhash (0xD5FFE779)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_HELI_PART_BROKEN"
	hash "0xBC74B4BE25EB6C8A"
	jhash (0xF4E4C439)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"BOOL"

--[[!
<summary>
	Max 1000.
	At 0 the main rotor will stall.
</summary>
]]--
native "_GET_HELI_MAIN_ROTOR_HEALTH"
	hash "0xE4CB7541F413D2C5"
	jhash (0xF01E2AAB)
	arguments {
		Vehicle "vehicle",
	}
	returns	"float"

--[[!
<summary>
	Max 1000.
	At 0 the tail rotor will stall.

</summary>
]]--
native "_GET_HELI_TAIL_ROTOR_HEALTH"
	hash "0xAE8CE82A4219AC8C"
	jhash (0xA41BC13D)
	arguments {
		Vehicle "vehicle",
	}
	returns	"float"

--[[!
<summary>
	Max 1000.
	At -100 both helicopter rotors will stall.
</summary>
]]--
native "_GET_HELI_ENGINE_HEALTH"
	hash "0xAC51915D27E4A5F7"
	jhash (0x8A68388F)
	arguments {
		Vehicle "vehicle",
	}
	returns	"float"

native "WAS_COUNTER_ACTIVATED"
	hash "0x3EC8BF18AA453FE9"
	jhash (0x2916D69B)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	NOTE: Debugging functions are not present in the retail version of the game.
</summary>
]]--
native "SET_VEHICLE_NAME_DEBUG"
	hash "0xBFDF984E2C22B94F"
	jhash (0xA712FF5C)
	arguments {
		Vehicle "vehicle",

		charPtr "name",
	}
	returns	"void"

--[[!
<summary>
	Sets a vehicle to be strongly resistant to explosions. p0 is the vehicle; set p1 to false to toggle the effect on/off.
</summary>
]]--
native "SET_VEHICLE_EXPLODES_ON_HIGH_EXPLOSION_DAMAGE"
	hash "0x71B0892EC081D60A"
	jhash (0x38CC692B)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "0x3441CAD2F2231923"
	hash "0x3441CAD2F2231923"
	jhash (0xC306A9A3)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x2B6747FAA9DB9D6B"
	hash "0x2B6747FAA9DB9D6B"
	jhash (0x95A9ACCB)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"Any"

--[[!
<summary>
	Works for vehicles with a retractable landing gear

	landing gear states:

	0: Deployed
	1: Closing
	2: Opening
	3: Retracted
</summary>
]]--
native "_SET_VEHICLE_LANDING_GEAR"
	hash "0xCFC8BE9A5E1FE575"
	jhash (0x24F873FB)
	arguments {
		Vehicle "vehicle",

		int "state",
	}
	returns	"void"

--[[!
<summary>
	landing gear states:

	0: Deployed
	1: Closing
	2: Opening
	3: Retracted
</summary>
]]--
native "_GET_VEHICLE_LANDING_GEAR"
	hash "0x9B0F3DCA3DB0F4CD"
	jhash (0xA6F02670)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

native "IS_ANY_VEHICLE_NEAR_POINT"
	hash "0x61E1DD6125A3EEE6"
	jhash (0x2867A834)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",
	}
	returns	"BOOL"

native "REQUEST_VEHICLE_HIGH_DETAIL_MODEL"
	hash "0xA6E9FDCB2C76785E"
	jhash (0x9DA21956)
	arguments {
		Any "p0",
	}
	returns	"void"

native "REMOVE_VEHICLE_HIGH_DETAIL_MODEL"
	hash "0x00689CDE5F7C6787"
	jhash (0x382BE070)
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_VEHICLE_HIGH_DETAIL"
	hash "0x1F25887F3C104278"
	jhash (0x55D41928)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	REQUEST_VEHICLE_ASSET(GET_HASH_KEY(cargobob3)), 3;

	vehicle found that have asset's:
	cargobob3
	submersible
</summary>
]]--
native "REQUEST_VEHICLE_ASSET"
	hash "0x81A15811460FAB3A"
	jhash (0x902B4F06)
	arguments {
		Hash "VehicleHash",

		int "vehicleAsset",
	}
	returns	"void"

native "HAS_VEHICLE_ASSET_LOADED"
	hash "0x1BBE0523B8DB9A21"
	jhash (0x8DAAC3CB)
	arguments {
		int "vehicleAsset",
	}
	returns	"BOOL"

native "REMOVE_VEHICLE_ASSET"
	hash "0xACE699C71AB9DEB5"
	jhash (0x9620E9C6)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Sets how much the crane on the tow truck is raised.

	towTruck: The tow truck vehicle.
	state:
	    1.0f is fully raised.
	    0.0f is fully lowered.
</summary>
]]--
native "_SET_TOW_TRUCK_CRANE_RAISED"
	hash "0xFE54B92A344583CA"
	jhash (0x88236E22)
	arguments {
		Vehicle "towTruck",

		float "state",
	}
	returns	"void"

--[[!
<summary>
	First two parameters swapped. Scripts verify that towTruck is the first parameter, not the second.
</summary>
]]--
native "ATTACH_VEHICLE_TO_TOW_TRUCK"
	hash "0x29A16F8D621C4508"
	jhash (0x8151571A)
	arguments {
		Vehicle "towTruck",

		Vehicle "vehicle",

		int "p2",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

--[[!
<summary>
	First two parameters swapped. Scripts verify that towTruck is the first parameter, not the second.
</summary>
]]--
native "DETACH_VEHICLE_FROM_TOW_TRUCK"
	hash "0xC2DB6B6708350ED8"
	jhash (0xC666CF33)
	arguments {
		Vehicle "towTruck",

		Vehicle "vehicle",
	}
	returns	"void"

native "DETACH_VEHICLE_FROM_ANY_TOW_TRUCK"
	hash "0xD0E9CE05A1E68CD8"
	jhash (0x3BF93651)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	MulleDK19: First two parameters swapped. Scripts verify that towTruck is the first parameter, not the second.
</summary>
]]--
native "IS_VEHICLE_ATTACHED_TO_TOW_TRUCK"
	hash "0x146DF9EC4C4B9FD4"
	jhash (0x9699CFDC)
	arguments {
		Vehicle "towTruck",

		Vehicle "vehicle",
	}
	returns	"BOOL"

native "GET_ENTITY_ATTACHED_TO_TOW_TRUCK"
	hash "0xEFEA18DCF10F8F75"
	jhash (0x11EC7844)
	arguments {
		Vehicle "towTruck",
	}
	returns	"Entity"

native "SET_VEHICLE_AUTOMATICALLY_ATTACHES"
	hash "0x8BA6F76BC53A1493"
	jhash (0x4273A8D3)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "0xF8EBCCC96ADB9FB7"
	hash "0xF8EBCCC96ADB9FB7"
	jhash (0xED23C8A3)
	arguments {
		Any "p0",

		float "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x56B94C6D7127DFBA"
	hash "0x56B94C6D7127DFBA"
	jhash (0xB1A52EF7)
	arguments {
		Any "p0",

		float "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x1093408B4B9D1146"
	hash "0x1093408B4B9D1146"
	jhash (0xF30C566F)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x30D779DE7C4F6DD3"
	hash "0x30D779DE7C4F6DD3"
	jhash (0xA7DF64D7)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x9AA47FFF660CB932"
	hash "0x9AA47FFF660CB932"
	jhash (0xDD7936F5)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0xA4822F1CF23F4810"
	hash "0xA4822F1CF23F4810"
	jhash (0x34E02FCD)
	arguments {
		Vector3Ptr "outVec",

		Any "p1",

		Vector3Ptr "outVec1",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",
	}
	returns	"BOOL"

--[[!
<summary>
	On accelerating, spins the driven wheels with the others braked, so you don't go anywhere.
</summary>
]]--
native "SET_VEHICLE_BURNOUT"
	hash "0xFB8794444A7D60FB"
	jhash (0x9B6EF0EA)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Returns whether the specified vehicle is currently in a burnout.


	vb.net
	Public Function isVehicleInBurnout(vh As Vehicle) As Boolean
	        Return Native.Function.Call(Of Boolean)(Hash.IS_VEHICLE_IN_BURNOUT, vh)
	    End Function
</summary>
]]--
native "IS_VEHICLE_IN_BURNOUT"
	hash "0x1297A88E081430EB"
	jhash (0x6632BC12)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	Reduces grip significantly so it's hard to go anywhere.
</summary>
]]--
native "SET_VEHICLE_REDUCE_GRIP"
	hash "0x222FF6A823D122E2"
	jhash (0x90D3A0D9)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Sets the turn signal enabled for a vehicle.
	Set turnSignal to 1 for left light, 0 for right light.
</summary>
]]--
native "SET_VEHICLE_INDICATOR_LIGHTS"
	hash "0xB5D45264751B7DF0"
	jhash (0xA6073B5D)
	arguments {
		Vehicle "vehicle",

		int "turnSignal",

		BOOL "toggle",
	}
	returns	"void"

native "SET_VEHICLE_BRAKE_LIGHTS"
	hash "0x92B35082E0B42F66"
	jhash (0x6D9BA11E)
	arguments {
		Vehicle "vehicle",

		BOOL "Toggle",
	}
	returns	"void"

--[[!
<summary>
	does this work while in air?
</summary>
]]--
native "SET_VEHICLE_HANDBRAKE"
	hash "0x684785568EF26A22"
	jhash (0xBA729A25)
	arguments {
		Vehicle "vehicle",

		BOOL "Toggle",
	}
	returns	"void"

native "0x48ADC8A773564670"
	hash "0x48ADC8A773564670"
	jhash (0x37BC6ACB)
	returns	"void"

native "0x91D6DD290888CBAB"
	hash "0x91D6DD290888CBAB"
	jhash (0x71D898EF)
	returns	"Any"

native "0x51DB102F4A3BA5E0"
	hash "0x51DB102F4A3BA5E0"
	jhash (0x0B0523B0)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Gets the trailer of a vehicle and puts it into the trailer parameter.
</summary>
]]--
native "GET_VEHICLE_TRAILER_VEHICLE"
	hash "0x1CDD6BADC297830D"
	jhash (0xAE84D758)
	arguments {
		Vehicle "vehicle",

		VehiclePtr "trailer",
	}
	returns	"BOOL"

native "0xCAC66558B944DA67"
	hash "0xCAC66558B944DA67"
	jhash (0x0B200CE2)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_VEHICLE_RUDDER_BROKEN"
	hash "0x09606148B6C71DEF"
	jhash (0x3FAC3CD4)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x1A78AD3D8240536F"
	hash "0x1A78AD3D8240536F"
	jhash (0x0858678C)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"Any"

--[[!
<summary>
	returns speed in mph
</summary>
]]--
native "0x53AF99BAA671CA47"
	hash "0x53AF99BAA671CA47"
	jhash (0x7D1A0616)
	arguments {
		Vehicle "vehicle",
	}
	returns	"float"

native "GET_VEHICLE_MAX_BRAKING"
	hash "0xAD7E85FC227197C4"
	jhash (0x03B926F6)
	arguments {
		Any "p0",
	}
	returns	"float"

native "GET_VEHICLE_MAX_TRACTION"
	hash "0xA132FB5370554DB0"
	jhash (0x7E5A1587)
	arguments {
		Any "p0",
	}
	returns	"float"

native "GET_VEHICLE_ACCELERATION"
	hash "0x5DD35C8D074E57AE"
	jhash (0x00478321)
	arguments {
		Any "p0",
	}
	returns	"float"

native "_GET_VEHICLE_MAX_SPEED"
	hash "0xF417C2502FFFED43"
	jhash (0x8F291C4A)
	arguments {
		Hash "modelHash",
	}
	returns	"float"

native "GET_VEHICLE_MODEL_MAX_BRAKING"
	hash "0xDC53FD41B4ED944C"
	jhash (0x7EF02883)
	arguments {
		Any "p0",
	}
	returns	"float"

native "0xBFBA3BA79CFF7EBF"
	hash "0xBFBA3BA79CFF7EBF"
	jhash (0xF3A7293F)
	arguments {
		Hash "modelHash",
	}
	returns	"float"

native "GET_VEHICLE_MODEL_MAX_TRACTION"
	hash "0x539DE94D44FDFD0D"
	jhash (0x7F985597)
	arguments {
		Any "p0",
	}
	returns	"float"

--[[!
<summary>
	Returns the acceleration of the specified model.

	Note: Notice that they have added "MODEL" to this definition, which possibly means that we should be passing a parameter with a model hash instead of a vehicle handle. 

	p0: The hash of the model.

</summary>
]]--
native "GET_VEHICLE_MODEL_ACCELERATION"
	hash "0x8C044C5C84505B6A"
	jhash (0x29CB3537)
	arguments {
		Any "p0",
	}
	returns	"float"

native "0x53409B5163D5B846"
	hash "0x53409B5163D5B846"
	jhash (0x37FBA7BC)
	arguments {
		Hash "modelHash",
	}
	returns	"float"

--[[!
<summary>
	Function pertains only to aviation vehicles.
</summary>
]]--
native "0xC6AD107DDC9054CC"
	hash "0xC6AD107DDC9054CC"
	jhash (0x95BB67EB)
	arguments {
		Hash "modelHash",
	}
	returns	"float"

native "0x5AA3F878A178C4FC"
	hash "0x5AA3F878A178C4FC"
	jhash (0x87C5D271)
	arguments {
		Any "p0",
	}
	returns	"float"

native "0x00C09F246ABEDD82"
	hash "0x00C09F246ABEDD82"
	jhash (0xCE67162C)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_VEHICLE_CLASS_MAX_TRACTION"
	hash "0xDBC86D85C5059461"
	jhash (0x5B4FDC16)
	arguments {
		Any "p0",
	}
	returns	"float"

native "GET_VEHICLE_CLASS_MAX_AGILITY"
	hash "0x4F930AD022D6DE3B"
	jhash (0x45F2BD83)
	arguments {
		Any "p0",
	}
	returns	"float"

native "GET_VEHICLE_CLASS_MAX_ACCELERATION"
	hash "0x2F83E7E45D9EA7AE"
	jhash (0x3E220A9B)
	arguments {
		Any "p0",
	}
	returns	"float"

native "GET_VEHICLE_CLASS_MAX_BRAKING"
	hash "0x4BF54C16EC8FEC03"
	jhash (0xD08CC1A5)
	arguments {
		Any "p0",
	}
	returns	"float"

native "0x2CE544C68FB812A0"
	hash "0x2CE544C68FB812A0"
	jhash (0xD6685803)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		BOOL "p5",
	}
	returns	"Any"

native "0x1033371FC8E842A7"
	hash "0x1033371FC8E842A7"
	jhash (0x0C0332A6)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "_OPEN_VEHICLE_BOMB_BAY"
	hash "0x87E7F24270732CB1"
	jhash (0x6574041D)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x3556041742A0DC74"
	hash "0x3556041742A0DC74"
	jhash (0xF8EC5751)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Possibly: Returns whether the searchlight (found on police vehicles) is toggled on.
</summary>
]]--
native "IS_VEHICLE_SEARCHLIGHT_ON"
	hash "0xC0F97FCE55094987"
	jhash (0xADAF3513)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "SET_VEHICLE_SEARCHLIGHT"
	hash "0x14E85C5EE7A4D542"
	jhash (0xE2C0DD8A)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x639431E895B9AA57"
	hash "0x639431E895B9AA57"
	jhash (0xAB0E79EB)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"BOOL"

native "CAN_SHUFFLE_SEAT"
	hash "0x30785D90C956BF35"
	jhash (0xB3EB01ED)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "GET_NUM_MOD_KITS"
	hash "0x33F2E3FE70EAAE1D"
	jhash (0xE4903AA0)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

--[[!
<summary>
	Set modKit to 0 if you plan to call SET_VEHICLE_MOD. That's what the game does. Most body modifications through SET_VEHICLE_MOD will not take effect until this is set to 0.
</summary>
]]--
native "SET_VEHICLE_MOD_KIT"
	hash "0x1F2AA07F00B3217A"
	jhash (0xB8132158)
	arguments {
		Vehicle "vehicle",

		int "modKit",
	}
	returns	"void"

native "GET_VEHICLE_MOD_KIT"
	hash "0x6325D1A044AE510D"
	jhash (0x9FE60927)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

native "GET_VEHICLE_MOD_KIT_TYPE"
	hash "0xFC058F5121E54C32"
	jhash (0xE5F76765)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

--[[!
<summary>
	Returns an int

	Wheel Types:
	0: Sport
	1: Muscle
	2: Lowrider
	3: SUV
	4: Offroad
	5: Tuner
	6: Bike Wheels
	7: High End

	Tested in Los Santos Customs
</summary>
]]--
native "GET_VEHICLE_WHEEL_TYPE"
	hash "0xB3ED1BFB4BE636DC"
	jhash (0xDA58D7AE)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

--[[!
<summary>
	0: Sport
	1: Muscle
	2: Lowrider
	3: SUV
	4: Offroad
	5: Tuner
	6: Bike Wheels
	7: High End
</summary>
]]--
native "SET_VEHICLE_WHEEL_TYPE"
	hash "0x487EB21CC7295BA1"
	jhash (0x64BDAAAD)
	arguments {
		Vehicle "vehicle",

		int "WheelType",
	}
	returns	"Any"

native "GET_NUM_MOD_COLORS"
	hash "0xA551BE18C11A476D"
	jhash (0x73722CD9)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"Any"

--[[!
<summary>
	paintType:
	0: Normal
	1: Metallic
	2: Pearl
	3: Matte
	4: Metal
	5: Chrome

	color: number of the color
</summary>
]]--
native "SET_VEHICLE_MOD_COLOR_1"
	hash "0x43FEB945EE7F85B8"
	jhash (0xCBE9A54D)
	arguments {
		Vehicle "vehicle",

		int "paintType",

		int "color",

		int "p3",
	}
	returns	"void"

--[[!
<summary>
	Changes the secondary paint type and color
	paintType:
	0: Normal
	1: Metallic
	2: Pearl
	3: Matte
	4: Metal
	5: Chrome

	color: number of the color
</summary>
]]--
native "SET_VEHICLE_MOD_COLOR_2"
	hash "0x816562BADFDEC83E"
	jhash (0xC32613C2)
	arguments {
		Vehicle "vehicle",

		int "paintType",

		int "color",
	}
	returns	"void"

native "GET_VEHICLE_MOD_COLOR_1"
	hash "0xE8D65CA700C9A693"
	jhash (0xE625510A)
	arguments {
		Vehicle "vehicle",

		intPtr "paintType",

		intPtr "color",

		intPtr "p3",
	}
	returns	"void"

native "GET_VEHICLE_MOD_COLOR_2"
	hash "0x81592BE4E3878728"
	jhash (0x9B76BB8E)
	arguments {
		Vehicle "vehicle",

		intPtr "paintType",

		intPtr "color",
	}
	returns	"void"

native "0xB45085B721EFD38C"
	hash "0xB45085B721EFD38C"
	jhash (0x9A0840FD)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"Any"

native "0x4967A516ED23A5A1"
	hash "0x4967A516ED23A5A1"
	jhash (0x9BDC0B49)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x9A83F5F9963775EF"
	hash "0x9A83F5F9963775EF"
	jhash (0x112D637A)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	Sets the vehicle mod.
	The vehicle must have a mod kit first.

	Any out of range ModIndex is stock.

	indexes: http://pastebin.com/w3q7Vux5

	#Mod Type
	Spoilers 
	Front Bumper 
	Rear Bumper 
	Side Skirt 
	Exhaust 
	Frame 
	Grille 
	Hood 
	Fender 
	Right Fender 
	Roof 
	Engine 
	Brakes 
	Transmission 
	Horns 
	Suspension 
	Armor 
	Front Wheels 
	Back Wheels - 24 //only for motocycles
	Plate holders 
	Trim Design 
	Ornaments 
	Dial Design 
	Steering Wheel 
	Shifter Leavers 
	Plaques 
	Hydraulics 

	Wheel enums: http://pastebin.com/qqhuivXK
</summary>
]]--
native "SET_VEHICLE_MOD"
	hash "0x6AF0636DDEDCB6DD"
	jhash (0xB52E5ED5)
	arguments {
		Vehicle "vehicle",

		int "modType",

		int "modIndex",

		BOOL "customTires",
	}
	returns	"void"

--[[!
<summary>
	Returns 4294967295 if the vehicle mod is stock
</summary>
]]--
native "GET_VEHICLE_MOD"
	hash "0x772960298DA26FDB"
	jhash (0xDC520069)
	arguments {
		Vehicle "vehicle",

		int "modType",
	}
	returns	"int"

--[[!
<summary>
	Only used for wheels(ModType = 23/24) Returns true if the wheels are custom wheels
</summary>
]]--
native "GET_VEHICLE_MOD_VARIATION"
	hash "0xB3924ECD70E095DC"
	jhash (0xC1B92003)
	arguments {
		Vehicle "vehicle",

		int "modType",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns how many possible mods a vehicle has for a given mod type
</summary>
]]--
native "GET_NUM_VEHICLE_MODS"
	hash "0xE38E9162A2500646"
	jhash (0x8A814FF9)
	arguments {
		Vehicle "vehicle",

		int "modType",
	}
	returns	"int"

native "REMOVE_VEHICLE_MOD"
	hash "0x92D619E420858204"
	jhash (0x9CC80A43)
	arguments {
		Vehicle "vehicle",

		int "modType",
	}
	returns	"void"

--[[!
<summary>
	Toggles:
	UNK17 
	Turbo 
	UNK19 
	Tire Smoke 
	UNK21 
	Xenon Headlights 
</summary>
]]--
native "TOGGLE_VEHICLE_MOD"
	hash "0x2A1F4F37F95BAD08"
	jhash (0xD095F811)
	arguments {
		Vehicle "vehicle",

		int "modType",

		BOOL "toggle",
	}
	returns	"void"

native "IS_TOGGLE_MOD_ON"
	hash "0x84B233A8C8FC8AE7"
	jhash (0xF0E1689F)
	arguments {
		Vehicle "vehicle",

		int "modType",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns the text label of a mod type for a given vehicle

	Use _GET_LABEL_TEXT to get the part name in the game's language
</summary>
]]--
native "GET_MOD_TEXT_LABEL"
	hash "0x8935624F8C5592CC"
	jhash (0x0BA39CA7)
	arguments {
		Vehicle "vehicle",

		int "modType",

		int "modValue",
	}
	returns	"charPtr"

--[[!
<summary>
	Returns the name for the type of vehicle mod(Armour, engine etc)

</summary>
]]--
native "GET_MOD_SLOT_NAME"
	hash "0x51F0FEB9F6AE98C0"
	jhash (0x5E113483)
	arguments {
		Vehicle "vehicle",

		int "modType",
	}
	returns	"charPtr"

--[[!
<summary>
	I would have thought that the params would be Vehicle vehicle and int liveryIndex but that doesn't seem to get anything other than a blank string out of this native. Tried both normal and localized text displaying.

	EDIT FROM SOMEONE ELSE: The 2nd parameter is the return from GET_VEHICLE_LIVERY.
</summary>
]]--
native "GET_LIVERY_NAME"
	hash "0xB4C7A93837C91A1F"
	jhash (0xED80B5BE)
	arguments {
		Vehicle "vehicle",

		charPtr "livery",
	}
	returns	"charPtr"

native "GET_VEHICLE_MOD_MODIFIER_VALUE"
	hash "0x90A38E9838E0A8C1"
	jhash (0x73AE5505)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "0x4593CF82AA179706"
	hash "0x4593CF82AA179706"
	jhash (0x94850968)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "PRELOAD_VEHICLE_MOD"
	hash "0x758F49C24925568A"
	jhash (0x6EA5F4A8)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "HAS_PRELOAD_MODS_FINISHED"
	hash "0x06F43E5175EB6D96"
	jhash (0xA8A0D246)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "RELEASE_PRELOAD_MODS"
	hash "0x445D79F995508307"
	jhash (0xD442521F)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Sets the tire smoke's color of this vehicle.

	p0: The vehicle that is the target of this method.
	p1: The red level in the RGB color code.
	p2: The green level in the RGB color code.
	p3: The blue level in the RGB color code.

	Note:
	setting r,g,b to 0 will give the car independance day tyre smoke
</summary>
]]--
native "SET_VEHICLE_TYRE_SMOKE_COLOR"
	hash "0xB5BA80F839791C0F"
	jhash (0x3EDEC0DB)
	arguments {
		Vehicle "vehicle",

		int "r",

		int "g",

		int "b",
	}
	returns	"void"

native "GET_VEHICLE_TYRE_SMOKE_COLOR"
	hash "0xB635392A4938B3C3"
	jhash (0x75280015)
	arguments {
		Vehicle "vehicle",

		intPtr "r",

		intPtr "g",

		intPtr "b",
	}
	returns	"void"

--[[!
<summary>
	Tint:
	None 
	Pure Black 
	Dark Smoke 
	Light Smoke 
	Stock 
	Limo 
	Green 
</summary>
]]--
native "SET_VEHICLE_WINDOW_TINT"
	hash "0x57C51E6BAD752696"
	jhash (0x497C8787)
	arguments {
		Vehicle "vehicle",

		int "tint",
	}
	returns	"void"

native "GET_VEHICLE_WINDOW_TINT"
	hash "0x0EE21293DAD47C95"
	jhash (0x13D53892)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

native "GET_NUM_VEHICLE_WINDOW_TINTS"
	hash "0x9D1224004B3A6707"
	jhash (0x625C7B66)
	returns	"int"

native "GET_VEHICLE_COLOR"
	hash "0xF3CC740D36221548"
	jhash (0x03BC8F1B)
	arguments {
		Vehicle "vehicle",

		intPtr "r",

		intPtr "g",

		intPtr "b",
	}
	returns	"void"

native "0xEEBFC7A7EFDC35B4"
	hash "0xEEBFC7A7EFDC35B4"
	jhash (0x749DEEA2)
	arguments {
		Any "p0",
	}
	returns	"int"

native "GET_VEHICLE_CAUSE_OF_DESTRUCTION"
	hash "0xE495D1EF4C91FD20"
	jhash (0x7F8C20DD)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	From the driver's perspective, is the left headlight broken.
</summary>
]]--
native "_IS_HEADLIGHT_L_BROKEN"
	hash "0x5EF77C9ADD3B11A3"
	jhash (0xA0777943)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	From the driver's perspective, is the right headlight broken.
</summary>
]]--
native "_IS_HEADLIGHT_R_BROKEN"
	hash "0xA7ECB73355EB2F20"
	jhash (0xF178390B)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	Vehicle power multiplier.
	Does not have to be looped each frame. Can be set once.
	Values lower than 1f don't work.

	Note: If the value is set with GET_RANDOM_FLOAT_IN_RANGE, the vehicle will have an absurdly high ammount of power, and will become almost undrivable for the player or NPCs. The range doesn't seem to matter.

	An high value like 10000000000f will visually remove the wheels that apply the power (front wheels for FWD, rear wheels for RWD), but the power multiplier will still apply, and the wheels still work.
</summary>
]]--
native "_SET_VEHICLE_ENGINE_POWER_MULTIPLIER"
	hash "0x93A3996368C94158"
	jhash (0xE943B09C)
	arguments {
		Vehicle "vehicle",

		float "value",
	}
	returns	"void"

native "0x1CF38D529D7441D9"
	hash "0x1CF38D529D7441D9"
	jhash (0xDF594D8D)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x1F9FB66F3A3842D2"
	hash "0x1F9FB66F3A3842D2"
	jhash (0x4D840FC4)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x54B0F614960F4A5F"
	hash "0x54B0F614960F4A5F"
	jhash (0x5AB26C2B)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",
	}
	returns	"Any"

native "0xE30524E1871F481D"
	hash "0xE30524E1871F481D"
	jhash (0xEF05F807)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x291E373D483E7EE7"
	hash "0x291E373D483E7EE7"
	jhash (0xD656E7E5)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	&lt;1.0 - Decreased torque
	=1.0 - Default torque
	&gt;1.0 - Increased torque

	Negative values will cause the vehicle to go backwards instead of forwards while accelerating.

	value - is between 0.2 and 1.8 in the decompiled scripts. 

	Does this need to be looped or can it be set once for a vehicle?
</summary>
]]--
native "_SET_VEHICLE_ENGINE_TORQUE_MULTIPLIER"
	hash "0xB59E4BD37AE292DB"
	jhash (0x642DA5AA)
	arguments {
		Vehicle "vehicle",

		float "value",
	}
	returns	"void"

native "0x0AD9E8F87FF7C16F"
	hash "0x0AD9E8F87FF7C16F"
	jhash (0x04F5546C)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Sets the wanted state of this vehicle.

	p0: The vehicle that is the target of this method.
	p1: The wanted state of this vehicle.
</summary>
]]--
native "SET_VEHICLE_IS_WANTED"
	hash "0xF7EC25A3EBEEC726"
	jhash (0xDAA388E8)
	arguments {
		Vehicle "p0",

		BOOL "p1",
	}
	returns	"Any"

native "0xF488C566413B4232"
	hash "0xF488C566413B4232"
	jhash (0xA25CCB8C)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0xC1F981A6F74F0C23"
	hash "0xC1F981A6F74F0C23"
	jhash (0x00966934)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x0F3B4D4E43177236"
	hash "0x0F3B4D4E43177236"
	jhash (0x113DF5FD)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x6636C535F6CC2725"
	hash "0x6636C535F6CC2725"
	jhash (0x7C8D6464)
	arguments {
		Any "p0",
	}
	returns	"float"

native "DISABLE_PLANE_AILERON"
	hash "0x23428FC53C60919C"
	jhash (0x7E84C45C)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	Returns true when in a vehicle, false whilst entering/exiting.
</summary>
]]--
native "_IS_VEHICLE_ENGINE_ON"
	hash "0xAE31E7DF9B5B132E"
	jhash (0x7DC6D022)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "0x1D97D1E3A70A649F"
	hash "0x1D97D1E3A70A649F"
	jhash (0xA03E42DF)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Only works on bikes, both X and Y work in the -1 - 1 range.

	X forces the bike to turn left or right (-1, 1)
	Y forces the bike to lean to the left or to the right (-1, 1)

	Example with X -1/Y 1
	http://i.imgur.com/TgIuAPJ.jpg
</summary>
]]--
native "_SET_BIKE_LEAN_ANGLE"
	hash "0x9CFA4896C3A53CBB"
	jhash (0x15D40761)
	arguments {
		Vehicle "vehicle",

		float "x",

		float "y",
	}
	returns	"void"

native "0xAB04325045427AAE"
	hash "0xAB04325045427AAE"
	jhash (0x1984F88D)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xCFD778E7904C255E"
	hash "0xCFD778E7904C255E"
	jhash (0x3FBE904F)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xACFB2463CC22BED2"
	hash "0xACFB2463CC22BED2"
	jhash (0xD1B71A25)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xB2D06FAEDE65B577"
	hash "0xB2D06FAEDE65B577"
	jhash (0xFEB0C0C8)
	returns	"Any"

native "0xE01903C47C7AC89E"
	hash "0xE01903C47C7AC89E"
	returns	"void"

native "0x02398B627547189C"
	hash "0x02398B627547189C"
	jhash (0x08CD58F9)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xB893215D8D4C015B"
	hash "0xB893215D8D4C015B"
	jhash (0x8C4B63E2)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "SET_VEHICLE_LOD_MULTIPLIER"
	hash "0x93AE6A61BE015BF1"
	jhash (0x569E5AE3)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x428BACCDF5E26EAD"
	hash "0x428BACCDF5E26EAD"
	jhash (0x1604C2F5)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x42A4BEB35D372407"
	hash "0x42A4BEB35D372407"
	jhash (0x8CDB0C09)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x2C8CBFE1EA5FC631"
	hash "0x2C8CBFE1EA5FC631"
	jhash (0xABC99E21)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x4D9D109F63FEE1D4"
	hash "0x4D9D109F63FEE1D4"
	jhash (0x900C878C)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x279D50DE5652D935"
	hash "0x279D50DE5652D935"
	jhash (0xB3200F72)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xE44A982368A4AF23"
	hash "0xE44A982368A4AF23"
	jhash (0xBAE491C7)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xF25E02CB9C5818F8"
	hash "0xF25E02CB9C5818F8"
	jhash (0xF0E59BC1)
	returns	"void"

native "0xBC3CCA5844452B06"
	hash "0xBC3CCA5844452B06"
	jhash (0x929801C6)
	arguments {
		float "p0",
	}
	returns	"void"

--[[!
<summary>
	Commands the driver of an armed vehicle (p0) to shoot its weapon at a target (p1). p3, p4 and p5 are the coordinates of the target. Example:

	WEAPON::SET_CURRENT_PED_VEHICLE_WEAPON(pilot,GAMEPLAY::GET_HASH_KEY("VEHICLE_WEAPON_PLANE_ROCKET"));						VEHICLE::SET_VEHICLE_SHOOT_AT_TARGET(pilot, target, targPos.x, targPos.y, targPos.z);
</summary>
]]--
native "SET_VEHICLE_SHOOT_AT_TARGET"
	hash "0x74CD9A9327A282EA"
	jhash (0x2343FFDF)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"void"

--[[!
<summary>
	Originally called "_SET_VEHICLE_OWNER", but this actually seems to GET the "owner", not set it.

	The resulting entity can be a Vehicle or Ped.
</summary>
]]--
native "_GET_VEHICLE_OWNER"
	hash "0x8F5EBAB1F260CFCE"
	jhash (0x4A557117)
	arguments {
		Vehicle "vehicle",

		EntityPtr "entity",
	}
	returns	"BOOL"

native "0x97CE68CB032583F0"
	hash "0x97CE68CB032583F0"
	jhash (0xE0FC6A32)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "0x182F266C2D9E2BEB"
	hash "0x182F266C2D9E2BEB"
	jhash (0x7D0DE7EA)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "GET_VEHICLE_PLATE_TYPE"
	hash "0x9CCC9525BF2408E0"
	jhash (0x65CA9286)
	arguments {
		Vehicle "vehicle",
	}
	returns	"Any"

--[[!
<summary>
	in script hook .net 

	Vehicle v = ...;
	Function.Call(Hash.TRACK_VEHICLE_VISIBILITY, v.Handle);
</summary>
]]--
native "TRACK_VEHICLE_VISIBILITY"
	hash "0x64473AEFDCF47DCA"
	jhash (0x78122DC1)
	arguments {
		Vehicle "vehicle",
	}
	returns	"void"

--[[!
<summary>
	must be called after TRACK_VEHICLE_VISIBILITY 

	-7RI993R

	it's not instant so probabilly must pass an 'update' to see correct result.
</summary>
]]--
native "IS_VEHICLE_VISIBLE"
	hash "0xAA0A52D24FB98293"
	jhash (0x7E0D6056)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "SET_VEHICLE_GRAVITY"
	hash "0x89F149B6131E57DA"
	jhash (0x07B2A6DC)
	arguments {
		Vehicle "vehicle",

		BOOL "Toggle",
	}
	returns	"void"

native "0xE6C0C80B8C867537"
	hash "0xE6C0C80B8C867537"
	jhash (0xD2B8ACBD)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x36492C2F0D134C56"
	hash "0x36492C2F0D134C56"
	jhash (0xA4A75FCF)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x06582AFF74894C75"
	hash "0x06582AFF74894C75"
	jhash (0x50F89338)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xDFFCEF48E511DB48"
	hash "0xDFFCEF48E511DB48"
	jhash (0xEB7D7C27)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x8D474C8FAEFF6CDE"
	hash "0x8D474C8FAEFF6CDE"
	jhash (0x5EB00A6A)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SET_VEHICLE_ENGINE_CAN_DEGRADE"
	hash "0x983765856F2564F9"
	jhash (0x081DAC12)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Adds some kind of shadow to the vehicle.
</summary>
]]--
native "0xF0E4BA16D1DB546C"
	hash "0xF0E4BA16D1DB546C"
	jhash (0x5BD8D82D)
	arguments {
		Vehicle "vehicle",

		int "p1",

		int "p2",
	}
	returns	"void"

native "0xF87D9F2301F7D206"
	hash "0xF87D9F2301F7D206"
	jhash (0x450AD03A)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x4198AB0022B15F87"
	hash "0x4198AB0022B15F87"
	jhash (0xBD085DCA)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x755D6D5267CBBD7E"
	hash "0x755D6D5267CBBD7E"
	jhash (0xABBDD5C6)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x0CDDA42F9E360CA6"
	hash "0x0CDDA42F9E360CA6"
	jhash (0x9B581DE7)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_VEHICLE_STOLEN"
	hash "0x4AF9BD80EEBEB453"
	jhash (0x20B61DDE)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "SET_VEHICLE_IS_STOLEN"
	hash "0x67B2C79AA7FF5738"
	jhash (0x70912E42)
	arguments {
		Vehicle "vehicle",

		BOOL "isStolen",
	}
	returns	"Any"

native "0xAD2D28A1AFDFF131"
	hash "0xAD2D28A1AFDFF131"
	jhash (0xED159AE6)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x5991A01434CE9677"
	hash "0x5991A01434CE9677"
	jhash (0xAF8CB3DF)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xB264C4D2F2B0A78B"
	hash "0xB264C4D2F2B0A78B"
	jhash (0x45F72495)
	arguments {
		Any "p0",
	}
	returns	"void"

native "DETACH_VEHICLE_FROM_CARGOBOB"
	hash "0x0E21D3DF1051399D"
	jhash (0x83D3D331)
	arguments {
		Vehicle "vehicle",

		Vehicle "cargobob",
	}
	returns	"void"

native "DETACH_VEHICLE_FROM_ANY_CARGOBOB"
	hash "0xADF7BE450512C12F"
	jhash (0x50E0EABE)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "IS_VEHICLE_ATTACHED_TO_CARGOBOB"
	hash "0xD40148F22E81A1D9"
	jhash (0x5DEEC76C)
	arguments {
		Vehicle "vehicle",

		Vehicle "cargobob",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns attached vehicle (Vehicle in parameter must be cargobob)
</summary>
]]--
native "GET_VEHICLE_ATTACHED_TO_CARGOBOB"
	hash "0x873B82D42AC2B9E5"
	jhash (0x301A1D24)
	arguments {
		Vehicle "cargobob",
	}
	returns	"Vehicle"

native "ATTACH_VEHICLE_TO_CARGOBOB"
	hash "0x4127F1D84E347769"
	jhash (0x607DC9D5)
	arguments {
		Vehicle "vehicle",

		Vehicle "cargobob",

		int "p2",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "0x571FEB383F629926"
	hash "0x571FEB383F629926"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xCBDB9B923CACC92D"
	hash "0xCBDB9B923CACC92D"
	arguments {
		Any "p0",
	}
	returns	"int"

--[[!
<summary>
	Returns true only when the hook is active, will return false if the magnet is active
</summary>
]]--
native "_IS_CARGOBOB_HOOK_ACTIVE"
	hash "0x1821D91AD4B56108"
	jhash (0xAF769B81)
	arguments {
		Vehicle "cargobob",
	}
	returns	"BOOL"

--[[!
<summary>
	Drops the Hook/Magnet on a cargobob

	state
	0 = hook
	1 = magnet
	=====================================
	Previous parameters were, (Vehicle cargobob).

	Note: I changed it to (bool state), as the notes committed above say 0 and 1 do different things, but if the parameter involves putting your vehicle in there how can you set 0 or 1 involving rather it drops a hook or magnet without a second parameter. It's already defined as messing with the cargobob. 

	Note 2: Parameters (int state) do not work. Have swapped it to bool state for anyone to test. If anyone finds the true parameters please correct and delete these two notes.

	Note 3: If this native name is ever found to be correct as you may notice a good bit there are separate natives for hook and magnet. So maybe if found in the future there may be a _ENABLE_CARGOBOB_MAGNET(bool state)
</summary>
]]--
native "_ENABLE_CARGOBOB_HOOK"
	hash "0x7BEB0C7A235F6F3B"
	jhash (0x4D3C9A99)
	arguments {
		BOOL "state",
	}
	returns	"void"

--[[!
<summary>
	Retracts the hook on the cargobob.

	Note: after you retract it the natives for dropping the hook no longer work
</summary>
]]--
native "_RETRACT_CARGOBOB_HOOK"
	hash "0x9768CF648F54C804"
	jhash (0xA8211EE9)
	arguments {
		Vehicle "cargobob",
	}
	returns	"void"

native "0x877C1EAEAC531023"
	hash "0x877C1EAEAC531023"
	jhash (0x3A8AB081)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		BOOL "p3",
	}
	returns	"void"

native "0xCF1182F682F65307"
	hash "0xCF1182F682F65307"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	Returns true only when the magnet is active, will return false if the hook is active
</summary>
]]--
native "_IS_CARGOBOB_MAGNET_ACTIVE"
	hash "0x6E08BF5B3722BAC9"
	arguments {
		Vehicle "cargobob",
	}
	returns	"BOOL"

--[[!
<summary>
	Setting Grab to true will cause the cargobob to grab a vehicle if there is one in range
	Setting Grab to false will make the cargobob drop any vehicle attached to the magnet
</summary>
]]--
native "_CARGOBOB_MAGNET_GRAB_VEHICLE"
	hash "0x9A665550F8DA349B"
	arguments {
		Vehicle "cargobob",

		BOOL "Grab",
	}
	returns	"void"

native "0xBCBFCD9D1DAC19E2"
	hash "0xBCBFCD9D1DAC19E2"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0xA17BAD153B51547E"
	hash "0xA17BAD153B51547E"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x66979ACF5102FD2F"
	hash "0x66979ACF5102FD2F"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x6D8EAC07506291FB"
	hash "0x6D8EAC07506291FB"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0xED8286F71A819BAA"
	hash "0xED8286F71A819BAA"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x685D5561680D088B"
	hash "0x685D5561680D088B"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0xE301BD63E9E13CF0"
	hash "0xE301BD63E9E13CF0"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x9BDDC73CC6A115D4"
	hash "0x9BDDC73CC6A115D4"
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x56EB5E94318D3FB6"
	hash "0x56EB5E94318D3FB6"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "DOES_VEHICLE_HAVE_WEAPONS"
	hash "0x25ECB9F8017D98E0"
	jhash (0xB2E1E1FB)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

native "0x2C4A1590ABF43E8B"
	hash "0x2C4A1590ABF43E8B"
	jhash (0x2EC19A8B)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "DISABLE_VEHICLE_WEAPON"
	hash "0xF4FC6A6F67D8D856"
	jhash (0xA688B7D1)
	arguments {
		BOOL "disabled",

		Hash "weaponHash",

		Vehicle "vehicle",

		Ped "owner",
	}
	returns	"void"

native "0xE05DD0E9707003A3"
	hash "0xE05DD0E9707003A3"
	jhash (0x123E5B90)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x21115BCD6E44656A"
	hash "0x21115BCD6E44656A"
	jhash (0xEBC225C1)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Returns an int

	Vehicle Classes:
	0: Compacts
	1: Sedans
	2: SUVs
	3: Coupes
	4: Muscle
	5: Sports Classics
	6: Sports
	7: Super
	8: Motorcycle
	9: Off-road
	10: Industrial
	11: Utility
	12: Vans/Pickups
	13: Bicycle
	14: Boats
	15: Helicopter
	16: Airplane
	17: Service
	18: Emergency
	19: Military
	20: Commercial
</summary>
]]--
native "GET_VEHICLE_CLASS"
	hash "0x29439776AAA00A62"
	jhash (0xC025338E)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

native "GET_VEHICLE_CLASS_FROM_NAME"
	hash "0xDEDF1C8BD47C2200"
	jhash (0xEA469980)
	arguments {
		Hash "modelHash",
	}
	returns	"int"

native "SET_PLAYERS_LAST_VEHICLE"
	hash "0xBCDF8BAF56C87B6A"
	jhash (0xDE86447D)
	arguments {
		Vehicle "vehicle",
	}
	returns	"Any"

native "0x300504B23BD3B711"
	hash "0x300504B23BD3B711"
	jhash (0x5130DB1E)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xE5810AC70602F2F5"
	hash "0xE5810AC70602F2F5"
	jhash (0xB6BE07E0)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x068F64F2470F9656"
	hash "0x068F64F2470F9656"
	jhash (0x4BB5605D)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xB8FBC8B1330CA9B4"
	hash "0xB8FBC8B1330CA9B4"
	jhash (0x51E0064F)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x10655FAB9915623D"
	hash "0x10655FAB9915623D"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x79DF7E806202CE01"
	hash "0x79DF7E806202CE01"
	jhash (0xAEF9611C)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x9007A2F21DC108D4"
	hash "0x9007A2F21DC108D4"
	jhash (0x585E49B6)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	value between 0.0 and 1.0
</summary>
]]--
native "_SET_HELICOPTER_ROLL_PITCH_YAW_MULT"
	hash "0x6E0859B530A365CC"
	jhash (0x6E67FD35)
	arguments {
		Vehicle "helicopter",

		float "multiplier",
	}
	returns	"void"

--[[!
<summary>
	In the decompiled scripts are 2 occurencies of this method. One is in the "country_race", setting the friction to 2, whereas in the "trevor1"-file there are 4 values: 0.3, 0.35, 0.4 and 0.65.
	I couldn't really figure out any difference, but you might succeed. In that case, please edit this.
</summary>
]]--
native "SET_VEHICLE_FRICTION_OVERRIDE"
	hash "0x1837AF7C627009BA"
	jhash (0x32AFD42E)
	arguments {
		Vehicle "vehicle",

		float "friction",
	}
	returns	"void"

native "SET_VEHICLE_MAX_STR_TRAP"
	hash "0xA37B9A517B133349"
	jhash (0x670913A4)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xF78F94D60248C737"
	hash "0xF78F94D60248C737"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	looks like hash collision
</summary>
]]--
native "GET_VEHICLE_DEFORMATION_GET_TREE"
	hash "0xA46413066687A328"
	jhash (0x98A10A86)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x5E569EC46EC21CAE"
	hash "0x5E569EC46EC21CAE"
	jhash (0xBC649C49)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x6D6AF961B72728AE"
	hash "0x6D6AF961B72728AE"
	jhash (0x8DD9AA0C)
	arguments {
		Any "p0",
	}
	returns	"void"

native "DOES_VEHICLE_EXIST_WITH_DECORATOR"
	hash "0x956B409B984D9BF7"
	jhash (0x39E68EDD)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x41062318F23ED854"
	hash "0x41062318F23ED854"
	jhash (0xAA8BD440)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xB5C51B5502E85E83"
	hash "0xB5C51B5502E85E83"
	arguments {
		Vehicle "vehicle",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0x500873A45724C863"
	hash "0x500873A45724C863"
	arguments {
		Vehicle "vehicle",

		Any "p1",
	}
	returns	"void"

native "0xB055A34527CB8FD7"
	hash "0xB055A34527CB8FD7"
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "0xF796359A959DF65D"
	hash "0xF796359A959DF65D"
	jhash (0xB5CC548B)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Sets the color of the neon lights of the specified vehicle.

	r: Percentage red between 0 and 255.
	g: Percentage green between 0 and 255.
	b: Percentage blue between 0 and 255.
</summary>
]]--
native "_SET_VEHICLE_NEON_LIGHTS_COLOUR"
	hash "0x8E0A582209A62695"
	arguments {
		Vehicle "vehicle",

		int "r",

		int "g",

		int "b",
	}
	returns	"void"

native "_GET_VEHICLE_NEON_LIGHTS_COLOUR"
	hash "0x7619EEE8C886757F"
	arguments {
		Vehicle "vehicle",

		intPtr "r",

		intPtr "g",

		intPtr "b",
	}
	returns	"void"

--[[!
<summary>
	Sets the neon lights of the specified vehicle on/off.

	Indices:
	0 = Left
	1 = Right
	2 = Front
	3 = Back
</summary>
]]--
native "_SET_VEHICLE_NEON_LIGHT_ENABLED"
	hash "0x2AA720E4287BF269"
	arguments {
		Vehicle "vehicle",

		int "index",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	indices:
	0 = Left
	1 = Right
	2 = Front
	3 = Back
</summary>
]]--
native "_IS_VEHICLE_NEON_LIGHT_ENABLED"
	hash "0x8C4B92553E4766A5"
	arguments {
		Vehicle "vehicle",

		int "index",
	}
	returns	"BOOL"

native "0x35E0654F4BAD7971"
	hash "0x35E0654F4BAD7971"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xB088E9A47AE6EDD5"
	hash "0xB088E9A47AE6EDD5"
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "0xDBA3C090E3D74690"
	hash "0xDBA3C090E3D74690"
	arguments {
		Vehicle "vehicle",
	}
	returns	"void"

--[[!
<summary>
	Seems related to vehicle health, like the one in IV.
	Max 1000, min 0.
	Vehicle does not necessarily explode or become undrivable at 0.
</summary>
]]--
native "GET_VEHICLE_BODY_HEALTH"
	hash "0xF271147EB7B40F12"
	jhash (0x2B2FCC28)
	arguments {
		Vehicle "vehicle",
	}
	returns	"float"

--[[!
<summary>
	p2 often set to 1000.0 in the decompiled scripts.
</summary>
]]--
native "SET_VEHICLE_BODY_HEALTH"
	hash "0xB77D05AC8C78AADB"
	jhash (0x920C2517)
	arguments {
		Vehicle "vehicle",

		float "value",
	}
	returns	"void"

native "0xDF7E3EEB29642C38"
	hash "0xDF7E3EEB29642C38"
	arguments {
		Vehicle "vehicle",

		Any "p1",

		Any "p2",
	}
	returns	"void"

--[[!
<summary>
	Gets the height of the vehicle's suspension.
	The higher the value the lower the suspension.
	0 is the stock suspension.
	0.005 is Lowered Suspension.
</summary>
]]--
native "_GET_VEHICLE_SUSPENSION_HEIGHT"
	hash "0x53952FD2BAA19F17"
	jhash (0xB73A1486)
	arguments {
		Vehicle "vehicle",
	}
	returns	"float"

native "0x84FD40F56075E816"
	hash "0x84FD40F56075E816"
	arguments {
		float "p0",
	}
	returns	"void"

native "0xA7DCDF4DED40A8F4"
	hash "0xA7DCDF4DED40A8F4"
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "_GET_VEHICLE_BODY_HEALTH_2"
	hash "0xB8EF61207C2393A9"
	arguments {
		Any "p0",
	}
	returns	"float"

native "0xD4C4642CB7F50B5D"
	hash "0xD4C4642CB7F50B5D"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xC361AA040D6637A8"
	hash "0xC361AA040D6637A8"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x99C82F8A139F3E4E"
	hash "0x99C82F8A139F3E4E"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xE16142B94664DEFD"
	hash "0xE16142B94664DEFD"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "CREATE_OBJECT"
	hash "0x509D5878EB39E842"
	jhash (0x2F7AA05C)
	arguments {
		Hash "modelHash",

		float "x",

		float "y",

		float "z",

		BOOL "networkHandle",

		BOOL "createHandle",

		BOOL "dynamic",
	}
	returns	"Object"

native "CREATE_OBJECT_NO_OFFSET"
	hash "0x9A294B2138ABB884"
	jhash (0x58040420)
	arguments {
		Hash "objectHash",

		float "posX",

		float "posY",

		float "posZ",

		BOOL "p4",

		BOOL "p5",

		BOOL "p6",
	}
	returns	"Object"

--[[!
<summary>
	Delete current object
</summary>
]]--
native "DELETE_OBJECT"
	hash "0x539E0AE3E6634B9F"
	jhash (0xD6EF9DA7)
	arguments {
		ObjectPtr "object",
	}
	returns	"void"

native "PLACE_OBJECT_ON_GROUND_PROPERLY"
	hash "0x58A850EAEE20FAA3"
	jhash (0x8F95A20B)
	arguments {
		Object "object",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns true if the object has finished moving.

	If false, moves the object towards the specified X, Y and Z coordinates with the specified X, Y and Z speed.

	See also: http://gtag.gtagaming.com/opcode-database/opcode/034E/
</summary>
]]--
native "SLIDE_OBJECT"
	hash "0x2FDFF4107B8C1147"
	jhash (0x63BFA7A0)
	arguments {
		Object "object",

		float "toX",

		float "toY",

		float "toZ",

		float "speedX",

		float "speedY",

		float "speedZ",

		BOOL "collisionCheck",
	}
	returns	"BOOL"

native "SET_OBJECT_TARGETTABLE"
	hash "0x8A7391690F5AFD81"
	jhash (0x3F88CD86)
	arguments {
		Object "object",

		BOOL "targettable",
	}
	returns	"Any"

native "0x77F33F2CCF64B3AA"
	hash "0x77F33F2CCF64B3AA"
	jhash (0x483C5C88)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Has 8 params in the latest patches, previous declaration: Object GET_CLOSEST_OBJECT_OF_TYPE(float x, float y, float z, float radius, Hash modelHash, BOOL p5)
</summary>
]]--
native "GET_CLOSEST_OBJECT_OF_TYPE"
	hash "0xE143FA2249364369"
	jhash (0x45619B33)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		Hash "modelHash",

		Any "p5",

		Any "p6",

		Any "p7",
	}
	returns	"Object"

native "HAS_OBJECT_BEEN_BROKEN"
	hash "0x8ABFB70C49CC43E2"
	jhash (0xFE21F891)
	arguments {
		Object "object",
	}
	returns	"BOOL"

native "HAS_CLOSEST_OBJECT_OF_TYPE_BEEN_BROKEN"
	hash "0x761B0E69AC4D007E"
	jhash (0x6FC0353D)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		Any "p5",
	}
	returns	"BOOL"

native "0x46494A2475701343"
	hash "0x46494A2475701343"
	jhash (0x7DB578DD)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		Any "p5",
	}
	returns	"BOOL"

native "_GET_OBJECT_OFFSET_FROM_COORDS"
	hash "0x163E252DE035A133"
	jhash (0x87A42A12)
	arguments {
		float "x",

		float "y",

		float "z",

		float "heading",

		float "xOffset",

		float "yOffset",

		float "zOffset",
	}
	returns	"Vector3"

--[[!
<summary>
	only documented params
	dont know what this does.... To Be Continued... 
</summary>
]]--
native "0x163F8B586BC95F2A"
	hash "0x163F8B586BC95F2A"
	jhash (0x65213FC3)
	arguments {
		Any "coords",

		float "radius",

		Hash "modelHash",

		float "x",

		float "y",

		float "z",

		Vector3Ptr "p6",

		int "p7",
	}
	returns	"Any"

--[[!
<summary>
	Used to lock/unlock doors to interior areas of the game.

	(Possible) Door Types:

	http://pastebin.com/9S2m3qA4
</summary>
]]--
native "SET_STATE_OF_CLOSEST_DOOR_OF_TYPE"
	hash "0xF82D8F1926A02C3D"
	jhash (0x38C951A4)
	arguments {
		Hash "type",

		float "x",

		float "y",

		float "z",

		BOOL "locked",

		float "heading",

		BOOL "p6",
	}
	returns	"void"

--[[!
<summary>
	locked is 0 if no door is found
	locked is 0 if door is unlocked
	locked is 1 if door is found and unlocked.
</summary>
]]--
native "GET_STATE_OF_CLOSEST_DOOR_OF_TYPE"
	hash "0xEDC1A5B84AEF33FF"
	jhash (0x4B44A83D)
	arguments {
		Hash "type",

		float "x",

		float "y",

		float "z",

		BOOLPtr "locked",

		floatPtr "heading",
	}
	returns	"void"

--[[!
<summary>
	when you set locked to 0 the door open and to 1 the door close
	OBJECT::_9B12F9A24FABEDB0(${prop_gate_prison_01}, 1845.0, 2605.0, 45.0, 0, 0.0, 50.0, 0);  //door open

	OBJECT::_9B12F9A24FABEDB0(${prop_gate_prison_01}, 1845.0, 2605.0, 45.0, 1, 0.0, 50.0, 0);  //door close

	p5-7 - Axis?
</summary>
]]--
native "_DOOR_CONTROL"
	hash "0x9B12F9A24FABEDB0"
	jhash (0x4E0A260B)
	arguments {
		Hash "doorHash",

		float "x",

		float "y",

		float "z",

		BOOL "locked",

		float "p5",

		float "p6",

		float "p7",
	}
	returns	"void"

native "ADD_DOOR_TO_SYSTEM"
	hash "0x6F8838D03D1DC226"
	jhash (0x9D2D778D)
	arguments {
		Hash "doorHash",

		Hash "modelHash",

		float "x",

		float "y",

		float "z",

		BOOL "p5",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"void"

native "REMOVE_DOOR_FROM_SYSTEM"
	hash "0x464D8E1427156FE4"
	jhash (0x00253286)
	arguments {
		Hash "doorHash",
	}
	returns	"void"

native "0x6BAB9442830C7F53"
	hash "0x6BAB9442830C7F53"
	jhash (0xDF83DB47)
	arguments {
		Hash "doorHash",

		Any "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "0x160AA1B32F6139B8"
	hash "0x160AA1B32F6139B8"
	jhash (0xD42A41C2)
	arguments {
		Hash "doorHash",
	}
	returns	"Any"

native "0x4BC2854478F3A749"
	hash "0x4BC2854478F3A749"
	jhash (0xD649B7E1)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x03C27E13B42A0E82"
	hash "0x03C27E13B42A0E82"
	jhash (0x4F44AF21)
	arguments {
		Hash "doorHash",

		float "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "0x9BA001CB45CBF627"
	hash "0x9BA001CB45CBF627"
	jhash (0x47531446)
	arguments {
		Hash "doorHash",

		float "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "0xB6E6FBA95C7324AC"
	hash "0xB6E6FBA95C7324AC"
	jhash (0x34883DE3)
	arguments {
		Hash "doorHash",

		float "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "0x65499865FCA6E5EC"
	hash "0x65499865FCA6E5EC"
	jhash (0xB74C3BD7)
	arguments {
		Hash "doorHash",
	}
	returns	"float"

native "0xC485E07E4F0B7958"
	hash "0xC485E07E4F0B7958"
	jhash (0xB4A9A558)
	arguments {
		Hash "doorHash",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "0xD9B71952F78A2640"
	hash "0xD9B71952F78A2640"
	jhash (0xECE58AE0)
	arguments {
		Hash "doorHash",

		BOOL "p1",
	}
	returns	"void"

native "0xA85A21582451E951"
	hash "0xA85A21582451E951"
	jhash (0xF736227C)
	arguments {
		Hash "doorHash",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Does door exist in system?
</summary>
]]--
native "0xC153C43EA202C8C1"
	hash "0xC153C43EA202C8C1"
	jhash (0x5AFCD8A1)
	arguments {
		Hash "doorHash",
	}
	returns	"BOOL"

native "IS_DOOR_CLOSED"
	hash "0xC531EE8A1145A149"
	jhash (0x48659CD7)
	arguments {
		Hash "door",
	}
	returns	"BOOL"

native "0xC7F29CA00F46350E"
	hash "0xC7F29CA00F46350E"
	jhash (0x9BF33E41)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x701FDA1E82076BA4"
	hash "0x701FDA1E82076BA4"
	jhash (0xF592AD10)
	returns	"void"

native "0xDF97CDD4FC08FD34"
	hash "0xDF97CDD4FC08FD34"
	jhash (0x17FF9393)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x589F80B325CC82C5"
	hash "0x589F80B325CC82C5"
	jhash (0xE9AE494F)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		Any "p3",

		AnyPtr "p4",
	}
	returns	"BOOL"

native "IS_GARAGE_EMPTY"
	hash "0x90E47239EA1980B8"
	jhash (0xA8B37DEA)
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0x024A60DEB0EA69F0"
	hash "0x024A60DEB0EA69F0"
	jhash (0xC33ED360)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0x1761DC5D8471CBAA"
	hash "0x1761DC5D8471CBAA"
	jhash (0x41924877)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0x85B6C850546FDDE2"
	hash "0x85B6C850546FDDE2"
	jhash (0x4BD59750)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",

		Any "p4",
	}
	returns	"BOOL"

native "0x673ED815D6E323B7"
	hash "0x673ED815D6E323B7"
	jhash (0x7B44D659)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",

		Any "p4",
	}
	returns	"BOOL"

native "0x372EF6699146A1E4"
	hash "0x372EF6699146A1E4"
	jhash (0x142C8F76)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0xF0EED5A6BC7B237A"
	hash "0xF0EED5A6BC7B237A"
	jhash (0x95A9AB2B)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0x190428512B240692"
	hash "0x190428512B240692"
	jhash (0xA565E27E)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"void"

native "0xF2E1A7133DD356A6"
	hash "0xF2E1A7133DD356A6"
	jhash (0x43BB7E48)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x66A49D021870FE88"
	hash "0x66A49D021870FE88"
	jhash (0x6158959E)
	returns	"void"

--[[!
<summary>
	p5 is usually 0.
</summary>
]]--
native "DOES_OBJECT_OF_TYPE_EXIST_AT_COORDS"
	hash "0xBFA48E2FF417213F"
	jhash (0x23FF2BA4)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		Hash "hash",

		BOOL "p5",
	}
	returns	"BOOL"

native "IS_POINT_IN_ANGLED_AREA"
	hash "0x2A70BAE8883E4C81"
	jhash (0x73BCFFDC)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		BOOL "p10",

		BOOL "p11",
	}
	returns	"BOOL"

native "0x4D89D607CB3DD1D2"
	hash "0x4D89D607CB3DD1D2"
	jhash (0x19B17769)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_OBJECT_PHYSICS_PARAMS"
	hash "0xF6DF6E90DE7DF90F"
	jhash (0xE8D11C58)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",

		float "p11",
	}
	returns	"void"

native "GET_OBJECT_FRAGMENT_DAMAGE_HEALTH"
	hash "0xB6FBFD079B8D0596"
	jhash (0xF0B330AD)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"float"

native "SET_ACTIVATE_OBJECT_PHYSICS_AS_SOON_AS_IT_IS_UNFROZEN"
	hash "0x406137F8EF90EAF5"
	jhash (0x3E263AE1)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_ANY_OBJECT_NEAR_POINT"
	hash "0x397DC58FF00298D1"
	jhash (0xE9E46941)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",
	}
	returns	"BOOL"

--[[!
<summary>
	p0 
	p1 
	p2 
	p3 
	P4 
</summary>
]]--
native "IS_OBJECT_NEAR_POINT"
	hash "0x8C90FE4B381BA60A"
	jhash (0x50A62C43)
	arguments {
		Hash "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"BOOL"

native "0x4A39DB43E47CF3AA"
	hash "0x4A39DB43E47CF3AA"
	jhash (0xE3261B35)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xE7E4C198B0185900"
	hash "0xE7E4C198B0185900"
	jhash (0x1E82C2AE)
	arguments {
		Object "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xF9C1681347C8BD15"
	hash "0xF9C1681347C8BD15"
	arguments {
		Object "object",
	}
	returns	"void"

native "TRACK_OBJECT_VISIBILITY"
	hash "0xB252BC036B525623"
	jhash (0x46D06B9A)
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_OBJECT_VISIBLE"
	hash "0x8B32ACE6326A7546"
	jhash (0xF4FD8AE4)
	arguments {
		Object "object",
	}
	returns	"BOOL"

native "0xC6033D32241F6FB5"
	hash "0xC6033D32241F6FB5"
	jhash (0xF4A1A14A)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xEB6F1A9B5510A5D2"
	hash "0xEB6F1A9B5510A5D2"
	jhash (0xAF016CC1)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xBCE595371A5FBAAF"
	hash "0xBCE595371A5FBAAF"
	jhash (0x3A68AA46)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xB48FCED898292E52"
	hash "0xB48FCED898292E52"
	jhash (0xA286DE96)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		AnyPtr "p4",
	}
	returns	"Any"

native "0x5C29F698D404C5E1"
	hash "0x5C29F698D404C5E1"
	jhash (0x21F51560)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x899BA936634A322E"
	hash "0x899BA936634A322E"
	jhash (0xF1B8817A)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x52AF537A0C5B8AAD"
	hash "0x52AF537A0C5B8AAD"
	jhash (0xE08C834D)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x260EE4FDBDF4DB01"
	hash "0x260EE4FDBDF4DB01"
	jhash (0x020497DE)
	arguments {
		Any "p0",
	}
	returns	"float"

--[[!
<summary>
	'modelHash' is only used if you use a custom object.

	List of pickupHash - http://ecb2.biz/releases/GTAV/lists/pickups.txt
</summary>
]]--
native "CREATE_PICKUP"
	hash "0xFBA08C503DD5FA58"
	jhash (0x5E14DF68)
	arguments {
		Hash "typeHash",

		float "posX",

		float "posY",

		float "posZ",

		int "p4",

		int "value",

		BOOL "p6",

		Hash "customModelHash",
	}
	returns	"Any"

--[[!
<summary>
	'modelHash' is only used if you use a custom object.

	List of pickupHash - http://ecb2.biz/releases/GTAV/lists/pickups.txt
</summary>
]]--
native "CREATE_PICKUP_ROTATE"
	hash "0x891804727E0A98B7"
	jhash (0xF015BFE2)
	arguments {
		Hash "typeHash",

		float "posX",

		float "posY",

		float "posZ",

		float "rotX",

		float "rotY",

		float "rotZ",

		int "p7",

		int "amount",

		Any "p9",

		BOOL "p10",

		Hash "customModelHash",
	}
	returns	"Any"

--[[!
<summary>
	'modelHash' is only used if you use a custom object.

	List of pickupHash - http://ecb2.biz/releases/GTAV/lists/pickups.txt
</summary>
]]--
native "CREATE_AMBIENT_PICKUP"
	hash "0x673966A0C0FD7171"
	jhash (0x17B99CE7)
	arguments {
		Hash "pickupHash",

		float "posX",

		float "posY",

		float "posZ",

		int "p4",

		int "value",

		Hash "modelHash",

		BOOL "p7",

		BOOL "p8",
	}
	returns	"Any"

--[[!
<summary>

	List of pickupHash - http://ecb2.biz/releases/GTAV/lists/pickups.txt
</summary>
]]--
native "CREATE_PORTABLE_PICKUP"
	hash "0x2EAF1FDB2FB55698"
	jhash (0x8C886BE5)
	arguments {
		Hash "pickupHash",

		float "x",

		float "y",

		float "z",

		BOOL "p4",

		Any "p5",
	}
	returns	"Any"

--[[!
<summary>
	p4 is always 1
	Toggle ?

	Please follow the camel case convention when naming parameters.
</summary>
]]--
native "0x125494B98A21AAF7"
	hash "0x125494B98A21AAF7"
	jhash (0x56A02502)
	arguments {
		Hash "objectModel",

		float "x",

		float "y",

		float "z",

		BOOL "p4",

		Hash "objectModel2",
	}
	returns	"Object"

native "ATTACH_PORTABLE_PICKUP_TO_PED"
	hash "0x8DC39368BDD57755"
	jhash (0x184F6AB3)
	arguments {
		Ped "ped",

		Any "p1",
	}
	returns	"void"

native "DETACH_PORTABLE_PICKUP_FROM_PED"
	hash "0xCF463D1E9A0AECB1"
	jhash (0x1D094562)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "0x0BF3B3BD47D79C08"
	hash "0x0BF3B3BD47D79C08"
	jhash (0x7EFBA039)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x78857FC65CADB909"
	hash "0x78857FC65CADB909"
	jhash (0xA3CDF152)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "GET_SAFE_PICKUP_COORDS"
	hash "0x6E16BC2503FF1FF0"
	jhash (0x618B5F67)
	arguments {
		float "x",

		float "y",

		float "z",

		Any "p3",

		Any "p4",
	}
	returns	"Vector3"

native "GET_PICKUP_COORDS"
	hash "0x225B8B35C88029B3"
	jhash (0xC2E1E2C5)
	arguments {
		Any "p0",
	}
	returns	"int"

native "REMOVE_ALL_PICKUPS_OF_TYPE"
	hash "0x27F9D613092159CF"
	jhash (0x40062C53)
	arguments {
		Any "p0",
	}
	returns	"void"

native "HAS_PICKUP_BEEN_COLLECTED"
	hash "0x80EC48E6679313F9"
	jhash (0x0BE5CCED)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "REMOVE_PICKUP"
	hash "0x3288D8ACAECD2AB2"
	jhash (0x64A7A0E0)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Changed p4 to. int. As the 3 times it is used in X360 scripts it's mostly 5 with one 1 used. p5 seems to stay 0. So may be a bool, but without guarantee I left it as Any.
</summary>
]]--
native "CREATE_MONEY_PICKUPS"
	hash "0x0589B5E791CE9B2B"
	jhash (0x36C9A5EA)
	arguments {
		float "xCoord",

		float "yCoord",

		float "zCoord",

		int "value",

		int "p4",

		Any "p5",
	}
	returns	"void"

native "DOES_PICKUP_EXIST"
	hash "0xAFC1CA75AD4074D1"
	jhash (0x9C6DA0B3)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "DOES_PICKUP_OBJECT_EXIST"
	hash "0xD9EFB6DBF7DAAEA3"
	jhash (0xE0B32108)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x5099BC55630B25AE"
	hash "0x5099BC55630B25AE"
	jhash (0x6052E62E)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x0378C08504160D0D"
	hash "0x0378C08504160D0D"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "_IS_PICKUP_WITHIN_RADIUS"
	hash "0xF9C36251F6E48E33"
	jhash (0xF139681B)
	arguments {
		Hash "pickupHash",

		float "x",

		float "y",

		float "z",

		float "radius",
	}
	returns	"BOOL"

native "SET_PICKUP_REGENERATION_TIME"
	hash "0x78015C9B4B3ECC9D"
	jhash (0xAB11267D)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x616093EC6B139DD9"
	hash "0x616093EC6B139DD9"
	jhash (0x7FADB4B9)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x88EAEC617CD26926"
	hash "0x88EAEC617CD26926"
	jhash (0x3A8F1BF7)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_TEAM_PICKUP_OBJECT"
	hash "0x53E0DF1A2A3CF0CA"
	jhash (0x77687DC5)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x92AEFB5F6E294023"
	hash "0x92AEFB5F6E294023"
	jhash (0xCBB5F9B6)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xA08FE5E49BDC39DD"
	hash "0xA08FE5E49BDC39DD"
	jhash (0x276A7807)
	arguments {
		Any "p0",

		float "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xDB41D07A45A6D4B7"
	hash "0xDB41D07A45A6D4B7"
	jhash (0x000E92DC)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x318516E02DE3ECE2"
	hash "0x318516E02DE3ECE2"
	jhash (0x9879AC51)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x31F924B53EADDF65"
	hash "0x31F924B53EADDF65"
	jhash (0xDB18FA01)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xF92099527DB8E2A7"
	hash "0xF92099527DB8E2A7"
	jhash (0xA7E936FD)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xA2C1F5E92AFE49ED"
	hash "0xA2C1F5E92AFE49ED"
	jhash (0xB241806C)
	returns	"void"

native "0x762DB2D380B48D04"
	hash "0x762DB2D380B48D04"
	jhash (0xD1BAAFB7)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	p3 is always 0 in scripts.
	draws red circular marker at pos
</summary>
]]--
native "0x3430676B11CDF21D"
	hash "0x3430676B11CDF21D"
	jhash (0x63B02FAD)
	arguments {
		float "x",

		float "y",

		float "z",

		Any "p3",
	}
	returns	"void"

native "0xB2D0BDE54F0E8E5A"
	hash "0xB2D0BDE54F0E8E5A"
	jhash (0x132B6D92)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	only documented to be continued...
	seems to return the hash from pickup
</summary>
]]--
native "0x08F96CA6C551AD51"
	hash "0x08F96CA6C551AD51"
	jhash (0xEDD01937)
	arguments {
		Any "p0",
	}
	returns	"Hash"

native "0x11D1E53A726891FE"
	hash "0x11D1E53A726891FE"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x971DA0055324D033"
	hash "0x971DA0055324D033"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x5EAAD83F8CFB4575"
	hash "0x5EAAD83F8CFB4575"
	jhash (0x6AE36192)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "SET_FORCE_OBJECT_THIS_FRAME"
	hash "0xF538081986E49E9D"
	jhash (0x3DA41C1A)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "_MARK_OBJECT_FOR_DELETION"
	hash "0xADBE4809F19F927A"
	jhash (0x2048A7DD)
	arguments {
		Object "object",
	}
	returns	"void"

native "TASK_PAUSE"
	hash "0xE73A266DB0CA9042"
	jhash (0x17A64668)
	arguments {
		Ped "ped",

		int "ms",
	}
	returns	"void"

--[[!
<summary>
	Make the specified ped stand still for `time` amount of (milli?)seconds.
</summary>
]]--
native "TASK_STAND_STILL"
	hash "0x919BE13EED931959"
	jhash (0x6F80965D)
	arguments {
		Ped "ped",

		int "time",
	}
	returns	"void"

native "TASK_JUMP"
	hash "0x0AE4086104E067B1"
	jhash (0x0356E3CE)
	arguments {
		Ped "ped",

		BOOL "p1",
	}
	returns	"void"

native "TASK_COWER"
	hash "0x3EB1FE9E8E908E15"
	jhash (0x9CF1C19B)
	arguments {
		Ped "ped",

		int "duration",
	}
	returns	"void"

native "TASK_HANDS_UP"
	hash "0xF2EAB31979A7F910"
	jhash (0x8DCC19C5)
	arguments {
		Ped "ped",

		int "duration",

		Ped "facingPed",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "UPDATE_TASK_HANDS_UP_DURATION"
	hash "0xA98FCAFD7893C834"
	jhash (0x3AA39BE9)
	arguments {
		Ped "ped",

		int "duration",
	}
	returns	"void"

native "TASK_OPEN_VEHICLE_DOOR"
	hash "0x965791A9A488A062"
	jhash (0x8EE06BF4)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		int "timeOut",

		int "doorIndex",

		float "speed",
	}
	returns	"void"

--[[!
<summary>
	speed 1.0 = walk, 2.0 = run
	p5 1 = normal, 3 = teleport to vehicle, 16 = teleport directly into vehicle
	p6 is always 0

	-kb


	Usage of seat 

	-1 = driver
	0 = passenger
	1 = left back seat
	2 = right back seat
	3 = outside left
	4 = outside right
</summary>
]]--
native "TASK_ENTER_VEHICLE"
	hash "0xC20E50AA46D09CA8"
	jhash (0xB8689B4E)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		int "timeout",

		int "seat",

		float "speed",

		int "p5",

		AnyPtr "p6",
	}
	returns	"void"

native "TASK_LEAVE_VEHICLE"
	hash "0xD3DBCE61A490BE02"
	jhash (0x7B1141C6)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		int "flags",
	}
	returns	"void"

--[[!
<summary>
	MulleDK19: Jenkins of this native is 0x4293601f. This is the actual name.
</summary>
]]--
native "_TASK_GET_OFF_BOAT"
	hash "0x9C00E77AF14B2DFF"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "TASK_SKY_DIVE"
	hash "0x601736CFE536B0A0"
	jhash (0xD3874AFA)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "TASK_PARACHUTE"
	hash "0xD2F1C53C97EE81AB"
	jhash (0xEC3060A2)
	arguments {
		Ped "ped",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	makes ped parachute to coords x y z. Works well with PATHFIND::GET_SAFE_COORD_FOR_PED
</summary>
]]--
native "TASK_PARACHUTE_TO_TARGET"
	hash "0xB33E291AFA6BD03A"
	jhash (0xE0104D6C)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "SET_PARACHUTE_TASK_TARGET"
	hash "0xC313379AF0FCEDA7"
	jhash (0x6ED3AD81)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "SET_PARACHUTE_TASK_THRUST"
	hash "0x0729BAC1B8C64317"
	jhash (0xD07C8AAA)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "TASK_RAPPEL_FROM_HELI"
	hash "0x09693B0312F91649"
	jhash (0x2C7ADB93)
	arguments {
		Ped "ped",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	drivingMode = 0xC00AB - driver abides by road rules.
	drivingMode = 0x40000 - driver drives recklessly.   // gtaVmod
	More info about driving modes: http://gtaforums.com/topic/822314-guide-driving-styles/
</summary>
]]--
native "TASK_VEHICLE_DRIVE_TO_COORD"
	hash "0xE2A2AA2F659D77A7"
	jhash (0xE4AC0387)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		float "x",

		float "y",

		float "z",

		float "speed",

		Any "p6",

		Hash "vehicleModel",

		int "drivingMode",

		float "stopRange",

		float "p10",
	}
	returns	"void"

native "TASK_VEHICLE_DRIVE_TO_COORD_LONGRANGE"
	hash "0x158BB33F920D360C"
	jhash (0x1490182A)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		float "x",

		float "y",

		float "z",

		float "speed",

		int "driveMode",

		float "stopRange",
	}
	returns	"void"

native "TASK_VEHICLE_DRIVE_WANDER"
	hash "0x480142959D337D00"
	jhash (0x36EC0EB0)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		float "speed",

		int "drivingStyle",
	}
	returns	"void"

--[[!
<summary>
	p6 always -1
	p7 always 10.0
	p8 always 1
</summary>
]]--
native "TASK_FOLLOW_TO_OFFSET_OF_ENTITY"
	hash "0x304AE42E357B8C7E"
	jhash (0x2DF5A6AC)
	arguments {
		Ped "ped",

		Entity "entity",

		float "offsetX",

		float "offsetY",

		float "offsetZ",

		float "movementSpeed",

		int "p6",

		float "stoppingRange",

		BOOL "p8",
	}
	returns	"void"

native "TASK_GO_STRAIGHT_TO_COORD"
	hash "0xD76B57B44F1E6F8B"
	jhash (0x80A9E7A7)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		float "speed",

		int "timeout",

		float "targetHeading",

		float "distanceToSlide",
	}
	returns	"void"

native "TASK_GO_STRAIGHT_TO_COORD_RELATIVE_TO_ENTITY"
	hash "0x61E360B7E040D12E"
	jhash (0xD26CAC68)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		Any "p6",
	}
	returns	"void"

--[[!
<summary>
	Makes the specified ped achieve the specified heading.

	pedHandle: The handle of the ped to assign the task to.
	heading: The desired heading.
	timeout: The time, in milliseconds, to allow the task to complete. If the task times out, it is cancelled, and the ped will stay at the heading it managed to reach in the time.
</summary>
]]--
native "TASK_ACHIEVE_HEADING"
	hash "0x93B93A37987F1F3D"
	jhash (0x0A0E9B42)
	arguments {
		Ped "ped",

		float "heading",

		int "timeout",
	}
	returns	"void"

native "TASK_FLUSH_ROUTE"
	hash "0x841142A1376E9006"
	jhash (0x34219154)
	returns	"void"

native "TASK_EXTEND_ROUTE"
	hash "0x1E7889778264843A"
	jhash (0x43271F69)
	arguments {
		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "TASK_FOLLOW_POINT_ROUTE"
	hash "0x595583281858626E"
	jhash (0xB837C816)
	arguments {
		Any "p0",

		float "p1",

		Any "p2",
	}
	returns	"void"

--[[!
<summary>
	The entity will move towards the target until time is over (duration) or get in target's range (distance). p5 and p6 are unknown, but you could leave p5 = 1073741824 or 100 or even 0 (didn't see any difference but on the decompiled scripts, they use 1073741824 mostly) and p6 = 0

	Note: I've only tested it on entity -&gt; ped and target -&gt; vehicle. It could work differently on other entities, didn't try it yet.

	Example: AI::TASK_GO_TO_ENTITY(pedHandle, vehicleHandle, 5000, 4.0, 100, 1073741824, 0)

	Ped will run towards the vehicle for 5 seconds and stop when time is over or when he gets 4 meters(?) around the vehicle (with duration = -1, the task duration will be ignored).


</summary>
]]--
native "TASK_GO_TO_ENTITY"
	hash "0x6A071245EB0D1882"
	jhash (0x374827C2)
	arguments {
		Entity "entity",

		Entity "target",

		int "duration",

		float "distance",

		float "speed",

		float "p5",

		int "p6",
	}
	returns	"void"

native "TASK_SMART_FLEE_COORD"
	hash "0x94587F17E9C365D5"
	jhash (0xB2E686FC)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"void"

--[[!
<summary>
	Makes a ped run away from another ped (fleeTarget).

	distance = ped will start running away if distance is less than this value
	fleeTime = ped will flee for this amount of time, set to "-1" to flee forever
</summary>
]]--
native "TASK_SMART_FLEE_PED"
	hash "0x22B0D0E37CCB840D"
	jhash (0xE52EB560)
	arguments {
		Ped "ped",

		Ped "fleeTarget",

		float "distance",

		Any "fleeTime",

		BOOL "p4",

		BOOL "p5",
	}
	returns	"void"

native "TASK_REACT_AND_FLEE_PED"
	hash "0x72C896464915D1B1"
	jhash (0x8A632BD8)
	arguments {
		Ped "ped",

		Ped "fleeTarget",
	}
	returns	"void"

native "TASK_SHOCKING_EVENT_REACT"
	hash "0x452419CBD838065B"
	jhash (0x9BD00ACF)
	arguments {
		Ped "ped",

		Any "event",
	}
	returns	"void"

native "TASK_WANDER_IN_AREA"
	hash "0xE054346CA3A0F315"
	jhash (0xC6981FB9)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		float "radius",

		float "minimalLenght",

		float "timeBetweenWalks",
	}
	returns	"void"

--[[!
<summary>
	Makes PED walk around the area.
</summary>
]]--
native "TASK_WANDER_STANDARD"
	hash "0xBB9CE077274F6A1B"
	jhash (0xAF59151A)
	arguments {
		Ped "ped",

		float "p1",

		int "p2",
	}
	returns	"void"

--[[!
<summary>
	Modes:
	0 - ignore heading
	1 - park forward
	2 - park backwards

	Depending on the angle of approach, the vehicle can park at the specified heading or at its exact opposite (-180) angle.

	Radius seems to define how close the vehicle has to be -after parking- to the position for this task considered completed. If the value is too small, the vehicle will try to park again until it's exactly where it should be. 20.0 Works well but lower values don't, like the radius is measured in centimeters or something.
</summary>
]]--
native "TASK_VEHICLE_PARK"
	hash "0x0F3E34E968EA374E"
	jhash (0x5C85FF90)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		float "x",

		float "y",

		float "z",

		float "heading",

		int "mode",

		float "radius",

		BOOL "keepEngineOn",
	}
	returns	"void"

--[[!
<summary>
	known "killTypes" are: "AR_stealth_kill_knife" and "AR_stealth_kill_a".
</summary>
]]--
native "TASK_STEALTH_KILL"
	hash "0xAA5DC05579D60BD9"
	jhash (0x0D64C2FA)
	arguments {
		Ped "killer",

		Ped "target",

		Hash "killType",

		float "p3",

		BOOL "p4",
	}
	returns	"Any"

--[[!
<summary>
	p0 should be 0?
</summary>
]]--
native "TASK_PLANT_BOMB"
	hash "0x965FEC691D55E9BF"
	jhash (0x33457535)
	arguments {
		int "p0",

		float "x",

		float "y",

		float "z",

		float "degreeAngle",
	}
	returns	"Any"

--[[!
<summary>
	The last three should all be 0.
	If no timeout, set timeout to -1.
	All parameters are taken from the ScriptHookV Dot Net wrapper.
</summary>
]]--
native "TASK_FOLLOW_NAV_MESH_TO_COORD"
	hash "0x15D3A79D4E44B913"
	jhash (0xFE4A10D9)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		float "speed",

		int "timeout",

		float "zeroFloat",

		int "zeroInt",

		float "zeroFloat2",
	}
	returns	"void"

native "TASK_FOLLOW_NAV_MESH_TO_COORD_ADVANCED"
	hash "0x17F58B88D085DBAC"
	jhash (0x6BF6E296)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",

		float "p6",

		Any "p7",

		float "p8",

		float "p9",

		float "p10",

		float "p11",
	}
	returns	"void"

native "SET_PED_PATH_CAN_USE_CLIMBOVERS"
	hash "0x8E06A6FE76C9EFF4"
	jhash (0xB7B7D442)
	arguments {
		Ped "ped",

		BOOL "Toggle",
	}
	returns	"Any"

native "SET_PED_PATH_CAN_USE_LADDERS"
	hash "0x77A5B103C87F476E"
	jhash (0x53A879EE)
	arguments {
		Ped "ped",

		BOOL "Toggle",
	}
	returns	"Any"

native "SET_PED_PATH_CAN_DROP_FROM_HEIGHT"
	hash "0xE361C5C71C431A4F"
	jhash (0x394B7AC9)
	arguments {
		int "ped",

		BOOL "Toggle",
	}
	returns	"Any"

native "0x88E32DB8C1A4AA4B"
	hash "0x88E32DB8C1A4AA4B"
	jhash (0x55E06443)
	arguments {
		Ped "p0",

		float "p1",
	}
	returns	"Any"

native "SET_PED_PATHS_WIDTH_PLANT"
	hash "0xF35425A4204367EC"
	jhash (0x9C606EE3)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_PED_PATH_PREFER_TO_AVOID_WATER"
	hash "0x38FE1EC73743793C"
	jhash (0x0EA39A29)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "SET_PED_PATH_AVOID_FIRE"
	hash "0x4455517B28441E60"
	jhash (0xDCC5B934)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_GLOBAL_MIN_BIRD_FLIGHT_HEIGHT"
	hash "0x6C6B148586F934F7"
	jhash (0x2AFB14B8)
	arguments {
		float "height",
	}
	returns	"void"

native "GET_NAVMESH_ROUTE_DISTANCE_REMAINING"
	hash "0xC6F5C0BCDC74D62D"
	jhash (0xD9281778)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"Any"

native "GET_NAVMESH_ROUTE_RESULT"
	hash "0x632E831F382A0FA8"
	jhash (0x96491602)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x3E38E28A1D80DDF6"
	hash "0x3E38E28A1D80DDF6"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "TASK_GO_TO_COORD_ANY_MEANS"
	hash "0x5BC448CB78FA3E88"
	jhash (0xF91DF93B)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",

		BOOL "p6",

		Any "p7",

		float "p8",
	}
	returns	"void"

native "TASK_GO_TO_COORD_ANY_MEANS_EXTRA_PARAMS"
	hash "0x1DD45F9ECFDB1BC9"
	jhash (0x094B75EF)
	arguments {
		Ped "p0",

		float "x",

		float "y",

		float "z",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",

		Any "p9",

		Any "p10",

		Any "p11",
	}
	returns	"void"

native "TASK_GO_TO_COORD_ANY_MEANS_EXTRA_PARAMS_WITH_CRUISE_SPEED"
	hash "0xB8ECD61F531A7B02"
	jhash (0x86DC03F9)
	arguments {
		Ped "ped",

		Vector3Ptr "position",

		Any "p2",

		Any "p3",

		Any "p4",

		int "drivingStyle",

		Any "p6",

		Any "p7",

		Any "p8",

		Any "p9",

		Any "p10",

		Any "p11",

		Any "p12",
	}
	returns	"void"

--[[!
<summary>
	float speed &gt; normal speed is 8.0f
	----------------------

	float speedMultiplier &gt; multiply the playback speed
	----------------------

	int duration: time in millisecond
	----------------------
	-1 _ _ _ _ _ _ _&gt; Default (see flag)
	0 _ _ _ _ _ _ _ &gt; Not play at all
	Small value _ _ &gt; Slow down animation speed
	Other _ _ _ _ _ &gt; freeze player control until specific time (ms) has 
	_ _ _ _ _ _ _ _ _ passed. (No effect if flag is set to be 
	_ _ _ _ _ _ _ _ _ controllable.)

	int flag:
	----------------------
	Odd number : loop infinitely
	Even number : Freeze at last frame
	Multiple of 4: Freeze at last frame but controllable

	01 to 15 &gt; Full body
	10 to 31 &gt; Upper body
	32 to 47 &gt; Full body &gt; Controllable
	48 to 63 &gt; Upper body &gt; Controllable
	...

	001 to 255 &gt; Normal
	256 to 511 &gt; Garbled
	...
</summary>
]]--
native "TASK_PLAY_ANIM"
	hash "0xEA47FE3719165B94"
	jhash (0x5AB552C6)
	arguments {
		Ped "ped",

		charPtr "animDictionary",

		charPtr "animationName",

		float "speed",

		float "speedMultiplier",

		int "duration",

		int "flag",

		float "playbackRate",

		BOOL "lockX",

		BOOL "lockY",

		BOOL "lockZ",
	}
	returns	"void"

--[[!
<summary>
	It's similar to the one above, except the first 6 floats let you specify the initial position and rotation of the task. (Ped gets teleported to the position). animTime is a float from 0.0 -&gt; 1.0, lets you start an animation from given point. The rest as in AI::TASK_PLAY_ANIM. 
</summary>
]]--
native "TASK_PLAY_ANIM_ADVANCED"
	hash "0x83CDB10EA29B370B"
	jhash (0x3DDEB0E6)
	arguments {
		Any "p0",

		charPtr "animDict",

		charPtr "animName",

		float "posX",

		float "posY",

		float "posZ",

		float "rotX",

		float "rotY",

		float "rotZ",

		float "speed",

		float "speedMultiplier",

		int "duration",

		Any "flag",

		float "animTime",

		Any "p14",

		Any "p15",
	}
	returns	"void"

native "STOP_ANIM_TASK"
	hash "0x97FF36A1D40EA00A"
	jhash (0x2B520A57)
	arguments {
		Ped "ped",

		charPtr "animDictionary",

		charPtr "animationName",

		float "p3",
	}
	returns	"void"

native "TASK_SCRIPTED_ANIMATION"
	hash "0x126EF75F1E17ABE5"
	jhash (0xFC2DCF47)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",

		float "p4",

		float "p5",
	}
	returns	"void"

native "PLAY_ENTITY_SCRIPTED_ANIM"
	hash "0x77A1EEC547E7FCF1"
	jhash (0x02F72AE5)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",

		float "p4",

		float "p5",
	}
	returns	"void"

native "STOP_ANIM_PLAYBACK"
	hash "0xEE08C992D238C5D1"
	jhash (0xE5F16398)
	arguments {
		Ped "ped",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "SET_ANIM_WEIGHT"
	hash "0x207F1A47C0342F48"
	jhash (0x17229D98)
	arguments {
		Any "p0",

		float "p1",

		Any "p2",

		Any "p3",

		BOOL "p4",
	}
	returns	"void"

native "SET_ANIM_RATE"
	hash "0x032D49C5E359C847"
	jhash (0x6DB46584)
	arguments {
		Any "p0",

		float "p1",

		Any "p2",

		BOOL "p3",
	}
	returns	"void"

native "SET_ANIM_LOOPED"
	hash "0x70033C3CC29A1FF4"
	jhash (0x095D61A4)
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",

		BOOL "p3",
	}
	returns	"void"

native "TASK_PLAY_PHONE_GESTURE_ANIMATION"
	hash "0x8FBB6758B3B3E9EC"
	jhash (0x1582162C)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",

		float "p4",

		float "p5",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"void"

--[[!
<summary>
	TODO: add hash from x360
</summary>
]]--
native "_TASK_STOP_PHONE_GESTURE_ANIMATION"
	hash "0x3FA00D4F4641BFAE"
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_PLAYING_PHONE_GESTURE_ANIM"
	hash "0xB8EBB1E9D3588C10"
	jhash (0x500B6805)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "GET_PHONE_GESTURE_ANIM_CURRENT_TIME"
	hash "0x47619ABE8B268C60"
	jhash (0x7B72AFD1)
	arguments {
		Any "p0",
	}
	returns	"float"

native "GET_PHONE_GESTURE_ANIM_TOTAL_TIME"
	hash "0x1EE0F68A7C25DEC6"
	jhash (0xEF8C3959)
	arguments {
		Any "p0",
	}
	returns	"float"

--[[!
<summary>
	Most probably plays a specific animation on vehicle. For example getting chop out of van etc...

	Here's how its used - 

	AI::TASK_VEHICLE_PLAY_ANIM(l_556[0/*1*/], "missfra0_chop_drhome", "InCar_GetOutofBack_Speedo");

	FYI : Speedo is the name of van in which chop was put in the mission.
</summary>
]]--
native "TASK_VEHICLE_PLAY_ANIM"
	hash "0x69F5C3BD0F3EBD89"
	jhash (0x2B28F598)
	arguments {
		Vehicle "vehicle",

		charPtr "animation_set",

		charPtr "animation_name",
	}
	returns	"void"

--[[!
<summary>
	p5 = 0, p6 = 2
</summary>
]]--
native "TASK_LOOK_AT_COORD"
	hash "0x6FA46612594F7973"
	jhash (0x7B784DD8)
	arguments {
		Entity "entity",

		float "x",

		float "y",

		float "z",

		float "duration",

		Any "p5",

		Any "p6",
	}
	returns	"void"

--[[!
<summary>
	param3: duration in ms, use -1 to look forever
	param4: using 2048 is fine
	param5: using 3 is fine
</summary>
]]--
native "TASK_LOOK_AT_ENTITY"
	hash "0x69F4BE8C8CC4796C"
	jhash (0x991D6619)
	arguments {
		Ped "pedHandle",

		Entity "lookAt",

		int "duration",

		int "unknown1",

		int "unknown2",
	}
	returns	"Any"

--[[!
<summary>
	Not clear what it actually does, but here's how script uses it - 

	if (OBJECT::HAS_PICKUP_BEEN_COLLECTED(...) 
	{
		if(ENTITY::DOES_ENTITY_EXIST(PLAYER::PLAYER_PED_ID()))
		{
			AI::TASK_CLEAR_LOOK_AT(PLAYER::PLAYER_PED_ID());
		}
		...
	}

	Another one where it doesn't "look" at current player - 

	AI::TASK_PLAY_ANIM(l_3ED, "missheist_agency2aig_2", "look_at_phone_a", 1000.0, -2.0, -1, 48, v_2, 0, 0, 0);
	PED::_2208438012482A1A(l_3ED, 0, 0);
	AI::TASK_CLEAR_LOOK_AT(l_3ED);
</summary>
]]--
native "TASK_CLEAR_LOOK_AT"
	hash "0x0F804F1DB19B9689"
	jhash (0x60EB4054)
	arguments {
		Ped "playerPed",
	}
	returns	"void"

native "OPEN_SEQUENCE_TASK"
	hash "0xE8854A4326B9E12B"
	jhash (0xABA6923E)
	arguments {
		ObjectPtr "taskSequence",
	}
	returns	"Any"

native "CLOSE_SEQUENCE_TASK"
	hash "0x39E72BC99E6360CB"
	jhash (0x1A7CEBD0)
	arguments {
		Object "taskSequence",
	}
	returns	"Any"

native "TASK_PERFORM_SEQUENCE"
	hash "0x5ABA3986D90D8A3B"
	jhash (0x4D9FBD11)
	arguments {
		Ped "ped",

		Object "taskSequence",
	}
	returns	"Any"

native "CLEAR_SEQUENCE_TASK"
	hash "0x3841422E9C488D8C"
	jhash (0x47ED03CE)
	arguments {
		ObjectPtr "taskSequence",
	}
	returns	"Any"

native "SET_SEQUENCE_TO_REPEAT"
	hash "0x58C70CF3A41E4AE7"
	jhash (0xCDDF1508)
	arguments {
		Object "taskSequence",

		BOOL "repeat",
	}
	returns	"void"

--[[!
<summary>
	returned values:
	0 to 7 = task that's currently in progress, 0 meaning the first one.
</summary>
]]--
native "GET_SEQUENCE_PROGRESS"
	hash "0x00A9010CFE1E3533"
	jhash (0xA3419909)
	arguments {
		Ped "ped",
	}
	returns	"int"

--[[!
<summary>
	from docks_heistb.c4:

	AI::GET_IS_TASK_ACTIVE(PLAYER::PLAYER_PED_ID(), 2))


</summary>
]]--
native "GET_IS_TASK_ACTIVE"
	hash "0xB0760331C7AA4155"
	jhash (0x86FDDF55)
	arguments {
		Ped "ped",

		int "taskNumber",
	}
	returns	"BOOL"

native "GET_SCRIPT_TASK_STATUS"
	hash "0x77F1BEB8863288D5"
	jhash (0xB2477B23)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "GET_ACTIVE_VEHICLE_MISSION_TYPE"
	hash "0x534AEBA6E5ED4CAB"
	jhash (0xAFA914EF)
	arguments {
		Vehicle "veh",
	}
	returns	"int"

native "TASK_LEAVE_ANY_VEHICLE"
	hash "0x504D54DF3F6F2247"
	jhash (0xDBDD79FA)
	arguments {
		Ped "ped",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "TASK_AIM_GUN_SCRIPTED"
	hash "0x7A192BE16D373D00"
	jhash (0x9D296BCD)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "TASK_AIM_GUN_SCRIPTED_WITH_TARGET"
	hash "0x8605AF0DE8B3A5AC"
	jhash (0xFD517CE3)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"void"

native "UPDATE_TASK_AIM_GUN_SCRIPTED_TARGET"
	hash "0x9724FB59A3E72AD0"
	jhash (0x67E73525)
	arguments {
		Ped "p0",

		Ped "p1",

		float "p2",

		float "p3",

		float "p4",

		BOOL "p5",
	}
	returns	"void"

native "GET_CLIP_SET_FOR_SCRIPTED_GUN_TASK"
	hash "0x3A8CADC7D37AACC5"
	jhash (0x249EB4EB)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	duration: the amount of time in milliseconds to do the task.  -1 will keep the task going until either another task is applied, or CLEAR_ALL_TASKS() is called with the ped
</summary>
]]--
native "TASK_AIM_GUN_AT_ENTITY"
	hash "0x9B53BB6E8943AF53"
	jhash (0xBE32B3B6)
	arguments {
		Ped "ped",

		Entity "entity",

		int "duration",

		BOOL "p3",
	}
	returns	"void"

--[[!
<summary>
	duration: the amount of time in milliseconds to do the task. -1 will keep the task going until either another task is applied, or CLEAR_ALL_TASKS() is called with the ped
</summary>
]]--
native "TASK_TURN_PED_TO_FACE_ENTITY"
	hash "0x5AD23D40115353AC"
	jhash (0x3C37C767)
	arguments {
		Ped "ped",

		Entity "entity",

		int "duration",
	}
	returns	"void"

--[[!
<summary>


</summary>
]]--
native "TASK_AIM_GUN_AT_COORD"
	hash "0x6671F3EEC681BDA1"
	jhash (0xFBF44AD3)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		int "time",

		BOOL "p5",

		BOOL "p6",
	}
	returns	"void"

native "TASK_SHOOT_AT_COORD"
	hash "0x46A6CC01E0826106"
	jhash (0x601C22E3)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		int "duration",

		Hash "firingPattern",
	}
	returns	"void"

native "TASK_SHUFFLE_TO_NEXT_VEHICLE_SEAT"
	hash "0x7AA80209BDA643EB"
	jhash (0xBEAF8F67)
	arguments {
		Ped "ped",

		Vehicle "vehicle",
	}
	returns	"void"

native "CLEAR_PED_TASKS"
	hash "0xE1EF3C1216AFF2CD"
	jhash (0xDE3316AB)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "CLEAR_PED_SECONDARY_TASK"
	hash "0x176CECF6F920D707"
	jhash (0xA635F451)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "TASK_EVERYONE_LEAVE_VEHICLE"
	hash "0x7F93691AB4B92272"
	jhash (0xC1971F30)
	arguments {
		Any "p0",
	}
	returns	"void"

native "TASK_GOTO_ENTITY_OFFSET"
	hash "0xE39B4FF4FDEBDE27"
	jhash (0x1A17A85E)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		float "p3",

		float "p4",

		float "p5",

		Any "p6",
	}
	returns	"void"

native "TASK_GOTO_ENTITY_OFFSET_XY"
	hash "0x338E7EF52B6095A9"
	jhash (0xBC1E3D0A)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		Any "p7",
	}
	returns	"void"

native "TASK_TURN_PED_TO_FACE_COORD"
	hash "0x1DDA930A0AC38571"
	jhash (0x30463D73)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",
	}
	returns	"void"

native "TASK_VEHICLE_TEMP_ACTION"
	hash "0xC429DCEEB339E129"
	jhash (0x0679DFB8)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "TASK_VEHICLE_MISSION"
	hash "0x659427E0EF36BCDE"
	jhash (0x20609E56)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		float "p4",

		Any "p5",

		float "p6",

		float "p7",

		BOOL "p8",
	}
	returns	"void"

native "TASK_VEHICLE_MISSION_PED_TARGET"
	hash "0x9454528DF15D657A"
	jhash (0xC81C4677)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		float "p4",

		Any "p5",

		float "p6",

		float "p7",

		BOOL "p8",
	}
	returns	"void"

native "TASK_VEHICLE_MISSION_COORS_TARGET"
	hash "0xF0AF20AA7731F8C3"
	jhash (0x6719C109)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",

		float "p6",

		Any "p7",

		float "p8",

		float "p9",

		BOOL "p10",
	}
	returns	"void"

--[[!
<summary>
	Makes a ped follow the targetVehicle with &lt;minDistance&gt; in between.

	note: minDistance is ignored if drivingstyle is avoiding traffic, but Rushed is fine.

	drivingStyle:
	0 = Rushed
	1 = Ignore Traffic Lights
	2 = Fast
	3 = Normal (Stop in Traffic)
	4 = Fast avoid traffic
	5 = Fast, stops in traffic but overtakes sometimes
	6 = Fast avoids traffic extremely

	if the target is closer than noRoadsDistance, the driver will ignore pathing/roads and follow you directly.

	Driving Styles guide: http://gtaforums.com/topic/822314-guide-driving-styles/
</summary>
]]--
native "TASK_VEHICLE_ESCORT"
	hash "0x0FA6E4B75F302400"
	jhash (0x9FDCB250)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		Vehicle "targetVehicle",

		int "p3",

		float "speed",

		int "drivingStyle",

		float "minDistance",

		int "p7",

		float "noRoadsDistance",
	}
	returns	"void"

--[[!
<summary>
	Makes a ped in a vehicle follow an entity (ped, vehicle, etc.)

	note: ped can halt vehicle (if losing sight?, continues if entity is near)

	drivingStyle:
	0 = Rushed
	1 = Ignore Traffic Lights
	2 = Fast
	3 = Normal (Stop in Traffic)
	4 = Fast avoid traffic
	5 = Fast, stops in traffic but overtakes sometimes
	6 = Fast avoids traffic extremely

	Console Hash: 0xA8B917D7
</summary>
]]--
native "_TASK_VEHICLE_FOLLOW"
	hash "0xFC545A9F0626E3B6"
	arguments {
		Ped "driver",

		Vehicle "vehicle",

		Entity "targetEntity",

		int "drivingStyle",

		float "speed",

		float "minDistance",
	}
	returns	"void"

--[[!
<summary>
	chases targetEnt fast and aggressively
	--
	Makes ped (needs to be in vehicle) chase targetEnt. Not 100% sure
</summary>
]]--
native "TASK_VEHICLE_CHASE"
	hash "0x3C08A8E30363B353"
	jhash (0x55634798)
	arguments {
		Ped "ped",

		Ped "targetEnt",
	}
	returns	"void"

--[[!
<summary>
	pilot, vehicle and altitude are rather self-explanatory.

	entityToFollow: you can provide a Vehicle entity or a Ped entity, the heli will protect them.

	'targetSpeed':  The pilot will dip the nose AS MUCH AS POSSIBLE so as to reach this value AS FAST AS POSSIBLE.  As such, you'll want to modulate it as opposed to calling it via a hard-wired, constant #.

	'radius' isn't just "stop within radius of X of target" like with ground vehicles.  In this case, the pilot will fly an entire circle around 'radius' and continue to do so.

	NOT CONFIRMED:  p7 appears to be a FlyingStyle enum.  Still investigating it as of this writing, but playing around with values here appears to result in different -behavior- as opposed to offsetting coordinates, altitude, target speed, etc.

	-Hingo


	NOTE: If the pilot finds enemies, it will engage them until it kills them, but will return to protect the ped/vehicle given shortly thereafter.
</summary>
]]--
native "TASK_VEHICLE_HELI_PROTECT"
	hash "0x1E09C32048FEFD1C"
	jhash (0x0CB415EE)
	arguments {
		Ped "pilot",

		Vehicle "vehicle",

		Entity "entityToFollow",

		float "targetSpeed",

		int "p4",

		float "radius",

		int "altitude",

		int "p7",
	}
	returns	"void"

native "SET_TASK_VEHICLE_CHASE_BEHAVIOR_FLAG"
	hash "0xCC665AAC360D31E7"
	jhash (0x2A83083F)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "SET_TASK_VEHICLE_CHASE_IDEAL_PURSUIT_DISTANCE"
	hash "0x639B642FACBE4EDD"
	jhash (0x04FD3EE7)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	Ped pilot should be in a heli.
	EntityToFollow can be a vehicle or Ped.

	x,y,z appear to be how close to the EntityToFollow the heli should be. Scripts use 0.0, 0.0, 80.0. Then the heli tries to position itself 80 units above the EntityToFollow. If you reduce it to -5.0, it tries to go below (if the EntityToFollow is a heli or plane)


	NOTE: If the pilot finds enemies, it will engage them, then remain there idle, not continuing to chase the Entity given.
</summary>
]]--
native "TASK_HELI_CHASE"
	hash "0xAC83B1DB38D0ADA0"
	jhash (0xAC290A21)
	arguments {
		Ped "pilot",

		Entity "entityToFollow",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

--[[!
<summary>
	may be is x.y.z i'm not sure krisetrainer
</summary>
]]--
native "TASK_PLANE_CHASE"
	hash "0x2D2386F273FF7A25"
	jhash (0x12FA1C28)
	arguments {
		Ped "p0",

		Vehicle "p1",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

--[[!
<summary>
	 Function.Call(Hash.TASK_PLANE_LAND, pilot, selectedAirplane, runwayStartPoint.X, runwayStartPoint.Y, runwayStartPoint.Z, runwayEndPoint.X, runwayEndPoint.Y, runwayEndPoint.Z); 
</summary>
]]--
native "TASK_PLANE_LAND"
	hash "0xBF19721FA34D32C0"
	jhash (0x5F7E23EA)
	arguments {
		Ped "pilot",

		Vehicle "plane",

		float "runwayStartX",

		float "runwayStartY",

		float "runwayStartZ",

		float "runwayEndX",

		float "runwayEndY",

		float "runwayEndZ",
	}
	returns	"void"

--[[!
<summary>
	Needs more research.

	Modified examples from "fm_mission_controller.ysc", line ~203551:
	AI::TASK_HELI_MISSION(ped, vehicle, 0, 0, posX, posY, posZ, 4, 1.0, -1.0, -1.0, 10, 10, 5.0, 0);
	AI::TASK_HELI_MISSION(ped, vehicle, 0, 0, posX, posY, posZ, 4, 1.0, -1.0, -1.0, 0, ?, 5.0, 4096);
</summary>
]]--
native "TASK_HELI_MISSION"
	hash "0xDAD029E187A2BEB4"
	jhash (0x0C143E97)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		Any "p2",

		Any "p3",

		float "posX",

		float "posY",

		float "posZ",

		int "p7",

		float "p8",

		float "p9",

		float "p10",

		int "p11",

		int "p12",

		float "p13",

		int "p14",
	}
	returns	"void"

--[[!
<summary>
	EXAMPLE USAGE:


	Fly around target (Precautiously, keeps high altitude):
	Function.Call(Hash.TASK_PLANE_MISSION, pilot, selectedAirplane, 0, 0, Target.X, Target.Y, Target.Z, 4, 100f, 0f, 90f, 0, 200f);

	Fly around target (Dangerously, keeps VERY low altitude):
	Function.Call(Hash.TASK_PLANE_MISSION, pilot, selectedAirplane, 0, 0, Target.X, Target.Y, Target.Z, 4, 100f, 0f, 90f, 0, -500f);

	Fly directly into target:
	Function.Call(Hash.TASK_PLANE_MISSION, pilot, selectedAirplane, 0, 0, Target.X, Target.Y, Target.Z, 4, 100f, 0f, 90f, 0, -5000f);




	EXPANDED INFORMATION FOR ADVANCED USAGE (custom pilot)

	'physicsSpeed': (THIS IS NOT YOUR ORDINARY SPEED PARAMETER: READ!!)
	Think of this -first- as a radius value, not a true speed value.  The ACTUAL effective speed of the plane will be that of the maximum speed permissible to successfully fly in a -circle- with a radius of 'physicsSpeed'.  This also means that the plane must complete a circle before it can begin its "bombing run", its straight line pass towards the target.  p9 appears to influence the angle at which a "bombing run" begins, although I can't confirm yet.

	VERY IMPORTANT: A "bombing run" will only occur if a plane can successfully determine a possible navigable route (the slower the value of 'physicsSpeed', the more precise the pilot can be due to less influence of physics on flightpath).  Otherwise, the pilot will continue to patrol around Destination (be it a dynamic Entity position vector or a fixed world coordinate vector.)

	0 = Plane's physics are almost entirely frozen, plane appears to "orbit" around precise destination point
	1-299 = Blend of "frozen, small radius" vs. normal vs. "accelerated, hyperfast, large radius"
	300+ =  Vehicle behaves entirely like a normal gameplay plane.
	n






	'patrolBlend' (The lower the value, the more the Destination is treated as a "fly AT" rather than a "fly AROUND point".)

	Scenario: Destination is an Entity on ground level, wide open field
	-5000 = Pilot kamikazes directly into Entity
	-1000 = Pilot flies extremely low -around- Entity, very prone to crashing
	-200 = Pilot flies lower than average around Entity.
	0 = Pilot flies around Entity, normal altitude
	200 = Pilot flies an extra eighty units or so higher than 0 while flying around Destination (this doesn't seem to correlate directly into distance units.)
</summary>
]]--
native "TASK_PLANE_MISSION"
	hash "0x23703CD154E83B88"
	jhash (0x1D007E65)
	arguments {
		Ped "Pilot",

		Vehicle "Aeroplane",

		int "targetVehicle",

		int "target",

		float "DestinationX",

		float "DestinationY",

		float "DestinationZ",

		int "p7",

		float "physicsSpeed",

		float "p9",

		float "p10",

		float "p11",

		float "patrolBlend",
	}
	returns	"void"

--[[!
<summary>
	You need to call PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS after TASK_BOAT_MISSION in order for the task to execute.

	Working example
	float vehicleMaxSpeed = VEHICLE::_GET_VEHICLE_MAX_SPEED(ENTITY::GET_ENTITY_MODEL(pedVehicle));
	AI::TASK_BOAT_MISSION(pedDriver, pedVehicle, 0, 0, waypointCoord.x, waypointCoord.y, waypointCoord.z, 4, vehicleMaxSpeed, 786469, -1.0, 7);
	PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(pedDriver, 1);
</summary>
]]--
native "TASK_BOAT_MISSION"
	hash "0x15C86013127CE63F"
	jhash (0x5865B031)
	arguments {
		Ped "pedDriver",

		Vehicle "boat",

		Any "p2",

		Any "p3",

		float "x",

		float "y",

		float "z",

		Any "p7",

		float "maxSpeed",

		Any "p9",

		float "p10",

		Any "p11",
	}
	returns	"void"

native "TASK_DRIVE_BY"
	hash "0x2F8AF0E82773A171"
	jhash (0x2B84D1C4)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		Any "p7",

		BOOL "p8",

		Any "p9",
	}
	returns	"void"

native "SET_DRIVEBY_TASK_TARGET"
	hash "0xE5B302114D8162EE"
	jhash (0xDA6A6FC1)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"void"

native "CLEAR_DRIVEBY_TASK_UNDERNEATH_DRIVING_TASK"
	hash "0xC35B5CDB2824CF69"
	jhash (0x9B76F7E6)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "IS_DRIVEBY_TASK_UNDERNEATH_DRIVING_TASK"
	hash "0x8785E6E40C7A8818"
	jhash (0xB23F46E6)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "CONTROL_MOUNTED_WEAPON"
	hash "0xDCFE42068FE0135A"
	jhash (0x500D9244)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "SET_MOUNTED_WEAPON_TARGET"
	hash "0xCCD892192C6D2BB9"
	jhash (0x98713C68)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"void"

native "IS_MOUNTED_WEAPON_TASK_UNDERNEATH_DRIVING_TASK"
	hash "0xA320EF046186FA3B"
	jhash (0x291E938C)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	Found in the decompiled script "apptextmessage.c4":
	AI::TASK_USE_MOBILE_PHONE(PLAYER::PLAYER_PED_ID(), 1, 1);

	Note: there should be another integer but I am sure how to change it ^_^ -wilie wonka
</summary>
]]--
native "TASK_USE_MOBILE_PHONE"
	hash "0xBD2A8EC3AF4DE7DB"
	jhash (0x225A38C8)
	arguments {
		Ped "ped",

		int "p1",
	}
	returns	"void"

native "TASK_USE_MOBILE_PHONE_TIMED"
	hash "0x5EE02954A14C69DB"
	jhash (0xC99C19F5)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	p2 tend to be 16, 17 or 1
	p3 to p7 tend to be 0.0
</summary>
]]--
native "TASK_CHAT_TO_PED"
	hash "0x8C338E0263E4FD19"
	jhash (0xA2BE1821)
	arguments {
		Ped "ped",

		Ped "target",

		Any "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",
	}
	returns	"void"

native "TASK_WARP_PED_INTO_VEHICLE"
	hash "0x9A7D091411C5F684"
	jhash (0x65D4A35D)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		Any "seat",
	}
	returns	"void"

--[[!
<summary>
	//this part of the code is to determine at which entity the player is aiming, for example if you want to create a mod where you give orders to peds
	Entity aimedentity;
	Player player = PLAYER::PLAYER_ID();
	PLAYER::_GET_AIMED_ENTITY(player, &amp;aimedentity);

	//bg is an array of peds
	AI::TASK_SHOOT_AT_ENTITY(bg[i], aimedentity, 5000, GAMEPLAY::GET_HASH_KEY("FIRING_PATTERN_FULL_AUTO"));

	in practical usage, getting the entity the player is aiming at and then task the peds to shoot at the entity, at a button press event would be better.
</summary>
]]--
native "TASK_SHOOT_AT_ENTITY"
	hash "0x08DA95E8298AE772"
	jhash (0xAC0631C9)
	arguments {
		Entity "entity",

		Entity "target",

		int "duration",

		Hash "firingPattern",
	}
	returns	"void"

native "TASK_CLIMB"
	hash "0x89D9FCC2435112F1"
	jhash (0x90847790)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "TASK_CLIMB_LADDER"
	hash "0xB6C987F9285A3814"
	jhash (0x35BB4EE0)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Immediately stops the pedestrian from whatever it's doing. They stop fighting, animations, etc. they forget what they were doing.


</summary>
]]--
native "CLEAR_PED_TASKS_IMMEDIATELY"
	hash "0xAAA34F8A7CB32098"
	jhash (0xBC045625)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "TASK_PERFORM_SEQUENCE_FROM_PROGRESS"
	hash "0x89221B16730234F0"
	jhash (0xFA60601B)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "SET_NEXT_DESIRED_MOVE_STATE"
	hash "0xF1B9F16E89E2C93A"
	jhash (0x4E937D57)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_PED_DESIRED_MOVE_BLEND_RATIO"
	hash "0x1E982AC8716912C5"
	jhash (0xC65FC712)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "GET_PED_DESIRED_MOVE_BLEND_RATIO"
	hash "0x8517D4A6CA8513ED"
	jhash (0x5FEFAB72)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	eg

	 AI::TASK_GOTO_ENTITY_AIMING(v_2, PLAYER::PLAYER_PED_ID(), 5.0, 25.0);

	ped = Ped you want to perform this task.
	target = the Entity they should aim at.
	distanceToStopAt = distance from the target, where the ped should stop to aim.
	StartAimingDist = distance where the ped should start to aim.
</summary>
]]--
native "TASK_GOTO_ENTITY_AIMING"
	hash "0xA9DA48FAB8A76C12"
	jhash (0xF1C493CF)
	arguments {
		Ped "ped",

		Entity "target",

		float "distanceToStopAt",

		float "StartAimingDist",
	}
	returns	"void"

--[[!
<summary>
	p1 is always GET_HASH_KEY("empty") in scripts, for the rare times this is used
</summary>
]]--
native "TASK_SET_DECISION_MAKER"
	hash "0xEB8517DDA73720DA"
	jhash (0x830AD50C)
	arguments {
		Ped "p0",

		Hash "p1",
	}
	returns	"void"

native "TASK_SET_SPHERE_DEFENSIVE_AREA"
	hash "0x933C06518B52A9A4"
	jhash (0x9F3C5D6A)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"void"

native "TASK_CLEAR_DEFENSIVE_AREA"
	hash "0x95A6C46A31D1917D"
	jhash (0x7A05BF0D)
	arguments {
		Any "p0",
	}
	returns	"void"

native "TASK_PED_SLIDE_TO_COORD"
	hash "0xD04FE6765D990A06"
	jhash (0x225380EF)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		float "heading",

		float "p5",
	}
	returns	"void"

native "TASK_PED_SLIDE_TO_COORD_HDG_RATE"
	hash "0x5A4A6A6D3DC64F52"
	jhash (0x38A995C1)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		float "heading",

		float "p5",

		float "p6",
	}
	returns	"void"

native "ADD_COVER_POINT"
	hash "0xD5C12A75C7B9497F"
	jhash (0xA0AF0B98)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		BOOL "p7",
	}
	returns	"ScrHandle"

native "REMOVE_COVER_POINT"
	hash "0xAE287C923D891715"
	jhash (0x0776888B)
	arguments {
		ScrHandle "coverpoint",
	}
	returns	"void"

--[[!
<summary>
	Checks if there is a cover point at position
</summary>
]]--
native "DOES_SCRIPTED_COVER_POINT_EXIST_AT_COORDS"
	hash "0xA98B8E3C088E5A31"
	jhash (0x29F97A71)
	arguments {
		float "x",

		float "y",

		float "z",
	}
	returns	"BOOL"

native "GET_SCRIPTED_COVER_POINT_COORDS"
	hash "0x594A1028FC2A3E85"
	jhash (0xC6B6CCC1)
	arguments {
		ScrHandle "coverpoint",
	}
	returns	"Vector3"

--[[!
<summary>
	Makes the specified ped attack the target ped.
	p2 should be 0
	p3 should be 16
</summary>
]]--
native "TASK_COMBAT_PED"
	hash "0xF166E48407BAC484"
	jhash (0xCB0D8932)
	arguments {
		Ped "ped",

		Ped "targetPed",

		int "p2",

		int "p3",
	}
	returns	"void"

native "TASK_COMBAT_PED_TIMED"
	hash "0x944F30DCB7096BDE"
	jhash (0xF5CA2A45)
	arguments {
		Any "p0",

		Ped "ped",

		int "p2",

		Any "p3",
	}
	returns	"void"

native "TASK_SEEK_COVER_FROM_POS"
	hash "0x75AC2B60386D89F2"
	jhash (0x83F18EE9)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		BOOL "p5",
	}
	returns	"void"

native "TASK_SEEK_COVER_FROM_PED"
	hash "0x84D32B3BEC531324"
	jhash (0xC1EC907E)
	arguments {
		Ped "ped",

		Ped "target",

		int "duration",

		BOOL "p3",
	}
	returns	"void"

native "TASK_SEEK_COVER_TO_COVER_POINT"
	hash "0xD43D95C7A869447F"
	jhash (0x3D026B29)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",

		BOOL "p6",
	}
	returns	"void"

--[[!
<summary>
	from michael2:
	AI::TASK_SEEK_COVER_TO_COORDS(ped, 967.5164794921875, -2121.603515625, 30.479299545288086, 978.94677734375, -2125.84130859375, 29.4752, -1, 1);


	appears to be shorter variation
	from michael3:
	AI::TASK_SEEK_COVER_TO_COORDS(ped, -2231.011474609375, 263.6326599121094, 173.60195922851562, -1, 0);
</summary>
]]--
native "TASK_SEEK_COVER_TO_COORDS"
	hash "0x39246A6958EF072C"
	jhash (0xFFFE754E)
	arguments {
		Ped "ped",

		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		Any "p7",

		BOOL "p8",
	}
	returns	"void"

native "TASK_PUT_PED_DIRECTLY_INTO_COVER"
	hash "0x4172393E6BE1FECE"
	jhash (0xC9F00E68)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		BOOL "p5",

		float "p6",

		BOOL "p7",

		BOOL "p8",

		Any "p9",

		BOOL "p10",
	}
	returns	"void"

native "TASK_EXIT_COVER"
	hash "0x79B258E397854D29"
	jhash (0xC829FAC9)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"void"

--[[!
<summary>
	from armenian3.c4

	AI::TASK_PUT_PED_DIRECTLY_INTO_MELEE(PlayerPed, armenianPed, 0.0, -1.0, 0.0, 0);

	Note: Don't ever call a parameter "bool" -.-
</summary>
]]--
native "TASK_PUT_PED_DIRECTLY_INTO_MELEE"
	hash "0x1C6CD14A876FFE39"
	jhash (0x79E1D27D)
	arguments {
		Ped "ped",

		Ped "meleeTarget",

		float "p2",

		float "p3",

		float "p4",

		BOOL "p5",
	}
	returns	"void"

--[[!
<summary>
	used in sequence task

	both parameters seems to be always 0
</summary>
]]--
native "TASK_TOGGLE_DUCK"
	hash "0xAC96609B9995EDF8"
	jhash (0x61CFBCBF)
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"void"

native "TASK_GUARD_CURRENT_POSITION"
	hash "0x4A58A47A72E3FCB4"
	jhash (0x2FB099E9)
	arguments {
		Ped "p0",

		float "p1",

		float "p2",

		BOOL "p3",
	}
	returns	"void"

native "TASK_GUARD_ASSIGNED_DEFENSIVE_AREA"
	hash "0xD2A207EEBDF9889B"
	jhash (0x7AF0133D)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		Any "p6",
	}
	returns	"void"

--[[!
<summary>
	p0 - Guessing PedID
	p1, p2, p3 - XYZ?
	p4 - ???
	p5 - Maybe the size of sphere from XYZ?
	p6 - ???
	p7, p8, p9 - XYZ again?
	p10 - Maybe the size of sphere from second XYZ?
</summary>
]]--
native "TASK_GUARD_SPHERE_DEFENSIVE_AREA"
	hash "0xC946FE14BE0EB5E2"
	jhash (0x86B76CB7)
	arguments {
		Ped "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		Any "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",
	}
	returns	"void"

--[[!
<summary>
	p0 - Guessing PedID
	p1, p2, p3 - XYZ?
	p4 
	p5 - "WORLD_HUMAN_GUARD_STAND" Guessing that is an animation name.
</summary>
]]--
native "TASK_STAND_GUARD"
	hash "0xAE032F8BBA959E90"
	jhash (0xD130F636)
	arguments {
		Ped "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		charPtr "p5",
	}
	returns	"void"

native "SET_DRIVE_TASK_CRUISE_SPEED"
	hash "0x5C9B84BD7D31D908"
	jhash (0x3CEC07B1)
	arguments {
		Ped "driver",

		float "cruiseSpeed",
	}
	returns	"void"

native "SET_DRIVE_TASK_MAX_CRUISE_SPEED"
	hash "0x404A5AA9B9F0B746"
	jhash (0x7FDF6131)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	This native is used to set the driving style for specific ped.

	Driving styles id seems to be:
	786468
	262144
	786469

	http://gtaforums.com/topic/822314-guide-driving-styles/
</summary>
]]--
native "SET_DRIVE_TASK_DRIVING_STYLE"
	hash "0xDACE1BE37D88AF67"
	jhash (0x59C5FAD7)
	arguments {
		Ped "ped",

		int "drivingStyle",
	}
	returns	"void"

native "ADD_COVER_BLOCKING_AREA"
	hash "0x45C597097DD7CB81"
	jhash (0x3536946F)
	arguments {
		float "playerX",

		float "playerY",

		float "playerZ",

		float "radiusX",

		float "radiusY",

		float "radiusZ",

		BOOL "p6",

		BOOL "p7",

		BOOL "p8",

		BOOL "p9",
	}
	returns	"void"

native "REMOVE_ALL_COVER_BLOCKING_AREAS"
	hash "0xDB6708C0B46F56D8"
	jhash (0xCF9221A7)
	returns	"void"

--[[!
<summary>
	Plays a scenario on a Ped at their current location.

	unkDelay - Usually 0 or -1, doesn't seem to have any effect. Might be a delay between sequences.
	playEnterAnim - Plays the "Enter" anim if true, otherwise plays the "Exit" anim. Scenarios that don't have any "Enter" anims won't play if this is set to true.

	----

	From "am_hold_up.ysc.c4" at line 339:

	AI::TASK_START_SCENARIO_IN_PLACE(NETWORK::NET_TO_PED(l_8D._f4), sub_adf(), 0, 1);

	I'm unsure of what the last two parameters are, however sub_adf() randomly returns 1 of 3 scenarios, those being:
	WORLD_HUMAN_SMOKING
	WORLD_HUMAN_HANG_OUT_STREET
	WORLD_HUMAN_STAND_MOBILE

	This makes sense, as these are what I commonly see when going by a liquor store.
</summary>
]]--
native "TASK_START_SCENARIO_IN_PLACE"
	hash "0x142A02425FF02BD9"
	jhash (0xE50D6DDE)
	arguments {
		Ped "ped",

		charPtr "scenarioName",

		int "unkDelay",

		BOOL "playEnterAnim",
	}
	returns	"void"

--[[!
<summary>
	The first parameter in every scenario has always been a Ped of some sort. The second like TASK_START_SCENARIO_IN_PLACE is the name of the scenario. 

	The next 4 parameters were harder to decipher. After viewing "hairdo_shop_mp.ysc.c4", and being confused from seeing the case in other scripts, they passed the first three of the arguments as one array from a function, and it looked like it was obviously x, y, and z.

	I haven't seen the sixth parameter go to or over 360, making me believe that it is rotation, but I really can't confirm anything.

	I have no idea what the last 3 parameters are, but I'll try to find out.
</summary>
]]--
native "TASK_START_SCENARIO_AT_POSITION"
	hash "0xFA4EFC79F69D4F07"
	jhash (0xAA2C4AC2)
	arguments {
		Ped "ped",

		charPtr "scenarioName",

		float "x",

		float "y",

		float "z",

		float "heading",

		Any "p6",

		BOOL "p7",

		BOOL "p8",
	}
	returns	"void"

native "TASK_USE_NEAREST_SCENARIO_TO_COORD"
	hash "0x277F471BA9DB000B"
	jhash (0x9C50FBF0)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",
	}
	returns	"void"

native "TASK_USE_NEAREST_SCENARIO_TO_COORD_WARP"
	hash "0x58E2E0F23F6B76C3"
	jhash (0x1BE9D65C)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",
	}
	returns	"void"

native "TASK_USE_NEAREST_SCENARIO_CHAIN_TO_COORD"
	hash "0x9FDA1B3D7E7028B3"
	jhash (0xE32FFB22)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",
	}
	returns	"void"

native "TASK_USE_NEAREST_SCENARIO_CHAIN_TO_COORD_WARP"
	hash "0x97A28E63F0BA5631"
	jhash (0xBAB4C0AE)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",
	}
	returns	"void"

native "DOES_SCENARIO_EXIST_IN_AREA"
	hash "0x5A59271FFADD33C1"
	jhash (0xFA7F5047)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "b",
	}
	returns	"BOOL"

native "DOES_SCENARIO_OF_TYPE_EXIST_IN_AREA"
	hash "0x0A9D0C2A3BBC86C1"
	jhash (0x0FB138A5)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		AnyPtr "p3",

		float "p4",

		BOOL "p5",
	}
	returns	"BOOL"

native "IS_SCENARIO_OCCUPIED"
	hash "0x788756D73AC2E07C"
	jhash (0x697FC008)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",
	}
	returns	"BOOL"

native "PED_HAS_USE_SCENARIO_TASK"
	hash "0x295E3CCEC879CCD7"
	jhash (0x9BE9C691)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "PLAY_ANIM_ON_RUNNING_SCENARIO"
	hash "0x748040460F8DF5DC"
	jhash (0x1984A5D1)
	arguments {
		Ped "ped",

		charPtr "animDict",

		charPtr "animName",
	}
	returns	"void"

native "DOES_SCENARIO_GROUP_EXIST"
	hash "0xF9034C136C9E00D3"
	jhash (0x5F072EB9)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "IS_SCENARIO_GROUP_ENABLED"
	hash "0x367A09DED4E05B99"
	jhash (0x90991122)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "SET_SCENARIO_GROUP_ENABLED"
	hash "0x02C8E5B49848664E"
	jhash (0x116997B1)
	arguments {
		AnyPtr "p0",

		BOOL "p1",
	}
	returns	"void"

native "RESET_SCENARIO_GROUPS_ENABLED"
	hash "0xDD902D0349AFAD3A"
	jhash (0xBF55025D)
	returns	"void"

native "SET_EXCLUSIVE_SCENARIO_GROUP"
	hash "0x535E97E1F7FC0C6A"
	jhash (0x59DB8F26)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "RESET_EXCLUSIVE_SCENARIO_GROUP"
	hash "0x4202BBCB8684563D"
	jhash (0x17F9DFE8)
	returns	"void"

native "IS_SCENARIO_TYPE_ENABLED"
	hash "0x3A815DB3EA088722"
	jhash (0xAE37E969)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	seems to enable/disable specific scenario-types from happening in the game world.

	Here are some scenario types from the scripts:
	"WORLD_MOUNTAIN_LION_REST"                                             
	"WORLD_MOUNTAIN_LION_WANDER"                                            
	"DRIVE"                                                                  
	"WORLD_VEHICLE_POLICE_BIKE"                                             
	"WORLD_VEHICLE_POLICE_CAR"                                             
	"WORLD_VEHICLE_POLICE_NEXT_TO_CAR"                                        
	"WORLD_VEHICLE_DRIVE_SOLO"                                                 
	"WORLD_VEHICLE_BIKER"                                                      
	"WORLD_VEHICLE_DRIVE_PASSENGERS"                                           
	"WORLD_VEHICLE_SALTON_DIRT_BIKE"                                           
	"WORLD_VEHICLE_BICYCLE_MOUNTAIN"                                           
	"PROP_HUMAN_SEAT_CHAIR"                                             
	"WORLD_VEHICLE_ATTRACTOR"                                             
	"WORLD_HUMAN_LEANING"                                                 
	"WORLD_HUMAN_HANG_OUT_STREET"                                        
	"WORLD_HUMAN_DRINKING"                                                
	"WORLD_HUMAN_SMOKING"                                                
	"WORLD_HUMAN_GUARD_STAND"                                            
	"WORLD_HUMAN_CLIPBOARD"                                              
	"WORLD_HUMAN_HIKER"                                                  
	"WORLD_VEHICLE_EMPTY"                                                      
	"WORLD_VEHICLE_BIKE_OFF_ROAD_RACE"                                      
	"WORLD_HUMAN_PAPARAZZI"                                               
	"WORLD_VEHICLE_PARK_PERPENDICULAR_NOSE_IN"                            
	"WORLD_VEHICLE_PARK_PARALLEL"                                              
	"WORLD_VEHICLE_CONSTRUCTION_SOLO"                               
	"WORLD_VEHICLE_CONSTRUCTION_PASSENGERS"                                                                    
	"WORLD_VEHICLE_TRUCK_LOGS" 
</summary>
]]--
native "SET_SCENARIO_TYPE_ENABLED"
	hash "0xEB47EC4E34FB7EE1"
	jhash (0xDB18E5DE)
	arguments {
		charPtr "scenarioType",

		BOOL "toggle",
	}
	returns	"void"

native "RESET_SCENARIO_TYPES_ENABLED"
	hash "0x0D40EE2A7F2B2D6D"
	jhash (0xF58FDEB4)
	returns	"void"

native "IS_PED_ACTIVE_IN_SCENARIO"
	hash "0xAA135F9482C82CC3"
	jhash (0x05038F1A)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "0x621C6E4729388E41"
	hash "0x621C6E4729388E41"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x8FD89A6240813FD0"
	hash "0x8FD89A6240813FD0"
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	Despite its name, it only attacks ONE hated target. The one closest to the specified position.
</summary>
]]--
native "TASK_COMBAT_HATED_TARGETS_IN_AREA"
	hash "0x4CF5F55DAC3280A0"
	jhash (0xDF099E18)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		float "radius",

		Any "p5",
	}
	returns	"void"

--[[!
<summary>
	Despite its name, it only attacks ONE hated target. The one closest hated target.

	p2 seems to be always 0
</summary>
]]--
native "TASK_COMBAT_HATED_TARGETS_AROUND_PED"
	hash "0x7BF835BB9E2698C8"
	jhash (0x2E7064E4)
	arguments {
		Ped "ped",

		float "radius",

		int "p2",
	}
	returns	"void"

native "TASK_COMBAT_HATED_TARGETS_AROUND_PED_TIMED"
	hash "0x2BBA30B854534A0C"
	jhash (0xF127AD6A)
	arguments {
		Any "p0",

		float "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

--[[!
<summary>
	In every case of this native, I've only seen the first parameter passed as 0, although I believe it's a Ped after seeing tasks around it using 0. That's because it's used in a Sequence Task.

	The last 3 parameters are definitely coordinates after seeing them passed in other scripts, and even being used straight from the player's coordinates.
	---
	It seems that - in the decompiled scripts - this native was used on a ped who was in a vehicle to throw a projectile out the window at the player. This is something any ped will naturally do if they have a throwable and they are doing driveby-combat (although not very accurately).
	It is possible, however, that this is how SWAT throws smoke grenades at the player when in cover
</summary>
]]--
native "TASK_THROW_PROJECTILE"
	hash "0x7285951DBF6B5A51"
	jhash (0xF65C20A7)
	arguments {
		Any "p0",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "TASK_SWAP_WEAPON"
	hash "0xA21C51255B205245"
	jhash (0xDAF4F8FC)
	arguments {
		Ped "ped",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	The only occurrence I found in a R* script ("assassin_construction.ysc.c4"):

	            if (((v_3 &lt; v_4) &amp;&amp; (AI::GET_SCRIPT_TASK_STATUS(PLAYER::PLAYER_PED_ID(), 0x6a67a5cc) != 1)) &amp;&amp; (v_5 &gt; v_3)) {
	                AI::TASK_RELOAD_WEAPON(PLAYER::PLAYER_PED_ID(), 1);
	            }
</summary>
]]--
native "TASK_RELOAD_WEAPON"
	hash "0x62D2916F56B9CD2D"
	jhash (0xCA6E91FD)
	arguments {
		Ped "ped",

		BOOL "doReload",
	}
	returns	"void"

native "IS_PED_GETTING_UP"
	hash "0x2A74E1D5F2F00EEC"
	jhash (0x320813E6)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	EX: Function.Call(Ped1, Ped2, Time, 0);

	The last parameter is always 0 for some reason I do not know. The first parameter is the pedestrian who will writhe to the pedestrian in the other parameter. The third paremeter is how long he will live, if set to -1, he will not die until killed or touched by another entity.


</summary>
]]--
native "TASK_WRITHE"
	hash "0xCDDC2B77CE54AC6E"
	jhash (0x0FDC54FC)
	arguments {
		Ped "ped",

		Ped "target",

		int "timeOut",

		BOOL "p3",
	}
	returns	"void"

--[[!
<summary>
	returns true is the ped is on the ground bleeding to death from a gunshot wound
</summary>
]]--
native "IS_PED_IN_WRITHE"
	hash "0xDEB6D52126E7D640"
	jhash (0x09E61921)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "OPEN_PATROL_ROUTE"
	hash "0xA36BFB5EE89F3D82"
	jhash (0xF33F83CA)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "CLOSE_PATROL_ROUTE"
	hash "0xB043ECA801B8CBC1"
	jhash (0x67305E59)
	returns	"void"

native "ADD_PATROL_ROUTE_NODE"
	hash "0x8EDF950167586B7C"
	jhash (0x21B48F10)
	arguments {
		Any "p0",

		AnyPtr "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		Any "p8",
	}
	returns	"void"

native "ADD_PATROL_ROUTE_LINK"
	hash "0x23083260DEC3A551"
	jhash (0xD8761BB3)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "CREATE_PATROL_ROUTE"
	hash "0xAF8A443CCC8018DC"
	jhash (0x0A6C7864)
	returns	"void"

native "DELETE_PATROL_ROUTE"
	hash "0x7767DD9D65E91319"
	jhash (0x2A4E6706)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

--[[!
<summary>
	After looking at some scripts the second parameter seems to be an id of some kind. Here are some I found from some R* scripts:

	"miss_Tower_01" (this went from 01 - 10)
	"miss_Ass0" (0, 4, 6, 3)
	"MISS_PATROL_8"

	I think they're patrol routes, but I'm not sure. And I believe the 3rd parameter is a BOOL, but I can't confirm other than only seeing 0 and 1 being passed.



	As far as I can see the patrol routes names such as "miss_Ass0" have been defined earlier in the scripts. This leads me to believe we can defined our own new patrol routes by following the same approach. 
	From the scripts

	    AI::OPEN_PATROL_ROUTE("miss_Ass0");
	    AI::ADD_PATROL_ROUTE_NODE(0, "WORLD_HUMAN_GUARD_STAND", l_738[0/*3*/], -139.4076690673828, -993.4732055664062, 26.2754, GAMEPLAY::GET_RANDOM_INT_IN_RANGE(5000, 10000));
	    AI::ADD_PATROL_ROUTE_NODE(1, "WORLD_HUMAN_GUARD_STAND", l_738[1/*3*/], -116.1391830444336, -987.4984130859375, 26.38541030883789, GAMEPLAY::GET_RANDOM_INT_IN_RANGE(5000, 10000));
	    AI::ADD_PATROL_ROUTE_NODE(2, "WORLD_HUMAN_GUARD_STAND", l_738[2/*3*/], -128.46847534179688, -979.0340576171875, 26.2754, GAMEPLAY::GET_RANDOM_INT_IN_RANGE(5000, 10000));
	    AI::ADD_PATROL_ROUTE_LINK(0, 1);
	    AI::ADD_PATROL_ROUTE_LINK(1, 2);
	    AI::ADD_PATROL_ROUTE_LINK(2, 0);
	    AI::CLOSE_PATROL_ROUTE();
	    AI::CREATE_PATROL_ROUTE();


</summary>
]]--
native "TASK_PATROL"
	hash "0xBDA5DF49D080FE4E"
	jhash (0xB92E5AF6)
	arguments {
		Ped "ped",

		charPtr "p1",

		Any "p2",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"void"

native "TASK_STAY_IN_COVER"
	hash "0xE5DA8615A6180789"
	jhash (0xA27A9413)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	x, y, z: offset in world coords from some entity.
</summary>
]]--
native "ADD_VEHICLE_SUBTASK_ATTACK_COORD"
	hash "0x5CF0D8F9BBA0DD75"
	jhash (0x50779A2C)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "ADD_VEHICLE_SUBTASK_ATTACK_PED"
	hash "0x85F462BADC7DA47F"
	jhash (0x80461113)
	arguments {
		Ped "ped",

		Ped "ped2",
	}
	returns	"void"

native "TASK_VEHICLE_SHOOT_AT_PED"
	hash "0x10AB107B887214D8"
	jhash (0x59677BA0)
	arguments {
		Ped "ped",

		Ped "target",

		float "p2",
	}
	returns	"void"

native "TASK_VEHICLE_AIM_AT_PED"
	hash "0xE41885592B08B097"
	jhash (0x920AE6DB)
	arguments {
		Ped "ped",

		Ped "target",
	}
	returns	"void"

native "TASK_VEHICLE_SHOOT_AT_COORD"
	hash "0x5190796ED39C9B6D"
	jhash (0xA7AAA4D6)
	arguments {
		Vehicle "vehicle",

		float "x",

		float "y",

		float "z",

		float "p4",
	}
	returns	"void"

native "TASK_VEHICLE_AIM_AT_COORD"
	hash "0x447C1E9EF844BC0F"
	jhash (0x010F47CE)
	arguments {
		Vehicle "vehicle",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

--[[!
<summary>
	Differs from TASK_VEHICLE_DRIVE_TO_COORDS in that it will pick the shortest possible road route without taking one-way streets and other "road laws" into consideration.

	WARNING:
	A behaviorFlag value of 0 will result in a clunky, stupid driver!

	Recommended settings:
	speed = 30.0f,
	behaviorFlag = 156, 
	stoppingRange = 5.0f;

	If you simply want to have your driver move to a fixed location, call it only once, or, when necessary in the event of interruption. 

	If using this to continually follow a Ped who is on foot:  You will need to run this in a tick loop.  Call it in with the Ped's updated coordinates every 20 ticks or so and you will have one hell of a smart, fast-reacting NPC driver -- provided he doesn't get stuck.  If your update frequency is too fast, the Ped may not have enough time to figure his way out of being stuck, and thus, remain stuck.  One way around this would be to implement an "anti-stuck" mechanism, which allows the driver to realize he's stuck, temporarily pause the tick, unstuck, then resume the tick.

	EDIT:  This is being discussed in more detail at http://gtaforums.com/topic/818504-any-idea-on-how-to-make-peds-clever-and-insanely-fast-c/  
</summary>
]]--
native "TASK_VEHICLE_GOTO_NAVMESH"
	hash "0x195AEEB13CEFE2EE"
	jhash (0x55CF3BCD)
	arguments {
		Ped "ped",

		Vehicle "vehicle",

		float "x",

		float "y",

		float "z",

		float "speed",

		int "behaviorFlag",

		float "stoppingRange",
	}
	returns	"void"

--[[!
<summary>
	movement_speed: mostly 2f, but also 1/1.2f, etc.
	p8: always false
	p9: 2f
	p10: 0.5f
	p11: true
	p12: 0 / 512 / 513, etc.
	p13: 0
	firing_pattern: ${firing_pattern_full_auto}, 0xC6EE6B4C
</summary>
]]--
native "TASK_GO_TO_COORD_WHILE_AIMING_AT_COORD"
	hash "0x11315AB3385B8AC0"
	jhash (0x1552DC91)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		float "aimAtX",

		float "aimAtY",

		float "aimAtZ",

		float "moveSpeed",

		BOOL "p8",

		float "p9",

		float "p10",

		BOOL "p11",

		Any "flags",

		BOOL "p13",

		Hash "firingPattern",
	}
	returns	"void"

native "TASK_GO_TO_COORD_WHILE_AIMING_AT_ENTITY"
	hash "0xB2A16444EAD9AE47"
	jhash (0x9BD52ABD)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		float "p5",

		BOOL "p6",

		float "p7",

		float "p8",

		BOOL "p9",

		Any "p10",

		BOOL "p11",

		Any "p12",

		Any "p13",
	}
	returns	"void"

native "TASK_GO_TO_COORD_AND_AIM_AT_HATED_ENTITIES_NEAR_COORD"
	hash "0xA55547801EB331FC"
	jhash (0x3F91358E)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		BOOL "p8",

		float "p9",

		float "p10",

		BOOL "p11",

		Any "p12",

		Any "p13",

		Any "p14",
	}
	returns	"void"

native "TASK_GO_TO_ENTITY_WHILE_AIMING_AT_COORD"
	hash "0x04701832B739DCE5"
	jhash (0xD896CD82)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",

		float "p7",

		float "p8",

		BOOL "p9",

		BOOL "p10",

		Any "p11",
	}
	returns	"void"

--[[!
<summary>
	shootatEntity:
	If true, peds will shoot at Entity till it is dead.
	If false, peds will just walk till they reach the entity and will cease shooting.
</summary>
]]--
native "TASK_GO_TO_ENTITY_WHILE_AIMING_AT_ENTITY"
	hash "0x97465886D35210E9"
	jhash (0x68E36B7A)
	arguments {
		Ped "ped",

		Entity "entityToWalkTo",

		Entity "entityToAimAt",

		float "speed",

		BOOL "shootatEntity",

		float "p5",

		float "p6",

		BOOL "p7",

		BOOL "p8",

		Hash "firingPattern",
	}
	returns	"void"

--[[!
<summary>
	makes ped p0 ragdoll like when falling from a great height
</summary>
]]--
native "SET_HIGH_FALL_TASK"
	hash "0x8C825BDC7741D37C"
	jhash (0xBBB26172)
	arguments {
		Ped "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "REQUEST_WAYPOINT_RECORDING"
	hash "0x9EEFB62EB27B5792"
	jhash (0xAFABFB5D)
	arguments {
		charPtr "name",
	}
	returns	"void"

native "GET_IS_WAYPOINT_RECORDING_LOADED"
	hash "0xCB4E8BE8A0063C5D"
	jhash (0x87125F5D)
	arguments {
		charPtr "name",
	}
	returns	"BOOL"

native "REMOVE_WAYPOINT_RECORDING"
	hash "0xFF1B8B4AA1C25DC8"
	jhash (0x624530B0)
	arguments {
		charPtr "name",
	}
	returns	"void"

native "WAYPOINT_RECORDING_GET_NUM_POINTS"
	hash "0x5343532C01A07234"
	jhash (0xF5F9B71E)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "WAYPOINT_RECORDING_GET_COORD"
	hash "0x2FB897405C90B361"
	jhash (0x19266913)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "WAYPOINT_RECORDING_GET_SPEED_AT_POINT"
	hash "0x005622AEBC33ACA9"
	jhash (0xC765633A)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"float"

native "WAYPOINT_RECORDING_GET_CLOSEST_WAYPOINT"
	hash "0xB629A298081F876F"
	jhash (0xC4CD35AF)
	arguments {
		AnyPtr "p0",

		float "p1",

		float "p2",

		float "p3",

		AnyPtr "p4",
	}
	returns	"BOOL"

native "TASK_FOLLOW_WAYPOINT_RECORDING"
	hash "0x0759591819534F7B"
	jhash (0xADF9904D)
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "IS_WAYPOINT_PLAYBACK_GOING_ON_FOR_PED"
	hash "0xE03B3F2D3DC59B64"
	jhash (0x85B7725F)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "GET_PED_WAYPOINT_PROGRESS"
	hash "0x2720AAA75001E094"
	jhash (0x3595B104)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_PED_WAYPOINT_DISTANCE"
	hash "0xE6A877C64CAF1BC5"
	jhash (0x084B35B0)
	arguments {
		Any "p0",
	}
	returns	"float"

native "SET_PED_WAYPOINT_ROUTE_OFFSET"
	hash "0xED98E10B0AFCE4B4"
	jhash (0xF867F747)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"Any"

native "GET_WAYPOINT_DISTANCE_ALONG_ROUTE"
	hash "0xA5B769058763E497"
	jhash (0xE8422AC4)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"float"

native "WAYPOINT_PLAYBACK_GET_IS_PAUSED"
	hash "0x701375A7D43F01CB"
	jhash (0xA6BB5717)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "WAYPOINT_PLAYBACK_PAUSE"
	hash "0x0F342546AA06FED5"
	jhash (0xFE39ECF8)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "WAYPOINT_PLAYBACK_RESUME"
	hash "0x244F70C84C547D2D"
	jhash (0x50F392EF)
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "WAYPOINT_PLAYBACK_OVERRIDE_SPEED"
	hash "0x7D7D2B47FA788E85"
	jhash (0x23E6BA96)
	arguments {
		Any "p0",

		float "p1",

		BOOL "p2",
	}
	returns	"void"

native "WAYPOINT_PLAYBACK_USE_DEFAULT_SPEED"
	hash "0x6599D834B12D0800"
	jhash (0x1BBB2CAC)
	arguments {
		Any "p0",
	}
	returns	"void"

native "USE_WAYPOINT_RECORDING_AS_ASSISTED_MOVEMENT_ROUTE"
	hash "0x5A353B8E6B1095B5"
	jhash (0x4DFD5FEC)
	arguments {
		AnyPtr "p0",

		BOOL "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "WAYPOINT_PLAYBACK_START_AIMING_AT_PED"
	hash "0x20E330937C399D29"
	jhash (0x75E60CF6)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "WAYPOINT_PLAYBACK_START_AIMING_AT_COORD"
	hash "0x8968400D900ED8B3"
	jhash (0xF120A34E)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",
	}
	returns	"void"

native "0xE70BA7B90F8390DC"
	hash "0xE70BA7B90F8390DC"
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",

		Any "p3",
	}
	returns	"void"

native "WAYPOINT_PLAYBACK_START_SHOOTING_AT_COORD"
	hash "0x057A25CFCC9DB671"
	jhash (0xCDDB44D5)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",

		Any "p5",
	}
	returns	"void"

native "WAYPOINT_PLAYBACK_STOP_AIMING_OR_SHOOTING"
	hash "0x47EFA040EBB8E2EA"
	jhash (0x6D7CF40C)
	arguments {
		Any "p0",
	}
	returns	"void"

native "ASSISTED_MOVEMENT_REQUEST_ROUTE"
	hash "0x817268968605947A"
	jhash (0x48262EDA)
	arguments {
		charPtr "route",
	}
	returns	"Any"

native "ASSISTED_MOVEMENT_REMOVE_ROUTE"
	hash "0x3548536485DD792B"
	jhash (0xB3CEC06F)
	arguments {
		charPtr "route",
	}
	returns	"void"

native "ASSISTED_MOVEMENT_IS_ROUTE_LOADED"
	hash "0x60F9A4393A21F741"
	jhash (0x79B067AF)
	arguments {
		charPtr "route",
	}
	returns	"BOOL"

native "ASSISTED_MOVEMENT_SET_ROUTE_PROPERTIES"
	hash "0xD5002D78B7162E1B"
	jhash (0x01CAAFCC)
	arguments {
		charPtr "route",

		int "props",
	}
	returns	"void"

native "ASSISTED_MOVEMENT_OVERRIDE_LOAD_DISTANCE_THIS_FRAME"
	hash "0x13945951E16EF912"
	jhash (0x8FB923EC)
	arguments {
		float "dist",
	}
	returns	"void"

native "TASK_VEHICLE_FOLLOW_WAYPOINT_RECORDING"
	hash "0x3123FAA6DB1CF7ED"
	jhash (0x959818B6)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		float "p7",

		BOOL "p8",

		float "p9",
	}
	returns	"void"

native "IS_WAYPOINT_PLAYBACK_GOING_ON_FOR_VEHICLE"
	hash "0xF5134943EA29868C"
	jhash (0x80DD15DB)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "GET_VEHICLE_WAYPOINT_PROGRESS"
	hash "0x9824CFF8FC66E159"
	jhash (0xD3CCF64E)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_VEHICLE_WAYPOINT_TARGET_POINT"
	hash "0x416B62AC8B9E5BBD"
	jhash (0x81049608)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "VEHICLE_WAYPOINT_PLAYBACK_PAUSE"
	hash "0x8A4E6AC373666BC5"
	jhash (0x7C00B415)
	arguments {
		Any "p0",
	}
	returns	"void"

native "VEHICLE_WAYPOINT_PLAYBACK_RESUME"
	hash "0xDC04FCAA7839D492"
	jhash (0xBEB14C82)
	arguments {
		Any "p0",
	}
	returns	"void"

native "VEHICLE_WAYPOINT_PLAYBACK_USE_DEFAULT_SPEED"
	hash "0x5CEB25A7D2848963"
	jhash (0x923C3AA4)
	arguments {
		Any "p0",
	}
	returns	"void"

native "VEHICLE_WAYPOINT_PLAYBACK_OVERRIDE_SPEED"
	hash "0x121F0593E0A431D7"
	jhash (0xBE1E7BB4)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	I cant believe I have to define this, this is one of the best natives.

	It makes the ped ignore basically all shocking events around it. Occasionally the ped may comment or gesture, but other than that they just continue their daily activities. This includes shooting and wounding the ped. And - most importantly - they do not flee.
</summary>
]]--
native "TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS"
	hash "0x90D2156198831D69"
	jhash (0x1B54FB6B)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	p2 always false
</summary>
]]--
native "TASK_FORCE_MOTION_STATE"
	hash "0x4F056E1AFFEF17AB"
	jhash (0xCAD2EF77)
	arguments {
		Ped "ped",

		Hash "state",

		BOOL "p2",
	}
	returns	"void"

native "0x2D537BA194896636"
	hash "0x2D537BA194896636"
	jhash (0x6F5D215F)
	arguments {
		Any "p0",

		AnyPtr "p1",

		float "p2",

		BOOL "p3",

		AnyPtr "p4",

		Any "p5",
	}
	returns	"void"

native "0xD5B35BEA41919ACB"
	hash "0xD5B35BEA41919ACB"
	jhash (0x71A5C5DB)
	arguments {
		Any "p0",

		AnyPtr "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		Any "p8",

		float "p9",

		BOOL "p10",

		AnyPtr "p11",

		Any "p12",
	}
	returns	"void"

native "0x921CE12C489C4C41"
	hash "0x921CE12C489C4C41"
	jhash (0x902656EB)
	arguments {
		int "PlayerID",
	}
	returns	"BOOL"

native "0x30ED88D5E0C56A37"
	hash "0x30ED88D5E0C56A37"
	jhash (0x92FDBAE6)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xD01015C7316AE176"
	hash "0xD01015C7316AE176"
	jhash (0x885724DE)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0xAB13A5565480B6D9"
	hash "0xAB13A5565480B6D9"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x717E4D1F2048376D"
	hash "0x717E4D1F2048376D"
	jhash (0x96C0277B)
	arguments {
		Ped "p0",
	}
	returns	"charPtr"

native "0xD5BB4025AE449A4E"
	hash "0xD5BB4025AE449A4E"
	jhash (0xA79BE783)
	arguments {
		Any "p0",

		AnyPtr "p1",

		float "p2",
	}
	returns	"void"

native "0xB0A6CFD2C69C1088"
	hash "0xB0A6CFD2C69C1088"
	jhash (0xF3538041)
	arguments {
		Any "p0",

		AnyPtr "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xA7FFBA498E4AAF67"
	hash "0xA7FFBA498E4AAF67"
	jhash (0x1EBB6F3D)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0xB4F47213DF45A64C"
	hash "0xB4F47213DF45A64C"
	jhash (0x72FA5EF2)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "IS_MOVE_BLEND_RATIO_STILL"
	hash "0x349CE7B56DAFD95C"
	jhash (0xE9DAF877)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_MOVE_BLEND_RATIO_WALKING"
	hash "0xF133BBBE91E1691F"
	jhash (0xD21639A8)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_MOVE_BLEND_RATIO_RUNNING"
	hash "0xD4D8636C0199A939"
	jhash (0xE76A2353)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_MOVE_BLEND_RATIO_SPRINTING"
	hash "0x24A2AD74FA9814E2"
	jhash (0xDD616893)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_STILL"
	hash "0xAC29253EEF8F0180"
	jhash (0x09E3418D)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_WALKING"
	hash "0xDE4C184B2B9B071A"
	jhash (0x4B865C4A)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_RUNNING"
	hash "0xC5286FFC176F28A2"
	jhash (0xE9A5578F)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_SPRINTING"
	hash "0x57E457CD2C0FC168"
	jhash (0x4F3E0633)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_STRAFING"
	hash "0xE45B7F222DE47E09"
	jhash (0xEFEED13C)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "TASK_SYNCHRONIZED_SCENE"
	hash "0xEEA929141F699854"
	jhash (0x4F217E7B)
	arguments {
		Any "p0",

		Any "scene",

		charPtr "animDictionary",

		charPtr "animationName",

		float "p4",

		float "p5",

		Any "p6",

		Any "p7",

		float "p8",

		Any "p9",
	}
	returns	"void"

--[[!
<summary>
	This function is called on peds in vehicles.

	anim: animation name
	p2, p3, p4: "sweep_low", "sweep_med" or "sweep_high"
	p5: no idea what it does but is usually -1
</summary>
]]--
native "TASK_SWEEP_AIM_ENTITY"
	hash "0x2047C02158D6405A"
	jhash (0x4D210467)
	arguments {
		Ped "ped",

		charPtr "anim",

		charPtr "p2",

		charPtr "p3",

		charPtr "p4",

		int "p5",

		Vehicle "vehicle",

		float "p7",

		float "p8",
	}
	returns	"void"

native "UPDATE_TASK_SWEEP_AIM_ENTITY"
	hash "0xE4973DBDBE6E44B3"
	jhash (0xF65F0F4F)
	arguments {
		Ped "ped",

		Entity "entity",
	}
	returns	"void"

native "TASK_SWEEP_AIM_POSITION"
	hash "0x7AFE8FDC10BC07D2"
	jhash (0x1683FE66)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",

		AnyPtr "p4",

		Any "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",
	}
	returns	"void"

native "UPDATE_TASK_SWEEP_AIM_POSITION"
	hash "0xBB106883F5201FC4"
	jhash (0x6345EC80)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

--[[!
<summary>
	Example from "me_amanda1.ysc.c4":
	AI::TASK_ARREST_PED(l_19F /* This is a Ped */ , PLAYER::PLAYER_PED_ID());

	Example from "armenian1.ysc.c4":
	if (!PED::IS_PED_INJURED(l_B18[0/*1*/])) {
	    AI::TASK_ARREST_PED(l_B18[0/*1*/], PLAYER::PLAYER_PED_ID());
	}

	I would love to have time to experiment to see if a player Ped can arrest another Ped. Might make for a good cop mod.


</summary>
]]--
native "TASK_ARREST_PED"
	hash "0xF3B9A78A178572B1"
	jhash (0xBC0F153D)
	arguments {
		Ped "ped",

		Ped "target",
	}
	returns	"void"

native "IS_PED_RUNNING_ARREST_TASK"
	hash "0x3DC52677769B4AE0"
	jhash (0x6942DB7A)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "IS_PED_BEING_ARRESTED"
	hash "0x90A09F3A45FED688"
	jhash (0x5FF6C2ED)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "UNCUFF_PED"
	hash "0x67406F2C8F87FC4F"
	jhash (0xA23A1D61)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "IS_PED_CUFFED"
	hash "0x74E559B3BC910685"
	jhash (0x511CE741)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "GET_ALLOCATED_STACK_SIZE"
	hash "0x8B3CA62B1EF19B62"
	jhash (0x4E9CA30A)
	returns	"int"

native "_GET_FREE_STACK_SLOTS_COUNT"
	hash "0xFEAD16FC8F9DFC0F"
	jhash (0x11A178B8)
	arguments {
		int "stackSize",
	}
	returns	"int"

native "SET_RANDOM_SEED"
	hash "0x444D98F98C11F3EC"
	jhash (0xDB3FEB5C)
	arguments {
		int "time",
	}
	returns	"void"

--[[!
<summary>
	Maximum value is 1.
	At a value of 0 the game will still run at a minimum time scale.

	Slow Motion 1: 0.6
	Slow Motion 2: 0.4
	Slow Motion 3: 0.2
</summary>
]]--
native "SET_TIME_SCALE"
	hash "0x1D408577D440E81E"
	jhash (0xA7F84694)
	arguments {
		float "time",
	}
	returns	"void"

native "SET_MISSION_FLAG"
	hash "0xC4301E5121A0ED73"
	jhash (0x57592D52)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "GET_MISSION_FLAG"
	hash "0xA33CDCCDA663159E"
	jhash (0x95115F97)
	returns	"BOOL"

native "SET_RANDOM_EVENT_FLAG"
	hash "0x971927086CFD2158"
	jhash (0xA77F31E8)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "GET_RANDOM_EVENT_FLAG"
	hash "0xD2D57F1D764117B1"
	jhash (0x794CC92C)
	returns	"Any"

native "0x24DA7D7667FD7B09"
	hash "0x24DA7D7667FD7B09"
	returns	"Any"

native "0x4DCDF92BF64236CD"
	hash "0x4DCDF92BF64236CD"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x31125FD509D9043F"
	hash "0x31125FD509D9043F"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xEBD3205A207939ED"
	hash "0xEBD3205A207939ED"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x97E7E2C04245115B"
	hash "0x97E7E2C04245115B"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xEB078CA2B5E82ADD"
	hash "0xEB078CA2B5E82ADD"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x703CC7F60CBB2B57"
	hash "0x703CC7F60CBB2B57"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x8951EB9C6906D3C8"
	hash "0x8951EB9C6906D3C8"
	returns	"void"

native "0xBA4B8D83BDC75551"
	hash "0xBA4B8D83BDC75551"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xE8B9C0EC9E183F35"
	hash "0xE8B9C0EC9E183F35"
	returns	"Any"

native "0x65D2EBB47E1CEC21"
	hash "0x65D2EBB47E1CEC21"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x6F2135B6129620C1"
	hash "0x6F2135B6129620C1"
	jhash (0x8B2DE971)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x8D74E26F54B4E5C3"
	hash "0x8D74E26F54B4E5C3"
	jhash (0xE77199F7)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xB335F761606DB47C"
	hash "0xB335F761606DB47C"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",

		BOOL "p3",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns weather name hash
</summary>
]]--
native "_GET_PREV_WEATHER_TYPE"
	hash "0x564B884A05EC45A3"
	jhash (0xA8171E9E)
	returns	"Hash"

--[[!
<summary>
	Returns weather name hash
</summary>
]]--
native "_GET_NEXT_WEATHER_TYPE"
	hash "0x711327CD09C8F162"
	jhash (0x353E8056)
	returns	"Hash"

native "IS_PREV_WEATHER_TYPE"
	hash "0x44F28F86433B10A9"
	jhash (0x250ADA61)
	arguments {
		charPtr "weatherType",
	}
	returns	"BOOL"

native "IS_NEXT_WEATHER_TYPE"
	hash "0x2FAA3A30BEC0F25D"
	jhash (0x99CB167F)
	arguments {
		charPtr "weatherType",
	}
	returns	"BOOL"

--[[!
<summary>
	The following weatherTypes are used in the scripts:
	"CLEAR"
	"EXTRASUNNY"
	"CLOUDS"
	"OVERCAST"
	"RAIN"
	"CLEARING"
	"THUNDER"
	"SMOG"
	"FOGGY"
	"XMAS"
	"SNOWLIGHT"
	"BLIZZARD"
</summary>
]]--
native "SET_WEATHER_TYPE_PERSIST"
	hash "0x704983DF373B198F"
	jhash (0xC6C04C75)
	arguments {
		charPtr "weatherType",
	}
	returns	"void"

--[[!
<summary>
	The following weatherTypes are used in the scripts:
	"CLEAR"
	"EXTRASUNNY"
	"CLOUDS"
	"OVERCAST"
	"RAIN"
	"CLEARING"
	"THUNDER"
	"SMOG"
	"FOGGY"
	"XMAS"
	"SNOWLIGHT"
	"BLIZZARD"
</summary>
]]--
native "SET_WEATHER_TYPE_NOW_PERSIST"
	hash "0xED712CA327900C8A"
	jhash (0xC869FE97)
	arguments {
		charPtr "weatherType",
	}
	returns	"void"

--[[!
<summary>
	The following weatherTypes are used in the scripts:
	"CLEAR"
	"EXTRASUNNY"
	"CLOUDS"
	"OVERCAST"
	"RAIN"
	"CLEARING"
	"THUNDER"
	"SMOG"
	"FOGGY"
	"XMAS"
	"SNOWLIGHT"
	"BLIZZARD"
</summary>
]]--
native "SET_WEATHER_TYPE_NOW"
	hash "0x29B487C359E19889"
	jhash (0x361E9EAC)
	arguments {
		charPtr "weatherType",
	}
	returns	"void"

native "_SET_WEATHER_TYPE_OVER_TIME"
	hash "0xFB5045B7C42B75BF"
	jhash (0x386F0D25)
	arguments {
		charPtr "weatherType",

		float "time",
	}
	returns	"void"

native "SET_RANDOM_WEATHER_TYPE"
	hash "0x8B05F884CF7E8020"
	jhash (0xE7AA1BC9)
	returns	"void"

native "CLEAR_WEATHER_TYPE_PERSIST"
	hash "0xCCC39339BEF76CF5"
	jhash (0x6AB757D8)
	returns	"void"

native "_GET_WEATHER_TYPE_TRANSITION"
	hash "0xF3BBE884A14BB413"
	jhash (0x9A5C1D56)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		floatPtr "progress_or_time",
	}
	returns	"void"

--[[!
<summary>
	I think this uses hashes as in parameters
</summary>
]]--
native "_SET_WEATHER_TYPE_TRANSITION"
	hash "0x578C752848ECFA0C"
	jhash (0x5CA74040)
	arguments {
		Any "p0",

		Any "p1",

		float "time",
	}
	returns	"void"

native "SET_OVERRIDE_WEATHER"
	hash "0xA43D5C6FE51ADBEF"
	jhash (0xD9082BB5)
	arguments {
		charPtr "weatherType",
	}
	returns	"void"

native "CLEAR_OVERRIDE_WEATHER"
	hash "0x338D2E3477711050"
	jhash (0x7740EA4E)
	returns	"void"

native "0xB8F87EAD7533B176"
	hash "0xB8F87EAD7533B176"
	jhash (0x625181DC)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xC3EAD29AB273ECE8"
	hash "0xC3EAD29AB273ECE8"
	jhash (0xBEBBFDC8)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xA7A1127490312C36"
	hash "0xA7A1127490312C36"
	jhash (0x6926AB03)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x31727907B2C43C55"
	hash "0x31727907B2C43C55"
	jhash (0xD447439D)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x405591EC8FD9096D"
	hash "0x405591EC8FD9096D"
	jhash (0x584E9C59)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xF751B16FB32ABC1D"
	hash "0xF751B16FB32ABC1D"
	jhash (0x5656D578)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xB3E6360DDE733E82"
	hash "0xB3E6360DDE733E82"
	jhash (0x0DE40C28)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x7C9C0B1EEB1F9072"
	hash "0x7C9C0B1EEB1F9072"
	jhash (0x98C9138B)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x6216B116083A7CB4"
	hash "0x6216B116083A7CB4"
	jhash (0xFB1A9CDE)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x9F5E6BB6B34540DA"
	hash "0x9F5E6BB6B34540DA"
	jhash (0x1C0CAE89)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xB9854DFDE0D833D6"
	hash "0xB9854DFDE0D833D6"
	jhash (0x4671AC2E)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xC54A08C85AE4D410"
	hash "0xC54A08C85AE4D410"
	jhash (0xDA02F415)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xA8434F1DFF41D6E7"
	hash "0xA8434F1DFF41D6E7"
	jhash (0x5F3DDEC0)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xC3C221ADDDE31A11"
	hash "0xC3C221ADDDE31A11"
	jhash (0x63A89684)
	arguments {
		float "p0",
	}
	returns	"void"

native "SET_WIND"
	hash "0xAC3A74E8384A9919"
	jhash (0xC6294698)
	arguments {
		float "p0",
	}
	returns	"void"

native "SET_WIND_SPEED"
	hash "0xEE09ECEDBABE47FC"
	jhash (0x45705F94)
	arguments {
		float "speed",
	}
	returns	"void"

native "GET_WIND_SPEED"
	hash "0xA8CF1CC0AFCD3F12"
	jhash (0x098F0F3C)
	returns	"float"

native "SET_WIND_DIRECTION"
	hash "0xEB0F4468467B4528"
	jhash (0x381AEEE9)
	arguments {
		float "direction",
	}
	returns	"void"

native "GET_WIND_DIRECTION"
	hash "0x1F400FEF721170DA"
	jhash (0x89499A0D)
	returns	"Vector3"

--[[!
<summary>
	puddles, rain fx on ground/buildings/puddles, rain sound
</summary>
]]--
native "_SET_RAIN_FX_INTENSITY"
	hash "0x643E26EA6E024D92"
	arguments {
		float "intensity",
	}
	returns	"void"

native "GET_RAIN_LEVEL"
	hash "0x96695E368AD855F3"
	jhash (0xC9F67F28)
	returns	"Any"

native "GET_SNOW_LEVEL"
	hash "0xC5868A966E5BE3AE"
	jhash (0x1B09184F)
	returns	"Any"

--[[!
<summary>
	creates single lightning+thunder at random position
</summary>
]]--
native "_CREATE_LIGHTNING_THUNDER"
	hash "0xF6062E089251C898"
	jhash (0xDF38165E)
	returns	"void"

native "0x02DEAAC8F8EA7FE7"
	hash "0x02DEAAC8F8EA7FE7"
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x11B56FBBF7224868"
	hash "0x11B56FBBF7224868"
	jhash (0x8727A4C5)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

--[[!
<summary>
	cloud hat types
	-----------------------
	Cloudy 01
	RAIN
	horizonband1
	horizonband2
	Puffs
	Wispy
	Horizon
	Stormy 01
	Clear 01
	Snowy 01
	Contrails
	altostratus
	Nimbus
	Cirrus
	cirrocumulus
	stratoscumulus
	horizonband3
	Stripey
	horsey
	shower
</summary>
]]--
native "_SET_CLOUD_HAT_TRANSITION"
	hash "0xFC4842A34657BFCB"
	jhash (0xED88FC61)
	arguments {
		charPtr "type",

		float "ms",
	}
	returns	"void"

native "0xA74802FB8D0B7814"
	hash "0xA74802FB8D0B7814"
	jhash (0xC9FA6E07)
	arguments {
		AnyPtr "p0",

		float "p1",
	}
	returns	"void"

native "_CLEAR_CLOUD_HAT"
	hash "0x957E790EA1727B64"
	jhash (0x2D7787BC)
	returns	"void"

native "0xF36199225D6D8C86"
	hash "0xF36199225D6D8C86"
	arguments {
		float "p0",
	}
	returns	"void"

native "0x20AC25E781AE4A84"
	hash "0x20AC25E781AE4A84"
	returns	"float"

native "GET_GAME_TIMER"
	hash "0x9CD27B0045628463"
	jhash (0xA4EA0691)
	returns	"Any"

native "GET_FRAME_TIME"
	hash "0x15C40837039FFAF7"
	jhash (0x96374262)
	returns	"float"

native "0xE599A503B3837E1B"
	hash "0xE599A503B3837E1B"
	returns	"float"

native "GET_FRAME_COUNT"
	hash "0xFC8202EFC642E6F2"
	jhash (0xB477A015)
	returns	"int"

native "GET_RANDOM_FLOAT_IN_RANGE"
	hash "0x313CE5879CEB6FCD"
	jhash (0x0562C4D0)
	arguments {
		float "startRange",

		float "endRange",
	}
	returns	"float"

--[[!
<summary>
	Another extremely useful native.

	You can use it simply like:
	if (GAMEPLAY::GET_RANDOM_INT_IN_RANGE(0, 2))

	and the if-statement will count it as false only if the random int is 0. That means there is a one in three chance of it being false. Put a "!" in front and it means there is a one in three chance of it being true.
</summary>
]]--
native "GET_RANDOM_INT_IN_RANGE"
	hash "0xD53343AA4FB7DD28"
	jhash (0x4051115B)
	arguments {
		int "startRange",

		int "endRange",
	}
	returns	"int"

--[[!
<summary>
	Gets the ground elevation at the specified position. Note that if the specified position is below ground level, the function will output zero!

	x: Position on the X-axis to get ground elevation at.
	y: Position on the Y-axis to get ground elevation at.
	z: Position on the Z-axis to get ground elevation at.
	groundZ: The ground elevation at the specified position.
</summary>
]]--
native "GET_GROUND_Z_FOR_3D_COORD"
	hash "0xC906A7DAB05C8D2B"
	jhash (0xA1BFD5E0)
	arguments {
		float "x",

		float "y",

		float "z",

		floatPtr "groundZ",
	}
	returns	"BOOL"

native "0x8BDC7BFC57A81E76"
	hash "0x8BDC7BFC57A81E76"
	jhash (0x64D91CED)
	arguments {
		float "X",

		float "Y",

		float "Zz",

		Any "p3",

		Any "p4",
	}
	returns	"Any"

native "ASIN"
	hash "0xC843060B5765DCE7"
	jhash (0x998E5CAD)
	arguments {
		float "p0",
	}
	returns	"float"

native "ACOS"
	hash "0x1D08B970013C34B6"
	jhash (0xF4038776)
	arguments {
		float "p0",
	}
	returns	"float"

native "TAN"
	hash "0x632106CC96E82E91"
	jhash (0xD320CE5E)
	arguments {
		float "p0",
	}
	returns	"float"

native "ATAN"
	hash "0xA9D1795CD5043663"
	jhash (0x7A03CC8E)
	arguments {
		float "p0",
	}
	returns	"float"

native "ATAN2"
	hash "0x8927CBF9D22261A4"
	jhash (0x2508AC81)
	arguments {
		float "p0",

		float "p1",
	}
	returns	"float"

native "GET_DISTANCE_BETWEEN_COORDS"
	hash "0xF1B760881820C952"
	jhash (0xF698765E)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		BOOL "unknown",
	}
	returns	"float"

native "GET_ANGLE_BETWEEN_2D_VECTORS"
	hash "0x186FC4BE848E1C92"
	jhash (0xDBF75E58)
	arguments {
		float "x1",

		float "y1",

		float "x2",

		float "y2",
	}
	returns	"float"

--[[!
<summary>
	dx = x1 
	dy = y1 
</summary>
]]--
native "GET_HEADING_FROM_VECTOR_2D"
	hash "0x2FFB6B224F4B2926"
	jhash (0xD209D52B)
	arguments {
		float "dx",

		float "dy",
	}
	returns	"float"

native "0x7F8F6405F4777AF6"
	hash "0x7F8F6405F4777AF6"
	jhash (0x89459F0A)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		BOOL "p9",
	}
	returns	"float"

native "0x21C235BC64831E5A"
	hash "0x21C235BC64831E5A"
	jhash (0xCAECF37E)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		BOOL "p9",
	}
	returns	"Vector3"

native "0xF56DFB7B61BE7276"
	hash "0xF56DFB7B61BE7276"
	jhash (0xC6CC812C)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",

		float "p11",

		AnyPtr "p12",
	}
	returns	"BOOL"

native "SET_BIT"
	hash "0x933D6A9EEC1BACD0"
	jhash (0x4EFE7E6B)
	arguments {
		intPtr "var",

		int "bit",
	}
	returns	"Any"

native "CLEAR_BIT"
	hash "0xE80492A9AC099A93"
	jhash (0x8BC9E618)
	arguments {
		intPtr "var",

		int "bit",
	}
	returns	"Any"

--[[!
<summary>
	the number one most useful native

	It looks like Rockstar put this native in a wrapper called "$" because they used it so much in the scripts. You can do it too, like this:

	Hash $(char* string){ return GAMEPLAY::GET_HASH_KEY(string);}

	Then just call it like so:
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(1, $("COP"), $("PLAYER"));
</summary>
]]--
native "GET_HASH_KEY"
	hash "0xD24D37CC275948CC"
	jhash (0x98EFF6F1)
	arguments {
		charPtr "value",
	}
	returns	"Hash"

native "0xF2F6A2FA49278625"
	hash "0xF2F6A2FA49278625"
	jhash (0x87B92190)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		AnyPtr "p9",

		AnyPtr "p10",

		AnyPtr "p11",

		AnyPtr "p12",
	}
	returns	"void"

native "IS_AREA_OCCUPIED"
	hash "0xA61B4DF533DCB56E"
	jhash (0xC013972F)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",

		BOOL "p7",

		BOOL "p8",

		BOOL "p9",

		BOOL "p10",

		Any "p11",

		BOOL "p12",
	}
	returns	"BOOL"

native "IS_POSITION_OCCUPIED"
	hash "0xADCDE75E1C60F32D"
	jhash (0x452E8D9E)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",

		BOOL "p5",

		BOOL "p6",

		BOOL "p7",

		BOOL "p8",

		Any "p9",

		BOOL "p10",
	}
	returns	"BOOL"

native "IS_POINT_OBSCURED_BY_A_MISSION_ENTITY"
	hash "0xE54E209C35FFA18D"
	jhash (0xC161558D)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		Any "p6",
	}
	returns	"BOOL"

native "CLEAR_AREA"
	hash "0xA56F01F3765B93A0"
	jhash (0x854E9AB8)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",

		BOOL "p5",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"void"

--[[!
<summary>
	GAMEPLAY::_0x957838AAF91BD12D(x, y, z, radius, false, false, false, false); seem to make all objects go away, peds, vehicles etc. All booleans set to true doesn't seem to change anything. 
</summary>
]]--
native "0x957838AAF91BD12D"
	hash "0x957838AAF91BD12D"
	jhash (0x20E4FFD9)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		BOOL "p4",

		BOOL "p5",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"void"

--[[!
<summary>
	Example: 		CLEAR_AREA_OF_VEHICLES(0, 0, 0, 10000, false, false, false, false, false);
</summary>
]]--
native "CLEAR_AREA_OF_VEHICLES"
	hash "0x01C7B9B38428AEB6"
	jhash (0x63320F3C)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		BOOL "p4",

		BOOL "p5",

		BOOL "p6",

		BOOL "p7",

		BOOL "p8",
	}
	returns	"void"

native "CLEAR_ANGLED_AREA_OF_VEHICLES"
	hash "0x11DB3500F042A8AA"
	jhash (0xF11A3018)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		BOOL "p7",

		BOOL "p8",

		BOOL "p9",

		BOOL "p10",

		BOOL "p11",
	}
	returns	"void"

--[[!
<summary>
	I didn't change it as I don't know the correct prefix, but I doubt it's a bool as it is now. I just looked through the PC scripts that this site provides you with a link to find. It shows the last param mainly uses, (0, 2, 6, 16, and 17). So I doubt it's a bool. As well as it doesn't use a ampersand so it wouldn't be a int*. It might, be a int. If anyone could please confirm.
</summary>
]]--
native "CLEAR_AREA_OF_OBJECTS"
	hash "0xDD9B9B385AAC7F5B"
	jhash (0xBB720FE7)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		BOOL "unk",
	}
	returns	"void"

--[[!
<summary>
	Example: 		CLEAR_AREA_OF_PEDS(0, 0, 0, 10000, true);
</summary>
]]--
native "CLEAR_AREA_OF_PEDS"
	hash "0xBE31FD6CE464AC59"
	jhash (0x25BE7FA8)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		BOOL "unk",
	}
	returns	"void"

native "CLEAR_AREA_OF_COPS"
	hash "0x04F8FC8FCF58F88D"
	jhash (0x95C53824)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		BOOL "unk",
	}
	returns	"void"

--[[!
<summary>
	unk is usually 0 in the scripts.
</summary>
]]--
native "CLEAR_AREA_OF_PROJECTILES"
	hash "0x0A1CB9094635D1A6"
	jhash (0x18DB5434)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		BOOL "unk",
	}
	returns	"void"

native "0x7EC6F9A478A6A512"
	hash "0x7EC6F9A478A6A512"
	returns	"void"

native "SET_SAVE_MENU_ACTIVE"
	hash "0xC9BF75D28165FF77"
	jhash (0xF5CCF164)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x397BAA01068BAA96"
	hash "0x397BAA01068BAA96"
	jhash (0x39771F21)
	returns	"Any"

native "SET_CREDITS_ACTIVE"
	hash "0xB938B7E6D3C0620C"
	jhash (0xEC2A0ECF)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0xB51B9AB9EF81868C"
	hash "0xB51B9AB9EF81868C"
	jhash (0x75B06B5A)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x075F1D57402C93BA"
	hash "0x075F1D57402C93BA"
	jhash (0x2569C9A7)
	returns	"Any"

native "TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME"
	hash "0x9DC711BC69C548DF"
	jhash (0x9F861FD4)
	arguments {
		charPtr "scriptName",
	}
	returns	"void"

--[[!
<summary>
	Useless when not using a *.ysc script.
</summary>
]]--
native "NETWORK_SET_SCRIPT_IS_SAFE_FOR_NETWORK_GAME"
	hash "0x9243BAC96D64C050"
	jhash (0x878486CE)
	returns	"void"

--[[!
<summary>
	Returns the index of the newly created hospital spawn point.

	p3 might be radius?


</summary>
]]--
native "ADD_HOSPITAL_RESTART"
	hash "0x1F464EF988465A81"
	jhash (0x4F3E3104)
	arguments {
		float "x",

		float "y",

		float "z",

		float "p3",

		Any "p4",
	}
	returns	"int"

--[[!
<summary>
	The game by default has 5 hospital respawn points. Disabling them all will cause the player to respawn at the last position they were.


</summary>
]]--
native "DISABLE_HOSPITAL_RESTART"
	hash "0xC8535819C450EBA8"
	jhash (0x09F49C72)
	arguments {
		int "hospitalIndex",

		BOOL "toggle",
	}
	returns	"void"

native "ADD_POLICE_RESTART"
	hash "0x452736765B31FC4B"
	jhash (0xE96C29FE)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",
	}
	returns	"Any"

native "DISABLE_POLICE_RESTART"
	hash "0x23285DED6EBD7EA3"
	jhash (0x0A280324)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x706B5EDCAA7FA663"
	hash "0x706B5EDCAA7FA663"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0xA2716D40842EAF79"
	hash "0xA2716D40842EAF79"
	returns	"void"

native "_DISABLE_AUTOMATIC_RESPAWN"
	hash "0x2C2B3493FBF51C71"
	jhash (0x296574AE)
	arguments {
		BOOL "disableRespawn",
	}
	returns	"void"

native "IGNORE_NEXT_RESTART"
	hash "0x21FFB63D8C615361"
	jhash (0xDA13A4B6)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Sets whether the game should fade out after the player dies.
</summary>
]]--
native "SET_FADE_OUT_AFTER_DEATH"
	hash "0x4A18E01DF2C87B86"
	jhash (0xC9F6F0BC)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Sets whether the game should fade out after the player is arrested.
</summary>
]]--
native "SET_FADE_OUT_AFTER_ARREST"
	hash "0x1E0B4DC0D990A4E7"
	jhash (0xCB074B9D)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Sets whether the game should fade in after the player dies or is arrested.
</summary>
]]--
native "SET_FADE_IN_AFTER_DEATH_ARREST"
	hash "0xDA66D2796BA33F12"
	jhash (0xACDE6985)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "SET_FADE_IN_AFTER_LOAD"
	hash "0xF3D78F59DFE18D79"
	jhash (0x6E00EB0B)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "REGISTER_SAVE_HOUSE"
	hash "0xC0714D0A7EEECA54"
	jhash (0x39C1849A)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		AnyPtr "p4",

		Any "p5",

		Any "p6",
	}
	returns	"Any"

--[[!
<summary>
	Both p1 &amp; p2 were always true in scripts. 
</summary>
]]--
native "SET_SAVE_HOUSE"
	hash "0x4F548CABEAE553BC"
	jhash (0xC3240BB4)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "OVERRIDE_SAVE_HOUSE"
	hash "0x1162EA8AE9D24EEA"
	jhash (0x47436C12)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",
	}
	returns	"Any"

native "0xA4A0065E39C9F25C"
	hash "0xA4A0065E39C9F25C"
	jhash (0xC4D71AB4)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"Any"

native "DO_AUTO_SAVE"
	hash "0x50EEAAD86232EE55"
	jhash (0x54C44B1A)
	returns	"void"

native "0x6E04F06094C87047"
	hash "0x6E04F06094C87047"
	jhash (0xA8546914)
	returns	"Any"

native "IS_AUTO_SAVE_IN_PROGRESS"
	hash "0x69240733738C19A0"
	jhash (0x36F75399)
	returns	"BOOL"

native "0x2107A3773771186D"
	hash "0x2107A3773771186D"
	jhash (0x78350773)
	returns	"Any"

native "0x06462A961E94B67C"
	hash "0x06462A961E94B67C"
	jhash (0x5A45B11A)
	returns	"void"

native "BEGIN_REPLAY_STATS"
	hash "0xE0E500246FF73D66"
	jhash (0x17F4F44D)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x69FE6DC87BD2A5E9"
	hash "0x69FE6DC87BD2A5E9"
	jhash (0x81216EE0)
	arguments {
		Any "p0",
	}
	returns	"void"

native "END_REPLAY_STATS"
	hash "0xA23E821FBDF8A5F2"
	jhash (0xCB570185)
	returns	"void"

native "0xD642319C54AADEB6"
	hash "0xD642319C54AADEB6"
	jhash (0xC58250F1)
	returns	"Any"

native "0x5B1F2E327B6B6FE1"
	hash "0x5B1F2E327B6B6FE1"
	jhash (0x50C39926)
	returns	"Any"

native "0x2B626A0150E4D449"
	hash "0x2B626A0150E4D449"
	jhash (0x710E5D1E)
	returns	"Any"

native "0xDC9274A7EF6B2867"
	hash "0xDC9274A7EF6B2867"
	jhash (0xC7BD1AF0)
	returns	"Any"

native "0x8098C8D6597AAE18"
	hash "0x8098C8D6597AAE18"
	jhash (0x22BE2423)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "CLEAR_REPLAY_STATS"
	hash "0x1B1AB132A16FDA55"
	jhash (0xC47DFF02)
	returns	"void"

native "0x72DE52178C291CB5"
	hash "0x72DE52178C291CB5"
	jhash (0xF62B3C48)
	returns	"Any"

native "0x44A0BDC559B35F6E"
	hash "0x44A0BDC559B35F6E"
	jhash (0x3589452B)
	returns	"Any"

native "0xEB2104E905C6F2E9"
	hash "0xEB2104E905C6F2E9"
	returns	"Any"

native "0x2B5E102E4A42F2BF"
	hash "0x2B5E102E4A42F2BF"
	jhash (0x144AAF22)
	returns	"Any"

native "IS_MEMORY_CARD_IN_USE"
	hash "0x8A75CE2956274ADD"
	jhash (0x40CE4DFD)
	returns	"BOOL"

native "SHOOT_SINGLE_BULLET_BETWEEN_COORDS"
	hash "0x867654CBC7606F2C"
	jhash (0xCB7415AC)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		int "damage",

		BOOL "p7",

		Hash "weaponHash",

		Ped "ownerPed",

		BOOL "isAudible",

		BOOL "isInvisible",

		float "speed",
	}
	returns	"void"

--[[!
<summary>
	only documented to be continued...
</summary>
]]--
native "0xE3A7742E0B7A2F8B"
	hash "0xE3A7742E0B7A2F8B"
	jhash (0x52ACCB7B)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		int "damage",

		BOOL "p7",

		Hash "weaponHash",

		Ped "ownerPed",

		BOOL "isNotAudible",

		BOOL "isInVisible",

		float "speed",

		Entity "entity",
	}
	returns	"void"

--[[!
<summary>
	Since latest patches has 18 parameters.
</summary>
]]--
native "0xBFE5756E7407064A"
	hash "0xBFE5756E7407064A"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",

		Any "p9",

		Any "p10",

		Any "p11",

		Any "p12",

		Any "p13",

		Any "p14",

		Any "p15",

		Any "p16",

		Any "p17",
	}
	returns	"void"

--[[!
<summary>
	Gets the dimensions of a model.

	Calculate (maximum - minimum) to get the size, in which case, Y will be how long the model is.
</summary>
]]--
native "GET_MODEL_DIMENSIONS"
	hash "0x03E8D3D5F549087A"
	jhash (0x91ABB8E0)
	arguments {
		Hash "modelHash",

		Vector3Ptr "minimum",

		Vector3Ptr "maximum",
	}
	returns	"void"

--[[!
<summary>
	Sets a visually fake wanted level on the user interface. Used by Rockstar's scripts to "override" regular wanted levels and make custom ones while the real wanted level and multipliers are ignored.
</summary>
]]--
native "SET_FAKE_WANTED_LEVEL"
	hash "0x1454F2448DE30163"
	jhash (0x85B1C9FA)
	arguments {
		int "fakeWantedLevel",
	}
	returns	"void"

native "0x4C9296CBCD1B971E"
	hash "0x4C9296CBCD1B971E"
	jhash (0x0022A430)
	returns	"Any"

native "IS_BIT_SET"
	hash "0xA921AA820C25702F"
	jhash (0x902E26AC)
	arguments {
		int "value",

		int "bit",
	}
	returns	"BOOL"

native "USING_MISSION_CREATOR"
	hash "0xF14878FC50BEC6EE"
	jhash (0x20AB0B6B)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0xDEA36202FC3382DF"
	hash "0xDEA36202FC3382DF"
	jhash (0x082BA6F2)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_MINIGAME_IN_PROGRESS"
	hash "0x19E00D7322C6F85B"
	jhash (0x348B9046)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "IS_MINIGAME_IN_PROGRESS"
	hash "0x2B4A15E44DE0F478"
	jhash (0x53A95E13)
	returns	"BOOL"

native "IS_THIS_A_MINIGAME_SCRIPT"
	hash "0x7B30F65D7B710098"
	jhash (0x7605EF6F)
	returns	"BOOL"

native "IS_SNIPER_INVERTED"
	hash "0x61A23B7EDA9BDA24"
	jhash (0x5C3BF51B)
	returns	"BOOL"

native "0xD3D15555431AB793"
	hash "0xD3D15555431AB793"
	jhash (0xBAF17315)
	returns	"Any"

native "GET_PROFILE_SETTING"
	hash "0xC488FF2356EA7791"
	jhash (0xD374BEBC)
	arguments {
		int "profileSetting",
	}
	returns	"int"

native "ARE_STRINGS_EQUAL"
	hash "0x0C515FAB3FF9EA92"
	jhash (0x877C0BC5)
	arguments {
		charPtr "string1",

		charPtr "string2",
	}
	returns	"BOOL"

--[[!
<summary>
	Compares two strings up to a specified number of characters.

	Parameters:
	str1 - String to be compared.
	str2 - String to be compared.
	matchCase - Comparison will be case-sensitive.
	maxLength - Maximum number of characters to compare. A value of -1 indicates an infinite length.

	Returns:
	A value indicating the relationship between the strings:
	&lt;0 - The first non-matching character in 'str1' is less than the one in 'str2'. (e.g. 'A' &lt; 'B', so result = -1)
	0 - The contents of both strings are equal.
	&gt;0 - The first non-matching character in 'str1' is less than the one in 'str2'. (e.g. 'B' &gt; 'A', so result = 1)

	Examples:
	GAMEPLAY::COMPARE_STRINGS("STRING", "string", false, -1); // 0; equal
	GAMEPLAY::COMPARE_STRINGS("TESTING", "test", false, 4); // 0; equal
	GAMEPLAY::COMPARE_STRINGS("R2D2", "R2xx", false, 2); // 0; equal
	GAMEPLAY::COMPARE_STRINGS("foo", "bar", false, -1); // 4; 'f' &gt; 'b'
	GAMEPLAY::COMPARE_STRINGS("A", "A", true, 1); // 0; equal

	When comparing case-sensitive strings, lower-case characters are greater than upper-case characters:
	GAMEPLAY::COMPARE_STRINGS("A", "a", true, 1); // -1; 'A' &lt; 'a'
	GAMEPLAY::COMPARE_STRINGS("a", "A", true, 1); // 1; 'a' &gt; 'A'

	~Fireboyd78
</summary>
]]--
native "COMPARE_STRINGS"
	hash "0x1E34710ECD4AB0EB"
	jhash (0xFE25A58F)
	arguments {
		charPtr "str1",

		charPtr "str2",

		BOOL "matchCase",

		int "maxLength",
	}
	returns	"int"

native "ABSI"
	hash "0xF0D31AD191A74F87"
	jhash (0xB44677C5)
	arguments {
		int "value",
	}
	returns	"int"

native "ABSF"
	hash "0x73D57CFFDD12C355"
	jhash (0xAF6F6E0B)
	arguments {
		float "value",
	}
	returns	"float"

native "IS_SNIPER_BULLET_IN_AREA"
	hash "0xFEFCF11B01287125"
	jhash (0x0483715C)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"BOOL"

native "IS_PROJECTILE_IN_AREA"
	hash "0x5270A8FBC098C3F8"
	jhash (0x78E1A557)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",
	}
	returns	"BOOL"

native "IS_PROJECTILE_TYPE_IN_AREA"
	hash "0x2E0DC353342C4A6D"
	jhash (0x2B73BCF6)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		Any "p6",

		BOOL "p7",
	}
	returns	"BOOL"

native "IS_PROJECTILE_TYPE_IN_ANGLED_AREA"
	hash "0xF0BC12401061DEA0"
	jhash (0xD1AE2681)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		Any "p7",

		BOOL "p8",
	}
	returns	"BOOL"

native "0x34318593248C8FB2"
	hash "0x34318593248C8FB2"
	jhash (0xBE81F1E2)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		Any "p3",

		float "p4",

		BOOL "p5",
	}
	returns	"BOOL"

native "0x8D7A43EC6A5FEA45"
	hash "0x8D7A43EC6A5FEA45"
	jhash (0x1A40454B)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",
	}
	returns	"Any"

native "0xDFB4138EEFED7B81"
	hash "0xDFB4138EEFED7B81"
	jhash (0x6BDE5CE4)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"Any"

--[[!
<summary>
	only documented to be continued...
</summary>
]]--
native "0x82FDE6A57EE4EE44"
	hash "0x82FDE6A57EE4EE44"
	jhash (0x507BC6F7)
	arguments {
		Ped "ped",

		Hash "weaponhash",

		float "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"Any"

native "IS_BULLET_IN_ANGLED_AREA"
	hash "0x1A8B5F3C01E2B477"
	jhash (0xE2DB58F7)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		BOOL "p7",
	}
	returns	"BOOL"

native "IS_BULLET_IN_AREA"
	hash "0x3F2023999AD51C1F"
	jhash (0xB54F46CA)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",
	}
	returns	"BOOL"

native "IS_BULLET_IN_BOX"
	hash "0xDE0F6D7450D37351"
	jhash (0xAB73ED26)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",
	}
	returns	"BOOL"

native "HAS_BULLET_IMPACTED_IN_AREA"
	hash "0x9870ACFB89A90995"
	jhash (0x902BC7D9)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",

		BOOL "p5",
	}
	returns	"BOOL"

native "HAS_BULLET_IMPACTED_IN_BOX"
	hash "0xDC8C5D7CFEAB8394"
	jhash (0x2C2618CC)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"BOOL"

native "IS_ORBIS_VERSION"
	hash "0xA72BC0B675B1519E"
	jhash (0x40282018)
	returns	"BOOL"

native "IS_DURANGO_VERSION"
	hash "0x4D982ADB1978442D"
	jhash (0x46FB06A5)
	returns	"BOOL"

native "IS_XBOX360_VERSION"
	hash "0xF6201B4DAF662A9D"
	jhash (0x24005CC8)
	returns	"BOOL"

native "IS_PS3_VERSION"
	hash "0xCCA1072C29D096C2"
	jhash (0x4C0D5303)
	returns	"BOOL"

native "IS_PC_VERSION"
	hash "0x48AF36444B965238"
	jhash (0x4D5D9EE3)
	returns	"BOOL"

native "IS_AUSSIE_VERSION"
	hash "0x9F1935CA1F724008"
	jhash (0x944BA1DC)
	returns	"BOOL"

native "IS_STRING_NULL"
	hash "0xF22B6C47C6EAB066"
	jhash (0x8E71E00F)
	arguments {
		charPtr "string",
	}
	returns	"BOOL"

native "IS_STRING_NULL_OR_EMPTY"
	hash "0xCA042B6957743895"
	jhash (0x42E9F2CA)
	arguments {
		charPtr "string",
	}
	returns	"BOOL"

--[[!
<summary>
	Useful if you're not familiar with std::stoi
</summary>
]]--
native "STRING_TO_INT"
	hash "0x5A5F40FE637EB584"
	jhash (0x590A8160)
	arguments {
		charPtr "string",

		intPtr "outInteger",
	}
	returns	"BOOL"

native "SET_BITS_IN_RANGE"
	hash "0x8EF07E15701D61ED"
	jhash (0x32094719)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "GET_BITS_IN_RANGE"
	hash "0x53158863FCC0893A"
	jhash (0xCA03A1E5)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "ADD_STUNT_JUMP"
	hash "0x1A992DA297A4630C"
	jhash (0xB630E5FF)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",

		float "p11",

		float "p12",

		float "p13",

		float "p14",

		Any "p15",

		Any "p16",
	}
	returns	"Any"

native "ADD_STUNT_JUMP_ANGLED"
	hash "0xBBE5D803A5360CBF"
	jhash (0xB9B7E777)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",

		float "p11",

		float "p12",

		float "p13",

		float "p14",

		float "p15",

		float "p16",

		Any "p17",

		Any "p18",
	}
	returns	"Any"

native "DELETE_STUNT_JUMP"
	hash "0xDC518000E39DAE1F"
	jhash (0x840CB5DA)
	arguments {
		Any "p0",
	}
	returns	"void"

native "ENABLE_STUNT_JUMP_SET"
	hash "0xE369A5783B866016"
	jhash (0x9D1E7785)
	arguments {
		Any "p0",
	}
	returns	"void"

native "DISABLE_STUNT_JUMP_SET"
	hash "0xA5272EBEDD4747F6"
	jhash (0x644C9FA4)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xD79185689F8FD5DF"
	hash "0xD79185689F8FD5DF"
	jhash (0x3C806A2D)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "IS_STUNT_JUMP_IN_PROGRESS"
	hash "0x7A3F19700A4D0525"
	jhash (0xF477D0B1)
	returns	"BOOL"

native "0x2272B0A1343129F4"
	hash "0x2272B0A1343129F4"
	jhash (0x021636EE)
	returns	"BOOL"

native "0x996DD1E1E02F1008"
	hash "0x996DD1E1E02F1008"
	jhash (0x006F9BA2)
	returns	"Any"

native "0x6856EC3D35C81EA4"
	hash "0x6856EC3D35C81EA4"
	returns	"Any"

native "CANCEL_STUNT_JUMP"
	hash "0xE6B7B0ACD4E4B75E"
	jhash (0xF43D9821)
	returns	"void"

--[[!
<summary>
	Pauses Game.

	That's what the game does when opening the Social Club UI.
</summary>
]]--
native "SET_GAME_PAUSED"
	hash "0x577D1284D6873711"
	jhash (0x8230FF6C)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "SET_THIS_SCRIPT_CAN_BE_PAUSED"
	hash "0xAA391C728106F7AF"
	jhash (0xA0C3CE29)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "SET_THIS_SCRIPT_CAN_REMOVE_BLIPS_CREATED_BY_ANY_SCRIPT"
	hash "0xB98236CAAECEF897"
	jhash (0xD06F1720)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0x071E2A839DE82D90"
	hash "0x071E2A839DE82D90"
	jhash (0xFF6191E1)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x557E43C447E700A8"
	hash "0x557E43C447E700A8"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	Formerly known as _LOWER_MAP_PROP_DENSITY.
	But it clearly loads the prop placements that are used in GTA Online, it even shows a GTAO loading screen when you toggle it on for the first time.
</summary>
]]--
native "_ENABLE_MP_DLC_MAPS"
	hash "0x9BAE5AD2508DF078"
	jhash (0x721B2492)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Sets an unknown flag used by CScene in determining which entities from CMapData scene nodes to draw, similar to 9BAE5AD2508DF078.

	Documented by NTAuthority (http://fivem.net/).
</summary>
]]--
native "_SET_UNK_MAP_FLAG"
	hash "0xC5F0A8EBD3F361CE"
	jhash (0xE202879D)
	arguments {
		int "flag",
	}
	returns	"void"

native "IS_FRONTEND_FADING"
	hash "0x7EA2B6AF97ECA6ED"
	jhash (0x8FF6232C)
	returns	"BOOL"

--[[!
<summary>
	spawns a few distant/out-of-sight peds, vehicles, animals etc each time it is called
</summary>
]]--
native "POPULATE_NOW"
	hash "0x7472BB270D7B4F3E"
	jhash (0x72C20700)
	returns	"void"

native "GET_INDEX_OF_CURRENT_LEVEL"
	hash "0xCBAD6729F7B1F4FC"
	jhash (0x6F203C6E)
	returns	"int"

--[[!
<summary>
	level can be from 0 to 3
</summary>
]]--
native "SET_GRAVITY_LEVEL"
	hash "0x740E14FAD5842351"
	jhash (0x2D833F4A)
	arguments {
		int "level",
	}
	returns	"void"

native "START_SAVE_DATA"
	hash "0xA9575F812C6A7997"
	jhash (0x881A694D)
	arguments {
		AnyPtr "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "STOP_SAVE_DATA"
	hash "0x74E20C9145FB66FD"
	jhash (0x3B1C07C8)
	returns	"void"

native "0xA09F896CE912481F"
	hash "0xA09F896CE912481F"
	jhash (0x9EF0BC64)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "REGISTER_INT_TO_SAVE"
	hash "0x34C9EE5986258415"
	jhash (0xB930956F)
	arguments {
		AnyPtr "p0",

		charPtr "name",
	}
	returns	"void"

native "0xA735353C77334EA0"
	hash "0xA735353C77334EA0"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "REGISTER_ENUM_TO_SAVE"
	hash "0x10C2FA78D0E128A1"
	jhash (0x9B38374A)
	arguments {
		AnyPtr "p0",

		charPtr "name",
	}
	returns	"void"

native "REGISTER_FLOAT_TO_SAVE"
	hash "0x7CAEC29ECB5DFEBB"
	jhash (0xDB06F7AD)
	arguments {
		AnyPtr "p0",

		charPtr "name",
	}
	returns	"void"

native "REGISTER_BOOL_TO_SAVE"
	hash "0xC8F4131414C835A1"
	jhash (0x5417E0E0)
	arguments {
		AnyPtr "p0",

		charPtr "name",
	}
	returns	"void"

native "REGISTER_TEXT_LABEL_TO_SAVE"
	hash "0xEDB1232C5BEAE62F"
	jhash (0x284352C4)
	arguments {
		AnyPtr "p0",

		charPtr "name",
	}
	returns	"void"

--[[!
<summary>
	Seems to have the same functionality as REGISTER_TEXT_LABEL_TO_SAVE?

	GAMEPLAY::_6F7794F28C6B2535(&amp;a_0._f1, "tlPlateText");
	GAMEPLAY::_6F7794F28C6B2535(&amp;a_0._f1C, "tlPlateText_pending");
	GAMEPLAY::_6F7794F28C6B2535(&amp;a_0._f10B, "tlCarAppPlateText");

	"tl" prefix sounds like "Text Label". ~Fireboyd78
</summary>
]]--
native "0x6F7794F28C6B2535"
	hash "0x6F7794F28C6B2535"
	jhash (0xE2089749)
	arguments {
		AnyPtr "p0",

		charPtr "name",
	}
	returns	"void"

--[[!
<summary>
	Only found 3 times in decompiled scripts. Not a whole lot to go off of.

	GAMEPLAY::_48F069265A0E4BEC(a_0, "Movie_Name_For_This_Player");
	GAMEPLAY::_48F069265A0E4BEC(&amp;a_0._fB, "Ringtone_For_This_Player");
	GAMEPLAY::_48F069265A0E4BEC(&amp;a_0._f1EC4._f12[v_A/*6*/], &amp;v_13); // where v_13 is "MPATMLOGSCRS0" thru "MPATMLOGSCRS15"
</summary>
]]--
native "0x48F069265A0E4BEC"
	hash "0x48F069265A0E4BEC"
	jhash (0xF91B8C33)
	arguments {
		AnyPtr "p0",

		charPtr "name",
	}
	returns	"void"

--[[!
<summary>
	Only found 2 times in decompiled scripts. Not a whole lot to go off of.

	GAMEPLAY::_8269816F6CFD40F8(&amp;a_0._f1F5A._f6[0/*8*/], "TEMPSTAT_LABEL"); // gets saved in a struct called "g_SaveData_STRING_ScriptSaves"
	GAMEPLAY::_8269816F6CFD40F8(&amp;a_0._f4B4[v_1A/*8*/], &amp;v_5); // where v_5 is "Name0" thru "Name9", gets saved in a struct called "OUTFIT_Name"
</summary>
]]--
native "0x8269816F6CFD40F8"
	hash "0x8269816F6CFD40F8"
	jhash (0x74E8FAD9)
	arguments {
		AnyPtr "p0",

		charPtr "name",
	}
	returns	"void"

--[[!
<summary>
	Another unknown label type...

	GAMEPLAY::_FAA457EF263E8763(a_0, "Thumb_label");
	GAMEPLAY::_FAA457EF263E8763(&amp;a_0._f10, "Photo_label");
	GAMEPLAY::_FAA457EF263E8763(a_0, "GXTlabel");
	GAMEPLAY::_FAA457EF263E8763(&amp;a_0._f21, "StringComp");
	GAMEPLAY::_FAA457EF263E8763(&amp;a_0._f43, "SecondStringComp");
	GAMEPLAY::_FAA457EF263E8763(&amp;a_0._f53, "ThirdStringComp");
	GAMEPLAY::_FAA457EF263E8763(&amp;a_0._f32, "SenderStringComp");
	GAMEPLAY::_FAA457EF263E8763(&amp;a_0._f726[v_1A/*16*/], &amp;v_20); // where v_20 is "LastJobTL_0_1" thru "LastJobTL_2_1", gets saved in a struct called "LAST_JobGamer_TL"
	GAMEPLAY::_FAA457EF263E8763(&amp;a_0._f4B, "PAID_PLAYER");
	GAMEPLAY::_FAA457EF263E8763(&amp;a_0._f5B, "RADIO_STATION");
</summary>
]]--
native "0xFAA457EF263E8763"
	hash "0xFAA457EF263E8763"
	jhash (0x6B4335DD)
	arguments {
		AnyPtr "p0",

		charPtr "name",
	}
	returns	"void"

--[[!
<summary>
	Second parameter might be length.
</summary>
]]--
native "_START_SAVE_STRUCT"
	hash "0xBF737600CDDBEADD"
	jhash (0xFB45728E)
	arguments {
		AnyPtr "p0",

		int "p1",

		charPtr "structName",
	}
	returns	"void"

native "STOP_SAVE_STRUCT"
	hash "0xEB1774DF12BB9F12"
	jhash (0xC2624A28)
	returns	"void"

--[[!
<summary>
	Second parameter might be length.
</summary>
]]--
native "_START_SAVE_ARRAY"
	hash "0x60FE567DF1B1AF9D"
	jhash (0x893A342C)
	arguments {
		AnyPtr "p0",

		int "p1",

		charPtr "arrayName",
	}
	returns	"void"

native "STOP_SAVE_ARRAY"
	hash "0x04456F95153C6BE4"
	jhash (0x0CAD8217)
	returns	"void"

native "0xDC0F817884CDD856"
	hash "0xDC0F817884CDD856"
	jhash (0x0B710A51)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x9B2BD3773123EA2F"
	hash "0x9B2BD3773123EA2F"
	jhash (0xE0F0684F)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xEB4A0C2D56441717"
	hash "0xEB4A0C2D56441717"
	jhash (0x3CE5BF6B)
	arguments {
		int "p0",
	}
	returns	"int"

--[[!
<summary>
	Create an incident at specified coordinates of type, (3 = Fire, 5 = Medical, 7 = Police). incidentId is the location where the new id is returned to.
</summary>
]]--
native "CREATE_INCIDENT"
	hash "0x3F892CAF67444AE7"
	jhash (0xFC5FF7B3)
	arguments {
		intPtr "incidentType",

		float "x",

		float "y",

		float "z",

		intPtr "p4",

		float "radius",

		intPtr "incidentId",
	}
	returns	"BOOL"

native "CREATE_INCIDENT_WITH_ENTITY"
	hash "0x05983472F0494E60"
	jhash (0xBBC35B03)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		float "p3",

		AnyPtr "p4",
	}
	returns	"BOOL"

--[[!
<summary>
	Delete an incident with a given id.
</summary>
]]--
native "DELETE_INCIDENT"
	hash "0x556C1AA270D5A207"
	jhash (0x212BD0DC)
	arguments {
		intPtr "incidentId",
	}
	returns	"void"

native "IS_INCIDENT_VALID"
	hash "0xC8BC6461E629BEAA"
	jhash (0x31FD0BA4)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xB08B85D860E7BA3C"
	hash "0xB08B85D860E7BA3C"
	jhash (0x0242D88E)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0xD261BA3E7E998072"
	hash "0xD261BA3E7E998072"
	jhash (0x1F38102E)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "FIND_SPAWN_POINT_IN_DIRECTION"
	hash "0x6874E2190B0C1972"
	jhash (0x71AEFD77)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		float "distance",

		Vector3Ptr "spawnPoint",
	}
	returns	"BOOL"

native "0x67F6413D3220E18D"
	hash "0x67F6413D3220E18D"
	jhash (0x42BF09B3)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",
	}
	returns	"Any"

native "0x1327E2FE9746BAEE"
	hash "0x1327E2FE9746BAEE"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xB129E447A2EDA4BF"
	hash "0xB129E447A2EDA4BF"
	jhash (0xFBDBE374)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x32C7A7E8C43A1F80"
	hash "0x32C7A7E8C43A1F80"
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"Any"

native "0xE6869BECDD8F2403"
	hash "0xE6869BECDD8F2403"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "ENABLE_TENNIS_MODE"
	hash "0x28A04B411933F8A6"
	jhash (0x0BD3F9EC)
	arguments {
		Ped "ped",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	Return whether tennis mode is active or not.
</summary>
]]--
native "IS_TENNIS_MODE"
	hash "0x5D5479D115290C3F"
	jhash (0x04A947BA)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xE266ED23311F24D4"
	hash "0xE266ED23311F24D4"
	jhash (0xC20A7D2B)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		float "p3",

		float "p4",

		BOOL "p5",
	}
	returns	"void"

native "0x17DF68D720AA77F8"
	hash "0x17DF68D720AA77F8"
	jhash (0x8501E727)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x19BFED045C647C49"
	hash "0x19BFED045C647C49"
	jhash (0x1A332D2D)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xE95B0C7D5BA3B96B"
	hash "0xE95B0C7D5BA3B96B"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x8FA9C42FC5D7C64B"
	hash "0x8FA9C42FC5D7C64B"
	jhash (0x0C8865DF)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		BOOL "p5",
	}
	returns	"void"

native "0x54F157E0336A3822"
	hash "0x54F157E0336A3822"
	jhash (0x49F977A9)
	arguments {
		Any "p0",

		AnyPtr "p1",

		float "p2",
	}
	returns	"void"

native "0xD10F442036302D50"
	hash "0xD10F442036302D50"
	jhash (0x6F009E33)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "RESET_DISPATCH_IDEAL_SPAWN_DISTANCE"
	hash "0x77A84429DD9F0A15"
	jhash (0xDA65ECAA)
	returns	"void"

native "SET_DISPATCH_IDEAL_SPAWN_DISTANCE"
	hash "0x6FE601A64180D423"
	jhash (0x6283BE32)
	arguments {
		float "p0",
	}
	returns	"void"

native "SET_DISPATCH_TIME_BETWEEN_SPAWN_ATTEMPTS"
	hash "0x44F7CBC1BEB3327D"
	jhash (0xABADB709)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "SET_DISPATCH_TIME_BETWEEN_SPAWN_ATTEMPTS_MULTIPLIER"
	hash "0x48838ED9937A15D1"
	jhash (0x1C996BCD)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x918C7B2D2FF3928B"
	hash "0x918C7B2D2FF3928B"
	jhash (0xF557BAF9)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",
	}
	returns	"Any"

native "0x2D4259F1FEB81DA9"
	hash "0x2D4259F1FEB81DA9"
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"Any"

native "REMOVE_DISPATCH_SPAWN_BLOCKING_AREA"
	hash "0x264AC28B01B353A5"
	jhash (0xA8D2FB92)
	arguments {
		Any "p0",
	}
	returns	"void"

native "RESET_DISPATCH_SPAWN_BLOCKING_AREAS"
	hash "0xAC7BFD5C1D83EA75"
	jhash (0x9A17F835)
	returns	"void"

native "0xD9F692D349249528"
	hash "0xD9F692D349249528"
	jhash (0xE0C9307E)
	returns	"void"

native "0xE532EC1A63231B4F"
	hash "0xE532EC1A63231B4F"
	jhash (0xA0D8C749)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xB8721407EE9C3FF6"
	hash "0xB8721407EE9C3FF6"
	jhash (0x24A4E0B2)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0xB3CD58CCA6CDA852"
	hash "0xB3CD58CCA6CDA852"
	jhash (0x66C3C59C)
	returns	"void"

native "0x2587A48BC88DFADF"
	hash "0x2587A48BC88DFADF"
	jhash (0xD9660339)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	only documented to be continued...
</summary>
]]--
native "0xCA78CFA0366592FE"
	hash "0xCA78CFA0366592FE"
	jhash (0xD2688412)
	arguments {
		BOOL "p0",

		charPtr "windowTitle",

		AnyPtr "p2",

		charPtr "defaultText",

		charPtr "defaultConcat1",

		charPtr "defaultConcat2",

		charPtr "defaultConcat3",

		charPtr "defaultConcat4",

		charPtr "defaultConcat5",

		charPtr "defaultConcat6",

		charPtr "defaultConcat7",

		int "maxInputLength",
	}
	returns	"void"

--[[!
<summary>
	NOTE: windowTitle uses enumerables, and an invalid value will display nothing.

	http://gtaforums.com/topic/788343-vrel-script-hook-v/?p=1067380474

	windowTitle's
	-----------------
	CELL_EMAIL_BOD	=	"Enter your Eyefind message"
	CELL_EMAIL_BODE	=	"Message too long. Try again"
	CELL_EMAIL_BODF	=	"Forbidden message. Try again"
	CELL_EMAIL_SOD	=	"Enter your Eyefind subject"
	CELL_EMAIL_SODE	=	"Subject too long. Try again"
	CELL_EMAIL_SODF	=	"Forbidden text. Try again"
	CELL_EMASH_BOD	=	"Enter your Eyefind message"
	CELL_EMASH_BODE	=	"Message too long. Try again"
	CELL_EMASH_BODF	=	"Forbidden message. Try again"
	CELL_EMASH_SOD	=	"Enter your Eyefind subject"
	CELL_EMASH_SODE	=	"Subject too long. Try again"
	CELL_EMASH_SODF	=	"Forbidden Text. Try again"
	FMMC_KEY_TIP10	=	"Enter Synopsis"
	FMMC_KEY_TIP12	=	"Enter Custom Team Name"
	FMMC_KEY_TIP12F	=	"Forbidden Text. Try again"
	FMMC_KEY_TIP12N	=	"Custom Team Name"
	FMMC_KEY_TIP8	=	"Enter Message"
	FMMC_KEY_TIP8F	=	"Forbidden Text. Try again"
	FMMC_KEY_TIP8FS	=	"Invalid Message. Try again"
	FMMC_KEY_TIP8S	=	"Enter Message"
	FMMC_KEY_TIP9	=	"Enter Outfit Name"
	FMMC_KEY_TIP9F	=	"Invalid Outfit Name. Try again"
	FMMC_KEY_TIP9N	=	"Outfit Name"
	PM_NAME_CHALL	=	"Enter Challenge Name"
</summary>
]]--
native "DISPLAY_ONSCREEN_KEYBOARD"
	hash "0x00DC833F2568DBF6"
	jhash (0xAD99F2CE)
	arguments {
		BOOL "p0",

		charPtr "windowTitle",

		charPtr "p2",

		charPtr "defaultText",

		charPtr "defaultConcat1",

		charPtr "defaultConcat2",

		charPtr "defaultConcat3",

		int "maxInputLength",
	}
	returns	"void"

--[[!
<summary>
	Returns the current status of the onscreen keyboard, and updates the output.

	Status Codes:

	0 - User still editing
	1 - User has finished editing
	2 - User has canceled editing
	3 - Keyboard isn't active
</summary>
]]--
native "UPDATE_ONSCREEN_KEYBOARD"
	hash "0x0CF2B696BBF945AE"
	jhash (0x23D0A1CE)
	returns	"int"

--[[!
<summary>
	Returns NULL unless UPDATE_ONSCREEN_KEYBOARD() returns 1 in the same tick.
</summary>
]]--
native "GET_ONSCREEN_KEYBOARD_RESULT"
	hash "0x8362B09B91893647"
	jhash (0x44828FB3)
	returns	"charPtr"

native "0x3ED1438C1F5C6612"
	hash "0x3ED1438C1F5C6612"
	jhash (0x3301EA47)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xA6A12939F16D85BE"
	hash "0xA6A12939F16D85BE"
	jhash (0x42B484ED)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x1EAE0A6E978894A2"
	hash "0x1EAE0A6E978894A2"
	jhash (0x8F60366E)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_EXPLOSIVE_AMMO_THIS_FRAME"
	hash "0xA66C71C98D5F2CFB"
	jhash (0x2EAFA1D1)
	arguments {
		Player "player",
	}
	returns	"Any"

native "SET_FIRE_AMMO_THIS_FRAME"
	hash "0x11879CDD803D30F4"
	jhash (0x7C18FC8A)
	arguments {
		Player "player",
	}
	returns	"Any"

native "SET_EXPLOSIVE_MELEE_THIS_FRAME"
	hash "0xFF1BED81BFDC0FE0"
	jhash (0x96663D56)
	arguments {
		Player "player",
	}
	returns	"Any"

native "SET_SUPER_JUMP_THIS_FRAME"
	hash "0x57FFF03E423A4C0B"
	jhash (0x86745EF3)
	arguments {
		Player "player",
	}
	returns	"Any"

native "0x6FDDF453C0C756EC"
	hash "0x6FDDF453C0C756EC"
	jhash (0xC3C10FCC)
	returns	"Any"

native "0xFB00CA71DA386228"
	hash "0xFB00CA71DA386228"
	jhash (0x054EC103)
	returns	"void"

native "0x5AA3BEFA29F03AD4"
	hash "0x5AA3BEFA29F03AD4"
	jhash (0x46B5A15C)
	returns	"Any"

--[[!
<summary>
	sets something to 1
</summary>
]]--
native "0xE3D969D2785FFB5E"
	hash "0xE3D969D2785FFB5E"
	returns	"void"

--[[!
<summary>
	Sets the localplayer playerinfo state back to playing (State 0)

	States are:
	0: "Playing"
	1: "Died"
	2: "Arrested"
	3: "Failed Mission",
	4: "Left Game",
	5: "Respawn",
	6: "In MP Cutscene"
</summary>
]]--
native "_RESET_LOCALPLAYER_STATE"
	hash "0xC0AA53F866B3134D"
	jhash (0x5D209F25)
	returns	"void"

native "0x0A60017F841A54F2"
	hash "0x0A60017F841A54F2"
	jhash (0x2D33F15A)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x1FF6BF9A63E5757F"
	hash "0x1FF6BF9A63E5757F"
	jhash (0xDF99925C)
	returns	"void"

native "0x1BB299305C3E8C13"
	hash "0x1BB299305C3E8C13"
	jhash (0xA27F4472)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x8EF5573A1F801A5C"
	hash "0x8EF5573A1F801A5C"
	jhash (0x07FF553F)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0x92790862E36C2ADA"
	hash "0x92790862E36C2ADA"
	returns	"void"

native "0xC7DB36C24634F52B"
	hash "0xC7DB36C24634F52B"
	returns	"void"

native "0x437138B6A830166A"
	hash "0x437138B6A830166A"
	returns	"void"

native "0x37DEB0AA183FB6D8"
	hash "0x37DEB0AA183FB6D8"
	returns	"void"

native "0xEA2F2061875EED90"
	hash "0xEA2F2061875EED90"
	returns	"Any"

native "0x3BBBD13E5041A79E"
	hash "0x3BBBD13E5041A79E"
	returns	"Any"

native "0xA049A5BE0F04F2F8"
	hash "0xA049A5BE0F04F2F8"
	returns	"Any"

native "0x4750FC27570311EC"
	hash "0x4750FC27570311EC"
	returns	"Any"

native "0x1B2366C3F2A5C8DF"
	hash "0x1B2366C3F2A5C8DF"
	returns	"Any"

--[[!
<summary>
	Exits the game and downloads a fresh social club update on next restart.
</summary>
]]--
native "_FORCE_SOCIAL_CLUB_UPDATE"
	hash "0xEB6891F03362FB12"
	returns	"void"

native "0x14832BF2ABA53FC5"
	hash "0x14832BF2ABA53FC5"
	returns	"Any"

native "0xC79AE21974B01FB2"
	hash "0xC79AE21974B01FB2"
	returns	"void"

--[[!
<summary>
	example:

	 if (GAMEPLAY::_684A41975F077262()) {
	        (a_0) = GAMEPLAY::_ABB2FA71C83A1B72();
	    } else { 
	        (a_0) = -1;
	    }
</summary>
]]--
native "0x684A41975F077262"
	hash "0x684A41975F077262"
	returns	"BOOL"

native "0xABB2FA71C83A1B72"
	hash "0xABB2FA71C83A1B72"
	returns	"Any"

native "0x4EBB7E87AA0DBED4"
	hash "0x4EBB7E87AA0DBED4"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x9689123E3F213AA5"
	hash "0x9689123E3F213AA5"
	returns	"BOOL"

native "0x9D8D44ADBBA61EF2"
	hash "0x9D8D44ADBBA61EF2"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x23227DF0B2115469"
	hash "0x23227DF0B2115469"
	returns	"void"

native "0xD10282B6E3751BA0"
	hash "0xD10282B6E3751BA0"
	returns	"Any"

native "PLAY_PED_RINGTONE"
	hash "0xF9E56683CA8E11A5"
	jhash (0x1D530E47)
	arguments {
		AnyPtr "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "IS_PED_RINGTONE_PLAYING"
	hash "0x1E8E5E20937E3137"
	jhash (0xFE576EE4)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "STOP_PED_RINGTONE"
	hash "0x6C5AE23EFA885092"
	jhash (0xFEEA107C)
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_MOBILE_PHONE_CALL_ONGOING"
	hash "0x7497D2CE2C30D24C"
	jhash (0x4ED1400A)
	returns	"BOOL"

native "0xC8B1B2425604CDD0"
	hash "0xC8B1B2425604CDD0"
	jhash (0x16FB88B5)
	returns	"Any"

native "CREATE_NEW_SCRIPTED_CONVERSATION"
	hash "0xD2C91A0B572AAE56"
	jhash (0xB2BC25F8)
	returns	"void"

native "ADD_LINE_TO_CONVERSATION"
	hash "0xC5EF963405593646"
	jhash (0x96CD0513)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		Any "p3",

		Any "p4",

		BOOL "p5",

		BOOL "p6",

		BOOL "p7",

		BOOL "p8",

		Any "p9",

		BOOL "p10",

		BOOL "p11",

		BOOL "p12",
	}
	returns	"void"

native "ADD_PED_TO_CONVERSATION"
	hash "0x95D9F4BC443956E7"
	jhash (0xF8D5EB86)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0x33E3C6C6F2F0B506"
	hash "0x33E3C6C6F2F0B506"
	jhash (0x73C6F979)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "0x892B6AB8F33606F5"
	hash "0x892B6AB8F33606F5"
	jhash (0x88203DDA)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "SET_MICROPHONE_POSITION"
	hash "0xB6AE90EDDE95C762"
	jhash (0xAD7BB191)
	arguments {
		BOOL "p0",

		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		float "x3",

		float "y3",

		float "z3",
	}
	returns	"void"

native "0x0B568201DD99F0EB"
	hash "0x0B568201DD99F0EB"
	jhash (0x1193ED6E)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x61631F5DF50D1C34"
	hash "0x61631F5DF50D1C34"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "START_SCRIPT_PHONE_CONVERSATION"
	hash "0x252E5F915EABB675"
	jhash (0x38E42D07)
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"void"

native "PRELOAD_SCRIPT_PHONE_CONVERSATION"
	hash "0x6004BCB0E226AAEA"
	jhash (0x9ACB213A)
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"void"

native "START_SCRIPT_CONVERSATION"
	hash "0x6B17C62C9635D2DC"
	jhash (0xE5DE7D9D)
	arguments {
		BOOL "p0",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "PRELOAD_SCRIPT_CONVERSATION"
	hash "0x3B3CAD6166916D87"
	jhash (0xDDF5C579)
	arguments {
		BOOL "p0",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "START_PRELOADED_CONVERSATION"
	hash "0x23641AFE870AF385"
	jhash (0xA170261B)
	returns	"void"

native "0xE73364DB90778FFA"
	hash "0xE73364DB90778FFA"
	jhash (0x336F3D35)
	returns	"Any"

native "IS_SCRIPTED_CONVERSATION_ONGOING"
	hash "0x16754C556D2EDE3D"
	jhash (0xCB8FD96F)
	returns	"BOOL"

native "IS_SCRIPTED_CONVERSATION_LOADED"
	hash "0xDF0D54BE7A776737"
	jhash (0xE1870EA9)
	returns	"BOOL"

native "GET_CURRENT_SCRIPTED_CONVERSATION_LINE"
	hash "0x480357EE890C295A"
	jhash (0x9620E41F)
	returns	"Any"

native "PAUSE_SCRIPTED_CONVERSATION"
	hash "0x8530AD776CD72B12"
	jhash (0xE2C9C6F8)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "RESTART_SCRIPTED_CONVERSATION"
	hash "0x9AEB285D1818C9AC"
	jhash (0x6CB24B56)
	returns	"void"

native "STOP_SCRIPTED_CONVERSATION"
	hash "0xD79DEEFB53455EBA"
	jhash (0xAB77DA7D)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "SKIP_TO_NEXT_SCRIPTED_CONVERSATION_LINE"
	hash "0x9663FE6B7A61EB00"
	jhash (0x85C98304)
	returns	"void"

native "INTERRUPT_CONVERSATION"
	hash "0xA018A12E5C5C2FA6"
	jhash (0xF3A67AF3)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0x8A694D7A68F8DC38"
	hash "0x8A694D7A68F8DC38"
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0xAA19F5572C38B564"
	hash "0xAA19F5572C38B564"
	jhash (0xB58B8FF3)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "0xB542DE8C3D1CB210"
	hash "0xB542DE8C3D1CB210"
	jhash (0x789D8C6C)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "REGISTER_SCRIPT_WITH_AUDIO"
	hash "0xC6ED9D5092438D91"
	jhash (0xA6203643)
	arguments {
		Any "p0",
	}
	returns	"void"

native "UNREGISTER_SCRIPT_WITH_AUDIO"
	hash "0xA8638BE228D4751A"
	jhash (0x66728EFE)
	returns	"void"

native "REQUEST_MISSION_AUDIO_BANK"
	hash "0x7345BDD95E62E0F2"
	jhash (0x916E37CA)
	arguments {
		charPtr "p0",

		BOOL "p1",
	}
	returns	"BOOL"

native "REQUEST_AMBIENT_AUDIO_BANK"
	hash "0xFE02FFBED8CA9D99"
	jhash (0x23C88BC7)
	arguments {
		charPtr "p0",

		BOOL "p1",
	}
	returns	"BOOL"

native "REQUEST_SCRIPT_AUDIO_BANK"
	hash "0x2F844A8B08D76685"
	jhash (0x21322887)
	arguments {
		charPtr "p0",

		BOOL "p1",
	}
	returns	"BOOL"

native "HINT_AMBIENT_AUDIO_BANK"
	hash "0x8F8C0E370AE62F5C"
	jhash (0xF1850DDC)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "HINT_SCRIPT_AUDIO_BANK"
	hash "0xFB380A29641EC31A"
	jhash (0x41FA0E51)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "RELEASE_MISSION_AUDIO_BANK"
	hash "0x0EC92A1BF0857187"
	jhash (0x8E8824C7)
	returns	"void"

native "RELEASE_AMBIENT_AUDIO_BANK"
	hash "0x65475A218FFAA93D"
	jhash (0x8C938784)
	returns	"void"

native "RELEASE_NAMED_SCRIPT_AUDIO_BANK"
	hash "0x77ED170667F50170"
	jhash (0x16707ABC)
	arguments {
		charPtr "audioBank",
	}
	returns	"void"

native "RELEASE_SCRIPT_AUDIO_BANK"
	hash "0x7A2D8AD0A9EB9C3F"
	jhash (0x22F865E5)
	returns	"void"

native "0x19AF7ED9B9D23058"
	hash "0x19AF7ED9B9D23058"
	jhash (0xA58BBF4F)
	returns	"void"

native "0x9AC92EED5E4793AB"
	hash "0x9AC92EED5E4793AB"
	returns	"void"

native "GET_SOUND_ID"
	hash "0x430386FE9BF80B45"
	jhash (0x6AE0AD56)
	returns	"int"

native "RELEASE_SOUND_ID"
	hash "0x353FC880830B88FA"
	jhash (0x9C080899)
	arguments {
		int "soundId",
	}
	returns	"void"

native "PLAY_SOUND"
	hash "0x7FF4944CC209192D"
	jhash (0xB6E1917F)
	arguments {
		int "soundId",

		charPtr "soundName",

		charPtr "setName",

		BOOL "p3",

		Any "p4",

		BOOL "p5",
	}
	returns	"void"

--[[!
<summary>
	list: http://konijima.com/assets/PLAY_SOUND_FRONTEND.txt
</summary>
]]--
native "PLAY_SOUND_FRONTEND"
	hash "0x67C540AA08E4A6F5"
	jhash (0x2E458F74)
	arguments {
		int "soundId",

		charPtr "soundName",

		charPtr "setName",

		BOOL "p3",
	}
	returns	"void"

native "0xCADA5A0D0702381E"
	hash "0xCADA5A0D0702381E"
	jhash (0xC70E6CFA)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "PLAY_SOUND_FROM_ENTITY"
	hash "0xE65F427EB70AB1ED"
	jhash (0x95AE00F8)
	arguments {
		int "soundId",

		charPtr "soundName",

		Entity "entity",

		charPtr "setName",

		BOOL "p4",

		Any "p5",
	}
	returns	"void"

native "PLAY_SOUND_FROM_COORD"
	hash "0x8D8686B622B88120"
	jhash (0xCAD3E2D5)
	arguments {
		int "soundId",

		charPtr "soundName",

		float "x",

		float "y",

		float "z",

		charPtr "setName",

		BOOL "p6",

		Any "p7",

		BOOL "p8",
	}
	returns	"void"

native "STOP_SOUND"
	hash "0xA3B0C41BA5CC0BB5"
	jhash (0xCD7F4030)
	arguments {
		int "soundId",
	}
	returns	"void"

native "GET_NETWORK_ID_FROM_SOUND_ID"
	hash "0x2DE3F0A134FFBC0D"
	jhash (0x2576F610)
	arguments {
		int "soundId",
	}
	returns	"Any"

native "0x75262FD12D0A1C84"
	hash "0x75262FD12D0A1C84"
	jhash (0xD064D4DC)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "SET_VARIABLE_ON_SOUND"
	hash "0xAD6B3148A78AE9B6"
	jhash (0x606EE5FA)
	arguments {
		int "soundId",

		AnyPtr "p1",

		float "p2",
	}
	returns	"void"

native "SET_VARIABLE_ON_STREAM"
	hash "0x2F9D3834AEB9EF79"
	jhash (0xF67BB44C)
	arguments {
		AnyPtr "p0",

		float "p1",
	}
	returns	"void"

native "OVERRIDE_UNDERWATER_STREAM"
	hash "0xF2A9CDABCEA04BD6"
	jhash (0x9A083B7E)
	arguments {
		AnyPtr "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	AUDIO::_733ADF241531E5C2("inTunnel", 1.0);
	AUDIO::_733ADF241531E5C2("inTunnel", 0.0);

	I do not know as of yet what this does, but this was found in the scripts.


</summary>
]]--
native "0x733ADF241531E5C2"
	hash "0x733ADF241531E5C2"
	jhash (0x62D026BE)
	arguments {
		AnyPtr "p0",

		float "p1",
	}
	returns	"void"

native "HAS_SOUND_FINISHED"
	hash "0xFCBDCE714A7C88E5"
	jhash (0xE85AEC2E)
	arguments {
		int "soundId",
	}
	returns	"BOOL"

--[[!
<summary>
	Plays ambient speech. See also _0x444180DB.

	pedHandle: The ped to play the ambient speech.
	speechName: Name of the speech to play, eg. "GENERIC_HI".
	speechParam: Can be one of the following:
	SPEECH_PARAMS_STANDARD
	SPEECH_PARAMS_ALLOW_REPEAT
	SPEECH_PARAMS_BEAT
	SPEECH_PARAMS_FORCE
	SPEECH_PARAMS_FORCE_FRONTEND
	SPEECH_PARAMS_FORCE_NO_REPEAT_FRONTEND
	SPEECH_PARAMS_FORCE_NORMAL
	SPEECH_PARAMS_FORCE_NORMAL_CLEAR
	SPEECH_PARAMS_FORCE_NORMAL_CRITICAL
	SPEECH_PARAMS_FORCE_SHOUTED
	SPEECH_PARAMS_FORCE_SHOUTED_CLEAR
	SPEECH_PARAMS_FORCE_SHOUTED_CRITICAL
	SPEECH_PARAMS_FORCE_PRELOAD_ONLY
	SPEECH_PARAMS_MEGAPHONE
	SPEECH_PARAMS_HELI
	SPEECH_PARAMS_FORCE_MEGAPHONE
	SPEECH_PARAMS_FORCE_HELI
	SPEECH_PARAMS_INTERRUPT
	SPEECH_PARAMS_INTERRUPT_SHOUTED
	SPEECH_PARAMS_INTERRUPT_SHOUTED_CLEAR
	SPEECH_PARAMS_INTERRUPT_SHOUTED_CRITICAL
	SPEECH_PARAMS_INTERRUPT_NO_FORCE
	SPEECH_PARAMS_INTERRUPT_FRONTEND
	SPEECH_PARAMS_INTERRUPT_NO_FORCE_FRONTEND
	SPEECH_PARAMS_ADD_BLIP
	SPEECH_PARAMS_ADD_BLIP_ALLOW_REPEAT
	SPEECH_PARAMS_ADD_BLIP_FORCE
	SPEECH_PARAMS_ADD_BLIP_SHOUTED
	SPEECH_PARAMS_ADD_BLIP_SHOUTED_FORCE
	SPEECH_PARAMS_ADD_BLIP_INTERRUPT
	SPEECH_PARAMS_ADD_BLIP_INTERRUPT_FORCE
	SPEECH_PARAMS_FORCE_PRELOAD_ONLY_SHOUTED
	SPEECH_PARAMS_FORCE_PRELOAD_ONLY_SHOUTED_CLEAR
	SPEECH_PARAMS_FORCE_PRELOAD_ONLY_SHOUTED_CRITICAL
	SPEECH_PARAMS_SHOUTED
	SPEECH_PARAMS_SHOUTED_CLEAR
	SPEECH_PARAMS_SHOUTED_CRITICAL

	Note: A list of Name and Parameters can be found here http://pastebin.com/1GZS5dCL
</summary>
]]--
native "_PLAY_AMBIENT_SPEECH1"
	hash "0x8E04FEDD28D42462"
	jhash (0x5C57B85D)
	arguments {
		Ped "pedHandle",

		charPtr "speechName",

		charPtr "speechParam",
	}
	returns	"void"

--[[!
<summary>
	Plays ambient speech. See also _0x5C57B85D.

	See _PLAY_AMBIENT_SPEECH1 for parameter specifications.
</summary>
]]--
native "_PLAY_AMBIENT_SPEECH2"
	hash "0xC6941B4A3A8FBBB9"
	jhash (0x444180DB)
	arguments {
		Ped "pedHandle",

		charPtr "speechName",

		charPtr "speechParam",
	}
	returns	"void"

--[[!
<summary>
	This is the same as _PLAY_AMBIENT_SPEECH1 and _PLAY_AMBIENT_SPEECH2 but it will allow you to play a speech file from a specific voice file. It works on players and all peds, even animals.

	EX (C#):
	GTA.Native.Function.Call(Hash._0x3523634255FC3318, Game.Player.Character, "GENERIC_INSULT_HIGH", "s_m_y_sheriff_01_white_full_01", "SPEECH_PARAMS_FORCE_SHOUTED", 0);

	The first param is the ped you want to play it on, the second is the speech name, the third is the voice name, the fourth is the speech param, and the last param is usually always 0.


</summary>
]]--
native "_PLAY_AMBIENT_SPEECH_WITH_VOICE"
	hash "0x3523634255FC3318"
	jhash (0x8386AE28)
	arguments {
		Ped "p0",

		charPtr "speechName",

		charPtr "voiceName",

		charPtr "speechParam",

		BOOL "p4",
	}
	returns	"void"

native "0xED640017ED337E45"
	hash "0xED640017ED337E45"
	jhash (0xA1A1402E)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		float "p2",

		float "p3",

		float "p4",

		AnyPtr "p5",
	}
	returns	"void"

native "OVERRIDE_TREVOR_RAGE"
	hash "0x13AD665062541A7E"
	jhash (0x05B9B5CF)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "RESET_TREVOR_RAGE"
	hash "0xE78503B10C4314E0"
	jhash (0xE80CF0D4)
	returns	"void"

native "SET_PLAYER_ANGRY"
	hash "0xEA241BB04110F091"
	jhash (0x782CA58D)
	arguments {
		Player "player",

		BOOL "IsAngry",
	}
	returns	"void"

--[[!
<summary>
	Last 2 parameters always seem to be 0.

	EX: Function.Call(Hash.PLAY_PAIN, TestPed, 6, 0, 0);

	Known Pain IDs
	________________________

	1 - CRASHES GAME (DON'T USE)
	6 - Scream (Short)
	7 - Scared Scream (Kinda Long)
	8 - On Fire


</summary>
]]--
native "PLAY_PAIN"
	hash "0xBC9AE166038A5CEC"
	jhash (0x874BD6CB)
	arguments {
		int "painID",

		float "p1",

		int "p2",
	}
	returns	"void"

native "0xD01005D2BA2EB778"
	hash "0xD01005D2BA2EB778"
	jhash (0x59A3A17D)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xDDC635D5B3262C56"
	hash "0xDDC635D5B3262C56"
	jhash (0x0E387BFE)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

--[[!
<summary>


	Audio List
	http://gtaforums.com/topic/795622-audio-for-mods/
</summary>
]]--
native "SET_AMBIENT_VOICE_NAME"
	hash "0x6C8065A3B780185B"
	jhash (0xBD2EA1A1)
	arguments {
		Ped "ped",

		charPtr "name",
	}
	returns	"void"

native "0x40CF0D12D142A9E8"
	hash "0x40CF0D12D142A9E8"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x7CDC8C3B89F661B3"
	hash "0x7CDC8C3B89F661B3"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xA5342D390CDA41D6"
	hash "0xA5342D390CDA41D6"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x7A73D05A607734C7"
	hash "0x7A73D05A607734C7"
	arguments {
		Any "p0",
	}
	returns	"void"

native "STOP_CURRENT_PLAYING_AMBIENT_SPEECH"
	hash "0xB8BEC0CA6F0EDB0F"
	jhash (0xBB8E64BF)
	arguments {
		Ped "p0",
	}
	returns	"void"

native "IS_AMBIENT_SPEECH_PLAYING"
	hash "0x9072C8B49907BFAD"
	jhash (0x1972E8AA)
	arguments {
		Ped "p0",
	}
	returns	"BOOL"

native "IS_SCRIPTED_SPEECH_PLAYING"
	hash "0xCC9AA18DCC7084F4"
	jhash (0x2C653904)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "IS_ANY_SPEECH_PLAYING"
	hash "0x729072355FA39EC9"
	jhash (0x2B74A6D6)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x49B99BF3FDA89A7A"
	hash "0x49B99BF3FDA89A7A"
	jhash (0x8BD5F11E)
	arguments {
		Any "p0",

		AnyPtr "p1",

		BOOL "p2",
	}
	returns	"BOOL"

native "IS_PED_IN_CURRENT_CONVERSATION"
	hash "0x049E937F18F4020C"
	jhash (0x7B2F0743)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	Why is this in the audio section?
</summary>
]]--
native "SET_PED_IS_DRUNK"
	hash "0x95D2D383D5396B8A"
	jhash (0xD2EA77A3)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xEE066C7006C49C0A"
	hash "0xEE066C7006C49C0A"
	jhash (0x498849F3)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0xC265DF9FB44A9FBD"
	hash "0xC265DF9FB44A9FBD"
	jhash (0x0CBAF2EF)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SET_ANIMAL_MOOD"
	hash "0xCC97B29285B1DC3B"
	jhash (0x3EA7C6CB)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "IS_MOBILE_PHONE_RADIO_ACTIVE"
	hash "0xB35CE999E8EF317E"
	jhash (0x6E502A5B)
	returns	"BOOL"

native "SET_MOBILE_PHONE_RADIO_STATE"
	hash "0xBF286C554784F3DF"
	jhash (0xE1E0ED34)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "GET_PLAYER_RADIO_STATION_INDEX"
	hash "0xE8AF77C4C06ADC93"
	jhash (0x1C4946AC)
	returns	"int"

--[[!
<summary>
	Returns active radio station name
</summary>
]]--
native "GET_PLAYER_RADIO_STATION_NAME"
	hash "0xF6D733C32076AD03"
	jhash (0xD909C107)
	returns	"charPtr"

--[[!
<summary>
	Returns String with radio station name.
</summary>
]]--
native "GET_RADIO_STATION_NAME"
	hash "0xB28ECA15046CA8B9"
	jhash (0x3DF493BC)
	arguments {
		int "radioStation",
	}
	returns	"charPtr"

native "GET_PLAYER_RADIO_STATION_GENRE"
	hash "0xA571991A7FE6CCEB"
	jhash (0x872CF0EA)
	returns	"Any"

native "IS_RADIO_RETUNING"
	hash "0xA151A7394A214E65"
	jhash (0xCF29097B)
	returns	"BOOL"

native "0x0626A247D2405330"
	hash "0x0626A247D2405330"
	returns	"Any"

--[[!
<summary>
	Tune Forward... ?
</summary>
]]--
native "0xFF266D1D0EB1195D"
	hash "0xFF266D1D0EB1195D"
	jhash (0x53DB6994)
	returns	"void"

--[[!
<summary>
	Tune Backwards... ?
</summary>
]]--
native "0xDD6BCF9E94425DF9"
	hash "0xDD6BCF9E94425DF9"
	jhash (0xD70ECC80)
	returns	"void"

native "SET_RADIO_TO_STATION_NAME"
	hash "0xC69EDA28699D5107"
	jhash (0x7B36E35E)
	arguments {
		charPtr "radioStation",
	}
	returns	"Any"

--[[!
<summary>
	Stations:
	RADIO_01_CLASS_ROCK
	RADIO_02_POP
	RADIO_03_HIPHOP_NEW
	RADIO_04_PUNK
	RADIO_05_TALK_01
	RADIO_06_COUNTRY
	RADIO_07_DANCE_01
	RADIO_08_MEXICAN
	RADIO_09_HIPHOP_OLD
	RADIO_11_TALK_02
	RADIO_12_REGGAE
	RADIO_13_JAZZ
	RADIO_14_DANCE_02
	RADIO_15_MOTOWN
	RADIO_16_SILVERLAKE
	RADIO_17_FUNK
	RADIO_18_90s_ROCK
	RADIO_OFF

	~leonardosc
</summary>
]]--
native "SET_VEH_RADIO_STATION"
	hash "0x1B9C0099CB942AC6"
	jhash (0xE391F55F)
	arguments {
		Vehicle "vehicle",

		charPtr "radioStation",
	}
	returns	"void"

native "0xC1805D05E6D4FE10"
	hash "0xC1805D05E6D4FE10"
	jhash (0x7ABB89D2)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_EMITTER_RADIO_STATION"
	hash "0xACF57305B12AF907"
	jhash (0x87431585)
	arguments {
		charPtr "emitterName",

		charPtr "radioStation",
	}
	returns	"void"

native "SET_STATIC_EMITTER_ENABLED"
	hash "0x399D2D3B33F1B8EB"
	jhash (0x91F72E92)
	arguments {
		AnyPtr "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Sets radio station by index.
</summary>
]]--
native "SET_RADIO_TO_STATION_INDEX"
	hash "0xA619B168B8A8570F"
	jhash (0x1D82766D)
	arguments {
		int "radioStation",
	}
	returns	"void"

native "SET_FRONTEND_RADIO_ACTIVE"
	hash "0xF7F26C6E9CC9EBB8"
	jhash (0xB1172075)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "UNLOCK_MISSION_NEWS_STORY"
	hash "0xB165AB7C248B2DC1"
	jhash (0xCCD9ABE4)
	arguments {
		Any "p0",
	}
	returns	"void"

native "GET_NUMBER_OF_PASSENGER_VOICE_VARIATIONS"
	hash "0x66E49BF55B4B1874"
	jhash (0x27305D37)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_AUDIBLE_MUSIC_TRACK_TEXT_ID"
	hash "0x50B196FC9ED6545B"
	jhash (0xA2B88CA7)
	returns	"Any"

native "PLAY_END_CREDITS_MUSIC"
	hash "0xCD536C4D33DCC900"
	jhash (0x8E88B3CC)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SKIP_RADIO_FORWARD"
	hash "0x6DDBBDD98E2E9C25"
	jhash (0x10D36630)
	returns	"void"

native "FREEZE_RADIO_STATION"
	hash "0x344F393B027E38C3"
	jhash (0x286BF543)
	arguments {
		charPtr "radioStation",
	}
	returns	"void"

native "UNFREEZE_RADIO_STATION"
	hash "0xFC00454CF60B91DD"
	jhash (0x4D46202C)
	arguments {
		charPtr "radioStation",
	}
	returns	"void"

native "SET_RADIO_AUTO_UNFREEZE"
	hash "0xC1AA9F53CE982990"
	jhash (0xA40196BF)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Sets startup radio station for player (char *radioStation i think)
</summary>
]]--
native "SET_INITIAL_PLAYER_STATION"
	hash "0x88795F13FACDA88D"
	jhash (0x9B069233)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "SET_USER_RADIO_CONTROL_ENABLED"
	hash "0x19F21E63AE6EAE4E"
	jhash (0x52E054CE)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_RADIO_TRACK"
	hash "0xB39786F201FEE30B"
	jhash (0x76E96212)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "SET_VEHICLE_RADIO_LOUD"
	hash "0xBB6F1CAEC68B0BCE"
	jhash (0x8D9EDD99)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "_IS_VEHICLE_RADIO_LOUD"
	hash "0x032A116663A4D5AC"
	jhash (0xCBA99F4A)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	Enables Radio on phone.

	Turn On:
	SET_MOBILE_RADIO_ENABLED_DURING_GAMEPLAY(1);

	Turn Off:
	SET_MOBILE_RADIO_ENABLED_DURING_GAMEPLAY(0);

	~ISOFX
</summary>
]]--
native "SET_MOBILE_RADIO_ENABLED_DURING_GAMEPLAY"
	hash "0x1098355A16064BB3"
	jhash (0x990085F0)
	arguments {
		BOOL "Toggle",
	}
	returns	"void"

native "0x109697E2FFBAC8A1"
	hash "0x109697E2FFBAC8A1"
	jhash (0x46B0C696)
	returns	"Any"

native "0x5F43D83FD6738741"
	hash "0x5F43D83FD6738741"
	jhash (0x2A3E5E8B)
	returns	"Any"

native "SET_VEHICLE_RADIO_ENABLED"
	hash "0x3B988190C0AA6C0B"
	jhash (0x6F812CAB)
	arguments {
		Vehicle "vehicle",

		BOOL "toggle",
	}
	returns	"void"

native "0x4E404A9361F75BB2"
	hash "0x4E404A9361F75BB2"
	jhash (0x128C3873)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x1654F24A88A8E3FE"
	hash "0x1654F24A88A8E3FE"
	jhash (0x1D766976)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "_MAX_RADIO_STATION_INDEX"
	hash "0xF1620ECB50E01DE7"
	jhash (0xCC91FCF5)
	returns	"int"

native "FIND_RADIO_STATION_INDEX"
	hash "0x8D67489793FF428B"
	jhash (0xECA1512F)
	arguments {
		int "station",
	}
	returns	"Any"

native "0x774BD811F656A122"
	hash "0x774BD811F656A122"
	jhash (0xB1FF7137)
	arguments {
		AnyPtr "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x2C96CDB04FCA358E"
	hash "0x2C96CDB04FCA358E"
	jhash (0xC8B514E2)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x031ACB6ABA18C729"
	hash "0x031ACB6ABA18C729"
	jhash (0xBE998184)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0xF3365489E0DD50F9"
	hash "0xF3365489E0DD50F9"
	jhash (0x8AFC488D)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_AMBIENT_ZONE_STATE"
	hash "0xBDA07E5950085E46"
	jhash (0x2849CAC9)
	arguments {
		AnyPtr "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "CLEAR_AMBIENT_ZONE_STATE"
	hash "0x218DD44AAAC964FF"
	jhash (0xCDFF3C82)
	arguments {
		AnyPtr "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_AMBIENT_ZONE_LIST_STATE"
	hash "0x9748FA4DE50CCE3E"
	jhash (0xBF80B412)
	arguments {
		AnyPtr "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "CLEAR_AMBIENT_ZONE_LIST_STATE"
	hash "0x120C48C614909FA4"
	jhash (0x38B9B8D4)
	arguments {
		AnyPtr "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_AMBIENT_ZONE_STATE_PERSISTENT"
	hash "0x1D6650420CEC9D3B"
	jhash (0xC1FFB672)
	arguments {
		AnyPtr "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "SET_AMBIENT_ZONE_LIST_STATE_PERSISTENT"
	hash "0xF3638DAE8C4045E1"
	jhash (0x5F5A2605)
	arguments {
		AnyPtr "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "IS_AMBIENT_ZONE_ENABLED"
	hash "0x01E2817A479A7F9B"
	jhash (0xBFABD872)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "SET_CUTSCENE_AUDIO_OVERRIDE"
	hash "0x3B4BF5F0859204D9"
	jhash (0xCE1332B7)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "GET_PLAYER_HEADSET_SOUND_ALTERNATE"
	hash "0xBCC29F935ED07688"
	jhash (0xD63CF33A)
	arguments {
		AnyPtr "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	Plays the given police radio message.
</summary>
]]--
native "PLAY_POLICE_REPORT"
	hash "0xDFEBD56D9BD1EB16"
	jhash (0x3F277B62)
	arguments {
		charPtr "name",

		float "p1",
	}
	returns	"Any"

native "0xB4F90FAF7670B16F"
	hash "0xB4F90FAF7670B16F"
	returns	"void"

--[[!
<summary>
	Plays the siren sound of a vehicle which is otherwise activated when fastly double-pressing the horn key.
	Only works on vehicles with a police siren.
</summary>
]]--
native "BLIP_SIREN"
	hash "0x1B9025BDA76822B6"
	jhash (0xC0EB6924)
	arguments {
		Vehicle "vehicle",
	}
	returns	"void"

--[[!
<summary>
	vehicle - the vehicle whose horn should be overwritten
	mute - p1 seems to be an option for muting the horn
	p2 - maybe a horn id, since the function AUDIO::GET_VEHICLE_DEFAULT_HORN(veh) exists?
</summary>
]]--
native "OVERRIDE_VEH_HORN"
	hash "0x3CDC1E622CCE0356"
	jhash (0x2ACAB783)
	arguments {
		Vehicle "vehicle",

		BOOL "mute",

		int "p2",
	}
	returns	"void"

--[[!
<summary>
	Checks whether the horn of a vehicle is currently played.
</summary>
]]--
native "IS_HORN_ACTIVE"
	hash "0x9D6BFC12B05C6121"
	jhash (0x20E2BDD0)
	arguments {
		Vehicle "vehicle",
	}
	returns	"BOOL"

--[[!
<summary>
	Makes pedestrians sound their horn longer, faster and more agressive when they use their horn.
</summary>
]]--
native "SET_AGGRESSIVE_HORNS"
	hash "0x395BF71085D1B1D9"
	jhash (0x01D6EABE)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0x02E93C796ABD3A97"
	hash "0x02E93C796ABD3A97"
	jhash (0x3C395AEE)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x58BB377BEC7CD5F4"
	hash "0x58BB377BEC7CD5F4"
	jhash (0x8CE63FA1)
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_STREAM_PLAYING"
	hash "0xD11FA52EB849D978"
	jhash (0xF1F51A14)
	returns	"BOOL"

native "GET_STREAM_PLAY_TIME"
	hash "0x4E72BBDBCA58A3DB"
	jhash (0xB4F0AD56)
	returns	"Any"

native "LOAD_STREAM"
	hash "0x1F1F957154EC51DF"
	jhash (0x0D89599D)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "LOAD_STREAM_WITH_START_OFFSET"
	hash "0x59C16B79F53B3712"
	jhash (0xE5B5745C)
	arguments {
		AnyPtr "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0x89049DD63C08B5D1"
	hash "0x89049DD63C08B5D1"
	jhash (0xA1D7FABE)
	arguments {
		Any "p0",
	}
	returns	"void"

native "PLAY_STREAM_FROM_VEHICLE"
	hash "0xB70374A758007DFA"
	jhash (0xF8E4BDA2)
	arguments {
		Any "p0",
	}
	returns	"void"

native "PLAY_STREAM_FROM_OBJECT"
	hash "0xEBAA9B64D76356FD"
	jhash (0xC5266BF7)
	arguments {
		Any "p0",
	}
	returns	"void"

native "PLAY_STREAM_FRONTEND"
	hash "0x58FCE43488F9F5F4"
	jhash (0x2C2A16BC)
	returns	"void"

native "SPECIAL_FRONTEND_EQUAL"
	hash "0x21442F412E8DE56B"
	jhash (0x6FE5D865)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "STOP_STREAM"
	hash "0xA4718A1419D18151"
	jhash (0xD1E364DE)
	returns	"void"

native "STOP_PED_SPEAKING"
	hash "0x9D64D7405520E3D3"
	jhash (0xFF92B49D)
	arguments {
		Ped "ped",

		BOOL "shaking",
	}
	returns	"void"

native "DISABLE_PED_PAIN_AUDIO"
	hash "0xA9A41C1E940FB0E8"
	jhash (0x3B8E2D5F)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

native "IS_AMBIENT_SPEECH_DISABLED"
	hash "0x932C2D096A2C3FFF"
	jhash (0x109D1F89)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	This native is the same exact spelled and same amount of parameters of the GTA IV version. So I am going to assume it does the same. So I renamed the parameters as the same as the GTA IV one.
</summary>
]]--
native "SET_SIREN_WITH_NO_DRIVER"
	hash "0x1FEF0683B96EBCF2"
	jhash (0x77182D58)
	arguments {
		Vehicle "vehicle",

		BOOL "set",
	}
	returns	"void"

native "0x9C11908013EA4715"
	hash "0x9C11908013EA4715"
	jhash (0xDE8BA3CD)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_HORN_ENABLED"
	hash "0x76D683C108594D0E"
	jhash (0x6EB92D05)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",
	}
	returns	"void"

native "SET_AUDIO_VEHICLE_PRIORITY"
	hash "0xE5564483E407F914"
	jhash (0x271A9766)
	arguments {
		Vehicle "vehicle",

		Any "p1",
	}
	returns	"void"

native "0x9D3AF56E94C9AE98"
	hash "0x9D3AF56E94C9AE98"
	jhash (0x2F0A16D1)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "USE_SIREN_AS_HORN"
	hash "0xFA932DE350266EF8"
	jhash (0xC6BC16F3)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x4F0C413926060B38"
	hash "0x4F0C413926060B38"
	jhash (0x33B0B007)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0xF1F8157B8C3F171C"
	hash "0xF1F8157B8C3F171C"
	jhash (0x1C0C5E4C)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0xD2DCCD8E16E20997"
	hash "0xD2DCCD8E16E20997"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x5DB8010EE71FDEF2"
	hash "0x5DB8010EE71FDEF2"
	jhash (0x6E660D3F)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x59E7B488451F4D3A"
	hash "0x59E7B488451F4D3A"
	jhash (0x23BE6432)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x01BB4D577D38BD9E"
	hash "0x01BB4D577D38BD9E"
	jhash (0xE81FAC68)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x1C073274E065C6D2"
	hash "0x1C073274E065C6D2"
	jhash (0x9365E042)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x2BE4BC731D039D5A"
	hash "0x2BE4BC731D039D5A"
	jhash (0x2A60A90E)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_VEHICLE_BOOST_ACTIVE"
	hash "0x4A04DE7CAB2739A1"
	jhash (0x072F15F2)
	arguments {
		Vehicle "vehicle",

		BOOL "Toggle",
	}
	returns	"Any"

native "0x6FDDAD856E36988A"
	hash "0x6FDDAD856E36988A"
	jhash (0x934BE749)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x06C0023BED16DD6B"
	hash "0x06C0023BED16DD6B"
	jhash (0xE61110A2)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	p1 appears to only be "0" or "3". I personally use "0" as p1.
</summary>
]]--
native "PLAY_VEHICLE_DOOR_OPEN_SOUND"
	hash "0x3A539D52857EA82D"
	jhash (0x84930330)
	arguments {
		Vehicle "vehicle",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	This native only comes up once. And in that one instance, p1 is "1".
</summary>
]]--
native "PLAY_VEHICLE_DOOR_CLOSE_SOUND"
	hash "0x62A456AA4769EF34"
	jhash (0xBA2CF407)
	arguments {
		Vehicle "vehicle",

		Any "p1",
	}
	returns	"void"

native "0xC15907D667F7CFB2"
	hash "0xC15907D667F7CFB2"
	jhash (0x563B635D)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_GAME_IN_CONTROL_OF_MUSIC"
	hash "0x6D28DC1671E334FD"
	jhash (0x7643170D)
	returns	"BOOL"

native "SET_GPS_ACTIVE"
	hash "0x3BD3F52BA9B1E4E8"
	jhash (0x0FC3379A)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "PLAY_MISSION_COMPLETE_AUDIO"
	hash "0xB138AAB8A70D3C69"
	jhash (0x3033EA1D)
	arguments {
		charPtr "p0",
	}
	returns	"Any"

native "IS_MISSION_COMPLETE_PLAYING"
	hash "0x19A30C23F5827F8A"
	jhash (0x939982A1)
	returns	"BOOL"

native "0x6F259F82D873B8B8"
	hash "0x6F259F82D873B8B8"
	jhash (0xCBE09AEC)
	returns	"Any"

native "0xF154B8D1775B2DEC"
	hash "0xF154B8D1775B2DEC"
	jhash (0xD2858D8A)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "START_AUDIO_SCENE"
	hash "0x013A80FC08F6E4F2"
	jhash (0xE48D757B)
	arguments {
		charPtr "p0",
	}
	returns	"BOOL"

native "STOP_AUDIO_SCENE"
	hash "0xDFE8422B3B94E688"
	jhash (0xA08D8C58)
	arguments {
		charPtr "scene",
	}
	returns	"void"

native "STOP_AUDIO_SCENES"
	hash "0xBAC7FC81A75EC1A1"
	jhash (0xF6C7342A)
	returns	"void"

native "IS_AUDIO_SCENE_ACTIVE"
	hash "0xB65B60556E2A9225"
	jhash (0xACBED05C)
	arguments {
		charPtr "scene",
	}
	returns	"BOOL"

native "SET_AUDIO_SCENE_VARIABLE"
	hash "0xEF21A9EF089A2668"
	jhash (0x19BB3CE8)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		float "p2",
	}
	returns	"void"

native "0xA5F377B175A699C5"
	hash "0xA5F377B175A699C5"
	jhash (0xE812925D)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x153973AB99FE8980"
	hash "0x153973AB99FE8980"
	jhash (0x2BC93264)
	arguments {
		Entity "p0",

		charPtr "p1",

		float "p2",
	}
	returns	"void"

native "0x18EB48CFC41F2EA0"
	hash "0x18EB48CFC41F2EA0"
	jhash (0x308ED0EC)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "AUDIO_IS_SCRIPTED_MUSIC_PLAYING"
	hash "0x845FFC3A4FEEFA3E"
	jhash (0x86E995D1)
	returns	"Any"

native "PREPARE_MUSIC_EVENT"
	hash "0x1E5185B72EF5158A"
	jhash (0x534A5C1C)
	arguments {
		charPtr "eventName",
	}
	returns	"BOOL"

native "CANCEL_MUSIC_EVENT"
	hash "0x5B17A90291133DA5"
	jhash (0x89FF942D)
	arguments {
		charPtr "eventName",
	}
	returns	"BOOL"

native "TRIGGER_MUSIC_EVENT"
	hash "0x706D57B0F50DA710"
	jhash (0xB6094948)
	arguments {
		charPtr "eventName",
	}
	returns	"BOOL"

native "0xA097AB275061FB21"
	hash "0xA097AB275061FB21"
	jhash (0x2705C4D5)
	returns	"Any"

native "GET_MUSIC_PLAYTIME"
	hash "0xE7A0D23DC414507B"
	jhash (0xD633C809)
	returns	"Any"

native "0xFBE20329593DEC9D"
	hash "0xFBE20329593DEC9D"
	jhash (0x53FC3FEC)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "CLEAR_ALL_BROKEN_GLASS"
	hash "0xB32209EFFDC04913"
	jhash (0xE6B033BF)
	returns	"Any"

native "0x70B8EC8FC108A634"
	hash "0x70B8EC8FC108A634"
	jhash (0x95050CAD)
	arguments {
		BOOL "p0",

		Any "p1",
	}
	returns	"void"

native "0x149AEE66F0CB3A99"
	hash "0x149AEE66F0CB3A99"
	jhash (0xE64F97A0)
	arguments {
		float "p0",

		float "p1",
	}
	returns	"void"

native "0x8BF907833BE275DE"
	hash "0x8BF907833BE275DE"
	arguments {
		float "p0",

		float "p1",
	}
	returns	"void"

native "0x062D5EAD4DA2FA6A"
	hash "0x062D5EAD4DA2FA6A"
	jhash (0xD87AF337)
	returns	"void"

native "PREPARE_ALARM"
	hash "0x9D74AE343DB65533"
	jhash (0x084932E8)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "START_ALARM"
	hash "0x0355EF116C4C97B2"
	jhash (0x703F524B)
	arguments {
		AnyPtr "p0",

		BOOL "p1",
	}
	returns	"void"

native "STOP_ALARM"
	hash "0xA1CADDCD98415A41"
	jhash (0xF987BE8C)
	arguments {
		AnyPtr "p0",

		BOOL "p1",
	}
	returns	"void"

native "STOP_ALL_ALARMS"
	hash "0x2F794A877ADD4C92"
	jhash (0xC3CB9DC6)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "IS_ALARM_PLAYING"
	hash "0x226435CB96CCFC8C"
	jhash (0x9D8E1D23)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns hash of default vehicle horn
</summary>
]]--
native "GET_VEHICLE_DEFAULT_HORN"
	hash "0x02165D55000219AC"
	jhash (0xE84ABC19)
	arguments {
		Vehicle "veh",
	}
	returns	"Hash"

native "0xACB5DCCA1EC76840"
	hash "0xACB5DCCA1EC76840"
	jhash (0xFD4B5B3B)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "RESET_PED_AUDIO_FLAGS"
	hash "0xF54BB7B61036F335"
	jhash (0xDF720C86)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xD2CC78CD3D0B50F9"
	hash "0xD2CC78CD3D0B50F9"
	jhash (0xC307D531)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xBF4DC1784BE94DFA"
	hash "0xBF4DC1784BE94DFA"
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",
	}
	returns	"void"

native "0x75773E11BA459E90"
	hash "0x75773E11BA459E90"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xD57AAAE0E2214D11"
	hash "0xD57AAAE0E2214D11"
	returns	"void"

native "0x552369F549563AD5"
	hash "0x552369F549563AD5"
	jhash (0x13EB5861)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x43FA0DFC5DF87815"
	hash "0x43FA0DFC5DF87815"
	jhash (0x7BED1872)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Possible flag names:
	(updated from b393d scripts - MoMadenU - 8/23/15)

	"ActivateSwitchWheelAudio"
	"AllowAmbientSpeechInSlowMo"
	"AllowCutsceneOverScreenFade"
	"AllowForceRadioAfterRetune"
	"AllowPainAndAmbientSpeechToPlayDuringCutscene"
	"AllowPlayerAIOnMission"
	"AllowPoliceScannerWhenPlayerHasNoControl"
	"AllowRadioDuringSwitch"
	"AllowRadioOverScreenFade"
	"AllowScoreAndRadio"
	"AllowScriptedSpeechInSlowMo"
	"AvoidMissionCompleteDelay"
	"DisableAbortConversationForDeathAndInjury"
	"DisableAbortConversationForRagdoll"
	"DisableBarks"
	"DisableFlightMusic"
	"DisableReplayScriptStreamRecording"
	"EnableHeadsetBeep"
	"ForceConversationInterrupt"
	"ForceSeamlessRadioSwitch"
	"ForceSniperAudio"
	"FrontendRadioDisabled"
	"HoldMissionCompleteWhenPrepared"
	"IsDirectorModeActive"
	"IsPlayerOnMissionForSpeech"
	"ListenerReverbDisabled"
	"LoadMPData"
	"MobileRadioInGame"
	"OnlyAllowScriptTriggerPoliceScanner"
	"PlayMenuMusic"
	"PoliceScannerDisabled"
	"ScriptedConvListenerMaySpeak"
	"SpeechDucksScore"
	"SuppressPlayerScubaBreathing"
	"WantedMusicDisabled"
	"WantedMusicOnMission"


	#######################################################################

	"IsDirectorModeActive" is an audio flag which will allow you to play speech infinitely without any pauses like in Director Mode.


</summary>
]]--
native "SET_AUDIO_FLAG"
	hash "0xB9EFD5C25018725A"
	jhash (0x1C09C9E0)
	arguments {
		charPtr "flagName",

		BOOL "toggle",
	}
	returns	"void"

native "PREPARE_SYNCHRONIZED_AUDIO_EVENT"
	hash "0xC7ABCACA4985A766"
	jhash (0xE1D91FD0)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "PREPARE_SYNCHRONIZED_AUDIO_EVENT_FOR_SCENE"
	hash "0x029FE7CD1B7E2E75"
	jhash (0x7652DD49)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "PLAY_SYNCHRONIZED_AUDIO_EVENT"
	hash "0x8B2FD4560E55DD2D"
	jhash (0x507F3241)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "STOP_SYNCHRONIZED_AUDIO_EVENT"
	hash "0x92D6A88E64A94430"
	jhash (0xADEED2B4)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xC8EDE9BDBCCBA6D4"
	hash "0xC8EDE9BDBCCBA6D4"
	jhash (0x55A21772)
	arguments {
		AnyPtr "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "0x950A154B8DAB6185"
	hash "0x950A154B8DAB6185"
	jhash (0xA17F9AB0)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"void"

native "0x12561FCBB62D5B9C"
	hash "0x12561FCBB62D5B9C"
	jhash (0x62B43677)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x044DBAD7A7FA2BE5"
	hash "0x044DBAD7A7FA2BE5"
	jhash (0x8AD670EC)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0xB4BBFD9CD8B3922B"
	hash "0xB4BBFD9CD8B3922B"
	jhash (0xD24B4D0C)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xE4E6DD5566D28C82"
	hash "0xE4E6DD5566D28C82"
	jhash (0x7262B5BA)
	returns	"void"

native "0x3A48AB4445D499BE"
	hash "0x3A48AB4445D499BE"
	jhash (0x93A44A1F)
	returns	"Any"

native "0x4ADA3F19BE4A6047"
	hash "0x4ADA3F19BE4A6047"
	jhash (0x13777A0B)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x0150B6FF25A9E2E5"
	hash "0x0150B6FF25A9E2E5"
	jhash (0x1134F68B)
	returns	"void"

native "0xBEF34B1D9624D5DD"
	hash "0xBEF34B1D9624D5DD"
	jhash (0xE0047BFD)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x806058BBDC136E06"
	hash "0x806058BBDC136E06"
	returns	"void"

native "0x544810ED9DB6BBE6"
	hash "0x544810ED9DB6BBE6"
	returns	"Any"

native "0x5B50ABB1FE3746F4"
	hash "0x5B50ABB1FE3746F4"
	returns	"Any"

--[[!
<summary>
	Args: char *cutsceneName, int (generally "8")
</summary>
]]--
native "REQUEST_CUTSCENE"
	hash "0x7A86743F475D9E09"
	jhash (0xB5977853)
	arguments {
		charPtr "cutsceneName",

		int "p1",
	}
	returns	"void"

native "0xC23DE0E91C30B58C"
	hash "0xC23DE0E91C30B58C"
	jhash (0xD98F656A)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "REMOVE_CUTSCENE"
	hash "0x440AF51A3462B86F"
	jhash (0x8052F533)
	returns	"void"

native "HAS_CUTSCENE_LOADED"
	hash "0xC59F528E9AB9F339"
	jhash (0xF9998582)
	returns	"BOOL"

native "HAS_THIS_CUTSCENE_LOADED"
	hash "0x228D3D94F8A11C3C"
	jhash (0x3C5619F2)
	arguments {
		charPtr "cutsceneName",
	}
	returns	"BOOL"

native "0x8D9DF6ECA8768583"
	hash "0x8D9DF6ECA8768583"
	jhash (0x25A2CABC)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xB56BBBCC2955D9CB"
	hash "0xB56BBBCC2955D9CB"
	jhash (0xDD8878E9)
	returns	"Any"

native "0x71B74D2AE19338D0"
	hash "0x71B74D2AE19338D0"
	jhash (0x7B93CDAA)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x4C61C75BEE8184C2"
	hash "0x4C61C75BEE8184C2"
	jhash (0x47DB08A9)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0x06A3524161C502BA"
	hash "0x06A3524161C502BA"
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xA1C996C2A744262E"
	hash "0xA1C996C2A744262E"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0xD00D76A7DFC9D852"
	hash "0xD00D76A7DFC9D852"
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x0ABC54DE641DC0FC"
	hash "0x0ABC54DE641DC0FC"
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "START_CUTSCENE"
	hash "0x186D5CB5E7B0FF7B"
	jhash (0x210106F6)
	arguments {
		charPtr "cutsceneName",
	}
	returns	"void"

native "START_CUTSCENE_AT_COORDS"
	hash "0x1C9ADDA3244A1FBF"
	jhash (0x58BEA436)
	arguments {
		charPtr "cutsceneName",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "STOP_CUTSCENE"
	hash "0xC7272775B4DC786E"
	jhash (0x5EE84DC7)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "STOP_CUTSCENE_IMMEDIATELY"
	hash "0xD220BDD222AC4A1E"
	jhash (0xF528A2AD)
	returns	"void"

native "SET_CUTSCENE_ORIGIN"
	hash "0xB812B3FD1C01CF27"
	jhash (0xB0AD7792)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "0x011883F41211432A"
	hash "0x011883F41211432A"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"void"

native "GET_CUTSCENE_TIME"
	hash "0xE625BEABBAFFDAB9"
	jhash (0x53F5B5AB)
	returns	"Any"

native "GET_CUTSCENE_TOTAL_DURATION"
	hash "0xEE53B14A19E480D4"
	jhash (0x0824EBE8)
	returns	"Any"

native "WAS_CUTSCENE_SKIPPED"
	hash "0x40C8656EDAEDD569"
	jhash (0xC9B6949D)
	returns	"BOOL"

native "HAS_CUTSCENE_FINISHED"
	hash "0x7C0A893088881D57"
	jhash (0x5DED14B4)
	returns	"BOOL"

native "IS_CUTSCENE_ACTIVE"
	hash "0x991251AFC3981F84"
	jhash (0xCCE2FE9D)
	returns	"BOOL"

native "IS_CUTSCENE_PLAYING"
	hash "0xD3C2E180A40F031E"
	jhash (0xA3A78392)
	returns	"BOOL"

native "GET_CUTSCENE_SECTION_PLAYING"
	hash "0x49010A6A396553D8"
	jhash (0x1026F0D6)
	returns	"Any"

native "GET_ENTITY_INDEX_OF_CUTSCENE_ENTITY"
	hash "0x0A2E9FDB9A8C62F6"
	jhash (0x1D09ABC7)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"Any"

native "0x583DF8E3D4AFBD98"
	hash "0x583DF8E3D4AFBD98"
	jhash (0x5AE68AE6)
	returns	"Any"

native "0x4CEBC1ED31E8925E"
	hash "0x4CEBC1ED31E8925E"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "REGISTER_ENTITY_FOR_CUTSCENE"
	hash "0xE40C1C56DF95C2E8"
	jhash (0x7CBC3EC7)
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "GET_ENTITY_INDEX_OF_REGISTERED_ENTITY"
	hash "0xC0741A26499654CD"
	jhash (0x46D18755)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"Any"

native "0x7F96F23FA9B73327"
	hash "0x7F96F23FA9B73327"
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_CUTSCENE_TRIGGER_AREA"
	hash "0x9896CE4721BE84BA"
	jhash (0x9D76D9DE)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"void"

native "CAN_SET_ENTER_STATE_FOR_REGISTERED_ENTITY"
	hash "0x645D0B458D8E17B5"
	jhash (0x55C30B26)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"BOOL"

native "CAN_SET_EXIT_STATE_FOR_REGISTERED_ENTITY"
	hash "0x4C6A6451C79E4662"
	jhash (0x8FF5D3C4)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"BOOL"

native "CAN_SET_EXIT_STATE_FOR_CAMERA"
	hash "0xB2CBCD0930DFB420"
	jhash (0xEDAE6C02)
	arguments {
		BOOL "p0",
	}
	returns	"BOOL"

native "0xC61B86C9F61EB404"
	hash "0xC61B86C9F61EB404"
	jhash (0x35721A08)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_CUTSCENE_FADE_VALUES"
	hash "0x8093F23ABACCC7D4"
	jhash (0xD19EF0DD)
	arguments {
		BOOL "p0",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "0x20746F7B1032A3C7"
	hash "0x20746F7B1032A3C7"
	arguments {
		BOOL "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x06EE9048FD080382"
	hash "0x06EE9048FD080382"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xA0FE76168A189DDB"
	hash "0xA0FE76168A189DDB"
	returns	"int"

native "0x2F137B508DE238F2"
	hash "0x2F137B508DE238F2"
	jhash (0x8338DA1D)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xE36A98D8AB3D3C66"
	hash "0xE36A98D8AB3D3C66"
	jhash (0x04377C10)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x5EDEF0CF8C1DAB3C"
	hash "0x5EDEF0CF8C1DAB3C"
	jhash (0xDBD88708)
	returns	"Any"

native "0x41FAA8FB2ECE8720"
	hash "0x41FAA8FB2ECE8720"
	jhash (0x28D54A7F)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "REGISTER_SYNCHRONISED_SCRIPT_SPEECH"
	hash "0x2131046957F31B04"
	jhash (0xB60CFBB9)
	returns	"void"

native "SET_CUTSCENE_PED_COMPONENT_VARIATION"
	hash "0xBA01E7B6DEEFBBC9"
	jhash (0x6AF994A1)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "0x2A56C06EBEF2B0D9"
	hash "0x2A56C06EBEF2B0D9"
	jhash (0x1E7DA95E)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "DOES_CUTSCENE_ENTITY_EXIST"
	hash "0x499EF20C5DB25C59"
	jhash (0x58E67409)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x0546524ADE2E9723"
	hash "0x0546524ADE2E9723"
	jhash (0x22E9A9DE)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "0x708BDD8CD795B043"
	hash "0x708BDD8CD795B043"
	jhash (0x4315A7C5)
	returns	"Any"

native "GET_INTERIOR_GROUP_ID"
	hash "0xE4A84ABF135EF91A"
	jhash (0x09D6376F)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_OFFSET_FROM_INTERIOR_IN_WORLD_COORDS"
	hash "0x9E3B3E6D66F6E22F"
	jhash (0x7D8F26A1)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"Vector3"

native "IS_INTERIOR_SCENE"
	hash "0xBC72B5D7A1CBD54D"
	jhash (0x55226C13)
	returns	"BOOL"

--[[!
<summary>
	Return if interior is valid.
</summary>
]]--
native "IS_VALID_INTERIOR"
	hash "0x26B0E73D7EAAF4D3"
	jhash (0x39C0B635)
	arguments {
		int "interiorID",
	}
	returns	"BOOL"

native "CLEAR_ROOM_FOR_ENTITY"
	hash "0xB365FC0C4E27FFA7"
	jhash (0x7DDADB92)
	arguments {
		Entity "entity",
	}
	returns	"void"

native "FORCE_ROOM_FOR_ENTITY"
	hash "0x52923C4710DD9907"
	jhash (0x10BD4435)
	arguments {
		Entity "entity",

		Any "interiorID",

		Any "p0",
	}
	returns	"void"

native "GET_ROOM_KEY_FROM_ENTITY"
	hash "0x47C2A06D4F5F424B"
	jhash (0xE4ACF8C3)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_KEY_FOR_ENTITY_IN_ROOM"
	hash "0x399685DB942336BC"
	jhash (0x91EA80EF)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	Returns the handle of the interior that the entity is in. Returns 0 if outside.
</summary>
]]--
native "GET_INTERIOR_FROM_ENTITY"
	hash "0x2107BA504071A6BB"
	jhash (0x5C644614)
	arguments {
		Entity "p0",
	}
	returns	"int"

native "0x82EBB79E258FA2B7"
	hash "0x82EBB79E258FA2B7"
	jhash (0xE645E162)
	arguments {
		Entity "entity",

		int "id",
	}
	returns	"void"

native "0x920D853F3E17F1DA"
	hash "0x920D853F3E17F1DA"
	jhash (0xD79803B5)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	Exemple of use(carmod_shop.c4)
	 INTERIOR::_AF348AFCB575A441("V_CarModRoom");
</summary>
]]--
native "0xAF348AFCB575A441"
	hash "0xAF348AFCB575A441"
	jhash (0x1F6B4B13)
	arguments {
		charPtr "name",
	}
	returns	"void"

native "0x405DC2AEF6AF95B9"
	hash "0x405DC2AEF6AF95B9"
	jhash (0x0E9529CC)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xA6575914D2A0B450"
	hash "0xA6575914D2A0B450"
	jhash (0x4FF3D3F5)
	returns	"Any"

native "0x23B59D8912F94246"
	hash "0x23B59D8912F94246"
	jhash (0x617DC75D)
	returns	"void"

--[[!
<summary>
	Get interior ID from Coordinates

	Example for VB.NET
	Dim interiorID As Integer = Native.Function.Call(Of Integer)(Hash.GET_INTERIOR_AT_COORDS, X, Y, Z)
</summary>
]]--
native "GET_INTERIOR_AT_COORDS"
	hash "0xB0F7F8663821D9C3"
	jhash (0xA17FBF37)
	arguments {
		float "X",

		float "Y",

		float "Z",
	}
	returns	"int"

native "ADD_PICKUP_TO_INTERIOR_ROOM_BY_NAME"
	hash "0x3F6167F351168730"
	jhash (0xA2A73564)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

--[[!
<summary>
	Load interior
</summary>
]]--
native "0x2CA429C029CCF247"
	hash "0x2CA429C029CCF247"
	jhash (0x3ADA414E)
	arguments {
		int "interior",
	}
	returns	"void"

native "UNPIN_INTERIOR"
	hash "0x261CCE7EED010641"
	jhash (0xFCFF792A)
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_INTERIOR_READY"
	hash "0x6726BDCCC1932F0E"
	jhash (0xE1EF6450)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x4C2330E61D3DEB56"
	hash "0x4C2330E61D3DEB56"
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	Returns an integer representing the requested interior at that location (if found?). The supplied interior string is not the same as the one used to load the interior.

	Use: INTERIOR::UNPIN_INTERIOR(INTERIOR::_0x96525B06(x, y, z, interior))
</summary>
]]--
native "0x05B7A89BD78797FC"
	hash "0x05B7A89BD78797FC"
	jhash (0x96525B06)
	arguments {
		float "x",

		float "y",

		float "z",

		charPtr "interior",
	}
	returns	"int"

native "0xF0F77ADB9F67E79D"
	hash "0xF0F77ADB9F67E79D"
	arguments {
		float "p0",

		float "p1",

		float "p2",

		Any "p3",
	}
	returns	"Any"

native "0xEEA5AC2EDA7C33E8"
	hash "0xEEA5AC2EDA7C33E8"
	jhash (0x7762249C)
	arguments {
		float "p0",

		float "p1",

		float "p2",
	}
	returns	"BOOL"

native "GET_INTERIOR_FROM_COLLISION"
	hash "0xEC4CF9FCB29A4424"
	jhash (0x7ED33DC1)
	arguments {
		float "p0",

		float "p1",

		float "p2",
	}
	returns	"Any"

native "0x55E86AF2712B36A1"
	hash "0x55E86AF2712B36A1"
	jhash (0xC80A5DDF)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x420BD37289EEE162"
	hash "0x420BD37289EEE162"
	jhash (0xDBA768A1)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x35F7DD45E8C0A16D"
	hash "0x35F7DD45E8C0A16D"
	jhash (0x39A3CC6F)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "REFRESH_INTERIOR"
	hash "0x41F37C3427C75AE0"
	jhash (0x9A29ACE6)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xA97F257D0151A6AB"
	hash "0xA97F257D0151A6AB"
	jhash (0x1F375B4C)
	arguments {
		Any "p0",
	}
	returns	"void"

native "DISABLE_INTERIOR"
	hash "0x6170941419D7D8EC"
	jhash (0x093ADEA5)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_INTERIOR_DISABLED"
	hash "0xBC5115A5A939DD15"
	jhash (0x81F34C71)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "CAP_INTERIOR"
	hash "0xD9175F941610DB54"
	jhash (0x34E735A6)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_INTERIOR_CAPPED"
	hash "0x92BAC8ACF88CEC26"
	jhash (0x18B17C80)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x9E6542F0CE8E70A3"
	hash "0x9E6542F0CE8E70A3"
	jhash (0x5EF9C5C2)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	ease - smooth transition between the camera's positions
	camera - handle of the script camera

	If you have created a script (rendering) camera, and want to go back to the 
	character (gameplay) camera, call this native with render set to 0.
	Setting ease to 1 will smooth the transition.
</summary>
]]--
native "RENDER_SCRIPT_CAMS"
	hash "0x07E5B515DB0636FC"
	jhash (0x74337969)
	arguments {
		BOOL "render",

		BOOL "ease",

		Any "camera",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"void"

native "0xC819F3CBB62BF692"
	hash "0xC819F3CBB62BF692"
	jhash (0xD3C08183)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "CREATE_CAM"
	hash "0xC3981DCE61D9E13F"
	jhash (0xE9BF2A7D)
	arguments {
		charPtr "camName",

		BOOL "p1",
	}
	returns	"Cam"

--[[!
<summary>
	camName is always set to "DEFAULT_SCRIPTED_CAMERA" in Rockstar's scripts
</summary>
]]--
native "CREATE_CAM_WITH_PARAMS"
	hash "0xB51194800B257161"
	jhash (0x23B02F15)
	arguments {
		charPtr "camName",

		float "posX",

		float "posY",

		float "posZ",

		float "rotX",

		float "rotY",

		float "rotZ",

		float "fov",

		BOOL "p8",

		int "p9",
	}
	returns	"Cam"

native "CREATE_CAMERA"
	hash "0x5E3CF89C6BCCA67D"
	jhash (0x5D6739AE)
	arguments {
		charPtr "camName",

		BOOL "p1",
	}
	returns	"Cam"

native "CREATE_CAMERA_WITH_PARAMS"
	hash "0x6ABFA3E16460F22D"
	jhash (0x0688BE9A)
	arguments {
		charPtr "camName",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		BOOL "p8",

		Any "p9",
	}
	returns	"Cam"

--[[!
<summary>
	Removes the cam.
</summary>
]]--
native "DESTROY_CAM"
	hash "0x865908C81A2C22E9"
	jhash (0xC39302BD)
	arguments {
		Cam "cam",

		BOOL "destroy",
	}
	returns	"void"

native "DESTROY_ALL_CAMS"
	hash "0x8E5FB15663F79120"
	jhash (0x10C151CE)
	arguments {
		BOOL "destroy",
	}
	returns	"void"

--[[!
<summary>
	Returns whether or not the passed camera handle exists.
</summary>
]]--
native "DOES_CAM_EXIST"
	hash "0xA7A932170592B50E"
	jhash (0x1EF89DC0)
	arguments {
		Cam "cam",
	}
	returns	"BOOL"

--[[!
<summary>
	Set camera as active/inactive.
</summary>
]]--
native "SET_CAM_ACTIVE"
	hash "0x026FB97D0A425F84"
	jhash (0x064659C2)
	arguments {
		Any "cam",

		BOOL "active",
	}
	returns	"void"

--[[!
<summary>
	Returns whether or not the passed camera handle is active.
</summary>
]]--
native "IS_CAM_ACTIVE"
	hash "0xDFB2B516207D3534"
	jhash (0x4B58F177)
	arguments {
		Any "cam",
	}
	returns	"BOOL"

native "IS_CAM_RENDERING"
	hash "0x02EC0AF5C5A49B7A"
	jhash (0x6EC6B5B2)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "GET_RENDERING_CAM"
	hash "0x5234F9F10919EABA"
	jhash (0x0FCF4DF1)
	returns	"Any"

native "GET_CAM_COORD"
	hash "0xBAC038F7459AE5AE"
	jhash (0x7C40F09C)
	arguments {
		Any "cam",
	}
	returns	"Vector3"

--[[!
<summary>
	The last parameter, as in other "ROT" methods, is usually 2.
</summary>
]]--
native "GET_CAM_ROT"
	hash "0x7D304C1C955E3E12"
	jhash (0xDAC84C9F)
	arguments {
		Any "cam",

		Any "p1",
	}
	returns	"Vector3"

native "GET_CAM_FOV"
	hash "0xC3330A45CCCDB26A"
	jhash (0xD6E9FCF5)
	arguments {
		Any "cam",
	}
	returns	"float"

native "GET_CAM_NEAR_CLIP"
	hash "0xC520A34DAFBF24B1"
	jhash (0xCFCD35EE)
	arguments {
		Any "cam",
	}
	returns	"float"

native "GET_CAM_FAR_CLIP"
	hash "0xB60A9CFEB21CA6AA"
	jhash (0x09F119B8)
	arguments {
		Any "cam",
	}
	returns	"float"

native "GET_CAM_FAR_DOF"
	hash "0x255F8DAFD540D397"
	jhash (0x98C5CCE9)
	arguments {
		Any "cam",
	}
	returns	"float"

native "SET_CAM_PARAMS"
	hash "0xBFD8727AEA3CCEBA"
	jhash (0x2167CEBF)
	arguments {
		Any "cam",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		Any "p8",

		Any "p9",

		Any "p10",

		Any "p11",
	}
	returns	"void"

--[[!
<summary>
	Sets the position of the cam.
</summary>
]]--
native "SET_CAM_COORD"
	hash "0x4D41783FB745E42E"
	jhash (0x7A8053AF)
	arguments {
		Any "cam",

		float "posX",

		float "posY",

		float "posZ",
	}
	returns	"void"

--[[!
<summary>
	Sets the rotation of the cam.
	Last parameter unknown.

	Last parameter seems to always be set to 2.
</summary>
]]--
native "SET_CAM_ROT"
	hash "0x85973643155D0B07"
	jhash (0xEE38B3C1)
	arguments {
		Cam "cam",

		float "rotX",

		float "rotY",

		float "rotZ",

		int "p4",
	}
	returns	"void"

--[[!
<summary>
	Sets the field of view of the cam.
</summary>
]]--
native "SET_CAM_FOV"
	hash "0xB13C14F66A00D047"
	jhash (0xD3D5D74F)
	arguments {
		Any "cam",

		float "fieldOfView",
	}
	returns	"void"

native "SET_CAM_NEAR_CLIP"
	hash "0xC7848EFCCC545182"
	jhash (0x46DB13B1)
	arguments {
		Any "cam",

		float "nearClip",
	}
	returns	"void"

native "SET_CAM_FAR_CLIP"
	hash "0xAE306F2A904BF86E"
	jhash (0x0D23E381)
	arguments {
		Any "cam",

		float "farClip",
	}
	returns	"void"

native "SET_CAM_MOTION_BLUR_STRENGTH"
	hash "0x6F0F77FBA9A8F2E6"
	jhash (0xFD6E0D67)
	arguments {
		Any "cam",

		float "blur",
	}
	returns	"void"

native "SET_CAM_NEAR_DOF"
	hash "0x3FA4BF0A7AB7DE2C"
	jhash (0xF28254DF)
	arguments {
		Any "cam",

		float "nearDOF",
	}
	returns	"void"

native "SET_CAM_FAR_DOF"
	hash "0xEDD91296CD01AEE0"
	jhash (0x58515E8E)
	arguments {
		Any "cam",

		float "farDOF",
	}
	returns	"void"

native "SET_CAM_DOF_STRENGTH"
	hash "0x5EE29B4D7D5DF897"
	jhash (0x3CC4EB3F)
	arguments {
		Any "cam",

		float "dofStrength",
	}
	returns	"void"

native "SET_CAM_DOF_PLANES"
	hash "0x3CF48F6F96E749DC"
	jhash (0xAD6C2B8F)
	arguments {
		Any "cam",

		float "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"void"

native "SET_CAM_USE_SHALLOW_DOF_MODE"
	hash "0x16A96863A17552BB"
	jhash (0x8306C256)
	arguments {
		Cam "cam",

		BOOL "toggle",
	}
	returns	"void"

native "SET_USE_HI_DOF"
	hash "0xA13B0222F3D94A94"
	jhash (0x8BBF2950)
	returns	"void"

native "0xF55E4046F6F831DC"
	hash "0xF55E4046F6F831DC"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0xE111A7C0D200CBC5"
	hash "0xE111A7C0D200CBC5"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x7DD234D6F3914C5B"
	hash "0x7DD234D6F3914C5B"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0xC669EEA5D031B7DE"
	hash "0xC669EEA5D031B7DE"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0xC3654A441402562D"
	hash "0xC3654A441402562D"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x2C654B4943BDDF7C"
	hash "0x2C654B4943BDDF7C"
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "ATTACH_CAM_TO_ENTITY"
	hash "0xFEDB7D269E8C60E3"
	jhash (0xAD7C45F6)
	arguments {
		Any "cam",

		Entity "entity",

		float "xOffset",

		float "yOffset",

		float "zOffset",

		BOOL "p5",
	}
	returns	"void"

native "ATTACH_CAM_TO_PED_BONE"
	hash "0x61A3DBA14AB7F411"
	jhash (0x506BB35C)
	arguments {
		int "cam",

		Ped "ped",

		int "boneIndex",

		float "x",

		float "y",

		float "z",

		BOOL "heading",
	}
	returns	"void"

native "DETACH_CAM"
	hash "0xA2FABBE87F4BAD82"
	jhash (0xF4FBF14A)
	arguments {
		int "camHandle",
	}
	returns	"Any"

--[[!
<summary>
	p1 is unknown, but the native seems to only be called once.

	The native is used as so,
	CAM::SET_CAM_INHERIT_ROLL_VEHICLE(l_544, getElem(2, &amp;l_525, 4));
	In the exile1 script.
</summary>
]]--
native "SET_CAM_INHERIT_ROLL_VEHICLE"
	hash "0x45F1DE9C34B93AE6"
	jhash (0xE4BD5342)
	arguments {
		int "cam",

		Any "p1",
	}
	returns	"void"

native "POINT_CAM_AT_COORD"
	hash "0xF75497BB865F0803"
	jhash (0x914BC21A)
	arguments {
		Any "cam",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

--[[!
<summary>
	p5 always seems to be 1 i.e TRUE
</summary>
]]--
native "POINT_CAM_AT_ENTITY"
	hash "0x5640BFF86B16E8DC"
	jhash (0x7597A0F7)
	arguments {
		Any "cam",

		Entity "entity",

		float "p2",

		float "p3",

		float "p4",

		BOOL "p5",
	}
	returns	"void"

--[[!
<summary>
	Parameters p0-p5 seems correct. The bool p6 is unknown, but through every X360 script it's always 1. Please correct p0-p5 if any prove to be wrong. 
</summary>
]]--
native "POINT_CAM_AT_PED_BONE"
	hash "0x68B2B5F33BA63C41"
	jhash (0x09F47049)
	arguments {
		int "cam",

		int "ped",

		int "boneIndex",

		float "x",

		float "y",

		float "z",

		BOOL "p6",
	}
	returns	"void"

native "STOP_CAM_POINTING"
	hash "0xF33AB75780BA57DE"
	jhash (0x5435F6A5)
	arguments {
		int "cam",
	}
	returns	"void"

native "SET_CAM_AFFECTS_AIMING"
	hash "0x8C1DC7770C51DC8D"
	jhash (0x0C74F9AF)
	arguments {
		Cam "cam",

		BOOL "toggle",
	}
	returns	"void"

native "0x661B5C8654ADD825"
	hash "0x661B5C8654ADD825"
	jhash (0xE1A0B2F1)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xA2767257A320FC82"
	hash "0xA2767257A320FC82"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x271017B9BA825366"
	hash "0x271017B9BA825366"
	jhash (0x43220969)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	NOTE: Debugging functions are not present in the retail version of the game.
</summary>
]]--
native "SET_CAM_DEBUG_NAME"
	hash "0x1B93E0107865DD40"
	jhash (0x9B00DF3F)
	arguments {
		Cam "camera",

		charPtr "name",
	}
	returns	"void"

--[[!
<summary>
	I filled p1-p6 (the floats) as they are as other natives with 6 floats in a row are similar and I see no other method. So if a test from anyone proves them wrong please correct.
</summary>
]]--
native "ADD_CAM_SPLINE_NODE"
	hash "0x8609C75EC438FB3B"
	jhash (0xAD3C7EAA)
	arguments {
		int "camera",

		float "x",

		float "y",

		float "z",

		float "xRot",

		float "yRot",

		float "zRot",

		int "p7",

		int "p8",

		int "p9",
	}
	returns	"void"

native "0x0A9F2A468B328E74"
	hash "0x0A9F2A468B328E74"
	jhash (0x30510511)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x0FB82563989CF4FB"
	hash "0x0FB82563989CF4FB"
	jhash (0xBA6C085B)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x609278246A29CA34"
	hash "0x609278246A29CA34"
	jhash (0xB4737F03)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "SET_CAM_SPLINE_PHASE"
	hash "0x242B5874F0A4E052"
	jhash (0xF0AED233)
	arguments {
		int "cam",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	Can use this with SET_CAM_SPLINE_PHASE to set the float it this native returns.
</summary>
]]--
native "GET_CAM_SPLINE_PHASE"
	hash "0xB5349E36C546509A"
	jhash (0x39784DD9)
	arguments {
		int "cam",
	}
	returns	"float"

--[[!
<summary>
	I'm pretty sure the parameter is the camera as usual, but I am not certain so I'm going to leave it as is.
</summary>
]]--
native "GET_CAM_SPLINE_NODE_PHASE"
	hash "0xD9D0E694C8282C96"
	jhash (0x7B9522F6)
	arguments {
		Any "p0",
	}
	returns	"float"

--[[!
<summary>
	I named p1 as timeDuration as it is obvious. I'm assuming tho it is ran in ms(Milliseconds) as usual.
</summary>
]]--
native "SET_CAM_SPLINE_DURATION"
	hash "0x1381539FEE034CDA"
	jhash (0x3E91FC8A)
	arguments {
		int "cam",

		int "timeDuration",
	}
	returns	"void"

native "0xD1B0F412F109EA5D"
	hash "0xD1B0F412F109EA5D"
	jhash (0x15E141CE)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	I named the beginning from Any to BOOL as this native is used in an if statement as well. 
</summary>
]]--
native "GET_CAM_SPLINE_NODE_INDEX"
	hash "0xB22B17DF858716A6"
	jhash (0xF8AEB6BD)
	arguments {
		int "cam",
	}
	returns	"BOOL"

native "0x83B8201ED82A9A2D"
	hash "0x83B8201ED82A9A2D"
	jhash (0x21D275DA)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		float "p3",
	}
	returns	"void"

native "0xA6385DEB180F319F"
	hash "0xA6385DEB180F319F"
	jhash (0xA3BD9E94)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",
	}
	returns	"void"

native "OVERRIDE_CAM_SPLINE_VELOCITY"
	hash "0x40B62FA033EB0346"
	jhash (0x326A17E2)
	arguments {
		int "cam",

		int "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "OVERRIDE_CAM_SPLINE_MOTION_BLUR"
	hash "0x7DCF7C708D292D55"
	jhash (0x633179E6)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "0x7BF1A54AE67AC070"
	hash "0x7BF1A54AE67AC070"
	jhash (0xC90B2DDC)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "IS_CAM_SPLINE_PAUSED"
	hash "0x0290F35C0AD97864"
	jhash (0x60B34FF5)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SET_CAM_ACTIVE_WITH_INTERP"
	hash "0x9FBDA379383A52A4"
	jhash (0x7983E7F0)
	arguments {
		int "camTo",

		int "camFrom",

		int "duration",

		BOOL "easeLocation",

		BOOL "easeRotation",
	}
	returns	"Any"

native "IS_CAM_INTERPOLATING"
	hash "0x036F97C908C2B52C"
	jhash (0x7159CB5D)
	arguments {
		int "cam",
	}
	returns	"BOOL"

--[[!
<summary>
	Possible shake types:

	LARGE_EXPLOSION_SHAKE
	JOLT_SHAKE
	SMALL_EXPLOSION_SHAKE
	MEDIUM_EXPLOSION_SHAKE
	SKY_DIVING_SHAKE
	DRUNK_SHAKE
	"VIBRATE_SHAKE"
	"HAND_SHAKE"
</summary>
]]--
native "SHAKE_CAM"
	hash "0x6A25241C340D3822"
	jhash (0x1D4211B0)
	arguments {
		int "cam",

		charPtr "type",

		float "amplitude",
	}
	returns	"void"

--[[!
<summary>
	Example from michael2 script.

	CAM::ANIMATED_SHAKE_CAM(l_5069, "shake_cam_all@", "light", "", 1f);
</summary>
]]--
native "ANIMATED_SHAKE_CAM"
	hash "0xA2746EEAE3E577CD"
	jhash (0xE1168767)
	arguments {
		int "cam",

		charPtr "p1",

		charPtr "p2",

		charPtr "p3",

		float "amplitude",
	}
	returns	"void"

native "IS_CAM_SHAKING"
	hash "0x6B24BFE83A2BE47B"
	jhash (0x0961FD9B)
	arguments {
		int "cam",
	}
	returns	"BOOL"

native "SET_CAM_SHAKE_AMPLITUDE"
	hash "0xD93DB43B82BC0D00"
	jhash (0x60FF6382)
	arguments {
		int "cam",

		float "amplitude",
	}
	returns	"void"

native "STOP_CAM_SHAKING"
	hash "0xBDECF64367884AC3"
	jhash (0x40D0EB87)
	arguments {
		int "cam",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Something to do with shake:
	CAM::_F4C8CF9E353AFECA("HAND_SHAKE", 0.2);
</summary>
]]--
native "0xF4C8CF9E353AFECA"
	hash "0xF4C8CF9E353AFECA"
	jhash (0x2B0F05CD)
	arguments {
		charPtr "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	CAM::_C2EAE3FB8CDBED31("SHAKE_CAM_medium", "medium", "", 0.5f);
</summary>
]]--
native "0xC2EAE3FB8CDBED31"
	hash "0xC2EAE3FB8CDBED31"
	jhash (0xCB75BD9C)
	arguments {
		charPtr "p0",

		charPtr "p1",

		charPtr "p2",

		float "p3",
	}
	returns	"void"

native "0xC912AF078AF19212"
	hash "0xC912AF078AF19212"
	jhash (0x6AEFE6A5)
	returns	"Any"

native "0x1C9D7949FA533490"
	hash "0x1C9D7949FA533490"
	jhash (0x26FCFB96)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Atleast one time in a script for the zRot Rockstar uses GET_ENTIT_HEADING to help fill the parameter.

	p9 is unknown at this time.
	p10 throughout all the X360 Scripts is always 2.
</summary>
]]--
native "PLAY_CAM_ANIM"
	hash "0x9A2D0FB2E7852392"
	jhash (0xBCEFB87E)
	arguments {
		int "cam",

		charPtr "animName",

		charPtr "animDictionary",

		float "x",

		float "y",

		float "z",

		float "xRot",

		float "yRot",

		float "zRot",

		BOOL "p9",

		int "p10",
	}
	returns	"BOOL"

native "IS_CAM_PLAYING_ANIM"
	hash "0xC90621D8A0CEECF2"
	jhash (0xB998CB49)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "SET_CAM_ANIM_CURRENT_PHASE"
	hash "0x4145A4C44FF3B5A6"
	jhash (0x3CB1D17F)
	arguments {
		Cam "cam",

		float "phase",
	}
	returns	"void"

native "GET_CAM_ANIM_CURRENT_PHASE"
	hash "0xA10B2DB49E92A6B0"
	jhash (0x345F72D0)
	arguments {
		Cam "cam",
	}
	returns	"float"

native "PLAY_SYNCHRONIZED_CAM_ANIM"
	hash "0xE32EFE9AB4A9AA0C"
	jhash (0x9458459E)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"BOOL"

native "0x503F5920162365B2"
	hash "0x503F5920162365B2"
	jhash (0x56F9ED27)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "0xF9D02130ECDD1D77"
	hash "0xF9D02130ECDD1D77"
	jhash (0x71570DBA)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0xC91C6C55199308CA"
	hash "0xC91C6C55199308CA"
	jhash (0x60B345DE)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "0xC8B5C4A79CC18B94"
	hash "0xC8B5C4A79CC18B94"
	jhash (0x44473EFC)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x5C48A1D6E3B33179"
	hash "0x5C48A1D6E3B33179"
	jhash (0xDA931D65)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "IS_SCREEN_FADED_OUT"
	hash "0xB16FCE9DDC7BA182"
	jhash (0x9CAA05FA)
	returns	"BOOL"

native "IS_SCREEN_FADED_IN"
	hash "0x5A859503B0C08678"
	jhash (0x4F37276D)
	returns	"BOOL"

native "IS_SCREEN_FADING_OUT"
	hash "0x797AC7CB535BA28F"
	jhash (0x79275A57)
	returns	"BOOL"

native "IS_SCREEN_FADING_IN"
	hash "0x5C544BC6C57AC575"
	jhash (0xC7C82800)
	returns	"BOOL"

--[[!
<summary>
	Fades the screen in.

	duration: The time the fade should take, in milliseconds.
</summary>
]]--
native "DO_SCREEN_FADE_IN"
	hash "0xD4E8E24955024033"
	jhash (0x66C1BDEE)
	arguments {
		int "duration",
	}
	returns	"void"

--[[!
<summary>
	Fades the screen out.

	duration: The time the fade should take, in milliseconds.
</summary>
]]--
native "DO_SCREEN_FADE_OUT"
	hash "0x891B5B39AC6302AF"
	jhash (0x89D01805)
	arguments {
		int "duration",
	}
	returns	"void"

native "SET_WIDESCREEN_BORDERS"
	hash "0xDCD4EA924F42D01A"
	jhash (0x1A75DC9A)
	arguments {
		BOOL "p0",

		int "p1",
	}
	returns	"Any"

native "GET_GAMEPLAY_CAM_COORD"
	hash "0x14D6F5678D8F1B37"
	jhash (0x9388CF79)
	returns	"Vector3"

native "GET_GAMEPLAY_CAM_ROT"
	hash "0x837765A25378F0BB"
	jhash (0x13A010B5)
	arguments {
		Any "p0",
	}
	returns	"int"

native "GET_GAMEPLAY_CAM_FOV"
	hash "0x65019750A0324133"
	jhash (0x4D6B3BFA)
	returns	"float"

--[[!
<summary>
	some camera effect that is used in the drunk-cheat, and turned off (by setting it to 0.0) along with the shaking effects once the drunk cheat is disabled.
</summary>
]]--
native "0x487A82C650EB7799"
	hash "0x487A82C650EB7799"
	jhash (0xA6E73135)
	arguments {
		float "p0",
	}
	returns	"void"

--[[!
<summary>
	some camera effect that is (also) used in the drunk-cheat, and turned off (by setting it to 0.0) along with the shaking effects once the drunk cheat is disabled. Possibly a cinematic or script-cam version of _0x487A82C650EB7799
</summary>
]]--
native "0x0225778816FDC28C"
	hash "0x0225778816FDC28C"
	jhash (0x1126E37C)
	arguments {
		float "p0",
	}
	returns	"void"

native "GET_GAMEPLAY_CAM_RELATIVE_HEADING"
	hash "0x743607648ADD4587"
	jhash (0xCAF839C2)
	returns	"float"

--[[!
<summary>
	Sets the camera position relative to heading in float from -360 to +360.
</summary>
]]--
native "SET_GAMEPLAY_CAM_RELATIVE_HEADING"
	hash "0xB4EC2312F4E5B1F1"
	jhash (0x20C6217C)
	arguments {
		float "heading",
	}
	returns	"Any"

native "GET_GAMEPLAY_CAM_RELATIVE_PITCH"
	hash "0x3A6867B4845BEDA2"
	jhash (0xFC5A4946)
	returns	"float"

--[[!
<summary>
	Sets the camera pitch.

	Parameters:
	x = pitches the camera on the x axis.
	Value2 = always seems to be hex 0x3F800000 (1.000000 float).
</summary>
]]--
native "SET_GAMEPLAY_CAM_RELATIVE_PITCH"
	hash "0x6D0858B8EDFD2B7D"
	jhash (0x6381B963)
	arguments {
		float "x",

		float "Value2",
	}
	returns	"Any"

native "_SET_GAMEPLAY_CAM_RAW_YAW"
	hash "0x103991D4A307D472"
	arguments {
		float "yaw",
	}
	returns	"void"

native "_SET_GAMEPLAY_CAM_RAW_PITCH"
	hash "0x759E13EBC1C15C5A"
	arguments {
		float "pitch",
	}
	returns	"void"

native "0x469F2ECDEC046337"
	hash "0x469F2ECDEC046337"
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Possible shake types:

	LARGE_EXPLOSION_SHAKE
	JOLT_SHAKE
	SMALL_EXPLOSION_SHAKE
	MEDIUM_EXPLOSION_SHAKE
	SKY_DIVING_SHAKE
	DRUNK_SHAKE
</summary>
]]--
native "SHAKE_GAMEPLAY_CAM"
	hash "0xFD55E49555E017CF"
	jhash (0xF2EFE660)
	arguments {
		charPtr "shakeName",

		float "intensity",
	}
	returns	"void"

native "IS_GAMEPLAY_CAM_SHAKING"
	hash "0x016C090630DF1F89"
	jhash (0x3457D681)
	returns	"BOOL"

--[[!
<summary>
	Sets the amplitude for the gameplay (i.e. 3rd or 1st) camera to shake. Used in script "drunk_controller.ysc.c4" to simulate making the player drunk.


</summary>
]]--
native "SET_GAMEPLAY_CAM_SHAKE_AMPLITUDE"
	hash "0xA87E00932DB4D85D"
	jhash (0x9219D44A)
	arguments {
		float "amplitude",
	}
	returns	"void"

native "STOP_GAMEPLAY_CAM_SHAKING"
	hash "0x0EF93E9F3D08C178"
	jhash (0xFD569E4E)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x8BBACBF51DA047A8"
	hash "0x8BBACBF51DA047A8"
	jhash (0x7D3007A2)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>


	Examples when this function will return 0 are:
	- During busted screen.
	- When player is coming out from a hospital.
	- When player is coming out from a police station.
</summary>
]]--
native "IS_GAMEPLAY_CAM_RENDERING"
	hash "0x39B5D1B10383F0C8"
	jhash (0x0EF276DA)
	returns	"BOOL"

native "0x3044240D2E0FA842"
	hash "0x3044240D2E0FA842"
	jhash (0xC0B00C20)
	returns	"BOOL"

native "0x705A276EBFF3133D"
	hash "0x705A276EBFF3133D"
	jhash (0x60C23785)
	returns	"BOOL"

native "0xDB90C6CCA48940F1"
	hash "0xDB90C6CCA48940F1"
	jhash (0x20BFF6E5)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Shows the crosshair even if it wouldn't show normally. Only works for one frame, so make sure to call it repeatedly.
</summary>
]]--
native "_ENABLE_CROSSHAIR_THIS_FRAME"
	hash "0xEA7F0AD7E9BA676F"
	jhash (0xA61FF9AC)
	returns	"void"

native "IS_GAMEPLAY_CAM_LOOKING_BEHIND"
	hash "0x70FDA869F3317EA9"
	jhash (0x33C83F17)
	returns	"BOOL"

native "0x2AED6301F67007D5"
	hash "0x2AED6301F67007D5"
	jhash (0x2701A9AD)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x49482F9FCD825AAA"
	hash "0x49482F9FCD825AAA"
	jhash (0xC4736ED3)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xFD3151CD37EA2245"
	hash "0xFD3151CD37EA2245"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xDD79DF9F4D26E1C9"
	hash "0xDD79DF9F4D26E1C9"
	jhash (0x6B0E9D57)
	returns	"void"

native "IS_SPHERE_VISIBLE"
	hash "0xE33D59DA70B58FDF"
	jhash (0xDD1329E2)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",
	}
	returns	"BOOL"

native "IS_FOLLOW_PED_CAM_ACTIVE"
	hash "0xC6D3D26810C8E0F9"
	jhash (0x9F9E856C)
	returns	"BOOL"

native "SET_FOLLOW_PED_CAM_CUTSCENE_CHAT"
	hash "0x44A113DD6FFC48D1"
	jhash (0x1425F6AC)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x271401846BD26E92"
	hash "0x271401846BD26E92"
	jhash (0x8DC53629)
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xC8391C309684595A"
	hash "0xC8391C309684595A"
	jhash (0x1F9DE6E4)
	returns	"void"

--[[!
<summary>
	minimum: Degrees between -180f and 180f.
	maximum: Degrees between -180f and 180f.

	Clamps the gameplay camera's current yaw.

	Eg. _CLAMP_GAMEPLAY_CAM_YAW(0.0f, 0.0f) will set the horizontal angle directly behind the player.
</summary>
]]--
native "_CLAMP_GAMEPLAY_CAM_YAW"
	hash "0x8F993D26E0CA5E8E"
	jhash (0x749909AC)
	arguments {
		float "minimum",

		float "maximum",
	}
	returns	"Any"

--[[!
<summary>
	minimum: Degrees between -90f and 90f.
	maximum: Degrees between -90f and 90f.

	Clamps the gameplay camera's current pitch.

	Eg. _CLAMP_GAMEPLAY_CAM_PITCH(0.0f, 0.0f) will set the vertical angle directly behind the player.
</summary>
]]--
native "_CLAMP_GAMEPLAY_CAM_PITCH"
	hash "0xA516C198B7DCA1E1"
	jhash (0xFA3A16E7)
	arguments {
		float "minimum",

		float "maximum",
	}
	returns	"Any"

--[[!
<summary>
	Seems to animate the gameplay camera zoom.

	Eg. _ANIMATE_GAMEPLAY_CAM_ZOOM(1f, 1000f);
	will animate the camera zooming in from 1000 meters away.

	Game scripts use it like this:

	// Setting this to 1 prevents V key from changing zoom
	PLAYER::SET_PLAYER_FORCED_ZOOM(PLAYER::PLAYER_ID(), 1);

	// These restrict how far you can move cam up/down left/right
	CAM::_CLAMP_GAMEPLAY_CAM_YAW(-20f, 50f);
	CAM::_CLAMP_GAMEPLAY_CAM_PITCH(-60f, 0f);

	CAM::_ANIMATE_GAMEPLAY_CAM_ZOOM(1f, 1f);
</summary>
]]--
native "_ANIMATE_GAMEPLAY_CAM_ZOOM"
	hash "0xDF2E1F7742402E81"
	jhash (0x77340650)
	arguments {
		float "p0",

		float "_distance",
	}
	returns	"void"

native "0xE9EA16D6E54CDCA4"
	hash "0xE9EA16D6E54CDCA4"
	jhash (0x4B22C5CB)
	arguments {
		Vehicle "p0",

		int "p1",
	}
	returns	"Any"

--[[!
<summary>
	Disables first person camera for the current frame.

	Found in decompiled scripts:
	GRAPHICS::DRAW_DEBUG_TEXT_2D("Disabling First Person Cam", 0.5, 0.8, 0.0, 0, 0, 255, 255);
	CAM::_DE2EF5DA284CC8DF();
</summary>
]]--
native "_DISABLE_FIRST_PERSON_CAM_THIS_FRAME"
	hash "0xDE2EF5DA284CC8DF"
	returns	"void"

native "0x59424BD75174C9B1"
	hash "0x59424BD75174C9B1"
	returns	"void"

native "GET_FOLLOW_PED_CAM_ZOOM_LEVEL"
	hash "0x33E6C8EFD0CD93E9"
	jhash (0x57583DF1)
	returns	"Any"

--[[!
<summary>
	Returns
	0 - Third Person Close
	1 - Third Person Mid
	2 - Third Person Far
	4 - First Person
</summary>
]]--
native "GET_FOLLOW_PED_CAM_VIEW_MODE"
	hash "0x8D4D46230B2C353A"
	jhash (0xA65FF946)
	returns	"int"

--[[!
<summary>
	Sets the type of Player camera:

	0 - Third Person Close
	1 - Third Person Mid
	2 - Third Person Far
	4 - First Person
</summary>
]]--
native "SET_FOLLOW_PED_CAM_VIEW_MODE"
	hash "0x5A4F9EDF1673F704"
	jhash (0x495DBE8D)
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_FOLLOW_VEHICLE_CAM_ACTIVE"
	hash "0xCBBDE6D335D6D496"
	jhash (0x8DD49B77)
	returns	"BOOL"

native "0x91EF6EE6419E5B97"
	hash "0x91EF6EE6419E5B97"
	jhash (0x9DB5D391)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x9DFE13ECDC1EC196"
	hash "0x9DFE13ECDC1EC196"
	jhash (0x92302899)
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"void"

native "GET_FOLLOW_VEHICLE_CAM_ZOOM_LEVEL"
	hash "0xEE82280AB767B690"
	jhash (0x8CD67DE3)
	returns	"Any"

native "SET_FOLLOW_VEHICLE_CAM_ZOOM_LEVEL"
	hash "0x19464CB6E4078C8A"
	jhash (0x8F55EBBE)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Returns the type of camera:

	0 - Third Person Close
	1 - Third Person Mid
	2 - Third Person Far
	4 - First Person
</summary>
]]--
native "GET_FOLLOW_VEHICLE_CAM_VIEW_MODE"
	hash "0xA4FF579AC0E3AAAE"
	jhash (0xA4B4DB03)
	returns	"Any"

--[[!
<summary>
	Sets the type of Player camera in vehicles:

	0 - Third Person Close
	1 - Third Person Mid
	2 - Third Person Far
	4 - First Person
</summary>
]]--
native "SET_FOLLOW_VEHICLE_CAM_VIEW_MODE"
	hash "0xAC253D7842768F48"
	jhash (0xC4FBBBD3)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	interprets the result of CAM::_0x19CAFA3C87F7C2FF()

	example: // checks if you're currently in first person
	if ((CAM::_EE778F8C7E1142E2(CAM::_19CAFA3C87F7C2FF()) == 4) &amp;&amp; (!__463_$28ED382849B17AFC())) {
	UI::_FDEC055AB549E328();
	UI::_SET_NOTIFICATION_TEXT_ENTRY("REC_FEED_WAR");
	l_CE[0/*1*/] = UI::_DRAW_NOTIFICATION(0, 1);
	}



</summary>
]]--
native "0xEE778F8C7E1142E2"
	hash "0xEE778F8C7E1142E2"
	jhash (0xF3B148A6)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x2A2173E46DAECD12"
	hash "0x2A2173E46DAECD12"
	jhash (0x1DEBCB45)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	Seems to return the current type of view
	example: // checks if you're currently in first person
	if ((CAM::_EE778F8C7E1142E2(CAM::_19CAFA3C87F7C2FF()) == 4) &amp;&amp; (!__463_$28ED382849B17AFC())) {
	    UI::_FDEC055AB549E328();
	    UI::_SET_NOTIFICATION_TEXT_ENTRY("REC_FEED_WAR");
	    l_CE[0/*1*/] = UI::_DRAW_NOTIFICATION(0, 1);
	}


</summary>
]]--
native "0x19CAFA3C87F7C2FF"
	hash "0x19CAFA3C87F7C2FF"
	returns	"Any"

native "IS_AIM_CAM_ACTIVE"
	hash "0x68EDDA28A5976D07"
	jhash (0xC24B4F6F)
	returns	"BOOL"

native "0x74BD83EA840F6BC9"
	hash "0x74BD83EA840F6BC9"
	jhash (0x8F320DE4)
	returns	"Any"

native "IS_FIRST_PERSON_AIM_CAM_ACTIVE"
	hash "0x5E346D934122613F"
	jhash (0xD6280468)
	returns	"BOOL"

native "0x1A31FE0049E542F6"
	hash "0x1A31FE0049E542F6"
	jhash (0x1BAA7182)
	returns	"void"

--[[!
<summary>
	GET_GAMEPLAY_CAM_ZOOM()   //gtaVmod
</summary>
]]--
native "_GET_GAMEPLAY_CAM_ZOOM"
	hash "0x7EC52CC40597D170"
	jhash (0x33951005)
	returns	"float"

native "0x70894BD0915C5BCA"
	hash "0x70894BD0915C5BCA"
	jhash (0x9F4AF763)
	arguments {
		float "p0",
	}
	returns	"Any"

native "0xCED08CBE8EBB97C7"
	hash "0xCED08CBE8EBB97C7"
	jhash (0x68BA0730)
	arguments {
		float "p0",

		float "p1",
	}
	returns	"void"

native "0x2F7F2B26DD3F18EE"
	hash "0x2F7F2B26DD3F18EE"
	jhash (0x2F29F0D5)
	arguments {
		float "p0",

		float "p1",
	}
	returns	"void"

native "0xBCFC632DB7673BF0"
	hash "0xBCFC632DB7673BF0"
	jhash (0x76DAC96C)
	arguments {
		float "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	Appear to have something to do with the clipping at close range.
</summary>
]]--
native "0x0AF7B437918103B3"
	hash "0x0AF7B437918103B3"
	jhash (0x0E21069D)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x42156508606DE65E"
	hash "0x42156508606DE65E"
	jhash (0x71E9C63E)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x4008EDF7D6E48175"
	hash "0x4008EDF7D6E48175"
	jhash (0xD1EEBC45)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "_GET_GAMEPLAY_CAM_COORDS"
	hash "0xA200EB1EE790F448"
	jhash (0x9C84BDA0)
	returns	"Vector3"

--[[!
<summary>
	p0 seems to consistently be 2 across scripts

	Function is called faily often by CAM::CREATE_CAM_WITH_PARAMS
</summary>
]]--
native "_GET_GAMEPLAY_CAM_ROT"
	hash "0x5B4E4C817FCC2DFB"
	jhash (0x1FFBEFC5)
	arguments {
		Any "p0",
	}
	returns	"Vector3"

native "0x26903D9CD1175F2C"
	hash "0x26903D9CD1175F2C"
	jhash (0xACADF916)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"int"

--[[!
<summary>
	Believe something to do with a Scripted Cam's FOV
</summary>
]]--
native "0x80EC114669DAEFF4"
	hash "0x80EC114669DAEFF4"
	jhash (0x721B763B)
	returns	"Any"

native "0x5F35F6732C3FBBA0"
	hash "0x5F35F6732C3FBBA0"
	jhash (0x23E3F106)
	arguments {
		Any "p0",
	}
	returns	"float"

native "0xD0082607100D7193"
	hash "0xD0082607100D7193"
	jhash (0x457AE195)
	returns	"Any"

native "0xDFC8CBC606FDB0FC"
	hash "0xDFC8CBC606FDB0FC"
	jhash (0x46CB3A49)
	returns	"Any"

native "0xA03502FC581F7D9B"
	hash "0xA03502FC581F7D9B"
	jhash (0x19297A7A)
	returns	"Any"

native "0x9780F32BCAF72431"
	hash "0x9780F32BCAF72431"
	jhash (0xF24777CA)
	returns	"Any"

native "0x162F9D995753DC19"
	hash "0x162F9D995753DC19"
	jhash (0x38992E83)
	returns	"Any"

native "SET_GAMEPLAY_COORD_HINT"
	hash "0xD51ADCD2D8BC0FB3"
	jhash (0xF27483C9)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"void"

native "SET_GAMEPLAY_PED_HINT"
	hash "0x2B486269ACD548D3"
	jhash (0x7C27343E)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",

		Any "p5",

		Any "p6",

		Any "p7",
	}
	returns	"void"

native "SET_GAMEPLAY_VEHICLE_HINT"
	hash "0xA2297E18F3E71C2E"
	jhash (0x2C9A11D8)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",

		Any "p5",

		Any "p6",

		Any "p7",
	}
	returns	"void"

native "SET_GAMEPLAY_OBJECT_HINT"
	hash "0x83E87508A2CA2AC6"
	jhash (0x2ED5E2F8)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",

		Any "p5",

		Any "p6",

		Any "p7",
	}
	returns	"void"

native "SET_GAMEPLAY_ENTITY_HINT"
	hash "0x189E955A8313E298"
	jhash (0x66C32306)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",
	}
	returns	"void"

native "IS_GAMEPLAY_HINT_ACTIVE"
	hash "0xE520FF1AD2785B40"
	jhash (0xAD8DA205)
	returns	"BOOL"

native "STOP_GAMEPLAY_HINT"
	hash "0xF46C581C61718916"
	jhash (0x1BC28B7B)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xCCD078C2665D2973"
	hash "0xCCD078C2665D2973"
	jhash (0xCAFEE798)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x247ACBC4ABBC9D1C"
	hash "0x247ACBC4ABBC9D1C"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xBF72910D0F26F025"
	hash "0xBF72910D0F26F025"
	returns	"Any"

native "SET_GAMEPLAY_HINT_FOV"
	hash "0x513403FB9C56211F"
	jhash (0x96FD173B)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xF8BDBF3D573049A1"
	hash "0xF8BDBF3D573049A1"
	jhash (0x72E8CD3A)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xD1F8363DFAD03848"
	hash "0xD1F8363DFAD03848"
	jhash (0x79472AE3)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x5D7B620DAE436138"
	hash "0x5D7B620DAE436138"
	jhash (0xFC7464A0)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xC92717EF615B6704"
	hash "0xC92717EF615B6704"
	jhash (0x3554AA0E)
	arguments {
		float "p0",
	}
	returns	"void"

native "GET_IS_MULTIPLAYER_BRIEF"
	hash "0xE3433EADAAF7EE40"
	jhash (0x2F0CE859)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_CINEMATIC_BUTTON_ACTIVE"
	hash "0x51669F7D1FB53D9F"
	jhash (0x3FBC5D00)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "IS_CINEMATIC_CAM_RENDERING"
	hash "0xB15162CB5826E9E8"
	jhash (0x80471AD9)
	returns	"BOOL"

native "SHAKE_CINEMATIC_CAM"
	hash "0xDCE214D9ED58F3CF"
	jhash (0x61815F31)
	arguments {
		AnyPtr "p0",

		float "p1",
	}
	returns	"void"

native "IS_CINEMATIC_CAM_SHAKING"
	hash "0xBBC08F6B4CB8FF0A"
	jhash (0x8376D939)
	returns	"BOOL"

native "SET_CINEMATIC_CAM_SHAKE_AMPLITUDE"
	hash "0xC724C701C30B2FE7"
	jhash (0x67510C4B)
	arguments {
		float "p0",
	}
	returns	"void"

native "STOP_CINEMATIC_CAM_SHAKING"
	hash "0x2238E588E588A6D7"
	jhash (0x71C12904)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "_DISABLE_VEHICLE_FIRST_PERSON_CAM_THIS_FRAME"
	hash "0xADFF1B2A555F5FBA"
	jhash (0x5AC6DAC9)
	returns	"void"

native "0x62ECFCFDEE7885D6"
	hash "0x62ECFCFDEE7885D6"
	jhash (0x837F8581)
	returns	"void"

native "0x9E4CFFF989258472"
	hash "0x9E4CFFF989258472"
	jhash (0x65DDE8AF)
	returns	"void"

native "0xF4F2C0D4EE209E20"
	hash "0xF4F2C0D4EE209E20"
	jhash (0xD75CDD75)
	returns	"void"

native "0xCA9D2AA3E326D720"
	hash "0xCA9D2AA3E326D720"
	jhash (0x96A07066)
	returns	"Any"

native "0x4F32C0D5A90A9B40"
	hash "0x4F32C0D5A90A9B40"
	returns	"Any"

native "CREATE_CINEMATIC_SHOT"
	hash "0x741B0129D4560F31"
	jhash (0xAC494E35)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "IS_CINEMATIC_SHOT_ACTIVE"
	hash "0xCC9F3371A7C28BC9"
	jhash (0xA4049042)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "STOP_CINEMATIC_SHOT"
	hash "0x7660C6E75D3A078E"
	jhash (0xD78358C5)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xA41BCD7213805AAC"
	hash "0xA41BCD7213805AAC"
	jhash (0xFBB85E02)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xDC9DA9E8789F5246"
	hash "0xDC9DA9E8789F5246"
	jhash (0x4938C82F)
	returns	"void"

--[[!
<summary>
	p0 = 0/1 or true/false
</summary>
]]--
native "SET_CINEMATIC_MODE_ACTIVE"
	hash "0xDCF0754AC3D6FD4E"
	jhash (0x2009E747)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x1F2300CB7FA7B7F6"
	hash "0x1F2300CB7FA7B7F6"
	jhash (0x6739AD55)
	returns	"Any"

native "0x17FCA7199A530203"
	hash "0x17FCA7199A530203"
	returns	"Any"

native "STOP_CUTSCENE_CAM_SHAKING"
	hash "0xDB629FFD9285FA06"
	jhash (0xF07D603D)
	returns	"void"

native "0x12DED8CA53D47EA5"
	hash "0x12DED8CA53D47EA5"
	jhash (0x067BA6F5)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x89215EC747DF244A"
	hash "0x89215EC747DF244A"
	jhash (0xFD99BE2B)
	arguments {
		float "p0",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		Any "p7",

		Any "p8",
	}
	returns	"Any"

native "0x5A43C76F7FC7BA5F"
	hash "0x5A43C76F7FC7BA5F"
	jhash (0xE206C450)
	returns	"void"

--[[!
<summary>
	if p0 is 0, effect is cancelled

	if p0 is 1, effect zooms in, gradually tilts cam clockwise apx 30 degrees, wobbles slowly. Motion blur is active until cancelled.

	if p0 is 2, effect immediately tilts cam clockwise apx 30 degrees, begins to wobble slowly, then gradually tilts cam back to normal. The wobbling will continue until the effect is cancelled.
</summary>
]]--
native "_SET_CAM_EFFECT"
	hash "0x80C8B1846639BB19"
	jhash (0xB06CCD38)
	arguments {
		int "p0",
	}
	returns	"void"

native "0x5C41E6BABC9E2112"
	hash "0x5C41E6BABC9E2112"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x21E253A7F8DA5DFB"
	hash "0x21E253A7F8DA5DFB"
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x11FA5D3479C7DD47"
	hash "0x11FA5D3479C7DD47"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xEAF0FA793D05C592"
	hash "0xEAF0FA793D05C592"
	returns	"Any"

native "0x8BFCEB5EA1B161B6"
	hash "0x8BFCEB5EA1B161B6"
	returns	"Any"

--[[!
<summary>
	Enables laser sight on any weapon.
</summary>
]]--
native "ENABLE_LASER_SIGHT_RENDERING"
	hash "0xC8B46D7727D864AA"
	jhash (0xE3438955)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "GET_WEAPON_COMPONENT_TYPE_MODEL"
	hash "0x0DB57B41EC1DB083"
	jhash (0x324FA47A)
	arguments {
		Hash "componentHash",
	}
	returns	"Hash"

--[[!
<summary>
	Returns the model of any weapon.

	Can also take an ammo hash?
	sub_6663a(&amp;l_115B, WEAPON::GET_WEAPONTYPE_MODEL(${ammo_rpg}));
</summary>
]]--
native "GET_WEAPONTYPE_MODEL"
	hash "0xF46CDC33180FDA94"
	jhash (0x44E1C269)
	arguments {
		Hash "weaponHash",
	}
	returns	"Any"

native "GET_WEAPONTYPE_SLOT"
	hash "0x4215460B9B8B7FA0"
	jhash (0x2E3759AF)
	arguments {
		Hash "weaponHash",
	}
	returns	"Any"

native "GET_WEAPONTYPE_GROUP"
	hash "0xC3287EE3050FB74C"
	jhash (0x5F2DE833)
	arguments {
		Hash "weaponHash",
	}
	returns	"Any"

native "SET_CURRENT_PED_WEAPON"
	hash "0xADF692B254977C0C"
	jhash (0xB8278882)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		BOOL "equipNow",
	}
	returns	"void"

--[[!
<summary>
	The return value seems to indicate if the weapon is ready to use.
	No idea what p2 does, it seems to be 1 most of the time.

</summary>
]]--
native "GET_CURRENT_PED_WEAPON"
	hash "0x3A87E44BB9A01D54"
	jhash (0xB0237302)
	arguments {
		Ped "ped",

		HashPtr "weaponHash",

		BOOL "p2",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns the ENTITY handle of the ped's current weapon.
</summary>
]]--
native "GET_CURRENT_PED_WEAPON_ENTITY_INDEX"
	hash "0x3B390A939AF0B5FC"
	jhash (0x5D73CD20)
	arguments {
		Ped "ped",
	}
	returns	"Entity"

--[[!
<summary>
	p1 is always 0 in the scripts.
</summary>
]]--
native "GET_BEST_PED_WEAPON"
	hash "0x8483E98E8B888AE2"
	jhash (0xB998D444)
	arguments {
		Ped "ped",

		BOOL "p1",
	}
	returns	"Hash"

native "SET_CURRENT_PED_VEHICLE_WEAPON"
	hash "0x75C55983C2C39DAA"
	jhash (0x8E6F2AF1)
	arguments {
		Ped "ped",

		Hash "weaponHash",
	}
	returns	"BOOL"

native "GET_CURRENT_PED_VEHICLE_WEAPON"
	hash "0x1017582BCD3832DC"
	jhash (0xF26C5D65)
	arguments {
		Ped "ped",

		HashPtr "weaponHash",
	}
	returns	"BOOL"

--[[!
<summary>
	p1 is anywhere from 4 to 7 in the scripts. Might be a weapon wheel group?
</summary>
]]--
native "IS_PED_ARMED"
	hash "0x475768A975D5AD17"
	jhash (0x0BFC892C)
	arguments {
		Ped "ped",

		int "p1",
	}
	returns	"BOOL"

native "IS_WEAPON_VALID"
	hash "0x937C71165CF334B3"
	jhash (0x38CA2954)
	arguments {
		Hash "weaponHash",
	}
	returns	"BOOL"

--[[!
<summary>
	p2 should be FALSE, otherwise it seems to always return FALSE

	bool could be that if that weapon is current weapon in hand
</summary>
]]--
native "HAS_PED_GOT_WEAPON"
	hash "0x8DECB02F88F428BC"
	jhash (0x43D2FA82)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		BOOL "p2",
	}
	returns	"BOOL"

native "IS_PED_WEAPON_READY_TO_SHOOT"
	hash "0xB80CA294F2F26749"
	jhash (0x02A32CB0)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "GET_PED_WEAPONTYPE_IN_SLOT"
	hash "0xEFFED78E9011134D"
	jhash (0x9BC64E16)
	arguments {
		Ped "ped",

		Hash "weaponSlot",
	}
	returns	"Hash"

--[[!
<summary>
	WEAPON::GET_AMMO_IN_PED_WEAPON(PLAYER::PLAYER_PED_ID(), a_0)

	From decompiled scripts
	Returns total ammo in weapon

	GTALua Example :
	natives.WEAPON.GET_AMMO_IN_PED_WEAPON(plyPed, WeaponHash)
</summary>
]]--
native "GET_AMMO_IN_PED_WEAPON"
	hash "0x015A522136D7F951"
	jhash (0x0C755733)
	arguments {
		Ped "ped",

		Hash "weaponhash",
	}
	returns	"int"

native "ADD_AMMO_TO_PED"
	hash "0x78F0424C34306220"
	jhash (0x7F0580C7)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		int "ammo",
	}
	returns	"void"

native "SET_PED_AMMO"
	hash "0x14E56BC5B5DB6A19"
	jhash (0xBF90DF1A)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		int "ammo",
	}
	returns	"void"

native "SET_PED_INFINITE_AMMO"
	hash "0x3EDCB0505123623B"
	jhash (0x9CB8D278)
	arguments {
		Ped "ped",

		BOOL "toggle",

		Hash "weaponHash",
	}
	returns	"void"

native "SET_PED_INFINITE_AMMO_CLIP"
	hash "0x183DADC6AA953186"
	jhash (0x5A5E3B67)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	ammoCount : Use -1 to set unlimited
</summary>
]]--
native "GIVE_WEAPON_TO_PED"
	hash "0xBF0FD6E56C964FCB"
	jhash (0xC4D88A85)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		int "ammoCount",

		BOOL "p4",

		BOOL "equipNow",
	}
	returns	"void"

--[[!
<summary>
	Gives a weapon to PED with a delay, example:

	WEAPON::GIVE_DELAYED_WEAPON_TO_PED(PED::PLAYER_PED_ID(), GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), 1000, false)


</summary>
]]--
native "GIVE_DELAYED_WEAPON_TO_PED"
	hash "0xB282DC6EBD803C75"
	jhash (0x5868D20D)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		int "time",

		BOOL "equipNow",
	}
	returns	"void"

native "REMOVE_ALL_PED_WEAPONS"
	hash "0xF25DF915FA38C5F3"
	jhash (0xA44CE817)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	This native removes a specified weapon from your selected ped.
	Weapon Hashes: http://pastebin.com/0wwDZgkF

	Example:
	C#:
	Function.Call(Hash.REMOVE_WEAPON_FROM_PED, Game.Player.Character, 0x99B507EA);

	C++:
	WEAPON::REMOVE_WEAPON_FROM_PED(PLAYER::PLAYER_PED_ID(), 0x99B507EA);

	The code above removes the knife from the player.

	~ISOFX
</summary>
]]--
native "REMOVE_WEAPON_FROM_PED"
	hash "0x4899CB088EDF59B8"
	jhash (0x9C37F220)
	arguments {
		Ped "ped",

		Hash "weaponHash",
	}
	returns	"void"

--[[!
<summary>
	Hides the players weapon during a cutscene.
</summary>
]]--
native "HIDE_PED_WEAPON_FOR_SCRIPTED_CUTSCENE"
	hash "0x6F6981D2253C208F"
	jhash (0x00CFD6E9)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Has 5 parameters since latest patches, previous declaration:
	void SET_PED_CURRENT_WEAPON_VISIBLE(Any p0, BOOL p1, BOOL p2, BOOL p3)
</summary>
]]--
native "SET_PED_CURRENT_WEAPON_VISIBLE"
	hash "0x0725A4CCFDED9A70"
	jhash (0x00BECD77)
	arguments {
		Ped "ped",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "SET_PED_DROPS_WEAPONS_WHEN_DEAD"
	hash "0x476AE72C1D19D1A8"
	jhash (0x8A444056)
	arguments {
		Ped "ped",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	It determines what weapons caused damage:

	If you want to define only a specific weapon, second parameter=weapon hash code, third parameter=0
	If you want to define any melee weapon, second parameter=0, third parameter=1.
	If you want to identify any weapon (firearms, melee, rockets, etc.), second parameter=0, third parameter=2.
</summary>
]]--
native "HAS_PED_BEEN_DAMAGED_BY_WEAPON"
	hash "0x2D343D2219CD027A"
	jhash (0xCDFBBCC6)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		int "weaponType",
	}
	returns	"BOOL"

native "CLEAR_PED_LAST_WEAPON_DAMAGE"
	hash "0x0E98F88A24C5F4B8"
	jhash (0x52C68832)
	arguments {
		Ped "ped",
	}
	returns	"void"

--[[!
<summary>
	It determines what weapons caused damage:

	If you want to define only a specific weapon, second parameter=weapon hash code, third parameter=0
	If you want to define any melee weapon, second parameter=0, third parameter=1.
	If you want to identify any weapon (firearms, melee, rockets, etc.), second parameter=0, third parameter=2.
</summary>
]]--
native "HAS_ENTITY_BEEN_DAMAGED_BY_WEAPON"
	hash "0x131D401334815E94"
	jhash (0x6DAABB39)
	arguments {
		Entity "entity",

		Hash "weaponHash",

		int "weaponType",
	}
	returns	"BOOL"

native "CLEAR_ENTITY_LAST_WEAPON_DAMAGE"
	hash "0xAC678E40BE7C74D2"
	jhash (0xCEC2732B)
	arguments {
		Entity "entity",
	}
	returns	"void"

native "SET_PED_DROPS_WEAPON"
	hash "0x6B7513D9966FBEC0"
	jhash (0x3D3329FA)
	arguments {
		Ped "ped",
	}
	returns	"void"

native "SET_PED_DROPS_INVENTORY_WEAPON"
	hash "0x208A1888007FC0E6"
	jhash (0x81FFB874)
	arguments {
		Ped "ped",

		Any "p1",

		float "p2",

		float "p3",

		float "p4",

		Any "p5",
	}
	returns	"void"

--[[!
<summary>
	p2 is mostly 1 in the scripts.
</summary>
]]--
native "GET_MAX_AMMO_IN_CLIP"
	hash "0xA38DCFFCEA8962FA"
	jhash (0x6961E2A4)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		BOOL "p2",
	}
	returns	"int"

native "GET_AMMO_IN_CLIP"
	hash "0x2E1202248937775C"
	jhash (0x73C100C3)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		intPtr "ammo",
	}
	returns	"BOOL"

native "SET_AMMO_IN_CLIP"
	hash "0xDCD2A934D65CB497"
	jhash (0xA54B0B10)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		int "ammo",
	}
	returns	"BOOL"

native "GET_MAX_AMMO"
	hash "0xDC16122C7A20C933"
	jhash (0x0B294796)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		intPtr "ammo",
	}
	returns	"BOOL"

native "SET_PED_AMMO_BY_TYPE"
	hash "0x5FD1E1F011E76D7E"
	jhash (0x311C52BB)
	arguments {
		Ped "ped",

		Any "ammoType",

		int "ammo",
	}
	returns	"void"

native "GET_PED_AMMO_BY_TYPE"
	hash "0x39D22031557946C1"
	jhash (0x54077C4D)
	arguments {
		Ped "ped",

		Any "ammoType",
	}
	returns	"int"

native "SET_PED_AMMO_TO_DROP"
	hash "0xA4EFEF9440A5B0EF"
	jhash (0x2386A307)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xE620FD3512A04F18"
	hash "0xE620FD3512A04F18"
	jhash (0xD6460EA2)
	arguments {
		float "p0",
	}
	returns	"void"

native "_GET_PED_AMMO_TYPE"
	hash "0x7FEAD38B326B9F74"
	jhash (0x09337863)
	arguments {
		Ped "ped",

		Hash "weaponHash",
	}
	returns	"Any"

--[[!
<summary>
	Pass ped. Pass address of Vector3.
	The coord will be put into the Vector3.
	The return will determine whether there was a coord found or not.
</summary>
]]--
native "GET_PED_LAST_WEAPON_IMPACT_COORD"
	hash "0x6C4D0409BA1A2BC2"
	jhash (0x9B266079)
	arguments {
		Ped "ped",

		Vector3Ptr "coord",
	}
	returns	"BOOL"

--[[!
<summary>
	p1/gadgetHash was always 0xFBAB5776 ("GADGET_PARACHUTE").
	p2 is always true.

</summary>
]]--
native "SET_PED_GADGET"
	hash "0xD0D7B1E680ED4A1A"
	jhash (0x8A256D0A)
	arguments {
		Ped "ped",

		Hash "gadgetHash",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	gadgetHash - was always 0xFBAB5776 ("GADGET_PARACHUTE").
</summary>
]]--
native "GET_IS_PED_GADGET_EQUIPPED"
	hash "0xF731332072F5156C"
	jhash (0x8DDD0B5B)
	arguments {
		Ped "ped",

		Hash "gadgetHash",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns the hash of the weapon. 

	            var num7 = WEAPON::GET_SELECTED_PED_WEAPON(num4);
	            sub_27D3(num7);
	            switch (num7)
	            {
	                case 0x24B17070:
</summary>
]]--
native "GET_SELECTED_PED_WEAPON"
	hash "0x0A6DB4965674D243"
	jhash (0xD240123E)
	arguments {
		Ped "ped",
	}
	returns	"Hash"

native "EXPLODE_PROJECTILES"
	hash "0xFC4BD125DE7611E4"
	jhash (0x35A0B955)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "REMOVE_ALL_PROJECTILES_OF_TYPE"
	hash "0xFC52E0F37E446528"
	jhash (0xA5F89919)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x840F03E9041E2C9C"
	hash "0x840F03E9041E2C9C"
	jhash (0x3612110D)
	arguments {
		Any "p0",
	}
	returns	"float"

native "GET_MAX_RANGE_OF_CURRENT_PED_WEAPON"
	hash "0x814C9D19DFD69679"
	jhash (0xB2B2BBAA)
	arguments {
		Ped "ped",
	}
	returns	"float"

--[[!
<summary>
	Third Parameter = unsure, but pretty sure it is weapon hash
	--&gt; get_hash_key("weapon_stickybomb")

	Fourth Parameter = unsure, almost always -1
</summary>
]]--
native "HAS_VEHICLE_GOT_PROJECTILE_ATTACHED"
	hash "0x717C8481234E3B88"
	jhash (0xA57E2E80)
	arguments {
		Ped "driver",

		Vehicle "vehicle",

		Hash "weapon",

		Any "p3",
	}
	returns	"BOOL"

native "GIVE_WEAPON_COMPONENT_TO_PED"
	hash "0xD966D51AA5B28BB9"
	jhash (0x3E1E286D)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		Hash "componentHash",
	}
	returns	"void"

native "REMOVE_WEAPON_COMPONENT_FROM_PED"
	hash "0x1E8BE90C74FB4C09"
	jhash (0x412AA00D)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		Hash "componentHash",
	}
	returns	"void"

native "HAS_PED_GOT_WEAPON_COMPONENT"
	hash "0xC593212475FAE340"
	jhash (0xDC0FC145)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		Hash "componentHash",
	}
	returns	"BOOL"

native "IS_PED_WEAPON_COMPONENT_ACTIVE"
	hash "0x0D78DE0572D3969E"
	jhash (0x7565FB19)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		Hash "componentHash",
	}
	returns	"BOOL"

native "_IS_PED_RELOADING"
	hash "0x8C0D57EA686FAD87"
	jhash (0x82EEAF0F)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "MAKE_PED_RELOAD"
	hash "0x20AE33F3AC9C0033"
	jhash (0x515292C2)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

--[[!
<summary>
	Nearly every instance of p1 I found was 31. Nearly every instance of p2 I found was false.
</summary>
]]--
native "REQUEST_WEAPON_ASSET"
	hash "0x5443438F033E29C3"
	jhash (0x65D139A5)
	arguments {
		Hash "weaponHash",

		int "p1",

		BOOL "p2",
	}
	returns	"void"

native "HAS_WEAPON_ASSET_LOADED"
	hash "0x36E353271F0E90EE"
	jhash (0x1891D5BB)
	arguments {
		Hash "weaponHash",
	}
	returns	"BOOL"

native "REMOVE_WEAPON_ASSET"
	hash "0xAA08EF13F341C8FC"
	jhash (0x2C0DFE3C)
	arguments {
		Hash "weaponHash",
	}
	returns	"void"

--[[!
<summary>
	Now has 8 params, previous declaration:
	Any CREATE_WEAPON_OBJECT(Hash weaponHash, int ammoCount, float x, float y, float z, BOOL showWorldModel, float heading)
</summary>
]]--
native "CREATE_WEAPON_OBJECT"
	hash "0x9541D3CF0D398F36"
	jhash (0x62F5987F)
	arguments {
		Hash "weaponHash",

		int "ammoCount",

		float "x",

		float "y",

		float "z",

		BOOL "showWorldModel",

		float "heading",

		Any "p7",
	}
	returns	"Any"

native "GIVE_WEAPON_COMPONENT_TO_WEAPON_OBJECT"
	hash "0x33E179436C0B31DB"
	jhash (0xF7612A37)
	arguments {
		Entity "weaponObject",

		Hash "addonHash",
	}
	returns	"void"

native "REMOVE_WEAPON_COMPONENT_FROM_WEAPON_OBJECT"
	hash "0xF7D82B0D66777611"
	jhash (0xA6E7ED3C)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "HAS_WEAPON_GOT_WEAPON_COMPONENT"
	hash "0x76A18844E743BF91"
	jhash (0x1D368510)
	arguments {
		Entity "weapon",

		Hash "addonHash",
	}
	returns	"BOOL"

native "GIVE_WEAPON_OBJECT_TO_PED"
	hash "0xB1FA61371AF7C4B7"
	jhash (0x639AF3EF)
	arguments {
		Entity "weaponObject",

		Ped "ped",
	}
	returns	"void"

native "DOES_WEAPON_TAKE_WEAPON_COMPONENT"
	hash "0x5CEE3DF569CECAB0"
	jhash (0xB1817BAA)
	arguments {
		Hash "weaponHash",

		Hash "componentHash",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns a weapon object for the currently equipped weapon.

	Unknown behavior when unarmed.
</summary>
]]--
native "GET_WEAPON_OBJECT_FROM_PED"
	hash "0xCAE1DC9A0E22A16D"
	jhash (0xDF939A38)
	arguments {
		Ped "ped",

		BOOL "unknown",
	}
	returns	"Entity"

--[[!
<summary>
	ColorIndex can be the following:


	1 
	2 
	3 
	4 
	5 
	6 
	7 
</summary>
]]--
native "SET_PED_WEAPON_TINT_INDEX"
	hash "0x50969B9B89ED5738"
	jhash (0xEB2A7B23)
	arguments {
		Ped "ped",

		Hash "weaponHash",

		int "colorIndex",
	}
	returns	"void"

native "GET_PED_WEAPON_TINT_INDEX"
	hash "0x2B9EEDC07BD06B9F"
	jhash (0x3F9C90A7)
	arguments {
		Ped "ped",

		Hash "weaponHash",
	}
	returns	"int"

--[[!
<summary>
	m8 do you even int tint?
</summary>
]]--
native "SET_WEAPON_OBJECT_TINT_INDEX"
	hash "0xF827589017D4E4A9"
	jhash (0x44ACC1DA)
	arguments {
		Entity "weapon",

		int "tint",
	}
	returns	"void"

native "GET_WEAPON_OBJECT_TINT_INDEX"
	hash "0xCD183314F7CD2E57"
	jhash (0xD91D9576)
	arguments {
		Entity "weapon",
	}
	returns	"int"

native "GET_WEAPON_TINT_COUNT"
	hash "0x5DCF6C5CAB2E9BF7"
	jhash (0x99E4EAAB)
	arguments {
		Hash "weaponHash",
	}
	returns	"int"

native "GET_WEAPON_HUD_STATS"
	hash "0xD92C739EE34C9EBA"
	jhash (0xA9AD3D98)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "GET_WEAPON_COMPONENT_HUD_STATS"
	hash "0xB3CAF387AE12E9F8"
	jhash (0xBB5498F4)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x3133B907D8B32053"
	hash "0x3133B907D8B32053"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"float"

--[[!
<summary>
	//Returns the size of the default weapon component clip.

	static int GET_WEAPON_CLIP_SIZE(Hash weaponHash) { return invoke&lt;int&gt;(0x583BE370B1EC6EB4, weaponHash); } // 0x583BE370B1EC6EB4 0x8D515E66

	Use it like this:

	char cClipSize[32];
	Hash cur;
	if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &amp;cur, 1))
	{
	    if (WEAPON::IS_WEAPON_VALID(cur))
	    {
	        int iClipSize = WEAPON::GET_WEAPON_CLIP_SIZE(cur);
	        sprintf_s(cClipSize, "ClipSize: %.d", iClipSize);
	        vDrawString(cClipSize, 0.5f, 0.5f);
	    }
	}

	//GagoCash

</summary>
]]--
native "GET_WEAPON_CLIP_SIZE"
	hash "0x583BE370B1EC6EB4"
	jhash (0x8D515E66)
	arguments {
		Hash "weaponHash",
	}
	returns	"int"

native "SET_PED_CHANCE_OF_FIRING_BLANKS"
	hash "0x8378627201D5497D"
	jhash (0xB4F44C6E)
	arguments {
		Hash "weaponHash",

		float "xBias",

		float "yBias",
	}
	returns	"void"

--[[!
<summary>
	This does not take a weapon hash...
</summary>
]]--
native "0xB4C8D77C80C0421E"
	hash "0xB4C8D77C80C0421E"
	jhash (0xEC2E5304)
	arguments {
		Ped "ped",

		float "p1",
	}
	returns	"Entity"

native "REQUEST_WEAPON_HIGH_DETAIL_MODEL"
	hash "0x48164DBB970AC3F0"
	jhash (0xE3BD00F9)
	arguments {
		Entity "weaponObject",
	}
	returns	"void"

--[[!
<summary>
	This native returns a true or false value.

	Ped ped = The ped whose weapon you want to check.

	~ISOFX
</summary>
]]--
native "IS_PED_CURRENT_WEAPON_SILENCED"
	hash "0x65F0C5AE05943EC7"
	jhash (0xBAF7BFBE)
	arguments {
		Ped "ped",
	}
	returns	"BOOL"

native "SET_WEAPON_SMOKEGRENADE_ASSIGNED"
	hash "0x4B7620C47217126C"
	jhash (0x76876154)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SET_FLASH_LIGHT_FADE_DISTANCE"
	hash "0xCEA66DAD478CD39B"
	jhash (0xB0127EA7)
	arguments {
		float "distance",
	}
	returns	"Any"

native "SET_WEAPON_ANIMATION_OVERRIDE"
	hash "0x1055AC3A667F09D9"
	jhash (0xA5DF7484)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "GET_WEAPON_DAMAGE_TYPE"
	hash "0x3BE0BB12D25FB305"
	jhash (0x013AFC13)
	arguments {
		Hash "weaponHash",
	}
	returns	"int"

native "0xE4DCEC7FD5B739A5"
	hash "0xE4DCEC7FD5B739A5"
	jhash (0x64646F1D)
	arguments {
		Any "p0",
	}
	returns	"void"

native "CAN_USE_WEAPON_ON_PARACHUTE"
	hash "0xBC7BE5ABC0879F74"
	jhash (0x135E7AD4)
	arguments {
		Hash "weaponHash",
	}
	returns	"BOOL"

native "CREATE_ITEMSET"
	hash "0x35AD299F50D91B24"
	jhash (0x0A113B2C)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "DESTROY_ITEMSET"
	hash "0xDE18220B1C183EDA"
	jhash (0x83CE1A4C)
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_ITEMSET_VALID"
	hash "0xB1B1EA596344DFAB"
	jhash (0xD201FC29)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "ADD_TO_ITEMSET"
	hash "0xE3945201F14637DD"
	jhash (0x6B0FE61B)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "REMOVE_FROM_ITEMSET"
	hash "0x25E68244B0177686"
	jhash (0xA9565228)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "GET_ITEMSET_SIZE"
	hash "0xD9127E83ABF7C631"
	jhash (0x2B31F41A)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_INDEXED_ITEM_IN_ITEMSET"
	hash "0x7A197E2521EE2BAB"
	jhash (0x3F712874)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "IS_IN_ITEMSET"
	hash "0x2D0FC594D1E9C107"
	jhash (0x0D4B9730)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "CLEAN_ITEMSET"
	hash "0x41BC0D722FC04221"
	jhash (0x919A4858)
	arguments {
		Any "p0",
	}
	returns	"void"

native "LOAD_ALL_OBJECTS_NOW"
	hash "0xBD6E84632DD4CB3F"
	jhash (0xC9DBDA90)
	returns	"void"

native "LOAD_SCENE"
	hash "0x4448EB75B4904BDB"
	jhash (0xB72403F5)
	arguments {
		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "NETWORK_UPDATE_LOAD_SCENE"
	hash "0xC4582015556D1C46"
	jhash (0xC76E023C)
	returns	"Any"

native "NETWORK_STOP_LOAD_SCENE"
	hash "0x64E630FAF5F60F44"
	jhash (0x24857907)
	returns	"void"

native "IS_NETWORK_LOADING_SCENE"
	hash "0x41CA5A33160EA4AB"
	jhash (0x6DCFC021)
	returns	"BOOL"

native "SET_INTERIOR_ACTIVE"
	hash "0xE37B76C387BE28ED"
	jhash (0xE1013910)
	arguments {
		Any "interior",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Request a model to be loaded.
</summary>
]]--
native "REQUEST_MODEL"
	hash "0x963D27A58DF860AC"
	jhash (0xFFF1B500)
	arguments {
		Hash "model",
	}
	returns	"void"

native "0xA0261AEF7ACFC51E"
	hash "0xA0261AEF7ACFC51E"
	jhash (0x48CEB6B4)
	arguments {
		Hash "model",
	}
	returns	"void"

--[[!
<summary>
	Check if a model has been loaded.
</summary>
]]--
native "HAS_MODEL_LOADED"
	hash "0x98A4EB5D89A0C952"
	jhash (0x62BFDB37)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

--[[!
<summary>
	Something to do with interiors that have been loaded.

	STREAMING::_8A7A40100EDFEC58(l_13BC, "V_FIB01_cur_elev");
	STREAMING::_8A7A40100EDFEC58(l_13BC, "limbo");
	STREAMING::_8A7A40100EDFEC58(l_13BB, "V_Office_gnd_lifts");
	STREAMING::_8A7A40100EDFEC58(l_13BB, "limbo");
	STREAMING::_8A7A40100EDFEC58(l_13BC, "v_fib01_jan_elev");
	STREAMING::_8A7A40100EDFEC58(l_13BC, "limbo");
</summary>
]]--
native "0x8A7A40100EDFEC58"
	hash "0x8A7A40100EDFEC58"
	jhash (0x939243FB)
	arguments {
		Any "interior",

		charPtr "p1",
	}
	returns	"void"

native "SET_MODEL_AS_NO_LONGER_NEEDED"
	hash "0xE532F5D78798DAAB"
	jhash (0xAE0F069E)
	arguments {
		Hash "model",
	}
	returns	"void"

native "IS_MODEL_IN_CDIMAGE"
	hash "0x35B9E0803292B641"
	jhash (0x1094782F)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns whether the specified model exists in the game.
</summary>
]]--
native "IS_MODEL_VALID"
	hash "0xC0296A2EDF545E92"
	jhash (0xAF8F8E9D)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns whether the specified model represents a vehicle.
</summary>
]]--
native "IS_MODEL_A_VEHICLE"
	hash "0x19AAC8F07BFEC53E"
	jhash (0xFFFC85D4)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

native "REQUEST_COLLISION_AT_COORD"
	hash "0x07503F7948F491A7"
	jhash (0xCD9805E7)
	arguments {
		float "x",

		float "y",

		float "z",
	}
	returns	"Any"

native "REQUEST_COLLISION_FOR_MODEL"
	hash "0x923CB32A3B874FCB"
	jhash (0x3930C042)
	arguments {
		Hash "model",
	}
	returns	"void"

native "HAS_COLLISION_FOR_MODEL_LOADED"
	hash "0x22CCA434E368F03A"
	jhash (0x41A094F8)
	arguments {
		Hash "model",
	}
	returns	"BOOL"

native "REQUEST_ADDITIONAL_COLLISION_AT_COORD"
	hash "0xC9156DC11411A9EA"
	jhash (0xC2CC1DF2)
	arguments {
		float "p0",

		float "p1",

		float "p2",
	}
	returns	"void"

native "DOES_ANIM_DICT_EXIST"
	hash "0x2DA49C3B79856961"
	jhash (0xCD31C872)
	arguments {
		charPtr "animDict",
	}
	returns	"BOOL"

native "REQUEST_ANIM_DICT"
	hash "0xD3BD40951412FEF6"
	jhash (0xDCA96950)
	arguments {
		charPtr "animDict",
	}
	returns	"void"

native "HAS_ANIM_DICT_LOADED"
	hash "0xD031A9162D01088C"
	jhash (0x05E6763C)
	arguments {
		charPtr "animDict",
	}
	returns	"BOOL"

native "REMOVE_ANIM_DICT"
	hash "0xF66A602F829E2A06"
	jhash (0x0AE050B5)
	arguments {
		charPtr "animDict",
	}
	returns	"void"

--[[!
<summary>
	Starts loading the specified animation set. An animation set provides movement animations for a ped. See SET_PED_MOVEMENT_CLIPSET.

	Animation set and clip set are synonymous.
</summary>
]]--
native "REQUEST_ANIM_SET"
	hash "0x6EA47DAE7FAD0EED"
	jhash (0x2988B3FC)
	arguments {
		charPtr "animSet",
	}
	returns	"void"

--[[!
<summary>
	Gets whether the specified animation set has finished loading. An animation set provides movement animations for a ped. See SET_PED_MOVEMENT_CLIPSET.

	Animation set and clip set are synonymous.
</summary>
]]--
native "HAS_ANIM_SET_LOADED"
	hash "0xC4EA073D86FB29B0"
	jhash (0x4FFF397D)
	arguments {
		charPtr "animSet",
	}
	returns	"BOOL"

--[[!
<summary>
	Unloads the specified animation set. An animation set provides movement animations for a ped. See SET_PED_MOVEMENT_CLIPSET.

	Animation set and clip set are synonymous.
</summary>
]]--
native "REMOVE_ANIM_SET"
	hash "0x16350528F93024B3"
	jhash (0xD04A817A)
	arguments {
		charPtr "animSet",
	}
	returns	"void"

--[[!
<summary>
	Alias for REQUEST_ANIM_SET.
</summary>
]]--
native "REQUEST_CLIP_SET"
	hash "0xD2A71E1A77418A49"
	jhash (0x546C627A)
	arguments {
		charPtr "clipSet",
	}
	returns	"void"

--[[!
<summary>
	Alias for HAS_ANIM_SET_LOADED.
</summary>
]]--
native "HAS_CLIP_SET_LOADED"
	hash "0x318234F4F3738AF3"
	jhash (0x230D5455)
	arguments {
		charPtr "clipSet",
	}
	returns	"BOOL"

--[[!
<summary>
	Alias for REMOVE_ANIM_SET.
</summary>
]]--
native "REMOVE_CLIP_SET"
	hash "0x01F73A131C18CD94"
	jhash (0x1E21F7AA)
	arguments {
		charPtr "clipSet",
	}
	returns	"void"

--[[!
<summary>
	Exemple: REQUEST_IPL("TrevorsTrailerTrash");

	IPL + Coords: http://pastebin.com/FyV5mMma
</summary>
]]--
native "REQUEST_IPL"
	hash "0x41B4893843BBDB74"
	jhash (0x3B70D1DB)
	arguments {
		charPtr "iplName",
	}
	returns	"void"

--[[!
<summary>
	Removes an IPL from the map.
	IPL List: http://pastebin.com/pwkh0uRP 

	Example:
	C#:
	Function.Call(Hash.REMOVE_IPL, "trevorstrailertidy");

	C++:
	STREAMING::REMOVE_IPL("trevorstrailertidy");

	iplName = Name of IPL you want to remove.

	~ISOFX
</summary>
]]--
native "REMOVE_IPL"
	hash "0xEE6C5AD3ECE0A82D"
	jhash (0xDF7CBD36)
	arguments {
		charPtr "iplName",
	}
	returns	"void"

native "IS_IPL_ACTIVE"
	hash "0x88A741E44A2B3495"
	jhash (0xB2C33714)
	arguments {
		charPtr "iplName",
	}
	returns	"BOOL"

native "SET_STREAMING"
	hash "0x6E0C692677008888"
	jhash (0x27EF6CB2)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "SET_GAME_PAUSES_FOR_STREAMING"
	hash "0x717CD6E6FAEBBEDC"
	jhash (0x9211A28A)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "SET_REDUCE_PED_MODEL_BUDGET"
	hash "0x77B5F9A36BF96710"
	jhash (0xAFCB2B86)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "SET_REDUCE_VEHICLE_MODEL_BUDGET"
	hash "0x80C527893080CCF3"
	jhash (0xCDB4FB7E)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "SET_DITCH_POLICE_MODELS"
	hash "0x42CBE54462D92634"
	jhash (0x3EA7FCE4)
	arguments {
		Any "p0",
	}
	returns	"void"

native "GET_NUMBER_OF_STREAMING_REQUESTS"
	hash "0x4060057271CEBC89"
	jhash (0xC2EE9A02)
	returns	"Any"

--[[!
<summary>
	maps script name (thread + 0xD0) by lookup via scriptfx.dat - does nothing when script name is empty
</summary>
]]--
native "REQUEST_PTFX_ASSET"
	hash "0x944955FB2A3935C8"
	jhash (0x2C649263)
	returns	"Any"

native "HAS_PTFX_ASSET_LOADED"
	hash "0xCA7D9B86ECA7481B"
	jhash (0x3EFF96BE)
	returns	"BOOL"

native "REMOVE_PTFX_ASSET"
	hash "0x88C6814073DD4A73"
	jhash (0xC10F178C)
	returns	"void"

native "REQUEST_NAMED_PTFX_ASSET"
	hash "0xB80D8756B4668AB6"
	jhash (0xCFEA19A9)
	arguments {
		charPtr "fxName",
	}
	returns	"void"

native "HAS_NAMED_PTFX_ASSET_LOADED"
	hash "0x8702416E512EC454"
	jhash (0x9ACC6446)
	arguments {
		charPtr "fxName",
	}
	returns	"BOOL"

native "_REMOVE_NAMED_PTFX_ASSET"
	hash "0x5F61EBBE1A00F96D"
	arguments {
		charPtr "fxName",
	}
	returns	"void"

native "SET_VEHICLE_POPULATION_BUDGET"
	hash "0xCB9E1EB3BE2AF4E9"
	jhash (0x1D56993C)
	arguments {
		int "p0",
	}
	returns	"void"

native "SET_PED_POPULATION_BUDGET"
	hash "0x8C95333CFC3340F3"
	jhash (0xD2D026CD)
	arguments {
		int "p0",
	}
	returns	"void"

native "CLEAR_FOCUS"
	hash "0x31B73D1EA9F01DA2"
	jhash (0x34D91E7A)
	returns	"void"

--[[!
<summary>
	Override the area where the camera will render the terrain
</summary>
]]--
native "_SET_FOCUS_AREA"
	hash "0xBB7454BAFF08FE25"
	jhash (0x14680A60)
	arguments {
		float "x",

		float "y",

		float "z",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"void"

--[[!
<summary>
	It seems to make the entity's coords mark the point from which LOD-distances are measured. In my testing, setting a vehicle as the focus entity and moving that vehicle more than 300 distance units away from the player will make the level of detail around the player go down drastically (shadows disappear, textures go extremely low res, etc). The player seems to be the default focus entity.
</summary>
]]--
native "SET_FOCUS_ENTITY"
	hash "0x198F77705FA0931D"
	jhash (0x18DB04AC)
	arguments {
		Entity "p0",
	}
	returns	"void"

native "IS_ENTITY_FOCUS"
	hash "0x2DDFF3FB9075D747"
	jhash (0xB456D707)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x0811381EF5062FEC"
	hash "0x0811381EF5062FEC"
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	MODZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZS
</summary>
]]--
native "0xAF12610C644A35C9"
	hash "0xAF12610C644A35C9"
	jhash (0x403CD434)
	arguments {
		AnyPtr "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x4E52E752C76E7E7A"
	hash "0x4E52E752C76E7E7A"
	jhash (0xA07BAEB9)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x219C7B8D53E429FD"
	hash "0x219C7B8D53E429FD"
	jhash (0x10B6AB36)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",

		Any "p5",
	}
	returns	"Any"

native "0x1F3F018BC3AFA77C"
	hash "0x1F3F018BC3AFA77C"
	jhash (0x72344191)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		Any "p7",

		Any "p8",
	}
	returns	"Any"

native "0x0AD9710CEE2F590F"
	hash "0x0AD9710CEE2F590F"
	jhash (0xC0157255)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		Any "p6",
	}
	returns	"Any"

native "0x1EE7D8DF4425F053"
	hash "0x1EE7D8DF4425F053"
	jhash (0xE80F8ABE)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x7D41E9D2D17C5B2D"
	hash "0x7D41E9D2D17C5B2D"
	jhash (0x1B3521F4)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x07C313F94746702C"
	hash "0x07C313F94746702C"
	jhash (0x42CFE9C0)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xBC9823AB80A3DCAC"
	hash "0xBC9823AB80A3DCAC"
	jhash (0x56253356)
	returns	"Any"

native "NEW_LOAD_SCENE_START"
	hash "0x212A8D0D2BABFAC2"
	jhash (0xDF9C38B6)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		Any "p7",
	}
	returns	"BOOL"

native "0xACCFB4ACF53551B0"
	hash "0xACCFB4ACF53551B0"
	jhash (0xFA037FEB)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		Any "p4",
	}
	returns	"BOOL"

native "NEW_LOAD_SCENE_STOP"
	hash "0xC197616D221FF4A4"
	jhash (0x7C05B1F6)
	returns	"void"

native "IS_NEW_LOAD_SCENE_ACTIVE"
	hash "0xA41A05B6CB741B85"
	jhash (0xAD234B7F)
	returns	"BOOL"

native "IS_NEW_LOAD_SCENE_LOADED"
	hash "0x01B8247A7A8B9AD1"
	jhash (0x3ECD839F)
	returns	"BOOL"

native "0x71E7B2E657449AAD"
	hash "0x71E7B2E657449AAD"
	jhash (0xEAA51103)
	returns	"Any"

native "START_PLAYER_SWITCH"
	hash "0xFAA23F2CBA159D67"
	jhash (0x0829E975)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "STOP_PLAYER_SWITCH"
	hash "0x95C0A5BBDC189AA1"
	jhash (0x2832C010)
	returns	"void"

--[[!
<summary>
	Returns true if the player is currently switching, false otherwise.
	(When the camera is in the sky moving from Trevor to Franklin for example)
</summary>
]]--
native "0xD9D2CFFF49FAB35F"
	hash "0xD9D2CFFF49FAB35F"
	jhash (0x56135ACC)
	returns	"BOOL"

native "GET_PLAYER_SWITCH_TYPE"
	hash "0xB3C94A90D9FC9E62"
	jhash (0x280DC015)
	returns	"Any"

native "GET_IDEAL_PLAYER_SWITCH_TYPE"
	hash "0xB5D7B26B45720E05"
	jhash (0xD5A450F1)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"Any"

native "GET_PLAYER_SWITCH_STATE"
	hash "0x470555300D10B2A5"
	jhash (0x39A0E1F2)
	returns	"Any"

native "GET_PLAYER_SHORT_SWITCH_STATE"
	hash "0x20F898A5D9782800"
	jhash (0x9B7BA38F)
	returns	"Any"

native "0x5F2013F8BC24EE69"
	hash "0x5F2013F8BC24EE69"
	jhash (0xF0BD420D)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x78C0D93253149435"
	hash "0x78C0D93253149435"
	jhash (0x02BA7AC2)
	returns	"Any"

native "0xC208B673CE446B61"
	hash "0xC208B673CE446B61"
	jhash (0x47352E14)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		Any "p8",
	}
	returns	"void"

native "0x0FDE9DBFC0A6BC65"
	hash "0x0FDE9DBFC0A6BC65"
	jhash (0x279077B0)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x43D1680C6D19A8E9"
	hash "0x43D1680C6D19A8E9"
	jhash (0x55CB21F9)
	returns	"void"

native "0x74DE2E8739086740"
	hash "0x74DE2E8739086740"
	jhash (0x1084F2F4)
	returns	"void"

native "0x8E2A065ABDAE6994"
	hash "0x8E2A065ABDAE6994"
	jhash (0x5B1E995D)
	returns	"void"

native "0xAD5FDF34B81BFE79"
	hash "0xAD5FDF34B81BFE79"
	jhash (0x4B4B9A13)
	returns	"void"

native "0xDFA80CB25D0A19B3"
	hash "0xDFA80CB25D0A19B3"
	jhash (0x408F7148)
	returns	"Any"

native "0xD4793DFF3AF2ABCD"
	hash "0xD4793DFF3AF2ABCD"
	returns	"void"

native "0xBD605B8E0E18B3BB"
	hash "0xBD605B8E0E18B3BB"
	returns	"void"

native "0xAAB3200ED59016BC"
	hash "0xAAB3200ED59016BC"
	jhash (0xFB4D062D)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0xD8295AF639FD9CB8"
	hash "0xD8295AF639FD9CB8"
	jhash (0x2349373B)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x933BBEEB8C61B5F4"
	hash "0x933BBEEB8C61B5F4"
	jhash (0x74C16879)
	returns	"Any"

native "SET_PLAYER_INVERTED_UP"
	hash "0x08C2D6C52A3104BB"
	jhash (0x569847E3)
	returns	"Any"

native "0x5B48A06DD0E792A5"
	hash "0x5B48A06DD0E792A5"
	jhash (0xC7A3D279)
	returns	"Any"

native "DESTROY_PLAYER_IN_PAUSE_MENU"
	hash "0x5B74EA8CFD5E3E7E"
	jhash (0x90F64284)
	returns	"Any"

native "0x1E9057A74FD73E23"
	hash "0x1E9057A74FD73E23"
	returns	"void"

native "0x0C15B0E443B2349D"
	hash "0x0C15B0E443B2349D"
	jhash (0x7154B6FD)
	returns	"Any"

native "0xA76359FC80B2438E"
	hash "0xA76359FC80B2438E"
	jhash (0xE5612C1A)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xBED8CA5FF5E04113"
	hash "0xBED8CA5FF5E04113"
	jhash (0x9CD6A451)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "0x472397322E92A856"
	hash "0x472397322E92A856"
	jhash (0x4267DA87)
	returns	"void"

native "0x40AEFD1A244741F2"
	hash "0x40AEFD1A244741F2"
	jhash (0x9FA4AF99)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x03F1A106BDA7DD3E"
	hash "0x03F1A106BDA7DD3E"
	returns	"void"

native "0x95A7DABDDBB78AE7"
	hash "0x95A7DABDDBB78AE7"
	jhash (0x9EF0A9CF)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x63EB2B972A218CAC"
	hash "0x63EB2B972A218CAC"
	jhash (0xF2CDD6A8)
	returns	"void"

native "0xFB199266061F820A"
	hash "0xFB199266061F820A"
	jhash (0x17B0A1CD)
	returns	"Any"

native "0xF4A0DADB70F57FA6"
	hash "0xF4A0DADB70F57FA6"
	jhash (0x3DA7AA5D)
	returns	"void"

native "0x5068F488DDB54DD8"
	hash "0x5068F488DDB54DD8"
	jhash (0xDAB4BAC0)
	returns	"Any"

native "PREFETCH_SRL"
	hash "0x3D245789CE12982C"
	jhash (0x37BE2FBB)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "IS_SRL_LOADED"
	hash "0xD0263801A4C5B0BB"
	jhash (0x670FA2A6)
	returns	"BOOL"

native "BEGIN_SRL"
	hash "0x9BADDC94EF83B823"
	jhash (0x24F49427)
	returns	"void"

native "END_SRL"
	hash "0x0A41540E63C9EE17"
	jhash (0x1977C56A)
	returns	"void"

native "SET_SRL_TIME"
	hash "0xA74A541C6884E7B8"
	jhash (0x30F8A487)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xEF39EE20C537E98C"
	hash "0xEF39EE20C537E98C"
	jhash (0x814D0752)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"void"

native "0xBEB2D9A1D9A8F55A"
	hash "0xBEB2D9A1D9A8F55A"
	jhash (0x62F02485)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x20C6C7E4EB082A7F"
	hash "0x20C6C7E4EB082A7F"
	jhash (0xA6459CAA)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xF8155A7F03DDFC8E"
	hash "0xF8155A7F03DDFC8E"
	jhash (0xF8F515E4)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_HD_AREA"
	hash "0xB85F26619073E775"
	jhash (0x80BAA035)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "CLEAR_HD_AREA"
	hash "0xCE58B1CFB9290813"
	jhash (0x7CAC6FA0)
	returns	"void"

native "0xB5A4DB34FE89B88A"
	hash "0xB5A4DB34FE89B88A"
	jhash (0xE243B2AF)
	returns	"void"

native "0xCCE26000E9A6FAD7"
	hash "0xCCE26000E9A6FAD7"
	jhash (0x897A510F)
	returns	"void"

native "0x0BC3144DEB678666"
	hash "0x0BC3144DEB678666"
	jhash (0xC0E83320)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xF086AD9354FAC3A3"
	hash "0xF086AD9354FAC3A3"
	jhash (0x1C576388)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x3D3D8B3BE5A83D35"
	hash "0x3D3D8B3BE5A83D35"
	jhash (0x3E9C4CBE)
	returns	"Any"

--[[!
<summary>
	List Of Scripts: http://pastebin.com/yK25rsY0

	~ISOFX
</summary>
]]--
native "REQUEST_SCRIPT"
	hash "0x6EB5F71AA68F2E8E"
	jhash (0xE26B2666)
	arguments {
		charPtr "scriptName",
	}
	returns	"void"

native "SET_SCRIPT_AS_NO_LONGER_NEEDED"
	hash "0xC90D2DCACD56184C"
	jhash (0x6FCB7795)
	arguments {
		charPtr "scriptName",
	}
	returns	"void"

--[[!
<summary>
	Returns if a script has been loaded into the game. Used to see if a script was loaded after requesting.
</summary>
]]--
native "HAS_SCRIPT_LOADED"
	hash "0xE6CC9F3BA0FB9EF1"
	jhash (0x5D67F751)
	arguments {
		charPtr "scriptName",
	}
	returns	"BOOL"

native "DOES_SCRIPT_EXIST"
	hash "0xFC04745FBE67C19A"
	jhash (0xDEAB87AB)
	arguments {
		charPtr "scriptName",
	}
	returns	"BOOL"

native "_REQUEST_STREAMED_SCRIPT"
	hash "0xD62A67D26D9653E6"
	jhash (0x1C68D9DC)
	arguments {
		Hash "scriptHash",
	}
	returns	"void"

native "_SET_STREAMED_SCRIPT_AS_NO_LONGER_NEEDED"
	hash "0xC5BC038960E9DB27"
	jhash (0x96C26F66)
	arguments {
		Hash "scriptHash",
	}
	returns	"void"

native "_HAS_STREAMED_SCRIPT_LOADED"
	hash "0x5F0F0C783EB16C04"
	jhash (0x06674818)
	arguments {
		Hash "scriptHash",
	}
	returns	"BOOL"

native "0xF86AA3C56BA31381"
	hash "0xF86AA3C56BA31381"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "TERMINATE_THREAD"
	hash "0xC8B189ED9138BCD4"
	jhash (0x253FD520)
	arguments {
		int "id",
	}
	returns	"void"

native "IS_THREAD_ACTIVE"
	hash "0x46E9AE36D8FA6417"
	jhash (0x78D7A5A0)
	arguments {
		int "threadId",
	}
	returns	"BOOL"

--[[!
<summary>
	The reversed code looks like this (Sasuke78200)

	//
	char g_szScriptName[64];

	char* _0xBE7ACD89(int a_iThreadID)
	{
		scrThread* l_pThread;

		// Get the script thread
		l_pThread = GetThreadByID(a_iThreadID);	

		if(l_pThread == 0 || l_pThread-&gt;m_iThreadState == 2)
		{
			strncpy(g_szScriptName, "", 64);
		}
		else
		{
			strncpy(g_szScriptName, l_pThread-&gt;m_szScriptName, 64);
		}	

		return g_szScriptName;
	}
</summary>
]]--
native "_GET_THREAD_NAME"
	hash "0x05A42BA9FC8DA96B"
	jhash (0xBE7ACD89)
	arguments {
		int "threadId",
	}
	returns	"charPtr"

native "0xDADFADA5A20143A8"
	hash "0xDADFADA5A20143A8"
	jhash (0xBB4E2F66)
	returns	"void"

--[[!
<summary>
	Returns state of some script thread which 16th offset member isn't equal to 2.
</summary>
]]--
native "0x30B4FA1C82DD4B9F"
	hash "0x30B4FA1C82DD4B9F"
	jhash (0x1E28B28F)
	returns	"int"

native "GET_ID_OF_THIS_THREAD"
	hash "0xC30338E8088E2E21"
	jhash (0xDE524830)
	returns	"int"

native "TERMINATE_THIS_THREAD"
	hash "0x1090044AD1DA76FA"
	jhash (0x3CD9CBB7)
	returns	"void"

--[[!
<summary>
	Gets the number of instances of the specified script is currently running.
</summary>
]]--
native "_GET_NUMBER_OF_INSTANCES_OF_STREAMED_SCRIPT"
	hash "0x2C83A9DA6BFFC4F9"
	jhash (0x029D3841)
	arguments {
		Hash "scriptHash",
	}
	returns	"int"

native "GET_THIS_SCRIPT_NAME"
	hash "0x442E0A7EDE4A738A"
	jhash (0xA40FD5D9)
	returns	"charPtr"

native "_GET_THIS_SCRIPT_HASH"
	hash "0x8A1C8B1738FFE87E"
	jhash (0x2BEE1F45)
	returns	"Hash"

native "GET_NUMBER_OF_EVENTS"
	hash "0x5F92A689A06620AA"
	jhash (0xA3525D60)
	arguments {
		int "p0",
	}
	returns	"int"

native "GET_EVENT_EXISTS"
	hash "0x936E6168A9BCEDB5"
	jhash (0xA1B447B5)
	arguments {
		int "p0",

		int "eventIndex",
	}
	returns	"BOOL"

native "GET_EVENT_AT_INDEX"
	hash "0xD8F66A3A60C62153"
	jhash (0xB49C1442)
	arguments {
		int "p0",

		int "eventIndex",
	}
	returns	"int"

native "GET_EVENT_DATA"
	hash "0x2902843FCD2B2D79"
	jhash (0x4280F92F)
	arguments {
		int "p0",

		int "eventIndex",

		intPtr "eventData",

		int "p3",
	}
	returns	"BOOL"

native "TRIGGER_SCRIPT_EVENT"
	hash "0x5AE99C571D5BBE5D"
	jhash (0x54763B35)
	arguments {
		Any "p0",

		intPtr "p1",

		int "p2",

		int "p3",
	}
	returns	"void"

native "SHUTDOWN_LOADING_SCREEN"
	hash "0x078EBE9809CCD637"
	jhash (0xA2826D17)
	returns	"void"

native "SET_NO_LOADING_SCREEN"
	hash "0x5262CC1995D07E09"
	jhash (0xC8055034)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "_GET_NO_LOADING_SCREEN"
	hash "0x18C1270EA7F199BC"
	returns	"BOOL"

native "0xB1577667C3708F9B"
	hash "0xB1577667C3708F9B"
	jhash (0xB03BCCDF)
	returns	"void"

--[[!
<summary>
	Unknown.

	Examples:
	UI::_0xABA17D7CE615ADBF("");
	UI::_0xABA17D7CE615ADBF("CELEB_WPLYRS");
	UI::_0xABA17D7CE615ADBF("CELL_SPINNER2");
	UI::_0xABA17D7CE615ADBF("ERROR_CHECKPROFANITY");
	UI::_0xABA17D7CE615ADBF("FM_COR_AUTOD");
	UI::_0xABA17D7CE615ADBF("FM_IHELP_WAT2");
	UI::_0xABA17D7CE615ADBF("FM_JIP_WAITO");
	UI::_0xABA17D7CE615ADBF("FMMC_DOWNLOAD");
	UI::_0xABA17D7CE615ADBF("FMMC_PLYLOAD");
	UI::_0xABA17D7CE615ADBF("FMMC_STARTTRAN");
	UI::_0xABA17D7CE615ADBF("KILL_STRIP_IDM");
	UI::_0xABA17D7CE615ADBF("MP_SPINLOADING");
</summary>
]]--
native "0xABA17D7CE615ADBF"
	hash "0xABA17D7CE615ADBF"
	jhash (0xCB7C8994)
	arguments {
		charPtr "p0",
	}
	returns	"void"

native "0xBD12F8228410D9B4"
	hash "0xBD12F8228410D9B4"
	jhash (0x903F5EE4)
	arguments {
		int "p0",
	}
	returns	"Any"

native "0x10D373323E5B9C0D"
	hash "0x10D373323E5B9C0D"
	jhash (0x94119534)
	returns	"void"

native "0xC65AB383CD91DF98"
	hash "0xC65AB383CD91DF98"
	jhash (0x71077FBD)
	returns	"void"

--[[!
<summary>
	Unknown function.

	Examples from "fm_mission_controller.ysc" -

	Line 562:
	if (UI::_0xD422FCC5F239A915()) {
	    if (sub_1b78c8()) {
	        UI::_0x10D373323E5B9C0D();
	    }
	}

	Line 6756:
	if (!UI::_0xD422FCC5F239A915()) {
	    UI::_0xABA17D7CE615ADBF("MP_SPINLOADING");
	    UI::_0xBD12F8228410D9B4(5);
	}


</summary>
]]--
native "0xD422FCC5F239A915"
	hash "0xD422FCC5F239A915"
	jhash (0xB8B3A5D0)
	returns	"BOOL"

native "0xB2A592B04648A9CB"
	hash "0xB2A592B04648A9CB"
	returns	"Any"

native "0x9245E81072704B8A"
	hash "0x9245E81072704B8A"
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Shows the cursor on screen for the frame its called. 
</summary>
]]--
native "_SHOW_CURSOR_THIS_FRAME"
	hash "0xAAE7CE1D63167423"
	returns	"void"

native "0x8DB8CFFD58B62552"
	hash "0x8DB8CFFD58B62552"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x98215325A695E78A"
	hash "0x98215325A695E78A"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x3D9ACB1EB139E702"
	hash "0x3D9ACB1EB139E702"
	returns	"Any"

native "0x632B2940C67F4EA9"
	hash "0x632B2940C67F4EA9"
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"BOOL"

native "0x6F1554B0CC2089FA"
	hash "0x6F1554B0CC2089FA"
	jhash (0xA7C8594B)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x55598D21339CB998"
	hash "0x55598D21339CB998"
	jhash (0x1DA7E41A)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x25F87B30C382FCA7"
	hash "0x25F87B30C382FCA7"
	jhash (0x1E63088A)
	returns	"void"

native "0xA8FDB297A8D25FBA"
	hash "0xA8FDB297A8D25FBA"
	jhash (0x5205C6F5)
	returns	"void"

--[[!
<summary>
	Removes a notification instantly instead of waiting for it to disappear
</summary>
]]--
native "_REMOVE_NOTIFICATION"
	hash "0xBE4390CB40B3E627"
	jhash (0xECA8ACB9)
	arguments {
		int "handle",
	}
	returns	"void"

native "0xA13C11E1B5C06BFC"
	hash "0xA13C11E1B5C06BFC"
	jhash (0x520FCB6D)
	returns	"void"

native "0x583049884A2EEE3C"
	hash "0x583049884A2EEE3C"
	jhash (0xC8BAB2F2)
	returns	"void"

native "0xFDB423997FA30340"
	hash "0xFDB423997FA30340"
	jhash (0x4D0449C6)
	returns	"void"

native "0xE1CD1E48E025E661"
	hash "0xE1CD1E48E025E661"
	jhash (0xD3F40140)
	returns	"void"

native "0xA9CBFD40B3FA3010"
	hash "0xA9CBFD40B3FA3010"
	jhash (0xC5223796)
	returns	"Any"

native "0xD4438C0564490E63"
	hash "0xD4438C0564490E63"
	jhash (0x709B4BCB)
	returns	"void"

native "0xB695E2CD0A2DA9EE"
	hash "0xB695E2CD0A2DA9EE"
	jhash (0x4A4A40A4)
	returns	"void"

native "0x82352748437638CA"
	hash "0x82352748437638CA"
	jhash (0x294405D4)
	returns	"Any"

native "0x56C8B608CFD49854"
	hash "0x56C8B608CFD49854"
	jhash (0xF881AB87)
	returns	"void"

native "0xADED7F5748ACAFE6"
	hash "0xADED7F5748ACAFE6"
	jhash (0x1D6859CA)
	returns	"void"

native "0x92F0DA1E27DB96DC"
	hash "0x92F0DA1E27DB96DC"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x17430B918701C342"
	hash "0x17430B918701C342"
	jhash (0xCF14D7F2)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x17AD8C9706BDD88A"
	hash "0x17AD8C9706BDD88A"
	jhash (0x24A97AF8)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x4A0C7C9BB10ABB36"
	hash "0x4A0C7C9BB10ABB36"
	jhash (0x44018EDB)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xFDD85225B2DEA55E"
	hash "0xFDD85225B2DEA55E"
	jhash (0xA4524B23)
	returns	"void"

native "0xFDEC055AB549E328"
	hash "0xFDEC055AB549E328"
	jhash (0xAFA1148B)
	returns	"void"

native "0x80FE4F3AB4E1B62A"
	hash "0x80FE4F3AB4E1B62A"
	jhash (0x3CD4307C)
	returns	"void"

native "0xBAE4F9B97CD43B30"
	hash "0xBAE4F9B97CD43B30"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x317EBA71D7543F52"
	hash "0x317EBA71D7543F52"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"void"

--[[!
<summary>
	Declares the entry type of a notification, for example "STRING".
</summary>
]]--
native "_SET_NOTIFICATION_TEXT_ENTRY"
	hash "0x202709F4C58A0424"
	jhash (0x574EE85C)
	arguments {
		charPtr "type",
	}
	returns	"void"

native "0x2B7E9A4EAAA93C89"
	hash "0x2B7E9A4EAAA93C89"
	jhash (0xED130FA1)
	arguments {
		charPtr "p0",

		int "p1",

		int "p2",

		int "p3",

		BOOL "p4",

		charPtr "picName1",

		charPtr "picName2",
	}
	returns	"int"

--[[!
<summary>
	picName1 &amp; picName2 must match. Possibilities: "CHAR_DEFAULT", "CHAR_FACEBOOK", "CHAR_SOCIAL_CLUB".
	flash is a bool for fading in.
	iconTypes:
	1 : Chat Box
	2 : Email
	3 : Add Friend Request
	4 : Nothing
	5 : Nothing
	6 : Nothing
	7 : Right Jumping Arrow
	8 : RP Icon
	9 : $ Icon

	"sender" is the very top header. This can be any old string.
	"subject" is the header under the sender.


</summary>
]]--
native "_SET_NOTIFICATION_MESSAGE"
	hash "0x1CCD9A37359072CF"
	jhash (0xE7E3C98B)
	arguments {
		charPtr "picName1",

		charPtr "picName2",

		BOOL "flash",

		int "iconType",

		charPtr "sender",

		charPtr "subject",
	}
	returns	"int"

native "0xC6F580E4C94926AC"
	hash "0xC6F580E4C94926AC"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		BOOL "p2",

		Any "p3",

		AnyPtr "p4",

		AnyPtr "p5",
	}
	returns	"Any"

--[[!
<summary>
	NOTE: 'duration' is a multiplier, so 1.0 is normal, 2.0 is twice as long (very slow), and 0.5 is half as long.
</summary>
]]--
native "0x1E6611149DB3DB6B"
	hash "0x1E6611149DB3DB6B"
	jhash (0x0EB382B7)
	arguments {
		charPtr "picName1",

		charPtr "picName2",

		BOOL "flash",

		int "iconType",

		charPtr "sender",

		charPtr "subject",

		float "duration",
	}
	returns	"int"

--[[!
<summary>
	picName1 &amp; picName2 must match. Possibilities: "CHAR_DEFAULT", "CHAR_FACEBOOK", "CHAR_SOCIAL_CLUB".
	flash is a bool for fading in.
	iconTypes:
	1 : Chat Box
	2 : Email
	3 : Add Friend Request
	4 : Nothing
	5 : Nothing
	6 : Nothing
	7 : Right Jumping Arrow
	8 : RP Icon
	9 : $ Icon

	"sender" is the very top header. This can be any old string.
	"subject" is the header under the sender.
	"duration" is a multiplier, so 1.0 is normal, 2.0 is twice as long (very slow), and 0.5 is half as long.
	"clanTag" shows a crew tag in the "sender" header, after the text. You need to use 3 underscores as padding. Maximum length of this field seems to be 7. (e.g. "MK" becomes "___MK", "ACE" becomes "___ACE", etc.)
</summary>
]]--
native "_SET_NOTIFICATION_MESSAGE_CLAN_TAG"
	hash "0x5CBF7BADE20DB93E"
	jhash (0x3E807FE3)
	arguments {
		charPtr "picName1",

		charPtr "picName2",

		BOOL "flash",

		int "iconType",

		charPtr "sender",

		charPtr "subject",

		float "duration",

		charPtr "clanTag",
	}
	returns	"int"

--[[!
<summary>
	picName1 &amp; picName2 must match. Possibilities: "CHAR_DEFAULT", "CHAR_FACEBOOK", "CHAR_SOCIAL_CLUB".
	flash is a bool for fading in.
	iconTypes:
	1 : Chat Box
	2 : Email
	3 : Add Friend Request
	4 : Nothing
	5 : Nothing
	6 : Nothing
	7 : Right Jumping Arrow
	8 : RP Icon
	9 : $ Icon

	"sender" is the very top header. This can be any old string.
	"subject" is the header under the sender.
	"duration" is a multiplier, so 1.0 is normal, 2.0 is twice as long (very slow), and 0.5 is half as long.
	"clanTag" shows a crew tag in the "sender" header, after the text. You need to use 3 underscores as padding. Maximum length of this field seems to be 7. (e.g. "MK" becomes "___MK", "ACE" becomes "___ACE", etc.)
	iconType2 is a mirror of iconType. It shows in the "subject" line, right under the original iconType.


</summary>
]]--
native "_SET_NOTIFICATION_MESSAGE_CLAN_TAG_2"
	hash "0x531B84E7DA981FB6"
	jhash (0xDEB491C8)
	arguments {
		charPtr "picName1",

		charPtr "picName2",

		BOOL "flash",

		int "iconType1",

		charPtr "sender",

		charPtr "subject",

		float "duration",

		charPtr "clanTag",

		int "iconType2",
	}
	returns	"int"

--[[!
<summary>
	Draws a notification above the map and returns the notifications handle

	Text should be set using _ADD_TEXT_COMPONENT_STRING.

	Color syntax:
	~r~ = Red
	~b~ = Blue
	~g~ = Green
	~y~ = Yellow
	~p~ = Purple
	~o~ = Orange
	~c~ = Grey)
	~m~ = Darker Grey
	~u~ = Black
	~n~ = New Line
	~s~ = Default White
	~h~ = Bold Text

	Special characters:
	¦ = Rockstar Verified Icon (U+00A6:Broken Bar - Alt+0166)
	÷ = Rockstar Icon (U+00F7:Division Sign - Alt+0247)
	∑ = Rockstar Icon 2 (U+2211:N-Ary Summation)

	(Source: http://qs.lc/mgw1u)
</summary>
]]--
native "_DRAW_NOTIFICATION"
	hash "0x2ED7843F8F801023"
	jhash (0x08F7AF78)
	arguments {
		BOOL "blink",

		BOOL "p1",
	}
	returns	"int"

native "_DRAW_NOTIFICATION_2"
	hash "0x44FA03975424A0EE"
	jhash (0x57B8D0D4)
	arguments {
		BOOL "blink",

		BOOL "p1",
	}
	returns	"int"

native "_DRAW_NOTIFICATION_3"
	hash "0x378E809BF61EC840"
	jhash (0x02BCAF9B)
	arguments {
		BOOL "blink",

		BOOL "p1",
	}
	returns	"int"

native "0xAA295B6F28BD587D"
	hash "0xAA295B6F28BD587D"
	jhash (0x02DED2B8)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",

		AnyPtr "p4",
	}
	returns	"Any"

native "0x97C9E4E7024A8F2C"
	hash "0x97C9E4E7024A8F2C"
	jhash (0xA9CCEF66)
	arguments {
		BOOL "p0",

		BOOL "p1",

		AnyPtr "p2",

		Any "p3",

		BOOL "p4",

		BOOL "p5",

		Any "p6",

		Any "p7",

		Any "p8",

		Any "p9",
	}
	returns	"Any"

native "0x137BC35589E34E1E"
	hash "0x137BC35589E34E1E"
	jhash (0x88B9B909)
	arguments {
		BOOL "p0",

		BOOL "p1",

		AnyPtr "p2",

		Any "p3",

		BOOL "p4",

		BOOL "p5",

		Any "p6",

		AnyPtr "p7",

		Any "p8",

		Any "p9",

		Any "p10",
	}
	returns	"Any"

native "0x33EE12743CCD6343"
	hash "0x33EE12743CCD6343"
	jhash (0xE05E7052)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "0xC8F3AAF93D0600BF"
	hash "0xC8F3AAF93D0600BF"
	jhash (0x4FA43BA4)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"Any"

native "0x7AE0589093A2E088"
	hash "0x7AE0589093A2E088"
	jhash (0x8C90D22F)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"Any"

native "_DRAW_NOTIFICATION_4"
	hash "0xF020C96915705B3A"
	jhash (0x8E319AB8)
	arguments {
		BOOL "blink",

		BOOL "p1",
	}
	returns	"int"

native "0x8EFCCF6EC66D85E4"
	hash "0x8EFCCF6EC66D85E4"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"Any"

native "0xB6871B0555B02996"
	hash "0xB6871B0555B02996"
	jhash (0x5E93FBFA)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",

		AnyPtr "p3",

		AnyPtr "p4",

		Any "p5",
	}
	returns	"Any"

--[[!
<summary>
	returns a notification handle, prints out a notification like below:
	type range: 0 
	if you set type to 1, image goes from 0 - 39 - Xbox you can add text to

	example: 
	UI::_0xD202B92CBF1D816F(1, 20, "Who you trynna get crazy with, ese? Don't you know I'm LOCO?!");
	- http://imgur.com/lGBPCz3



</summary>
]]--
native "0xD202B92CBF1D816F"
	hash "0xD202B92CBF1D816F"
	arguments {
		int "type",

		int "image",

		charPtr "text",
	}
	returns	"Any"

--[[!
<summary>
	returns a notification handle, prints out a notification like below:
	type range: 0 
	if you set type to 1, button accepts "~INPUT_SOMETHING~"

	example:
	UI::_0xDD6CB2CCE7C2735C(1, "~INPUT_TALK~", "Who you trynna get crazy with, ese? Don't you know I'm LOCO?!");
	- http://imgur.com/UPy0Ial



</summary>
]]--
native "0xDD6CB2CCE7C2735C"
	hash "0xDD6CB2CCE7C2735C"
	arguments {
		int "type",

		charPtr "button",

		charPtr "text",
	}
	returns	"Any"

--[[!
<summary>
	SET_TEXT_ENTRY_2("STRING")

	**NOT to be confused with 0x25FBB336DF1804CB 0x3E35563E**
</summary>
]]--
native "_SET_TEXT_ENTRY_2"
	hash "0xB87A37EEB7FAA67D"
	jhash (0xF42C43C7)
	arguments {
		charPtr "p0",
	}
	returns	"void"

native "_DRAW_SUBTITLE_TIMED"
	hash "0x9D77056A530643F6"
	jhash (0x38F82261)
	arguments {
		int "time",

		BOOL "p1",
	}
	returns	"void"

native "0x853648FD1063A213"
	hash "0x853648FD1063A213"
	jhash (0xDD524A11)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x8A9BA1AB3E237613"
	hash "0x8A9BA1AB3E237613"
	jhash (0x672EFB45)
	returns	"Any"

--[[!
<summary>
	The following were found in the decompiled script files:
	STRING, TWOSTRINGS, NUMBER, PERCENTAGE, FO_TWO_NUM, ESMINDOLLA, ESDOLLA, MTPHPER_XPNO, AHD_DIST, CMOD_STAT_0, CMOD_STAT_1, CMOD_STAT_2, CMOD_STAT_3, DFLT_MNU_OPT, F3A_TRAFDEST, ES_HELP_SOC3
</summary>
]]--
native "_SET_TEXT_ENTRY"
	hash "0x25FBB336DF1804CB"
	jhash (0x3E35563E)
	arguments {
		charPtr "text",
	}
	returns	"void"

--[[!
<summary>
	After applying the properties to the text (See UI::SET_TEXT_), this will draw the text in the applied position. Also 0.0f &lt; x, y &lt; 1.0f, percentage of the axis.
</summary>
]]--
native "_DRAW_TEXT"
	hash "0xCD015E5BB0D96A57"
	jhash (0x6F8350CE)
	arguments {
		float "x",

		float "y",
	}
	returns	"void"

--[[!
<summary>
	Used for setting text entry and then getting the text width via _GET_TEXT_SCREEN_WIDTH.

	Example:
	_SET_TEXT_ENTRY_FOR_WIDTH("NUMBER");
	ADD_TEXT_COMPONENT_FLOAT(69.420f, 2);
	FLOAT width = _GET_TEXT_SCREEN_WIDTH(1);

	// YOU CANNOT DRAW TEXT WITH THIS. USE _SET_TEXT_ENTRY FOR THAT
</summary>
]]--
native "_SET_TEXT_ENTRY_FOR_WIDTH"
	hash "0x54CE8AC98E120CAB"
	jhash (0x51E7A037)
	arguments {
		charPtr "text",
	}
	returns	"void"

--[[!
<summary>
	p0 is always (I think) 1.
	Use _SET_TEXT_ENTRY_FOR_WIDTH instead of _set_text_entry before adding text components for correct width.
</summary>
]]--
native "_GET_TEXT_SCREEN_WIDTH"
	hash "0x85F061DA64ED2F67"
	jhash (0xD12A643A)
	arguments {
		BOOL "p0",
	}
	returns	"float"

native "_SET_TEXT_GXT_ENTRY"
	hash "0x521FB041D93DD0E4"
	jhash (0x94B82066)
	arguments {
		charPtr "entry",
	}
	returns	"void"

native "0x9040DFB09BE75706"
	hash "0x9040DFB09BE75706"
	jhash (0xAA318785)
	arguments {
		float "p0",

		float "p1",
	}
	returns	"Any"

--[[!
<summary>
	Set the type of data that is being added to the text element, e.g (INTEGER,FLOAT,ITEM_STRING,STRING,SUBSTRING_TIME)
</summary>
]]--
native "_SET_TEXT_COMPONENT_FORMAT"
	hash "0x8509B634FBE7DA11"
	jhash (0xB245FC10)
	arguments {
		charPtr "inputType",
	}
	returns	"void"

--[[!
<summary>
	shape goes from -1 to 2 (may be more)

	Example:
	void textboxTop(char* text) {
	    UI::_SET_TEXT_COMPONENT_FORMAT("STRING");
	    //UI::_ADD_TEXT_COMPONENT_ITEM_STRING(text);
	    UI::_ADD_TEXT_COMPONENT_STRING(text);
	    UI::_0x238FFE5C7B0498A6(0, 0, 0, -1);
	}

	Image:
</summary>
]]--
native "_DISPLAY_HELP_TEXT_FROM_STRING_LABEL"
	hash "0x238FFE5C7B0498A6"
	jhash (0xB59B530D)
	arguments {
		Any "p0",

		BOOL "loop",

		BOOL "beep",

		int "shape",
	}
	returns	"void"

native "0x0A24DA3A41B718F5"
	hash "0x0A24DA3A41B718F5"
	jhash (0x00E20F2D)
	arguments {
		charPtr "p0",
	}
	returns	"void"

native "0x10BDDBFC529428DD"
	hash "0x10BDDBFC529428DD"
	jhash (0xF63A13EC)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	example:

	UI::BEGIN_TEXT_COMMAND_SET_BLIP_NAME("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING("Name");
	UI::END_TEXT_COMMAND_SET_BLIP_NAME(blip);
</summary>
]]--
native "BEGIN_TEXT_COMMAND_SET_BLIP_NAME"
	hash "0xF9113A30DE5C6670"
	jhash (0xF4C211F6)
	arguments {
		charPtr "gxtentry",
	}
	returns	"void"

native "END_TEXT_COMMAND_SET_BLIP_NAME"
	hash "0xBC38B49BCB83BC9B"
	jhash (0xE8E59820)
	arguments {
		Blip "blip",
	}
	returns	"void"

native "0x23D69E0465570028"
	hash "0x23D69E0465570028"
	jhash (0x0E103475)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xCFDBDF5AE59BA0F4"
	hash "0xCFDBDF5AE59BA0F4"
	jhash (0x2944A6C5)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xE124FA80A759019C"
	hash "0xE124FA80A759019C"
	jhash (0x550665AE)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xFCC75460ABA29378"
	hash "0xFCC75460ABA29378"
	jhash (0x67785AF2)
	returns	"void"

native "0x8F9EE5687F8EECCD"
	hash "0x8F9EE5687F8EECCD"
	jhash (0xBF855650)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xA86911979638106F"
	hash "0xA86911979638106F"
	jhash (0x6E7FDA1C)
	arguments {
		Any "p0",
	}
	returns	"void"

native "ADD_TEXT_COMPONENT_INTEGER"
	hash "0x03B504CF259931BC"
	jhash (0xFE272A57)
	arguments {
		int "value",
	}
	returns	"void"

native "ADD_TEXT_COMPONENT_FLOAT"
	hash "0xE7DCB5B874BCD96E"
	jhash (0x24D78013)
	arguments {
		float "value",

		int "decimalPlaces",
	}
	returns	"void"

native "_ADD_TEXT_COMPONENT_ITEM_STRING"
	hash "0xC63CD5D2920ACBE7"
	jhash (0xDCE05406)
	arguments {
		charPtr "labelName",
	}
	returns	"void"

--[[!
<summary>
	Takes a Hash of an input (example : INPUT_FRONTEND_RIGHT) and displays it's name it when you display your notification above the map.

	^^ I don't know how somebody figured it prints the name of the input, when this native is clearly used for displaying Street Names in the scripts. Can you give an example if it actually does print out the INPUTS?
</summary>
]]--
native "0x17299B63C7683A2B"
	hash "0x17299B63C7683A2B"
	jhash (0x150E03B6)
	arguments {
		Hash "inputName",
	}
	returns	"void"

native "0x80EAD8E2E1D5D52E"
	hash "0x80EAD8E2E1D5D52E"
	jhash (0x5DE98F0A)
	arguments {
		int "blipId",
	}
	returns	"void"

native "_ADD_TEXT_COMPONENT_STRING"
	hash "0x6C188BE134E074AA"
	jhash (0x27A244D8)
	arguments {
		charPtr "text",
	}
	returns	"void"

--[[!
<summary>
	Adds a timer (e.g. "00:00:00:000"). The appearance of the timer depends on the flags, which need more research.
</summary>
]]--
native "ADD_TEXT_COMPONENT_SUBSTRING_TIME"
	hash "0x1115F16B8AB9E8BF"
	jhash (0x135B3CD0)
	arguments {
		int "timestamp",

		int "flags",
	}
	returns	"void"

native "0x0E4C749FF9DE9CC4"
	hash "0x0E4C749FF9DE9CC4"
	jhash (0x12929BDF)
	arguments {
		int "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x761B77454205A61D"
	hash "0x761B77454205A61D"
	jhash (0x65E1D404)
	arguments {
		charPtr "p0",

		int "p1",
	}
	returns	"void"

--[[!
<summary>
	Alias for _ADD_TEXT_COMPONENT_STRING.
</summary>
]]--
native "_ADD_TEXT_COMPONENT_STRING2"
	hash "0x94CF4AC034C9C986"
	jhash (0xC736999E)
	arguments {
		charPtr "text",
	}
	returns	"Any"

--[[!
<summary>
	Alias for _ADD_TEXT_COMPONENT_STRING.
</summary>
]]--
native "_ADD_TEXT_COMPONENT_STRING3"
	hash "0x5F68520888E69014"
	jhash (0x0829A799)
	arguments {
		charPtr "text",
	}
	returns	"Any"

native "0x39BBF623FC803EAC"
	hash "0x39BBF623FC803EAC"
	jhash (0x6F1A1901)
	arguments {
		int "p0",
	}
	returns	"void"

--[[!
<summary>
	Returns a substring of a specified length starting at a specified position.

	Example:
	// Get "STRING" text from "MY_STRING"
	subStr = UI::_GET_TEXT_SUBSTRING("MY_STRING", 3, 6);
</summary>
]]--
native "_GET_TEXT_SUBSTRING"
	hash "0x169BD9382084C8C0"
	jhash (0x34A396EE)
	arguments {
		charPtr "text",

		int "position",

		int "length",
	}
	returns	"charPtr"

--[[!
<summary>
	Returns a substring of a specified length starting at a specified position. The result is guaranteed not to exceed the specified max length.

	NOTE: The 'maxLength' parameter might actually be the size of the buffer that is returned. More research is needed. -CL69

	Example:
	// Condensed example of how Rockstar uses this function
	strLen = UI::GET_LENGTH_OF_LITERAL_STRING(GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT());
	subStr = UI::_GET_TEXT_SUBSTRING_SAFE(GAMEPLAY::GET_ONSCREEN_KEYBOARD_RESULT(), 0, strLen, 63);

	--

	"fm_race_creator.ysc", line 85115:
	// parameters modified for clarity
	BOOL sub_8e5aa(char *text, int length) {
	    for (i = 0; i &lt;= (length - 2); i += 1) {
	        if (!GAMEPLAY::ARE_STRINGS_EQUAL(UI::_GET_TEXT_SUBSTRING_SAFE(text, i, i + 1, 1), " ")) {
	            return FALSE;
	        }
	    }
	    return TRUE;
	}
</summary>
]]--
native "_GET_TEXT_SUBSTRING_SAFE"
	hash "0xB2798643312205C5"
	jhash (0x0183A66C)
	arguments {
		charPtr "text",

		int "position",

		int "length",

		int "maxLength",
	}
	returns	"charPtr"

--[[!
<summary>
	Returns a substring that is between two specified positions. The length of the string will be calculated using (endPosition - startPosition).

	Example:
	// Get "STRING" text from "MY_STRING"
	subStr = UI::_GET_TEXT_SUBSTRING_SLICE("MY_STRING", 3, 9);
	// Overflows are possibly replaced with underscores (needs verification)
	subStr = UI::_GET_TEXT_SUBSTRING_SLICE("MY_STRING", 3, 10); // "STRING_"?
</summary>
]]--
native "_GET_TEXT_SUBSTRING_SLICE"
	hash "0xCE94AEBA5D82908A"
	jhash (0xFA6373BB)
	arguments {
		charPtr "text",

		int "startPosition",

		int "endPosition",
	}
	returns	"charPtr"

--[[!
<summary>
	Gets a string literal from a label name.
</summary>
]]--
native "_GET_LABEL_TEXT"
	hash "0x7B5280EBA9840C72"
	jhash (0x95C4B5AD)
	arguments {
		charPtr "labelName",
	}
	returns	"charPtr"

native "CLEAR_PRINTS"
	hash "0xCC33FA791322B9D9"
	jhash (0x216CB1C5)
	returns	"void"

native "CLEAR_BRIEF"
	hash "0x9D292F73ADBD9313"
	jhash (0x9F75A929)
	returns	"void"

native "CLEAR_ALL_HELP_MESSAGES"
	hash "0x6178F68A87A4D3A0"
	jhash (0x9E5D9198)
	returns	"void"

native "CLEAR_THIS_PRINT"
	hash "0xCF708001E1E536DD"
	jhash (0x06878327)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "CLEAR_SMALL_PRINTS"
	hash "0x2CEA2839313C09AC"
	jhash (0xA869A238)
	returns	"void"

native "DOES_TEXT_BLOCK_EXIST"
	hash "0x1C7302E725259789"
	jhash (0x96F74838)
	arguments {
		charPtr "gxt",
	}
	returns	"BOOL"

--[[!
<summary>
	Request a gxt into the passed slot.
</summary>
]]--
native "REQUEST_ADDITIONAL_TEXT"
	hash "0x71A78003C8E71424"
	jhash (0x9FA9175B)
	arguments {
		charPtr "gxt",

		int "slot",
	}
	returns	"void"

native "_REQUEST_ADDITIONAL_TEXT_2"
	hash "0x6009F9F1AE90D8A6"
	jhash (0xF4D27EBE)
	arguments {
		charPtr "gxt",

		int "slot",
	}
	returns	"void"

native "HAS_ADDITIONAL_TEXT_LOADED"
	hash "0x02245FE4BED318B8"
	jhash (0xB0E56045)
	arguments {
		Any "additionalText",
	}
	returns	"BOOL"

native "CLEAR_ADDITIONAL_TEXT"
	hash "0x2A179DF17CCF04CD"
	jhash (0x518141E0)
	arguments {
		Any "additionalText",

		BOOL "p1",
	}
	returns	"void"

native "IS_STREAMING_ADDITIONAL_TEXT"
	hash "0x8B6817B71B85EBF0"
	jhash (0xF079E4EB)
	arguments {
		Any "additionalText",
	}
	returns	"BOOL"

--[[!
<summary>
	Checks if the specified gxt has loaded into the passed slot.
</summary>
]]--
native "HAS_THIS_ADDITIONAL_TEXT_LOADED"
	hash "0xADBF060E2B30C5BC"
	jhash (0x80A52040)
	arguments {
		charPtr "gxt",

		int "slot",
	}
	returns	"BOOL"

native "IS_MESSAGE_BEING_DISPLAYED"
	hash "0x7984C03AA5CC2F41"
	jhash (0x6A77FE8D)
	returns	"BOOL"

--[[!
<summary>
	Checks if the passed gxt name exists in the game files.
</summary>
]]--
native "DOES_TEXT_LABEL_EXIST"
	hash "0xAC09CA973C564252"
	jhash (0x6ECAE560)
	arguments {
		charPtr "gxt",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns the string length of the string from the gxt string .
</summary>
]]--
native "GET_LENGTH_OF_STRING_WITH_THIS_TEXT_LABEL"
	hash "0x801BD273D3A23F74"
	jhash (0xA4CA7BE5)
	arguments {
		charPtr "gxt",
	}
	returns	"Any"

--[[!
<summary>
	Returns the length of the string passed (much like strlen).
</summary>
]]--
native "GET_LENGTH_OF_LITERAL_STRING"
	hash "0xF030907CCBB8A9FD"
	jhash (0x99379D55)
	arguments {
		charPtr "string",
	}
	returns	"int"

native "0x43E4111189E54F0E"
	hash "0x43E4111189E54F0E"
	jhash (0x7DBC0764)
	arguments {
		charPtr "p0",
	}
	returns	"int"

--[[!
<summary>
	This functions converts the hash of a street name into a readable string.

	For how to get the hashes, see PATHFIND::GET_STREET_NAME_AT_COORD.
</summary>
]]--
native "GET_STREET_NAME_FROM_HASH_KEY"
	hash "0xD0EF8A959B8A4CB9"
	jhash (0x1E8E310C)
	arguments {
		Hash "hash",
	}
	returns	"charPtr"

native "IS_HUD_PREFERENCE_SWITCHED_ON"
	hash "0x1930DFA731813EC4"
	jhash (0xC3BC1B4F)
	returns	"BOOL"

native "IS_RADAR_PREFERENCE_SWITCHED_ON"
	hash "0x9EB6522EA68F22FE"
	jhash (0x14AEAA28)
	returns	"BOOL"

native "IS_SUBTITLE_PREFERENCE_SWITCHED_ON"
	hash "0xAD6DACA4BA53E0A4"
	jhash (0x63BA19F5)
	returns	"BOOL"

--[[!
<summary>
	If Hud should be displayed
</summary>
]]--
native "DISPLAY_HUD"
	hash "0xA6294919E56FF02A"
	jhash (0xD10E4E31)
	arguments {
		BOOL "Toggle",
	}
	returns	"Any"

native "0x7669F9E39DC17063"
	hash "0x7669F9E39DC17063"
	jhash (0xC380AC85)
	returns	"void"

native "0x402F9ED62087E898"
	hash "0x402F9ED62087E898"
	jhash (0xC47AB1B0)
	returns	"void"

--[[!
<summary>
	If Minimap / Radar should be displayed.
</summary>
]]--
native "DISPLAY_RADAR"
	hash "0xA0EBB943C300E693"
	jhash (0x52816BD4)
	arguments {
		BOOL "Toggle",
	}
	returns	"Any"

native "IS_HUD_HIDDEN"
	hash "0xA86478C6958735C5"
	jhash (0x40BADA1D)
	returns	"BOOL"

native "IS_RADAR_HIDDEN"
	hash "0x157F93B036700462"
	jhash (0x1AB3B954)
	returns	"BOOL"

native "0xAF754F20EB5CD51A"
	hash "0xAF754F20EB5CD51A"
	returns	"Any"

--[[!
<summary>
	Enable / disable showing route for the Blip-object.
</summary>
]]--
native "SET_BLIP_ROUTE"
	hash "0x4F7D8A9BFB0B43E9"
	jhash (0x3E160C90)
	arguments {
		Blip "blip",

		BOOL "enabled",
	}
	returns	"void"

native "SET_BLIP_ROUTE_COLOUR"
	hash "0x837155CD2F63DA09"
	jhash (0xDDE7C65C)
	arguments {
		Blip "blip",

		int "colour",
	}
	returns	"void"

native "ADD_NEXT_MESSAGE_TO_PREVIOUS_BRIEFS"
	hash "0x60296AF4BA14ABC5"
	jhash (0xB58B25BD)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x57D760D55F54E071"
	hash "0x57D760D55F54E071"
	jhash (0x9854485F)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "RESPONDING_AS_TEMP"
	hash "0xBD12C5EEE184C337"
	jhash (0xDCA3F423)
	arguments {
		float "p0",
	}
	returns	"void"

--[[!
<summary>
	p0 = ranges from 0 to 200 
</summary>
]]--
native "SET_RADAR_ZOOM"
	hash "0x096EF57A0C999BBA"
	jhash (0x2A50D1A6)
	arguments {
		int "p0",
	}
	returns	"void"

native "0xF98E4B3E56AFC7B1"
	hash "0xF98E4B3E56AFC7B1"
	jhash (0x25EC28C0)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "_SET_RADAR_ZOOM_LEVEL_THIS_FRAME"
	hash "0xCB7CC0D58405AD41"
	jhash (0x09CF1CE5)
	arguments {
		float "zoomLevel",
	}
	returns	"void"

native "0xD2049635DEB9C375"
	hash "0xD2049635DEB9C375"
	jhash (0xE8D3A910)
	returns	"void"

--[[!
<summary>
	This is a sample of some of the colours.

	HUD_COLOUR_PURE_WHITE = 0;
	HUD_COLOUR_WHITE = 1;
	HUD_COLOUR_BLACK = 2;
	HUD_COLOUR_FREEMODE = 116;
	HUD_COLOUR_PAUSE_BG = 117;
	HUD_COLOUR_BLUE = 9;
	HUD_COLOUR_RED = 6;
	HUD_COLOUR_NORTH_BLUE = 134;


</summary>
]]--
native "GET_HUD_COLOUR"
	hash "0x7C9C91AB74A0360F"
	jhash (0x63F66A0B)
	arguments {
		int "hudColour",

		intPtr "r",

		intPtr "g",

		intPtr "b",

		intPtr "a",
	}
	returns	"void"

native "0xD68A5FF8A3A89874"
	hash "0xD68A5FF8A3A89874"
	jhash (0x0E41E45C)
	arguments {
		int "r",

		int "g",

		int "b",

		int "a",
	}
	returns	"void"

native "0x16A304E6CB2BFAB9"
	hash "0x16A304E6CB2BFAB9"
	jhash (0x6BE3ACA8)
	arguments {
		int "r",

		int "g",

		int "b",

		int "a",
	}
	returns	"void"

native "0x1CCC708F0F850613"
	hash "0x1CCC708F0F850613"
	jhash (0x3B216749)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xF314CF4F0211894E"
	hash "0xF314CF4F0211894E"
	jhash (0xF6E7E92B)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "FLASH_ABILITY_BAR"
	hash "0x02CFBA0C9E9275CE"
	jhash (0x3648960D)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_ABILITY_BAR_VALUE"
	hash "0x9969599CCFF5D85E"
	jhash (0x24E53FD8)
	arguments {
		float "p0",

		float "p1",
	}
	returns	"void"

native "FLASH_WANTED_DISPLAY"
	hash "0xA18AFB39081B6A1F"
	jhash (0x629F866B)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "0xBA8D65C1C65702E5"
	hash "0xBA8D65C1C65702E5"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xDB88A37483346780"
	hash "0xDB88A37483346780"
	jhash (0x3330175B)
	arguments {
		float "size",

		Any "p1",
	}
	returns	"float"

--[[!
<summary>
	p0 fundamentally has no purpose. Possibly with other natives it could serve a purpose?
	size ranges from 0 to 3 or 4 I believe.
</summary>
]]--
native "SET_TEXT_SCALE"
	hash "0x07C837F9A01C34C9"
	jhash (0xB6E15B23)
	arguments {
		float "p0",

		float "size",
	}
	returns	"void"

native "SET_TEXT_COLOUR"
	hash "0xBE6B23FFA53FB442"
	jhash (0xE54DD2C8)
	arguments {
		int "red",

		int "green",

		int "blue",

		int "alpha",
	}
	returns	"void"

native "SET_TEXT_CENTRE"
	hash "0xC02F4DBFB51D988B"
	jhash (0xE26D39A1)
	arguments {
		BOOL "align",
	}
	returns	"void"

native "SET_TEXT_RIGHT_JUSTIFY"
	hash "0x6B3C4650BC8BEE47"
	jhash (0x45B60520)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	used with p0 = 0, 1, 2

	example: 

	UI::_4E096588B13FFECA(2);

	^Tested that. 
	And it seems 2 does the same as UI::SET_TEXT_RIGHT_JUSTIFY
	0 seems to be the X coordinates provided to the draw_text.
	1 seems to be a minor increase on the X coord of draw_text

	~Sod
</summary>
]]--
native "0x4E096588B13FFECA"
	hash "0x4E096588B13FFECA"
	jhash (0x68CDFA60)
	arguments {
		int "p0",
	}
	returns	"void"

--[[!
<summary>
	It sets the text in a specified box and wraps the text if it exceeds the boundries. Both values are for X axis. Useful when positioning text set to center or aligned to the right.

	start - left boundry on screen position (0.0 - 1.0)
	end - right boundry on screen position (0.0 - 1.0)
</summary>
]]--
native "SET_TEXT_WRAP"
	hash "0x63145D9C883A1A70"
	jhash (0x6F60AB54)
	arguments {
		float "start",

		float "end",
	}
	returns	"void"

native "SET_TEXT_LEADING"
	hash "0xA50ABC31E3CDFAFF"
	jhash (0x98CE21D4)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_TEXT_PROPORTIONAL"
	hash "0x038C1F517D7FDCF8"
	jhash (0xF49D8A08)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_TEXT_FONT"
	hash "0x66E0276CC5F6B9DA"
	jhash (0x80BC530D)
	arguments {
		int "p0",
	}
	returns	"void"

native "SET_TEXT_DROP_SHADOW"
	hash "0x1CA3E9EAC9D93E5E"
	jhash (0xE2A11511)
	returns	"void"

--[[!
<summary>
	distance - shadow distance in pixels, both horizontal and vertical
	r, g, b, a 
</summary>
]]--
native "SET_TEXT_DROPSHADOW"
	hash "0x465C84BC39F1C351"
	jhash (0xE6587517)
	arguments {
		int "distance",

		int "r",

		int "g",

		int "b",

		int "a",
	}
	returns	"void"

native "SET_TEXT_OUTLINE"
	hash "0x2513DFB0FB8400FE"
	jhash (0xC753412F)
	returns	"void"

native "SET_TEXT_EDGE"
	hash "0x441603240D202FA6"
	jhash (0x3F1A5DAB)
	arguments {
		int "p0",

		int "r",

		int "g",

		int "b",

		int "a",
	}
	returns	"void"

native "SET_TEXT_RENDER_ID"
	hash "0x5F15302936E07111"
	jhash (0xC5C3B7F3)
	arguments {
		Any "p0",
	}
	returns	"void"

native "GET_DEFAULT_SCRIPT_RENDERTARGET_RENDER_ID"
	hash "0x52F0982D7FD156B6"
	jhash (0x8188935F)
	returns	"Any"

native "REGISTER_NAMED_RENDERTARGET"
	hash "0x57D9C12635E25CE3"
	jhash (0xFAE5D6F0)
	arguments {
		AnyPtr "p0",

		BOOL "p1",
	}
	returns	"BOOL"

native "IS_NAMED_RENDERTARGET_REGISTERED"
	hash "0x78DCDC15C9F116B4"
	jhash (0x284057F5)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "RELEASE_NAMED_RENDERTARGET"
	hash "0xE9F6FFE837354DD4"
	jhash (0xD3F6C892)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "LINK_NAMED_RENDERTARGET"
	hash "0xF6C09E276AEB3F2D"
	jhash (0x6844C4B9)
	arguments {
		Any "p0",
	}
	returns	"void"

native "GET_NAMED_RENDERTARGET_RENDER_ID"
	hash "0x1A6478B61C6BDC3B"
	jhash (0xF9D7A401)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "IS_NAMED_RENDERTARGET_LINKED"
	hash "0x113750538FA31298"
	jhash (0x8B52601F)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "CLEAR_HELP"
	hash "0x8DFCED7A656F8802"
	jhash (0xE6D85741)
	arguments {
		BOOL "Enable",
	}
	returns	"Any"

native "IS_HELP_MESSAGE_ON_SCREEN"
	hash "0xDAD37F45428801AE"
	jhash (0x4B3C9CA9)
	returns	"BOOL"

--[[!
<summary>
	example

	if (UI::IS_HELP_MESSAGE_BEING_DISPLAYED()&amp;&amp;(!UI::_214CD562A939246A())) {
	        return 0;
	}
</summary>
]]--
native "0x214CD562A939246A"
	hash "0x214CD562A939246A"
	jhash (0x812CBE0E)
	returns	"BOOL"

native "IS_HELP_MESSAGE_BEING_DISPLAYED"
	hash "0x4D79439A6B55AC67"
	jhash (0xA65F262A)
	returns	"BOOL"

native "IS_HELP_MESSAGE_FADING_OUT"
	hash "0x327EDEEEAC55C369"
	jhash (0x3E50AE92)
	returns	"BOOL"

--[[!
<summary>
	example:

	if (!((v_7)==UI::_4A9923385BDB9DAD())) {
	        UI::SET_BLIP_SPRITE((v_6), (v_7));
	    }
</summary>
]]--
native "0x4A9923385BDB9DAD"
	hash "0x4A9923385BDB9DAD"
	jhash (0x87871CE0)
	returns	"int"

native "_GET_BLIP_INFO_ID_ITERATOR"
	hash "0x186E5D252FA50E7D"
	jhash (0xB9827942)
	returns	"Any"

native "GET_NUMBER_OF_ACTIVE_BLIPS"
	hash "0x9A3FF3DE163034E8"
	jhash (0x144020FA)
	returns	"Any"

native "GET_NEXT_BLIP_INFO_ID"
	hash "0x14F96AA50D6FBEA7"
	jhash (0x9356E92F)
	arguments {
		Blip "blip",
	}
	returns	"Blip"

native "GET_FIRST_BLIP_INFO_ID"
	hash "0x1BEDE233E6CD2A1F"
	jhash (0x64C0273D)
	arguments {
		Blip "blip",
	}
	returns	"Blip"

native "GET_BLIP_INFO_ID_COORD"
	hash "0xFA7C7F0AADF25D09"
	jhash (0xB7374A66)
	arguments {
		Any "p0",
	}
	returns	"int"

native "GET_BLIP_INFO_ID_DISPLAY"
	hash "0x1E314167F701DC3B"
	jhash (0xD0FC19F4)
	arguments {
		Blip "blip",
	}
	returns	"Any"

native "GET_BLIP_INFO_ID_TYPE"
	hash "0xBE9B0959FFD0779B"
	jhash (0x501D7B4E)
	arguments {
		Blip "blip",
	}
	returns	"Any"

--[[!
<summary>

</summary>
]]--
native "GET_BLIP_INFO_ID_ENTITY_INDEX"
	hash "0x4BA4E2553AFEDC2C"
	jhash (0xA068C40B)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_BLIP_INFO_ID_PICKUP_INDEX"
	hash "0x9B6786E4C03DD382"
	jhash (0x86913D37)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	Returns the Blip Object of given Entity.
</summary>
]]--
native "GET_BLIP_FROM_ENTITY"
	hash "0xBC8DBDCA2436F7E8"
	jhash (0x005A2A47)
	arguments {
		Entity "p0",
	}
	returns	"Blip"

native "ADD_BLIP_FOR_RADIUS"
	hash "0x46818D79B1F7499A"
	jhash (0x4626756C)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",
	}
	returns	"Blip"

--[[!
<summary>
	Returns red ( default ) blip attached to entity.
</summary>
]]--
native "ADD_BLIP_FOR_ENTITY"
	hash "0x5CDE92C702A8FCE7"
	jhash (0x30822554)
	arguments {
		Entity "entity",
	}
	returns	"int"

native "ADD_BLIP_FOR_PICKUP"
	hash "0xBE339365C863BD36"
	jhash (0x16693C3A)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	Creates an orange ( default ) Blip-object. Returns a Blip-object which can then be modified.
</summary>
]]--
native "ADD_BLIP_FOR_COORD"
	hash "0x5A039BB0BCA604B6"
	jhash (0xC6F43D0E)
	arguments {
		float "x",

		float "y",

		float "z",
	}
	returns	"Any"

native "0x72DD432F3CDFC0EE"
	hash "0x72DD432F3CDFC0EE"
	jhash (0xBF25E7B2)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		int "p4",
	}
	returns	"void"

native "0x60734CC207C9833C"
	hash "0x60734CC207C9833C"
	jhash (0xE7E1E32B)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_BLIP_COORDS"
	hash "0xAE2AF67E9D9AF65D"
	jhash (0x680A34D4)
	arguments {
		Player "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "GET_BLIP_COORDS"
	hash "0x586AFE3FF72D996E"
	jhash (0xEF6FF47B)
	arguments {
		Any "p0",
	}
	returns	"int"

--[[!
<summary>
	Takes a blip object and adds a sprite to it on the map.
</summary>
]]--
native "SET_BLIP_SPRITE"
	hash "0xDF735600A4696DAF"
	jhash (0x8DBBB0B9)
	arguments {
		int "blip",

		int "spriteId",
	}
	returns	"void"

native "GET_BLIP_SPRITE"
	hash "0x1FC877464A04FC4F"
	jhash (0x72FF2E73)
	arguments {
		int "blip",
	}
	returns	"int"

native "SET_BLIP_NAME_FROM_TEXT_FILE"
	hash "0xEAA0FFE120D92784"
	jhash (0xAC8A5461)
	arguments {
		int "blip",

		charPtr "blipname",
	}
	returns	"void"

native "SET_BLIP_NAME_TO_PLAYER_NAME"
	hash "0x127DE7B20C60A6A3"
	jhash (0x03A0B8F9)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	Sets alpha-channel for blip color.

	Example:

	Blip blip = UI::ADD_BLIP_FOR_ENTITY(entity);
	UI::SET_BLIP_COLOUR(blip , 3);
	UI::SET_BLIP_ALPHA(blip , 64);

</summary>
]]--
native "SET_BLIP_ALPHA"
	hash "0x45FF974EEE1C8734"
	jhash (0xA791FCCD)
	arguments {
		Blip "blip",

		int "alpha",
	}
	returns	"void"

native "GET_BLIP_ALPHA"
	hash "0x970F608F0EE6C885"
	jhash (0x297AF6C8)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "SET_BLIP_FADE"
	hash "0x2AEE8F8390D2298C"
	jhash (0xA5999031)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

--[[!
<summary>
	After some testing, looks like you need to use UI:CEIL() on the rotation (vehicle/ped heading) before using it there.
</summary>
]]--
native "SET_BLIP_ROTATION"
	hash "0xF87683CDF73C3F6E"
	jhash (0x6B8F44FE)
	arguments {
		Blip "blip",

		int "rotation",
	}
	returns	"void"

--[[!
<summary>
	Adds up after viewing multiple R* scripts. I believe that the duration is in miliseconds.


</summary>
]]--
native "SET_BLIP_FLASH_TIMER"
	hash "0xD3CD6FD297AE87CC"
	jhash (0x8D5DF611)
	arguments {
		int "blip",

		int "duration",
	}
	returns	"void"

native "SET_BLIP_FLASH_INTERVAL"
	hash "0xAA51DB313C010A7E"
	jhash (0xEAF67377)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	Color:

	0: white
	1: red
	2: green
	3: blue
	17: orange
	19: purple
	20: grey
	21: brown
	23: pink
	25: dark green
	27: dark purple
	29: dark blue
	Default (Function not used): yellow

	Those are not the only ones. i.e: 17 is Trevor's orange.
</summary>
]]--
native "SET_BLIP_COLOUR"
	hash "0x03D7FB09E75D6B7E"
	jhash (0xBB3C5A41)
	arguments {
		int "blip",

		int "color",
	}
	returns	"void"

native "SET_BLIP_SECONDARY_COLOUR"
	hash "0x14892474891E09EB"
	jhash (0xC6384D32)
	arguments {
		int "blip",

		float "r",

		float "g",

		float "b",
	}
	returns	"void"

native "GET_BLIP_COLOUR"
	hash "0xDF729E8D20CF7327"
	jhash (0xDD6A1E54)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_BLIP_HUD_COLOUR"
	hash "0x729B5F1EFBC0AAEE"
	jhash (0xE88B4BC2)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "IS_BLIP_SHORT_RANGE"
	hash "0xDA5F8727EB75B926"
	jhash (0x1226765A)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "IS_BLIP_ON_MINIMAP"
	hash "0xE41CA53051197A27"
	jhash (0x258CBA3A)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xDD2238F57B977751"
	hash "0xDD2238F57B977751"
	jhash (0x3E47F357)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x54318C915D27E4CE"
	hash "0x54318C915D27E4CE"
	jhash (0x43996428)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_BLIP_HIGH_DETAIL"
	hash "0xE2590BC29220CEBB"
	jhash (0xD5842BFF)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_BLIP_AS_MISSION_CREATOR_BLIP"
	hash "0x24AC0137444F9FD5"
	jhash (0x802FB686)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "IS_MISSION_CREATOR_BLIP"
	hash "0x26F49BF3381D933D"
	jhash (0x24ACC4E9)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "DISABLE_BLIP_NAME_FOR_VAR"
	hash "0x5C90988E7C8E1AF4"
	jhash (0xFFD7476C)
	returns	"Any"

native "0x4167EFE0527D706E"
	hash "0x4167EFE0527D706E"
	jhash (0xC5EB849A)
	returns	"Any"

native "0xF1A6C18B35BCADE6"
	hash "0xF1A6C18B35BCADE6"
	jhash (0xA2CAAB4F)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_BLIP_FLASHES"
	hash "0xB14552383D39CE3E"
	jhash (0xC0047F15)
	arguments {
		int "blip",

		BOOL "toggle",
	}
	returns	"void"

native "SET_BLIP_FLASHES_ALTERNATE"
	hash "0x2E8D9498C56DD0D1"
	jhash (0x1A81202B)
	arguments {
		int "blip",

		BOOL "p1",
	}
	returns	"void"

native "IS_BLIP_FLASHING"
	hash "0xA5E41FD83AD6CEF0"
	jhash (0x52E111D7)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SET_BLIP_AS_SHORT_RANGE"
	hash "0xBE8BE4FE60E27B72"
	jhash (0x5C67725E)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_BLIP_SCALE"
	hash "0xD38744167B2FA257"
	jhash (0x1E6EC434)
	arguments {
		int "blip",

		float "scale",
	}
	returns	"void"

native "SET_BLIP_PRIORITY"
	hash "0xAE9FC9EF6A9FAC79"
	jhash (0xCE87DA6F)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	p1:
	3 = Shows on Main map but not Radar
</summary>
]]--
native "SET_BLIP_DISPLAY"
	hash "0x9029B2F3DA924928"
	jhash (0x2B521F91)
	arguments {
		Blip "blip",

		int "p1",
	}
	returns	"void"

--[[!
<summary>
	int index:

	1 = No Text on blip or Distance
	2 = Text on blip
	3 = No text, just distance
	4+ No Text on blip or distance
</summary>
]]--
native "SET_BLIP_CATEGORY"
	hash "0x234CDD44D996FD9A"
	jhash (0xEF72F533)
	arguments {
		Blip "blip",

		int "index",
	}
	returns	"void"

--[[!
<summary>
	In the C++ SDK, this seems not to work-- the blip isn't removed immediately. I use it for saving cars.

	E.g.:

	Ped pped = PLAYER::PLAYER_PED_ID();
	Vehicle v = PED::GET_VEHICLE_PED_IS_USING(pped);
	Blip b = UI::ADD_BLIP_FOR_ENTITY(v);

	works fine.
	But later attempting to delete it with:

	Blip b = UI::GET_BLIP_FROM_ENTITY(v);
	if (UI::DOES_BLIP_EXIST(b)) UI::REMOVE_BLIP(&amp;b);

	doesn't work. And yes, doesn't work without the DOES_BLIP_EXIST check either. Also, if you attach multiple blips to the same thing (say, a vehicle), and that thing disappears, the blips randomly attach to other things (in my case, a vehicle).

	Thus for me, UI::REMOVE_BLIP(&amp;b) only works if there's one blip, (in my case) the vehicle is marked as no longer needed, you drive away from it and it eventually despawns, AND there is only one blip attached to it. I never intentionally attach multiple blips but if the user saves the car, this adds a blip. Then if they delete it, it is supposed to remove the blip, but it doesn't. Then they can immediately save it again, causing another blip to re-appear.
	-------------

	Passing the address of the variable instead of the value works for me.
	e.g.
	int blip = UI::ADD_BLIP_FOR_ENTITY(ped);
	UI::REMOVE_BLIP(&amp;blip);


	Remove blip will currently crash your game, just artificially remove the blip by setting the sprite to a id that is 'invisible'.

	--
	It crashes my game.
</summary>
]]--
native "REMOVE_BLIP"
	hash "0x86A652570E5F25DD"
	jhash (0xD8C3C1CD)
	arguments {
		BlipPtr "blip",
	}
	returns	"void"

--[[!
<summary>
	false for enemy
	true for friendly
</summary>
]]--
native "SET_BLIP_AS_FRIENDLY"
	hash "0x6F6F290102C02AB4"
	jhash (0xF290CFD8)
	arguments {
		int "blip",

		BOOL "toggle",
	}
	returns	"void"

native "PULSE_BLIP"
	hash "0x742D6FD43115AF73"
	jhash (0x44253855)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SHOW_NUMBER_ON_BLIP"
	hash "0xA3C0B359DCB848B6"
	jhash (0x7BFC66C6)
	arguments {
		int "blip",

		int "number",
	}
	returns	"void"

native "HIDE_NUMBER_ON_BLIP"
	hash "0x532CFF637EF80148"
	jhash (0x0B6D610D)
	arguments {
		int "blip",
	}
	returns	"void"

native "0x75A16C3DA34F1245"
	hash "0x75A16C3DA34F1245"
	jhash (0x1D99F676)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Adds a green checkmark on top of a blip.
</summary>
]]--
native "0x74513EA3E505181E"
	hash "0x74513EA3E505181E"
	jhash (0x3DCF0092)
	arguments {
		int "blip",

		BOOL "toggle",
	}
	returns	"void"

native "0x5FBCA48327B914DF"
	hash "0x5FBCA48327B914DF"
	jhash (0xD1C3D71B)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Highlights a blip by a cyan circle.
</summary>
]]--
native "0xB81656BC81FE24D1"
	hash "0xB81656BC81FE24D1"
	jhash (0x8DE82C15)
	arguments {
		int "blip",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Highlights a blip by a half cyan circle.
</summary>
]]--
native "0x23C3EB807312F01A"
	hash "0x23C3EB807312F01A"
	jhash (0x4C8F02B4)
	arguments {
		int "blip",

		BOOL "toggle",
	}
	returns	"void"

native "0xDCFB5D4DB8BF367E"
	hash "0xDCFB5D4DB8BF367E"
	jhash (0xABBE1E45)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xC4278F70131BAA6D"
	hash "0xC4278F70131BAA6D"
	jhash (0x6AA6A1CC)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Makes a blip go small when off the minimap.
</summary>
]]--
native "0x2B6D467DAB714E8D"
	hash "0x2B6D467DAB714E8D"
	jhash (0xC575F0BC)
	arguments {
		int "blip",

		BOOL "toggle",
	}
	returns	"void"

native "0x25615540D894B814"
	hash "0x25615540D894B814"
	jhash (0x40E25DB8)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "DOES_BLIP_EXIST"
	hash "0xA6DB27D19ECBB7DA"
	jhash (0xAE92DD96)
	arguments {
		Blip "blip",
	}
	returns	"BOOL"

--[[!
<summary>
	This native removes the current waypoint from the map.

	Example:
	C#:
	Function.Call(Hash.SET_WAYPOINT_OFF);

	C++:
	UI::SET_WAYPOINT_OFF();

	~ISOFX
</summary>
]]--
native "SET_WAYPOINT_OFF"
	hash "0xA7E4E2D361C2627F"
	jhash (0xB3496E1B)
	returns	"void"

native "0xD8E694757BCEA8E9"
	hash "0xD8E694757BCEA8E9"
	jhash (0x62BABF2C)
	returns	"void"

native "REFRESH_WAYPOINT"
	hash "0x81FA173F170560D1"
	jhash (0xB395D753)
	returns	"void"

native "IS_WAYPOINT_ACTIVE"
	hash "0x1DD1F58F493F1DA5"
	jhash (0x5E4DF47B)
	returns	"BOOL"

native "SET_NEW_WAYPOINT"
	hash "0xFE43368D2AA4F2FC"
	jhash (0x8444E1F0)
	arguments {
		float "x",

		float "y",
	}
	returns	"void"

native "SET_BLIP_BRIGHT"
	hash "0xB203913733F27884"
	jhash (0x72BEE6DF)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "SET_BLIP_SHOW_CONE"
	hash "0x13127EC3665E8EE1"
	jhash (0xFF545AD8)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xC594B315EDF2D4AF"
	hash "0xC594B315EDF2D4AF"
	jhash (0x41B0D022)
	arguments {
		Ped "ped",
	}
	returns	"void"

--[[!
<summary>
	Strange but every example in decompiled scripts have 3 parameters instead of 2.
</summary>
]]--
native "SET_MINIMAP_COMPONENT"
	hash "0x75A9A10948D1DEA6"
	jhash (0x419DCDC4)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

--[[!
<summary>
	Something with Social Club or online.
</summary>
]]--
native "0x60E892BA4F5BDCA4"
	hash "0x60E892BA4F5BDCA4"
	returns	"void"

native "GET_MAIN_PLAYER_BLIP_ID"
	hash "0xDCD4EC3F419D02FA"
	jhash (0xAB93F020)
	returns	"Any"

native "0x41350B4FC28E3941"
	hash "0x41350B4FC28E3941"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "HIDE_LOADING_ON_FADE_THIS_FRAME"
	hash "0x4B0311D3CDC4648F"
	jhash (0x35087963)
	returns	"void"

native "SET_RADAR_AS_INTERIOR_THIS_FRAME"
	hash "0x59E727A1C9D3E31A"
	jhash (0x6F2626E1)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "SET_RADAR_AS_EXTERIOR_THIS_FRAME"
	hash "0xE81B7D2A3DAB2D81"
	jhash (0x39ABB10E)
	returns	"void"

--[[!
<summary>
	Sets the position of the arrow icon representing the player on both the minimap and world map.
</summary>
]]--
native "_SET_PLAYER_BLIP_POSITION_THIS_FRAME"
	hash "0x77E2DD177910E1CF"
	jhash (0x54E75C7D)
	arguments {
		float "x",

		float "y",
	}
	returns	"void"

native "0x9049FE339D5F6F6F"
	hash "0x9049FE339D5F6F6F"
	jhash (0x199DED14)
	returns	"Any"

native "_DISABLE_RADAR_THIS_FRAME"
	hash "0x5FBAE526203990C9"
	jhash (0x1A4318F7)
	returns	"void"

native "0x20FE7FDFEEAD38C0"
	hash "0x20FE7FDFEEAD38C0"
	jhash (0xCE36E3FE)
	returns	"void"

--[[!
<summary>
	When calling this, the current frame will have the players "arrow icon" be focused on the dead center of the radar. 
</summary>
]]--
native "_CENTER_PLAYER_ON_RADAR_THIS_FRAME"
	hash "0x6D14BFDC33B34F55"
	jhash (0x334EFD46)
	returns	"void"

native "SET_WIDESCREEN_FORMAT"
	hash "0xC3B07BA00A83B0F1"
	jhash (0xF016E08F)
	arguments {
		Any "p0",
	}
	returns	"void"

native "DISPLAY_AREA_NAME"
	hash "0x276B6CE369C33678"
	jhash (0x489FDD41)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Displays your cash.

	Example:
	DISPLAY_CASH(1);

	~ISOFX
</summary>
]]--
native "DISPLAY_CASH"
	hash "0x96DEC8D5430208B7"
	jhash (0x0049DF83)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Related to displaying cash on the HUD
	Always called before UI::_SET_SINGLEPLAYER_HUD_CASH in decompiled scripts
</summary>
]]--
native "0x170F541E1CADD1DE"
	hash "0x170F541E1CADD1DE"
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Display player cash in upper right corner with the given increment or deduction. This appears when the player looses or acquires cash.

	ex: https://i.gyazo.com/e96df9e9ce1c0a31da8c32a2da8e8ba4.png
</summary>
]]--
native "_SET_SINGLEPLAYER_HUD_CASH"
	hash "0x0772DF77852C2E30"
	arguments {
		int "pocketcash",

		int "bankcash",
	}
	returns	"void"

native "DISPLAY_AMMO_THIS_FRAME"
	hash "0xA5E78BA2B1331C55"
	jhash (0x60693CEE)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Displays the crosshair for this frame.
</summary>
]]--
native "DISPLAY_SNIPER_SCOPE_THIS_FRAME"
	hash "0x73115226F4814E62"
	jhash (0xBC6C73CB)
	returns	"void"

native "HIDE_HUD_AND_RADAR_THIS_FRAME"
	hash "0x719FF505F097FD20"
	jhash (0xB75D4AD2)
	returns	"void"

native "0xE67C6DFD386EA5E7"
	hash "0xE67C6DFD386EA5E7"
	jhash (0x5476B9FD)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xC2D15BEF167E27BC"
	hash "0xC2D15BEF167E27BC"
	jhash (0xF4F3C796)
	returns	"void"

native "0x95CF81BD06EE1887"
	hash "0x95CF81BD06EE1887"
	jhash (0x7BFFE82F)
	returns	"void"

native "SET_MULTIPLAYER_BANK_CASH"
	hash "0xDD21B55DF695CD0A"
	jhash (0x2C842D03)
	returns	"void"

native "REMOVE_MULTIPLAYER_BANK_CASH"
	hash "0xC7C6789AA1CFEDD0"
	jhash (0x728B4EF4)
	returns	"void"

native "SET_MULTIPLAYER_HUD_CASH"
	hash "0xFD1D220394BCB824"
	jhash (0xA8DB435E)
	arguments {
		int "p0",

		int "p1",
	}
	returns	"void"

native "REMOVE_MULTIPLAYER_HUD_CASH"
	hash "0x968F270E39141ECA"
	jhash (0x07BF4A7D)
	returns	"void"

native "HIDE_HELP_TEXT_THIS_FRAME"
	hash "0xD46923FC481CA285"
	jhash (0xF3807BED)
	returns	"void"

--[[!
<summary>
	The messages are localized strings.
	Examples:
	"No_bus_money"
	"Enter_bus"
	"Tour_help"
	"LETTERS_HELP2"
	"Dummy"

	**The bool appears to always be false (if it even is a bool, as it's represented by a zero)**
	--------
	p1 doesn't seem to make a difference, regardless of the state it's in. 
</summary>
]]--
native "DISPLAY_HELP_TEXT_THIS_FRAME"
	hash "0x960C9FF8F616E41C"
	jhash (0x18E3360A)
	arguments {
		charPtr "message",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Forces the weapons wheel to appear on screen.
</summary>
]]--
native "0xEB354E5376BC81A7"
	hash "0xEB354E5376BC81A7"
	jhash (0x1EFFB02A)
	arguments {
		BOOL "forcedShow",
	}
	returns	"Any"

native "0x0AFC4AF510774B47"
	hash "0x0AFC4AF510774B47"
	jhash (0xB26FED2B)
	returns	"void"

native "0xA48931185F0536FE"
	hash "0xA48931185F0536FE"
	jhash (0x22E9F555)
	returns	"Hash"

native "0x72C1056D678BB7D8"
	hash "0x72C1056D678BB7D8"
	jhash (0x83B608A0)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xA13E93403F26C812"
	hash "0xA13E93403F26C812"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x14C9FDCC41F81F63"
	hash "0x14C9FDCC41F81F63"
	jhash (0xE70D1F43)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_GPS_FLAGS"
	hash "0x5B440763A4C8D15B"
	jhash (0x60539BAB)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "CLEAR_GPS_FLAGS"
	hash "0x21986729D6A3A830"
	jhash (0x056AFCE6)
	returns	"void"

native "0x1EAC5F91BCBC5073"
	hash "0x1EAC5F91BCBC5073"
	jhash (0xFB9BABF5)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "CLEAR_GPS_RACE_TRACK"
	hash "0x7AA5B4CE533C858B"
	jhash (0x40C59829)
	returns	"void"

native "0xDB34E8D56FC13B08"
	hash "0xDB34E8D56FC13B08"
	jhash (0x7F93799B)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x311438A071DD9B1A"
	hash "0x311438A071DD9B1A"
	jhash (0xEEBDFE55)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0x900086F371220B6F"
	hash "0x900086F371220B6F"
	jhash (0xDA0AF00E)
	arguments {
		BOOL "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0xE6DE0561D9232A64"
	hash "0xE6DE0561D9232A64"
	jhash (0xCF2E3E24)
	returns	"void"

native "0x3D3D15AF7BCAAF83"
	hash "0x3D3D15AF7BCAAF83"
	jhash (0xC3DCBEDB)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xA905192A6781C41B"
	hash "0xA905192A6781C41B"
	jhash (0xFE485135)
	arguments {
		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "0x3DDA37128DD1ACA8"
	hash "0x3DDA37128DD1ACA8"
	jhash (0xE87CBE4C)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x67EEDEA1B9BAFD94"
	hash "0x67EEDEA1B9BAFD94"
	jhash (0x0D9969E4)
	returns	"void"

native "CLEAR_GPS_PLAYER_WAYPOINT"
	hash "0xFF4FB7C8CDFA3DA7"
	jhash (0x0B9C7FC2)
	returns	"void"

native "SET_GPS_FLASHES"
	hash "0x320D0E0D936A0E9B"
	jhash (0xE991F733)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x7B21E0BB01E8224A"
	hash "0x7B21E0BB01E8224A"
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	adds a short flash to the Radar/Minimap
	Usage: UI.FLASH_MINIMAP_DISPLAY
</summary>
]]--
native "FLASH_MINIMAP_DISPLAY"
	hash "0xF2DD778C22B15BDA"
	jhash (0xB8359952)
	returns	"void"

native "0x6B1DE27EE78E6A19"
	hash "0x6B1DE27EE78E6A19"
	jhash (0x79A6CAF6)
	arguments {
		Any "p0",
	}
	returns	"void"

native "TOGGLE_STEALTH_RADAR"
	hash "0x6AFDFB93754950C7"
	jhash (0xC68D47C4)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "KEY_HUD_COLOUR"
	hash "0x1A5CD7752DD28CD3"
	jhash (0xD5BFCADB)
	arguments {
		BOOL "p0",

		Any "p1",
	}
	returns	"void"

native "SET_MISSION_NAME"
	hash "0x5F28ECF5FC84772F"
	jhash (0x68DCAE10)
	arguments {
		BOOL "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0xE45087D85F468BC2"
	hash "0xE45087D85F468BC2"
	jhash (0x8D9A1734)
	arguments {
		BOOL "p0",

		AnyPtr "p1",
	}
	returns	"void"

--[[!
<summary>
	Every occurrence I found had 0 for every parameter.

</summary>
]]--
native "0x817B86108EB94E51"
	hash "0x817B86108EB94E51"
	jhash (0xD2161E77)
	arguments {
		BOOL "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",

		AnyPtr "p4",

		AnyPtr "p5",

		AnyPtr "p6",

		AnyPtr "p7",

		AnyPtr "p8",
	}
	returns	"void"

native "SET_MINIMAP_BLOCK_WAYPOINT"
	hash "0x58FADDED207897DC"
	jhash (0xA41C3B62)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Toggles the minimap-map and the map(in the mainmenu) visibility, no map will be drawn anymore
</summary>
]]--
native "_SET_DRAW_MAP_VISIBLE"
	hash "0x9133955F1A2DA957"
	jhash (0x02F5F1D1)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0xF8DEE0A5600CBB93"
	hash "0xF8DEE0A5600CBB93"
	jhash (0xD8D77733)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xE0130B41D3CF4574"
	hash "0xE0130B41D3CF4574"
	jhash (0xA4098ACC)
	returns	"Any"

native "0x6E31B91145873922"
	hash "0x6E31B91145873922"
	jhash (0x65B705F6)
	arguments {
		float "p0",

		float "p1",

		float "p2",
	}
	returns	"BOOL"

native "0x62E849B7EB28E770"
	hash "0x62E849B7EB28E770"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x0923DBF87DFF735E"
	hash "0x0923DBF87DFF735E"
	jhash (0xE010F081)
	arguments {
		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "0x71BDB63DBAF8DA59"
	hash "0x71BDB63DBAF8DA59"
	jhash (0x5133A750)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x35EDD5B2E3FF01C0"
	hash "0x35EDD5B2E3FF01C0"
	jhash (0x20FD3E87)
	returns	"void"

--[[!
<summary>
	Locks the minimap to the specified angle in integer degrees.

	angle: The angle in whole degrees. If less than 0 or greater than 360, unlocks the angle.
</summary>
]]--
native "LOCK_MINIMAP_ANGLE"
	hash "0x299FAEBB108AE05B"
	jhash (0xDEC733E4)
	arguments {
		int "angle",
	}
	returns	"void"

native "UNLOCK_MINIMAP_ANGLE"
	hash "0x8183455E16C42E3A"
	jhash (0x742043F9)
	returns	"void"

--[[!
<summary>
	Locks the minimap to the specified world position.
</summary>
]]--
native "LOCK_MINIMAP_POSITION"
	hash "0x1279E861A329E73F"
	jhash (0xB9632A91)
	arguments {
		float "x",

		float "y",
	}
	returns	"void"

native "UNLOCK_MINIMAP_POSITION"
	hash "0x3E93E06DB8EF1F30"
	jhash (0x5E8E6F54)
	returns	"void"

--[[!
<summary>
	Argument must be 0.0f or above 38.0f, or it will be ignored.
</summary>
]]--
native "_SET_MINIMAP_ATTITUDE_INDICATOR_LEVEL"
	hash "0xD201F3FF917A506D"
	jhash (0x0308EDF6)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x3F5CC444DCAAA8F2"
	hash "0x3F5CC444DCAAA8F2"
	jhash (0x7FB6FB2A)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x975D66A0BC17064C"
	hash "0x975D66A0BC17064C"
	jhash (0xF07D8CEF)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x06A320535F5F0248"
	hash "0x06A320535F5F0248"
	jhash (0x827F14DE)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Toggles the big minimap state like in GTA:Online.
</summary>
]]--
native "_SET_RADAR_BIGMAP_ENABLED"
	hash "0x231C8F89D0539D8F"
	jhash (0x08EB83D2)
	arguments {
		BOOL "toggleBigMap",

		BOOL "showFullMap",
	}
	returns	"void"

--[[!
<summary>
	See frontend.xml for ids.
</summary>
]]--
native "IS_HUD_COMPONENT_ACTIVE"
	hash "0xBC4C9EA5391ECC0D"
	jhash (0x6214631F)
	arguments {
		int "id",
	}
	returns	"BOOL"

native "IS_SCRIPTED_HUD_COMPONENT_ACTIVE"
	hash "0xDD100EB17A94FF65"
	jhash (0x2B86F382)
	arguments {
		int "id",
	}
	returns	"BOOL"

native "HIDE_SCRIPTED_HUD_COMPONENT_THIS_FRAME"
	hash "0xE374C498D8BADC14"
	jhash (0x31ABA127)
	arguments {
		int "id",
	}
	returns	"void"

native "0x09C0403ED9A751C2"
	hash "0x09C0403ED9A751C2"
	jhash (0xE8C8E535)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "HIDE_HUD_COMPONENT_THIS_FRAME"
	hash "0x6806C51AD12B83B8"
	jhash (0xDB2D0762)
	arguments {
		int "id",
	}
	returns	"void"

native "SHOW_HUD_COMPONENT_THIS_FRAME"
	hash "0x0B4DF1FA60C0E664"
	jhash (0x95E1546E)
	arguments {
		int "id",
	}
	returns	"void"

native "0xA4DEDE28B1814289"
	hash "0xA4DEDE28B1814289"
	jhash (0x52746FE1)
	returns	"void"

native "RESET_RETICULE_VALUES"
	hash "0x12782CE0A636E9F0"
	jhash (0xBE27AA3F)
	returns	"void"

native "RESET_HUD_COMPONENT_VALUES"
	hash "0x450930E616475D0D"
	jhash (0xD15B46DA)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_HUD_COMPONENT_POSITION"
	hash "0xAABB1F56E2A17CED"
	jhash (0x2F3A0D15)
	arguments {
		Any "p0",

		float "p1",

		float "p2",
	}
	returns	"void"

native "GET_HUD_COMPONENT_POSITION"
	hash "0x223CA69A8C4417FD"
	jhash (0x080DCED6)
	arguments {
		Any "p0",
	}
	returns	"int"

native "0xB57D8DD645CFA2CF"
	hash "0xB57D8DD645CFA2CF"
	jhash (0x5BBCC934)
	returns	"void"

native "0xF9904D11F1ACBEC3"
	hash "0xF9904D11F1ACBEC3"
	jhash (0xFE9A39F8)
	arguments {
		float "x",

		float "y",

		float "z",

		AnyPtr "p3",

		AnyPtr "p4",
	}
	returns	"Any"

--[[!
<summary>
	Shows a hud element for reporting jobs
</summary>
]]--
native "0x523A590C1A3CC0D3"
	hash "0x523A590C1A3CC0D3"
	jhash (0x10DE5150)
	returns	"void"

--[[!
<summary>
	Hides the hud element displayed by _0x523A590C1A3CC0D3
</summary>
]]--
native "0xEE4C0E6DBC6F2C6F"
	hash "0xEE4C0E6DBC6F2C6F"
	jhash (0x67649EE0)
	returns	"void"

native "0x9135584D09A3437E"
	hash "0x9135584D09A3437E"
	jhash (0x9D2C94FA)
	returns	"Any"

native "0x2432784ACA090DA4"
	hash "0x2432784ACA090DA4"
	jhash (0x45472FD5)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x7679CC1BCEBE3D4C"
	hash "0x7679CC1BCEBE3D4C"
	jhash (0x198F32D7)
	arguments {
		Any "p0",

		float "p1",

		float "p2",
	}
	returns	"void"

native "0x784BA7E0ECEB4178"
	hash "0x784BA7E0ECEB4178"
	jhash (0x93045157)
	arguments {
		Any "p0",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "0xB094BC1DB4018240"
	hash "0xB094BC1DB4018240"
	jhash (0x18B012B7)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "0x788E7FD431BD67F1"
	hash "0x788E7FD431BD67F1"
	jhash (0x97852A82)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"void"

native "CLEAR_FLOATING_HELP"
	hash "0x50085246ABD3FEFA"
	jhash (0xB181F88F)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x6DD05E9D83EFA4C9"
	hash "0x6DD05E9D83EFA4C9"
	jhash (0xC969F2D0)
	arguments {
		Any "p0",

		charPtr "p1",

		BOOL "p2",

		BOOL "p3",

		AnyPtr "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",
	}
	returns	"void"

native "0x6E0EB3EB47C8D7AA"
	hash "0x6E0EB3EB47C8D7AA"
	jhash (0xEFD2564A)
	returns	"Any"

--[[!
<summary>
	p0 was the return of NET_TO_PED in fm_mission_controler.
	p4 was always "".

</summary>
]]--
native "0xBFEFE3321A3F5015"
	hash "0xBFEFE3321A3F5015"
	jhash (0xF5CD2AA4)
	arguments {
		Any "p0",

		charPtr "p1",

		BOOL "p2",

		BOOL "p3",

		charPtr "p4",

		Any "p5",
	}
	returns	"Any"

native "0x31698AA80E0223F8"
	hash "0x31698AA80E0223F8"
	jhash (0x3D081FE4)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x4E929E7A5796FD26"
	hash "0x4E929E7A5796FD26"
	jhash (0x60118951)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	I'm fairly certain this is a collision
</summary>
]]--
native "ADD_TREVOR_RANDOM_MODIFIER"
	hash "0x595B5178E412E199"
	jhash (0x63959059)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x63BB75ABEDC1F6A0"
	hash "0x63BB75ABEDC1F6A0"
	jhash (0xD41DF479)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xEE76FF7E6A0166B0"
	hash "0xEE76FF7E6A0166B0"
	jhash (0x767DED29)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xA67F9C46D612B6F1"
	hash "0xA67F9C46D612B6F1"
	jhash (0xB01A5434)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x613ED644950626AE"
	hash "0x613ED644950626AE"
	jhash (0x7E3AA40A)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0x3158C77A7E888AB4"
	hash "0x3158C77A7E888AB4"
	jhash (0x5777EC77)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xD48FE545CD46F857"
	hash "0xD48FE545CD46F857"
	jhash (0xF4418611)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0xCF228E2AA03099C3"
	hash "0xCF228E2AA03099C3"
	jhash (0x0EBB003F)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xDEA2B8283BAA3944"
	hash "0xDEA2B8283BAA3944"
	jhash (0x627A559B)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0xEB709A36958ABE0D"
	hash "0xEB709A36958ABE0D"
	jhash (0xF11414C4)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x7B7723747CCB55B6"
	hash "0x7B7723747CCB55B6"
	jhash (0x939218AB)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x01A358D9128B7A86"
	hash "0x01A358D9128B7A86"
	jhash (0xAB5B7C18)
	returns	"Any"

native "GET_CURRENT_WEBSITE_ID"
	hash "0x97D47996FC48CBAD"
	jhash (0x42A55B14)
	returns	"Any"

native "0xE3B05614DCE1D014"
	hash "0xE3B05614DCE1D014"
	jhash (0xD217EE7E)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xB99C4E4D9499DF29"
	hash "0xB99C4E4D9499DF29"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xAF42195A42C63BBA"
	hash "0xAF42195A42C63BBA"
	returns	"Any"

--[[!
<summary>
	You can only use text entries. No custom text.

	Example: SET_WARNING_MESSAGE("t20", 3, "adder", false, -1, 0, 0, true);
</summary>
]]--
native "SET_WARNING_MESSAGE"
	hash "0x7B1776B3B53F8D74"
	jhash (0xBE699BDE)
	arguments {
		charPtr "entryLine1",

		int "instructionalKey",

		charPtr "entryLine2",

		BOOL "p3",

		Any "p4",

		AnyPtr "p5",

		AnyPtr "p6",

		BOOL "background",
	}
	returns	"void"

--[[!
<summary>
	You can only use text entries. No custom text.
</summary>
]]--
native "_SET_WARNING_MESSAGE_2"
	hash "0xDC38CC1E35B6A5D7"
	jhash (0x2DB9EAB5)
	arguments {
		charPtr "entryHeader",

		charPtr "entryLine1",

		int "instructionalKey",

		charPtr "entryLine2",

		BOOL "p4",

		Any "p5",

		AnyPtr "p6",

		AnyPtr "p7",

		BOOL "background",
	}
	returns	"void"

--[[!
<summary>
	You can only use text entries. No custom text.
</summary>
]]--
native "_SET_WARNING_MESSAGE_3"
	hash "0x701919482C74B5AB"
	jhash (0x749929D3)
	arguments {
		charPtr "entryHeader",

		charPtr "entryLine1",

		Any "instructionalKey",

		charPtr "entryLine2",

		BOOL "p4",

		Any "p5",

		Any "p6",

		AnyPtr "p7",

		AnyPtr "p8",

		BOOL "p9",
	}
	returns	"void"

native "0x0C5A80A9E096D529"
	hash "0x0C5A80A9E096D529"
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"BOOL"

native "0xDAF87174BE7454FF"
	hash "0xDAF87174BE7454FF"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x6EF54AB721DC6242"
	hash "0x6EF54AB721DC6242"
	returns	"void"

--[[!
<summary>

</summary>
]]--
native "IS_MEDICAL_DISABLED"
	hash "0xE18B138FABC53103"
	jhash (0x94C834AD)
	returns	"Any"

native "0x7792424AA0EAC32E"
	hash "0x7792424AA0EAC32E"
	jhash (0x2F9A309C)
	returns	"void"

native "0x5354C5BA2EA868A4"
	hash "0x5354C5BA2EA868A4"
	jhash (0xE4FD20D8)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x1EAE6DD17B7A5EFA"
	hash "0x1EAE6DD17B7A5EFA"
	jhash (0x13E7A5A9)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x551DF99658DB6EE8"
	hash "0x551DF99658DB6EE8"
	jhash (0x786CA0A2)
	arguments {
		float "p0",

		float "p1",

		float "p2",
	}
	returns	"Any"

native "0x2708FC083123F9FF"
	hash "0x2708FC083123F9FF"
	jhash (0xCBEC9369)
	returns	"void"

native "0x1121BFA1A1A522A8"
	hash "0x1121BFA1A1A522A8"
	jhash (0x3F4AFB13)
	returns	"Any"

native "0x82CEDC33687E1F50"
	hash "0x82CEDC33687E1F50"
	jhash (0x2F28F0A6)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x211C4EF450086857"
	hash "0x211C4EF450086857"
	jhash (0x801D0D86)
	returns	"void"

native "0xBF4F34A85CA2970C"
	hash "0xBF4F34A85CA2970C"
	jhash (0x317775DF)
	returns	"void"

--[[!
<summary>
	Does stuff like this:
	https://gyazo.com/7fcb78ea3520e3dbc5b2c0c0f3712617

	Example:
	int GetHash = Function.Call&lt;int&gt;(Hash.GET_HASH_KEY, "fe_menu_version_corona_lobby");
	Function.Call(Hash.ACTIVATE_FRONTEND_MENU, GetHash, 0, -1);

	BOOL p1 is unknown but is usually set as false in decompiled scripts.
	int p2 is unknown but -1 always works, not sure why though.

	~ISOFX

</summary>
]]--
native "ACTIVATE_FRONTEND_MENU"
	hash "0xEF01D36B9C9D0C7B"
	jhash (0x01D83872)
	arguments {
		Hash "menuhash",

		BOOL "p1",

		int "p2",
	}
	returns	"void"

--[[!
<summary>
	Before using this native click the native above and look at the decription.

	Example:
	int GetHash = Function.Call&lt;int&gt;(Hash.GET_HASH_KEY, "fe_menu_version_corona_lobby");
	Function.Call(Hash.ACTIVATE_FRONTEND_MENU, GetHash, 0, -1);
	Function.Call(Hash.RESTART_FRONTEND_MENU(GetHash, -1);

	This native refreshes the frontend menu.

	p1 = Hash of Menu
	p2 = Unknown but always works with -1.

	~ISOFX
</summary>
]]--
native "RESTART_FRONTEND_MENU"
	hash "0x10706DC6AD2D49C0"
	jhash (0xB07DAF98)
	arguments {
		Hash "menuHash",

		int "p1",
	}
	returns	"void"

native "0x2309595AD6145265"
	hash "0x2309595AD6145265"
	jhash (0x33D6868F)
	returns	"Any"

native "SET_PAUSE_MENU_ACTIVE"
	hash "0xDF47FC56C71569CF"
	jhash (0x1DCD878E)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "DISABLE_FRONTEND_THIS_FRAME"
	hash "0x6D3465A73092F0E6"
	jhash (0xD86A029E)
	returns	"void"

native "0xBA751764F0821256"
	hash "0xBA751764F0821256"
	jhash (0x7F349900)
	returns	"void"

native "0xCC3FDDED67BCFC63"
	hash "0xCC3FDDED67BCFC63"
	jhash (0x630CD8EE)
	returns	"void"

native "SET_FRONTEND_ACTIVE"
	hash "0x745711A75AB09277"
	jhash (0x81E1AD32)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "IS_PAUSE_MENU_ACTIVE"
	hash "0xB0034A223497FFCB"
	jhash (0xD3600591)
	returns	"BOOL"

native "0x2F057596F2BD0061"
	hash "0x2F057596F2BD0061"
	jhash (0xC85C4487)
	returns	"Any"

native "GET_PAUSE_MENU_STATE"
	hash "0x272ACD84970869C5"
	jhash (0x92F50134)
	returns	"Any"

native "0x5BFF36D6ED83E0AE"
	hash "0x5BFF36D6ED83E0AE"
	returns	"Vector3"

native "IS_PAUSE_MENU_RESTARTING"
	hash "0x1C491717107431C7"
	jhash (0x3C4CF4D9)
	returns	"BOOL"

native "0x2162C446DFDF38FD"
	hash "0x2162C446DFDF38FD"
	jhash (0x2DFD35C7)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x77F16B447824DA6C"
	hash "0x77F16B447824DA6C"
	jhash (0x0A89336C)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xCDCA26E80FAECB8F"
	hash "0xCDCA26E80FAECB8F"
	jhash (0xC84BE309)
	returns	"void"

native "0xDD564BDD0472C936"
	hash "0xDD564BDD0472C936"
	jhash (0x9FE8FD5E)
	arguments {
		Any "p0",
	}
	returns	"void"

native "OBJECT_DECAL_TOGGLE"
	hash "0x444D8CF241EC25C5"
	jhash (0x0029046E)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x84698AB38D0C6636"
	hash "0x84698AB38D0C6636"
	jhash (0xC51BC42F)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x2A25ADC48F87841F"
	hash "0x2A25ADC48F87841F"
	jhash (0x016D7AF9)
	returns	"Any"

native "0xDE03620F8703A9DF"
	hash "0xDE03620F8703A9DF"
	returns	"Any"

native "0x359AF31A4B52F5ED"
	hash "0x359AF31A4B52F5ED"
	returns	"Any"

native "0x13C4B962653A5280"
	hash "0x13C4B962653A5280"
	returns	"Any"

native "0xC8E1071177A23BE5"
	hash "0xC8E1071177A23BE5"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

--[[!
<summary>
	hash collision?
</summary>
]]--
native "ENABLE_DEATHBLOOD_SEETHROUGH"
	hash "0x4895BDEA16E7C080"
	jhash (0x15B24768)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xC78E239AC5B2DDB9"
	hash "0xC78E239AC5B2DDB9"
	jhash (0x6C67131A)
	arguments {
		BOOL "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0xF06EBB91A81E09E3"
	hash "0xF06EBB91A81E09E3"
	jhash (0x11D09737)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x3BAB9A4E4F2FF5C7"
	hash "0x3BAB9A4E4F2FF5C7"
	jhash (0xD3BF3ABD)
	returns	"Any"

native "0xEC9264727EEC0F28"
	hash "0xEC9264727EEC0F28"
	jhash (0xC06B763D)
	returns	"void"

native "0x14621BB1DF14E2B2"
	hash "0x14621BB1DF14E2B2"
	jhash (0xB9392CE7)
	returns	"void"

native "0x66E7CB63C97B7D20"
	hash "0x66E7CB63C97B7D20"
	jhash (0x92DAFA78)
	returns	"Any"

native "0x593FEAE1F73392D4"
	hash "0x593FEAE1F73392D4"
	jhash (0x22CA9F2A)
	returns	"Any"

native "0x4E3CD0EF8A489541"
	hash "0x4E3CD0EF8A489541"
	jhash (0xDA7951A2)
	returns	"Any"

native "0xF284AC67940C6812"
	hash "0xF284AC67940C6812"
	jhash (0x7D95AFFF)
	returns	"Any"

native "0x2E22FEFA0100275E"
	hash "0x2E22FEFA0100275E"
	jhash (0x96863460)
	returns	"Any"

native "0x0CF54F20DE43879C"
	hash "0x0CF54F20DE43879C"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x36C1451A88A09630"
	hash "0x36C1451A88A09630"
	jhash (0x8543AAC8)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x7E17BE53E1AAABAF"
	hash "0x7E17BE53E1AAABAF"
	jhash (0x6025AA2F)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0xA238192F33110615"
	hash "0xA238192F33110615"
	jhash (0x46794EB2)
	arguments {
		intPtr "p0",

		intPtr "p1",

		intPtr "p2",
	}
	returns	"BOOL"

native "SET_USERIDS_UIHIDDEN"
	hash "0xEF4CED81CEBEDC6D"
	jhash (0x4370999E)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0xCA6B2F7CE32AB653"
	hash "0xCA6B2F7CE32AB653"
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0x90A6526CF0381030"
	hash "0x90A6526CF0381030"
	jhash (0xD6CC4766)
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0x24A49BEAF468DC90"
	hash "0x24A49BEAF468DC90"
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"BOOL"

native "0x5FBD7095FE7AE57F"
	hash "0x5FBD7095FE7AE57F"
	jhash (0x51972B04)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x8F08017F9D7C47BD"
	hash "0x8F08017F9D7C47BD"
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",
	}
	returns	"BOOL"

--[[!
<summary>
	p0 was always 0xAE2602A3. 
</summary>
]]--
native "0x052991E59076E4E4"
	hash "0x052991E59076E4E4"
	jhash (0xD43BB56D)
	arguments {
		Hash "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "CLEAR_PED_IN_PAUSE_MENU"
	hash "0x5E62BE5DC58E9E06"
	jhash (0x28058ACF)
	returns	"void"

native "GIVE_PED_TO_PAUSE_MENU"
	hash "0xAC0BFBDC3BE00E14"
	jhash (0x2AD2C9CE)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x3CA6050692BC61B0"
	hash "0x3CA6050692BC61B0"
	jhash (0x127310EB)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xECF128344E9FF9F1"
	hash "0xECF128344E9FF9F1"
	jhash (0x8F45D327)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "_SHOW_SOCIAL_CLUB_LEGAL_SCREEN"
	hash "0x805D7CBB36FD6C4C"
	jhash (0x19FCBBB2)
	returns	"void"

native "0xF13FE2A80C05C561"
	hash "0xF13FE2A80C05C561"
	jhash (0x850690FF)
	returns	"Any"

native "0x6F72CD94F7B5B68C"
	hash "0x6F72CD94F7B5B68C"
	jhash (0x9D4934F4)
	returns	"Any"

native "0x75D3691713C3B05A"
	hash "0x75D3691713C3B05A"
	jhash (0x57218529)
	returns	"void"

native "0xD2B32BE3FC1626C6"
	hash "0xD2B32BE3FC1626C6"
	jhash (0x5F86AA39)
	returns	"void"

--[[!
<summary>
	UI::0x7AD67C95("Gallery");
	UI::0x7AD67C95("Missions");
	UI::0x7AD67C95("General");
	UI::0x7AD67C95("Playlists");

</summary>
]]--
native "0x9E778248D6685FE0"
	hash "0x9E778248D6685FE0"
	jhash (0x7AD67C95)
	arguments {
		charPtr "p0",
	}
	returns	"void"

native "0xC406BE343FC4B9AF"
	hash "0xC406BE343FC4B9AF"
	jhash (0xD4DA14EF)
	returns	"Any"

native "0x1185A8087587322C"
	hash "0x1185A8087587322C"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x8817605C2BA76200"
	hash "0x8817605C2BA76200"
	returns	"void"

--[[!
<summary>
	Returns whether or not the text chat (MULTIPLAYER_CHAT Scaleform component) is active.
</summary>
]]--
native "_IS_TEXT_CHAT_ACTIVE"
	hash "0xB118AF58B5F332A1"
	returns	"BOOL"

--[[!
<summary>
	Aborts the current message in the text chat.
</summary>
]]--
native "_ABORT_TEXT_CHAT"
	hash "0x1AC8F4AD40E22127"
	returns	"void"

--[[!
<summary>
	Sets an unknown boolean value in the text chat.
</summary>
]]--
native "_SET_TEXT_CHAT_UNK"
	hash "0x1DB21A44B09E8BA3"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xCEF214315D276FD1"
	hash "0xCEF214315D276FD1"
	jhash (0xFF06772A)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xD30C50DF888D58B5"
	hash "0xD30C50DF888D58B5"
	jhash (0x96C4C4DD)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "DOES_PED_HAVE_AI_BLIP"
	hash "0x15B8ECF844EE67ED"
	jhash (0x3BE1257F)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xE52B8E7F85D39A08"
	hash "0xE52B8E7F85D39A08"
	jhash (0xD8E31B1A)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "HIDE_SPECIAL_ABILITY_LOCKON_OPERATION"
	hash "0x3EED80DFF7325CAA"
	jhash (0x872C2CFB)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x0C4BBF625CA98C4E"
	hash "0x0C4BBF625CA98C4E"
	jhash (0xFFDF46F0)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x97C65887D4B37FA9"
	hash "0x97C65887D4B37FA9"
	jhash (0xF9DC2AF7)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x7CD934010E115C2C"
	hash "0x7CD934010E115C2C"
	jhash (0x06349065)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x56176892826A4FE8"
	hash "0x56176892826A4FE8"
	jhash (0xCA52CF43)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xA277800A9EAE340E"
	hash "0xA277800A9EAE340E"
	returns	"Any"

native "0x2632482FD6B9AB87"
	hash "0x2632482FD6B9AB87"
	returns	"void"

native "0x808519373FD336A3"
	hash "0x808519373FD336A3"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x04655F9D075D0AE5"
	hash "0x04655F9D075D0AE5"
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	NOTE: Debugging functions are not present in the retail version of the game.
</summary>
]]--
native "SET_DEBUG_LINES_AND_SPHERES_DRAWING_ACTIVE"
	hash "0x175B6BFC15CDD0C5"
	jhash (0x1418CA37)
	arguments {
		BOOL "enabled",
	}
	returns	"void"

--[[!
<summary>
	NOTE: Debugging functions are not present in the retail version of the game.
</summary>
]]--
native "DRAW_DEBUG_LINE"
	hash "0x7FDFADE676AA3CB0"
	jhash (0xABF783AB)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		int "r",

		int "g",

		int "b",

		int "alpha",
	}
	returns	"void"

--[[!
<summary>
	NOTE: Debugging functions are not present in the retail version of the game.
</summary>
]]--
native "DRAW_DEBUG_LINE_WITH_TWO_COLOURS"
	hash "0xD8B9A8AC5608FF94"
	jhash (0xE8BFF632)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		int "r1",

		int "g1",

		int "b1",

		int "r2",

		int "g2",

		int "b2",

		int "alpha1",

		int "alpha2",
	}
	returns	"void"

--[[!
<summary>
	NOTE: Debugging functions are not present in the retail version of the game.
</summary>
]]--
native "DRAW_DEBUG_SPHERE"
	hash "0xAAD68E1AB39DA632"
	jhash (0x304D0EEF)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		int "r",

		int "g",

		int "b",

		int "alpha",
	}
	returns	"void"

--[[!
<summary>
	NOTE: Debugging functions are not present in the retail version of the game.
</summary>
]]--
native "DRAW_DEBUG_BOX"
	hash "0x083A2CA4F2E573BD"
	jhash (0x8524A848)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		int "r",

		int "g",

		int "b",

		int "alpha",
	}
	returns	"void"

--[[!
<summary>
	NOTE: Debugging functions are not present in the retail version of the game.
</summary>
]]--
native "DRAW_DEBUG_CROSS"
	hash "0x73B1189623049839"
	jhash (0xB6DF3709)
	arguments {
		float "x",

		float "y",

		float "z",

		float "size",

		int "r",

		int "g",

		int "b",

		int "alpha",
	}
	returns	"void"

--[[!
<summary>
	NOTE: Debugging functions are not present in the retail version of the game.
</summary>
]]--
native "DRAW_DEBUG_TEXT"
	hash "0x3903E216620488E8"
	jhash (0x269B006F)
	arguments {
		charPtr "text",

		float "x",

		float "y",

		float "z",

		int "r",

		int "g",

		int "b",

		int "alpha",
	}
	returns	"void"

--[[!
<summary>
	NOTE: Debugging functions are not present in the retail version of the game.
</summary>
]]--
native "DRAW_DEBUG_TEXT_2D"
	hash "0xA3BB2E9555C05A8F"
	jhash (0x528B973B)
	arguments {
		charPtr "text",

		float "x",

		float "y",

		float "z",

		int "r",

		int "g",

		int "b",

		int "alpha",
	}
	returns	"void"

--[[!
<summary>
	Draws a depth-tested line from one point to another.
	----------------
	x1, y1, z1 : Coordinates for the first point
	x2, y2, z2 : Coordinates for the second point
	r, g, b, alpha : Color with RGBA-Values
	I recommend using a predefined function to call this.
	[VB.NET]
	Public Sub DrawLine(from As Vector3, [to] As Vector3, col As Color)
	    [Function].Call(Hash.DRAW_LINE, from.X, from.Y, from.Z, [to].X, [to].Y, [to].Z, col.R, col.G, col.B, col.A)
	End Sub

	[C#]
	public void DrawLine(Vector3 from, Vector3 to, Color col)
	{
	    Function.Call(Hash.DRAW_LINE, from.X, from.Y, from.Z, to.X, to.Y, to.Z, col.R, col.G, col.B, col.A)
	}
</summary>
]]--
native "DRAW_LINE"
	hash "0x6B7256074AE34680"
	jhash (0xB3426BCC)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		int "r",

		int "g",

		int "b",

		int "alpha",
	}
	returns	"void"

--[[!
<summary>
	x/y/z - Location of a vertex (in world coords), presumably.
	----------------
	x1, y1, z1     : Coordinates for the first point
	x2, y2, z2     : Coordinates for the second point
	x3, y3, z3     : Coordinates for the third point
	r, g, b, alpha : Color with RGBA-Values

	Keep in mind that only one side of the drawn triangle is visible: It's the side, in which the vector-product of the vectors heads to: (b-a)x(c-a) Or (b-a)x(c-b).
	But be aware: The function seems to work somehow differently. I have trouble having them drawn in rotated orientation. Try it yourself and if you somehow succeed, please edit this and post your solution.
	I recommend using a predefined function to call this.
	[VB.NET]
	Public Sub DrawPoly(a As Vector3, b As Vector3, c As Vector3, col As Color)
	    [Function].Call(Hash.DRAW_POLY, a.X, a.Y, a.Z, b.X, b.Y, b.Z, c.X, c.Y, c.Z, col.R, col.G, col.B, col.A)
	End Sub

	[C#]
	public void DrawPoly(Vector3 a, Vector3 b, Vector3 c, Color col)
	{
	    Function.Call(Hash.DRAW_POLY, a.X, a.Y, a.Z, b.X, b.Y, b.Z, c.X, c.Y, c.Z, col.R, col.G, col.B, col.A);
	}
	BTW: Intersecting triangles are not supported: They overlap in the order they were called.
</summary>
]]--
native "DRAW_POLY"
	hash "0xAC26716048436851"
	jhash (0xABD19253)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		float "x3",

		float "y3",

		float "z3",

		int "r",

		int "g",

		int "b",

		int "alpha",
	}
	returns	"void"

--[[!
<summary>
	x,y,z = start pos
	x2,y2,z2 = end pos

	Draw's a 3D Box between the two x,y,z coords.
	--------------
	Keep in mind that the edges of the box do only align to the worlds base-vectors. Therefore something like rotation cannot be applied. That means this function is pretty much useless, unless you want a static unicolor box somewhere.
	I recommend using a predefined function to call this.
	[VB.NET]
	Public Sub DrawBox(a As Vector3, b As Vector3, col As Color)
	    [Function].Call(Hash.DRAW_BOX,a.X, a.Y, a.Z,b.X, b.Y, b.Z,col.R, col.G, col.B, col.A)
	End Sub

	[C#]
	public void DrawBox(Vector3 a, Vector3 b, Color col)
	{
	    Function.Call(Hash.DRAW_BOX,a.X, a.Y, a.Z,b.X, b.Y, b.Z,col.R, col.G, col.B, col.A);
	}
</summary>
]]--
native "DRAW_BOX"
	hash "0xD3A9971CADAC7252"
	jhash (0xCD4D9DD5)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		int "r",

		int "g",

		int "b",

		int "alpha",
	}
	returns	"void"

native "0x23BA6B0C2AD7B0D3"
	hash "0x23BA6B0C2AD7B0D3"
	jhash (0xC44C2F44)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x1DD2139A9A20DCE8"
	hash "0x1DD2139A9A20DCE8"
	jhash (0xBA9AD458)
	returns	"Any"

native "0x90A78ECAA4E78453"
	hash "0x90A78ECAA4E78453"
	jhash (0xADBBA287)
	returns	"Any"

native "0x0A46AF8A78DC5E0A"
	hash "0x0A46AF8A78DC5E0A"
	jhash (0x9E553002)
	returns	"void"

native "0x4862437A486F91B0"
	hash "0x4862437A486F91B0"
	jhash (0x56C1E488)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0x1670F8D05056F257"
	hash "0x1670F8D05056F257"
	jhash (0x226B08EA)
	arguments {
		AnyPtr "p0",
	}
	returns	"int"

native "0x7FA5D82B8F58EC06"
	hash "0x7FA5D82B8F58EC06"
	jhash (0x1F3CADB0)
	returns	"Any"

native "0x5B0316762AFD4A64"
	hash "0x5B0316762AFD4A64"
	jhash (0xA9DC8558)
	returns	"Any"

native "0x346EF3ECAAAB149E"
	hash "0x346EF3ECAAAB149E"
	jhash (0x88EAF398)
	returns	"void"

native "0xA67C35C56EB1BD9D"
	hash "0xA67C35C56EB1BD9D"
	jhash (0x47B0C137)
	returns	"Any"

native "0x0D6CA79EEEBD8CA3"
	hash "0x0D6CA79EEEBD8CA3"
	jhash (0x65376C9B)
	returns	"Any"

--[[!
<summary>
	4 matches across 2 scripts.

	appcamera:
	called after UI::HIDE_HUD_AND_RADAR_THIS_FRAME() and before GRAPHICS::0x108F36CC();

	cellphone_controller:
	called after GRAPHICS::0xE9F2B68F(0, 0) and before GRAPHICS::0x108F36CC();


</summary>
]]--
native "0xD801CC02177FA3F1"
	hash "0xD801CC02177FA3F1"
	jhash (0x9CBA682A)
	returns	"void"

native "0x1BBC135A4D25EDDE"
	hash "0x1BBC135A4D25EDDE"
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	1 match in 1 script. cellphone_controller.
	p0 was -1. 
</summary>
]]--
native "0x3DEC726C25A11BAC"
	hash "0x3DEC726C25A11BAC"
	jhash (0x3B15D33C)
	arguments {
		int "p0",
	}
	returns	"Any"

native "0x0C0C4E81E1AC60A0"
	hash "0x0C0C4E81E1AC60A0"
	jhash (0xEC5D0317)
	returns	"Any"

native "0x759650634F07B6B4"
	hash "0x759650634F07B6B4"
	jhash (0x25D569EB)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xCB82A0BF0E3E3265"
	hash "0xCB82A0BF0E3E3265"
	jhash (0xCFCDC518)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x6A12D88881435DCA"
	hash "0x6A12D88881435DCA"
	jhash (0x108F36CC)
	returns	"void"

native "0x1072F115DAB0717E"
	hash "0x1072F115DAB0717E"
	jhash (0xE9F2B68F)
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"void"

native "GET_MAXIMUM_NUMBER_OF_PHOTOS"
	hash "0x34D23450F028B0BF"
	jhash (0x727AA63F)
	returns	"Any"

native "0xDC54A7AF8B3A14EF"
	hash "0xDC54A7AF8B3A14EF"
	jhash (0x239272BD)
	returns	"Any"

native "0x473151EBC762C6DA"
	hash "0x473151EBC762C6DA"
	jhash (0x21DBF0C9)
	returns	"Any"

--[[!
<summary>
	2 matches across 2 scripts. Only showed in appcamera &amp; appmedia. Both were 0. 
</summary>
]]--
native "0x2A893980E96B659A"
	hash "0x2A893980E96B659A"
	jhash (0x199FABF0)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	3 matches across 3 scripts. First 2 were 0, 3rd was 1. Possibly a bool.
	appcamera, appmedia, and cellphone_controller. 
</summary>
]]--
native "0xF5BED327CEA362B1"
	hash "0xF5BED327CEA362B1"
	jhash (0x596B900D)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x4AF92ACD3141D96C"
	hash "0x4AF92ACD3141D96C"
	jhash (0xC9EF81ED)
	returns	"void"

native "0xE791DF1F73ED2C8B"
	hash "0xE791DF1F73ED2C8B"
	jhash (0x9D84554C)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xEC72C258667BE5EA"
	hash "0xEC72C258667BE5EA"
	jhash (0x9C106AD9)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x40AFB081F8ADD4EE"
	hash "0x40AFB081F8ADD4EE"
	jhash (0x762E5C5F)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "_DRAW_LIGHT_WITH_RANGE_WITH_SHADOW"
	hash "0xF49E9A9716A04595"
	arguments {
		float "x",

		float "y",

		float "z",

		int "r",

		int "g",

		int "b",

		float "range",

		float "intensity",

		float "shadow",
	}
	returns	"void"

native "DRAW_LIGHT_WITH_RANGE"
	hash "0xF2A1B2771A01DBD4"
	jhash (0x6A396E9A)
	arguments {
		float "x",

		float "y",

		float "z",

		int "r",

		int "g",

		int "b",

		float "range",

		float "intensity",
	}
	returns	"void"

--[[!
<summary>
	x,y,z - coordinate where the spotlight is located
	dirVectorX, dirVectorY, dirVectorZ - the direction vector the spotlight should aim at from its current position
	r, g, b - color of the spotlight
	distance - the maximum distance the light can reach
	brightness - the brightness of the light
	roundness - "smoothness" of the circle edge
	radius - the radius size of the spotlightd
	fadeout - dont know how to describe, look at this instead: http://i.imgur.com/DemAWeO.jpg

	Example in C# (spotlight aims at the closest vehicle):
	Vector3 myPos = Game.Player.Character.Position;
	Vehicle nearest = World.GetClosestVehicle(myPos , 1000f);
	Vector3 destinationCoords = nearest.Position;
	Vector3 dirVector = destinationCoords - myPos;
	dirVector.Normalize();
	Function.Call(Hash.DRAW_SPOT_LIGHT, pos.X, pos.Y, pos.Z, dirVector.X, dirVector.Y, dirVector.Z, 255, 255, 255, 100.0f, 1f, 0.0f, 13.0f, 1f);
</summary>
]]--
native "DRAW_SPOT_LIGHT"
	hash "0xD0F64B265C8C8B33"
	jhash (0xBDBC410C)
	arguments {
		float "x",

		float "y",

		float "z",

		float "dirVectorX",

		float "dirVectorY",

		float "dirVectorZ",

		int "r",

		int "g",

		int "b",

		float "distance",

		float "brightness",

		float "roundness",

		float "radius",

		float "fadeout",
	}
	returns	"void"

native "_DRAW_SPOT_LIGHT_WITH_SHADOW"
	hash "0x5BCA583A583194DB"
	jhash (0x32BF9598)
	arguments {
		float "x",

		float "y",

		float "z",

		float "dirVectorX",

		float "dirVectorY",

		float "dirVectorZ",

		int "r",

		int "g",

		int "b",

		float "distance",

		float "brightness",

		float "roundness",

		float "radius",

		float "fadeout",

		float "shadow",
	}
	returns	"void"

native "0xC9B18B4619F48F7B"
	hash "0xC9B18B4619F48F7B"
	jhash (0x93628786)
	arguments {
		float "p0",
	}
	returns	"void"

--[[!
<summary>
	Only found twice in decompiled scripts. Something to do with an entity/object?

	On a side note, it's very interesting how the hash for this native is "DEADC0DE" - this is usually used as padding for initializing a buffer of some sort. I wonder if this native is actually "dead"? -CL69

	"carmod_shop.ysc", line 9520:
	if (ENTITY::DOES_ENTITY_EXIST(l_324._f6)) {
	    GRAPHICS::_0xDEADC0DEDEADC0DE(l_324._f6);
	}

	"fm_mission_controller.ysc", line 189641:
	if (GAMEPLAY::IS_BIT_SET(g_1870E1._f7B64[a_0/*104*/]._f25, 28)) {
	    GRAPHICS::_0xDEADC0DEDEADC0DE(NETWORK::NET_TO_OBJ(l_4064._f26A._f87[a_0/*1*/]));
	    if (!GAMEPLAY::IS_BIT_SET(g_1870E1._f7B64[a_0/*104*/]._f25, 31)) {
	        if (!ENTITY::IS_ENTITY_DEAD(v_7)) {
	            AUDIO::PLAY_SOUND_FROM_ENTITY(-1, "EMP_Vehicle_Hum", v_7, "DLC_HEIST_BIOLAB_DELIVER_EMP_SOUNDS", 0, 0);
	            GAMEPLAY::SET_BIT(&amp;g_1870E1._f7B64[a_0/*104*/]._f25, 31);
	        }
	    }
	}
</summary>
]]--
native "0xDEADC0DEDEADC0DE"
	hash "0xDEADC0DEDEADC0DE"
	arguments {
		Object "object",
	}
	returns	"void"

--[[!
<summary>
	enum MarkerTypes
	{
		MarkerTypeUpsideDownCone = 0,
		MarkerTypeVerticalCylinder = 1,
		MarkerTypeThickChevronUp = 2,
		MarkerTypeThinChevronUp = 3,
		MarkerTypeCheckeredFlagRect = 4,
		MarkerTypeCheckeredFlagCircle = 5,
		MarkerTypeVerticleCircle = 6,
		MarkerTypePlaneModel = 7,
		MarkerTypeLostMCDark = 8,
		MarkerTypeLostMCLight = 9,
		MarkerTypeNumber0 = 10,
		MarkerTypeNumber1 = 11,
		MarkerTypeNumber2 = 12,
		MarkerTypeNumber3 = 13,
		MarkerTypeNumber4 = 14,
		MarkerTypeNumber5 = 15,
		MarkerTypeNumber6 = 16,
		MarkerTypeNumber7 = 17,
		MarkerTypeNumber8 = 18,
		MarkerTypeNumber9 = 19,
		MarkerTypeChevronUpx1 = 20,
		MarkerTypeChevronUpx2 = 21,
		MarkerTypeChevronUpx3 = 22,
		MarkerTypeHorizontalCircleFat = 23,
		MarkerTypeReplayIcon = 24,
		MarkerTypeHorizontalCircleSkinny = 25,
		MarkerTypeHorizontalCircleSkinny_Arrow = 26,
		MarkerTypeHorizontalSplitArrowCircle = 27,
		MarkerTypeDebugSphere = 28
	};

	dirX/Y/Z represent a heading on each axis in which the marker should face, alternatively you can rotate each axis independently with rotX/Y/Z (and set dirX/Y/Z all to 0).

	faceCamera - Rotates only the y-axis (the heading) towards the camera

	p19 - no effect, default value in script is 2

	rotate - Rotates only on the y-axis (the heading)

	textureDict - Name of texture dictionary to load texture from (e.g. "GolfPutting")

	textureName - Name of texture inside dictionary to load (e.g. "PuttingMarker")

	drawOnEnts - Draws the marker onto any entities that intersect it
</summary>
]]--
native "DRAW_MARKER"
	hash "0x28477EC23D892089"
	jhash (0x48D84A02)
	arguments {
		int "type",

		float "x",

		float "y",

		float "z",

		float "dirX",

		float "dirY",

		float "dirZ",

		float "rotX",

		float "rotY",

		float "rotZ",

		float "scaleX",

		float "scaleY",

		float "scaleZ",

		Any "colorR",

		Any "colorG",

		Any "colorB",

		int "alpha",

		BOOL "bobUpAndDown",

		BOOL "faceCamera",

		int "p19",

		BOOL "rotate",

		charPtr "textureDict",

		charPtr "textureName",

		BOOL "drawOnEnts",
	}
	returns	"void"

--[[!
<summary>
	p0 Ranges from 0 to 45 (not confirmed 100%)

	p1 checkpoint X location
	p2 checkpoint Y location
	p3 checkpoint Z location

	p4 checkpoint X2 location (facing to next checkpoint?)
	p5 checkpoint Y2 location (facing to next checkpoint?)
	p6 checkpoint Z2 location (facing to next checkpoint?)

	p7 Checkpoint radius

	p8 R color (0-255)
	p9 G color (0-255)
	p10 B color (0-255)
	p11 Alpha (0-255)

	p12 ¿? - Default in GTA V scripts is 0.

	Public Enum CheckType

	            Traditional = 0
	            SmallArow = 5
	            DoubleArrow = 6
	            TripleArrow = 7
	            CycleArrow = 8
	            ArrowInCircle = 10
	            DoubleArrowInCircle = 11
	            TripleArrowInCircle = 12
	            CycleArrowInCircle = 13
	            CheckerInCircle = 14
	            Arrow = 15
	        End Enum

	Set param3 -1 of your Z value so it doesn't float 




	CheckType:
	0-4     Cylinder: 1 arrow, 2 arrow, 3 arrows, CycleArrow, Checker
	5-9     Cylinder: 1 arrow, 2 arrow, 3 arrows, CycleArrow, Checker
	10-14       Ring: 1 arrow, 2 arrow, 3 arrows, CycleArrow, Checker
	15-19   1 arrow, 2 arrow, 3 arrows, CycleArrow, Checker      
	20-24  Cylinder: 1 arrow, 2 arrow, 3 arrows, CycleArrow, Checker 
	25-29  Cylinder: 1 arrow, 2 arrow, 3 arrows, CycleArrow, Checker    
	30-34  Cylinder: 1 arrow, 2 arrow, 3 arrows, CycleArrow, Checker 
	35-38      Ring: Airplane Up, Left, Right, UpsideDown
	39 ?
	40         Ring: just a ring
	41?
	42-44   Cylinder with number use p12 to set displayed number
	45-47   Cylinder no arrow or number

	If using type 42-44 p12 sets number / number and shape to display

	  0-99           Just numbers 0-99
	100-109                 Arrow 0-9
	110-119            Two arrows 0-9
	120-129          Three arrows 0-9
	130-139                Circle 0-9
	140-149            CycleArrow 0-9
	150-159                Circle 0-9
	160-169  Circle  with pointer 0-9
	170-179       Perferated ring 0-9
	180-189                Sphere 0-9
</summary>
]]--
native "CREATE_CHECKPOINT"
	hash "0x0134F0835AB6BFCB"
	jhash (0xF541B690)
	arguments {
		int "Type",

		float "x",

		float "y",

		float "z",

		float "x2",

		float "y2",

		float "z2",

		float "radius",

		int "R",

		int "G",

		int "B",

		int "Alpha",

		int "p12",
	}
	returns	"Any"

native "0x4B5B4DA5D79F1943"
	hash "0x4B5B4DA5D79F1943"
	jhash (0x80151CCF)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

--[[!
<summary>
	float nearHeight - How tall the checkpoint will be when you're within the scale radius.

	float farHeight - How tall the checkpoint will be when you're outside the scale radius.

	float scaleRadius - The radius from the checkpoint you need to be before the checkpoint will scale from farHeight to nearHeight.


</summary>
]]--
native "SET_CHECKPOINT_CYLINDER_HEIGHT"
	hash "0x2707AAE9D9297D89"
	jhash (0xFF0F9B22)
	arguments {
		int "checkpoint",

		float "nearHeight",

		float "farHeight",

		float "scaleRadius",
	}
	returns	"void"

--[[!
<summary>
	Sets the checkpoint color.
</summary>
]]--
native "SET_CHECKPOINT_RGBA"
	hash "0x7167371E8AD747F7"
	jhash (0xEF9C8CB3)
	arguments {
		int "checkpoint",

		int "colorR",

		int "colorG",

		int "colorB",

		int "alpha",
	}
	returns	"void"

--[[!
<summary>
	Sets the checkpoint icon color.
</summary>
]]--
native "_SET_CHECKPOINT_ICON_RGBA"
	hash "0xB9EA40907C680580"
	jhash (0xA5456DBB)
	arguments {
		int "checkpoint",

		int "colorR",

		int "colorG",

		int "colorB",

		int "colorA",
	}
	returns	"void"

native "0xF51D36185993515D"
	hash "0xF51D36185993515D"
	jhash (0x20EABD0F)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"void"

native "0x615D3925E87A3B26"
	hash "0x615D3925E87A3B26"
	jhash (0x1E3A3126)
	arguments {
		Any "p0",
	}
	returns	"void"

native "DELETE_CHECKPOINT"
	hash "0xF5ED37F54CD4D52E"
	jhash (0xB66CF3CA)
	arguments {
		int "checkpoint",
	}
	returns	"void"

native "0x22A249A53034450A"
	hash "0x22A249A53034450A"
	jhash (0x932FDB81)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xDC459CFA0CCE245B"
	hash "0xDC459CFA0CCE245B"
	jhash (0x7E946E87)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "REQUEST_STREAMED_TEXTURE_DICT"
	hash "0xDFA2EF8E04127DD5"
	jhash (0x4C9B035F)
	arguments {
		charPtr "textureDict",

		BOOL "toggle",
	}
	returns	"void"

native "HAS_STREAMED_TEXTURE_DICT_LOADED"
	hash "0x0145F696AAAAD2E4"
	jhash (0x3F436EEF)
	arguments {
		charPtr "textureDict",
	}
	returns	"BOOL"

native "SET_STREAMED_TEXTURE_DICT_AS_NO_LONGER_NEEDED"
	hash "0xBE2CACCF5A8AA805"
	jhash (0xF07DDA38)
	arguments {
		charPtr "textureDict",
	}
	returns	"void"

--[[!
<summary>
	Draws a rectangle on the screen.

	-posX: The relative X point of the center of the rectangle. (0.0-1.0, 0.0 is the left edge of the screen, 1.0 is the right edge of the screen)

	-posY: The relative Y point of the center of the rectangle. (0.0-1.0, 0.0 is the top edge of the screen, 1.0 is the bottom edge of the screen)

	-width: The relative width of the rectangle. (0.0-1.0, 1.0 means the whole screen width)

	-height: The relative height of the rectangle. (0.0-1.0, 1.0 means the whole screen height)

	-R: Red part of the color. (0-255)

	-G: Green part of the color. (0-255)

	-B: Blue part of the color. (0-255)

	-A: Alpha part of the color. (0-255, 0 means totally transparent, 255 means totall opaque)

	The total number of rectangles to be drawn in one frame is apparently limited to 399.

</summary>
]]--
native "DRAW_RECT"
	hash "0x3A618A217E5154F0"
	jhash (0xDD2BFC77)
	arguments {
		float "posX",

		float "posY",

		float "width",

		float "height",

		int "R",

		int "G",

		int "B",

		int "A",
	}
	returns	"void"

native "0xC6372ECD45D73BCD"
	hash "0xC6372ECD45D73BCD"
	jhash (0xF8FBCC25)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Called before drawing stuff.

	Examples:
	GRAPHICS::_61BB1D9B3A95D802(7);
	GRAPHICS::DRAW_RECT(0.5, 0.5, 3.0, 3.0, v_4, v_5, v_6, a_0._f172, 0);

	GRAPHICS::_61BB1D9B3A95D802(1);
	GRAPHICS::DRAW_RECT(0.5, 0.5, 1.5, 1.5, 0, 0, 0, 255, 0);

	Appears to be the layer it's drawn on 

</summary>
]]--
native "0x61BB1D9B3A95D802"
	hash "0x61BB1D9B3A95D802"
	jhash (0xADF81D24)
	arguments {
		int "layer",
	}
	returns	"void"

--[[!
<summary>
	Seems to move all the drawn text on the screen to given coordinates.
	It also removed all the drawn sprites of my screen so not to sure what the exact function is.

</summary>
]]--
native "_SET_SCREEN_DRAW_POSITION"
	hash "0xB8A850F20A067EB6"
	jhash (0x228A2598)
	arguments {
		int "x",

		int "y",
	}
	returns	"void"

native "0xE3A3DB414A373DAB"
	hash "0xE3A3DB414A373DAB"
	jhash (0x3FE33BD6)
	returns	"void"

native "0xF5A2C681787E579D"
	hash "0xF5A2C681787E579D"
	jhash (0x76C641E4)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"void"

native "0x6DD8F5AA635EB4B2"
	hash "0x6DD8F5AA635EB4B2"
	arguments {
		float "p0",

		float "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"void"

native "GET_SAFE_ZONE_SIZE"
	hash "0xBAF107B6BB2C97F0"
	jhash (0x3F0D1A6F)
	returns	"Any"

--[[!
<summary>
	Draws a 2D sprite on the screen.

	Parameters:
	textureDict - Name of texture dictionary to load texture from (e.g. "CommonMenu", "MPWeaponsCommon", etc.)

	textureName - Name of texture to load from texture dictionary (e.g. "last_team_standing_icon", "tennis_icon", etc.)

	screenX/Y - Screen offset (0.5 = center)
	scaleX/Y - Texture scaling. Negative values can be used to flip the texture on that axis. (0.5 = half)

	heading - Texture rotation in degrees (default = 0.0) positive is clockwise, measured in degrees

	colorR/G/B - Sprite color (default = 255/255/255)

	alpha - Alpha intensity (default = 255 (100% opacity))
</summary>
]]--
native "DRAW_SPRITE"
	hash "0xE7FFAE5EBF23D890"
	jhash (0x1FEC16B0)
	arguments {
		charPtr "textureDict",

		charPtr "textureName",

		float "screenX",

		float "screenY",

		float "scaleX",

		float "scaleY",

		float "heading",

		int "colorR",

		int "colorG",

		int "colorB",

		int "colorA",
	}
	returns	"void"

native "ADD_ENTITY_ICON"
	hash "0x9CD43EEE12BF4DD0"
	jhash (0xF3027D21)
	arguments {
		Entity "entity",

		charPtr "icon",
	}
	returns	"Any"

native "SET_ENTITY_ICON_VISIBILITY"
	hash "0xE0E8BEECCA96BA31"
	jhash (0xD1D2FD52)
	arguments {
		Entity "entity",

		BOOL "toggle",
	}
	returns	"void"

native "SET_ENTITY_ICON_COLOR"
	hash "0x1D5F595CCAE2E238"
	jhash (0x6EE1E946)
	arguments {
		Entity "entity",

		int "r",

		int "g",

		int "b",

		int "a",
	}
	returns	"void"

--[[!
<summary>
	Sets the on-screen drawing origin for draw-functions (which is normally x=0,y=0 in the upper left corner of the screen) to a world coordinate.
	From now on, the screen coordinate which displays the given world coordinate on the screen is seen as x=0,y=0.

	Example in C#:
	Vector3 boneCoord = somePed.GetBoneCoord(Bone.SKEL_Head);
	Function.Call(Hash.SET_DRAW_ORIGIN, boneCoord.X, boneCoord.Y, boneCoord.Z, 0);
	Function.Call(Hash.DRAW_SPRITE, "helicopterhud", "hud_corner", -0.01, -0.015, 0.013, 0.013, 0.0, 255, 0, 0, 200);
	Function.Call(Hash.DRAW_SPRITE, "helicopterhud", "hud_corner", 0.01, -0.015, 0.013, 0.013, 90.0, 255, 0, 0, 200);
	Function.Call(Hash.DRAW_SPRITE, "helicopterhud", "hud_corner", -0.01, 0.015, 0.013, 0.013, 270.0, 255, 0, 0, 200);
	Function.Call(Hash.DRAW_SPRITE, "helicopterhud", "hud_corner", 0.01, 0.015, 0.013, 0.013, 180.0, 255, 0, 0, 200);
	Function.Call(Hash.CLEAR_DRAW_ORIGIN);

	Result: http://www11.pic-upload.de/19.06.15/bkqohvil2uao.jpg
	If the pedestrian starts walking around now, the sprites are always around her head, no matter where the head is displayed on the screen.

	This function also effects the drawing of texts and other UI-elements.
	The effect can be reset by calling GRAPHICS::CLEAR_DRAW_ORIGIN().
</summary>
]]--
native "SET_DRAW_ORIGIN"
	hash "0xAA0008F3BBB8F416"
	jhash (0xE10198D5)
	arguments {
		float "x",

		float "y",

		float "z",

		Any "p3",
	}
	returns	"void"

--[[!
<summary>
	Resets the screen's draw-origin which was changed by the function GRAPHICS::SET_DRAW_ORIGIN(...) back to x=0,y=0.

	See GRAPHICS::SET_DRAW_ORIGIN(...) for further information.
</summary>
]]--
native "CLEAR_DRAW_ORIGIN"
	hash "0xFF0B610F6BE0D7AF"
	jhash (0xDD76B263)
	returns	"void"

--[[!
<summary>
	Might be more appropriate in AUDIO?
</summary>
]]--
native "ATTACH_TV_AUDIO_TO_ENTITY"
	hash "0x845BAD77CC770633"
	jhash (0x784944DB)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Might be more appropriate in AUDIO?
</summary>
]]--
native "SET_TV_AUDIO_FRONTEND"
	hash "0x113D2C5DC57E1774"
	jhash (0x2E0DFA35)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "LOAD_MOVIE_MESH_SET"
	hash "0xB66064452270E8F1"
	jhash (0x9627905C)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "RELEASE_MOVIE_MESH_SET"
	hash "0xEB119AA014E89183"
	jhash (0x4FA5501D)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x9B6E70C5CEEF4EEB"
	hash "0x9B6E70C5CEEF4EEB"
	jhash (0x9D5D9B38)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	This native is not implemented and always returns the same.
	Sets X to 1280 and Y to 720.
</summary>
]]--
native "GET_SCREEN_RESOLUTION"
	hash "0x888D57E407E63624"
	jhash (0x29F3572F)
	arguments {
		intPtr "x",

		intPtr "y",
	}
	returns	"void"

--[[!
<summary>
	Returns current screen resolution.
</summary>
]]--
native "_GET_SCREEN_ACTIVE_RESOLUTION"
	hash "0x873C9F3104101DD3"
	arguments {
		intPtr "x",

		intPtr "y",
	}
	returns	"void"

native "_GET_SCREEN_ASPECT_RATIO"
	hash "0xF1307EF624A80D87"
	arguments {
		BOOL "b",
	}
	returns	"float"

native "0xB2EBE8CBC58B90E9"
	hash "0xB2EBE8CBC58B90E9"
	returns	"Any"

--[[!
<summary>
	Setting Aspect Ratio Manually in game will return:

	false - for Narrow format Aspect Ratios (3:2, 4:3, 5:4, etc. )
	true - for Wide format Aspect Ratios (5:3, 16:9, 16:10, etc. )

	Setting Aspect Ratio to "Auto" in game will return "false" or "true" based on the actual set Resolution Ratio.



	(80T)
</summary>
]]--
native "GET_IS_WIDESCREEN"
	hash "0x30CF4BDA4FCB1905"
	jhash (0xEC717AEF)
	returns	"BOOL"

--[[!
<summary>
	false = Any resolution &lt; 1280x720
	true = Any resolution &gt;= 1280x720



	(80T)
</summary>
]]--
native "GET_IS_HIDEF"
	hash "0x84ED31191CC5D2C9"
	jhash (0x1C340359)
	returns	"BOOL"

native "0xEFABC7722293DA7C"
	hash "0xEFABC7722293DA7C"
	returns	"void"

--[[!
<summary>
	Enables Night Vision.

	Example:
	C#: Function.Call(Hash.SET_NIGHTVISION, true);
	C++: GRAPHICS::SET_NIGHTVISION(true);

	BOOL Toggle:
	true = turns night vision on for your player.
	false = turns night vision off for your player.

	~ISOFX
</summary>
]]--
native "SET_NIGHTVISION"
	hash "0x18F621F7A5B1F85D"
	jhash (0xD1E5565F)
	arguments {
		BOOL "Toggle",
	}
	returns	"void"

native "0x35FB78DC42B7BD21"
	hash "0x35FB78DC42B7BD21"
	returns	"Any"

--[[!
<summary>
	Gets whether or not NIGHTVISION is INactive.

	Note:  When nightvision is actually active, this native will return FALSE!
</summary>
]]--
native "_IS_NIGHTVISION_INACTIVE"
	hash "0x2202A3F42C8E5F79"
	jhash (0x62619061)
	returns	"BOOL"

native "0xEF398BEEE4EF45F9"
	hash "0xEF398BEEE4EF45F9"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_NOISEOVERIDE"
	hash "0xE787BF1C5CF823C9"
	jhash (0xD576F5DD)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_NOISINESSOVERIDE"
	hash "0xCB6A7C3BB17A0C67"
	jhash (0x046B62D9)
	arguments {
		float "p0",
	}
	returns	"void"

--[[!
<summary>
	Convert a world coordinate into its relative screen coordinate.  (WorldToScreen)

	 x Returns whether or not the operation was successful. x

	*UPDATE* Returns float value equivalents of the 3d co-ordinate.

	For .NET users...

	Public Shared Function World3DToScreen2d(pos as vector3) As Vector2

	        Dim x2dp, y2dp As New Native.OutputArgument

	        Native.Function.Call(Of Boolean)(Native.Hash._WORLD3D_TO_SCREEN2D, pos.x, pos.y, pos.z, x2dp, y2dp)
	        Return New Vector2(x2dp.GetResult(Of Single), y2dp.GetResult(Of Single))

	    End Function

	//USE VERY SMALL VALUES FOR THE SCALE OF RECTS/TEXT because it is dramatically larger on screen than in 3D, e.g '0.05' small.

</summary>
]]--
native "_WORLD3D_TO_SCREEN2D"
	hash "0x34E82F05DF2974F5"
	jhash (0x1F950E4B)
	arguments {
		float "x3d",

		float "y3d",

		float "z3d",

		floatPtr "x2d",

		floatPtr "y2d",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns the texture resolution of the passed texture dict+name.

	Note: Most texture resolutions are doubled compared to the console version of the game.
</summary>
]]--
native "GET_TEXTURE_RESOLUTION"
	hash "0x35736EE65BD00C11"
	jhash (0x096DAA4D)
	arguments {
		charPtr "textureDict",

		charPtr "textureName",
	}
	returns	"Vector3"

native "0xE2892E7E55D7073A"
	hash "0xE2892E7E55D7073A"
	jhash (0x455F1084)
	arguments {
		float "p0",
	}
	returns	"void"

--[[!
<summary>
	Purpose of p0 and p1 unknown.
</summary>
]]--
native "SET_FLASH"
	hash "0x0AB84296FED9CFC6"
	jhash (0x7E55A1EE)
	arguments {
		float "p0",

		float "p1",

		float "fadeIn",

		float "duration",

		float "fadeOut",
	}
	returns	"void"

native "0x3669F1B198DCAA4F"
	hash "0x3669F1B198DCAA4F"
	jhash (0x0DCC0B8B)
	returns	"void"

--[[!
<summary>
	Disables all emissive textures and lights like city lights, car lights, cop car lights. Particles still emit light

	Used in Humane Labs Heist for EMP.
</summary>
]]--
native "_SET_BLACKOUT"
	hash "0x1268615ACE24D504"
	jhash (0xAA2A0EAF)
	arguments {
		BOOL "enable",
	}
	returns	"void"

native "0xC35A6D07C93802B2"
	hash "0xC35A6D07C93802B2"
	returns	"void"

--[[!
<summary>
	Creates a tracked point, useful for checking the visibility of a 3D point on screen.
</summary>
]]--
native "CREATE_TRACKED_POINT"
	hash "0xE2C9439ED45DEA60"
	jhash (0x3129C31A)
	returns	"Object"

native "SET_TRACKED_POINT_INFO"
	hash "0x164ECBB3CF750CB0"
	jhash (0x28689AA4)
	arguments {
		Object "point",

		float "x",

		float "y",

		float "z",

		float "radius",
	}
	returns	"Any"

native "IS_TRACKED_POINT_VISIBLE"
	hash "0xC45CCDAAC9221CA8"
	jhash (0x0BFC4F64)
	arguments {
		Object "point",
	}
	returns	"BOOL"

native "DESTROY_TRACKED_POINT"
	hash "0xB25DC90BAD56CA42"
	jhash (0x14AC675F)
	arguments {
		Object "point",
	}
	returns	"void"

native "0xBE197EAA669238F4"
	hash "0xBE197EAA669238F4"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"Any"

native "0x61F95E5BB3E0A8C6"
	hash "0x61F95E5BB3E0A8C6"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xAE51BC858F32BA66"
	hash "0xAE51BC858F32BA66"
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"void"

native "0x649C97D52332341A"
	hash "0x649C97D52332341A"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x2C42340F916C5930"
	hash "0x2C42340F916C5930"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x14FC5833464340A8"
	hash "0x14FC5833464340A8"
	returns	"void"

native "0x0218BA067D249DEA"
	hash "0x0218BA067D249DEA"
	returns	"void"

native "0x1612C45F9E3E0D44"
	hash "0x1612C45F9E3E0D44"
	returns	"void"

native "0x5DEBD9C4DC995692"
	hash "0x5DEBD9C4DC995692"
	returns	"void"

native "0x6D955F6A9E0295B1"
	hash "0x6D955F6A9E0295B1"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"void"

native "0x302C91AB2D477F7E"
	hash "0x302C91AB2D477F7E"
	returns	"void"

native "0x03FC694AE06C5A20"
	hash "0x03FC694AE06C5A20"
	jhash (0x48F16186)
	returns	"void"

native "0xD2936CAB8B58FCBD"
	hash "0xD2936CAB8B58FCBD"
	arguments {
		Any "p0",

		BOOL "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",

		float "p7",
	}
	returns	"void"

native "0x5F0F3F56635809EF"
	hash "0x5F0F3F56635809EF"
	jhash (0x13D4ABC0)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x5E9DAF5A20F15908"
	hash "0x5E9DAF5A20F15908"
	jhash (0xD2157428)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x36F6626459D91457"
	hash "0x36F6626459D91457"
	jhash (0xC07C64C9)
	arguments {
		float "p0",
	}
	returns	"void"

--[[!
<summary>
	When this is set to ON, shadows only draw as you get nearer.

	When OFF, they draw from a further distance.
</summary>
]]--
native "_SET_FAR_SHADOWS_SUPPRESSED"
	hash "0x80ECBC0C856D3B0B"
	jhash (0xFE903D0F)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x25FC3E33A31AD0C9"
	hash "0x25FC3E33A31AD0C9"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xB11D94BC55F41932"
	hash "0xB11D94BC55F41932"
	jhash (0xDE10BA1F)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x27CB772218215325"
	hash "0x27CB772218215325"
	returns	"void"

native "0x6DDBF9DFFC4AC080"
	hash "0x6DDBF9DFFC4AC080"
	jhash (0x9F470BE3)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xD39D13C9FEBF0511"
	hash "0xD39D13C9FEBF0511"
	jhash (0x4A124267)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x02AC28F3A01FA04A"
	hash "0x02AC28F3A01FA04A"
	jhash (0xB19B2764)
	arguments {
		float "p0",
	}
	returns	"Any"

native "0x0AE73D8DF3A762B2"
	hash "0x0AE73D8DF3A762B2"
	jhash (0x342FA2B4)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xA51C4B86B71652AE"
	hash "0xA51C4B86B71652AE"
	jhash (0x5D3BFFC9)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	p8 seems to always be false. 
</summary>
]]--
native "0x312342E1A4874F3F"
	hash "0x312342E1A4874F3F"
	jhash (0xD9653728)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		BOOL "p8",
	}
	returns	"void"

native "0x2485D34E50A22E84"
	hash "0x2485D34E50A22E84"
	jhash (0x72BA8A14)
	arguments {
		float "p0",

		float "p1",

		float "p2",
	}
	returns	"void"

native "0x12995F2E53FFA601"
	hash "0x12995F2E53FFA601"
	jhash (0x804F444C)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",

		Any "p9",

		Any "p10",

		Any "p11",
	}
	returns	"void"

native "0xDBAA5EC848BA2D46"
	hash "0xDBAA5EC848BA2D46"
	jhash (0xBB1A1294)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xC0416B061F2B7E5E"
	hash "0xC0416B061F2B7E5E"
	jhash (0x1A1A72EF)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	12 matches across 4 scripts. All 4 scripts were job creators.

	type ranged from 0 - 2.
	p4 was always 0.2f. Likely scale.
	assuming p5 - p8 is RGBA, the graphic is always yellow (255, 255, 0, 255).

	Tested but noticed nothing.


</summary>
]]--
native "0xB1BB03742917A5D6"
	hash "0xB1BB03742917A5D6"
	jhash (0x3BB12B75)
	arguments {
		int "type",

		float "x",

		float "y",

		float "z",

		float "p4",

		int "r",

		int "g",

		int "b",

		int "a",
	}
	returns	"void"

--[[!
<summary>
	Only appeared in Golf &amp; Golf_mp. Parameters were all ptrs. 
</summary>
]]--
native "0x9CFDD90B2B844BF7"
	hash "0x9CFDD90B2B844BF7"
	jhash (0x4EA70FB4)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"void"

native "0x06F761EA47C1D3ED"
	hash "0x06F761EA47C1D3ED"
	jhash (0x0D830DC7)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xA4819F5E23E2FFAD"
	hash "0xA4819F5E23E2FFAD"
	jhash (0xA08B46AD)
	returns	"Any"

native "0xA4664972A9B8F8BA"
	hash "0xA4664972A9B8F8BA"
	jhash (0xECD470F0)
	arguments {
		Any "p0",
	}
	returns	"int"

--[[!
<summary>
	Toggles Heatvision on/off.
</summary>
]]--
native "SET_SEETHROUGH"
	hash "0x7E08924259E08CE0"
	jhash (0x74D4995C)
	arguments {
		BOOL "Toggle",
	}
	returns	"void"

--[[!
<summary>
	Returns whether or not SEETHROUGH is active.
</summary>
]]--
native "_IS_SEETHROUGH_ACTIVE"
	hash "0x44B80ABAB9D80BD3"
	jhash (0x1FE547F2)
	returns	"BOOL"

native "0xD7D0B00177485411"
	hash "0xD7D0B00177485411"
	jhash (0x654F0287)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0xB3C641F3630BF6DA"
	hash "0xB3C641F3630BF6DA"
	jhash (0xF6B837F0)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xE59343E9E96529E7"
	hash "0xE59343E9E96529E7"
	jhash (0xD906A3A9)
	returns	"Any"

native "0xE63D7C6EECECB66B"
	hash "0xE63D7C6EECECB66B"
	jhash (0xD34A6CBA)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xE3E2C1B4C59DBC77"
	hash "0xE3E2C1B4C59DBC77"
	jhash (0xD8CC7221)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	time in ms to transition to fully blurred screen
</summary>
]]--
native "_TRANSITION_TO_BLURRED"
	hash "0xA328A24AAA6B7FDC"
	jhash (0x5604B890)
	arguments {
		float "transitionTime",
	}
	returns	"BOOL"

--[[!
<summary>
	time in ms to transition from fully blurred to normal
</summary>
]]--
native "_TRANSITION_FROM_BLURRED"
	hash "0xEFACC8AEF94430D5"
	jhash (0x46617502)
	arguments {
		float "transitionTime",
	}
	returns	"BOOL"

native "0xDE81239437E8C5A8"
	hash "0xDE81239437E8C5A8"
	jhash (0xDB7AECDA)
	returns	"void"

native "IS_PARTICLE_FX_DELAYED_BLINK"
	hash "0x5CCABFFCA31DDE33"
	jhash (0xEA432A94)
	returns	"float"

native "0x7B226C785A52A0A9"
	hash "0x7B226C785A52A0A9"
	jhash (0x926B8734)
	returns	"Any"

--[[!
<summary>
	doesn't render main cam if false
</summary>
]]--
native "_ENABLE_GAMEPLAY_CAM"
	hash "0xDFC252D8A3E15AB7"
	jhash (0x30ADE541)
	arguments {
		BOOL "enabled",
	}
	returns	"void"

native "0xEB3DAC2C86001E5E"
	hash "0xEB3DAC2C86001E5E"
	returns	"Any"

native "0xE1C8709406F2C41C"
	hash "0xE1C8709406F2C41C"
	jhash (0x0113EAE4)
	returns	"void"

native "0x851CD923176EBA7C"
	hash "0x851CD923176EBA7C"
	jhash (0xDCBA251B)
	returns	"void"

--[[!
<summary>
	Every p2 - p5 occurrence was 0f. 
</summary>
]]--
native "0xBA3D65906822BED5"
	hash "0xBA3D65906822BED5"
	jhash (0x513D444B)
	arguments {
		BOOL "p0",

		BOOL "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"void"

native "0x7AC24EAB6D74118D"
	hash "0x7AC24EAB6D74118D"
	jhash (0xB2410EAB)
	arguments {
		BOOL "p0",
	}
	returns	"BOOL"

native "0xBCEDB009461DA156"
	hash "0xBCEDB009461DA156"
	jhash (0x5AB94128)
	returns	"Any"

native "0x27FEB5254759CDE3"
	hash "0x27FEB5254759CDE3"
	jhash (0xD63FCB3E)
	arguments {
		charPtr "textureDict",

		BOOL "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	GRAPHICS::START_PARTICLE_FX_NON_LOOPED_AT_COORD("scr_paleto_roof_impact", -140.8576f, 6420.789f, 41.1391f, 0f, 0f, 267.3957f, 0x3F800000, 0, 0, 0);

	p8-10 are normally false
</summary>
]]--
native "START_PARTICLE_FX_NON_LOOPED_AT_COORD"
	hash "0x25129531F77B9ED3"
	jhash (0xDD79D679)
	arguments {
		charPtr "effectName",

		float "x",

		float "y",

		float "z",

		float "xRot",

		float "yRot",

		float "zRot",

		float "scale",

		BOOL "p8",

		BOOL "p9",

		BOOL "p10",
	}
	returns	"BOOL"

--[[!
<summary>
	Used for fireworks and stuff lel.
	jenkins hash = 0x633F8C48
</summary>
]]--
native "_START_PARTICLE_FX_NON_LOOPED_AT_COORD_2"
	hash "0xF56B8137DF10135D"
	arguments {
		charPtr "effectName",

		float "x",

		float "y",

		float "z",

		float "xRot",

		float "yRot",

		float "zRot",

		float "scale",

		BOOL "p8",

		BOOL "p9",

		BOOL "p10",
	}
	returns	"BOOL"

--[[!
<summary>
	GRAPHICS::START_PARTICLE_FX_NON_LOOPED_ON_PED_BONE("scr_sh_bong_smoke", PLAYER::PLAYER_PED_ID(), -0.025f, 0.13f, 0f, 0f, 0f, 0f, 31086, 0x3F800000, 0, 0, 0);

	p10-12 are normally false
</summary>
]]--
native "START_PARTICLE_FX_NON_LOOPED_ON_PED_BONE"
	hash "0x0E7E72961BA18619"
	jhash (0x53DAEF4E)
	arguments {
		charPtr "effectName",

		Ped "ped",

		float "xOffset",

		float "yOffset",

		float "zOffset",

		float "xRot",

		float "yRot",

		float "zRot",

		int "boneIndex",

		float "scale",

		BOOL "p10",

		BOOL "p11",

		BOOL "p12",
	}
	returns	"BOOL"

--[[!
<summary>
	GRAPHICS::_A41B6A43642AC2CF("scr_sh_bong_smoke", PLAYER::PLAYER_PED_ID(), -0.025, 0.13, 0.0, 0.0, 0.0, 0.0, 31086, 0x3f800000, 0, 0, 0);

	p10-12 are normally 0/false
</summary>
]]--
native "_START_PARTICLE_FX_NON_LOOPED_ON_PED_BONE_2"
	hash "0xA41B6A43642AC2CF"
	jhash (0x161780C1)
	arguments {
		charPtr "effectName",

		Ped "ped",

		float "xOffset",

		float "yOffset",

		float "zOffset",

		float "xRot",

		float "yRot",

		float "zRot",

		int "boneIndex",

		float "scale",

		BOOL "p10",

		BOOL "p11",

		BOOL "p12",
	}
	returns	"BOOL"

--[[!
<summary>
	Starts a particle effect on an entity for example your player.
	List of Particle Effects: http://pastebin.com/s6b3CmAy

	Example:
	C#:
	Function.Call(Hash.REQUEST_NAMED_PTFX_ASSET("scr_rcbarry2");                     Function.Call(Hash._SET_PTFX_ASSET_NEXT_CALL("scr_rcbarry2");                             Function.Call(Hash.START_PARTICLE_FX_NON_LOOPED_ON_ENTITY, "scr_clown_appears", Game.Player.Character, 0.0, 0.0, -0.5, 0.0, 0.0, 0.0, 1.0, false, false, false);

	C++:
	STREAMING::REQUEST_NAMED_PTFX_ASSET("scr_rcbarry2");                                    GRAPHICS::_SET_PTFX_ASSET_NEXT_CALL("scr_rcbarry2");                                          GRAPHICS::START_PARTICLE_FX_NON_LOOPED_ON_ENTITY("scr_clown_appears", PLAYER::PLAYER_PED_ID(), 0.0, 0.0, -0.5, 0.0, 0.0, 0.0, 1.0, false, false, false);

	The code above starts a purple puff-like effect.

	~ISOFX
</summary>
]]--
native "START_PARTICLE_FX_NON_LOOPED_ON_ENTITY"
	hash "0x0D53A3B8DA0809D2"
	jhash (0x9604DAD4)
	arguments {
		charPtr "effectName",

		Entity "entity",

		float "xOffset",

		float "yOffset",

		float "zOffset",

		float "xRot",

		float "yRot",

		float "zRot",

		float "scale",

		BOOL "p9",

		BOOL "p10",

		BOOL "p11",
	}
	returns	"BOOL"

--[[!
<summary>
	Console Hash: 0x469A2B4A
</summary>
]]--
native "_START_PARTICLE_FX_NON_LOOPED_ON_ENTITY_2"
	hash "0xC95EB1DB6E92113D"
	arguments {
		charPtr "effectName",

		Entity "entity",

		float "xOffset",

		float "yOffset",

		float "zOffset",

		float "xRot",

		float "yRot",

		float "zRot",

		float "scale",

		BOOL "p9",

		BOOL "p10",

		BOOL "p11",
	}
	returns	"BOOL"

native "SET_PARTICLE_FX_NON_LOOPED_COLOUR"
	hash "0x26143A59EF48B262"
	jhash (0x7B689E20)
	arguments {
		float "r",

		float "g",

		float "b",
	}
	returns	"void"

native "SET_PARTICLE_FX_NON_LOOPED_ALPHA"
	hash "0x77168D722C58B2FC"
	jhash (0x497EAFF2)
	arguments {
		float "alpha",
	}
	returns	"void"

native "0x8CDE909A0370BB3A"
	hash "0x8CDE909A0370BB3A"
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	GRAPHICS::START_PARTICLE_FX_LOOPED_AT_COORD("scr_fbi_falling_debris", 93.7743f, -749.4572f, 70.86904f, 0f, 0f, 0f, 0x3F800000, 0, 0, 0, 0)


</summary>
]]--
native "START_PARTICLE_FX_LOOPED_AT_COORD"
	hash "0xE184F4F0DC5910E7"
	jhash (0xD348E3E6)
	arguments {
		charPtr "effectName",

		float "x",

		float "y",

		float "z",

		float "xRot",

		float "yRot",

		float "zRot",

		float "scale",

		BOOL "p8",

		BOOL "p9",

		BOOL "p10",

		BOOL "p11",
	}
	returns	"Any"

native "START_PARTICLE_FX_LOOPED_ON_PED_BONE"
	hash "0xF28DA9F38CD1787C"
	jhash (0xF8FC196F)
	arguments {
		charPtr "effectName",

		Ped "ped",

		float "xOffset",

		float "yOffset",

		float "zOffset",

		float "xRot",

		float "yRot",

		float "zRot",

		int "boneIndex",

		float "scale",

		BOOL "p10",

		BOOL "p11",

		BOOL "p12",
	}
	returns	"Any"

native "START_PARTICLE_FX_LOOPED_ON_ENTITY"
	hash "0x1AE42C1660FD6517"
	jhash (0x0D06FF62)
	arguments {
		charPtr "effectName",

		Entity "entity",

		float "xOffset",

		float "yOffset",

		float "zOffset",

		float "xRot",

		float "yRot",

		float "zRot",

		float "scale",

		BOOL "p9",

		BOOL "p10",

		BOOL "p11",
	}
	returns	"Any"

native "_START_PARTICLE_FX_LOOPED_ON_ENTITY_BONE"
	hash "0xC6EB449E33977F0B"
	arguments {
		charPtr "effectName",

		Entity "entity",

		float "xOffset",

		float "yOffset",

		float "zOffset",

		float "xRot",

		float "yRot",

		float "zRot",

		int "boneIndex",

		float "scale",

		BOOL "p10",

		BOOL "p11",

		BOOL "p12",
	}
	returns	"Any"

native "0x6F60E89A7B64EE1D"
	hash "0x6F60E89A7B64EE1D"
	jhash (0x110752B2)
	arguments {
		charPtr "effectName",

		Entity "entity",

		float "xOffset",

		float "yOffset",

		float "zOffset",

		float "xRot",

		float "yRot",

		float "zRot",

		float "scale",

		BOOL "p9",

		BOOL "p10",

		BOOL "p11",
	}
	returns	"Any"

native "0xDDE23F30CC5A0F03"
	hash "0xDDE23F30CC5A0F03"
	arguments {
		charPtr "effectName",

		Entity "entity",

		float "xOffset",

		float "yOffset",

		float "zOffset",

		float "xRot",

		float "yRot",

		float "zRot",

		int "boneIndex",

		float "scale",

		BOOL "p10",

		BOOL "p11",

		BOOL "p12",
	}
	returns	"Any"

native "STOP_PARTICLE_FX_LOOPED"
	hash "0x8F75998877616996"
	jhash (0xD245455B)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "REMOVE_PARTICLE_FX"
	hash "0xC401503DFE8D53CF"
	jhash (0x6BA48C7E)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "REMOVE_PARTICLE_FX_FROM_ENTITY"
	hash "0xB8FEAEEBCC127425"
	jhash (0xCEDE52E9)
	arguments {
		Any "p0",
	}
	returns	"void"

native "REMOVE_PARTICLE_FX_IN_RANGE"
	hash "0xDD19FA1C6D657305"
	jhash (0x7EB8F275)
	arguments {
		float "X",

		float "Y",

		float "Z",

		float "radius",
	}
	returns	"void"

native "DOES_PARTICLE_FX_LOOPED_EXIST"
	hash "0x74AFEF0D2E1E409B"
	jhash (0xCBF91D2A)
	arguments {
		Any "ptfxHandle",
	}
	returns	"BOOL"

native "SET_PARTICLE_FX_LOOPED_OFFSETS"
	hash "0xF7DDEBEC43483C43"
	jhash (0x641F7790)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"void"

native "SET_PARTICLE_FX_LOOPED_EVOLUTION"
	hash "0x5F0C4B5B1C393BE2"
	jhash (0x1CBC1373)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "SET_PARTICLE_FX_LOOPED_COLOUR"
	hash "0x7F8F65877F88783B"
	jhash (0x5219D530)
	arguments {
		Any "ptfxHandle",

		float "r",

		float "g",

		float "b",

		BOOL "p4",
	}
	returns	"void"

native "SET_PARTICLE_FX_LOOPED_ALPHA"
	hash "0x726845132380142E"
	jhash (0x5ED49BE1)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "SET_PARTICLE_FX_LOOPED_SCALE"
	hash "0xB44250AAA456492D"
	jhash (0x099B8B49)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "_SET_PARTICLE_FX_LOOPED_RANGE"
	hash "0xDCB194B85EF7B541"
	jhash (0x233DE879)
	arguments {
		Any "ptfxHandle",

		float "range",
	}
	returns	"void"

native "SET_PARTICLE_FX_CAM_INSIDE_VEHICLE"
	hash "0xEEC4047028426510"
	jhash (0x19EC0001)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_PARTICLE_FX_CAM_INSIDE_NONPLAYER_VEHICLE"
	hash "0xACEE6F360FC1F6B6"
	jhash (0x6B125A02)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_PARTICLE_FX_SHOOTOUT_BOAT"
	hash "0x96EF97DAEB89BEF5"
	jhash (0xD938DEE0)
	arguments {
		Any "p0",
	}
	returns	"void"

native "SET_PARTICLE_FX_BLOOD_SCALE"
	hash "0x5F6DF3D92271E8A1"
	jhash (0x18136DE0)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_CAMERA_ENDTIME"
	hash "0xD821490579791273"
	jhash (0xC61C75E9)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x9DCE1F0F78260875"
	hash "0x9DCE1F0F78260875"
	jhash (0xCE8B8748)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x27E32866E9A5C416"
	hash "0x27E32866E9A5C416"
	arguments {
		float "p0",
	}
	returns	"void"

native "0xBB90E12CAC1DAB25"
	hash "0xBB90E12CAC1DAB25"
	arguments {
		float "p0",
	}
	returns	"void"

native "0xCA4AE345A153D573"
	hash "0xCA4AE345A153D573"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x54E22EA2C1956A8D"
	hash "0x54E22EA2C1956A8D"
	arguments {
		float "p0",
	}
	returns	"void"

native "0x949F397A288B28B3"
	hash "0x949F397A288B28B3"
	arguments {
		float "p0",
	}
	returns	"void"

native "0x9B079E5221D984D3"
	hash "0x9B079E5221D984D3"
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	LastGenConsole = 0x9C720B61
</summary>
]]--
native "_SET_PTFX_ASSET_NEXT_CALL"
	hash "0x6C38AF3693A69A91"
	arguments {
		charPtr "name",
	}
	returns	"void"

native "_SET_PTFX_ASSET_OLD_2_NEW"
	hash "0xEA1E2D93F6F75ED9"
	arguments {
		charPtr "Old",

		charPtr "New",
	}
	returns	"void"

native "0x89C8553DD3274AAE"
	hash "0x89C8553DD3274AAE"
	arguments {
		charPtr "name",
	}
	returns	"void"

native "0xA46B73FAA3460AE1"
	hash "0xA46B73FAA3460AE1"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xF78B803082D4386F"
	hash "0xF78B803082D4386F"
	arguments {
		float "p0",
	}
	returns	"void"

native "WASH_DECALS_IN_RANGE"
	hash "0x9C30613D50A6ADEF"
	jhash (0xDEECBC57)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "WASH_DECALS_FROM_VEHICLE"
	hash "0x5B712761429DBC14"
	jhash (0x2929F11A)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "FADE_DECALS_IN_RANGE"
	hash "0xD77EDADB0420E6E0"
	jhash (0xF81E884A)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "REMOVE_DECALS_IN_RANGE"
	hash "0x5D6B2D4830A67C62"
	jhash (0x06A619A0)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "REMOVE_DECALS_FROM_OBJECT"
	hash "0xCCF71CBDDF5B6CB9"
	jhash (0x8B67DCA7)
	arguments {
		Object "obj",
	}
	returns	"void"

native "REMOVE_DECALS_FROM_OBJECT_FACING"
	hash "0xA6F6F70FDC6D144C"
	jhash (0xF4999A55)
	arguments {
		Object "obj",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "REMOVE_DECALS_FROM_VEHICLE"
	hash "0xE91F1B65F2B48D57"
	jhash (0x831D06CA)
	arguments {
		Vehicle "vehicle",
	}
	returns	"void"

native "ADD_DECAL"
	hash "0xB302244A1839BDAD"
	jhash (0xEAD0C412)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",

		float "p11",

		float "p12",

		float "p13",

		float "p14",

		float "p15",

		float "p16",

		BOOL "p17",

		BOOL "p18",

		BOOL "p19",
	}
	returns	"Any"

native "ADD_PETROL_DECAL"
	hash "0x4F5212C7AD880DF8"
	jhash (0x1259DF42)
	arguments {
		float "x",

		float "y",

		float "z",

		float "Groundlvl",

		float "Width",

		float "Transparency",
	}
	returns	"Any"

native "0x99AC7F0D8B9C893D"
	hash "0x99AC7F0D8B9C893D"
	jhash (0xE3938B0B)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x967278682CB6967A"
	hash "0x967278682CB6967A"
	jhash (0xBAEC6ADD)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x0A123435A26C36CD"
	hash "0x0A123435A26C36CD"
	jhash (0xCCCA6855)
	returns	"void"

native "REMOVE_DECAL"
	hash "0xED3F346429CCD659"
	jhash (0xA4363188)
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_DECAL_ALIVE"
	hash "0xC694D74949CAFD0C"
	jhash (0xCDD4A61A)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "GET_DECAL_WASH_LEVEL"
	hash "0x323F647679A09103"
	jhash (0x054448EF)
	arguments {
		Any "p0",
	}
	returns	"float"

native "0xD9454B5752C857DC"
	hash "0xD9454B5752C857DC"
	jhash (0xEAB6417C)
	returns	"void"

native "0x27CFB1B1E078CB2D"
	hash "0x27CFB1B1E078CB2D"
	jhash (0xC2703B88)
	returns	"void"

native "0x4B5CFC83122DF602"
	hash "0x4B5CFC83122DF602"
	jhash (0xA706E84D)
	returns	"void"

native "0x2F09F7976C512404"
	hash "0x2F09F7976C512404"
	jhash (0x242C6A04)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"BOOL"

native "0x8A35C742130C6080"
	hash "0x8A35C742130C6080"
	jhash (0x335695CF)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0xB7ED70C49521A61D"
	hash "0xB7ED70C49521A61D"
	jhash (0x7B786555)
	arguments {
		Any "p0",
	}
	returns	"void"

native "MOVE_VEHICLE_DECALS"
	hash "0x84C8D7C2D30D3280"
	jhash (0xCE9E6CF2)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	Now has 15 parameters, previous declaration:
	BOOL _0x428BDCB9DA58DA53(Any p0, Any p1, Any p2, float p3, float p4, float p5, float p6, float p7, float p8, float p9, float p10, float p11, float p12, Any p13)

	boneIndex is always chassis_dummy in the scripts. The x/y/z params are location relative to the chassis bone. They are usually rotations and measurements. Haven't reversed which are what yet.

	Scale is how big the decal will be.

	p13 is always 0.

	For alpha, 200 seems to match what the game is doing, I think. I don't have access to the new scripts to see what this parameter is, but based on guessing this seems (kind of) accurate.
</summary>
]]--
native "_ADD_CLAN_DECAL_TO_VEHICLE"
	hash "0x428BDCB9DA58DA53"
	jhash (0x12077738)
	arguments {
		Vehicle "vehicle",

		Ped "ped",

		int "boneIndex",

		float "x1",

		float "x2",

		float "x3",

		float "y1",

		float "y2",

		float "y3",

		float "z1",

		float "z2",

		float "z3",

		float "scale",

		Any "p13",

		int "alpha",
	}
	returns	"BOOL"

native "0xD2300034310557E4"
	hash "0xD2300034310557E4"
	jhash (0x667046A8)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xFE26117A5841B2FF"
	hash "0xFE26117A5841B2FF"
	jhash (0x4F4D76E8)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

--[[!
<summary>
	This function is called before ADD_CLAN_DECAL_TO_VEHICLE to see if it needs to run. IDK if it's for clan decal or not, but the 2nd parameter might be decal index? It's always passed 0. Not sure what this function really does. But it does return 0 if the clan tag is not on, and 1 if it is.
</summary>
]]--
native "_HAS_VEHICLE_GOT_DECAL"
	hash "0x060D935D3981A275"
	jhash (0x6D58F73B)
	arguments {
		Vehicle "vehicle",

		Any "p1",
	}
	returns	"BOOL"

native "0x0E4299C549F0D1F1"
	hash "0x0E4299C549F0D1F1"
	jhash (0x9BABCBA4)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x02369D5C8A51FDCF"
	hash "0x02369D5C8A51FDCF"
	jhash (0xFDF6D8DA)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x46D1A61A21F566FC"
	hash "0x46D1A61A21F566FC"
	jhash (0x2056A015)
	arguments {
		float "p0",
	}
	returns	"void"

native "0x2A2A52824DB96700"
	hash "0x2A2A52824DB96700"
	jhash (0x0F486429)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x1600FD8CF72EBC12"
	hash "0x1600FD8CF72EBC12"
	jhash (0xD87CC710)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xEFB55E7C25D3B3BE"
	hash "0xEFB55E7C25D3B3BE"
	jhash (0xE29EE145)
	returns	"void"

native "0xA44FF770DFBC5DAE"
	hash "0xA44FF770DFBC5DAE"
	returns	"void"

native "DISABLE_VEHICLE_DISTANTLIGHTS"
	hash "0xC9F98AC1884E73A2"
	jhash (0x7CFAE36F)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0x03300B57FCAC6DDB"
	hash "0x03300B57FCAC6DDB"
	jhash (0x60F72371)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x98EDF76A7271E4F2"
	hash "0x98EDF76A7271E4F2"
	returns	"void"

--[[!
<summary>
	Forces footstep tracks on all surfaces.
</summary>
]]--
native "_SET_FORCE_PED_FOOTSTEPS_TRACKS"
	hash "0xAEEDAD1420C65CC0"
	arguments {
		BOOL "enabled",
	}
	returns	"void"

--[[!
<summary>
	Forces vehicle trails on all surfaces.
</summary>
]]--
native "_SET_FORCE_VEHICLE_TRAILS"
	hash "0x4CC7F0FEA5283FE0"
	arguments {
		BOOL "enabled",
	}
	returns	"void"

native "0xD7021272EB0A451E"
	hash "0xD7021272EB0A451E"
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

--[[!
<summary>
	Loads the specified timecycle modifier. Modifiers are defined separately in another file (e.g. "timecycle_mods_1.xml")

	Parameters:
	modifierName - The modifier to load (e.g. "V_FIB_IT3", "scanline_cam", etc.)
</summary>
]]--
native "SET_TIMECYCLE_MODIFIER"
	hash "0x2C933ABF17A1DF41"
	jhash (0xA81F3638)
	arguments {
		charPtr "modifierName",
	}
	returns	"void"

native "SET_TIMECYCLE_MODIFIER_STRENGTH"
	hash "0x82E7FFCD5B2326B3"
	jhash (0x458F4F45)
	arguments {
		float "p0",
	}
	returns	"void"

native "SET_TRANSITION_TIMECYCLE_MODIFIER"
	hash "0x3BCF567485E1971C"
	jhash (0xBB2BA72A)
	arguments {
		AnyPtr "p0",

		float "p1",
	}
	returns	"void"

native "0x1CBA05AE7BD7EE05"
	hash "0x1CBA05AE7BD7EE05"
	jhash (0x56345F6B)
	arguments {
		float "p0",
	}
	returns	"void"

native "CLEAR_TIMECYCLE_MODIFIER"
	hash "0x0F07E7745A236711"
	jhash (0x8D8DF8EE)
	returns	"void"

native "GET_TIMECYCLE_MODIFIER_INDEX"
	hash "0xFDF3D97C674AFB66"
	jhash (0x594FEEC4)
	returns	"Any"

native "0x459FD2C8D0AB78BC"
	hash "0x459FD2C8D0AB78BC"
	jhash (0x03C44E4B)
	returns	"Any"

native "PUSH_TIMECYCLE_MODIFIER"
	hash "0x58F735290861E6B4"
	jhash (0x7E082045)
	returns	"void"

native "POP_TIMECYCLE_MODIFIER"
	hash "0x3C8938D7D872211E"
	jhash (0x79D7D235)
	returns	"void"

native "0xBBF327DED94E4DEB"
	hash "0xBBF327DED94E4DEB"
	jhash (0x85BA15A4)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xBDEB86F4D5809204"
	hash "0xBDEB86F4D5809204"
	jhash (0x9559BB38)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xBF59707B3E5ED531"
	hash "0xBF59707B3E5ED531"
	jhash (0x554BA16E)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x1A8E2C8B9CF4549C"
	hash "0x1A8E2C8B9CF4549C"
	jhash (0xE8F538B5)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x15E33297C3E8DC60"
	hash "0x15E33297C3E8DC60"
	jhash (0x805BAB08)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x5096FD9CCB49056D"
	hash "0x5096FD9CCB49056D"
	jhash (0x908A335E)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x92CCC17A7A2285DA"
	hash "0x92CCC17A7A2285DA"
	jhash (0x6776720A)
	returns	"void"

native "0xBB0527EC6341496D"
	hash "0xBB0527EC6341496D"
	returns	"Any"

native "0x2C328AF17210F009"
	hash "0x2C328AF17210F009"
	arguments {
		float "p0",
	}
	returns	"void"

native "0x2BF72AD5B41AA739"
	hash "0x2BF72AD5B41AA739"
	returns	"void"

native "REQUEST_SCALEFORM_MOVIE"
	hash "0x11FE353CF9733E6F"
	jhash (0xC67E3DCB)
	arguments {
		charPtr "scaleformName",
	}
	returns	"Any"

native "REQUEST_SCALEFORM_MOVIE_INSTANCE"
	hash "0xC514489CFB8AF806"
	jhash (0x7CC8057D)
	arguments {
		charPtr "scaleformName",
	}
	returns	"Any"

--[[!
<summary>
	Similar to REQUEST_SCALEFORM_MOVIE, but seems to be some kind of "interactive" scaleform movie?

	These seem to be the only scaleforms ever requested by this native:
	"breaking_news"
	"desktop_pc"
	"ECG_MONITOR"
	"Hacking_PC"
	"TEETH_PULLING"



</summary>
]]--
native "_REQUEST_SCALEFORM_MOVIE3"
	hash "0xBD06C611BB9048C2"
	arguments {
		charPtr "scaleformName",
	}
	returns	"Any"

native "HAS_SCALEFORM_MOVIE_LOADED"
	hash "0x85F01B8D5B90570E"
	jhash (0xDDFB6448)
	arguments {
		int "scaleform",
	}
	returns	"BOOL"

native "0x0C1C5D756FB5F337"
	hash "0x0C1C5D756FB5F337"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "HAS_SCALEFORM_CONTAINER_MOVIE_LOADED_INTO_PARENT"
	hash "0x8217150E1217EBFD"
	jhash (0x1DFE8D8A)
	arguments {
		int "handle",
	}
	returns	"BOOL"

native "SET_SCALEFORM_MOVIE_AS_NO_LONGER_NEEDED"
	hash "0x1D132D614DD86811"
	jhash (0x5FED3BA1)
	arguments {
		intPtr "scaleformPtr",
	}
	returns	"void"

native "SET_SCALEFORM_MOVIE_TO_USE_SYSTEM_TIME"
	hash "0x6D8EB211944DCE08"
	jhash (0x18C9DE8D)
	arguments {
		int "p0",

		BOOL "p1",
	}
	returns	"void"

native "DRAW_SCALEFORM_MOVIE"
	hash "0x54972ADAF0294A93"
	jhash (0x48DA6A58)
	arguments {
		int "handle",

		float "x",

		float "y",

		float "width",

		float "height",

		int "r",

		int "g",

		int "b",

		int "a",
	}
	returns	"void"

native "DRAW_SCALEFORM_MOVIE_FULLSCREEN"
	hash "0x0DF606929C105BE1"
	jhash (0x7B48E696)
	arguments {
		int "scaleform",

		int "r",

		int "g",

		int "b",

		int "a",
	}
	returns	"void"

native "DRAW_SCALEFORM_MOVIE_FULLSCREEN_MASKED"
	hash "0xCF537FDE4FBD4CE5"
	jhash (0x9C59FC06)
	arguments {
		int "scaleform1",

		int "scaleform2",

		int "red",

		int "green",

		int "blue",

		int "alpha",
	}
	returns	"void"

native "0x87D51D72255D4E78"
	hash "0x87D51D72255D4E78"
	jhash (0xC4F63A89)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",

		float "p11",

		float "p12",

		Any "p13",
	}
	returns	"void"

native "0x1CE592FDC749D6F5"
	hash "0x1CE592FDC749D6F5"
	jhash (0x899933C8)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",

		float "p11",

		float "p12",

		Any "p13",
	}
	returns	"void"

--[[!
<summary>
	Calls the Scaleform function.
</summary>
]]--
native "_CALL_SCALEFORM_MOVIE_FUNCTION_VOID"
	hash "0xFBD96D87AC96D533"
	jhash (0x7AB77B57)
	arguments {
		int "scaleform",

		charPtr "functionName",
	}
	returns	"void"

--[[!
<summary>
	Calls the Scaleform function and passes the parameters as floats.

	The number of parameters passed to the function varies, so the end of the parameter list is represented by -1.0.
</summary>
]]--
native "_CALL_SCALEFORM_MOVIE_FUNCTION_FLOAT_PARAMS"
	hash "0xD0837058AE2E4BEE"
	jhash (0x557EDA1D)
	arguments {
		int "scaleform",

		charPtr "functionName",

		float "param1",

		float "param2",

		float "param3",

		float "param4",

		float "param5",
	}
	returns	"void"

--[[!
<summary>
	Calls the Scaleform function and passes the parameters as strings.

	The number of parameters passed to the function varies, so the end of the parameter list is represented by 0 (NULL).
</summary>
]]--
native "_CALL_SCALEFORM_MOVIE_FUNCTION_STRING_PARAMS"
	hash "0x51BC1ED3CC44E8F7"
	jhash (0x91A7FCEB)
	arguments {
		int "scaleform",

		charPtr "functionName",

		charPtr "param1",

		charPtr "param2",

		charPtr "param3",

		charPtr "param4",

		charPtr "param5",
	}
	returns	"void"

--[[!
<summary>
	Calls the Scaleform function and passes both float and string parameters (in their respective order).

	The number of parameters passed to the function varies, so the end of the float parameters is represented by -1.0, and the end of the string parameters is represented by 0 (NULL).

	NOTE: The order of parameters in the function prototype is important! All float parameters must come first, followed by the string parameters.

	Examples:
	// function MY_FUNCTION(floatParam1, floatParam2, stringParam)
	GRAPHICS::_CALL_SCALEFORM_MOVIE_FUNCTION_MIXED_PARAMS(scaleform, "MY_FUNCTION", 10.0, 20.0, -1.0, -1.0, -1.0, "String param", 0, 0, 0, 0);

	// function MY_FUNCTION_2(floatParam, stringParam1, stringParam2)
	GRAPHICS::_CALL_SCALEFORM_MOVIE_FUNCTION_MIXED_PARAMS(scaleform, "MY_FUNCTION_2", 10.0, -1.0, -1.0, -1.0, -1.0, "String param #1", "String param #2", 0, 0, 0);
</summary>
]]--
native "_CALL_SCALEFORM_MOVIE_FUNCTION_MIXED_PARAMS"
	hash "0xEF662D8D57E290B1"
	jhash (0x6EAF56DE)
	arguments {
		int "scaleform",

		charPtr "functionName",

		float "floatParam1",

		float "floatParam2",

		float "floatParam3",

		float "floatParam4",

		float "floatParam5",

		charPtr "stringParam1",

		charPtr "stringParam2",

		charPtr "stringParam3",

		charPtr "stringParam4",

		charPtr "stringParam5",
	}
	returns	"void"

--[[!
<summary>
	Pushes a function from the Hud component Scaleform onto the stack. Same behavior as GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION, just a hud component id instead of a Scaleform.

	Known components:
	19 
	20 

	This native requires more research - all information can be found inside of 'hud.gfx'. Using a decompiler, the different components are located under "scripts\__Packages\com\rockstargames\gtav\hud\hudComponents" and "scripts\__Packages\com\rockstargames\gtav\Multiplayer". -CL69
</summary>
]]--
native "_PUSH_SCALEFORM_MOVIE_FUNCTION_FROM_HUD_COMPONENT"
	hash "0x98C494FD5BDFBFD5"
	jhash (0x5D66CE1E)
	arguments {
		int "hudComponent",

		charPtr "functionName",
	}
	returns	"BOOL"

--[[!
<summary>
	Push a function from the Scaleform onto the stack

</summary>
]]--
native "_PUSH_SCALEFORM_MOVIE_FUNCTION"
	hash "0xF6E48914C7A8694E"
	jhash (0x215ABBE8)
	arguments {
		int "scaleform",

		charPtr "functionName",
	}
	returns	"BOOL"

--[[!
<summary>
	Possibly calls "global" Scaleform functions - needs more research!
</summary>
]]--
native "0xAB58C27C2E6123C6"
	hash "0xAB58C27C2E6123C6"
	jhash (0xF6015178)
	arguments {
		charPtr "functionName",
	}
	returns	"BOOL"

--[[!
<summary>
	Is global Scaleform function valid?
</summary>
]]--
native "0xB9449845F73F5E9C"
	hash "0xB9449845F73F5E9C"
	jhash (0x5E219B67)
	arguments {
		charPtr "functionName",
	}
	returns	"BOOL"

--[[!
<summary>
	Pops and calls the Scaleform function on the stack
</summary>
]]--
native "_POP_SCALEFORM_MOVIE_FUNCTION_VOID"
	hash "0xC6796A8FFA375E53"
	jhash (0x02DBF2D7)
	returns	"void"

--[[!
<summary>
	Pops and calls the Scaleform movie on the stack. Returns data from the function (not sure if this is a string).
</summary>
]]--
native "_POP_SCALEFORM_MOVIE_FUNCTION"
	hash "0xC50AA39A577AF886"
	jhash (0x2F38B526)
	returns	"Any"

--[[!
<summary>
	Seems to take data that is returned from "_POP_SCALEFORM_MOVIE_FUNCTION" and checks to see if it's not null/empty.

	"agency_heist3b.ysc", line 71836:
	if (CONTROLS::IS_CONTROL_JUST_PRESSED(2, 201) || CONTROLS::IS_CONTROL_JUST_PRESSED(2, 237)) {
	    GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION(l_46, "SET_INPUT_EVENT_SELECT");
	    l_45 = GRAPHICS::_POP_SCALEFORM_MOVIE_FUNCTION();
	}
	if (GRAPHICS::_0x768FF8961BA904D6(l_45)) {
	    v_13 = GRAPHICS::_0x2DE7EFA66B906036(l_45);
	    if (v_13 == 6) {
	        sub_73269(a_0);
	    }
	}

</summary>
]]--
native "0x768FF8961BA904D6"
	hash "0x768FF8961BA904D6"
	jhash (0x5CD7C3C0)
	arguments {
		Any "funcData",
	}
	returns	"BOOL"

--[[!
<summary>
	Needs a bit more research, but it seems to return an int.

	"agency_heist3b.ysc", line 71836:
	if (CONTROLS::IS_CONTROL_JUST_PRESSED(2, 201) || CONTROLS::IS_CONTROL_JUST_PRESSED(2, 237)) {
	    GRAPHICS::_PUSH_SCALEFORM_MOVIE_FUNCTION(l_46, "SET_INPUT_EVENT_SELECT");
	    l_45 = GRAPHICS::_POP_SCALEFORM_MOVIE_FUNCTION();
	}
	if (GRAPHICS::_0x768FF8961BA904D6(l_45)) {
	    v_13 = GRAPHICS::_0x2DE7EFA66B906036(l_45);
	    if (v_13 == 6) {
	        sub_73269(a_0);
	    }
	}
</summary>
]]--
native "0x2DE7EFA66B906036"
	hash "0x2DE7EFA66B906036"
	jhash (0x2CFB0E6D)
	arguments {
		Any "funcData",
	}
	returns	"int"

native "SITTING_TV"
	hash "0xE1E258829A885245"
	jhash (0x516862EB)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	Pushes an integer for the Scaleform function onto the stack.
</summary>
]]--
native "_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_INT"
	hash "0xC3D0841A0CC546A6"
	jhash (0x716777CB)
	arguments {
		int "value",
	}
	returns	"void"

--[[!
<summary>
	Pushes a float for the Scaleform function onto the stack.
</summary>
]]--
native "_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_FLOAT"
	hash "0xD69736AAE04DB51A"
	jhash (0x9A01FFDA)
	arguments {
		float "value",
	}
	returns	"void"

--[[!
<summary>
	Pushes a boolean for the Scaleform function onto the stack.
</summary>
]]--
native "_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_BOOL"
	hash "0xC58424BA936EB458"
	jhash (0x0D4AE8CB)
	arguments {
		BOOL "value",
	}
	returns	"void"

--[[!
<summary>
	Called prior to adding a text component to the UI. After doing so, GRAPHICS::_END_TEXT_COMPONENT is called.

	Examples:
	GRAPHICS::_BEGIN_TEXT_COMPONENT("NUMBER");
	UI::ADD_TEXT_COMPONENT_INTEGER(GAMEPLAY::ABSI(a_1));
	GRAPHICS::_END_TEXT_COMPONENT();

	GRAPHICS::_BEGIN_TEXT_COMPONENT("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(a_2);
	GRAPHICS::_END_TEXT_COMPONENT();

	GRAPHICS::_BEGIN_TEXT_COMPONENT("STRTNM2");
	UI::_0x17299B63C7683A2B(v_3);
	UI::_0x17299B63C7683A2B(v_4);
	GRAPHICS::_END_TEXT_COMPONENT();

	GRAPHICS::_BEGIN_TEXT_COMPONENT("STRTNM1");
	UI::_0x17299B63C7683A2B(v_3);
	GRAPHICS::_END_TEXT_COMPONENT();



	This is _BEGIN_TEXT_COMPONENT
</summary>
]]--
native "_BEGIN_TEXT_COMPONENT"
	hash "0x80338406F3475E55"
	jhash (0x3AC9CB55)
	arguments {
		charPtr "componentType",
	}
	returns	"void"

native "_END_TEXT_COMPONENT"
	hash "0x362E2D3FE93A9959"
	jhash (0x386CE0B8)
	returns	"void"

native "0xAE4E8157D9ECF087"
	hash "0xAE4E8157D9ECF087"
	jhash (0x2E80DB52)
	returns	"void"

native "_PUSH_SCALEFORM_MOVIE_FUNCTION_PARAMETER_STRING"
	hash "0xBA7148484BD90365"
	jhash (0x4DAAD55B)
	arguments {
		charPtr "value",
	}
	returns	"void"

native "0xE83A3E3557A56640"
	hash "0xE83A3E3557A56640"
	jhash (0xCCBF0334)
	arguments {
		charPtr "p0",
	}
	returns	"void"

native "0x5E657EF1099EDD65"
	hash "0x5E657EF1099EDD65"
	jhash (0x91A081A1)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xEC52C631A1831C03"
	hash "0xEC52C631A1831C03"
	jhash (0x83A9811D)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x9304881D6F6537EA"
	hash "0x9304881D6F6537EA"
	jhash (0x7AF85862)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Check to see if hud component Scaleform has loaded?
</summary>
]]--
native "_HAS_HUD_SCALEFORM_LOADED"
	hash "0xDF6E5987D2B4D140"
	jhash (0x79B43255)
	arguments {
		int "hudComponent",
	}
	returns	"BOOL"

native "0xF44A5456AC3F4F97"
	hash "0xF44A5456AC3F4F97"
	jhash (0x03D87600)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xD1C7CB175E012964"
	hash "0xD1C7CB175E012964"
	jhash (0xE9183D3A)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SET_TV_CHANNEL"
	hash "0xBAABBB23EB6E484E"
	jhash (0x41A8A627)
	arguments {
		int "channel",
	}
	returns	"void"

native "GET_TV_CHANNEL"
	hash "0xFC1E275A90D39995"
	jhash (0x6B96145A)
	returns	"int"

native "SET_TV_VOLUME"
	hash "0x2982BF73F66E9DDC"
	jhash (0xF3504F4D)
	arguments {
		float "volume",
	}
	returns	"void"

native "GET_TV_VOLUME"
	hash "0x2170813D3DD8661B"
	jhash (0x39555CF0)
	returns	"float"

--[[!
<summary>
	Draws a TV channel, but the parameters are unknown.

	All calls to this native are preceded by calls to GRAPHICS::_0x61BB1D9B3A95D802 and GRAPHICS::_0xC6372ECD45D73BCD, respectively.

	"act_cinema.ysc", line 1483:
	UI::SET_HUD_COMPONENT_POSITION(15, 0.0, -0.0375);
	UI::SET_TEXT_RENDER_ID(l_AE);
	GRAPHICS::_0x61BB1D9B3A95D802(4);
	GRAPHICS::_0xC6372ECD45D73BCD(1);
	if (GRAPHICS::_0x0AD973CA1E077B60(${movie_arthouse})) {
	    GRAPHICS::DRAW_TV_CHANNEL(0.5, 0.5, 0.7375, 1.0, 0.0, 255, 255, 255, 255);
	} else { 
	    GRAPHICS::DRAW_TV_CHANNEL(0.5, 0.5, 1.0, 1.0, 0.0, 255, 255, 255, 255);
	}

	"am_mp_property_int.ysc", line 102545:
	if (ENTITY::DOES_ENTITY_EXIST(a_2._f3)) {
	    if (UI::IS_NAMED_RENDERTARGET_LINKED(ENTITY::GET_ENTITY_MODEL(a_2._f3))) {
	        UI::SET_TEXT_RENDER_ID(a_2._f1);
	        GRAPHICS::_0x61BB1D9B3A95D802(4);
	        GRAPHICS::_0xC6372ECD45D73BCD(1);
	        GRAPHICS::DRAW_TV_CHANNEL(0.5, 0.5, 1.0, 1.0, 0.0, 255, 255, 255, 255);
	        if (GRAPHICS::GET_TV_CHANNEL() == -1) {
	            sub_a8fa5(a_2, 1);
	        } else { 
	            sub_a8fa5(a_2, 1);
	            GRAPHICS::ATTACH_TV_AUDIO_TO_ENTITY(a_2._f3);
	        }
	        UI::SET_TEXT_RENDER_ID(UI::GET_DEFAULT_SCRIPT_RENDERTARGET_RENDER_ID());
	    }
	}

</summary>
]]--
native "DRAW_TV_CHANNEL"
	hash "0xFDDC2B4ED3C69DF0"
	jhash (0x8129EF89)
	arguments {
		float "posX",

		float "posY",

		float "p2",

		float "p3",

		float "p4",

		int "r",

		int "g",

		int "b",

		int "a",
	}
	returns	"void"

native "0xF7B38B8305F1FE8B"
	hash "0xF7B38B8305F1FE8B"
	jhash (0xB262DE67)
	arguments {
		int "p0",

		charPtr "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x2201C576FACAEBE8"
	hash "0x2201C576FACAEBE8"
	jhash (0x78C4DCBE)
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",
	}
	returns	"void"

native "0xBEB3D46BB7F043C0"
	hash "0xBEB3D46BB7F043C0"
	jhash (0xCBE7068F)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x0AD973CA1E077B60"
	hash "0x0AD973CA1E077B60"
	jhash (0x4D1EB0FB)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x74C180030FDE4B69"
	hash "0x74C180030FDE4B69"
	jhash (0x796DE696)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xD1C55B110E4DF534"
	hash "0xD1C55B110E4DF534"
	jhash (0xD99EC000)
	arguments {
		Any "p0",
	}
	returns	"void"

native "ENABLE_MOVIE_SUBTITLES"
	hash "0x873FA65C778AD970"
	jhash (0xC2DEBA3D)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xD3A10FC7FD8D98CD"
	hash "0xD3A10FC7FD8D98CD"
	jhash (0xE40A0F1A)
	returns	"Any"

native "0xF1CEA8A4198D8E9A"
	hash "0xF1CEA8A4198D8E9A"
	jhash (0x2E7D9B98)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	First parameter isn't used
	Second is a ped handle
	Third is unknown
	and last is Vector3

</summary>
]]--
native "0x98C4FE6EC34154CA"
	hash "0x98C4FE6EC34154CA"
	jhash (0x9A0E3BFE)
	arguments {
		AnyPtr "p0",

		Ped "pedHandle",

		Any "p2",

		float "posX",

		float "posY",

		float "posZ",
	}
	returns	"BOOL"

native "0x7A42B2E236E71415"
	hash "0x7A42B2E236E71415"
	jhash (0x431AA036)
	returns	"void"

native "0x108BE26959A9D9BB"
	hash "0x108BE26959A9D9BB"
	jhash (0x24A7A7F6)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xA356990E161C9E65"
	hash "0xA356990E161C9E65"
	jhash (0xA1CB6C94)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x1C4FC5752BCD8E48"
	hash "0x1C4FC5752BCD8E48"
	jhash (0x3B637AA7)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",

		float "p11",

		float "p12",
	}
	returns	"void"

native "0x5CE62918F8D703C7"
	hash "0x5CE62918F8D703C7"
	jhash (0xDF552973)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",

		Any "p9",

		Any "p10",

		Any "p11",
	}
	returns	"void"

--[[!
<summary>
	playLength - is how long to play the effect for in milliseconds. If 0, it plays the default length
	if loop is true, the effect wont stop until you call _STOP_SCREEN_EFFECT on it. (only loopable effects)

</summary>
]]--
native "_START_SCREEN_EFFECT"
	hash "0x2206BF9A37B7F724"
	jhash (0x1D980479)
	arguments {
		charPtr "effectName",

		int "playLength",

		BOOL "loop",
	}
	returns	"void"

native "_STOP_SCREEN_EFFECT"
	hash "0x068E835A1D0DC0E3"
	jhash (0x06BB5CDA)
	arguments {
		charPtr "effectName",
	}
	returns	"void"

native "_GET_SCREEN_EFFECT_IS_ACTIVE"
	hash "0x36AD3E690DA5ACEB"
	jhash (0x089D5921)
	arguments {
		charPtr "effectName",
	}
	returns	"BOOL"

native "_STOP_ALL_SCREEN_EFFECTS"
	hash "0xB4EDDC19532BFB85"
	jhash (0x4E6D875B)
	returns	"void"

native "0xD2209BE128B5418C"
	hash "0xD2209BE128B5418C"
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "STAT_CLEAR_SLOT_FOR_RELOAD"
	hash "0xEB0A72181D4AA4AD"
	jhash (0x84BDD475)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "STAT_LOAD"
	hash "0xA651443F437B1CE6"
	jhash (0x9E5629F4)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "STAT_SAVE"
	hash "0xE07BCA305B82D2FD"
	jhash (0xE10A7CA4)
	arguments {
		int "p0",

		BOOL "p1",

		int "p2",
	}
	returns	"BOOL"

native "0x5688585E6D563CD8"
	hash "0x5688585E6D563CD8"
	jhash (0xC62406A6)
	arguments {
		Any "p0",
	}
	returns	"void"

native "STAT_LOAD_PENDING"
	hash "0xA1750FFAFA181661"
	jhash (0x4E9AC983)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "STAT_SAVE_PENDING"
	hash "0x7D3A583856F2C5AC"
	jhash (0xC3FD3822)
	returns	"Any"

native "STAT_SAVE_PENDING_OR_REQUESTED"
	hash "0xBBB6AD006F1BBEA3"
	jhash (0xA3407CA3)
	returns	"Any"

native "STAT_DELETE_SLOT"
	hash "0x49A49BED12794D70"
	jhash (0x2F171B94)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "STAT_SLOT_IS_LOADED"
	hash "0x0D0A9F0E7BD91E3C"
	jhash (0x7A299C13)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x7F2C4CDF2E82DF4C"
	hash "0x7F2C4CDF2E82DF4C"
	jhash (0x0BF0F4B2)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xE496A53BA5F50A56"
	hash "0xE496A53BA5F50A56"
	jhash (0xCE6B62B5)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xF434A10BA01C37D0"
	hash "0xF434A10BA01C37D0"
	jhash (0xCE7A2411)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x7E6946F68A38B74F"
	hash "0x7E6946F68A38B74F"
	jhash (0x22804C20)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xA8733668D1047B51"
	hash "0xA8733668D1047B51"
	jhash (0x395D18B1)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xECB41AC6AB754401"
	hash "0xECB41AC6AB754401"
	jhash (0xED7000C8)
	returns	"Any"

native "0x9B4BD21D69B1E609"
	hash "0x9B4BD21D69B1E609"
	returns	"void"

native "0xC0E0D686DDFC6EAE"
	hash "0xC0E0D686DDFC6EAE"
	jhash (0x099FCC86)
	returns	"Any"

native "STAT_SET_INT"
	hash "0xB3271D7AB655B441"
	jhash (0xC9CC1C5C)
	arguments {
		Hash "statName",

		int "value",

		BOOL "save",
	}
	returns	"BOOL"

native "STAT_SET_FLOAT"
	hash "0x4851997F37FE9B3C"
	jhash (0x6CEA96F2)
	arguments {
		Hash "statName",

		float "value",

		BOOL "save",
	}
	returns	"BOOL"

native "STAT_SET_BOOL"
	hash "0x4B33C4243DE0C432"
	jhash (0x55D79DFB)
	arguments {
		Hash "statName",

		BOOL "value",

		BOOL "save",
	}
	returns	"BOOL"

--[[!
<summary>
	The following values have been found in the decompiled scripts:
	"RC_ABI1"
	"RC_ABI2"
	"RC_BA1"
	"RC_BA2"
	"RC_BA3"
	"RC_BA3A"
	"RC_BA3C"
	"RC_BA4"
	"RC_DRE1"
	"RC_EPS1"
	"RC_EPS2"
	"RC_EPS3"
	"RC_EPS4"
	"RC_EPS5"
	"RC_EPS6"
	"RC_EPS7"
	"RC_EPS8"
	"RC_EXT1"
	"RC_EXT2"
	"RC_EXT3"
	"RC_EXT4"
	"RC_FAN1"
	"RC_FAN2"
	"RC_FAN3"
	"RC_HAO1"
	"RC_HUN1"
	"RC_HUN2"
	"RC_JOS1"
	"RC_JOS2"
	"RC_JOS3"
	"RC_JOS4"
	"RC_MAU1"
	"RC_MIN1"
	"RC_MIN2"
	"RC_MIN3"
	"RC_MRS1"
	"RC_MRS2"
	"RC_NI1"
	"RC_NI1A"
	"RC_NI1B"
	"RC_NI1C"
	"RC_NI1D"
	"RC_NI2"
	"RC_NI3"
	"RC_OME1"
	"RC_OME2"
	"RC_PA1"
	"RC_PA2"
	"RC_PA3"
	"RC_PA3A"
	"RC_PA3B"
	"RC_PA4"
	"RC_RAM1"
	"RC_RAM2"
	"RC_RAM3"
	"RC_RAM4"
	"RC_RAM5"
	"RC_SAS1"
	"RC_TON1"
	"RC_TON2"
	"RC_TON3"
	"RC_TON4"
	"RC_TON5"
</summary>
]]--
native "STAT_SET_GXT_LABEL"
	hash "0x17695002FD8B2AE0"
	jhash (0xC1224AA7)
	arguments {
		Hash "statName",

		charPtr "value",

		BOOL "save",
	}
	returns	"BOOL"

--[[!
<summary>
	'value' is a structure to a structure, 'numFields' is how many fields there are in said structure (usually 7).

	The structure looks like this:

	int year
	int month
	int day
	int hour
	int minute
	int second
	int unknown

	The decompiled scripts use TIME::GET_POSIX_TIME to fill this structure.
</summary>
]]--
native "STAT_SET_DATE"
	hash "0x2C29BFB64F4FCBE4"
	jhash (0x36BE807B)
	arguments {
		Hash "statName",

		AnyPtr "value",

		int "numFields",

		BOOL "save",
	}
	returns	"BOOL"

native "STAT_SET_STRING"
	hash "0xA87B2335D12531D7"
	jhash (0xB1EF2E21)
	arguments {
		Hash "statName",

		charPtr "value",

		BOOL "save",
	}
	returns	"BOOL"

native "STAT_SET_POS"
	hash "0xDB283FDE680FE72E"
	jhash (0x1192C9A3)
	arguments {
		Hash "statName",

		float "x",

		float "y",

		float "z",

		BOOL "save",
	}
	returns	"BOOL"

native "STAT_SET_MASKED_INT"
	hash "0x7BBB1B54583ED410"
	jhash (0x2CBAA739)
	arguments {
		Hash "statName",

		Any "p1",

		Any "p2",

		int "p3",

		BOOL "save",
	}
	returns	"BOOL"

native "STAT_SET_USER_ID"
	hash "0x8CDDF1E452BABE11"
	jhash (0xDBE78ED7)
	arguments {
		Hash "statName",

		charPtr "value",

		BOOL "save",
	}
	returns	"BOOL"

--[[!
<summary>
	p1 always true. 
</summary>
]]--
native "STAT_SET_CURRENT_POSIX_TIME"
	hash "0xC2F84B7F9C4D0C61"
	jhash (0xA286F015)
	arguments {
		Hash "statName",

		BOOL "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	p2 appears to always be -1
</summary>
]]--
native "STAT_GET_INT"
	hash "0x767FBC2AC802EF3D"
	jhash (0x1C6FE43E)
	arguments {
		Hash "statHash",

		intPtr "outValue",

		int "p2",
	}
	returns	"BOOL"

native "STAT_GET_FLOAT"
	hash "0xD7AE6C9C9C6AC54C"
	jhash (0xFCBDA612)
	arguments {
		Hash "statHash",

		floatPtr "outValue",

		Any "p2",
	}
	returns	"BOOL"

native "STAT_GET_BOOL"
	hash "0x11B5E6D2AE73F48E"
	jhash (0x28A3DD2B)
	arguments {
		Hash "statHash",

		BOOLPtr "outValue",

		Any "p2",
	}
	returns	"BOOL"

native "STAT_GET_DATE"
	hash "0x8B0FACEFC36C824B"
	jhash (0xD762D16C)
	arguments {
		Hash "statHash",

		AnyPtr "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "STAT_GET_STRING"
	hash "0xE50384ACC2C3DB74"
	jhash (0x10CE4BDE)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "STAT_GET_POS"
	hash "0x350F82CCB186AA1B"
	jhash (0xC846ECCE)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",

		Any "p4",
	}
	returns	"BOOL"

native "STAT_GET_MASKED_INT"
	hash "0x655185A06D9EEAAB"
	jhash (0xE9D9B70F)
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"BOOL"

native "STAT_GET_USER_ID"
	hash "0x2365C388E393BBE2"
	jhash (0xE2E8B6BA)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "STAT_GET_LICENSE_PLATE"
	hash "0x5473D4195058B2E4"
	jhash (0x1544B29F)
	arguments {
		Hash "statName",
	}
	returns	"charPtr"

native "STAT_SET_LICENSE_PLATE"
	hash "0x69FF13266D7296DA"
	jhash (0x3507D253)
	arguments {
		Hash "statName",

		charPtr "str",
	}
	returns	"BOOL"

native "STAT_INCREMENT"
	hash "0x9B5A68C6489E9909"
	jhash (0xDFC5F71E)
	arguments {
		Hash "statName",

		float "value",
	}
	returns	"void"

native "0x5A556B229A169402"
	hash "0x5A556B229A169402"
	jhash (0x46F21343)
	returns	"BOOL"

native "0xB1D2BB1E1631F5B1"
	hash "0xB1D2BB1E1631F5B1"
	jhash (0x02F283CE)
	returns	"BOOL"

native "0xBED9F5693F34ED17"
	hash "0xBED9F5693F34ED17"
	jhash (0xC4110917)
	arguments {
		Hash "statName",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

--[[!
<summary>
	STATS::0x343B27E2(0);
	STATS::0x343B27E2(1);
	STATS::0x343B27E2(2);
	STATS::0x343B27E2(3);
	STATS::0x343B27E2(4);
	STATS::0x343B27E2(5);
	STATS::0x343B27E2(6);
	STATS::0x343B27E2(7);

	Identical in ingamehud &amp; maintransition.

</summary>
]]--
native "0x26D7399B9587FE89"
	hash "0x26D7399B9587FE89"
	jhash (0x343B27E2)
	arguments {
		int "p0",
	}
	returns	"void"

--[[!
<summary>
	STATS::0xE3247582(0);
	STATS::0xE3247582(1);
	STATS::0xE3247582(2);
	STATS::0xE3247582(3);
	STATS::0xE3247582(4);
	STATS::0xE3247582(5);
	STATS::0xE3247582(6);

</summary>
]]--
native "0xA78B8FA58200DA56"
	hash "0xA78B8FA58200DA56"
	jhash (0xE3247582)
	arguments {
		int "p0",
	}
	returns	"void"

native "0xE0E854F5280FB769"
	hash "0xE0E854F5280FB769"
	jhash (0xFD66A429)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xF2D4B2FE415AAFC3"
	hash "0xF2D4B2FE415AAFC3"
	jhash (0x9B431236)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x7583B4BE4C5A41B5"
	hash "0x7583B4BE4C5A41B5"
	jhash (0x347B4436)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x2CE056FF3723F00B"
	hash "0x2CE056FF3723F00B"
	jhash (0x2C1D6C31)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x68F01422BE1D838F"
	hash "0x68F01422BE1D838F"
	jhash (0x24DD4929)
	arguments {
		int "profileSetting",

		int "value",
	}
	returns	"void"

native "0xF4D8E7AC2A27758C"
	hash "0xF4D8E7AC2A27758C"
	jhash (0xDFC25D66)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x94F12ABF9C79E339"
	hash "0x94F12ABF9C79E339"
	jhash (0xCA160BCC)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x80C75307B1C42837"
	hash "0x80C75307B1C42837"
	jhash (0xB5BF87B2)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		Any "p3",
	}
	returns	"Any"

native "0x61E111E323419E07"
	hash "0x61E111E323419E07"
	jhash (0x1F938864)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		Any "p3",
	}
	returns	"Any"

native "0xC4BB08EE7907471E"
	hash "0xC4BB08EE7907471E"
	jhash (0x3F8E893B)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		Any "p3",
	}
	returns	"Any"

native "0xD16C2AD6B8E32854"
	hash "0xD16C2AD6B8E32854"
	jhash (0xFB93C5A2)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		Any "p3",
	}
	returns	"Any"

native "0xBA52FF538ED2BC71"
	hash "0xBA52FF538ED2BC71"
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		Any "p3",

		AnyPtr "p4",
	}
	returns	"Any"

native "0x2B4CDCA6F07FF3DA"
	hash "0x2B4CDCA6F07FF3DA"
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		Any "p3",

		AnyPtr "p4",
	}
	returns	"Any"

native "STAT_GET_BOOL_MASKED"
	hash "0x10FE3F1B79F9B071"
	jhash (0x6ACE1B7D)
	arguments {
		Hash "statName",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "STAT_SET_BOOL_MASKED"
	hash "0x5BC62EC1937B9E5B"
	jhash (0x7842C4D6)
	arguments {
		Hash "statName",

		BOOL "p1",

		Any "p2",

		BOOL "save",
	}
	returns	"BOOL"

native "0x5009DFD741329729"
	hash "0x5009DFD741329729"
	jhash (0x61ECC465)
	arguments {
		charPtr "p0",

		Any "p1",
	}
	returns	"void"

native "PLAYSTATS_NPC_INVITE"
	hash "0x93054C88E6AA7C44"
	jhash (0x598C06F3)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "PLAYSTATS_AWARD_XP"
	hash "0x46F917F6B4128FE4"
	jhash (0x8770017B)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "PLAYSTATS_RANK_UP"
	hash "0xC7F2DE41D102BFB4"
	jhash (0x56AFB9F5)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x098760C7461724CD"
	hash "0x098760C7461724CD"
	jhash (0x896CDF8D)
	returns	"void"

native "0xA071E0ED98F91286"
	hash "0xA071E0ED98F91286"
	jhash (0x1A66945F)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xC5BE134EC7BA96A0"
	hash "0xC5BE134EC7BA96A0"
	jhash (0xC960E161)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "PLAYSTATS_MISSION_STARTED"
	hash "0xC19A2925C34D2231"
	jhash (0x3AAB699C)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		BOOL "p3",
	}
	returns	"void"

native "PLAYSTATS_MISSION_OVER"
	hash "0x7C4BB33A8CED7324"
	jhash (0x5B90B5FF)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		BOOL "p3",

		BOOL "p4",

		BOOL "p5",
	}
	returns	"void"

native "PLAYSTATS_MISSION_CHECKPOINT"
	hash "0xC900596A63978C1D"
	jhash (0xCDC52280)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x71862B1D855F32E1"
	hash "0x71862B1D855F32E1"
	jhash (0xAC2C7C63)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x121FB4DDDC2D5291"
	hash "0x121FB4DDDC2D5291"
	jhash (0x413539BC)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		float "p3",
	}
	returns	"void"

native "PLAYSTATS_RACE_CHECKPOINT"
	hash "0x9C375C315099DDE4"
	jhash (0x580D5508)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "0x6DEE77AFF8C21BD1"
	hash "0x6DEE77AFF8C21BD1"
	jhash (0x489E27E7)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "PLAYSTATS_MATCH_STARTED"
	hash "0xBC80E22DED931E3D"
	jhash (0x2BDE85C1)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"void"

native "PLAYSTATS_SHOP_ITEM"
	hash "0x176852ACAAC173D1"
	jhash (0xA4746384)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "0x1CAE5D2E3F9A07F0"
	hash "0x1CAE5D2E3F9A07F0"
	jhash (0x6602CED6)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"void"

native "0xAFC7E5E075A96F46"
	hash "0xAFC7E5E075A96F46"
	arguments {
		float "p0",

		float "p1",

		float "p2",
	}
	returns	"void"

native "0xCB00196B31C39EB1"
	hash "0xCB00196B31C39EB1"
	jhash (0x759E0EC9)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x2B69F5074C894811"
	hash "0x2B69F5074C894811"
	jhash (0x62073DF7)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x7EEC2A316C250073"
	hash "0x7EEC2A316C250073"
	jhash (0x30558CFD)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0xADDD1C754E2E2914"
	hash "0xADDD1C754E2E2914"
	jhash (0x06CE3692)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",

		Any "p9",
	}
	returns	"void"

native "0x79AB33F0FBFAC40C"
	hash "0x79AB33F0FBFAC40C"
	jhash (0x8D5C7B37)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xDDF24D535060F811"
	hash "0xDDF24D535060F811"
	jhash (0x37D152BB)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "PLAYSTATS_FRIEND_ACTIVITY"
	hash "0x0F71DE29AB2258F1"
	jhash (0xD1FA1BDB)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "PLAYSTATS_ODDJOB_DONE"
	hash "0x69DEA3E9DB727B4C"
	jhash (0xFE14A8EA)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "PLAYSTATS_PROP_CHANGE"
	hash "0xBA739D6D5A05D6E7"
	jhash (0x25740A1D)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "PLAYSTATS_CLOTH_CHANGE"
	hash "0x34B973047A2268B9"
	jhash (0x3AFF9E58)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "0xE95C8A1875A02CA4"
	hash "0xE95C8A1875A02CA4"
	jhash (0x79716890)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "PLAYSTATS_CHEAT_APPLIED"
	hash "0x6058665D72302D3F"
	jhash (0x345166F3)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xF8C54A461C3E11DC"
	hash "0xF8C54A461C3E11DC"
	jhash (0x04181752)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"void"

native "0xF5BB8DAC426A52C0"
	hash "0xF5BB8DAC426A52C0"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"void"

native "0xA736CF7FB7C5BFF4"
	hash "0xA736CF7FB7C5BFF4"
	jhash (0x31002201)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"void"

native "0x14E0B2D1AD1044E0"
	hash "0x14E0B2D1AD1044E0"
	jhash (0xDDD1F1F3)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"void"

native "0x90D0622866E80445"
	hash "0x90D0622866E80445"
	jhash (0x66FEB701)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x5DA3A8DE8CB6226F"
	hash "0x5DA3A8DE8CB6226F"
	jhash (0x9E2B9522)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xD1032E482629049E"
	hash "0xD1032E482629049E"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xF4FF020A08BC8863"
	hash "0xF4FF020A08BC8863"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x46326E13DA4E0546"
	hash "0x46326E13DA4E0546"
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "LEADERBOARDS_GET_NUMBER_OF_COLUMNS"
	hash "0x117B45156D7EFF2E"
	jhash (0x0A56EE34)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "LEADERBOARDS_GET_COLUMN_ID"
	hash "0xC4B5467A1886EA7E"
	jhash (0x3821A334)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "LEADERBOARDS_GET_COLUMN_TYPE"
	hash "0xBF4FEF46DB7894D3"
	jhash (0x6F2820F4)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "LEADERBOARDS_READ_CLEAR_ALL"
	hash "0xA34CB6E6F0DF4A0B"
	jhash (0x233E058A)
	returns	"Any"

native "LEADERBOARDS_READ_CLEAR"
	hash "0x7CCE5C737A665701"
	jhash (0x7090012F)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "LEADERBOARDS_READ_PENDING"
	hash "0xAC392C8483342AC2"
	jhash (0xEEB8BF5C)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0xA31FD15197B192BD"
	hash "0xA31FD15197B192BD"
	jhash (0x1789437B)
	returns	"Any"

native "LEADERBOARDS_READ_SUCCESSFUL"
	hash "0x2FB19228983E832C"
	jhash (0x3AC5B2F1)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "LEADERBOARDS2_READ_FRIENDS_BY_ROW"
	hash "0x918B101666F9CB83"
	jhash (0xBD91B136)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",

		BOOL "p3",

		Any "p4",

		Any "p5",
	}
	returns	"BOOL"

native "LEADERBOARDS2_READ_BY_HANDLE"
	hash "0xC30713A383BFBF0E"
	jhash (0x6B553408)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "LEADERBOARDS2_READ_BY_ROW"
	hash "0xA9CDB1E3F0A49883"
	jhash (0xCA931F34)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",

		AnyPtr "p3",

		Any "p4",

		AnyPtr "p5",

		Any "p6",
	}
	returns	"BOOL"

native "LEADERBOARDS2_READ_BY_RANK"
	hash "0xBA2C7DB0C129449A"
	jhash (0x1B03F59F)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "LEADERBOARDS2_READ_BY_RADIUS"
	hash "0x5CE587FB5A42C8C4"
	jhash (0xC5B7E685)
	arguments {
		AnyPtr "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "LEADERBOARDS2_READ_BY_SCORE_INT"
	hash "0x7EEC7E4F6984A16A"
	jhash (0xAC020C18)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "LEADERBOARDS2_READ_BY_SCORE_FLOAT"
	hash "0xE662C8B759D08F3C"
	jhash (0xC678B29F)
	arguments {
		AnyPtr "p0",

		float "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0xC38DC1E90D22547C"
	hash "0xC38DC1E90D22547C"
	jhash (0x9BEC3401)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0xF1AE5DCDBFCA2721"
	hash "0xF1AE5DCDBFCA2721"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0xA0F93D5465B3094D"
	hash "0xA0F93D5465B3094D"
	jhash (0xC977D6E2)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x71B008056E5692D6"
	hash "0x71B008056E5692D6"
	jhash (0xF2DB6A82)
	returns	"void"

native "0x34770B9CE0E03B91"
	hash "0x34770B9CE0E03B91"
	jhash (0x766A74FE)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x88578F6EC36B4A3A"
	hash "0x88578F6EC36B4A3A"
	jhash (0x6B90E730)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x38491439B6BA7F7D"
	hash "0x38491439B6BA7F7D"
	jhash (0x509A286F)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"float"

native "LEADERBOARDS2_WRITE_DATA"
	hash "0xAE2206545888AE49"
	jhash (0x5F9DF634)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x0BCA1D2C47B0D269"
	hash "0x0BCA1D2C47B0D269"
	jhash (0x7524E27B)
	arguments {
		Any "p0",

		Any "p1",

		float "p2",
	}
	returns	"void"

native "0x2E65248609523599"
	hash "0x2E65248609523599"
	jhash (0x1C5CCC3A)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "LEADERBOARDS_CACHE_DATA_ROW"
	hash "0xB9BB18E2C40142ED"
	jhash (0x44F7D82B)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "LEADERBOARDS_CLEAR_CACHE_DATA"
	hash "0xD4B02A6B476E1FDC"
	jhash (0x87F498C1)
	returns	"void"

native "0x8EC74CEB042E7CFF"
	hash "0x8EC74CEB042E7CFF"
	jhash (0x88AE9667)
	arguments {
		Any "p0",
	}
	returns	"void"

native "LEADERBOARDS_GET_CACHE_EXISTS"
	hash "0x9C51349BE6CDFE2C"
	jhash (0xFC8A71F3)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "LEADERBOARDS_GET_CACHE_TIME"
	hash "0xF04C1C27DA35F6C8"
	jhash (0xEDF02302)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x58A651CD201D89AD"
	hash "0x58A651CD201D89AD"
	jhash (0xCE7CB520)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "LEADERBOARDS_GET_CACHE_DATA_ROW"
	hash "0x9120E8DBA3D69273"
	jhash (0xA11289EC)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0x11FF1C80276097ED"
	hash "0x11FF1C80276097ED"
	jhash (0x4AC39C6C)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0x30A6614C1F7799B8"
	hash "0x30A6614C1F7799B8"
	jhash (0x3E69E7C3)
	arguments {
		Any "p0",

		float "p1",

		Any "p2",
	}
	returns	"void"

native "0x6483C25849031C4F"
	hash "0x6483C25849031C4F"
	jhash (0x2FFD2FA5)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		AnyPtr "p3",
	}
	returns	"void"

--[[!
<summary>
	example from completionpercentage_controller.ysc.c4

	if (STATS::_5EAD2BF6484852E4()) {
	            GAMEPLAY::SET_BIT(g_17b95._f20df._ff10, 15);
	            STATS::_11FF1C80276097ED(0xe9ec4dd1, 200, 0);
	        }
</summary>
]]--
native "0x5EAD2BF6484852E4"
	hash "0x5EAD2BF6484852E4"
	jhash (0x23D70C39)
	returns	"BOOL"

native "0xC141B8917E0017EC"
	hash "0xC141B8917E0017EC"
	jhash (0x0AD43306)
	returns	"void"

native "0xB475F27C6A994D65"
	hash "0xB475F27C6A994D65"
	jhash (0xC7DE5C30)
	returns	"void"

native "0xF1A1803D3476F215"
	hash "0xF1A1803D3476F215"
	jhash (0xA3DAC790)
	arguments {
		int "value",
	}
	returns	"void"

native "0x38BAAA5DD4C9D19F"
	hash "0x38BAAA5DD4C9D19F"
	jhash (0x726FAE66)
	arguments {
		int "value",
	}
	returns	"void"

native "0x55384438FC55AD8E"
	hash "0x55384438FC55AD8E"
	jhash (0xF03895A4)
	arguments {
		int "value",
	}
	returns	"void"

native "0x723C1CE13FBFDB67"
	hash "0x723C1CE13FBFDB67"
	jhash (0x4C39CF10)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x0D01D20616FC73FB"
	hash "0x0D01D20616FC73FB"
	jhash (0x2180AE13)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x428EAF89E24F6C36"
	hash "0x428EAF89E24F6C36"
	jhash (0xEE292B91)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"void"

native "0x047CBED6F6F8B63C"
	hash "0x047CBED6F6F8B63C"
	jhash (0xA063CABD)
	returns	"void"

native "0xC980E62E33DF1D5C"
	hash "0xC980E62E33DF1D5C"
	jhash (0x62C19A3D)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x6F361B8889A792A3"
	hash "0x6F361B8889A792A3"
	jhash (0x3B4EF322)
	returns	"void"

native "0xC847B43F369AC0B5"
	hash "0xC847B43F369AC0B5"
	returns	"void"

native "0xA5C80D8E768A9E66"
	hash "0xA5C80D8E768A9E66"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x9A62EC95AE10E011"
	hash "0x9A62EC95AE10E011"
	returns	"Any"

native "0x4C89FE2BDEB3F169"
	hash "0x4C89FE2BDEB3F169"
	returns	"Any"

native "0xC6E0E2616A7576BB"
	hash "0xC6E0E2616A7576BB"
	returns	"Any"

native "0x5BD5F255321C4AAF"
	hash "0x5BD5F255321C4AAF"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xDEAAF77EB3687E97"
	hash "0xDEAAF77EB3687E97"
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"Any"

native "0xC70DDCE56D0D3A99"
	hash "0xC70DDCE56D0D3A99"
	jhash (0x54E775E0)
	returns	"Any"

native "0x886913BBEACA68C1"
	hash "0x886913BBEACA68C1"
	jhash (0xE3F0D62D)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "0x4FEF53183C3C6414"
	hash "0x4FEF53183C3C6414"
	returns	"Any"

native "0x567384DFA67029E6"
	hash "0x567384DFA67029E6"
	returns	"Any"

native "0x3270F67EED31FBC1"
	hash "0x3270F67EED31FBC1"
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0xCE5AA445ABA8DEE0"
	hash "0xCE5AA445ABA8DEE0"
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "0x98E2BC1CA26287C3"
	hash "0x98E2BC1CA26287C3"
	returns	"void"

native "0x629526ABA383BCAA"
	hash "0x629526ABA383BCAA"
	returns	"void"

native "0xB3DA2606774A8E2D"
	hash "0xB3DA2606774A8E2D"
	returns	"Any"

native "0xDAC073C7901F9E15"
	hash "0xDAC073C7901F9E15"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xF6792800AC95350D"
	hash "0xF6792800AC95350D"
	arguments {
		Any "p0",
	}
	returns	"void"

native "ADD_SCRIPT_TO_RANDOM_PED"
	hash "0x4EE5367468A65CCC"
	jhash (0xECC76C3D)
	arguments {
		charPtr "p0",

		Any "p1",

		Any "p2",

		BOOL "p3",
	}
	returns	"void"

native "REGISTER_OBJECT_SCRIPT_BRAIN"
	hash "0x0BE84C318BA6EC22"
	jhash (0xB6BCC608)
	arguments {
		charPtr "scriptName",

		Hash "p1",

		int "p2",

		float "p3",

		int "p4",

		int "p5",
	}
	returns	"void"

native "IS_OBJECT_WITHIN_BRAIN_ACTIVATION_RANGE"
	hash "0xCCBA154209823057"
	jhash (0xBA4CAA56)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "REGISTER_WORLD_POINT_SCRIPT_BRAIN"
	hash "0x3CDC7136613284BD"
	jhash (0x725D91F7)
	arguments {
		AnyPtr "p0",

		float "p1",

		Any "p2",
	}
	returns	"void"

--[[!
<summary>
	Gets whether the world point the calling script is registered to is within desired range of the player.
</summary>
]]--
native "IS_WORLD_POINT_WITHIN_BRAIN_ACTIVATION_RANGE"
	hash "0xC5042CC6F5E3D450"
	jhash (0x2CF305A0)
	returns	"BOOL"

native "ENABLE_SCRIPT_BRAIN_SET"
	hash "0x67AA4D73F0CFA86B"
	jhash (0x2765919F)
	arguments {
		Any "p0",
	}
	returns	"void"

native "DISABLE_SCRIPT_BRAIN_SET"
	hash "0x14D8518E9760F08F"
	jhash (0xFBD13FAD)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x0B40ED49D7D6FF84"
	hash "0x0B40ED49D7D6FF84"
	jhash (0x19B27825)
	returns	"void"

--[[!
<summary>
	Something like flush_all_scripts 

	Most of time comes after NETWORK_END_TUTORIAL_SESSION() or before TERMINATE_THIS_THREAD()
</summary>
]]--
native "0x4D953DF78EBF8158"
	hash "0x4D953DF78EBF8158"
	jhash (0xF3A3AB08)
	returns	"void"

--[[!
<summary>
	Possible values:

	ACT_CINEMA
	act_cinema
	AM_MP_CARWASH_LAUNCH
	AM_MP_GARAGE_CONTROL
	AM_MP_PROPERTY_EXT
	chop
	fairgroundHub
	launcher_BasejumpHeli
	launcher_BasejumpPack
	launcher_CarWash
	launcher_golf
	launcher_Hunting_Ambient
	launcher_MrsPhilips
	launcher_OffroadRacing
	launcher_pilotschool
	launcher_Racing
	launcher_rampage
	launcher_rampage
	launcher_range
	launcher_stunts
	launcher_stunts
	launcher_tennis
	launcher_Tonya
	launcher_Triathlon
	launcher_Yoga
	ob_mp_bed_low
	ob_mp_bed_med
</summary>
]]--
native "0x6D6840CEE8845831"
	hash "0x6D6840CEE8845831"
	jhash (0x949FE53E)
	arguments {
		charPtr "action",
	}
	returns	"void"

--[[!
<summary>
	Looks like a cousin of above function _6D6840CEE8845831 as it was found among them. Must be similar

	Here are possible values of argument - 

	"ob_tv"
	"launcher_Darts"
</summary>
]]--
native "0x6E91B04E08773030"
	hash "0x6E91B04E08773030"
	jhash (0x29CE8BAA)
	arguments {
		charPtr "action",
	}
	returns	"void"

--[[!
<summary>
	Creates a mobile phone of the specified type.

	Possible phone types:

	0 - Default phone / Michael's phone
	1 - Trevor's phone
	2 - Franklin's phone
	4 - Prologue phone

	These values represent bit flags, so a value of '3' would toggle Trevor and Franklin's phones together, causing unexpected behavior and most likely crash the game.
</summary>
]]--
native "CREATE_MOBILE_PHONE"
	hash "0xA4E8E696C532FBC7"
	jhash (0x5BBC5E23)
	arguments {
		int "phoneType",
	}
	returns	"void"

--[[!
<summary>
	Destroys the currently active mobile phone.
</summary>
]]--
native "DESTROY_MOBILE_PHONE"
	hash "0x3BC861DF703E5097"
	jhash (0x1A65037B)
	returns	"void"

native "SET_MOBILE_PHONE_SCALE"
	hash "0xCBDD322A73D6D932"
	jhash (0x09BCF1BE)
	arguments {
		float "scale",
	}
	returns	"void"

--[[!
<summary>
	Last parameter is unknown and always zero.
</summary>
]]--
native "SET_MOBILE_PHONE_ROTATION"
	hash "0xBB779C0CA917E865"
	jhash (0x209C28CF)
	arguments {
		float "rotX",

		float "rotY",

		float "rotZ",

		Any "p3",
	}
	returns	"void"

native "GET_MOBILE_PHONE_ROTATION"
	hash "0x1CEFB61F193070AE"
	jhash (0x17A29F23)
	arguments {
		Vector3Ptr "rotation",

		Any "p1",
	}
	returns	"void"

native "SET_MOBILE_PHONE_POSITION"
	hash "0x693A5C6D6734085B"
	jhash (0x841800B3)
	arguments {
		float "posX",

		float "posY",

		float "posZ",
	}
	returns	"void"

native "GET_MOBILE_PHONE_POSITION"
	hash "0x584FDFDA48805B86"
	jhash (0xB2E1E1A0)
	arguments {
		Vector3Ptr "position",
	}
	returns	"void"

--[[!
<summary>
	If bool Toggle = true so the mobile is hide to screen.
	If bool Toggle = false so the mobile is show to screen.
</summary>
]]--
native "SCRIPT_IS_MOVING_MOBILE_PHONE_OFFSCREEN"
	hash "0xF511F759238A5122"
	jhash (0x29828690)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "CAN_PHONE_BE_SEEN_ON_SCREEN"
	hash "0xC4E2813898C97A4B"
	jhash (0x5F978584)
	returns	"BOOL"

--[[!
<summary>
	For move the finger of player, the value of int goes 1 at 5.
</summary>
]]--
native "_MOVE_FINGER"
	hash "0x95C9E72F3D7DEC9B"
	arguments {
		int "p0",
	}
	returns	"void"

--[[!
<summary>
	if the bool "Toggle" is "true" so the phone is lean.
	if the bool "Toggle" is "false" so the phone is not lean.
</summary>
]]--
native "_SET_PHONE_LEAN"
	hash "0x44E44169EF70138E"
	arguments {
		BOOL "Toggle",
	}
	returns	"void"

native "CELL_CAM_ACTIVATE"
	hash "0xFDE8F069C542D126"
	jhash (0x234C1AE9)
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x015C49A93E3E086E"
	hash "0x015C49A93E3E086E"
	jhash (0x4479B304)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	Needs more research. If the "phone_cam12" filter is applied, this function is called with "TRUE"; otherwise, "FALSE".

	Example (XBOX 360):

	// check current filter selection
	if (GAMEPLAY::ARE_STRINGS_EQUAL(getElem(g_2471024, &amp;l_17, 4), "phone_cam12") != 0)
	{
	    MOBILE::_0xC273BB4D(0); // FALSE
	}
	else
	{
	    MOBILE::_0xC273BB4D(1); // TRUE
	}
</summary>
]]--
native "0xA2CCBE62CD4C91A4"
	hash "0xA2CCBE62CD4C91A4"
	jhash (0xC273BB4D)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0x1B0B4AEED5B9B41C"
	hash "0x1B0B4AEED5B9B41C"
	arguments {
		float "p0",
	}
	returns	"void"

native "0x53F4892D18EC90A4"
	hash "0x53F4892D18EC90A4"
	arguments {
		float "p0",
	}
	returns	"void"

native "0x3117D84EFA60F77B"
	hash "0x3117D84EFA60F77B"
	arguments {
		float "p0",
	}
	returns	"void"

native "0x15E69E2802C24B8D"
	hash "0x15E69E2802C24B8D"
	arguments {
		float "p0",
	}
	returns	"void"

native "0xAC2890471901861C"
	hash "0xAC2890471901861C"
	arguments {
		float "p0",
	}
	returns	"void"

native "0xD6ADE981781FCA09"
	hash "0xD6ADE981781FCA09"
	arguments {
		float "p0",
	}
	returns	"void"

native "0xF1E22DC13F5EEBAD"
	hash "0xF1E22DC13F5EEBAD"
	arguments {
		float "p0",
	}
	returns	"void"

native "0x466DA42C89865553"
	hash "0x466DA42C89865553"
	jhash (0x66DCD9D2)
	arguments {
		float "p0",
	}
	returns	"void"

native "CELL_CAM_IS_CHAR_VISIBLE_NO_FACE_CHECK"
	hash "0x439E9BC95B7E7FBE"
	jhash (0xBEA88097)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "GET_MOBILE_PHONE_RENDER_ID"
	hash "0xB4A53E05F68B6FA1"
	jhash (0x88E4FECE)
	arguments {
		AnyPtr "renderId",
	}
	returns	"void"

native "0xBD4D7EAF8A30F637"
	hash "0xBD4D7EAF8A30F637"
	arguments {
		charPtr "name",
	}
	returns	"BOOL"

--[[!
<summary>
	ex.:

	v_15 = sub_10cc8(VEHICLE::GET_VEHICLE_WHEEL_TYPE(a_2));
	if (!MOBILE::_247F0F73A182EA0B(v_15)) {
	    return 1;
	}

</summary>
]]--
native "0x247F0F73A182EA0B"
	hash "0x247F0F73A182EA0B"
	arguments {
		Hash "hash",
	}
	returns	"BOOL"

native "APP_DATA_VALID"
	hash "0x846AA8E7D55EE5B6"
	jhash (0x72BDE002)
	returns	"BOOL"

native "APP_GET_INT"
	hash "0xD3A58A12C77D9D4B"
	jhash (0x2942AAD2)
	arguments {
		charPtr "property",
	}
	returns	"int"

native "APP_GET_FLOAT"
	hash "0x1514FB24C02C2322"
	jhash (0xD87F3A1C)
	arguments {
		charPtr "property",
	}
	returns	"float"

native "APP_GET_STRING"
	hash "0x749B023950D2311C"
	jhash (0x849CEB80)
	arguments {
		charPtr "property",
	}
	returns	"charPtr"

native "APP_SET_INT"
	hash "0x607E8E3D3E4F9611"
	jhash (0x1B509C32)
	arguments {
		charPtr "property",

		int "value",
	}
	returns	"void"

native "APP_SET_FLOAT"
	hash "0x25D7687C68E0DAA4"
	jhash (0xF3076135)
	arguments {
		charPtr "property",

		float "value",
	}
	returns	"void"

native "APP_SET_STRING"
	hash "0x3FF2FCEC4B7721B4"
	jhash (0x23DF19A8)
	arguments {
		charPtr "property",

		charPtr "value",
	}
	returns	"void"

native "APP_SET_APP"
	hash "0xCFD0406ADAF90D2B"
	jhash (0x8BAC4146)
	arguments {
		charPtr "appName",
	}
	returns	"void"

native "APP_SET_BLOCK"
	hash "0x262AB456A3D21F93"
	jhash (0xC2D54DD9)
	arguments {
		charPtr "blockName",
	}
	returns	"void"

native "APP_CLEAR_BLOCK"
	hash "0x5FE1DF3342DB7DBA"
	jhash (0xDAB86A18)
	returns	"void"

native "APP_CLOSE_APP"
	hash "0xE41C65E07A5F05FC"
	jhash (0x03767C7A)
	returns	"void"

native "APP_CLOSE_BLOCK"
	hash "0xE8E3FCF72EAC0EF8"
	jhash (0xED97B202)
	returns	"void"

native "APP_HAS_LINKED_SOCIAL_CLUB_ACCOUNT"
	hash "0x71EEE69745088DA0"
	jhash (0xD368BA15)
	returns	"BOOL"

native "APP_HAS_SYNCED_DATA"
	hash "0xCA52279A7271517F"
	jhash (0x1DE2A63D)
	arguments {
		charPtr "appName",
	}
	returns	"BOOL"

native "APP_SAVE_DATA"
	hash "0x95C5D356CDA6E85F"
	jhash (0x84A3918D)
	returns	"void"

native "APP_GET_DELETED_FILE_STATUS"
	hash "0xC9853A2BE3DED1A6"
	jhash (0x784D550B)
	returns	"Any"

native "APP_DELETE_APP_DATA"
	hash "0x44151AEA95C8A003"
	jhash (0x2A2FBD1C)
	arguments {
		charPtr "appName",
	}
	returns	"BOOL"

native "SET_CLOCK_TIME"
	hash "0x47C3B5848C3E45D8"
	jhash (0x26F6AF14)
	arguments {
		int "hour",

		int "minute",

		int "second",
	}
	returns	"void"

native "PAUSE_CLOCK"
	hash "0x4055E40BD2DBEC1D"
	jhash (0xB02D6124)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "ADVANCE_CLOCK_TIME_TO"
	hash "0xC8CA9670B9D83B3B"
	jhash (0x57B8DA7C)
	arguments {
		int "hour",

		int "minute",

		int "second",
	}
	returns	"void"

native "ADD_TO_CLOCK_TIME"
	hash "0xD716F30D8C8980E2"
	jhash (0xCC40D20D)
	arguments {
		int "hours",

		int "minutes",

		int "seconds",
	}
	returns	"void"

--[[!
<summary>
	Gets the current ingame hour, expressed without zeros. (09:34 will be represented as 9)
</summary>
]]--
native "GET_CLOCK_HOURS"
	hash "0x25223CA6B4D20B7F"
	jhash (0x7EF8316F)
	returns	"int"

--[[!
<summary>
	Gets the current ingame clock minute.
</summary>
]]--
native "GET_CLOCK_MINUTES"
	hash "0x13D2B8ADD79640F2"
	jhash (0x94AAC486)
	returns	"int"

--[[!
<summary>
	Gets the current ingame clock second. Note that ingame clock seconds change really fast.
</summary>
]]--
native "GET_CLOCK_SECONDS"
	hash "0x494E97C2EF27C470"
	jhash (0x099C927E)
	returns	"int"

native "SET_CLOCK_DATE"
	hash "0xB096419DF0D06CE7"
	jhash (0x96891C94)
	arguments {
		int "day",

		int "month",

		int "year",
	}
	returns	"void"

--[[!
<summary>
	Gets the current day of the week.

	0: Sunday
	1: Monday
	2: Tuesday
	3: Wednesday
	4: Thursday
	5: Friday
	6: Saturday
</summary>
]]--
native "GET_CLOCK_DAY_OF_WEEK"
	hash "0xD972E4BD7AEB235F"
	jhash (0x84E4A289)
	returns	"int"

native "GET_CLOCK_DAY_OF_MONTH"
	hash "0x3D10BC92A4DB1D35"
	jhash (0xC7A5ACB7)
	returns	"int"

native "GET_CLOCK_MONTH"
	hash "0xBBC72712E80257A1"
	jhash (0x3C48A3D5)
	returns	"int"

native "GET_CLOCK_YEAR"
	hash "0x961777E64BDAF717"
	jhash (0xB8BECF15)
	returns	"int"

native "GET_MILLISECONDS_PER_GAME_MINUTE"
	hash "0x2F8B4D1C595B11DB"
	jhash (0x3B74095C)
	returns	"int"

--[[!
<summary>
	Gets system time as year, month, day, hour, minute and second.

	Example usage:

		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;

		TIME::GET_POSIX_TIME(&amp;year, &amp;month, &amp;day, &amp;hour, &amp;minute, &amp;second);

</summary>
]]--
native "GET_POSIX_TIME"
	hash "0xDA488F299A5B164E"
	jhash (0xE15A5281)
	arguments {
		intPtr "year",

		intPtr "month",

		intPtr "day",

		intPtr "hour",

		intPtr "minute",

		intPtr "second",
	}
	returns	"void"

native "_GET_LOCAL_TIME"
	hash "0x8117E09A19EEF4D3"
	arguments {
		intPtr "year",

		intPtr "month",

		intPtr "day",

		intPtr "hour",

		intPtr "minute",

		intPtr "second",
	}
	returns	"void"

--[[!
<summary>
	Gets local system time as year, month, day, hour, minute and second.

	Example usage:

	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;

	TIME::GET_LOCAL_TIME(&amp;year, &amp;month, &amp;day, &amp;hour, &amp;minute, &amp;second);

</summary>
]]--
native "GET_LOCAL_TIME"
	hash "0x50C7A99057A69748"
	jhash (0x124BCFA2)
	arguments {
		intPtr "year",

		intPtr "month",

		intPtr "day",

		intPtr "hour",

		intPtr "minute",

		intPtr "second",
	}
	returns	"void"

native "SET_ROADS_IN_AREA"
	hash "0xBF1A602B5BA52FEE"
	jhash (0xEBC7B918)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		BOOL "p6",

		BOOL "p7",
	}
	returns	"void"

native "SET_ROADS_IN_ANGLED_AREA"
	hash "0x1A5AA1208AF5DB59"
	jhash (0xBD088F4B)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		BOOL "p7",

		BOOL "p8",

		BOOL "p9",
	}
	returns	"void"

native "SET_PED_PATHS_IN_AREA"
	hash "0x34F060F4BF92E018"
	jhash (0x2148EA84)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"void"

--[[!
<summary>
	When onGround == true outPosition is a position located on the nearest pavement.

	When a safe coord could not be found the result of a function is false and outPosition == Vector3.Zero.
</summary>
]]--
native "GET_SAFE_COORD_FOR_PED"
	hash "0xB61C8E878A4199CA"
	jhash (0xB370270A)
	arguments {
		float "x",

		float "y",

		float "z",

		BOOL "onGround",

		Vector3Ptr "outPosition",

		int "flags",
	}
	returns	"BOOL"

--[[!
<summary>
	Vector3 nodePos;
	GET_CLOSEST_VEHICLE_NODE(x,y,z,&amp;nodePos,...)

	p5, p6 are always the same:
	0x40400000 (3.0), 0
</summary>
]]--
native "GET_CLOSEST_VEHICLE_NODE"
	hash "0x240A18690AE96513"
	jhash (0x6F5F1E6C)
	arguments {
		float "x",

		float "y",

		float "z",

		Vector3Ptr "outPosition",

		int "p4",

		float "p5",

		int "p6",
	}
	returns	"BOOL"

--[[!
<summary>
	Get the closest vehicle node to a given position, unknown1 = 3.0, unknown2 = 0
</summary>
]]--
native "GET_CLOSEST_MAJOR_VEHICLE_NODE"
	hash "0x2EABE3B06F58C1BE"
	jhash (0x04B5F15B)
	arguments {
		float "x",

		float "y",

		float "z",

		Vector3Ptr "outPosition",

		float "unknown1",

		int "unknown2",
	}
	returns	"BOOL"

native "GET_CLOSEST_VEHICLE_NODE_WITH_HEADING"
	hash "0xFF071FB798B803B0"
	jhash (0x8BD5759B)
	arguments {
		float "x",

		float "y",

		float "z",

		Vector3Ptr "outPosition",

		floatPtr "outHeading",

		int "p5",

		float "p6",

		int "p7",
	}
	returns	"BOOL"

native "GET_NTH_CLOSEST_VEHICLE_NODE"
	hash "0xE50E52416CCF948B"
	jhash (0xF125BFCC)
	arguments {
		float "x",

		float "y",

		float "z",

		int "nthClosest",

		Vector3Ptr "outPosition",

		Any "unknown1",

		Any "unknown2",

		Any "unknown3",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns the id.
</summary>
]]--
native "GET_NTH_CLOSEST_VEHICLE_NODE_ID"
	hash "0x22D7275A79FE8215"
	jhash (0x3F358BEA)
	arguments {
		float "x",

		float "y",

		float "z",

		Any "p3",

		Any "p4",

		float "p5",

		float "p6",
	}
	returns	"int"

--[[!
<summary>
	Get the nth closest vehicle node and its heading. (unknown2 = 9, unknown3 = 3.0, unknown4 = 2.5)
</summary>
]]--
native "GET_NTH_CLOSEST_VEHICLE_NODE_WITH_HEADING"
	hash "0x80CA6A8B6C094CC4"
	jhash (0x7349C856)
	arguments {
		float "x",

		float "y",

		float "z",

		int "nthClosest",

		Vector3Ptr "outPosition",

		floatPtr "heading",

		AnyPtr "unknown1",

		int "unknown2",

		float "unknown3",

		float "unknown4",
	}
	returns	"BOOL"

native "GET_NTH_CLOSEST_VEHICLE_NODE_ID_WITH_HEADING"
	hash "0x6448050E9C2A7207"
	jhash (0xC1AEB88D)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		Any "p3",

		AnyPtr "p4",

		AnyPtr "p5",

		Any "p6",

		float "p7",

		float "p8",
	}
	returns	"Any"

--[[!
<summary>
	p10 always equal 0x40400000
	p11 always equal 0
</summary>
]]--
native "GET_NTH_CLOSEST_VEHICLE_NODE_FAVOUR_DIRECTION"
	hash "0x45905BE8654AE067"
	jhash (0x928A4DEC)
	arguments {
		float "x",

		float "y",

		float "z",

		float "desiredX",

		float "desiredY",

		float "desiredZ",

		int "nthClosest",

		Vector3Ptr "outPosition",

		floatPtr "outHeading",

		int "p9",

		Any "p10",

		Any "p11",
	}
	returns	"BOOL"

native "GET_VEHICLE_NODE_PROPERTIES"
	hash "0x0568566ACBB5DEDC"
	jhash (0xCC90110B)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		AnyPtr "p3",

		AnyPtr "p4",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns true if the id is non zero.
</summary>
]]--
native "IS_VEHICLE_NODE_ID_VALID"
	hash "0x1EAF30FCFBF5AF74"
	jhash (0x57DFB1EF)
	arguments {
		int "vehicleNodeId",
	}
	returns	"BOOL"

native "GET_VEHICLE_NODE_POSITION"
	hash "0x703123E5E7D429C2"
	jhash (0xE38E252D)
	arguments {
		Any "p0",

		Vector3Ptr "outPosition",
	}
	returns	"void"

--[[!
<summary>
	p0 = VEHICLE_NODE_ID
</summary>
]]--
native "0xA2AE5C478B96E3B6"
	hash "0xA2AE5C478B96E3B6"
	jhash (0xEE4B1219)
	arguments {
		int "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	p0 = VEHICLE_NODE_ID
</summary>
]]--
native "0x4F5070AA58F69279"
	hash "0x4F5070AA58F69279"
	jhash (0x56737A3C)
	arguments {
		int "p0",
	}
	returns	"BOOL"

native "GET_CLOSEST_ROAD"
	hash "0x132F52BBA570FE92"
	jhash (0x567B0E11)
	arguments {
		float "x",

		float "y",

		float "z",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",

		Any "p9",

		Any "p10",
	}
	returns	"Any"

native "LOAD_ALL_PATH_NODES"
	hash "0x80E4A6EDDB0BE8D9"
	jhash (0xC66E28C3)
	arguments {
		BOOL "p0",
	}
	returns	"BOOL"

native "0x228E5C6AD4D74BFD"
	hash "0x228E5C6AD4D74BFD"
	jhash (0xD6A3B458)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xF7B79A50B905A30D"
	hash "0xF7B79A50B905A30D"
	jhash (0x86E80A17)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"BOOL"

native "0x07FB139B592FA687"
	hash "0x07FB139B592FA687"
	jhash (0x2CDA5012)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"BOOL"

native "SET_ROADS_BACK_TO_ORIGINAL"
	hash "0x1EE7063B80FFC77C"
	jhash (0x86AC4A85)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"void"

native "SET_ROADS_BACK_TO_ORIGINAL_IN_ANGLED_AREA"
	hash "0x0027501B9F3B407E"
	jhash (0x9DB5D209)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"void"

native "0x0B919E1FB47CC4E0"
	hash "0x0B919E1FB47CC4E0"
	jhash (0x3C5085E4)
	arguments {
		float "p0",
	}
	returns	"void"

native "0xAA76052DDA9BFC3E"
	hash "0xAA76052DDA9BFC3E"
	jhash (0xD0F51299)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"void"

native "SET_PED_PATHS_BACK_TO_ORIGINAL"
	hash "0xE04B48F2CC926253"
	jhash (0x3F1ABDA4)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"void"

native "GET_RANDOM_VEHICLE_NODE"
	hash "0x93E0DB8440B73A7D"
	jhash (0xAD1476EA)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",

		BOOL "p4",

		BOOL "p5",

		BOOL "p6",

		Vector3Ptr "outPosition",

		floatPtr "heading",
	}
	returns	"BOOL"

--[[!
<summary>
	Determines the name of the street which is the closest to the given coordinates.

	x,y,z - the coordinates of the street
	streetName - returns a hash to the name of the street the coords are on
	crossingRoad - if the coordinates are on an intersection, a hash to the name of the crossing road

	Note: the names are returned as hashes, the strings can be returned using the function UI::GET_STREET_NAME_FROM_HASH_KEY.
</summary>
]]--
native "GET_STREET_NAME_AT_COORD"
	hash "0x2EB41072B4C1E4C0"
	jhash (0xDEBEEFCF)
	arguments {
		float "x",

		float "y",

		float "z",

		HashPtr "streetName",

		HashPtr "crossingRoad",
	}
	returns	"void"

native "GENERATE_DIRECTIONS_TO_COORD"
	hash "0xF90125F1F79ECDF8"
	jhash (0xED35C094)
	arguments {
		float "x",

		float "y",

		float "z",

		Any "p3",

		Any "p4",

		VehiclePtr "vehicle",

		Any "p6",
	}
	returns	"Any"

native "SET_IGNORE_NO_GPS_FLAG"
	hash "0x72751156E7678833"
	jhash (0xB72CF194)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x1FC289A0C3FF470F"
	hash "0x1FC289A0C3FF470F"
	jhash (0x90DF7A4C)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "SET_GPS_DISABLED_ZONE"
	hash "0xDC20483CD3DD5201"
	jhash (0x720B8073)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"void"

native "0xBBB45C3CF5C8AA85"
	hash "0xBBB45C3CF5C8AA85"
	jhash (0x4B770634)
	returns	"Any"

native "0x869DAACBBE9FA006"
	hash "0x869DAACBBE9FA006"
	jhash (0x286F82CC)
	returns	"Any"

native "0x16F46FB18C8009E4"
	hash "0x16F46FB18C8009E4"
	jhash (0xF6422F9A)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"Any"

native "IS_POINT_ON_ROAD"
	hash "0x125BF4ABFC536B09"
	jhash (0xCF198055)
	arguments {
		float "x",

		float "y",

		float "z",

		Entity "entity",
	}
	returns	"BOOL"

native "0xD3A6A0EF48823A8C"
	hash "0xD3A6A0EF48823A8C"
	returns	"Any"

native "0xD0BC1C6FB18EE154"
	hash "0xD0BC1C6FB18EE154"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"void"

native "0x2801D0012266DF07"
	hash "0x2801D0012266DF07"
	arguments {
		Any "p0",
	}
	returns	"void"

native "ADD_NAVMESH_REQUIRED_REGION"
	hash "0x387EAD7EE42F6685"
	jhash (0x12B086EA)
	arguments {
		float "p0",

		float "p1",

		float "p2",
	}
	returns	"void"

native "REMOVE_NAVMESH_REQUIRED_REGIONS"
	hash "0x916F0A3CDEC3445E"
	jhash (0x637BB680)
	returns	"void"

native "DISABLE_NAVMESH_IN_AREA"
	hash "0x4C8872D8CDBE1B8B"
	jhash (0x6E37F132)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"void"

native "ARE_ALL_NAVMESH_REGIONS_LOADED"
	hash "0x8415D95B194A3AEA"
	jhash (0x34C4E789)
	returns	"BOOL"

native "IS_NAVMESH_LOADED_IN_AREA"
	hash "0xF813C7E63F9062A5"
	jhash (0x4C2BA99E)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"BOOL"

native "0x01708E8DD3FF8C65"
	hash "0x01708E8DD3FF8C65"
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"Any"

native "ADD_NAVMESH_BLOCKING_OBJECT"
	hash "0xFCD5C8E06E502F5A"
	jhash (0x2952BA56)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		BOOL "p7",

		Any "p8",
	}
	returns	"Any"

native "UPDATE_NAVMESH_BLOCKING_OBJECT"
	hash "0x109E99373F290687"
	jhash (0x4E9776D0)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		Any "p8",
	}
	returns	"void"

native "REMOVE_NAVMESH_BLOCKING_OBJECT"
	hash "0x46399A7895957C0E"
	jhash (0x098602B0)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x0EAEB0DB4B132399"
	hash "0x0EAEB0DB4B132399"
	jhash (0x4B67D7EE)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x29C24BFBED8AB8FB"
	hash "0x29C24BFBED8AB8FB"
	jhash (0x3FE8C5A0)
	arguments {
		float "p0",

		float "p1",
	}
	returns	"float"

native "0x8ABE8608576D9CE3"
	hash "0x8ABE8608576D9CE3"
	jhash (0x3ED21C90)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"float"

native "0x336511A34F2E5185"
	hash "0x336511A34F2E5185"
	jhash (0xA07C5B7D)
	arguments {
		float "p0",

		float "p1",
	}
	returns	"float"

native "0x3599D741C9AC6310"
	hash "0x3599D741C9AC6310"
	jhash (0x76751DD4)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"float"

--[[!
<summary>
	Calculates the travel distance between a set of points.
</summary>
]]--
native "CALCULATE_TRAVEL_DISTANCE_BETWEEN_POINTS"
	hash "0xADD95C7005C4A197"
	jhash (0xB114489B)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",
	}
	returns	"float"

native "IS_CONTROL_ENABLED"
	hash "0x1CEA6BFDF248E5D9"
	jhash (0x9174AF84)
	arguments {
		int "index",

		int "control",
	}
	returns	"BOOL"

--[[!
<summary>
	index always is 2 for xbox 360 controller and razerblade
	control = one of these codes :
	A - 18 
	B 
	X 
	Y 
	LB 
	RB 
	L2 
	R2 
	L3 
	R3 
	Select 
	Start 

	D-Pad:
	Left 
	Right 
	Up 
	Down 

	Left-Stick:
	Left 
	Right 
	Up 
	Down 

	Right-Stick:
	Left 
	Right 
	Up 
	Down 

	~ sorry for the long comment
	~ houri.s :)

	---
	Note that the values don't necessarily equal a controller button. Instead they equal certain "actions" like jumping or sprinting. E.g. the function will also return true when 'W' is pressed given a value of 32.

	Will also only return true if the control is pressed at a certain strength. Consider checking whether the control value is more than 0 for more precise results, especially when working with the analog sticks (see GET_CONTROL_VALUE).


	For PC:

	index does not seem to matter. 
	Mapped keyboard keys are:

	7 
	8 
	9 
	10 - Page up 
	11 - Page down
	12 - mouse move down
	13 - mouse move right
	18 - space or enter
	19 - L Alt
	20 
	21 - L Shift
	23 
	32 
	34 
	36 - L Ctrl
	39 - [
	40 - ]
	44 
	45 
	51 
	56 
	57 
	58 
	60 - num pad 5
	61 - num pad 8
	197 - ]
	199 
	202 
	205 
	206 
	209 - L Shift
	211 
	213 
	214 
	215 
	216 
	217 - Caps lock
	224 - L Ctrl
	232 
	233 
	234 
	235 
	236 
	239 - Cam +
	240 - Cam -
	243 - ~
	244 
	245 
	246 
	249 
</summary>
]]--
native "IS_CONTROL_PRESSED"
	hash "0xF3A21BCD95725A4A"
	jhash (0x517A4384)
	arguments {
		int "index",

		int "control",
	}
	returns	"BOOL"

native "IS_CONTROL_RELEASED"
	hash "0x648EE3E7F38877DD"
	jhash (0x1F91A06E)
	arguments {
		int "index",

		int "control",
	}
	returns	"BOOL"

native "IS_CONTROL_JUST_PRESSED"
	hash "0x580417101DDB492F"
	jhash (0x4487F579)
	arguments {
		int "index",

		int "control",
	}
	returns	"BOOL"

native "IS_CONTROL_JUST_RELEASED"
	hash "0x50F940259D3841E6"
	jhash (0x2314444B)
	arguments {
		int "index",

		int "control",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns a value between 127 and 254 whereas 127 means no and 254 maximum input. Returns a value between 0 and 127 when the "opposite" control is used (e.g. a value of 254 for the analog stick to the left equals 0 to the right).
</summary>
]]--
native "GET_CONTROL_VALUE"
	hash "0xD95E79E8686D2C27"
	jhash (0xC526F3C6)
	arguments {
		int "index",

		int "control",
	}
	returns	"int"

native "GET_CONTROL_NORMAL"
	hash "0xEC3C9B8D5327B563"
	jhash (0x5DE226A5)
	arguments {
		int "index",

		int "control",
	}
	returns	"float"

native "0x5B73C77D9EB66E24"
	hash "0x5B73C77D9EB66E24"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x5B84D09CEC5209C5"
	hash "0x5B84D09CEC5209C5"
	jhash (0xC49343BB)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"float"

native "0xE8A25867FBA3B05E"
	hash "0xE8A25867FBA3B05E"
	arguments {
		Any "p0",

		Any "p1",

		float "p2",
	}
	returns	"BOOL"

native "IS_DISABLED_CONTROL_PRESSED"
	hash "0xE2587F8CBBD87B1D"
	jhash (0x32A93544)
	arguments {
		int "index",

		int "control",
	}
	returns	"BOOL"

native "IS_DISABLED_CONTROL_JUST_PRESSED"
	hash "0x91AEF906BCA88877"
	jhash (0xEE6ABD32)
	arguments {
		int "index",

		int "control",
	}
	returns	"BOOL"

native "IS_DISABLED_CONTROL_JUST_RELEASED"
	hash "0x305C8DCD79DA8B0F"
	jhash (0xD6A679E1)
	arguments {
		int "index",

		int "control",
	}
	returns	"BOOL"

native "GET_DISABLED_CONTROL_NORMAL"
	hash "0x11E65974A982637C"
	jhash (0x66FF4FAA)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"float"

--[[!
<summary>
	The "disabled" variant of _0x5B84D09CEC5209C5.
</summary>
]]--
native "0x4F8A26A890FD62FB"
	hash "0x4F8A26A890FD62FB"
	jhash (0xF2A65A4C)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"float"

native "0xD7D22F5592AED8BA"
	hash "0xD7D22F5592AED8BA"
	jhash (0x0E8EF929)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	index usually 2

	returns true if the last input method was made with mouse + keyboard, false if it was made with a gamepad
</summary>
]]--
native "_GET_LAST_INPUT_METHOD"
	hash "0xA571D46727E2B718"
	arguments {
		int "index",
	}
	returns	"BOOL"

native "0x13337B38DB572509"
	hash "0x13337B38DB572509"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xFC695459D4D0E219"
	hash "0xFC695459D4D0E219"
	arguments {
		float "p0",

		float "p1",
	}
	returns	"BOOL"

native "0x23F09EADC01449D6"
	hash "0x23F09EADC01449D6"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x6CD79468A1E595C6"
	hash "0x6CD79468A1E595C6"
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	p0 is either 0(zero) or 2.

	p2 appears to always be true.

	And yes, it returns a char*. Don't go changing that again.
</summary>
]]--
native "0x0499D7B09FC9B407"
	hash "0x0499D7B09FC9B407"
	jhash (0x3551727A)
	arguments {
		int "p0",

		int "control",

		BOOL "p2",
	}
	returns	"charPtr"

native "0x80C2FD58D720C801"
	hash "0x80C2FD58D720C801"
	jhash (0x3EE71F6A)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"Any"

native "0x8290252FFF36ACB5"
	hash "0x8290252FFF36ACB5"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0xCB0360EFEFB2580D"
	hash "0xCB0360EFEFB2580D"
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	p0 always seems to be 0
	duration in milliseconds 
	frequency should range from about 10 (slow vibration) to 255 (very fast)
</summary>
]]--
native "SET_PAD_SHAKE"
	hash "0x48B3886C1358D0D5"
	jhash (0x5D38BD2F)
	arguments {
		int "p0",

		int "duration",

		int "frequency",
	}
	returns	"void"

native "0x14D29BB12D47F68C"
	hash "0x14D29BB12D47F68C"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"void"

native "STOP_PAD_SHAKE"
	hash "0x38C16A305E8CDC8D"
	jhash (0x8F75657E)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xF239400E16C23E08"
	hash "0xF239400E16C23E08"
	jhash (0x7D65EB6E)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xA0CEFCEA390AAB9B"
	hash "0xA0CEFCEA390AAB9B"
	arguments {
		Any "p0",
	}
	returns	"void"

native "IS_LOOK_INVERTED"
	hash "0x77B612531280010D"
	jhash (0x313434B2)
	returns	"BOOL"

--[[!
<summary>
	Used with IS_LOOK_INVERTED() and negates its affect.
</summary>
]]--
native "0xE1615EC03B3BB4FD"
	hash "0xE1615EC03B3BB4FD"
	returns	"Any"

native "GET_LOCAL_PLAYER_AIM_STATE"
	hash "0xBB41AFBBBC0A0287"
	jhash (0x81802053)
	returns	"Any"

native "0x59B9A7AF4C95133C"
	hash "0x59B9A7AF4C95133C"
	returns	"Any"

native "0x0F70731BACCFBB96"
	hash "0x0F70731BACCFBB96"
	returns	"Any"

native "0xFC859E2374407556"
	hash "0xFC859E2374407556"
	returns	"Any"

native "SET_PLAYERPAD_SHAKES_WHEN_CONTROLLER_DISABLED"
	hash "0x798FDEB5B1575088"
	jhash (0xA86BD91F)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_INPUT_EXCLUSIVE"
	hash "0xEDE476E5EE29EDB1"
	jhash (0x4E8E29E6)
	arguments {
		int "index",

		int "control",
	}
	returns	"void"

native "DISABLE_CONTROL_ACTION"
	hash "0xFE99B66D079CF6BC"
	jhash (0x3800C0DC)
	arguments {
		int "index",

		int "control",

		BOOL "disable",
	}
	returns	"void"

native "ENABLE_CONTROL_ACTION"
	hash "0x351220255D64C155"
	jhash (0xD2753551)
	arguments {
		int "index",

		int "control",

		BOOL "enable",
	}
	returns	"void"

native "DISABLE_ALL_CONTROL_ACTIONS"
	hash "0x5F4B6931816E599B"
	jhash (0x16753CF4)
	arguments {
		int "index",
	}
	returns	"void"

native "ENABLE_ALL_CONTROL_ACTIONS"
	hash "0xA5FFE9B05F199DE7"
	jhash (0xFC2F119F)
	arguments {
		int "index",
	}
	returns	"void"

--[[!
<summary>
	Used in carsteal3 script with p0 = "Carsteal4_spycar".
</summary>
]]--
native "0x3D42B92563939375"
	hash "0x3D42B92563939375"
	jhash (0xD2C80B2E)
	arguments {
		charPtr "p0",
	}
	returns	"BOOL"

native "0x4683149ED1DDE7A1"
	hash "0x4683149ED1DDE7A1"
	jhash (0xBBFC9050)
	arguments {
		charPtr "p0",
	}
	returns	"BOOL"

native "0x643ED62D5EA3BEBD"
	hash "0x643ED62D5EA3BEBD"
	jhash (0x42140FF9)
	returns	"void"

native "0x7F4724035FDCA1DD"
	hash "0x7F4724035FDCA1DD"
	jhash (0x2CEDE6C5)
	arguments {
		int "index",
	}
	returns	"void"

native "0xAD6875BBC0FC899C"
	hash "0xAD6875BBC0FC899C"
	jhash (0x621388FF)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x6CC86E78358D5119"
	hash "0x6CC86E78358D5119"
	returns	"void"

native "0xFCCAE5B92A830878"
	hash "0xFCCAE5B92A830878"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x15FF52B809DB2353"
	hash "0x15FF52B809DB2353"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xF8CC1EBE0B62E29F"
	hash "0xF8CC1EBE0B62E29F"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x22DA66936E0FFF37"
	hash "0x22DA66936E0FFF37"
	jhash (0xB41064A4)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x8F5EA1C01D65A100"
	hash "0x8F5EA1C01D65A100"
	jhash (0x9DB63CFF)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xC84527E235FCA219"
	hash "0xC84527E235FCA219"
	jhash (0xF09157B0)
	arguments {
		charPtr "p0",

		BOOL "p1",

		charPtr "p2",

		AnyPtr "p3",

		AnyPtr "p4",

		charPtr "type",

		BOOL "p6",
	}
	returns	"BOOL"

--[[!
<summary>
	NOTE: 'p1' might be some kind of array.
</summary>
]]--
native "0xA5EFC3E847D60507"
	hash "0xA5EFC3E847D60507"
	jhash (0xD96860FC)
	arguments {
		charPtr "p0",

		charPtr "p1",

		charPtr "p2",

		charPtr "p3",

		BOOL "p4",
	}
	returns	"BOOL"

native "0x648E7A5434AF7969"
	hash "0x648E7A5434AF7969"
	jhash (0x459F2683)
	arguments {
		charPtr "p0",

		AnyPtr "p1",

		BOOL "p2",

		AnyPtr "p3",

		AnyPtr "p4",

		AnyPtr "p5",

		charPtr "type",
	}
	returns	"BOOL"

--[[!
<summary>
	NOTE: 'p2' might be some kind of array.
</summary>
]]--
native "0x4645DE9980999E93"
	hash "0x4645DE9980999E93"
	jhash (0xDBB83E2B)
	arguments {
		charPtr "p0",

		charPtr "p1",

		charPtr "p2",

		charPtr "p3",

		charPtr "type",
	}
	returns	"BOOL"

native "0x692D808C34A82143"
	hash "0x692D808C34A82143"
	jhash (0xBB6321BD)
	arguments {
		charPtr "p0",

		float "p1",

		charPtr "type",
	}
	returns	"BOOL"

native "0xA69AC4ADE82B57A4"
	hash "0xA69AC4ADE82B57A4"
	jhash (0xE8D56DA2)
	arguments {
		int "p0",
	}
	returns	"BOOL"

native "0x9CB0BFA7A9342C3D"
	hash "0x9CB0BFA7A9342C3D"
	jhash (0xCB6A351E)
	arguments {
		int "p0",

		BOOL "p1",
	}
	returns	"BOOL"

native "0x52818819057F2B40"
	hash "0x52818819057F2B40"
	jhash (0xA4D1B30E)
	arguments {
		int "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	if ((NETWORK::_597F8DBA9B206FC7() &gt; 0) &amp;&amp; DATAFILE::_01095C95CD46B624(0)) {
	    v_10 = DATAFILE::_GET_ROOT_OBJECT();
	    v_11 = DATAFILE::_OBJECT_VALUE_GET_INTEGER(v_10, "pt");
	    sub_20202(2, v_11);
	    a_0 += 1;
	} else { 
	    a_0 += 1;
	}

</summary>
]]--
native "0x01095C95CD46B624"
	hash "0x01095C95CD46B624"
	jhash (0xB8515B2F)
	arguments {
		int "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	Loads a User-Generated Content (UGC) file. These files can be found in "[GTA5]\data\ugc" and "[GTA5]\common\patch\ugc". They seem to follow a naming convention, most likely of "[name]_[part].ugc". See example below for usage.

	Returns whether or not the file was successfully loaded.

	Example:
	DATAFILE::_LOAD_UGC_FILE("RockstarPlaylists") // loads "rockstarplaylists_00.ugc"
</summary>
]]--
native "_LOAD_UGC_FILE"
	hash "0xC5238C011AF405E4"
	jhash (0x660C468E)
	arguments {
		charPtr "filename",
	}
	returns	"BOOL"

native "DATAFILE_CREATE"
	hash "0xD27058A1CA2B13EE"
	jhash (0x95F8A221)
	returns	"void"

native "DATAFILE_DELETE"
	hash "0x9AB9C1CFC8862DFB"
	jhash (0xDEF31B0A)
	returns	"void"

native "0x2ED61456317B8178"
	hash "0x2ED61456317B8178"
	jhash (0x4E03F632)
	returns	"void"

native "0xC55854C7D7274882"
	hash "0xC55854C7D7274882"
	jhash (0xF11F956F)
	returns	"void"

--[[!
<summary>
	Should actually be called "_GET_ROOT_OBJECT"
</summary>
]]--
native "0x906B778CA1DC72B6"
	hash "0x906B778CA1DC72B6"
	jhash (0x86DDF9C2)
	returns	"AnyPtr"

--[[!
<summary>
	Saves a JSON file? It might even be saving it to the Rockstar Cloud, but I have no way of verifying this

	"shrinkletter.c4", line ~378:
	DATAFILE::DATAFILE_CREATE();
	v_5 = DATAFILE::PRELOAD_FIND();
	DATAFILE::_E7E035450A7948D5(v_5, "in", a_2);
	DATAFILE::_8FF3847DADD8E30C(v_5, "st", &amp;a_2._f1);
	DATAFILE::_8FF3847DADD8E30C(v_5, "mp", &amp;a_2._f2);
	DATAFILE::_8FF3847DADD8E30C(v_5, "ms", &amp;a_2._f3);
	DATAFILE::_8FF3847DADD8E30C(v_5, "sc", &amp;a_2._f5);
	DATAFILE::_8FF3847DADD8E30C(v_5, "pr", &amp;a_2._f6);
	DATAFILE::_8FF3847DADD8E30C(v_5, "fa", &amp;a_2._f7);
	DATAFILE::_8FF3847DADD8E30C(v_5, "sm", &amp;a_2._f8);
	DATAFILE::_8FF3847DADD8E30C(v_5, "kp", &amp;a_2._f9);
	DATAFILE::_8FF3847DADD8E30C(v_5, "sv", &amp;a_2._fA);
	DATAFILE::_8FF3847DADD8E30C(v_5, "yo", &amp;a_2._fB);
	DATAFILE::_8FF3847DADD8E30C(v_5, "fi", &amp;a_2._fC);
	DATAFILE::_8FF3847DADD8E30C(v_5, "rc", &amp;a_2._fD);
	DATAFILE::_8FF3847DADD8E30C(v_5, "co", &amp;a_2._fE);
	DATAFILE::_E7E035450A7948D5(v_5, "su", a_2._fF);
	DATAFILE::_83BCCE3224735F05("gta5/psych/index.json"); // saves the file?
</summary>
]]--
native "0x83BCCE3224735F05"
	hash "0x83BCCE3224735F05"
	jhash (0x768CBB35)
	arguments {
		charPtr "filename",
	}
	returns	"BOOL"

--[[!
<summary>
	This one's kind of weird. It seems to take a pointer to a BOOL and fills it with either TRUE/FALSE?
</summary>
]]--
native "0x4DFDD9EB705F8140"
	hash "0x4DFDD9EB705F8140"
	jhash (0x0B4087F7)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	Example:
	if (!DATAFILE::_BEDB96A7584AA8CF())
	{
	    if (!g_109E3)
		{
	        if (((sub_d4f() == 2) == 0) &amp;&amp; (!NETWORK::NETWORK_IS_GAME_IN_PROGRESS()))
			{
	            if (NETWORK::NETWORK_IS_CLOUD_AVAILABLE())
				{
	                g_17A8B = 0;
	            }
	            if (!g_D52C)
				{
	                sub_730();
	            }
	        }
	    }
	}


</summary>
]]--
native "0xBEDB96A7584AA8CF"
	hash "0xBEDB96A7584AA8CF"
	jhash (0x5DCD0796)
	returns	"BOOL"

native "_OBJECT_VALUE_ADD_BOOLEAN"
	hash "0x35124302A556A325"
	jhash (0x9B29D99B)
	arguments {
		AnyPtr "objectData",

		charPtr "key",

		BOOL "value",
	}
	returns	"void"

native "_OBJECT_VALUE_ADD_INTEGER"
	hash "0xE7E035450A7948D5"
	jhash (0xEFCF554A)
	arguments {
		AnyPtr "objectData",

		charPtr "key",

		int "value",
	}
	returns	"void"

native "_OBJECT_VALUE_ADD_FLOAT"
	hash "0xC27E1CC2D795105E"
	jhash (0xE972CACF)
	arguments {
		AnyPtr "objectData",

		charPtr "key",

		float "value",
	}
	returns	"void"

native "_OBJECT_VALUE_ADD_STRING"
	hash "0x8FF3847DADD8E30C"
	jhash (0xD437615C)
	arguments {
		AnyPtr "objectData",

		charPtr "key",

		charPtr "value",
	}
	returns	"void"

native "_OBJECT_VALUE_ADD_VECTOR3"
	hash "0x4CD49B76338C7DEE"
	jhash (0x75FC6C3C)
	arguments {
		AnyPtr "objectData",

		charPtr "key",

		float "valueX",

		float "valueY",

		float "valueZ",
	}
	returns	"void"

native "_OBJECT_VALUE_ADD_OBJECT"
	hash "0xA358F56F10732EE1"
	jhash (0x96A8E05F)
	arguments {
		AnyPtr "objectData",

		charPtr "key",
	}
	returns	"AnyPtr"

native "_OBJECT_VALUE_ADD_ARRAY"
	hash "0x5B11728527CA6E5F"
	jhash (0x03939B8D)
	arguments {
		AnyPtr "objectData",

		charPtr "key",
	}
	returns	"AnyPtr"

native "_OBJECT_VALUE_GET_BOOLEAN"
	hash "0x1186940ED72FFEEC"
	jhash (0x8876C872)
	arguments {
		AnyPtr "objectData",

		charPtr "key",
	}
	returns	"BOOL"

native "_OBJECT_VALUE_GET_INTEGER"
	hash "0x78F06F6B1FB5A80C"
	jhash (0xA6C68693)
	arguments {
		AnyPtr "objectData",

		charPtr "key",
	}
	returns	"int"

native "_OBJECT_VALUE_GET_FLOAT"
	hash "0x06610343E73B9727"
	jhash (0xA92C1AF4)
	arguments {
		AnyPtr "objectData",

		charPtr "key",
	}
	returns	"float"

native "_OBJECT_VALUE_GET_STRING"
	hash "0x3D2FD9E763B24472"
	jhash (0x942160EC)
	arguments {
		AnyPtr "objectData",

		charPtr "key",
	}
	returns	"charPtr"

native "_OBJECT_VALUE_GET_VECTOR3"
	hash "0x46CD3CB66E0825CC"
	jhash (0xE84A127A)
	arguments {
		AnyPtr "objectData",

		charPtr "key",
	}
	returns	"Vector3"

native "_OBJECT_VALUE_GET_OBJECT"
	hash "0xB6B9DDC412FCEEE2"
	jhash (0xC9C13D8D)
	arguments {
		AnyPtr "objectData",

		charPtr "key",
	}
	returns	"AnyPtr"

native "_OBJECT_VALUE_GET_ARRAY"
	hash "0x7A983AA9DA2659ED"
	jhash (0x1F2F7D00)
	arguments {
		AnyPtr "objectData",

		charPtr "key",
	}
	returns	"AnyPtr"

--[[!
<summary>
	Types:
	1 = Boolean
	2 = Integer
	3 = Float
	4 = String
	5 = Vector3
	6 = Object
	7 = Array
</summary>
]]--
native "_OBJECT_VALUE_GET_TYPE"
	hash "0x031C55ED33227371"
	jhash (0x2678342A)
	arguments {
		AnyPtr "objectData",

		charPtr "key",
	}
	returns	"int"

native "_ARRAY_VALUE_ADD_BOOLEAN"
	hash "0xF8B0F5A43E928C76"
	jhash (0x08174B90)
	arguments {
		AnyPtr "arrayData",

		BOOL "value",
	}
	returns	"void"

native "_ARRAY_VALUE_ADD_INTEGER"
	hash "0xCABDB751D86FE93B"
	jhash (0xF29C0B36)
	arguments {
		AnyPtr "arrayData",

		int "value",
	}
	returns	"void"

native "_ARRAY_VALUE_ADD_FLOAT"
	hash "0x57A995FD75D37F56"
	jhash (0xE4302123)
	arguments {
		AnyPtr "arrayData",

		float "value",
	}
	returns	"void"

native "_ARRAY_VALUE_ADD_STRING"
	hash "0x2F0661C155AEEEAA"
	jhash (0xF3C01350)
	arguments {
		AnyPtr "arrayData",

		charPtr "value",
	}
	returns	"void"

native "_ARRAY_VALUE_ADD_VECTOR3"
	hash "0x407F8D034F70F0C2"
	jhash (0x16F464B6)
	arguments {
		AnyPtr "arrayData",

		float "valueX",

		float "valueY",

		float "valueZ",
	}
	returns	"void"

native "_ARRAY_VALUE_ADD_OBJECT"
	hash "0x6889498B3E19C797"
	jhash (0xC174C71B)
	arguments {
		AnyPtr "arrayData",
	}
	returns	"AnyPtr"

native "_ARRAY_VALUE_GET_BOOLEAN"
	hash "0x50C1B2874E50C114"
	jhash (0xA2E5F921)
	arguments {
		AnyPtr "arrayData",

		int "arrayIndex",
	}
	returns	"BOOL"

native "_ARRAY_VALUE_GET_INTEGER"
	hash "0x3E5AE19425CD74BE"
	jhash (0xBB120CFC)
	arguments {
		AnyPtr "arrayData",

		int "arrayIndex",
	}
	returns	"int"

native "_ARRAY_VALUE_GET_FLOAT"
	hash "0xC0C527B525D7CFB5"
	jhash (0x08AD2CC2)
	arguments {
		AnyPtr "arrayData",

		int "arrayIndex",
	}
	returns	"float"

native "_ARRAY_VALUE_GET_STRING"
	hash "0xD3F2FFEB8D836F52"
	jhash (0x93F985A6)
	arguments {
		AnyPtr "arrayData",

		int "arrayIndex",
	}
	returns	"charPtr"

native "_ARRAY_VALUE_GET_VECTOR3"
	hash "0x8D2064E5B64A628A"
	jhash (0x80E3DA55)
	arguments {
		AnyPtr "arrayData",

		int "arrayIndex",
	}
	returns	"Vector3"

native "_ARRAY_VALUE_GET_OBJECT"
	hash "0x8B5FADCC4E3A145F"
	jhash (0xECE81278)
	arguments {
		AnyPtr "arrayData",

		int "arrayIndex",
	}
	returns	"AnyPtr"

native "_ARRAY_VALUE_GET_SIZE"
	hash "0x065DB281590CEA2D"
	jhash (0xA8A21766)
	arguments {
		AnyPtr "arrayData",
	}
	returns	"int"

--[[!
<summary>
	Types:
	1 = Boolean
	2 = Integer
	3 = Float
	4 = String
	5 = Vector3
	6 = Object
	7 = Array
</summary>
]]--
native "_ARRAY_VALUE_GET_TYPE"
	hash "0x3A0014ADB172A3C5"
	jhash (0xFA2402C8)
	arguments {
		AnyPtr "arrayData",

		int "arrayIndex",
	}
	returns	"int"

--[[!
<summary>
	Starts a fire:

	xyz: Location of fire
	int: The max amount of times a fire can spread to other objects.
	bool: Whether or not the fire is powered by gasoline.
</summary>
]]--
native "START_SCRIPT_FIRE"
	hash "0x6B83617E04503888"
	jhash (0xE7529357)
	arguments {
		float "X",

		float "Y",

		float "Z",

		int "maxChildren",

		BOOL "isGasFire",
	}
	returns	"Any"

native "REMOVE_SCRIPT_FIRE"
	hash "0x7FF548385680673F"
	jhash (0x6B21FE26)
	arguments {
		Any "scriptHandle",
	}
	returns	"void"

native "START_ENTITY_FIRE"
	hash "0xF6A9D9708F6F23DF"
	jhash (0x8928428E)
	arguments {
		Entity "entity",
	}
	returns	"Any"

native "STOP_ENTITY_FIRE"
	hash "0x7F0DD2EBBB651AFF"
	jhash (0xCE8C9066)
	arguments {
		Entity "entity",
	}
	returns	"void"

native "IS_ENTITY_ON_FIRE"
	hash "0x28D3FED7190D3A0B"
	jhash (0x8C73E64F)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "GET_NUMBER_OF_FIRES_IN_RANGE"
	hash "0x50CAD495A460B305"
	jhash (0x654D93B7)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",
	}
	returns	"int"

native "STOP_FIRE_IN_RANGE"
	hash "0x056A8A219B8E829F"
	jhash (0x725C7205)
	arguments {
		float "x",

		float "y",

		float "z",

		float "radius",
	}
	returns	"void"

--[[!
<summary>
	Returns TRUE if it found something. FALSE if not.
</summary>
]]--
native "GET_CLOSEST_FIRE_POS"
	hash "0x352A9F6BCF90081F"
	jhash (0xC4977B47)
	arguments {
		EntityPtr "fire",

		float "x",

		float "y",

		float "z",
	}
	returns	"BOOL"

--[[!
<summary>
	enum ExplosionTypes
	{
		EXPLOSION_GRENADE,
		EXPLOSION_GRENADELAUNCHER,
		EXPLOSION_STICKYBOMB,
		EXPLOSION_MOLOTOV,
		EXPLOSION_ROCKET,
		EXPLOSION_TANKSHELL,
		EXPLOSION_HI_OCTANE,
		EXPLOSION_CAR,
		EXPLOSION_PLANE,
		EXPLOSION_PETROL_PUMP,
		EXPLOSION_BIKE,
		EXPLOSION_DIR_STEAM,
		EXPLOSION_DIR_FLAME,
		EXPLOSION_DIR_WATER_HYDRANT,
		EXPLOSION_DIR_GAS_CANISTER,
		EXPLOSION_BOAT,
		EXPLOSION_SHIP_DESTROY,
		EXPLOSION_TRUCK,
		EXPLOSION_BULLET,
		EXPLOSION_SMOKEGRENADELAUNCHER,
		EXPLOSION_SMOKEGRENADE,
		EXPLOSION_BZGAS,
		EXPLOSION_FLARE,
		EXPLOSION_GAS_CANISTER,
		EXPLOSION_EXTINGUISHER,
		EXPLOSION_PROGRAMMABLEAR,
		EXPLOSION_TRAIN,
		EXPLOSION_BARREL,
		EXPLOSION_PROPANE,
		EXPLOSION_BLIMP,
		EXPLOSION_DIR_FLAME_EXPLODE,
		EXPLOSION_TANKER,
		EXPLOSION_PLANE_ROCKET,
		EXPLOSION_VEHICLE_BULLET,
		EXPLOSION_GAS_TANK,
		EXPLOSION_BIRD_CRAP
	};
</summary>
]]--
native "ADD_EXPLOSION"
	hash "0xE3AD2BDBAEE269AC"
	jhash (0x10AF5258)
	arguments {
		float "x",

		float "y",

		float "z",

		int "explosionType",

		float "damageScale",

		BOOL "isAudible",

		BOOL "isInvisible",

		float "cameraShake",
	}
	returns	"void"

--[[!
<summary>
	enum ExplosionTypes
	{
		EXPLOSION_GRENADE,
		EXPLOSION_GRENADELAUNCHER,
		EXPLOSION_STICKYBOMB,
		EXPLOSION_MOLOTOV,
		EXPLOSION_ROCKET,
		EXPLOSION_TANKSHELL,
		EXPLOSION_HI_OCTANE,
		EXPLOSION_CAR,
		EXPLOSION_PLANE,
		EXPLOSION_PETROL_PUMP,
		EXPLOSION_BIKE,
		EXPLOSION_DIR_STEAM,
		EXPLOSION_DIR_FLAME,
		EXPLOSION_DIR_WATER_HYDRANT,
		EXPLOSION_DIR_GAS_CANISTER,
		EXPLOSION_BOAT,
		EXPLOSION_SHIP_DESTROY,
		EXPLOSION_TRUCK,
		EXPLOSION_BULLET,
		EXPLOSION_SMOKEGRENADELAUNCHER,
		EXPLOSION_SMOKEGRENADE,
		EXPLOSION_BZGAS,
		EXPLOSION_FLARE,
		EXPLOSION_GAS_CANISTER,
		EXPLOSION_EXTINGUISHER,
		EXPLOSION_PROGRAMMABLEAR,
		EXPLOSION_TRAIN,
		EXPLOSION_BARREL,
		EXPLOSION_PROPANE,
		EXPLOSION_BLIMP,
		EXPLOSION_DIR_FLAME_EXPLODE,
		EXPLOSION_TANKER,
		EXPLOSION_PLANE_ROCKET,
		EXPLOSION_VEHICLE_BULLET,
		EXPLOSION_GAS_TANK,
		EXPLOSION_BIRD_CRAP
	};
</summary>
]]--
native "ADD_OWNED_EXPLOSION"
	hash "0x172AA1B624FA1013"
	jhash (0x27EE0D67)
	arguments {
		Ped "ped",

		float "x",

		float "y",

		float "z",

		int "explosionType",

		float "damageScale",

		BOOL "isAudible",

		BOOL "isInvisible",

		float "cameraShake",
	}
	returns	"void"

--[[!
<summary>
	enum ExplosionTypes
	{
		EXPLOSION_GRENADE,
		EXPLOSION_GRENADELAUNCHER,
		EXPLOSION_STICKYBOMB,
		EXPLOSION_MOLOTOV,
		EXPLOSION_ROCKET,
		EXPLOSION_TANKSHELL,
		EXPLOSION_HI_OCTANE,
		EXPLOSION_CAR,
		EXPLOSION_PLANE,
		EXPLOSION_PETROL_PUMP,
		EXPLOSION_BIKE,
		EXPLOSION_DIR_STEAM,
		EXPLOSION_DIR_FLAME,
		EXPLOSION_DIR_WATER_HYDRANT,
		EXPLOSION_DIR_GAS_CANISTER,
		EXPLOSION_BOAT,
		EXPLOSION_SHIP_DESTROY,
		EXPLOSION_TRUCK,
		EXPLOSION_BULLET,
		EXPLOSION_SMOKEGRENADELAUNCHER,
		EXPLOSION_SMOKEGRENADE,
		EXPLOSION_BZGAS,
		EXPLOSION_FLARE,
		EXPLOSION_GAS_CANISTER,
		EXPLOSION_EXTINGUISHER,
		EXPLOSION_PROGRAMMABLEAR,
		EXPLOSION_TRAIN,
		EXPLOSION_BARREL,
		EXPLOSION_PROPANE,
		EXPLOSION_BLIMP,
		EXPLOSION_DIR_FLAME_EXPLODE,
		EXPLOSION_TANKER,
		EXPLOSION_PLANE_ROCKET,
		EXPLOSION_VEHICLE_BULLET,
		EXPLOSION_GAS_TANK,
		EXPLOSION_BIRD_CRAP
	};

</summary>
]]--
native "_ADD_SPECFX_EXPLOSION"
	hash "0x36DD3FE58B5E5212"
	jhash (0xCF358946)
	arguments {
		float "x",

		float "y",

		float "z",

		int "explosionType",

		Hash "explosionFx",

		float "damageScale",

		BOOL "isAudible",

		BOOL "isInvisible",

		float "cameraShake",
	}
	returns	"void"

native "IS_EXPLOSION_IN_AREA"
	hash "0x2E2EBA0EE7CED0E0"
	jhash (0xFB40075B)
	arguments {
		int "explosionType",

		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",
	}
	returns	"BOOL"

native "0x6070104B699B2EF4"
	hash "0x6070104B699B2EF4"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"Any"

native "IS_EXPLOSION_IN_SPHERE"
	hash "0xAB0F816885B0E483"
	jhash (0xD455A7F3)
	arguments {
		int "explosionType",

		float "x",

		float "y",

		float "z",

		float "radius",
	}
	returns	"BOOL"

native "IS_EXPLOSION_IN_ANGLED_AREA"
	hash "0xA079A6C51525DC4B"
	jhash (0x0128FED9)
	arguments {
		int "explosionType",

		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		float "angle",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns a handle to the first entity within the a circle spawned inside the 2 points from a radius. It could return a ped or an entity, but the scripts expect a ped, but still check if it's a ped.
</summary>
]]--
native "_GET_PED_INSIDE_EXPLOSION_AREA"
	hash "0x14BA4BA137AF6CEC"
	jhash (0xAEC0D176)
	arguments {
		int "explosionType",

		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		float "radius",
	}
	returns	"Entity"

native "SET_DECISION_MAKER"
	hash "0xB604A2942ADED0EE"
	jhash (0x19CEAC9E)
	arguments {
		Ped "ped",

		Hash "name",
	}
	returns	"void"

native "CLEAR_DECISION_MAKER_EVENT_RESPONSE"
	hash "0x4FC9381A7AEE8968"
	jhash (0x07ABD94D)
	arguments {
		Hash "name",

		int "type",
	}
	returns	"void"

native "BLOCK_DECISION_MAKER_EVENT"
	hash "0xE42FCDFD0E4196F7"
	jhash (0x57506EA6)
	arguments {
		Hash "name",

		int "type",
	}
	returns	"void"

native "UNBLOCK_DECISION_MAKER_EVENT"
	hash "0xD7CD9CF34F2C99E8"
	jhash (0x62A3161D)
	arguments {
		Hash "name",

		int "type",
	}
	returns	"void"

--[[!
<summary>
	duration is float here
</summary>
]]--
native "ADD_SHOCKING_EVENT_AT_POSITION"
	hash "0xD9F8455409B525E9"
	jhash (0x0B30F779)
	arguments {
		int "type",

		float "x",

		float "y",

		float "z",

		float "duration",
	}
	returns	"ScrHandle"

--[[!
<summary>
	duration is float here
</summary>
]]--
native "ADD_SHOCKING_EVENT_FOR_ENTITY"
	hash "0x7FD8F3BE76F89422"
	jhash (0xA81B5B71)
	arguments {
		int "type",

		Entity "entity",

		float "duration",
	}
	returns	"ScrHandle"

native "IS_SHOCKING_EVENT_IN_SPHERE"
	hash "0x1374ABB7C15BAB92"
	jhash (0x2F98823E)
	arguments {
		int "type",

		float "x",

		float "y",

		float "z",

		float "radius",
	}
	returns	"BOOL"

native "REMOVE_SHOCKING_EVENT"
	hash "0x2CDA538C44C6CCE5"
	jhash (0xF82D5A87)
	arguments {
		ScrHandle "event",
	}
	returns	"BOOL"

native "REMOVE_ALL_SHOCKING_EVENTS"
	hash "0xEAABE8FDFA21274C"
	jhash (0x64DF6282)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "REMOVE_SHOCKING_EVENT_SPAWN_BLOCKING_AREAS"
	hash "0x340F1415B68AEADE"
	jhash (0xA0CE89C8)
	returns	"void"

native "SUPPRESS_SHOCKING_EVENTS_NEXT_FRAME"
	hash "0x2F9A292AD0A3BD89"
	jhash (0x4CC674B5)
	returns	"void"

native "SUPPRESS_SHOCKING_EVENT_TYPE_NEXT_FRAME"
	hash "0x3FD2EC8BF1F1CF30"
	jhash (0xA0FDCB82)
	arguments {
		int "type",
	}
	returns	"void"

native "SUPPRESS_AGITATION_EVENTS_NEXT_FRAME"
	hash "0x5F3B7749C112D552"
	jhash (0x80340396)
	returns	"void"

native "GET_ZONE_AT_COORDS"
	hash "0x27040C25DE6CB2F4"
	jhash (0xC9018181)
	arguments {
		float "x",

		float "y",

		float "z",
	}
	returns	"int"

--[[!
<summary>
	'zoneName' corresponds to an entry in 'popzone.ipl'.
</summary>
]]--
native "GET_ZONE_FROM_NAME_ID"
	hash "0x98CD1D2934B76CC1"
	jhash (0x8EC68304)
	arguments {
		charPtr "zoneName",
	}
	returns	"int"

native "GET_ZONE_POPSCHEDULE"
	hash "0x4334BC40AA0CB4BB"
	jhash (0x20AB2FC9)
	arguments {
		int "zoneId",
	}
	returns	"int"

native "GET_NAME_OF_ZONE"
	hash "0xCD90657D4C30E1CA"
	jhash (0x7875CE91)
	arguments {
		float "x",

		float "y",

		float "z",
	}
	returns	"charPtr"

native "SET_ZONE_ENABLED"
	hash "0xBA5ECEEA120E5611"
	jhash (0x04E21B03)
	arguments {
		int "zoneId",

		BOOL "toggle",
	}
	returns	"void"

native "GET_ZONE_SCUMMINESS"
	hash "0x5F7B268D15BA0739"
	jhash (0xB2FB5C4C)
	arguments {
		int "zoneId",
	}
	returns	"int"

--[[!
<summary>
	Only used once in the decompiled scripts. Seems to be related to scripted vehicle generators.

	Modified example from "am_imp_exp.c4", line 6406:
	/* popSchedules[0] = ZONE::GET_ZONE_POPSCHEDULE(ZONE::GET_ZONE_AT_COORDS(891.3, 807.9, 188.1));
	etc.
	*/
	ZONE::OVERRIDE_POPSCHEDULE_VEHICLE_MODEL(popSchedules[index], vehicleHash);
	STREAMING::REQUEST_MODEL(vehicleHash);
</summary>
]]--
native "OVERRIDE_POPSCHEDULE_VEHICLE_MODEL"
	hash "0x5F7D596BAC2E7777"
	jhash (0x3F0A3680)
	arguments {
		int "scheduleId",

		Hash "vehicleHash",
	}
	returns	"void"

--[[!
<summary>
	Only used once in the decompiled scripts. Seems to be related to scripted vehicle generators.

	Modified example from "am_imp_exp.c4", line 6418:
	/* popSchedules[0] = ZONE::GET_ZONE_POPSCHEDULE(ZONE::GET_ZONE_AT_COORDS(891.3, 807.9, 188.1));
	etc.
	*/
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(vehicleHash);
	ZONE::CLEAR_POPSCHEDULE_OVERRIDE_VEHICLE_MODEL(popSchedules[index]);
</summary>
]]--
native "CLEAR_POPSCHEDULE_OVERRIDE_VEHICLE_MODEL"
	hash "0x5C0DE367AA0D911C"
	jhash (0x7A72A24E)
	arguments {
		int "scheduleId",
	}
	returns	"void"

--[[!
<summary>
	Returns a hash representing which part of the map the given coords are located.

	Possible return values:
	${city}
	${countryside}
</summary>
]]--
native "GET_HASH_OF_MAP_AREA_AT_COORDS"
	hash "0x7EE64D51E8498728"
	jhash (0xB5C5C99B)
	arguments {
		float "x",

		float "y",

		float "z",
	}
	returns	"Hash"

--[[!
<summary>
	Creates a rope at the specific position, that extends in the specified direction when not attached to any entities.
	__

	Add_Rope(pos.x,pos.y,pos.z,0.0,0.0,0.0,20.0,4,20.0,1.0,0.0,false,false,false,5.0,false,NULL)

	When attached, Position&lt;vector&gt; does not matter
	When attached, Angle&lt;vector&gt; does not matter

	Rope Type:
	4 and bellow is a thick rope
	5 and up are small metal wires
	0 crashes the game

	Max_length - Rope is forced to this length, generally best to keep this the same as your rope length.

	Rigid - If max length is zero, and this is set to false the rope will become rigid (it will force a specific distance, what ever length is, between the objects).

	breakable - Whether or not shooting the rope will break it.

	unkPtr - unknown ptr, always 0 in orig scripts
	__

	Lengths can be calculated like so:

	float distance = abs(x1 - x2) + abs(y1 - y2) + abs(z1 - z2); // Rope length


	NOTES:

	Rope does NOT interact with anything you attach it to, in some cases it make interact with the world AFTER it breaks (seems to occur if you set the type to -1).

	Rope will sometimes contract and fall to the ground like you'd expect it to, but since it doesn't interact with the world the effect is just jarring.
</summary>
]]--
native "ADD_ROPE"
	hash "0xE832D760399EB220"
	jhash (0xA592EC74)
	arguments {
		float "x",

		float "y",

		float "z",

		float "rotX",

		float "rotY",

		float "rotZ",

		float "length",

		int "ropeType",

		float "maxLength",

		float "minLength",

		float "p10",

		BOOL "p11",

		BOOL "p12",

		BOOL "rigid",

		float "p14",

		BOOL "breakWhenShot",

		AnyPtr "unkPtr",
	}
	returns	"Object"

native "DELETE_ROPE"
	hash "0x52B4829281364649"
	jhash (0x748D72AF)
	arguments {
		ObjectPtr "rope",
	}
	returns	"void"

native "DELETE_CHILD_ROPE"
	hash "0xAA5D6B1888E4DB20"
	jhash (0xB19B4706)
	arguments {
		Object "rope",
	}
	returns	"Any"

--[[!
<summary>
	Ptr is correct
</summary>
]]--
native "DOES_ROPE_EXIST"
	hash "0xFD5448BE3111ED96"
	jhash (0x66E4A3AC)
	arguments {
		ObjectPtr "rope",
	}
	returns	"BOOL"

native "ROPE_DRAW_SHADOW_ENABLED"
	hash "0xF159A63806BB5BA8"
	jhash (0x51523B8C)
	arguments {
		ObjectPtr "rope",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	Rope presets can be found in the gamefiles. One example is "ropeFamily3", it is NOT a hash but rather a string.
</summary>
]]--
native "LOAD_ROPE_DATA"
	hash "0xCBB203C04D1ABD27"
	jhash (0x9E8F1644)
	arguments {
		Object "rope",

		charPtr "rope_preset",
	}
	returns	"Any"

native "PIN_ROPE_VERTEX"
	hash "0x2B320CF14146B69A"
	jhash (0xAE1D101B)
	arguments {
		Object "rope",

		int "vertex",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "UNPIN_ROPE_VERTEX"
	hash "0x4B5AE2EEE4A8F180"
	jhash (0xB30B552F)
	arguments {
		Object "rope",

		int "vertex",
	}
	returns	"Any"

native "GET_ROPE_VERTEX_COUNT"
	hash "0x3655F544CD30F0B5"
	jhash (0x5131CD2C)
	arguments {
		Object "rope",
	}
	returns	"int"

--[[!
<summary>
	Attaches entity 1 to entity 2.
</summary>
]]--
native "ATTACH_ENTITIES_TO_ROPE"
	hash "0x3D95EC8B6D940AC3"
	jhash (0x7508668F)
	arguments {
		Object "rope",

		Entity "ent1",

		Entity "ent2",

		float "ent1_x",

		float "ent1_y",

		float "ent1_z",

		float "ent2_x",

		float "ent2_y",

		float "ent2_z",

		float "length",

		BOOL "p10",

		BOOL "p11",

		AnyPtr "p12",

		AnyPtr "p13",
	}
	returns	"Any"

--[[!
<summary>
	The position supplied can be anywhere, and the entity should anchor relative to that point from it's origin.
</summary>
]]--
native "ATTACH_ROPE_TO_ENTITY"
	hash "0x4B490A6832559A65"
	jhash (0xB25D9536)
	arguments {
		Any "rope",

		Entity "entity",

		float "x",

		float "y",

		float "z",

		BOOL "p5",
	}
	returns	"void"

native "DETACH_ROPE_FROM_ENTITY"
	hash "0xBCF3026912A8647D"
	jhash (0x3E720BEE)
	arguments {
		Any "rope",

		Entity "entity",
	}
	returns	"Any"

native "ROPE_SET_UPDATE_PINVERTS"
	hash "0xC8D667EE52114ABA"
	jhash (0xEAF291A0)
	arguments {
		Object "rope",
	}
	returns	"void"

native "0xDC57A637A20006ED"
	hash "0xDC57A637A20006ED"
	jhash (0x80DB77A7)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x36CCB9BE67B970FD"
	hash "0x36CCB9BE67B970FD"
	jhash (0xC67D5CF6)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x84DE3B5FB3E666F0"
	hash "0x84DE3B5FB3E666F0"
	jhash (0x7A18BB9C)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "GET_ROPE_LAST_VERTEX_COORD"
	hash "0x21BB0FBD3E217C2D"
	jhash (0x91F6848B)
	arguments {
		Object "rope",
	}
	returns	"Any"

native "GET_ROPE_VERTEX_COORD"
	hash "0xEA61CA8E80F09E4D"
	jhash (0x84374452)
	arguments {
		Object "rope",

		int "vertex",
	}
	returns	"Any"

native "START_ROPE_WINDING"
	hash "0x1461C72C889E343E"
	jhash (0x5187BED3)
	arguments {
		Object "rope",
	}
	returns	"void"

native "STOP_ROPE_WINDING"
	hash "0xCB2D4AB84A19AA7C"
	jhash (0x46826B53)
	arguments {
		Object "rope",
	}
	returns	"void"

native "START_ROPE_UNWINDING_FRONT"
	hash "0x538D1179EC1AA9A9"
	jhash (0xFC0DB4C3)
	arguments {
		Object "rope",
	}
	returns	"void"

native "STOP_ROPE_UNWINDING_FRONT"
	hash "0xFFF3A50779EFBBB3"
	jhash (0x2EEDB18F)
	arguments {
		Object "rope",
	}
	returns	"void"

native "ROPE_CONVERT_TO_SIMPLE"
	hash "0x5389D48EFA2F079A"
	jhash (0x43E92628)
	arguments {
		Object "rope",
	}
	returns	"void"

--[[!
<summary>
	Loads rope textures for all ropes in the current scene.
</summary>
]]--
native "ROPE_LOAD_TEXTURES"
	hash "0x9B9039DBF2D258C1"
	jhash (0xBA97CE91)
	returns	"Any"

native "ROPE_ARE_TEXTURES_LOADED"
	hash "0xF2D0E6A75CC05597"
	jhash (0x5FDC1047)
	returns	"BOOL"

--[[!
<summary>
	Unloads rope textures for all ropes in the current scene.
</summary>
]]--
native "ROPE_UNLOAD_TEXTURES"
	hash "0x6CE36C35C1AC8163"
	jhash (0x584463E0)
	returns	"Any"

native "0x271C9D3ACA5D6409"
	hash "0x271C9D3ACA5D6409"
	arguments {
		Object "rope",
	}
	returns	"BOOL"

native "0xBC0CE682D4D05650"
	hash "0xBC0CE682D4D05650"
	jhash (0x106BA127)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",

		Any "p9",

		Any "p10",

		Any "p11",

		Any "p12",

		Any "p13",
	}
	returns	"void"

native "0xB1B6216CA2E7B55E"
	hash "0xB1B6216CA2E7B55E"
	jhash (0x7C6F7668)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xB743F735C03D7810"
	hash "0xB743F735C03D7810"
	jhash (0x686672DD)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	Get a rope's length.  Can be modified with ROPE_FORCE_LENGTH
</summary>
]]--
native "_GET_ROPE_LENGTH"
	hash "0x73040398DFF9A4A6"
	jhash (0xFD309DC8)
	arguments {
		Object "rope",
	}
	returns	"float"

--[[!
<summary>
	Forces a rope to a certain length.
</summary>
]]--
native "ROPE_FORCE_LENGTH"
	hash "0xD009F759A723DB1B"
	jhash (0xABF3130F)
	arguments {
		Object "rope",

		float "length",
	}
	returns	"Any"

--[[!
<summary>
	Reset a rope to a certain length.
</summary>
]]--
native "ROPE_RESET_LENGTH"
	hash "0xC16DE94D9BEA14A0"
	jhash (0xC8A423A3)
	arguments {
		Object "rope",

		BOOL "length",
	}
	returns	"Any"

native "APPLY_IMPULSE_TO_CLOTH"
	hash "0xE37F721824571784"
	jhash (0xA2A5C9FE)
	arguments {
		float "posX",

		float "posY",

		float "posZ",

		float "vecX",

		float "vecY",

		float "vecZ",

		float "impulse",
	}
	returns	"void"

native "SET_DAMPING"
	hash "0xEEA3B200A6FEB65B"
	jhash (0xCFB37773)
	arguments {
		Object "rope",

		int "vertex",

		float "value",
	}
	returns	"void"

--[[!
<summary>
	seems to be frequently used with the NETWORK::NET_TO_x natives, particularly with vehicles. It is often the only ROPE:: native in a script
</summary>
]]--
native "ACTIVATE_PHYSICS"
	hash "0x710311ADF0E20730"
	jhash (0x031711B8)
	arguments {
		Entity "entity",
	}
	returns	"void"

native "SET_CGOFFSET"
	hash "0xD8FA3908D7B86904"
	jhash (0x59910AB2)
	arguments {
		Object "rope",

		float "x",

		float "y",

		float "z",
	}
	returns	"void"

native "GET_CGOFFSET"
	hash "0x8214A4B5A7A33612"
	jhash (0x49A11F0D)
	arguments {
		Object "rope",
	}
	returns	"Vector3"

native "SET_CG_AT_BOUNDCENTER"
	hash "0xBE520D9761FF811F"
	jhash (0xA5B55421)
	arguments {
		Object "rope",
	}
	returns	"void"

native "BREAK_ENTITY_GLASS"
	hash "0x2E648D16F6E308F3"
	jhash (0xD0E0402F)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		Any "p9",

		BOOL "p10",
	}
	returns	"void"

native "SET_DISABLE_BREAKING"
	hash "0x5CEC1A84620E7D5B"
	jhash (0xEE77C326)
	arguments {
		Object "rope",

		BOOL "enabled",
	}
	returns	"Any"

native "0xCC6E963682533882"
	hash "0xCC6E963682533882"
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	sometimes used used with NET_TO_OBJ
</summary>
]]--
native "SET_DISABLE_FRAG_DAMAGE"
	hash "0x01BA3AED21C16CFB"
	jhash (0x97269DC8)
	arguments {
		Object "object",

		BOOL "toggle",
	}
	returns	"void"

--[[!
<summary>
	This function set height to the value of z-axis of the water surface.

	This function works with sea and lake. However it does not work will with shallow river (e.g. raton canyon will return -100000.0f)
</summary>
]]--
native "GET_WATER_HEIGHT"
	hash "0xF6829842C06AE524"
	jhash (0xD864E17C)
	arguments {
		float "x",

		float "y",

		float "z",

		floatPtr "height",
	}
	returns	"BOOL"

native "GET_WATER_HEIGHT_NO_WAVES"
	hash "0x8EE6B53CE13A9794"
	jhash (0x262017F8)
	arguments {
		float "x",

		float "y",

		float "z",

		floatPtr "height",
	}
	returns	"BOOL"

native "TEST_PROBE_AGAINST_WATER"
	hash "0xFFA5D878809819DB"
	jhash (0xAA4AE00C)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",
	}
	returns	"BOOL"

native "TEST_PROBE_AGAINST_ALL_WATER"
	hash "0x8974647ED222EA5F"
	jhash (0x4A962D55)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",
	}
	returns	"BOOL"

native "TEST_VERTICAL_PROBE_AGAINST_ALL_WATER"
	hash "0x2B3451FA1E3142E2"
	jhash (0x4C71D143)
	arguments {
		float "x",

		float "y",

		float "z",

		Any "p3",

		AnyPtr "p4",
	}
	returns	"BOOL"

--[[!
<summary>
	Sets the water height for a given position.

</summary>
]]--
native "MODIFY_WATER"
	hash "0xC443FD757C3BA637"
	jhash (0xC49E005A)
	arguments {
		float "x",

		float "y",

		float "z",

		float "height",
	}
	returns	"void"

native "0xFDBF4CDBC07E1706"
	hash "0xFDBF4CDBC07E1706"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"Any"

native "0xB1252E3E59A82AAF"
	hash "0xB1252E3E59A82AAF"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xB96B00E976BE977F"
	hash "0xB96B00E976BE977F"
	arguments {
		float "p0",
	}
	returns	"void"

native "0x2B2A2CC86778B619"
	hash "0x2B2A2CC86778B619"
	returns	"Any"

native "0x5E5E99285AE812DB"
	hash "0x5E5E99285AE812DB"
	returns	"void"

--[[!
<summary>
	Returns a ray (?) going from x1, y1, z1 to x2, y2, z2.
	entity = 0 most of the time.
	p8 = 7 most of the time.

	Result of this function is passed to WORLDPROBE::_GET_RAYCAST_RESULT as a first argument.
</summary>
]]--
native "0x7EE9F5D83DD4F90E"
	hash "0x7EE9F5D83DD4F90E"
	jhash (0xEFAF4BA6)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		int "flags",

		Entity "entity",

		int "p8",
	}
	returns	"int"

--[[!
<summary>
	Not sure how or why this differs from 0x7EE9F5D83DD4F90E, but it does.

	This function casts a ray from Point1 to Point2 and returns it's ray handle.  A simple ray cast will 'shoot' a line from point A to point B, and return whether or not the ray reached it's destination or if it hit anything and if it did hit anything, will return the handle of what it hit (entity handle) and coordinates of where the ray reached.

	You can use _GET_RAYCAST_RESULT to get the result of the raycast

	Entity is an entity to ignore, such as the player.

	Flags are intersection bit flags.  They tell the ray what to care about and what not to care about when casting. Passing -1 will intersect with everything, presumably.

	Flags:
	1: Intersect with map
	2: Intersect with mission entities? (includes train)
	4: Intersect with peds? (same as 8)
	8: Intersect with peds? (same as 4)
	10: vehicles
	16: Intersect with objects
	19: Unkown
	32: Unknown
	64: Unknown
	128: Unknown
	256: Intersect with vegetation (plants, coral. trees not included)
	512: Unknown
</summary>
]]--
native "_CAST_RAY_POINT_TO_POINT"
	hash "0x377906D8A31E5586"
	jhash (0x8251485D)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		int "flags",

		Entity "entity",

		int "p8",
	}
	returns	"int"

native "0x052837721A854EC7"
	hash "0x052837721A854EC7"
	jhash (0xCEEAD94B)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "0xFE466162C4401D18"
	hash "0xFE466162C4401D18"
	jhash (0x249BC876)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		Any "p9",

		Any "p10",

		Any "p11",

		Any "p12",
	}
	returns	"Any"

native "0x37181417CE7C8900"
	hash "0x37181417CE7C8900"
	jhash (0x13BC46C0)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

--[[!
<summary>
	Raycast from point to point, where the ray has a radius. 

	flags:
	vehicles=10
	peds =12

	Iterating through flags yields many ped / vehicle/ object combinations

	p8 = 7, but no idea what it does

	Entity is an entity to ignore
</summary>
]]--
native "_CAST_3D_RAY_POINT_TO_POINT"
	hash "0x28579D1B8F8AAC80"
	jhash (0x591EA833)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",

		float "radius",

		int "flags",

		Entity "entity",

		int "p8",
	}
	returns	"Any"

native "0xE6AC6C45FBE83004"
	hash "0xE6AC6C45FBE83004"
	jhash (0x4559460A)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		Any "p7",

		Any "p8",

		Any "p9",
	}
	returns	"Any"

native "0xFF6BE494C7987F34"
	hash "0xFF6BE494C7987F34"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"Any"

--[[!
<summary>
	Parameters:
	rayHandle - Ray Handle from a casted ray, as returned by CAST_RAY_POINT_TO_POINT
	hit - Where to store whether or not it hit anything. False is when the ray reached its destination.
	endCoords - Where to store the world-coords of where the ray was stopped (by hitting its desired max range or by colliding with an entity/the map)
	surfaceNormal - Where to store the surface-normal coords (NOT relative to the game world) of where the entity was hit by the ray
	entityHit - Where to store the handle of the entity hit by the ray

	Returns:
	Result? Some type of enum.

	NOTE: To get the offset-coords of where the ray hit relative to the entity that it hit (which is NOT the same as surfaceNormal), you can use these two natives:
	Vector3 offset = ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(entityHit, endCoords.x, endCoords.y, endCoords.z);
	Vector3 entitySpotCoords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(entityHit, offset.x, offset.y, offset.z);

	Use ENTITY::GET_ENTITY_TYPE(entityHit) to quickly distinguish what type of entity you hit (ped/vehicle/object - 1/2/3)
</summary>
]]--
native "_GET_RAYCAST_RESULT"
	hash "0x3D87450E15D98694"
	jhash (0xF3C2875A)
	arguments {
		int "rayHandle",

		BOOLPtr "hit",

		Vector3Ptr "endCoords",

		Vector3Ptr "surfaceNormal",

		EntityPtr "entityHit",
	}
	returns	"int"

--[[!
<summary>
	behaves exactly the same way as _GET_RAYCAST_RESULT except it has one extra unknown parameter (unk). I tried using unk as a pointer/output parameter and I got garbage float values every time for each entity hit

	At first I was hoping maybe it was a pointer array (like the sizeAndPeds param in GET_PED_NEARBY_PEDS) for getting multiple entities caught in the CAST_3D_RAY_POINT_TO_POINT ray; but it's not, I tried.
</summary>
]]--
native "0x65287525D951F6BE"
	hash "0x65287525D951F6BE"
	jhash (0x4301E10C)
	arguments {
		int "rayHandle",

		BOOLPtr "hit",

		Vector3Ptr "endCoords",

		Vector3Ptr "surfaceNormal",

		Any "unk",

		EntityPtr "entityHit",
	}
	returns	"int"

native "0x2B3334BCA57CD799"
	hash "0x2B3334BCA57CD799"
	jhash (0xEC2AAF06)
	arguments {
		Entity "p0",
	}
	returns	"void"

--[[!
<summary>
	Returns whether the player is signed into Social Club.
</summary>
]]--
native "NETWORK_IS_SIGNED_IN"
	hash "0x054354A99211EB96"
	jhash (0xADD0B40F)
	returns	"Any"

--[[!
<summary>
	Returns whether the game is not in offline mode.
</summary>
]]--
native "NETWORK_IS_SIGNED_ONLINE"
	hash "0x1077788E268557C2"
	jhash (0x6E5BDCE2)
	returns	"Any"

native "0xBD545D44CCE70597"
	hash "0xBD545D44CCE70597"
	returns	"Any"

native "0xEBCAB9E5048434F4"
	hash "0xEBCAB9E5048434F4"
	returns	"Any"

native "0x74FB3E29E6D10FA9"
	hash "0x74FB3E29E6D10FA9"
	returns	"Any"

native "0x7808619F31FF22DB"
	hash "0x7808619F31FF22DB"
	returns	"Any"

native "0xA0FA4EC6A05DA44E"
	hash "0xA0FA4EC6A05DA44E"
	returns	"Any"

native "0x85443FF4C328F53B"
	hash "0x85443FF4C328F53B"
	jhash (0x3FB40673)
	returns	"Any"

native "0x8D11E61A4ABF49CC"
	hash "0x8D11E61A4ABF49CC"
	returns	"Any"

native "NETWORK_IS_CLOUD_AVAILABLE"
	hash "0x9A4CF4F48AD77302"
	jhash (0xC7FF5AFC)
	returns	"Any"

native "0x67A5589628E0CFF6"
	hash "0x67A5589628E0CFF6"
	jhash (0x66EC713F)
	returns	"Any"

native "0xBA9775570DB788CF"
	hash "0xBA9775570DB788CF"
	jhash (0x358D1D77)
	returns	"Any"

--[[!
<summary>
	if you are host, return true else return false
</summary>
]]--
native "NETWORK_IS_HOST"
	hash "0x8DB296B814EDDA07"
	jhash (0xE46AC10F)
	returns	"BOOL"

native "0xA306F470D1660581"
	hash "0xA306F470D1660581"
	returns	"Any"

native "0x4237E822315D8BA9"
	hash "0x4237E822315D8BA9"
	returns	"Any"

native "NETWORK_HAVE_ONLINE_PRIVILEGES"
	hash "0x25CB5A9F37BFD063"
	jhash (0xEF63BFDF)
	returns	"Any"

native "0x1353F87E89946207"
	hash "0x1353F87E89946207"
	returns	"Any"

native "0x72D918C99BCACC54"
	hash "0x72D918C99BCACC54"
	jhash (0x1F88819D)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xAEEF48CDF5B6CE7C"
	hash "0xAEEF48CDF5B6CE7C"
	jhash (0x2D817A5E)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x78321BEA235FD8CD"
	hash "0x78321BEA235FD8CD"
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"BOOL"

native "0x595F028698072DD9"
	hash "0x595F028698072DD9"
	jhash (0xBB54AA3D)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"BOOL"

native "0x83F28CE49FBBFFBA"
	hash "0x83F28CE49FBBFFBA"
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"BOOL"

native "0x76BF03FADBF154F5"
	hash "0x76BF03FADBF154F5"
	returns	"Any"

native "0x9614B71F8ADB982B"
	hash "0x9614B71F8ADB982B"
	returns	"Any"

native "0x5EA784D197556507"
	hash "0x5EA784D197556507"
	returns	"Any"

native "0xA8ACB6459542A8C8"
	hash "0xA8ACB6459542A8C8"
	returns	"Any"

native "0x83FE8D7229593017"
	hash "0x83FE8D7229593017"
	returns	"void"

native "NETWORK_CAN_BAIL"
	hash "0x580CE4438479CC61"
	jhash (0x60E1FEDF)
	returns	"BOOL"

native "NETWORK_BAIL"
	hash "0x95914459A87EBA28"
	jhash (0x87D79A04)
	returns	"void"

native "0x283B6062A2C01E9B"
	hash "0x283B6062A2C01E9B"
	jhash (0x96E28FE2)
	returns	"void"

native "0xAF50DA1A3F8B1BA4"
	hash "0xAF50DA1A3F8B1BA4"
	jhash (0xA520B982)
	arguments {
		intPtr "p0",
	}
	returns	"BOOL"

native "0x9747292807126EDA"
	hash "0x9747292807126EDA"
	jhash (0x05518C0F)
	returns	"Any"

native "NETWORK_CAN_ENTER_MULTIPLAYER"
	hash "0x7E782A910C362C25"
	jhash (0x4A23B9C9)
	returns	"BOOL"

native "NETWORK_SESSION_ENTER"
	hash "0x330ED4D05491934F"
	jhash (0x543CD2BE)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"Any"

--[[!
<summary>
	Appears to be a NETWORK_SESSION_HOST_* native.
</summary>
]]--
native "0x2CFC76E0D087C994"
	hash "0x2CFC76E0D087C994"
	jhash (0x4E53202A)
	arguments {
		int "p0",

		int "p1",

		int "maxPlayers",

		BOOL "p3",
	}
	returns	"BOOL"

--[[!
<summary>
	Appears to be a NETWORK_SESSION_HOST_* native.
</summary>
]]--
native "0x94BC51E9449D917F"
	hash "0x94BC51E9449D917F"
	jhash (0xD7624E6B)
	arguments {
		int "p0",

		int "p1",

		int "p2",

		int "maxPlayers",

		BOOL "p4",
	}
	returns	"BOOL"

native "0xBE3E347A87ACEB82"
	hash "0xBE3E347A87ACEB82"
	jhash (0x3F75CC38)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

--[[!
<summary>
	Does nothing in online but in offline it will cause the screen to fade to black. Nothing happens past then, the screen will sit at black until you restart GTA. Other stuff must be needed to actually host a session.
</summary>
]]--
native "NETWORK_SESSION_HOST"
	hash "0x6F3D4ED9BEE4E61D"
	jhash (0x6716460F)
	arguments {
		int "p0",

		int "maxPlayers",

		BOOL "p2",
	}
	returns	"BOOL"

--[[!
<summary>
	Appears to be a NETWORK_SESSION_HOST_* native.
</summary>
]]--
native "0xED34C0C02C098BB7"
	hash "0xED34C0C02C098BB7"
	jhash (0x8AC9EA19)
	arguments {
		int "p0",

		int "maxPlayers",
	}
	returns	"BOOL"

--[[!
<summary>
	Does nothing in online but in offline it will cause the screen to fade to black. Nothing happens past then, the screen will sit at black until you restart GTA. Other stuff must be needed to actually host a session.
</summary>
]]--
native "NETWORK_SESSION_HOST_FRIENDS_ONLY"
	hash "0xB9CFD27A5D578D83"
	jhash (0x26864403)
	arguments {
		int "p0",

		int "maxPlayers",
	}
	returns	"BOOL"

native "0xFBCFA2EA2E206890"
	hash "0xFBCFA2EA2E206890"
	jhash (0x56E75FE4)
	returns	"BOOL"

native "0x74732C6CA90DA2B4"
	hash "0x74732C6CA90DA2B4"
	jhash (0xA95299B9)
	returns	"BOOL"

native "0xF3929C2379B60CCE"
	hash "0xF3929C2379B60CCE"
	jhash (0x3D2C1916)
	returns	"BOOL"

native "0xCEF70AA5B3F89BA1"
	hash "0xCEF70AA5B3F89BA1"
	jhash (0xDB67785D)
	returns	"BOOL"

--[[!
<summary>
	p0 is always false and p1 varies.
</summary>
]]--
native "NETWORK_SESSION_END"
	hash "0xA02E59562D711006"
	jhash (0xBCACBEA2)
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	Only works when you are host.
</summary>
]]--
native "NETWORK_SESSION_KICK_PLAYER"
	hash "0xFA8904DC5F304220"
	jhash (0x1E20138A)
	arguments {
		Player "player",
	}
	returns	"void"

--[[!
<summary>
	Im sure the correct name for this native wouldn't be hard to find. I haven't tried anything since first figuring out this natives purpose.
</summary>
]]--
native "_NETWORK_SESSION_ARE_PLAYERS_VOTING_TO_KICK"
	hash "0xD6D09A6F32F49EF1"
	jhash (0x8A559D26)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "0x59DF79317F85A7E0"
	hash "0x59DF79317F85A7E0"
	returns	"Any"

native "0xFFE1E5B792D92B34"
	hash "0xFFE1E5B792D92B34"
	returns	"Any"

native "0x49EC8030F5015F8B"
	hash "0x49EC8030F5015F8B"
	jhash (0x3C3E2AB6)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x8B6A4DD0AF9CE215"
	hash "0x8B6A4DD0AF9CE215"
	jhash (0x5F29A7E0)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x56CE820830EF040B"
	hash "0x56CE820830EF040B"
	jhash (0x36EAD960)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xCAE55F48D3D7875C"
	hash "0xCAE55F48D3D7875C"
	jhash (0x5BE529F7)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xF49ABC20D8552257"
	hash "0xF49ABC20D8552257"
	jhash (0x454C7B67)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x4811BBAC21C5FCD5"
	hash "0x4811BBAC21C5FCD5"
	jhash (0xE5961511)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x5539C3EBF104A53A"
	hash "0x5539C3EBF104A53A"
	jhash (0xAE396263)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x702BC4D605522539"
	hash "0x702BC4D605522539"
	jhash (0x913FD7D6)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x3F52E880AAF6C8CA"
	hash "0x3F52E880AAF6C8CA"
	jhash (0xB3D9A67F)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xF1EEA2DDA9FFA69D"
	hash "0xF1EEA2DDA9FFA69D"
	jhash (0x6CC062FC)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x1153FA02A659051C"
	hash "0x1153FA02A659051C"
	jhash (0x57F9BC83)
	returns	"void"

native "0xC19F6C8E7865A6FF"
	hash "0xC19F6C8E7865A6FF"
	jhash (0xF3768F90)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x236406F60CF216D6"
	hash "0x236406F60CF216D6"
	jhash (0x0EC62629)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"void"

native "0x058F43EC59A8631A"
	hash "0x058F43EC59A8631A"
	jhash (0x5E557307)
	returns	"void"

native "0x6D03BFBD643B2A02"
	hash "0x6D03BFBD643B2A02"
	jhash (0x74E8C53E)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0x600F8CB31C7AAB6E"
	hash "0x600F8CB31C7AAB6E"
	jhash (0x959E43A3)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xE532D6811B3A4D2A"
	hash "0xE532D6811B3A4D2A"
	jhash (0x7771AB83)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xF7B2CFDE5C9F700D"
	hash "0xF7B2CFDE5C9F700D"
	jhash (0xA13045D4)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"BOOL"

native "NETWORK_IS_FINDING_GAMERS"
	hash "0xDDDF64C91BFCF0AA"
	jhash (0xA6DAA79F)
	returns	"Any"

native "0xF9B83B77929D8863"
	hash "0xF9B83B77929D8863"
	jhash (0xBEDC4503)
	returns	"Any"

native "NETWORK_GET_NUM_FOUND_GAMERS"
	hash "0xA1B043EE79A916FB"
	jhash (0xF4B80C7E)
	returns	"Any"

native "NETWORK_GET_FOUND_GAMER"
	hash "0x9DCFF2AFB68B3476"
	jhash (0xA08C9141)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"BOOL"

native "NETWORK_CLEAR_FOUND_GAMERS"
	hash "0x6D14CCEE1B40381A"
	jhash (0x6AA9A154)
	returns	"void"

native "0x85A0EF54A500882C"
	hash "0x85A0EF54A500882C"
	jhash (0x42BD0780)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x2CC848A861D01493"
	hash "0x2CC848A861D01493"
	jhash (0xBEB98840)
	returns	"Any"

native "0x94A8394D150B013A"
	hash "0x94A8394D150B013A"
	jhash (0x08029970)
	returns	"Any"

native "0x5AE17C6B0134B7F1"
	hash "0x5AE17C6B0134B7F1"
	jhash (0xC871E745)
	returns	"Any"

native "0x02A8BEC6FD9AF660"
	hash "0x02A8BEC6FD9AF660"
	jhash (0xB5ABC4B4)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x86E0660E4F5C956D"
	hash "0x86E0660E4F5C956D"
	jhash (0x3F7EDBBD)
	returns	"void"

native "NETWORK_IS_PLAYER_ANIMATION_DRAWING_SYNCHRONIZED"
	hash "0xC6F8AB8A4189CF3A"
	jhash (0x3D6360B5)
	returns	"void"

native "NETWORK_SESSION_CANCEL_INVITE"
	hash "0x2FBF47B1B36D36F9"
	jhash (0x20317535)
	returns	"void"

native "0xA29177F7703B5644"
	hash "0xA29177F7703B5644"
	jhash (0x3FD49D3B)
	returns	"void"

native "NETWORK_HAS_PENDING_INVITE"
	hash "0xAC8C7B9B88C4A668"
	jhash (0x0C207D6E)
	returns	"Any"

native "0xC42DD763159F3461"
	hash "0xC42DD763159F3461"
	jhash (0xFBBAC350)
	returns	"Any"

native "0x62A0296C1BB1CEB3"
	hash "0x62A0296C1BB1CEB3"
	jhash (0x0907A6BF)
	returns	"Any"

native "0x23DFB504655D0CE4"
	hash "0x23DFB504655D0CE4"
	jhash (0x6A0BEA60)
	returns	"Any"

native "NETWORK_SESSION_GET_INVITER"
	hash "0xE57397B4A3429DD0"
	jhash (0xE9C6B3FD)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xD313DE83394AF134"
	hash "0xD313DE83394AF134"
	jhash (0x3EA9D44C)
	returns	"Any"

native "0xBDB6F89C729CF388"
	hash "0xBDB6F89C729CF388"
	returns	"Any"

native "NETWORK_SUPPRESS_INVITE"
	hash "0xA0682D67EF1FBA3D"
	jhash (0x323DC78C)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "NETWORK_BLOCK_INVITES"
	hash "0x34F9E9049454A7A0"
	jhash (0xD156FD1A)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xCFEB8AF24FC1D0BB"
	hash "0xCFEB8AF24FC1D0BB"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xF814FEC6A19FD6E0"
	hash "0xF814FEC6A19FD6E0"
	jhash (0x32B7A076)
	returns	"void"

native "0x6B07B9CE4D390375"
	hash "0x6B07B9CE4D390375"
	jhash (0x0FCE995D)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x7AC752103856FB20"
	hash "0x7AC752103856FB20"
	jhash (0xA639DCA2)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x74698374C45701D2"
	hash "0x74698374C45701D2"
	jhash (0x70ED476A)
	returns	"Any"

native "0x140E6A44870A11CE"
	hash "0x140E6A44870A11CE"
	jhash (0x50507BED)
	returns	"void"

--[[!
<summary>
	Seems to put the singleplayer char in a Online Solo 

	Works without any parameters
</summary>
]]--
native "NETWORK_SESSION_HOST_SINGLE_PLAYER"
	hash "0xC74C33FCA52856D5"
	jhash (0xF3B1CA85)
	arguments {
		Any "p0",
	}
	returns	"void"

native "NETWORK_SESSION_LEAVE_SINGLE_PLAYER"
	hash "0x3442775428FD2DAA"
	jhash (0xC692F86A)
	returns	"void"

native "NETWORK_IS_GAME_IN_PROGRESS"
	hash "0x10FAB35428CCC9D7"
	jhash (0x09B88E3E)
	returns	"BOOL"

native "NETWORK_IS_SESSION_ACTIVE"
	hash "0xD83C2B94E7508980"
	jhash (0x715CB8C4)
	returns	"BOOL"

native "NETWORK_IS_IN_SESSION"
	hash "0xCA97246103B63917"
	jhash (0x4BC4105E)
	returns	"BOOL"

--[[!
<summary>
	This checks if player is playing on gta online or not.
	Please add an if and block your mod if this is "true".
</summary>
]]--
native "NETWORK_IS_SESSION_STARTED"
	hash "0x9DE624D2FC4B603F"
	jhash (0x9D854A37)
	returns	"BOOL"

native "NETWORK_IS_SESSION_BUSY"
	hash "0xF4435D66A8E2905E"
	jhash (0x8592152D)
	returns	"BOOL"

native "NETWORK_CAN_SESSION_END"
	hash "0x4EEBC3694E49C572"
	jhash (0xE1FCCDBA)
	returns	"BOOL"

native "0x271CC6AB59EBF9A5"
	hash "0x271CC6AB59EBF9A5"
	jhash (0x7017257D)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xBA416D68C631496A"
	hash "0xBA416D68C631496A"
	jhash (0x4977AC28)
	returns	"Any"

native "0xA73667484D7037C3"
	hash "0xA73667484D7037C3"
	jhash (0xE6EEF8AF)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xB4AB419E0D86ACAE"
	hash "0xB4AB419E0D86ACAE"
	jhash (0x6BB93227)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x53AFD64C6758F2F9"
	hash "0x53AFD64C6758F2F9"
	returns	"Any"

native "NETWORK_SESSION_VOICE_HOST"
	hash "0x9C1556705F864230"
	jhash (0x345C2980)
	returns	"void"

native "NETWORK_SESSION_VOICE_LEAVE"
	hash "0x6793E42BE02B575D"
	jhash (0xE566C7DA)
	returns	"void"

native "0xABD5E88B8A2D3DB2"
	hash "0xABD5E88B8A2D3DB2"
	jhash (0x9DFD89E6)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "NETWORK_SET_KEEP_FOCUSPOINT"
	hash "0x7F8413B7FC2AA6B9"
	jhash (0x075321B5)
	arguments {
		BOOL "p0",

		Any "p1",
	}
	returns	"void"

native "0x5B8ED3DB018927B1"
	hash "0x5B8ED3DB018927B1"
	jhash (0x6EFC2FD0)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x855BC38818F6F684"
	hash "0x855BC38818F6F684"
	jhash (0x60AA4AA1)
	returns	"Any"

native "0xB5D3453C98456528"
	hash "0xB5D3453C98456528"
	returns	"Any"

native "0xEF0912DDF7C4CB4B"
	hash "0xEF0912DDF7C4CB4B"
	jhash (0x132CA01E)
	returns	"Any"

native "NETWORK_SEND_TEXT_MESSAGE"
	hash "0x3A214F2EC889B100"
	jhash (0xAFFEA720)
	arguments {
		charPtr "message",

		intPtr "playerhandle",
	}
	returns	"Any"

native "NETWORK_SET_ACTIVITY_SPECTATOR"
	hash "0x75138790B4359A74"
	jhash (0xFC9AD060)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "NETWORK_IS_ACTIVITY_SPECTATOR"
	hash "0x12103B9E0C9F92FB"
	jhash (0xAF329720)
	returns	"Any"

native "NETWORK_SET_ACTIVITY_SPECTATOR_MAX"
	hash "0x9D277B76D1D12222"
	jhash (0x74E0BC0A)
	arguments {
		Any "p0",
	}
	returns	"void"

native "NETWORK_GET_ACTIVITY_PLAYER_NUM"
	hash "0x73E2B500410DA5A2"
	jhash (0x31F951FD)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "NETWORK_IS_ACTIVITY_SPECTATOR_FROM_HANDLE"
	hash "0x2763BBAA72A7BCB9"
	jhash (0x58F1DF7D)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "NETWORK_HOST_TRANSITION"
	hash "0xA60BB5CE242BB254"
	jhash (0x146764FB)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"Any"

native "0x71FB0EBCD4915D56"
	hash "0x71FB0EBCD4915D56"
	jhash (0x2FF65C0B)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0xA091A5E44F0072E5"
	hash "0xA091A5E44F0072E5"
	jhash (0x47D61C99)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "NETWORK_DO_TRANSITION_QUICKMATCH_WITH_GROUP"
	hash "0x9C4AB58491FDC98A"
	jhash (0x5CE60A11)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		AnyPtr "p4",

		Any "p5",
	}
	returns	"BOOL"

native "0xA06509A691D12BE4"
	hash "0xA06509A691D12BE4"
	jhash (0x0D7E5CF9)
	returns	"Any"

native "0xB13E88E655E5A3BC"
	hash "0xB13E88E655E5A3BC"
	jhash (0x36A5F2DA)
	returns	"void"

native "0x6512765E3BE78C50"
	hash "0x6512765E3BE78C50"
	returns	"Any"

native "0x0DBD5D7E3C5BEC3B"
	hash "0x0DBD5D7E3C5BEC3B"
	returns	"Any"

native "0x5DC577201723960A"
	hash "0x5DC577201723960A"
	returns	"Any"

native "0x5A6AA44FF8E931E6"
	hash "0x5A6AA44FF8E931E6"
	returns	"Any"

native "0x261E97AD7BCF3D40"
	hash "0x261E97AD7BCF3D40"
	jhash (0x7EF353E1)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x39917E1B4CB0F911"
	hash "0x39917E1B4CB0F911"
	jhash (0xF60986FC)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "NETWORK_SET_TRANSITION_CREATOR_HANDLE"
	hash "0xEF26739BCD9907D5"
	jhash (0x1DD01FE7)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "NETWORK_CLEAR_TRANSITION_CREATOR_HANDLE"
	hash "0xFB3272229A82C759"
	jhash (0x8BB336F7)
	returns	"void"

native "NETWORK_INVITE_GAMERS_TO_TRANSITION"
	hash "0x4A595C32F77DFF76"
	jhash (0x5332E645)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"BOOL"

native "NETWORK_SET_GAMER_INVITED_TO_TRANSITION"
	hash "0xCA2C8073411ECDB6"
	jhash (0x17F1C69D)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "NETWORK_LEAVE_TRANSITION"
	hash "0xD23A1A815D21DB19"
	jhash (0x3A3599B7)
	returns	"Any"

native "NETWORK_LAUNCH_TRANSITION"
	hash "0x2DCF46CB1A4F0884"
	jhash (0xE3570BA2)
	returns	"Any"

native "0xA2E9C1AB8A92E8CD"
	hash "0xA2E9C1AB8A92E8CD"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "NETWORK_BAIL_TRANSITION"
	hash "0xEAA572036990CD1B"
	jhash (0xB59D74CA)
	returns	"void"

native "NETWORK_DO_TRANSITION_TO_GAME"
	hash "0x3E9BB38102A589B0"
	jhash (0x1B2114D2)
	arguments {
		BOOL "p0",

		Any "p1",
	}
	returns	"BOOL"

native "NETWORK_DO_TRANSITION_TO_NEW_GAME"
	hash "0x4665F51EFED00034"
	jhash (0x58AFBE63)
	arguments {
		BOOL "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"BOOL"

--[[!
<summary>
	p2 is true 3/4 of the occurrences I found.
	'players' is the number of players for a session. On PS3/360 it's always 18. On PC it's 32.
</summary>
]]--
native "NETWORK_DO_TRANSITION_TO_FREEMODE"
	hash "0x3AAD8B2FCA1E289F"
	jhash (0xC7CB8ADF)
	arguments {
		AnyPtr "p0",

		Any "p1",

		BOOL "p2",

		int "players",
	}
	returns	"BOOL"

native "NETWORK_DO_TRANSITION_TO_NEW_FREEMODE"
	hash "0x9E80A5BA8109F974"
	jhash (0xAD13375E)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"BOOL"

native "NETWORK_IS_TRANSITION_TO_GAME"
	hash "0x9D7696D8F4FA6CB7"
	jhash (0x17146B2B)
	returns	"Any"

native "NETWORK_GET_TRANSITION_MEMBERS"
	hash "0x73B000F7FBC55829"
	jhash (0x31F19263)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"Any"

native "0x521638ADA1BA0D18"
	hash "0x521638ADA1BA0D18"
	jhash (0xCEE79711)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0xEBEFC2E77084F599"
	hash "0xEBEFC2E77084F599"
	jhash (0xE0C28DB5)
	arguments {
		Any "p0",

		AnyPtr "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	the first arg seems to be the network player handle (&amp;handle) and the second var is pretty much always "" and the third seems to be a number between 0 and ~10 and the 4th is is something like 0 to 5 and I guess the 5th is a bool cuz it is always 0 or 1
</summary>
]]--
native "NETWORK_SEND_TRANSITION_GAMER_INSTRUCTION"
	hash "0x31D1D2B858D25E6B"
	jhash (0x468B0884)
	arguments {
		PlayerPtr "playerHandle",

		charPtr "p1",

		int "p2",

		int "p3",

		BOOL "p4",
	}
	returns	"BOOL"

native "NETWORK_MARK_TRANSITION_GAMER_AS_FULLY_JOINED"
	hash "0x5728BB6D63E3FF1D"
	jhash (0x03383F57)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_TRANSITION_HOST"
	hash "0x0B824797C9BF2159"
	jhash (0x0C0900BC)
	returns	"Any"

native "NETWORK_IS_TRANSITION_HOST_FROM_HANDLE"
	hash "0x6B5C83BA3EFE6A10"
	jhash (0x0E2854C4)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_GET_TRANSITION_HOST"
	hash "0x65042B9774C4435E"
	jhash (0x73098D40)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_IN_TRANSITION"
	hash "0x68049AEFF83D8F0A"
	jhash (0xC3CDB626)
	returns	"Any"

native "NETWORK_IS_TRANSITION_STARTED"
	hash "0x53FA83401D9C07FE"
	jhash (0x7917E111)
	returns	"Any"

native "NETWORK_IS_TRANSITION_BUSY"
	hash "0x520F3282A53D26B7"
	jhash (0xA357A2C6)
	returns	"Any"

native "0x292564C735375EDF"
	hash "0x292564C735375EDF"
	jhash (0x8262C70E)
	returns	"Any"

native "0xC571D0E77D8BBC29"
	hash "0xC571D0E77D8BBC29"
	returns	"Any"

native "0x2B3A8F7CA3A38FDE"
	hash "0x2B3A8F7CA3A38FDE"
	jhash (0xC71E607B)
	returns	"void"

native "0x43F4DBA69710E01E"
	hash "0x43F4DBA69710E01E"
	jhash (0x82D32D07)
	returns	"void"

native "0x37A4494483B9F5C9"
	hash "0x37A4494483B9F5C9"
	jhash (0xC901AA9F)
	returns	"Any"

native "0x0C978FDA19692C2C"
	hash "0x0C978FDA19692C2C"
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xD0A484CB2F829FBE"
	hash "0xD0A484CB2F829FBE"
	returns	"Any"

native "0x30DE938B516F0AD2"
	hash "0x30DE938B516F0AD2"
	jhash (0xCCA9C022)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xEEEDA5E6D7080987"
	hash "0xEEEDA5E6D7080987"
	jhash (0x1E5F6AEF)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x973D76AA760A6CB6"
	hash "0x973D76AA760A6CB6"
	jhash (0x0532DDD2)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "NETWORK_HAS_PLAYER_STARTED_TRANSITION"
	hash "0x9AC9CCBFA8C29795"
	jhash (0x4ABD1E59)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x2615AA2A695930C1"
	hash "0x2615AA2A695930C1"
	jhash (0xCDEBCCE7)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_JOIN_TRANSITION"
	hash "0x9D060B08CD63321A"
	jhash (0xB054EC4B)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_HAS_INVITED_GAMER_TO_TRANSITION"
	hash "0x7284A47B3540E6CF"
	jhash (0x4F41DF6B)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x3F9990BF5F22759C"
	hash "0x3F9990BF5F22759C"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_ACTIVITY_SESSION"
	hash "0x05095437424397FA"
	jhash (0x577DAA8A)
	returns	"Any"

native "0x4A9FDE3A5A6D0437"
	hash "0x4A9FDE3A5A6D0437"
	jhash (0x18F03AFD)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xC3C7A6AFDB244624"
	hash "0xC3C7A6AFDB244624"
	jhash (0x8B99B72B)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0xC116FF9B4D488291"
	hash "0xC116FF9B4D488291"
	jhash (0x877C0E1C)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0x1171A97A3D3981B6"
	hash "0x1171A97A3D3981B6"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0x742B58F723233ED9"
	hash "0x742B58F723233ED9"
	jhash (0x5E832444)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xCEFA968912D0F78D"
	hash "0xCEFA968912D0F78D"
	jhash (0x3FDA00F3)
	returns	"Any"

native "NETWORK_ACCEPT_PRESENCE_INVITE"
	hash "0xFA91550DF9318B22"
	jhash (0xE5DA4CED)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xF0210268DB0974B1"
	hash "0xF0210268DB0974B1"
	jhash (0x93C665FA)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xDFF09646E12EC386"
	hash "0xDFF09646E12EC386"
	jhash (0xD50DF46C)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x4962CC4AA2F345B7"
	hash "0x4962CC4AA2F345B7"
	jhash (0x19EC65D9)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "NETWORK_GET_PRESENCE_INVITE_HANDLE"
	hash "0x38D5B0FEBB086F75"
	jhash (0xB2451429)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x26E1CD96B0903D60"
	hash "0x26E1CD96B0903D60"
	jhash (0xC5E0C989)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x24409FC4C55CB22D"
	hash "0x24409FC4C55CB22D"
	jhash (0xA4302183)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xD39B3FFF8FFDD5BF"
	hash "0xD39B3FFF8FFDD5BF"
	jhash (0x51B2D848)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x728C4CC7920CD102"
	hash "0x728C4CC7920CD102"
	jhash (0x4677C656)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x3DBF2DF0AEB7D289"
	hash "0x3DBF2DF0AEB7D289"
	jhash (0xF5E3401C)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x8806CEBFABD3CE05"
	hash "0x8806CEBFABD3CE05"
	jhash (0x7D593B4C)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x76D9B976C4C09FDE"
	hash "0x76D9B976C4C09FDE"
	jhash (0xE96CFE7D)
	returns	"Any"

native "0xC88156EBB786F8D5"
	hash "0xC88156EBB786F8D5"
	jhash (0xAB969F00)
	returns	"Any"

native "0x439BFDE3CD0610F6"
	hash "0x439BFDE3CD0610F6"
	jhash (0x3242F952)
	returns	"Any"

native "0xEBF8284D8CADEB53"
	hash "0xEBF8284D8CADEB53"
	jhash (0x9773F36A)
	returns	"void"

native "NETWORK_REMOVE_TRANSITION_INVITE"
	hash "0x7524B431B2E6F7EE"
	jhash (0xFDE84CB7)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x726E0375C7A26368"
	hash "0x726E0375C7A26368"
	jhash (0xF7134E73)
	returns	"void"

native "0xF083835B70BA9BFE"
	hash "0xF083835B70BA9BFE"
	jhash (0xC47352E7)
	returns	"void"

native "NETWORK_INVITE_GAMERS"
	hash "0x9D80CD1D0E6327DE"
	jhash (0x52FB8074)
	arguments {
		AnyPtr "p0",

		Any "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"BOOL"

native "NETWORK_HAS_INVITED_GAMER"
	hash "0x4D86CD31E8976ECE"
	jhash (0xEC651BC0)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_GET_CURRENTLY_SELECTED_GAMER_HANDLE_FROM_INVITE_MENU"
	hash "0x74881E6BCAE2327C"
	jhash (0x72BA00CE)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_SET_CURRENTLY_SELECTED_GAMER_HANDLE_FROM_INVITE_MENU"
	hash "0x7206F674F2A3B1BB"
	jhash (0xFD95899E)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x66F010A4B031A331"
	hash "0x66F010A4B031A331"
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x44B37CDCAE765AAE"
	hash "0x44B37CDCAE765AAE"
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x0D77A82DC2D0DA59"
	hash "0x0D77A82DC2D0DA59"
	jhash (0x0808D4CC)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "FILLOUT_PM_PLAYER_LIST"
	hash "0xCBBD7C4991B64809"
	jhash (0xCE40F423)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "FILLOUT_PM_PLAYER_LIST_WITH_NAMES"
	hash "0x716B6DB9D1886106"
	jhash (0xB8DF604E)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "USING_NETWORK_WEAPONTYPE"
	hash "0xE26CCFF8094D8C74"
	jhash (0xF49C1533)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x796A87B3B68D1F3D"
	hash "0x796A87B3B68D1F3D"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x2FC5650B0271CB57"
	hash "0x2FC5650B0271CB57"
	jhash (0xA812B6CB)
	returns	"Any"

native "0x01ABCE5E7CBDA196"
	hash "0x01ABCE5E7CBDA196"
	returns	"Any"

native "0x120364DE2845DAF8"
	hash "0x120364DE2845DAF8"
	jhash (0xF30E5814)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"Any"

native "0xFD8B834A8BA05048"
	hash "0xFD8B834A8BA05048"
	jhash (0xC6609191)
	returns	"Any"

native "NETWORK_IS_CHATTING_IN_PLATFORM_PARTY"
	hash "0x8DE9945BCC9AEC52"
	jhash (0x51367B34)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_IN_PARTY"
	hash "0x966C2BC2A7FE3F30"
	jhash (0xF9D7D67D)
	returns	"Any"

native "NETWORK_IS_PARTY_MEMBER"
	hash "0x676ED266AADD31E0"
	jhash (0x1D0C929D)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x2BF66D2E7414F686"
	hash "0x2BF66D2E7414F686"
	jhash (0x9156EFC0)
	returns	"Any"

native "0x14922ED3E38761F0"
	hash "0x14922ED3E38761F0"
	jhash (0x8FA6EE0E)
	returns	"Any"

native "0xFA2888E3833C8E96"
	hash "0xFA2888E3833C8E96"
	jhash (0x7F70C15A)
	returns	"void"

native "0x25D990F8E0E3F13C"
	hash "0x25D990F8E0E3F13C"
	returns	"void"

native "0x77FADDCBE3499DF7"
	hash "0x77FADDCBE3499DF7"
	jhash (0x8179C48A)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xF1B84178F8674195"
	hash "0xF1B84178F8674195"
	jhash (0x41702C8A)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x599E4FA1F87EB5FF"
	hash "0x599E4FA1F87EB5FF"
	jhash (0x208DD848)
	returns	"Any"

native "0xE30CF56F1EFA5F43"
	hash "0xE30CF56F1EFA5F43"
	jhash (0xF9B6426D)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

--[[!
<summary>
	This one is funny :D
</summary>
]]--
native "NETWORK_PLAYER_IS_CHEATER"
	hash "0x655B91F1495A9090"
	jhash (0xA51DC214)
	returns	"Any"

native "0x172F75B6EE2233BA"
	hash "0x172F75B6EE2233BA"
	jhash (0x1720ABA6)
	returns	"Any"

native "NETWORK_PLAYER_IS_BADSPORT"
	hash "0x19D8DA0E5A68045A"
	jhash (0xA19708E3)
	returns	"Any"

native "0x46FB3ED415C7641C"
	hash "0x46FB3ED415C7641C"
	jhash (0xF9A51B92)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0xEC5E3AF5289DCA81"
	hash "0xEC5E3AF5289DCA81"
	jhash (0x4C2C6B6A)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0xE66C690248F11150"
	hash "0xE66C690248F11150"
	jhash (0x4818ACD0)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "NETWORK_SET_THIS_SCRIPT_IS_NETWORK_SCRIPT"
	hash "0x1CA59E306ECB80A5"
	jhash (0x470810ED)
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",
	}
	returns	"void"

native "0xD1110739EEADB592"
	hash "0xD1110739EEADB592"
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",
	}
	returns	"BOOL"

native "NETWORK_GET_THIS_SCRIPT_IS_NETWORK_SCRIPT"
	hash "0x2910669969E9535E"
	jhash (0xD9BF6549)
	returns	"BOOL"

--[[!
<summary>
	Seems to always return 0, but it's used in quite a few loops.

	for (num3 = 0; num3 &lt; NETWORK::0xCCD8C02D(); num3++)
	    {
	        if (NETWORK::NETWORK_IS_PARTICIPANT_ACTIVE(PLAYER::0x98F3B274(num3)) != 0)
	        {
	            var num5 = NETWORK::NETWORK_GET_PLAYER_INDEX(PLAYER::0x98F3B274(num3));
</summary>
]]--
native "_NETWORK_GET_NUM_PARTICIPANTS_HOST"
	hash "0xA6C90FBC38E395EE"
	jhash (0xCCD8C02D)
	returns	"int"

native "NETWORK_GET_NUM_PARTICIPANTS"
	hash "0x18D0456E86604654"
	jhash (0x3E25A3C5)
	returns	"int"

native "NETWORK_GET_SCRIPT_STATUS"
	hash "0x57D158647A6BFABF"
	jhash (0x2BE9235A)
	returns	"Any"

native "NETWORK_REGISTER_HOST_BROADCAST_VARIABLES"
	hash "0x3E9B2F01C50DF595"
	jhash (0xDAF3B0AE)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"void"

native "NETWORK_REGISTER_PLAYER_BROADCAST_VARIABLES"
	hash "0x3364AA97340CA215"
	jhash (0xBE3D32B4)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"void"

native "0x64F62AFB081E260D"
	hash "0x64F62AFB081E260D"
	jhash (0xA71A1D2A)
	returns	"void"

native "0x5D10B3795F3FC886"
	hash "0x5D10B3795F3FC886"
	jhash (0x0B739F53)
	returns	"Any"

native "NETWORK_GET_PLAYER_INDEX"
	hash "0x24FB80D107371267"
	jhash (0xBE1C1506)
	arguments {
		Ped "PedHandle",
	}
	returns	"Any"

native "NETWORK_GET_PARTICIPANT_INDEX"
	hash "0x1B84DF6AF2A46938"
	jhash (0xC4D91094)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x6C0E2E0125610278"
	hash "0x6C0E2E0125610278"
	jhash (0x40DBF464)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	Returns the amount of players connected in the current session. Only works when connected to a session/server.
</summary>
]]--
native "NETWORK_GET_NUM_CONNECTED_PLAYERS"
	hash "0xA4A79DD2D9600654"
	jhash (0xF7952E62)
	returns	"int"

native "NETWORK_IS_PLAYER_CONNECTED"
	hash "0x93DC1BE4E1ABE9D1"
	jhash (0x168EE2C2)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xCF61D4B4702EE9EB"
	hash "0xCF61D4B4702EE9EB"
	jhash (0xF4F13B06)
	returns	"Any"

native "NETWORK_IS_PARTICIPANT_ACTIVE"
	hash "0x6FF8FF40B6357D45"
	jhash (0x4E2C348B)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_PLAYER_ACTIVE"
	hash "0xB8DFD30D6973E135"
	jhash (0x43657B17)
	arguments {
		int "playerID",
	}
	returns	"BOOL"

native "NETWORK_IS_PLAYER_A_PARTICIPANT"
	hash "0x3CA58F6CB7CBD784"
	jhash (0xB08B6992)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_HOST_OF_THIS_SCRIPT"
	hash "0x83CD99A1E6061AB5"
	jhash (0x6970BA94)
	returns	"BOOL"

native "NETWORK_GET_HOST_OF_THIS_SCRIPT"
	hash "0xC7B4D79B01FA7A5C"
	jhash (0x89EA7B54)
	returns	"Any"

--[[!
<summary>
	Ex. gamemode:

	freemode



</summary>
]]--
native "NETWORK_GET_HOST_OF_SCRIPT"
	hash "0x1D6A14F1F9A736FC"
	jhash (0x9C95D0BB)
	arguments {
		charPtr "gamemode",

		Any "p1",

		Any "p2",
	}
	returns	"int"

native "NETWORK_SET_MISSION_FINISHED"
	hash "0x3B3D11CD9FFCDFC9"
	jhash (0x3083FAD7)
	returns	"void"

native "NETWORK_IS_SCRIPT_ACTIVE"
	hash "0x9D40DF90FAD26098"
	jhash (0x4A65250C)
	arguments {
		AnyPtr "p0",

		Any "p1",

		BOOL "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0x3658E8CD94FC121A"
	hash "0x3658E8CD94FC121A"
	jhash (0x8F7D9F46)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "0x638A3A81733086DB"
	hash "0x638A3A81733086DB"
	jhash (0xDB8B5D71)
	returns	"Any"

native "0x1AD5B71586B94820"
	hash "0x1AD5B71586B94820"
	jhash (0xCEA55F4C)
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0x2302C0264EA58D31"
	hash "0x2302C0264EA58D31"
	jhash (0x8DCFE77D)
	returns	"void"

native "0x741A3D8380319A81"
	hash "0x741A3D8380319A81"
	jhash (0x331D9A27)
	returns	"void"

--[[!
<summary>
	Return the local Participant ID
</summary>
]]--
native "PARTICIPANT_ID"
	hash "0x90986E8876CE0A83"
	jhash (0x9C35A221)
	returns	"int"

--[[!
<summary>
	Return the local Participant ID.

	This native is exactly the same as 'PARTICIPANT_ID' native.
</summary>
]]--
native "PARTICIPANT_ID_TO_INT"
	hash "0x57A3BDDAD8E5AA0A"
	jhash (0x907498B0)
	returns	"int"

native "NETWORK_GET_DESTROYER_OF_NETWORK_ID"
	hash "0x7A1ADEEF01740A24"
	jhash (0x4FCA6436)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"Any"

native "0x4CACA84440FA26F6"
	hash "0x4CACA84440FA26F6"
	jhash (0x28A45454)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "NETWORK_GET_ENTITY_KILLER_OF_PLAYER"
	hash "0x42B2DAA6B596F5F8"
	jhash (0xA7E7E04F)
	arguments {
		Player "player",

		AnyPtr "p1",
	}
	returns	"Entity"

native "NETWORK_RESURRECT_LOCAL_PLAYER"
	hash "0xEA23C49EAA83ACFB"
	jhash (0xF1F9D4B4)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"void"

native "NETWORK_SET_LOCAL_PLAYER_INVINCIBLE_TIME"
	hash "0x2D95C7E2D7E07307"
	jhash (0xFEA9B85C)
	arguments {
		Any "p0",
	}
	returns	"void"

native "NETWORK_IS_LOCAL_PLAYER_INVINCIBLE"
	hash "0x8A8694B48715B000"
	jhash (0x8DE13B36)
	returns	"Any"

native "0x9DD368BF06983221"
	hash "0x9DD368BF06983221"
	jhash (0x8D27280E)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x524FF0AEFF9C3973"
	hash "0x524FF0AEFF9C3973"
	jhash (0xB72F086D)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xB07D3185E11657A5"
	hash "0xB07D3185E11657A5"
	jhash (0xEDA68956)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_GET_NETWORK_ID_FROM_ENTITY"
	hash "0xA11700682F3AD45C"
	jhash (0x9E35DAB6)
	arguments {
		Entity "entity",
	}
	returns	"int"

native "NETWORK_GET_ENTITY_FROM_NETWORK_ID"
	hash "0xCE4E5D9B0A4FF560"
	jhash (0x5B912C3F)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xC7827959479DCC78"
	hash "0xC7827959479DCC78"
	jhash (0xD7F934F4)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_GET_ENTITY_IS_LOCAL"
	hash "0x0991549DE4D64762"
	jhash (0x813353ED)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x06FAACD625D80CAA"
	hash "0x06FAACD625D80CAA"
	jhash (0x31A630A4)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x7368E683BB9038D6"
	hash "0x7368E683BB9038D6"
	jhash (0x5C645F64)
	arguments {
		Any "p0",
	}
	returns	"void"

native "NETWORK_DOES_NETWORK_ID_EXIST"
	hash "0x38CE16C96BD11344"
	jhash (0xB8D2C99E)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_DOES_ENTITY_EXIST_WITH_NETWORK_ID"
	hash "0x18A47D074708FD68"
	jhash (0x1E2E3177)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_REQUEST_CONTROL_OF_NETWORK_ID"
	hash "0xA670B3662FAFFBD0"
	jhash (0x9262A60A)
	arguments {
		int "netID",
	}
	returns	"BOOL"

native "NETWORK_HAS_CONTROL_OF_NETWORK_ID"
	hash "0x4D36070FE0215186"
	jhash (0x92E77D21)
	arguments {
		int "netID",
	}
	returns	"BOOL"

native "NETWORK_REQUEST_CONTROL_OF_ENTITY"
	hash "0xB69317BF5E782347"
	jhash (0xA05FEBD7)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "NETWORK_REQUEST_CONTROL_OF_DOOR"
	hash "0x870DDFD5A4A796E4"
	jhash (0xF60DAAF6)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_HAS_CONTROL_OF_ENTITY"
	hash "0x01BF60A500E28887"
	jhash (0x005FD797)
	arguments {
		Entity "entity",
	}
	returns	"BOOL"

native "NETWORK_HAS_CONTROL_OF_PICKUP"
	hash "0x5BC9495F0B3B6FA6"
	jhash (0xF7784FC8)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_HAS_CONTROL_OF_DOOR"
	hash "0xCB3C68ADB06195DF"
	jhash (0x136326EC)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xC01E93FAC20C3346"
	hash "0xC01E93FAC20C3346"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "VEH_TO_NET"
	hash "0xB4C94523F023419C"
	jhash (0xF17634EB)
	arguments {
		Vehicle "vehicle",
	}
	returns	"int"

native "PED_TO_NET"
	hash "0x0EDEC3C276198689"
	jhash (0x86A0B759)
	arguments {
		Ped "ped",
	}
	returns	"int"

native "OBJ_TO_NET"
	hash "0x99BFDC94A603E541"
	jhash (0x1E05F29F)
	arguments {
		Object "object",
	}
	returns	"int"

native "NET_TO_VEH"
	hash "0x367B936610BA360C"
	jhash (0x7E02FAEA)
	arguments {
		int "netHandle",
	}
	returns	"Vehicle"

native "NET_TO_PED"
	hash "0xBDCD95FC216A8B3E"
	jhash (0x87717DD4)
	arguments {
		int "netHandle",
	}
	returns	"Ped"

native "NET_TO_OBJ"
	hash "0xD8515F5FEA14CB3F"
	jhash (0x27AA14D8)
	arguments {
		int "netHandle",
	}
	returns	"Object"

native "NET_TO_ENT"
	hash "0xBFFEAB45A9A9094A"
	jhash (0x5E149683)
	arguments {
		int "netHandle",
	}
	returns	"Entity"

native "NETWORK_GET_LOCAL_HANDLE"
	hash "0xE86051786B66CD8E"
	jhash (0x08023B16)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"void"

native "NETWORK_HANDLE_FROM_USER_ID"
	hash "0xDCD51DD8F87AEC5C"
	jhash (0x74C2C1B7)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",
	}
	returns	"void"

native "NETWORK_HANDLE_FROM_MEMBER_ID"
	hash "0xA0FD21BED61E5C4C"
	jhash (0x9BFC9FE2)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",
	}
	returns	"void"

--[[!
<summary>
	"p2" mainly set as 13 in scripts 

	From scripts use:
	struct&lt;13&gt; func_27(var uParam0)
	{
	     struct&lt;13&gt; Var0;
	     network_handle_from_player(uParam0, &amp;Var0, 13);
	     return Var0;
	}
</summary>
]]--
native "NETWORK_HANDLE_FROM_PLAYER"
	hash "0x388EB2B86C73B6B3"
	jhash (0xD3498917)
	arguments {
		Player "player",

		intPtr "handle",

		int "p2",
	}
	returns	"void"

--[[!
<summary>
	last gen _0xF8D7AF3B
</summary>
]]--
native "0xBC1D768F2F5D6C05"
	hash "0xBC1D768F2F5D6C05"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x58575AC3CF2CA8EC"
	hash "0x58575AC3CF2CA8EC"
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "NETWORK_HANDLE_FROM_FRIEND"
	hash "0xD45CB817D7E177D2"
	jhash (0x3B0BB3A3)
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",
	}
	returns	"void"

native "NETWORK_GAMERTAG_FROM_HANDLE_START"
	hash "0x9F0C0A981D73FA56"
	jhash (0xEBA00C2A)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_GAMERTAG_FROM_HANDLE_PENDING"
	hash "0xB071E27958EF4CF0"
	jhash (0xF000828E)
	returns	"Any"

native "NETWORK_GAMERTAG_FROM_HANDLE_SUCCEEDED"
	hash "0xFD00798DBA7523DD"
	jhash (0x89C2B5EA)
	returns	"Any"

native "NETWORK_GET_GAMERTAG_FROM_HANDLE"
	hash "0x426141162EBE5CDB"
	jhash (0xA18A1B26)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "0xD66C9E72B3CC4982"
	hash "0xD66C9E72B3CC4982"
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"int"

native "0x58CC181719256197"
	hash "0x58CC181719256197"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "NETWORK_ARE_HANDLES_THE_SAME"
	hash "0x57DBA049E110F217"
	jhash (0x45975AE3)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	param 2 is 13 much like NETWORK_HANDLE_FROM_PLAYER
</summary>
]]--
native "NETWORK_IS_HANDLE_VALID"
	hash "0x6F79B93B0A8E4133"
	jhash (0xF0996C6E)
	arguments {
		intPtr "p0",

		int "p1",
	}
	returns	"BOOL"

native "NETWORK_GET_PLAYER_FROM_GAMER_HANDLE"
	hash "0xCE5F689CF5A0A49D"
	jhash (0x2E96EF1E)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "NETWORK_MEMBER_ID_FROM_GAMER_HANDLE"
	hash "0xC82630132081BB6F"
	jhash (0x62EF0A63)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "NETWORK_IS_GAMER_IN_MY_SESSION"
	hash "0x0F10B05DDF8D16E9"
	jhash (0x59127716)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_SHOW_PROFILE_UI"
	hash "0x859ED1CEA343FCA8"
	jhash (0xF00A20B0)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

--[[!
<summary>
	Returns the name of a given player.
</summary>
]]--
native "NETWORK_PLAYER_GET_NAME"
	hash "0x7718D2E2060837D2"
	jhash (0xCE48F260)
	arguments {
		Player "player",
	}
	returns	"charPtr"

native "_NETWORK_PLAYER_GET_USER_ID"
	hash "0x4927FC39CD0869A0"
	jhash (0x4EC0D983)
	arguments {
		Player "player",

		AnyPtr "userID",
	}
	returns	"charPtr"

--[[!
<summary>
	No longer used for DEV checks since first mods released on PS3 &amp; 360.
</summary>
]]--
native "NETWORK_PLAYER_IS_ROCKSTAR_DEV"
	hash "0x544ABDDA3B409B6D"
	jhash (0xF6659045)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "0x565E430DB3B05BEC"
	hash "0x565E430DB3B05BEC"
	jhash (0xD265B049)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_INACTIVE_PROFILE"
	hash "0x7E58745504313A2E"
	jhash (0x95481343)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_GET_MAX_FRIENDS"
	hash "0xAFEBB0D5D8F687D2"
	jhash (0x048171BC)
	returns	"Any"

native "NETWORK_GET_FRIEND_COUNT"
	hash "0x203F1CFD823B27A4"
	jhash (0xA396ACDE)
	returns	"Any"

native "NETWORK_GET_FRIEND_NAME"
	hash "0xE11EBBB2A783FE8B"
	jhash (0x97420B6D)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "_NETWORK_GET_FRIEND_NAME"
	hash "0x4164F227D052E293"
	arguments {
		int "friendIndex",
	}
	returns	"charPtr"

native "NETWORK_IS_FRIEND_ONLINE"
	hash "0x425A44533437B64D"
	jhash (0xE0A42430)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x87EB7A3FFCB314DB"
	hash "0x87EB7A3FFCB314DB"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_FRIEND_IN_SAME_TITLE"
	hash "0x2EA9A3BEDF3F17B8"
	jhash (0xC54365C2)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_FRIEND_IN_MULTIPLAYER"
	hash "0x57005C18827F3A28"
	jhash (0x400BDDD9)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_FRIEND"
	hash "0x1A24A179F9B31654"
	jhash (0x2DA4C282)
	arguments {
		intPtr "player",
	}
	returns	"BOOL"

native "NETWORK_IS_PENDING_FRIEND"
	hash "0x0BE73DA6984A6E33"
	jhash (0x5C85FF81)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "NETWORK_IS_ADDING_FRIEND"
	hash "0x6EA101606F6E4D81"
	jhash (0xBB7EC8C4)
	returns	"Any"

native "NETWORK_ADD_FRIEND"
	hash "0x8E02D73914064223"
	jhash (0x20E5B3EE)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	"CLEAR_PLAYER_BAILED_FROM_PLAYERICLE"...seriously?
</summary>
]]--
native "0xBAD8F2A42B844821"
	hash "0xBAD8F2A42B844821"
	jhash (0x94AE7172)
	arguments {
		int "friendIndex",
	}
	returns	"BOOL"

native "0x1B857666604B1A74"
	hash "0x1B857666604B1A74"
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	"GET_OBJ_ENTITY" seems highly unlikely.
</summary>
]]--
native "0x82377B65E943F72D"
	hash "0x82377B65E943F72D"
	jhash (0xB802B671)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_CAN_SET_WAYPOINT"
	hash "0xC927EC229934AF60"
	jhash (0x009E68F3)
	returns	"BOOL"

native "0xB309EBEA797E001F"
	hash "0xB309EBEA797E001F"
	jhash (0x5C0AB2A9)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x26F07DD83A5F7F98"
	hash "0x26F07DD83A5F7F98"
	jhash (0x9A176B6E)
	returns	"Any"

native "NETWORK_HAS_HEADSET"
	hash "0xE870F9F1F7B4F1FA"
	jhash (0xA7DC5657)
	returns	"BOOL"

native "0x7D395EA61622E116"
	hash "0x7D395EA61622E116"
	jhash (0x5C05B7E1)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xC0D2AF00BCC234CA"
	hash "0xC0D2AF00BCC234CA"
	returns	"Any"

native "NETWORK_GAMER_HAS_HEADSET"
	hash "0xF2FD55CB574BCC55"
	jhash (0xD036DA4A)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_GAMER_TALKING"
	hash "0x71C33B22606CD88A"
	jhash (0x99B58DBC)
	arguments {
		intPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_CAN_COMMUNICATE_WITH_GAMER"
	hash "0xA150A4F065806B1F"
	jhash (0xD05EB7F6)
	arguments {
		intPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_GAMER_MUTED_BY_ME"
	hash "0xCE60DE011B6C7978"
	jhash (0x001B4046)
	arguments {
		intPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_AM_I_MUTED_BY_GAMER"
	hash "0xDF02A2C93F1F26DA"
	jhash (0x7685B333)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_GAMER_BLOCKED_BY_ME"
	hash "0xE944C4F5AF1B5883"
	jhash (0x3FDCC8D7)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_AM_I_BLOCKED_BY_GAMER"
	hash "0x15337C7C268A27B2"
	jhash (0xD19B312C)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0xB57A49545BA53CE7"
	hash "0xB57A49545BA53CE7"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0xCCA4318E1AB03F1F"
	hash "0xCCA4318E1AB03F1F"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x07DD29D5E22763F1"
	hash "0x07DD29D5E22763F1"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x135F9B7B7ADD2185"
	hash "0x135F9B7B7ADD2185"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_IS_PLAYER_TALKING"
	hash "0x031E11F3D447647E"
	jhash (0xDA9FD9DB)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "NETWORK_PLAYER_HAS_HEADSET"
	hash "0x3FB99A8B08D18FD6"
	jhash (0x451FB6B6)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "NETWORK_IS_PLAYER_MUTED_BY_ME"
	hash "0x8C71288AE68EDE39"
	jhash (0x7A21050E)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "NETWORK_AM_I_MUTED_BY_PLAYER"
	hash "0x9D6981DFC91A8604"
	jhash (0xE128F2B0)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "NETWORK_IS_PLAYER_BLOCKED_BY_ME"
	hash "0x57AF1F8E27483721"
	jhash (0xAE4F4560)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "NETWORK_AM_I_BLOCKED_BY_PLAYER"
	hash "0x87F395D957D4353D"
	jhash (0x953EF45E)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "NETWORK_GET_PLAYER_LOUDNESS"
	hash "0x21A1684A25C2867F"
	jhash (0xF2F67014)
	arguments {
		Any "p0",
	}
	returns	"float"

native "NETWORK_SET_TALKER_PROXIMITY"
	hash "0xCBF12D65F95AD686"
	jhash (0x67555C66)
	arguments {
		float "p0",
	}
	returns	"void"

native "NETWORK_GET_TALKER_PROXIMITY"
	hash "0x84F0F13120B4E098"
	jhash (0x19991ADD)
	returns	"Any"

native "NETWORK_SET_VOICE_ACTIVE"
	hash "0xBABEC9E69A91C57B"
	jhash (0x8011247F)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0xCFEB46DCD7D8D5EB"
	hash "0xCFEB46DCD7D8D5EB"
	jhash (0x1A3EA6CD)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xAF66059A131AA269"
	hash "0xAF66059A131AA269"
	jhash (0xCAB21090)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "NETWORK_SET_TEAM_ONLY_CHAT"
	hash "0xD5B4883AC32F24C3"
	jhash (0x3813019A)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0x6F697A66CE78674E"
	hash "0x6F697A66CE78674E"
	jhash (0xC8CC9E75)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "NETWORK_SET_OVERRIDE_SPECTATOR_MODE"
	hash "0x70DA3BF8DACD3210"
	jhash (0xA0FD42D3)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0x3C5C1E2C2FF814B1"
	hash "0x3C5C1E2C2FF814B1"
	jhash (0xC9DDA85B)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x9D7AFCBF21C51712"
	hash "0x9D7AFCBF21C51712"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xF46A1E03E8755980"
	hash "0xF46A1E03E8755980"
	jhash (0xD33AFF79)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x6A5D89D7769A40D8"
	hash "0x6A5D89D7769A40D8"
	jhash (0x4FFEFE43)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x3039AE5AD2C9C0C4"
	hash "0x3039AE5AD2C9C0C4"
	jhash (0x74EE2D8B)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x97DD4C5944CC2E6A"
	hash "0x97DD4C5944CC2E6A"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x57B192B4D4AD23D5"
	hash "0x57B192B4D4AD23D5"
	jhash (0x2F98B405)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xDDF73E2B1FEC5AB4"
	hash "0xDDF73E2B1FEC5AB4"
	jhash (0x95F1C60D)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x0FF2862B61A58AF9"
	hash "0x0FF2862B61A58AF9"
	jhash (0x1BCD3DDF)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "NETWORK_SET_VOICE_CHANNEL"
	hash "0xEF6212C2EFEF1A23"
	jhash (0x3974879F)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xE036A705F989E049"
	hash "0xE036A705F989E049"
	jhash (0x9ECF722A)
	returns	"void"

native "IS_NETWORK_VEHICLE_BEEN_DAMAGED_BY_ANY_OBJECT"
	hash "0xDBD2056652689917"
	jhash (0xF1E84832)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0xF03755696450470C"
	hash "0xF03755696450470C"
	jhash (0x7F9B9052)
	returns	"void"

native "0x5E3AA4CA2B6FB0EE"
	hash "0x5E3AA4CA2B6FB0EE"
	jhash (0x7BBEA8CF)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xCA575C391FEA25CC"
	hash "0xCA575C391FEA25CC"
	jhash (0xE797A4B6)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xADB57E5B663CCA8B"
	hash "0xADB57E5B663CCA8B"
	jhash (0x92268BB5)
	arguments {
		Player "p0",

		floatPtr "p1",

		floatPtr "p2",
	}
	returns	"void"

--[[!
<summary>
	Same as _IS_TEXT_CHAT_ACTIVE, except it does not check if the text chat HUD component is initialized, and therefore may crash.
</summary>
]]--
native "_NETWORK_IS_TEXT_CHAT_ACTIVE"
	hash "0x5FCF4D7069B09026"
	returns	"BOOL"

--[[!
<summary>
	Starts a new singleplayer game (at the prologue).

	~ ClareXoBearrx3
</summary>
]]--
native "SHUTDOWN_AND_LAUNCH_SINGLE_PLAYER_GAME"
	hash "0x593850C16A36B692"
	jhash (0x92B7351C)
	returns	"void"

native "NETWORK_SET_FRIENDLY_FIRE_OPTION"
	hash "0xF808475FA571D823"
	jhash (0x6BAF95FA)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "NETWORK_SET_RICH_PRESENCE"
	hash "0x1DCCACDCFC569362"
	jhash (0x932A6CED)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"void"

native "0x3E200C2BCF4164EB"
	hash "0x3E200C2BCF4164EB"
	jhash (0x017E6777)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x5ED0356A0CE3A34F"
	hash "0x5ED0356A0CE3A34F"
	jhash (0xE1F86C6A)
	returns	"Any"

native "0x9769F811D1785B03"
	hash "0x9769F811D1785B03"
	jhash (0xBE6A30C3)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		BOOL "p4",

		BOOL "p5",
	}
	returns	"void"

native "0xBF22E0F32968E967"
	hash "0xBF22E0F32968E967"
	jhash (0x22E03AD0)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

--[[!
<summary>
	Used in am_mp_property_ext and am_mp_property_int
</summary>
]]--
native "0x715135F4B82AC90D"
	hash "0x715135F4B82AC90D"
	jhash (0xCEAE5AFC)
	arguments {
		Entity "entity",
	}
	returns	"void"

native "_NETWORK_PLAYER_IS_IN_CLAN"
	hash "0x579CCED0265D4896"
	jhash (0xF5F4BD95)
	returns	"BOOL"

native "NETWORK_CLAN_PLAYER_IS_ACTIVE"
	hash "0xB124B57F571D8F18"
	jhash (0xAB8319A3)
	arguments {
		ScrHandlePtr "netHandle",
	}
	returns	"BOOL"

--[[!
<summary>
	p1 is 35 in the scripts.
</summary>
]]--
native "NETWORK_CLAN_PLAYER_GET_DESC"
	hash "0xEEE6EACBE8874FBA"
	jhash (0x6EE4A282)
	arguments {
		ScrHandlePtr "description",

		int "p1",

		ScrHandlePtr "netHandle",
	}
	returns	"BOOL"

native "0x7543BB439F63792B"
	hash "0x7543BB439F63792B"
	jhash (0x54E79E9C)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0xF45352426FF3A4F0"
	hash "0xF45352426FF3A4F0"
	jhash (0xF633805A)
	arguments {
		AnyPtr "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0x1F471B79ACC90BEF"
	hash "0x1F471B79ACC90BEF"
	jhash (0x807B3450)
	returns	"Any"

native "NETWORK_CLAN_GET_MEMBERSHIP_DESC"
	hash "0x48DE78AF2C8885B8"
	jhash (0x3369DD1F)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"BOOL"

native "NETWORK_CLAN_DOWNLOAD_MEMBERSHIP"
	hash "0xA989044E70010ABE"
	jhash (0x8E8CB520)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_CLAN_DOWNLOAD_MEMBERSHIP_PENDING"
	hash "0x5B9E023DC6EBEDC0"
	jhash (0x1FDB590F)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0xB3F64A6A91432477"
	hash "0xB3F64A6A91432477"
	jhash (0x83ED8E08)
	returns	"Any"

native "NETWORK_CLAN_REMOTE_MEMBERSHIPS_ARE_IN_CACHE"
	hash "0xBB6E6FEE99D866B2"
	jhash (0x40202867)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "NETWORK_CLAN_GET_MEMBERSHIP_COUNT"
	hash "0xAAB11F6C4ADBC2C1"
	jhash (0x25924010)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "NETWORK_CLAN_GET_MEMBERSHIP_VALID"
	hash "0x48A59CF88D43DF0E"
	jhash (0x48914F6A)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"BOOL"

native "NETWORK_CLAN_GET_MEMBERSHIP"
	hash "0xC8BC2011F67B3411"
	jhash (0xCDC4A590)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",
	}
	returns	"BOOL"

native "NETWORK_CLAN_JOIN"
	hash "0x9FAAA4F4FC71F87F"
	jhash (0x79C916C5)
	arguments {
		Any "clanHandle",
	}
	returns	"Any"

native "0x729E3401F0430686"
	hash "0x729E3401F0430686"
	jhash (0xBDA90BAC)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x2B51EDBEFC301339"
	hash "0x2B51EDBEFC301339"
	jhash (0x8E952B12)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0xC32EA7A2F6CA7557"
	hash "0xC32EA7A2F6CA7557"
	jhash (0x966C90FD)
	returns	"Any"

native "0x5835D9CD92E83184"
	hash "0x5835D9CD92E83184"
	jhash (0xBA672146)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x13518FF1C6B28938"
	hash "0x13518FF1C6B28938"
	jhash (0x7963FA4D)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xA134777FF7F33331"
	hash "0xA134777FF7F33331"
	jhash (0x88B13CDC)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x113E6E3E50E286B0"
	hash "0x113E6E3E50E286B0"
	jhash (0xD6E3D5EA)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x9AA46BADAD0E27ED"
	hash "0x9AA46BADAD0E27ED"
	jhash (0xE22445DA)
	returns	"Any"

native "0x042E4B70B93E6054"
	hash "0x042E4B70B93E6054"
	jhash (0x455DDF5C)
	returns	"void"

native "NETWORK_GET_PRIMARY_CLAN_DATA_START"
	hash "0xCE86D8191B762107"
	jhash (0x89DB0EC7)
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0xB5074DB804E28CE7"
	hash "0xB5074DB804E28CE7"
	jhash (0xA4EF02F3)
	returns	"Any"

native "0x5B4F04F19376A0BA"
	hash "0x5B4F04F19376A0BA"
	jhash (0x068A054E)
	returns	"Any"

native "NETWORK_GET_PRIMARY_CLAN_DATA_NEW"
	hash "0xC080FF658B2E41DA"
	jhash (0x9B8631EB)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "SET_NETWORK_ID_CAN_MIGRATE"
	hash "0x299EEB23175895FC"
	jhash (0x47C8E5FF)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES"
	hash "0xE05E81A888FA63C8"
	jhash (0x68D486B2)
	arguments {
		int "NetID",

		BOOL "DoesExist",
	}
	returns	"Any"

native "0xA8A024587329F36A"
	hash "0xA8A024587329F36A"
	jhash (0x4D15FDB1)
	arguments {
		int "netID",

		Player "player",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SET_ENTITY_CAN_BLEND"
	hash "0xD830567D88A1E873"
	jhash (0xDE8C0DB8)
	arguments {
		Any "p0",

		BOOL "toggle",
	}
	returns	"void"

native "0xF1CA12B18AEF5298"
	hash "0xF1CA12B18AEF5298"
	jhash (0x09CBC4B0)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_NETWORK_ID_VISIBLE_IN_CUTSCENE"
	hash "0xA6928482543022B4"
	jhash (0x199E75EF)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xAAA553E7DD28A457"
	hash "0xAAA553E7DD28A457"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x3FA36981311FA4FF"
	hash "0x3FA36981311FA4FF"
	jhash (0x00AE4E17)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0xA1607996431332DF"
	hash "0xA1607996431332DF"
	jhash (0xEA5176C0)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SET_LOCAL_PLAYER_VISIBLE_IN_CUTSCENE"
	hash "0xD1065D68947E7B6E"
	jhash (0x59F3479B)
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_LOCAL_PLAYER_INVISIBLE_LOCALLY"
	hash "0xE5F773C1A1D9D168"
	jhash (0x764F6222)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_LOCAL_PLAYER_VISIBLE_LOCALLY"
	hash "0x7619364C82D3BF14"
	jhash (0x324B56DB)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "SET_PLAYER_INVISIBLE_LOCALLY"
	hash "0x12B37D54667DB0B8"
	jhash (0x18227209)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "SET_PLAYER_VISIBLE_LOCALLY"
	hash "0xFAA10F1FAFB11AF2"
	jhash (0xBA2BB4B4)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "FADE_OUT_LOCAL_PLAYER"
	hash "0x416DBD4CD6ED8DD2"
	jhash (0x8FA7CEBD)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "NETWORK_FADE_OUT_ENTITY"
	hash "0xDE564951F95E09ED"
	jhash (0x47EDEE56)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_FADE_IN_ENTITY"
	hash "0x1F4ED342ACEFE62D"
	jhash (0x9B9FCD02)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x631DC5DFF4B110E3"
	hash "0x631DC5DFF4B110E3"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x422F32CC7E56ABAD"
	hash "0x422F32CC7E56ABAD"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "IS_PLAYER_IN_CUTSCENE"
	hash "0xE73092F4157CD126"
	jhash (0xE0A619BD)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "SET_ENTITY_VISIBLE_IN_CUTSCENE"
	hash "0xE0031D3C8F36AB82"
	jhash (0xDBFB067B)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "SET_ENTITY_LOCALLY_INVISIBLE"
	hash "0xE135A9FF3F5D05D8"
	jhash (0x51ADCC5F)
	arguments {
		Entity "entity",
	}
	returns	"void"

native "SET_ENTITY_LOCALLY_VISIBLE"
	hash "0x241E289B5C059EDC"
	jhash (0x235A57B3)
	arguments {
		Entity "entity",
	}
	returns	"void"

native "IS_DAMAGE_TRACKER_ACTIVE_ON_NETWORK_ID"
	hash "0x6E192E33AD436366"
	jhash (0x597063BA)
	arguments {
		int "netID",
	}
	returns	"BOOL"

native "ACTIVATE_DAMAGE_TRACKER_ON_NETWORK_ID"
	hash "0xD45B1FFCCD52FF19"
	jhash (0x95D07BA5)
	arguments {
		int "netID",

		BOOL "p1",
	}
	returns	"void"

native "IS_SPHERE_VISIBLE_TO_ANOTHER_MACHINE"
	hash "0xD82CF8E64C8729D8"
	jhash (0x23C5274E)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",
	}
	returns	"BOOL"

native "IS_SPHERE_VISIBLE_TO_PLAYER"
	hash "0xDC3A310219E5DA62"
	jhash (0xE9FCFB32)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"BOOL"

native "RESERVE_NETWORK_MISSION_OBJECTS"
	hash "0x4E5C93BD0C32FBF8"
	jhash (0x391DF4F3)
	arguments {
		Any "p0",
	}
	returns	"void"

native "RESERVE_NETWORK_MISSION_PEDS"
	hash "0xB60FEBA45333D36F"
	jhash (0x54998C37)
	arguments {
		Any "p0",
	}
	returns	"void"

native "RESERVE_NETWORK_MISSION_VEHICLES"
	hash "0x76B02E21ED27A469"
	jhash (0x5062875E)
	arguments {
		Any "p0",
	}
	returns	"void"

native "CAN_REGISTER_MISSION_OBJECTS"
	hash "0x800DD4721A8B008B"
	jhash (0x7F85DFDE)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "CAN_REGISTER_MISSION_PEDS"
	hash "0xBCBF4FEF9FA5D781"
	jhash (0xCCAA5CE9)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "CAN_REGISTER_MISSION_VEHICLES"
	hash "0x7277F1F2E085EE74"
	jhash (0x818B6830)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "CAN_REGISTER_MISSION_ENTITIES"
	hash "0x69778E7564BADE6D"
	jhash (0x83794008)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "GET_NUM_RESERVED_MISSION_OBJECTS"
	hash "0xAA81B5F10BC43AC2"
	jhash (0x16A80CD6)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "GET_NUM_RESERVED_MISSION_PEDS"
	hash "0x1F13D5AE5CB17E17"
	jhash (0x6C25975C)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "GET_NUM_RESERVED_MISSION_VEHICLES"
	hash "0xCF3A965906452031"
	jhash (0xA9A308F3)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "0x12B6281B6C6706C0"
	hash "0x12B6281B6C6706C0"
	jhash (0x603FA104)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "0xCB215C4B56A7FAE7"
	hash "0xCB215C4B56A7FAE7"
	jhash (0xD8FEC4F8)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "0x0CD9AB83489430EA"
	hash "0x0CD9AB83489430EA"
	jhash (0x20527695)
	arguments {
		BOOL "p0",
	}
	returns	"Any"

native "0xC7BE335216B5EC7C"
	hash "0xC7BE335216B5EC7C"
	jhash (0x8687E285)
	returns	"Any"

native "0x0C1F7D49C39D2289"
	hash "0x0C1F7D49C39D2289"
	jhash (0x744AC008)
	returns	"Any"

native "0x0AFCE529F69B21FF"
	hash "0x0AFCE529F69B21FF"
	jhash (0xC3A12135)
	returns	"Any"

native "0xA72835064DD63E4C"
	hash "0xA72835064DD63E4C"
	jhash (0x6A036061)
	returns	"Any"

native "GET_NETWORK_TIME"
	hash "0x7A5487FE9FAA6B48"
	jhash (0x998103C2)
	returns	"int"

--[[!
<summary>
	Something to do with time, used in context:
	NETWORK::GET_TIME_DIFFERENCE(NETWORK::_89023FBBF9200E9F(), ... )
</summary>
]]--
native "0x89023FBBF9200E9F"
	hash "0x89023FBBF9200E9F"
	jhash (0x98AA48E5)
	returns	"Any"

native "0x46718ACEEDEAFC84"
	hash "0x46718ACEEDEAFC84"
	jhash (0x4538C4A2)
	returns	"Any"

--[[!
<summary>
	Adds the first argument to the second.
</summary>
]]--
native "GET_TIME_OFFSET"
	hash "0x017008CCDAD48503"
	jhash (0x2E079AE6)
	arguments {
		int "a",

		int "b",
	}
	returns	"int"

--[[!
<summary>
	Subtracts the second argument from the first, then returns whether the result is negative.
</summary>
]]--
native "_SUBTRACT_B_FROM_A_AND_CHECK_IF_NEGATIVE"
	hash "0xCB2CF5148012C8D0"
	jhash (0x50EF8FC6)
	arguments {
		int "a",

		int "b",
	}
	returns	"BOOL"

--[[!
<summary>
	Subtracts the first argument from the second, then returns whether the result is negative.
</summary>
]]--
native "_SUBTRACT_A_FROM_B_AND_CHECK_IF_NEGATIVE"
	hash "0xDE350F8651E4346C"
	jhash (0xBBB6DF61)
	arguments {
		int "a",

		int "b",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns true if the two arguments are equal; otherwise returns false.
</summary>
]]--
native "_ARE_INTEGERS_EQUAL"
	hash "0xF5BC95857BD6D512"
	jhash (0x8B4D1C06)
	arguments {
		int "a",

		int "b",
	}
	returns	"int"

--[[!
<summary>
	Subtracts the second argument from the first.
</summary>
]]--
native "GET_TIME_DIFFERENCE"
	hash "0xA2C6FC031D46FFF0"
	jhash (0x5666A837)
	arguments {
		int "a",

		int "b",
	}
	returns	"int"

native "_FORMAT_TIME"
	hash "0x9E23B1777A927DAD"
	jhash (0x8218944E)
	arguments {
		int "time",
	}
	returns	"charPtr"

native "0x9A73240B49945C76"
	hash "0x9A73240B49945C76"
	jhash (0xF2FDF2E0)
	returns	"int"

--[[!
<summary>
	Takes the specified time and writes it to the structure specified in the second argument.

	struct date_time
	{
	    int year;
	    int PADDING1;
	    int month;
	    int PADDING2;
	    int day;
	    int PADDING3;
	    int hour;
	    int PADDING4;
	    int minute;
	    int PADDING5;
	    int second;
	    int PADDING6;
	};
</summary>
]]--
native "_GET_DATE_AND_TIME_FROM_UNIX_EPOCH"
	hash "0xAC97AF97FA68E5D5"
	jhash (0xBB7CCE49)
	arguments {
		int "unixEpoch",

		AnyPtr "timeStructure",
	}
	returns	"void"

native "NETWORK_SET_IN_SPECTATOR_MODE"
	hash "0x423DE3854BB50894"
	jhash (0x5C4C8458)
	arguments {
		BOOL "p0",

		Any "p1",
	}
	returns	"void"

native "0x419594E137637120"
	hash "0x419594E137637120"
	jhash (0x54058F5F)
	arguments {
		BOOL "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xFC18DB55AE19E046"
	hash "0xFC18DB55AE19E046"
	jhash (0xA7E36020)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x5C707A667DF8B9FA"
	hash "0x5C707A667DF8B9FA"
	jhash (0x64235620)
	arguments {
		BOOL "p0",

		Any "p1",
	}
	returns	"void"

native "NETWORK_IS_IN_SPECTATOR_MODE"
	hash "0x048746E388762E11"
	jhash (0x3EAD9DB8)
	returns	"intPtr"

native "NETWORK_SET_IN_MP_CUTSCENE"
	hash "0x9CA5DE655269FEC4"
	jhash (0x8434CB43)
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"void"

native "NETWORK_IS_IN_MP_CUTSCENE"
	hash "0x6CC27C9FA2040220"
	jhash (0x4BB33316)
	returns	"Any"

native "NETWORK_IS_PLAYER_IN_MP_CUTSCENE"
	hash "0x63F9EE203C3619F2"
	jhash (0x56F961E4)
	arguments {
		Player "player",
	}
	returns	"BOOL"

native "SET_NETWORK_VEHICLE_RESPOT_TIMER"
	hash "0xEC51713AB6EC36E8"
	jhash (0x2C30912D)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x6274C4712850841E"
	hash "0x6274C4712850841E"
	jhash (0xEA235081)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "USE_PLAYER_COLOUR_INSTEAD_OF_TEAM_COLOUR"
	hash "0x5FFE9B4144F9712F"
	jhash (0x4DD46DAE)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0x21D04D7BC538C146"
	hash "0x21D04D7BC538C146"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x77758139EC9B66C7"
	hash "0x77758139EC9B66C7"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "NETWORK_CREATE_SYNCHRONISED_SCENE"
	hash "0x7CD6BC4C2BBDD526"
	jhash (0xB06FE3FE)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",

		Any "p9",
	}
	returns	"Any"

native "NETWORK_ADD_PED_TO_SYNCHRONISED_SCENE"
	hash "0x742A637471BCECD9"
	jhash (0xB386713E)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		AnyPtr "p3",

		float "p4",

		float "p5",

		Any "p6",

		Any "p7",

		float "p8",

		Any "p9",
	}
	returns	"void"

native "NETWORK_ADD_ENTITY_TO_SYNCHRONISED_SCENE"
	hash "0xF2404D68CBC855FA"
	jhash (0x10DD636C)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		AnyPtr "p3",

		float "p4",

		float "p5",

		Any "p6",
	}
	returns	"void"

native "0xCF8BD3B0BD6D42D7"
	hash "0xCF8BD3B0BD6D42D7"
	jhash (0xBFFE8B5C)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "NETWORK_ATTACH_SYNCHRONISED_SCENE_TO_ENTITY"
	hash "0x478DCBD2A98B705A"
	jhash (0x3FE5B222)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "NETWORK_START_SYNCHRONISED_SCENE"
	hash "0x9A1B3FCDB36C8697"
	jhash (0xA9DFDC40)
	arguments {
		Any "p0",
	}
	returns	"void"

native "NETWORK_STOP_SYNCHRONISED_SCENE"
	hash "0xC254481A4574CB2F"
	jhash (0x97B1CDF6)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x02C40BF885C567B6"
	hash "0x02C40BF885C567B6"
	jhash (0x16AED87B)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xC9B43A33D09CADA7"
	hash "0xC9B43A33D09CADA7"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xFB1F9381E80FA13F"
	hash "0xFB1F9381E80FA13F"
	jhash (0x0679CE71)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x5A6FFA2433E2F14C"
	hash "0x5A6FFA2433E2F14C"
	jhash (0xC62E77B3)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		Any "p8",
	}
	returns	"BOOL"

native "0x4BA92A18502BCA61"
	hash "0x4BA92A18502BCA61"
	jhash (0x74D6B13C)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",

		float "p7",

		float "p8",

		float "p9",

		float "p10",

		Any "p11",
	}
	returns	"BOOL"

native "0x3C891A251567DFCE"
	hash "0x3C891A251567DFCE"
	jhash (0x90700C7D)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "0xFB8F2A6F3DF08CBE"
	hash "0xFB8F2A6F3DF08CBE"
	jhash (0x44BFB619)
	returns	"void"

native "NETWORK_GET_RESPAWN_RESULT"
	hash "0x371EA43692861CF1"
	jhash (0xDDFE9FBC)
	arguments {
		Any "p0",

		intPtr "p1",

		intPtr "p2",
	}
	returns	"void"

native "0x6C34F1208B8923FD"
	hash "0x6C34F1208B8923FD"
	jhash (0x03287FD2)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x17E0198B3882C2CB"
	hash "0x17E0198B3882C2CB"
	jhash (0x408A9436)
	returns	"void"

native "0xFB680D403909DC70"
	hash "0xFB680D403909DC70"
	jhash (0xFFB2ADA1)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "NETWORK_END_TUTORIAL_SESSION"
	hash "0xD0AFAFF5A51D72F7"
	jhash (0xBA57E53E)
	returns	"void"

native "NETWORK_IS_IN_TUTORIAL_SESSION"
	hash "0xADA24309FE08DACF"
	jhash (0x34DD7B28)
	returns	"Any"

native "0xB37E4E6A2388CA7B"
	hash "0xB37E4E6A2388CA7B"
	jhash (0x755A2B3E)
	returns	"Any"

native "0x35F0B98A8387274D"
	hash "0x35F0B98A8387274D"
	jhash (0xA003C40B)
	returns	"Any"

native "0x3B39236746714134"
	hash "0x3B39236746714134"
	jhash (0x5E1020CC)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x9DE986FC9A87C474"
	hash "0x9DE986FC9A87C474"
	jhash (0xE66A0B40)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0xBBDF066252829606"
	hash "0xBBDF066252829606"
	jhash (0x72052DB3)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x919B3C98ED8292F9"
	hash "0x919B3C98ED8292F9"
	jhash (0xB0313590)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	Works in Singleplayer too.
</summary>
]]--
native "NETWORK_OVERRIDE_CLOCK_TIME"
	hash "0xE679E3E06E363892"
	jhash (0xC077BCD6)
	arguments {
		int "Hours",

		int "Minutes",

		int "Seconds",
	}
	returns	"void"

native "0xD972DF67326F966E"
	hash "0xD972DF67326F966E"
	jhash (0xC4AEAF49)
	returns	"void"

native "0xD7C95D322FF57522"
	hash "0xD7C95D322FF57522"
	jhash (0x2465296D)
	returns	"Any"

native "NETWORK_ADD_ENTITY_AREA"
	hash "0x494C8FB299290269"
	jhash (0x51030E5B)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"Any"

native "0x376C6375BA60293A"
	hash "0x376C6375BA60293A"
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",

		float "p6",
	}
	returns	"Any"

native "0x25B99872D588A101"
	hash "0x25B99872D588A101"
	jhash (0x4C2C2B12)
	arguments {
		float "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",

		float "p5",
	}
	returns	"Any"

native "NETWORK_REMOVE_ENTITY_AREA"
	hash "0x93CF869BAA0C4874"
	jhash (0xEAB97F25)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xE64A3CA08DFA37A9"
	hash "0xE64A3CA08DFA37A9"
	jhash (0x69956127)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x4DF7CFFF471A7FB1"
	hash "0x4DF7CFFF471A7FB1"
	jhash (0xCB1CD6D3)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x4A2D4E8BF4265B0F"
	hash "0x4A2D4E8BF4265B0F"
	jhash (0xC6D53AA0)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x2B1813ABA29016C5"
	hash "0x2B1813ABA29016C5"
	jhash (0x155465EE)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x924426BFFD82E915"
	hash "0x924426BFFD82E915"
	jhash (0x29532731)
	returns	"Any"

native "0x8132C0EB8B2B3293"
	hash "0x8132C0EB8B2B3293"
	returns	"Any"

native "0x42FB3B532D526E6C"
	hash "0x42FB3B532D526E6C"
	jhash (0xD760CAD5)
	returns	"void"

native "0x0467C11ED88B7D28"
	hash "0x0467C11ED88B7D28"
	returns	"Any"

native "0x10BD227A753B0D84"
	hash "0x10BD227A753B0D84"
	jhash (0x231CFD12)
	returns	"Any"

native "NETWORK_DOES_TUNABLE_EXIST"
	hash "0x85E5F8B9B898B20A"
	jhash (0x9FCE9C9A)
	arguments {
		AnyPtr "index",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x8BE1146DFD5D4468"
	hash "0x8BE1146DFD5D4468"
	jhash (0xE4B3726A)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0xE5608CA7BC163A5F"
	hash "0xE5608CA7BC163A5F"
	jhash (0x41E8912A)
	arguments {
		charPtr "p0",

		charPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0xAA6A47A573ABB75A"
	hash "0xAA6A47A573ABB75A"
	jhash (0x8A04E1FE)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0xE4E53E1419D81127"
	hash "0xE4E53E1419D81127"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x40FCE03E50E8DBE8"
	hash "0x40FCE03E50E8DBE8"
	arguments {
		Any "p0",

		Any "p1",

		intPtr "p2",
	}
	returns	"BOOL"

native "0x972BC203BBC4C4D5"
	hash "0x972BC203BBC4C4D5"
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0xEA16B69D93D71A45"
	hash "0xEA16B69D93D71A45"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0xC7420099936CE286"
	hash "0xC7420099936CE286"
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",
	}
	returns	"BOOL"

native "0x187382F8A3E0A6C3"
	hash "0x187382F8A3E0A6C3"
	jhash (0xA78571CA)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x7DB53B37A2F211A0"
	hash "0x7DB53B37A2F211A0"
	jhash (0x053BB329)
	returns	"Any"

native "NETWORK_RESET_BODY_TRACKER"
	hash "0x72433699B4E6DD64"
	jhash (0x3914463F)
	returns	"void"

native "0xD38C4A6D047C019D"
	hash "0xD38C4A6D047C019D"
	jhash (0x17CBC608)
	returns	"Any"

native "0x2E0BF682CC778D49"
	hash "0x2E0BF682CC778D49"
	jhash (0xBFAA349B)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x0EDE326D47CD0F3E"
	hash "0x0EDE326D47CD0F3E"
	jhash (0xBEB7281A)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

--[[!
<summary>
	In the console script dumps, this is only referenced once. 
	NETWORK::NETWORK_EXPLODE_VEHICLE(vehicle, 1, 0, 0);
</summary>
]]--
native "NETWORK_EXPLODE_VEHICLE"
	hash "0x301A42153C9AD707"
	jhash (0x0E1B38AE)
	arguments {
		Vehicle "vehicle",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"Any"

native "0xCD71A4ECAB22709E"
	hash "0xCD71A4ECAB22709E"
	jhash (0xBC54371B)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xA7E30DE9272B6D49"
	hash "0xA7E30DE9272B6D49"
	jhash (0x644141C5)
	arguments {
		Any "p0",

		float "p1",

		float "p2",

		float "p3",

		float "p4",
	}
	returns	"void"

native "0x407091CF6037118E"
	hash "0x407091CF6037118E"
	arguments {
		Any "p0",
	}
	returns	"void"

native "NETWORK_SET_PROPERTY_ID"
	hash "0x1775961C2FBBCB5C"
	jhash (0x5A74E873)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xC2B82527CA77053E"
	hash "0xC2B82527CA77053E"
	jhash (0x38BC35C8)
	returns	"void"

native "0x367EF5E2F439B4C6"
	hash "0x367EF5E2F439B4C6"
	jhash (0x53C9563C)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x94538037EE44F5CF"
	hash "0x94538037EE44F5CF"
	jhash (0x6B97075B)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xBD0BE0BFC927EAC1"
	hash "0xBD0BE0BFC927EAC1"
	returns	"void"

native "0x237D5336A9A54108"
	hash "0x237D5336A9A54108"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x99B72C7ABDE5C910"
	hash "0x99B72C7ABDE5C910"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0xF2EAC213D5EA0623"
	hash "0xF2EAC213D5EA0623"
	jhash (0x965EA007)
	returns	"Any"

native "0xEA14EEF5B7CD2C30"
	hash "0xEA14EEF5B7CD2C30"
	jhash (0xEEFC8A55)
	returns	"Any"

native "0xB606E6CC59664972"
	hash "0xB606E6CC59664972"
	jhash (0x866D1B67)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x1D4DC17C38FEAFF0"
	hash "0x1D4DC17C38FEAFF0"
	jhash (0xED4A272F)
	returns	"Any"

native "0x662635855957C411"
	hash "0x662635855957C411"
	jhash (0x4ACF110C)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xB4271092CA7EDF48"
	hash "0xB4271092CA7EDF48"
	jhash (0x1AA3A0D5)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xCA94551B50B4932C"
	hash "0xCA94551B50B4932C"
	jhash (0x37877757)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x2A7776C709904AB0"
	hash "0x2A7776C709904AB0"
	jhash (0x1CF89DA5)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x6F44CBF56D79FAC0"
	hash "0x6F44CBF56D79FAC0"
	jhash (0x16E53875)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x58C21165F6545892"
	hash "0x58C21165F6545892"
	jhash (0x365C50EE)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x2EAC52B4019E2782"
	hash "0x2EAC52B4019E2782"
	jhash (0x25E2DBA9)
	returns	"Any"

native "SET_STORE_ENABLED"
	hash "0x9641A9FF718E9C5E"
	jhash (0xC1F6443B)
	arguments {
		BOOL "toggle",
	}
	returns	"void"

native "0xA2F952104FC6DD4B"
	hash "0xA2F952104FC6DD4B"
	jhash (0x1FDC75DC)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x72D0706CD6CCDB58"
	hash "0x72D0706CD6CCDB58"
	jhash (0xCA7A0A49)
	returns	"void"

native "0x722F5D28B61C5EA8"
	hash "0x722F5D28B61C5EA8"
	jhash (0x44A58B0A)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x883D79C4071E18B3"
	hash "0x883D79C4071E18B3"
	jhash (0xD32FA11F)
	returns	"Any"

native "0x265635150FB0D82E"
	hash "0x265635150FB0D82E"
	jhash (0xA7FA70AE)
	returns	"void"

native "0x444C4525ECE0A4B9"
	hash "0x444C4525ECE0A4B9"
	jhash (0xCC7DCE24)
	returns	"void"

native "0x59328EB08C5CEB2B"
	hash "0x59328EB08C5CEB2B"
	jhash (0x70F6D3AD)
	returns	"Any"

native "0xFAE628F1E9ADB239"
	hash "0xFAE628F1E9ADB239"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0xC64DED7EF0D2FE37"
	hash "0xC64DED7EF0D2FE37"
	jhash (0x2B7B57B3)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "0x4C61B39930D045DA"
	hash "0x4C61B39930D045DA"
	jhash (0xBAF52DD8)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x3A3D5568AF297CD5"
	hash "0x3A3D5568AF297CD5"
	jhash (0x9B9AFFF1)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x4F18196C8D38768D"
	hash "0x4F18196C8D38768D"
	jhash (0xC38E9DB0)
	returns	"void"

native "0xC7ABAC5DE675EE3B"
	hash "0xC7ABAC5DE675EE3B"
	jhash (0x32A4EB22)
	returns	"Any"

native "0x0B0CC10720653F3B"
	hash "0x0B0CC10720653F3B"
	jhash (0x9262744C)
	returns	"Any"

native "0x8B0C2964BA471961"
	hash "0x8B0C2964BA471961"
	returns	"Any"

native "0x88B588B41FF7868E"
	hash "0x88B588B41FF7868E"
	returns	"Any"

native "0x67FC09BC554A75E5"
	hash "0x67FC09BC554A75E5"
	returns	"Any"

native "0x966DD84FB6A46017"
	hash "0x966DD84FB6A46017"
	returns	"void"

native "0x152D90E4C1B4738A"
	hash "0x152D90E4C1B4738A"
	jhash (0x08243B79)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x9FEDF86898F100E9"
	hash "0x9FEDF86898F100E9"
	jhash (0x798D6C27)
	returns	"Any"

native "0x5E24341A7F92A74B"
	hash "0x5E24341A7F92A74B"
	jhash (0xE69E8D0D)
	returns	"Any"

native "0x24E4E51FC16305F9"
	hash "0x24E4E51FC16305F9"
	jhash (0x742075FE)
	returns	"Any"

native "0xFBC5E768C7A77A6A"
	hash "0xFBC5E768C7A77A6A"
	jhash (0xCE569932)
	returns	"Any"

native "0xC55A0B40FFB1ED23"
	hash "0xC55A0B40FFB1ED23"
	jhash (0x82146BE9)
	returns	"Any"

native "0x17440AA15D1D3739"
	hash "0x17440AA15D1D3739"
	jhash (0x133FF2D5)
	returns	"void"

native "0x9BF438815F5D96EA"
	hash "0x9BF438815F5D96EA"
	jhash (0xCBA7242F)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"BOOL"

native "0x692D58DF40657E8C"
	hash "0x692D58DF40657E8C"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		AnyPtr "p3",

		Any "p4",

		BOOL "p5",
	}
	returns	"BOOL"

native "0x158EC424F35EC469"
	hash "0x158EC424F35EC469"
	jhash (0xDED82A6E)
	arguments {
		AnyPtr "p0",

		BOOL "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0xC7397A83F7A2A462"
	hash "0xC7397A83F7A2A462"
	arguments {
		AnyPtr "p0",

		Any "p1",

		BOOL "p2",

		AnyPtr "p3",
	}
	returns	"BOOL"

native "0x6D4CB481FAC835E8"
	hash "0x6D4CB481FAC835E8"
	jhash (0x40CF0783)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0xD5A4B59980401588"
	hash "0xD5A4B59980401588"
	jhash (0x4609D596)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"BOOL"

native "0x3195F8DD0D531052"
	hash "0x3195F8DD0D531052"
	jhash (0x4C2C0D1F)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"BOOL"

native "0xF9E1CCAE8BA4C281"
	hash "0xF9E1CCAE8BA4C281"
	jhash (0x9EFBD5D1)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"BOOL"

native "0x9F6E2821885CAEE2"
	hash "0x9F6E2821885CAEE2"
	jhash (0xA6D8B798)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		AnyPtr "p3",

		AnyPtr "p4",
	}
	returns	"BOOL"

native "0x678BB03C1A3BD51E"
	hash "0x678BB03C1A3BD51E"
	jhash (0x67E74842)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		AnyPtr "p3",

		AnyPtr "p4",
	}
	returns	"BOOL"

native "SET_BALANCE_ADD_MACHINE"
	hash "0x815E5E3073DA1D67"
	jhash (0xE123C7AC)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "SET_BALANCE_ADD_MACHINES"
	hash "0xB8322EEB38BE7C26"
	jhash (0x22C33603)
	arguments {
		AnyPtr "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0xA7862BC5ED1DFD7E"
	hash "0xA7862BC5ED1DFD7E"
	jhash (0x37F5BD93)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"BOOL"

native "0x97A770BEEF227E2B"
	hash "0x97A770BEEF227E2B"
	jhash (0x1CFB3F51)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"BOOL"

native "0x5324A0E3E4CE3570"
	hash "0x5324A0E3E4CE3570"
	jhash (0x87D1E6BD)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		AnyPtr "p3",
	}
	returns	"BOOL"

native "0xE9B99B6853181409"
	hash "0xE9B99B6853181409"
	jhash (0x021D5A94)
	returns	"void"

native "0xD53ACDBEF24A46E8"
	hash "0xD53ACDBEF24A46E8"
	jhash (0x4908A514)
	returns	"Any"

native "0x02ADA21EA2F6918F"
	hash "0x02ADA21EA2F6918F"
	jhash (0x50296140)
	returns	"Any"

native "0x941E5306BCD7C2C7"
	hash "0x941E5306BCD7C2C7"
	jhash (0x3970B0DA)
	returns	"Any"

native "0xC87E740D9F3872CC"
	hash "0xC87E740D9F3872CC"
	jhash (0xC1487110)
	returns	"Any"

native "0xEDF7F927136C224B"
	hash "0xEDF7F927136C224B"
	jhash (0xCC2356E3)
	returns	"Any"

native "0xE0A6138401BCB837"
	hash "0xE0A6138401BCB837"
	jhash (0x2DE69817)
	returns	"Any"

native "0x769951E2455E2EB5"
	hash "0x769951E2455E2EB5"
	jhash (0x81BD8D3B)
	returns	"Any"

native "0x3A17A27D75C74887"
	hash "0x3A17A27D75C74887"
	jhash (0x8E1D8F78)
	returns	"Any"

native "0xBA96394A0EECFA65"
	hash "0xBA96394A0EECFA65"
	jhash (0x0D35DD93)
	returns	"void"

native "0xCD67AD041A394C9C"
	hash "0xCD67AD041A394C9C"
	jhash (0x8F3137E6)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x584770794D758C18"
	hash "0x584770794D758C18"
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x8C8D2739BA44AF0F"
	hash "0x8C8D2739BA44AF0F"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x703F12425ECA8BF5"
	hash "0x703F12425ECA8BF5"
	jhash (0xB9137BA7)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xAEAB987727C5A8A4"
	hash "0xAEAB987727C5A8A4"
	jhash (0x9FEEAA9C)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xA7BAB11E7C9C6C5A"
	hash "0xA7BAB11E7C9C6C5A"
	jhash (0x5E8A7559)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x55AA95F481D694D2"
	hash "0x55AA95F481D694D2"
	jhash (0x331AEABF)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xC0173D6BFF4E0348"
	hash "0xC0173D6BFF4E0348"
	jhash (0x0E5E8E5C)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xBF09786A7FCAB582"
	hash "0xBF09786A7FCAB582"
	jhash (0xA5A0C695)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x7CF0448787B23758"
	hash "0x7CF0448787B23758"
	jhash (0x91534C6E)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xBAF6BABF9E7CCC13"
	hash "0xBAF6BABF9E7CCC13"
	jhash (0x744A9EA5)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0xCFD115B373C0DF63"
	hash "0xCFD115B373C0DF63"
	jhash (0xA19A238D)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x37025B27D9B658B1"
	hash "0x37025B27D9B658B1"
	jhash (0xFF7D44E6)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x1D610EB0FEA716D9"
	hash "0x1D610EB0FEA716D9"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x7FCC39C46C3C03BD"
	hash "0x7FCC39C46C3C03BD"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x32DD916F3F7C9672"
	hash "0x32DD916F3F7C9672"
	jhash (0xA2C5BD9D)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x3054F114121C21EA"
	hash "0x3054F114121C21EA"
	jhash (0xA850DDE1)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xA9240A96C74CCA13"
	hash "0xA9240A96C74CCA13"
	jhash (0x8F6754AE)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x1ACCFBA3D8DAB2EE"
	hash "0x1ACCFBA3D8DAB2EE"
	jhash (0x1E34953F)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x759299C5BB31D2A9"
	hash "0x759299C5BB31D2A9"
	jhash (0x771FE190)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x87E5C46C187FE0AE"
	hash "0x87E5C46C187FE0AE"
	jhash (0x3276D9D3)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x4E548C0D7AE39FF9"
	hash "0x4E548C0D7AE39FF9"
	jhash (0x41A0FB02)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x70EA8DA57840F9BE"
	hash "0x70EA8DA57840F9BE"
	jhash (0x11DC0F27)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x993CBE59D350D225"
	hash "0x993CBE59D350D225"
	jhash (0x0DEB3F5A)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x171DF6A0C07FB3DC"
	hash "0x171DF6A0C07FB3DC"
	jhash (0x84315226)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x7FD2990AF016795E"
	hash "0x7FD2990AF016795E"
	jhash (0x38FC2EEB)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		Any "p2",

		Any "p3",

		Any "p4",
	}
	returns	"Any"

native "0x5E0165278F6339EE"
	hash "0x5E0165278F6339EE"
	jhash (0x1C4F9FDB)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x2D5DC831176D0114"
	hash "0x2D5DC831176D0114"
	jhash (0xA69AE16C)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xEBFA8D50ADDC54C4"
	hash "0xEBFA8D50ADDC54C4"
	jhash (0xF50BC67A)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x162C23CA83ED0A62"
	hash "0x162C23CA83ED0A62"
	jhash (0xB3BBD241)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x40F7E66472DF3E5C"
	hash "0x40F7E66472DF3E5C"
	jhash (0x70A2845C)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x5A34CD9C3C5BEC44"
	hash "0x5A34CD9C3C5BEC44"
	jhash (0x346B506C)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x68103E2247887242"
	hash "0x68103E2247887242"
	jhash (0x0095DB71)
	returns	"void"

native "0x1DE0F5F50D723CAA"
	hash "0x1DE0F5F50D723CAA"
	jhash (0xAD334B40)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0x274A1519DFC1094F"
	hash "0x274A1519DFC1094F"
	jhash (0x980D45D7)
	arguments {
		AnyPtr "p0",

		BOOL "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0xD05D1A6C74DA3498"
	hash "0xD05D1A6C74DA3498"
	jhash (0x48CCC328)
	arguments {
		AnyPtr "p0",

		BOOL "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0x45E816772E93A9DB"
	hash "0x45E816772E93A9DB"
	jhash (0x8E664EFD)
	returns	"Any"

native "0x299EF3C576773506"
	hash "0x299EF3C576773506"
	jhash (0x611E0BE2)
	returns	"Any"

native "0x793FF272D5B365F4"
	hash "0x793FF272D5B365F4"
	jhash (0xF0211AC1)
	returns	"Any"

native "0x5A0A3D1A186A5508"
	hash "0x5A0A3D1A186A5508"
	jhash (0x1F0DD8AF)
	returns	"Any"

native "0xA1E5E0204A6FCC70"
	hash "0xA1E5E0204A6FCC70"
	jhash (0x405ECA16)
	returns	"void"

native "0xB746D20B17F2A229"
	hash "0xB746D20B17F2A229"
	jhash (0x9567392B)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x63B406D7884BFA95"
	hash "0x63B406D7884BFA95"
	jhash (0xF79FFF3C)
	returns	"Any"

native "0x4D02279C83BE69FE"
	hash "0x4D02279C83BE69FE"
	jhash (0xA7F3F82B)
	returns	"Any"

native "0x597F8DBA9B206FC7"
	hash "0x597F8DBA9B206FC7"
	jhash (0x410C61D1)
	returns	"Any"

native "0x5CAE833B0EE0C500"
	hash "0x5CAE833B0EE0C500"
	jhash (0x0D4F845D)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x61A885D3F7CFEE9A"
	hash "0x61A885D3F7CFEE9A"
	jhash (0xE13C1F7F)
	returns	"void"

native "0xF98DDE0A8ED09323"
	hash "0xF98DDE0A8ED09323"
	jhash (0x213C6D36)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xFD75DABC0957BF33"
	hash "0xFD75DABC0957BF33"
	jhash (0x511E6F50)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xF53E48461B71EECB"
	hash "0xF53E48461B71EECB"
	jhash (0xB4668B23)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x098AB65B9ED9A9EC"
	hash "0x098AB65B9ED9A9EC"
	jhash (0x30B51753)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0xDC48473142545431"
	hash "0xDC48473142545431"
	jhash (0x02DAD93F)
	returns	"Any"

native "0x0AE1F1653B554AB9"
	hash "0x0AE1F1653B554AB9"
	jhash (0x2D947814)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x62B9FEC9A11F10EF"
	hash "0x62B9FEC9A11F10EF"
	jhash (0x37A28C26)
	returns	"Any"

native "0xA75E2B6733DA5142"
	hash "0xA75E2B6733DA5142"
	jhash (0x11E8B5CD)
	returns	"Any"

native "0x43865688AE10F0D7"
	hash "0x43865688AE10F0D7"
	jhash (0x429AEAB3)
	returns	"Any"

native "TEXTURE_DOWNLOAD_REQUEST"
	hash "0x16160DA74A8E74A2"
	jhash (0xAD546CC3)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		BOOL "p3",
	}
	returns	"Any"

native "0x0B203B4AFDE53A4F"
	hash "0x0B203B4AFDE53A4F"
	jhash (0x1856D008)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		BOOL "p2",
	}
	returns	"Any"

native "0x308F96458B7087CC"
	hash "0x308F96458B7087CC"
	jhash (0x68C9AF69)
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		AnyPtr "p4",

		BOOL "p5",
	}
	returns	"Any"

native "TEXTURE_DOWNLOAD_RELEASE"
	hash "0x487EB90B98E9FB19"
	jhash (0xEE8D9E70)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x5776ED562C134687"
	hash "0x5776ED562C134687"
	jhash (0xE4547765)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "TEXTURE_DOWNLOAD_GET_NAME"
	hash "0x3448505B6E35262D"
	jhash (0xA40EF65A)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x8BD6C6DEA20E82C6"
	hash "0x8BD6C6DEA20E82C6"
	jhash (0x03225BA3)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x60EDD13EB3AC1FF3"
	hash "0x60EDD13EB3AC1FF3"
	jhash (0x4DEBC227)
	returns	"Any"

native "0xEFFB25453D8600F9"
	hash "0xEFFB25453D8600F9"
	jhash (0x5C065D55)
	returns	"Any"

native "0x66B59CFFD78467AF"
	hash "0x66B59CFFD78467AF"
	jhash (0x0CA1167F)
	returns	"Any"

native "0x606E4D3E3CCCF3EB"
	hash "0x606E4D3E3CCCF3EB"
	jhash (0x424C6E27)
	returns	"Any"

--[[!
<summary>
	if(_IS_ROCKSTAR_BANNED() == 0) means the player is rockstar banned
</summary>
]]--
native "_IS_ROCKSTAR_BANNED"
	hash "0x8020A73847E0CA7D"
	jhash (0xD3BBE42F)
	returns	"int"

--[[!
<summary>
	if(_IS_SOCIALCLUB_BANNED() == 0) means the player is social club banned
</summary>
]]--
native "_IS_SOCIALCLUB_BANNED"
	hash "0xA0AD7E2AF5349F61"
	jhash (0xBDBB5948)
	returns	"int"

--[[!
<summary>
	if(_IS_PLAYER_BANNED() == 0) means the player is banned(Social Club or Rockstar)
</summary>
]]--
native "_IS_PLAYER_BANNED"
	hash "0x5F91D5D0B36AA310"
	jhash (0x97287D68)
	returns	"int"

native "0x422D396F80A96547"
	hash "0x422D396F80A96547"
	jhash (0xC6EA802E)
	returns	"Any"

native "0xA699957E60D80214"
	hash "0xA699957E60D80214"
	jhash (0xFD261E30)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xC22912B1D85F26B1"
	hash "0xC22912B1D85F26B1"
	jhash (0x8570DD34)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0x593570C289A77688"
	hash "0x593570C289A77688"
	returns	"Any"

native "0x91B87C55093DE351"
	hash "0x91B87C55093DE351"
	returns	"Any"

native "0x36391F397731595D"
	hash "0x36391F397731595D"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xDEB2B99A1AF1A2A6"
	hash "0xDEB2B99A1AF1A2A6"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x9465E683B12D3F6B"
	hash "0x9465E683B12D3F6B"
	jhash (0x273C6180)
	returns	"void"

native "0xB7C7F6AD6424304B"
	hash "0xB7C7F6AD6424304B"
	jhash (0x371BBA08)
	returns	"void"

native "0xC505036A35AFD01B"
	hash "0xC505036A35AFD01B"
	jhash (0xA100CC97)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x267C78C60E806B9A"
	hash "0x267C78C60E806B9A"
	jhash (0xBB2D33D3)
	arguments {
		Any "p0",

		BOOL "p1",
	}
	returns	"void"

native "0x6BFF5F84102DF80A"
	hash "0x6BFF5F84102DF80A"
	arguments {
		Any "p0",
	}
	returns	"void"

native "0x5C497525F803486B"
	hash "0x5C497525F803486B"
	returns	"void"

native "0x6FB7BB3607D27FA2"
	hash "0x6FB7BB3607D27FA2"
	returns	"Any"

native "0x45A83257ED02D9BC"
	hash "0x45A83257ED02D9BC"
	returns	"void"

native "0x3DA5ECD1A56CBA6D"
	hash "0x3DA5ECD1A56CBA6D"
	jhash (0x66DA9935)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "NETWORK_DELETE_CHARACTER"
	hash "0x05A50AF38947EB8D"
	jhash (0xA9F7E9C3)
	arguments {
		int "characterIndex",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xA921DED15FDF28F5"
	hash "0xA921DED15FDF28F5"
	jhash (0x19F0C471)
	arguments {
		Any "p0",
	}
	returns	"void"

native "NETWORK_GIVE_PLAYER_JOBSHARE_CASH"
	hash "0xFB18DF9CB95E0105"
	jhash (0xC6047FDB)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "NETWORK_RECEIVE_PLAYER_JOBSHARE_CASH"
	hash "0x56A3B51944C50598"
	jhash (0x4ED71C1A)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x1C2473301B1C66BA"
	hash "0x1C2473301B1C66BA"
	jhash (0xA27B9FE8)
	returns	"Any"

native "0xF9C812CD7C46E817"
	hash "0xF9C812CD7C46E817"
	jhash (0x07C92F21)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		BOOL "p3",
	}
	returns	"void"

native "0x81404F3DC124FE5B"
	hash "0x81404F3DC124FE5B"
	jhash (0x8474E6F0)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"BOOL"

native "0x3A54E33660DED67F"
	hash "0x3A54E33660DED67F"
	jhash (0xE3802533)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "NETWORK_EARN_FROM_PICKUP"
	hash "0xED1517D3AF17C698"
	jhash (0x70A0ED62)
	arguments {
		int "amount",
	}
	returns	"Any"

native "0xA03D4ACE0A3284CE"
	hash "0xA03D4ACE0A3284CE"
	jhash (0x33C20BC4)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xF514621E8EA463D0"
	hash "0xF514621E8EA463D0"
	jhash (0x30B3EC0A)
	arguments {
		Any "p0",
	}
	returns	"void"

native "0xB1CC1B9EC3007A2A"
	hash "0xB1CC1B9EC3007A2A"
	jhash (0xEAF04923)
	arguments {
		Any "p0",
	}
	returns	"void"

native "NETWORK_EARN_FROM_BETTING"
	hash "0x827A5BA1A44ACA6D"
	jhash (0xA0F7F07C)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "NETWORK_EARN_FROM_JOB"
	hash "0xB2CC4836834E8A98"
	jhash (0x0B6997FC)
	arguments {
		int "amount",

		AnyPtr "p1",
	}
	returns	"Any"

native "0x61326EE6DF15B0CA"
	hash "0x61326EE6DF15B0CA"
	jhash (0x5E81F55C)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x2B171E6B2F64D8DF"
	hash "0x2B171E6B2F64D8DF"
	jhash (0x2BEFB6C4)
	arguments {
		Any "p0",

		AnyPtr "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_EARN_FROM_BOUNTY"
	hash "0x131BB5DA15453ACF"
	jhash (0x127F2DAE)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",

		Any "p3",
	}
	returns	"void"

native "NETWORK_EARN_FROM_IMPORT_EXPORT"
	hash "0xF92A014A634442D6"
	jhash (0xF11FC458)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "NETWORK_EARN_FROM_HOLDUPS"
	hash "0x45B8154E077D9E4D"
	jhash (0xE6B90E9C)
	arguments {
		Any "p0",
	}
	returns	"void"

native "NETWORK_EARN_FROM_PROPERTY"
	hash "0x849648349D77F5C5"
	jhash (0x9BE4F7E1)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x515B4A22E4D3C6D7"
	hash "0x515B4A22E4D3C6D7"
	jhash (0x866004A8)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x4337511FA8221D36"
	hash "0x4337511FA8221D36"
	jhash (0xCC068380)
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Example:

	--Function--

	void EarnMoney(int value)
	{
	   NETWORKCASH::NETWORK_EARN_FROM_ROCKSTAR(value);
	}

	Use like that after:

	EarnMoney(1337); //1337 HAX BRO :D

	*************************

	This merely adds an entry in the Network Transaction Log; it does not grant cash to the player.


</summary>
]]--
native "NETWORK_EARN_FROM_ROCKSTAR"
	hash "0x02CE1D6AC0FC73EA"
	jhash (0x5A3733CC)
	arguments {
		int "characterIndex",
	}
	returns	"void"

--[[!
<summary>
	Now has 8 params.
</summary>
]]--
native "NETWORK_EARN_FROM_VEHICLE"
	hash "0xB539BD8A4C1EECF8"
	jhash (0xF803589D)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",
	}
	returns	"void"

--[[!
<summary>
	Now has 9 parameters.
</summary>
]]--
native "0x3F4D00167E41E0AD"
	hash "0x3F4D00167E41E0AD"
	jhash (0x96B8BEE8)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",

		Any "p6",

		Any "p7",

		Any "p8",
	}
	returns	"void"

native "0x6EA318C91C1A8786"
	hash "0x6EA318C91C1A8786"
	arguments {
		Any "p0",

		AnyPtr "p1",

		Any "p2",
	}
	returns	"void"

--[[!
<summary>
	Example for p1: "AM_DISTRACT_COPS"
</summary>
]]--
native "0xFB6DB092FBAE29E6"
	hash "0xFB6DB092FBAE29E6"
	arguments {
		int "p0",

		charPtr "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0x6816FB4416760775"
	hash "0x6816FB4416760775"
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"void"

native "0xAB3CAA6B422164DA"
	hash "0xAB3CAA6B422164DA"
	jhash (0x5AA379D9)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",

		Any "p4",
	}
	returns	"BOOL"

native "0x7303E27CC6532080"
	hash "0x7303E27CC6532080"
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",

		BOOL "p3",

		AnyPtr "p4",

		Any "p5",
	}
	returns	"BOOL"

native "NETWORK_BUY_ITEM"
	hash "0xF0077C797F66A355"
	jhash (0xA07B6368)
	arguments {
		Ped "player",

		Hash "item",

		Any "p2",

		Any "p3",

		BOOL "p4",

		AnyPtr "p5",

		Any "p6",

		Any "p7",

		Any "p8",

		BOOL "p9",
	}
	returns	"void"

native "NETWORK_SPENT_TAXI"
	hash "0x17C3A7D31EAE39F9"
	jhash (0x1F3DB3E3)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x5FD5ED82CBBE9989"
	hash "0x5FD5ED82CBBE9989"
	jhash (0xBE70849B)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0xAFE08B35EC0C9EAE"
	hash "0xAFE08B35EC0C9EAE"
	jhash (0x451A2644)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x9346E14F2AF74D46"
	hash "0x9346E14F2AF74D46"
	jhash (0x224A3488)
	arguments {
		Any "p0",

		AnyPtr "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "NETWORK_SPENT_BETTING"
	hash "0x1C436FD11FFA692F"
	jhash (0xF8A07513)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"void"

native "0xEE99784E4467689C"
	hash "0xEE99784E4467689C"
	jhash (0x8957038E)
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",

		BOOL "p3",
	}
	returns	"void"

native "NETWORK_BUY_HEALTHCARE"
	hash "0xD9B067E55253E3DD"
	jhash (0x832150E5)
	arguments {
		int "cost",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	p1 = 0 (always)
	p2 = 1 (always)
</summary>
]]--
native "NETWORK_BUY_AIRSTRIKE"
	hash "0x763B4BD305338F19"
	jhash (0x40470683)
	arguments {
		int "cost",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	p1 = 0 (always)
	p2 = 1 (always)
</summary>
]]--
native "NETWORK_BUY_HELI_STRIKE"
	hash "0x81AA4610E3FD3A69"
	jhash (0x047547D4)
	arguments {
		int "cost",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_AMMO_DROP"
	hash "0xB162DC95C0A3317B"
	jhash (0x4B643076)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	p1 is just an assumption. p2 was false and p3 was true. 
</summary>
]]--
native "NETWORK_BUY_BOUNTY"
	hash "0x7B718E197453F2D9"
	jhash (0xCB89CBE0)
	arguments {
		int "amount",

		Player "victim",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "NETWORK_BUY_PROPERTY"
	hash "0x650A08A280870AF6"
	jhash (0x7D479AAB)
	arguments {
		float "propertyCost",

		Hash "propertyName",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "NETWORK_SPENT_HELI_PICKUP"
	hash "0x7BF1D73DB2ECA492"
	jhash (0x27EEBCAB)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_BOAT_PICKUP"
	hash "0x524EE43A37232C00"
	jhash (0xB241CABD)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_BULL_SHARK"
	hash "0xA6DD8458CE24012C"
	jhash (0xDE7D398C)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_CASH_DROP"
	hash "0x289016EC778D60E0"
	jhash (0x87BD1D11)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_HIRE_MUGGER"
	hash "0xE404BFB981665BF0"
	jhash (0xE792C4A5)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x995A65F15F581359"
	hash "0x995A65F15F581359"
	jhash (0xE6AAA0D5)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_HIRE_MERCENARY"
	hash "0xE7B80E2BF9D80BD6"
	jhash (0x99CF02C4)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_BUY_WANTEDLEVEL"
	hash "0xE1B13771A843C4F6"
	jhash (0xE7CB4F95)
	arguments {
		Any "p0",

		AnyPtr "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "NETWORK_SPENT_BUY_OFFTHERADAR"
	hash "0xA628A745E2275C5D"
	jhash (0x20DDCF2F)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_BUY_REVEAL_PLAYERS"
	hash "0x6E176F1B18BC0637"
	jhash (0x2F7836E2)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_CARWASH"
	hash "0xEC03C719DB2F4306"
	jhash (0x8283E028)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"void"

native "NETWORK_SPENT_CINEMA"
	hash "0x6B38ECB05A63A685"
	jhash (0x1100CAF5)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "NETWORK_SPENT_TELESCOPE"
	hash "0x7FE61782AD94CC09"
	jhash (0xAE7FF044)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_HOLDUPS"
	hash "0xD9B86B9872039763"
	jhash (0x1B3803B1)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_BUY_PASSIVE_MODE"
	hash "0x6D3A430D1A809179"
	jhash (0x7E97C92C)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_PROSTITUTES"
	hash "0xB21B89501CFAC79E"
	jhash (0x78436D07)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_ARREST_BAIL"
	hash "0x812F5488B1B2A299"
	jhash (0x5AEE2FC1)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "NETWORK_SPENT_PAY_VEHICLE_INSURANCE_PREMIUM"
	hash "0x9FF28D88C766E3E8"
	jhash (0x4E665BB2)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		BOOL "p3",

		BOOL "p4",
	}
	returns	"void"

native "NETWORK_SPENT_CALL_PLAYER"
	hash "0xACDE7185B374177C"
	jhash (0x1A89B5FC)
	arguments {
		Any "p0",

		AnyPtr "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "NETWORK_SPENT_BOUNTY"
	hash "0x29B260B84947DFCC"
	jhash (0x3401FC96)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x6A445B64ED7ABEB5"
	hash "0x6A445B64ED7ABEB5"
	jhash (0x54198922)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	Now has 3 parameters.
</summary>
]]--
native "0x20194D48EAEC9A41"
	hash "0x20194D48EAEC9A41"
	jhash (0xC5D8B1E9)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"Any"

native "0x7C99101F7FCE2EE5"
	hash "0x7C99101F7FCE2EE5"
	jhash (0x3D96A21C)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "0xD5BB406F4E04019F"
	hash "0xD5BB406F4E04019F"
	jhash (0x2E51C61C)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x8204DA7934DF3155"
	hash "0x8204DA7934DF3155"
	jhash (0xD57A5125)
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x9D26502BB97BFE62"
	hash "0x9D26502BB97BFE62"
	arguments {
		Any "p0",

		BOOL "p1",

		BOOL "p2",
	}
	returns	"void"

native "0x8A7B3952DD64D2B5"
	hash "0x8A7B3952DD64D2B5"
	jhash (0xD9622D64)
	arguments {
		Any "p0",

		Any "p1",

		BOOL "p2",

		BOOL "p3",
	}
	returns	"void"

native "0x7C4FCCD2E4DEB394"
	hash "0x7C4FCCD2E4DEB394"
	returns	"Any"

native "0x76EF28DA05EA395A"
	hash "0x76EF28DA05EA395A"
	jhash (0x16184FB5)
	returns	"Any"

native "0xA40F9C2623F6A8B5"
	hash "0xA40F9C2623F6A8B5"
	jhash (0x4F5B781C)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x5CBAD97E059E1B94"
	hash "0x5CBAD97E059E1B94"
	jhash (0xADF8F882)
	returns	"Any"

native "0xA6FA3979BED01B81"
	hash "0xA6FA3979BED01B81"
	returns	"Any"

native "0xDC18531D7019A535"
	hash "0xDC18531D7019A535"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "NETWORK_CAN_RECEIVE_PLAYER_CASH"
	hash "0x5D17BE59D2123284"
	jhash (0x41F5F10E)
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0xF70EFA14FE091429"
	hash "0xF70EFA14FE091429"
	jhash (0x8B755993)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xE260E0BB9CD995AC"
	hash "0xE260E0BB9CD995AC"
	jhash (0x8F266745)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xE154B48B68EF72BC"
	hash "0xE154B48B68EF72BC"
	jhash (0x531E4892)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x6FCF8DDEA146C45B"
	hash "0x6FCF8DDEA146C45B"
	jhash (0xB96C7ABE)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x278F76C3B0A8F109"
	hash "0x278F76C3B0A8F109"
	jhash (0x71D0CF3E)
	arguments {
		int "p0",
	}
	returns	"int"

native "0xFF56381874F82086"
	hash "0xFF56381874F82086"
	jhash (0x2E9D628C)
	arguments {
		int "p0",

		int "p1",

		AnyPtr "outComponent",
	}
	returns	"BOOL"

native "INIT_SHOP_PED_COMPONENT"
	hash "0x1E8C308FD312C036"
	jhash (0xB818C7FC)
	arguments {
		AnyPtr "outComponent",
	}
	returns	"void"

native "INIT_SHOP_PED_PROP"
	hash "0xEB0A2B758F7B850F"
	jhash (0xF5659E50)
	arguments {
		AnyPtr "outProp",
	}
	returns	"void"

native "0x50F457823CE6EB5F"
	hash "0x50F457823CE6EB5F"
	jhash (0xC937FF3D)
	arguments {
		int "p0",

		int "p1",

		int "p2",

		int "p3",
	}
	returns	"int"

--[[!
<summary>
	playerId is 0 for Michael, 1 for Franklin, 2 for Trevor, 3 for freemode male, and 4 for freemode female.

	componentId is between 0 and 11 and corresponds to the usual component slots.

	p1 could be the outfit number; unsure.

	p2 is usually -1; unknown function.

	p3 appears to be a boolean flag; unknown function.

	p4 is usually -1; unknown function.
</summary>
]]--
native "_GET_NUM_PROPS_FROM_OUTFIT"
	hash "0x9BDF59818B1E38C1"
	jhash (0x594E862C)
	arguments {
		int "playerId",

		int "p1",

		int "p2",

		BOOL "p3",

		int "p4",

		int "componentId",
	}
	returns	"int"

native "GET_SHOP_PED_QUERY_COMPONENT"
	hash "0x249E310B2D920699"
	jhash (0xC0718904)
	arguments {
		int "componentId",

		AnyPtr "outComponent",
	}
	returns	"void"

native "GET_SHOP_PED_COMPONENT"
	hash "0x74C0E2A57EC66760"
	jhash (0xB39677C5)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "GET_SHOP_PED_QUERY_PROP"
	hash "0xDE44A00999B2837D"
	jhash (0x1D3C1466)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x5D5CAFF661DDF6FC"
	hash "0x5D5CAFF661DDF6FC"
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "GET_HASH_NAME_FOR_COMPONENT"
	hash "0x0368B3A838070348"
	jhash (0xC8A4BF12)
	arguments {
		Entity "entity",

		int "componentId",

		int "drawableVariant",

		int "textureVariant",
	}
	returns	"Hash"

native "GET_HASH_NAME_FOR_PROP"
	hash "0x5D6160275CAEC8DD"
	jhash (0x7D876DC0)
	arguments {
		Entity "entity",

		int "componentId",

		int "propIndex",

		int "propTextureIndex",
	}
	returns	"Hash"

native "0xC17AD0E5752BECDA"
	hash "0xC17AD0E5752BECDA"
	jhash (0x159751B4)
	arguments {
		Hash "componentHash",
	}
	returns	"int"

native "GET_VARIANT_COMPONENT"
	hash "0x6E11F282F11863B6"
	jhash (0xE4FF7103)
	arguments {
		Hash "componentHash",

		int "componentId",

		AnyPtr "p2",

		AnyPtr "p3",

		AnyPtr "p4",
	}
	returns	"void"

--[[!
<summary>
	Returns number of possible values of the componentId argument of GET_FORCED_COMPONENT.
</summary>
]]--
native "_GET_NUM_FORCED_COMPONENTS"
	hash "0xC6B9DB42C04DD8C3"
	jhash (0xCE70F183)
	arguments {
		Hash "componentHash",
	}
	returns	"int"

native "0x017568A8182D98A6"
	hash "0x017568A8182D98A6"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_FORCED_COMPONENT"
	hash "0x6C93ED8C2F74859B"
	jhash (0x382C70BE)
	arguments {
		Hash "componentHash",

		int "componentId",

		AnyPtr "p2",

		AnyPtr "p3",

		AnyPtr "p4",
	}
	returns	"void"

native "0xE1CA84EBF72E691D"
	hash "0xE1CA84EBF72E691D"
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",

		AnyPtr "p3",

		AnyPtr "p4",
	}
	returns	"void"

native "0x341DE7ED1D2A1BFD"
	hash "0x341DE7ED1D2A1BFD"
	jhash (0x8E2C7FD5)
	arguments {
		Hash "componentHash",

		Hash "drawableSlotHash",

		int "p2",
	}
	returns	"BOOL"

--[[!
<summary>
	characters

	0: Michael
	1: Franklin
	2: Trevor
	3: MPMale
	4: MPFemale
</summary>
]]--
native "0xF3FBE2D50A6A8C28"
	hash "0xF3FBE2D50A6A8C28"
	jhash (0x1ECD23E7)
	arguments {
		int "character",

		BOOL "p1",
	}
	returns	"int"

native "GET_SHOP_PED_QUERY_OUTFIT"
	hash "0x6D793F03A631FE56"
	jhash (0x2F8013A1)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "GET_SHOP_PED_OUTFIT"
	hash "0xB7952076E444979D"
	jhash (0xCAFE9209)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"void"

native "0x073CA26B079F956E"
	hash "0x073CA26B079F956E"
	jhash (0x2798F56F)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xA9F9C2E0FDE11CBB"
	hash "0xA9F9C2E0FDE11CBB"
	jhash (0x6641A864)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "_GET_PROP_FROM_OUTFIT"
	hash "0x19F2A026EDF0013F"
	jhash (0x818534AC)
	arguments {
		Any "outfit",

		Any "slot",

		AnyPtr "item",
	}
	returns	"BOOL"

native "GET_NUM_DLC_VEHICLES"
	hash "0xA7A866D21CD2329B"
	jhash (0x8EAF9CF6)
	returns	"int"

native "GET_DLC_VEHICLE_MODEL"
	hash "0xECC01B7C5763333C"
	jhash (0xA2201E09)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "GET_DLC_VEHICLE_DATA"
	hash "0x33468EDC08E371F6"
	jhash (0xCF428FA4)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "GET_DLC_VEHICLE_FLAGS"
	hash "0x5549EE11FA22FCF2"
	jhash (0xAB12738C)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	Gets the total number of DLC weapons.
</summary>
]]--
native "GET_NUM_DLC_WEAPONS"
	hash "0xEE47635F352DA367"
	jhash (0x2B757E6C)
	returns	"int"

native "GET_DLC_WEAPON_DATA"
	hash "0x79923CD21BECE14E"
	jhash (0xD88EC8EA)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "GET_NUM_DLC_WEAPON_COMPONENTS"
	hash "0x405425358A7D61FE"
	jhash (0x476B23A9)
	arguments {
		Any "p0",
	}
	returns	"int"

native "GET_DLC_WEAPON_COMPONENT_DATA"
	hash "0x6CF598A2957C2BF8"
	jhash (0x4B83FCAF)
	arguments {
		Any "p0",

		Any "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "_IS_OUTFIT_EMPTY"
	hash "0xD4D7B033C3AA243C"
	jhash (0x06396058)
	arguments {
		Any "outfit",
	}
	returns	"BOOL"

native "0x0564B9FF9631B82C"
	hash "0x0564B9FF9631B82C"
	jhash (0x35BCA844)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xC098810437312FFF"
	hash "0xC098810437312FFF"
	jhash (0x59352658)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "IS_DLC_PRESENT"
	hash "0x812595A0644CE1DE"
	jhash (0x1F321943)
	arguments {
		Hash "DlcHash",
	}
	returns	"BOOL"

native "0xF2E07819EF1A5289"
	hash "0xF2E07819EF1A5289"
	jhash (0x881B1FDB)
	returns	"Any"

native "0x9489659372A81585"
	hash "0x9489659372A81585"
	jhash (0xC2169164)
	returns	"Any"

native "0xA213B11DFF526300"
	hash "0xA213B11DFF526300"
	jhash (0xF79A97F5)
	returns	"Any"

native "0x8D30F648014A92B5"
	hash "0x8D30F648014A92B5"
	jhash (0xF69B729C)
	returns	"Any"

native "GET_IS_LOADING_SCREEN_ACTIVE"
	hash "0x10D0A8F259E93EC9"
	jhash (0x517B601B)
	returns	"BOOL"

--[[!
<summary>
	Sets the value of the specified variable to 0.
	Always returns true.

	bool _NULLIFY(void* variable, int unused)
	{
	    *variable = NULL;
	    return true;
	}
</summary>
]]--
native "_NULLIFY"
	hash "0x46E2B844905BC5F0"
	jhash (0x6087C10C)
	arguments {
		AnyPtr "variable",

		Any "unused",
	}
	returns	"BOOL"

--[[!
<summary>
	Unloads GROUP_MAP (GTAO/MP) DLC data and loads GROUP_MAP_SP DLC. Neither are loaded by default, 0888C3502DBBEEF5 is a cognate to this function and loads MP DLC (and unloads SP DLC by extension).

	-- NTAuthority (http://fivem.net/)

	The original (and wrong) definition is below:

	This unload the GTA:O DLC map parts (like high end garages/apartments).
	Works in singleplayer.
</summary>
]]--
native "_LOAD_SP_DLC_MAPS"
	hash "0xD7C10C4A637992C9"
	returns	"void"

--[[!
<summary>
	This loads the GTA:O dlc map parts (high end garages, apartments).
	Works in singleplayer.
	In order to use GTA:O heist IPL's you have to call this native with the following params: _9BAE5AD2508DF078(1);
</summary>
]]--
native "_LOAD_MP_DLC_MAPS"
	hash "0x0888C3502DBBEEF5"
	returns	"void"

--[[!
<summary>
	It does not actually seem to wait the amount of milliseconds stated like the normal WAIT() command does, but it does seem to make task sequences work more smoothly
</summary>
]]--
native "WAIT"
	hash "0x4EDE34FBADD967A6"
	jhash (0x7715C03B)
	arguments {
		int "ms",
	}
	returns	"void"

--[[!
<summary>
	By default the stack size value is 0x200 (512)
	return : script thread id, 0 if failed
</summary>
]]--
native "START_NEW_SCRIPT"
	hash "0xE81651AD79516E48"
	jhash (0x3F166D0E)
	arguments {
		charPtr "scriptName",

		int "stackSize",
	}
	returns	"int"

--[[!
<summary>
	return : script thread id, 0 if failed
	Pass pointer to struct of args in p1, size of struct goes into p2
</summary>
]]--
native "START_NEW_SCRIPT_WITH_ARGS"
	hash "0xB8BA7F44DF1575E1"
	jhash (0x4A2100E4)
	arguments {
		charPtr "scriptName",

		AnyPtr "args",

		int "argCount",

		int "stackSize",
	}
	returns	"int"

native "_START_NEW_STREAMED_SCRIPT"
	hash "0xEB1C67C3A5333A92"
	jhash (0x8D15BE5D)
	arguments {
		Hash "scriptHash",

		int "stackSize",
	}
	returns	"int"

native "_START_NEW_STREAMED_SCRIPT_WITH_ARGS"
	hash "0xC4BB298BD441BE78"
	jhash (0xE38A3AD4)
	arguments {
		Hash "scriptHash",

		AnyPtr "args",

		int "argCount",

		int "stackSize",
	}
	returns	"int"

--[[!
<summary>
	Counts up. Every 1000 is 1 real-time second. Use SETTIMERA(int value) to set the timer (e.g.: SETTIMERA(0)).
</summary>
]]--
native "TIMERA"
	hash "0x83666F9FB8FEBD4B"
	jhash (0x45C8C188)
	returns	"int"

native "TIMERB"
	hash "0xC9D9444186B5A374"
	jhash (0x330A9C0C)
	returns	"int"

native "SETTIMERA"
	hash "0xC1B1E9A034A63A62"
	jhash (0x35785333)
	arguments {
		int "value",
	}
	returns	"void"

native "SETTIMERB"
	hash "0x5AE11BC36633DE4E"
	jhash (0x27C1B7C6)
	arguments {
		int "value",
	}
	returns	"void"

native "TIMESTEP"
	hash "0x0000000050597EE2"
	jhash (0x50597EE2)
	returns	"float"

native "SIN"
	hash "0x0BADBFA3B172435F"
	jhash (0xBF987F58)
	arguments {
		float "value",
	}
	returns	"float"

native "COS"
	hash "0xD0FFB162F40A139C"
	jhash (0x00238FE9)
	arguments {
		float "value",
	}
	returns	"float"

native "SQRT"
	hash "0x71D93B57D07F9804"
	jhash (0x145C7701)
	arguments {
		float "value",
	}
	returns	"float"

native "POW"
	hash "0xE3621CC40F31FE2E"
	jhash (0x85D134F8)
	arguments {
		float "base",

		float "exponent",
	}
	returns	"float"

native "VMAG"
	hash "0x652D2EEEF1D3E62C"
	jhash (0x1FCF1ECD)
	arguments {
		float "p0",

		float "p1",

		float "p2",
	}
	returns	"float"

native "VMAG2"
	hash "0xA8CEACB4F35AE058"
	jhash (0xE796E629)
	arguments {
		float "p0",

		float "p1",

		float "p2",
	}
	returns	"float"

--[[!
<summary>
	calculate vertical distance between two points?
</summary>
]]--
native "VDIST"
	hash "0x2A488C176D52CCA5"
	jhash (0x3C08ECB7)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",
	}
	returns	"float"

native "VDIST2"
	hash "0xB7A628320EFF8E47"
	jhash (0xC85DEF1F)
	arguments {
		float "x1",

		float "y1",

		float "z1",

		float "x2",

		float "y2",

		float "z2",
	}
	returns	"float"

native "SHIFT_LEFT"
	hash "0xEDD95A39E5544DE8"
	jhash (0x314CC6CD)
	arguments {
		int "value",

		int "bitShift",
	}
	returns	"int"

native "SHIFT_RIGHT"
	hash "0x97EF1E5BCE9DC075"
	jhash (0x352633CA)
	arguments {
		int "value",

		int "bitShift",
	}
	returns	"int"

native "FLOOR"
	hash "0xF34EE736CF047844"
	jhash (0x32E9BE04)
	arguments {
		float "value",
	}
	returns	"int"

--[[!
<summary>
	I'm guessing this rounds a float value up to the next whole number, and FLOOR rounds it down
</summary>
]]--
native "CEIL"
	hash "0x11E019C8F43ACC8A"
	jhash (0xD536A1DF)
	arguments {
		float "value",
	}
	returns	"int"

native "ROUND"
	hash "0xF2DB717A73826179"
	jhash (0x323B0E24)
	arguments {
		float "value",
	}
	returns	"int"

native "TO_FLOAT"
	hash "0xBBDA792448DB5A89"
	jhash (0x67116627)
	arguments {
		int "value",
	}
	returns	"float"

native "DECOR_SET_TIME"
	hash "0x95AED7B8E39ECAA4"
	jhash (0xBBAEEF94)
	arguments {
		Entity "entity",

		charPtr "propertyName",

		int "value",
	}
	returns	"BOOL"

--[[!
<summary>
	This function sets metadata of type bool to specified entity.

</summary>
]]--
native "DECOR_SET_BOOL"
	hash "0x6B1E8E2ED1335B71"
	jhash (0x8E101F5C)
	arguments {
		Entity "entity",

		charPtr "propertyName",

		BOOL "value",
	}
	returns	"BOOL"

--[[!
<summary>
	TODO: add x360 hash
</summary>
]]--
native "_DECOR_SET_FLOAT"
	hash "0x211AB1DD8D0F363A"
	arguments {
		Entity "entity",

		charPtr "propertyName",

		float "value",
	}
	returns	"BOOL"

--[[!
<summary>
	Sets property to int.
</summary>
]]--
native "DECOR_SET_INT"
	hash "0x0CE3AA5E1CA19E10"
	jhash (0xDB718B21)
	arguments {
		Entity "entity",

		charPtr "propertyName",

		int "value",
	}
	returns	"BOOL"

native "DECOR_GET_BOOL"
	hash "0xDACE671663F2F5DB"
	jhash (0xDBCE51E0)
	arguments {
		Entity "entity",

		charPtr "propertyName",
	}
	returns	"BOOL"

--[[!
<summary>
	TODO: add x360 hash
</summary>
]]--
native "_DECOR_GET_FLOAT"
	hash "0x6524A2F114706F43"
	arguments {
		Entity "entity",

		charPtr "propertyName",
	}
	returns	"float"

native "DECOR_GET_INT"
	hash "0xA06C969B02A97298"
	jhash (0xDDDE59B5)
	arguments {
		Entity "entity",

		charPtr "propertyName",
	}
	returns	"int"

--[[!
<summary>
	Checks if property with that name exists on entity.
</summary>
]]--
native "DECOR_EXIST_ON"
	hash "0x05661B80A8C9165F"
	jhash (0x74EF9C40)
	arguments {
		Entity "entity",

		charPtr "propertyName",
	}
	returns	"BOOL"

native "DECOR_REMOVE"
	hash "0x00EE9F297C738720"
	jhash (0xE0E2640B)
	arguments {
		Entity "entity",

		charPtr "propertyName",
	}
	returns	"BOOL"

--[[!
<summary>
	Defines type of property for property name.

	1 - float,
	2 - bool,
	3 
	5 - uint? (used in scripts only once)
</summary>
]]--
native "DECOR_REGISTER"
	hash "0x9FD90732F56403CE"
	jhash (0x68BD42A9)
	arguments {
		charPtr "propertyName",

		int "type",
	}
	returns	"void"

--[[!
<summary>
	Is property of that type.

	1 
	2 
	3 
	5 - uint? (used in scripts only once)

</summary>
]]--
native "DECOR_IS_REGISTERED_AS_TYPE"
	hash "0x4F14F9F870D6FBC8"
	jhash (0x7CF0971D)
	arguments {
		charPtr "propertyName",

		int "type",
	}
	returns	"BOOL"

--[[!
<summary>
	Called after all decorator type initializations.
</summary>
]]--
native "DECOR_REGISTER_LOCK"
	hash "0xA9D14EEA259F9248"
	jhash (0x7F3F1C02)
	returns	"void"

native "0x241FCA5B1AA14F75"
	hash "0x241FCA5B1AA14F75"
	returns	"Any"

native "0x03A93FF1A2CA0864"
	hash "0x03A93FF1A2CA0864"
	jhash (0x6BE5DF29)
	returns	"Any"

native "0xBB8EA16ECBC976C4"
	hash "0xBB8EA16ECBC976C4"
	jhash (0x5ECF955D)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x93028F1DB42BFD08"
	hash "0x93028F1DB42BFD08"
	jhash (0xD1ED1D48)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x2C015348CF19CA1D"
	hash "0x2C015348CF19CA1D"
	jhash (0x19EE0CCB)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "SC_INBOX_MESSAGE_GET_DATA_INT"
	hash "0xA00EFE4082C4056E"
	jhash (0x88068C7C)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0xFFE5C16F402D851D"
	hash "0xFFE5C16F402D851D"
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "SC_INBOX_MESSAGE_GET_DATA_STRING"
	hash "0x7572EF42FC6A9B6D"
	jhash (0x15607620)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0x9A2C8064B6C1E41A"
	hash "0x9A2C8064B6C1E41A"
	jhash (0xEBE420A4)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xF3E31D16CBDCB304"
	hash "0xF3E31D16CBDCB304"
	jhash (0x2C959AF9)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xDA024BDBD600F44A"
	hash "0xDA024BDBD600F44A"
	jhash (0x0B9A3512)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xA68D3D229F4F3B06"
	hash "0xA68D3D229F4F3B06"
	jhash (0x75324674)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "SC_INBOX_MESSAGE_GET_UGCDATA"
	hash "0x69D82604A1A5A254"
	jhash (0x88CA3BFC)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x6AFD2CD753FEEF83"
	hash "0x6AFD2CD753FEEF83"
	jhash (0x628F489B)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x87E0052F08BD64E6"
	hash "0x87E0052F08BD64E6"
	jhash (0xAB3346B5)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x040ADDCBAFA1018A"
	hash "0x040ADDCBAFA1018A"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

native "0x16DA8172459434AA"
	hash "0x16DA8172459434AA"
	returns	"Any"

native "0x4737980E8A283806"
	hash "0x4737980E8A283806"
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x44ACA259D67651DB"
	hash "0x44ACA259D67651DB"
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"void"

native "SC_EMAIL_MESSAGE_PUSH_GAMER_TO_RECIP_LIST"
	hash "0x2330C12A7A605D16"
	jhash (0x9A703A2B)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0x55DF6DB45179236E"
	hash "0x55DF6DB45179236E"
	jhash (0xD094F11A)
	returns	"void"

native "0x116FB94DC4B79F17"
	hash "0x116FB94DC4B79F17"
	jhash (0xAF3C081B)
	arguments {
		AnyPtr "p0",
	}
	returns	"void"

native "0xBFA0A56A817C6C7D"
	hash "0xBFA0A56A817C6C7D"
	jhash (0x2FB9F53C)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xBC1CC91205EC8D6E"
	hash "0xBC1CC91205EC8D6E"
	jhash (0x6C5738AB)
	returns	"Any"

native "0xDF649C4E9AFDD788"
	hash "0xDF649C4E9AFDD788"
	jhash (0x468668F0)
	returns	"Any"

native "0x1F1E9682483697C7"
	hash "0x1F1E9682483697C7"
	jhash (0x90C74343)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x287F1F75D2803595"
	hash "0x287F1F75D2803595"
	jhash (0x3ACE6D6B)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x487912FD248EFDDF"
	hash "0x487912FD248EFDDF"
	jhash (0x579B4510)
	arguments {
		Any "p0",

		float "p1",
	}
	returns	"BOOL"

native "0x8416FE4E4629D7D7"
	hash "0x8416FE4E4629D7D7"
	jhash (0xDF45B2A7)
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0x75632C5ECD7ED843"
	hash "0x75632C5ECD7ED843"
	jhash (0xDF084A6B)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x1753344C770358AE"
	hash "0x1753344C770358AE"
	jhash (0xFFED3676)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x82E4A58BABC15AE7"
	hash "0x82E4A58BABC15AE7"
	jhash (0xA796D7A7)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x85535ACF97FC0969"
	hash "0x85535ACF97FC0969"
	jhash (0x09497F31)
	arguments {
		Any "p0",
	}
	returns	"Any"

--[[!
<summary>
	Unknown.

	Seems to return either 0, 1, or -1.
</summary>
]]--
native "0x930DE22F07B1CCE3"
	hash "0x930DE22F07B1CCE3"
	jhash (0x4D8A6521)
	arguments {
		Any "p0",
	}
	returns	"int"

native "0xF6BAAAF762E1BF40"
	hash "0xF6BAAAF762E1BF40"
	jhash (0x7AA36406)
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0xF22CA0FD74B80E7A"
	hash "0xF22CA0FD74B80E7A"
	jhash (0xF379DCE4)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x9237E334F6E43156"
	hash "0x9237E334F6E43156"
	jhash (0x65D84665)
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x700569DBA175A77C"
	hash "0x700569DBA175A77C"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x1D4446A62D35B0D0"
	hash "0x1D4446A62D35B0D0"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0x2E89990DDFF670C3"
	hash "0x2E89990DDFF670C3"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"Any"

native "0xD0EE05FE193646EA"
	hash "0xD0EE05FE193646EA"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0x1989C6E6F67E76A8"
	hash "0x1989C6E6F67E76A8"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0x07C61676E5BB52CD"
	hash "0x07C61676E5BB52CD"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x8147FFF6A718E1AD"
	hash "0x8147FFF6A718E1AD"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x0F73393BAC7E6730"
	hash "0x0F73393BAC7E6730"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0xD302E99EDF0449CF"
	hash "0xD302E99EDF0449CF"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0x5C4EBFFA98BDB41C"
	hash "0x5C4EBFFA98BDB41C"
	arguments {
		Any "p0",
	}
	returns	"Any"

native "0xFF8F3A92B75ED67A"
	hash "0xFF8F3A92B75ED67A"
	jhash (0xC96456BA)
	returns	"Any"

native "0x4A7D6E727F941747"
	hash "0x4A7D6E727F941747"
	jhash (0x8E7AEEB7)
	arguments {
		AnyPtr "p0",
	}
	returns	"Any"

native "0x8CC469AB4D349B7C"
	hash "0x8CC469AB4D349B7C"
	jhash (0xE778B2A7)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0x699E4A5C8C893A18"
	hash "0x699E4A5C8C893A18"
	jhash (0xCE7D50A8)
	arguments {
		Any "p0",

		AnyPtr "p1",

		AnyPtr "p2",
	}
	returns	"BOOL"

native "0x19853B5B17D77BCA"
	hash "0x19853B5B17D77BCA"
	jhash (0xD26CCA46)
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x6BFB12CE158E3DD4"
	hash "0x6BFB12CE158E3DD4"
	jhash (0x24D84334)
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0xFE4C1D0D3B9CC17E"
	hash "0xFE4C1D0D3B9CC17E"
	jhash (0x8A023024)
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0xD8122C407663B995"
	hash "0xD8122C407663B995"
	jhash (0x1C65B038)
	returns	"Any"

native "0x3001BEF2FECA3680"
	hash "0x3001BEF2FECA3680"
	jhash (0x4D4C37B3)
	returns	"BOOL"

native "0x92DA6E70EF249BD1"
	hash "0x92DA6E70EF249BD1"
	jhash (0xAED95A6F)
	arguments {
		charPtr "p0",

		intPtr "p1",
	}
	returns	"BOOL"

native "0x675721C9F644D161"
	hash "0x675721C9F644D161"
	jhash (0x486867E6)
	returns	"void"

--[[!
<summary>
	Returns the nickname of the logged-in Rockstar Social Club account.
</summary>
]]--
native "_SC_GET_NICKNAME"
	hash "0x198D161F458ECC7F"
	returns	"charPtr"

native "0x225798743970412B"
	hash "0x225798743970412B"
	arguments {
		intPtr "p0",
	}
	returns	"BOOL"

native "0x418DC16FAE452C1C"
	hash "0x418DC16FAE452C1C"
	arguments {
		int "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	Always return 0
</summary>
]]--
native "_RETURN_ZERO"
	hash "0xF2CA003F167E21D2"
	jhash (0x106C8317)
	returns	"int"

native "0xEF7D17BC6C85264C"
	hash "0xEF7D17BC6C85264C"
	jhash (0xD87F3A9E)
	returns	"BOOL"

native "_GET_BROADCAST_FINSHED_LOS_SOUND"
	hash "0xB0C56BD3D808D863"
	jhash (0xC0B971EA)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x8AA464D4E0F6ACCD"
	hash "0x8AA464D4E0F6ACCD"
	jhash (0x94BCAC7C)
	returns	"Any"

--[[!
<summary>
	Only occurrence was false, in maintransition. 
</summary>
]]--
native "0xFC309E94546FCDB5"
	hash "0xFC309E94546FCDB5"
	jhash (0x7D90EEE5)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xC6DC823253FBB366"
	hash "0xC6DC823253FBB366"
	jhash (0x734CFEDA)
	returns	"Any"

native "0xC7E7181C09F33B69"
	hash "0xC7E7181C09F33B69"
	jhash (0x8C227332)
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0xFA1E0E893D915215"
	hash "0xFA1E0E893D915215"
	jhash (0x5C350D78)
	arguments {
		BOOL "p0",
	}
	returns	"void"

--[[!
<summary>
	american = 0
	french = 1 
	german = 2
	italian =3
	spanish = 4
	portuguese = 5
	polish = 6
	russian = 7
	korean = 8
	chinese = 9
	japanese = 10
	mexican = 11
</summary>
]]--
native "_GET_UI_LANGUAGE_ID"
	hash "0x2BDD44CC428A7EAE"
	returns	"int"

--[[!
<summary>
	Returns the user's defined langauge as ID 

	english: 12
	french = 7
	german = 22
	italian = 21
	japanese = 9
	korean = 17
	portuguese = 16
	spanish = 10
	russian = 25
</summary>
]]--
native "_GET_USER_LANGUAGE_ID"
	hash "0xA8AE43AEC1A61314"
	returns	"int"

native "0x48621C9FCA3EBD28"
	hash "0x48621C9FCA3EBD28"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x81CBAE94390F9F89"
	hash "0x81CBAE94390F9F89"
	returns	"void"

native "0x13B350B8AD0EEE10"
	hash "0x13B350B8AD0EEE10"
	returns	"void"

native "0x293220DA1B46CEBC"
	hash "0x293220DA1B46CEBC"
	arguments {
		float "p0",

		float "p1",

		BOOL "p2",
	}
	returns	"void"

--[[!
<summary>
	first one seems to be a string of a mission name, second one seems to be a bool/toggle
</summary>
]]--
native "0x208784099002BC30"
	hash "0x208784099002BC30"
	arguments {
		charPtr "p0",

		Any "p1",
	}
	returns	"void"

native "0xEB2D525B57F42B40"
	hash "0xEB2D525B57F42B40"
	returns	"void"

native "0xF854439EFBB3B583"
	hash "0xF854439EFBB3B583"
	returns	"void"

native "0xAF66DCEE6609B148"
	hash "0xAF66DCEE6609B148"
	returns	"void"

native "0x66972397E0757E7A"
	hash "0x66972397E0757E7A"
	arguments {
		Any "p0",

		Any "p1",

		Any "p2",
	}
	returns	"void"

native "0xC3AC2FFF9612AC81"
	hash "0xC3AC2FFF9612AC81"
	arguments {
		Any "p0",
	}
	returns	"void"

--[[!
<summary>
	Stops both RECORDING and ACTION REPLAY
</summary>
]]--
native "_STOP_RECORDING"
	hash "0x071A5197D6AFC8B3"
	returns	"void"

native "0x88BB3507ED41A240"
	hash "0x88BB3507ED41A240"
	returns	"void"

native "0x644546EC5287471B"
	hash "0x644546EC5287471B"
	returns	"Any"

--[[!
<summary>
	Checks if you're recording, returns TRUE when you start recording (F1) or turn on action replay (F2)
</summary>
]]--
native "_IS_RECORDING"
	hash "0x1897CA71995A90B4"
	returns	"BOOL"

native "0xDF4B952F7D381B95"
	hash "0xDF4B952F7D381B95"
	returns	"Any"

native "0x4282E08174868BE3"
	hash "0x4282E08174868BE3"
	returns	"Any"

native "0x33D47E85B476ABCD"
	hash "0x33D47E85B476ABCD"
	arguments {
		BOOL "p0",
	}
	returns	"BOOL"

native "0x7E2BD3EF6C205F09"
	hash "0x7E2BD3EF6C205F09"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"void"

--[[!
<summary>
	Returns a bool if interior rendering is disabled, if yes, all "normal" rendered interiors are invisible
</summary>
]]--
native "_IS_INTERIOR_RENDERING_DISABLED"
	hash "0x95AB8B5C992C7B58"
	returns	"BOOL"

--[[!
<summary>
	Disables some other rendering (internal)
</summary>
]]--
native "0x5AD3932DAEB1E5D3"
	hash "0x5AD3932DAEB1E5D3"
	returns	"void"

native "0xE058175F8EAFE79A"
	hash "0xE058175F8EAFE79A"
	arguments {
		BOOL "p0",
	}
	returns	"void"

native "0x3353D13F09307691"
	hash "0x3353D13F09307691"
	returns	"void"

native "0x49DA8145672B2725"
	hash "0x49DA8145672B2725"
	returns	"void"

--[[!
<summary>
	bool is always true in game scripts
</summary>
]]--
native "_NETWORK_SHOP_GET_PRICE"
	hash "0xC27009422FCCA88D"
	arguments {
		Hash "hash",

		Hash "hash2",

		BOOL "p2",
	}
	returns	"int"

native "0x3C4487461E9B0DCB"
	hash "0x3C4487461E9B0DCB"
	returns	"Any"

native "0x2B949A1E6AEC8F6A"
	hash "0x2B949A1E6AEC8F6A"
	returns	"Any"

native "0x85F6C9ABA1DE2BCF"
	hash "0x85F6C9ABA1DE2BCF"
	returns	"Any"

native "0x357B152EF96C30B6"
	hash "0x357B152EF96C30B6"
	returns	"Any"

native "0xCF38DAFBB49EDE5E"
	hash "0xCF38DAFBB49EDE5E"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0xE3E5A7C64CA2C6ED"
	hash "0xE3E5A7C64CA2C6ED"
	returns	"Any"

native "0x0395CB47B022E62C"
	hash "0x0395CB47B022E62C"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0xA135AC892A58FC07"
	hash "0xA135AC892A58FC07"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x72EB7BA9B69BF6AB"
	hash "0x72EB7BA9B69BF6AB"
	returns	"Any"

native "0x170910093218C8B9"
	hash "0x170910093218C8B9"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0xC13C38E47EA5DF31"
	hash "0xC13C38E47EA5DF31"
	arguments {
		AnyPtr "p0",
	}
	returns	"BOOL"

native "0xB24F0944DA203D9E"
	hash "0xB24F0944DA203D9E"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x74A0FD0688F1EE45"
	hash "0x74A0FD0688F1EE45"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x2F41D51BA3BCD1F1"
	hash "0x2F41D51BA3BCD1F1"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x810E8431C0614BF9"
	hash "0x810E8431C0614BF9"
	returns	"Any"

native "0x35A1B3E1D1315CFA"
	hash "0x35A1B3E1D1315CFA"
	arguments {
		BOOL "p0",

		BOOL "p1",
	}
	returns	"BOOL"

native "0x897433D292B44130"
	hash "0x897433D292B44130"
	arguments {
		AnyPtr "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "_NETWORK_SHOP_BASKET_START"
	hash "0x279F08B1A4B29B7E"
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		Any "p3",
	}
	returns	"BOOL"

native "0xA65568121DF2EA26"
	hash "0xA65568121DF2EA26"
	returns	"Any"

native "0xF30980718C8ED876"
	hash "0xF30980718C8ED876"
	arguments {
		AnyPtr "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x27F76CC6C55AD30E"
	hash "0x27F76CC6C55AD30E"
	returns	"Any"

native "0xE1A0450ED46A7812"
	hash "0xE1A0450ED46A7812"
	arguments {
		Any "p0",

		AnyPtr "p1",
	}
	returns	"BOOL"

native "0x39BE7CEA8D9CC8E6"
	hash "0x39BE7CEA8D9CC8E6"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x3C5FD37B5499582E"
	hash "0x3C5FD37B5499582E"
	arguments {
		AnyPtr "p0",

		Any "p1",

		Any "p2",

		Any "p3",

		Any "p4",

		Any "p5",
	}
	returns	"BOOL"

native "0xE2A99A9B524BEFFF"
	hash "0xE2A99A9B524BEFFF"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

native "0x51F1A8E48C3D2F6D"
	hash "0x51F1A8E48C3D2F6D"
	arguments {
		Any "p0",

		BOOL "p1",

		Any "p2",
	}
	returns	"BOOL"

native "0x0A6D923DFFC9BD89"
	hash "0x0A6D923DFFC9BD89"
	returns	"Any"

native "0x112CEF1615A1139F"
	hash "0x112CEF1615A1139F"
	returns	"Any"

native "0xD47A2C1BA117471D"
	hash "0xD47A2C1BA117471D"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0xC2F7FE5309181C7D"
	hash "0xC2F7FE5309181C7D"
	arguments {
		Any "p0",

		Any "p1",
	}
	returns	"BOOL"

native "0x23789E777D14CE44"
	hash "0x23789E777D14CE44"
	returns	"Any"

native "0x350AA5EBC03D3BD2"
	hash "0x350AA5EBC03D3BD2"
	returns	"Any"

native "0x498C1E05CE5F7877"
	hash "0x498C1E05CE5F7877"
	returns	"Any"

native "0x9507D4271988E1AE"
	hash "0x9507D4271988E1AE"
	arguments {
		Any "p0",
	}
	returns	"BOOL"

--[[!
<summary>
	Returns game version e.g. '1.28'
</summary>
]]--
native "_GET_GAME_VERSION"
	hash "0xFCA9373EF340AC0A"
	returns	"charPtr"
