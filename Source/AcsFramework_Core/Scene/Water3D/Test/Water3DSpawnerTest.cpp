// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Water3D/Water3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 水面ノードから平面メッシュを取り出す。 */
	const AMeshComponent3D* MeshOf( ANode* Node ) noexcept
	{
		return Node != nullptr ? Node->GetComponent<AMeshComponent3D>() : nullptr;
	}

	/** 水面ノードから水面設定を取り出す。 */
	const AWaterSurface3DComponent* WaterOf( ANode* Node ) noexcept
	{
		return Node != nullptr ? Node->GetComponent<AWaterSurface3DComponent>() : nullptr;
	}
}


void RunWater3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FWater3DSpawnParams / 既定で水面を置ける" );

	{
		const FWater3DSpawnParams Params;
		Harness.Check( Params.IsValid(), "既定値は有効" );
		Harness.CheckEqualF32( Params.Size.x, 4.0f, "見える広さを持つ" );
		Harness.Check( Params.Name == FStringView( "Water" ), "見つけやすい名前を持つ" );
	}

	Harness.BeginSuite( "FWater3DSpawnParams / 壊れた数値を置く前に弾く" );

	{
		FWater3DSpawnParams ZeroSize;
		ZeroSize.Size.x = 0.0f;
		Harness.Check( !ZeroSize.IsValid(), "広さ0を弾く" );

		FWater3DSpawnParams BrokenPosition;
		BrokenPosition.Position.y = std::numeric_limits<f32>::quiet_NaN();
		Harness.Check( !BrokenPosition.IsValid(), "有限でない位置を弾く" );

		FWater3DSpawnParams NoFlow;
		NoFlow.FlowDirection = FVec2{ 0.0f, 0.0f };
		Harness.Check( !NoFlow.IsValid(), "向きのない流れを弾く" );

		FWater3DSpawnParams BrokenRoughness;
		BrokenRoughness.Roughness = 1.1f;
		Harness.Check( !BrokenRoughness.IsValid(), "範囲外の粗さを弾く" );

		FWater3DSpawnParams BrokenLifetime;
		BrokenLifetime.RippleLifetime = -1.0f;
		Harness.Check( !BrokenLifetime.IsValid(), "負の波紋寿命を弾く" );
	}

	Harness.BeginSuite( "CWater3DSpawner / 描画と波紋に使える水面を置く" );

	{
		CSceneNodeGraph Graph;
		FWater3DSpawnParams Params;
		Params.Position = FVec3{ 2.0f, 0.25f, -3.0f };
		Params.Size = FVec2{ 6.0f, 3.5f };
		Params.ShallowColor = FVec3{ 0.10f, 0.55f, 0.72f };
		Params.DeepColor = FVec3{ 0.01f, 0.08f, 0.22f };
		Params.WaveAmplitude = 0.18f;
		Params.RippleLifetime = 5.5f;
		Params.Name = FStringView( "Pool" );

		ANode* const Placed = CWater3DSpawner::SpawnInto( Graph, Params );
		Harness.Check( Placed != nullptr, "置ける" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 1u, "ルートの下に付く" );

		if ( Placed != nullptr )
		{
			Harness.Check( Placed->Id().IsValid(), "波紋へ渡せる識別子が付く" );
			Harness.Check( Graph.Get( Placed->Id() ) == Placed, "識別子から取り出せる" );
			Harness.CheckEqualF32( Placed->Local().position.y, 0.25f, "高さが入る" );
			Harness.CheckEqualF32( Placed->Local().scale.x, 6.0f, "X方向の広さが入る" );
			Harness.CheckEqualF32( Placed->Local().scale.z, 3.5f, "Z方向の広さが入る" );
			Harness.Check( Placed->Name() == FStringView( "Pool" ), "名前が入る" );
		}

		const AMeshComponent3D* const Mesh = MeshOf( Placed );
		Harness.Check( Mesh != nullptr, "描く平面が付く" );
		if ( Mesh != nullptr )
		{
			Harness.Check( Mesh->Primitive() == EMeshPrimitive3D::Plane, "XZ平面を使う" );
			Harness.Check( !Mesh->CastsShadow(), "水面自身は影を落とさない" );
		}

		const AWaterSurface3DComponent* const Water = WaterOf( Placed );
		Harness.Check( Water != nullptr, "ACSの水面部品が付く" );
		if ( Water != nullptr )
		{
			Harness.CheckEqualF32( Water->shallowColor.y, 0.55f, "浅い色が入る" );
			Harness.CheckEqualF32( Water->deepColor.z, 0.22f, "深い色が入る" );
			Harness.CheckEqualF32( Water->waveAmplitude, 0.18f, "波の高さが入る" );
			Harness.CheckEqualF32( Water->rippleLifetime, 5.5f, "波紋寿命が入る" );
		}
	}

	Harness.BeginSuite( "CWater3DSpawner / 置けないときは登録を残さない" );

	{
		CSceneNodeGraph Graph;
		FWater3DSpawnParams Broken;
		Broken.OpticalDepth = 0.0f;

		Harness.Check( CWater3DSpawner::SpawnInto( Graph, Broken ) == nullptr, "置けない" );
		Harness.CheckEqualU64( Graph.Root().ChildCount(), 0u, "半端な子が残らない" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "識別子の空きも消費しない" );
	}
}
