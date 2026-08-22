// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/UI/Ui3DScene.h"

#include "AcsFramework_Core/Assets/AssetLoaderSubsystem.h"
#include "AcsFramework_Core/Scene/Animation3D/AnimatedModel3DSpawner.h"
#include "AcsFramework_Core/Scene/Light3D/Light3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"
#include "AcsFramework_Core/Scene/Sprite3D/Sprite3DSpawner.h"
#include "AcsFramework_Core/Scene/Water3D/Water3DSpawner.h"
#include "AcsFramework_Core/UI/InteractionReticle3D/InteractionReticle3DLayout.h"

namespace
{
	/** ACSへ反映済みの選択輪郭設定と完全に同じならtrue。 */
	bool IsSameInteractionHighlightParams( const FInteractionHighlight3DParams& Left, const FInteractionHighlight3DParams& Right ) noexcept
	{
		return Left.bEnabled == Right.bEnabled && Left.Color.x == Right.Color.x && Left.Color.y == Right.Color.y && Left.Color.z == Right.Color.z && Left.Intensity == Right.Intensity && Left.ThicknessPixels == Right.ThicknessPixels;
	}
}

void AUi3DScene::OnEnter() noexcept
{
	ALegacyScene3DAdapter::OnEnter();
	ClearSelectionHighlight();
	m_AppliedInteractionHighlightNode = FNodeId{};
	m_bInteractionHighlightApplied = false;
	m_WorldLabels.Bind( Graph() );
	m_Billboards.Bind( Graph() );
	if ( !m_InteractionFocus.Bind( Graph(), m_WorldLabels ) ) ACS_LOG_WARN( "AUi3DScene: 3D視線フォーカスを場面へ接続できなかった" );
	m_Ui.Init();
}


void AUi3DScene::OnExit() noexcept
{
	ClearSelectionHighlight();
	m_AppliedInteractionHighlightNode = FNodeId{};
	m_bInteractionHighlightApplied = false;
	m_Ui.Shutdown();
	m_InteractionFocus.Unbind();
	m_Billboards.Unbind();
	m_WorldLabels.Unbind();
	m_DebugDraw3D.Shutdown();
	ALegacyScene3DAdapter::OnExit();
}


void AUi3DScene::OnUpdate( f32 DeltaSeconds ) noexcept
{
	ALegacyScene3DAdapter::OnUpdate( DeltaSeconds );
	m_Ui.Tick( DeltaSeconds );
}


void AUi3DScene::OnRender( FRenderContext& Context ) noexcept
{
	(void)RefreshActiveCamera();
	(void)m_Billboards.UpdateFacing( Camera() );
	SyncInteractionHighlight_Internal();
	ALegacyScene3DAdapter::OnRender( Context );
}


void AUi3DScene::OnEvent( const FEvent& Event ) noexcept
{
	m_Ui.HandleInput( Event );
	ALegacyScene3DAdapter::OnEvent( Event );
}


void AUi3DScene::OnDrawHud( FRenderContext& Context, CSpriteBatch& Sprites ) noexcept
{
	ALegacyScene3DAdapter::OnDrawHud( Context, Sprites );
	m_WorldLabels.Draw( Camera(), Context, Sprites );
	DrawInteractionReticle_Internal( Context, Sprites );
	m_Ui.Draw( Context );
}


FInteractionFocus3DUpdateResult AUi3DScene::UpdateInteractionFocus( bool bActivateRequested ) noexcept
{
	return m_InteractionFocus.Update( Camera(), bActivateRequested );
}


ANode* AUi3DScene::SpawnModel3D( const FModel3DSpawnParams& Params, ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return nullptr;
	const bool bNeedsLoad = !Params.MeshAsset && Params.MeshPath.Data() != nullptr && Params.MeshPath.Size() > 0u;
	if ( !bNeedsLoad ) return CModel3DSpawner::SpawnInto( Graph(), Params, Parent );

	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 静的3Dモデル用のasset読込窓口が無い" );
		return nullptr;
	}
	return CModel3DSpawner::SpawnInto( Graph(), Params, Assets->Models(), Parent );
}


ANode* AUi3DScene::SpawnImage3D( const FSprite3DSpawnParams& Params, ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return nullptr;
	if ( Params.IsReady() ) return CSprite3DSpawner::SpawnInto( Graph(), Params, Parent );

	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 固定向き3D画像板用のasset読込窓口が無い" );
		return nullptr;
	}
	return CSprite3DSpawner::SpawnInto( Graph(), Params, Assets->Images(), Parent );
}


ANode* AUi3DScene::SpawnBillboard3D( const FSprite3DSpawnParams& Params,
	EBillboard3DMode Mode, f32 RollDegrees, ANode* Parent ) noexcept
{
	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 3Dビルボード用の画像読込窓口が無い" );
		return nullptr;
	}
	return m_Billboards.Spawn( Params, Assets->Images(), Mode, RollDegrees, Parent );
}


ANode* AUi3DScene::SpawnAnimatedModel3D( const FAnimatedModel3DSpawnParams& Params, ANode* Parent ) noexcept
{
	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 骨付き3Dモデル用のasset読込窓口が無い" );
		return nullptr;
	}
	return CAnimatedModel3DSpawner::SpawnInto( Graph(), Params, Assets->Models(), Parent );
}


ANode* AUi3DScene::SpawnLight3D( const FLight3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CLight3DSpawner::SpawnInto( Graph(), Params, Parent );
}


ANode* AUi3DScene::SpawnWater3D( const FWater3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CWater3DSpawner::SpawnInto( Graph(), Params, Parent );
}


bool AUi3DScene::DestroyNode3D( ANode*& Node ) noexcept
{
	if ( Node == nullptr ) return false;
	const FNodeId NodeId = Graph().IdOf( Node );
	if ( !NodeId.IsValid() || Graph().Get( NodeId ) != Node ) return false;
	if ( !Node->IsPendingDestroy() && !Graph().Destroy( NodeId ) ) return false;

	Node = nullptr;
	return true;
}


bool AUi3DScene::DrawLine3D( FVec3 Start, FVec3 End, FVec4 Color ) noexcept
{
	return m_DebugDraw3D.DrawLine( Start, End, Color );
}


bool AUi3DScene::DrawAabb3D( const FAabb3& Bounds, FVec4 Color ) noexcept
{
	return m_DebugDraw3D.DrawAabb( Bounds, Color );
}


bool AUi3DScene::DrawSphere3D( const FSphere& Sphere, FVec4 Color, u32 Segments ) noexcept
{
	return m_DebugDraw3D.DrawSphere( Sphere, Color, Segments );
}


bool AUi3DScene::OnRenderTransparent3D( const FScene3DTransparentRenderContext& Context ) noexcept
{
	(void)m_DebugDraw3D.Render( Context.Device, Context.Commands, Context.Camera, Context.ColorTarget );
	return false;
}


void AUi3DScene::SyncInteractionHighlight_Internal() noexcept
{
	ANode* const FocusedNode = m_InteractionFocus.FocusedNode();
	if ( !m_InteractionHighlightParams.bEnabled || !m_InteractionHighlightParams.IsValid() || FocusedNode == nullptr )
	{
		if ( m_bInteractionHighlightApplied ) ClearSelectionHighlight();
		m_AppliedInteractionHighlightNode = FNodeId{};
		m_bInteractionHighlightApplied = false;
		return;
	}

	if ( m_bInteractionHighlightApplied && m_AppliedInteractionHighlightNode == FocusedNode->Id() && IsSameInteractionHighlightParams( m_AppliedInteractionHighlightParams, m_InteractionHighlightParams ) ) return;

	if ( SetSelectionHighlight( FocusedNode->Id(), m_InteractionHighlightParams.Color, m_InteractionHighlightParams.Intensity, m_InteractionHighlightParams.ThicknessPixels ) )
	{
		m_AppliedInteractionHighlightNode = FocusedNode->Id();
		m_AppliedInteractionHighlightParams = m_InteractionHighlightParams;
		m_bInteractionHighlightApplied = true;
		return;
	}

	if ( m_bInteractionHighlightApplied ) ClearSelectionHighlight();
	m_AppliedInteractionHighlightNode = FNodeId{};
	m_bInteractionHighlightApplied = false;
}


void AUi3DScene::DrawInteractionReticle_Internal( FRenderContext& Context, CSpriteBatch& Sprites ) noexcept
{
	const bool bFocused = m_InteractionFocus.FocusedNode() != nullptr;
	const FInteractionReticle3DLayout Layout = MakeInteractionReticle3DLayout( m_InteractionReticleParams, m_InteractionFocus.Params().ScreenPosition, Context.Width(), Context.Height(), bFocused, m_InteractionFocus.TargetCount() > 0u );
	if ( !Layout.bVisible ) return;

	for ( usize Index = 0u; Index < FInteractionReticle3DLayout::kRectangleCount; ++Index )
	{
		const FVec4& Rectangle = Layout.Rectangles[Index];
		Sprites.DrawRect( Rectangle.x + m_InteractionReticleParams.ShadowOffset, Rectangle.y + m_InteractionReticleParams.ShadowOffset, Rectangle.z, Rectangle.w, m_InteractionReticleParams.ShadowColor );
	}
	for ( usize Index = 0u; Index < FInteractionReticle3DLayout::kRectangleCount; ++Index )
	{
		const FVec4& Rectangle = Layout.Rectangles[Index];
		Sprites.DrawRect( Rectangle.x, Rectangle.y, Rectangle.z, Rectangle.w, Layout.Color );
	}
}
