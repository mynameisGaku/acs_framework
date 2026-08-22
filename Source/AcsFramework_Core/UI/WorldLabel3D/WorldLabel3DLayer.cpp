// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabel3DLayer.h"

#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabelProjector3D.h"

#include <cmath>

namespace
{
	/** 見つからない要素番号。 */
	constexpr usize kInvalidIndex = static_cast<usize>( -1 );

	/** 3成分が有限ならtrueを返す。 */
	bool IsFinite( FVec3 Value ) noexcept
	{
		return std::isfinite( Value.x ) && std::isfinite( Value.y ) && std::isfinite( Value.z );
	}
}


void CWorldLabel3DLayer::Bind( CSceneNodeGraph& Graph ) noexcept
{
	Clear();
	m_Graph = &Graph;
	m_RootIdentity = Graph.HasRoot() ? &Graph.Root() : nullptr;
}


void CWorldLabel3DLayer::Unbind() noexcept
{
	Clear();
	m_Graph = nullptr;
	m_RootIdentity = nullptr;
}


FWorldLabel3DHandle CWorldLabel3DLayer::AddNodeLabel( ANode& Node, const FWorldLabel3DParams& Params ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() || m_NextHandle == 0u || Node.IsPendingDestroy() || !Node.Id().IsValid() || m_Graph->Get( Node.Id() ) != &Node ) return {};

	FEntry Entry;
	if ( !TryMakeEntry_Internal( Params, Entry ) ) return {};

	Entry.Handle = FWorldLabel3DHandle::FromValue( m_NextHandle );
	Entry.Node = Node.Id();
	Entry.bAttachedToNode = true;
	const FWorldLabel3DHandle Handle = Entry.Handle;
	if ( !m_Entries.TryAdd( Move( Entry ) ) ) return {};

	++m_NextHandle;
	return Handle;
}


FWorldLabel3DHandle CWorldLabel3DLayer::AddWorldLabel( FVec3 WorldPosition, const FWorldLabel3DParams& Params ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() || m_NextHandle == 0u || !IsFinite( WorldPosition ) ) return {};

	FEntry Entry;
	if ( !TryMakeEntry_Internal( Params, Entry ) ) return {};

	Entry.Handle = FWorldLabel3DHandle::FromValue( m_NextHandle );
	Entry.WorldPosition = WorldPosition;
	const FWorldLabel3DHandle Handle = Entry.Handle;
	if ( !m_Entries.TryAdd( Move( Entry ) ) ) return {};

	++m_NextHandle;
	return Handle;
}


bool CWorldLabel3DLayer::SetText( FWorldLabel3DHandle Handle, FStringView Text ) noexcept
{
	const usize Index = FindEntryIndex_Internal( Handle );
	if ( Index == kInvalidIndex ) return false;

	FString Candidate;
	if ( !TryCopyText_Internal( Text, Candidate ) ) return false;
	m_Entries[Index].Text = Move( Candidate );
	return true;
}


bool CWorldLabel3DLayer::SetWorldPosition( FWorldLabel3DHandle Handle, FVec3 WorldPosition ) noexcept
{
	const usize Index = FindEntryIndex_Internal( Handle );
	if ( Index == kInvalidIndex || m_Entries[Index].bAttachedToNode || !IsFinite( WorldPosition ) ) return false;
	m_Entries[Index].WorldPosition = WorldPosition;
	return true;
}


bool CWorldLabel3DLayer::SetVisible( FWorldLabel3DHandle Handle, bool bVisible ) noexcept
{
	const usize Index = FindEntryIndex_Internal( Handle );
	if ( Index == kInvalidIndex ) return false;
	m_Entries[Index].bVisible = bVisible;
	return true;
}


FStringView CWorldLabel3DLayer::Text( FWorldLabel3DHandle Handle ) const noexcept
{
	const usize Index = FindEntryIndex_Internal( Handle );
	return Index != kInvalidIndex ? m_Entries[Index].Text.View() : FStringView{};
}


bool CWorldLabel3DLayer::Remove( FWorldLabel3DHandle Handle ) noexcept
{
	const usize Index = FindEntryIndex_Internal( Handle );
	if ( Index == kInvalidIndex ) return false;
	m_Entries.RemoveAt( Index );
	return true;
}


void CWorldLabel3DLayer::Clear() noexcept
{
	m_Entries.Reset();
}


bool CWorldLabel3DLayer::TryProjectLabel( FWorldLabel3DHandle Handle, const CCamera& Camera, u32 ViewportWidth, u32 ViewportHeight, FVec2& OutScreenPosition ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() ) return false;
	const usize Index = FindEntryIndex_Internal( Handle );
	return Index != kInvalidIndex && TryProjectEntry_Internal( m_Entries[Index], Camera, ViewportWidth, ViewportHeight, OutScreenPosition );
}


void CWorldLabel3DLayer::Draw( const CCamera& Camera, FRenderContext& Context, CSpriteBatch& Sprites ) noexcept
{
	if ( !RefreshGraphIdentity_Internal() || !Context.HasFont() || Context.Width() == 0u || Context.Height() == 0u ) return;

	FFont& Font = Context.GetFont();
	const f32 LineHeight = Font.LineHeight();
	if ( !std::isfinite( LineHeight ) || LineHeight <= 0.0f ) return;

	for ( usize Index = 0u; Index < m_Entries.Num(); ++Index )
	{
		const FEntry& Entry = m_Entries[Index];
		FVec2 ScreenPosition;
		if ( !TryProjectEntry_Internal( Entry, Camera, Context.Width(), Context.Height(), ScreenPosition ) ) continue;

		const f32 TextWidth = Font.MeasureWidth( Entry.Text.Data() );
		if ( !std::isfinite( TextWidth ) || TextWidth < 0.0f ) continue;

		const f32 AnchorX = ScreenPosition.x + Entry.ScreenOffset.x;
		const f32 AnchorY = ScreenPosition.y + Entry.ScreenOffset.y;
		const f32 TextX = Entry.bCentered ? AnchorX - TextWidth * 0.5f : AnchorX;
		const f32 TextY = AnchorY - LineHeight;
		if ( Entry.bDrawBackground && Entry.BackgroundColor.w > 0.0f )
		{
			Sprites.DrawRect( TextX - Entry.HorizontalPadding, TextY - Entry.VerticalPadding, TextWidth + Entry.HorizontalPadding * 2.0f, LineHeight + Entry.VerticalPadding * 2.0f, Entry.BackgroundColor );
		}
		Sprites.DrawString( Font, Entry.Text.Data(), TextX, TextY, Entry.TextColor );
	}
}


bool CWorldLabel3DLayer::TryMakeEntry_Internal( const FWorldLabel3DParams& Params, FEntry& OutEntry ) noexcept
{
	if ( !Params.IsValid() ) return false;

	FString Text;
	if ( !TryCopyText_Internal( Params.Text, Text ) ) return false;

	FEntry Candidate;
	Candidate.Text = Move( Text );
	Candidate.WorldOffset = Params.WorldOffset;
	Candidate.ScreenOffset = Params.ScreenOffset;
	Candidate.TextColor = Params.TextColor;
	Candidate.BackgroundColor = Params.BackgroundColor;
	Candidate.MaximumDistance = Params.MaximumDistance;
	Candidate.HorizontalPadding = Params.HorizontalPadding;
	Candidate.VerticalPadding = Params.VerticalPadding;
	Candidate.bDrawBackground = Params.bDrawBackground;
	Candidate.bCentered = Params.bCentered;
	OutEntry = Move( Candidate );
	return true;
}


bool CWorldLabel3DLayer::TryCopyText_Internal( FStringView Text, FString& OutText ) noexcept
{
	if ( Text.Data() == nullptr || Text.Size() == 0u || Text.Size() > FWorldLabel3DParams::kMaximumTextBytes || Text.Find( '\0' ) != FStringView::kNpos ) return false;

	FString Candidate;
	if ( !Candidate.TryReserve( Text.Size() ) || !Candidate.TryAppend( Text ) ) return false;
	OutText = Move( Candidate );
	return true;
}


bool CWorldLabel3DLayer::IsNodeVisible_Internal( const ANode& Node ) noexcept
{
	const ANode* Current = &Node;
	while ( Current != nullptr )
	{
		if ( !Current->IsEnabled() || !Current->IsVisible() || Current->IsPendingDestroy() ) return false;
		Current = Current->Parent();
	}
	return true;
}


bool CWorldLabel3DLayer::TryProjectEntry_Internal( const FEntry& Entry, const CCamera& Camera, u32 ViewportWidth, u32 ViewportHeight, FVec2& OutScreenPosition ) const noexcept
{
	if ( !Entry.bVisible ) return false;

	FVec3 WorldPosition = Entry.WorldPosition;
	if ( Entry.bAttachedToNode )
	{
		if ( m_Graph == nullptr ) return false;
		ANode* const Node = m_Graph->Get( Entry.Node );
		if ( Node == nullptr || !IsNodeVisible_Internal( *Node ) ) return false;
		WorldPosition = Node->World().position;
	}
	WorldPosition += Entry.WorldOffset;
	return CWorldLabelProjector3D::TryProject( Camera, WorldPosition, ViewportWidth, ViewportHeight, Entry.MaximumDistance, OutScreenPosition );
}


bool CWorldLabel3DLayer::RefreshGraphIdentity_Internal() noexcept
{
	if ( m_Graph == nullptr || !m_Graph->HasRoot() ) return false;

	ANode* const CurrentRoot = &m_Graph->Root();
	if ( CurrentRoot == m_RootIdentity ) return true;

	Clear();
	m_RootIdentity = CurrentRoot;
	return true;
}


usize CWorldLabel3DLayer::FindEntryIndex_Internal( FWorldLabel3DHandle Handle ) const noexcept
{
	if ( !Handle.IsValid() ) return kInvalidIndex;
	for ( usize Index = 0u; Index < m_Entries.Num(); ++Index )
	{
		if ( m_Entries[Index].Handle == Handle ) return Index;
	}
	return kInvalidIndex;
}
