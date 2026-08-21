// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Weather3D/Weather3DAppearance.h"

#include <cmath>

namespace
{
	/** Frameworkが天候ごとに補う雲と霧の値。 */
	struct FWeather3DProfile
	{
		/** 雲量の下限。 */
		f32 MinimumCloudCoverage;

		/** 雲の濃さの倍率。 */
		f32 CloudDensityMultiplier;

		/** 追加する霧の濃さ。 */
		f32 ExtraFogDensity;

		/** 霧の色を寄せる先。 */
		FVec3 FogColorTarget;

		/** 霧色を寄せる割合。 */
		f32 FogColorBlend;
	};

	/** EWeatherKindの数値順に並べたFramework側の見た目。 */
	const FWeather3DProfile kProfiles[] =
	{
		{ 0.00f, 1.00f, 0.0000f, FVec3{ 0.00f, 0.00f, 0.00f }, 0.00f },
		{ 0.72f, 1.15f, 0.0004f, FVec3{ 0.20f, 0.25f, 0.34f }, 0.18f },
		{ 0.82f, 1.30f, 0.0015f, FVec3{ 0.16f, 0.22f, 0.32f }, 0.32f },
		{ 0.94f, 1.55f, 0.0030f, FVec3{ 0.12f, 0.17f, 0.25f }, 0.46f },
		{ 0.78f, 1.10f, 0.0010f, FVec3{ 0.55f, 0.62f, 0.72f }, 0.28f },
		{ 0.98f, 1.75f, 0.0045f, FVec3{ 0.08f, 0.11f, 0.18f }, 0.60f },
		{ 0.58f, 0.90f, 0.0100f, FVec3{ 0.48f, 0.52f, 0.58f }, 0.82f },
		{ 0.88f, 1.65f, 0.0080f, FVec3{ 0.48f, 0.28f, 0.10f }, 0.76f },
	};

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

	/** 検証済み天候種別に対応する見た目を返す。 */
	const FWeather3DProfile& ProfileFor( EWeatherKind Kind ) noexcept
	{
		return kProfiles[static_cast<usize>( Kind )];
	}
}


bool FWeather3DAppearance::IsSupportedKind( EWeatherKind Kind ) noexcept
{
	switch ( Kind )
	{
	case EWeatherKind::Clear:
	case EWeatherKind::Cloudy:
	case EWeatherKind::Rain:
	case EWeatherKind::HeavyRain:
	case EWeatherKind::Snow:
	case EWeatherKind::Storm:
	case EWeatherKind::Fog:
	case EWeatherKind::Sandstorm:
		return true;
	default:
		return false;
	}
}


bool FWeather3DAppearance::TryEvaluate( const CWeatherSystem& Weather, FWeather3DAppearance& OutAppearance ) noexcept
{
	const EWeatherKind Current = Weather.CurrentWeather();
	const EWeatherKind Target = Weather.TargetWeather();
	const f32 Transition = Weather.TransitionT();
	if ( !IsSupportedKind( Current ) || !IsSupportedKind( Target ) ) return false;
	if ( !std::isfinite( Transition ) || Transition < 0.0f || Transition > 1.0f ) return false;

	const FWeather3DProfile& CurrentProfile = ProfileFor( Current );
	const FWeather3DProfile& TargetProfile = ProfileFor( Target );

	FWeather3DAppearance Candidate;
	Candidate.EnvironmentLightMultiplier = Weather.AmbientLightMultiplier();
	Candidate.ParticleDensity = Weather.ParticleDensity();
	Candidate.SkyTint = Weather.SkyTintMultiplier();
	Candidate.WindStrength = Weather.WindStrength();
	Candidate.FogDensityMultiplier = Weather.FogDensityMultiplier();
	Candidate.MinimumCloudCoverage = Blend( CurrentProfile.MinimumCloudCoverage, TargetProfile.MinimumCloudCoverage, Transition );
	Candidate.CloudDensityMultiplier = Blend( CurrentProfile.CloudDensityMultiplier, TargetProfile.CloudDensityMultiplier, Transition );
	Candidate.ExtraFogDensity = Blend( CurrentProfile.ExtraFogDensity, TargetProfile.ExtraFogDensity, Transition );
	Candidate.FogColorTarget = Blend( CurrentProfile.FogColorTarget, TargetProfile.FogColorTarget, Transition );
	Candidate.FogColorBlend = Blend( CurrentProfile.FogColorBlend, TargetProfile.FogColorBlend, Transition );
	if ( !Candidate.IsValid() ) return false;

	OutAppearance = Candidate;
	return true;
}


bool FWeather3DAppearance::IsValid() const noexcept
{
	if ( !std::isfinite( EnvironmentLightMultiplier ) || EnvironmentLightMultiplier < 0.0f ) return false;
	if ( !std::isfinite( ParticleDensity ) || ParticleDensity < 0.0f ) return false;
	if ( !IsNonNegativeColor( SkyTint ) ) return false;
	if ( !std::isfinite( WindStrength ) || WindStrength < 0.0f || WindStrength > 1.0f ) return false;
	if ( !std::isfinite( FogDensityMultiplier ) || FogDensityMultiplier < 0.0f ) return false;
	if ( !std::isfinite( MinimumCloudCoverage ) || MinimumCloudCoverage < 0.0f || MinimumCloudCoverage > 1.0f ) return false;
	if ( !std::isfinite( CloudDensityMultiplier ) || CloudDensityMultiplier <= 0.0f ) return false;
	if ( !std::isfinite( ExtraFogDensity ) || ExtraFogDensity < 0.0f ) return false;
	if ( !IsNonNegativeColor( FogColorTarget ) ) return false;
	if ( !std::isfinite( FogColorBlend ) || FogColorBlend < 0.0f || FogColorBlend > 1.0f ) return false;

	return true;
}
