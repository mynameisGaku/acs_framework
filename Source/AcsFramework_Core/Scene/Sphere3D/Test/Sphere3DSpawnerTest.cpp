// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Sphere3D/Sphere3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 置いた球ノードから表示用の球部品を取り出す。 */
	const AMeshComponent3D* SphereMeshOf( const FCollidableModel3DSpawnResult& Sphere ) noexcept
	{
		return Sphere.Node != nullptr ? Sphere.Node->GetComponent<AMeshComponent3D>() : nullptr;
	}
}


void RunSphere3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FSphere3DSpawnParams / 既定値だけで衝突付き球になる" );

	{
		const FSphere3DSpawnParams Sphere;
		Harness.Check( Sphere.IsValid(), "既定値をそのまま使える" );
		Harness.CheckEqualF32( Sphere.Radius, 0.5f, "既定の半径" );
		Harness.Check( Sphere.CollisionLayer != 0u, "既定で衝突問い合わせへ現れる" );
		Harness.Check( Sphere.bCastsShadow, "既定で立体の影を落とす" );

		const FSphere3DSpawnParams Sized = FSphere3DSpawnParams::FromRadius(
			1.25f, FVec3{ 2.0f, 1.25f, -3.0f } );
		Harness.Check( Sized.IsValid(), "半径と中心位置だけで設定を作れる" );
		Harness.CheckEqualF32( Sized.Radius, 1.25f, "指定した半径" );
		Harness.CheckEqualF32( Sized.Position.x, 2.0f, "指定した中心X" );
		Harness.CheckEqualF32( Sized.Position.y, 1.25f, "指定した中心Y" );
		Harness.CheckEqualF32( Sized.Position.z, -3.0f, "指定した中心Z" );
	}

	Harness.BeginSuite( "FSphere3DSpawnParams / 表示と衝突を壊す値を配置前に弾く" );

	{
		FSphere3DSpawnParams Broken;
		Broken.Radius = 0.0f;
		Harness.Check( !Broken.IsValid(), "半径0を拒否する" );

		Broken = FSphere3DSpawnParams{};
		Broken.Radius = -1.0f;
		Harness.Check( !Broken.IsValid(), "負の半径を拒否する" );

		Broken = FSphere3DSpawnParams{};
		Broken.Radius = std::numeric_limits<f32>::infinity();
		Harness.Check( !Broken.IsValid(), "有限でない半径を拒否する" );

		Broken = FSphere3DSpawnParams{};
		Broken.Radius = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な直径へ変換できない半径を拒否する" );

		Broken = FSphere3DSpawnParams{};
		Broken.Position.z = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない中心位置を拒否する" );

		Broken = FSphere3DSpawnParams{};
		Broken.Color.w = 1.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の表面色を拒否する" );

		Broken = FSphere3DSpawnParams{};
		Broken.Metallic = -0.01f;
		Harness.Check( !Broken.IsValid(), "負の金属度を拒否する" );

		Broken = FSphere3DSpawnParams{};
		Broken.Roughness = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない粗さを拒否する" );

		Broken = FSphere3DSpawnParams{};
		Broken.CollisionLayer = 0u;
		Harness.Check( !Broken.IsValid(), "問い合わせ不能なレイヤー0を拒否する" );
	}

	Harness.BeginSuite( "CSphere3DSpawner / 表示と球型衝突を同じ半径で置く" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FSphere3DSpawnParams Params = FSphere3DSpawnParams::FromRadius(
			1.25f, FVec3{ 1.0f, 2.0f, 3.0f } );
		Params.Color = FVec4{ 0.20f, 0.35f, 0.55f, 1.0f };
		Params.Metallic = 0.15f;
		Params.Roughness = 0.30f;
		Params.bCastsShadow = false;
		Params.CollisionLayer = 0x4u;
		Params.Name = FStringView( "SolidBall" );

		FCollidableModel3DSpawnResult Sphere = CSphere3DSpawner::SpawnInto(
			Graph, Collision, Params );
		Harness.Check( Sphere.Succeeded(), "球表示と衝突を一括生成できる" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u, "表示ノードを1個だけ置く" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 1u, "球衝突を1個だけ登録する" );

		if ( Sphere.Node != nullptr )
		{
			Harness.Check( Sphere.Node->Name() == FStringView( "SolidBall" ), "指定名を付ける" );
			Harness.CheckEqualF32( Sphere.Node->Local().position.x, 1.0f, "指定した中心Xを使う" );
			Harness.CheckEqualF32( Sphere.Node->Local().position.y, 2.0f, "指定した中心Yを使う" );
			Harness.CheckEqualF32( Sphere.Node->Local().position.z, 3.0f, "指定した中心Zを使う" );
			Harness.CheckEqualF32( Sphere.Node->Local().scale.x, 2.5f, "X尺度へ直径を使う" );
			Harness.CheckEqualF32( Sphere.Node->Local().scale.y, 2.5f, "Y尺度へ直径を使う" );
			Harness.CheckEqualF32( Sphere.Node->Local().scale.z, 2.5f, "Z尺度へ直径を使う" );
		}

		const AMeshComponent3D* const Mesh = SphereMeshOf( Sphere );
		Harness.Check( Mesh != nullptr, "表示用の球部品を持つ" );
		if ( Mesh != nullptr )
		{
			Harness.Check( Mesh->Primitive() == EMeshPrimitive3D::Sphere, "表示形状は球" );
			Harness.CheckEqualF32( Mesh->Color().z, 0.55f, "指定した表面色を使う" );
			Harness.CheckEqualF32( Mesh->Material().pbr.metallic, 0.15f, "指定した金属度を使う" );
			Harness.CheckEqualF32( Mesh->Material().pbr.roughness, 0.30f, "指定した粗さを使う" );
			Harness.Check( !Mesh->CastsShadow(), "指定した影設定を反映する" );
		}

		FWorldCollisionShape3D WorldShape;
		Harness.Check( Collision.TryGetWorldShape( Sphere.Shape, WorldShape ), "生成したworld衝突を読める" );
		Harness.Check( WorldShape.Kind == FWorldCollisionShape3D::EKind::Sphere, "球の衝突は球" );
		Harness.CheckNearF32( WorldShape.Sphere.center.x, 1.0f, 0.001f, "衝突中心Xは表示中心と一致する" );
		Harness.CheckNearF32( WorldShape.Sphere.center.y, 2.0f, 0.001f, "衝突中心Yは表示中心と一致する" );
		Harness.CheckNearF32( WorldShape.Sphere.center.z, 3.0f, 0.001f, "衝突中心Zは表示中心と一致する" );
		Harness.CheckNearF32( WorldShape.Sphere.radius, 1.25f, 0.001f, "衝突半径は表示半径と一致する" );
		Harness.Check( WorldShape.Layer == 0x4u && WorldShape.bQueryable,
			"指定レイヤーで問い合わせ可能にする" );

		Harness.Check( CModel3DSpawner::DestroyCollidable( Graph, Collision, Sphere ),
			"既存の一括破棄で表示と球を片付けられる" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "破棄時に球衝突を直ちに外す" );
	}

	Harness.BeginSuite( "CSphere3DSpawner / 親の非一様拡縮を安全側のworld球へ反映する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn( FStringView( "SphereRoot" ) );
		Harness.Check( Parent.Succeeded(), "球を繋ぐ親を作れる" );
		if ( Parent.Node != nullptr )
		{
			Parent.Node->SetPosition( FVec3{ 10.0f, 1.0f, -5.0f } );
			Parent.Node->SetScale( FVec3{ 2.0f, 3.0f, 4.0f } );
		}

		const FSphere3DSpawnParams Params = FSphere3DSpawnParams::FromRadius(
			0.75f, FVec3{ 1.0f, 2.0f, 3.0f } );
		const FCollidableModel3DSpawnResult Sphere = CSphere3DSpawner::SpawnInto(
			Graph, Collision, Params, Parent.Node );
		Harness.Check( Sphere.Succeeded(), "非一様拡縮の親へ球を置ける" );
		Harness.Check( Sphere.Node != nullptr && Sphere.Node->Parent() == Parent.Node,
			"指定した親へ球を繋ぐ" );

		FWorldCollisionShape3D WorldShape;
		Harness.Check( Collision.TryGetWorldShape( Sphere.Shape, WorldShape ),
			"親変形後のworld球を読める" );
		Harness.CheckNearF32( WorldShape.Sphere.center.x, 12.0f, 0.001f, "親のX変形を中心へ反映する" );
		Harness.CheckNearF32( WorldShape.Sphere.center.y, 7.0f, 0.001f, "親のY変形を中心へ反映する" );
		Harness.CheckNearF32( WorldShape.Sphere.center.z, 7.0f, 0.001f, "親のZ変形を中心へ反映する" );
		Harness.CheckNearF32( WorldShape.Sphere.radius, 3.0f, 0.001f,
			"最大親尺度4で表示楕円体を包む外接半径を使う" );
	}

	Harness.BeginSuite( "CSphere3DSpawner / 失敗時に半端な球を残さない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FSphere3DSpawnParams Broken;
		Broken.Radius = 0.0f;

		const FCollidableModel3DSpawnResult Failed = CSphere3DSpawner::SpawnInto(
			Graph, Collision, Broken );
		Harness.Check( !Failed.Succeeded(), "不正値を生成前に拒否する" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "不正値でノードを足さない" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u, "不正値で形状を足さない" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };

		const FCollidableModel3DSpawnResult Failed = CSphere3DSpawner::SpawnInto(
			Graph, OtherCollision, FSphere3DSpawnParams{} );
		Harness.Check( !Failed.Succeeded(), "別場面の衝突集合を拒否する" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "登録失敗時に表示ノードを巻き戻す" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "登録失敗時に識別子も解放する" );
		Harness.CheckEqualU64( OtherCollision.ShapeCount(), 0u, "別場面へ形状を残さない" );
	}
}
