// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayCooldown.h"

#include <cmath>
#include <limits>


FGameplayCooldown::FGameplayCooldown( f32 DurationSeconds ) noexcept
{
	SetDurationSeconds( DurationSeconds );
}


bool FGameplayCooldown::SetDurationSeconds(
	f32 DurationSeconds ) noexcept
{
	if ( !std::isfinite( DurationSeconds )
		|| DurationSeconds <= 0.0f ) return false;

	m_DurationSeconds = DurationSeconds;
	if ( !m_bIsCoolingDown ) m_ActiveDurationSeconds = DurationSeconds;
	return true;
}


bool FGameplayCooldown::TryUse() noexcept
{
	if ( m_bIsCoolingDown ) return false;

	m_ActiveDurationSeconds = m_DurationSeconds;
	m_ElapsedSeconds = 0.0;
	m_bIsCoolingDown = true;
	return true;
}


bool FGameplayCooldown::Update( f32 DeltaSeconds ) noexcept
{
	if ( !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0f ) return false;

	m_bWasCompleted = false;
	if ( !m_bIsCoolingDown ) return true;

	const f64 NextElapsed = m_ElapsedSeconds
		+ static_cast<f64>( DeltaSeconds );
	/** f32設定と各更新秒の丸めを2回分だけ許す相対誤差。 */
	const f64 RelativeTolerance =
		static_cast<f64>( m_ActiveDurationSeconds )
		* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
	/** 巨大な秒数でも有効な残り時間を飲み込まない許容誤差の上限。 */
	constexpr f64 MaximumToleranceSeconds = 0.000001;
	const f64 CompletionTolerance = RelativeTolerance < MaximumToleranceSeconds
		? RelativeTolerance : MaximumToleranceSeconds;
	if ( NextElapsed + CompletionTolerance
		>= static_cast<f64>( m_ActiveDurationSeconds ) )
	{
		Complete_Internal();
		return true;
	}

	m_ElapsedSeconds = NextElapsed;
	return true;
}


void FGameplayCooldown::Reset() noexcept
{
	m_ActiveDurationSeconds = m_DurationSeconds;
	m_ElapsedSeconds = 0.0;
	m_bIsCoolingDown = false;
	m_bWasCompleted = false;
}


FGameplayCooldownState FGameplayCooldown::CaptureState() const noexcept
{
	FGameplayCooldownState State;
	State.DurationSeconds = m_DurationSeconds;
	State.ActiveDurationSeconds = m_ActiveDurationSeconds;
	State.ElapsedSeconds = m_ElapsedSeconds;
	State.bIsCoolingDown = m_bIsCoolingDown;
	State.bWasCompleted = m_bWasCompleted;
	return State;
}


bool FGameplayCooldown::RestoreState(
	const FGameplayCooldownState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	m_DurationSeconds = State.DurationSeconds;
	m_ActiveDurationSeconds = State.ActiveDurationSeconds;
	m_ElapsedSeconds = State.ElapsedSeconds;
	m_bIsCoolingDown = State.bIsCoolingDown;
	m_bWasCompleted = State.bWasCompleted;
	return true;
}


f32 FGameplayCooldown::GetRemainingSeconds() const noexcept
{
	if ( !m_bIsCoolingDown ) return 0.0f;

	const f64 Remaining = static_cast<f64>( m_ActiveDurationSeconds )
		- m_ElapsedSeconds;
	return Remaining > 0.0 ? static_cast<f32>( Remaining ) : 0.0f;
}


f32 FGameplayCooldown::GetProgress() const noexcept
{
	if ( !m_bIsCoolingDown ) return 1.0f;

	const f64 Progress = m_ElapsedSeconds
		/ static_cast<f64>( m_ActiveDurationSeconds );
	if ( Progress <= 0.0 ) return 0.0f;
	if ( Progress >= 1.0 ) return 1.0f;
	return static_cast<f32>( Progress );
}


void FGameplayCooldown::Complete_Internal() noexcept
{
	m_ActiveDurationSeconds = m_DurationSeconds;
	m_ElapsedSeconds = 0.0;
	m_bIsCoolingDown = false;
	m_bWasCompleted = true;
}
