// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/SimulationSubsystem.h"

// GameInstance スコープへ登録する。盤面と記録はシーンを跨いで続く。
ACS_REGISTER_SUBSYSTEM( CSimulationSubsystem, ESubsystemScope::GameInstance )

namespace
{
	/** AdvanceSteps で一度に進められる上限。無限ループを避けるための歯止め。 */
	constexpr u32 kMaximumManualSteps = 1000000u;
}


void CSimulationSubsystem::OnDeinitialize() noexcept
{
	m_Rule.Reset();
	m_InputSource.Reset();
	m_Events.Clear();
}


bool CSimulationSubsystem::Configure( f64 StepSeconds, u32 MaximumStepsPerAdvance ) noexcept
{
	return m_Driver.Configure( StepSeconds, MaximumStepsPerAdvance );
}


bool CSimulationSubsystem::SetRule( TUniquePtr<ISimulationRule> Rule ) noexcept
{
	if ( !Rule ) return false;

	m_Rule = Move( Rule );

	ResetForNewRun();

	return true;
}


bool CSimulationSubsystem::SetInputSource( TUniquePtr<IActionInputSource> Source ) noexcept
{
	if ( !Source ) return false;

	m_InputSource = Move( Source );

	return true;
}


void CSimulationSubsystem::StartRecording( u64 Seed ) noexcept
{
	m_Tape.Clear();
	m_Tape.SetSeed( Seed );
	m_Random.Reseed( Seed );

	ResetForNewRun();

	m_Mode = ESimulationMode::Recording;
}


bool CSimulationSubsystem::StartReplay() noexcept
{
	if ( m_Tape.Num() == 0u ) return false;

	m_Random.Reseed( m_Tape.GetSeed() );

	ResetForNewRun();

	m_Mode = ESimulationMode::Replaying;

	return true;
}


void CSimulationSubsystem::StartLive( u64 Seed ) noexcept
{
	m_Random.Reseed( Seed );

	ResetForNewRun();

	m_Mode = ESimulationMode::Live;
}


u32 CSimulationSubsystem::Update( f64 DeltaSeconds ) noexcept
{
	const u32 StepCount = m_Driver.Advance( DeltaSeconds );

	for ( u32 Step = 0u; Step < StepCount; ++Step )
	{
		AdvanceOneStep();
	}

	return StepCount;
}


u32 CSimulationSubsystem::AdvanceSteps( u32 StepCount ) noexcept
{
	const u32 Clamped = ( StepCount > kMaximumManualSteps ) ? kMaximumManualSteps : StepCount;

	for ( u32 Step = 0u; Step < Clamped; ++Step )
	{
		AdvanceOneStep();
	}

	return Clamped;
}


bool CSimulationSubsystem::TryCaptureSnapshot( CSimulationSnapshot& OutSnapshot ) const noexcept
{
	if ( !m_Rule ) return false;

	return OutSnapshot.TryCaptureFrom( m_Driver, m_Random, *m_Rule, m_LastInput, m_PreviousInput );
}


bool CSimulationSubsystem::TryRestoreSnapshot( const CSimulationSnapshot& Snapshot ) noexcept
{
	if ( !m_Rule ) return false;

	FActionInput RestoredLastInput;
	FActionInput RestoredPreviousInput;
	if ( !Snapshot.TryRestoreTo( m_Driver, m_Random, *m_Rule, RestoredLastInput, RestoredPreviousInput ) ) return false;

	m_LastInput = RestoredLastInput;
	m_PreviousInput = RestoredPreviousInput;

	// 戻る前のフレームで溜まったものを持ち越すと、同じことが二度起きたように見える。
	m_Events.Clear();

	return true;
}


void CSimulationSubsystem::ResolveInput( u32 Tick, FActionInput& OutInput ) noexcept
{
	OutInput = FActionInput();

	if ( m_Mode == ESimulationMode::Replaying )
	{
		// 再生中は入力元を見ない。テープに無いティックは中立で進める
		// (記録の終端より先まで回した場合)。
		m_Tape.TryGet( Tick, OutInput );
		return;
	}

	if ( m_InputSource )
	{
		if ( !m_InputSource->TryGetActionInput( OutInput ) ) OutInput = FActionInput();
	}
}


void CSimulationSubsystem::AdvanceOneStep() noexcept
{
	const u32 Tick = m_Driver.GetTick();

	m_PreviousInput = m_LastInput;
	ResolveInput( Tick, m_LastInput );

	if ( m_Mode == ESimulationMode::Recording ) m_Tape.Record( Tick, m_LastInput );

	if ( m_Rule )
	{
		FSimulationContext Context;
		Context.Input = m_LastInput;
		Context.PreviousInput = m_PreviousInput;
		Context.Tick = Tick;
		Context.StepSeconds = m_Driver.GetStepSeconds();
		Context.Random = &m_Random;
		Context.Events = &m_Events;

		m_Rule->AdvanceStep( Context );
	}

	m_Driver.AdvanceTick();
}


void CSimulationSubsystem::ResetForNewRun() noexcept
{
	m_Driver.Reset();
	m_Events.Clear();

	m_LastInput = FActionInput();
	m_PreviousInput = FActionInput();

	if ( m_Rule ) m_Rule->ResetState();
}
