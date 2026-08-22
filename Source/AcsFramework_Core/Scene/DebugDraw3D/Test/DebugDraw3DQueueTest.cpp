// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/DebugDraw3D/DebugDraw3DQueue.h"
#include "Common/Test/TestHarness.h"

#include <limits>


void RunDebugDraw3DQueueTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FDebugLine3D / 描画前に不正値を止める" );

	{
		FDebugLine3D Line;
		Line.Start = FVec3{ 1.0f, 2.0f, 3.0f };
		Line.End = FVec3{ 4.0f, 5.0f, 6.0f };
		Harness.Check( Line.IsValid(), "既定色と有限座標を受け付ける" );

		Line.Start.x = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Line.IsValid(), "有限でない座標を拒否する" );
		Line.Start.x = 1.0f;
		Line.Color.z = 1.01f;
		Harness.Check( !Line.IsValid(), "表示域を超える色を拒否する" );
		Line.Color.z = 1.0f;
		Line.Color.w = 0.0f;
		Harness.Check( !Line.IsValid(), "完全に透明な線を拒否する" );
	}

	Harness.BeginSuite( "CDebugDraw3DQueue / 1フレーム線を上限付きで保持する" );

	{
		CDebugDraw3DQueue Queue( 2u );
		Harness.Check( Queue.TryLine( FVec3{ 0.0f, 0.0f, 0.0f }, FVec3{ 1.0f, 0.0f, 0.0f } ), "1本目を登録できる" );
		Harness.Check( Queue.TryLine( FVec3{ 0.0f, 0.0f, 0.0f }, FVec3{ 0.0f, 1.0f, 0.0f } ), "2本目を登録できる" );
		Harness.Check( !Queue.TryLine( FVec3{ 0.0f, 0.0f, 0.0f }, FVec3{ 0.0f, 0.0f, 1.0f } ), "上限を超える線を拒否する" );
		Harness.CheckEqualU64( Queue.Num(), 2u, "拒否後も登録済み本数を保つ" );
		Harness.CheckEqualU64( Queue.RejectedDrawCount(), 1u, "拒否した要求を数える" );
		Harness.CheckEqualF32( Queue.Get( 0u ).End.x, 1.0f, "登録順と座標を保つ" );

		Queue.Clear();
		Harness.CheckEqualU64( Queue.Num(), 0u, "描画後にキューだけを空にできる" );
		Harness.CheckEqualU64( Queue.RejectedDrawCount(), 1u, "累計診断値はClearで消さない" );
	}

	Harness.BeginSuite( "CDebugDraw3DQueue / AABBを12辺まとめて扱う" );

	{
		const FAabb3 Bounds = FAabb3::FromCenterExtents( FVec3{ 2.0f, 3.0f, 4.0f }, FVec3{ 1.0f, 2.0f, 3.0f } );
		CDebugDraw3DQueue Queue( 12u );
		Harness.Check( Queue.TryAabb( Bounds, FVec4{ 1.0f, 0.5f, 0.1f, 1.0f } ), "12辺が入るときだけ箱を登録できる" );
		Harness.CheckEqualU64( Queue.Num(), 12u, "箱を12本の線へ展開する" );
		Harness.CheckEqualF32( Queue.Get( 0u ).Start.x, 1.0f, "最初の辺は最小Xから始まる" );
		Harness.CheckEqualF32( Queue.Get( 0u ).End.x, 3.0f, "最初の辺は最大Xへ進む" );

		CDebugDraw3DQueue TooSmall( 11u );
		Harness.Check( !TooSmall.TryAabb( Bounds ), "12辺未満の空きでは箱を拒否する" );
		Harness.CheckEqualU64( TooSmall.Num(), 0u, "容量不足の箱を途中まで追加しない" );

		CDebugDraw3DQueue InvalidBox( 12u );
		Harness.Check( !InvalidBox.TryAabb( FAabb3::FromCenterExtents( FVec3{}, FVec3{ -1.0f, 1.0f, 1.0f } ) ), "負の半サイズを拒否する" );
		Harness.CheckEqualU64( InvalidBox.Num(), 0u, "不正な箱を登録しない" );
	}

	Harness.BeginSuite( "CDebugDraw3DQueue / 球を3方向の閉じた円へ展開する" );

	{
		const FSphere Sphere{ FVec3{ 1.0f, 2.0f, 3.0f }, 2.0f };
		CDebugDraw3DQueue Queue( 12u );
		Harness.Check( Queue.TrySphere( Sphere, FVec4{ 1.0f, 0.2f, 0.7f, 1.0f }, 4u ), "4分割の3円を登録できる" );
		Harness.CheckEqualU64( Queue.Num(), 12u, "分割数の3倍を線として保持する" );
		Harness.CheckEqualF32( Queue.Get( 0u ).Start.x, 3.0f, "XY円は半径分だけXへ始まる" );
		Harness.CheckNearF32( Queue.Get( 0u ).End.x, 1.0f, 0.00001f, "XY円の次点は中心Xへ戻る" );
		Harness.CheckNearF32( Queue.Get( 0u ).End.y, 4.0f, 0.00001f, "XY円の次点は半径分だけYへ進む" );
		Harness.CheckEqualF32( Queue.Get( 1u ).Start.z, 3.0f, "XZ円の開始Zは中心と同じ" );
		Harness.CheckNearF32( Queue.Get( 11u ).End.y, 4.0f, 0.00001f, "最後のYZ辺を開始点へ閉じる" );

		CDebugDraw3DQueue TooSmall( 12u );
		Harness.Check( TooSmall.TryLine( FVec3{}, FVec3{ 1.0f, 0.0f, 0.0f } ), "容量確認前の線を登録できる" );
		Harness.Check( !TooSmall.TrySphere( Sphere, FVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, 4u ), "全円が入らない球を拒否する" );
		Harness.CheckEqualU64( TooSmall.Num(), 1u, "容量不足でも既存線と球の原子性を保つ" );

		CDebugDraw3DQueue InvalidSphere;
		Harness.Check( !InvalidSphere.TrySphere( FSphere{ FVec3{}, 0.0f } ), "半径0の球を拒否する" );
		Harness.Check( !InvalidSphere.TrySphere( Sphere, FVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, 3u ), "小さすぎる分割数を拒否する" );
		Harness.Check( !InvalidSphere.TrySphere( Sphere, FVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, CDebugDraw3DQueue::kMaximumSphereSegments + 1u ), "大きすぎる分割数を拒否する" );
		/** 円周計算を有限範囲外へ押し出す最大有限値。 */
		const f32 Maximum = std::numeric_limits<f32>::max();
		Harness.Check( !InvalidSphere.TrySphere( FSphere{ FVec3{ Maximum, 0.0f, 0.0f }, Maximum } ), "円周計算で有限範囲を超える球を拒否する" );
		Harness.CheckEqualU64( InvalidSphere.Num(), 0u, "不正な球を途中まで登録しない" );
		Harness.CheckEqualU64( InvalidSphere.RejectedDrawCount(), 4u, "拒否した球要求を1件ずつ数える" );
	}
}
