// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotReader.h"

#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotFormat.h"

namespace
{
	/** 指定理由の失敗結果を作る。 */
	FSceneLoadResult MakeFailure( ESceneSerializeError Error, u32 BytesRead = 0u, u32 FormatVersion = 0u ) noexcept
	{
		FSceneLoadResult Failed;
		Failed.Error = Error;
		Failed.BytesRead = BytesRead;
		Failed.FormatVersion = FormatVersion;
		return Failed;
	}

	/**
	 * 名前表を全て検証し、入力バイト列を指すビューへ分ける。
	 *
	 * @param Data 名前表の先頭。
	 * @param Size 名前表の大きさ。
	 * @param NodeCount 期待する名前数。
	 * @param OutNames 検証済みの名前ビュー。
	 * @return 成功なら None。不足、上限超過、確保失敗は対応する理由。
	 */
	ESceneSerializeError ParseNames( const u8* Data, usize Size, u32 NodeCount, TArray<FStringView>& OutNames ) noexcept
	{
		OutNames.Reset();
		if ( Data == nullptr || !OutNames.TryReserve( NodeCount ) ) return ESceneSerializeError::AllocationFailure;

		const u8* Cursor = Data;
		const u8* const End = Data + Size;
		for ( u32 NodeIndex = 0u; NodeIndex < NodeCount; ++NodeIndex )
		{
			u32 NameSize = 0u;
			if ( !FSceneSnapshotFormat::ReadU32( Cursor, End, NameSize ) ) return ESceneSerializeError::TruncatedData;
			if ( NameSize > FSceneSnapshotFormat::MaxNodeNameBytes ) return ESceneSerializeError::SerializedSizeOverflow;
			if ( NameSize > static_cast<usize>( End - Cursor ) ) return ESceneSerializeError::TruncatedData;
			if ( !OutNames.TryAdd( FStringView( reinterpret_cast<const char*>( Cursor ), NameSize ) ) )
			{
				return ESceneSerializeError::AllocationFailure;
			}
			Cursor += NameSize;
		}

		return Cursor == End ? ESceneSerializeError::None : ESceneSerializeError::InvalidStructure;
	}

	/**
	 * 復元した木へ深さ優先 (DFS) の先行順で名前を戻す。
	 *
	 * @param Root 復元した木の根。
	 * @param Names 順番に対応する名前。
	 * @return 成功なら None。構造不一致と確保失敗は対応する理由。
	 */
	ESceneSerializeError ApplyNames( ANode& Root, const TArray<FStringView>& Names ) noexcept
	{
		TArray<ANode*> Stack;
		if ( !Stack.TryReserve( Names.Num() ) || !Stack.TryAdd( &Root ) ) return ESceneSerializeError::AllocationFailure;

		usize NodeIndex = 0u;
		while ( !Stack.IsEmpty() )
		{
			ANode* const Node = Stack.Last();
			Stack.Pop();
			if ( Node == nullptr || NodeIndex >= Names.Num() ) return ESceneSerializeError::InvalidStructure;
			Node->SetName( Names[NodeIndex] );
			++NodeIndex;

			const u32 ChildCount = Node->ChildCount();
			for ( u32 ChildIndex = ChildCount; ChildIndex > 0u; --ChildIndex )
			{
				ANode* const Child = Node->Child( ChildIndex - 1u );
				if ( Child == nullptr ) return ESceneSerializeError::InvalidStructure;
				if ( !Stack.TryAdd( Child ) ) return ESceneSerializeError::AllocationFailure;
			}
		}

		return NodeIndex == Names.Num() ? ESceneSerializeError::None : ESceneSerializeError::InvalidStructure;
	}
}


FSceneLoadResult CSceneSnapshotReader::ReadFrom( const CSceneSnapshotBuffer& Buffer ) noexcept
{
	return ReadFrom( Buffer.Data(), Buffer.Size() );
}


FSceneLoadResult CSceneSnapshotReader::ReadFrom( const u8* Data, usize Size ) noexcept
{
	if ( Data == nullptr || Size == 0u )
	{
		FSceneLoadResult Failed;
		Failed.Error = ESceneSerializeError::NullInput;
		return Failed;
	}
	if ( Size > static_cast<usize>( ~u32( 0 ) ) ) return MakeFailure( ESceneSerializeError::SerializedSizeOverflow );

	// 旧 Framework が書いた生の ACS v2/v3/v4 は、今まで通り Engine へ直接渡す。
	if ( !FSceneSnapshotFormat::IsEnvelope( Data, Size ) ) return TryLoadNodeTree( Data, static_cast<u32>( Size ) );
	if ( Size < FSceneSnapshotFormat::HeaderBytes ) return MakeFailure( ESceneSerializeError::TruncatedData );

	u32 EnvelopeVersion = 0u;
	u32 EngineBytes = 0u;
	u32 NodeCount = 0u;
	u32 NameBytes = 0u;
	if ( !FSceneSnapshotFormat::ReadHeader( Data, Size, EnvelopeVersion, EngineBytes, NodeCount, NameBytes ) )
	{
		return MakeFailure( ESceneSerializeError::InvalidStructure );
	}
	if ( EnvelopeVersion != FSceneSnapshotFormat::Version )
	{
		return MakeFailure( ESceneSerializeError::UnsupportedVersion, FSceneSnapshotFormat::HeaderBytes, EnvelopeVersion );
	}
	if ( NodeCount == 0u ) return MakeFailure( ESceneSerializeError::EmptyTree, FSceneSnapshotFormat::HeaderBytes );
	if ( NodeCount > kSceneSerializeMaxNodeCount )
	{
		return MakeFailure( ESceneSerializeError::NodeLimitExceeded, FSceneSnapshotFormat::HeaderBytes );
	}

	const u64 TotalBytes64 = static_cast<u64>( FSceneSnapshotFormat::HeaderBytes ) + EngineBytes + NameBytes;
	if ( TotalBytes64 > Size ) return MakeFailure( ESceneSerializeError::TruncatedData, FSceneSnapshotFormat::HeaderBytes );

	const u8* const EngineData = Data + FSceneSnapshotFormat::HeaderBytes;
	const u8* const NameData = EngineData + EngineBytes;
	TArray<FStringView> Names;
	const ESceneSerializeError Parsed = ParseNames( NameData, NameBytes, NodeCount, Names );
	if ( Parsed != ESceneSerializeError::None ) return MakeFailure( Parsed, FSceneSnapshotFormat::HeaderBytes + EngineBytes );

	FSceneLoadResult Loaded = TryLoadNodeTree( EngineData, EngineBytes );
	if ( !Loaded.Succeeded() ) return Loaded;
	if ( Loaded.BytesRead != EngineBytes ) return MakeFailure( ESceneSerializeError::InvalidStructure, Loaded.BytesRead, Loaded.FormatVersion );
	const ESceneSerializeError Applied = ApplyNames( *Loaded.Root, Names );
	if ( Applied != ESceneSerializeError::None ) return MakeFailure( Applied, Loaded.BytesRead, Loaded.FormatVersion );

	Loaded.BytesRead = static_cast<u32>( TotalBytes64 );
	return Loaded;
}
