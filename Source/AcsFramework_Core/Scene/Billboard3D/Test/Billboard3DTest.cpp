// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Billboard3D/Billboard3DLayer.h"
#include "AcsFramework_Core/Scene/Billboard3D/Billboard3DMath.h"
#include "AcsFramework_Core/Assets/Image/ImageLibrary.h"
#include "AcsFramework_Core/Scene/Sprite3D/Sprite3DSpawnParams.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** ベクトル各成分が期待値へ十分近いことを確かめる。 */
	void CheckNear( CTestHarness& Harness, FVec3 Actual, FVec3 Expected, const char* Label ) noexcept
	{
		const bool bNear = LengthSq( Actual - Expected ) <= 1.0e-6f;
		Harness.Check( bNear, Label );
	}

	/** 1画素の有効なRGBA画像を作る。 */
	TSharedPtr<AAsset> MakeImage() noexcept
	{
		TArray<byte> Pixels;
		Pixels.SetNum( 4u );
		if ( Pixels.Num() != 4u ) return TSharedPtr<AAsset>();
		Pixels[0] = 255u;
		Pixels[1] = 255u;
		Pixels[2] = 255u;
		Pixels[3] = 255u;
		return MakeShared<AImageAsset>( 1u, 1u, EPixelFormat::R8G8B8A8, Move( Pixels ) );
	}

	/** グラフへ3D画像板ノードを1つ作る。 */
	ANode* SpawnSpriteNode( CSceneNodeGraph& Graph, ANode* Parent = nullptr ) noexcept
	{
		const FScene3DSpawnResult Spawned = Graph.TrySpawn( FStringView( "Billboard" ), Parent );
		if ( !Spawned ) return nullptr;
		Spawned.Node->AddComponent<ASprite3DComponent>();
		return Spawned.Node;
	}
}


void RunBillboard3DTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "TryCalculateBillboard3DRotation / カメラへ正面を向ける" );

	{
		FQuat Rotation;
		Harness.Check( TryCalculateBillboard3DRotation( FVec3{}, FVec3{ 0.0f, 0.0f, 5.0f }, FQuat::Identity(), EBillboard3DMode::FaceCamera, 0.0f, Rotation ),
			"正面のカメラへ向けられる" );
		CheckNear( Harness, Normalize( Rotate( Rotation, FVec3::Forward() ) ), FVec3::Forward(),
			"画像板の正面がカメラ方向と一致する" );
		CheckNear( Harness, Normalize( Rotate( Rotation, FVec3::Up() ) ), FVec3::Up(),
			"roll 0ではworld上方向を保つ" );

		const FVec3 CameraPosition{ 4.0f, 2.0f, -3.0f };
		Harness.Check( TryCalculateBillboard3DRotation( FVec3{}, CameraPosition, FQuat::Identity(), EBillboard3DMode::FaceCamera, 0.0f, Rotation ),
			"斜め上のカメラへ向けられる" );
		CheckNear( Harness, Normalize( Rotate( Rotation, FVec3::Forward() ) ), Normalize( CameraPosition ),
			"上下を含むカメラ方向へ正面が一致する" );
	}

	Harness.BeginSuite( "TryCalculateBillboard3DRotation / Y軸固定と親回転を扱う" );

	{
		FQuat Rotation;
		const FVec3 CameraPosition{ 5.0f, 8.0f, 5.0f };
		Harness.Check( TryCalculateBillboard3DRotation( FVec3{}, CameraPosition, FQuat::Identity(), EBillboard3DMode::FaceCameraYAxis, 0.0f, Rotation ),
			"Y軸固定で斜めのカメラへ向けられる" );
		CheckNear( Harness, Normalize( Rotate( Rotation, FVec3::Forward() ) ), Normalize( FVec3{ 5.0f, 0.0f, 5.0f } ),
			"Y軸固定では水平成分だけへ正面を向ける" );
		CheckNear( Harness, Normalize( Rotate( Rotation, FVec3::Up() ) ), FVec3::Up(),
			"Y軸固定では画像板の上をworld上方向へ保つ" );

		const FQuat ParentRotation = FQuat::AxisAngle( FVec3::Up(), 0.73f );
		Harness.Check( TryCalculateBillboard3DRotation( FVec3{ 2.0f, 1.0f, -1.0f }, FVec3{ -4.0f, 3.0f, 6.0f }, ParentRotation, EBillboard3DMode::FaceCamera, 0.0f, Rotation ),
			"回転した親の内側でもローカル回転を作れる" );
		FTransform3D ParentTransform;
		ParentTransform.rotation = ParentRotation;
		FTransform3D ChildTransform;
		ChildTransform.rotation = Rotation;
		const FQuat WorldRotation = ParentTransform.Compose( ChildTransform ).rotation;
		CheckNear( Harness, Normalize( Rotate( WorldRotation, FVec3::Forward() ) ), Normalize( FVec3{ -6.0f, 2.0f, 7.0f } ),
			"親回転を合成したworld正面がカメラへ一致する" );
	}

	Harness.BeginSuite( "TryCalculateBillboard3DRotation / rollと不正値を分離する" );

	{
		FQuat WithoutRoll;
		FQuat WithRoll;
		Harness.Check( TryCalculateBillboard3DRotation( FVec3{}, FVec3{ 1.0f, 2.0f, 4.0f }, FQuat::Identity(), EBillboard3DMode::FaceCamera, 0.0f, WithoutRoll ),
			"rollなしの向きを作れる" );
		Harness.Check( TryCalculateBillboard3DRotation( FVec3{}, FVec3{ 1.0f, 2.0f, 4.0f }, FQuat::Identity(), EBillboard3DMode::FaceCamera, 45.0f, WithRoll ),
			"roll付きの向きを作れる" );
		CheckNear( Harness, Normalize( Rotate( WithRoll, FVec3::Forward() ) ), Normalize( Rotate( WithoutRoll, FVec3::Forward() ) ),
			"rollを加えても正面方向は変わらない" );
		Harness.Check( Dot( Normalize( Rotate( WithRoll, FVec3::Up() ) ), Normalize( Rotate( WithoutRoll, FVec3::Up() ) ) ) < 0.8f,
			"rollは画像板内の上方向を回す" );

		FQuat Vertical;
		Harness.Check( TryCalculateBillboard3DRotation( FVec3{}, FVec3{ 0.0f, 5.0f, 0.0f }, FQuat::Identity(), EBillboard3DMode::FaceCamera, 0.0f, Vertical ),
			"真上でも全軸追従なら代替上方向から向きを決める" );
		Harness.Check( !TryCalculateBillboard3DRotation( FVec3{}, FVec3{ 0.0f, 5.0f, 0.0f }, FQuat::Identity(), EBillboard3DMode::FaceCameraYAxis, 0.0f, Vertical ),
			"水平差0のY軸固定は向きが決まらないため拒む" );

		const FQuat Sentinel{ 0.1f, 0.2f, 0.3f, 0.4f };
		FQuat Output = Sentinel;
		Harness.Check( !TryCalculateBillboard3DRotation( FVec3{}, FVec3{}, FQuat::Identity(), EBillboard3DMode::FaceCamera, 0.0f, Output ),
			"画像板とカメラが同位置なら拒む" );
		Harness.CheckEqualF32( Output.x, Sentinel.x, "失敗時は出力を変更しない" );
		Harness.Check( !TryCalculateBillboard3DRotation( FVec3{}, FVec3::Forward(), FQuat::Identity(), static_cast<EBillboard3DMode>( 255u ), 0.0f, Output ),
			"未知の向き指定を拒む" );
		Harness.Check( !TryCalculateBillboard3DRotation( FVec3{}, FVec3::Forward(), FQuat::Identity(), EBillboard3DMode::FaceCamera, std::numeric_limits<f32>::quiet_NaN(), Output ),
			"非数rollを拒む" );
	}

	Harness.BeginSuite( "CBillboard3DLayer / 世代付きノードを追従する" );

	{
		CSceneNodeGraph Graph;
		CSceneNodeGraph OtherGraph;
		CBillboard3DLayer Layer;
		ANode* const Node = SpawnSpriteNode( Graph );
		ANode* const OtherNode = SpawnSpriteNode( OtherGraph );
		Harness.Check( Node != nullptr && !Layer.Track( *Node ), "未接続では追従へ加えない" );

		Layer.Bind( Graph );
		Harness.Check( Layer.IsBoundTo( Graph ), "指定グラフへ接続する" );
		Harness.Check( Node != nullptr && Layer.Track( *Node ), "接続中グラフの画像板を追従へ加える" );
		Harness.CheckEqualU64( Layer.TrackedCount(), 1u, "追従数を数える" );
		Harness.Check( Node != nullptr && Layer.Track( *Node, EBillboard3DMode::FaceCameraYAxis, 12.0f ),
			"同じ画像板は重複せず設定を更新する" );
		Harness.CheckEqualU64( Layer.TrackedCount(), 1u, "重複登録で件数を増やさない" );
		Harness.Check( OtherNode != nullptr && !Layer.Track( *OtherNode ), "別グラフの画像板を拒む" );

		const FScene3DSpawnResult Plain = Graph.TrySpawn( FStringView( "Plain" ) );
		Harness.Check( Plain && !Layer.Track( *Plain.Node ), "3D画像部品の無いノードを拒む" );

		CCamera Camera;
		Camera.SetLookAt( FVec3{ 4.0f, 3.0f, 5.0f }, FVec3{} );
		Harness.CheckEqualU64( Layer.UpdateFacing( Camera ), 1u, "有効な画像板1件の向きを更新する" );
		if ( Node != nullptr )
		{
			CheckNear( Harness, Normalize( Rotate( Node->World().rotation, FVec3::Forward() ) ), Normalize( FVec3{ 4.0f, 0.0f, 5.0f } ),
				"登録更新したY軸固定設定を描画前に反映する" );
			Harness.Check( Layer.SetFacing( *Node, EBillboard3DMode::FaceCamera, -20.0f ), "追従中の向き設定を変更できる" );
			Harness.Check( Layer.Remove( *Node ), "追従だけを外せる" );
			Harness.Check( Graph.Get( Node->Id() ) == Node, "追従を外しても画像板ノードは残す" );
			Harness.Check( Layer.Track( *Node ), "場面差し替え検証の追従を登録し直す" );
		}
		Harness.CheckEqualU64( Layer.TrackedCount(), 1u, "再登録後の追従数は1" );
		Graph.SwapContents( OtherGraph );
		Harness.CheckEqualU64( Layer.UpdateFacing( Camera ), 0u, "root差し替え直後は古い識別子を更新しない" );
		Harness.CheckEqualU64( Layer.TrackedCount(), 0u, "同じ識別子が再利用されても別ノードへ誤追従しない" );
	}

	Harness.BeginSuite( "CBillboard3DLayer / 生成とノード破棄を一体で扱う" );

	{
		CSceneNodeGraph Graph;
		CBillboard3DLayer Layer;
		Layer.Bind( Graph );
		CImageLibrary UnboundLibrary;
		FSprite3DSpawnParams Params = FSprite3DSpawnParams::FromImage( FStringView( "circle.png" ), FVec3{ 1.0f, 2.0f, 3.0f }, FVec2{ 0.7f, 0.5f } );
		Params.ImageAsset = MakeImage();

		ANode* const Spawned = Layer.Spawn( Params, UnboundLibrary, EBillboard3DMode::FaceCamera, 15.0f );
		Harness.Check( Spawned != nullptr, "読込済み画像からビルボードを1回で作る" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 2u, "rootと画像板だけを登録する" );
		Harness.CheckEqualU64( Layer.TrackedCount(), 1u, "生成と同時に追従へ加える" );

		if ( Spawned != nullptr ) (void)Graph.Destroy( Spawned->Id() );
		Graph.ResolveStructuralChanges();
		CCamera Camera;
		Camera.SetLookAt( FVec3{ 0.0f, 2.0f, -5.0f }, FVec3{} );
		Harness.CheckEqualU64( Layer.UpdateFacing( Camera ), 0u, "破棄済み画像板は更新しない" );
		Harness.CheckEqualU64( Layer.TrackedCount(), 0u, "破棄済み識別子を追従から自動で外す" );

		FSprite3DSpawnParams Broken = Params;
		Broken.Size.x = 0.0f;
		Harness.Check( Layer.Spawn( Broken, UnboundLibrary ) == nullptr, "不正な画像指定を拒む" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "失敗時は半端なノードを残さない" );
		Layer.Unbind();
		Harness.Check( !Layer.IsBound() && Layer.TrackedCount() == 0u, "解除時に接続と追従を消す" );
	}
}
