// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Simulation/ActionInputTape.h"

namespace
{
	/** テープの目印。別形式のバイト列を読み込まないための確認に使う。 */
	constexpr u32 kTapeMagic = 0xAC57A1E0u;

	/** テープの形式の版。並びを変えたら上げる。 */
	constexpr u32 kTapeVersion = 1u;

	/** 1 件を書き出したときの大きさ (Tick + Axes + Buttons)。 */
	constexpr usize kEntryBytes = sizeof( u32 ) + sizeof( f32 ) * kActionAxisCount + sizeof( u32 );

	/** 先頭に書く情報の大きさ (magic + version + seed + count + lastTick)。 */
	constexpr usize kHeaderBytes = sizeof( u32 ) * 2u + sizeof( u64 ) + sizeof( u32 ) * 2u;

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


bool CActionInputTape::Record( u32 Tick, const FActionInput& Input ) noexcept
{
	// 直前と同じなら書かない。入力は大半のティックで変わらないので、
	// 全部書くとテープが無駄に伸びる。
	if ( m_Entries.Num() != 0u )
	{
		const FEntry& Last = m_Entries[m_Entries.Num() - 1u];
		if ( Tick < Last.Tick ) return false;

		if ( Last.Input.Equals( Input ) )
		{
			if ( Tick > m_LastTick ) m_LastTick = Tick;
			return true;
		}
	}

	FEntry Entry;
	Entry.Tick = Tick;
	Entry.Input = Input;

	if ( !m_Entries.TryAdd( Entry ) ) return false;

	if ( Tick > m_LastTick ) m_LastTick = Tick;

	return true;
}


bool CActionInputTape::TryGet( u32 Tick, FActionInput& OutInput ) const noexcept
{
	const usize Index = FindEntryIndex( Tick );
	if ( Index >= m_Entries.Num() ) return false;

	OutInput = m_Entries[Index].Input;

	return true;
}


void CActionInputTape::Clear() noexcept
{
	m_Entries.Reset();
	m_LastTick = 0u;
	m_Seed = 0u;
}


usize CActionInputTape::GetRequiredBytes() const noexcept
{
	return kHeaderBytes + kEntryBytes * m_Entries.Num();
}


bool CActionInputTape::TrySaveToBuffer( u8* Buffer, usize Capacity, usize& OutWritten ) const noexcept
{
	OutWritten = 0u;

	const usize Required = GetRequiredBytes();
	if ( Buffer == nullptr || Capacity < Required ) return false;

	u8* Cursor = Buffer;

	WriteValue( Cursor, kTapeMagic );
	WriteValue( Cursor, kTapeVersion );
	WriteValue( Cursor, m_Seed );
	WriteValue( Cursor, static_cast<u32>( m_Entries.Num() ) );
	WriteValue( Cursor, m_LastTick );

	for ( usize Index = 0u; Index < m_Entries.Num(); ++Index )
	{
		const FEntry& Entry = m_Entries[Index];

		WriteValue( Cursor, Entry.Tick );
		for ( u32 Axis = 0u; Axis < kActionAxisCount; ++Axis ) WriteValue( Cursor, Entry.Input.Axes[Axis] );
		WriteValue( Cursor, Entry.Input.Buttons );
	}

	OutWritten = Required;

	return true;
}


bool CActionInputTape::TryLoadFromBuffer( const u8* Buffer, usize Size ) noexcept
{
	Clear();

	if ( Buffer == nullptr || Size < kHeaderBytes ) return false;

	const u8* Cursor = Buffer;

	u32 Magic = 0u;
	u32 Version = 0u;
	u64 Seed = 0u;
	u32 Count = 0u;
	u32 LastTick = 0u;

	ReadValue( Cursor, Magic );
	ReadValue( Cursor, Version );
	ReadValue( Cursor, Seed );
	ReadValue( Cursor, Count );
	ReadValue( Cursor, LastTick );

	if ( Magic != kTapeMagic || Version != kTapeVersion ) return false;
	if ( Size < kHeaderBytes + kEntryBytes * static_cast<usize>( Count ) ) return false;

	if ( !m_Entries.TryReserve( static_cast<usize>( Count ) ) ) return false;

	for ( u32 Index = 0u; Index < Count; ++Index )
	{
		FEntry Entry;

		ReadValue( Cursor, Entry.Tick );
		for ( u32 Axis = 0u; Axis < kActionAxisCount; ++Axis ) ReadValue( Cursor, Entry.Input.Axes[Axis] );
		ReadValue( Cursor, Entry.Input.Buttons );

		// 並びが崩れたテープは読まない。読めてしまうと、再生が静かにずれる。
		if ( Index != 0u && Entry.Tick < m_Entries[m_Entries.Num() - 1u].Tick )
		{
			Clear();
			return false;
		}

		if ( !m_Entries.TryAdd( Entry ) )
		{
			Clear();
			return false;
		}
	}

	m_Seed = Seed;
	m_LastTick = LastTick;

	return true;
}


usize CActionInputTape::FindEntryIndex( u32 Tick ) const noexcept
{
	if ( m_Entries.Num() == 0u ) return m_Entries.Num();
	if ( Tick < m_Entries[0].Tick ) return m_Entries.Num();

	// Tick 以下で最も後ろの記録を二分探索で探す。テープは Tick の昇順で並んでいる。
	usize Low = 0u;
	usize High = m_Entries.Num() - 1u;

	while ( Low < High )
	{
		const usize Middle = Low + ( High - Low + 1u ) / 2u;

		if ( m_Entries[Middle].Tick <= Tick ) Low = Middle;
		else                                  High = Middle - 1u;
	}

	return Low;
}
