// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Prefab/PrefabRegistrar.h"


bool CPrefabRegistrar::Add( const FString& Name, PrefabFactoryFn Factory, void* UserData ) noexcept
{
	if ( m_Prefabs == nullptr || m_Names == nullptr || Factory == nullptr ) return false;

	const char* const StableName = m_Names->Intern( Name );
	if ( StableName == nullptr ) return false;

	const FPrefabId Id = m_Prefabs->Register( StableName, Factory, UserData );
	if ( !Id.IsValid() )
	{
		ACS_LOG_WARN( "CPrefabRegistrar: 登録できませんでした '%s'", StableName );
		return false;
	}

	++m_AddedCount;

	return true;
}
