// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Bridge3D/Bridge3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 橋の全パーツが指定親へ繋がっているか返す。 */
	bool AllPartsUseParent_Internal( const FBridge3DSpawnResult& Bridge,
		const ANode& Parent ) noexcept
	{
		if ( Bridge.Deck.Node == nullptr || Bridge.Deck.Node->Parent() != &Parent ) return false;
		const FFence3DSpawnResult* const Railings[]
		{
			&Bridge.NegativeSideRailing,
			&Bridge.PositiveSideRailing,
		};
		for ( const FFence3DSpawnResult* const Railing : Railings )
		{
			for ( usize Index = 0u; Index < Railing->Posts.Num(); ++Index )
			{
				if ( Railing->Posts[Index].Node == nullptr
					|| Railing->Posts[Index].Node->Parent() != &Parent ) return false;
			}
			for ( usize Index = 0u; Index < Railing->Rails.Num(); ++Index )
			{
				if ( Railing->Rails[Index].Node == nullptr
					|| Railing->Rails[Index].Node->Parent() != &Parent ) return false;
			}
		}
		return true;
	}

	/** 橋の全パーツが破棄予定ではないか返す。 */
	bool NoPartIsPendingDestroy_Internal( const FBridge3DSpawnResult& Bridge ) noexcept
	{
		if ( Bridge.Deck.Node == nullptr || Bridge.Deck.Node->IsPendingDestroy() ) return false;
		const FFence3DSpawnResult* const Railings[]
		{
			&Bridge.NegativeSideRailing,
			&Bridge.PositiveSideRailing,
		};
		for ( const FFence3DSpawnResult* const Railing : Railings )
		{
			for ( usize Index = 0u; Index < Railing->Posts.Num(); ++Index )
			{
				if ( Railing->Posts[Index].Node == nullptr
					|| Railing->Posts[Index].Node->IsPendingDestroy() ) return false;
			}
			for ( usize Index = 0u; Index < Railing->Rails.Num(); ++Index )
			{
				if ( Railing->Rails[Index].Node == nullptr
					|| Railing->Rails[Index].Node->IsPendingDestroy() ) return false;
			}
		}
		return true;
	}

	/** 指定方向から床板中心・寸法と両側柵の始点を調べる。 */
	void CheckDirection_Internal( CTestHarness& Harness,
		EBridge3DDirection Direction, f32 ExpectedDeckX, f32 ExpectedDeckZ,
		f32 ExpectedDeckSizeX, f32 ExpectedDeckSizeZ,
		f32 ExpectedNegativeX, f32 ExpectedNegativeZ,
		f32 ExpectedPositiveX, f32 ExpectedPositiveZ )
	{
		FBridge3DSpawnParams Params = FBridge3DSpawnParams::FromDimensions(
			2.0f, 4.0f, 1.2f, FVec3{}, Direction );
		Params.PostThickness = 0.2f;
		FGround3DSpawnParams Deck;
		FFence3DSpawnParams NegativeRailing;
		FFence3DSpawnParams PositiveRailing;
		Harness.Check( Params.TryBuildParts(
			Deck, NegativeRailing, PositiveRailing ), "指定方向の橋設定を作れる" );
		Harness.CheckNearF32( Deck.Position.x, ExpectedDeckX, 0.001f,
			"床板中心Xを出口側へ進める" );
		Harness.CheckNearF32( Deck.Position.z, ExpectedDeckZ, 0.001f,
			"床板中心Zを出口側へ進める" );
		Harness.CheckNearF32( Deck.Size.x, ExpectedDeckSizeX, 0.001f,
			"床板のX寸法を方向へ合わせる" );
		Harness.CheckNearF32( Deck.Size.y, ExpectedDeckSizeZ, 0.001f,
			"床板のZ寸法を方向へ合わせる" );
		Harness.CheckNearF32( NegativeRailing.StartPostBottomCenter.x,
			ExpectedNegativeX, 0.001f, "負側柵の始点Xを床板内へ置く" );
		Harness.CheckNearF32( NegativeRailing.StartPostBottomCenter.z,
			ExpectedNegativeZ, 0.001f, "負側柵の始点Zを床板内へ置く" );
		Harness.CheckNearF32( PositiveRailing.StartPostBottomCenter.x,
			ExpectedPositiveX, 0.001f, "正側柵の始点Xを床板内へ置く" );
		Harness.CheckNearF32( PositiveRailing.StartPostBottomCenter.z,
			ExpectedPositiveZ, 0.001f, "正側柵の始点Zを床板内へ置く" );
		Harness.CheckNearF32( NegativeRailing.Length, 3.8f, 0.001f,
			"始終端の支柱を床板からはみ出させない" );
	}
}


void RunBridge3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FBridge3DSpawnParams / 既定値だけで歩ける橋になる" );

	{
		const FBridge3DSpawnParams Bridge;
		Harness.Check( Bridge.IsValid(), "既定値をそのまま使える" );
		Harness.CheckEqualF32( Bridge.Width, 3.0f, "既定の床板幅" );
		Harness.CheckEqualF32( Bridge.Length, 8.0f, "既定の床板長" );
		Harness.CheckEqualF32( Bridge.RailingHeight, 1.15f, "既定の柵高" );
		Harness.Check( Bridge.Direction == EBridge3DDirection::PositiveZ,
			"既定でZ正方向へ伸びる" );
		Harness.Check( Bridge.CollisionLayer != 0u,
			"既定で衝突問い合わせへ現れる" );

		const FBridge3DSpawnParams Sized = FBridge3DSpawnParams::FromDimensions(
			4.0f, 12.0f, 1.4f, FVec3{ 1.0f, 2.0f, 3.0f },
			EBridge3DDirection::NegativeX );
		Harness.Check( Sized.IsValid(), "幅、長さ、柵高、方向だけで設定を作れる" );
		Harness.CheckEqualF32( Sized.Width, 4.0f, "指定した床板幅" );
		Harness.CheckEqualF32( Sized.Length, 12.0f, "指定した床板長" );
		Harness.CheckEqualF32( Sized.EntranceCenter.y, 2.0f,
			"指定した床板上高さ" );
		Harness.Check( Sized.Direction == EBridge3DDirection::NegativeX,
			"指定した方向を保つ" );
		Harness.Check( FBridge3DSpawnResult{}.IsEmpty(), "既定の生成結果は空" );
	}

	Harness.BeginSuite( "FBridge3DSpawnParams / 半端な橋を作る値を配置前に弾く" );

	{
		FBridge3DSpawnParams Broken;
		Broken.Width = 0.0f;
		Harness.Check( !Broken.IsValid(), "床板幅0を拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.Length = std::numeric_limits<f32>::infinity();
		Harness.Check( !Broken.IsValid(), "有限でない長さを拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.DeckThickness = 0.0f;
		Harness.Check( !Broken.IsValid(), "床板厚0を拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.RailingHeight = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない柵高を拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.Width = Broken.PostThickness * 2.0f;
		Harness.Check( !Broken.IsValid(), "支柱だけで内幅が無くなる床板を拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.Length = Broken.PostThickness;
		Harness.Check( !Broken.IsValid(), "始終端支柱が重なる長さを拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.Direction = static_cast<EBridge3DDirection>( 0xffu );
		Harness.Check( !Broken.IsValid(), "未知の方向を拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.RailCount = 0u;
		Harness.Check( !Broken.IsValid(), "横桟0本を拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.DeckColor.x = 1.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の床板色を拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.RailingColor.w = -0.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の柵色を拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.RailingMetallic = -0.01f;
		Harness.Check( !Broken.IsValid(), "負の柵金属度を拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.CollisionLayer = 0u;
		Harness.Check( !Broken.IsValid(), "問い合わせ不能なレイヤー0を拒否する" );

		Broken = FBridge3DSpawnParams{};
		Broken.Length = std::numeric_limits<f32>::max();
		Broken.EntranceCenter.z = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な出口位置にならない値を拒否する" );

		FGround3DSpawnParams Deck;
		Deck.Position = FVec3{ 7.0f, 8.0f, 9.0f };
		FFence3DSpawnParams NegativeRailing;
		NegativeRailing.Length = 17.0f;
		FFence3DSpawnParams PositiveRailing;
		PositiveRailing.Length = 19.0f;
		Harness.Check( !Broken.TryBuildParts(
			Deck, NegativeRailing, PositiveRailing ), "不正値の部分計算を拒否する" );
		Harness.CheckEqualF32( Deck.Position.x, 7.0f,
			"失敗時に床板出力を変えない" );
		Harness.CheckEqualF32( NegativeRailing.Length, 17.0f,
			"失敗時に負側柵出力を変えない" );
		Harness.CheckEqualF32( PositiveRailing.Length, 19.0f,
			"失敗時に正側柵出力を変えない" );
	}

	Harness.BeginSuite( "FBridge3DSpawnParams / XとZの正負4方向へ床板内の柵を作る" );

	CheckDirection_Internal( Harness, EBridge3DDirection::PositiveX,
		2.0f, 0.0f, 4.0f, 2.0f, 0.1f, -0.9f, 0.1f, 0.9f );
	CheckDirection_Internal( Harness, EBridge3DDirection::NegativeX,
		-2.0f, 0.0f, 4.0f, 2.0f, -0.1f, -0.9f, -0.1f, 0.9f );
	CheckDirection_Internal( Harness, EBridge3DDirection::PositiveZ,
		0.0f, 2.0f, 2.0f, 4.0f, -0.9f, 0.1f, 0.9f, 0.1f );
	CheckDirection_Internal( Harness, EBridge3DDirection::NegativeZ,
		0.0f, -2.0f, 2.0f, 4.0f, -0.9f, -0.1f, 0.9f, -0.1f );

	Harness.BeginSuite( "CBridge3DSpawner / 床板と両側柵を一括配置する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FBridge3DSpawnParams Params = FBridge3DSpawnParams::FromDimensions(
			4.0f, 6.0f, 1.2f, FVec3{ 1.0f, 2.0f, 3.0f } );
		Params.DeckThickness = 0.5f;
		Params.MaximumPostSpacing = 2.0f;
		Params.PostThickness = 0.2f;
		Params.RailCount = 2u;
		Params.DeckColor = FVec4{ 0.22f, 0.28f, 0.36f, 1.0f };
		Params.RailingColor = FVec4{ 0.12f, 0.16f, 0.20f, 1.0f };
		Params.DeckMetallic = 0.1f;
		Params.DeckRoughness = 0.7f;
		Params.RailingMetallic = 0.8f;
		Params.RailingRoughness = 0.3f;
		Params.bDeckCastsShadow = false;
		Params.bRailingsCastShadow = true;
		Params.CollisionLayer = 0x4u;
		Params.DeckName = FStringView( "SteelBridgeDeck" );
		Params.RailingPostName = FStringView( "SteelBridgePost" );
		Params.RailingRailName = FStringView( "SteelBridgeRail" );

		FBridge3DSpawnResult Bridge = CBridge3DSpawner::SpawnInto(
			Graph, Collision, Params );
		Harness.Check( Bridge.Succeeded(), "床板と両側柵を一括生成できる" );
		Harness.CheckEqualU64( Bridge.NegativeSideRailing.PostCount(), 4u,
			"負側へ最大間隔を守る支柱を置く" );
		Harness.CheckEqualU64( Bridge.PositiveSideRailing.PostCount(), 4u,
			"正側へ同数の支柱を置く" );
		Harness.CheckEqualU64( Bridge.PartCount(), 13u,
			"床板1個と両側の支柱・横桟を数える" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 13u,
			"表示ノードを必要数だけ置く" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 13u,
			"全パーツへ箱型衝突を登録する" );

		if ( Bridge.Succeeded() )
		{
			Harness.Check( Bridge.Deck.Node->Name() == FStringView( "SteelBridgeDeck" ),
				"床板へ指定名を付ける" );
			Harness.Check( Bridge.NegativeSideRailing.Posts[0u].Node->Name()
				== FStringView( "SteelBridgePost" ), "支柱へ指定名を付ける" );
			Harness.Check( Bridge.PositiveSideRailing.Rails[0u].Node->Name()
				== FStringView( "SteelBridgeRail" ), "横桟へ指定名を付ける" );
			Harness.CheckNearF32( Bridge.Deck.Node->Local().position.z, 6.0f,
				0.001f, "床板中心を入口から長さ半分先へ置く" );
			Harness.CheckNearF32( Bridge.Deck.Node->Local().scale.x, 4.0f,
				0.001f, "床板へ指定幅を使う" );
			Harness.CheckNearF32( Bridge.Deck.Node->Local().scale.z, 6.0f,
				0.001f, "床板へ指定長を使う" );
			Harness.CheckNearF32(
				Bridge.NegativeSideRailing.Posts[0u].Node->Local().position.x,
				-0.9f, 0.001f, "負側支柱を床板端の内側へ置く" );
			Harness.CheckNearF32(
				Bridge.PositiveSideRailing.Posts[0u].Node->Local().position.x,
				2.9f, 0.001f, "正側支柱を床板端の内側へ置く" );
			Harness.CheckNearF32(
				Bridge.NegativeSideRailing.Posts[0u].Node->Local().position.z,
				3.1f, 0.001f, "入口支柱を床板内へ半幅進める" );
			Harness.CheckNearF32(
				Bridge.NegativeSideRailing.Posts[3u].Node->Local().position.z,
				8.9f, 0.001f, "出口支柱を床板内へ半幅戻す" );
		}

		const AMeshComponent3D* const DeckMesh = Bridge.Succeeded()
			? Bridge.Deck.Node->GetComponent<AMeshComponent3D>() : nullptr;
		const AMeshComponent3D* const RailingMesh = Bridge.Succeeded()
			? Bridge.NegativeSideRailing.Posts[0u].Node
				->GetComponent<AMeshComponent3D>() : nullptr;
		Harness.Check( DeckMesh != nullptr
			&& DeckMesh->Primitive() == EMeshPrimitive3D::Plane,
			"床板は平面表示を使う" );
		Harness.Check( RailingMesh != nullptr
			&& RailingMesh->Primitive() == EMeshPrimitive3D::Cube,
			"柵は立方体表示を使う" );
		if ( DeckMesh != nullptr && RailingMesh != nullptr )
		{
			Harness.CheckEqualF32( DeckMesh->Color().z, 0.36f,
				"指定した床板色を使う" );
			Harness.CheckEqualF32( DeckMesh->Material().pbr.roughness, 0.7f,
				"指定した床板粗さを使う" );
			Harness.Check( !DeckMesh->CastsShadow(), "指定した床板の影設定を使う" );
			Harness.CheckEqualF32( RailingMesh->Material().pbr.metallic, 0.8f,
				"指定した柵金属度を使う" );
			Harness.Check( RailingMesh->CastsShadow(), "指定した柵の影設定を使う" );
		}

		FWorldCollisionShape3D DeckShape;
		FWorldCollisionShape3D PostShape;
		Harness.Check( Bridge.Succeeded()
			&& Collision.TryGetWorldShape( Bridge.Deck.Shape, DeckShape ),
			"床板のworld衝突を読める" );
		Harness.Check( Bridge.Succeeded() && Collision.TryGetWorldShape(
			Bridge.NegativeSideRailing.Posts[0u].Shape, PostShape ),
			"支柱のworld衝突を読める" );
		Harness.CheckNearF32( DeckShape.Box.center.y, 1.75f, 0.001f,
			"床板衝突を上面から下へ持たせる" );
		Harness.CheckNearF32( DeckShape.Box.half_size.x, 2.0f, 0.001f,
			"床板衝突へ指定半幅を使う" );
		Harness.CheckNearF32( PostShape.Box.center.y, 2.6f, 0.001f,
			"支柱衝突を床板上面から立てる" );
		Harness.Check( DeckShape.Layer == 0x4u && PostShape.Layer == 0x4u,
			"全パーツへ同じ衝突レイヤーを使う" );
		Harness.Check( CBridge3DSpawner::Destroy( Graph, Collision, Bridge ),
			"確認後の橋を片付けられる" );
	}

	Harness.BeginSuite( "CBridge3DSpawner / 指定親の変形を全パーツへ共通適用する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn( FStringView( "BridgeRoot" ) );
		Harness.Check( Parent.Succeeded(), "橋を繋ぐ親を作れる" );
		if ( Parent.Node != nullptr )
		{
			Parent.Node->SetPosition( FVec3{ 10.0f, 1.0f, -5.0f } );
			Parent.Node->SetScale( FVec3{ 2.0f, 1.0f, 3.0f } );
		}

		FBridge3DSpawnResult Bridge = CBridge3DSpawner::SpawnInto(
			Graph, Collision, FBridge3DSpawnParams{}, Parent.Node );
		Harness.Check( Bridge.Succeeded(), "指定親の下へ橋を置ける" );
		if ( Bridge.Succeeded() && Parent.Node != nullptr )
		{
			Harness.CheckEqualU64( Parent.Node->ChildCount(), Bridge.PartCount(),
				"親の直下へ全パーツを置く" );
			Harness.Check( AllPartsUseParent_Internal( Bridge, *Parent.Node ),
				"全パーツが同じ親を使う" );
		}

		FWorldCollisionShape3D DeckShape;
		Harness.Check( Bridge.Succeeded()
			&& Collision.TryGetWorldShape( Bridge.Deck.Shape, DeckShape ),
			"親変形後の床板衝突を読める" );
		Harness.CheckNearF32( DeckShape.Box.center.x, 10.0f, 0.001f,
			"親のX移動を反映する" );
		Harness.CheckNearF32( DeckShape.Box.center.y, 0.775f, 0.001f,
			"親のY移動と床板厚を反映する" );
		Harness.CheckNearF32( DeckShape.Box.center.z, 7.0f, 0.001f,
			"親Z尺度を床板中心へ反映する" );
		Harness.CheckNearF32( DeckShape.Box.half_size.x, 3.0f, 0.001f,
			"親X尺度を床板幅へ反映する" );
		Harness.CheckNearF32( DeckShape.Box.half_size.z, 12.0f, 0.001f,
			"親Z尺度を床板長へ反映する" );
		Harness.Check( CBridge3DSpawner::Destroy( Graph, Collision, Bridge ),
			"親付き橋を片付けられる" );
	}

	Harness.BeginSuite( "CBridge3DSpawner / 不正入力と別場面で半端物を残さない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FBridge3DSpawnParams Broken;
		Broken.Length = 0.0f;
		const FBridge3DSpawnResult Failed = CBridge3DSpawner::SpawnInto(
			Graph, Collision, Broken );
		Harness.Check( !Failed.Succeeded() && Failed.IsEmpty(),
			"不正値を生成前に拒否する" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u,
			"不正値でノードを足さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u,
			"不正値で形状を足さない" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		const FBridge3DSpawnResult Failed = CBridge3DSpawner::SpawnInto(
			Graph, OtherCollision, FBridge3DSpawnParams{} );
		Harness.Check( !Failed.Succeeded() && Failed.IsEmpty(),
			"別場面の衝突集合を拒否する" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u,
			"登録失敗時に生成ノードを巻き戻す" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u,
			"登録失敗時に識別子も解放する" );
		Harness.CheckEqualU64( OtherCollision.ShapeCount(), 0u,
			"別場面へ形状を残さない" );
	}

	Harness.BeginSuite( "CBridge3DSpawner / 全パーツを検証してから一括破棄する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FBridge3DSpawnResult Bridge = CBridge3DSpawner::SpawnInto(
			Graph, Collision, FBridge3DSpawnParams{} );
		Harness.Check( Bridge.Succeeded(), "破棄確認用の橋を置ける" );
		Harness.Check( CBridge3DSpawner::Destroy( Graph, Collision, Bridge ),
			"床板と両側柵を一括破棄できる" );
		Harness.Check( Bridge.IsEmpty() && !Bridge.Succeeded(),
			"成功時だけ結果を空にする" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u,
			"全形状を直ちに外す" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u,
			"全ノードを残さない" );
		Harness.Check( !CBridge3DSpawner::Destroy( Graph, Collision, Bridge ),
			"空結果の二重破棄を拒否する" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FBridge3DSpawnResult Bridge = CBridge3DSpawner::SpawnInto(
			Graph, Collision, FBridge3DSpawnParams{} );
		const FCollisionShapeId3D OriginalShape =
			Bridge.PositiveSideRailing.Posts[0u].Shape;
		Bridge.PositiveSideRailing.Posts[0u].Shape =
			Bridge.NegativeSideRailing.Posts[0u].Shape;

		Harness.Check( !CBridge3DSpawner::Destroy( Graph, Collision, Bridge ),
			"側をまたぐ重複形状番号を破棄前に拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), Bridge.PartCount(),
			"重複結果でも形状を外さない" );
		Harness.Check( NoPartIsPendingDestroy_Internal( Bridge ),
			"重複結果でもノードを破棄予定にしない" );

		Bridge.PositiveSideRailing.Posts[0u].Shape = OriginalShape;
		Harness.Check( CBridge3DSpawner::Destroy( Graph, Collision, Bridge ),
			"結果を戻せば片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		FBridge3DSpawnResult Bridge = CBridge3DSpawner::SpawnInto(
			Graph, Collision, FBridge3DSpawnParams{} );

		Harness.Check( !CBridge3DSpawner::Destroy(
			OtherGraph, OtherCollision, Bridge ), "別場面からの破棄を拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), Bridge.PartCount(),
			"元場面の形状を保つ" );
		Harness.Check( NoPartIsPendingDestroy_Internal( Bridge ),
			"元場面のノードを保つ" );
		Harness.Check( CBridge3DSpawner::Destroy( Graph, Collision, Bridge ),
			"元場面なら片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FBridge3DSpawnResult Bridge = CBridge3DSpawner::SpawnInto(
			Graph, Collision, FBridge3DSpawnParams{} );
		const FNodeId DeckNode = Graph.IdOf( Bridge.Deck.Node );
		Harness.Check( Graph.Destroy( DeckNode ), "床板を先に破棄予定へ移せる" );
		Harness.Check( CBridge3DSpawner::Destroy( Graph, Collision, Bridge ),
			"床板が破棄予定でも両側柵と形状を片付けられる" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u,
			"破棄予定を含む全形状を外す" );
	}
}
