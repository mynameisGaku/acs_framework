// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Light3D/Light3DSpawner.h"

namespace
{
	/** 反対向き判定と単位回転判定に使う余裕。 */
	constexpr f32 kDirectionEpsilon = 0.000001f;

	/** 真下へ向ける180度回転のラジアン値。 */
	constexpr f32 kHalfTurnRadians = 3.14159265358979323846f;
}


ANode* CLight3DSpawner::SpawnInto( CSceneNodeGraph& Graph, const FLight3DSpawnParams& Params, ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return nullptr;

	const FScene3DSpawnResult Spawned = Graph.TrySpawn( Params.Name, Parent );
	if ( !Spawned ) return nullptr;

	ApplyTransform_Internal( *Spawned.Node, Params );
	ApplyLight_Internal( *Spawned.Node, Params );
	return Spawned.Node;
}


ANode* CLight3DSpawner::SpawnInto( ANode& Parent, const FLight3DSpawnParams& Params ) noexcept
{
	if ( !Params.IsValid() ) return nullptr;

	TObjectPtr<ANode> Node = NewObject<ANode>();
	if ( !Node ) return nullptr;

	if ( Params.Name.Data() != nullptr && Params.Name.Size() != 0u ) Node->SetName( Params.Name );
	ApplyTransform_Internal( *Node, Params );
	ApplyLight_Internal( *Node, Params );

	ANode* const Placed = Node.Get();
	Parent.AddChild( Move( Node ) );
	return Placed;
}


FQuat CLight3DSpawner::DirectionRotation_Internal( FVec3 DirectionToLight ) noexcept
{
	const FVec3 Up{ 0.0f, 1.0f, 0.0f };
	const FVec3 Target = Normalize( DirectionToLight );
	f32 Cosine = Dot( Up, Target );
	if ( Cosine > 1.0f ) Cosine = 1.0f;
	if ( Cosine < -1.0f ) Cosine = -1.0f;

	if ( Cosine >= 1.0f - kDirectionEpsilon ) return FQuat::Identity();
	if ( Cosine <= -1.0f + kDirectionEpsilon ) return FQuat::AxisAngle( FVec3{ 1.0f, 0.0f, 0.0f }, kHalfTurnRadians );

	const FVec3 Axis = Cross( Up, Target );
	return Normalize( FQuat{ Axis.x, Axis.y, Axis.z, 1.0f + Cosine } );
}


void CLight3DSpawner::ApplyTransform_Internal( ANode& Node, const FLight3DSpawnParams& Params ) noexcept
{
	if ( Params.Kind == ELight3DKind::Point )
	{
		Node.Local().position = Params.Position;
		return;
	}

	Node.Local().rotation = DirectionRotation_Internal( Params.DirectionToLight );
}


void CLight3DSpawner::ApplyLight_Internal( ANode& Node, const FLight3DSpawnParams& Params ) noexcept
{
	ALightComponent3D& Light = Node.AddComponent<ALightComponent3D>();
	Light.SetLightKind( Params.Kind );
	Light.SetColor( Params.Color );
	Light.SetIntensity( Params.Intensity );
	if ( Params.Kind == ELight3DKind::Point ) Light.SetRange( Params.Range );
}
