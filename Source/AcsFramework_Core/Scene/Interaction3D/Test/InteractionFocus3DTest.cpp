// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3D.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DInput.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DTransition.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractableModel3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 原点から+Zを見る、中央レイ検証用カメラを作る。 */
	CCamera MakeCamera() noexcept
	{
		CCamera Camera;
		Camera.SetPerspective( 60.0f * kDeg2Rad, 1.0f, 0.1f, 100.0f );
		Camera.SetLookAt( FVec3{}, FVec3{ 0.0f, 0.0f, 1.0f } );
		return Camera;
	}

	/** 指定親のローカル位置へ、実形状判定できる立方体を置く。 */
	ANode* PlaceCube( CSceneNodeGraph& Graph, ANode& Parent, FVec3 Position ) noexcept
	{
		FModel3DSpawnParams Params = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, Position );
		return CModel3DSpawner::SpawnInto( Graph, Params, &Parent );
	}
}


void RunInteractionFocus3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "AdvanceInteractionFocus3D / 候補と状態だけから出入りと決定を返す" );

	{
		const FNodeId First( 1u, 1u );
		const FNodeId Second( 2u, 1u );
		FInteractionFocus3DState State;
		FInteractionFocus3DInput Input;
		Input.CandidateNode = First;

		FInteractionFocus3DUpdateResult Result = AdvanceInteractionFocus3D( State, Input );
		Harness.Check( Result.FocusEntered() && !Result.FocusLeft() && !Result.Activated(), "対象なしから入る" );
		State.FocusedNode = Result.FocusedNode;

		Input.bActivateRequested = true;
		Result = AdvanceInteractionFocus3D( State, Input );
		Harness.Check( !Result.FocusChanged() && Result.ActivatedNode == First, "同じ対象への決定だけを返す" );
		State.FocusedNode = Result.FocusedNode;

		Input.CandidateNode = Second;
		Input.bActivateRequested = false;
		Result = AdvanceInteractionFocus3D( State, Input );
		Harness.Check( Result.FocusEntered() && Result.FocusLeft() && Result.FocusedNode == Second, "対象切替は退出と進入の両方になる" );
		State.FocusedNode = Result.FocusedNode;

		Input.CandidateNode = FNodeId{};
		Input.bActivateRequested = true;
		Result = AdvanceInteractionFocus3D( State, Input );
		Harness.Check( Result.FocusLeft() && !Result.FocusEntered() && !Result.Activated(), "対象なしでは決定を成立させない" );
	}

	Harness.BeginSuite( "FInteractionFocus3DParams / 画面位置と距離を有限範囲へ制限する" );

	{
		FInteractionFocus3DParams Params;
		Harness.Check( Params.IsValid(), "中央4world距離を既定にできる" );
		Params.ScreenPosition.x = -0.01f;
		Harness.Check( !Params.IsValid(), "画面左外を拒否する" );
		Params.ScreenPosition.x = 0.5f;
		Params.MaximumDistance = std::numeric_limits<f32>::infinity();
		Harness.Check( !Params.IsValid(), "有限でない距離を拒否する" );
	}

	Harness.BeginSuite( "CInteractionFocus3D / 実形状、祖先、遮蔽、案内表示を接続する" );

	{
		CSceneNodeGraph Graph;
		CWorldLabel3DLayer Labels;
		Labels.Bind( Graph );

		CSceneNodeGraph OtherGraph;
		CWorldLabel3DLayer OtherLabels;
		OtherLabels.Bind( OtherGraph );
		CInteractionFocus3D Mismatched;
		Harness.Check( !Mismatched.Bind( Graph, OtherLabels ), "別グラフのラベルレイヤーを拒否する" );

		const FScene3DSpawnResult Target = Graph.TrySpawn( FStringView( "Target" ) );
		Harness.Check( Target.Succeeded() && Target.Node != nullptr, "登録する親ノードを置ける" );
		if ( !Target ) return;
		Target.Node->SetPosition( FVec3{ 0.0f, 0.0f, 4.0f } );
		ANode* const ChildShape = PlaceCube( Graph, *Target.Node, FVec3{} );
		Harness.Check( ChildShape != nullptr, "親の下へ命中形状を置ける" );
		if ( ChildShape == nullptr ) return;

		CInteractionFocus3D Focus;
		FInteractionFocus3DParams FocusParams;
		FocusParams.MaximumDistance = 8.0f;
		Harness.Check( Focus.Bind( Graph, Labels, FocusParams ), "同じ場面の判定とラベルへ接続できる" );

		FWorldLabel3DParams Prompt;
		Prompt.Text = FStringView( "ENTER: USE" );
		Prompt.WorldOffset = FVec3{};
		Prompt.MaximumDistance = 8.0f;
		Harness.Check( Focus.RegisterTarget( *Target.Node, Prompt ), "親を操作対象として登録できる" );
		Harness.Check( !Focus.RegisterTarget( *Target.Node, Prompt ), "同じ世代付き対象の重複を拒否する" );

		const CCamera Camera = MakeCamera();
		FInteractionFocus3DUpdateResult Result = Focus.Update( Camera );
		Harness.Check( Result.FocusEntered() && Result.FocusedNode == Target.Id, "子形状への命中を登録親へまとめる" );
		Harness.Check( Focus.FocusedNode() == Target.Node && Labels.LabelCount() == 1u, "現在対象を解決して案内を1件だけ表示する" );

		Result = Focus.Update( Camera, true );
		Harness.Check( !Result.FocusChanged() && Result.ActivatedNode == Target.Id, "同じ対象への決定を1回返す" );
		Harness.CheckEqualU64( Labels.LabelCount(), 1u, "同じ対象では案内を作り直さない" );

		Labels.Clear();
		Focus.Update( Camera );
		Harness.CheckEqualU64( Labels.LabelCount(), 1u, "共有ラベル全消去後は現在案内だけを復元する" );

		FModel3DSpawnParams OccluderParams = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{ 0.0f, 0.0f, 2.0f } );
		ANode* const Occluder = CModel3DSpawner::SpawnInto( Graph, OccluderParams );
		Harness.Check( Occluder != nullptr, "未登録の遮蔽物を置ける" );
		if ( Occluder == nullptr ) return;
		Result = Focus.Update( Camera );
		Harness.Check( Result.FocusLeft() && !Result.FocusedNode.IsValid(), "未登録の最前面形状が奥の対象を遮る" );
		Harness.CheckEqualU64( Labels.LabelCount(), 0u, "遮られた対象の案内を消す" );

		Occluder->SetVisible( false );
		Result = Focus.Update( Camera );
		Harness.Check( Result.FocusEntered() && Result.FocusedNode == Target.Id, "見えない遮蔽物を飛ばして再進入する" );

		CSceneNodeGraph Replacement;
		Graph.SwapContents( Replacement );
		Harness.Check( Focus.FocusedNode() == nullptr, "scene内容差し替え直後も旧IDを新rootへ解決しない" );
		Result = Focus.Update( Camera );
		Harness.Check( Result.FocusLeft() && Focus.TargetCount() == 0u && Focus.FocusedNode() == nullptr, "scene内容差し替えで古い対象を消す" );
		Harness.CheckEqualU64( Labels.LabelCount(), 0u, "scene内容差し替えで古い案内を消す" );

		Focus.Unbind();
		Harness.Check( !Focus.IsBound(), "退場時に全接続を外す" );
	}

	Harness.BeginSuite( "CInteractableModel3DSpawner / 生成と操作対象登録を一括化する" );

	{
		CSceneNodeGraph Graph;
		CWorldLabel3DLayer Labels;
		Labels.Bind( Graph );

		CInteractionFocus3D Focus;
		FInteractionFocus3DParams FocusParams;
		FocusParams.MaximumDistance = 8.0f;
		Harness.Check( Focus.Bind( Graph, Labels, FocusParams ),
			"一括生成先の視線フォーカスを場面へ接続できる" );

		const FModel3DSpawnParams Params = FModel3DSpawnParams::FromPrimitive(
			EMeshPrimitive3D::Cube, FVec3{ 0.0f, 0.0f, 4.0f } );
		ANode* const Target = CInteractableModel3DSpawner::SpawnInto(
			Graph, Focus, Params, FStringView( "ENTER: USE" ), FVec3{} );
		Harness.Check( Target != nullptr, "立方体の生成と操作対象登録を1回で完了する" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 2u,
			"rootと生成モデルだけを場面へ登録する" );
		Harness.CheckEqualU64( Focus.TargetCount(), 1u,
			"生成モデルを操作対象として1件登録する" );

		if ( Target != nullptr )
		{
			const FInteractionFocus3DUpdateResult Result = Focus.Update( MakeCamera(), true );
			Harness.Check( Result.FocusEntered() && Result.FocusedNode == Target->Id(),
				"一括生成した実形状を視線で捉える" );
			Harness.Check( Result.ActivatedNode == Target->Id(),
				"一括生成した対象への決定を返す" );
			Harness.CheckEqualU64( Labels.LabelCount(), 1u,
				"一括登録した操作案内をフォーカス中だけ表示する" );
		}
	}

	{
		CSceneNodeGraph Graph;
		CInteractionFocus3D UnboundFocus;
		const FModel3DSpawnParams Params = FModel3DSpawnParams::FromPrimitive(
			EMeshPrimitive3D::Cube, FVec3{ 0.0f, 0.0f, 4.0f } );

		ANode* const Failed = CInteractableModel3DSpawner::SpawnInto(
			Graph, UnboundFocus, Params, FStringView( "ENTER: USE" ) );
		Harness.Check( Failed == nullptr, "未接続の視線フォーカスでは一括生成を失敗にする" );
		Harness.CheckEqualU64( UnboundFocus.TargetCount(), 0u,
			"登録失敗時は操作対象を残さない" );

		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u,
			"登録できなかった生成モデルを場面へ残さない" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u,
			"巻き戻した生成モデルの識別子も解放する" );
	}
}
