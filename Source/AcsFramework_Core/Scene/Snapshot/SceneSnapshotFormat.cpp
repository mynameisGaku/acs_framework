// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotFormat.h"


bool FSceneSnapshotFormat::IsEnvelope( const u8* Data, usize Size ) noexcept
{
	if ( Data == nullptr || Size < sizeof( u32 ) ) return false;

	u32 FoundMagic = 0u;
	MemCopy( &FoundMagic, Data, sizeof( FoundMagic ) );
	return FoundMagic == Magic;
}


bool FSceneSnapshotFormat::WriteHeader( u8* Data, usize Size, u32 EngineBytes, u32 NodeCount, u32 NameBytes ) noexcept
{
	if ( Data == nullptr || Size < HeaderBytes ) return false;

	u8* Cursor = Data;
	const u8* const End = Data + HeaderBytes;
	return WriteU32( Cursor, End, Magic )
		&& WriteU32( Cursor, End, Version )
		&& WriteU32( Cursor, End, HeaderBytes )
		&& WriteU32( Cursor, End, EngineBytes )
		&& WriteU32( Cursor, End, NodeCount )
		&& WriteU32( Cursor, End, NameBytes )
		&& Cursor == End;
}


bool FSceneSnapshotFormat::ReadHeader( const u8* Data, usize Size, u32& OutVersion, u32& OutEngineBytes, u32& OutNodeCount, u32& OutNameBytes ) noexcept
{
	OutVersion = 0u;
	OutEngineBytes = 0u;
	OutNodeCount = 0u;
	OutNameBytes = 0u;

	if ( Data == nullptr || Size < HeaderBytes ) return false;

	const u8* Cursor = Data;
	const u8* const End = Data + HeaderBytes;
	u32 FoundMagic = 0u;
	u32 FoundHeaderBytes = 0u;
	return ReadU32( Cursor, End, FoundMagic )
		&& ReadU32( Cursor, End, OutVersion )
		&& ReadU32( Cursor, End, FoundHeaderBytes )
		&& ReadU32( Cursor, End, OutEngineBytes )
		&& ReadU32( Cursor, End, OutNodeCount )
		&& ReadU32( Cursor, End, OutNameBytes )
		&& Cursor == End
		&& FoundMagic == Magic
		&& FoundHeaderBytes == HeaderBytes;
}


bool FSceneSnapshotFormat::WriteU32( u8*& Cursor, const u8* End, u32 Value ) noexcept
{
	if ( Cursor == nullptr || End == nullptr || Cursor > End || static_cast<usize>( End - Cursor ) < sizeof( Value ) ) return false;

	MemCopy( Cursor, &Value, sizeof( Value ) );
	Cursor += sizeof( Value );
	return true;
}


bool FSceneSnapshotFormat::ReadU32( const u8*& Cursor, const u8* End, u32& OutValue ) noexcept
{
	OutValue = 0u;
	if ( Cursor == nullptr || End == nullptr || Cursor > End || static_cast<usize>( End - Cursor ) < sizeof( OutValue ) ) return false;

	MemCopy( &OutValue, Cursor, sizeof( OutValue ) );
	Cursor += sizeof( OutValue );
	return true;
}
