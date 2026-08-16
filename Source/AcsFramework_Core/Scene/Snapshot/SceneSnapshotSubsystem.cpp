// SPDX-License-Identifier: Apache-2.0
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotSubsystem.h"

#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotFile.h"
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotReader.h"
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotStatus.h"
#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotWriter.h"

// GameInstance スコープへ登録する。入れ物をシーンを跨いで使い回す。
ACS_REGISTER_SUBSYSTEM( CSceneSnapshotSubsystem, ESubsystemScope::GameInstance )


bool CSceneSnapshotSubsystem::SaveToFile( const ANode& Root, const FString& Path ) noexcept
{
	m_LastWrittenBytes = 0u;

	const FSceneSaveResult Result = CSceneSnapshotWriter::WriteTo( Root, m_Buffer );
	m_LastError = Result.Error;

	if ( !Result.Succeeded() )
	{
		ACS_LOG_WARN( "CSceneSnapshotSubsystem: 書き出せません (%s)", CSceneSnapshotStatus::GetName( Result.Error ) );
		return false;
	}

	if ( !CSceneSnapshotFile::Write( Path, m_Buffer.Data(), static_cast<usize>( Result.BytesWritten ) ) ) return false;

	m_LastWrittenBytes = static_cast<usize>( Result.BytesWritten );
	++m_SaveCount;

	return true;
}


TObjectPtr<ANode> CSceneSnapshotSubsystem::LoadFromFile( const FString& Path ) noexcept
{
	usize ReadSize = 0u;
	if ( !CSceneSnapshotFile::Read( Path, m_Buffer, ReadSize ) ) return TObjectPtr<ANode>();

	const FSceneLoadResult Result = CSceneSnapshotReader::ReadFrom( m_Buffer.Data(), ReadSize );
	m_LastError = Result.Error;

	if ( !Result.Succeeded() )
	{
		ACS_LOG_WARN( "CSceneSnapshotSubsystem: 起こせません (%s)", CSceneSnapshotStatus::GetName( Result.Error ) );
		return TObjectPtr<ANode>();
	}

	++m_LoadCount;

	return Result.Root;
}


FString CSceneSnapshotSubsystem::MakeLastErrorMessage() const
{
	return CSceneSnapshotStatus::MakeMessage( m_LastError );
}
