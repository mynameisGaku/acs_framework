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

	Harness.BeginSuite( "CStreetLamp3DSpawner / 街灯の3ノードと衝突を同期更新する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision( Graph );
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "StreetRoot" ) );
		FStreetLamp3DSpawnResult StreetLamp =
			CStreetLamp3DSpawner::SpawnInto( Graph, Collision,
				FStreetLamp3DSpawnParams::At(
					FVec3{ -1.0f, 0.0f, 1.0f } ), Parent.Node );
		const FCollisionShapeId3D OriginalShape = StreetLamp.Post.Shape;
		const FNodeId OriginalBulbId = StreetLamp.Lamp.BulbId();
		const FNodeId OriginalLightId = StreetLamp.Lamp.LightId();

		FStreetLamp3DSpawnParams Updated =
			FStreetLamp3DSpawnParams::At( FVec3{ 2.0f, 0.25f, -3.0f } );
		Updated.PostHeight = 3.2f;
		Updated.PostWidth = 0.20f;
		Updated.BulbRadius = 0.30f;
		Updated.PostColor = FVec4{ 0.28f, 0.32f, 0.40f, 1.0f };
		Updated.PostMetallic = 0.90f;
		Updated.PostRoughness = 0.18f;
		Updated.LampColor = FVec3{ 0.20f, 0.55f, 1.0f };
		Updated.EmissiveStrength = 6.0f;
		Updated.LightIntensity = 3.0f;
		Updated.LightRange = 8.5f;
		Updated.CollisionLayer = 0x40u;
		Updated.PostName = FStringView( "UpdatedStreetPost" );
		Updated.BulbName = FStringView( "UpdatedStreetBulb" );
		Updated.LightName = FStringView( "UpdatedStreetLight" );

		Harness.Check( CStreetLamp3DSpawner::TryApplyTo(
			Graph, Collision, StreetLamp, Updated ),
			"有効な新指定を街灯全体へ一括反映できる" );
		Harness.Check( StreetLamp.Post.Shape == OriginalShape
			&& StreetLamp.Lamp.BulbId() == OriginalBulbId
			&& StreetLamp.Lamp.LightId() == OriginalLightId,
			"更新しても衝突形状と3ノードの番号を保つ" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 5u,
			"更新でノードを作り直さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 1u,
			"更新で衝突形状を作り直さない" );

		ANode* const Post = StreetLamp.Post.Node;
		ANode* const Bulb = StreetLamp.Lamp.Bulb();
		ANode* const LightNode = StreetLamp.Lamp.Light();
		CheckVector_Internal( Harness,
			Post != nullptr ? Post->Local().position : FVec3{},
			FVec3{ 2.0f, 1.85f, -3.0f },
			"ポスト中心を新しい床位置と高さから更新する" );
		CheckVector_Internal( Harness,
			Post != nullptr ? Post->Local().scale : FVec3{},
			FVec3{ 0.20f, 3.2f, 0.20f },
			"ポストを新しい幅と高さへ更新する" );
		CheckVector_Internal( Harness,
			Bulb != nullptr ? Bulb->Local().position : FVec3{},
			FVec3{ 2.0f, 3.75f, -3.0f },
			"発光球の下端を更新後もポスト上端へ接触させる" );
		CheckVector_Internal( Harness,
			LightNode != nullptr ? LightNode->Local().position : FVec3{},
			Bulb != nullptr ? Bulb->Local().position : FVec3{},
			"点光源を更新後も発光球と同じ位置へ置く" );
		CheckVector_Internal( Harness,
			Bulb != nullptr ? Bulb->Local().scale : FVec3{},
			FVec3{ 0.60f, 0.60f, 0.60f },
			"発光球を新しい半径へ更新する" );
		Harness.Check( Post != nullptr
			&& Post->Name() == FStringView( "UpdatedStreetPost" )
			&& Bulb != nullptr
			&& Bulb->Name() == FStringView( "UpdatedStreetBulb" )
			&& LightNode != nullptr
			&& LightNode->Name() == FStringView( "UpdatedStreetLight" ),
			"街灯3ノードの役割名を同期更新する" );

		AMeshComponent3D* const PostMesh = MeshOf_Internal( Post );
		AMeshComponent3D* const BulbMesh = MeshOf_Internal( Bulb );
		ALightComponent3D* const Light = LightOf_Internal( LightNode );
		Harness.Check( PostMesh != nullptr
			&& PostMesh->Primitive() == EMeshPrimitive3D::Cube,
			"更新後もポストを立方体表示に保つ" );
		if ( PostMesh != nullptr )
		{
			Harness.CheckEqualF32( PostMesh->Color().x, 0.28f,
				"ポストを新しい表面色へ更新する" );
			Harness.CheckEqualF32( PostMesh->Material().pbr.metallic, 0.90f,
				"ポストを新しい金属度へ更新する" );
			Harness.CheckEqualF32( PostMesh->Material().pbr.roughness, 0.18f,
				"ポストを新しい粗さへ更新する" );
		}
		Harness.Check( BulbMesh != nullptr && BulbMesh->MaterialLoaded(),
			"更新後も発光球の材質を確定する" );
		if ( BulbMesh != nullptr && BulbMesh->MaterialLoaded() )
		{
			CheckVector_Internal( Harness, BulbMesh->Material().pbr.emissive,
				Updated.LampColor, "発光球を新しい共有色へ更新する" );
			Harness.CheckEqualF32(
				BulbMesh->Material().pbr.emissiveStrength,
				Updated.EmissiveStrength,
				"発光球を新しいHDR強度へ更新する" );
		}
		if ( Light != nullptr )
		{
			FPointLight Output{};
			Harness.Check( Light->FillPoint( Output ),
				"更新後の点光源を描画値へ変換できる" );
			Harness.CheckNearF32( Output.range, Updated.LightRange, 0.0001f,
				"点光源を新しい到達距離へ更新する" );
			Harness.CheckNearF32( Output.color.z,
				Updated.LampColor.z * Updated.LightIntensity, 0.0001f,
				"点光源を新しい色と照明強度へ更新する" );
		}

		FWorldCollisionShape3D WorldShape;
		Harness.Check( Collision.TryGetWorldShape(
			StreetLamp.Post.Shape, WorldShape ),
			"更新後のポスト衝突を読める" );
		CheckVector_Internal( Harness, WorldShape.Box.center,
			FVec3{ 2.0f, 1.85f, -3.0f },
			"ポスト衝突を新しい中心へ更新する" );
		CheckVector_Internal( Harness, WorldShape.Box.half_size,
			FVec3{ 0.10f, 1.6f, 0.10f },
			"ポスト衝突を新しい半寸法へ更新する" );
		Harness.Check( WorldShape.Layer == 0x40u && WorldShape.bQueryable,
			"ポスト衝突を新しいレイヤーへ更新する" );
	}

	Harness.BeginSuite( "CStreetLamp3DSpawner / 更新失敗時は街灯全体を変えない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision( Graph );
		const FScene3DSpawnResult Parent = Graph.TrySpawn(
			FStringView( "StreetRoot" ) );
		const FScene3DSpawnResult OtherParent = Graph.TrySpawn(
			FStringView( "OtherRoot" ) );
		FStreetLamp3DSpawnParams Initial =
			FStreetLamp3DSpawnParams::At( FVec3{ 1.0f, 0.0f, 2.0f } );
		Initial.CollisionLayer = 0x2u;
		FStreetLamp3DSpawnResult StreetLamp =
			CStreetLamp3DSpawner::SpawnInto(
				Graph, Collision, Initial, Parent.Node );
		ANode* const Post = StreetLamp.Post.Node;
		ANode* const Bulb = StreetLamp.Lamp.Bulb();
		ANode* const LightNode = StreetLamp.Lamp.Light();

		FStreetLamp3DSpawnParams Updated =
			FStreetLamp3DSpawnParams::At( FVec3{ 8.0f, 1.0f, -4.0f } );
		Updated.PostHeight = 4.0f;
		Updated.LampColor = FVec3{ 0.2f, 0.4f, 1.0f };
		Updated.CollisionLayer = 0x80u;
		FStreetLamp3DSpawnParams Invalid = Updated;
		Invalid.PostWidth = 0.0f;

		Harness.Check( !CStreetLamp3DSpawner::TryApplyTo(
			Graph, Collision, StreetLamp, Invalid ),
			"不正な新指定を拒否する" );
		CheckVector_Internal( Harness,
			Post != nullptr ? Post->Local().position : FVec3{},
			FVec3{ 1.0f, 1.2f, 2.0f },
			"不正入力ではポストを変えない" );
		CheckVector_Internal( Harness,
			Bulb != nullptr ? Bulb->Local().position : FVec3{},
			FVec3{ 1.0f, 2.58f, 2.0f },
			"不正入力では発光球を変えない" );
		FStreetLamp3DSpawnParams InvalidLamp = Updated;
		InvalidLamp.LightRange = 0.0f;
		Harness.Check( !CStreetLamp3DSpawner::TryApplyTo(
			Graph, Collision, StreetLamp, InvalidLamp ),
			"不正なランプ指定もポスト更新前に拒否する" );
		CheckVector_Internal( Harness,
			Post != nullptr ? Post->Local().position : FVec3{},
			FVec3{ 1.0f, 1.2f, 2.0f },
			"ランプ指定が不正でもポストを変えない" );

		CSceneNodeGraph WrongGraph;
		CSceneCollision3D WrongCollision( WrongGraph );
		Harness.Check( !CStreetLamp3DSpawner::TryApplyTo(
			WrongGraph, WrongCollision, StreetLamp, Updated ),
			"別場面からの更新を拒否する" );
		CheckVector_Internal( Harness,
			LightNode != nullptr ? LightNode->Local().position : FVec3{},
			FVec3{ 1.0f, 2.58f, 2.0f },
			"別場面では点光源を変えない" );

		CSceneCollision3D EmptyCollision( Graph );
		Harness.Check( !CStreetLamp3DSpawner::TryApplyTo(
			Graph, EmptyCollision, StreetLamp, Updated ),
			"ポスト形状を持たない衝突集合を拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 1u,
			"別衝突集合による失敗でも元の形状を保つ" );

		if ( Bulb != nullptr && OtherParent.Node != nullptr )
			Bulb->Reparent( *OtherParent.Node );
		Graph.ResolveStructuralChanges();
		Harness.Check( Bulb != nullptr && Bulb->Parent() == OtherParent.Node,
			"共通親を崩す更新検証を準備できる" );
		Harness.Check( !CStreetLamp3DSpawner::TryApplyTo(
			Graph, Collision, StreetLamp, Updated ),
			"3部品が共通親を失った更新を拒否する" );
		CheckVector_Internal( Harness,
			Post != nullptr ? Post->Local().position : FVec3{},
			FVec3{ 1.0f, 1.2f, 2.0f },
			"共通親不一致ではポストを変えない" );
		if ( Bulb != nullptr && Parent.Node != nullptr )
			Bulb->Reparent( *Parent.Node );
		Graph.ResolveStructuralChanges();
		Harness.Check( Bulb != nullptr && Bulb->Parent() == Parent.Node,
			"発光球を生成時の共通親へ戻せる" );

		FStreetLamp3DSpawnResult Forged = StreetLamp;
		Forged.Post.Node = Bulb;
		Harness.Check( !CStreetLamp3DSpawner::TryApplyTo(
			Graph, Collision, Forged, Updated ),
			"発光球をポストに見せた重複結果を拒否する" );

		FWorldCollisionShape3D WorldShape;
		Harness.Check( Collision.TryGetWorldShape(
			StreetLamp.Post.Shape, WorldShape ),
			"失敗後も元のポスト衝突を読める" );
		Harness.Check( WorldShape.Layer == 0x2u,
			"全失敗後も元の衝突レイヤーを保つ" );
		CheckVector_Internal( Harness, WorldShape.Box.center,
			FVec3{ 1.0f, 1.2f, 2.0f },
			"全失敗後も元の衝突中心を保つ" );
		Harness.Check( Post != nullptr
			&& Post->Name() == FStringView( "StreetLampPost" )
			&& Bulb != nullptr
			&& Bulb->Name() == FStringView( "StreetLampBulb" )
			&& LightNode != nullptr
			&& LightNode->Name() == FStringView( "StreetLampLight" ),
			"全失敗後も街灯3ノードの名前を保つ" );
		AMeshComponent3D* const PostMesh = MeshOf_Internal( Post );
		AMeshComponent3D* const BulbMesh = MeshOf_Internal( Bulb );
		Harness.Check( PostMesh != nullptr
			&& PostMesh->Color().x == Initial.PostColor.x,
			"全失敗後もポスト色を保つ" );
		Harness.Check( BulbMesh != nullptr && BulbMesh->MaterialLoaded()
			&& BulbMesh->Material().pbr.emissive.x == Initial.LampColor.x
			&& BulbMesh->Material().pbr.emissiveStrength
				== Initial.EmissiveStrength,
			"全失敗後も発光球の材質を保つ" );

		Harness.Check( Graph.Destroy( StreetLamp.Lamp.BulbId() ),
			"破棄予定部品の更新検証を準備できる" );
		Harness.Check( !CStreetLamp3DSpawner::TryApplyTo(
			Graph, Collision, StreetLamp, Updated ),
			"発光球が破棄予定なら街灯更新を拒否する" );
		CheckVector_Internal( Harness,
			LightNode != nullptr ? LightNode->Local().position : FVec3{},
			FVec3{ 1.0f, 2.58f, 2.0f },
			"破棄予定による失敗時は残る点光源を変えない" );
		Harness.Check( !CStreetLamp3DSpawner::TryApplyTo(
			Graph, Collision, FStreetLamp3DSpawnResult{}, Updated ),
			"空の街灯結果を拒否する" );
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
