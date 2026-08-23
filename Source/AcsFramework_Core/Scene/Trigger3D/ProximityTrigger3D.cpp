// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Trigger3D/ProximityTrigger3D.h"

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"
#include "AcsFramework_Core/Scene/DebugDraw3D/DebugDraw3DQueue.h"


bool CProximityTrigger3D::Bind( CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision, ANode& Origin,
	const FProximityTrigger3DParams& Params ) noexcept
{
	if ( IsBound() || !Graph.HasRoot() || !Collision.IsBoundTo( Graph )
		|| !Params.IsValid() || Origin.IsPendingDestroy() ) return false;
	const FNodeId OriginId = Graph.IdOf( &Origin );
	if ( !OriginId.IsValid() || Graph.Get( OriginId ) != &Origin ) return false;

	m_Graph = &Graph;
	m_Collision = &Collision;
	m_RootIdentity = &Graph.Root();
	m_Origin = OriginId;
	m_Params = Params;
	m_InsideNodes.Reset();
	return true;
}


void CProximityTrigger3D::Unbind() noexcept
{
	m_Graph = nullptr;
	m_Collision = nullptr;
	m_RootIdentity = nullptr;
	m_Origin = FNodeId{};
	m_Params = FProximityTrigger3DParams{};
	m_InsideNodes.Reset();
}


bool CProximityTrigger3D::Update(
	FProximityTrigger3DUpdateResult& OutResult ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() ) return false;
	ANode* const CurrentOrigin = m_Graph->Get( m_Origin );
	if ( CurrentOrigin == nullptr )
	{
		Unbind();
		return false;
	}

	TArray<FNodeId> CurrentNodes;
	if ( !TryCollectInsideNodes_Internal( *CurrentOrigin, CurrentNodes ) ) return false;

	FProximityTrigger3DUpdateResult Candidate;
	if ( !TryBuildUpdateResult_Internal( CurrentNodes, Candidate ) ) return false;

	m_InsideNodes = Move( CurrentNodes );
	OutResult = Move( Candidate );
	return true;
}


bool CProximityTrigger3D::SetParams(
	const FProximityTrigger3DParams& Params ) noexcept
{
	if ( !Params.IsValid() ) return false;
	m_Params = Params;
	return true;
}


void CProximityTrigger3D::ResetState() noexcept
{
	m_InsideNodes.Reset();
}


bool CProximityTrigger3D::IsBound() const noexcept
{
	return m_Graph != nullptr && m_Collision != nullptr && m_Origin.IsValid();
}


bool CProximityTrigger3D::IsBoundTo( const CSceneNodeGraph& Graph,
	const CSceneCollision3D& Collision ) const noexcept
{
	return m_Graph == &Graph && m_Collision == &Collision
		&& Collision.IsBoundTo( Graph ) && Origin() != nullptr;
}


ANode* CProximityTrigger3D::Origin() const noexcept
{
	if ( !IsBound() || !m_Graph->HasRoot()
		|| &m_Graph->Root() != m_RootIdentity ) return nullptr;
	ANode* const Node = m_Graph->Get( m_Origin );
	return Node != nullptr && !Node->IsPendingDestroy() ? Node : nullptr;
}


bool CProximityTrigger3D::IsInside( FNodeId Node ) const noexcept
{
	return ContainsNode_Internal( m_InsideNodes, Node );
}


bool CProximityTrigger3D::TryGetWorldBox( FAabb3& OutBox ) const noexcept
{
	if ( m_Params.Kind != FProximityTrigger3DParams::EKind::Box ) return false;
	ANode* const CurrentOrigin = Origin();
	return CurrentOrigin != nullptr && IsNodeActive_Internal( *CurrentOrigin )
		&& CSceneCollision3D::TryMakeWorldBox(
		*CurrentOrigin, m_Params.LocalCenter, m_Params.LocalHalfSize, OutBox );
}


bool CProximityTrigger3D::TryGetWorldSphere( FSphere& OutSphere ) const noexcept
{
	if ( m_Params.Kind != FProximityTrigger3DParams::EKind::Sphere ) return false;
	ANode* const CurrentOrigin = Origin();
	return CurrentOrigin != nullptr && IsNodeActive_Internal( *CurrentOrigin )
		&& CSceneCollision3D::TryMakeWorldSphere(
		*CurrentOrigin, m_Params.LocalCenter, m_Params.LocalRadius, OutSphere );
}


bool CProximityTrigger3D::TryCollectInsideNodes_Internal( const ANode& Origin,
	TArray<FNodeId>& OutNodes ) noexcept
{
	TArray<FNodeId> Candidate;
	if ( !IsNodeActive_Internal( Origin ) )
	{
		OutNodes = Move( Candidate );
		return true;
	}

	TArray<ANode*> Hits;
	switch ( m_Params.Kind )
	{
	case FProximityTrigger3DParams::EKind::Sphere:
	{
		FSphere WorldSphere;
		if ( !TryGetWorldSphere( WorldSphere )
			|| !m_Collision->TryOverlapSphere(
				WorldSphere, Hits, {}, m_Params.CollisionMask ) ) return false;
		break;
	}
	case FProximityTrigger3DParams::EKind::Box:
	{
		FAabb3 WorldBox;
		if ( !TryGetWorldBox( WorldBox )
			|| !m_Collision->TryOverlapBox(
				WorldBox, Hits, {}, m_Params.CollisionMask ) ) return false;
		break;
	}
	default:
		return false;
	}

	for ( ANode* const Hit : Hits )
	{
		if ( Hit == nullptr || Hit == &Origin || Hit->IsPendingDestroy() ) continue;
		const FNodeId NodeId = m_Graph->IdOf( Hit );
		if ( !NodeId.IsValid() || m_Graph->Get( NodeId ) != Hit
			|| ContainsNode_Internal( Candidate, NodeId ) ) continue;
		if ( !Candidate.TryAdd( NodeId ) ) return false;
	}

	OutNodes = Move( Candidate );
	return true;
}


bool CProximityTrigger3D::TryBuildUpdateResult_Internal(
	const TArray<FNodeId>& CurrentNodes,
	FProximityTrigger3DUpdateResult& OutResult ) const noexcept
{
	FProximityTrigger3DUpdateResult Candidate;
	for ( const FNodeId Node : CurrentNodes )
	{
		if ( !Candidate.InsideNodes.TryAdd( Node ) ) return false;
		if ( !ContainsNode_Internal( m_InsideNodes, Node )
			&& !Candidate.EnteredNodes.TryAdd( Node ) ) return false;
	}
	for ( const FNodeId Node : m_InsideNodes )
	{
		if ( !ContainsNode_Internal( CurrentNodes, Node )
			&& !Candidate.ExitedNodes.TryAdd( Node ) ) return false;
	}

	OutResult = Move( Candidate );
	return true;
}


bool CProximityTrigger3D::IsNodeActive_Internal( const ANode& Node ) noexcept
{
	const ANode* Current = &Node;
	while ( Current != nullptr )
	{
		if ( !Current->IsEnabled() || Current->IsPendingDestroy() ) return false;
		Current = Current->Parent();
	}
	return true;
}


bool CProximityTrigger3D::ContainsNode_Internal(
	const TArray<FNodeId>& Nodes, FNodeId Node ) noexcept
{
	if ( !Node.IsValid() ) return false;
	for ( const FNodeId Candidate : Nodes )
	{
		if ( Candidate == Node ) return true;
	}
	return false;
}


bool CProximityTrigger3D::RefreshGraphIdentity_Internal() noexcept
{
	if ( !IsBound() || !m_Graph->HasRoot()
		|| !m_Collision->IsBoundTo( *m_Graph ) ) return false;
	if ( &m_Graph->Root() == m_RootIdentity ) return true;

	Unbind();
	return false;
}


bool TryQueueProximityTrigger3D( const CProximityTrigger3D& Trigger,
	CDebugDraw3DQueue& Queue, FVec4 Color, u32 SphereSegments ) noexcept
{
	switch ( Trigger.Params().Kind )
	{
	case FProximityTrigger3DParams::EKind::Sphere:
	{
		FSphere WorldSphere;
		return Trigger.TryGetWorldSphere( WorldSphere )
			&& Queue.TrySphere( WorldSphere, Color, SphereSegments );
	}
	case FProximityTrigger3DParams::EKind::Box:
	{
		FAabb3 WorldBox;
		return Trigger.TryGetWorldBox( WorldBox )
			&& Queue.TryAabb( WorldBox, Color );
	}
	default:
		return false;
	}
}
