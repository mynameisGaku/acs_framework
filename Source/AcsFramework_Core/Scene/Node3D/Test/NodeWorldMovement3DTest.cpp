// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Node3D/NodeWorldMovement3D.h"
#include "Common/Test/TestHarness.h"

#include <cmath>
#include <limits>

namespace
{
	/** 名前付きノードを指定親へ置く。 */
	ANode* SpawnNode( CSceneNodeGraph& Graph, const char* Name, ANode* Parent = nullptr ) noexcept
	{
		/** 場面グラフが所有する生成結果。 */
		const FScene3DSpawnResult Spawned = Graph.TrySpawn( FStringView( Name ), Parent );
		return Spawned ? Spawned.Node : nullptr;
	}

	/** 3成分を小さな浮動小数誤差込みで比較する。 */
	void CheckVectorNear( CTestHarness& Harness, FVec3 Actual, FVec3 Expected, const char* Label ) noexcept
	{
		/** 3軸すべてが許容差内かどうか。 */
		const bool bNear = std::abs( Actual.x - Expected.x ) <= 1.0e-4f
			&& std::abs( Actual.y - Expected.y ) <= 1.0e-4f
			&& std::abs( Actual.z - Expected.z ) <= 1.0e-4f;
		Harness.Check( bNear, Label );
	}

	/** 計算失敗時に出力の番兵値が保たれることを確かめる。 */
	void CheckOutputPreserved( CTestHarness& Harness, const ANode& Node, FVec3 WorldTranslation, const char* Label ) noexcept
	{
		/** 失敗前後を比較するための番兵値。 */
		FVec3 LocalPosition{ 17.0f, -23.0f, 31.0f };
		/** 対象入力を受理できたかどうか。 */
		const bool bSucceeded = TryCalculateLocalPositionAfterWorldTranslation3D( Node, WorldTranslation, LocalPosition );
		Harness.Check( !bSucceeded && LocalPosition.x == 17.0f && LocalPosition.y == -23.0f && LocalPosition.z == 31.0f, Label );
	}
}


void RunNodeWorldMovement3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "TryTranslateNodeWorld3D / 親なしノードをworld量で動かす" );

	{
		/** 対象ノードを所有する場面グラフ。 */
		CSceneNodeGraph Graph;
		/** world移動する親なしノード。 */
		ANode* const Node = SpawnNode( Graph, "WorldMovingRoot" );
		Harness.Check( Node != nullptr, "親なしノードを置ける" );
		if ( Node == nullptr ) return;
		Node->SetPosition( FVec3{ 1.0f, 2.0f, 3.0f } );

		/** 計算だけを行った後のローカル位置。 */
		FVec3 CalculatedLocalPosition{};
		Harness.Check( TryCalculateLocalPositionAfterWorldTranslation3D( *Node, FVec3{ 4.0f, -1.0f, 2.0f }, CalculatedLocalPosition ), "親なしの適用先を計算できる" );
		CheckVectorNear( Harness, CalculatedLocalPosition, FVec3{ 5.0f, 1.0f, 5.0f }, "親なしではworldとlocalが一致する" );
		CheckVectorNear( Harness, Node->Position(), FVec3{ 1.0f, 2.0f, 3.0f }, "計算だけではノードを変更しない" );

		Harness.Check( TryTranslateNodeWorld3D( *Node, FVec3{ 4.0f, -1.0f, 2.0f } ), "親なしノードへworld移動を適用できる" );
		CheckVectorNear( Harness, Node->Position(), FVec3{ 5.0f, 1.0f, 5.0f }, "親なしノードのローカル位置を移動する" );
		CheckVectorNear( Harness, Node->World().position, FVec3{ 5.0f, 1.0f, 5.0f }, "親なしノードのworld位置を移動する" );
	}

	Harness.BeginSuite( "TryTranslateNodeWorld3D / 親Transformをローカル位置へ戻す" );

	{
		/** 親子ノードを所有する場面グラフ。 */
		CSceneNodeGraph Graph;
		/** 移動・回転・非一様拡縮を持つ親ノード。 */
		ANode* const Parent = SpawnNode( Graph, "WorldMovingParent" );
		/** world移動する子ノード。 */
		ANode* const Child = Parent != nullptr ? SpawnNode( Graph, "WorldMovingChild", Parent ) : nullptr;
		Harness.Check( Parent != nullptr && Child != nullptr, "親子ノードを置ける" );
		if ( Parent == nullptr || Child == nullptr ) return;

		Parent->SetPosition( FVec3{ 10.0f, 2.0f, -5.0f } );
		Parent->RotateDeg( FVec3{ 0.0f, 90.0f, 0.0f } );
		Parent->SetScale( FVec3{ 2.0f, 3.0f, 4.0f } );
		Child->SetPosition( FVec3{ 1.0f, 2.0f, 3.0f } );

		/** 計算前の子ノードのローカル位置。 */
		const FVec3 LocalBefore = Child->Position();
		/** 計算前の子ノードのworld位置。 */
		const FVec3 WorldBefore = Child->World().position;
		/** 親座標へ戻して適用するworld移動量。 */
		const FVec3 WorldTranslation{ 2.0f, -3.0f, 4.0f };
		/** world移動後に必要な子ノードのローカル位置。 */
		FVec3 CalculatedLocalPosition{};
		Harness.Check( TryCalculateLocalPositionAfterWorldTranslation3D( *Child, WorldTranslation, CalculatedLocalPosition ), "親付きの適用先を計算できる" );
		CheckVectorNear( Harness, Child->Position(), LocalBefore, "親付き計算でもノードを変更しない" );

		Child->SetPosition( CalculatedLocalPosition );
		CheckVectorNear( Harness, Child->World().position, WorldBefore + WorldTranslation, "計算したローカル位置でworld移動量が一致する" );
		Child->SetPosition( LocalBefore );

		Harness.Check( TryTranslateNodeWorld3D( *Child, WorldTranslation ), "親付きノードへworld移動を直接適用できる" );
		CheckVectorNear( Harness, Child->World().position, WorldBefore + WorldTranslation, "直接適用でもworld移動量が一致する" );
	}

	Harness.BeginSuite( "TryTranslateNodeWorld3D / 不正入力で出力とノードを保つ" );

	{
		/** 失敗時の状態不変を確認する場面グラフ。 */
		CSceneNodeGraph Graph;
		/** 通常の失敗確認に使うノード。 */
		ANode* const Node = SpawnNode( Graph, "WorldMovingSafeNode" );
		Harness.Check( Node != nullptr, "失敗確認用ノードを置ける" );
		if ( Node == nullptr ) return;
		Node->SetPosition( FVec3{ 1.0f, 2.0f, 3.0f } );

		/** 有限入力ではないことを表す値。 */
		const f32 Infinity = std::numeric_limits<f32>::infinity();
		/** 数値ではない入力を表す値。 */
		const f32 NotANumber = std::numeric_limits<f32>::quiet_NaN();
		/** world位置との加算を溢れさせる有限値。 */
		const f32 MaximumFinite = std::numeric_limits<f32>::max();
		CheckOutputPreserved( Harness, *Node, FVec3{ Infinity, 0.0f, 0.0f }, "無限大の移動量で計算出力を保つ" );
		CheckOutputPreserved( Harness, *Node, FVec3{ 0.0f, NotANumber, 0.0f }, "NaNの移動量で計算出力を保つ" );

		Node->SetPosition( FVec3{ MaximumFinite, 2.0f, 3.0f } );
		CheckOutputPreserved( Harness, *Node, FVec3{ MaximumFinite, 0.0f, 0.0f }, "world位置の加算overflowで計算出力を保つ" );
		Node->SetPosition( FVec3{ 1.0f, 2.0f, 3.0f } );

		/** 直接適用失敗前のローカル位置。 */
		const FVec3 LocalBefore = Node->Position();
		Harness.Check( !TryTranslateNodeWorld3D( *Node, FVec3{ Infinity, 0.0f, 0.0f } ), "無限大の移動量を直接適用しない" );
		CheckVectorNear( Harness, Node->Position(), LocalBefore, "直接適用失敗でノード位置を保つ" );

		Node->Destroy();
		CheckOutputPreserved( Harness, *Node, FVec3{ 1.0f, 0.0f, 0.0f }, "破棄予定ノードで計算出力を保つ" );
		Harness.Check( !TryTranslateNodeWorld3D( *Node, FVec3{ 1.0f, 0.0f, 0.0f } ), "破棄予定ノードを直接移動しない" );
		CheckVectorNear( Harness, Node->Position(), LocalBefore, "破棄予定ノードの位置を保つ" );
	}

	Harness.BeginSuite( "TryTranslateNodeWorld3D / 逆変換できない親を拒否する" );

	{
		/** 非可逆な親子ノードを所有する場面グラフ。 */
		CSceneNodeGraph Graph;
		/** X軸を0倍にする親ノード。 */
		ANode* const Parent = SpawnNode( Graph, "SingularWorldMovingParent" );
		/** 非可逆な親の子ノード。 */
		ANode* const Child = Parent != nullptr ? SpawnNode( Graph, "SingularWorldMovingChild", Parent ) : nullptr;
		Harness.Check( Parent != nullptr && Child != nullptr, "非可逆確認用の親子ノードを置ける" );
		if ( Parent == nullptr || Child == nullptr ) return;

		Parent->SetScale( FVec3{ 0.0f, 1.0f, 1.0f } );
		Child->SetPosition( FVec3{ 1.0f, 2.0f, 3.0f } );
		/** 逆変換失敗前の子ノードのローカル位置。 */
		const FVec3 LocalBefore = Child->Position();
		CheckOutputPreserved( Harness, *Child, FVec3{ 1.0f, 0.0f, 0.0f }, "非可逆な親で計算出力を保つ" );
		Harness.Check( !TryTranslateNodeWorld3D( *Child, FVec3{ 1.0f, 0.0f, 0.0f } ), "非可逆な親の子を直接移動しない" );
		CheckVectorNear( Harness, Child->Position(), LocalBefore, "非可逆な親で子ノード位置を保つ" );
	}
}
