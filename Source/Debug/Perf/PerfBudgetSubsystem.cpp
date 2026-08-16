// SPDX-License-Identifier: Apache-2.0
#include "Debug/Perf/PerfBudgetSubsystem.h"

// GameInstance スコープへ登録する。予算はシーンを跨いで同じものを見る。
ACS_REGISTER_SUBSYSTEM( CPerfBudgetSubsystem, ESubsystemScope::GameInstance )


void CPerfBudgetSubsystem::Configure( f32 FrameBudgetMilliseconds ) noexcept
{
	m_FrameBudgetMilliseconds = FrameBudgetMilliseconds;
	m_Budget.SetFrameBudget( FrameBudgetMilliseconds );

	m_Plan.AddFrameworkDefaults();
	m_Plan.ApplyTo( m_Budget );
}


const char* CPerfBudgetSubsystem::DefineCategory( const FString& Category, f32 BudgetMilliseconds, u32 BudgetBytes ) noexcept
{
	if ( !m_Plan.Add( Category, BudgetMilliseconds, BudgetBytes ) ) return nullptr;

	m_Plan.ApplyTo( m_Budget );

	return m_Plan.FindStableName( Category );
}


void CPerfBudgetSubsystem::BeginFrame() noexcept
{
	m_Budget.BeginFrame();
	m_bFrameOpen = true;
}


void CPerfBudgetSubsystem::EndFrame() noexcept
{
	if ( !m_bFrameOpen ) return;

	m_Budget.EndFrame();
	m_bFrameOpen = false;
}


void CPerfBudgetSubsystem::RecordTimeMilliseconds( const char* Category, f32 ElapsedMilliseconds ) noexcept
{
	if ( Category == nullptr ) return;

	m_Budget.RecordTimeMs( Category, ElapsedMilliseconds );
}


void CPerfBudgetSubsystem::RecordMemoryAlloc( const char* Category, u32 Bytes ) noexcept
{
	if ( Category == nullptr ) return;

	m_Budget.RecordMemoryAlloc( Category, Bytes );
}


void CPerfBudgetSubsystem::RecordMemoryFree( const char* Category, u32 Bytes ) noexcept
{
	if ( Category == nullptr ) return;

	m_Budget.RecordMemoryFree( Category, Bytes );
}


void CPerfBudgetSubsystem::CaptureSnapshot( CPerfBudgetSnapshot& OutSnapshot ) const noexcept
{
	OutSnapshot.CaptureFrom( m_Budget );
}


f32 CPerfBudgetSubsystem::GetAverageFrameMilliseconds() const noexcept
{
	return m_Budget.AverageFrameMs();
}


bool CPerfBudgetSubsystem::IsOverFrameBudget() const noexcept
{
	if ( m_FrameBudgetMilliseconds <= 0.0f ) return false;

	return m_Budget.AverageFrameMs() > m_FrameBudgetMilliseconds;
}
