// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Stairs3D/Stairs3DSpawner.h"

#include "AcsFramework_Core/Scene/Block3D/Block3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

FStairs3DSpawnResult CStairs3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FStairs3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};

	// 全段の結果領域を先に確保し、途中の配列拡張失敗を避ける。
	FStairs3DSpawnResult Stairs;
	if ( !Stairs.Steps.TryReserve( static_cast<usize>( Params.StepCount ) ) ) return {};

	for ( u32 Index = 0u; Index < Params.StepCount; ++Index )
	{
		// 低い側を1とする現在段の高さ倍率。
		const f32 Ordinal = static_cast<f32>( Index + 1u );
		// 全段の下端を同じ床面へ揃える現在直方体の高さ。
		const f32 Height = Params.StepHeight * Ordinal;
		// 基準端から現在踏面中心までの上る方向の距離。
		const f32 ForwardOffset = Params.StepDepth * ( static_cast<f32>( Index ) + 0.5f );

		// Z方向へ上る既定形状。X方向では幅と奥行きを入れ替える。
		FVec3 Size{ Params.Width, Height, Params.StepDepth };
		// 下端を床面へ揃えた現在段の中心位置。
		FVec3 Position = Params.BottomEdgeCenter;
		Position.y += Height * 0.5f;
		switch ( Params.Direction )
		{
		case EStairs3DDirection::PositiveX:
			Size = FVec3{ Params.StepDepth, Height, Params.Width };
			Position.x += ForwardOffset;
			break;
		case EStairs3DDirection::NegativeX:
			Size = FVec3{ Params.StepDepth, Height, Params.Width };
			Position.x -= ForwardOffset;
			break;
		case EStairs3DDirection::PositiveZ:
			Position.z += ForwardOffset;
			break;
		case EStairs3DDirection::NegativeZ:
			Position.z -= ForwardOffset;
			break;
		default:
			Rollback_Internal( Graph, Collision, Stairs );
			return {};
		}

		// 全段で見た目と衝突レイヤーを共有する直方体設定。
		FBlock3DSpawnParams StepParams = FBlock3DSpawnParams::FromSize( Size, Position );
		StepParams.Color = Params.Color;
		StepParams.Metallic = Params.Metallic;
		StepParams.Roughness = Params.Roughness;
		StepParams.bCastsShadow = Params.bCastsShadow;
		StepParams.CollisionLayer = Params.CollisionLayer;
		StepParams.Name = Params.StepName;

		// 現在段の表示と箱型衝突を同じ寸法で作る。
		FCollidableModel3DSpawnResult Step = CBlock3DSpawner::SpawnInto(
			Graph, Collision, StepParams, Parent );
		if ( !Step )
		{
			Rollback_Internal( Graph, Collision, Stairs );
			return {};
		}
		if ( !Stairs.Steps.TryAdd( Step ) )
		{
			(void)CModel3DSpawner::DestroyCollidable( Graph, Collision, Step );
			Rollback_Internal( Graph, Collision, Stairs );
			return {};
		}
	}
	return Stairs;
}


bool CStairs3DSpawner::Destroy( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FStairs3DSpawnResult& Stairs ) noexcept
{
	if ( !CanDestroy_Internal( Graph, Collision, Stairs ) ) return false;
	for ( usize Remaining = Stairs.Steps.Num(); Remaining > 0u; --Remaining )
	{
		if ( !CModel3DSpawner::DestroyCollidable(
			Graph, Collision, Stairs.Steps[Remaining - 1u] ) ) return false;
	}
	Stairs.Steps.Reset();
	return true;
}


bool CStairs3DSpawner::IsOwnedStep_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FCollidableModel3DSpawnResult& Step ) noexcept
{
	if ( !Step || Step.Node == &Graph.Root() ) return false;
	// ポインターだけでなく、この場面が発行した生存中の識別子でも所有を確かめる。
	const FNodeId NodeId = Graph.IdOf( Step.Node );
	return NodeId.IsValid() && Graph.Get( NodeId ) == Step.Node
		&& Collision.IsRegisteredTo( Step.Shape, *Step.Node );
}


bool CStairs3DSpawner::CanDestroy_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FStairs3DSpawnResult& Stairs ) noexcept
{
	if ( !Stairs ) return false;
	for ( usize Index = 0u; Index < Stairs.Steps.Num(); ++Index )
	{
		if ( !IsOwnedStep_Internal( Graph, Collision, Stairs.Steps[Index] ) ) return false;
		for ( usize OtherIndex = Index + 1u; OtherIndex < Stairs.Steps.Num(); ++OtherIndex )
		{
			if ( Stairs.Steps[Index].Node == Stairs.Steps[OtherIndex].Node
				|| Stairs.Steps[Index].Shape == Stairs.Steps[OtherIndex].Shape ) return false;
		}
	}
	return true;
}


void CStairs3DSpawner::Rollback_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FStairs3DSpawnResult& Stairs ) noexcept
{
	for ( usize Remaining = Stairs.Steps.Num(); Remaining > 0u; --Remaining )
	{
		(void)CModel3DSpawner::DestroyCollidable(
			Graph, Collision, Stairs.Steps[Remaining - 1u] );
	}
	Stairs.Steps.Reset();
}
