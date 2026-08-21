// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Weather3D/Weather3DScene.h"

#include <cmath>

namespace
{
	/** 3成分が有限か返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}

	/** 有限で負成分を持たない色か返す。 */
	bool IsNonNegativeColor( FVec3 Value ) noexcept
	{
		return IsFinite( Value ) && Value.x >= 0.0f && Value.y >= 0.0f && Value.z >= 0.0f;
	}

	/** 2つの数値を指定割合で混ぜる。 */
	f32 Blend( f32 From, f32 To, f32 Amount ) noexcept
	{
		return From + ( To - From ) * Amount;
	}

	/** 2つの色を指定割合で混ぜる。 */
	FVec3 Blend( FVec3 From, FVec3 To, f32 Amount ) noexcept
	{
		return FVec3
		{
			Blend( From.x, To.x, Amount ),
			Blend( From.y, To.y, Amount ),
			Blend( From.z, To.z, Amount ),
		};
	}

	/** 2つの色を成分ごとに掛ける。 */
	FVec3 Multiply( FVec3 Left, FVec3 Right ) noexcept
	{
		return FVec3{ Left.x * Right.x, Left.y * Right.y, Left.z * Right.z };
	}
}


bool AWeather3DScene::FEnvironmentBaseline::IsValid() const noexcept
{
	if ( !std::isfinite( CloudCoverage ) || CloudCoverage < 0.0f || CloudCoverage > 1.0f ) return false;
	if ( !std::isfinite( CloudDensity ) || CloudDensity < 0.0f ) return false;
	if ( !std::isfinite( CloudWind ) ) return false;
	if ( !IsNonNegativeColor( FogColor ) || !std::isfinite( FogDensity ) || FogDensity < 0.0f ) return false;
	if ( !IsNonNegativeColor( SkyZenith ) || !IsNonNegativeColor( SkyHorizon ) || !IsNonNegativeColor( SkyGround ) ) return false;
	if ( !std::isfinite( EnvironmentLightMultiplier ) || EnvironmentLightMultiplier < 0.0f ) return false;

	return true;
}


bool AWeather3DScene::SetWeather( EWeatherKind Kind, f32 TransitionSeconds ) noexcept
{
	if ( !FWeather3DAppearance::IsSupportedKind( Kind ) ) return false;
	if ( !std::isfinite( TransitionSeconds ) || TransitionSeconds < 0.0f ) return false;

	m_Weather.SetWeather( Kind, TransitionSeconds );
	FWeather3DAppearance Candidate;
	if ( FWeather3DAppearance::TryEvaluate( m_Weather, Candidate ) )
	{
		m_Appearance = Candidate;
		if ( m_bEnvironmentCaptured ) ApplyWeather_Internal();
	}

	return true;
}


bool AWeather3DScene::SetWeatherWindDirection( FVec2 Direction ) noexcept
{
	if ( !std::isfinite( Direction.x ) || !std::isfinite( Direction.y ) ) return false;
	const f32 LengthSquared = Direction.x * Direction.x + Direction.y * Direction.y;
	if ( !std::isfinite( LengthSquared ) || LengthSquared <= 0.000001f ) return false;

	const f32 InverseLength = 1.0f / std::sqrt( LengthSquared );
	m_Weather.SetWindDirection( FVec2{ Direction.x * InverseLength, Direction.y * InverseLength } );
	return true;
}


void AWeather3DScene::OnEnter() noexcept
{
	AEffect3DScene::OnEnter();
	m_Weather.Reset();
	m_Appearance = FWeather3DAppearance{};
	m_Baseline = FEnvironmentBaseline{};
	m_bEnvironmentCaptured = false;
}


void AWeather3DScene::OnExit() noexcept
{
	RestoreEnvironment_Internal();
	m_bEnvironmentCaptured = false;
	AEffect3DScene::OnExit();
}


void AWeather3DScene::OnUpdate( f32 DeltaSeconds ) noexcept
{
	AEffect3DScene::OnUpdate( DeltaSeconds );
	if ( !m_bEnvironmentCaptured && !CaptureEnvironment_Internal() ) return;

	const f32 SafeDeltaSeconds = std::isfinite( DeltaSeconds ) && DeltaSeconds > 0.0f ? DeltaSeconds : 0.0f;
	m_Weather.Tick( SafeDeltaSeconds );

	FWeather3DAppearance Candidate;
	if ( !FWeather3DAppearance::TryEvaluate( m_Weather, Candidate ) ) return;

	m_Appearance = Candidate;
	ApplyWeather_Internal();
}


bool AWeather3DScene::CaptureEnvironment_Internal() noexcept
{
	const FScene3DClouds& CurrentClouds = Clouds();
	const FScene3DFog& CurrentFog = Fog();
	const CSky& CurrentSky = Sky();

	FEnvironmentBaseline Candidate;
	Candidate.CloudCoverage = CurrentClouds.Coverage;
	Candidate.CloudDensity = CurrentClouds.Density;
	Candidate.CloudWind = CurrentClouds.Wind;
	Candidate.FogColor = CurrentFog.Color;
	Candidate.FogDensity = CurrentFog.Density;
	Candidate.SkyZenith = CurrentSky.ZenithColor();
	Candidate.SkyHorizon = CurrentSky.HorizonColor();
	Candidate.SkyGround = CurrentSky.GroundColor();
	Candidate.EnvironmentLightMultiplier = EnvironmentLightMultiplier();
	if ( !Candidate.IsValid() ) return false;

	m_Baseline = Candidate;
	m_bEnvironmentCaptured = true;
	return true;
}


void AWeather3DScene::ApplyWeather_Internal() noexcept
{
	if ( !m_bEnvironmentCaptured || !m_Appearance.IsValid() ) return;

	const f32 CloudCoverage = m_Baseline.CloudCoverage > m_Appearance.MinimumCloudCoverage
		? m_Baseline.CloudCoverage : m_Appearance.MinimumCloudCoverage;
	const f32 CloudDensity = m_Baseline.CloudDensity * m_Appearance.CloudDensityMultiplier;
	const f32 WindScale = 1.0f + ( m_Appearance.WindStrength - 0.10f ) * 2.5f;
	const f32 CloudWind = m_Baseline.CloudWind * ( WindScale > 0.0f ? WindScale : 0.0f );
	const FVec3 FogColor = Blend( m_Baseline.FogColor, m_Appearance.FogColorTarget, m_Appearance.FogColorBlend );
	const f32 FogDensity = m_Baseline.FogDensity * m_Appearance.FogDensityMultiplier + m_Appearance.ExtraFogDensity;
	const FVec3 SkyZenith = Multiply( m_Baseline.SkyZenith, m_Appearance.SkyTint );
	const FVec3 SkyHorizon = Multiply( m_Baseline.SkyHorizon, m_Appearance.SkyTint );
	const FVec3 SkyGround = Multiply( m_Baseline.SkyGround, m_Appearance.SkyTint );
	const f32 EnvironmentLight = m_Baseline.EnvironmentLightMultiplier * m_Appearance.EnvironmentLightMultiplier;

	if ( !std::isfinite( CloudCoverage ) || CloudCoverage < 0.0f || CloudCoverage > 1.0f ) return;
	if ( !std::isfinite( CloudDensity ) || CloudDensity < 0.0f || !std::isfinite( CloudWind ) ) return;
	if ( !IsNonNegativeColor( FogColor ) || !std::isfinite( FogDensity ) || FogDensity < 0.0f ) return;
	if ( !IsNonNegativeColor( SkyZenith ) || !IsNonNegativeColor( SkyHorizon ) || !IsNonNegativeColor( SkyGround ) ) return;
	if ( !std::isfinite( EnvironmentLight ) || EnvironmentLight < 0.0f ) return;

	FScene3DClouds& CurrentClouds = Clouds();
	CurrentClouds.Coverage = CloudCoverage;
	CurrentClouds.Density = CloudDensity;
	CurrentClouds.Wind = CloudWind;
	FScene3DFog& CurrentFog = Fog();
	CurrentFog.Color = FogColor;
	CurrentFog.Density = FogDensity;
	Sky().SetZenithColor( SkyZenith );
	Sky().SetHorizonColor( SkyHorizon );
	Sky().SetGroundColor( SkyGround );
	SetEnvironmentLightMultiplier( EnvironmentLight );
}


void AWeather3DScene::RestoreEnvironment_Internal() noexcept
{
	if ( !m_bEnvironmentCaptured ) return;

	Clouds().Coverage = m_Baseline.CloudCoverage;
	Clouds().Density = m_Baseline.CloudDensity;
	Clouds().Wind = m_Baseline.CloudWind;
	Fog().Color = m_Baseline.FogColor;
	Fog().Density = m_Baseline.FogDensity;
	Sky().SetZenithColor( m_Baseline.SkyZenith );
	Sky().SetHorizonColor( m_Baseline.SkyHorizon );
	Sky().SetGroundColor( m_Baseline.SkyGround );
	SetEnvironmentLightMultiplier( m_Baseline.EnvironmentLightMultiplier );
}
