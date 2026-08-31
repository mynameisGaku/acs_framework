// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayTimerState.h"

#include <cmath>
#include <limits>


namespace
{
	/** 反復したf32経過秒を完了境界へ揃える許容秒を返す。 */
	f64 CalculateCompletionToleranceSeconds_Internal(
		f32 DurationSeconds ) noexcept
	{
		/** f32設定と各更新秒の丸めを2回分だけ許す相対誤差。 */
		const f64 RelativeTolerance = static_cast<f64>( DurationSeconds )
			* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
		/** 巨大な秒数でも有効な残り時間を飲み込まない許容誤差の上限。 */
		constexpr f64 MaximumToleranceSeconds = 0.000001;
		return RelativeTolerance < MaximumToleranceSeconds
			? RelativeTolerance : MaximumToleranceSeconds;
	}
}


bool FGameplayTimerState::IsValid() const noexcept
{
	if ( !std::isfinite( DurationSeconds ) || DurationSeconds <= 0.0f
		|| !std::isfinite( ActiveDurationSeconds )
		|| ActiveDurationSeconds <= 0.0f
		|| !std::isfinite( ElapsedSeconds )
		|| ElapsedSeconds < 0.0 ) return false;

	if ( !bHasStarted )
	{
		return ActiveDurationSeconds == DurationSeconds
			&& ElapsedSeconds == 0.0 && !bIsRunning
			&& !bIsComplete && !bWasCompleted;
	}
	if ( bIsComplete )
	{
		return !bIsRunning
			&& ElapsedSeconds == static_cast<f64>( ActiveDurationSeconds );
	}

	/** まだ完了していない計測に許す境界誤差。 */
	const f64 CompletionTolerance =
		CalculateCompletionToleranceSeconds_Internal( ActiveDurationSeconds );
	if ( ElapsedSeconds + CompletionTolerance
		>= static_cast<f64>( ActiveDurationSeconds ) ) return false;

	/** 完了直後に再開始した通知は、時間が進むまでだけ保持できる。 */
	if ( bWasCompleted && ElapsedSeconds != 0.0 ) return false;
	return true;
}
