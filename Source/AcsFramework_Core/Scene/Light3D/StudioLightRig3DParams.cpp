// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/StudioLightRig3DParams.h"

#include <cmath>

namespace
{
	/** 水平な見る方向を安全に正規化できる最小長の2乗。 */
	constexpr f32 kMinimumHorizontalLengthSquared = 0.000000000001f;

	/** 被写体半径に対するキーライトの正面、左、高さの倍率。 */
	constexpr FVec3 kKeyOffsetScale{ 1.8f, 1.4f, 1.7f };

	/** 被写体半径に対するフィルライトの正面、右、高さの倍率。 */
	constexpr FVec3 kFillOffsetScale{ 1.6f, 1.5f, 0.8f };

	/** 被写体半径に対するリムライトの背面、右、高さの倍率。 */
	constexpr FVec3 kRimOffsetScale{ 1.7f, 0.5f, 1.6f };

	/** 3成分が有限か返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y )
			&& std::isfinite( Value.z );
	}
}


FStudioLightRig3DParams FStudioLightRig3DParams::AroundSubject(
	FVec3 InSubjectCenter, FVec3 InViewDirectionToCamera,
	f32 InSubjectRadius ) noexcept
{
	FStudioLightRig3DParams Params;
	Params.SubjectCenter = InSubjectCenter;
	Params.ViewDirectionToCamera = InViewDirectionToCamera;
	Params.SubjectRadius = InSubjectRadius;
	return Params;
}


bool FStudioLightRig3DParams::TryBuildLights(
	FLight3DSpawnParams& OutKey, FLight3DSpawnParams& OutFill,
	FLight3DSpawnParams& OutRim ) const noexcept
{
	if ( !IsFinite( SubjectCenter ) || !IsFinite( ViewDirectionToCamera )
		|| !std::isfinite( SubjectRadius ) || SubjectRadius <= 0.0f
		|| !std::isfinite( RangeScale ) || RangeScale <= 0.0f ) return false;

	FVec3 TowardCamera{ ViewDirectionToCamera.x, 0.0f, ViewDirectionToCamera.z };
	const f32 HorizontalLengthSquared = LengthSq( TowardCamera );
	if ( !std::isfinite( HorizontalLengthSquared )
		|| HorizontalLengthSquared <= kMinimumHorizontalLengthSquared ) return false;
	TowardCamera = Normalize( TowardCamera );
	const FVec3 ViewLeft = Normalize( Cross( FVec3::Up(), TowardCamera ) );
	if ( !IsFinite( TowardCamera ) || !IsFinite( ViewLeft ) ) return false;

	const f32 Range = SubjectRadius * RangeScale;
	const FVec3 KeyPosition = SubjectCenter
		+ TowardCamera * ( SubjectRadius * kKeyOffsetScale.x )
		+ ViewLeft * ( SubjectRadius * kKeyOffsetScale.y )
		+ FVec3::Up() * ( SubjectRadius * kKeyOffsetScale.z );
	const FVec3 FillPosition = SubjectCenter
		+ TowardCamera * ( SubjectRadius * kFillOffsetScale.x )
		- ViewLeft * ( SubjectRadius * kFillOffsetScale.y )
		+ FVec3::Up() * ( SubjectRadius * kFillOffsetScale.z );
	const FVec3 RimPosition = SubjectCenter
		- TowardCamera * ( SubjectRadius * kRimOffsetScale.x )
		- ViewLeft * ( SubjectRadius * kRimOffsetScale.y )
		+ FVec3::Up() * ( SubjectRadius * kRimOffsetScale.z );

	FLight3DSpawnParams Key = FLight3DSpawnParams::Point(
		KeyPosition, Range, KeyColor, KeyIntensity );
	Key.Name = FStringView( "StudioKeyLight" );
	FLight3DSpawnParams Fill = FLight3DSpawnParams::Point(
		FillPosition, Range, FillColor, FillIntensity );
	Fill.Name = FStringView( "StudioFillLight" );
	FLight3DSpawnParams Rim = FLight3DSpawnParams::Point(
		RimPosition, Range, RimColor, RimIntensity );
	Rim.Name = FStringView( "StudioRimLight" );
	if ( !Key.IsValid() || !Fill.IsValid() || !Rim.IsValid() ) return false;

	OutKey = Key;
	OutFill = Fill;
	OutRim = Rim;
	return true;
}


bool FStudioLightRig3DParams::IsValid() const noexcept
{
	FLight3DSpawnParams Key;
	FLight3DSpawnParams Fill;
	FLight3DSpawnParams Rim;
	return TryBuildLights( Key, Fill, Rim );
}
