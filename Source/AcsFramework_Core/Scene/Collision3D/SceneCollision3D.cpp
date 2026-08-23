// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"

#include <cmath>


CSceneCollision3D::CSceneCollision3D( CSceneNodeGraph& Graph ) noexcept
	: m_Graph( &Graph )
	, m_RootIdentity( Graph.HasRoot() ? &Graph.Root() : nullptr )
{
}


bool CSceneCollision3D::TryMakeWorldBox( const ANode& Node, FVec3 LocalCenter,
	FVec3 LocalHalfSize, FAabb3& OutBox ) noexcept
{
	return TryMakeWorldBox_Internal( Node, LocalCenter, LocalHalfSize, OutBox );
}


bool CSceneCollision3D::TryMakeWorldSphere( const ANode& Node, FVec3 LocalCenter,
	f32 LocalRadius, FSphere& OutSphere ) noexcept
{
	return TryMakeWorldSphere_Internal( Node, LocalCenter, LocalRadius, OutSphere );
}


FCollisionShapeId3D CSceneCollision3D::TryAdd( ANode& Node,
	const FCollisionShape3DParams& Params ) noexcept
{
	switch ( Params.Kind )
	{
	case FCollisionShape3DParams::EKind::Bounds:
		return TryAddBounds( Node, Params.Layer );
	case FCollisionShape3DParams::EKind::Box:
		return TryAddBox( Node, Params.LocalCenter, Params.LocalHalfSize, Params.Layer );
	case FCollisionShape3DParams::EKind::Sphere:
		return TryAddSphere( Node, Params.LocalCenter, Params.LocalRadius, Params.Layer );
	default:
		return {};
	}
}


FCollisionShapeId3D CSceneCollision3D::TryAddBounds( ANode& Node, u32 Layer ) noexcept
{
	FVec3 Minimum;
	FVec3 Maximum;
	bool bIsSphere = false;
	if ( !TryFindLocalBounds_Internal( Node, Minimum, Maximum, bIsSphere ) ) return {};

	const FVec3 Center
	{
		( Minimum.x + Maximum.x ) * 0.5f,
		( Minimum.y + Maximum.y ) * 0.5f,
		( Minimum.z + Maximum.z ) * 0.5f,
	};
	const FVec3 HalfSize
	{
		( Maximum.x - Minimum.x ) * 0.5f,
		( Maximum.y - Minimum.y ) * 0.5f,
		( Maximum.z - Minimum.z ) * 0.5f,
	};

	if ( !bIsSphere ) return TryAddBox( Node, Center, HalfSize, Layer );
	const f32 Radius = HalfSize.x > HalfSize.y
		? ( HalfSize.x > HalfSize.z ? HalfSize.x : HalfSize.z )
		: ( HalfSize.y > HalfSize.z ? HalfSize.y : HalfSize.z );
	return TryAddSphere( Node, Center, Radius, Layer );
}


FCollisionShapeId3D CSceneCollision3D::TryAddBox( ANode& Node, FVec3 LocalCenter,
	FVec3 LocalHalfSize, u32 Layer ) noexcept
{
	return TryAdd_Internal( Node, EShapeKind::Box, LocalCenter, LocalHalfSize, 0.0f, Layer );
}


FCollisionShapeId3D CSceneCollision3D::TryAddSphere( ANode& Node, FVec3 LocalCenter,
	f32 LocalRadius, u32 Layer ) noexcept
{
	return TryAdd_Internal( Node, EShapeKind::Sphere, LocalCenter, FVec3{}, LocalRadius, Layer );
}


bool CSceneCollision3D::TrySetLayer( FCollisionShapeId3D Shape, u32 Layer ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() ) return false;
	FRegistration* const Registration = FindRegistration_Internal( Shape );
	if ( Registration == nullptr || m_Graph == nullptr ) return false;

	ANode* const Node = m_Graph->Get( Registration->Node );
	if ( Node == nullptr || !m_World.IsAlive( Shape ) ) return false;

	const u32 AppliedLayer = IsNodeActive_Internal( *Node ) ? Layer : 0u;
	if ( !m_World.TrySetLayer( Shape, AppliedLayer ) ) return false;
	Registration->Layer = Layer;
	return true;
}


bool CSceneCollision3D::Remove( FCollisionShapeId3D Shape ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() ) return false;
	for ( usize Index = 0u; Index < m_Registrations.Num(); ++Index )
	{
		if ( m_Registrations[Index].Shape != Shape ) continue;
		if ( !m_World.TryRemove( Shape ) ) return false;

		m_Registrations.RemoveAt( Index );
		return true;
	}
	return false;
}


void CSceneCollision3D::Clear() noexcept
{
	m_World.ClearAll();
	m_Registrations.Reset();
}


bool CSceneCollision3D::TryGetWorldShape( FCollisionShapeId3D Shape,
	FWorldCollisionShape3D& OutShape ) noexcept
{
	if ( !Shape.IsValid() || !RefreshGraphIdentity_Internal() || m_Graph == nullptr ) return false;
	FRegistration* const Registration = FindRegistration_Internal( Shape );
	if ( Registration == nullptr || !m_World.IsAlive( Shape ) ) return false;

	ANode* const Node = m_Graph->Get( Registration->Node );
	if ( Node == nullptr ) return false;

	FWorldCollisionShape3D WorldShape;
	WorldShape.Shape = Registration->Shape;
	WorldShape.Node = Registration->Node;
	WorldShape.Layer = Registration->Layer;
	WorldShape.bQueryable = Registration->Layer != 0u && IsNodeActive_Internal( *Node );
	if ( Registration->Kind == EShapeKind::Box )
	{
		WorldShape.Kind = FWorldCollisionShape3D::EKind::Box;
		if ( !TryMakeWorldBox_Internal( *Node, Registration->LocalCenter,
			Registration->LocalHalfSize, WorldShape.Box ) ) return false;
	}
	else
	{
		WorldShape.Kind = FWorldCollisionShape3D::EKind::Sphere;
		if ( !TryMakeWorldSphere_Internal( *Node, Registration->LocalCenter,
			Registration->LocalRadius, WorldShape.Sphere ) ) return false;
	}

	OutShape = WorldShape;
	return true;
}


bool CSceneCollision3D::TryGetWorldShapes(
	TArray<FWorldCollisionShape3D>& OutShapes, u32 Mask ) noexcept
{
	if ( Mask == 0u )
	{
		if ( !RefreshGraphIdentity_Internal() ) return false;
		TArray<FWorldCollisionShape3D> Empty;
		OutShapes = Move( Empty );
		return true;
	}
	if ( !Sync() || m_Graph == nullptr ) return false;

	TArray<FWorldCollisionShape3D> WorldShapes;
	if ( !WorldShapes.TryReserve( m_Registrations.Num() ) ) return false;
	for ( usize Index = 0u; Index < m_Registrations.Num(); ++Index )
	{
		const FRegistration& Registration = m_Registrations[Index];
		if ( ( Registration.Layer & Mask ) == 0u ) continue;

		ANode* const Node = m_Graph->Get( Registration.Node );
		if ( Node == nullptr ) return false;
		if ( !IsNodeActive_Internal( *Node ) ) continue;

		FWorldCollisionShape3D WorldShape;
		if ( !TryGetWorldShape( Registration.Shape, WorldShape )
			|| !WorldShape.bQueryable || !WorldShapes.TryAdd( WorldShape ) ) return false;
	}

	OutShapes = Move( WorldShapes );
	return true;
}


bool CSceneCollision3D::Sync() noexcept
{
	if ( !RefreshGraphIdentity_Internal() ) return false;

	bool bSucceeded = true;
	usize Index = 0u;
	while ( Index < m_Registrations.Num() )
	{
		FRegistration& Registration = m_Registrations[Index];
		ANode* const Node = m_Graph->Get( Registration.Node );
		if ( Node == nullptr || !m_World.IsAlive( Registration.Shape ) )
		{
			if ( m_World.IsAlive( Registration.Shape ) && !m_World.TryRemove( Registration.Shape ) )
				bSucceeded = false;
			m_Registrations.RemoveAt( Index );
			continue;
		}

		if ( !IsNodeActive_Internal( *Node ) )
		{
			if ( !m_World.TrySetLayer( Registration.Shape, 0u ) ) bSucceeded = false;
			++Index;
			continue;
		}

		bool bShapeUpdated = false;
		if ( Registration.Kind == EShapeKind::Box )
		{
			FAabb3 WorldBox;
			bShapeUpdated = TryMakeWorldBox_Internal(
				*Node, Registration.LocalCenter, Registration.LocalHalfSize, WorldBox )
				&& m_World.TryUpdateAabb( Registration.Shape, WorldBox );
		}
		else
		{
			FSphere WorldSphere;
			bShapeUpdated = TryMakeWorldSphere_Internal(
				*Node, Registration.LocalCenter, Registration.LocalRadius, WorldSphere )
				&& m_World.TryUpdateSphere( Registration.Shape, WorldSphere );
		}

		if ( !bShapeUpdated || !m_World.TrySetLayer( Registration.Shape, Registration.Layer ) )
			bSucceeded = false;
		++Index;
	}
	return bSucceeded;
}


bool CSceneCollision3D::TryOverlapBox( const FAabb3& Box, TArray<ANode*>& OutNodes,
	FCollisionShapeId3D Exclude, u32 Mask ) noexcept
{
	if ( !Sync() ) return false;

	TArray<FCollisionShapeId3D> Shapes;
	if ( !m_World.TryOverlapAabb( Box, Shapes, Exclude, Mask ) ) return false;

	TArray<ANode*> Nodes;
	if ( !TryResolveNodes_Internal( Shapes, Nodes ) ) return false;
	OutNodes = Move( Nodes );
	return true;
}


bool CSceneCollision3D::TryOverlapSphere( const FSphere& Sphere, TArray<ANode*>& OutNodes,
	FCollisionShapeId3D Exclude, u32 Mask ) noexcept
{
	if ( !Sync() ) return false;

	TArray<FCollisionShapeId3D> Shapes;
	if ( !m_World.TryOverlapSphere( Sphere, Shapes, Exclude, Mask ) ) return false;

	TArray<ANode*> Nodes;
	if ( !TryResolveNodes_Internal( Shapes, Nodes ) ) return false;
	OutNodes = Move( Nodes );
	return true;
}


bool CSceneCollision3D::TrySweepSphere( const FSceneRay& Ray, f32 Radius,
	FSceneSweepHit3D& OutHit, FCollisionShapeId3D Exclude, u32 Mask ) noexcept
{
	if ( !Ray.IsValid() || !std::isfinite( Radius ) || Radius < 0.0f || !Sync() ) return false;

	FCollisionSweepHit3D CollisionHit;
	if ( !m_World.TrySweepSphere(
		Ray.ToRay3(), Radius, 0.0f, Ray.MaxDistance, CollisionHit, Exclude, Mask ) ) return false;

	FRegistration* const Registration = FindRegistration_Internal( CollisionHit.Shape );
	if ( Registration == nullptr || m_Graph == nullptr ) return false;
	ANode* const Node = m_Graph->Get( Registration->Node );
	if ( Node == nullptr ) return false;

	FSceneSweepHit3D Hit;
	Hit.Node = Node;
	Hit.Shape = CollisionHit.Shape;
	Hit.Distance = CollisionHit.T;
	Hit.Center = CollisionHit.Center;
	Hit.Normal = CollisionHit.Normal;
	Hit.bStartedOverlapping = CollisionHit.StartedOverlapping;
	OutHit = Hit;
	return true;
}


bool CSceneCollision3D::TryMoveCharacter( const FKinematicCharacterMovementInput3D& Input, const FKinematicCharacterState3D& State, f32 DeltaSeconds, const FKinematicCharacterMovementParams3D& Params, FKinematicCharacterMovementResult3D& OutResult ) noexcept
{
	if ( !Sync() ) return false;
	return TryMoveKinematicCharacter3D( m_World, Input, State, DeltaSeconds, Params, OutResult );
}


FCollisionShapeId3D CSceneCollision3D::TryAdd_Internal( ANode& Node, EShapeKind Kind,
	FVec3 LocalCenter, FVec3 LocalHalfSize, f32 LocalRadius, u32 Layer ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() || Node.IsPendingDestroy() ) return {};
	const FNodeId NodeId = m_Graph->IdOf( &Node );
	if ( !NodeId.IsValid() || FindRegistrationByNode_Internal( NodeId ) != nullptr ) return {};

	FCollisionShapeId3D Shape;
	const u32 AppliedLayer = IsNodeActive_Internal( Node ) ? Layer : 0u;
	if ( Kind == EShapeKind::Box )
	{
		FAabb3 WorldBox;
		if ( !TryMakeWorldBox_Internal( Node, LocalCenter, LocalHalfSize, WorldBox ) ) return {};
		Shape = m_World.TryAddAabb( WorldBox, AppliedLayer );
	}
	else
	{
		FSphere WorldSphere;
		if ( !TryMakeWorldSphere_Internal( Node, LocalCenter, LocalRadius, WorldSphere ) ) return {};
		Shape = m_World.TryAddSphere( WorldSphere, AppliedLayer );
	}
	if ( !Shape.IsValid() ) return {};

	FRegistration Registration;
	Registration.Shape = Shape;
	Registration.Node = NodeId;
	Registration.Kind = Kind;
	Registration.LocalCenter = LocalCenter;
	Registration.LocalHalfSize = LocalHalfSize;
	Registration.LocalRadius = LocalRadius;
	Registration.Layer = Layer;
	if ( !m_Registrations.TryAdd( Registration ) )
	{
		m_World.TryRemove( Shape );
		return {};
	}
	return Shape;
}


bool CSceneCollision3D::TryFindLocalBounds_Internal( ANode& Node, FVec3& OutMinimum,
	FVec3& OutMaximum, bool& OutIsSphere ) noexcept
{
	if ( AMeshComponent3D* const Mesh = Node.GetComponent<AMeshComponent3D>() )
	{
		if ( Mesh->Primitive() == EMeshPrimitive3D::Mesh && Mesh->Mesh() != nullptr )
		{
			const TArray<FMeshVertex>& Vertices = Mesh->Mesh()->Vertices();
			for ( usize Index = 0u; Index < Vertices.Num(); ++Index )
			{
				if ( !IsFinite_Internal( Vertices[Index].position ) ) return false;
			}
		}

		FVec3 Minimum;
		FVec3 Maximum;
		Mesh->LocalBounds( Minimum, Maximum );
		if ( !IsFinite_Internal( Minimum ) || !IsFinite_Internal( Maximum ) ) return false;
		if ( Minimum.x > Maximum.x || Minimum.y > Maximum.y || Minimum.z > Maximum.z ) return false;

		OutMinimum = Minimum;
		OutMaximum = Maximum;
		OutIsSphere = Mesh->Primitive() == EMeshPrimitive3D::Sphere;
		return true;
	}

	ASkinnedMeshComponent3D* const Skin = Node.GetComponent<ASkinnedMeshComponent3D>();
	if ( Skin == nullptr || !Skin->MeshAsset() || Skin->MeshAsset()->Vertices().IsEmpty() ) return false;

	FVec3 Minimum{ 3.4028235e38f, 3.4028235e38f, 3.4028235e38f };
	FVec3 Maximum{ -3.4028235e38f, -3.4028235e38f, -3.4028235e38f };
	const TArray<FSkinnedVertex>& Vertices = Skin->MeshAsset()->Vertices();
	for ( usize Index = 0u; Index < Vertices.Num(); ++Index )
	{
		const FVec3 Position = Vertices[Index].position;
		if ( !IsFinite_Internal( Position ) ) return false;
		if ( Position.x < Minimum.x ) Minimum.x = Position.x;
		if ( Position.y < Minimum.y ) Minimum.y = Position.y;
		if ( Position.z < Minimum.z ) Minimum.z = Position.z;
		if ( Position.x > Maximum.x ) Maximum.x = Position.x;
		if ( Position.y > Maximum.y ) Maximum.y = Position.y;
		if ( Position.z > Maximum.z ) Maximum.z = Position.z;
	}

	OutMinimum = Minimum;
	OutMaximum = Maximum;
	OutIsSphere = false;
	return true;
}


bool CSceneCollision3D::TryMakeWorldBox_Internal( const ANode& Node, FVec3 LocalCenter,
	FVec3 LocalHalfSize, FAabb3& OutBox ) noexcept
{
	if ( !IsFinite_Internal( LocalCenter ) || !IsFinite_Internal( LocalHalfSize ) ) return false;
	if ( LocalHalfSize.x < 0.0f || LocalHalfSize.y < 0.0f || LocalHalfSize.z < 0.0f ) return false;

	const FMat4 World = Node.World().ToMat4();
	FVec3 Minimum{ 3.4028235e38f, 3.4028235e38f, 3.4028235e38f };
	FVec3 Maximum{ -3.4028235e38f, -3.4028235e38f, -3.4028235e38f };
	for ( u32 Corner = 0u; Corner < 8u; ++Corner )
	{
		const FVec3 Local
		{
			LocalCenter.x + ( ( Corner & 1u ) != 0u ? LocalHalfSize.x : -LocalHalfSize.x ),
			LocalCenter.y + ( ( Corner & 2u ) != 0u ? LocalHalfSize.y : -LocalHalfSize.y ),
			LocalCenter.z + ( ( Corner & 4u ) != 0u ? LocalHalfSize.z : -LocalHalfSize.z ),
		};
		const FVec3 Value = TransformPoint( Local, World );
		if ( !IsFinite_Internal( Value ) ) return false;

		if ( Value.x < Minimum.x ) Minimum.x = Value.x;
		if ( Value.y < Minimum.y ) Minimum.y = Value.y;
		if ( Value.z < Minimum.z ) Minimum.z = Value.z;
		if ( Value.x > Maximum.x ) Maximum.x = Value.x;
		if ( Value.y > Maximum.y ) Maximum.y = Value.y;
		if ( Value.z > Maximum.z ) Maximum.z = Value.z;
	}

	OutBox = FAabb3::FromMinMax( Minimum, Maximum );
	return true;
}


bool CSceneCollision3D::TryMakeWorldSphere_Internal( const ANode& Node, FVec3 LocalCenter,
	f32 LocalRadius, FSphere& OutSphere ) noexcept
{
	if ( !IsFinite_Internal( LocalCenter ) || !std::isfinite( LocalRadius ) || LocalRadius <= 0.0f )
		return false;

	const FTransform3D World = Node.World();
	const FVec3 Center = TransformPoint( LocalCenter, World.ToMat4() );
	if ( !IsFinite_Internal( Center ) || !IsFinite_Internal( World.scale ) ) return false;

	const f32 ScaleX = std::fabs( World.scale.x );
	const f32 ScaleY = std::fabs( World.scale.y );
	const f32 ScaleZ = std::fabs( World.scale.z );
	const f32 LargestScale = ScaleX > ScaleY
		? ( ScaleX > ScaleZ ? ScaleX : ScaleZ )
		: ( ScaleY > ScaleZ ? ScaleY : ScaleZ );
	const f32 Radius = LocalRadius * LargestScale;
	if ( !std::isfinite( Radius ) || Radius <= 0.0f ) return false;

	OutSphere = FSphere{ Center, Radius };
	return true;
}


bool CSceneCollision3D::IsNodeActive_Internal( const ANode& Node ) noexcept
{
	const ANode* Current = &Node;
	while ( Current != nullptr )
	{
		if ( !Current->IsEnabled() || Current->IsPendingDestroy() ) return false;
		Current = Current->Parent();
	}
	return true;
}


bool CSceneCollision3D::IsFinite_Internal( FVec3 Value ) noexcept
{
	return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
}


CSceneCollision3D::FRegistration* CSceneCollision3D::FindRegistration_Internal(
	FCollisionShapeId3D Shape ) noexcept
{
	for ( usize Index = 0u; Index < m_Registrations.Num(); ++Index )
	{
		if ( m_Registrations[Index].Shape == Shape ) return &m_Registrations[Index];
	}
	return nullptr;
}


CSceneCollision3D::FRegistration* CSceneCollision3D::FindRegistrationByNode_Internal(
	FNodeId Node ) noexcept
{
	for ( usize Index = 0u; Index < m_Registrations.Num(); ++Index )
	{
		if ( m_Registrations[Index].Node == Node ) return &m_Registrations[Index];
	}
	return nullptr;
}


bool CSceneCollision3D::TryResolveNodes_Internal( const TArray<FCollisionShapeId3D>& Shapes,
	TArray<ANode*>& OutNodes ) noexcept
{
	if ( m_Graph == nullptr ) return false;
	for ( usize Index = 0u; Index < Shapes.Num(); ++Index )
	{
		FRegistration* const Registration = FindRegistration_Internal( Shapes[Index] );
		if ( Registration == nullptr ) return false;
		ANode* const Node = m_Graph->Get( Registration->Node );
		if ( Node == nullptr || !OutNodes.TryAdd( Node ) ) return false;
	}
	return true;
}


bool CSceneCollision3D::RefreshGraphIdentity_Internal() noexcept
{
	if ( m_Graph == nullptr || !m_Graph->HasRoot() ) return false;

	ANode* const CurrentRoot = &m_Graph->Root();
	if ( CurrentRoot == m_RootIdentity ) return true;

	Clear();
	m_RootIdentity = CurrentRoot;
	return true;
}
