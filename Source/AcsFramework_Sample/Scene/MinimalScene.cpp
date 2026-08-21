// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Sample/Scene/MinimalScene.h"

#include "AcsFramework_Core/Scene/Model3D/Model3DSpawner.h"

// ---- ここから下が «全部» ----------------------------------------------------

void AMinimalScene::OnEnter() noexcept
{
	ALegacyScene3DAdapter::OnEnter();

	// 床。
	FModel3DSpawnParams Floor =
		FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Plane, FVec3{ 0.0f, 0.0f, 0.0f } );
	Floor.Scale = FVec3{ 12.0f, 1.0f, 12.0f };
	CModel3DSpawner::SpawnInto( Graph(), Floor );

	// 置いて回す物。
	FModel3DSpawnParams Cube =
		FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{ 0.0f, 1.0f, 0.0f } );
	Cube.Color = FVec4{ 0.85f, 0.45f, 0.25f, 1.0f };
	m_Spinner = CModel3DSpawner::SpawnInto( Graph(), Cube );

	// 全部が入る位置までカメラを引く。
	FrameScene();
}


void AMinimalScene::OnUpdate( f32 DeltaSeconds ) noexcept
{
	ALegacyScene3DAdapter::OnUpdate( DeltaSeconds );

	if ( m_Spinner != nullptr ) m_Spinner->RotateDeg( FVec3{ 0.0f, 40.0f * DeltaSeconds, 0.0f } );
}
