// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Visual3D/VisualPreset3D.h"
#include "Common/Test/TestHarness.h"

void RunVisualPreset3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "EVisualPreset3D / 軽量設定は高価な画面空間効果を切る" );

	{
		FScene3DAmbientOcclusion AmbientOcclusion;
		FScene3DReflections Reflections;
		FScene3DGlobalIllumination GlobalIllumination;
		FPostProcessParams PostProcess;
		Harness.Check( TryApplyVisualPreset3DSettings( EVisualPreset3D::Performance,
			AmbientOcclusion, Reflections, GlobalIllumination, PostProcess ), "軽量設定を適用できる" );
		Harness.CheckEqualF32( AmbientOcclusion.Intensity, 0.75f, "軽い遮蔽を残す" );
		Harness.CheckEqualF32( Reflections.Intensity, 0.0f, "画面空間反射を切る" );
		Harness.CheckEqualF32( GlobalIllumination.Intensity, 0.0f, "画面空間間接光を切る" );
		Harness.Check( !PostProcess.taa_enabled, "TAAを切る" );
		Harness.Check( PostProcess.fxaa_enabled, "軽い輪郭補正を使う" );
		Harness.CheckEqualF32( PostProcess.grain_intensity, 0.0f, "粒状効果を切る" );
	}

	Harness.BeginSuite( "EVisualPreset3D / 標準設定は見た目と負荷を両立する" );

	{
		FScene3DAmbientOcclusion AmbientOcclusion;
		FScene3DReflections Reflections;
		FScene3DGlobalIllumination GlobalIllumination;
		FPostProcessParams PostProcess;
		PostProcess.grain_time = 7.25f;
		PostProcess.delta_time = 0.02f;
		PostProcess.ssr_intensity = 0.73f;
		Harness.Check( TryApplyVisualPreset3DSettings( EVisualPreset3D::Balanced,
			AmbientOcclusion, Reflections, GlobalIllumination, PostProcess ), "標準設定を適用できる" );
		Harness.CheckEqualF32( AmbientOcclusion.Intensity, 1.0f, "接地感を保つ" );
		Harness.CheckEqualF32( AmbientOcclusion.Radius, 0.50f, "人物規模の遮蔽半径にする" );
		Harness.CheckEqualF32( Reflections.Intensity, 0.35f, "反射を控えめに使う" );
		Harness.CheckEqualF32( GlobalIllumination.Intensity, 0.45f, "色の回り込みを控えめに使う" );
		Harness.Check( !PostProcess.taa_enabled && PostProcess.fxaa_enabled, "履歴を持たない輪郭補正を使う" );
		Harness.CheckEqualF32( PostProcess.grain_time, 7.25f, "ACSが持つ粒状効果の時刻を保つ" );
		Harness.CheckEqualF32( PostProcess.delta_time, 0.02f, "ACSが持つフレーム時間を保つ" );
		Harness.CheckEqualF32( PostProcess.ssr_intensity, 0.73f, "ACSが描画時に使う反射合成値を保つ" );
	}

	Harness.BeginSuite( "EVisualPreset3D / 映画調設定は高品質効果をまとめて有効にする" );

	{
		FScene3DAmbientOcclusion AmbientOcclusion;
		FScene3DReflections Reflections;
		FScene3DGlobalIllumination GlobalIllumination;
		FPostProcessParams PostProcess;
		Harness.Check( TryApplyVisualPreset3DSettings( EVisualPreset3D::Cinematic,
			AmbientOcclusion, Reflections, GlobalIllumination, PostProcess ), "映画調設定を適用できる" );
		Harness.CheckEqualF32( Reflections.Intensity, 0.60f, "反射を強める" );
		Harness.CheckEqualF32( GlobalIllumination.Intensity, 0.75f, "間接光を強める" );
		Harness.CheckEqualF32( GlobalIllumination.MaxDistance, 7.0f, "間接光の探索距離を広げる" );
		Harness.Check( PostProcess.taa_enabled, "TAAを使う" );
		Harness.Check( PostProcess.fxaa_enabled, "TAA失敗時の代替処理を残す" );
		Harness.CheckEqualF32( PostProcess.cg_contrast, 1.06f, "コントラストをわずかに上げる" );
		Harness.CheckEqualF32( PostProcess.vignette_intensity, 0.14f, "画面端を穏やかに締める" );
	}

	Harness.BeginSuite( "EVisualPreset3D / 切替と失敗を原子的に扱う" );

	{
		FScene3DAmbientOcclusion AmbientOcclusion;
		FScene3DReflections Reflections;
		FScene3DGlobalIllumination GlobalIllumination;
		FPostProcessParams PostProcess;
		Harness.Check( TryApplyVisualPreset3DSettings( EVisualPreset3D::Cinematic,
			AmbientOcclusion, Reflections, GlobalIllumination, PostProcess ), "切替前の映画調設定を適用できる" );
		Harness.Check( TryApplyVisualPreset3DSettings( EVisualPreset3D::Performance,
			AmbientOcclusion, Reflections, GlobalIllumination, PostProcess ), "映画調から軽量設定へ切り替えられる" );
		Harness.CheckEqualF32( Reflections.Intensity, 0.0f, "反射を残さない" );
		Harness.CheckEqualF32( PostProcess.chromatic_aberration, 0.0f, "色収差を残さない" );
		Harness.CheckEqualF32( PostProcess.cg_contrast, 1.0f, "色調を中立へ戻す" );
		Harness.Check( !PostProcess.taa_enabled, "TAAを残さない" );

		AmbientOcclusion.Intensity = 3.25f;
		Reflections.Intensity = 2.25f;
		GlobalIllumination.Intensity = 1.75f;
		PostProcess.bloom_intensity = 1.25f;
		Harness.Check( !TryApplyVisualPreset3DSettings( static_cast<EVisualPreset3D>( 255u ),
			AmbientOcclusion, Reflections, GlobalIllumination, PostProcess ), "未知のプリセットを拒否する" );
		Harness.CheckEqualF32( AmbientOcclusion.Intensity, 3.25f, "遮蔽設定を維持する" );
		Harness.CheckEqualF32( Reflections.Intensity, 2.25f, "反射設定を維持する" );
		Harness.CheckEqualF32( GlobalIllumination.Intensity, 1.75f, "間接光設定を維持する" );
		Harness.CheckEqualF32( PostProcess.bloom_intensity, 1.25f, "ポスト処理設定を維持する" );
	}
}
