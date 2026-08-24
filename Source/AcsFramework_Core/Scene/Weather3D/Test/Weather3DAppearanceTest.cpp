// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Weather3D/Weather3DAppearance.h"
#include "AcsFramework_Core/Scene/Weather3D/TimeOfDay3DAppearance.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>

namespace
{
	/** 浮動小数の小さな誤差を許して比較する。 */
	void CheckNear( CTestHarness& Harness, f32 Actual, f32 Expected, const char* Label ) noexcept
	{
		Harness.Check( std::abs( Actual - Expected ) < 0.0001f, Label );
	}
}

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

	Harness.BeginSuite( "FTimeOfDay3DAppearance / ACSの正午を昼の3D環境へ変換する" );

	{
		CAmbientDirector Director;
		Director.SetTimeOfDay( 12.0f );
		FTimeOfDay3DAppearance Appearance;
		Harness.Check( FTimeOfDay3DAppearance::TryEvaluate(
			Director, 0.0f, Appearance ), "正午を評価できる" );
		Harness.Check( Appearance.IsValid(), "全ての描画値が有効" );
		CheckNear( Harness, Appearance.SunDirectionToLight.y, 1.0f, "太陽が天頂に来る" );
		CheckNear( Harness, Appearance.SunIntensity, 1.6f, "昼の太陽強度を使う" );
		CheckNear( Harness, Appearance.EnvironmentLightMultiplier, 1.0f, "昼の環境光を基準値にする" );
		Harness.Check( Appearance.SkyZenith.z > Appearance.SkyZenith.x, "天頂は青を強くする" );

		CSky Sky;
		Sky.SetSunDirection( FVec3{ 1.0f, 0.0f, 0.0f } );
		Sky.SetSunColor( FVec3{ 0.1f, 0.2f, 0.3f } );
		Harness.Check( Appearance.TryApplySunTo( Sky ), "空の太陽へ反映できる" );
		CheckNear( Harness, Sky.SunDirection().x, Appearance.SunDirectionToLight.x, "空と方向光のX方向を揃える" );
		CheckNear( Harness, Sky.SunDirection().y, Appearance.SunDirectionToLight.y, "空と方向光のY方向を揃える" );
		CheckNear( Harness, Sky.SunDirection().z, Appearance.SunDirectionToLight.z, "空と方向光のZ方向を揃える" );
		CheckNear( Harness, Sky.SunColor().x, Appearance.SunColor.x, "空と方向光の赤を揃える" );
		CheckNear( Harness, Sky.SunColor().y, Appearance.SunColor.y, "空と方向光の緑を揃える" );
		CheckNear( Harness, Sky.SunColor().z, Appearance.SunColor.z, "空と方向光の青を揃える" );
	}

	Harness.BeginSuite( "FTimeOfDay3DAppearance / 太陽軌道の方位を場面へ合わせる" );

	{
		CAmbientDirector Director;
		Director.SetTimeOfDay( 6.0f );
		FTimeOfDay3DAppearance Appearance;
		Harness.Check( FTimeOfDay3DAppearance::TryEvaluate(
			Director, 90.0f, Appearance ), "日の出と方位を評価できる" );
		CheckNear( Harness, Appearance.SunDirectionToLight.x, 0.0f, "東西軌道をX軸から回す" );
		CheckNear( Harness, Appearance.SunDirectionToLight.y, 0.0f, "日の出は地平線上" );
		CheckNear( Harness, Appearance.SunDirectionToLight.z, -1.0f, "正角度は+Xから-Zへ回す" );
		Harness.Check( Appearance.SkyHorizon.x > Appearance.SkyHorizon.z, "日の出の地平線を暖色にする" );
		Harness.Check( Appearance.SunIntensity > 0.0f && Appearance.SunIntensity < 0.001f,
			"地平線では既定太陽へ戻らない微小光だけを残す" );
	}

	Harness.BeginSuite( "FTimeOfDay3DAppearance / 夜は昼より直射光と環境光を暗くする" );

	{
		CAmbientDirector Director;
		FTimeOfDay3DAppearance Day;
		FTimeOfDay3DAppearance Night;
		Director.SetTimeOfDay( 12.0f );
		const bool bDay = FTimeOfDay3DAppearance::TryEvaluate( Director, 0.0f, Day );
		Director.SetTimeOfDay( 0.0f );
		const bool bNight = FTimeOfDay3DAppearance::TryEvaluate( Director, 0.0f, Night );
		Harness.Check( bDay && bNight, "昼夜を同じ入力規則で評価できる" );
		Harness.Check( Night.SunIntensity < Day.SunIntensity, "夜の直射光を弱くする" );
		Harness.Check( Night.EnvironmentLightMultiplier < Day.EnvironmentLightMultiplier,
			"夜の環境光を弱くする" );
		Harness.Check( Night.SkyZenith.z > Night.SkyZenith.x, "夜の天頂も青みを保つ" );
	}

	Harness.BeginSuite( "FTimeOfDay3DAppearance / 不正入力では出力を変えない" );

	{
		CAmbientDirector Director;
		FTimeOfDay3DAppearance Output;
		Output.SunIntensity = 0.42f;
		const f32 QuietNaN = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !FTimeOfDay3DAppearance::TryEvaluate( Director, QuietNaN, Output ),
			"有限でない方位を拒否する" );
		CheckNear( Harness, Output.SunIntensity, 0.42f, "失敗時は既存出力を保つ" );

		Director.SetTimeOfDay( QuietNaN );
		Harness.Check( !FTimeOfDay3DAppearance::TryEvaluate( Director, 0.0f, Output ),
			"壊れたACS時刻を描画へ渡さない" );
		CheckNear( Harness, Output.SunIntensity, 0.42f, "壊れた時刻でも出力を保つ" );

		CSky Sky;
		Sky.SetSunDirection( FVec3{ 1.0f, 0.0f, 0.0f } );
		Sky.SetSunColor( FVec3{ 0.1f, 0.2f, 0.3f } );
		FTimeOfDay3DAppearance InvalidAppearance;
		InvalidAppearance.SunIntensity = QuietNaN;
		Harness.Check( !InvalidAppearance.TryApplySunTo( Sky ), "壊れた見た目を空へ反映しない" );
		CheckNear( Harness, Sky.SunDirection().x, 1.0f, "失敗時は空の太陽方向を保つ" );
		CheckNear( Harness, Sky.SunColor().x, 0.1f, "失敗時は空の太陽色を保つ" );
	}
}
