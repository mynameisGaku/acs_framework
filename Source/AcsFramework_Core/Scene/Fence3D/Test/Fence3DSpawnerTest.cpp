// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Fence3D/Fence3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 全支柱と横桟が指定した親へ繋がっているか返す。 */
	bool AllPartsUseParent( const FFence3DSpawnResult& Fence,
		const ANode& Parent ) noexcept
	{
		if ( !Fence ) return false;
		for ( usize Index = 0u; Index < Fence.Posts.Num(); ++Index )
		{
			if ( Fence.Posts[Index].Node == nullptr
				|| Fence.Posts[Index].Node->Parent() != &Parent ) return false;
		}
		for ( usize Index = 0u; Index < Fence.Rails.Num(); ++Index )
		{
			if ( Fence.Rails[Index].Node == nullptr
				|| Fence.Rails[Index].Node->Parent() != &Parent ) return false;
		}
		return true;
	}

	/** 全支柱と横桟が破棄予定ではないか返す。 */
	bool NoPartIsPendingDestroy( const FFence3DSpawnResult& Fence ) noexcept
	{
		if ( !Fence ) return false;
		for ( usize Index = 0u; Index < Fence.Posts.Num(); ++Index )
		{
			if ( Fence.Posts[Index].Node == nullptr
				|| Fence.Posts[Index].Node->IsPendingDestroy() ) return false;
		}
		for ( usize Index = 0u; Index < Fence.Rails.Num(); ++Index )
		{
			if ( Fence.Rails[Index].Node == nullptr
				|| Fence.Rails[Index].Node->IsPendingDestroy() ) return false;
		}
		return true;
	}

	/** 指定方向の終点支柱、横桟中心、横桟寸法を調べる。 */
	void CheckDirection( CTestHarness& Harness, EFence3DDirection Direction,
		f32 ExpectedEndX, f32 ExpectedEndZ, f32 ExpectedRailX,
		f32 ExpectedRailZ, f32 ExpectedRailSizeX, f32 ExpectedRailSizeZ )
	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FFence3DSpawnParams Params = FFence3DSpawnParams::FromDimensions(
			4.0f, 1.2f, FVec3{}, Direction );
		Params.MaximumPostSpacing = 8.0f;
		Params.RailCount = 1u;
		FFence3DSpawnResult Fence = CFence3DSpawner::SpawnInto(
			Graph, Collision, Params );
		Harness.Check( Fence.Succeeded() && Fence.PostCount() == 2u
			&& Fence.RailCount() == 1u, "指定方向へ両端支柱と横桟を置ける" );
		if ( Fence.Succeeded() )
		{
			const ANode& EndPost = *Fence.Posts[1u].Node;
			const ANode& Rail = *Fence.Rails[0u].Node;
			Harness.CheckNearF32( EndPost.Local().position.x, ExpectedEndX,
				0.001f, "終点支柱のXを方向へ合わせる" );
			Harness.CheckNearF32( EndPost.Local().position.z, ExpectedEndZ,
				0.001f, "終点支柱のZを方向へ合わせる" );
			Harness.CheckNearF32( Rail.Local().position.x, ExpectedRailX,
				0.001f, "横桟中心Xを両端の中間へ置く" );
			Harness.CheckNearF32( Rail.Local().position.z, ExpectedRailZ,
				0.001f, "横桟中心Zを両端の中間へ置く" );
			Harness.CheckNearF32( Rail.Local().scale.x, ExpectedRailSizeX,
				0.001f, "横桟のX寸法を方向へ合わせる" );
			Harness.CheckNearF32( Rail.Local().scale.z, ExpectedRailSizeZ,
				0.001f, "横桟のZ寸法を方向へ合わせる" );
		}
		Harness.Check( CFence3DSpawner::Destroy( Graph, Collision, Fence ),
			"方向確認後の柵を片付けられる" );
	}
}


void RunFence3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FFence3DSpawnParams / 既定値だけで衝突付き柵になる" );

	{
		const FFence3DSpawnParams Fence;
		Harness.Check( Fence.IsValid(), "既定値をそのまま使える" );
		Harness.CheckEqualF32( Fence.Length, 4.0f, "既定の中心間長さ" );
		Harness.CheckEqualF32( Fence.Height, 1.2f, "既定の柵高" );
		Harness.CheckEqualF32( Fence.MaximumPostSpacing, 2.0f,
			"既定の最大支柱間隔" );
		Harness.CheckEqualU64( Fence.RequiredSectionCount(), 2u,
			"既定値は2区間に分ける" );
		Harness.CheckEqualU64( Fence.RailCount, 2u, "既定で横桟を2本置く" );
		Harness.Check( Fence.Direction == EFence3DDirection::PositiveZ,
			"既定でZ正方向へ伸びる" );
		Harness.Check( Fence.CollisionLayer != 0u,
			"既定で衝突問い合わせへ現れる" );

		FFence3DSpawnParams Sized = FFence3DSpawnParams::FromDimensions(
			5.0f, 1.5f, FVec3{ 1.0f, 2.0f, 3.0f },
			EFence3DDirection::NegativeX );
		Sized.MaximumPostSpacing = 2.0f;
		Harness.Check( Sized.IsValid(), "長さ、高さ、始点、方向だけで設定を作れる" );
		Harness.CheckEqualU64( Sized.RequiredSectionCount(), 3u,
			"最大間隔を守るよう端数区間を切り上げる" );
		Harness.CheckEqualF32( Sized.StartPostBottomCenter.y, 2.0f,
			"指定した底面高さを保つ" );
		Harness.Check( Sized.Direction == EFence3DDirection::NegativeX,
			"指定した方向を保つ" );
		Harness.Check( FFence3DSpawnResult{}.IsEmpty(), "既定の生成結果は空" );
	}

	Harness.BeginSuite( "FFence3DSpawnParams / 半端な柵を作る値を配置前に弾く" );

	{
		FFence3DSpawnParams Broken;
		Broken.Length = 0.0f;
		Harness.Check( !Broken.IsValid() && Broken.RequiredSectionCount() == 0u,
			"長さ0を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.Height = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない高さを拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.MaximumPostSpacing = -1.0f;
		Harness.Check( !Broken.IsValid(), "負の最大支柱間隔を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.PostThickness = 0.0f;
		Harness.Check( !Broken.IsValid(), "支柱幅0を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.RailCount = 0u;
		Harness.Check( !Broken.IsValid(), "横桟0本を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.RailCount = FFence3DSpawnParams::kMaximumRailCount + 1u;
		Harness.Check( !Broken.IsValid(), "上限を超える横桟数を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.RailHeight = 0.0f;
		Harness.Check( !Broken.IsValid(), "横桟高0を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.RailThickness = std::numeric_limits<f32>::infinity();
		Harness.Check( !Broken.IsValid(), "有限でない横桟厚を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.StartPostBottomCenter.z = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない始点を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.Direction = static_cast<EFence3DDirection>( 0xffu );
		Harness.Check( !Broken.IsValid(), "未知の方向を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.Color.w = 1.01f;
		Harness.Check( !Broken.IsValid(), "範囲外の表面色を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.Metallic = -0.01f;
		Harness.Check( !Broken.IsValid(), "負の金属度を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.Roughness = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !Broken.IsValid(), "有限でない粗さを拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.CollisionLayer = 0u;
		Harness.Check( !Broken.IsValid(), "問い合わせ不能なレイヤー0を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.Length = 257.0f;
		Broken.MaximumPostSpacing = 1.0f;
		Harness.Check( !Broken.IsValid() && Broken.RequiredSectionCount() == 0u,
			"上限を超える区間数を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.Length = 1.0f;
		Broken.MaximumPostSpacing = 0.1f;
		Broken.PostThickness = 0.11f;
		Harness.Check( !Broken.IsValid(), "互いに重なる支柱間隔を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.Height = 0.9f;
		Broken.RailCount = 2u;
		Broken.RailHeight = 0.31f;
		Harness.Check( !Broken.IsValid(), "均等間隔より高い横桟を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.Length = std::numeric_limits<f32>::max();
		Broken.MaximumPostSpacing = std::numeric_limits<f32>::max();
		Broken.StartPostBottomCenter.z = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な終点にならない値を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.Height = std::numeric_limits<f32>::max();
		Broken.StartPostBottomCenter.y = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な上端にならない値を拒否する" );

		Broken = FFence3DSpawnParams{};
		Broken.Length = std::numeric_limits<f32>::max();
		Broken.MaximumPostSpacing = std::numeric_limits<f32>::max();
		Broken.PostThickness = std::numeric_limits<f32>::max();
		Broken.StartPostBottomCenter.x = std::numeric_limits<f32>::max();
		Harness.Check( !Broken.IsValid(), "有限な横端にならない値を拒否する" );
	}

	Harness.BeginSuite( "CFence3DSpawner / 最大間隔で分けた支柱を横桟で繋ぐ" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FFence3DSpawnParams Params = FFence3DSpawnParams::FromDimensions(
			5.0f, 1.5f, FVec3{ 1.0f, 2.0f, 3.0f } );
		Params.MaximumPostSpacing = 2.0f;
		Params.PostThickness = 0.20f;
		Params.RailCount = 3u;
		Params.RailHeight = 0.10f;
		Params.RailThickness = 0.08f;
		Params.Color = FVec4{ 0.20f, 0.35f, 0.55f, 1.0f };
		Params.Metallic = 0.15f;
		Params.Roughness = 0.30f;
		Params.bCastsShadow = false;
		Params.CollisionLayer = 0x4u;
		Params.PostName = FStringView( "IronPost" );
		Params.RailName = FStringView( "IronRail" );

		FFence3DSpawnResult Fence = CFence3DSpawner::SpawnInto(
			Graph, Collision, Params );
		Harness.Check( Fence.Succeeded(), "4本の支柱と3本の横桟を一括生成できる" );
		Harness.CheckEqualU64( Fence.PostCount(), 4u, "5mを最大2m間隔の4支柱に分ける" );
		Harness.CheckEqualU64( Fence.RailCount(), 3u, "指定した横桟を3本置く" );
		Harness.CheckEqualU64( Fence.PartCount(), 7u, "結果へ全7部分を保持する" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 7u, "表示ノードを7個だけ置く" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 7u, "箱型衝突を7個だけ登録する" );

		if ( Fence.Succeeded() )
		{
			Harness.Check( Fence.Posts[0u].Node->Name() == FStringView( "IronPost" ),
				"全支柱用の指定名を付ける" );
			Harness.Check( Fence.Rails[0u].Node->Name() == FStringView( "IronRail" ),
				"全横桟用の指定名を付ける" );
			Harness.CheckNearF32( Fence.Posts[0u].Node->Local().position.z, 3.0f,
				0.001f, "始点支柱を指定位置へ置く" );
			Harness.CheckNearF32( Fence.Posts[1u].Node->Local().position.z, 4.666667f,
				0.001f, "中間支柱を均等間隔へ置く" );
			Harness.CheckNearF32( Fence.Posts[3u].Node->Local().position.z, 8.0f,
				0.001f, "終点支柱を指定長さの先へ置く" );
			Harness.CheckNearF32( Fence.Posts[0u].Node->Local().position.y, 2.75f,
				0.001f, "支柱底面を指定高さへ揃える" );
			Harness.CheckNearF32( Fence.Posts[0u].Node->Local().scale.y, 1.5f,
				0.001f, "支柱へ指定高を使う" );
			Harness.CheckNearF32( Fence.Rails[0u].Node->Local().position.y, 2.375f,
				0.001f, "最下横桟を底面と上端の4分の1へ置く" );
			Harness.CheckNearF32( Fence.Rails[2u].Node->Local().position.y, 3.125f,
				0.001f, "最上横桟を底面と上端の4分の3へ置く" );
			Harness.CheckNearF32( Fence.Rails[0u].Node->Local().position.z, 5.5f,
				0.001f, "横桟を両端支柱の中間へ置く" );
			Harness.CheckNearF32( Fence.Rails[0u].Node->Local().scale.x, 0.08f,
				0.001f, "横桟へ指定厚を使う" );
			Harness.CheckNearF32( Fence.Rails[0u].Node->Local().scale.y, 0.10f,
				0.001f, "横桟へ指定高を使う" );
			Harness.CheckNearF32( Fence.Rails[0u].Node->Local().scale.z, 5.0f,
				0.001f, "横桟を両端支柱の中心間へ伸ばす" );
		}

		const AMeshComponent3D* const PostMesh = Fence.Succeeded()
			? Fence.Posts[0u].Node->GetComponent<AMeshComponent3D>() : nullptr;
		const AMeshComponent3D* const RailMesh = Fence.Succeeded()
			? Fence.Rails[0u].Node->GetComponent<AMeshComponent3D>() : nullptr;
		Harness.Check( PostMesh != nullptr && RailMesh != nullptr
			&& PostMesh->Primitive() == EMeshPrimitive3D::Cube
			&& RailMesh->Primitive() == EMeshPrimitive3D::Cube,
			"支柱と横桟は立方体表示を使う" );
		if ( PostMesh != nullptr && RailMesh != nullptr )
		{
			Harness.CheckEqualF32( PostMesh->Color().z, 0.55f,
				"指定した表面色を使う" );
			Harness.CheckEqualF32( RailMesh->Material().pbr.metallic, 0.15f,
				"指定した金属度を使う" );
			Harness.CheckEqualF32( RailMesh->Material().pbr.roughness, 0.30f,
				"指定した粗さを使う" );
			Harness.Check( !PostMesh->CastsShadow() && !RailMesh->CastsShadow(),
				"指定した影設定を全部分へ反映する" );
		}

		FWorldCollisionShape3D PostShape;
		FWorldCollisionShape3D RailShape;
		Harness.Check( Fence.Succeeded()
			&& Collision.TryGetWorldShape( Fence.Posts[0u].Shape, PostShape ),
			"支柱のworld衝突を読める" );
		Harness.Check( Fence.Succeeded()
			&& Collision.TryGetWorldShape( Fence.Rails[0u].Shape, RailShape ),
			"横桟のworld衝突を読める" );
		Harness.Check( PostShape.Kind == FWorldCollisionShape3D::EKind::Box
			&& RailShape.Kind == FWorldCollisionShape3D::EKind::Box,
			"支柱と横桟の衝突は箱" );
		Harness.CheckNearF32( PostShape.Box.half_size.y, 0.75f, 0.001f,
			"支柱衝突へ高さ半分を使う" );
		Harness.CheckNearF32( RailShape.Box.half_size.z, 2.5f, 0.001f,
			"横桟衝突を表示と同じ中心間長さにする" );
		Harness.Check( PostShape.Layer == 0x4u && RailShape.Layer == 0x4u,
			"全部分へ同じ衝突レイヤーを使う" );
		Harness.Check( CFence3DSpawner::Destroy( Graph, Collision, Fence ),
			"確認後の柵を片付けられる" );
	}

	Harness.BeginSuite( "CFence3DSpawner / XとZの正負4方向へ柵を伸ばす" );

	CheckDirection( Harness, EFence3DDirection::PositiveX,
		4.0f, 0.0f, 2.0f, 0.0f, 4.0f, 0.10f );
	CheckDirection( Harness, EFence3DDirection::NegativeX,
		-4.0f, 0.0f, -2.0f, 0.0f, 4.0f, 0.10f );
	CheckDirection( Harness, EFence3DDirection::PositiveZ,
		0.0f, 4.0f, 0.0f, 2.0f, 0.10f, 4.0f );
	CheckDirection( Harness, EFence3DDirection::NegativeZ,
		0.0f, -4.0f, 0.0f, -2.0f, 0.10f, 4.0f );

	Harness.BeginSuite( "CFence3DSpawner / 指定親の変形を全支柱と横桟へ共通適用する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		const FScene3DSpawnResult Parent = Graph.TrySpawn( FStringView( "FenceRoot" ) );
		Harness.Check( Parent.Succeeded(), "柵を繋ぐ親を作れる" );
		if ( Parent.Node != nullptr )
		{
			Parent.Node->SetPosition( FVec3{ 10.0f, 1.0f, -5.0f } );
			Parent.Node->SetScale( FVec3{ 2.0f, 1.0f, 3.0f } );
		}

		FFence3DSpawnParams Params = FFence3DSpawnParams::FromDimensions(
			4.0f, 1.2f, FVec3{ 1.0f, 2.0f, 3.0f } );
		Params.MaximumPostSpacing = 8.0f;
		Params.RailCount = 1u;
		FFence3DSpawnResult Fence = CFence3DSpawner::SpawnInto(
			Graph, Collision, Params, Parent.Node );
		Harness.Check( Fence.Succeeded(), "指定親の下へ柵を置ける" );
		if ( Parent.Node != nullptr )
		{
			Harness.CheckEqualU64( Parent.Node->ChildCount(), 3u,
				"親の直下へ支柱2本と横桟1本を置く" );
			Harness.Check( AllPartsUseParent( Fence, *Parent.Node ),
				"全部分が同じ親を使う" );
		}

		FWorldCollisionShape3D StartPostShape;
		Harness.Check( Fence.Succeeded()
			&& Collision.TryGetWorldShape( Fence.Posts[0u].Shape, StartPostShape ),
			"親変形後の始点支柱衝突を読める" );
		Harness.CheckNearF32( StartPostShape.Box.center.x, 12.0f, 0.001f,
			"親のX変形を反映する" );
		Harness.CheckNearF32( StartPostShape.Box.center.y, 3.6f, 0.001f,
			"親のY変形と支柱中心高を反映する" );
		Harness.CheckNearF32( StartPostShape.Box.center.z, 4.0f, 0.001f,
			"親のZ変形を反映する" );
		Harness.CheckNearF32( StartPostShape.Box.half_size.x, 0.16f, 0.001f,
			"親X尺度を支柱幅へ反映する" );
		Harness.CheckNearF32( StartPostShape.Box.half_size.z, 0.24f, 0.001f,
			"親Z尺度を支柱幅へ反映する" );
		Harness.Check( CFence3DSpawner::Destroy( Graph, Collision, Fence ),
			"親付き柵を片付けられる" );
	}

	Harness.BeginSuite( "CFence3DSpawner / 不正入力と別場面で半端物を残さない" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FFence3DSpawnParams Broken;
		Broken.Length = 0.0f;
		const FFence3DSpawnResult Failed = CFence3DSpawner::SpawnInto(
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
		const FFence3DSpawnResult Failed = CFence3DSpawner::SpawnInto(
			Graph, OtherCollision, FFence3DSpawnParams{} );
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

	Harness.BeginSuite( "CFence3DSpawner / 全支柱と横桟を検証してから一括破棄する" );

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FFence3DSpawnResult Fence = CFence3DSpawner::SpawnInto(
			Graph, Collision, FFence3DSpawnParams{} );
		Harness.Check( Fence.Succeeded(), "破棄確認用の柵を置ける" );
		Harness.Check( CFence3DSpawner::Destroy( Graph, Collision, Fence ),
			"全支柱と横桟を一括破棄できる" );
		Harness.Check( Fence.IsEmpty() && !Fence.Succeeded(),
			"成功時だけ結果を空にする" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u,
			"全形状を直ちに外す" );
		Graph.ResolveStructuralChanges();
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u,
			"全ノードを残さない" );
		Harness.Check( !CFence3DSpawner::Destroy( Graph, Collision, Fence ),
			"空結果の二重破棄を拒否する" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FFence3DSpawnResult Fence = CFence3DSpawner::SpawnInto(
			Graph, Collision, FFence3DSpawnParams{} );
		const FCollisionShapeId3D OriginalShape = Fence.Rails[0u].Shape;
		Fence.Rails[0u].Shape = Fence.Posts[0u].Shape;

		Harness.Check( !CFence3DSpawner::Destroy( Graph, Collision, Fence ),
			"支柱と横桟で重複した形状番号を破棄前に拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 5u,
			"重複結果でも形状を外さない" );
		Harness.Check( NoPartIsPendingDestroy( Fence ),
			"重複結果でもノードを破棄予定にしない" );

		Fence.Rails[0u].Shape = OriginalShape;
		Harness.Check( CFence3DSpawner::Destroy( Graph, Collision, Fence ),
			"結果を戻せば片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		CSceneNodeGraph OtherGraph;
		CSceneCollision3D OtherCollision{ OtherGraph };
		FFence3DSpawnResult Fence = CFence3DSpawner::SpawnInto(
			Graph, Collision, FFence3DSpawnParams{} );

		Harness.Check( !CFence3DSpawner::Destroy( OtherGraph, OtherCollision, Fence ),
			"別場面からの破棄を拒否する" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 5u,
			"元場面の形状を保つ" );
		Harness.Check( NoPartIsPendingDestroy( Fence ),
			"元場面のノードを保つ" );
		Harness.Check( CFence3DSpawner::Destroy( Graph, Collision, Fence ),
			"元場面なら片付けられる" );
	}

	{
		CSceneNodeGraph Graph;
		CSceneCollision3D Collision{ Graph };
		FFence3DSpawnResult Fence = CFence3DSpawner::SpawnInto(
			Graph, Collision, FFence3DSpawnParams{} );
		const FNodeId StartPostNode = Graph.IdOf( Fence.Posts[0u].Node );
		Harness.Check( Graph.Destroy( StartPostNode ),
			"始点支柱を先に破棄予定へ移せる" );
		Harness.Check( CFence3DSpawner::Destroy( Graph, Collision, Fence ),
			"一部が破棄予定でも残りと形状を片付けられる" );
		Harness.CheckEqualU64( Collision.ShapeCount(), 0u,
			"破棄予定を含む全形状を外す" );
	}
}
