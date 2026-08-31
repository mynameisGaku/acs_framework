// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputMask.h"

#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"


FActionInputMask FActionInputMask::All() noexcept
{
	return FActionInputMask{};
}


FActionInputMask FActionInputMask::None() noexcept
{
	FActionInputMask Result;
	Result.DisableAll();
	return Result;
}


void FActionInputMask::EnableAll() noexcept
{
	m_ActionMask = kAllActionMask;
	m_AxisMask = kAllAxisMask;
}


void FActionInputMask::DisableAll() noexcept
{
	m_ActionMask = 0u;
	m_AxisMask = 0u;
}


bool FActionInputMask::SetActionEnabled( u32 ActionIndex, bool bEnabled ) noexcept
{
	if ( ActionIndex >= kActionButtonCount ) return false;

	const u32 ActionBit = 1u << ActionIndex;
	m_ActionMask = bEnabled
		? ( m_ActionMask | ActionBit )
		: ( m_ActionMask & ~ActionBit );
	return true;
}


bool FActionInputMask::SetAxisEnabled( u32 AxisIndex, bool bEnabled ) noexcept
{
	if ( AxisIndex >= kActionAxisCount ) return false;

	const u32 AxisBit = 1u << AxisIndex;
	m_AxisMask = bEnabled
		? ( m_AxisMask | AxisBit )
		: ( m_AxisMask & ~AxisBit );
	return true;
}


bool FActionInputMask::IsActionEnabled( u32 ActionIndex ) const noexcept
{
	return ActionIndex < kActionButtonCount
		&& ( m_ActionMask & ( 1u << ActionIndex ) ) != 0u;
}


bool FActionInputMask::IsAxisEnabled( u32 AxisIndex ) const noexcept
{
	return AxisIndex < kActionAxisCount
		&& ( m_AxisMask & ( 1u << AxisIndex ) ) != 0u;
}


bool FActionInputMask::TrySetMasks( u32 ActionMask, u32 AxisMask ) noexcept
{
	if ( ( AxisMask & ~kAllAxisMask ) != 0u ) return false;

	m_ActionMask = ActionMask;
	m_AxisMask = AxisMask;
	return true;
}


FActionInput FActionInputMask::Apply( const FActionInput& Input ) const noexcept
{
	FActionInput Result = Input;
	Result.Buttons &= m_ActionMask;
	for ( u32 AxisIndex = 0u; AxisIndex < kActionAxisCount; ++AxisIndex )
	{
		if ( !IsAxisEnabled( AxisIndex ) ) Result.Axes[AxisIndex] = 0.0f;
	}
	return Result;
}


void FActionInputMask::ApplyHistory( const FActionInput& CurrentInput,
	const FActionInput& PreviousInput,
	FActionInput& OutCurrentInput,
	FActionInput& OutPreviousInput ) const noexcept
{
	const FActionInput FilteredCurrentInput = Apply( CurrentInput );
	const FActionInput FilteredPreviousInput = Apply( PreviousInput );
	OutCurrentInput = FilteredCurrentInput;
	OutPreviousInput = FilteredPreviousInput;
}


void FActionInputMask::ApplyHistory( const CActionInputTracker& Input,
	FActionInput& OutCurrentInput,
	FActionInput& OutPreviousInput ) const noexcept
{
	ApplyHistory( Input.GetCurrentInput(), Input.GetPreviousInput(),
		OutCurrentInput, OutPreviousInput );
}


FActionInputMask FActionInputMask::Intersect(
	const FActionInputMask& Other ) const noexcept
{
	FActionInputMask Result;
	Result.m_ActionMask = m_ActionMask & Other.m_ActionMask;
	Result.m_AxisMask = m_AxisMask & Other.m_AxisMask;
	return Result;
}
