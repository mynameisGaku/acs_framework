// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabel3DLayer.h"
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabelProjector3D.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 原点を正面中央へ映す検証用カメラを作る。 */
	CCamera MakeCamera() noexcept
	{
		CCamera Camera;
		Camera.SetPerspective( 60.0f * kDeg2Rad, 2.0f, 0.1f, 100.0f );
		Camera.SetLookAt( FVec3{ 0.0f, 0.0f, -5.0f }, FVec3{} );
		return Camera;
	}

	/** 浮動小数の小さな誤差を許して比較する。 */
	void CheckNear( CTestHarness& Harness, f32 Actual, f32 Expected, const char* Label ) noexcept
	{
		constexpr f32 kTolerance = 0.001f;
		const f32 Difference = Actual > Expected ? Actual - Expected : Expected - Actual;
		Harness.Check( Difference < kTolerance, Label );
	}
}


void RunWorldLabel3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FWorldLabel3DParams / 描画を壊す値を追加前に拒否する" );

	{
		FWorldLabel3DParams Params;
		Params.Text = FStringView( "PLAYER" );
		Harness.Check( Params.IsValid(), "文字だけ指定すれば読みやすい既定見た目になる" );

		Params.Text = FStringView{};
		Harness.Check( !Params.IsValid(), "空文字列を拒否する" );
		const char EmbeddedNull[]{ 'A', '\0', 'B' };
		Params.Text = FStringView( EmbeddedNull, 3u );
		Harness.Check( !Params.IsValid(), "途中のNULで表示が切れる文字列を拒否する" );
		Params.Text = FStringView( "PLAYER" );
		Params.WorldOffset.y = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Params.IsValid(), "有限でないworld位置のずれを拒否する" );
		Params.WorldOffset.y = 1.8f;
		Params.TextColor.x = 1.01f;
		Harness.Check( !Params.IsValid(), "色の範囲外を拒否する" );
		Params.TextColor.x = 1.0f;
		Params.MaximumDistance = 0.0f;
		Harness.Check( !Params.IsValid(), "表示距離0を拒否する" );
	}

	Harness.BeginSuite( "CWorldLabelProjector3D / 前方かつ画面内だけをpixelへ変換する" );

	{
		const CCamera Camera = MakeCamera();
		FVec2 Screen{ -1.0f, -1.0f };
		Harness.Check( CWorldLabelProjector3D::TryProject( Camera, FVec3{}, 800u, 400u, 100.0f, Screen ), "正面の点を射影できる" );
		CheckNear( Harness, Screen.x, 400.0f, "正面のXは画面中央" );
		CheckNear( Harness, Screen.y, 200.0f, "正面のYは画面中央" );

		const FVec2 BeforeFailure = Screen;
		Harness.Check( !CWorldLabelProjector3D::TryProject( Camera, FVec3{ 0.0f, 0.0f, -10.0f }, 800u, 400u, 100.0f, Screen ), "カメラ後方を隠す" );
		Harness.Check( Screen.x == BeforeFailure.x && Screen.y == BeforeFailure.y, "失敗時は出力を変えない" );
		Harness.Check( !CWorldLabelProjector3D::TryProject( Camera, FVec3{ 100.0f, 0.0f, 0.0f }, 800u, 400u, 1000.0f, Screen ), "画面外を隠す" );
		Harness.Check( !CWorldLabelProjector3D::TryProject( Camera, FVec3{}, 0u, 400u, 100.0f, Screen ), "幅0の画面を拒否する" );
		Harness.Check( !CWorldLabelProjector3D::TryProject( Camera, FVec3{}, 800u, 400u, 4.0f, Screen ), "最大距離を超えた点を隠す" );
	}

	Harness.BeginSuite( "CWorldLabel3DLayer / ノードへ安全に追従して操作できる" );

	{
		CSceneNodeGraph Graph;
		CWorldLabel3DLayer Labels;
		FWorldLabel3DParams Params;
		Params.Text = FStringView( "ENEMY" );
		Params.WorldOffset = FVec3{};
		Params.ScreenOffset = FVec2{};

		Harness.Check( !Labels.AddWorldLabel( FVec3{}, Params ).IsValid(), "場面接続前の追加を拒否する" );
		Labels.Bind( Graph );
		const FScene3DSpawnResult Spawned = Graph.TrySpawn( FStringView( "Enemy" ) );
		Harness.Check( Spawned.Succeeded(), "追従対象を場面へ置ける" );
		if ( !Spawned ) return;

		const FWorldLabel3DHandle NodeLabel = Labels.AddNodeLabel( *Spawned.Node, Params );
		Harness.Check( NodeLabel.IsValid(), "ノードラベルを1件追加できる" );
		Harness.CheckEqualU64( Labels.LabelCount(), 1u, "登録数へ反映する" );
		Harness.Check( Labels.Text( NodeLabel ) == FStringView( "ENEMY" ), "文字列をレイヤーが所有する" );

		const CCamera Camera = MakeCamera();
		FVec2 FirstScreen;
		Harness.Check( Labels.TryProjectLabel( NodeLabel, Camera, 800u, 400u, FirstScreen ), "現在ノード位置を射影できる" );
		CheckNear( Harness, FirstScreen.x, 400.0f, "初期位置は画面中央" );
		Spawned.Node->SetPosition( FVec3{ 1.0f, 0.0f, 0.0f } );
		FVec2 MovedScreen;
		Harness.Check( Labels.TryProjectLabel( NodeLabel, Camera, 800u, 400u, MovedScreen ) && MovedScreen.x > FirstScreen.x, "ノード移動へ自動追従する" );

		Harness.Check( Labels.SetText( NodeLabel, FStringView( "BOSS" ) ), "表示文字列を差し替えられる" );
		Harness.Check( Labels.Text( NodeLabel ) == FStringView( "BOSS" ), "差し替えた文字列を返す" );
		const char BrokenText[]{ 'X', '\0', 'Y' };
		Harness.Check( !Labels.SetText( NodeLabel, FStringView( BrokenText, 3u ) ) && Labels.Text( NodeLabel ) == FStringView( "BOSS" ), "不正な差し替えで以前の文字列を保つ" );
		Harness.Check( Labels.SetVisible( NodeLabel, false ), "明示的に隠せる" );
		Harness.Check( !Labels.TryProjectLabel( NodeLabel, Camera, 800u, 400u, MovedScreen ), "隠したラベルを射影候補にしない" );
		Labels.SetVisible( NodeLabel, true );
		Graph.Root().SetVisible( false );
		Harness.Check( !Labels.TryProjectLabel( NodeLabel, Camera, 800u, 400u, MovedScreen ), "見えない祖先の子ラベルも隠す" );
		Graph.Root().SetVisible( true );

		Harness.Check( Graph.Destroy( Spawned.Id ), "追従ノードを破棄予定にできる" );
		Harness.Check( !Labels.TryProjectLabel( NodeLabel, Camera, 800u, 400u, MovedScreen ), "破棄予定ノードを参照しない" );
		Graph.ResolveStructuralChanges();
		Harness.Check( !Labels.TryProjectLabel( NodeLabel, Camera, 800u, 400u, MovedScreen ), "破棄後の古い識別子を参照しない" );

		CSceneNodeGraph ReplacementContents;
		Graph.SwapContents( ReplacementContents );
		Harness.Check( !Labels.TryProjectLabel( NodeLabel, Camera, 800u, 400u, MovedScreen ), "scene内容の差し替え後に古いラベルを使わない" );
		Harness.CheckEqualU64( Labels.LabelCount(), 0u, "root差し替え時に古い登録を消す" );

		CSceneNodeGraph ReplacementGraph;
		Labels.Bind( ReplacementGraph );
		Harness.CheckEqualU64( Labels.LabelCount(), 0u, "別グラフへの接続時に古いラベルを消す" );
		Harness.Check( !Labels.Remove( NodeLabel ), "消えたhandleを再利用しない" );

		FWorldLabel3DParams FixedParams = Params;
		FixedParams.Text = FStringView( "GOAL" );
		const FWorldLabel3DHandle FixedLabel = Labels.AddWorldLabel( FVec3{}, FixedParams );
		Harness.Check( FixedLabel.IsValid(), "固定worldラベルを追加できる" );
		Harness.Check( Labels.SetWorldPosition( FixedLabel, FVec3{ -1.0f, 0.0f, 0.0f } ), "固定位置を変更できる" );
		FVec2 FixedScreen;
		Harness.Check( Labels.TryProjectLabel( FixedLabel, Camera, 800u, 400u, FixedScreen ) && FixedScreen.x < 400.0f, "変更した固定位置へ射影する" );
		Harness.Check( !Labels.SetWorldPosition( FixedLabel, FVec3{ std::numeric_limits<f32>::infinity(), 0.0f, 0.0f } ), "有限でない固定位置を拒否する" );
		Harness.Check( Labels.Remove( FixedLabel ) && Labels.LabelCount() == 0u, "固定ラベルを削除できる" );
		Labels.Unbind();
		Harness.Check( !Labels.IsBound(), "退場時にグラフ接続を外せる" );
	}
}
