// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Billboard3D/Billboard3DLayer.h"

#include "AcsFramework_Core/Assets/Image/ImageLibrary.h"
#include "AcsFramework_Core/Scene/Billboard3D/Billboard3DMath.h"
#include "AcsFramework_Core/Scene/Sprite3D/Sprite3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Sprite3D/Sprite3DSpawner.h"

#include <cmath>

namespace
{
	/** 見つからない要素番号。 */
	constexpr usize kInvalidIndex = static_cast<usize>( -1 );
}


void CBillboard3DLayer::Bind( CSceneNodeGraph& Graph ) noexcept
{
	Clear();
	m_Graph = &Graph;
	m_RootIdentity = Graph.HasRoot() ? &Graph.Root() : nullptr;
}


void CBillboard3DLayer::Unbind() noexcept
{
	Clear();
	m_Graph = nullptr;
	m_RootIdentity = nullptr;
}


ANode* CBillboard3DLayer::Spawn( const FSprite3DSpawnParams& Params, CImageLibrary& Library,
	EBillboard3DMode Mode, f32 RollDegrees, ANode* Parent ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() || !Params.IsValid() || !IsBillboard3DModeValid( Mode ) || !std::isfinite( RollDegrees ) ) return nullptr;
	if ( !m_Entries.TryReserve( m_Entries.Num() + 1u ) ) return nullptr;

	ANode* const Node = CSprite3DSpawner::SpawnInto( *m_Graph, Params, Library, Parent );
	if ( Node == nullptr ) return nullptr;

	FEntry Entry;
	Entry.Node = Node->Id();
	Entry.Mode = Mode;
	Entry.RollDegrees = RollDegrees;
	if ( m_Entries.TryAdd( Entry ) ) return Node;

	(void)m_Graph->Destroy( Node->Id() );
	ACS_LOG_WARN( "CBillboard3DLayer: 追従登録を確保できなかったため画像板を破棄した" );
	return nullptr;
}


bool CBillboard3DLayer::Track( ANode& Node, EBillboard3DMode Mode, f32 RollDegrees ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() || !IsBillboard3DModeValid( Mode ) || !std::isfinite( RollDegrees ) || Node.IsPendingDestroy() || !Node.Id().IsValid() || m_Graph->Get( Node.Id() ) != &Node || Node.GetComponent<ASprite3DComponent>() == nullptr ) return false;

	const usize ExistingIndex = FindEntryIndex_Internal( Node.Id() );
	if ( ExistingIndex != kInvalidIndex )
	{
		m_Entries[ExistingIndex].Mode = Mode;
		m_Entries[ExistingIndex].RollDegrees = RollDegrees;
		return true;
	}

	FEntry Entry;
	Entry.Node = Node.Id();
	Entry.Mode = Mode;
	Entry.RollDegrees = RollDegrees;
	return m_Entries.TryAdd( Entry );
}


bool CBillboard3DLayer::SetFacing( ANode& Node, EBillboard3DMode Mode, f32 RollDegrees ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() || !IsBillboard3DModeValid( Mode ) || !std::isfinite( RollDegrees ) ) return false;
	const usize Index = FindEntryIndex_Internal( Node.Id() );
	if ( Index == kInvalidIndex || m_Graph->Get( Node.Id() ) != &Node ) return false;
	m_Entries[Index].Mode = Mode;
	m_Entries[Index].RollDegrees = RollDegrees;
	return true;
}


bool CBillboard3DLayer::Remove( ANode& Node ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() ) return false;
	const usize Index = FindEntryIndex_Internal( Node.Id() );
	if ( Index == kInvalidIndex || m_Graph->Get( Node.Id() ) != &Node ) return false;
	m_Entries.RemoveAt( Index );
	return true;
}


void CBillboard3DLayer::Clear() noexcept
{
	m_Entries.Reset();
}


u32 CBillboard3DLayer::UpdateFacing( const CCamera& Camera ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() ) return 0u;

	u32 UpdatedCount = 0u;
	for ( usize ReverseIndex = m_Entries.Num(); ReverseIndex > 0u; --ReverseIndex )
	{
		const usize Index = ReverseIndex - 1u;
		ANode* const Node = m_Graph->Get( m_Entries[Index].Node );
		if ( Node == nullptr || Node->IsPendingDestroy() || Node->GetComponent<ASprite3DComponent>() == nullptr )
		{
			m_Entries.RemoveAt( Index );
			continue;
		}

		const FQuat ParentWorldRotation = Node->Parent() != nullptr ? Node->Parent()->World().rotation : FQuat::Identity();
		FQuat LocalRotation;
		if ( !TryCalculateBillboard3DRotation( Node->World().position, Camera.Eye(), ParentWorldRotation, m_Entries[Index].Mode, m_Entries[Index].RollDegrees, LocalRotation ) ) continue;
		Node->Local().rotation = LocalRotation;
		++UpdatedCount;
	}
	return UpdatedCount;
}


bool CBillboard3DLayer::RefreshGraphIdentity_Internal() noexcept
{
	if ( m_Graph == nullptr || !m_Graph->HasRoot() ) return false;

	ANode* const CurrentRoot = &m_Graph->Root();
	if ( CurrentRoot == m_RootIdentity ) return true;
	Clear();
	m_RootIdentity = CurrentRoot;
	return true;
}


usize CBillboard3DLayer::FindEntryIndex_Internal( FNodeId Node ) const noexcept
{
	if ( !Node.IsValid() ) return kInvalidIndex;
	for ( usize Index = 0u; Index < m_Entries.Num(); ++Index )
	{
		if ( m_Entries[Index].Node == Node ) return Index;
	}
	return kInvalidIndex;
}
