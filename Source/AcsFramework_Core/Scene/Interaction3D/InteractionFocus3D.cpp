// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3D.h"

#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DInput.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractionFocus3DTransition.h"
#include "AcsFramework_Core/Scene/Pick3D/ScenePicker.h"

namespace
{
	/** 見つからない対象位置。 */
	constexpr usize kInvalidIndex = static_cast<usize>( -1 );
}


CInteractionFocus3D::~CInteractionFocus3D() noexcept
{
	Unbind();
}


bool CInteractionFocus3D::Bind( CSceneNodeGraph& Graph, CWorldLabel3DLayer& Labels, const FInteractionFocus3DParams& Params ) noexcept
{
	if ( IsBound() || !Graph.HasRoot() || !Labels.IsBoundTo( Graph ) || !Params.IsValid() ) return false;
	m_Graph = &Graph;
	m_Labels = &Labels;
	m_RootIdentity = &Graph.Root();
	m_Params = Params;
	return true;
}


void CInteractionFocus3D::Unbind() noexcept
{
	ClearTargets();
	m_Graph = nullptr;
	m_Labels = nullptr;
	m_RootIdentity = nullptr;
	m_Params = FInteractionFocus3DParams{};
}


bool CInteractionFocus3D::SetParams( const FInteractionFocus3DParams& Params ) noexcept
{
	if ( !Params.IsValid() ) return false;
	m_Params = Params;
	return true;
}


bool CInteractionFocus3D::RegisterTarget( ANode& Node, FStringView Prompt, FVec3 WorldOffset ) noexcept
{
	FWorldLabel3DParams Label;
	Label.Text = Prompt;
	Label.WorldOffset = WorldOffset;
	Label.MaximumDistance = m_Params.MaximumDistance;
	return RegisterTarget( Node, Label );
}


bool CInteractionFocus3D::RegisterTarget( ANode& Node, const FWorldLabel3DParams& PromptLabel ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() || !PromptLabel.IsValid() || Node.IsPendingDestroy() || !Node.Id().IsValid() || m_Graph->Get( Node.Id() ) != &Node || FindTargetIndex_Internal( Node.Id() ) != kInvalidIndex ) return false;

	FEntry Entry;
	if ( !TryCopyPrompt_Internal( PromptLabel.Text, Entry.Prompt ) ) return false;
	Entry.Node = Node.Id();
	Entry.LabelParams = PromptLabel;
	Entry.LabelParams.Text = FStringView{};
	return m_Targets.TryAdd( Move( Entry ) );
}


bool CInteractionFocus3D::UnregisterTarget( ANode& Node ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() ) return false;
	const usize Index = FindTargetIndex_Internal( Node.Id() );
	if ( Index == kInvalidIndex ) return false;
	if ( m_State.FocusedNode == m_Targets[Index].Node ) RemovePrompt_Internal();
	m_Targets.RemoveAt( Index );
	if ( m_State.FocusedNode == Node.Id() ) m_State = FInteractionFocus3DState{};
	return true;
}


void CInteractionFocus3D::ClearTargets() noexcept
{
	RemovePrompt_Internal();
	m_Targets.Reset();
	m_State = FInteractionFocus3DState{};
}


FInteractionFocus3DUpdateResult CInteractionFocus3D::Update( const CCamera& Camera, bool bActivateRequested ) noexcept
{
	if ( !IsBound() ) return {};

	FInteractionFocus3DInput Input;
	if ( RefreshGraphIdentity_Internal() )
	{
		const FSceneRay Ray = FSceneRay::FromNormalizedScreen( Camera, m_Params.ScreenPosition, m_Params.MaximumDistance );
		const FSceneRayHit Hit = CScenePicker::RaycastGeometry( *m_Graph, Ray );
		Input.CandidateNode = FindRegisteredAncestor_Internal( Hit.Node );
		Input.bActivateRequested = bActivateRequested;
	}

	const FInteractionFocus3DUpdateResult Result = AdvanceInteractionFocus3D( m_State, Input );
	if ( Result.FocusChanged() ) RemovePrompt_Internal();
	m_State.FocusedNode = Result.FocusedNode;
	if ( m_State.FocusedNode.IsValid() ) EnsurePrompt_Internal();
	return Result;
}


ANode* CInteractionFocus3D::FocusedNode() const noexcept
{
	if ( m_Graph == nullptr || !m_Graph->HasRoot() || &m_Graph->Root() != m_RootIdentity || FindTargetIndex_Internal( m_State.FocusedNode ) == kInvalidIndex ) return nullptr;
	ANode* const Node = m_Graph->Get( m_State.FocusedNode );
	return Node != nullptr && !Node->IsPendingDestroy() ? Node : nullptr;
}


bool CInteractionFocus3D::TryCopyPrompt_Internal( FStringView Prompt, FString& OutPrompt ) noexcept
{
	if ( Prompt.Data() == nullptr || Prompt.Size() == 0u || Prompt.Size() > FWorldLabel3DParams::kMaximumTextBytes || Prompt.Find( '\0' ) != FStringView::kNpos ) return false;
	FString Candidate;
	if ( !Candidate.TryReserve( Prompt.Size() ) || !Candidate.TryAppend( Prompt ) ) return false;
	OutPrompt = Move( Candidate );
	return true;
}


usize CInteractionFocus3D::FindTargetIndex_Internal( FNodeId Node ) const noexcept
{
	if ( !Node.IsValid() ) return kInvalidIndex;
	for ( usize Index = 0u; Index < m_Targets.Num(); ++Index )
	{
		if ( m_Targets[Index].Node == Node ) return Index;
	}
	return kInvalidIndex;
}


FNodeId CInteractionFocus3D::FindRegisteredAncestor_Internal( ANode* HitNode ) const noexcept
{
	ANode* Current = HitNode;
	for ( u32 Depth = 0u; Current != nullptr && Depth <= kNodeMaxTreeDepth; ++Depth )
	{
		if ( FindTargetIndex_Internal( Current->Id() ) != kInvalidIndex ) return Current->Id();
		Current = Current->Parent();
	}
	return {};
}


bool CInteractionFocus3D::EnsurePrompt_Internal() noexcept
{
	if ( m_Labels == nullptr || m_Graph == nullptr ) return false;
	if ( m_PromptLabel.IsValid() && !m_Labels->Text( m_PromptLabel ).IsEmpty() ) return true;
	m_PromptLabel = FWorldLabel3DHandle{};

	const usize Index = FindTargetIndex_Internal( m_State.FocusedNode );
	if ( Index == kInvalidIndex ) return false;
	ANode* const Node = m_Graph->Get( m_Targets[Index].Node );
	if ( Node == nullptr || Node->IsPendingDestroy() ) return false;

	FWorldLabel3DParams Label = m_Targets[Index].LabelParams;
	Label.Text = m_Targets[Index].Prompt.View();
	m_PromptLabel = m_Labels->AddNodeLabel( *Node, Label );
	return m_PromptLabel.IsValid();
}


void CInteractionFocus3D::RemovePrompt_Internal() noexcept
{
	if ( m_Labels != nullptr && m_PromptLabel.IsValid() ) m_Labels->Remove( m_PromptLabel );
	m_PromptLabel = FWorldLabel3DHandle{};
}


bool CInteractionFocus3D::RefreshGraphIdentity_Internal() noexcept
{
	if ( m_Graph == nullptr || m_Labels == nullptr || !m_Graph->HasRoot() || !m_Labels->IsBoundTo( *m_Graph ) ) return false;
	ANode* const CurrentRoot = &m_Graph->Root();
	if ( CurrentRoot == m_RootIdentity ) return true;

	RemovePrompt_Internal();
	m_Targets.Reset();
	m_RootIdentity = CurrentRoot;
	return true;
}
