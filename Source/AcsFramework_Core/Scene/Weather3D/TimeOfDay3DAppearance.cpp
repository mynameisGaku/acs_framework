// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Weather3D/TimeOfDay3DAppearance.h"

#include <cmath>

namespace
{
	/** 夜でも場面既定の太陽へ切り替わらない最小の正の光量。 */
	constexpr f32 kNightSunIntensity = 0.0001f;

	/** 昼の太陽光量。既存のFramework太陽と同じ値。 */
	constexpr f32 kDaySunIntensity = 1.6f;

	/** 環境光の昼基準となるACSの環境色輝度。 */
	constexpr f32 kDayAmbientLuminance = 0.95f;

	/** 夜も形を読めるように残す環境光の下限。 */
	constexpr f32 kMinimumEnvironmentLight = 0.06f;

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

	/** 2つの値を指定割合で混ぜる。 */
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

	/** 線形RGBの知覚輝度を返す。 */
	f32 Luminance( FVec3 Color ) noexcept
	{
		return Color.x * 0.2126f + Color.y * 0.7152f + Color.z * 0.0722f;
	}
}


bool FTimeOfDay3DAppearance::TryEvaluate( const CAmbientDirector& Director,
	f32 SunAzimuthDegrees, FTimeOfDay3DAppearance& OutAppearance ) noexcept
{
	const f32 Hours = Director.TimeOfDay();
	if ( !std::isfinite( Hours ) || Hours < 0.0f || Hours >= 24.0f ) return false;
	if ( !std::isfinite( SunAzimuthDegrees ) ) return false;

	const FVec3 TimeSky = Director.SkyColor();
	const FVec3 TimeAmbient = Director.AmbientColor();
	const FVec3 TimeSunDirection = Director.SunDirection();
	if ( !IsNonNegativeColor( TimeSky ) || !IsNonNegativeColor( TimeAmbient ) || !IsFinite( TimeSunDirection ) ) return false;

	const f32 Daylight = acs::Saturate( TimeSunDirection.y );
	const f32 DayBlend = std::sqrt( Daylight );
	const FVec3 NightZenith{ 0.02f, 0.03f, 0.08f };
	const FVec3 DayZenith{ TimeSky.x * 0.32f, TimeSky.y * 0.54f, TimeSky.z * 0.82f };
	const FVec3 WarmSun{ 1.0f, 0.42f, 0.18f };
	const FVec3 DaySun{ 1.0f, 0.95f, 0.85f };

	const f32 AzimuthRadians = SunAzimuthDegrees * 0.01745329251994329577f;
	const f32 AzimuthCosine = std::cos( AzimuthRadians );
	const f32 AzimuthSine = std::sin( AzimuthRadians );

	FTimeOfDay3DAppearance Candidate;
	Candidate.SkyZenith = Blend( NightZenith, DayZenith, DayBlend );
	Candidate.SkyHorizon = Blend( TimeSky, TimeAmbient, 0.25f );
	Candidate.SkyGround = FVec3
	{
		0.01f + TimeAmbient.x * 0.20f,
		0.01f + TimeAmbient.y * 0.20f,
		0.015f + TimeAmbient.z * 0.20f,
	};
	Candidate.SunDirectionToLight = FVec3
	{
		TimeSunDirection.x * AzimuthCosine,
		TimeSunDirection.y,
		-TimeSunDirection.x * AzimuthSine,
	};
	Candidate.SunColor = Blend( WarmSun, DaySun, DayBlend );
	Candidate.SunIntensity = Blend( kNightSunIntensity, kDaySunIntensity, DayBlend );
	Candidate.EnvironmentLightMultiplier = acs::Saturate( Luminance( TimeAmbient ) / kDayAmbientLuminance );
	if ( Candidate.EnvironmentLightMultiplier < kMinimumEnvironmentLight ) Candidate.EnvironmentLightMultiplier = kMinimumEnvironmentLight;
	if ( !Candidate.IsValid() ) return false;

	OutAppearance = Candidate;
	return true;
}


bool FTimeOfDay3DAppearance::TryApplySunTo( CSky& Sky ) const noexcept
{
	if ( !IsValid() ) return false;

	Sky.SetSunDirection( SunDirectionToLight );
	Sky.SetSunColor( SunColor );
	return true;
}


bool FTimeOfDay3DAppearance::IsValid() const noexcept
{
	if ( !IsNonNegativeColor( SkyZenith ) || !IsNonNegativeColor( SkyHorizon ) || !IsNonNegativeColor( SkyGround ) ) return false;
	if ( !IsFinite( SunDirectionToLight ) ) return false;
	const f32 DirectionLengthSquared = LengthSq( SunDirectionToLight );
	if ( !std::isfinite( DirectionLengthSquared )
		|| std::abs( DirectionLengthSquared - 1.0f ) > 0.001f ) return false;
	if ( !IsNonNegativeColor( SunColor ) || !std::isfinite( SunIntensity ) || SunIntensity <= 0.0f ) return false;
	if ( !std::isfinite( EnvironmentLightMultiplier ) || EnvironmentLightMultiplier < 0.0f || EnvironmentLightMultiplier > 1.0f ) return false;

	return true;
}
