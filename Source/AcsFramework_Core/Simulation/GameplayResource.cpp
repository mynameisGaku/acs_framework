// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/GameplayResource.h"

#include <cmath>


FGameplayResource::FGameplayResource( f32 MaximumValue ) noexcept
{
	(void)TryConfigure( MaximumValue, MaximumValue );
}


FGameplayResource::FGameplayResource( f32 MaximumValue,
	f32 CurrentValue ) noexcept
{
	(void)TryConfigure( MaximumValue, CurrentValue );
}


bool FGameplayResource::TryConfigure( f32 MaximumValue,
	f32 CurrentValue ) noexcept
{
	if ( !std::isfinite( MaximumValue ) || MaximumValue <= 0.0f
		|| !std::isfinite( CurrentValue ) || CurrentValue < 0.0f
		|| CurrentValue > MaximumValue ) return false;

	m_MaximumValue = MaximumValue;
	m_CurrentValue = CurrentValue == 0.0f ? 0.0f : CurrentValue;
	return true;
}


bool FGameplayResource::TrySetMaximumValue( f32 MaximumValue ) noexcept
{
	if ( !std::isfinite( MaximumValue ) || MaximumValue <= 0.0f ) return false;

	/** 新上限を超えないように確定する次の現在値。 */
	const f32 NextCurrentValue = m_CurrentValue > MaximumValue
		? MaximumValue : m_CurrentValue;
	m_MaximumValue = MaximumValue;
	m_CurrentValue = NextCurrentValue;
	return true;
}


bool FGameplayResource::TrySetCurrentValue( f32 CurrentValue ) noexcept
{
	if ( !std::isfinite( CurrentValue ) || CurrentValue < 0.0f
		|| CurrentValue > m_MaximumValue ) return false;

	m_CurrentValue = CurrentValue == 0.0f ? 0.0f : CurrentValue;
	return true;
}


bool FGameplayResource::TrySpend( f32 Amount ) noexcept
{
	if ( !std::isfinite( Amount ) || Amount < 0.0f
		|| Amount > m_CurrentValue ) return false;

	m_CurrentValue = Amount == m_CurrentValue
		? 0.0f : m_CurrentValue - Amount;
	return true;
}


bool FGameplayResource::TryRestore( f32 Amount ) noexcept
{
	/** 呼出側が増加量を使わない場合の受け皿。 */
	f32 IgnoredAppliedAmount = 0.0f;
	return TryRestore( Amount, IgnoredAppliedAmount );
}


bool FGameplayResource::TryRestore( f32 Amount,
	f32& OutAppliedAmount ) noexcept
{
	if ( !std::isfinite( Amount ) || Amount < 0.0f ) return false;

	/** 上限まで現在不足している量。 */
	const f32 MissingValue = m_MaximumValue - m_CurrentValue;
	/** 上限を超えない回復要求量。 */
	const f32 ClampedAmount = Amount < MissingValue ? Amount : MissingValue;
	/** 単精度の丸めも反映した次の現在値。 */
	const f32 NextCurrentValue = ClampedAmount == MissingValue
		? m_MaximumValue : m_CurrentValue + ClampedAmount;
	/** 実際に現在値へ反映できた増加量。 */
	const f32 AppliedAmount = NextCurrentValue - m_CurrentValue;
	m_CurrentValue = NextCurrentValue;
	OutAppliedAmount = AppliedAmount;
	return true;
}


FGameplayResourceState FGameplayResource::CaptureState() const noexcept
{
	FGameplayResourceState State;
	State.MaximumValue = m_MaximumValue;
	State.CurrentValue = m_CurrentValue;
	return State;
}


bool FGameplayResource::RestoreState(
	const FGameplayResourceState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	m_MaximumValue = State.MaximumValue;
	m_CurrentValue = State.CurrentValue == 0.0f ? 0.0f : State.CurrentValue;
	return true;
}


f32 FGameplayResource::GetRatio() const noexcept
{
	return m_CurrentValue / m_MaximumValue;
}
