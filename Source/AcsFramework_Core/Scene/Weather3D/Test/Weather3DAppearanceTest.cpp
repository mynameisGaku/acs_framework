// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Weather3D/Weather3DAppearance.h"
#include "Common/Test/TestHarness.h"

#include <limits>

void RunWeather3DAppearanceTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FWeather3DAppearance / 既知の天候だけを受け付ける" );

	{
		Harness.Check( FWeather3DAppearance::IsSupportedKind( EWeatherKind::Clear ), "晴天を受け付ける" );
		Harness.Check( FWeather3DAppearance::IsSupportedKind( EWeatherKind::Storm ), "嵐を受け付ける" );
		Harness.Check( FWeather3DAppearance::IsSupportedKind( EWeatherKind::Sandstorm ), "砂嵐を受け付ける" );
		Harness.Check( !FWeather3DAppearance::IsSupportedKind( static_cast<EWeatherKind>( 255u ) ), "未知の値を弾く" );
	}

	Harness.BeginSuite( "FWeather3DAppearance / 晴天は場面の基準を変えない" );

	{
		CWeatherSystem Weather;
		FWeather3DAppearance Appearance;
		Harness.Check( FWeather3DAppearance::TryEvaluate( Weather, Appearance ), "評価できる" );
		Harness.CheckEqualF32( Appearance.EnvironmentLightMultiplier, 1.0f, "環境光を保つ" );
		Harness.CheckEqualF32( Appearance.MinimumCloudCoverage, 0.0f, "雲量を強制しない" );
		Harness.CheckEqualF32( Appearance.CloudDensityMultiplier, 1.0f, "雲の濃さを保つ" );
		Harness.CheckEqualF32( Appearance.ExtraFogDensity, 0.0f, "霧を足さない" );
		Harness.Check( Appearance.IsValid(), "描画へ渡せる" );
	}

	Harness.BeginSuite( "FWeather3DAppearance / 嵐は雲と霧と環境光を同じ状態へ揃える" );

	{
		CWeatherSystem Weather;
		Weather.SetWeather( EWeatherKind::Storm, 0.0f );

		FWeather3DAppearance Appearance;
		Harness.Check( FWeather3DAppearance::TryEvaluate( Weather, Appearance ), "評価できる" );
		Harness.CheckEqualF32( Appearance.EnvironmentLightMultiplier, 0.50f, "環境光を暗くする" );
		Harness.CheckEqualF32( Appearance.ParticleDensity, 2.50f, "降水量を公開する" );
		Harness.CheckEqualF32( Appearance.MinimumCloudCoverage, 0.98f, "空を雲で覆う" );
		Harness.CheckEqualF32( Appearance.CloudDensityMultiplier, 1.75f, "雲を厚くする" );
		Harness.CheckEqualF32( Appearance.FogDensityMultiplier, 2.0f, "遠景を霞ませる" );
		Harness.CheckEqualF32( Appearance.WindStrength, 1.0f, "強風にする" );
	}

	Harness.BeginSuite( "FWeather3DAppearance / 遷移途中も同じ時刻入力から一意に決まる" );

	{
		CWeatherSystem Weather;
		Weather.SetWeather( EWeatherKind::Cloudy, 2.0f );
		Weather.Tick( 1.0f );

		FWeather3DAppearance Appearance;
		Harness.Check( FWeather3DAppearance::TryEvaluate( Weather, Appearance ), "途中を評価できる" );
		Harness.CheckEqualF32( Weather.TransitionT(), 0.5f, "半分進む" );
		Harness.CheckEqualF32( Appearance.EnvironmentLightMultiplier, 0.925f, "ACSの明るさも半分" );
		Harness.CheckEqualF32( Appearance.MinimumCloudCoverage, 0.36f, "Frameworkの雲量も半分" );
		Harness.CheckEqualF32( Appearance.FogColorBlend, 0.09f, "霧色も半分" );
	}

	Harness.BeginSuite( "FWeather3DAppearance / 壊れた状態では出力を変えない" );

	{
		CWeatherSystem Broken;
		Broken.SetWeather( static_cast<EWeatherKind>( 255u ), 0.0f );

		FWeather3DAppearance Output;
		Output.MinimumCloudCoverage = 0.42f;
		Harness.Check( !FWeather3DAppearance::TryEvaluate( Broken, Output ), "未知の天候を評価しない" );
		Harness.CheckEqualF32( Output.MinimumCloudCoverage, 0.42f, "既存出力を保つ" );

		FWeather3DAppearance Invalid;
		Invalid.ExtraFogDensity = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Invalid.IsValid(), "有限でない値を描画へ渡さない" );
	}
}
