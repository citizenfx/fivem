/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <map>
#include <string>

#include <Error.h>

#include <ShaderInfo.h>

#define RAGE_FORMATS_GAME payne
#define RAGE_FORMATS_GAME_PAYNE
#include <gtaDrawable.h>

#undef RAGE_FORMATS_GAME_PAYNE
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#include <gtaDrawable.h>

#include <convert/base.h>

namespace rage
{
extern FORMATS_EXPORT int g_2curGeom;
extern FORMATS_EXPORT std::map<int, void*> g_2vbMapping;
extern FORMATS_EXPORT std::map<int, void*> g_2ibMapping;

template<>
five::pgDictionary<five::grcTexturePC>* convert(payne::pgDictionary<payne::grcTexturePC>* txd)
{
	five::pgDictionary<five::grcTexturePC>* out = new(false) five::pgDictionary<five::grcTexturePC>();
	out->SetBlockMap();

	five::pgDictionary<five::grcTexturePC> newTextures;

	if (txd->GetCount()) // amazingly there's 0-sized TXDs?
	{
		for (auto& texture : *txd)
		{
			payne::grcTexturePC* nyTexture = texture.second;
			five::grcTexturePC* fiveTexture = new(false) five::grcTexturePC(
				nyTexture->GetWidth(),
				nyTexture->GetHeight(),
				nyTexture->GetPixelFormat(),
				nyTexture->GetStride(),
				nyTexture->GetLevels(),
				nyTexture->GetPixelData()
				);

			fiveTexture->SetName(nyTexture->GetName());

			trace("%s: %p\n", nyTexture->GetName(), fiveTexture->GetPixelData());

			newTextures.Add(texture.first, fiveTexture);
		}
	}

	out->SetFrom(&newTextures);

	return out;
}

inline std::string LookupShaderName(uint32_t fxHash)
{
	auto fxNames = {
		"im",

		"default",
		"default_doublesided",
		"default_spec",
		"cloth_default",
		"rope_default",
		"normal",
		"normal_wall_rain",
		"normal_grnd_rain",
		"normal_spec",
		"normal_spec_noreflect",
		"normal_spec_anim_uv",
		"normal_spec_grnd_rain",
		"normal_spec_reflect_grnd_rain",
		"normal_spec_reflect",
		"normal_spec_reflect_emissive",
		"normal_spec_reflect_emissivenight",
		"normal_spec_cubemap_reflect",
		"normal_reflect",
		"normal_reflect_grnd_rain",
		"normal_cubemap_reflect",
		"cutout_fence",
		"spec",
		"spec_reflect",
		"sscope_lense",
		"sscope_crosshair",
		"reflect",
		"cubemap_reflect",
		"glass",
		"glass_spec",
		"glass_normal_spec_reflect",
		"glass_reflect",
		"glass_emissive",
		"glass_emissivenight",
		"projtex2",
		"projtex",
		"projtex_steep",
		"projtex_specmap",
		"projtex_steep_specmap",
		"projtex_blood",
		"projtex_static_blood",
		"parallax",
		"parallax_grnd_rain",
		"parallax_reflect_grnd_rain",
		"parallax_specmap",
		"parallax_specmap_grnd_rain",
		"parallax_specmap_reflect_grnd_rain",
		"parallax_steep",
		"parallax_steep",
		"emissive",
		"emissivestrong",
		"emissivenight",
		"emissive_spec",
		"emissive_uvanim",

		"wire",
		"grass",
		"grass_object",
		"grass_object_normal",
		"grass_object_normal_spec_reflect",
		"trees",
		"radar",
		"billboard_nobump",
		"diffuse_instance",

		"terrain_va_4lyr",
		"terrain_va_3lyr",
		"terrain_va_2lyr",

		"terrain_cb_2lyr",
		"terrain_cb_2lyr_spec_reflect",

		"terrain_cb_4lyr",
		"terrain_cb_4lyr_grnd_rain",
		"terrain_cb_4lyr_2tex",
		"terrain_cb_4lyr_cm",
		"terrain_cb_4lyr_2tex_spec",
		"terrain_cb_4lyr_spec",
		"terrain_cb_7lyr",

		"decal",
		"decal_glue",
		"decal_dirt",
		"decal_amb_only",
		"decal_normal_only",
		"decal_normal_only_grnd_rain",
		"decal_zbias",
		"normal_decal",
		"normal_decal_grnd_rain",
		"normal_spec_decal",
		"normal_spec_noreflect_decal",
		"normal_spec_decal_glue",
		"normal_spec_decal_grnd_rain",
		"normal_spec_reflect_decal",
		"normal_spec_reflect_decal_grnd_rain",
		"normal_reflect_decal",
		"normal_reflect_decal_grnd_rain",
		"spec_decal",
		"spec_reflect_decal",
		"reflect_decal",

		"vehicle_basic",
		"vehicle_badges",
		"vehicle_chrome",
		"vehicle_disc",
		"vehicle_generic",
		"vehicle_interior",
		"vehicle_interior2",
		"vehicle_lightsemissive",
		"vehicle_mesh",
		"vehicle_paint1",
		"vehicle_paint2",
		"vehicle_paint3",
		"vehicle_paint1_enveff",
		"vehicle_paint2_enveff",
		"vehicle_paint3_enveff",
		"vehicle_paint1_normal",
		"vehicle_paint2_normal",
		"vehicle_paint3_normal",
		"vehicle_paint1_normal_reflect",
		"vehicle_paint2_normal_reflect",
		"vehicle_paint3_normal_reflect",
		"vehicle_rims1",
		"vehicle_rims2",
		"vehicle_rubber",
		"vehicle_shuts",
		"vehicle_tire",
		"vehicle_track",
		"vehicle_vehglass",
		"vehicle_rotorblade",
		"vehicle_rotorblade_droop",

		"ped",
		"ped_ao",
		"ped_blendshape",
		"ped_cloth",
		"ped_cloth_sweat",
		"ped_cpvwind",
		"ped_cpvwind_wrinkle",
		"ped_cpvwind_wrinkle_cloth_sweat",
		"ped_wrinkle",
		"ped_wrinkle_cloth_sweat",
		"ped_wrinkle_skin_sweat",
		"ped_palette",
		"ped_reflect",
		"ped_reflect_wrinkle",
		"ped_decal",
		"ped_detail",
		"ped_hair_sorted_alpha",
		"ped_hair_sorted_alpha_blendshape",
		"ped_hair_sorted_alpha_exp",
		"ped_hair_sorted_alpha_exp_blendshape",
		"ped_hair_sorted_alpha_cpv",
		"ped_hair_sorted_alpha_cpv_blendshape",
		"ped_hair_sorted_alpha_cpv_bent",
		"ped_hair_sorted_alpha_cpv_bent_reflect",
		"ped_hair_sorted_alpha_cpv_bent_blendshape",
		"ped_hair_sorted_ssa_cloth_2sided",
		"ped_hair_spiked",
		"ped_hair_spiked_blendshape",
		"ped_hair_long_alpha_exp",
		"ped_hair_long_alpha_exp_blendshape",
		"ped_insignia",
		"ped_fur",
		"ped_enveff",
		"ped_skin",
		"ped_skin_sweat",
		"ped_skin_blendshape",
		"ped_skin_blendshape_sweat",
		"ped_skin_blendshape_wrinkle",
		"ped_skin_blendshape_wrinkle_sweat",
		"ped_default",
		"ped_default_palette",

		"ptxgpudrop_render",
		"rmptfx_cameffect",
		"rmptfx_gpurender",
		"rmptfx_litsprite",
		"rmptfx_litsprite_blood",
		"rmptfx_mesh",
		"rmptfx_mesh_normal",
		"rmptfx_raindrops",
		"rmptfx_smoke",
		"rmptfx_fire",
		"rmptfx_particulates",
		"rmptfx_litsprite3D",
		"rmptfx_litsprite3D_MT",
		"rmptfx_smoke3D",
		"rmptfx_smoke3D_MT",

		"postfx",
		"fxaa",
		"mlaa",
		"water",
		"waterTex",
		"mirror",

		"atmoscatt_clouds",
		"dome_clouds",
		"sky_system",

		"micromovement",
		"micromovement_default",
		"micromovement_normal",
		"micromovement_normal_spec",
		"micromovement_spec",
		"poolwater",

		"character_decal_map",
		"velocity_buffer",

		"silhouette",
		"skin_blendshape",
		"skin_blendshape_normalmap",
		"skin_landbaron",

		"tracer_default_ai",
		"tracer_default_player",
		"tracer_default_player_mp",
		"tracer_bullettime_ai",
		"tracer_bullettime_player",
		"tracer_bullettime_player_mp",
		"tracer_bulletcam",
		"tracer_heathaze",

		"weapon_normal_spec_reflect",

		"laserbeam",
		"light_volumes",

		"debug_DTQ"
	};

	for (auto& fx : fxNames)
	{
		if (HashString(fx) == fxHash)
		{
			return fx;
		}
	}

	return "default";
}

inline std::string LookupSpsName(uint32_t spsHash)
{
	auto spsNames = {
		"alpha.sps",
		"billboard_nobump.sps",
		"cloth_default.sps",
		"cloth_alpha.sps",
		"cubemap_reflect.sps",
		"cutout.sps",
		"cutout_fence.sps",
		"decal.sps",
		"decal_amb_only.sps",
		"decal_dirt.sps",
		"decal_glue.sps",
		"decal_normal_only.sps",
		"decal_normal_only_grnd_rain.sps",
		"decal_zbias.sps",
		"default.sps",
		"default_doublesided_cutout.sps",
		"default_spec.sps",
		"diffuse_instance.sps",
		"dome_clouds.sps",
		"emissive.sps",
		"emissive_uvanim.sps",
		"emissivenight.sps",
		"emissivenight_alpha.sps",
		"emissivestrong.sps",
		"emissivestrong_alpha.sps",
		"emissive_alpha.sps",
		"emissive_uvanim_alpha.sps",
		"emissive_spec.sps",
		"emissive_spec_alpha.sps",
		"fence_ssa.sps",
		"glass.sps",
		"glass_emissive.sps",
		"glass_emissivenight.sps",
		"glass_emissivenight_alpha.sps",
		"glass_emissive_alpha.sps",
		"glass_normal_spec_reflect.sps",
		"glass_reflect.sps",
		"glass_spec.sps",
		"grass.sps",
		"grass_object.sps",
		"grass_object_normal.sps",
		"grass_object_normal_spec_reflect.sps",
		"gta_alpha.sps",
		"gta_cubemap_reflect.sps",
		"gta_cutout.sps",
		"gta_cutout_fence.sps",
		"gta_decal.sps",
		"gta_decal_amb_only.sps",
		"gta_decal_dirt.sps",
		"gta_decal_glue.sps",
		"gta_decal_normal_only.sps",
		"gta_default.sps",
		"gta_diffuse_instance.sps",
		"gta_emissive.sps",
		"gta_emissivenight.sps",
		"gta_emissivenight_alpha.sps",
		"gta_emissivestrong.sps",
		"gta_emissivestrong_alpha.sps",
		"gta_emissive_alpha.sps",
		"gta_glass.sps",
		"gta_glass_emissive.sps",
		"gta_glass_emissivenight.sps",
		"gta_glass_emissivenight_alpha.sps",
		"gta_glass_emissive_alpha.sps",
		"gta_glass_normal_spec_reflect.sps",
		"gta_glass_reflect.sps",
		"gta_glass_spec.sps",
		"gta_hair_sorted_alpha_expensive.sps",
		"gta_leaves.sps",
		"gta_mirror.sps",
		"gta_normal.sps",
		"gta_normal_alpha.sps",
		"gta_normal_cubemap_reflect.sps",
		"gta_normal_cutout.sps",
		"gta_normal_decal.sps",
		"gta_normal_reflect.sps",
		"gta_normal_reflect_alpha.sps",
		"gta_normal_reflect_decal.sps",
		"gta_normal_reflect_screendooralpha.sps",
		"gta_normal_screendooralpha.sps",
		"gta_normal_spec.sps",
		"gta_normal_spec_alpha.sps",
		"gta_normal_spec_cubemap_reflect.sps",
		"gta_normal_spec_decal.sps",
		"gta_normal_spec_reflect.sps",
		"gta_normal_spec_reflect_alpha.sps",
		"gta_normal_spec_reflect_decal.sps",
		"gta_normal_spec_reflect_emissive.sps",
		"gta_normal_spec_reflect_emissivenight.sps",
		"gta_normal_spec_reflect_emissivenight_alpha.sps",
		"gta_normal_spec_reflect_emissive_alpha.sps",
		"gta_normal_spec_screendooralpha.sps",
		"gta_parallax.sps",
		"gta_parallax_specmap.sps",
		"gta_parallax_steep.sps",
		"gta_ped.sps",
		"gta_ped_skin.sps",
		"gta_ped_skin_blendshape.sps",
		"gta_radar.sps",
		"gta_reflect.sps",
		"gta_reflect_alpha.sps",
		"gta_reflect_decal.sps",
		"gta_rmptfx_mesh.sps",
		"gta_spec.sps",
		"gta_spec_alpha.sps",
		"gta_spec_const.sps",
		"gta_spec_decal.sps",
		"gta_spec_reflect.sps",
		"gta_spec_reflect_alpha.sps",
		"gta_spec_reflect_decal.sps",
		"gta_spec_reflect_screendooralpha.sps",
		"gta_spec_screendooralpha.sps",
		"gta_ssa_fence.sps",
		"gta_terrain_va_2lyr.sps",
		"gta_terrain_va_3lyr.sps",
		"gta_terrain_va_4lyr.sps",
		"gta_trees.sps",
		"gta_vehicle_badges.sps",
		"gta_vehicle_basic.sps",
		"gta_vehicle_chrome.sps",
		"gta_vehicle_disc.sps",
		"gta_vehicle_generic.sps",
		"gta_vehicle_interior.sps",
		"gta_vehicle_interior2.sps",
		"gta_vehicle_lights.sps",
		"gta_vehicle_lightsemissive.sps",
		"gta_vehicle_mesh.sps",
		"gta_vehicle_nosplash.sps",
		"gta_vehicle_nowater.sps",
		"gta_vehicle_paint1.sps",
		"gta_vehicle_paint2.sps",
		"gta_vehicle_paint3.sps",
		"gta_vehicle_rims1.sps",
		"gta_vehicle_rims1_alpha.sps",
		"gta_vehicle_rims2.sps",
		"gta_vehicle_rubber.sps",
		"gta_vehicle_shuts.sps",
		"gta_vehicle_tire.sps",
		"gta_vehicle_vehglass.sps",
		"gta_wire.sps",
		"leaves.sps",
		"light_volumes.sps",
		"micromovement.sps",
		"micromovement_default.sps",
		"micromovement_normal.sps",
		"micromovement_normal_spec.sps",
		"micromovement_spec.sps",
		"mirror.sps",
		"normal.sps",
		"normal_wall_rain.sps",
		"normal_grnd_rain.sps",
		"normal_alpha.sps",
		"normal_alpha_grnd_rain.sps",
		"normal_cubemap_reflect.sps",
		"normal_cutout.sps",
		"normal_decal.sps",
		"normal_decal_grnd_rain.sps",
		"normal_reflect.sps",
		"normal_reflect_alpha.sps",
		"normal_reflect_alpha_grnd_rain.sps",
		"normal_reflect_decal.sps",
		"normal_reflect_decal_grnd_rain.sps",
		"normal_reflect_grnd_rain.sps",
		"normal_reflect_screendooralpha.sps",
		"normal_screendooralpha.sps",
		"normal_spec.sps",
		"normal_spec_noreflect.sps",
		"normal_spec_anim_uv.sps",
		"normal_spec_grnd_rain.sps",
		"normal_spec_alpha.sps",
		"normal_spec_alpha_grnd_rain.sps",
		"normal_spec_cubemap_reflect.sps",
		"normal_spec_cutout.sps",
		"normal_spec_decal.sps",
		"normal_spec_noreflect_decal.sps",
		"normal_spec_decal_glue.sps",
		"normal_spec_decal_grnd_rain.sps",
		"normal_spec_reflect.sps",
		"normal_spec_reflect_alpha.sps",
		"normal_spec_reflect_decal.sps",
		"normal_spec_reflect_decal_grnd_rain.sps",
		"normal_spec_reflect_emissive.sps",
		"normal_spec_reflect_emissivenight.sps",
		"normal_spec_reflect_emissivenight_alpha.sps",
		"normal_spec_reflect_emissive_alpha.sps",
		"normal_spec_reflect_grnd_rain.sps",
		"normal_spec_screendooralpha.sps",
		"parallax.sps",
		"parallax_grnd_rain.sps",
		"parallax_reflect_grnd_rain.sps",
		"parallax_specmap.sps",
		"parallax_specmap_grnd_rain.sps",
		"parallax_specmap_reflect_grnd_rain.sps",
		"parallax_steep.sps",
		"ped.sps",
		"ped_ao.sps",
		"ped_blendshape.sps",
		"ped_cloth.sps",
		"ped_cloth_sweat.sps",
		"ped_cpvwind.sps",
		"ped_cpvwind_wrinkle.sps",
		"ped_cpvwind_wrinkle_cloth_sweat.sps",
		"ped_alpha.sps",
		"ped_alpha_blendshape.sps",
		"ped_cutout.sps",
		"ped_cutout_blendshape.sps",
		"ped_decal.sps",
		"ped_default.sps",
		"ped_default_palette.sps",
		"ped_detail.sps",
		"ped_enveff.sps",
		"ped_fur.sps",
		"ped_hair_sorted_alpha.sps",
		"ped_hair_sorted_alpha_blendshape.sps",
		"ped_hair_sorted_alpha_expensive.sps",
		"ped_hair_sorted_alpha_expensive_blendshape.sps",
		"ped_hair_sorted_alpha_cpv.sps",
		"ped_hair_sorted_alpha_cpv_blendshape.sps",
		"ped_hair_sorted_alpha_cpv_bent.sps",
		"ped_hair_sorted_alpha_cpv_bent_blendshape.sps",
		"ped_hair_sorted_ssa.sps",
		"ped_hair_sorted_ssa_blendshape.sps",
		"ped_hair_sorted_ssa_cloth_2sided.sps",
		"ped_hair_sorted_ssa_cpv.sps",
		"ped_hair_sorted_ssa_cpv_bent.sps",
		"ped_hair_sorted_ssa_cpv_bent_blendshape.sps",
		"ped_hair_sorted_ssa_cpv_bent_reflect.sps",
		"ped_hair_sorted_ssa_cpv_blendshape.sps",
		"ped_hair_sorted_ssa_expensive.sps",
		"ped_hair_sorted_ssa_expensive_blendshape.sps",
		"ped_hair_spiked.sps",
		"ped_hair_spiked_blendshape.sps",
		"ped_hair_spiked_ssa.sps",
		"ped_hair_spiked_ssa_blendshape.sps",
		"ped_hair_long_alpha_expensive.sps",
		"ped_hair_long_alpha_expensive_blendshape.sps",
		"ped_hair_long_ssa_expensive.sps",
		"ped_hair_long_ssa_expensive_blendshape.sps",
		"ped_insignia.sps",
		"ped_palette.sps",
		"ped_reflect.sps",
		"ped_reflect_alpha.sps",
		"ped_reflect_wrinkle.sps",
		"ped_reflect_wrinkle_alpha.sps",
		"ped_skin.sps",
		"ped_skin_sweat.sps",
		"ped_skin_blendshape.sps",
		"ped_skin_blendshape_sweat.sps",
		"ped_skin_blendshape_wrinkle.sps",
		"ped_skin_blendshape_wrinkle_sweat.sps",
		"ped_ssa.sps",
		"ped_ssa_blendshape.sps",
		"ped_wrinkle.sps",
		"ped_wrinkle_cloth_sweat.sps",
		"ped_wrinkle_skin_sweat.sps",
		"ped_enveff.sps",
		"poolwater.sps",
		"radar.sps",
		"rage_billboard_nobump.sps",
		"rage_default.sps",
		"reflect.sps",
		"reflect_alpha.sps",
		"reflect_decal.sps",
		"rope_default.sps",
		"rmptfx_mesh.sps",
		"rmptfx_mesh_normal.sps",
		"skin_blendshape.sps",
		"skin_blendshape_normalmap.sps",
		"skin_landbaron.sps",
		"sky_system.sps",
		"spec.sps",
		"spec_alpha.sps",
		"spec_const.sps",
		"spec_decal.sps",
		"spec_reflect.sps",
		"spec_reflect_alpha.sps",
		"spec_reflect_decal.sps",
		"spec_reflect_screendooralpha.sps",
		"spec_screendooralpha.sps",
		"sscope_lense.sps",
		"sscope_crosshair.sps",
		"terrain_cb_2lyr.sps",
		"terrain_cb_2lyr_spec_reflect.sps",
		"terrain_cb_4lyr.sps",
		"terrain_cb_4lyr_grnd_rain.sps",
		"terrain_cb_4lyr_2tex.sps",
		"terrain_cb_4lyr_cm.sps",
		"terrain_cb_4lyr_spec.sps",
		"terrain_cb_4lyr_2tex_spec.sps",
		"terrain_cb_7lyr.sps",
		"terrain_va_2lyr.sps",
		"terrain_va_3lyr.sps",
		"terrain_va_4lyr.sps",
		"tracer_bulletcam.sps",
		"tracer_bullettime_ai.sps",
		"tracer_bullettime_player.sps",
		"tracer_bullettime_player_mp.sps",
		"tracer_default_ai.sps",
		"tracer_default_player.sps",
		"tracer_default_player_mp.sps",
		"tracer_heathaze.sps",
		"trees.sps",
		"true_mirror.sps",
		"vehicle_badges.sps",
		"vehicle_basic.sps",
		"vehicle_chrome.sps",
		"vehicle_disc.sps",
		"vehicle_generic.sps",
		"vehicle_interior.sps",
		"vehicle_interior2.sps",
		"vehicle_lights.sps",
		"vehicle_lightsemissive.sps",
		"vehicle_mesh.sps",
		"vehicle_nosplash.sps",
		"vehicle_nowater.sps",
		"vehicle_paint1.sps",
		"vehicle_paint1_enveff.sps",
		"vehicle_paint1_normal.sps",
		"vehicle_paint1_normal_reflect.sps",
		"vehicle_paint2.sps",
		"vehicle_paint2_enveff.sps",
		"vehicle_paint2_normal.sps",
		"vehicle_paint2_normal_reflect.sps",
		"vehicle_paint3.sps",
		"vehicle_paint3_enveff.sps",
		"vehicle_paint3_normal.sps",
		"vehicle_paint3_normal_reflect.sps",
		"vehicle_rims1.sps",
		"vehicle_rims1_alpha.sps",
		"vehicle_rims2.sps",
		"vehicle_rubber.sps",
		"vehicle_shuts.sps",
		"vehicle_tire.sps",
		"vehicle_track.sps",
		"vehicle_vehglass.sps",
		"velocity_buffer.sps",
		"vehicle_rotorblade.sps",
		"vehicle_rotorblade_droop.sps",
		"vehicle_rotorblade_droop_ssa.sps",
		"weapon_normal_spec_reflect.sps",
		"wire.sps"
	};

	for (auto& sps : spsNames)
	{
		if (HashString(sps) == spsHash)
		{
			if (sps == "normal_spec_noreflect.sps")
			{
				return "normal_spec.sps";
			}
			else if (sps == "micromovement.sps")
			{
				return "trees.sps";
			}
			else if (sps == "micromovement_default.sps")
			{
				return "trees.sps";
			}
			else if (sps == "micromovement_normal.sps")
			{
				return "trees_normal.sps";
			}
			else if (sps == "micromovement_normal_spec.sps")
			{
				return "trees_normal_spec.sps";
			}
			else if (sps == "micromovement_spec.sps")
			{
				return "trees.sps";
			}
			else if (strstr(sps, "grass_object"))
			{
				return "grass.sps";
			}

			return sps;
		}
	}

	return "default.sps";
}

template<>
five::grmShaderGroup* convert(payne::grmShaderGroup* shaderGroup)
{
	auto out = new(false) five::grmShaderGroup;

	auto texDict = shaderGroup->GetTextures();

	if (texDict)
	{
		out->SetTextures(convert<five::pgDictionary<five::grcTexturePC>*>(texDict));
	}

	five::pgPtr<five::grmShaderFx> newShaders[512];
	
	for (int i = 0; i < shaderGroup->GetNumShaders(); i++)
	{
		auto oldShader = shaderGroup->GetShader(i);
		auto oldSpsName = LookupSpsName(oldShader->GetSpsHash());
		auto oldShaderName = LookupShaderName(oldShader->GetShaderHash());
		//auto newSpsName = ConvertSpsName_NY_Five(oldSpsName);
		//auto newShaderName = ConvertShaderName_NY_Five(oldShaderName);

		auto spsFile = fxc::SpsFile::Load(MakeRelativeCitPath(fmt::sprintf(L"citizen\\shaders\\db\\%s", ToWide(oldSpsName))));

		if (spsFile)
		{
			oldShaderName = spsFile->GetShader();
		}

		five::grmShaderFx* newShader = new(false) five::grmShaderFx();
		newShader->DoPreset(oldShaderName.c_str(), oldSpsName.c_str());

		if (spsFile)
		{
			auto db = spsFile->GetParameter("__rage_drawbucket");

			if (db)
			{
				newShader->SetDrawBucket(db->GetInt());
			}
		}
		else
		{
			// no known sps drawbucket, set the original one
			newShader->SetDrawBucket(oldShader->GetDrawBucket());
		}

		
		int rescount = 0;

		auto shaderFile = fxc::ShaderFile::Load(MakeRelativeCitPath(va(L"citizen\\shaders\\win32_40_final\\%s.fxc", ToWide(oldShaderName))));

		auto parameters = oldShader->GetParameters();

		for (int j = 0; j < oldShader->GetParameterCount(); j++)
		{
			auto param = &parameters[j];
			auto hash = param->GetHash();

			uint32_t newSamplerName = 0;
			uint32_t newValueName = 0;
			size_t newValueSize;

			if (hash == 0x2B5170FD) // TextureSampler
			{
				newSamplerName = HashString("DiffuseSampler");

				if (oldSpsName.find("cable") != std::string::npos)
				{
					newSamplerName = HashString("TextureSamp");
				}
			}

			if (!newSamplerName)
			{
				auto newParam = newShader->GetParameter(hash);

				if (newParam)
				{
					if (newParam->IsSampler())
					{
						newSamplerName = hash;
					}
				}
			}

			if (!newSamplerName && !newValueName)
			{
				auto newParam = newShader->GetParameter(hash);

				if (newParam && !newParam->IsSampler())
				{
					for (auto& parameter : shaderFile->GetLocalParameters())
					{
						if (HashString(parameter.first.c_str()) == hash)
						{
							newValueName = hash;
							newValueSize = parameter.second->GetDefaultValue().size();

							break;
						}
					}
				}
			}

			if (newSamplerName)
			{
				payne::grcTexturePC* texture = reinterpret_cast<payne::grcTexturePC*>(param->GetValue());

				const char* textureName = (texture) ? texture->GetName() : "none";

				// look up if we just created one of these as local texture
				bool found = false;

				if (out->GetTextures())
				{
					for (auto& outTexture : *out->GetTextures())
					{
						if (!_stricmp(outTexture.second->GetName(), textureName))
						{
							newShader->SetParameter(newSamplerName, outTexture.second);
							rescount++;
							found = true;

							break;
						}
					}
				}

				// ... it's an external reference
				if (!found)
				{
					newShader->SetParameter(newSamplerName, textureName);
					rescount++;
				}
			}
			else if (newValueName)
			{
				newShader->SetParameter(newValueName, param->GetValue(), newValueSize);
			}
			else if (hash == HashString("specularFactor"))
			{
				float multiplier[4];
				memcpy(multiplier, param->GetValue(), sizeof(multiplier));

				multiplier[0] /= 100.0f;

				if (newShader->GetParameter("specularIntensityMult"))
				{
					newShader->SetParameter("specularIntensityMult", &multiplier, sizeof(multiplier));
				}
			}
		}

		/*if (auto falloffMult = newShader->GetParameter("SpecularFalloffMult"); falloffMult != nullptr)
		{
			float falloffMultValue[] = { *(float*)falloffMult->GetValue() * 4.0f, 0.0f, 0.0f, 0.0f };

			if (oldShaderName.find("decal") == 0)
			{
				falloffMultValue[0] = 100.f;
			}

			memcpy(falloffMult->GetValue(), falloffMultValue, sizeof(falloffMultValue));
		}

		if (auto emissiveMult = newShader->GetParameter("EmissiveMultiplier"); emissiveMult != nullptr)
		{
			float emissiveMultValue[] = { *(float*)emissiveMult->GetValue() / 4.0f, 0.0f, 0.0f, 0.0f };
			memcpy(emissiveMult->GetValue(), emissiveMultValue, sizeof(emissiveMultValue));
		}*/

		if (oldShaderName == "trees")
		{
			float alphaValues[4] = { 0.5f, 0.0f, 0.0f, 0.0f };
			newShader->SetParameter("AlphaTest", alphaValues, sizeof(alphaValues));
		}

		if (newShader->GetResourceCount() != rescount)
		{
			// add missing layer2/3 maps
			if (oldShaderName == "terrain_cb_4lyr_lod")
			{
				int missingEntries = newShader->GetResourceCount() - rescount;

				if (missingEntries >= 1)
				{
					newShader->SetParameter("TextureSampler_layer3", "none");
				}

				if (missingEntries >= 2)
				{
					newShader->SetParameter("TextureSampler_layer2", "none");
				}
			}
			else if (oldShaderName.find("trees") != std::string::npos)
			{
				newShader->SetParameter("SfxWindSampler3D", "none");
			}
			else if (oldShaderName == "glass_normal_spec_reflect" || oldShaderName == "glass_reflect")
			{
				newShader->SetParameter("EnvironmentSampler", "none");
			}
			else if (oldShaderName == "grass")
			{
				newShader->SetParameter("TextureGrassSampler", "none");
			}
			else if (oldShaderName == "glass_emissive")
			{
				newShader->SetParameter("BumpSampler", "none");
				newShader->SetParameter("EnvironmentSampler", "none");
			}
			else
			{
				FatalError("Got %d samplers in the Five shader, but only %d matched from NY...", newShader->GetResourceCount(), rescount);
			}
		}

		// TODO: change other arguments?
		newShaders[i] = newShader;
	}

	out->SetShaders(shaderGroup->GetNumShaders(), newShaders);

	return out;
}

template<>
five::grcIndexBufferD3D* convert(payne::grcIndexBufferD3D* buffer)
{
	return new(false) five::grcIndexBufferD3D(buffer->GetIndexCount(), buffer->GetIndexData());
}

template<>
five::grcVertexFormat* convert(payne::grcVertexFormat* format)
{
	return new(false) five::grcVertexFormat(format->GetMask(), format->GetVertexSize(), format->GetFieldCount(), format->GetFVF());
}

enum class FVFTypeP
{
	Nothing = 0,
	Float16_2,
	Float,
	Float16_4,
	Float_unk,
	Float2,
	Float3,
	Float4,
	UByte4,
	Color,
	Dec3N
};

template<>
five::grcVertexBufferD3D* convert(payne::grcVertexBufferD3D* buffer)
{
	// really hacky fix for multiple vertex buffers sharing the same array
	static std::map<void*, void*> g_vertexBufferMatches;

	void* oldBuffer = buffer->GetVertices();

	if (g_vertexBufferMatches.find(oldBuffer) != g_vertexBufferMatches.end())
	{
		oldBuffer = g_vertexBufferMatches[oldBuffer];
	}
	else
	{
		auto vertexFormat = buffer->GetVertexFormat();
		
		auto mask = vertexFormat->GetMask();

		for (int i = 0; i < buffer->GetCount(); i++)
		{
			char* thisBit = (char*)oldBuffer + (buffer->GetStride() * i);
			size_t off = 0;

			for (int j = 0; j < 32; j++)
			{
				if (vertexFormat->GetMask() & (1 << j))
				{
					uint64_t fvf = vertexFormat->GetFVF();
					auto type = (FVFTypeP)((fvf >> (j * 4)) & 0xF);

					switch (type)
					{
					case FVFTypeP::Float4:
						off += 4;
					case FVFTypeP::Float3:
						off += 4;
					case FVFTypeP::Float2:
						off += 4;
					case FVFTypeP::Nothing:
					case FVFTypeP::Float:
					case FVFTypeP::Float_unk:
						off += 4;
						break;
					case FVFTypeP::Dec3N:
						off += 4;
						break;
					case FVFTypeP::Color:
					case FVFTypeP::UByte4:
						if (j == 4)
						{
							uint8_t* rgba = (uint8_t*)(thisBit + off);

							rgba[1] = uint8_t(rgba[1] * 0.5f); // 50% of original green
						}

						off += 4;
						break;
					case FVFTypeP::Float16_2:
						off += 4;
						break;
					default:
						trace("unknown vertex type?\n");
						break;
					}
				}
			}
		}
	}

	auto out = new(false) five::grcVertexBufferD3D;

	out->SetVertexFormat(convert<five::grcVertexFormat*>(buffer->GetVertexFormat()));
	out->SetVertices(buffer->GetCount(), buffer->GetStride(), oldBuffer);

	g_vertexBufferMatches[buffer->GetVertices()] = out->GetVertices();

	return out;
}

template<>
five::grmGeometryQB* convert(payne::grmGeometryQB* geometry)
{
	auto out = new(false) five::grmGeometryQB;

	out->SetIndexBuffer(convert<five::grcIndexBufferD3D*>(geometry->GetIndexBuffer(0)));
	out->SetVertexBuffer(convert<five::grcVertexBufferD3D*>(geometry->GetVertexBuffer(0)));

	return out;
}

template<>
five::grmModel* convert(payne::grmModel* model)
{
	auto out = new(false) five::grmModel;
	int ni = 0;

	{
		auto& oldGeometries = model->GetGeometries();
		five::grmGeometryQB* geometries[256];

		for (int i = 0; i < oldGeometries.GetCount(); i++)
		{
			g_curGeom = i;

			auto oldie = oldGeometries.Get(i);

			//if (oldie->GetVertexBuffer(0)->GetStride() == 0x24)
			{
				geometries[ni] = convert<five::grmGeometryQB*>(oldie);
				ni++;
			}
		}

		out->SetGeometries(ni, geometries);
	}

	out->SetShaderMappings(/*model->GetShaderMappingCount()*/ni, model->GetShaderMappings());

	return out;
}

template<>
five::gtaDrawable* convert(payne::gtaDrawable* drawable)
{
	auto out = new(false) five::gtaDrawable;

	auto& oldLodGroup = drawable->GetLodGroup();

	out->SetBlockMap();

	out->SetShaderGroup(convert<five::grmShaderGroup*>(drawable->GetShaderGroup()));

	auto& lodGroup = out->GetLodGroup();
	
	Vector3 minBounds = oldLodGroup.GetBoundsMin();
	minBounds = Vector3(minBounds.x - oldLodGroup.GetRadius(), minBounds.y - oldLodGroup.GetRadius(), minBounds.z - oldLodGroup.GetRadius());

	Vector3 maxBounds = oldLodGroup.GetBoundsMax();
	maxBounds = Vector3(maxBounds.x + oldLodGroup.GetRadius(), maxBounds.y + oldLodGroup.GetRadius(), maxBounds.z + oldLodGroup.GetRadius());

	lodGroup.SetBounds(
		minBounds,
		maxBounds,
		oldLodGroup.GetCenter(), oldLodGroup.GetRadius()
	);
	//lodGroup.SetBounds(Vector3(-66.6f, -66.6f, -10.0f), Vector3(66.6f, 66.6f, 10.0f), Vector3(0.0f, 0.0f, 0.0f), 94.8f);

	for (int i = 0; i < 4; i++)
	{
		auto oldModel = oldLodGroup.GetModel(i);

		if (oldModel)
		{
			auto newModel = convert<five::grmModel*>(oldModel);

			lodGroup.SetModel(i, newModel);
			lodGroup.SetDrawBucketMask(i, newModel->CalcDrawBucketMask(out->GetShaderGroup())); // TODO: change this

			{
				payne::GeometryBound* oldBounds = oldModel->GetGeometryBounds();

				if (oldBounds)
				{
					int extraSize = 1;

					if (newModel->GetGeometries().GetCount() > 1)
					{
						extraSize = 1;
					}

					std::vector<five::GeometryBound> geometryBounds(newModel->GetGeometries().GetCount() + extraSize);

					for (int i = 0; i < geometryBounds.size(); i++)
					{
						geometryBounds[i].aabbMin = Vector4(oldBounds[i].aabbMin.x, oldBounds[i].aabbMin.y, oldBounds[i].aabbMin.z, oldBounds[i].aabbMin.w);
						geometryBounds[i].aabbMax = Vector4(oldBounds[i].aabbMax.x, oldBounds[i].aabbMax.y, oldBounds[i].aabbMax.z, oldBounds[i].aabbMax.w);
					}

					newModel->SetGeometryBounds(geometryBounds.size(), &geometryBounds[0]);
				}
			}
		}
	}

	out->SetPrimaryModel();
	out->SetName("paynely.#dr");
	/*
	if (drawable->GetNumLightAttrs())
	{
		std::vector<five::CLightAttr> lightAttrs(drawable->GetNumLightAttrs());

		for (int i = 0; i < lightAttrs.size(); i++)
		{
			payne::CLightAttr& inAttr = *drawable->GetLightAttr(i);
			five::CLightAttr& outAttr = lightAttrs[i];

			outAttr.position[0] = inAttr.position[0];
			outAttr.position[1] = inAttr.position[1];
			outAttr.position[2] = inAttr.position[2];
			outAttr.color[0] = inAttr.color[0];
			outAttr.color[1] = inAttr.color[1];
			outAttr.color[2] = inAttr.color[2];
			outAttr.flashiness = inAttr.flashiness;
			outAttr.intensity = inAttr.lightIntensity;
			outAttr.flags = inAttr.flags;
			outAttr.boneID = inAttr.boneID;
			outAttr.lightType = inAttr.lightType;
			outAttr.groupID = 0;
			outAttr.timeFlags = (inAttr.flags & 64) ? 0b111100'000000'000011'111111 : 0b111111'111111'111111'111111;
			outAttr.falloff = (inAttr.lightFalloff == 0.0f) ? 17.0f : inAttr.lightFalloff;
			outAttr.falloffExponent = 64.f;
			outAttr.cullingPlane = { 0.0f, 0.0f, 1.0f, 200.0f };
			outAttr.shadowBlur = 0;
			outAttr.unk1 = 0;
			outAttr.unk2 = 0;
			outAttr.unk3 = 0;
			outAttr.volumeIntensity = inAttr.volumeIntensity;
			outAttr.volumeSizeScale = inAttr.volumeSize;
			outAttr.volumeOuterColor[0] = inAttr.color[0];
			outAttr.volumeOuterColor[1] = inAttr.color[1];
			outAttr.volumeOuterColor[2] = inAttr.color[2];
			outAttr.lightHash = inAttr.lumHash & 0xFF; // ?
			outAttr.volumeOuterIntensity = inAttr.volumeIntensity;
			outAttr.coronaSize = inAttr.coronaSize;
			outAttr.volumeOuterExponent = 64.f;
			outAttr.lightFadeDistance = inAttr.lightFadeDistance;
			outAttr.shadowFadeDistance = inAttr.shadowFadeDistance;
			outAttr.specularFadeDistance = 0.0f;
			outAttr.volumetricFadeDistance = 0.0f;
			outAttr.shadowNearClip = 0.01f;
			outAttr.coronaIntensity = 0.2f;
			outAttr.coronaZBias = 0.1f;
			outAttr.direction[0] = inAttr.direction[0];
			outAttr.direction[1] = inAttr.direction[1];
			outAttr.direction[2] = inAttr.direction[2];
			outAttr.tangent[0] = inAttr.tangent[0];
			outAttr.tangent[1] = inAttr.tangent[1];
			outAttr.tangent[2] = inAttr.tangent[2];
			outAttr.coneInnerAngle = 5.f;
			outAttr.coneOuterAngle = 60.f;
			outAttr.extents[0] = 1.f;
			outAttr.extents[1] = 1.f;
			outAttr.extents[2] = 1.f;
			outAttr.projectedTextureHash = 0;
			outAttr.unk4 = 0;
		}

		out->SetLightAttrs(&lightAttrs[0], lightAttrs.size());
	}*/

	return out;
}

template<>
five::pgDictionary<five::gtaDrawable>* convert(payne::pgDictionary<payne::gtaDrawable>* dwd)
{
	five::pgDictionary<five::gtaDrawable>* out = new(false) five::pgDictionary<five::gtaDrawable>();
	out->SetBlockMap();

	five::pgDictionary<five::gtaDrawable> newDrawables;

	if (dwd->GetCount()) // amazingly there's 0-sized TXDs?
	{
		for (auto& drawable : *dwd)
		{
			newDrawables.Add(drawable.first, convert<five::gtaDrawable*>(drawable.second));
		}
	}

	out->SetFrom(&newDrawables);

	return out;
}

/*template<>
five::fragType* convert(payne::fragType* frag)
{
	__debugbreak();

	return nullptr;
}*/
}
