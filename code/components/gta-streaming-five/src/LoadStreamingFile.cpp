/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <jitasm.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <Pool.h>

#include <algorithm>
#include <array>

#include <gameSkeleton.h>

#include <boost/algorithm/string.hpp>

#include <Error.h>

#include <CoreConsole.h>

#include <IteratorView.h>
#include <ICoreGameInit.h>
#include <GameInit.h>

#include <MinHook.h>

#include <CrossBuildRuntime.h>

#include <CustomRtti.h>

#if __has_include(<StatusText.h>)
#include <StatusText.h>
#include <nutsnbolts.h>
#endif

#ifdef GTA_FIVE
static void(*dataFileMgr__loadDat)(void*, const char*, bool);
static void(*dataFileMgr__loadDefDat)(void*, const char*, bool);
#elif IS_RDR3
static void(*dataFileMgr__loadDat)(void*, const char*, bool, void*);
static void(*dataFileMgr__loadDefDat)(void*, const char*, bool, void*);
#endif

static bool IsEvaluationServer()
{
	std::string policyVal;

	if (Instance<ICoreGameInit>::Get()->GetData("policy", &policyVal))
	{
#ifndef _DEBUG
		if (policyVal.find("[local_evaluation]") != std::string::npos)
#endif
		{
			return true;
		}
	}

	return false;
}

// blocked assets part of 2545
static const std::set<std::string> g_blockedNames2372 =
{
	"astron.yft",
	"astron.ytd",
	"astron_hi.yft",
	"champion.yft",
	"champion.ytd",
	"champion_hi.yft",
	"cinquemila.yft",
	"cinquemila.ytd",
	"cinquemila_hi.yft",
	"deity.yft",
	"deity.ytd",
	"deity_hi.yft",
	"ignus.yft",
	"ignus.ytd",
	"ignus_hi.yft",
	"iwagen.yft",
	"iwagen.ytd",
	"iwagen_hi.yft",
	"jubilee.yft",
	"jubilee.ytd",
	"jubilee_hi.yft",
	"reever.yft",
	"reever.ytd",
	"reever_hi.yft",
	"shinobi.yft",
	"shinobi.ytd",
	"shinobi_hi.yft",
	"zeno.yft",
	"zeno.ytd",
	"zeno_hi.yft",
	"zeno+hi.ytd",
	"sf_mp_apa_crashed_usaf.ytyp",
	"sf_mp_apa_crashed_usaf_01a.ydr",
	"sf_mp_apa_crashed_usaf_crashed_usaf_01a.ytd",
	"sf_mp_apa_yacht.ydr",
	"sf_mp_apa_yacht.ytyp",
	"sf_mp_apa_yacht_win.ydr",
	"sf_mp_apa_yacht_door.ydr",
	"sf_mp_apa_yacht_door.ytd",
	"sf_mp_apa_yacht_door.ytyp",
	"sf_mp_apa_yacht_door2.ydr",
	"sf_mp_apa_yacht_jacuzzi.ycd",
	"sf_mp_apa_yacht_jacuzzi.ytyp",
	"sf_mp_apa_yacht_jacuzzi_camera.ydr",
	"sf_mp_apa_yacht_jacuzzi_ripple003.ydr",
	"sf_mp_apa_yacht_jacuzzi_ripple1.ydr",
	"sf_mp_apa_yacht_jacuzzi_ripple1.ytd",
	"sf_mp_apa_yacht_jacuzzi_ripple2.ydr",
	"sf_mp_apa_y1_l1a.ydr",
	"sf_mp_apa_y1_l1b.ydr",
	"sf_mp_apa_y1_l1c.ydr",
	"sf_mp_apa_y1_l1d.ydr",
	"sf_mp_apa_y1_l2a.ydr",
	"sf_mp_apa_y1_l2b.ydr",
	"sf_mp_apa_y1_l2c.ydr",
	"sf_mp_apa_y1_l2d.ydr",
	"sf_mp_apa_y2_l1a.ydr",
	"sf_mp_apa_y2_l1b.ydr",
	"sf_mp_apa_y2_l1c.ydr",
	"sf_mp_apa_y2_l1d.ydr",
	"sf_mp_apa_y2_l2a.ydr",
	"sf_mp_apa_y2_l2b.ydr",
	"sf_mp_apa_y2_l2c.ydr",
	"sf_mp_apa_y2_l2d.ydr",
	"sf_mp_apa_y3_l1a.ydr",
	"sf_mp_apa_y3_l1b.ydr",
	"sf_mp_apa_y3_l1c.ydr",
	"sf_mp_apa_y3_l1d.ydr",
	"sf_mp_apa_y3_l2a.ydr",
	"sf_mp_apa_y3_l2b.ydr",
	"sf_mp_apa_y3_l2c.ydr",
	"sf_mp_apa_y3_l2d.ydr",
	"sf_mp_apa_yacht_light.ytd",
	"sf_mp_apa_yacht_lightrig.ytyp",
	"sf_mp_apa_yacht_text.ytyp",
	"sf_prop_ap_name_text.ydr",
	"sf_prop_ap_port_text.ydr",
	"sf_prop_ap_starb_text.ydr",
	"sf_prop_ap_stern_text.ydr",
	"sf_p_sf_grass_gls_s_01a.yft",
	"sf_p_sf_grass_gls_s_02a.yft",
	"sf_prop_sf_accs_02.ytyp",
	"sf_prop_sf_apple_01a.ydr",
	"sf_prop_sf_apple_01b.ydr",
	"sf_prop_sf_bed_dog_01a.ydr",
	"sf_prop_sf_bed_dog_01a+hidr.ytd",
	"sf_prop_sf_bed_dog_01b.ydr",
	"sf_prop_sf_bed_dog_01b+hidr.ytd",
	"sf_prop_sf_bong_01a.ydr",
	"sf_prop_sf_bot_broken_01a.ydr",
	"sf_prop_sf_bowl_fruit_01a.ydr",
	"sf_prop_sf_bowl_fruit_01a.ytd",
	"sf_prop_sf_can_01a.ydr",
	"sf_prop_sf_cleaning_pad_01a.ydr",
	"sf_prop_sf_g_bong_01a.ydr",
	"sf_prop_sf_game_clock_01a.ydr",
	"sf_prop_sf_helmet_01a.ydr",
	"sf_prop_sf_npc_phone_01a.ydr",
	"sf_prop_sf_pack_can_01a.ydr",
	"sf_prop_sf_scr_m_lrg_01a.ydr",
	"sf_prop_sf_scr_m_lrg_01b.ydr",
	"sf_prop_sf_scr_m_lrg_01c.ydr",
	"sf_prop_sf_scrn_drp_01a.ydr",
	"sf_prop_sf_scrn_la_01a.ydr",
	"sf_prop_sf_scrn_la_02a.ydr",
	"sf_prop_sf_scrn_la_03a.ydr",
	"sf_prop_sf_scrn_la_04a.ydr",
	"sf_prop_sf_scrn_ppp_01a.ydr",
	"sf_prop_sf_scrn_tablet_01a.ydr",
	"sf_prop_sf_scrn_tablet_01a+hidr.ytd",
	"sf_prop_sf_scrn_tr_01a.ydr",
	"sf_prop_sf_scrn_tr_02a.ydr",
	"sf_prop_sf_scrn_tr_03a.ydr",
	"sf_prop_sf_scrn_tr_04a.ydr",
	"sf_prop_sf_sign_neon_01a.ydr",
	"sf_prop_sf_tablet_01a.ydr",
	"sf_prop_sf_tablet_01a.ytd",
	"sf_prop_sf_tv_flat_scr_01a.ydr",
	"sf_prop_sf_usb_drive_01a.ydr",
	"w_am_jerrycan_sf.ydr",
	"w_am_jerrycan_sf.ytd",
	"w_ar_heavyrifleh.ydr",
	"w_ar_heavyrifleh.ytd",
	"w_ar_heavyrifleh_hi.ydr",
	"w_ar_heavyrifleh_sight.ydr",
	"w_ar_heavyrifleh_sight.ytd",
	"w_ar_heavyrifleh_sight_hi.ydr",
	"w_at_hrh_camo1.ydr",
	"w_at_hrh_camo1.ytd",
	"w_lr_compactml.ydr",
	"w_lr_compactml.ytd",
	"w_lr_compactml_hi.ydr",
	"w_lr_compactml_mag1.ydr",
	"w_lr_compactml_mag1.ytd",
	"w_lr_ml_40mm.ydr",
	"w_lr_ml_40mm.ytd",
	"w_pi_appistol_sts.ydr",
	"w_pi_appistol_sts.ytd",
	"w_pi_appistol_sts_hi.ydr",
	"w_pi_appistol_sts_mag1.ydr",
	"w_pi_appistol_sts_mag1.ytd",
	"w_pi_appistol_sts_mag2.ydr",
	"w_pi_appistol_sts_mag2.ytd",
	"w_sb_microsmg_las.ydr",
	"w_sb_microsmg_las.ytd",
	"w_sb_microsmg_las_hi.ydr",
	"w_sb_microsmg_las_mag1.ydr",
	"w_sb_microsmg_las_mag2.ydr",
	"w_sg_pumpshotgun_chs.ydr",
	"w_sg_pumpshotgun_chs.ytd",
	"w_sg_pumpshotgun_chs_hi.ydr",
};

// blocked assets on 2189 and below
static const std::set<std::string> g_blockedNames2189 =
{
	"tr_int_placement_tr.ymap",
	"tr_int_placement_tr_interior_0_tuner_mod_garage_milo_.ymap",
	"tr_int_placement_tr_interior_1_tuner_car_meet_milo_.ymap",
	"tr_int_placement_tr_interior_2_tuner_methlab_1_milo_.ymap",
	"tr_int_placement_tr_interior_3_tuner_methlab_1_milo_.ymap",
	"tr_int_placement_tr_interior_4_tuner_methlab_1_milo_.ymap",
	"tr_int_placement_tr_interior_5_tuner_methlab_1_milo_.ymap",
	"tr_int_placement_tr_interior_6_tuner_car_meet_milo_.ymap",
	"tr_dlc_int_01_tr.ycd",
	"tr_dlc_int_01_tr.ytyp",
	"tr_int1_bedroom_empty_col_proxy.ydr",
	"tr_int1_campbed.ydr",
	"tr_int1_carbon_fibre_base.ydr",
	"tr_int1_carbon_fibre_mirror.ydr",
	"tr_int1_chalkboard.ydr",
	"tr_int1_clotheslocker.ydr",
	"tr_int1_clutter_col_proxy.ydr",
	"tr_int1_coffee_table_style2_004.ydr",
	"tr_int1_coffee_table_style2_005.ydr",
	"tr_int1_coffee_table_style2_006.ydr",
	"tr_int1_coffee_table_style2_007.ydr",
	"tr_int1_coffee_table_style2_008.ydr",
	"tr_int1_coffee_table_style2_01.ydr",
	"tr_int1_coffee_table_style2_02.ydr",
	"tr_int1_coffee_table_style2_03.ydr",
	"tr_int1_comp_barrels.ydr",
	"tr_int1_comp_barrels00dark.ydr",
	"tr_int1_comp_structure_01.ydr",
	"tr_int1_comp_structure_02.ydr",
	"tr_int1_comp_structure_03.ydr",
	"tr_int1_comp_structure_04.ydr",
	"tr_int1_comp_structure_05.ydr",
	"tr_int1_comp_structure_06.ydr",
	"tr_int1_comp_structure_07.ydr",
	"tr_int1_comp_structure_08.ydr",
	"tr_int1_comp_structure_09.ydr",
	"tr_int1_desklamp_beam_01.ydr",
	"tr_int1_drinkscabinet_002.ydr",
	"tr_int1_drinkscabinet_003.ydr",
	"tr_int1_drinkscabinet_004.ydr",
	"tr_int1_drinkscabinet_005.ydr",
	"tr_int1_drinkscabinet_006.ydr",
	"tr_int1_drinkscabinet_007.ydr",
	"tr_int1_drinkscabinet_008.ydr",
	"tr_int1_drinkscabinet_009.ydr",
	"tr_int1_drinkscabinet_1.ydr",
	"tr_int1_emblem_tarp_1.ydr",
	"tr_int1_emblem_tarp_2.ydr",
	"tr_int1_gunlocker.ydr",
	"tr_int1_highlights_proxy001.ydr",
	"tr_int1_lightamericana_proxy001.ydr",
	"tr_int1_lightcapgamer_proxy001.ydr",
	"tr_int1_lightcap_proxy001.ydr",
	"tr_int1_lightcorona_proxy001.ydr",
	"tr_int1_lightjap_proxy001.ydr",
	"tr_int1_lightled_proxy001.ydr",
	"tr_int1_lightsprayroom_proxy.ydr",
	"tr_int1_light_bedroomproxy.ydr",
	"tr_int1_light_hooks.ydr",
	"tr_int1_light_proxy.ydr",
	"tr_int1_mod_armchair_009.ydr",
	"tr_int1_mod_armchair_05.ydr",
	"tr_int1_mod_banners005.ydr",
	"tr_int1_mod_banners007.ydr",
	"tr_int1_mod_banners008.ydr",
	"tr_int1_mod_banners009.ydr",
	"tr_int1_mod_banners010.ydr",
	"tr_int1_mod_banners1.ydr",
	"tr_int1_mod_barnachair_003.ydr",
	"tr_int1_mod_barnachair_004.ydr",
	"tr_int1_mod_barnachair_005.ydr",
	"tr_int1_mod_barnachair_006.ydr",
	"tr_int1_mod_barnachair_2.ydr",
	"tr_int1_mod_beams1.ydr",
	"tr_int1_mod_cabinet.ydr",
	"tr_int1_mod_carlift.ydr",
	"tr_int1_mod_cctv_table.ydr",
	"tr_int1_mod_ceillinglights_006.ydr",
	"tr_int1_mod_ceillinglights_07.ydr",
	"tr_int1_mod_ceillinglights_9.ydr",
	"tr_int1_mod_decals_01.ydr",
	"tr_int1_mod_dirt.ydr",
	"tr_int1_mod_dirtb.ydr",
	"tr_int1_mod_elec_01.ydr",
	"tr_int1_mod_elec_02.ydr",
	"tr_int1_mod_framework.ydr",
	"tr_int1_mod_hood.ydr",
	"tr_int1_mod_hood001.ydr",
	"tr_int1_mod_int_col_proxy.ydr",
	"tr_int1_mod_int_det_style_2.ydr",
	"tr_int1_mod_int_grind_col_proxy.ydr",
	"tr_int1_mod_int_ledstrip_ref.ydr",
	"tr_int1_mod_int_ledstrip_ref002.ydr",
	"tr_int1_mod_int_neonreflection001.ydr",
	"tr_int1_mod_int_shell.ydr",
	"tr_int1_mod_int_style_2.ydr",
	"tr_int1_mod_int_style_3.ydr",
	"tr_int1_mod_int_style_4.ydr",
	"tr_int1_mod_int_style_5.ydr",
	"tr_int1_mod_int_style_6.ydr",
	"tr_int1_mod_int_style_7.ydr",
	"tr_int1_mod_int_style_8.ydr",
	"tr_int1_mod_int_style_9.ydr",
	"tr_int1_mod_int_tool_col_proxy.ydr",
	"tr_int1_mod_lamps.ydr",
	"tr_int1_mod_lamps_source_on.ydr",
	"tr_int1_mod_lframe_01a_proxy.ydr",
	"tr_int1_mod_lights4_01.ydr",
	"tr_int1_mod_lights_008.ydr",
	"tr_int1_mod_lights_009.ydr",
	"tr_int1_mod_lights_1.ydr",
	"tr_int1_mod_lights_2.ydr",
	"tr_int1_mod_lights_3.ydr",
	"tr_int1_mod_mezzanine_style1.ydr",
	"tr_int1_mod_mezzanine_style2.ydr",
	"tr_int1_mod_mezzanine_style3.ydr",
	"tr_int1_mod_mezzanine_style4.ydr",
	"tr_int1_mod_mezzanine_style5.ydr",
	"tr_int1_mod_mezzanine_style6.ydr",
	"tr_int1_mod_mezzanine_style7.ydr",
	"tr_int1_mod_mezzanine_style8.ydr",
	"tr_int1_mod_mezzanine_style9.ydr",
	"tr_int1_mod_mirror_04.ydr",
	"tr_int1_mod_mirror_05.ydr",
	"tr_int1_mod_mirror_07.ydr",
	"tr_int1_mod_murals_09.ydr",
	"tr_int1_mod_mural_neon.ydr",
	"tr_int1_mod_neontubes_blue.ydr",
	"tr_int1_mod_neontubes_green02.ydr",
	"tr_int1_mod_office_01.ydr",
	"tr_int1_mod_office_table_01.ydr",
	"tr_int1_mod_pillars01.ydr",
	"tr_int1_mod_pillars010.ydr",
	"tr_int1_mod_pillars02.ydr",
	"tr_int1_mod_pillars03.ydr",
	"tr_int1_mod_pillars04.ydr",
	"tr_int1_mod_pillars05.ydr",
	"tr_int1_mod_pillars06.ydr",
	"tr_int1_mod_pillars08.ydr",
	"tr_int1_mod_pillars09.ydr",
	"tr_int1_mod_posters09.ydr",
	"tr_int1_mod_recessed_light003.ydr",
	"tr_int1_mod_reffloor_1.ydr",
	"tr_int1_mod_reffloor_2.ydr",
	"tr_int1_mod_reffloor_3.ydr",
	"tr_int1_mod_sofa_003.ydr",
	"tr_int1_mod_sofa_009.ydr",
	"tr_int1_mod_sofa_010.ydr",
	"tr_int1_mod_sofa_011.ydr",
	"tr_int1_mod_sofa_012.ydr",
	"tr_int1_mod_sofa_2.ydr",
	"tr_int1_mod_sofa_8.ydr",
	"tr_int1_mod_spray004.ydr",
	"tr_int1_mod_spray008.ydr",
	"tr_int1_mod_spray009.ydr",
	"tr_int1_mod_spray01.ydr",
	"tr_int1_mod_spray010.ydr",
	"tr_int1_mod_spray02.ydr",
	"tr_int1_mod_spray03.ydr",
	"tr_int1_mod_spray05.ydr",
	"tr_int1_mod_spray06.ydr",
	"tr_int1_mod_style05_posters.ydr",
	"tr_int1_mod_table.ydr",
	"tr_int1_mod_vinyl_05.ydr",
	"tr_int1_mod_window_01.ydr",
	"tr_int1_mod_window_02.ydr",
	"tr_int1_mod_window_03.ydr",
	"tr_int1_office_drawers.ydr",
	"tr_int1_plan_cube.ydr",
	"tr_int1_plan_table008.ydr",
	"tr_int1_plan_table009.ydr",
	"tr_int1_plan_table01.ydr",
	"tr_int1_plan_table010.ydr",
	"tr_int1_plan_table02.ydr",
	"tr_int1_plan_table03.ydr",
	"tr_int1_plan_table05.ydr",
	"tr_int1_play_text.ydr",
	"tr_int1_roller_door_ref_proxy.ydr",
	"tr_int1_sideboard_style2_003.ydr",
	"tr_int1_sideboard_style2_004.ydr",
	"tr_int1_sideboard_style2_005.ydr",
	"tr_int1_sideboard_style2_006.ydr",
	"tr_int1_sideboard_style2_01.ydr",
	"tr_int1_sideboard_style2_010.ydr",
	"tr_int1_sideboard_style2_011.ydr",
	"tr_int1_sideboard_style2_012.ydr",
	"tr_int1_sideboard_style2_013.ydr",
	"tr_int1_sideboard_style2_014.ydr",
	"tr_int1_sideboard_style2_015.ydr",
	"tr_int1_sideboard_style2_017.ydr",
	"tr_int1_sideboard_style2_018.ydr",
	"tr_int1_sideboard_style2_019.ydr",
	"tr_int1_sideboard_style2_02.ydr",
	"tr_int1_smodd_cm_heatlamp_001.ydr",
	"tr_int1_smodd_cm_weldmachine_001.ydr",
	"tr_int1_smodd_cor_hose_001.ydr",
	"tr_int1_smodd_cs_jerrycan01_001.ydr",
	"tr_int1_smod_barrel_01a_001.ydr",
	"tr_int1_smod_carcreeper_001.ydr",
	"tr_int1_smod_carjack_01.ydr",
	"tr_int1_smod_compressor_03.ydr",
	"tr_int1_smod_engine_hoist_001.ydr",
	"tr_int1_smod_oilcan_01a_001.ydr",
	"tr_int1_smod_sacktruck_02a_001.ydr",
	"tr_int1_smod_toolchest9.ydr",
	"tr_int1_smod_toolchest_02_001.ydr",
	"tr_int1_smod_toolchest_05_001.ydr",
	"tr_int1_smoking_table.ydr",
	"tr_int1_smoking_table008.ydr",
	"tr_int1_smoking_table009.ydr",
	"tr_int1_smoking_table009x.ydr",
	"tr_int1_smoking_table009x001.ydr",
	"tr_int1_smoking_table009x002.ydr",
	"tr_int1_smoking_table010.ydr",
	"tr_int1_smoking_table2.ydr",
	"tr_int1_style_8_decals.ydr",
	"tr_int1_tool_draw_01d.ydr",
	"tr_int1_tool_draw_01d001.ydr",
	"tr_int1_tool_draw_01d002.ydr",
	"tr_int1_tool_draw_01d003.ydr",
	"tr_int1_tool_draw_01d004.ydr",
	"tr_int1_tool_draw_01d005.ydr",
	"tr_int1_tool_draw_01d006.ydr",
	"tr_int1_tool_draw_01d007.ydr",
	"tr_int1_tool_draw_01e003.ydr",
	"tr_int1_tool_draw_01e004.ydr",
	"tr_int1_tool_draw_01e005.ydr",
	"tr_int1_tool_draw_01e006.ydr",
	"tr_int1_tool_draw_01e007.ydr",
	"tr_int1_tool_draw_01e008.ydr",
	"tr_int1_tuner_mod_garage.ytd",
	"tr_int1_tuner_posters.ydr",
	"tr_int1_tyre_marks.ydr",
	"tr_int1_vend_skin_2.ydr",
	"tr_int1_vend_skin_3.ydr",
	"tr_int1_vend_skin_4.ydr",
	"tr_int1_vend_skin_7.ydr",
	"tr_int1_vend_skin_8.ydr",
	"tr_int1_v_45_racks.ydr",
	"tr_int1_v_res_fh_coftableb.ydr",
	"tr_int1_v_res_fh_coftableb001.ydr",
	"tr_tuner_mod_garage.ybn",
	"tr_dlc_int_04_tr.ytyp",
	"tr_int4_blends.ydr",
	"tr_int4_conduit.ydr",
	"tr_int4_details.ydr",
	"tr_int4_door.ydr",
	"tr_int4_hiddenshell.ydr",
	"tr_int4_methkit_basic.ydr",
	"tr_int4_methkit_bas_decals.ydr",
	"tr_int4_methkit_lightproxy.ydr",
	"tr_int4_methkit_set_decals.ydr",
	"tr_int4_methkit_set_details.ydr",
	"tr_int4_misc_details.ydr",
	"tr_int4_racks.ydr",
	"tr_int4_shell.ydr",
	"tr_int4_sidewindd.ydr",
	"tr_int4_structure_cs.ydr",
	"tr_int4_structure_ns.ydr",
	"tr_int4_tuner_methlab_1_txd.ytd",
	"tr_tuner_methlab_1.ybn",
	"tr_dlc_int_02_tr.ytyp",
	"tr_int2_angled_kerbs.ydr",
	"tr_int2_blends_meet.ydr",
	"tr_int2_bulks.ydr",
	"tr_int2_cables.ydr",
	"tr_int2_cables_003.ydr",
	"tr_int2_cables_2.ydr",
	"tr_int2_cable_trays.ydr",
	"tr_int2_caps.ydr",
	"tr_int2_carwarecareware_skidders.ydr",
	"tr_int2_carwareconc_decals_basic.ydr",
	"tr_int2_carware_brands_decals.ydr",
	"tr_int2_carware_fldecals_urban.ydr",
	"tr_int2_ceiling.ydr",
	"tr_int2_ceiling_decals.ydr",
	"tr_int2_ceiling_decs.ydr",
	"tr_int2_ceiling_fan.ydr",
	"tr_int2_ceilng_vents.ydr",
	"tr_int2_chainlinkfence.ydr",
	"tr_int2_chimney.ydr",
	"tr_int2_chimney_02.ydr",
	"tr_int2_chimney_03.ydr",
	"tr_int2_chimney_04.ydr",
	"tr_int2_chimney_05.ydr",
	"tr_int2_chimney_06.ydr",
	"tr_int2_chimney_07.ydr",
	"tr_int2_chimney_08.ydr",
	"tr_int2_clothes_boxes.ydr",
	"tr_int2_conc_bases_tuns.ydr",
	"tr_int2_crane.ydr",
	"tr_int2_crane_02.ydr",
	"tr_int2_crane_03.ydr",
	"tr_int2_crane_04.ydr",
	"tr_int2_debris.ydr",
	"tr_int2_debris_decals.ydr",
	"tr_int2_decal_test.ydr",
	"tr_int2_details_02.ydr",
	"tr_int2_details_04.ydr",
	"tr_int2_detail_shell.ydr",
	"tr_int2_donuts002.ydr",
	"tr_int2_donuts003.ydr",
	"tr_int2_donuts004.ydr",
	"tr_int2_donuts005.ydr",
	"tr_int2_donuts006.ydr",
	"tr_int2_donuts1.ydr",
	"tr_int2_drains.ydr",
	"tr_int2_ducting.ydr",
	"tr_int2_ducting_02.ydr",
	"tr_int2_ducting_03.ydr",
	"tr_int2_ducting_04.ydr",
	"tr_int2_ducting_05.ydr",
	"tr_int2_ducting_06.ydr",
	"tr_int2_ducting_meet.ydr",
	"tr_int2_ducting_view_01.ydr",
	"tr_int2_ducting_view_02.ydr",
	"tr_int2_exit_signs.ydr",
	"tr_int2_exit_signs001.ydr",
	"tr_int2_exit_signs002.ydr",
	"tr_int2_fluoro_ceiling_sandbox.ydr",
	"tr_int2_fluoro_ref_only_mesh.ydr",
	"tr_int2_gas_pipes.ydr",
	"tr_int2_hoarding.ydr",
	"tr_int2_insulation.ydr",
	"tr_int2_kerbs.ydr",
	"tr_int2_large_duct.ydr",
	"tr_int2_large_duct_02.ydr",
	"tr_int2_large_duct_03.ydr",
	"tr_int2_large_duct_04.ydr",
	"tr_int2_large_duct_05.ydr",
	"tr_int2_large_duct_06.ydr",
	"tr_int2_light_proxy_main_fancy.ydr",
	"tr_int2_light_proxy_meet_cheap.ydr",
	"tr_int2_light_proxy_meet_fancy.ydr",
	"tr_int2_light_prox_mn_cheap.ydr",
	"tr_int2_lintels.ydr",
	"tr_int2_lps_wall_lamp.ydr",
	"tr_int2_main_gates.ydr",
	"tr_int2_meetcables.ydr",
	"tr_int2_meets_blends.ydr",
	"tr_int2_meet_collision_proxy.ydr",
	"tr_int2_meet_cracks.ydr",
	"tr_int2_meet_dbris.ydr",
	"tr_int2_meet_drains.ydr",
	"tr_int2_meet_dubs.ydr",
	"tr_int2_meet_pillars.ydr",
	"tr_int2_meet_pipe.ydr",
	"tr_int2_metal_beam.ydr",
	"tr_int2_metal_beam_02.ydr",
	"tr_int2_metal_beam_03.ydr",
	"tr_int2_metal_beam_04.ydr",
	"tr_int2_metal_debris.ydr",
	"tr_int2_metal_support.ydr",
	"tr_int2_metal_wall.ydr",
	"tr_int2_new_hut.ydr",
	"tr_int2_outer_lines.ydr",
	"tr_int2_plaster_chips_decal.ydr",
	"tr_int2_prop_tr_light_ceiling_01a.ydr",
	"tr_int2_prop_tr_serv_tu_light047.ydr",
	"tr_int2_puddles.ydr",
	"tr_int2_railing.ydr",
	"tr_int2_rails.ydr",
	"tr_int2_rails_new.ydr",
	"tr_int2_rebar_decals.ydr",
	"tr_int2_round_column.ydr",
	"tr_int2_rusty_pipes.ydr",
	"tr_int2_rusty_pipes_02.ydr",
	"tr_int2_rusty_pipes_03.ydr",
	"tr_int2_rusty_pipes_04.ydr",
	"tr_int2_rusty_pipes_05.ydr",
	"tr_int2_rusty_pipes_06.ydr",
	"tr_int2_rusty_pipes_07.ydr",
	"tr_int2_rusty_pipes_08.ydr",
	"tr_int2_rusty_pipes_10.ydr",
	"tr_int2_sandbox_barrier.ydr",
	"tr_int2_sandbox_collision_proxy.ydr",
	"tr_int2_sandbox_signage.ydr",
	"tr_int2_sb_structure.ydr",
	"tr_int2_scores.ydr",
	"tr_int2_scuff_decals.ydr",
	"tr_int2_shell.ydr",
	"tr_int2_shell_blends.ydr",
	"tr_int2_skidders.ydr",
	"tr_int2_sliding_door_003.ydr",
	"tr_int2_sliding_door_004.ydr",
	"tr_int2_sliding_door_01.ydr",
	"tr_int2_sliding_door_02.ydr",
	"tr_int2_start_spot.ydr",
	"tr_int2_tats_n_sht.ydr",
	"tr_int2_track_lines.ydr",
	"tr_int2_tuner_car_meet_txd.ytd",
	"tr_int2_turn_marks.ydr",
	"tr_int2_view_rm1_decals.ydr",
	"tr_int2_view_rm1_details.ydr",
	"tr_int2_view_rm2_decals.ydr",
	"tr_int2_wee_stanes.ydr",
	"tr_tuner_car_meet.ybn",
	"tr_dlc_int_04_tr.ytyp",
	"tr_int4_blends.ydr",
	"tr_int4_conduit.ydr",
	"tr_int4_details.ydr",
	"tr_int4_door.ydr",
	"tr_int4_hiddenshell.ydr",
	"tr_int4_methkit_basic.ydr",
	"tr_int4_methkit_bas_decals.ydr",
	"tr_int4_methkit_lightproxy.ydr",
	"tr_int4_methkit_set_decals.ydr",
	"tr_int4_methkit_set_details.ydr",
	"tr_int4_misc_details.ydr",
	"tr_int4_racks.ydr",
	"tr_int4_shell.ydr",
	"tr_int4_sidewindd.ydr",
	"tr_int4_structure_cs.ydr",
	"tr_int4_structure_ns.ydr",
	"tr_int4_tuner_methlab_1_txd.ytd",
	"tr_tuner_methlab_1.ybn",
	"calico+hi.ytd",
	"calico.yft",
	"calico.ytd",
	"calico_hi.yft",
	"comet6+hi.ytd",
	"comet6.yft",
	"comet6.ytd",
	"comet6_hi.yft",
	"cypher+hi.ytd",
	"cypher.yft",
	"cypher.ytd",
	"cypher_hi.yft",
	"dominator7+hi.ytd",
	"dominator7.yft",
	"dominator7.ytd",
	"dominator7_hi.yft",
	"dominator8.yft",
	"dominator8.ytd",
	"dominator8_hi.yft",
	"euros.yft",
	"euros.ytd",
	"euros_hi.yft",
	"freightcar2.yft",
	"freightcar2.ytd",
	"freightcar2_hi.yft",
	"futo2+hi.ytd",
	"futo2.yft",
	"futo2.ytd",
	"futo2_hi.yft",
	"growler+hi.ytd",
	"growler.yft",
	"growler.ytd",
	"growler_hi.yft",
	"jester4+hi.ytd",
	"jester4.yft",
	"jester4.ytd",
	"jester4_hi.yft",
	"previon.yft",
	"previon.ytd",
	"previon_hi.yft",
	"remus+hi.ytd",
	"remus.yft",
	"remus.ytd",
	"remus_hi.yft",
	"rt3000+hi.ytd",
	"rt3000.yft",
	"rt3000.ytd",
	"rt3000_hi.yft",
	"sultan3+hi.ytd",
	"sultan3.yft",
	"sultan3.ytd",
	"sultan3_hi.yft",
	"tailgater2+hi.ytd",
	"tailgater2.yft",
	"tailgater2.ytd",
	"tailgater2_hi.yft",
	"va_rt3000.ycd",
	"vectre.yft",
	"vectre.ytd",
	"vectre_hi.yft",
	"vehicles_coquette_w1_interior.ytd",
	"vehicles_coquette_w2_interior.ytd",
	"vehicles_fmj_w_interior.ytd",
	"vehicles_fmj_w_race_interior.ytd",
	"vehicles_futo_race_interior.ytd",
	"vehicles_jugular_race_interior.ytd",
	"vehicles_jugular_w_interior.ytd",
	"vehicles_monroe_race_interior.ytd",
	"vehicles_muscle_o_race_interior.ytd",
	"vehicles_pfister_interior.ytd",
	"vehicles_pfister_race_interior.ytd",
	"vehicles_supergt_w1_interior.ytd",
	"vehicles_supergt_w2_interior.ytd",
	"warrener2.yft",
	"warrener2.ytd",
	"warrener2_hi.yft",
	"zr350.yft",
	"zr350.ytd",
	"zr350_hi.yft",
};

static std::vector<std::string> g_beforeLevelMetas;
static std::vector<std::string> g_afterLevelMetas;

static std::vector<std::string> g_defaultMetas;

static std::vector<std::string> g_gtxdFiles;
static std::vector<std::pair<std::string, std::string>> g_dataFiles;
static std::vector<std::pair<std::string, std::string>> g_loadedDataFiles;

class CDataFileMgr
{
public:
	struct DataFile
	{
		char name[128];
		char pad[16]; // 128
		int32_t type; // 140
		int32_t index; // 148
		bool locked; // 152
		bool flag2; // 153
		bool flag3; // 154
		bool disabled; // 155
		bool persistent; // 156
		bool overlay;
		char pad2[10];
	};
};

namespace DataFileType
{
static int DLC_ITYP_REQUEST;
static int DLC_POP_GROUPS;
static int DLC_WEAPON_PICKUPS;
}

static void* g_dataFileMgr;

#ifdef GTA_FIVE
static void LoadDats(void* dataFileMgr, const char* name, bool enabled)
{
	// load before-level metas
	for (const auto& meta : g_beforeLevelMetas)
	{
		dataFileMgr__loadDat(dataFileMgr, meta.c_str(), enabled);
	}

	// load the level
	dataFileMgr__loadDat(dataFileMgr, name, enabled);

	// load after-level metas
	for (const auto& meta : g_afterLevelMetas)
	{
		dataFileMgr__loadDat(dataFileMgr, meta.c_str(), enabled);
	}
}

static void LoadDefDats(void* dataFileMgr, const char* name, bool enabled)
{
	g_dataFileMgr = dataFileMgr;

	// load before-level metas
	trace("Loading content XML: %s\n", name);

	// load the level
	dataFileMgr__loadDefDat(dataFileMgr, name, enabled);
}

#elif IS_RDR3
static void LoadDats(void* dataFileMgr, const char* name, bool enabled, void* unk)
{
	// load before-level metas
	for (const auto& meta : g_beforeLevelMetas)
	{
		dataFileMgr__loadDat(dataFileMgr, meta.c_str(), enabled, unk);
	}

	// load the level
	dataFileMgr__loadDat(dataFileMgr, name, enabled, unk);

	// load after-level metas
	for (const auto& meta : g_afterLevelMetas)
	{
		dataFileMgr__loadDat(dataFileMgr, meta.c_str(), enabled, unk);
	}
}

static void LoadDefDats(void* dataFileMgr, const char* name, bool enabled, void* unk)
{
	g_dataFileMgr = dataFileMgr;

	// load before-level metas
	trace("Loading content XML: %s\n", name);

	// load the level
	dataFileMgr__loadDefDat(dataFileMgr, name, enabled, unk);
}
#endif

static std::vector<std::string> g_oldEntryList;

static int SehRoutine(const char* whatPtr, PEXCEPTION_POINTERS exception)
{
	if (IsErrorException(exception))
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	if (exception->ExceptionRecord->ExceptionCode & 0x80000000)
	{
		if (!whatPtr)
		{
			whatPtr = "a safe-call operation";
		}

		FatalErrorNoExcept("An exception occurred (%08x at %p) during %s. The game will be terminated.",
			exception->ExceptionRecord->ExceptionCode, exception->ExceptionRecord->ExceptionAddress,
			whatPtr);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

#include <sysAllocator.h>
#include <atArray.h>

template<typename T>
static auto SafeCall(const T& fn, const char* whatPtr = nullptr)
{
#ifndef _DEBUG
	__try
	{
#endif
		return fn();
#ifndef _DEBUG
	}
	__except (SehRoutine(whatPtr, GetExceptionInformation()))
	{
		return std::result_of_t<T()>();
	}
#endif
}

#ifdef GTA_FIVE
struct EnumEntry
{
	uint32_t hash;
	uint32_t index;
};
#elif IS_RDR3
struct EnumEntry
{
	uint32_t hash;
	char pad1[4];
	uint32_t index;
	char pad2[4];
};
#endif

static EnumEntry* g_dataFileTypes;

static int LookupDataFileType(const std::string& type)
{
	uint32_t thisHash = HashRageString(boost::to_upper_copy(type).c_str());

	for (EnumEntry* i = g_dataFileTypes; (i->hash != 0) || (i->index != 0xFFFFFFFF); ++i)
	{
		if (i->hash == thisHash)
		{
			return i->index;
		}
	}

	return -1;
}

class CDataFileMountInterface
{
public:
	virtual ~CDataFileMountInterface() = default;

	virtual bool LoadDataFile(CDataFileMgr::DataFile* entry) = 0;

	virtual void UnloadDataFile(CDataFileMgr::DataFile* entry) = 0;
};

class CfxPackfileMounter : public CDataFileMountInterface
{
public:
	virtual bool LoadDataFile(CDataFileMgr::DataFile* entry) override;

	virtual void UnloadDataFile(CDataFileMgr::DataFile* entry) override;
};

static hook::cdecl_stub<void(CDataFileMgr::DataFile* entry)> _addPackfile([]()
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("EB 15 48 8B 0B 40 38 7B 0C 74 07 E8", 11));
#elif IS_RDR3
	return hook::get_call(hook::get_pattern("48 8B 0B 40 38 7B ? 74 ? E8 ? ? ? ? EB", 9));
#endif
});

static hook::cdecl_stub<void(CDataFileMgr::DataFile* entry)> _removePackfile([]()
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("EB 15 48 8B 0B 40 38 7B 0C 74 07 E8", 18));
#elif IS_RDR3
	return hook::get_call(hook::get_pattern("48 8B 0B 40 38 7B ? 74 ? E8 ? ? ? ? EB", 16));
#endif
});

static hook::cdecl_stub<void(void*)> _initManifestChunk([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("48 8D 4F 10 B2 01 48 89 2F", -0x2E);
#elif IS_RDR3
	return hook::get_pattern("75 ? 48 8B 09 E8 ? ? ? ? 48 8D 4B 10 48", -22);
#endif
});

static hook::cdecl_stub<void(void*)> _loadManifestChunk([]()
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("45 38 AE C0 00 00 00 0F 95 C3 E8", -5));
#elif IS_RDR3
	return hook::get_call(hook::get_pattern("48 8D 95 B8 02 00 00 48 8D 0D", 20));
#endif
});

static hook::cdecl_stub<void(void*)> _clearManifestChunk([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("33 FF 48 8D 4B 10 B2 01", -0x15);
#elif IS_RDR3
	return hook::get_call(hook::get_pattern("33 C9 E8 ? ? ? ? 48 8D 55 10 89 45 10 48", 33));
#endif
});

static void* manifestChunkPtr;

bool CfxPackfileMounter::LoadDataFile(CDataFileMgr::DataFile* entry)
{
	entry->disabled = true;
	//entry->persistent = true;
	//entry->locked = true;
	//entry->overlay = true;
	_initManifestChunk(manifestChunkPtr);
	_addPackfile(entry);
	_loadManifestChunk(manifestChunkPtr);
	_clearManifestChunk(manifestChunkPtr);
	return true;
}

void CfxPackfileMounter::UnloadDataFile(CDataFileMgr::DataFile* entry)
{
	_removePackfile(entry);
}

static void** g_extraContentManager;

#ifdef GTA_FIVE
static void(*g_disableContentGroup)(void*, uint32_t);
static void(*g_enableContentGroup)(void*, uint32_t);
static void(*g_clearContentCache)(int);
#elif IS_RDR3
static void(*g_disableContentGroup)(void*, const uint32_t&);
static void(*g_enableContentGroup)(void*, const uint32_t&);
#endif

static CfxPackfileMounter g_staticRpfMounter;

#include <Streaming.h>

void ForAllStreamingFiles(const std::function<void(const std::string&)>& cb);

static CDataFileMountInterface* LookupDataFileMounter(const std::string& type);

static hook::cdecl_stub<bool(void* streaming, int idx)> _isResourceNotCached([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("74 07 8A 40 48 24 01 EB 02 B0 01", -0x1B);
#elif IS_RDR3
	return hook::get_pattern("74 07 8A 40 76 24 01 EB 02 B0 01", -0x1B);
#endif
});

#ifdef IS_RDR3
static hook::cdecl_stub<void()> _initFuncCoverPointManagerSessionReload([]()
{
	return hook::get_pattern("74 1C 80 B8 72 05 00 00 00 74", -14);
});
#endif

static bool g_reloadMapStore = false;

static std::set<std::string> loadedCollisions;

int GetDummyCollectionIndexByTag(const std::string& tag);
extern std::unordered_map<int, std::string> g_handlesToTag;

fwEvent<> OnReloadMapStore;

#ifdef GTA_FIVE
extern uint32_t GetCurrentMapGroup();

static hook::cdecl_stub<void()> _reloadMapIfNeeded([]()
{
	return hook::get_pattern("74 1F 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? C6 05", -0xB);
});

static char* loadChangeSet = nullptr;
static void ReloadMapStoreNative()
{
	uint8_t origCode[0x4F3];
	memcpy(origCode, loadChangeSet, sizeof(origCode));

	// nop a call before the r13d load
	hook::nop(loadChangeSet + 0x28, 5);

	// jump straight into the right block
	hook::put<uint8_t>(loadChangeSet + 0x41, 0xE9);
	hook::put<int32_t>(loadChangeSet + 0x42, 0x116);

	// don't load CS
	hook::nop(loadChangeSet + 0x300, 5);

	// don't use cache state
	hook::nop(loadChangeSet + 0x356, 10);
	hook::put<uint16_t>(loadChangeSet + 0x356, 0x00B3);

	// ignore staticboundsstore too (crashes in release, thanks stack)
	hook::nop(loadChangeSet + 0x434, 5);

	// and the array fill/clear for this fake array
	hook::nop(loadChangeSet + 0x395, 5);
	hook::nop(loadChangeSet + 0x489, 5);

	// ignore trailer
	hook::nop(loadChangeSet + 0x4A3, 54);

	// call
	uint32_t hash = 0xDEADBDEF;
	uint8_t csBuf[512] = { 0 };
	uint8_t unkBuf[512] = { 0 };
	((void (*)(void*, void*, void*))loadChangeSet)(csBuf, unkBuf, &hash);

	DWORD oldProtect;
	VirtualProtect(loadChangeSet, sizeof(origCode), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(loadChangeSet, origCode, sizeof(origCode));
	VirtualProtect(loadChangeSet, sizeof(origCode), oldProtect, &oldProtect);

	// reload map stuff
	_reloadMapIfNeeded();
}
#endif

static void ReloadMapStore()
{
	// filename, streamingIndex
	std::vector<std::pair<std::string, uint32_t>> collisionFiles;

	if (!g_reloadMapStore)
	{
		return;
	}

	auto mgr = streaming::Manager::GetInstance();


	// Find collision files that need reloading
	ForAllStreamingFiles([&](const std::string& file)
	{
		if (file.find(".ybn") == std::string::npos)
		{
			return;
		}
		if (loadedCollisions.find(file) != loadedCollisions.end())
		{
			return;
		}

		auto obj = streaming::GetStreamingIndexForName(file);

		if (obj == 0)
		{
			return;
		}

		auto relId = obj - streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ybn")->baseIdx;

		if (
			_isResourceNotCached(mgr, obj)
#ifdef GTA_FIVE
			|| GetDummyCollectionIndexByTag(g_handlesToTag[mgr->Entries[obj].handle]) == -1
#endif
		   )
		{
			// currently, Reloading custom collision causes it to break in RDR3
#ifndef IS_RDR3
			collisionFiles.push_back(std::make_pair(file, obj));
#endif
		}
		else
		{
			trace("Skipped %s - it's cached! (id %d)\n", file.c_str(), relId);
		}
	});


	static constexpr uint8_t batchSize = 4;

	for (int count = 0; count < collisionFiles.size(); count += batchSize)
	{
		int end = std::min((count + batchSize), (int)collisionFiles.size());

		for (int i = count; i < end; i++)
		{
			mgr->RequestObject(collisionFiles[i].second, 0);
			trace("Loaded %s (id %d)\n", collisionFiles[i].first.c_str(), (collisionFiles[i].second - mgr->moduleMgr.GetStreamingModule("ybn")->baseIdx));
		}

#if __has_include(<StatusText.h>)
		ActivateStatusText(va("Preloading collisions (%.0f%%)", (count / (double)collisionFiles.size()) * 100.0), 5, 2);
#endif

		streaming::LoadObjectsNow(0);

		for (int i = count; i < end; i++)
		{
			mgr->ReleaseObject(collisionFiles[i].second);
		}
	}

#if __has_include(<StatusText.h>)
	DeactivateStatusText(2);
#endif

	OnReloadMapStore();

#ifdef GTA_FIVE
	// needs verification for newer builds
	if (!xbr::IsGameBuildOrGreater<xbr::Build::Latest + 1>())
	{
		ReloadMapStoreNative();
	}
	else
#endif
	{
		uint32_t mapGroup =
#ifdef GTA_FIVE
		GetCurrentMapGroup()
#else
		HashString("GROUP_MAP")
#endif
		;

		// workaround by unloading/reloading MP map group
		g_disableContentGroup(*g_extraContentManager, mapGroup); // GROUP_MAP

		// again for enablement
		OnReloadMapStore();

		g_enableContentGroup(*g_extraContentManager, mapGroup);
	}

#ifdef GTA_FIVE
	g_clearContentCache(0);
#elif IS_RDR3
	_initFuncCoverPointManagerSessionReload();
#endif

	loadedCollisions.clear();

#ifdef GTA_FIVE
	// load gtxd files
	for (auto& file : g_gtxdFiles)
	{
		auto mounter = LookupDataFileMounter("GTXD_PARENTING_DATA");

		CDataFileMgr::DataFile ventry;
		memset(&ventry, 0, sizeof(ventry));
		strcpy(ventry.name, file.c_str()); // muahaha
		ventry.type = LookupDataFileType("GTXD_PARENTING_DATA");

		mounter->LoadDataFile(&ventry);

		trace("Mounted gtxd parenting data %s\n", file);
	}
#endif

	g_reloadMapStore = false;
}

class CfxPseudoMounter : public CDataFileMountInterface
{
public:
	virtual bool LoadDataFile(CDataFileMgr::DataFile* entry) override
	{
		if (strcmp(entry->name, "RELOAD_MAP_STORE") == 0)
		{
			g_reloadMapStore = true;

			return true;
		}

		return false;
	}

	virtual void UnloadDataFile(CDataFileMgr::DataFile* entry) override
	{
		if (strcmp(entry->name, "RELOAD_MAP_STORE") == 0)
		{
			// empty?
			loadedCollisions.clear();
		}
	}
};

static CfxPseudoMounter g_staticPseudoMounter;

#ifdef GTA_FIVE
void LoadCache(const char* tagName);
#endif
void LoadManifest(const char* tagName);

class CfxCacheMounter : public CDataFileMountInterface
{
public:
	virtual bool LoadDataFile(CDataFileMgr::DataFile* entry) override
	{
		LoadManifest(entry->name);
#ifdef GTA_FIVE
		LoadCache(entry->name);
#endif

		return true;
	}

	virtual void UnloadDataFile(CDataFileMgr::DataFile* entry) override
	{
	}
};

static CfxCacheMounter g_staticCacheMounter;

struct IgnoreCaseLess
{
	inline bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) < 0;
	}
};

class CDataFileMount
{
public:
	static inline CDataFileMountInterface** sm_Interfaces;
};

// TODO: this might need to be a ref counter instead?
static std::set<std::string, IgnoreCaseLess> g_permanentItyps;
static std::map<uint32_t, std::string> g_itypHashList;

class CfxProxyItypMounter : public CDataFileMountInterface
{
private:
	std::string ParseBaseName(CDataFileMgr::DataFile* entry)
	{
		char baseName[256];
		char* sp = strrchr(entry->name, '/');
		strcpy_s(baseName, sp ? sp + 1 : entry->name);

		auto dp = strrchr(baseName, '.');

		if (dp)
		{
			dp[0] = '\0';
		}

		return baseName;
	}

public:
	virtual bool LoadDataFile(CDataFileMgr::DataFile* entry) override
	{
		// parse dir/dir/blah.ityp into blah
		std::string baseName = ParseBaseName(entry);

		g_itypHashList.insert({ HashString(baseName.c_str()), baseName });

		uint32_t slotId;

		auto module = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ytyp");

#ifdef GTA_FIVE
		if (*module->FindSlot(&slotId, baseName.c_str()) != -1)
#elif IS_RDR3
		if (*module->FindSlotFromHashKey(&slotId, HashString(baseName.c_str())) != -1)
#endif
		{
			// rage::fwAssetStore<rage::fwMapTypesContents,rage::fwMapTypesDef>
			auto refPool = (atPoolBase*)((char*)module + 56);
			auto refPtr = refPool->GetAt<char>(slotId);

			if (refPtr)
			{
#ifdef GTA_FIVE
				uint16_t* flags = (uint16_t*)(refPtr + 16);
#elif IS_RDR3
				uint16_t* flags = (uint16_t*)(refPtr + 24);
#endif

				if (*flags & 4)
				{
					*flags &= ~0x14;

					trace("Removing existing #typ %s\n", baseName);

					g_permanentItyps.insert(baseName);

					streaming::Manager::GetInstance()->ReleaseObject(slotId + module->baseIdx);
				}
			}
		}

		CDataFileMount::sm_Interfaces[DataFileType::DLC_ITYP_REQUEST]->LoadDataFile(entry);

		return true;
	}

	virtual void UnloadDataFile(CDataFileMgr::DataFile* entry) override
	{
		std::string baseName = ParseBaseName(entry);

		uint32_t slotId;

		auto module = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ytyp");

#ifdef GTA_FIVE
		if (*module->FindSlot(&slotId, baseName.c_str()) != -1)
#elif IS_RDR3
		if (*module->FindSlotFromHashKey(&slotId, HashString(baseName.c_str())) != -1)
#endif
		{
			if (g_permanentItyps.find(baseName) != g_permanentItyps.end())
			{
				trace("Loading old #typ %s\n", baseName);

				g_permanentItyps.erase(baseName);

				streaming::Manager::GetInstance()->RequestObject(slotId + module->baseIdx, 7);

				auto refPool = (atPoolBase*)((char*)module + 56);
				auto refPtr = refPool->GetAt<char>(slotId);

				if (refPtr)
				{
#ifdef GTA_FIVE
					*(uint16_t*)(refPtr + 16) |= 4;
#elif IS_RDR3
					*(uint16_t*)(refPtr + 24) |= 4;
#endif
				}
			}
		}

		CDataFileMount::sm_Interfaces[DataFileType::DLC_ITYP_REQUEST]->UnloadDataFile(entry);
	}
};

static CfxProxyItypMounter g_proxyDlcItypMounter;

struct CInteriorProxy
{
	virtual ~CInteriorProxy() = 0;

	uint32_t mapData;
};

#ifdef GTA_FIVE
static hook::thiscall_stub<int(void* store, int* out, uint32_t* inHash)> _getIndexByKey([]()
{
	return hook::get_pattern("39 1C 91 74 4F 44 8B 4C 91 08 45 3B", -0x34);
});

#include <atPool.h>

static atPool<CInteriorProxy>** g_interiorProxyPool;

struct ProxyFile
{
	uint32_t startAt;
	uint32_t hash;
	atArray<uint32_t> proxyHashes;
};

static atArray<ProxyFile>* g_interiorProxyArray;
// 1604: 0x142544440;

class CfxProxyInteriorOrderMounter : public CDataFileMountInterface
{
public:
	virtual bool LoadDataFile(CDataFileMgr::DataFile* entry) override
	{
		CDataFileMount::sm_Interfaces[173]->LoadDataFile(entry);

		return true;
	}

	virtual void UnloadDataFile(CDataFileMgr::DataFile* entry) override
	{
		uint32_t entryHash = HashString(entry->name);

		auto mapDataStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ymap");

		for (auto& entry : *g_interiorProxyArray)
		{
			if (entry.hash == entryHash)
			{
				int i = entry.startAt;

				for (auto& proxyHash : entry.proxyHashes)
				{
					auto proxy = (*g_interiorProxyPool)->GetAt(i);

					if (proxy)
					{
						bool can = true;

						if (proxy->mapData)
						{
							auto pool = (atPoolBase*)((char*)mapDataStore + 56);
							auto entry = pool->GetAt<char>(proxy->mapData);

							if (entry && (*(uint32_t*)(entry + 32) & 0xC00) == 0x800)
							{
								can = false;
							}
						}

						if (can || streaming::IsStreamerShuttingDown())
						{
							// goodbye, interior proxy
							trace("deleted interior proxy %08x\n", proxyHash);
							delete proxy;
						}
					}
					else
					{
						trace(":( didn't find interior proxy %08x\n", proxyHash);
					}

					i++;
				}
			}
		}
	}
};

static CfxProxyInteriorOrderMounter g_proxyInteriorOrderMounter;

static hook::cdecl_stub<void()> _initVehiclePaintRamps([]()
{
	return xbr::IsGameBuildOrGreater<2545>() ? hook::get_pattern("83 F9 FF 74 52", -0x34) : nullptr;
});
#endif

static CDataFileMountInterface* LookupDataFileMounter(const std::string& type)
{
	if (type == "CFX_PSEUDO_ENTRY")
	{
		return &g_staticPseudoMounter;
	}

	if (type == "CFX_PSEUDO_CACHE")
	{
		return &g_staticCacheMounter;
	}

	int fileType = LookupDataFileType(type);

	if (fileType < 0)
	{
		return nullptr;
	}

	if (fileType == 0)
	{
		return &g_staticRpfMounter;
	}

#ifdef GTA_FIVE
	// don't allow TEXTFILE_METAFILE entries (these don't work and will fail to unload)
	if (fileType == 160) // TEXTFILE_METAFILE 
	{
		return nullptr;
	}

	if (fileType == 173) // INTERIOR_PROXY_ORDER_FILE
	{
		return &g_proxyInteriorOrderMounter;
	}
#endif

	if (fileType == DataFileType::DLC_ITYP_REQUEST)
	{
		return &g_proxyDlcItypMounter;
	}

	return CDataFileMount::sm_Interfaces[fileType];
}

static void LoadDataFiles();

static void HandleDataFile(const std::pair<std::string, std::string>& dataFile, const std::function<bool(CDataFileMountInterface*, CDataFileMgr::DataFile& entry)>& fn, const char* op)
{
	std::string typeName;
	std::string fileName;

	std::tie(typeName, fileName) = dataFile;

	trace("%s %s %s.\n", op, typeName, fileName);

	CDataFileMountInterface* mounter = LookupDataFileMounter(typeName);

	if (mounter == nullptr)
	{
		trace("Could not add data_file %s - invalid type %s.\n", fileName, typeName);
		return;
	}

	if (mounter)
	{
		std::string className = SearchTypeName(mounter);

		CDataFileMgr::DataFile entry;
		memset(&entry, 0, sizeof(entry));
		strcpy_s(entry.name, fileName.c_str());
		entry.type = LookupDataFileType(typeName);

		bool result = SafeCall([&]()
		{
			return fn(mounter, entry);
		}, va("%s of %s in data file mounter %s", op, fileName, className));

		if (result)
		{
			trace("done %s %s in data file mounter %s.\n", op, fileName, className);
		}
		else
		{
			trace("failed %s %s in data file mounter %s.\n", op, fileName, className);
		}
	}
}

template<typename TFn, typename TList>
inline void HandleDataFileList(const TList& list, const TFn& fn, const char* op = "loading")
{
	for (const auto& dataFile : list)
	{
		HandleDataFile(dataFile, fn, op);
	}
}

template<typename TFn, typename TList>
inline void HandleDataFileListWithTypes(TList& list, const TFn& fn, const std::set<int>& types, const char* op = "loading")
{
	for (auto it = list.begin(); it != list.end(); )
	{
		if (types.find(LookupDataFileType(it->first)) != types.end())
		{
			HandleDataFile(*it, fn, op);

			it = list.erase(it);
		}
		else
		{
			++it;
		}
	}
}

enum class LoadType
{
	Startup,
	BeforeMapLoad,
	BeforeSession,
	AfterSessionEarlyStage,
	AfterSession
};

void LoadStreamingFiles(LoadType loadType = LoadType::AfterSession);

static LONG FilterUnmountOperation(CDataFileMgr::DataFile& entry)
{
	if (entry.type == DataFileType::DLC_ITYP_REQUEST)
	{
		trace("failed to unload DLC_ITYP_REQUEST %s\n", entry.name);

		return EXCEPTION_EXECUTE_HANDLER;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

namespace streaming
{
	void DLL_EXPORT AddMetaToLoadList(bool before, const std::string& meta)
	{
		((before) ? g_beforeLevelMetas : g_afterLevelMetas).push_back(meta);
	}

	void DLL_EXPORT AddDefMetaToLoadList(const std::string& meta)
	{
		g_defaultMetas.push_back(meta);
	}

	void DLL_EXPORT AddDataFileToLoadList(const std::string& type, const std::string& path)
	{
#ifdef GTA_FIVE
		if (type == "GTXD_PARENTING_DATA")
		{
			g_gtxdFiles.push_back(path);
			return;
		}
#endif

		g_dataFiles.push_back({ type, path });

		if (Instance<ICoreGameInit>::Get()->GetGameLoaded() && !Instance<ICoreGameInit>::Get()->HasVariable("gameKilled"))
		{
			LoadStreamingFiles(LoadType::AfterSessionEarlyStage);
			LoadStreamingFiles();
			LoadDataFiles();
		}
	}

	void DLL_EXPORT RemoveDataFileFromLoadList(const std::string& type, const std::string& path)
	{
		auto dataFilePair = std::make_pair(type, path);

		g_dataFiles.erase(std::remove(g_dataFiles.begin(), g_dataFiles.end(), dataFilePair), g_dataFiles.end());

		if (std::find(g_loadedDataFiles.begin(), g_loadedDataFiles.end(), dataFilePair) == g_loadedDataFiles.end())
		{
			return;
		}

		g_loadedDataFiles.erase(std::remove(g_loadedDataFiles.begin(), g_loadedDataFiles.end(), dataFilePair), g_loadedDataFiles.end());

		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			auto singlePair = { dataFilePair };

			HandleDataFileList(singlePair, [](CDataFileMountInterface* mounter, CDataFileMgr::DataFile& entry)
			{
				__try
				{
					mounter->UnloadDataFile(&entry);

					return true;
				}
				__except (FilterUnmountOperation(entry))
				{
					return false;
				}
			}, "removing");
		}
	}
}

static hook::cdecl_stub<rage::fiCollection* ()> getRawStreamer([]()
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("48 8B D3 4C 8B 00 48 8B C8 41 FF 90 ? 01 00 00 8B D8 E8", -5));
#elif IS_RDR3
	return hook::get_call(hook::get_pattern("45 33 C0 48 8B D6 41 FF 91 ? ? ? ? 8B E8", -11));
#endif
});

#include <unordered_set>

static std::set<std::tuple<std::string, std::string>> g_customStreamingFiles;
std::set<std::string> g_customStreamingFileRefs;
static std::map<std::string, std::vector<std::string>, std::less<>> g_customStreamingFilesByTag;
static std::unordered_map<int, std::list<uint32_t>> g_handleStack;
static std::set<std::pair<streaming::strStreamingModule*, int>> g_pendingRemovals;
std::unordered_map<int, std::string> g_handlesToTag;
static std::set<std::string> g_pedsToRegister;

static std::unordered_set<int> g_ourIndexes;

static std::string GetBaseName(const std::string& name)
{
	std::string retval = name;

	std::string policyVal;

	if (Instance<ICoreGameInit>::Get()->GetData("policy", &policyVal))
	{
#ifndef _DEBUG
		if (policyVal.find("[subdir_file_mapping]") != std::string::npos)
#endif
		{
			std::replace(retval.begin(), retval.end(), '^', '/');
		}
	}

	// is this trying to override a file extant in platform:/textures/?
	auto texturesName = fmt::sprintf("platform:/textures/%s", retval);

	if (auto device = rage::fiDevice::GetDevice(texturesName.c_str(), true); device && device->GetFileAttributes(texturesName.c_str()) != -1)
	{
		retval = texturesName;
	}

	return retval;
}

namespace rage
{
	static hook::cdecl_stub<void(uint16_t)> pgRawStreamerInvalidateEntry([]()
	{
#ifdef GTA_FIVE
		return hook::get_pattern("44 0F B7 C3 41 8B C0 41 81 E0 FF 03 00 00 C1", -0x51);
#elif IS_RDR3
		return hook::get_pattern("48 85 D2 75 ? BA ? ? ? ? B9 ? ? ? ? E8", -0x1B);
#endif
	});
}

#ifdef GTA_FIVE
static hook::cdecl_stub<void(int, const char*)> initGfxTexture([]()
{
	return hook::get_pattern("4C 23 C0 41 83 78 10 FF", -0x57);
});
#endif

std::unordered_map<std::string, int> g_resourceStats;

static void LoadStreamingFiles(LoadType loadType)
{
	auto cstreaming = streaming::Manager::GetInstance();
	std::vector<std::tuple<int, std::string>> newGfx;

	// register any custom streaming assets
	for (auto it = g_customStreamingFiles.begin(); it != g_customStreamingFiles.end(); )
	{
		auto [file, tag] = *it;

		bool isMod = tag.find("mod_") == 0 || tag.find("faux_pack") == 0;

		if (loadType == LoadType::Startup || loadType == LoadType::BeforeMapLoad || loadType == LoadType::AfterSessionEarlyStage)
		{
			// only support tags mod_ and faux_pack
			if (!isMod)
			{
				++it;
				continue;
			}

			// don't allow spoofing for a (comp)cache
			if (file.find("cache:/") != std::string::npos)
			{
				++it;
				continue;
			}
		}

		// get basename ('thing.ytd') and asset name ('thing')
		const char* slashPos = strrchr(file.c_str(), '/');

		if (slashPos == nullptr)
		{
			it = g_customStreamingFiles.erase(it);
			continue;
		}

		auto baseName = GetBaseName(std::string(slashPos + 1));
		auto nameWithoutExt = baseName.substr(0, baseName.find_last_of('.'));

#ifdef GTA_FIVE
		if (!xbr::IsGameBuildOrGreater<2372>() && g_blockedNames2189.find(baseName) != g_blockedNames2189.end())
		{
			trace("ignoring %s: it's a file for GTA V build 2372 or above\n", file);

			it = g_customStreamingFiles.erase(it);
			continue;
		}

		if (!xbr::IsGameBuildOrGreater<2545>() && g_blockedNames2372.find(baseName) != g_blockedNames2372.end() && !IsEvaluationServer())
		{
			trace("ignoring %s: it's a file for GTA V build 2545 or above, _or_ a local evaluation server\n", file);

			it = g_customStreamingFiles.erase(it);
			continue;
		}
#endif

		const char* extPos = strrchr(baseName.c_str(), '.');

		if (extPos == nullptr)
		{
			trace("can't register %s: it doesn't have an extension, why is this in stream/?\n", file);
			it = g_customStreamingFiles.erase(it);
			continue;
		}

		// get CStreaming instance and associated streaming module
		std::string ext = extPos + 1;

		if (ext == "rpf")
		{
			trace("can't register %s: it's an RPF, these don't belong in stream/ without extracting them first\n", file);
			it = g_customStreamingFiles.erase(it);
			continue;
		}

		g_resourceStats[ext]++;

		// this may get used on unloading
		if (baseName == "busy_spinner.gfx")
		{
			it = g_customStreamingFiles.erase(it);
			continue;
		}

		if (loadType == LoadType::Startup)
		{
			if (ext != "ytd" && ext != "gfx")
			{
				++it;
				continue;
			}
		}

		if (loadType != LoadType::AfterSession && loadType != LoadType::AfterSessionEarlyStage)
		{
			if (ext == "ymap" || ext == "ytyp" || ext == "ybn")
			{
				++it;
				continue;
			}
		}

		it = g_customStreamingFiles.erase(it);

		// check if the file even exists
		{
			auto holderDevice = rage::fiDevice::GetDevice(file.c_str(), true);

			if (holderDevice->GetFileAttributes(file.c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				continue;
			}
		}

		auto strModule = cstreaming->moduleMgr.GetStreamingModule(ext.c_str());

		if (strModule)
		{
			// try to create/get an asset in the streaming module
			// RegisterStreamingFile will still work if one exists as long as the handle remains 0
			uint32_t strId = -1;

#ifdef GTA_FIVE
			strModule->FindSlot(&strId, nameWithoutExt.c_str());

			if (strId == -1)
			{
				strModule->FindSlotFromHashKey(&strId, nameWithoutExt.c_str());

				if (ext == "gfx")
				{
					newGfx.push_back({ strId, nameWithoutExt });
				}
			}
#elif IS_RDR3
			strModule->FindSlotFromHashKey(&strId, HashString(nameWithoutExt.c_str()));
#endif

			g_ourIndexes.insert(strId + strModule->baseIdx);
			g_pendingRemovals.erase({ strModule, strId });

			// get the raw streamer and make an entry in there
			auto rawStreamer = getRawStreamer();
			int collectionId = 0;

#ifdef GTA_FIVE
			if (auto idx = streaming::GetRawStreamerForFile(file.c_str(), &rawStreamer))
			{
				collectionId = idx;
			}
#endif

			uint32_t fileId = strId + strModule->baseIdx;

			// if the asset is already registered...
			if (cstreaming->Entries[strId + strModule->baseIdx].handle != 0)
			{
				uint32_t idx = rawStreamer->GetEntryByName(file.c_str());

				if (strId != -1)
				{
					auto& entry = cstreaming->Entries[strId + strModule->baseIdx];
					console::DPrintf("gta:streaming", "overriding handle for %s (was %x) -> %x\n", baseName, entry.handle, (collectionId << 16) | idx);

					// if no old handle was saved, save the old handle
					auto& hs = g_handleStack[strId + strModule->baseIdx];

					if (hs.empty())
					{
						hs.push_front(entry.handle);
					}

					entry.handle = (collectionId << 16) | idx;
					g_handlesToTag[entry.handle] = tag;

					// save the new handle
					hs.push_front(entry.handle);
				}
			}
			else
			{
				fileId = -1;
				streaming::RegisterRawStreamingFile(&fileId, file.c_str(), true, baseName.c_str(), false);

				if (fileId != -1)
				{
					auto& entry = cstreaming->Entries[fileId];
					g_handleStack[fileId].push_front(entry.handle);

					// only for 'real' rawStreamer (mod variant likely won't reregister)
					if (streaming::IsRawHandle(entry.handle))
					{
#ifdef GTA_FIVE
						if (auto rawEntry = rawStreamer->GetEntry(streaming::GetEntryIndex(entry.handle)))
						{
							// if timestamp is 0 then GetEntry triggers the invalidation as well
							rawEntry->timestamp = 0;
							rawStreamer->GetEntry(streaming::GetEntryIndex(entry.handle));
						}
#else
						rage::pgRawStreamerInvalidateEntry(streaming::GetEntryIndex(entry.handle));
#endif
					}

					g_handlesToTag[entry.handle] = tag;
				}
				else
				{
					trace("failed to reg %s? %d\n", baseName, fileId);
				}
			}

			
			// verify whether the file is a zlib'd file from a real backing RPF7
			// (length and resource value mismatching)
#ifdef GTA_FIVE
			auto device = rage::fiDevice::GetDevice(file.c_str(), true);

			if (device && fileId != -1)
			{
				rage::ResourceFlags resFlags;

				if (device->GetResourceVersion(file.c_str(), &resFlags) == 0)
				{
					// we have to check using findData, as this currently uses the compressed size in vfs-core
					rage::fiFindData fd;
					auto findHandle = device->FindFirst(file.c_str(), &fd);

					if (findHandle != -1)
					{
						device->FindClose(findHandle);

						if (resFlags.flag1 != fd.fileSize)
						{
							auto& entry = cstreaming->Entries[fileId];

							// incorrect: fix it in the raw streamer
							auto strEntry = rawStreamer->GetEntry(streaming::GetEntryIndex(entry.handle));
							strEntry->fe.size = fd.fileSize;
						}
					}
				}
			}
#endif
		}
		else if (ext != "ymf")
		{
			trace("can't register %s: no streaming module (does this file even belong in stream?)\n", file);
		}

#ifdef GTA_FIVE
		// register ped asset
		if (baseName.find('/') != std::string::npos)
		{
			g_pedsToRegister.insert(baseName.substr(0, baseName.find('/')));
		}
#endif
	}

#ifdef GTA_FIVE
	if (!newGfx.empty())
	{
		for (const auto& [id, name] : newGfx)
		{
			initGfxTexture(id, name.c_str());
		}
	}
#endif
}

static std::multimap<std::string, std::string, std::less<>> g_manifestNames;

#include <fiCustomDevice.h>

class ForcedDevice : public rage::fiCustomDevice
{
private:
	rage::fiDevice* m_device;
	std::string m_fileName;

public:
	ForcedDevice(rage::fiDevice* parent, const std::string& fileName)
		: m_device(parent), m_fileName(fileName)
	{
	}

	virtual uint64_t Open(const char* fileName, bool readOnly) override
	{
		return m_device->Open(m_fileName.c_str(), readOnly);
	}

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr) override
	{
		return m_device->OpenBulk(m_fileName.c_str(), ptr);
	}

	virtual uint64_t OpenBulkWrap(const char* fileName, uint64_t* ptr, void*) override
	{
		return OpenBulk(fileName, ptr);
	}

	virtual uint64_t Create(const char* fileName) override
	{
		return -1;
	}

	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead) override
	{
		return m_device->Read(handle, buffer, toRead);
	}

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead) override
	{
		return m_device->ReadBulk(handle, ptr, buffer, toRead);
	}

	virtual int32_t GetCollectionId() override
	{
		return m_device->GetCollectionId();
	}

	virtual uint32_t Write(uint64_t, void*, int) override
	{
		return -1;
	}

	virtual uint32_t WriteBulk(uint64_t, int, int, int, int) override
	{
		return -1;
	}

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method) override
	{
		return m_device->Seek(handle, distance, method);
	}

	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method) override
	{
		return m_device->SeekLong(handle, distance, method);
	}

	virtual int32_t Close(uint64_t handle) override
	{
		return m_device->Close(handle);
	}

	virtual int32_t CloseBulk(uint64_t handle) override
	{
		return m_device->CloseBulk(handle);
	}

	virtual int GetFileLength(uint64_t handle) override
	{
		return m_device->GetFileLength(handle);
	}

	virtual uint64_t GetFileLengthLong(const char* fileName) override
	{
		return m_device->GetFileLengthLong(m_fileName.c_str());
	}

	virtual uint64_t GetFileLengthUInt64(uint64_t handle) override
	{
		return m_device->GetFileLengthUInt64(handle);
	}

	virtual bool RemoveFile(const char* file) override
	{
		return false;
	}

	virtual int RenameFile(const char* from, const char* to) override
	{
		return false;
	}

	virtual int CreateDirectory(const char* dir) override
	{
		return false;
	}

	virtual int RemoveDirectory(const char* dir) override
	{
		return false;
	}

	virtual uint64_t GetFileTime(const char* file) override
	{
		return m_device->GetFileTime(m_fileName.c_str());
	}

	virtual bool SetFileTime(const char* file, FILETIME fileTime) override
	{
		return false;
	}

	virtual uint32_t GetFileAttributes(const char* path) override
	{
		return m_device->GetFileAttributes(m_fileName.c_str());
	}

	virtual int m_yx() override
	{
		return m_device->m_yx();
	}

	virtual bool IsCollection() override
	{
		return m_device->IsCollection();
	}

	virtual const char* GetName() override
	{
		return "RageVFSDeviceAdapter";
	}

	virtual int GetResourceVersion(const char* fileName, rage::ResourceFlags* version) override
	{
		return m_device->GetResourceVersion(m_fileName.c_str(), version);
	}

	virtual uint64_t CreateLocal(const char* fileName) override
	{
		return m_device->CreateLocal(m_fileName.c_str());
	}

	virtual void* m_xy(void* a, int len, void* c) override
	{
		return m_device->m_xy(a, len, (void*)m_fileName.c_str());
	}
};

#ifdef GTA_FIVE
static hook::cdecl_stub<void(void*, void* packfile, const char*)> loadManifest([]()
{
	return hook::get_pattern("49 8B F0 4C 8B F1 48 85 D2 0F 84", -0x23);
});
#elif IS_RDR3
static hook::cdecl_stub<void(void*, void* packfile, const char*, bool)> loadManifest([]()
{
	return hook::get_pattern("83 A5 ? ? ? ? 00 E8 ? ? ? ? 48 8B C8 4C", -0x38);
});
#endif

static std::unordered_multimap<uint32_t, uint32_t> g_itypToMapDataDeps;

void LoadManifest(const char* tagName)
{
	auto range = g_manifestNames.equal_range(tagName);

	for (auto& namePair : fx::GetIteratorView(range))
	{
		auto name = namePair.second;
		auto rel = new ForcedDevice(rage::fiDevice::GetDevice(name.c_str(), true), name);

		// see if we can even read the file
		{
			auto handle = rel->Open(name.c_str(), true);

			if (handle == uint64_t(-1))
			{
				continue;
			}

			char buf[16];
			if (rel->Read(handle, buf, 16) != 16)
			{
				continue;
			}

			rel->Close(handle);
		}

		_initManifestChunk(manifestChunkPtr);

		rage::fiDevice::MountGlobal("localPack:/", rel, true);

#ifdef GTA_FIVE
		loadManifest(manifestChunkPtr, (void*)1, tagName);
#elif IS_RDR3
		loadManifest(manifestChunkPtr, (void*)1, tagName, false);
#endif

		rage::fiDevice::Unmount("localPack:/");

		struct CItypDependencies
		{
#ifdef IS_RDR3
			void* vtable;
#endif
			uint32_t itypName;
			uint32_t manifestFlags;

			atArray<uint32_t> itypDepArray;
		};

		struct CImapDependencies
		{
#ifdef IS_RDR3
			void* vtable;
#endif

			uint32_t imapName;
			uint32_t manifestFlags;

			atArray<uint32_t> imapDepArray;
		};

#ifdef GTA_FIVE
		struct manifestData
		{
			char pad[16];
			atArray<CImapDependencies> imapDependencies_2;
			char pad2[16];
			atArray<CItypDependencies> itypDependencies;
		}* manifestChunk = (manifestData*)manifestChunkPtr;

		for (auto& dep : manifestChunk->itypDependencies)
		{
			if (auto it = g_itypHashList.find(dep.itypName); it != g_itypHashList.end())
			{
				auto name = fmt::sprintf("dummy/%s.ityp", it->second);
				trace("Fixing manifest-required #typ dependency for %s\n", name);

				auto mounter = LookupDataFileMounter("DLC_ITYP_REQUEST");

				CDataFileMgr::DataFile entry = { 0 };
				strcpy_s(entry.name, name.c_str());

				mounter->UnloadDataFile(&entry);
			}
		}

		for (auto& dep : manifestChunk->imapDependencies_2)
		{
			for (auto idx : dep.imapDepArray)
			{
				g_itypToMapDataDeps.emplace(idx, dep.imapName);
			}
		}
#elif IS_RDR3
		struct manifestArray
		{
			void** m_entries;
			uint32_t m_count;
			char pad_C[4];
			uint32_t unk_10;
			char pad_14[3];
			char unk_17;
		};

		struct manifestData
		{
			char pad[16];
			manifestArray imapDependencies_2;
			char pad2[56];
			manifestArray itypDependencies;
		}* manifestChunk = (manifestData*)manifestChunkPtr;

		for (int i = 0; i < manifestChunk->itypDependencies.m_count; i++)
		{
			if (auto dep = (CItypDependencies*)manifestChunk->itypDependencies.m_entries[i])
			{
				if (auto it = g_itypHashList.find(dep->itypName); it != g_itypHashList.end())
				{
					auto name = fmt::sprintf("dummy/%s.ityp", it->second);
					trace("Fixing manifest-required #typ dependency for %s\n", name);

					auto mounter = LookupDataFileMounter("DLC_ITYP_REQUEST");

					CDataFileMgr::DataFile entry = { 0 };
					strcpy_s(entry.name, name.c_str());

					mounter->UnloadDataFile(&entry);
				}
			}
		}

		for (int i = 0; i < manifestChunk->imapDependencies_2.m_count; i++)
		{
			if (auto dep = (CImapDependencies*)manifestChunk->imapDependencies_2.m_entries[i])
			{
				for (auto idx : dep->imapDepArray)
				{
					g_itypToMapDataDeps.emplace(idx, dep->imapName);
				}
			}
		}
#endif

		_loadManifestChunk(manifestChunkPtr);
		_clearManifestChunk(manifestChunkPtr);
	}
}

#ifdef GTA_FIVE
#include <EntitySystem.h>
#include <RageParser.h>

struct CPedModelInfo
{
private:
	uint64_t vtbl;
	uint8_t pad[16];
public:
	uint32_t hash;

private:
	uint8_t pad2[428];

public:
	atArray<char> streamFolder;

private:
	uint8_t pad3[188];
};

static hook::cdecl_stub<void(fwFactoryBase<fwArchetype>*, atArray<CPedModelInfo*>&)> _getAllPedArchetypes([]()
{
	return hook::get_call(hook::get_pattern("44 8B E0 4C 89 6C 24 20 44 89 6C 24 28 E8", 13));
});

static void RegisterPeds()
{
	auto pedArchetypeFactory = (*g_archetypeFactories)[6];

	atArray<CPedModelInfo*> mis;
	_getAllPedArchetypes(pedArchetypeFactory, mis);

	for (auto mi : mis)
	{
		for (const auto& ped : g_pedsToRegister)
		{
			if (mi->hash == HashString(ped.c_str()))
			{
				mi->streamFolder.Expand(ped.size() + 1);

				strcpy(&mi->streamFolder[0], ped.c_str());
				mi->streamFolder.m_count = ped.size() + 1;
			}
		}
	}
}
#endif

static void LoadDataFiles()
{
	trace("Loading mounted data files (total: %d)\n", g_dataFiles.size());

	// sort data file array by type, a little
	auto dfSort = [](const std::pair<std::string, std::string>& type)
	{
		auto h = HashString(type.first.c_str());

		if (h == HashString("VEHICLE_LAYOUTS_FILE") || h == HashString("HANDLING_FILE"))
		{
			return 0;
		}
		else
		{
			return 100;
		}
	};

	// we use stable_sort as equivalent entries need to retain equivalent order
	std::stable_sort(g_dataFiles.begin(), g_dataFiles.end(), [dfSort](const auto& left, const auto& right)
	{
		return dfSort(left) < dfSort(right);
	});

#if __has_include(<StatusText.h>)
	ActivateStatusText("Loading add-on data files", 5, 2);
	OnLookAliveFrame();
#endif

	HandleDataFileList(g_dataFiles, [](CDataFileMountInterface* mounter, CDataFileMgr::DataFile& entry)
	{
#if __has_include(<StatusText.h>)
		OnLookAliveFrame();
#endif

		return mounter->LoadDataFile(&entry);
	});

	g_loadedDataFiles.insert(g_loadedDataFiles.end(), g_dataFiles.begin(), g_dataFiles.end());
	g_dataFiles.clear();

#if __has_include(<StatusText.h>)
	DeactivateStatusText(2);
	OnLookAliveFrame();
#endif

	if (g_reloadMapStore)
	{
		trace("Performing deferred RELOAD_MAP_STORE.\n");

		ReloadMapStore();
	}

#ifdef GTA_FIVE
	if (!g_pedsToRegister.empty())
	{
		RegisterPeds();

		g_pedsToRegister.clear();
	}
#endif
}

DLL_EXPORT void ForceMountDataFile(const std::pair<std::string, std::string>& dataFile)
{
	std::vector<std::pair<std::string, std::string>> dataFiles = { dataFile };

	HandleDataFileList(dataFiles, [](CDataFileMountInterface* mounter, CDataFileMgr::DataFile& entry)
	{
		return mounter->LoadDataFile(&entry);
	});
}

void ForAllStreamingFiles(const std::function<void(const std::string&)>& cb)
{
	for (auto& entry : g_customStreamingFileRefs)
	{
		cb(entry);
	}
}

#include <nutsnbolts.h>

static bool g_reloadStreamingFiles;
static std::atomic<int> g_lockedStreamingFiles;

#ifdef GTA_FIVE
void origCfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags);
void origCfxCollection_BackoutStreamingTag(const std::string& tag);
#endif

void DLL_EXPORT CfxCollection_SetStreamingLoadLocked(bool locked)
{
	if (locked)
	{
		g_lockedStreamingFiles++;
	}
	else
	{
		g_lockedStreamingFiles--;
	}
}

void DLL_EXPORT CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags)
{
	size_t start = 0;

	if (auto it = fileName.find_last_of('/'); it != std::string::npos)
	{
		start = it + 1;
	}

	auto baseName = GetBaseName(fileName.substr(start));

	if (baseName.find(".ymf") == baseName.length() - 4)
	{
		g_manifestNames.emplace(tag, fileName);
	}

	g_customStreamingFilesByTag[tag].push_back(fileName);
	g_customStreamingFiles.insert({ fileName, tag });
	g_customStreamingFileRefs.insert(baseName);

	g_reloadStreamingFiles = true;

#ifdef GTA_FIVE
	origCfxCollection_AddStreamingFileByTag(tag, fileName, flags);
#endif
}

void DLL_EXPORT CfxCollection_BackoutStreamingTag(const std::string& tag)
{
	// undo whatever AddStreamingFileByTag did
	for (auto& name : g_customStreamingFilesByTag[tag])
	{
		g_customStreamingFiles.erase({ name, tag });
		g_customStreamingFileRefs.erase(name);
	}

	g_manifestNames.erase(tag);
	g_customStreamingFilesByTag.erase(tag);

#ifdef GTA_FIVE
	origCfxCollection_BackoutStreamingTag(tag);
#endif
}

void DLL_EXPORT CfxCollection_RemoveStreamingTag(const std::string& tag)
{
	// ensure that we can call into game code here
	// #FIXME: should we not always be on the main thread?!
	rage::sysMemAllocator::UpdateAllocatorValue();

	for (const auto& file : g_customStreamingFilesByTag[tag])
	{
		// get basename ('thing.ytd') and asset name ('thing')
		size_t start = 0;

		if (auto it = file.find_last_of('/'); it != std::string::npos)
		{
			start = it + 1;
		}

		auto baseName = GetBaseName(file.substr(start));
		auto nameWithoutExt = baseName.substr(0, baseName.find_last_of('.'));

		// get dot position and skip if no dot
		auto dotPos = strrchr(baseName.c_str(), '.');

		if (!dotPos)
		{
			continue;
		}

		// get CStreaming instance and associated streaming module
		auto cstreaming = streaming::Manager::GetInstance();
		auto strModule = cstreaming->moduleMgr.GetStreamingModule(dotPos + 1);

		if (strModule)
		{
			uint32_t strId;

#ifdef GTA_FIVE
			strModule->FindSlot(&strId, nameWithoutExt.c_str());
#elif IS_RDR3
			strModule->FindSlotFromHashKey(&strId, HashString(nameWithoutExt.c_str()));
#endif

			if (strId != -1)
			{
#ifdef IS_RDR3
				auto rawStreamer = getRawStreamer();
				uint32_t idx = (rawStreamer->GetCollectionId() << 16) | rawStreamer->GetEntryByName(file.c_str());
#endif

				// remove from our index set
				g_ourIndexes.erase(strId + strModule->baseIdx);

				// erase existing stack entry
				auto& handleData = g_handleStack[strId + strModule->baseIdx];

				for (auto it = handleData.begin(); it != handleData.end(); ++it)
				{
#ifdef GTA_FIVE
					auto rawStreamer = streaming::GetRawStreamerByIndex(streaming::GetCollectionIndex(*it));
					auto entryName = rawStreamer->GetEntryName(streaming::GetEntryIndex(*it));
					if (entryName && strcmp(file.c_str(), entryName) == 0)
#elif IS_RDR3
					if (*it == idx)
#endif
					{
						it = handleData.erase(it);
					}
				}

				// if not empty, set the handle to the current stack entry
				auto& entry = cstreaming->Entries[strId + strModule->baseIdx];

				if (!handleData.empty())
				{
					entry.handle = handleData.front();
				}
				else
				{
					g_pendingRemovals.insert({ strModule, strId });

					g_customStreamingFileRefs.erase(baseName);
					entry.handle = 0;
				}
			}
		}
	}

	for (auto& file : g_customStreamingFilesByTag[tag])
	{
		g_customStreamingFiles.erase(std::tuple<std::string, std::string>{ file, tag });
	}

	g_customStreamingFilesByTag.erase(tag);
	g_manifestNames.erase(tag);
}

static void UnloadDataFiles()
{
	if (!g_loadedDataFiles.empty())
	{
		trace("Unloading data files (%d entries)\n", g_loadedDataFiles.size());

		HandleDataFileList(g_loadedDataFiles,
			[](CDataFileMountInterface* mounter, CDataFileMgr::DataFile& entry)
		{
			mounter->UnloadDataFile(&entry);
			return true;
		}, "unloading");

		g_loadedDataFiles.clear();
	}
}

static void UnloadDataFilesOfTypes(const std::set<int>& types)
{
	HandleDataFileListWithTypes(g_loadedDataFiles, [](CDataFileMountInterface* mounter, CDataFileMgr::DataFile& entry)
	{
		mounter->UnloadDataFile(&entry);
		return true;
	}, types, "pre-unloading");
}

static const char* NormalizePath(char* out, const char* in, size_t length)
{
	strncpy(out, in, length);
	out[length - 1] = '\0';

	size_t l = strlen(out);

	for (size_t i = 0; i < l; i++)
	{
		if (out[i] == '\\')
		{
			out[i] = '/';
		}
	}

	return out;
}

static const char* pgRawStreamer__GetEntryNameToBuffer(rage::fiCollection* streamer, uint16_t index, char* buffer, int len)
{
	const char* fileName = streamer->m_entries[index].fileName;

	if (fileName == nullptr)
	{
		buffer[0] = '\0';
		return buffer;
	}

	strncpy(buffer, fileName, len - 1);
	buffer[len - 1] = '\0';

	return buffer;
}

static void DisplayRawStreamerError [[noreturn]] (rage::fiCollection* streamer, uint16_t index, const char* why)
{
	auto streamingMgr = streaming::Manager::GetInstance();

	uint32_t attemptIndex = (((rage::fiCollection*)streamer)->GetCollectionId() << 16) | index;
	std::string extraData;

	for (size_t i = 0; i < streamingMgr->numEntries; i++)
	{
		const auto& entry = streamingMgr->Entries[i];

		if (entry.handle == attemptIndex)
		{
			std::string tag = g_handlesToTag[entry.handle];

			extraData += fmt::sprintf("Streaming tag: %s\n", tag);
			extraData += fmt::sprintf("File name: %s\n", streaming::GetStreamingNameForIndex(i));
			extraData += fmt::sprintf("Handle stack size: %d\n", g_handleStack[i].size());
			extraData += fmt::sprintf("Tag exists: %s\n", g_customStreamingFilesByTag.find(tag) != g_customStreamingFilesByTag.end() ? "yes" : "no");
		}
	}

	FatalError("Invalid pgRawStreamer call - %s.\nStreaming index: %d\n%s\n\nIf this issue persists make sure to remove incompatible asi mods.", why, index, extraData);
}

static void ValidateRawStreamerReq(rage::fiCollection* streamer, uint16_t index)
{
	uint32_t index0 = index >> 10;
	uint32_t index1 = index & 0x3FF;

	if (index0 >= std::size(streamer->m_entries.memory))
	{
		DisplayRawStreamerError(streamer, index, "index >= size(entries)");
	}

	auto entryList = streamer->m_entries.memory[index0];
	if (!entryList)
	{
		DisplayRawStreamerError(streamer, index, "!entryList");
	}

	const char* fileName = entryList[index1].fileName;

	if (fileName == nullptr)
	{
		DisplayRawStreamerError(streamer, index, "fileName == NULL");
	}
}

static int64_t(*g_origOpenCollectionEntry)(rage::fiCollection* streamer, uint16_t index, uint64_t* ptr);

static int64_t pgRawStreamer__OpenCollectionEntry(rage::fiCollection* streamer, uint16_t index, uint64_t* ptr)
{
	ValidateRawStreamerReq(streamer, index);

	return g_origOpenCollectionEntry(streamer, index, ptr);
}

static int64_t(*g_origGetEntry)(rage::fiCollection* streamer, uint16_t index);

static int64_t pgRawStreamer__GetEntry(rage::fiCollection* streamer, uint16_t index)
{
	ValidateRawStreamerReq(streamer, index);

	return g_origGetEntry(streamer, index);
}

static bool g_unloadingCfx;

namespace streaming
{
	bool IsStreamerShuttingDown()
	{
		return g_unloadingCfx;
	}
}

static void* g_streamingInternals;
static bool g_lockReload;

std::unordered_set<std::string> g_streamingSuffixSet;

static hook::cdecl_stub<void()> _waitUntilStreamerClear([]()
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("80 A1 ? 01 00 00 FE 8B EA ", 12));
#elif IS_RDR3
	return hook::get_call(hook::get_pattern("80 A3 ? ? ? ? FE 48 8D 0D ? ? ? ? BA FF FF 00 00", 24));
#endif
});

static hook::cdecl_stub<void(void*)> _resyncStreamers([]()
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("80 A1 ? 01 00 00 FE 8B EA ", 24));
#elif IS_RDR3
	return hook::get_call(hook::get_pattern("80 A3 ? ? ? ? FE 48 8D 0D ? ? ? ? BA FF FF 00 00", 19));
#endif
});

static hook::cdecl_stub<void()> _unloadTextureLODs([]()
{
#ifdef GTA_FIVE
	// there's two of these, both seem to do approximately the same thing, but the first one we want
	return hook::get_pattern("48 85 DB 75 1B 8D 47 01 49 8D", -0x84);
#elif IS_RDR3
	return hook::get_pattern("49 8B 1C C2 48 85 DB 75 ? 48", -54);
#endif
});

#ifdef GTA_FIVE
static void FlushCustomAssets()
{
	auto strManager = streaming::Manager::GetInstance();

	for (size_t i = 0; i < strManager->numEntries; i++)
	{
		auto& entry = strManager->Entries[i];

		// if loaded or loading
		if ((entry.flags & 3) == 2 || (entry.flags & 3) == 3)
		{
			// if this is registered by us
			// #TODO: check if model info streaming module
			if (g_handlesToTag.find(entry.handle) != g_handlesToTag.end() ||
				strManager->moduleMgr.GetStreamingModule(i) == rage::fwArchetypeManager::GetStreamingModule())
			{
				// force-unload the object (canceling the request)
				// if this breaks next reload, 'so be it', we just want to get to the main menu safely

				// copied/adapted from unloading code above
				if (entry.flags & 0xFFFC)
				{
					entry.flags &= ~0xFFFC;
				}

				if (entry.flags & 0x40000000)
				{
					entry.flags &= ~0x40000000;
				}

				// ClearRequiredFlag
				strManager->ReleaseObject(i, 0xF1);

				// RemoveObject
				strManager->ReleaseObject(i);
			}
		}
	}
}
#endif

static void SafelyDrainStreamer()
{
	g_unloadingCfx = true;

	trace("Shutdown: waiting for streaming to finish\n");

	_waitUntilStreamerClear();

#ifdef GTA_FIVE
	trace("Shutdown: canceling requests\n");

	FlushCustomAssets();
#endif

	trace("Shutdown: updating GTA streamer state\n");

	_resyncStreamers(g_streamingInternals);

	trace("Shutdown: unloading texture LODs\n");

	_unloadTextureLODs();

	trace("Shutdown: streamer tasks done\n");

	g_resourceStats.clear();
}

#ifdef GTA_FIVE
static void(*g_origAddMapBoolEntry)(void* map, int* index, bool* value);

void WrapAddMapBoolEntry(void* map, int* index, bool* value)
{
	// don't allow this for any files of our own
	if (g_ourIndexes.find(*index) == g_ourIndexes.end())
	{
		g_origAddMapBoolEntry(map, index, value);
	}
}

static void(*g_origExecuteGroup)(void* mgr, uint32_t hashValue, bool value);

static void ExecuteGroupForWeaponInfo(void* mgr, uint32_t hashValue, bool value)
{
	g_origExecuteGroup(mgr, hashValue, value);

	for (auto it = g_loadedDataFiles.begin(); it != g_loadedDataFiles.end();)
	{
		auto [fileType, fileName] = *it;

		if (fileType == "WEAPONINFO_FILE_PATCH" || fileType == "WEAPONINFO_FILE")
		{
			HandleDataFile(*it, [](CDataFileMountInterface* mounter, CDataFileMgr::DataFile& entry)
			{
				mounter->UnloadDataFile(&entry);
				return true;
			}, "early-unloading for CWeaponMgr");

			it = g_loadedDataFiles.erase(it);
		}
		else
		{
			it++;
		}
	}
}

static void(*g_origUnloadWeaponInfos)();

static hook::cdecl_stub<void(void*)> wib_ctor([]()
{
	return hook::get_pattern("41 8D 50 01 48 8D 41", -0x35);
});

struct CWeaponInfoBlob
{
	char m_pad[248];

	CWeaponInfoBlob()
	{
		wib_ctor(this);
	}
};

static atArray<CWeaponInfoBlob>* g_weaponInfoArray;

static void UnloadWeaponInfosStub()
{
	g_origUnloadWeaponInfos();

	g_weaponInfoArray->Clear();
	g_weaponInfoArray->Expand(kNumWeaponInfoBlobs);
}

static hook::cdecl_stub<void(int32_t)> rage__fwArchetypeManager__FreeArchetypes([]()
{
	return hook::get_pattern("8B F9 8B DE 66 41 3B F0 73 33", -0x19);
});

static void(*g_origUnloadMapTypes)(void*, uint32_t);

void fwMapTypesStore__Unload(char* assetStore, uint32_t index)
{
	auto pool = (atPoolBase*)(assetStore + 56);
	auto entry = pool->GetAt<char>(index);

	if (entry != nullptr)
	{
		if (*(uintptr_t*)entry != 0)
		{
			if (g_unloadingCfx)
			{
				*(uint16_t*)(entry + 16) &= ~0x14;
			}

			g_origUnloadMapTypes(assetStore, index);
		}
		else
		{
			AddCrashometry("maptypesstore_workaround_2", "true");
		}
	}
	else
	{
		AddCrashometry("maptypesstore_workaround", "true");
	}
}

static void ModifyHierarchyStatusHook(streaming::strStreamingModule* module, int idx, int* status)
{
	if (*status == 1 && g_ourIndexes.find(module->baseIdx + idx) != g_ourIndexes.end())
	{
		auto thisName = streaming::GetStreamingNameForIndex(module->baseIdx + idx);

		// if this is, say, vb_02.ymap, and we also loaded hei_vb_02.ymap, skip this file
		if (g_streamingSuffixSet.find(thisName) == g_streamingSuffixSet.end())
		{
			*status = 2;
		}
	}
}

static bool(*g_orig_fwStaticBoundsStore__ModifyHierarchyStatus)(streaming::strStreamingModule* module, int idx, int status);

static bool fwStaticBoundsStore__ModifyHierarchyStatus(streaming::strStreamingModule* module, int idx, int status)
{
	// don't disable hierarchy overlay for any custom overrides
	ModifyHierarchyStatusHook(module, idx, &status);

	return g_orig_fwStaticBoundsStore__ModifyHierarchyStatus(module, idx, status);
}

static bool(*g_orig_fwMapDataStore__ModifyHierarchyStatusRecursive)(streaming::strStreamingModule* module, int idx, int status);

static bool fwMapDataStore__ModifyHierarchyStatusRecursive(streaming::strStreamingModule* module, int idx, int status)
{
	// don't disable hierarchy overlay for any custom overrides
	ModifyHierarchyStatusHook(module, idx, &status);

	return g_orig_fwMapDataStore__ModifyHierarchyStatusRecursive(module, idx, status);
}

static void (*g_origLoadReplayDlc)(void* ecw);

static void LoadReplayDlc(void* ecw)
{
	g_lockReload = false;

	LoadStreamingFiles(LoadType::BeforeSession);

	g_origLoadReplayDlc(ecw);

	LoadStreamingFiles(LoadType::AfterSessionEarlyStage);
	LoadStreamingFiles(LoadType::AfterSession);
	LoadDataFiles();
}

static void (*g_origfwMapTypes__ConstructArchetypes)(void* mapTypes, int32_t idx);

static void fwMapTypes__ConstructArchetypesStub(void* mapTypes, int32_t idx)
{
	// free this archetype file since we're recreating it anyway
	// this should be safe, as an asset won't get loaded without it having been unloaded before
	rage__fwArchetypeManager__FreeArchetypes(idx);

	g_origfwMapTypes__ConstructArchetypes(mapTypes, idx);
}

static void (*g_origfwMapDataStore__FinishLoading)(void* store, int32_t idx, CMapData** data);

static void fwMapDataStore__FinishLoadingHook(streaming::strStreamingModule* store, int32_t idx, CMapData** data)
{
	CMapData* mapData = *data;

	static_assert(offsetof(CMapData, name) == 8, "CMapData::name");

	for (fwEntityDef* entity : mapData->entities)
	{
		if (entity->GetTypeIdentifier()->m_nameHash == HashRageString("CMloInstanceDef"))
		{
			if (!(mapData->contentFlags & 8))
			{
				trace("Fixed fwMapData contentFlags (missing 'interior' flag) in %s.\n", streaming::GetStreamingNameForIndex(idx + store->baseIdx));
				mapData->contentFlags |= 8;
			}
		}
	}

	// b2802 - four-april-virginia
	// b2699 - skylark-whiskey-jig
	CDistantLODLight& lodLights = mapData->distantLodLights;
	if (lodLights.numStreetLights > lodLights.positions.GetCount())
	{
		auto name = streaming::GetStreamingNameForIndex(idx + store->baseIdx);
		trace("Fixed CMapData DistantLODLightsSOA (numStreetLights > positions.GetCount()) in %s.\n", name);

		lodLights.numStreetLights = lodLights.positions.GetCount();
	}

	return g_origfwMapDataStore__FinishLoading(store, idx, data);
}
#endif

static bool ret0()
{
	return false;
}

#ifdef GTA_FIVE
static void (*g_origLoadVehicleMeta)(CDataFileMgr::DataFile* entry, bool notMapTypes, uint32_t modelHash);
static void (*g_origAddArchetype)(fwArchetype*, uint32_t typesHash);

static void GetTxdRelationships(std::map<int, int>& map)
{
	static auto module = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ytd");
	
	atPoolBase* entryPool = (atPoolBase*)((char*)module + 56);
	for (size_t i = 0; i < entryPool->GetSize(); i++)
	{
		if (auto entry = entryPool->GetAt<char>(i); entry)
		{
			int idx = -1;

			if (xbr::IsGameBuildOrGreater<1868>())
			{
				idx = *(int32_t*)(entry + 16);
			}
			else
			{
				idx = *(uint16_t*)(entry + 16);

				if (idx == 0xFFFF)
				{
					idx = -1;
				}
			}

			if (idx >= 0)
			{
				map[i] = idx;
			}
		}
	}
}

static std::multimap<uint32_t, std::pair<int, int>> g_undoTxdRelationships;
static thread_local bool overrideTypesHash;

#include <VFSManager.h>

// since algorithms are hard, this
// copied from SO: https://stackoverflow.com/a/5816029
static void calc_z(std::string& s, std::vector<int>& z)
{
	int len = s.size();
	z.resize(len);

	int l = 0, r = 0;
	for (int i = 1; i < len; ++i)
		if (z[i - l] + i <= r)
			z[i] = z[i - l];
		else
		{
			l = i;
			if (i > r)
				r = i;
			for (z[i] = r - i; r < len; ++r, ++z[i])
				if (s[r] != s[z[i]])
					break;
			--r;
		}
}

static std::unordered_set<uint32_t> g_hashes;

static void LoadVehicleMetaForDlc(CDataFileMgr::DataFile* entry, bool notMapTypes, uint32_t modelHash)
{
	// try logging any and all txdstore relationships we made, to find any differences
	std::map<int, int> txdRelationships;
	GetTxdRelationships(txdRelationships);

	// we use DLC name as hash
	auto entryHash = HashString(entry->name);

	overrideTypesHash = true;
	g_origLoadVehicleMeta(entry, notMapTypes, entryHash);
	g_hashes.insert(entryHash);
	overrideTypesHash = false;

	// get the txdstore relationships, again
	std::map<int, int> txdRelationshipsAfter;
	GetTxdRelationships(txdRelationshipsAfter);

	// find a difference
	std::vector<std::pair<int, int>> newRelationships;
	std::set_difference(txdRelationshipsAfter.begin(), txdRelationshipsAfter.end(), txdRelationships.begin(), txdRelationships.end(), std::back_inserter(newRelationships));

	for (auto& relationship : newRelationships)
	{
		if (auto relIt = txdRelationships.find(relationship.first); relIt != txdRelationships.end())
		{
			g_undoTxdRelationships.emplace(entryHash, *relIt);
		}
		else
		{
			g_undoTxdRelationships.insert({ entryHash, { relationship.first, -1 } });
		}
	}
}

static void AddVehicleArchetype(fwArchetype* self, uint32_t typesHash)
{
	if (overrideTypesHash)
	{
		typesHash = 0xF000;
	}

	g_origAddArchetype(self, typesHash);
}

static bool (*g_origParserCreateAndLoadAnyType)(void* self, const char* path, const char* extension, void* parStructure, void* parParsableStructure, void* a6);

static bool ParserCreateAndLoadAnyType(void* self, const char* path, const char* extension, void* parStructure, void* parParsableStructure, void* a6)
{
	bool success = g_origParserCreateAndLoadAnyType(self, path, extension, parStructure, parParsableStructure, a6);
	if (success && overrideTypesHash)
	{
		uint32_t entryHash = HashString(path);
		uint16_t entryCount = *(uint16_t*)(*(char**)parParsableStructure + 8);
		g_archetypeFactories->Get(5)->AddStorageBlock(entryHash, entryCount);
	}
	return success;
}

static void (*g_origUnloadVehicleMeta)(CDataFileMgr::DataFile* entry);

static void UnloadVehicleMetaForDlc(CDataFileMgr::DataFile* entry)
{
	auto hash = HashString(entry->name);
	g_origUnloadVehicleMeta(entry);

	// unload TXD relationships
	for (auto& relationship : fx::GetIteratorView(g_undoTxdRelationships.equal_range(hash)))
	{
		static auto module = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ytd");

		atPoolBase* entryPool = (atPoolBase*)((char*)module + 56);
		if (auto entry = entryPool->GetAt<char>(relationship.second.first); entry)
		{
			if (xbr::IsGameBuildOrGreater<1868>())
			{
				*(int32_t*)(entry + 16) = relationship.second.second;
			}
			else
			{
				*(uint16_t*)(entry + 16) = relationship.second.second;
			}
		}
	}

	g_undoTxdRelationships.erase(hash);

	// unload vehicle models
	rage__fwArchetypeManager__FreeArchetypes(hash);
}

static void (*g_origFreeArchetypes)(uint32_t idx);

static void FreeArchetypesHook(uint32_t idx)
{
	if (idx == 0xF000)
	{
		for (uint32_t hash : g_hashes)
		{
			g_origFreeArchetypes(hash);
		}

		g_hashes.clear();
	}

	g_origFreeArchetypes(idx);
}
#endif

#ifdef GTA_FIVE
DLL_IMPORT extern fwEvent<> PreSetupLoadingScreens;
#endif

#if defined(GTA_FIVE) || IS_RDR3
rage::fiCollection* (*g_GetRawStreamer)(void);
static int32_t chunkyArrayCountOffset = 0;
static int32_t chunkyArrayOffset = 0;
void* (*g_chunkyArrayAppend)(hook::FlexStruct* self);
void* chunkyArrayAppend(hook::FlexStruct* self)
{
	const int32_t loadedEntriesCount = self->Get<int32_t>(chunkyArrayCountOffset);
	if (loadedEntriesCount >= 0xFFFF)
	{
		std::stringstream ss;
		for (auto& [ext, num] : g_resourceStats)
		{
			ss << ext << ": " << num << ", ";
		}
		AddCrashometry("asset_stats", ss.str());

		AddCrashometry("pgRawStreamer", std::to_string(g_GetRawStreamer()->m_entries.count));
#ifdef GTA_FIVE
		AddCrashometry("pgRawStreamer(ytd)", std::to_string(streaming::GetRawStreamerByIndex(1)->m_entries.count));
		AddCrashometry("pgRawStreamer(mod)", std::to_string(streaming::GetRawStreamerByIndex(2)->m_entries.count));
#endif
		
		FatalError("ERR_STR_FAILURE: trying to add more assets to pgRawStreamer when it's already full (65535).");
	}

	return g_chunkyArrayAppend(self);
}

static ConsoleCommand pgRawStreamer_AssetsCountCmd("assetscount", []()
{
	std::stringstream ss;
	for (auto& [ext, num] : g_resourceStats)
	{
		ss << ext << ": " << num << ", ";
	}
	trace("%s\n", ss.str());
	trace("Total loaded assets in pgRawStreamer - %d/65535\n", g_GetRawStreamer()->m_entries.count);

#ifdef GTA_FIVE
	trace("Total loaded assets in pgRawStreamer(ytd) - %d/65535\n", streaming::GetRawStreamerByIndex(1)->m_entries.count);
	trace("Total loaded assets in pgRawStreamer(mod) - %d/65535\n", streaming::GetRawStreamerByIndex(2)->m_entries.count);
#endif
});

const rage::chunkyArray<rage::fiCollection::RawEntry, 1024, 64>& rage::GetPgRawStreamerEntries()
{
	return g_GetRawStreamer()->m_entries;
}

#endif

static void CleanupStreaming()
{
	// safely drain the RAGE streamer before we unload everything
	SafelyDrainStreamer();

	g_lockReload = true;
	g_unloadingCfx = true;

	UnloadDataFiles();

	std::set<std::string> tags;

	for (auto& tag : g_customStreamingFilesByTag)
	{
		tags.insert(tag.first);
	}

	for (auto& tag : tags)
	{
		CfxCollection_RemoveStreamingTag(tag);
	}

	auto mapDataStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ymap");
	auto typesStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ytyp");
	auto navMeshStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ynv");
	auto staticBoundsStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ybn");
	auto str = streaming::Manager::GetInstance();

	for (auto [module, idx] : g_pendingRemovals)
	{
		if (module == typesStore)
		{
			struct fwMapDataDef
			{
#ifdef GTA_FIVE
				uint8_t pad[24];
#elif IS_RDR3
				uint8_t pad[32];
#endif
				union
				{
					uint32_t idx;
					uint32_t* idxArray;
				} dependencies;

				uint8_t pad2[6];
				uint16_t dependencyCount;

				void RemoveDependency(uint32_t idx)
				{
					if (dependencyCount == 1)
					{
						if (dependencies.idx == idx)
						{
							dependencyCount = 0;
							dependencies.idx = -1;
						}
					}
					else
					{
						for (int i = 0; i < dependencyCount; i++)
						{
							if (dependencies.idxArray[i] == idx)
							{
								if ((i + 1) < dependencyCount)
								{
									memmove(&dependencies.idxArray[i], &dependencies.idxArray[i + 1], sizeof(uint32_t) * (dependencyCount - i));
								}

								dependencyCount--;
								i--;
							}
						}

						// move out of array if we're 1 now
						if (dependencyCount == 1)
						{
							auto soleIdx = dependencies.idxArray[0];
							rage::GetAllocator()->Free(dependencies.idxArray);

							dependencies.idx = soleIdx;
						}
					}
				}
			};

			atPoolBase* entryPool = (atPoolBase*)((char*)module + 56);
			auto entry = entryPool->GetAt<char>(idx);

#ifdef GTA_FIVE
			*(uint16_t*)(entry + 16) &= ~0x14;
#elif IS_RDR3
			*(uint16_t*)(entry + 24) &= ~0x14;
#endif

			// remove from any dependent mapdata
			for (auto entry : fx::GetIteratorView(g_itypToMapDataDeps.equal_range(*(uint32_t*)(entry + 12))))
			{
				auto mapDataHash = entry.second;
				auto mapDataIdx = streaming::GetStreamingIndexForLocalHashKey(mapDataStore, mapDataHash);

				if (mapDataIdx != -1)
				{
					atPoolBase* entryPool = (atPoolBase*)((char*)mapDataStore + 56);
					auto mdEntry = entryPool->GetAt<fwMapDataDef>(mapDataIdx);

					if (mdEntry)
					{
						mdEntry->RemoveDependency(idx);
					}
				}
			}
		}

		// if this is loaded by means of dependents, in Five we should remove the flags indicating this, or RemoveObject will fail and RemoveSlot will lead to inconsistent state
		// in RDR3 this will have a special-case check in RemoveObject for dependents, but in case it fails we shall remove this still (otherwise RemoveSlot will corrupt)
		//
		// we don't do this for fwStaticBoundsStore since we don't call RemoveSlot for other reasons (will lead to odd state for interiors)
		if (module != staticBoundsStore && str->Entries[idx + module->baseIdx].flags & 0xFFFC)
		{
			str->Entries[idx + module->baseIdx].flags &= ~0xFFFC;
		}

		// ClearRequiredFlag
		str->ReleaseObject(idx + module->baseIdx, 0xF1);

		// RemoveObject
		str->ReleaseObject(idx + module->baseIdx);

#ifdef GTA_FIVE
		if (module == typesStore)
		{
			// if unloaded at *runtime* but flags were set, archetypes likely weren't freed - we should
			// free them now.
			rage__fwArchetypeManager__FreeArchetypes(idx);
		}
#endif
	}

	// call RemoveSlot after we have removed all objects, or dependency tracking may crash
	for (auto [module, idx] : g_pendingRemovals)
	{
		// navmeshstore won't remove from some internal 'name hash' and therefore re-registration crashes
		// staticboundsstore has a weird issue too at times (regarding interior proxies?)
		if (module != navMeshStore && module != staticBoundsStore)
		{
			module->RemoveSlot(idx);
		}
	}

	g_pendingRemovals.clear();

	g_unloadingCfx = false;
}

static HookFunction hookFunction([]()
{
#ifdef GTA_FIVE
	g_GetRawStreamer = (decltype(g_GetRawStreamer))hook::get_pattern<uint8_t>("48 83 EC ? 48 8B 05 ? ? ? ? 48 85 C0 75 ? 8D 50");
	auto chunkyArrayAppendLoc = hook::get_pattern<uint8_t>("40 53 48 83 EC ? F7 81 ? ? ? ? ? ? ? ? 48 8B D9 75");
	chunkyArrayCountOffset = *(int32_t*)(chunkyArrayAppendLoc + 8);
	chunkyArrayOffset = *hook::get_pattern<int32_t>("48 8D 8F ? ? ? ? E8 ? ? ? ? 48 8D 4C 24", 3);
	g_chunkyArrayAppend = hook::trampoline(chunkyArrayAppendLoc, &chunkyArrayAppend);
#elif IS_RDR3
	g_GetRawStreamer = (decltype(g_GetRawStreamer))hook::get_pattern<uint8_t>("48 83 EC ? 48 8B 05 ? ? ? ? 48 85 C0 75 ? 8D 50 ? B9");
	auto chunkyArrayAppendLoc = hook::get_pattern<uint8_t>("40 53 48 83 EC ? 8B 91 ? ? ? ? 48 8B D9 F7 C2");
	chunkyArrayCountOffset = *(int32_t*)(chunkyArrayAppendLoc + 8);
	chunkyArrayOffset = *hook::get_pattern<int32_t>("48 8D 9F ? ? ? ? 48 89 07 83 A3", 3);
	g_chunkyArrayAppend = hook::trampoline(chunkyArrayAppendLoc, &chunkyArrayAppend);
#endif

#ifdef GTA_FIVE

	loadChangeSet = hook::get_pattern<char>("48 81 EC 50 03 00 00 49 8B F0 4C", -0x18);

	PreSetupLoadingScreens.Connect([]()
	{
		FlushCustomAssets();
	});

	{
		auto location = hook::pattern("BA A1 85 94 52 41 B8 01").count(1).get(0).get<char>(0x34);
		g_interiorProxyPool = (decltype(g_interiorProxyPool))(location + *(int32_t*)location + 4);
	}

	g_interiorProxyArray = hook::get_address<decltype(g_interiorProxyArray)>(hook::get_pattern("83 FA FF 75 4D 48 8D 0D ? ? ? ? BA", 8));

	// vehicle metadata removal could be per DLC
	// therefore, replace 0xF000 with an actual hash of the filename
	{
		auto location = hook::get_pattern("41 B8 00 F0 00 00 33 D2 E8", 8);
		hook::set_call(&g_origLoadVehicleMeta, location);
		hook::call(location, LoadVehicleMetaForDlc);

		location = hook::get_pattern("8B D5 48 8B CE 89 46 18 40 84 FF 74 0A", 0x17);
		hook::set_call(&g_origAddArchetype, location);
		hook::call(location, AddVehicleArchetype);

		location = hook::get_pattern("89 55 EC 49 8B D2 48 89 44 24", 11);
		hook::set_call(&g_origParserCreateAndLoadAnyType, location);
		hook::call(location, ParserCreateAndLoadAnyType);
	}

	// unloading wrapper
	{
		MH_Initialize();

		auto location = hook::get_pattern("49 89 43 18 49 8D 43 10 33 F6", -0x21);
		MH_CreateHook(location, UnloadVehicleMetaForDlc, (void**)&g_origUnloadVehicleMeta);
		MH_EnableHook(location);

		location = hook::get_pattern("8B F9 8B DE 66 41 3B F0 73 33", -0x19);
		MH_CreateHook(location, FreeArchetypesHook, (void**)&g_origFreeArchetypes);
		MH_EnableHook(location);
	}
#endif

	// process streamer-loaded resource: check 'free instantly' flag even if no dependencies exist (change jump target)
#ifdef GTA_FIVE
	hook::put<int8_t>(hook::get_pattern<int8_t>("4C 63 C0 85 C0 7E 54 48 8B", 6), 0x25);
#elif IS_RDR3
	hook::put<int8_t>(hook::get_pattern<int8_t>("4C 63 C8 85 C0 7E 62 4C 8B", 21), 0x2E);
#endif

	// same function: stub to change free-instantly flag if needed by bypass streaming
#ifdef GTA_FIVE
	static struct : jitasm::Frontend
	{
		static bool ShouldRequestBeAllowed()
		{
			if (streaming::IsStreamerShuttingDown())
			{
				return false;
			}

			return true;
		}

		void InternalMain() override
		{
			sub(rsp, 0x28);

			// restore rcx as the call stub used this
			mov(rcx, r14);

			// call the original function that's meant to be called
			mov(rax, qword_ptr[rax + (xbr::IsGameBuildOrGreater<2802>() ? 0xD8 : 0xA8)]);
			call(rax);

			// save the result in a register (r12 is used as output by this function)
			mov(r12, rax);

			// store the streaming request in a1
			mov(rcx, rsi);
			mov(rax, (uintptr_t)&ShouldRequestBeAllowed);
			call(rax);

			// perform a switcharoo of rax and r12
			// (r12 is the result the game wants, rax is what we want in r12)
			xchg(r12, rax);

			add(rsp, 0x28);
			ret();
		}
	} streamingBypassStub;

	{
		auto location = hook::get_pattern("45 8A E7 FF 90 ? 00 00 00");
		hook::nop(location, 9);
		hook::call_rcx(location, streamingBypassStub.GetCode());
	}
#endif

#ifdef GTA_FIVE
	g_streamingInternals = hook::get_address<void*>(hook::get_pattern("80 A1 ? 01 00 00 FE 8B EA ", 20));
	manifestChunkPtr = hook::get_address<void*>(hook::get_pattern("C7 80 ? 01 00 00 02 00 00 00 E8 ? ? ? ? 8B 06", -4));
#elif IS_RDR3
	g_streamingInternals = hook::get_address<void*>(hook::get_pattern("80 A3 ? ? ? ? FE 48 8D 0D ? ? ? ? BA FF FF 00 00", 10));
	manifestChunkPtr = hook::get_address<void*>(hook::get_pattern<char>("33 C9 E8 ? ? ? ? 48 8D 55 10 89 45 10 48", 17));
#endif

	// level load
#ifdef GTA_FIVE
	void* hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 0D ? ? ? ? 41 B0 01 48 8B D3").count(1).get(0).get<void>(18);
#elif IS_RDR3
	void* hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 0D ? ? ? ? 4C 8D 0D ? ? ? ? 41 B0 01 48 8B D3 E8").count(1).get(0).get<void>(25);
#endif
	hook::set_call(&dataFileMgr__loadDat, hookPoint);
	hook::call(hookPoint, LoadDats);

	//hookPoint = hook::pattern("E8 ? ? ? ? 33 C9 E8 ? ? ? ? 41 8B CE E8 ? ? ? ?").count(1).get(0).get<void>(0); //Jayceon - If I understood right, is this what we were supposed to do? It seems wrong to me
	hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 1D ? ? ? ? 41 8B F7").count(1).get(0).get<void>(0);
	hook::set_call(&dataFileMgr__loadDefDat, hookPoint);
	hook::call(hookPoint, LoadDefDats); //Call the new function to load the handling files

	// don't normalize paths in pgRawStreamer
#ifdef GTA_FIVE
	hook::call(hook::get_pattern("48 8B D6 E8 ? ? ? ? B2 01 48", 3), NormalizePath);
#elif IS_RDR3
	hook::call(hook::get_pattern("75 ? B2 01 48 8B CB E8 ? ? ? ? 48 8B F8 48 85 C0", -43), NormalizePath);
#endif

	g_dataFileTypes = hook::get_pattern<EnumEntry>("61 44 DF 04 00 00 00 00");

#define X(NAME) DataFileType::NAME = LookupDataFileType(#NAME)
	X(DLC_ITYP_REQUEST);
	X(DLC_POP_GROUPS);
	X(DLC_WEAPON_PICKUPS);
#undef X

	rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_BEFORE_MAP_LOADED)
		{
			if (!g_dataFileMgr)
			{
				return;
			}

			trace("Loading default meta overrides (total: %d)\n", g_defaultMetas.size());

			for (const auto& dat : g_defaultMetas)
			{
				trace("Loading default meta %s\n", dat);

#ifdef GTA_FIVE
				dataFileMgr__loadDat(g_dataFileMgr, dat.c_str(), true);
#elif IS_RDR3
				dataFileMgr__loadDat(g_dataFileMgr, dat.c_str(), true, nullptr);
#endif
			}

			trace("Done loading default meta overrides!\n");
		}
	});

#ifdef GTA_FIVE
	OnKillNetwork.Connect([](auto)
	{
		FlushCustomAssets();
	}, 0);

	OnKillNetworkDone.Connect([]()
	{
		g_pedsToRegister.clear();
	}, 99925);
#endif

	OnKillNetworkDone.Connect([]()
	{
		// safely drain the RAGE streamer before we unload everything
		SafelyDrainStreamer();

		g_unloadingCfx = false;

		// unload pre-unloaded data files
		UnloadDataFilesOfTypes({ DataFileType::DLC_POP_GROUPS, DataFileType::DLC_WEAPON_PICKUPS });
	}, 99900);

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		CleanupStreaming();
	}, -9999);

	OnMainGameFrame.Connect([=]()
	{
		if (
			g_reloadStreamingFiles && g_lockedStreamingFiles == 0 && !g_lockReload
#ifdef IS_RDR3
			&& Instance<ICoreGameInit>::Get()->GetGameLoaded()
#endif
		)
		{
			LoadStreamingFiles(LoadType::AfterSessionEarlyStage);
			LoadStreamingFiles(LoadType::AfterSession);

			g_reloadStreamingFiles = false;
		}
	});

	{
#ifdef GTA_FIVE
		char* location = hook::get_pattern<char>("48 63 82 90 00 00 00 49 8B 8C C0 ? ? ? ? 48", 11);
#elif IS_RDR3
		char* location = hook::get_pattern<char>("8B 82 90 00 00 00 49 8B 8C C0 ? ? ? ? 48", 10);
#endif

		CDataFileMount::sm_Interfaces = (decltype(CDataFileMount::sm_Interfaces))(hook::get_adjusted(0x140000000) + *(int32_t*)location); // why is this an RVA?!
	}

	{
#ifdef GTA_FIVE
		char* location = hook::get_pattern<char>("79 91 C8 BC E8 ? ? ? ? 48 8D", -0x30);

		location += 0x1A;
		g_extraContentManager = (void**)(*(int32_t*)location + location + 4);
		location -= 0x1A;

		hook::set_call(&g_disableContentGroup, location + 0x23);
		hook::set_call(&g_enableContentGroup, location + 0x34);
		hook::set_call(&g_clearContentCache, location + ((xbr::IsGameBuildOrGreater<2189>()) ? 0x5C : 0x50));

#elif IS_RDR3
		char* location = hook::get_pattern<char>("E8 ? ? ? ? 8B 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 8D 95");
		g_extraContentManager = hook::get_address<void**>(location + 14);

		hook::set_call(&g_disableContentGroup, location);
		hook::set_call(&g_enableContentGroup, location + 31);
#endif
	}

	rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_SESSION)
		{
			g_lockReload = false;

			LoadStreamingFiles(LoadType::BeforeSession);
		}
		else if (type == rage::INIT_CORE)
		{
			LoadStreamingFiles(LoadType::Startup);
		}
	});

	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_BEFORE_MAP_LOADED)
		{
			LoadStreamingFiles(LoadType::BeforeMapLoad);
		}
		else if (type == rage::INIT_SESSION)
		{
			LoadStreamingFiles(LoadType::AfterSessionEarlyStage);
			LoadStreamingFiles(LoadType::AfterSession);
			LoadDataFiles();

#ifdef GTA_FIVE
			if (xbr::IsGameBuildOrGreater<2545>())
			{
				_initVehiclePaintRamps();
			}
#endif
		}
	});

	// support CfxRequest for pgRawStreamer
#ifdef GTA_FIVE
	hook::jump(hook::get_pattern("4D 63 C1 41 8B C2 41 81 E2 FF 03 00 00", -0xD), pgRawStreamer__GetEntryNameToBuffer);
#elif IS_RDR3
	hook::jump(hook::get_pattern("4D 63 C1 81 E2 FF 03 00 00 48 C1 E8 0A 48 8B 84 C1 B0 05 00 00", -8), pgRawStreamer__GetEntryNameToBuffer);
#endif


	{
		// mapdatastore/maptypesstore 'should async place'
		
		// typesstore
		{
#ifdef GTA_FIVE
			int offset = xbr::IsGameBuildOrGreater<2802>() ? 35 : 29;
			auto vtbl = hook::get_address<void**>(hook::get_pattern("45 8D 41 1C 48 8B D9 C7 40 D8 00 01 00 00", 22));
			hook::put(&vtbl[offset], ret0);
#elif IS_RDR3
			auto vtbl = hook::get_address<void**>(hook::get_pattern("C7 40 D8 00 01 00 00 45 8D 41 49 E8", 19));
			hook::put(&vtbl[34], ret0);
#endif
		}

		// datastore
		{
#ifdef GTA_FIVE
			int offset = xbr::IsGameBuildOrGreater<2802>() ? 35 : 29;
			auto vtbl = hook::get_address<void**>(hook::get_pattern("44 8D 46 0E C7 40 D8 C7 01 00 00 E8", 19));
			hook::put(&vtbl[offset], ret0);
#elif IS_RDR3
			auto vtbl = hook::get_address<void**>(hook::get_pattern("C7 40 D8 C7 01 00 00 44 8D 47 49 E8", 19));
			hook::put(&vtbl[34], ret0);
#endif
		}

		// raw #map/#typ loading
		hook::nop(hook::get_pattern("D1 E8 A8 01 74 ? 48 8B 84", 4), 2);
	}

#ifdef GTA_FIVE
	// replay dlc loading
	{
		auto location = hook::get_pattern("0F 84 ? ? ? ? 48 8B 0D ? ? ? ? C6 05 ? ? ? ? 01 E8", 20);
		hook::set_call(&g_origLoadReplayDlc, location);
		hook::call(location, LoadReplayDlc);
	}

	// special point for CWeaponMgr streaming unload
	// (game calls CExtraContentManager::ExecuteTitleUpdateDataPatchGroup with a specific group intended for weapon info here)
	{
		auto location = hook::get_pattern<char>("45 33 C0 BA E9 C8 73 AA E8", 8);
		hook::set_call(&g_origExecuteGroup, location);
		hook::call(location, ExecuteGroupForWeaponInfo);

		g_weaponInfoArray = hook::get_address<decltype(g_weaponInfoArray)>(location + 0x74);
	}

	// don't create an unarmed weapon when *unloading* a WEAPONINFO_FILE in the mounter (this will get badly freed later
	// which will lead to InitSession failing)
	{
		hook::return_function(hook::get_pattern("7C 94 48 85 F6 74 ? 48 8B 06 BA 01 00 00 00", xbr::IsGameBuildOrGreater<2802>() ? 0x3D : 0x3C));
	}

	// fully clean weaponinfoblob array when resetting weapon manager
	// not doing so will lead to parser crashes when a half-reset value is reused
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("45 33 C0 BA E9 C8 73 AA E8", -0x11), UnloadWeaponInfosStub, (void**)&g_origUnloadWeaponInfos);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	// fwMapTypesStore double unloading workaround
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("4C 63 C2 33 ED 46 0F B6 0C 00 8B 41 4C", -18), fwMapTypesStore__Unload, (void**)&g_origUnloadMapTypes);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	// fwMapDataStore::FinishLoading map flag fixing
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("25 00 0C 00 00 3D 00 08 00 00 49 8B 06", -0x6F), fwMapDataStore__FinishLoadingHook, (void**)&g_origfwMapDataStore__FinishLoading);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	// fwMapTypes::ConstructArchetypes hook (for #typ override issues with different sizes)
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("FF 50 28 0F B7 46 20 33 ED", -0x21), fwMapTypes__ConstructArchetypesStub, (void**)&g_origfwMapTypes__ConstructArchetypes);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	// do not ever register our streaming files as part of DLC packfile dependencies
	{
		auto location = hook::get_pattern("48 8B CE C6 85 ? ? 00 00 01 89 44 24 20 E8", 14);
		hook::set_call(&g_origAddMapBoolEntry, location);
		hook::call(location, WrapAddMapBoolEntry);
	}

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("45 8B E8 4C 8B F1 83 FA FF 0F 84", -0x18), fwStaticBoundsStore__ModifyHierarchyStatus, (void**)&g_orig_fwStaticBoundsStore__ModifyHierarchyStatus);
	MH_CreateHook(hook::get_pattern("45 33 D2 84 C0 0F 84 ? 01 00 00 4C", -0x28), fwMapDataStore__ModifyHierarchyStatusRecursive, (void**)&g_orig_fwMapDataStore__ModifyHierarchyStatusRecursive);
	MH_EnableHook(MH_ALL_HOOKS);
#endif

	// debug hook for pgRawStreamer::OpenCollectionEntry

#ifdef GTA_FIVE
	g_origOpenCollectionEntry = hook::trampoline(hook::get_pattern("8B D5 81 E2", -0x24), pgRawStreamer__OpenCollectionEntry);
	g_origGetEntry = hook::trampoline(hook::get_pattern("0F B7 C3 48 8B 5C 24 30 8B D0 25 FF", -0x14), pgRawStreamer__GetEntry);
#elif IS_RDR3
	g_origOpenCollectionEntry = hook::trampoline(hook::get_pattern("49 8B F0 48 8B 84 C1", -0x2D), pgRawStreamer__OpenCollectionEntry);
#endif
});
