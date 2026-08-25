// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/UI/Ui3DScene.h"

#include "AcsFramework_Core/Assets/AssetLoaderSubsystem.h"
#include "AcsFramework_Core/Audio/Spatial/SpatialAudioSubsystem.h"
#include "AcsFramework_Core/Scene/Animation3D/AnimatedModel3DSpawner.h"
#include "AcsFramework_Core/Scene/Block3D/Block3DSpawner.h"
#include "AcsFramework_Core/Scene/Bridge3D/Bridge3DSpawner.h"
#include "AcsFramework_Core/Scene/Checkpoint3D/Checkpoint3DSpawner.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3D.h"
#include "AcsFramework_Core/Scene/Character3D/ThirdPersonCharacter3DSpawner.h"
#include "AcsFramework_Core/Scene/Corridor3D/Corridor3DSpawner.h"
#include "AcsFramework_Core/Scene/Doorway3D/Doorway3DSpawner.h"
#include "AcsFramework_Core/Scene/Fence3D/Fence3DSpawner.h"
#include "AcsFramework_Core/Scene/Ground3D/Ground3DSpawner.h"
#include "AcsFramework_Core/Scene/Interaction3D/InteractableModel3DSpawner.h"
#include "AcsFramework_Core/Scene/Light3D/Lamp3DSpawner.h"
#include "AcsFramework_Core/Scene/Light3D/Light3DSpawner.h"
#include "AcsFramework_Core/Scene/Light3D/StudioLightRig3DSpawner.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"
#include "AcsFramework_Core/Scene/Pick3D/ScenePicker.h"
#include "AcsFramework_Core/Scene/Room3D/Room3DSpawner.h"
#include "AcsFramework_Core/Scene/Sphere3D/Sphere3DSpawner.h"
#include "AcsFramework_Core/Scene/Sprite3D/Sprite3DSpawner.h"
#include "AcsFramework_Core/Scene/Stairs3D/Stairs3DSpawner.h"
#include "AcsFramework_Core/Scene/StreetLamp3D/StreetLamp3DSpawner.h"
#include "AcsFramework_Core/Scene/Water3D/Water3DSpawner.h"
#include "AcsFramework_Core/UI/InteractionReticle3D/InteractionReticle3DLayout.h"

namespace
{
	/** world衝突形状を種類に応じた既存デバッグ線へ登録する。 */
	bool DrawWorldCollisionShape_Internal( CDebugDraw3DLayer& Layer,
		const FWorldCollisionShape3D& WorldShape, FVec4 Color, u32 SphereSegments ) noexcept
	{
		if ( !WorldShape.bQueryable ) return false;
		switch ( WorldShape.Kind )
		{
		case FWorldCollisionShape3D::EKind::Box:
			return Layer.DrawAabb( WorldShape.Box, Color );
		case FWorldCollisionShape3D::EKind::Sphere:
			return Layer.DrawSphere( WorldShape.Sphere, Color, SphereSegments );
		default:
			return false;
		}
	}

	/** ACSへ反映済みの選択輪郭設定と完全に同じならtrue。 */
	bool IsSameInteractionHighlightParams( const FInteractionHighlight3DParams& Left, const FInteractionHighlight3DParams& Right ) noexcept
	{
		return Left.bEnabled == Right.bEnabled && Left.Color.x == Right.Color.x && Left.Color.y == Right.Color.y && Left.Color.z == Right.Color.z && Left.Intensity == Right.Intensity && Left.ThicknessPixels == Right.ThicknessPixels;
	}
}

AUi3DScene::AUi3DScene() noexcept
	: m_Collision3D( Graph() )
{
}


bool AUi3DScene::TryApplyVisualPreset3D( EVisualPreset3D Preset ) noexcept
{
	return TryApplyVisualPreset3DSettings( Preset, AmbientOcclusion(), Reflections(),
		GlobalIllumination(), PostParams() );
}


void AUi3DScene::OnEnter() noexcept
{
	ALegacyScene3DAdapter::OnEnter();
	m_Collision3D.Clear();
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
	m_Collision3D.Clear();
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
	(void)RefreshActiveCamera();
	return m_InteractionFocus.Update( Camera(), bActivateRequested );
}


bool AUi3DScene::BindProximityTrigger3D( CProximityTrigger3D& Trigger,
	ANode& Origin, const FProximityTrigger3DParams& Params ) noexcept
{
	return Trigger.Bind( Graph(), m_Collision3D, Origin, Params );
}


bool AUi3DScene::BindCheckpoint3D( CCheckpoint3D& Checkpoint,
	ANode& Origin, FCollisionShapeId3D TargetShape,
	const FCheckpoint3DParams& Params ) noexcept
{
	return Checkpoint.Bind( Graph(), m_Collision3D, Origin, TargetShape, Params );
}


FCheckpoint3DSpawnResult AUi3DScene::SpawnCheckpoint3D(
	CCheckpoint3D& Checkpoint, FCollisionShapeId3D TargetShape,
	FVec3 Position, const FCheckpoint3DParams& Params, ANode* Parent ) noexcept
{
	return CCheckpoint3DSpawner::SpawnInto(
		Graph(), m_Collision3D, Checkpoint, TargetShape, Position, Params, Parent );
}


FCheckpoint3DSpawnResult AUi3DScene::SpawnCheckpoint3D(
	CCheckpoint3D& Checkpoint, FCollisionShapeId3D TargetShape,
	FVec3 Position, f32 LocalRadius, u32 CollisionMask,
	bool bActivateOnce, ANode* Parent ) noexcept
{
	return SpawnCheckpoint3D( Checkpoint, TargetShape, Position,
		FCheckpoint3DParams::Around(
			LocalRadius, CollisionMask, bActivateOnce ), Parent );
}


bool AUi3DScene::DestroyCheckpoint3D( CCheckpoint3D& Checkpoint,
	FCheckpoint3DSpawnResult& Spawned ) noexcept
{
	return CCheckpoint3DSpawner::Destroy(
		Graph(), m_Collision3D, Checkpoint, Spawned );
}


bool AUi3DScene::BindThirdPersonCharacter3D( CThirdPersonCharacter3D& Controller,
	ANode& Character, const FThirdPersonCharacter3DParams& Params ) noexcept
{
	return Controller.Bind( m_Collision3D, *this, Character, Params );
}


FThirdPersonCharacter3DSpawnResult AUi3DScene::SpawnThirdPersonCharacter3D(
	CThirdPersonCharacter3D& Controller, const FModel3DSpawnParams& ModelParams,
	const FThirdPersonCharacter3DSpawnParams& SpawnParams, ANode* Parent ) noexcept
{
	if ( !ModelParams.IsValid() ) return {};
	const bool bNeedsLoad = !ModelParams.MeshAsset && ModelParams.MeshPath.Data() != nullptr
		&& ModelParams.MeshPath.Size() > 0u;
	if ( !bNeedsLoad ) return CThirdPersonCharacter3DSpawner::SpawnInto(
		Graph(), m_Collision3D, *this, Controller, ModelParams, SpawnParams, Parent );

	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 第三者視点用の静的3Dモデル読込窓口が無い" );
		return {};
	}
	return CThirdPersonCharacter3DSpawner::SpawnInto(
		Graph(), m_Collision3D, *this, Controller, ModelParams,
		Assets->Models(), SpawnParams, Parent );
}


FThirdPersonCharacter3DSpawnResult AUi3DScene::SpawnThirdPersonCharacter3D(
	CThirdPersonCharacter3D& Controller, const FAnimatedModel3DSpawnParams& ModelParams,
	const FThirdPersonCharacter3DSpawnParams& SpawnParams, ANode* Parent ) noexcept
{
	if ( !ModelParams.IsValid() ) return {};
	if ( ModelParams.MeshAsset ) return CThirdPersonCharacter3DSpawner::SpawnInto(
		Graph(), m_Collision3D, *this, Controller, ModelParams, SpawnParams, Parent );

	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 第三者視点用の骨格3Dモデル読込窓口が無い" );
		return {};
	}
	return CThirdPersonCharacter3DSpawner::SpawnInto(
		Graph(), m_Collision3D, *this, Controller, ModelParams,
		Assets->Models(), SpawnParams, Parent );
}


bool AUi3DScene::DestroyThirdPersonCharacter3D( CThirdPersonCharacter3D& Controller,
	FThirdPersonCharacter3DSpawnResult& Character ) noexcept
{
	return CThirdPersonCharacter3DSpawner::Destroy(
		Graph(), m_Collision3D, Controller, Character );
}


FSceneRay AUi3DScene::MakeScreenRay3D( FVec2 NormalizedScreenPosition, f32 MaximumDistance ) noexcept
{
	(void)RefreshActiveCamera();
	return FSceneRay::FromNormalizedScreen( Camera(), NormalizedScreenPosition, MaximumDistance );
}


FSceneRayHit AUi3DScene::Raycast3D( const FSceneRay& Ray ) noexcept
{
	return CScenePicker::RaycastGeometry( Graph(), Ray );
}


FSceneRayHit AUi3DScene::PickScreen3D( FVec2 NormalizedScreenPosition, f32 MaximumDistance ) noexcept
{
	return Raycast3D( MakeScreenRay3D( NormalizedScreenPosition, MaximumDistance ) );
}


ANode* AUi3DScene::SpawnNode3D( FStringView Name, ANode* Parent ) noexcept
{
	const FScene3DSpawnResult Spawned = Graph().TrySpawn( Name, Parent );
	return Spawned ? Spawned.Node : nullptr;
}


FCollidableModel3DSpawnResult AUi3DScene::SpawnGround3D(
	const FGround3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CGround3DSpawner::SpawnInto( Graph(), m_Collision3D, Params, Parent );
}


FCollidableModel3DSpawnResult AUi3DScene::SpawnGround3D( FVec2 Size,
	FVec3 Position, u32 CollisionLayer, ANode* Parent ) noexcept
{
	FGround3DSpawnParams Params = FGround3DSpawnParams::FromSize( Size, Position );
	Params.CollisionLayer = CollisionLayer;
	return SpawnGround3D( Params, Parent );
}


bool AUi3DScene::TryUpdateGround3D(
	const FCollidableModel3DSpawnResult& Ground,
	const FGround3DSpawnParams& Params ) noexcept
{
	return CGround3DSpawner::TryApplyTo(
		Graph(), m_Collision3D, Ground, Params );
}


FCollidableModel3DSpawnResult AUi3DScene::SpawnBlock3D(
	const FBlock3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CBlock3DSpawner::SpawnInto( Graph(), m_Collision3D, Params, Parent );
}


FCollidableModel3DSpawnResult AUi3DScene::SpawnBlock3D( FVec3 Size,
	FVec3 Position, u32 CollisionLayer, ANode* Parent ) noexcept
{
	FBlock3DSpawnParams Params = FBlock3DSpawnParams::FromSize( Size, Position );
	Params.CollisionLayer = CollisionLayer;
	return SpawnBlock3D( Params, Parent );
}


bool AUi3DScene::TryUpdateBlock3D(
	const FCollidableModel3DSpawnResult& Block,
	const FBlock3DSpawnParams& Params ) noexcept
{
	return CBlock3DSpawner::TryApplyTo(
		Graph(), m_Collision3D, Block, Params );
}


FCollidableModel3DSpawnResult AUi3DScene::SpawnSphere3D(
	const FSphere3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CSphere3DSpawner::SpawnInto( Graph(), m_Collision3D, Params, Parent );
}


FCollidableModel3DSpawnResult AUi3DScene::SpawnSphere3D( f32 Radius,
	FVec3 Position, u32 CollisionLayer, ANode* Parent ) noexcept
{
	FSphere3DSpawnParams Params = FSphere3DSpawnParams::FromRadius( Radius, Position );
	Params.CollisionLayer = CollisionLayer;
	return SpawnSphere3D( Params, Parent );
}


bool AUi3DScene::TryUpdateSphere3D(
	const FCollidableModel3DSpawnResult& Sphere,
	const FSphere3DSpawnParams& Params ) noexcept
{
	return CSphere3DSpawner::TryApplyTo(
		Graph(), m_Collision3D, Sphere, Params );
}


FBridge3DSpawnResult AUi3DScene::SpawnBridge3D(
	const FBridge3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CBridge3DSpawner::SpawnInto( Graph(), m_Collision3D, Params, Parent );
}


FBridge3DSpawnResult AUi3DScene::SpawnBridge3D( f32 Width,
	f32 Length, f32 RailingHeight, FVec3 EntranceCenter,
	EBridge3DDirection Direction, u32 CollisionLayer, ANode* Parent ) noexcept
{
	FBridge3DSpawnParams Params = FBridge3DSpawnParams::FromDimensions(
		Width, Length, RailingHeight, EntranceCenter, Direction );
	Params.CollisionLayer = CollisionLayer;
	return SpawnBridge3D( Params, Parent );
}


bool AUi3DScene::DestroyBridge3D( FBridge3DSpawnResult& Bridge ) noexcept
{
	return CBridge3DSpawner::Destroy( Graph(), m_Collision3D, Bridge );
}


FCorridor3DSpawnResult AUi3DScene::SpawnCorridor3D(
	const FCorridor3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CCorridor3DSpawner::SpawnInto( Graph(), m_Collision3D, Params, Parent );
}


FCorridor3DSpawnResult AUi3DScene::SpawnCorridor3D( f32 InnerWidth,
	f32 Length, f32 WallHeight, FVec3 EntranceCenter,
	ECorridor3DDirection Direction, u32 CollisionLayer, ANode* Parent ) noexcept
{
	FCorridor3DSpawnParams Params = FCorridor3DSpawnParams::FromDimensions(
		InnerWidth, Length, WallHeight, EntranceCenter, Direction );
	Params.CollisionLayer = CollisionLayer;
	return SpawnCorridor3D( Params, Parent );
}


bool AUi3DScene::DestroyCorridor3D( FCorridor3DSpawnResult& Corridor ) noexcept
{
	return CCorridor3DSpawner::Destroy( Graph(), m_Collision3D, Corridor );
}


FDoorway3DSpawnResult AUi3DScene::SpawnDoorway3D(
	const FDoorway3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CDoorway3DSpawner::SpawnInto( Graph(), m_Collision3D, Params, Parent );
}


FDoorway3DSpawnResult AUi3DScene::SpawnDoorway3D( f32 WallWidth,
	f32 WallHeight, f32 OpeningWidth, f32 OpeningHeight,
	f32 WallThickness, FVec3 BottomCenter, EDoorway3DOrientation Orientation,
	u32 CollisionLayer, ANode* Parent ) noexcept
{
	FDoorway3DSpawnParams Params = FDoorway3DSpawnParams::FromOpening(
		WallWidth, WallHeight, OpeningWidth, OpeningHeight, BottomCenter, Orientation );
	Params.WallThickness = WallThickness;
	Params.CollisionLayer = CollisionLayer;
	return SpawnDoorway3D( Params, Parent );
}


bool AUi3DScene::DestroyDoorway3D( FDoorway3DSpawnResult& Doorway ) noexcept
{
	return CDoorway3DSpawner::Destroy( Graph(), m_Collision3D, Doorway );
}


FFence3DSpawnResult AUi3DScene::SpawnFence3D(
	const FFence3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CFence3DSpawner::SpawnInto( Graph(), m_Collision3D, Params, Parent );
}


FFence3DSpawnResult AUi3DScene::SpawnFence3D( f32 Length, f32 Height,
	f32 MaximumPostSpacing, FVec3 StartPostBottomCenter,
	EFence3DDirection Direction, u32 CollisionLayer, ANode* Parent ) noexcept
{
	FFence3DSpawnParams Params = FFence3DSpawnParams::FromDimensions(
		Length, Height, StartPostBottomCenter, Direction );
	Params.MaximumPostSpacing = MaximumPostSpacing;
	Params.CollisionLayer = CollisionLayer;
	return SpawnFence3D( Params, Parent );
}


bool AUi3DScene::DestroyFence3D( FFence3DSpawnResult& Fence ) noexcept
{
	return CFence3DSpawner::Destroy( Graph(), m_Collision3D, Fence );
}


FStairs3DSpawnResult AUi3DScene::SpawnStairs3D(
	const FStairs3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CStairs3DSpawner::SpawnInto( Graph(), m_Collision3D, Params, Parent );
}


FStairs3DSpawnResult AUi3DScene::SpawnStairs3D( u32 StepCount,
	f32 Width, f32 StepDepth, f32 StepHeight, FVec3 BottomEdgeCenter,
	EStairs3DDirection Direction, u32 CollisionLayer, ANode* Parent ) noexcept
{
	FStairs3DSpawnParams Params = FStairs3DSpawnParams::FromSteps(
		StepCount, Width, StepDepth, StepHeight, BottomEdgeCenter, Direction );
	Params.CollisionLayer = CollisionLayer;
	return SpawnStairs3D( Params, Parent );
}


bool AUi3DScene::DestroyStairs3D( FStairs3DSpawnResult& Stairs ) noexcept
{
	return CStairs3DSpawner::Destroy( Graph(), m_Collision3D, Stairs );
}


FStreetLamp3DSpawnResult AUi3DScene::SpawnStreetLamp3D(
	const FStreetLamp3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CStreetLamp3DSpawner::SpawnInto(
		Graph(), m_Collision3D, Params, Parent );
}


FStreetLamp3DSpawnResult AUi3DScene::SpawnStreetLamp3D(
	FVec3 BasePosition, ANode* Parent ) noexcept
{
	return SpawnStreetLamp3D(
		FStreetLamp3DSpawnParams::At( BasePosition ), Parent );
}


bool AUi3DScene::TryUpdateStreetLamp3D(
	const FStreetLamp3DSpawnResult& StreetLamp,
	const FStreetLamp3DSpawnParams& Params ) noexcept
{
	return CStreetLamp3DSpawner::TryApplyTo(
		Graph(), m_Collision3D, StreetLamp, Params );
}


bool AUi3DScene::DestroyStreetLamp3D(
	FStreetLamp3DSpawnResult& StreetLamp ) noexcept
{
	return CStreetLamp3DSpawner::Destroy(
		Graph(), m_Collision3D, StreetLamp );
}


FRoom3DSpawnResult AUi3DScene::SpawnRoom3D(
	const FRoom3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CRoom3DSpawner::SpawnInto( Graph(), m_Collision3D, Params, Parent );
}


FRoom3DSpawnResult AUi3DScene::SpawnRoom3D( FVec2 InnerSize,
	f32 WallHeight, FVec3 FloorTopPosition, u32 CollisionLayer,
	ANode* Parent ) noexcept
{
	FRoom3DSpawnParams Params = FRoom3DSpawnParams::FromInnerSize(
		InnerSize, WallHeight, FloorTopPosition );
	Params.CollisionLayer = CollisionLayer;
	return SpawnRoom3D( Params, Parent );
}


bool AUi3DScene::DestroyRoom3D( FRoom3DSpawnResult& Room ) noexcept
{
	return CRoom3DSpawner::Destroy( Graph(), m_Collision3D, Room );
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


ANode* AUi3DScene::SpawnInteractableModel3D( const FModel3DSpawnParams& Params,
	FStringView Prompt, FVec3 WorldOffset, ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return nullptr;
	const bool bNeedsLoad = !Params.MeshAsset && Params.MeshPath.Data() != nullptr
		&& Params.MeshPath.Size() > 0u;
	if ( !bNeedsLoad ) return CInteractableModel3DSpawner::SpawnInto(
		Graph(), m_InteractionFocus, Params, Prompt, WorldOffset, Parent );

	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 操作対象用の静的3Dモデル読込窓口が無い" );
		return nullptr;
	}
	return CInteractableModel3DSpawner::SpawnInto(
		Graph(), m_InteractionFocus, Params, Assets->Models(), Prompt,
		WorldOffset, Parent );
}


bool AUi3DScene::DestroyInteractableModel3D( ANode*& Model ) noexcept
{
	return CInteractableModel3DSpawner::Destroy(
		Graph(), m_InteractionFocus, Model );
}


FCollidableModel3DSpawnResult AUi3DScene::SpawnInteractableCollidableModel3D(
	const FModel3DSpawnParams& Params, FStringView Prompt,
	const FCollisionShape3DParams& CollisionParams, FVec3 WorldOffset,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};
	const bool bNeedsLoad = !Params.MeshAsset && Params.MeshPath.Data() != nullptr
		&& Params.MeshPath.Size() > 0u;
	if ( !bNeedsLoad ) return CInteractableModel3DSpawner::SpawnCollidableInto(
		Graph(), m_Collision3D, m_InteractionFocus, Params, Prompt,
		CollisionParams, WorldOffset, Parent );

	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 衝突付き操作対象用の静的3Dモデル読込窓口が無い" );
		return {};
	}
	return CInteractableModel3DSpawner::SpawnCollidableInto(
		Graph(), m_Collision3D, m_InteractionFocus, Params, Assets->Models(),
		Prompt, CollisionParams, WorldOffset, Parent );
}


bool AUi3DScene::DestroyInteractableCollidableModel3D(
	FCollidableModel3DSpawnResult& Model ) noexcept
{
	return CInteractableModel3DSpawner::Destroy(
		Graph(), m_Collision3D, m_InteractionFocus, Model );
}


FCollidableModel3DSpawnResult AUi3DScene::SpawnCollidableModel3D(
	const FModel3DSpawnParams& Params, const FCollisionShape3DParams& CollisionParams,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};
	const bool bNeedsLoad = !Params.MeshAsset && Params.MeshPath.Data() != nullptr && Params.MeshPath.Size() > 0u;
	if ( !bNeedsLoad ) return CModel3DSpawner::SpawnCollidableInto(
		Graph(), m_Collision3D, Params, CollisionParams, Parent );

	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 衝突付き静的3Dモデル用のasset読込窓口が無い" );
		return {};
	}
	return CModel3DSpawner::SpawnCollidableInto(
		Graph(), m_Collision3D, Params, Assets->Models(), CollisionParams, Parent );
}


bool AUi3DScene::DestroyCollidableModel3D(
	FCollidableModel3DSpawnResult& Model ) noexcept
{
	return CModel3DSpawner::DestroyCollidable( Graph(), m_Collision3D, Model );
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
	if ( !Params.IsValid() ) return nullptr;
	if ( Params.MeshAsset ) return CAnimatedModel3DSpawner::SpawnInto( Graph(), Params, Parent );

	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 骨付き3Dモデル用のasset読込窓口が無い" );
		return nullptr;
	}
	return CAnimatedModel3DSpawner::SpawnInto( Graph(), Params, Assets->Models(), Parent );
}


ANode* AUi3DScene::SpawnInteractableAnimatedModel3D(
	const FAnimatedModel3DSpawnParams& Params, FStringView Prompt,
	FVec3 WorldOffset, ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return nullptr;
	if ( Params.MeshAsset ) return CInteractableModel3DSpawner::SpawnInto(
		Graph(), m_InteractionFocus, Params, Prompt, WorldOffset, Parent );

	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 操作対象用の骨格3Dモデル読込窓口が無い" );
		return nullptr;
	}
	return CInteractableModel3DSpawner::SpawnInto(
		Graph(), m_InteractionFocus, Params, Assets->Models(), Prompt,
		WorldOffset, Parent );
}


FCollidableModel3DSpawnResult
AUi3DScene::SpawnInteractableCollidableAnimatedModel3D(
	const FAnimatedModel3DSpawnParams& Params, FStringView Prompt,
	const FCollisionShape3DParams& CollisionParams, FVec3 WorldOffset,
	ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};
	if ( Params.MeshAsset ) return CInteractableModel3DSpawner::SpawnCollidableInto(
		Graph(), m_Collision3D, m_InteractionFocus, Params, Prompt,
		CollisionParams, WorldOffset, Parent );

	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 衝突付き操作対象用の骨格3Dモデル読込窓口が無い" );
		return {};
	}
	return CInteractableModel3DSpawner::SpawnCollidableInto(
		Graph(), m_Collision3D, m_InteractionFocus, Params, Assets->Models(),
		Prompt, CollisionParams, WorldOffset, Parent );
}


FCollidableModel3DSpawnResult AUi3DScene::SpawnCollidableAnimatedModel3D(
	const FAnimatedModel3DSpawnParams& Params,
	const FCollisionShape3DParams& CollisionParams, ANode* Parent ) noexcept
{
	if ( !Params.IsValid() ) return {};
	if ( Params.MeshAsset ) return CAnimatedModel3DSpawner::SpawnCollidableInto(
		Graph(), m_Collision3D, Params, CollisionParams, Parent );

	CAssetLoaderSubsystem* const Assets = GetSubsystem<CAssetLoaderSubsystem>();
	if ( Assets == nullptr )
	{
		ACS_LOG_WARN( "AUi3DScene: 衝突付き骨格3Dモデル用のasset読込窓口が無い" );
		return {};
	}
	return CAnimatedModel3DSpawner::SpawnCollidableInto(
		Graph(), m_Collision3D, Params, Assets->Models(), CollisionParams, Parent );
}


ANode* AUi3DScene::SpawnLight3D( const FLight3DSpawnParams& Params, ANode* Parent ) noexcept
{
	return CLight3DSpawner::SpawnInto( Graph(), Params, Parent );
}


FLamp3DSpawnResult AUi3DScene::SpawnLamp3D(
	const FLamp3DParams& Params, ANode* Parent ) noexcept
{
	return CLamp3DSpawner::SpawnInto( Graph(), Params, Parent );
}


FLamp3DSpawnResult AUi3DScene::SpawnLamp3D(
	FVec3 Position, ANode* Parent ) noexcept
{
	return SpawnLamp3D( FLamp3DParams::At( Position ), Parent );
}


bool AUi3DScene::TryUpdateLamp3D(
	const FLamp3DSpawnResult& Spawned,
	const FLamp3DParams& Params ) noexcept
{
	return CLamp3DSpawner::TryApplyTo( Graph(), Spawned, Params );
}


bool AUi3DScene::DestroyLamp3D( FLamp3DSpawnResult& Spawned ) noexcept
{
	return CLamp3DSpawner::Destroy( Graph(), Spawned );
}


FStudioLightRig3DSpawnResult AUi3DScene::SpawnStudioLightRig3D(
	const FStudioLightRig3DParams& Params, ANode* Parent ) noexcept
{
	return CStudioLightRig3DSpawner::SpawnInto( Graph(), Params, Parent );
}


FStudioLightRig3DSpawnResult AUi3DScene::SpawnStudioLightRig3D(
	FVec3 SubjectCenter, FVec3 ViewDirectionToCamera,
	f32 SubjectRadius, ANode* Parent ) noexcept
{
	return SpawnStudioLightRig3D(
		FStudioLightRig3DParams::AroundSubject(
			SubjectCenter, ViewDirectionToCamera, SubjectRadius ), Parent );
}


bool AUi3DScene::TryUpdateStudioLightRig3D(
	const FStudioLightRig3DSpawnResult& Spawned,
	const FStudioLightRig3DParams& Params ) noexcept
{
	return CStudioLightRig3DSpawner::TryApplyTo(
		Graph(), Spawned, Params );
}


bool AUi3DScene::DestroyStudioLightRig3D(
	FStudioLightRig3DSpawnResult& Spawned ) noexcept
{
	return CStudioLightRig3DSpawner::Destroy( Graph(), Spawned );
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


bool AUi3DScene::PlaySound3D( FStringView AssetPath, FVec3 WorldPosition, f32 Volume, f32 MaximumDistance ) noexcept
{
	FSpatialPlayRequest Request;
	Request.AssetPath = FString( AssetPath );
	Request.Position = WorldPosition;
	Request.BaseVolume = Volume;
	Request.MaxDistance = MaximumDistance;
	return PlaySound3D( Request );
}


bool AUi3DScene::PlaySound3D( const FSpatialPlayRequest& Request ) noexcept
{
	if ( !Request.IsValid() || !RefreshSpatialAudioListener() ) return false;
	CSpatialAudioSubsystem* const Spatial = GetSubsystem<CSpatialAudioSubsystem>();
	return Spatial != nullptr && Spatial->PlayOnce( Request );
}


bool AUi3DScene::RefreshSpatialAudioListener() noexcept
{
	CSpatialAudioSubsystem* const Spatial = GetSubsystem<CSpatialAudioSubsystem>();
	if ( Spatial == nullptr ) return false;

	FAudioListener Listener;
	if ( !TryMakeCameraAudioListener( Listener ) ) return false;
	Spatial->SetListenerNode( nullptr );
	Spatial->SetListener( Listener );
	return true;
}


bool AUi3DScene::TryMakeCameraAudioListener( FAudioListener& OutListener ) noexcept
{
	(void)RefreshActiveCamera();
	return CSpatialListenerBinder::TryMakeFromCamera( *this, OutListener );
}


bool AUi3DScene::DrawLine3D( FVec3 Start, FVec3 End, FVec4 Color ) noexcept
{
	return m_DebugDraw3D.DrawLine( Start, End, Color );
}


bool AUi3DScene::DrawArrow3D( FVec3 Start, FVec3 End, FVec4 Color, f32 HeadSize ) noexcept
{
	return m_DebugDraw3D.DrawArrow( Start, End, Color, HeadSize );
}


bool AUi3DScene::DrawAxes3D( FVec3 Origin, FQuat Rotation, f32 AxisLength, f32 HeadSize ) noexcept
{
	return m_DebugDraw3D.DrawAxes( Origin, Rotation, AxisLength, HeadSize );
}


bool AUi3DScene::DrawGrid3D( FVec3 Center, f32 HalfExtent, u32 Divisions, FVec4 Color ) noexcept
{
	return m_DebugDraw3D.DrawGrid( Center, HalfExtent, Divisions, Color );
}


bool AUi3DScene::DrawCircle3D( FVec3 Center, FVec3 Normal, f32 Radius,
	FVec4 Color, u32 Segments ) noexcept
{
	return m_DebugDraw3D.DrawCircle( Center, Normal, Radius, Color, Segments );
}


bool AUi3DScene::DrawCone3D( FVec3 Apex, FVec3 Direction, f32 Length,
	f32 BaseRadius, FVec4 Color, u32 Segments ) noexcept
{
	return m_DebugDraw3D.DrawCone( Apex, Direction, Length, BaseRadius, Color, Segments );
}


bool AUi3DScene::DrawCylinder3D( FVec3 Center, FVec3 Axis, f32 Height,
	f32 Radius, FVec4 Color, u32 Segments ) noexcept
{
	return m_DebugDraw3D.DrawCylinder( Center, Axis, Height, Radius, Color, Segments );
}


bool AUi3DScene::DrawBox3D( FVec3 Center, FQuat Rotation, FVec3 HalfSize,
	FVec4 Color ) noexcept
{
	return m_DebugDraw3D.DrawBox( Center, Rotation, HalfSize, Color );
}


bool AUi3DScene::DrawAabb3D( const FAabb3& Bounds, FVec4 Color ) noexcept
{
	return m_DebugDraw3D.DrawAabb( Bounds, Color );
}


bool AUi3DScene::DrawSphere3D( const FSphere& Sphere, FVec4 Color, u32 Segments ) noexcept
{
	return m_DebugDraw3D.DrawSphere( Sphere, Color, Segments );
}


bool AUi3DScene::DrawCollisionShape3D( FCollisionShapeId3D Shape,
	FVec4 Color, u32 SphereSegments ) noexcept
{
	FWorldCollisionShape3D WorldShape;
	return m_Collision3D.TryGetWorldShape( Shape, WorldShape )
		&& DrawWorldCollisionShape_Internal( m_DebugDraw3D, WorldShape, Color, SphereSegments );
}


u32 AUi3DScene::DrawCollisionShapes3D( u32 CollisionMask,
	FVec4 Color, u32 SphereSegments ) noexcept
{
	TArray<FWorldCollisionShape3D> WorldShapes;
	if ( !m_Collision3D.TryGetWorldShapes( WorldShapes, CollisionMask ) ) return 0u;

	u32 DrawnShapeCount = 0u;
	for ( usize Index = 0u; Index < WorldShapes.Num(); ++Index )
	{
		if ( DrawWorldCollisionShape_Internal(
			m_DebugDraw3D, WorldShapes[Index], Color, SphereSegments ) ) ++DrawnShapeCount;
	}
	return DrawnShapeCount;
}


bool AUi3DScene::DrawProximityTrigger3D( const CProximityTrigger3D& Trigger,
	FVec4 Color, u32 SphereSegments ) noexcept
{
	return Trigger.IsBoundTo( Graph(), m_Collision3D )
		&& m_DebugDraw3D.DrawProximityTrigger( Trigger, Color, SphereSegments );
}


bool AUi3DScene::DrawCheckpoint3D( const CCheckpoint3D& Checkpoint,
	FVec4 Color, u32 SphereSegments ) noexcept
{
	return Checkpoint.IsBoundTo( Graph(), m_Collision3D )
		&& m_DebugDraw3D.DrawProximityTrigger(
			Checkpoint.Range(), Color, SphereSegments );
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
