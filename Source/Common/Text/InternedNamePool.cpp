// SPDX-License-Identifier: Apache-2.0
#include "Common/Text/InternedNamePool.h"


const char* CInternedNamePool::Intern( const FString& Name ) noexcept
{
	if ( const char* const Existing = Find( Name ) ) return Existing;

	TUniquePtr<FString> Copy = MakeUnique<FString>();
	if ( !Copy )
	{
		ACS_LOG_WARN( "CInternedNamePool: 名前の確保に失敗しました" );
		return nullptr;
	}

	if ( !Copy->TryAppend( Name.View() ) )
	{
		ACS_LOG_WARN( "CInternedNamePool: 名前の複製に失敗しました" );
		return nullptr;
	}

	const char* const StableName = Copy->Data();
	if ( !m_Names.TryAdd( Move( Copy ) ) )
	{
		ACS_LOG_WARN( "CInternedNamePool: 名前プールの確保に失敗しました" );
		return nullptr;
	}

	return StableName;
}


const char* CInternedNamePool::Find( const FString& Name ) const noexcept
{
	for ( usize Index = 0u; Index < m_Names.Num(); ++Index )
	{
		const TUniquePtr<FString>& Existing = m_Names[Index];
		if ( Existing.Get() != nullptr && *Existing == Name ) return Existing->Data();
	}

	return nullptr;
}
