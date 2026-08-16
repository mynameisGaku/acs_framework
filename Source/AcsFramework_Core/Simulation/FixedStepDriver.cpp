// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/FixedStepDriver.h"

namespace
{
	/** 既定のステップ幅 (60 ステップ毎秒)。 */
	constexpr f64 kDefaultStepSeconds = 1.0 / 60.0;

	/** 既定の 1 フレームあたり最大ステップ数。 */
	constexpr u32 kDefaultMaximumSteps = 8u;
}


CFixedStepDriver::CFixedStepDriver() noexcept
{
	// 設定を忘れても回るようにしておく。忘れたまま 0 ステップしか進まないと、
	// 「動かない」原因が時計にあると気付きにくい。
	Configure( kDefaultStepSeconds, kDefaultMaximumSteps );
}


bool CFixedStepDriver::Configure( f64 StepSeconds, u32 MaximumStepsPerAdvance ) noexcept
{
	if ( StepSeconds <= 0.0 || MaximumStepsPerAdvance == 0u ) return false;

	FFixedStepOptions Options;
	Options.step_seconds = StepSeconds;
	Options.maximum_steps_per_advance = MaximumStepsPerAdvance;
	Options.maximum_accumulated_seconds = StepSeconds * static_cast<f64>( MaximumStepsPerAdvance );

	if ( !m_Clock.Configure( Options ) ) return false;

	m_StepSeconds = static_cast<f32>( StepSeconds );

	return true;
}


u32 CFixedStepDriver::Advance( f64 DeltaSeconds ) noexcept
{
	const FFixedStepAdvanceResult Result = m_Clock.Advance( DeltaSeconds );

	m_bWasClamped = Result.was_clamped;

	if ( !Result.accepted ) return 0u;

	return Result.step_count;
}


f32 CFixedStepDriver::GetAlpha() const noexcept
{
	return static_cast<f32>( m_Clock.InterpolationAlpha() );
}


u64 CFixedStepDriver::GetTotalStepCount() const noexcept
{
	return m_Clock.TotalStepCount();
}


f64 CFixedStepDriver::GetDroppedSeconds() const noexcept
{
	return m_Clock.TotalDroppedSeconds();
}


void CFixedStepDriver::Reset() noexcept
{
	m_Clock.Reset();
	m_Tick = 0u;
	m_bWasClamped = false;
}


bool CFixedStepDriver::TryCaptureSnapshot( FFixedStepClockSnapshot& OutSnapshot, u32& OutTick ) const noexcept
{
	if ( !m_Clock.TryCaptureSnapshot( &OutSnapshot ) ) return false;

	OutTick = m_Tick;

	return true;
}


bool CFixedStepDriver::TryRestoreSnapshot( const FFixedStepClockSnapshot& Snapshot, u32 Tick ) noexcept
{
	if ( !m_Clock.TryRestoreSnapshot( &Snapshot ) ) return false;

	m_Tick = Tick;
	m_StepSeconds = static_cast<f32>( Snapshot.options.step_seconds );

	return true;
}
