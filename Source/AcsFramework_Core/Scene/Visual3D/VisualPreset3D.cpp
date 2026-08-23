// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Visual3D/VisualPreset3D.h"

namespace
{
	/** 全プリセットが管理する共通の色調と露出を中立値へ揃える。 */
	void ApplyCommonPostControls_Internal( FPostProcessParams& PostProcess ) noexcept
	{
		PostProcess.bloom_enabled = true;
		PostProcess.gamma = 2.2f;
		PostProcess.tonemap_kind = 0;
		PostProcess.cg_saturation = 1.0f;
		PostProcess.cg_contrast = 1.0f;
		PostProcess.cg_temperature = 0.0f;
		PostProcess.cg_tint = 0.0f;
		PostProcess.cg_lift = FVec3{};
		PostProcess.cg_gain = FVec3{ 1.0f, 1.0f, 1.0f };
		PostProcess.taa_blend_factor = 0.1f;
		PostProcess.auto_exposure_enabled = true;
		PostProcess.auto_exposure_key = 0.5f;
		PostProcess.auto_exposure_min = 0.05f;
		PostProcess.auto_exposure_max = 12.0f;
		PostProcess.auto_exposure_speed = 1.8f;
		PostProcess.fxaa_enabled = true;
	}

	/** 軽量プリセットの描画設定を候補へ書き込む。 */
	void ApplyPerformancePreset_Internal( FScene3DAmbientOcclusion& AmbientOcclusion,
		FScene3DReflections& Reflections, FScene3DGlobalIllumination& GlobalIllumination,
		FPostProcessParams& PostProcess ) noexcept
	{
		AmbientOcclusion.Intensity = 0.75f;
		AmbientOcclusion.Radius = 0.40f;
		Reflections.Intensity = 0.0f;
		GlobalIllumination.Intensity = 0.0f;
		GlobalIllumination.MaxDistance = 5.0f;

		PostProcess.bloom_threshold = 1.25f;
		PostProcess.bloom_intensity = 0.22f;
		PostProcess.bloom_radius = 0.72f;
		PostProcess.bloom_scatter = 0.55f;
		PostProcess.exposure = 0.92f;
		PostProcess.vignette_intensity = 0.04f;
		PostProcess.vignette_radius = 0.90f;
		PostProcess.chromatic_aberration = 0.0f;
		PostProcess.grain_intensity = 0.0f;
		PostProcess.cas_strength = 0.24f;
		PostProcess.taa_enabled = false;
		PostProcess.auto_exposure_max = 8.0f;
		PostProcess.auto_exposure_speed = 2.2f;
	}

	/** 標準プリセットの描画設定を候補へ書き込む。 */
	void ApplyBalancedPreset_Internal( FScene3DAmbientOcclusion& AmbientOcclusion,
		FScene3DReflections& Reflections, FScene3DGlobalIllumination& GlobalIllumination,
		FPostProcessParams& PostProcess ) noexcept
	{
		AmbientOcclusion.Intensity = 1.0f;
		AmbientOcclusion.Radius = 0.50f;
		Reflections.Intensity = 0.35f;
		GlobalIllumination.Intensity = 0.45f;
		GlobalIllumination.MaxDistance = 5.0f;

		PostProcess.bloom_threshold = 1.10f;
		PostProcess.bloom_intensity = 0.34f;
		PostProcess.bloom_radius = 0.85f;
		PostProcess.bloom_scatter = 0.64f;
		PostProcess.exposure = 0.92f;
		PostProcess.vignette_intensity = 0.075f;
		PostProcess.vignette_radius = 0.84f;
		PostProcess.chromatic_aberration = 0.0f;
		PostProcess.grain_intensity = 0.002f;
		PostProcess.cas_strength = 0.18f;
		PostProcess.taa_enabled = false;
	}

	/** 高品質プリセットの描画設定を候補へ書き込む。 */
	void ApplyCinematicPreset_Internal( FScene3DAmbientOcclusion& AmbientOcclusion,
		FScene3DReflections& Reflections, FScene3DGlobalIllumination& GlobalIllumination,
		FPostProcessParams& PostProcess ) noexcept
	{
		AmbientOcclusion.Intensity = 1.05f;
		AmbientOcclusion.Radius = 0.65f;
		Reflections.Intensity = 0.60f;
		GlobalIllumination.Intensity = 0.75f;
		GlobalIllumination.MaxDistance = 7.0f;

		PostProcess.bloom_threshold = 0.90f;
		PostProcess.bloom_intensity = 0.52f;
		PostProcess.bloom_radius = 1.0f;
		PostProcess.bloom_scatter = 0.72f;
		PostProcess.exposure = 0.94f;
		PostProcess.vignette_intensity = 0.14f;
		PostProcess.vignette_radius = 0.74f;
		PostProcess.chromatic_aberration = 0.0008f;
		PostProcess.grain_intensity = 0.006f;
		PostProcess.cg_saturation = 1.03f;
		PostProcess.cg_contrast = 1.06f;
		PostProcess.cg_temperature = 0.02f;
		PostProcess.cas_strength = 0.14f;
		PostProcess.taa_enabled = true;
		PostProcess.auto_exposure_key = 0.48f;
		PostProcess.auto_exposure_min = 0.04f;
		PostProcess.auto_exposure_speed = 1.4f;
	}
}


bool TryApplyVisualPreset3DSettings( EVisualPreset3D Preset,
	FScene3DAmbientOcclusion& AmbientOcclusion,
	FScene3DReflections& Reflections,
	FScene3DGlobalIllumination& GlobalIllumination,
	FPostProcessParams& PostProcess ) noexcept
{
	FScene3DAmbientOcclusion NextAmbientOcclusion = AmbientOcclusion;
	FScene3DReflections NextReflections = Reflections;
	FScene3DGlobalIllumination NextGlobalIllumination = GlobalIllumination;
	FPostProcessParams NextPostProcess = PostProcess;
	ApplyCommonPostControls_Internal( NextPostProcess );

	switch ( Preset )
	{
	case EVisualPreset3D::Performance:
		ApplyPerformancePreset_Internal( NextAmbientOcclusion, NextReflections,
			NextGlobalIllumination, NextPostProcess );
		break;
	case EVisualPreset3D::Balanced:
		ApplyBalancedPreset_Internal( NextAmbientOcclusion, NextReflections,
			NextGlobalIllumination, NextPostProcess );
		break;
	case EVisualPreset3D::Cinematic:
		ApplyCinematicPreset_Internal( NextAmbientOcclusion, NextReflections,
			NextGlobalIllumination, NextPostProcess );
		break;
	default:
		return false;
	}

	AmbientOcclusion = NextAmbientOcclusion;
	Reflections = NextReflections;
	GlobalIllumination = NextGlobalIllumination;
	PostProcess = NextPostProcess;
	return true;
}
