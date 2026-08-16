// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotStatus.h"
#include "Common/Test/TestHarness.h"


void RunSceneSnapshotStatusTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "CSceneSnapshotStatus / やり直す価値の判定" );

	{
		// ここを取り違えると「入れ物が小さいだけ」を «壊れている» と扱って諦めたり、
		// 逆に壊れたデータへ何度もやり直しをかけたりする。
		Harness.Check( CSceneSnapshotStatus::IsSuccess( ESceneSerializeError::None ), "None は成功" );
		Harness.Check( !CSceneSnapshotStatus::IsSuccess( ESceneSerializeError::BufferTooSmall ), "それ以外は成功でない" );

		Harness.Check( CSceneSnapshotStatus::IsBufferTooSmall( ESceneSerializeError::BufferTooSmall ), "入れ物不足を見分ける" );
		Harness.Check( !CSceneSnapshotStatus::IsBufferTooSmall( ESceneSerializeError::InvalidMagic ), "他と混ぜない" );

		Harness.Check( CSceneSnapshotStatus::IsCorruptData( ESceneSerializeError::InvalidMagic ), "目印違いは壊れている" );
		Harness.Check( CSceneSnapshotStatus::IsCorruptData( ESceneSerializeError::TruncatedData ), "途中で切れも壊れている" );
		Harness.Check( CSceneSnapshotStatus::IsCorruptData( ESceneSerializeError::CyclicNodeGraph ), "輪になった親子も壊れている" );

		Harness.Check( !CSceneSnapshotStatus::IsCorruptData( ESceneSerializeError::BufferTooSmall ), "入れ物不足は壊れていない" );
		Harness.Check( !CSceneSnapshotStatus::IsCorruptData( ESceneSerializeError::AllocationFailure ), "確保失敗も壊れていない" );
		Harness.Check( !CSceneSnapshotStatus::IsCorruptData( ESceneSerializeError::None ), "成功は壊れていない" );
	}

	Harness.BeginSuite( "CSceneSnapshotStatus / 説明が出る" );

	{
		Harness.Check( !CSceneSnapshotStatus::MakeMessage( ESceneSerializeError::None ).IsEmpty(), "None にも説明がある" );
		Harness.Check( !CSceneSnapshotStatus::MakeMessage( ESceneSerializeError::NodeLimitExceeded ).IsEmpty(), "上限超過に説明がある" );

		const FString Unknown = CSceneSnapshotStatus::MakeMessage( static_cast<ESceneSerializeError>( 200u ) );
		Harness.Check( !Unknown.IsEmpty(), "知らない値でも空にならない" );

		Harness.Check( CSceneSnapshotStatus::GetName( ESceneSerializeError::InvalidMagic ) != nullptr, "英名が取れる" );
	}
}
