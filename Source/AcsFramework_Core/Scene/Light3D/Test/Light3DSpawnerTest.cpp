// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/Light3DSpawner.h"
#include "Common/Test/TestHarness.h"

#include <limits>

namespace
{
	/** 置いたノードから光の部品を取り出す。 */
	ALightComponent3D* LightOf( ANode* Node ) noexcept
	{
		return Node != nullptr ? Node->GetComponent<ALightComponent3D>() : nullptr;
	}

	/** 浮動小数の小さな誤差を許して比較する。 */
	void CheckNear( CTestHarness& Harness, f32 Actual, f32 Expected, const char* Label ) noexcept
	{
		constexpr f32 kTolerance = 0.0001f;
		const f32 Difference = Actual > Expected ? Actual - Expected : Expected - Actual;
		Harness.Check( Difference < kTolerance, Label );
	}

	/** 方向の3成分を誤差付きで比較する。 */
	void CheckDirection( CTestHarness& Harness, FVec3 Actual, FVec3 Expected, const char* Label ) noexcept
	{
		constexpr f32 kTolerance = 0.0001f;
		const bool bNear = LengthSq( Actual - Expected ) < kTolerance * kTolerance;
		Harness.Check( bNear, Label );
	}
}


void RunLight3DSpawnerTests( CTestHarness& Harness )
{
	Harness.BeginSuite( "FLight3DSpawnParams / 種類ごとの短い指定を作る" );

	{
		const FLight3DSpawnParams Defaults;
		Harness.Check( Defaults.IsValid(), "既定値だけで太陽になる" );
		Harness.Check( Defaults.Kind == ELight3DKind::Directional, "既定は平行光" );
		Harness.Check( Defaults.Intensity > 1.0f, "既定の太陽は見た目に十分な強さ" );

		const FLight3DSpawnParams Sun = FLight3DSpawnParams::Sun( FVec3{ 1.0f, 2.0f, 3.0f } );
		Harness.Check( Sun.IsValid() && Sun.Kind == ELight3DKind::Directional, "方向だけで太陽を作れる" );
		Harness.Check( Sun.Name == FStringView( "Sun" ), "太陽の名前が付く" );

		const FLight3DSpawnParams Point = FLight3DSpawnParams::Point( FVec3{ 1.0f, 2.0f, 3.0f }, 8.0f );
		Harness.Check( Point.IsValid() && Point.Kind == ELight3DKind::Point, "位置と距離で点光源を作れる" );
		CheckNear( Harness, Point.Range, 8.0f, "点光源の距離が入る" );
	}

	Harness.BeginSuite( "CLight3DSpawner / 太陽の方向を回転へ変換して置く" );

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();
		FLight3DSpawnParams Params = FLight3DSpawnParams::Sun( FVec3{ 2.0f, 0.0f, 0.0f }, FVec3{ 0.8f, 0.9f, 1.1f }, 2.0f );
		Params.Name = FStringView( "KeyLight" );

		ANode* const Placed = CLight3DSpawner::SpawnInto( *Parent, Params );
		ALightComponent3D* const Light = LightOf( Placed );
		Harness.Check( Placed != nullptr && Light != nullptr, "ノードと光の部品を1回で置ける" );
		Harness.CheckEqualU64( Parent->ChildCount(), 1u, "親へ1灯だけ追加する" );

		if ( Placed != nullptr && Light != nullptr )
		{
			Harness.Check( Placed->Name() == FStringView( "KeyLight" ), "指定した名前が付く" );
			Harness.Check( Light->LightKind() == ELight3DKind::Directional, "平行光として設定する" );
			CheckDirection( Harness, Light->WorldDirection(), FVec3{ 1.0f, 0.0f, 0.0f }, "長さを除いて指定方向へ向ける" );

			FDirLight Output{};
			Harness.Check( Light->FillDirectional( Output ), "描画へ渡す平行光を作れる" );
			CheckNear( Harness, Output.color.x, 1.6f, "色へ強さを掛ける" );
			CheckNear( Harness, Output.color.z, 2.2f, "HDR色も上限で切らない" );
		}
	}

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();
		const FLight3DSpawnParams Params;
		ANode* const Placed = CLight3DSpawner::SpawnInto( *Parent, Params );
		ALightComponent3D* const Light = LightOf( Placed );
		Harness.Check( Light != nullptr, "既定の斜め方向も置ける" );
		if ( Light != nullptr ) CheckDirection( Harness, Light->WorldDirection(), Normalize( Params.DirectionToLight ), "3軸を含む方向をそのまま再現する" );
	}

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();
		ANode* const Placed = CLight3DSpawner::SpawnInto( *Parent, FLight3DSpawnParams::Sun( FVec3{ 0.0f, -2.0f, 0.0f } ) );
		ALightComponent3D* const Light = LightOf( Placed );
		Harness.Check( Light != nullptr, "真下の太陽も特異点で失敗しない" );
		if ( Light != nullptr ) CheckDirection( Harness, Light->WorldDirection(), FVec3{ 0.0f, -1.0f, 0.0f }, "180度回転を正しく作る" );
	}

	Harness.BeginSuite( "CLight3DSpawner / 点光源を識別子付きで置く" );

	{
		CSceneNodeGraph Graph;
		FLight3DSpawnParams Params = FLight3DSpawnParams::Point( FVec3{ 1.0f, 2.0f, 3.0f }, 7.5f, FVec3{ 0.5f, 0.25f, 0.1f }, 4.0f );
		ANode* const Placed = CLight3DSpawner::SpawnInto( Graph, Params );
		ALightComponent3D* const Light = LightOf( Placed );

		Harness.Check( Placed != nullptr && Placed->Id().IsValid(), "識別子付きノードを置ける" );
		Harness.Check( Placed != nullptr && Graph.Get( Placed->Id() ) == Placed, "識別子から同じ光を取れる" );
		Harness.Check( Light != nullptr && Light->LightKind() == ELight3DKind::Point, "点光源の部品を付ける" );

		if ( Light != nullptr )
		{
			FPointLight Output{};
			Harness.Check( Light->FillPoint( Output ), "描画へ渡す点光源を作れる" );
			CheckDirection( Harness, Output.position, FVec3{ 1.0f, 2.0f, 3.0f }, "位置をノードへ入れる" );
			CheckNear( Harness, Output.range, 7.5f, "到達距離を部品へ入れる" );
			CheckNear( Harness, Output.color.y, 1.0f, "点光源の色へ強さを掛ける" );
		}
	}

	Harness.BeginSuite( "CLight3DSpawner / 不正な光を半端に残さない" );

	{
		const f32 Infinity = std::numeric_limits<f32>::infinity();
		const f32 QuietNaN = std::numeric_limits<f32>::quiet_NaN();

		FLight3DSpawnParams ZeroDirection = FLight3DSpawnParams::Sun( FVec3{} );
		FLight3DSpawnParams NegativeColor = FLight3DSpawnParams::Sun( FVec3{ 0.0f, 1.0f, 0.0f } );
		NegativeColor.Color.x = -0.1f;
		FLight3DSpawnParams InfiniteIntensity = FLight3DSpawnParams::Sun( FVec3{ 0.0f, 1.0f, 0.0f } );
		InfiniteIntensity.Intensity = Infinity;
		FLight3DSpawnParams OverflowColor = FLight3DSpawnParams::Sun( FVec3{ 0.0f, 1.0f, 0.0f } );
		OverflowColor.Color.x = std::numeric_limits<f32>::max();
		OverflowColor.Intensity = 2.0f;
		FLight3DSpawnParams OverflowDirection = FLight3DSpawnParams::Sun( FVec3{ std::numeric_limits<f32>::max(), 1.0f, 0.0f } );
		FLight3DSpawnParams BrokenPoint = FLight3DSpawnParams::Point( FVec3{ QuietNaN, 0.0f, 0.0f }, 0.0f );
		FLight3DSpawnParams UnknownKind;
		UnknownKind.Kind = static_cast<ELight3DKind>( 255u );

		Harness.Check( !ZeroDirection.IsValid(), "長さ0の太陽方向を弾く" );
		Harness.Check( !NegativeColor.IsValid(), "負の色を弾く" );
		Harness.Check( !InfiniteIntensity.IsValid(), "有限でない強さを弾く" );
		Harness.Check( !OverflowColor.IsValid(), "色と強さの積が溢れる指定を弾く" );
		Harness.Check( !OverflowDirection.IsValid(), "長さ計算が溢れる方向を弾く" );
		Harness.Check( !BrokenPoint.IsValid(), "有限でない位置と0距離を弾く" );
		Harness.Check( !UnknownKind.IsValid(), "未知の種類を弾く" );

		TObjectPtr<ANode> Parent = NewObject<ANode>();
		Harness.Check( CLight3DSpawner::SpawnInto( *Parent, ZeroDirection ) == nullptr, "親ノードへの不正配置を拒否する" );
		Harness.CheckEqualU64( Parent->ChildCount(), 0u, "親へ半端なノードを残さない" );

		CSceneNodeGraph Graph;
		Harness.Check( CLight3DSpawner::SpawnInto( Graph, BrokenPoint ) == nullptr, "識別子付きの不正配置を拒否する" );
		Harness.CheckEqualU64( Graph.RegisteredCount(), 1u, "識別子の空きも消費しない" );
	}

	{
		TObjectPtr<ANode> Parent = NewObject<ANode>();
		FLight3DSpawnParams Off = FLight3DSpawnParams::Point( FVec3{}, 5.0f );
		Off.Intensity = 0.0f;
		ANode* const Placed = CLight3DSpawner::SpawnInto( *Parent, Off );
		ALightComponent3D* const Light = LightOf( Placed );
		Harness.Check( Light != nullptr && !Light->IsEmitting(), "強さ0は配置したまま消灯できる" );
	}
}
