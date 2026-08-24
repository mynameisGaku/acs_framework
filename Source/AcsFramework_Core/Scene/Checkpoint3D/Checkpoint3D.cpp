// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3D.h"

#include "AcsFramework_Core/Scene/Collision3D/SceneCollision3D.h"


bool CCheckpoint3D::Bind( CSceneNodeGraph& Graph, CSceneCollision3D& Collision,
	ANode& Origin, FCollisionShapeId3D TargetShape,
	const FCheckpoint3DParams& Params ) noexcept
{
	if ( IsBound() || !Graph.HasRoot() || !Collision.IsBoundTo( Graph )
		|| !Params.IsValid() || Origin.IsPendingDestroy()
		|| !TargetShape.IsValid() ) return false;

	const FNodeId OriginId = Graph.IdOf( &Origin );
	if ( !OriginId.IsValid() || Graph.Get( OriginId ) != &Origin ) return false;

	FWorldCollisionShape3D WorldTarget;
	if ( !Collision.TryGetWorldShape( TargetShape, WorldTarget )
		|| !WorldTarget.Node.IsValid() || WorldTarget.Node == OriginId
		|| ( WorldTarget.Layer & Params.Range.CollisionMask ) == 0u ) return false;
	ANode* const TargetNode = Graph.Get( WorldTarget.Node );
	if ( TargetNode == nullptr || TargetNode->IsPendingDestroy()
		|| !Collision.IsRegisteredTo( TargetShape, *TargetNode ) ) return false;

	if ( !m_Range.Bind( Graph, Collision, Origin, Params.Range ) ) return false;
	u64 NextBindingRevision = m_BindingRevision + 1u;
	if ( NextBindingRevision == 0u ) NextBindingRevision = 1u;

	m_Graph = &Graph;
	m_Collision = &Collision;
	m_RootIdentity = &Graph.Root();
	m_Origin = OriginId;
	m_Target = WorldTarget.Node;
	m_TargetShape = TargetShape;
	m_Params = Params;
	m_bTargetInside = false;
	m_bHasActivated = false;
	m_BindingRevision = NextBindingRevision;
	return true;
}


void CCheckpoint3D::Unbind() noexcept
{
	m_Range.Unbind();
	m_Graph = nullptr;
	m_Collision = nullptr;
	m_RootIdentity = nullptr;
	m_Origin = FNodeId{};
	m_Target = FNodeId{};
	m_TargetShape = FCollisionShapeId3D{};
	m_Params = FCheckpoint3DParams{};
	m_bTargetInside = false;
	m_bHasActivated = false;
}


bool CCheckpoint3D::Update( FCheckpoint3DUpdateResult& OutResult ) noexcept
{
	if ( !RefreshBinding_Internal() ) return false;

	FProximityTrigger3DUpdateResult RangeResult;
	if ( !m_Range.Update( RangeResult ) ) return false;

	const bool bTargetInside = RangeResult.IsInside( m_Target );
	const bool bEntered = RangeResult.DidEnter( m_Target );
	const bool bActivated = bEntered
		&& ( !m_Params.bActivateOnce || !m_bHasActivated );
	const bool bHasActivated = m_bHasActivated || bActivated;

	FCheckpoint3DUpdateResult Candidate;
	Candidate.bActivatedThisUpdate = bActivated;
	Candidate.bTargetInside = bTargetInside;
	Candidate.bHasActivated = bHasActivated;

	m_bTargetInside = bTargetInside;
	m_bHasActivated = bHasActivated;
	OutResult = Candidate;
	return true;
}


bool CCheckpoint3D::SetParams( const FCheckpoint3DParams& Params ) noexcept
{
	if ( !Params.IsValid() ) return false;
	if ( IsBound() )
	{
		FWorldCollisionShape3D WorldTarget;
		if ( !m_Collision->TryGetWorldShape( m_TargetShape, WorldTarget )
			|| WorldTarget.Node != m_Target
			|| ( WorldTarget.Layer & Params.Range.CollisionMask ) == 0u ) return false;
	}
	if ( !m_Range.SetParams( Params.Range ) ) return false;
	m_Params = Params;
	return true;
}


void CCheckpoint3D::ResetActivation() noexcept
{
	m_Range.ResetState();
	m_bTargetInside = false;
	m_bHasActivated = false;
}


bool CCheckpoint3D::IsBound() const noexcept
{
	return m_Graph != nullptr && m_Collision != nullptr
		&& m_Origin.IsValid() && m_Target.IsValid() && m_TargetShape.IsValid();
}


bool CCheckpoint3D::HasBindingIdentity( const CSceneNodeGraph& Graph,
	const CSceneCollision3D& Collision, FNodeId Origin,
	u64 BindingRevision ) const noexcept
{
	return BindingRevision != 0u
		&& m_Collision == &Collision && HasBindingOrigin( Graph, Origin )
		&& m_BindingRevision == BindingRevision;
}


bool CCheckpoint3D::HasBindingOrigin( const CSceneNodeGraph& Graph,
	FNodeId Origin ) const noexcept
{
	return IsBound() && m_Graph == &Graph && m_Origin == Origin;
}


bool CCheckpoint3D::IsBoundTo( const CSceneNodeGraph& Graph,
	CSceneCollision3D& Collision ) const noexcept
{
	ANode* const CurrentTarget = Target();
	return m_Graph == &Graph && m_Collision == &Collision
		&& Collision.IsBoundTo( Graph ) && Origin() != nullptr
		&& CurrentTarget != nullptr
		&& Collision.IsRegisteredTo( m_TargetShape, *CurrentTarget );
}


ANode* CCheckpoint3D::Origin() const noexcept
{
	if ( !IsBound() || !m_Graph->HasRoot()
		|| &m_Graph->Root() != m_RootIdentity ) return nullptr;
	ANode* const Node = m_Graph->Get( m_Origin );
	return Node != nullptr && !Node->IsPendingDestroy() ? Node : nullptr;
}


ANode* CCheckpoint3D::Target() const noexcept
{
	if ( !IsBound() || !m_Graph->HasRoot()
		|| &m_Graph->Root() != m_RootIdentity ) return nullptr;
	ANode* const Node = m_Graph->Get( m_Target );
	return Node != nullptr && !Node->IsPendingDestroy() ? Node : nullptr;
}


bool CCheckpoint3D::RefreshBinding_Internal() noexcept
{
	if ( !IsBound() ) return false;
	if ( !m_Graph->HasRoot() || !m_Collision->IsBoundTo( *m_Graph ) )
	{
		Unbind();
		return false;
	}
	if ( &m_Graph->Root() != m_RootIdentity )
	{
		Unbind();
		return false;
	}

	ANode* const CurrentOrigin = Origin();
	ANode* const CurrentTarget = Target();
	if ( CurrentOrigin == nullptr || CurrentTarget == nullptr
		|| CurrentOrigin == CurrentTarget
		|| !m_Collision->IsRegisteredTo( m_TargetShape, *CurrentTarget ) )
	{
		Unbind();
		return false;
	}
	return true;
}
