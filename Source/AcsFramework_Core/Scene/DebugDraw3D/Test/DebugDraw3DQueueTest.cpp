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

	Harness.BeginSuite( "CDebugDraw3DQueue / 方向を立体矢印として原子的に扱う" );

	{
		const FVec4 Color{ 1.0f, 0.72f, 0.16f, 1.0f };
		CDebugDraw3DQueue Queue( CDebugDraw3DQueue::kArrowLineCount );
		Harness.Check( Queue.TryArrow( FVec3{}, FVec3{ 1.0f, 0.0f, 0.0f }, Color, 0.25f ), "X方向の矢印を登録できる" );
		Harness.CheckEqualU64( Queue.Num(), CDebugDraw3DQueue::kArrowLineCount, "胴体1本と矢尻4本へ展開する" );
		Harness.CheckEqualF32( Queue.Get( 0u ).Start.x, 0.0f, "胴体を始点から登録する" );
		Harness.CheckEqualF32( Queue.Get( 0u ).End.x, 1.0f, "胴体を終点まで登録する" );
		Harness.CheckEqualF32( Queue.Get( 1u ).Start.x, 1.0f, "矢尻を終点から登録する" );
		Harness.CheckNearF32( Queue.Get( 1u ).End.x, 0.75f, 0.00001f, "矢尻の根元を指定長だけ戻す" );
		Harness.CheckNearF32( Queue.Get( 1u ).End.z, -0.125f, 0.00001f, "矢尻を進行方向と直交する向きへ開く" );

		CDebugDraw3DQueue VerticalQueue( CDebugDraw3DQueue::kArrowLineCount );
		Harness.Check( VerticalQueue.TryArrow( FVec3{}, FVec3{ 0.0f, 2.0f, 0.0f }, Color, 0.5f ), "真上方向でも矢尻の基準軸を切り替えられる" );
		Harness.CheckNearF32( VerticalQueue.Get( 1u ).End.y, 1.5f, 0.00001f, "真上方向でも矢尻を後方へ置く" );

		CDebugDraw3DQueue TooSmall( CDebugDraw3DQueue::kArrowLineCount );
		Harness.Check( TooSmall.TryLine( FVec3{}, FVec3{ 0.0f, 0.0f, 1.0f } ), "容量確認前の線を登録できる" );
		Harness.Check( !TooSmall.TryArrow( FVec3{}, FVec3{ 1.0f, 0.0f, 0.0f }, Color, 0.25f ), "5本分の空きがない矢印を拒否する" );
		Harness.CheckEqualU64( TooSmall.Num(), 1u, "容量不足でも既存線と矢印の原子性を保つ" );

		CDebugDraw3DQueue InvalidArrow;
		Harness.Check( !InvalidArrow.TryArrow( FVec3{}, FVec3{}, Color, 0.25f ), "長さ0の矢印を拒否する" );
		Harness.Check( !InvalidArrow.TryArrow( FVec3{}, FVec3{ 1.0f, 0.0f, 0.0f }, Color, 0.0f ), "長さ0の矢尻を拒否する" );
		Harness.Check( !InvalidArrow.TryArrow( FVec3{}, FVec3{ 1.0f, 0.0f, 0.0f }, Color, 1.01f ), "全体より長い矢尻を拒否する" );
		Harness.Check( !InvalidArrow.TryArrow( FVec3{}, FVec3{ 1.0f, 0.0f, 0.0f }, Color, std::numeric_limits<f32>::quiet_NaN() ), "有限でない矢尻長を拒否する" );
		/** 差分計算を有限範囲外へ押し出す最大有限値。 */
		const f32 Maximum = std::numeric_limits<f32>::max();
		Harness.Check( !InvalidArrow.TryArrow( FVec3{ Maximum, 0.0f, 0.0f }, FVec3{ -Maximum, 0.0f, 0.0f }, Color, 0.25f ), "方向計算で有限範囲を超える矢印を拒否する" );
		Harness.CheckEqualU64( InvalidArrow.Num(), 0u, "不正な矢印を途中まで登録しない" );
		Harness.CheckEqualU64( InvalidArrow.RejectedDrawCount(), 5u, "拒否した矢印要求を1件ずつ数える" );
	}

	Harness.BeginSuite( "CDebugDraw3DQueue / 回転した3D座標軸を原子的に扱う" );

	{
		const FVec3 Origin{ 1.0f, 2.0f, 3.0f };
		CDebugDraw3DQueue Queue( CDebugDraw3DQueue::kAxesLineCount );
		Harness.Check( Queue.TryAxes( Origin, FQuat::Identity(), 2.0f, 0.5f ), "world座標軸を登録できる" );
		Harness.CheckEqualU64( Queue.Num(), CDebugDraw3DQueue::kAxesLineCount, "3本の矢印を15本の線へ展開する" );
		Harness.CheckEqualF32( Queue.Get( 0u ).End.x, 3.0f, "最初の胴体を正のX軸へ伸ばす" );
		Harness.CheckEqualF32( Queue.Get( 0u ).Color.x, 1.0f, "X軸を赤で登録する" );
		Harness.CheckEqualF32( Queue.Get( CDebugDraw3DQueue::kArrowLineCount ).End.y, 4.0f, "2本目の胴体を正のY軸へ伸ばす" );
		Harness.CheckEqualF32( Queue.Get( CDebugDraw3DQueue::kArrowLineCount ).Color.y, 1.0f, "Y軸を緑で登録する" );
		Harness.CheckEqualF32( Queue.Get( CDebugDraw3DQueue::kArrowLineCount * 2u ).End.z, 5.0f, "3本目の胴体を正のZ軸へ伸ばす" );
		Harness.CheckEqualF32( Queue.Get( CDebugDraw3DQueue::kArrowLineCount * 2u ).Color.z, 1.0f, "Z軸を青で登録する" );

		CDebugDraw3DQueue NormalizedQueue( CDebugDraw3DQueue::kAxesLineCount );
		Harness.Check( NormalizedQueue.TryAxes( FVec3{}, FQuat{ 0.0f, 0.0f, 0.0f, 2.0f }, 1.0f, 0.2f ), "正規化されていない有限回転を受け付ける" );
		Harness.CheckNearF32( NormalizedQueue.Get( 0u ).End.x, 1.0f, 0.00001f, "回転を正規化して座標軸へ使う" );

		CDebugDraw3DQueue RotatedQueue( CDebugDraw3DQueue::kAxesLineCount );
		const FQuat HalfTurn = FQuat::AxisAngle( FVec3::Up(), 3.14159265358979323846f );
		Harness.Check( RotatedQueue.TryAxes( FVec3{}, HalfTurn, 1.0f, 0.2f ), "回転したローカル座標軸を登録できる" );
		Harness.CheckNearF32( RotatedQueue.Get( 0u ).End.x, -1.0f, 0.00001f, "Y軸半回転をローカルX軸へ反映する" );
		Harness.CheckNearF32( RotatedQueue.Get( CDebugDraw3DQueue::kArrowLineCount * 2u ).End.z, -1.0f, 0.00001f, "Y軸半回転をローカルZ軸へ反映する" );

		CDebugDraw3DQueue TooSmall( CDebugDraw3DQueue::kAxesLineCount );
		Harness.Check( TooSmall.TryLine( FVec3{}, FVec3{ 0.0f, 0.0f, 1.0f } ), "容量確認前の線を登録できる" );
		Harness.Check( !TooSmall.TryAxes( FVec3{}, FQuat::Identity(), 1.0f, 0.2f ), "15本分の空きがない座標軸を拒否する" );
		Harness.CheckEqualU64( TooSmall.Num(), 1u, "容量不足でも既存線と座標軸の原子性を保つ" );

		CDebugDraw3DQueue InvalidAxes;
		const f32 NotANumber = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !InvalidAxes.TryAxes( FVec3{ NotANumber, 0.0f, 0.0f } ), "有限でない原点を拒否する" );
		Harness.Check( !InvalidAxes.TryAxes( FVec3{}, FQuat{ 0.0f, 0.0f, 0.0f, 0.0f } ), "長さ0の回転を拒否する" );
		Harness.Check( !InvalidAxes.TryAxes( FVec3{}, FQuat{ NotANumber, 0.0f, 0.0f, 1.0f } ), "有限でない回転を拒否する" );
		Harness.Check( !InvalidAxes.TryAxes( FVec3{}, FQuat::Identity(), 0.0f, 0.2f ), "長さ0の軸を拒否する" );
		Harness.Check( !InvalidAxes.TryAxes( FVec3{}, FQuat::Identity(), 1.0f, 0.0f ), "長さ0の矢尻を拒否する" );
		Harness.Check( !InvalidAxes.TryAxes( FVec3{}, FQuat::Identity(), 1.0f, 1.01f ), "軸より長い矢尻を拒否する" );
		Harness.Check( !InvalidAxes.TryAxes( FVec3{}, FQuat::Identity(), NotANumber, 0.2f ), "有限でない軸長を拒否する" );
		/** 軸終点の計算を有限範囲外へ押し出す最大有限値。 */
		const f32 Maximum = std::numeric_limits<f32>::max();
		Harness.Check( !InvalidAxes.TryAxes( FVec3{ Maximum, 0.0f, 0.0f }, FQuat::Identity(), Maximum, 0.2f ), "終点計算で有限範囲を超える座標軸を拒否する" );
		Harness.CheckEqualU64( InvalidAxes.Num(), 0u, "不正な座標軸を途中まで登録しない" );
		Harness.CheckEqualU64( InvalidAxes.RejectedDrawCount(), 8u, "拒否した座標軸要求を1件ずつ数える" );
	}

	Harness.BeginSuite( "CDebugDraw3DQueue / 水平XZグリッドを原子的に扱う" );

	{
		const FVec3 Center{ 1.0f, 2.0f, 3.0f };
		const FVec4 Color{ 0.28f, 0.36f, 0.48f, 1.0f };
		CDebugDraw3DQueue Queue( 6u );
		Harness.Check( Queue.TryGrid( Center, 2.0f, 2u, Color ), "2分割の水平グリッドを登録できる" );
		Harness.CheckEqualU64( Queue.Num(), 6u, "X方向とZ方向へ各3本を登録する" );
		Harness.CheckEqualF32( Queue.Get( 0u ).Start.x, -1.0f, "最初のX線を負のX端から始める" );
		Harness.CheckEqualF32( Queue.Get( 0u ).End.x, 3.0f, "最初のX線を正のX端まで伸ばす" );
		Harness.CheckEqualF32( Queue.Get( 0u ).Start.z, 1.0f, "最初のX線を負のZ端へ置く" );
		Harness.CheckEqualF32( Queue.Get( 1u ).End.z, 5.0f, "最初のZ線を正のZ端まで伸ばす" );
		Harness.CheckEqualF32( Queue.Get( 2u ).Start.z, 3.0f, "中央のX線を中心Zへ置く" );
		Harness.CheckEqualF32( Queue.Get( 3u ).Start.x, 1.0f, "中央のZ線を中心Xへ置く" );
		Harness.CheckEqualF32( Queue.Get( 4u ).Start.z, 5.0f, "最後のX線を正のZ端へ厳密に置く" );
		Harness.CheckEqualF32( Queue.Get( 0u ).Color.y, Color.y, "指定色を全グリッド線へ使う" );

		CDebugDraw3DQueue MaximumQueue( CDebugDraw3DQueue::kMaximumGridLineCount );
		Harness.Check( MaximumQueue.TryGrid( FVec3{}, 64.0f, CDebugDraw3DQueue::kMaximumGridDivisions, Color ), "最大分割数を受け付ける" );
		Harness.CheckEqualU64( MaximumQueue.Num(), CDebugDraw3DQueue::kMaximumGridLineCount, "最大分割でも固定上限内の線数になる" );

		CDebugDraw3DQueue TooSmall( 6u );
		Harness.Check( TooSmall.TryLine( FVec3{}, FVec3{ 0.0f, 1.0f, 0.0f } ), "容量確認前の線を登録できる" );
		Harness.Check( !TooSmall.TryGrid( Center, 2.0f, 2u, Color ), "全線分の空きがないグリッドを拒否する" );
		Harness.CheckEqualU64( TooSmall.Num(), 1u, "容量不足でも既存線とグリッドの原子性を保つ" );

		CDebugDraw3DQueue InvalidGrid;
		const f32 NotANumber = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !InvalidGrid.TryGrid( FVec3{ NotANumber, 0.0f, 0.0f } ), "有限でない中心を拒否する" );
		Harness.Check( !InvalidGrid.TryGrid( FVec3{}, 0.0f ), "片側距離0のグリッドを拒否する" );
		Harness.Check( !InvalidGrid.TryGrid( FVec3{}, -1.0f ), "負の片側距離を拒否する" );
		Harness.Check( !InvalidGrid.TryGrid( FVec3{}, NotANumber ), "有限でない片側距離を拒否する" );
		Harness.Check( !InvalidGrid.TryGrid( FVec3{}, 1.0f, 0u ), "分割数0を拒否する" );
		Harness.Check( !InvalidGrid.TryGrid( FVec3{}, 1.0f, CDebugDraw3DQueue::kMaximumGridDivisions + 1u ), "最大を超える分割数を拒否する" );
		Harness.Check( !InvalidGrid.TryGrid( FVec3{}, 1.0f, 1u, FVec4{ 1.0f, 1.0f, 1.0f, 0.0f } ), "透明なグリッド色を拒否する" );
		Harness.Check( !InvalidGrid.TryGrid( FVec3{}, std::numeric_limits<f32>::max() ), "直径計算で有限範囲を超えるグリッドを拒否する" );
		Harness.CheckEqualU64( InvalidGrid.Num(), 0u, "不正なグリッドを途中まで登録しない" );
		Harness.CheckEqualU64( InvalidGrid.RejectedDrawCount(), 8u, "拒否したグリッド要求を1件ずつ数える" );
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
