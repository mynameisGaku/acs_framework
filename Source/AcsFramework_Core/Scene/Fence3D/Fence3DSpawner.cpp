// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Fence3D/Fence3DSpawner.h"

#include "AcsFramework_Core/Scene/Block3D/Block3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

FFence3DSpawnResult CFence3DSpawner::SpawnInto( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FFence3DSpawnParams& Params,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};

	// 最大間隔を守るために必要な区間数。
	const u32 SectionCount = Params.RequiredSectionCount();
	// 両端を含む支柱数。
	const u32 PostCount = SectionCount + 1u;
	// 隣り合う支柱中心の実間隔。
	const f32 PostSpacing = Params.Length / static_cast<f32>( SectionCount );
	// 始点から終点へ進む各軸の符号。
	f32 DirectionX = 0.0f;
	f32 DirectionZ = 0.0f;
	switch ( Params.Direction )
	{
	case EFence3DDirection::PositiveX:
		DirectionX = 1.0f;
		break;
	case EFence3DDirection::NegativeX:
		DirectionX = -1.0f;
		break;
	case EFence3DDirection::PositiveZ:
		DirectionZ = 1.0f;
		break;
	case EFence3DDirection::NegativeZ:
		DirectionZ = -1.0f;
		break;
	default:
		return {};
	}

	// 全配列領域を生成前に確保し、途中の配列拡張失敗を避ける。
	FFence3DSpawnResult Fence;
	if ( !Fence.Posts.TryReserve( static_cast<usize>( PostCount ) )
		|| !Fence.Rails.TryReserve( static_cast<usize>( Params.RailCount ) ) ) return {};

	// 全支柱で見た目と衝突設定を共有し、位置だけを差し替える。
	FBlock3DSpawnParams PartParams = FBlock3DSpawnParams::FromSize(
		FVec3{ Params.PostThickness, Params.Height, Params.PostThickness } );
	PartParams.Color = Params.Color;
	PartParams.Metallic = Params.Metallic;
	PartParams.Roughness = Params.Roughness;
	PartParams.bCastsShadow = Params.bCastsShadow;
	PartParams.CollisionLayer = Params.CollisionLayer;
	PartParams.Name = Params.PostName;

	for ( u32 Index = 0u; Index < PostCount; ++Index )
	{
		// 始点から現在支柱までの軸方向距離。
		const f32 ForwardOffset = PostSpacing * static_cast<f32>( Index );
		PartParams.Position = FVec3{
			Params.StartPostBottomCenter.x + DirectionX * ForwardOffset,
			Params.StartPostBottomCenter.y + Params.Height * 0.5f,
			Params.StartPostBottomCenter.z + DirectionZ * ForwardOffset,
		};
		FCollidableModel3DSpawnResult Post = CBlock3DSpawner::SpawnInto(
			Graph, Collision, PartParams, Parent );
		if ( !Post )
		{
			Rollback_Internal( Graph, Collision, Fence );
			return {};
		}
		if ( !Fence.Posts.TryAdd( Post ) )
		{
			(void)CModel3DSpawner::DestroyCollidable( Graph, Collision, Post );
			Rollback_Internal( Graph, Collision, Fence );
			return {};
		}
	}

	// 横桟は両端支柱の中心間を繋ぎ、柵面に直交する厚みを保つ。
	PartParams.Size = Params.Direction == EFence3DDirection::PositiveX
		|| Params.Direction == EFence3DDirection::NegativeX
		? FVec3{ Params.Length, Params.RailHeight, Params.RailThickness }
		: FVec3{ Params.RailThickness, Params.RailHeight, Params.Length };
	PartParams.Position.x = Params.StartPostBottomCenter.x + DirectionX * Params.Length * 0.5f;
	PartParams.Position.z = Params.StartPostBottomCenter.z + DirectionZ * Params.Length * 0.5f;
	PartParams.Name = Params.RailName;
	for ( u32 Index = 0u; Index < Params.RailCount; ++Index )
	{
		// 底面と上端の間へ等分した、下から数える現在横桟の高さ倍率。
		const f32 Ordinal = static_cast<f32>( Index + 1u );
		PartParams.Position.y = Params.StartPostBottomCenter.y
			+ Params.Height * Ordinal / static_cast<f32>( Params.RailCount + 1u );
		FCollidableModel3DSpawnResult Rail = CBlock3DSpawner::SpawnInto(
			Graph, Collision, PartParams, Parent );
		if ( !Rail )
		{
			Rollback_Internal( Graph, Collision, Fence );
			return {};
		}
		if ( !Fence.Rails.TryAdd( Rail ) )
		{
			(void)CModel3DSpawner::DestroyCollidable( Graph, Collision, Rail );
			Rollback_Internal( Graph, Collision, Fence );
			return {};
		}
	}
	return Fence;
}


bool CFence3DSpawner::Destroy( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FFence3DSpawnResult& Fence ) noexcept
{
	if ( !CanDestroy_Internal( Graph, Collision, Fence ) ) return false;
	// 生成と逆の順で上側横桟から始点支柱へ片付ける。
	for ( usize Remaining = Fence.Rails.Num(); Remaining > 0u; --Remaining )
	{
		if ( !CModel3DSpawner::DestroyCollidable(
			Graph, Collision, Fence.Rails[Remaining - 1u] ) ) return false;
	}
	for ( usize Remaining = Fence.Posts.Num(); Remaining > 0u; --Remaining )
	{
		if ( !CModel3DSpawner::DestroyCollidable(
			Graph, Collision, Fence.Posts[Remaining - 1u] ) ) return false;
	}
	Fence.Rails.Reset();
	Fence.Posts.Reset();
	return true;
}


bool CFence3DSpawner::IsOwnedPart_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FCollidableModel3DSpawnResult& Part ) noexcept
{
	if ( !Part || Part.Node == &Graph.Root() ) return false;
	// ポインターだけでなく、この場面が発行した生存中の識別子でも所有を確かめる。
	const FNodeId NodeId = Graph.IdOf( Part.Node );
	return NodeId.IsValid() && Graph.Get( NodeId ) == Part.Node
		&& Collision.IsRegisteredTo( Part.Shape, *Part.Node );
}


const FCollidableModel3DSpawnResult* CFence3DSpawner::PartAt_Internal(
	const FFence3DSpawnResult& Fence, usize Index ) noexcept
{
	if ( Index < Fence.Posts.Num() ) return &Fence.Posts[Index];
	Index -= Fence.Posts.Num();
	return Index < Fence.Rails.Num() ? &Fence.Rails[Index] : nullptr;
}


bool CFence3DSpawner::CanDestroy_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, const FFence3DSpawnResult& Fence ) noexcept
{
	if ( !Fence ) return false;
	// 支柱の後ろへ横桟を繋いだ論理配列の全要素数。
	const usize PartCount = Fence.Posts.Num() + Fence.Rails.Num();
	for ( usize Index = 0u; Index < PartCount; ++Index )
	{
		const FCollidableModel3DSpawnResult* const Part = PartAt_Internal( Fence, Index );
		if ( Part == nullptr || !IsOwnedPart_Internal( Graph, Collision, *Part ) ) return false;
		for ( usize OtherIndex = Index + 1u; OtherIndex < PartCount; ++OtherIndex )
		{
			const FCollidableModel3DSpawnResult* const Other = PartAt_Internal(
				Fence, OtherIndex );
			if ( Other == nullptr || Part->Node == Other->Node
				|| Part->Shape == Other->Shape ) return false;
		}
	}
	return true;
}


void CFence3DSpawner::Rollback_Internal( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, FFence3DSpawnResult& Fence ) noexcept
{
	for ( usize Remaining = Fence.Rails.Num(); Remaining > 0u; --Remaining )
	{
		(void)CModel3DSpawner::DestroyCollidable(
			Graph, Collision, Fence.Rails[Remaining - 1u] );
	}
	for ( usize Remaining = Fence.Posts.Num(); Remaining > 0u; --Remaining )
	{
		(void)CModel3DSpawner::DestroyCollidable(
			Graph, Collision, Fence.Posts[Remaining - 1u] );
	}
	Fence.Rails.Reset();
	Fence.Posts.Reset();
}
