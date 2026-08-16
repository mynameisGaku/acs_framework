// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/SimulationSnapshot.h"

namespace
{
	/** スナップショットの目印。別形式のバイト列を読み込まないための確認に使う。 */
	constexpr u32 kSnapshotMagic = 0xAC575A00u;

	/** スナップショットの形式の版。並びを変えたら上げる。 */
	constexpr u32 kSnapshotVersion = 1u;

	/** 先頭に書く固定部分の大きさ。 */
	constexpr usize kFixedBytes =
		sizeof( u32 ) * 2u                          // magic + version
		+ sizeof( u32 )                             // tick
		+ sizeof( u64 )                             // draw count
		+ sizeof( FRandomSnapshot )
		+ sizeof( FFixedStepClockSnapshot )
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


void CSimulationSnapshot::Clear() noexcept
{
	m_Tick = 0u;
	m_DrawCount = 0u;
	m_RandomState = FRandomSnapshot{};
	m_ClockState = FFixedStepClockSnapshot{};
	m_RuleBytes.Reset();
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

	const usize Required = GetRequiredBytes();
	if ( Capacity < Required ) return false;

	u8* Cursor = Buffer;

	WriteValue( Cursor, kSnapshotMagic );
	WriteValue( Cursor, kSnapshotVersion );
	WriteValue( Cursor, m_Tick );
	WriteValue( Cursor, m_DrawCount );
	WriteValue( Cursor, m_RandomState );
	WriteValue( Cursor, m_ClockState );
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

	if ( Buffer == nullptr || Size < kFixedBytes ) return false;

	const u8* Cursor = Buffer;

	u32 Magic = 0u;
	u32 Version = 0u;
	u32 RuleByteCount = 0u;

	ReadValue( Cursor, Magic );
	ReadValue( Cursor, Version );
	ReadValue( Cursor, m_Tick );
	ReadValue( Cursor, m_DrawCount );
	ReadValue( Cursor, m_RandomState );
	ReadValue( Cursor, m_ClockState );
	ReadValue( Cursor, RuleByteCount );

	if ( Magic != kSnapshotMagic || Version != kSnapshotVersion )
	{
		Clear();
		return false;
	}

	if ( Size < kFixedBytes + static_cast<usize>( RuleByteCount ) )
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
