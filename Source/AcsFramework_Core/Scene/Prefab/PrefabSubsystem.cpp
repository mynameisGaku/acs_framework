// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Prefab/PrefabSubsystem.h"

#include "AcsFramework_Core/Scene/Prefab/PrefabRegistrar.h"
#include "AcsFramework_Core/Scene/Prefab/PrefabSpawner.h"

// GameInstance スコープへ登録する。対応表はシーンを跨いで同じものを使う。
ACS_REGISTER_SUBSYSTEM( CPrefabSubsystem, ESubsystemScope::GameInstance )


bool CPrefabSubsystem::AddProvider( TUniquePtr<IPrefabProvider> Provider ) noexcept
{
	if ( !Provider ) return false;

	IPrefabProvider* const Raw = Provider.Get();
	if ( !m_Providers.TryAdd( Move( Provider ) ) )
	{
		ACS_LOG_WARN( "CPrefabSubsystem: 作り方の提供元の確保に失敗しました" );
		return false;
	}

	CPrefabRegistrar Registrar( m_Prefabs, m_Names );
	Raw->ProvidePrefabs( Registrar );

	m_RegisteredCount += Registrar.GetAddedCount();

	return true;
}


FPrefabId CPrefabSubsystem::FindId( const FString& Name ) const noexcept
{
	if ( Name.IsEmpty() ) return FPrefabId();

	return m_Prefabs.FindByName( Name.Data() );
}


ANode* CPrefabSubsystem::SpawnAttached( const FString& Name, ANode& Parent, const FPrefabSpawnParams& Params ) noexcept
{
	return SpawnAttached( FindId( Name ), Parent, Params );
}


ANode* CPrefabSubsystem::SpawnAttached( FPrefabId Id, ANode& Parent, const FPrefabSpawnParams& Params ) noexcept
{
	if ( !Id.IsValid() ) return nullptr;

	ANode* const Spawned = CPrefabSpawner::SpawnAttached( m_Prefabs, Id, Parent, Params );
	if ( Spawned != nullptr ) ++m_SpawnedCount;

	return Spawned;
}


TObjectPtr<ANode> CPrefabSubsystem::SpawnDetached( const FString& Name, const FPrefabSpawnParams& Params ) noexcept
{
	const FPrefabId Id = FindId( Name );
	if ( !Id.IsValid() ) return TObjectPtr<ANode>();

	TObjectPtr<ANode> Spawned = CPrefabSpawner::SpawnDetached( m_Prefabs, Id, Params );
	if ( Spawned.Get() != nullptr ) ++m_SpawnedCount;

	return Spawned;
}
