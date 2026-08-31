// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayInterval.h"

#include <cmath>
#include <limits>


namespace
{
	/** 反復したf32経過秒を間隔境界へ揃える許容秒を返す。 */
	f64 CalculateIntervalToleranceSeconds_Internal(
		f32 IntervalSeconds ) noexcept
	{
		/** f32設定と各更新秒の丸めを2回分だけ許す相対誤差。 */
		const f64 RelativeTolerance = static_cast<f64>( IntervalSeconds )
			* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
		/** 巨大な間隔でも有効な残り時間を飲み込まない許容誤差の上限。 */
		constexpr f64 MaximumToleranceSeconds = 0.000001;
		return RelativeTolerance < MaximumToleranceSeconds
			? RelativeTolerance : MaximumToleranceSeconds;
	}
}


FGameplayInterval::FGameplayInterval( f32 IntervalSeconds ) noexcept
{
	(void)SetIntervalSeconds( IntervalSeconds );
}


bool FGameplayInterval::SetIntervalSeconds( f32 IntervalSeconds ) noexcept
{
	if ( !std::isfinite( IntervalSeconds )
		|| IntervalSeconds <= 0.0f ) return false;

	m_IntervalSeconds = IntervalSeconds;
	if ( !m_bHasStarted ) m_ActiveIntervalSeconds = IntervalSeconds;
	return true;
}


bool FGameplayInterval::Start() noexcept
{
	if ( m_bHasStarted ) return false;

	m_ActiveIntervalSeconds = m_IntervalSeconds;
	m_AccumulatedSeconds = 0.0;
	m_bHasStarted = true;
	m_bIsRunning = true;
	return true;
}


void FGameplayInterval::Restart() noexcept
{
	m_ActiveIntervalSeconds = m_IntervalSeconds;
	m_AccumulatedSeconds = 0.0;
	m_bHasStarted = true;
	m_bIsRunning = true;
}


bool FGameplayInterval::Pause() noexcept
{
	if ( !m_bIsRunning ) return false;

	m_bIsRunning = false;
	return true;
}


bool FGameplayInterval::Resume() noexcept
{
	if ( !m_bHasStarted || m_bIsRunning ) return false;

	m_bIsRunning = true;
	return true;
}


bool FGameplayInterval::Update( f32 DeltaSeconds,
	u32& OutOccurrenceCount, u32 MaximumCatchUpCount ) noexcept
{
	if ( !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0f
		|| MaximumCatchUpCount == 0u ) return false;

	if ( !m_bIsRunning )
	{
		OutOccurrenceCount = 0u;
		return true;
	}

	/** 今回の明示時間を加えた、未処理ぶんを含む持越し秒。 */
	const f64 NextAccumulatedSeconds = m_AccumulatedSeconds
		+ static_cast<f64>( DeltaSeconds );
	/** 反復した単精度時間だけを間隔境界へ揃える許容秒。 */
	const f64 IntervalTolerance = CalculateIntervalToleranceSeconds_Internal(
		m_ActiveIntervalSeconds );
	/** 今回までに処理可能な全到達回数。巨大値は変換せず上限判定へ使う。 */
	const f64 AvailableOccurrenceCount = std::floor(
		( NextAccumulatedSeconds + IntervalTolerance )
		/ static_cast<f64>( m_ActiveIntervalSeconds ) );
	const u32 OccurrenceCount =
		AvailableOccurrenceCount >= static_cast<f64>( MaximumCatchUpCount )
		? MaximumCatchUpCount
		: static_cast<u32>( AvailableOccurrenceCount );
	/** 今回返した回数ぶんだけを除き、上限超過ぶんを残す秒数。 */
	const f64 RemainingSeconds = NextAccumulatedSeconds
		- static_cast<f64>( OccurrenceCount )
		* static_cast<f64>( m_ActiveIntervalSeconds );
	m_AccumulatedSeconds = RemainingSeconds > IntervalTolerance
		? RemainingSeconds : 0.0;
	OutOccurrenceCount = OccurrenceCount;
	return true;
}


void FGameplayInterval::Reset() noexcept
{
	m_ActiveIntervalSeconds = m_IntervalSeconds;
	m_AccumulatedSeconds = 0.0;
	m_bHasStarted = false;
	m_bIsRunning = false;
}


FGameplayIntervalState FGameplayInterval::CaptureState() const noexcept
{
	FGameplayIntervalState State;
	State.IntervalSeconds = m_IntervalSeconds;
	State.ActiveIntervalSeconds = m_ActiveIntervalSeconds;
	State.AccumulatedSeconds = m_AccumulatedSeconds;
	State.bHasStarted = m_bHasStarted;
	State.bIsRunning = m_bIsRunning;
	return State;
}


bool FGameplayInterval::RestoreState(
	const FGameplayIntervalState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	m_IntervalSeconds = State.IntervalSeconds;
	m_ActiveIntervalSeconds = State.ActiveIntervalSeconds;
	m_AccumulatedSeconds = State.AccumulatedSeconds;
	m_bHasStarted = State.bHasStarted;
	m_bIsRunning = State.bIsRunning;
	return true;
}


f32 FGameplayInterval::GetSecondsUntilNextOccurrence() const noexcept
{
	if ( !m_bHasStarted ) return m_IntervalSeconds;

	/** 開始時の間隔から、次回へ持ち越した秒数を引いた残り時間。 */
	const f64 RemainingSeconds = static_cast<f64>( m_ActiveIntervalSeconds )
		- m_AccumulatedSeconds;
	return RemainingSeconds > 0.0
		? static_cast<f32>( RemainingSeconds ) : 0.0f;
}


f32 FGameplayInterval::GetProgress() const noexcept
{
	if ( !m_bHasStarted ) return 0.0f;

	/** 開始時の間隔に対する次回到達までの進行割合。 */
	const f64 Progress = m_AccumulatedSeconds
		/ static_cast<f64>( m_ActiveIntervalSeconds );
	if ( Progress <= 0.0 ) return 0.0f;
	if ( Progress >= 1.0 ) return 1.0f;
	return static_cast<f32>( Progress );
}
