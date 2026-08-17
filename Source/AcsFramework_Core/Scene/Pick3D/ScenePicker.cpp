// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Pick3D/ScenePicker.h"

namespace
{
	/**
	 * 手前から順に並ぶよう、当たった記録を差し込む。
	 *
	 * @param Hits 並んでいる配列。
	 * @param Hit 差し込むもの。
	 * @return 入れられたら true。
	 */
	bool InsertByDistance( TArray<FSceneRayHit>& Hits, const FSceneRayHit& Hit ) noexcept
	{
		usize Position = 0u;
		while ( Position < Hits.Num() && Hits[Position].Distance <= Hit.Distance ) ++Position;

		if ( !Hits.TryAdd( Hit ) ) return false;

		// 末尾に入ったものを、正しい位置まで前へ送る。数が少ない前提の素直なやり方。
		for ( usize Index = Hits.Num() - 1u; Index > Position; --Index )
		{
			const FSceneRayHit Temporary = Hits[Index];
			Hits[Index] = Hits[Index - 1u];
			Hits[Index - 1u] = Temporary;
		}
		return true;
	}
}


bool CScenePicker::WorldBounds( ANode& Node, FAabb3& OutBox ) noexcept
{
	const AMeshComponent3D* const Mesh = Node.GetComponent<AMeshComponent3D>();
	if ( Mesh == nullptr ) return false;

	FVec3 LocalMinimum;
	FVec3 LocalMaximum;
	Mesh->LocalBounds( LocalMinimum, LocalMaximum );

	// 8 隅を世界へ移してから包み直す。回転していると実際より大きな箱になるが、
	// **取りこぼすよりは大きめに拾う方がまし** (掴めないより、掴みすぎの方が気付ける)。
	const FMat4 World = Node.World().ToMat4();
	FVec3 Minimum{ 3.4028235e38f, 3.4028235e38f, 3.4028235e38f };
	FVec3 Maximum{ -3.4028235e38f, -3.4028235e38f, -3.4028235e38f };

	for ( u32 Corner = 0u; Corner < 8u; ++Corner )
	{
		const FVec3 Local
		{
			( Corner & 1u ) != 0u ? LocalMaximum.x : LocalMinimum.x,
			( Corner & 2u ) != 0u ? LocalMaximum.y : LocalMinimum.y,
			( Corner & 4u ) != 0u ? LocalMaximum.z : LocalMinimum.z,
		};
		const FVec3 Value = TransformPoint( Local, World );

		if ( Value.x < Minimum.x ) Minimum.x = Value.x;
		if ( Value.y < Minimum.y ) Minimum.y = Value.y;
		if ( Value.z < Minimum.z ) Minimum.z = Value.z;
		if ( Value.x > Maximum.x ) Maximum.x = Value.x;
		if ( Value.y > Maximum.y ) Maximum.y = Value.y;
		if ( Value.z > Maximum.z ) Maximum.z = Value.z;
	}

	// 板は厚みが 0 なので、そのままだと線が «面の中» を通っても当たらないことがある。
	// ごく薄い厚みを与えて、平らなものも掴めるようにする。
	constexpr f32 kMinimumThickness = 1.0e-4f;
	if ( Maximum.x - Minimum.x < kMinimumThickness ) { Minimum.x -= kMinimumThickness; Maximum.x += kMinimumThickness; }
	if ( Maximum.y - Minimum.y < kMinimumThickness ) { Minimum.y -= kMinimumThickness; Maximum.y += kMinimumThickness; }
	if ( Maximum.z - Minimum.z < kMinimumThickness ) { Minimum.z -= kMinimumThickness; Maximum.z += kMinimumThickness; }

	OutBox = FAabb3::FromMinMax( Minimum, Maximum );
	return true;
}


bool CScenePicker::HitNode( ANode& Node, const FSceneRay& Ray, FSceneRayHit& OutHit ) noexcept
{
	FAabb3 Box;
	if ( !WorldBounds( Node, Box ) ) return false;

	const FRayHit3 Hit = RaycastAabb( Ray.ToRay3(), Box, Ray.MaxDistance );
	if ( !Hit.hit ) return false;

	OutHit.Node = &Node;
	OutHit.Distance = Hit.t;
	OutHit.Point = Hit.point;
	OutHit.Normal = Hit.normal;
	return true;
}


FSceneRayHit CScenePicker::Raycast( ANode& Root, const FSceneRay& Ray ) noexcept
{
	FSceneRayHit Nearest;
	if ( !Ray.IsValid() ) return Nearest;

	TArray<ANode*> Stack;
	if ( !Stack.TryAdd( &Root ) ) return Nearest;

	while ( !Stack.IsEmpty() )
	{
		ANode* const Node = Stack.Last();
		Stack.Pop();
		if ( Node == nullptr ) continue;

		// 消したものを掴めてしまうのを防ぐ。子ごと飛ばす。
		if ( !Node->IsVisible() || !Node->IsEnabled() ) continue;

		FSceneRayHit Hit;
		if ( HitNode( *Node, Ray, Hit ) )
		{
			if ( !Nearest.IsHit() || Hit.Distance < Nearest.Distance ) Nearest = Hit;
		}

		for ( u32 Index = 0u; Index < Node->ChildCount(); ++Index )
		{
			if ( !Stack.TryAdd( Node->Child( Index ) ) ) break;
		}
	}

	return Nearest;
}


usize CScenePicker::RaycastAll( ANode& Root, const FSceneRay& Ray, TArray<FSceneRayHit>& OutHits ) noexcept
{
	const usize Before = OutHits.Num();
	if ( !Ray.IsValid() ) return 0u;

	TArray<ANode*> Stack;
	if ( !Stack.TryAdd( &Root ) ) return 0u;

	while ( !Stack.IsEmpty() )
	{
		ANode* const Node = Stack.Last();
		Stack.Pop();
		if ( Node == nullptr ) continue;
		if ( !Node->IsVisible() || !Node->IsEnabled() ) continue;

		FSceneRayHit Hit;
		if ( HitNode( *Node, Ray, Hit ) )
		{
			if ( !InsertByDistance( OutHits, Hit ) ) return OutHits.Num() - Before;
		}

		for ( u32 Index = 0u; Index < Node->ChildCount(); ++Index )
		{
			if ( !Stack.TryAdd( Node->Child( Index ) ) ) break;
		}
	}

	return OutHits.Num() - Before;
}
