// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/SimulationSnapshot.h"

namespace
{
	/** スナップショットの目印。別形式のバイト列を読み込まないための確認に使う。 */
	constexpr u32 kSnapshotMagic = 0xAC575A00u;

	/** 入力履歴を持たない旧形式の版。 */
	constexpr u32 kLegacySnapshotVersion = 1u;

	/** スナップショットの現在の形式の版。 */
	constexpr u32 kSnapshotVersion = 2u;

	/** 旧v1が保存した乱数状態の生メモリ大きさ。 */
	constexpr usize kLegacyRandomSnapshotBytes = 32u;

	/** 旧v1が保存した時計状態の生メモリ大きさ。 */
	constexpr usize kLegacyClockSnapshotBytes = 48u;

	static_assert( sizeof( FRandomSnapshot ) == kLegacyRandomSnapshotBytes, "v1乱数形式の明示移行が必要です" );
	static_assert( sizeof( FFixedStepClockSnapshot ) == kLegacyClockSnapshotBytes, "v1時計形式の明示移行が必要です" );

	/** 現在形式でfield単位に書く乱数状態の大きさ。 */
	constexpr usize kRandomSnapshotBytes = sizeof( u32 ) * 6u + sizeof( u64 );

	/** 現在形式でfield単位に書く時計状態の大きさ。 */
	constexpr usize kClockSnapshotBytes = sizeof( f64 ) * 4u + sizeof( u32 ) + sizeof( u64 );

	/** アクション入力1つをバイト列へ書く大きさ。 */
	constexpr usize kActionInputBytes = sizeof( f32 ) * kActionAxisCount + sizeof( u32 );

	/** 旧形式で先頭に書く固定部分の大きさ。 */
	constexpr usize kLegacyFixedBytes =
		sizeof( u32 ) * 2u                          // magic + version
		+ sizeof( u32 )                             // tick
		+ sizeof( u64 )                             // draw count
		+ kLegacyRandomSnapshotBytes
		+ kLegacyClockSnapshotBytes
		+ sizeof( u32 );                            // rule byte count

	/** 現在形式で先頭に書く固定部分の大きさ。 */
	constexpr usize kFixedBytes =
		sizeof( u32 ) * 2u                          // magic + version
		+ sizeof( u32 )                             // tick
		+ sizeof( u64 )                             // draw count
		+ kRandomSnapshotBytes
		+ kClockSnapshotBytes
		+ sizeof( u32 )                             // has input history
		+ kActionInputBytes * 2u                    // last + previous input
		+ sizeof( u32 );                            // rule byte count

	/**
	 * バイト列へ値を書き進める。
	 *
	 * @param Cursor 書き位置 (進む)。
	 * @param Value 書く値。
	 */
	template <typename T>
	void WriteValue( u8*& Cursor, const T& Value ) noexcept
	{
		MemCopy( Cursor, &Value, sizeof( T ) );
		Cursor += sizeof( T );
	}

	/**
	 * バイト列から値を読み進める。
	 *
	 * @param Cursor 読み位置 (進む)。
	 * @param OutValue 読んだ値の入れ先。
	 */
	template <typename T>
	void ReadValue( const u8*& Cursor, T& OutValue ) noexcept
	{
		MemCopy( &OutValue, Cursor, sizeof( T ) );
		Cursor += sizeof( T );
	}

	/** アクション入力を構造体の余白に依存せず書き出す。 */
	void WriteActionInput( u8*& Cursor, const FActionInput& Input ) noexcept
	{
		for ( u32 AxisIndex = 0u; AxisIndex < kActionAxisCount; ++AxisIndex ) WriteValue( Cursor, Input.Axes[AxisIndex] );
		WriteValue( Cursor, Input.Buttons );
	}

	/** アクション入力を構造体の余白に依存せず読み込む。 */
	void ReadActionInput( const u8*& Cursor, FActionInput& OutInput ) noexcept
	{
		for ( u32 AxisIndex = 0u; AxisIndex < kActionAxisCount; ++AxisIndex ) ReadValue( Cursor, OutInput.Axes[AxisIndex] );
		ReadValue( Cursor, OutInput.Buttons );
	}

	/** 乱数状態を型のpaddingへ依存せず書き出す。 */
	void WriteRandomSnapshot( u8*& Cursor, const FRandomSnapshot& Snapshot ) noexcept
	{
		WriteValue( Cursor, Snapshot.version );
		WriteValue( Cursor, Snapshot.state0 );
		WriteValue( Cursor, Snapshot.state1 );
		WriteValue( Cursor, Snapshot.state2 );
		WriteValue( Cursor, Snapshot.state3 );
		WriteValue( Cursor, Snapshot.reserved );
		WriteValue( Cursor, Snapshot.signature );
	}

	/** 乱数状態を型のpaddingへ依存せず読み込む。 */
	void ReadRandomSnapshot( const u8*& Cursor, FRandomSnapshot& OutSnapshot ) noexcept
	{
		ReadValue( Cursor, OutSnapshot.version );
		ReadValue( Cursor, OutSnapshot.state0 );
		ReadValue( Cursor, OutSnapshot.state1 );
		ReadValue( Cursor, OutSnapshot.state2 );
		ReadValue( Cursor, OutSnapshot.state3 );
		ReadValue( Cursor, OutSnapshot.reserved );
		ReadValue( Cursor, OutSnapshot.signature );
	}

	/** 時計状態を型のpaddingへ依存せず書き出す。 */
	void WriteClockSnapshot( u8*& Cursor, const FFixedStepClockSnapshot& Snapshot ) noexcept
	{
		WriteValue( Cursor, Snapshot.options.step_seconds );
		WriteValue( Cursor, Snapshot.options.maximum_steps_per_advance );
		WriteValue( Cursor, Snapshot.options.maximum_accumulated_seconds );
		WriteValue( Cursor, Snapshot.accumulated_seconds );
		WriteValue( Cursor, Snapshot.total_dropped_seconds );
		WriteValue( Cursor, Snapshot.total_step_count );
	}

	/** 時計状態を型のpaddingへ依存せず読み込む。 */
	void ReadClockSnapshot( const u8*& Cursor, FFixedStepClockSnapshot& OutSnapshot ) noexcept
	{
		ReadValue( Cursor, OutSnapshot.options.step_seconds );
		ReadValue( Cursor, OutSnapshot.options.maximum_steps_per_advance );
		ReadValue( Cursor, OutSnapshot.options.maximum_accumulated_seconds );
		ReadValue( Cursor, OutSnapshot.accumulated_seconds );
		ReadValue( Cursor, OutSnapshot.total_dropped_seconds );
		ReadValue( Cursor, OutSnapshot.total_step_count );
	}
}


bool CSimulationSnapshot::TryCaptureFrom( const CFixedStepDriver& Driver, const CDeterministicRandom& Random, const ISimulationRule& Rule ) noexcept
{
	Clear();

	// 盤面を出せない規則なら、時計と乱数だけ写しても «続きから» にはならない。写さない。
	if ( !Rule.TrySaveState( m_RuleBytes ) )
	{
		m_RuleBytes.Reset();
		return false;
	}

	if ( !Driver.TryCaptureSnapshot( m_ClockState, m_Tick ) )
	{
		Clear();
		return false;
	}

	Random.CaptureSnapshot( m_RandomState, m_DrawCount );

	m_bValid = true;

	return true;
}


bool CSimulationSnapshot::TryCaptureFrom( const CFixedStepDriver& Driver, const CDeterministicRandom& Random, const ISimulationRule& Rule,
	const FActionInput& LastInput, const FActionInput& PreviousInput ) noexcept
{
	if ( !TryCaptureFrom( Driver, Random, Rule ) ) return false;

	m_LastInput = LastInput;
	m_PreviousInput = PreviousInput;
	m_bHasInputHistory = true;

	return true;
}


bool CSimulationSnapshot::TryRestoreTo( CFixedStepDriver& OutDriver, CDeterministicRandom& OutRandom, ISimulationRule& OutRule ) const noexcept
{
	if ( !m_bValid ) return false;

	// 途中で失敗しても «一部だけ戻った» 状態を作らないよう、先に戻せるかを確かめる。
	// 時計と乱数は写しから復元するだけなので、失敗するとしたら形が合わないときだけ。
	CFixedStepDriver ProbeDriver;
	if ( !ProbeDriver.TryRestoreSnapshot( m_ClockState, m_Tick ) ) return false;

	CDeterministicRandom ProbeRandom;
	if ( !ProbeRandom.TryRestoreSnapshot( m_RandomState, m_DrawCount ) ) return false;

	if ( !OutRule.TryRestoreState( m_RuleBytes.GetData(), m_RuleBytes.Num() ) ) return false;

	OutDriver.TryRestoreSnapshot( m_ClockState, m_Tick );
	OutRandom.TryRestoreSnapshot( m_RandomState, m_DrawCount );

	return true;
}


bool CSimulationSnapshot::TryRestoreTo( CFixedStepDriver& OutDriver, CDeterministicRandom& OutRandom, ISimulationRule& OutRule,
	FActionInput& OutLastInput, FActionInput& OutPreviousInput ) const noexcept
{
	if ( !m_bHasInputHistory ) return false;

	const FActionInput RestoredLastInput = m_LastInput;
	const FActionInput RestoredPreviousInput = m_PreviousInput;

	if ( !TryRestoreTo( OutDriver, OutRandom, OutRule ) ) return false;

	OutLastInput = RestoredLastInput;
	OutPreviousInput = RestoredPreviousInput;

	return true;
}


void CSimulationSnapshot::Clear() noexcept
{
	m_Tick = 0u;
	m_DrawCount = 0u;
	m_RandomState = FRandomSnapshot{};
	m_ClockState = FFixedStepClockSnapshot{};
	m_RuleBytes.Reset();
	m_LastInput = FActionInput();
	m_PreviousInput = FActionInput();
	m_bHasInputHistory = false;
	m_bValid = false;
}


usize CSimulationSnapshot::GetRequiredBytes() const noexcept
{
	return kFixedBytes + m_RuleBytes.Num();
}


bool CSimulationSnapshot::TrySaveToBuffer( u8* Buffer, usize Capacity, usize& OutWritten ) const noexcept
{
	OutWritten = 0u;

	if ( !m_bValid || Buffer == nullptr ) return false;
	if ( m_RuleBytes.Num() > static_cast<usize>( 0xFFFFFFFFu ) ) return false;

	const usize Required = GetRequiredBytes();
	if ( Capacity < Required ) return false;

	u8* Cursor = Buffer;

	WriteValue( Cursor, kSnapshotMagic );
	WriteValue( Cursor, kSnapshotVersion );
	WriteValue( Cursor, m_Tick );
	WriteValue( Cursor, m_DrawCount );
	WriteRandomSnapshot( Cursor, m_RandomState );
	WriteClockSnapshot( Cursor, m_ClockState );
	WriteValue( Cursor, m_bHasInputHistory ? 1u : 0u );
	WriteActionInput( Cursor, m_LastInput );
	WriteActionInput( Cursor, m_PreviousInput );
	WriteValue( Cursor, static_cast<u32>( m_RuleBytes.Num() ) );

	if ( m_RuleBytes.Num() != 0u )
	{
		MemCopy( Cursor, m_RuleBytes.GetData(), m_RuleBytes.Num() );
	}

	OutWritten = Required;

	return true;
}


bool CSimulationSnapshot::TryLoadFromBuffer( const u8* Buffer, usize Size ) noexcept
{
	Clear();

	if ( Buffer == nullptr || Size < sizeof( u32 ) * 2u ) return false;

	const u8* Cursor = Buffer;

	u32 Magic = 0u;
	u32 Version = 0u;
	u32 HasInputHistory = 0u;
	u32 RuleByteCount = 0u;

	ReadValue( Cursor, Magic );
	ReadValue( Cursor, Version );

	if ( Magic != kSnapshotMagic || ( Version != kLegacySnapshotVersion && Version != kSnapshotVersion ) )
	{
		Clear();
		return false;
	}

	const usize FixedBytes = ( Version == kLegacySnapshotVersion ) ? kLegacyFixedBytes : kFixedBytes;
	if ( Size < FixedBytes )
	{
		Clear();
		return false;
	}

	ReadValue( Cursor, m_Tick );
	ReadValue( Cursor, m_DrawCount );

	if ( Version == kLegacySnapshotVersion )
	{
		ReadValue( Cursor, m_RandomState );
		ReadValue( Cursor, m_ClockState );
	}
	else
	{
		ReadRandomSnapshot( Cursor, m_RandomState );
		ReadClockSnapshot( Cursor, m_ClockState );
		ReadValue( Cursor, HasInputHistory );
		ReadActionInput( Cursor, m_LastInput );
		ReadActionInput( Cursor, m_PreviousInput );

		if ( HasInputHistory > 1u )
		{
			Clear();
			return false;
		}

		m_bHasInputHistory = HasInputHistory == 1u;
		if ( !m_bHasInputHistory )
		{
			m_LastInput = FActionInput();
			m_PreviousInput = FActionInput();
		}
	}

	ReadValue( Cursor, RuleByteCount );

	const usize AvailableRuleBytes = Size - FixedBytes;
	if ( static_cast<usize>( RuleByteCount ) > AvailableRuleBytes
		|| ( Version == kSnapshotVersion && static_cast<usize>( RuleByteCount ) != AvailableRuleBytes ) )
	{
		Clear();
		return false;
	}

	if ( RuleByteCount != 0u )
	{
		if ( !m_RuleBytes.TryReserve( static_cast<usize>( RuleByteCount ) ) )
		{
			Clear();
			return false;
		}

		m_RuleBytes.SetNum( static_cast<usize>( RuleByteCount ) );
		MemCopy( m_RuleBytes.GetData(), Cursor, static_cast<usize>( RuleByteCount ) );
	}

	m_bValid = true;

	return true;
}
