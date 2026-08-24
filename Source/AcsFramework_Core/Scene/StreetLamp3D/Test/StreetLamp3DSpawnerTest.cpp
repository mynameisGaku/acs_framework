// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/StreetLamp3D/StreetLamp3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 置いたノードから見た目の部品を取り出す。 */
	AMeshComponent3D* MeshOf_Internal( ANode* Node ) noexcept
	{
		return Node != nullptr ? Node->GetComponent<AMeshComponent3D>() : nullptr;
	}

	/** 置いたノードから光の部品を取り出す。 */
	ALightComponent3D* LightOf_Internal( ANode* Node ) noexcept
	{
		return Node != nullptr ? Node->GetComponent<ALightComponent3D>() : nullptr;
	}

	/** 3成分を小さな浮動小数誤差を許して比較する。 */
	void CheckVector_Internal( CTestHarness& Harness, FVec3 Actual,
		FVec3 Expected, const char* Label ) noexcept
	{
		constexpr f32 kTolerance = 0.0001f;
		Harness.Check( LengthSq( Actual - Expected )
			< kTolerance * kTolerance, Label );
	}

	/** 街灯の3ノードが全て生存していればtrue。 */
	bool AllPartsAlive_Internal(
		const FStreetLamp3DSpawnResult& StreetLamp ) noexcept
	{
		return StreetLamp.Post.Node != nullptr
			&& !StreetLamp.Post.Node->IsPendingDestroy()
			&& StreetLamp.Lamp.Bulb() != nullptr
			&& StreetLamp.Lamp.Light() != nullptr;
	}
}


void RunStreetLamp3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FStreetLamp3DSpawnParams / 床位置からポストとランプを決める" );

	{
		FStreetLamp3DSpawnParams Params =
			FStreetLamp3DSpawnParams::At( FVec3{ 1.0f, 0.5f, -2.0f } );
		Params.PostHeight = 3.0f;
		Params.PostWidth = 0.16f;
		Params.BulbRadius = 0.24f;
		Params.PostColor = FVec4{ 0.20f, 0.24f, 0.30f, 1.0f };
		Params.PostMetallic = 0.85f;
		Params.PostRoughness = 0.28f;
		Params.LampColor = FVec3{ 0.18f, 0.52f, 1.0f };
		Params.EmissiveStrength = 5.0f;
		Params.LightIntensity = 2.8f;
		Params.LightRange = 7.5f;
		Params.CollisionLayer = 0x20u;
		Params.PostName = FStringView( "BlueStreetPost" );
		Params.BulbName = FStringView( "BlueStreetBulb" );
		Params.LightName = FStringView( "BlueStreetLight" );

		FBlock3DSpawnParams Post;
		FLamp3DParams Lamp;
		Harness.Check( Params.IsValid(),
			"床位置と寸法から有効な街灯になる" );
		Harness.Check( Params.TryBuildParts( Post, Lamp ),
			"ポストとランプの指定を一括計算できる" );
		CheckVector_Internal( Harness, Post.Position,
			FVec3{ 1.0f, 2.0f, -2.0f },
			"ポスト中心を底面から高さの半分へ置く" );
		CheckVector_Internal( Harness, Post.Size,
			FVec3{ 0.16f, 3.0f, 0.16f },
			"ポストを指定幅と高さへ揃える" );
		Harness.CheckEqualF32( Post.Metallic, 0.85f,
			"ポストへ金属らしさを渡す" );
		Harness.CheckEqualF32( Post.Roughness, 0.28f,
			"ポストへ粗さを渡す" );
		Harness.CheckEqualU64( Post.CollisionLayer, 0x20u,
			"ポストへ衝突レイヤーを渡す" );
		Harness.Check( Post.Name == FStringView( "BlueStreetPost" ),
			"ポスト名を渡す" );

		CheckVector_Internal( Harness, Lamp.Position,
			FVec3{ 1.0f, 3.74f, -2.0f },
			"発光球の下端をポスト上端へ接する位置に置く" );
		Harness.CheckEqualF32( Lamp.Radius, 0.24f,
			"発光球へ指定半径を渡す" );
		CheckVector_Internal( Harness, Lamp.Color, Params.LampColor,
			"発光球と点光源へ共有色を渡す" );
		Harness.CheckEqualF32( Lamp.LightIntensity, 2.8f,
			"点光源へ照明強度を渡す" );
		Harness.CheckEqualF32( Lamp.Range, 7.5f,
			"点光源へ到達距離を渡す" );
		Harness.Check( Lamp.BulbName == FStringView( "BlueStreetBulb" )
			&& Lamp.LightName == FStringView( "BlueStreetLight" ),
			"ランプの2ノード名を渡す" );
	}

	Harness.BeginSuite( "FStreetLamp3DSpawnParams / 不正値では出力を変えない" );

	{
		const f32 QuietNaN = std::numeric_limits<f32>::quiet_NaN();
		const f32 Maximum = std::numeric_limits<f32>::max();
		FStreetLamp3DSpawnParams BrokenPosition;
		BrokenPosition.BasePosition.x = QuietNaN;
		FStreetLamp3DSpawnParams ZeroHeight;
		ZeroHeight.PostHeight = 0.0f;
		FStreetLamp3DSpawnParams ZeroWidth;
		ZeroWidth.PostWidth = 0.0f;
		FStreetLamp3DSpawnParams ZeroBulb;
		ZeroBulb.BulbRadius = 0.0f;
		FStreetLamp3DSpawnParams BrokenPostColor;
		BrokenPostColor.PostColor.z = 1.1f;
		FStreetLamp3DSpawnParams BrokenMetal;
		BrokenMetal.PostMetallic = -0.1f;
		FStreetLamp3DSpawnParams BrokenLampColor;
		BrokenLampColor.LampColor.x = -0.1f;
		FStreetLamp3DSpawnParams BrokenLight;
		BrokenLight.LightRange = 0.0f;
		FStreetLamp3DSpawnParams EmptyLayer;
		EmptyLayer.CollisionLayer = 0u;
		FStreetLamp3DSpawnParams Overflow;
		Overflow.BasePosition.y = Maximum;
		Overflow.PostHeight = Maximum;

		Harness.Check( !BrokenPosition.IsValid(), "有限でない床位置を拒否する" );
		Harness.Check( !ZeroHeight.IsValid(), "0以下のポスト高さを拒否する" );
		Harness.Check( !ZeroWidth.IsValid(), "0以下のポスト幅を拒否する" );
		Harness.Check( !ZeroBulb.IsValid(), "0以下の発光球半径を拒否する" );
		Harness.Check( !BrokenPostColor.IsValid(), "範囲外のポスト色を拒否する" );
		Harness.Check( !BrokenMetal.IsValid(), "範囲外の金属らしさを拒否する" );
		Harness.Check( !BrokenLampColor.IsValid(), "負のランプ色を拒否する" );
		Harness.Check( !BrokenLight.IsValid(), "0以下の到達距離を拒否する" );
		Harness.Check( !EmptyLayer.IsValid(), "衝突しない0レイヤーを拒否する" );
		Harness.Check( !Overflow.IsValid(), "派生位置が溢れる指定を拒否する" );

		FBlock3DSpawnParams Post = FBlock3DSpawnParams::FromSize(
			FVec3{ 9.0f, 8.0f, 7.0f }, FVec3{ 6.0f, 5.0f, 4.0f } );
		Post.Name = FStringView( "KeepPost" );
		FLamp3DParams Lamp = FLamp3DParams::At( FVec3{ 3.0f, 2.0f, 1.0f } );
		Lamp.BulbName = FStringView( "KeepBulb" );
		Harness.Check( !BrokenPosition.TryBuildParts( Post, Lamp ),
			"不正な指定の変換は失敗する" );
		Harness.Check( Post.Name == FStringView( "KeepPost" )
			&& Lamp.BulbName == FStringView( "KeepBulb" ),
			"失敗時は2つの出力を変更しない" );
	}

	Harness.BeginSuite( "CStreetLamp3DSpawner / 衝突付き街灯を一括配置する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision( Graph );
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "StreetRoot" ) );
		FStreetLamp3DSpawnParams Params =
			FStreetLamp3DSpawnParams::At( FVec3{ -1.5f, 0.0f, 2.0f } );
		Params.PostHeight = 2.8f;
		Params.PostWidth = 0.14f;
		Params.BulbRadius = 0.20f;
		FStreetLamp3DSpawnResult StreetLamp =
			CStreetLamp3DSpawner::SpawnInto(
				Graph, Collision, Params, Parent.Node );

		ANode* const Post = StreetLamp.Post.Node;
		ANode* const Bulb = StreetLamp.Lamp.Bulb();
		ANode* const LightNode = StreetLamp.Lamp.Light();
		AMeshComponent3D* const PostMesh = MeshOf_Internal( Post );
		AMeshComponent3D* const BulbMesh = MeshOf_Internal( Bulb );
		ALightComponent3D* const Light = LightOf_Internal( LightNode );
		Harness.Check( StreetLamp.Succeeded(),
			"ポスト、衝突、発光球、点光源を全て置ける" );
		Harness.CheckEqualU64( StreetLamp.PartCount(), 3u,
			"街灯を3ノードとして数える" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 5u,
			"root、指定親、街灯3ノードだけを登録する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 1u,
			"ポストの箱型衝突だけを登録する" );
		Harness.Check( Post != nullptr && Bulb != nullptr
			&& LightNode != nullptr && Post->Parent() == Parent.Node
			&& Bulb->Parent() == Parent.Node
			&& LightNode->Parent() == Parent.Node,
			"街灯3ノードへ同じ親変形を適用する" );
		Harness.Check( Post != nullptr && Collision.IsRegisteredTo(
			StreetLamp.Post.Shape, *Post ),
			"ポストと箱型衝突を対応付ける" );
		CheckVector_Internal( Harness,
			Post != nullptr ? Post->Local().position : FVec3{},
			FVec3{ -1.5f, 1.4f, 2.0f },
			"ポスト中心を床位置から持ち上げる" );
		CheckVector_Internal( Harness,
			Bulb != nullptr ? Bulb->Local().position : FVec3{},
			FVec3{ -1.5f, 3.0f, 2.0f },
			"発光球をポスト上端へ載せる" );
		CheckVector_Internal( Harness,
			LightNode != nullptr ? LightNode->Local().position : FVec3{},
			Bulb != nullptr ? Bulb->Local().position : FVec3{},
			"点光源を発光球と同じ位置へ置く" );
		Harness.Check( PostMesh != nullptr
			&& PostMesh->Primitive() == EMeshPrimitive3D::Cube
			&& PostMesh->CastsShadow(),
			"影を落とす金属ポストを付ける" );
		Harness.Check( BulbMesh != nullptr
			&& BulbMesh->Primitive() == EMeshPrimitive3D::Sphere
			&& !BulbMesh->CastsShadow(),
			"影を落とさない発光球を付ける" );
		Harness.Check( Light != nullptr
			&& Light->LightKind() == ELight3DKind::Point,
			"発光球の位置へACS点光源を付ける" );
		Harness.Check( Post != nullptr
			&& Post->Name() == FStringView( "StreetLampPost" )
			&& Bulb != nullptr
			&& Bulb->Name() == FStringView( "StreetLampBulb" )
			&& LightNode != nullptr
			&& LightNode->Name() == FStringView( "StreetLampLight" ),
			"街灯3ノードを役割名で区別できる" );

		CSceneNodeGraph WrongGraph;
		CSceneCollision3D WrongCollision( WrongGraph );
		Harness.Check( !CStreetLamp3DSpawner::Destroy(
			WrongGraph, WrongCollision, StreetLamp ),
			"別場面からの一括破棄を拒否する" );
		Harness.Check( AllPartsAlive_Internal( StreetLamp )
			&& Collision.ShapeCount() == 1u,
			"別場面による失敗時は街灯と衝突を変えない" );

		FStreetLamp3DSpawnResult Forged = StreetLamp;
		Forged.Post.Node = StreetLamp.Lamp.Bulb();
		Harness.Check( !CStreetLamp3DSpawner::Destroy(
			Graph, Collision, Forged ),
			"別部品をポストに見せた重複結果を拒否する" );
		Harness.Check( AllPartsAlive_Internal( StreetLamp )
			&& Collision.ShapeCount() == 1u,
			"重複結果による失敗時は本来の街灯を変えない" );

		Harness.Check( CStreetLamp3DSpawner::Destroy(
			Graph, Collision, StreetLamp ),
			"生成時の場面から街灯を一括破棄できる" );
		Harness.Check( StreetLamp.IsEmpty(),
			"破棄成功時は街灯結果を空にする" );
		Harness.Check( Post != nullptr && Post->IsPendingDestroy()
			&& Bulb != nullptr && Bulb->IsPendingDestroy()
			&& LightNode != nullptr && LightNode->IsPendingDestroy(),
			"街灯3ノードを全て破棄予定へ移す" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u,
			"ポストの衝突を残さない" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.RegisteredCount(), 2u,
			"確定後はrootと指定親だけを残す" );
	}

	Harness.BeginSuite( "CStreetLamp3DSpawner / 個別破棄後も残りを片付ける" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision( Graph );
		FStreetLamp3DSpawnResult StreetLamp =
			CStreetLamp3DSpawner::SpawnInto(
				Graph, Collision, FStreetLamp3DSpawnParams{} );
		Harness.Check( StreetLamp
			&& Graph.Destroy( StreetLamp.Lamp.BulbId() ),
			"発光球だけを先に破棄予定へ移せる" );
		Harness.Check( CStreetLamp3DSpawner::Destroy(
			Graph, Collision, StreetLamp ),
			"残る点光源、ポスト、衝突をまとめて片付けられる" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u,
			"個別破棄を含めて街灯ノードを残さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u,
			"個別破棄後もポスト衝突を残さない" );
	}

	Harness.BeginSuite( "CStreetLamp3DSpawner / 無効親と入力を半端に残さない" );

	{
		CSceneNodeGraph OwnerGraph;
		const FScene3DSpawnResult ForeignParent = OwnerGraph.TrySpawn(
			FStringView( "ForeignParent" ) );
		CSceneNodeGraph TargetGraph;
		CSceneCollision3D TargetCollision( TargetGraph );
		const u32 BeforeCount = TargetGraph.RegisteredCount();
		const FStreetLamp3DSpawnResult ForeignResult =
			CStreetLamp3DSpawner::SpawnInto( TargetGraph,
				TargetCollision, FStreetLamp3DSpawnParams{},
				ForeignParent.Node );
		Harness.Check( ForeignResult.IsEmpty(), "別場面の親を拒否する" );
		Harness.CheckEqualU64( TargetGraph.RegisteredCount(), BeforeCount,
			"無効親では街灯ノードを残さない" );
		Harness.CheckEqualU64( TargetCollision.ShapeCount(), 0u,
			"無効親ではポスト衝突を残さない" );

		FStreetLamp3DSpawnParams Invalid;
		Invalid.PostWidth = -1.0f;
		const FStreetLamp3DSpawnResult InvalidResult =
			CStreetLamp3DSpawner::SpawnInto(
				TargetGraph, TargetCollision, Invalid );
		Harness.Check( InvalidResult.IsEmpty(), "不正な街灯設定を拒否する" );
		Harness.CheckEqualU64( TargetGraph.RegisteredCount(), BeforeCount,
			"不正入力では街灯ノードを残さない" );
		Harness.CheckEqualU64( TargetCollision.ShapeCount(), 0u,
			"不正入力ではポスト衝突を残さない" );
	}
}
