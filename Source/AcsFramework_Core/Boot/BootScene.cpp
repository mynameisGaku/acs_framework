// SPDX-License-Identifier: Apache-2.0
#include "BootScene.h"

// Scenes
#include "Debug/DebugTopSample/DebugTopScene.h"
#include "AcsFramework_Sample/Scene/SampleScene.h"

#include "AcsFramework_Core/Fade/FadeSubsystem.h"

void ABootScene::OnEnter() noexcept
{
#if _DEBUG
	GetSubsystem<CFadeSubsystem>()->FadeOut( 0.5f );
	Scenes().ChangeScene( MakeUnique<ADebugTopScene>() );
#else
	Scenes().ChangeScene( MakeUnique<ASampleScene>() );
#endif
}
