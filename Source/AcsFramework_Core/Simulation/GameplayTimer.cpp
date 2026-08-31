// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayTimer.h"

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


FGameplayTimer::FGameplayTimer( f32 DurationSeconds ) noexcept
{
	(void)SetDurationSeconds( DurationSeconds );
}


bool FGameplayTimer::SetDurationSeconds( f32 DurationSeconds ) noexcept
{
	if ( !std::isfinite( DurationSeconds )
		|| DurationSeconds <= 0.0f ) return false;

	m_DurationSeconds = DurationSeconds;
	if ( !m_bHasStarted ) m_ActiveDurationSeconds = DurationSeconds;
	return true;
}


bool FGameplayTimer::Start() noexcept
{
	if ( m_bHasStarted ) return false;

	m_ActiveDurationSeconds = m_DurationSeconds;
	m_ElapsedSeconds = 0.0;
	m_bHasStarted = true;
	m_bIsRunning = true;
	m_bIsComplete = false;
	return true;
}


void FGameplayTimer::Restart() noexcept
{
	m_ActiveDurationSeconds = m_DurationSeconds;
	m_ElapsedSeconds = 0.0;
	m_bHasStarted = true;
	m_bIsRunning = true;
	m_bIsComplete = false;
}


bool FGameplayTimer::Pause() noexcept
{
	if ( !m_bIsRunning ) return false;

	m_bIsRunning = false;
	return true;
}


bool FGameplayTimer::Resume() noexcept
{
	if ( !m_bHasStarted || m_bIsRunning || m_bIsComplete ) return false;

	m_bIsRunning = true;
	return true;
}


bool FGameplayTimer::Update( f32 DeltaSeconds ) noexcept
{
	if ( !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0f ) return false;

	m_bWasCompleted = false;
	if ( !m_bIsRunning ) return true;

	/** 今回の明示時間を加えた次の経過秒。 */
	const f64 NextElapsedSeconds = m_ElapsedSeconds
		+ static_cast<f64>( DeltaSeconds );
	/** 反復した単精度時間だけを完了境界へ揃える許容秒。 */
	const f64 CompletionTolerance =
		CalculateCompletionToleranceSeconds_Internal( m_ActiveDurationSeconds );
	if ( NextElapsedSeconds + CompletionTolerance
		>= static_cast<f64>( m_ActiveDurationSeconds ) )
	{
		Complete_Internal();
		return true;
	}

	m_ElapsedSeconds = NextElapsedSeconds;
	return true;
}


void FGameplayTimer::Reset() noexcept
{
	m_ActiveDurationSeconds = m_DurationSeconds;
	m_ElapsedSeconds = 0.0;
	m_bHasStarted = false;
	m_bIsRunning = false;
	m_bIsComplete = false;
	m_bWasCompleted = false;
}


FGameplayTimerState FGameplayTimer::CaptureState() const noexcept
{
	FGameplayTimerState State;
	State.DurationSeconds = m_DurationSeconds;
	State.ActiveDurationSeconds = m_ActiveDurationSeconds;
	State.ElapsedSeconds = m_ElapsedSeconds;
	State.bHasStarted = m_bHasStarted;
	State.bIsRunning = m_bIsRunning;
	State.bIsComplete = m_bIsComplete;
	State.bWasCompleted = m_bWasCompleted;
	return State;
}


bool FGameplayTimer::RestoreState(
	const FGameplayTimerState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	m_DurationSeconds = State.DurationSeconds;
	m_ActiveDurationSeconds = State.ActiveDurationSeconds;
	m_ElapsedSeconds = State.ElapsedSeconds;
	m_bHasStarted = State.bHasStarted;
	m_bIsRunning = State.bIsRunning;
	m_bIsComplete = State.bIsComplete;
	m_bWasCompleted = State.bWasCompleted;
	return true;
}


f32 FGameplayTimer::GetRemainingSeconds() const noexcept
{
	if ( !m_bHasStarted ) return m_DurationSeconds;
	if ( m_bIsComplete ) return 0.0f;

	/** 開始時秒数から経過秒を引いた残り時間。 */
	const f64 RemainingSeconds = static_cast<f64>( m_ActiveDurationSeconds )
		- m_ElapsedSeconds;
	return RemainingSeconds > 0.0
		? static_cast<f32>( RemainingSeconds ) : 0.0f;
}


f32 FGameplayTimer::GetProgress() const noexcept
{
	if ( !m_bHasStarted ) return 0.0f;
	if ( m_bIsComplete ) return 1.0f;

	/** 開始時秒数に対する現在の経過割合。 */
	const f64 Progress = m_ElapsedSeconds
		/ static_cast<f64>( m_ActiveDurationSeconds );
	if ( Progress <= 0.0 ) return 0.0f;
	if ( Progress >= 1.0 ) return 1.0f;
	return static_cast<f32>( Progress );
}


void FGameplayTimer::Complete_Internal() noexcept
{
	m_ElapsedSeconds = static_cast<f64>( m_ActiveDurationSeconds );
	m_bIsRunning = false;
	m_bIsComplete = true;
	m_bWasCompleted = true;
}
