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
}
