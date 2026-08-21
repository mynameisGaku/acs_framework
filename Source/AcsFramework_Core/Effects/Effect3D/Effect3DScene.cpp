// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Effects/Effect3D/Effect3DScene.h"

FEffect3DHandle AEffect3DScene::PlayEffect3D( FStringView AssetPath, FVec3 WorldPosition ) noexcept
{
	return m_Effects.Play( AssetPath, WorldPosition );
}


FEffect3DHandle AEffect3DScene::PlayEffect3D( FStringView AssetPath, const FEffect3DPlayParams& Params ) noexcept
{
	return m_Effects.Play( AssetPath, Params );
}


void AEffect3DScene::OnUpdate( f32 DeltaSeconds ) noexcept
{
	ALegacyScene3DAdapter::OnUpdate( DeltaSeconds );
	m_Effects.Update( DeltaSeconds );
}


void AEffect3DScene::OnExit() noexcept
{
	m_Effects.Shutdown();
	ALegacyScene3DAdapter::OnExit();
}


bool AEffect3DScene::OnRenderTransparent3D( const FScene3DTransparentRenderContext& Context ) noexcept
{
	return m_Effects.Render( Context.Device, Context.Commands, Context.Camera, Context.ColorTarget, Context.DepthTarget );
}
