// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/Input/ActionInputBuffer.h"

#include <cmath>
#include <limits>

#include "AcsFramework_Core/Simulation/Input/ActionInputTracker.h"


FActionInputBuffer::FActionInputBuffer( f32 WindowSeconds ) noexcept
{
	SetWindowSeconds( WindowSeconds );
}


bool FActionInputBuffer::SetWindowSeconds( f32 WindowSeconds ) noexcept
{
	if ( !std::isfinite( WindowSeconds ) || WindowSeconds <= 0.0f ) return false;

	m_WindowSeconds = WindowSeconds;
	return true;
}


bool FActionInputBuffer::Update( const CActionInputTracker& Input, f32 DeltaSeconds ) noexcept
{
	return Update( Input.GetCurrentInput(), Input.GetPreviousInput(), DeltaSeconds );
}


bool FActionInputBuffer::Update( const FActionInput& CurrentInput, const FActionInput& PreviousInput, f32 DeltaSeconds ) noexcept
{
	if ( !std::isfinite( DeltaSeconds ) || DeltaSeconds < 0.0f ) return false;

	Advance_Internal( DeltaSeconds );
	CapturePressed_Internal( CurrentInput, PreviousInput );
	return true;
}


bool FActionInputBuffer::BufferAction( u32 ActionIndex ) noexcept
{
	if ( ActionIndex >= kActionButtonCount ) return false;

	m_RemainingSeconds[ActionIndex] = static_cast<f64>( m_WindowSeconds );
	m_CapturedWindowSeconds[ActionIndex] = m_WindowSeconds;
	return true;
}


bool FActionInputBuffer::IsBuffered( u32 ActionIndex ) const noexcept
{
	return ActionIndex < kActionButtonCount && m_RemainingSeconds[ActionIndex] > 0.0f;
}


bool FActionInputBuffer::Consume( u32 ActionIndex ) noexcept
{
	if ( !IsBuffered( ActionIndex ) ) return false;

	m_RemainingSeconds[ActionIndex] = 0.0;
	m_CapturedWindowSeconds[ActionIndex] = 0.0f;
	return true;
}


void FActionInputBuffer::Clear( u32 ActionIndex ) noexcept
{
	if ( ActionIndex >= kActionButtonCount ) return;

	m_RemainingSeconds[ActionIndex] = 0.0;
	m_CapturedWindowSeconds[ActionIndex] = 0.0f;
}


void FActionInputBuffer::Reset() noexcept
{
	for ( u32 ActionIndex = 0u; ActionIndex < kActionButtonCount; ++ActionIndex )
	{
		m_RemainingSeconds[ActionIndex] = 0.0;
		m_CapturedWindowSeconds[ActionIndex] = 0.0f;
	}
}


f32 FActionInputBuffer::GetRemainingSeconds( u32 ActionIndex ) const noexcept
{
	return ActionIndex < kActionButtonCount
		? static_cast<f32>( m_RemainingSeconds[ActionIndex] )
		: 0.0f;
}


void FActionInputBuffer::Advance_Internal( f32 DeltaSeconds ) noexcept
{
	if ( DeltaSeconds <= 0.0f ) return;

	/** 入力時のf32値を保ったまま、累積減算を安定させる経過秒。 */
	const f64 DeltaSeconds64 = static_cast<f64>( DeltaSeconds );

	for ( u32 ActionIndex = 0u; ActionIndex < kActionButtonCount; ++ActionIndex )
	{
		f64& RemainingSeconds = m_RemainingSeconds[ActionIndex];

		/** 装填時のf32猶予と各経過秒の丸めを2回分だけ許す相対誤差。 */
		const f64 ExpiryTolerance = static_cast<f64>( m_CapturedWindowSeconds[ActionIndex] )
			* static_cast<f64>( std::numeric_limits<f32>::epsilon() ) * 2.0;
		if ( RemainingSeconds <= DeltaSeconds64 + ExpiryTolerance )
		{
			RemainingSeconds = 0.0;
			m_CapturedWindowSeconds[ActionIndex] = 0.0f;
		}
		else RemainingSeconds -= DeltaSeconds64;
	}
}


void FActionInputBuffer::CapturePressed_Internal( const FActionInput& CurrentInput, const FActionInput& PreviousInput ) noexcept
{
	const u32 PressedButtons = CurrentInput.Buttons & ~PreviousInput.Buttons;
	for ( u32 ActionIndex = 0u; ActionIndex < kActionButtonCount; ++ActionIndex )
	{
		if ( ( PressedButtons & ( 1u << ActionIndex ) ) != 0u ) BufferAction( ActionIndex );
	}
}
