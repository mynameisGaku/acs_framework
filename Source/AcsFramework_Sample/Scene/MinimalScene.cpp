// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Sample/Scene/MinimalScene.h"

#include "AcsFramework_Core/Scene/Ground3D/Ground3DSpawnParams.h"
#include "AcsFramework_Core/Scene/Model3D/Model3DSpawnParams.h"

// ---- ここから下が «全部» ----------------------------------------------------

void AMinimalScene::OnEnter() noexcept
{
	AUi3DScene::OnEnter();

	// 表示面と歩ける厚みを同じ尺度で持つ床。
	(void)SpawnGround3D( FVec2{ 12.0f, 12.0f } );

	// 置いて回す物。
	FModel3DSpawnParams Cube =
		FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Cube, FVec3{ 0.0f, 1.0f, 0.0f } );
	Cube.Color = FVec4{ 0.85f, 0.45f, 0.25f, 1.0f };
	m_Spinner = SpawnModel3D( Cube );

	// 全部が入る位置までカメラを引く。
	FrameScene();
}


void AMinimalScene::OnUpdate( f32 DeltaSeconds ) noexcept
{
	AUi3DScene::OnUpdate( DeltaSeconds );

	if ( m_Spinner != nullptr ) m_Spinner->RotateDeg( FVec3{ 0.0f, 40.0f * DeltaSeconds, 0.0f } );
}
