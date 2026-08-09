// SPDX-License-Identifier: Apache-2.0
#include "AssetLoaderSubsystem.h"

// GameInstanceの寿命に合わせて1件の読み込み窓口を登録する。
ACS_REGISTER_SUBSYSTEM( CAssetLoaderSubsystem, ESubsystemScope::GameInstance )

void CAssetLoaderSubsystem::Begin( const TArray<FString>& Paths, FSimpleDelegate OnComplete )
{
	m_Batch.Start( m_Registry, Paths, Move( OnComplete ) );
}

void CAssetLoaderSubsystem::Cancel() noexcept
{
	m_Batch.Cancel();
}

f32 CAssetLoaderSubsystem::GetProgress() const noexcept
{
	return m_Batch.GetProgress();
}

TSharedPtr<AAsset> CAssetLoaderSubsystem::GetAsset( usize Index ) const noexcept
{
	return m_Batch.GetAsset( Index );
}

void CAssetLoaderSubsystem::Update() noexcept
{
	m_Batch.Update();
}
