// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Weather3D/Weather3DScene.h"

#include "AcsFramework_Core/Scene/Light3D/Light3DSpawner.h"

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


bool AWeather3DScene::EnableTimeOfDay3D( f32 InitialHour,
	f32 GameHoursPerRealSecond, f32 SunAzimuthDegrees ) noexcept
{
	if ( !m_bSceneEntered || !std::isfinite( InitialHour ) || !std::isfinite( GameHoursPerRealSecond )
		|| GameHoursPerRealSecond < 0.0f || !std::isfinite( SunAzimuthDegrees ) ) return false;

	CAmbientDirector CandidateDirector;
	CandidateDirector.SetTimeOfDay( InitialHour );
	CandidateDirector.SetTimeScale( GameHoursPerRealSecond );
	FTimeOfDay3DAppearance CandidateAppearance;
	if ( !FTimeOfDay3DAppearance::TryEvaluate(
		CandidateDirector, SunAzimuthDegrees, CandidateAppearance ) ) return false;

	if ( m_bTimeOfDayEnabled )
	{
		if ( !ApplyTimeOfDaySun_Internal( CandidateAppearance ) ) return false;
	}
	else
	{
		FLight3DSpawnParams SunParams = FLight3DSpawnParams::Sun(
			CandidateAppearance.SunDirectionToLight,
			CandidateAppearance.SunColor, CandidateAppearance.SunIntensity );
		SunParams.Name = FStringView( "TimeOfDaySun" );
		ANode* const Sun = CLight3DSpawner::SpawnInto( Graph(), SunParams );
		if ( Sun == nullptr ) return false;

		const FNodeId SunId = Graph().IdOf( Sun );
		if ( !SunId.IsValid() )
		{
			(void)Graph().Destroy( Sun->Id() );
			return false;
		}
		m_TimeOfDaySun = SunId;
	}

	m_TimeOfDay.SetTimeOfDay( InitialHour );
	m_TimeOfDay.SetTimeScale( GameHoursPerRealSecond );
	m_TimeOfDaySunAzimuthDegrees = SunAzimuthDegrees;
	m_TimeOfDayAppearance = CandidateAppearance;
	m_bTimeOfDayEnabled = true;
	if ( m_bEnvironmentCaptured ) ApplyWeather_Internal();
	return true;
}


void AWeather3DScene::DisableTimeOfDay3D() noexcept
{
	if ( !m_bTimeOfDayEnabled ) return;

	DestroyTimeOfDaySun_Internal();
	m_bTimeOfDayEnabled = false;
	if ( m_bEnvironmentCaptured ) ApplyWeather_Internal();
}


bool AWeather3DScene::SetTimeOfDay3D( f32 Hours ) noexcept
{
	if ( !m_bTimeOfDayEnabled || !std::isfinite( Hours ) ) return false;

	CAmbientDirector CandidateDirector;
	CandidateDirector.SetTimeOfDay( Hours );
	FTimeOfDay3DAppearance CandidateAppearance;
	if ( !FTimeOfDay3DAppearance::TryEvaluate(
		CandidateDirector, m_TimeOfDaySunAzimuthDegrees, CandidateAppearance ) ) return false;
	if ( !ApplyTimeOfDaySun_Internal( CandidateAppearance ) ) return false;

	m_TimeOfDay.SetTimeOfDay( Hours );
	m_TimeOfDayAppearance = CandidateAppearance;
	if ( m_bEnvironmentCaptured ) ApplyWeather_Internal();
	return true;
}


bool AWeather3DScene::SetTimeOfDayRate3D( f32 GameHoursPerRealSecond ) noexcept
{
	if ( !m_bTimeOfDayEnabled || !std::isfinite( GameHoursPerRealSecond )
		|| GameHoursPerRealSecond < 0.0f ) return false;

	m_TimeOfDay.SetTimeScale( GameHoursPerRealSecond );
	return true;
}


bool AWeather3DScene::SetTimeOfDaySunAzimuth3D( f32 SunAzimuthDegrees ) noexcept
{
	if ( !m_bTimeOfDayEnabled || !std::isfinite( SunAzimuthDegrees ) ) return false;

	FTimeOfDay3DAppearance CandidateAppearance;
	if ( !FTimeOfDay3DAppearance::TryEvaluate(
		m_TimeOfDay, SunAzimuthDegrees, CandidateAppearance ) ) return false;
	if ( !ApplyTimeOfDaySun_Internal( CandidateAppearance ) ) return false;

	m_TimeOfDaySunAzimuthDegrees = SunAzimuthDegrees;
	m_TimeOfDayAppearance = CandidateAppearance;
	if ( m_bEnvironmentCaptured ) ApplyWeather_Internal();
	return true;
}


bool AWeather3DScene::FEnvironmentBaseline::IsValid() const noexcept
{
	if ( !std::isfinite( CloudCoverage ) || CloudCoverage < 0.0f || CloudCoverage > 1.0f ) return false;
	if ( !std::isfinite( CloudDensity ) || CloudDensity < 0.0f ) return false;
	if ( !std::isfinite( CloudWind ) ) return false;
	if ( !IsNonNegativeColor( FogColor ) || !std::isfinite( FogDensity ) || FogDensity < 0.0f ) return false;
	if ( !IsNonNegativeColor( SkyZenith ) || !IsNonNegativeColor( SkyHorizon ) || !IsNonNegativeColor( SkyGround ) ) return false;
	if ( !IsFinite( SkySunDirection ) || LengthSq( SkySunDirection ) <= 0.000001f ) return false;
	if ( !IsNonNegativeColor( SkySunColor ) ) return false;
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
	m_bSceneEntered = true;
	m_Weather.Reset();
	m_Appearance = FWeather3DAppearance{};
	m_TimeOfDay.SetTimeOfDay( 12.0f );
	m_TimeOfDay.SetTimeScale( 1.0f / 60.0f );
	m_TimeOfDayAppearance = FTimeOfDay3DAppearance{};
	m_Baseline = FEnvironmentBaseline{};
	m_bEnvironmentCaptured = false;
	m_bTimeOfDayEnabled = false;
	m_TimeOfDaySun = FNodeId{};
	m_TimeOfDaySunAzimuthDegrees = 0.0f;
}


void AWeather3DScene::OnExit() noexcept
{
	RestoreEnvironment_Internal();
	DestroyTimeOfDaySun_Internal();
	m_bEnvironmentCaptured = false;
	m_bTimeOfDayEnabled = false;
	m_bSceneEntered = false;
	AEffect3DScene::OnExit();
}


void AWeather3DScene::OnUpdate( f32 DeltaSeconds ) noexcept
{
	AEffect3DScene::OnUpdate( DeltaSeconds );
	if ( !m_bEnvironmentCaptured && !CaptureEnvironment_Internal() ) return;

	const f32 SafeDeltaSeconds = std::isfinite( DeltaSeconds ) && DeltaSeconds > 0.0f ? DeltaSeconds : 0.0f;
	m_Weather.Tick( SafeDeltaSeconds );
	if ( m_bTimeOfDayEnabled )
	{
		const f32 PreviousHour = m_TimeOfDay.TimeOfDay();
		m_TimeOfDay.Tick( SafeDeltaSeconds );
		FTimeOfDay3DAppearance CandidateAppearance;
		if ( FTimeOfDay3DAppearance::TryEvaluate(
			m_TimeOfDay, m_TimeOfDaySunAzimuthDegrees, CandidateAppearance )
			&& ApplyTimeOfDaySun_Internal( CandidateAppearance ) )
		{
			m_TimeOfDayAppearance = CandidateAppearance;
		}
		else
		{
			m_TimeOfDay.SetTimeOfDay( PreviousHour );
		}
	}

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
	Candidate.SkySunDirection = CurrentSky.SunDirection();
	Candidate.SkySunColor = CurrentSky.SunColor();
	Candidate.EnvironmentLightMultiplier = EnvironmentLightMultiplier();
	if ( !Candidate.IsValid() ) return false;

	m_Baseline = Candidate;
	m_bEnvironmentCaptured = true;
	return true;
}


void AWeather3DScene::ApplyWeather_Internal() noexcept
{
	if ( !m_bEnvironmentCaptured || !m_Appearance.IsValid() ) return;
	FVec3 BaseSkyZenith = m_Baseline.SkyZenith;
	FVec3 BaseSkyHorizon = m_Baseline.SkyHorizon;
	FVec3 BaseSkyGround = m_Baseline.SkyGround;
	FVec3 BaseSkySunDirection = m_Baseline.SkySunDirection;
	FVec3 BaseSkySunColor = m_Baseline.SkySunColor;
	f32 TimeEnvironmentLightMultiplier = 1.0f;
	const bool bApplyTimeOfDay = m_bTimeOfDayEnabled && m_TimeOfDayAppearance.IsValid();
	if ( bApplyTimeOfDay )
	{
		BaseSkyZenith = m_TimeOfDayAppearance.SkyZenith;
		BaseSkyHorizon = m_TimeOfDayAppearance.SkyHorizon;
		BaseSkyGround = m_TimeOfDayAppearance.SkyGround;
		BaseSkySunDirection = m_TimeOfDayAppearance.SunDirectionToLight;
		BaseSkySunColor = m_TimeOfDayAppearance.SunColor;
		TimeEnvironmentLightMultiplier = m_TimeOfDayAppearance.EnvironmentLightMultiplier;
	}

	const f32 CloudCoverage = m_Baseline.CloudCoverage > m_Appearance.MinimumCloudCoverage
		? m_Baseline.CloudCoverage : m_Appearance.MinimumCloudCoverage;
	const f32 CloudDensity = m_Baseline.CloudDensity * m_Appearance.CloudDensityMultiplier;
	const f32 WindScale = 1.0f + ( m_Appearance.WindStrength - 0.10f ) * 2.5f;
	const f32 CloudWind = m_Baseline.CloudWind * ( WindScale > 0.0f ? WindScale : 0.0f );
	const FVec3 FogColor = Blend( m_Baseline.FogColor, m_Appearance.FogColorTarget, m_Appearance.FogColorBlend );
	const f32 FogDensity = m_Baseline.FogDensity * m_Appearance.FogDensityMultiplier + m_Appearance.ExtraFogDensity;
	const FVec3 SkyZenith = Multiply( BaseSkyZenith, m_Appearance.SkyTint );
	const FVec3 SkyHorizon = Multiply( BaseSkyHorizon, m_Appearance.SkyTint );
	const FVec3 SkyGround = Multiply( BaseSkyGround, m_Appearance.SkyTint );
	const f32 EnvironmentLight = m_Baseline.EnvironmentLightMultiplier
		* TimeEnvironmentLightMultiplier * m_Appearance.EnvironmentLightMultiplier;

	if ( !std::isfinite( CloudCoverage ) || CloudCoverage < 0.0f || CloudCoverage > 1.0f ) return;
	if ( !std::isfinite( CloudDensity ) || CloudDensity < 0.0f || !std::isfinite( CloudWind ) ) return;
	if ( !IsNonNegativeColor( FogColor ) || !std::isfinite( FogDensity ) || FogDensity < 0.0f ) return;
	if ( !IsNonNegativeColor( SkyZenith ) || !IsNonNegativeColor( SkyHorizon ) || !IsNonNegativeColor( SkyGround ) ) return;
	if ( !IsFinite( BaseSkySunDirection ) || LengthSq( BaseSkySunDirection ) <= 0.000001f ) return;
	if ( !IsNonNegativeColor( BaseSkySunColor ) ) return;
	if ( !std::isfinite( EnvironmentLight ) || EnvironmentLight < 0.0f ) return;

	FScene3DClouds& CurrentClouds = Clouds();
	CurrentClouds.Coverage = CloudCoverage;
	CurrentClouds.Density = CloudDensity;
	CurrentClouds.Wind = CloudWind;
	FScene3DFog& CurrentFog = Fog();
	CurrentFog.Color = FogColor;
	CurrentFog.Density = FogDensity;
	CSky& CurrentSky = Sky();
	if ( bApplyTimeOfDay )
	{
		if ( !m_TimeOfDayAppearance.TryApplySunTo( CurrentSky ) ) return;
	}
	else
	{
		CurrentSky.SetSunDirection( BaseSkySunDirection );
		CurrentSky.SetSunColor( BaseSkySunColor );
	}
	CurrentSky.SetZenithColor( SkyZenith );
	CurrentSky.SetHorizonColor( SkyHorizon );
	CurrentSky.SetGroundColor( SkyGround );
	SetEnvironmentLightMultiplier( EnvironmentLight );
}


bool AWeather3DScene::ApplyTimeOfDaySun_Internal( const FTimeOfDay3DAppearance& Appearance ) noexcept
{
	if ( !Appearance.IsValid() || !m_TimeOfDaySun.IsValid() ) return false;
	ANode* const Sun = Graph().Get( m_TimeOfDaySun );
	if ( Sun == nullptr || Sun->IsPendingDestroy() ) return false;

	const FLight3DSpawnParams SunParams = FLight3DSpawnParams::Sun(
		Appearance.SunDirectionToLight, Appearance.SunColor, Appearance.SunIntensity );
	return CLight3DSpawner::TryApplyTo( *Sun, SunParams );
}


void AWeather3DScene::DestroyTimeOfDaySun_Internal() noexcept
{
	if ( !m_TimeOfDaySun.IsValid() ) return;
	ANode* const Sun = Graph().Get( m_TimeOfDaySun );
	if ( Sun != nullptr && !Sun->IsPendingDestroy() ) (void)Graph().Destroy( m_TimeOfDaySun );
	m_TimeOfDaySun = FNodeId{};
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
	Sky().SetSunDirection( m_Baseline.SkySunDirection );
	Sky().SetSunColor( m_Baseline.SkySunColor );
	SetEnvironmentLightMultiplier( m_Baseline.EnvironmentLightMultiplier );
}
