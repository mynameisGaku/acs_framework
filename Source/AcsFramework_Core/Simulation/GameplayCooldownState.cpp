// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayCooldownState.h"

#include <cmath>
#include <limits>


bool FGameplayCooldownState::IsValid() const noexcept
{
	if ( !std::isfinite( DurationSeconds ) || DurationSeconds <= 0.0f
		|| !std::isfinite( ActiveDurationSeconds )
		|| ActiveDurationSeconds <= 0.0f
		|| !std::isfinite( ElapsedSeconds )
		|| ElapsedSeconds < 0.0 ) return false;

	if ( !bIsCoolingDown )
	{
		return ActiveDurationSeconds == DurationSeconds
			&& ElapsedSeconds == 0.0;
	}
	/** 完了通知付きの再使用待ちは、同じ更新で開始した直後だけ生成できる。 */
	if ( bWasCompleted && ElapsedSeconds != 0.0 ) return false;

	/** f32設定と各更新秒の丸めを2回分だけ許す相対誤差。 */
	const f64 RelativeTolerance = static_cast<f64>( ActiveDurationSeconds )
		* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
	/** 巨大な秒数でも有効な残り時間を飲み込まない許容誤差の上限。 */
	constexpr f64 MaximumToleranceSeconds = 0.000001;
	const f64 CompletionTolerance = RelativeTolerance < MaximumToleranceSeconds
		? RelativeTolerance : MaximumToleranceSeconds;
	return ElapsedSeconds + CompletionTolerance
		< static_cast<f64>( ActiveDurationSeconds );
}
