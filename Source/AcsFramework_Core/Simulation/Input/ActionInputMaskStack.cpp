// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputMaskStack.h"

#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"


bool FActionInputMaskStack::Push( const FActionInputMask& Mask ) noexcept
{
	if ( m_LayerCount >= kActionInputMaskStackCapacity ) return false;

	m_Layers[m_LayerCount] = Mask;
	++m_LayerCount;
	m_CombinedMask = m_CombinedMask.Intersect( Mask );
	return true;
}


bool FActionInputMaskStack::Pop() noexcept
{
	if ( m_LayerCount == 0u ) return false;

	--m_LayerCount;
	m_Layers[m_LayerCount] = FActionInputMask::All();
	RebuildCombinedMask_Internal();
	return true;
}


bool FActionInputMaskStack::TryReplaceTop(
	const FActionInputMask& Mask ) noexcept
{
	if ( m_LayerCount == 0u ) return false;

	m_Layers[m_LayerCount - 1u] = Mask;
	RebuildCombinedMask_Internal();
	return true;
}


void FActionInputMaskStack::Clear() noexcept
{
	for ( u32 LayerIndex = 0u;
		LayerIndex < kActionInputMaskStackCapacity; ++LayerIndex )
	{
		m_Layers[LayerIndex] = FActionInputMask::All();
	}
	m_LayerCount = 0u;
	m_CombinedMask = FActionInputMask::All();
}


bool FActionInputMaskStack::TryGetTop(
	FActionInputMask& OutMask ) const noexcept
{
	if ( m_LayerCount == 0u ) return false;

	OutMask = m_Layers[m_LayerCount - 1u];
	return true;
}


FActionInput FActionInputMaskStack::Apply(
	const FActionInput& Input ) const noexcept
{
	return m_CombinedMask.Apply( Input );
}


void FActionInputMaskStack::ApplyHistory(
	const FActionInput& CurrentInput,
	const FActionInput& PreviousInput,
	FActionInput& OutCurrentInput,
	FActionInput& OutPreviousInput ) const noexcept
{
	m_CombinedMask.ApplyHistory( CurrentInput, PreviousInput,
		OutCurrentInput, OutPreviousInput );
}


void FActionInputMaskStack::ApplyHistory(
	const CActionInputTracker& Input,
	FActionInput& OutCurrentInput,
	FActionInput& OutPreviousInput ) const noexcept
{
	m_CombinedMask.ApplyHistory( Input,
		OutCurrentInput, OutPreviousInput );
}


FActionInputMaskStackState FActionInputMaskStack::CaptureState() const noexcept
{
	FActionInputMaskStackState State;
	State.LayerCount = m_LayerCount;
	for ( u32 LayerIndex = 0u; LayerIndex < m_LayerCount; ++LayerIndex )
	{
		State.ActionMasks[LayerIndex] = m_Layers[LayerIndex].GetActionMask();
		State.AxisMasks[LayerIndex] = m_Layers[LayerIndex].GetAxisMask();
	}
	return State;
}


bool FActionInputMaskStack::RestoreState(
	const FActionInputMaskStackState& State ) noexcept
{
	if ( !State.IsValid() ) return false;

	FActionInputMask CandidateLayers[kActionInputMaskStackCapacity] = {};
	FActionInputMask CandidateCombined;
	for ( u32 LayerIndex = 0u; LayerIndex < State.LayerCount; ++LayerIndex )
	{
		if ( !CandidateLayers[LayerIndex].TrySetMasks(
			State.ActionMasks[LayerIndex],
			State.AxisMasks[LayerIndex] ) ) return false;
		CandidateCombined = CandidateCombined.Intersect(
			CandidateLayers[LayerIndex] );
	}

	for ( u32 LayerIndex = 0u;
		LayerIndex < kActionInputMaskStackCapacity; ++LayerIndex )
	{
		m_Layers[LayerIndex] = CandidateLayers[LayerIndex];
	}
	m_LayerCount = State.LayerCount;
	m_CombinedMask = CandidateCombined;
	return true;
}


void FActionInputMaskStack::RebuildCombinedMask_Internal() noexcept
{
	m_CombinedMask = FActionInputMask::All();
	for ( u32 LayerIndex = 0u; LayerIndex < m_LayerCount; ++LayerIndex )
	{
		m_CombinedMask = m_CombinedMask.Intersect( m_Layers[LayerIndex] );
	}
}
