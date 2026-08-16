// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotStatus.h"


bool CSceneSnapshotStatus::IsCorruptData( ESceneSerializeError Error ) noexcept
{
	switch ( Error )
	{
	case ESceneSerializeError::TruncatedData:
	case ESceneSerializeError::InvalidMagic:
	case ESceneSerializeError::UnsupportedVersion:
	case ESceneSerializeError::InvalidComponentName:
	case ESceneSerializeError::InvalidComponentPayload:
	case ESceneSerializeError::InvalidStructure:
	case ESceneSerializeError::DuplicateNodeReference:
	case ESceneSerializeError::CyclicNodeGraph:
		return true;

	default:
		return false;
	}
}


FString CSceneSnapshotStatus::MakeMessage( ESceneSerializeError Error )
{
	switch ( Error )
	{
	case ESceneSerializeError::None:                          return FString( "問題ありません" );
	case ESceneSerializeError::NullInput:                     return FString( "渡されたデータがありません" );
	case ESceneSerializeError::TruncatedData:                 return FString( "データが途中で切れています" );
	case ESceneSerializeError::InvalidMagic:                  return FString( "この形式のデータではありません" );
	case ESceneSerializeError::UnsupportedVersion:            return FString( "対応していない版のデータです" );
	case ESceneSerializeError::EmptyTree:                     return FString( "書き出すノードがありません" );
	case ESceneSerializeError::NodeLimitExceeded:             return FString( "ノードが多すぎます (上限 65536)" );
	case ESceneSerializeError::ComponentLimitExceeded:        return FString( "1 ノードあたりのコンポーネントが多すぎます (上限 1024)" );
	case ESceneSerializeError::InvalidComponentName:          return FString( "コンポーネント名を読めません" );
	case ESceneSerializeError::ComponentPayloadLimitExceeded: return FString( "コンポーネントの中身が大きすぎます (上限 4096 バイト)" );
	case ESceneSerializeError::InvalidComponentPayload:       return FString( "コンポーネントの中身を読めません" );
	case ESceneSerializeError::InvalidStructure:              return FString( "木の形が壊れています" );
	case ESceneSerializeError::AllocationFailure:             return FString( "メモリを確保できませんでした" );
	case ESceneSerializeError::NullRoot:                      return FString( "起点のノードがありません" );
	case ESceneSerializeError::NullOutput:                    return FString( "書き出し先がありません" );
	case ESceneSerializeError::BufferTooSmall:                return FString( "入れ物が足りません (大きくすればやり直せます)" );
	case ESceneSerializeError::DuplicateNodeReference:        return FString( "同じノードが二重に現れています" );
	case ESceneSerializeError::CyclicNodeGraph:               return FString( "親子が輪になっています" );
	case ESceneSerializeError::SerializedSizeOverflow:        return FString( "書き出しの大きさが上限を超えました" );
	case ESceneSerializeError::SceneChangedDuringSave:        return FString( "書き出している最中にシーンが変わりました" );
	default:                                                  return FString( "分からない結果です" );
	}
}
