// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotWriter.h"

#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotFormat.h"
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotStatus.h"

namespace
{
	/** 最初に用意しておく大きさ。小さなシーンなら次回以降も確保せず使える。 */
	constexpr usize kInitialBytes = 64u * 1024u;

	/** 失敗理由だけを持つ保存結果を作る。 */
	FSceneSaveResult MakeFailure( ESceneSerializeError Error, u32 RequiredBytes = 0u ) noexcept
	{
		FSceneSaveResult Failed;
		Failed.Error = Error;
		Failed.RequiredBytes = RequiredBytes;
		return Failed;
	}

	/**
	 * ACS と同じ深さ優先 (DFS) の先行順でノードを並べ、名前表の大きさを測る。
	 *
	 * @param Root 起点のノード。
	 * @param ExpectedNodeCount ACS が検証したノード数。
	 * @param OutNodes 並べたノード。
	 * @param OutNameBytes 長さ欄を含む名前表の大きさ。
	 * @return 成功なら None。構造変化、上限超過、確保失敗は対応する理由。
	 */
	ESceneSerializeError CollectNodes( const ANode& Root, u32 ExpectedNodeCount, TArray<const ANode*>& OutNodes, u32& OutNameBytes ) noexcept
	{
		OutNodes.Reset();
		OutNameBytes = 0u;
		if ( ExpectedNodeCount == 0u ) return ESceneSerializeError::EmptyTree;

		TArray<const ANode*> Stack;
		if ( !OutNodes.TryReserve( ExpectedNodeCount ) || !Stack.TryReserve( ExpectedNodeCount ) || !Stack.TryAdd( &Root ) )
		{
			return ESceneSerializeError::AllocationFailure;
		}

		while ( !Stack.IsEmpty() )
		{
			const ANode* const Node = Stack.Last();
			Stack.Pop();
			if ( Node == nullptr ) return ESceneSerializeError::InvalidStructure;
			if ( OutNodes.Num() >= ExpectedNodeCount ) return ESceneSerializeError::SceneChangedDuringSave;
			if ( !OutNodes.TryAdd( Node ) ) return ESceneSerializeError::AllocationFailure;

			const usize NameSize = Node->Name().Size();
			if ( NameSize > FSceneSnapshotFormat::MaxNodeNameBytes ) return ESceneSerializeError::SerializedSizeOverflow;

			const u64 NextNameBytes = static_cast<u64>( OutNameBytes ) + sizeof( u32 ) + NameSize;
			if ( NextNameBytes > static_cast<u64>( ~u32( 0 ) ) ) return ESceneSerializeError::SerializedSizeOverflow;
			OutNameBytes = static_cast<u32>( NextNameBytes );

			const u32 ChildCount = Node->ChildCount();
			for ( u32 ChildIndex = ChildCount; ChildIndex > 0u; --ChildIndex )
			{
				const ANode* const Child = Node->Child( ChildIndex - 1u );
				if ( Child == nullptr ) return ESceneSerializeError::InvalidStructure;
				if ( !Stack.TryAdd( Child ) ) return ESceneSerializeError::AllocationFailure;
			}
		}

		return OutNodes.Num() == ExpectedNodeCount ? ESceneSerializeError::None : ESceneSerializeError::SceneChangedDuringSave;
	}

	/**
	 * 保存後も同じ木か確かめながら、深さ優先 (DFS) の先行順で名前表を書く。
	 *
	 * @param Root 起点のノード。
	 * @param ExpectedNodes 保存前に並べたノード。
	 * @param Cursor 名前表の書き先。
	 * @param End 名前表の終端。
	 * @return 成功なら None。構造変化、上限超過、確保失敗は対応する理由。
	 */
	ESceneSerializeError WriteNames( const ANode& Root, const TArray<const ANode*>& ExpectedNodes, u8*& Cursor, const u8* End ) noexcept
	{
		TArray<const ANode*> Stack;
		if ( !Stack.TryReserve( ExpectedNodes.Num() ) || !Stack.TryAdd( &Root ) ) return ESceneSerializeError::AllocationFailure;

		usize NodeIndex = 0u;
		while ( !Stack.IsEmpty() )
		{
			const ANode* const Node = Stack.Last();
			Stack.Pop();
			if ( Node == nullptr || NodeIndex >= ExpectedNodes.Num() || Node != ExpectedNodes[NodeIndex] ) return ESceneSerializeError::SceneChangedDuringSave;
			++NodeIndex;

			const FStringView Name = Node->Name();
			if ( Name.Size() > FSceneSnapshotFormat::MaxNodeNameBytes ) return ESceneSerializeError::SerializedSizeOverflow;
			if ( !FSceneSnapshotFormat::WriteU32( Cursor, End, static_cast<u32>( Name.Size() ) ) ) return ESceneSerializeError::SceneChangedDuringSave;
			if ( Name.Size() > static_cast<usize>( End - Cursor ) ) return ESceneSerializeError::SceneChangedDuringSave;
			if ( Name.Size() != 0u ) MemCopy( Cursor, Name.Data(), Name.Size() );
			Cursor += Name.Size();

			const u32 ChildCount = Node->ChildCount();
			for ( u32 ChildIndex = ChildCount; ChildIndex > 0u; --ChildIndex )
			{
				const ANode* const Child = Node->Child( ChildIndex - 1u );
				if ( Child == nullptr ) return ESceneSerializeError::SceneChangedDuringSave;
				if ( !Stack.TryAdd( Child ) ) return ESceneSerializeError::AllocationFailure;
			}
		}

		return NodeIndex == ExpectedNodes.Num() && Cursor == End ? ESceneSerializeError::None : ESceneSerializeError::SceneChangedDuringSave;
	}
}


FSceneSaveResult CSceneSnapshotWriter::WriteTo( const ANode& Root, CSceneSnapshotBuffer& Buffer ) noexcept
{
	const FSceneSaveResult Measured = TrySaveNodeTree( &Root, nullptr, 0u );
	if ( !CSceneSnapshotStatus::IsBufferTooSmall( Measured.Error ) ) return Measured;

	TArray<const ANode*> Nodes;
	u32 NameBytes = 0u;
	const ESceneSerializeError Collected = CollectNodes( Root, Measured.NodeCount, Nodes, NameBytes );
	if ( Collected != ESceneSerializeError::None ) return MakeFailure( Collected );

	const u64 TotalBytes64 = static_cast<u64>( FSceneSnapshotFormat::HeaderBytes ) + Measured.RequiredBytes + NameBytes;
	if ( TotalBytes64 > static_cast<u64>( ~u32( 0 ) ) ) return MakeFailure( ESceneSerializeError::SerializedSizeOverflow );

	const u32 TotalBytes = static_cast<u32>( TotalBytes64 );
	const usize BufferBytes = TotalBytes > kInitialBytes ? static_cast<usize>( TotalBytes ) : kInitialBytes;
	if ( !Buffer.EnsureSize( BufferBytes ) ) return MakeFailure( ESceneSerializeError::AllocationFailure, TotalBytes );

	u8* const EngineData = Buffer.Data() + FSceneSnapshotFormat::HeaderBytes;
	FSceneSaveResult Saved = TrySaveNodeTree( &Root, EngineData, Measured.RequiredBytes );
	if ( !Saved.Succeeded() ) return Saved;
	if ( Saved.BytesWritten != Measured.RequiredBytes || Saved.NodeCount != Measured.NodeCount )
	{
		return MakeFailure( ESceneSerializeError::SceneChangedDuringSave, TotalBytes );
	}

	u8* NameCursor = EngineData + Saved.BytesWritten;
	const u8* const NameEnd = NameCursor + NameBytes;
	const ESceneSerializeError NamesWritten = WriteNames( Root, Nodes, NameCursor, NameEnd );
	if ( NamesWritten != ESceneSerializeError::None ) return MakeFailure( NamesWritten, TotalBytes );

	if ( !FSceneSnapshotFormat::WriteHeader( Buffer.Data(), Buffer.Size(), Saved.BytesWritten, Saved.NodeCount, NameBytes ) )
	{
		return MakeFailure( ESceneSerializeError::AllocationFailure, TotalBytes );
	}

	Saved.BytesWritten = TotalBytes;
	Saved.RequiredBytes = TotalBytes;
	return Saved;
}
