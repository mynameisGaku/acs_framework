// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayChargePool.h"

#include <cmath>
#include <limits>


namespace
{
	/** 反復したf32経過秒を1チャージ境界へ揃える許容秒を返す。 */
	f64 CalculateChargeToleranceSeconds_Internal(
		f32 RechargeSeconds ) noexcept
	{
		/** f32設定と各更新秒の丸めを2回分だけ許す相対誤差。 */
		const f64 RelativeTolerance = static_cast<f64>( RechargeSeconds )
			* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
		/** 巨大な回復秒でも有効な残り時間を飲み込まない許容誤差の上限。 */
		constexpr f64 MaximumToleranceSeconds = 0.000001;
		return RelativeTolerance < MaximumToleranceSeconds
			? RelativeTolerance : MaximumToleranceSeconds;
	}
}


FGameplayChargePool::FGameplayChargePool( u32 MaximumCharges,
	f32 RechargeSeconds ) noexcept
{
	(void)TryConfigure( MaximumCharges, MaximumCharges, RechargeSeconds );
}


FGameplayChargePool::FGameplayChargePool( u32 MaximumCharges,
	u32 CurrentCharges, f32 RechargeSeconds ) noexcept
{
	(void)TryConfigure( MaximumCharges, CurrentCharges, RechargeSeconds );
}


bool FGameplayChargePool::TryConfigure( u32 MaximumCharges,
	u32 CurrentCharges, f32 RechargeSeconds ) noexcept
{
	if ( MaximumCharges == 0u || CurrentCharges > MaximumCharges
		|| !std::isfinite( RechargeSeconds )
		|| RechargeSeconds <= 0.0f ) return false;

	m_MaximumCharges = MaximumCharges;
	m_CurrentCharges = CurrentCharges;
	m_RechargeSeconds = RechargeSeconds;
	m_ActiveRechargeSeconds = RechargeSeconds;
	m_AccumulatedSeconds = 0.0;
	m_bIsPaused = false;
	return true;
}


bool FGameplayChargePool::TrySetMaximumCharges(
	u32 MaximumCharges ) noexcept
{
	if ( MaximumCharges == 0u ) return false;

	const bool bWasFull = IsFull();
	const u32 NextCurrentCharges = m_CurrentCharges > MaximumCharges
		? MaximumCharges : m_CurrentCharges;
	m_MaximumCharges = MaximumCharges;
	m_CurrentCharges = NextCurrentCharges;
	if ( IsFull() )
	{
		NormalizeFull_Internal();
	}
	else if ( bWasFull )
	{
		m_ActiveRechargeSeconds = m_RechargeSeconds;
		m_AccumulatedSeconds = 0.0;
		m_bIsPaused = false;
	}
	return true;
}


bool FGameplayChargePool::TrySetRechargeSeconds(
	f32 RechargeSeconds ) noexcept
{
	if ( !std::isfinite( RechargeSeconds )
		|| RechargeSeconds <= 0.0f ) return false;

	m_RechargeSeconds = RechargeSeconds;
	if ( IsFull() ) m_ActiveRechargeSeconds = RechargeSeconds;
	return true;
}


bool FGameplayChargePool::TryConsume( u32 ChargeCount ) noexcept
{
	if ( ChargeCount > m_CurrentCharges ) return false;
	if ( ChargeCount == 0u ) return true;

	const bool bWasFull = IsFull();
	m_CurrentCharges -= ChargeCount;
	if ( bWasFull )
	{
		m_ActiveRechargeSeconds = m_RechargeSeconds;
		m_AccumulatedSeconds = 0.0;
		m_bIsPaused = false;
	}
	return true;
}


u32 FGameplayChargePool::RestoreCharges( u32 ChargeCount ) noexcept
{
	const u32 MissingCharges = GetMissingCharges();
	const u32 RestoredCharges = ChargeCount < MissingCharges
		? ChargeCount : MissingCharges;
	m_CurrentCharges += RestoredCharges;
	if ( IsFull() ) NormalizeFull_Internal();
	return RestoredCharges;
}


bool FGameplayChargePool::Update( f32 DeltaSeconds,
	u32& OutRestoredChargeCount, u32 MaximumCatchUpCount ) noexcept
{
	if ( !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0f
		|| MaximumCatchUpCount == 0u ) return false;
	if ( IsFull() || m_bIsPaused )
	{
		OutRestoredChargeCount = 0u;
		return true;
	}

	/** 今回の明示時間を加えた、未処理ぶんを含む持越し秒。 */
	const f64 NextAccumulatedSeconds = m_AccumulatedSeconds
		+ static_cast<f64>( DeltaSeconds );
	if ( !std::isfinite( NextAccumulatedSeconds ) ) return false;
	const f64 ActiveTolerance = CalculateChargeToleranceSeconds_Internal(
		m_ActiveRechargeSeconds );
	if ( NextAccumulatedSeconds + ActiveTolerance
		< static_cast<f64>( m_ActiveRechargeSeconds ) )
	{
		m_AccumulatedSeconds = NextAccumulatedSeconds;
		OutRestoredChargeCount = 0u;
		return true;
	}

	/** 現在進行中の1回を回復した後に残る秒数。 */
	f64 RemainingSeconds = NextAccumulatedSeconds
		- static_cast<f64>( m_ActiveRechargeSeconds );
	if ( RemainingSeconds < 0.0 ) RemainingSeconds = 0.0;
	++m_CurrentCharges;
	u32 RestoredChargeCount = 1u;
	if ( IsFull() )
	{
		NormalizeFull_Internal();
		OutRestoredChargeCount = RestoredChargeCount;
		return true;
	}

	m_ActiveRechargeSeconds = m_RechargeSeconds;
	if ( RestoredChargeCount >= MaximumCatchUpCount )
	{
		m_AccumulatedSeconds = RemainingSeconds;
		OutRestoredChargeCount = RestoredChargeCount;
		return true;
	}

	const f64 RechargeTolerance = CalculateChargeToleranceSeconds_Internal(
		m_ActiveRechargeSeconds );
	/** 新設定で今回までに処理可能な追加回復数。 */
	const f64 AvailableAdditionalCount = std::floor(
		( RemainingSeconds + RechargeTolerance )
		/ static_cast<f64>( m_ActiveRechargeSeconds ) );
	const u32 MissingCharges = GetMissingCharges();
	const u32 RemainingCatchUpCount = MaximumCatchUpCount
		- RestoredChargeCount;
	const u32 AdditionalLimit = MissingCharges < RemainingCatchUpCount
		? MissingCharges : RemainingCatchUpCount;
	const u32 AdditionalCount =
		AvailableAdditionalCount >= static_cast<f64>( AdditionalLimit )
		? AdditionalLimit : static_cast<u32>( AvailableAdditionalCount );
	RestoredChargeCount += AdditionalCount;
	m_CurrentCharges += AdditionalCount;
	RemainingSeconds -= static_cast<f64>( AdditionalCount )
		* static_cast<f64>( m_ActiveRechargeSeconds );
	if ( IsFull() )
	{
		NormalizeFull_Internal();
	}
	else
	{
		m_AccumulatedSeconds = RemainingSeconds > RechargeTolerance
			? RemainingSeconds : 0.0;
	}
	OutRestoredChargeCount = RestoredChargeCount;
	return true;
}


bool FGameplayChargePool::Pause() noexcept
{
	if ( IsFull() || m_bIsPaused ) return false;

	m_bIsPaused = true;
	return true;
}


bool FGameplayChargePool::Resume() noexcept
{
	if ( IsFull() || !m_bIsPaused ) return false;

	m_bIsPaused = false;
	return true;
}


void FGameplayChargePool::Fill() noexcept
{
	m_CurrentCharges = m_MaximumCharges;
	NormalizeFull_Internal();
}


void FGameplayChargePool::Empty() noexcept
{
	m_CurrentCharges = 0u;
	m_ActiveRechargeSeconds = m_RechargeSeconds;
	m_AccumulatedSeconds = 0.0;
	m_bIsPaused = false;
}


FGameplayChargePoolState FGameplayChargePool::CaptureState() const noexcept
{
	FGameplayChargePoolState State;
	State.MaximumCharges = m_MaximumCharges;
	State.CurrentCharges = m_CurrentCharges;
	State.RechargeSeconds = m_RechargeSeconds;
	State.ActiveRechargeSeconds = m_ActiveRechargeSeconds;
	State.AccumulatedSeconds = m_AccumulatedSeconds;
	State.bIsPaused = m_bIsPaused;
	return State;
}


bool FGameplayChargePool::RestoreState(
	const FGameplayChargePoolState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	m_MaximumCharges = State.MaximumCharges;
	m_CurrentCharges = State.CurrentCharges;
	m_RechargeSeconds = State.RechargeSeconds;
	m_ActiveRechargeSeconds = State.ActiveRechargeSeconds;
	m_AccumulatedSeconds = State.AccumulatedSeconds;
	m_bIsPaused = State.bIsPaused;
	return true;
}


f32 FGameplayChargePool::GetSecondsUntilNextCharge() const noexcept
{
	if ( IsFull() ) return 0.0f;

	const f64 Remaining = static_cast<f64>( m_ActiveRechargeSeconds )
		- m_AccumulatedSeconds;
	return Remaining > 0.0 ? static_cast<f32>( Remaining ) : 0.0f;
}


f32 FGameplayChargePool::GetRechargeProgress() const noexcept
{
	if ( IsFull() ) return 1.0f;

	const f64 Progress = m_AccumulatedSeconds
		/ static_cast<f64>( m_ActiveRechargeSeconds );
	if ( Progress <= 0.0 ) return 0.0f;
	if ( Progress >= 1.0 ) return 1.0f;
	return static_cast<f32>( Progress );
}


void FGameplayChargePool::NormalizeFull_Internal() noexcept
{
	m_ActiveRechargeSeconds = m_RechargeSeconds;
	m_AccumulatedSeconds = 0.0;
	m_bIsPaused = false;
}
