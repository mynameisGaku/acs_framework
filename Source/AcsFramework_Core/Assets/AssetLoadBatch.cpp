// SPDX-License-Identifier: Apache-2.0
#include "AssetLoadBatch.h"

#include "AcsFramework_Core/Text/StringConvert.h"

void FAssetLoadBatch::Start_Internal( CAssetRegistry* Registry, const TArray<FString>& Paths, FSimpleDelegate OnComplete ) noexcept
{
	ResetObservation();
	m_RequestedCount = Paths.Num();
	m_OnComplete = Move( OnComplete );

	if ( m_RequestedCount == 0u )
	{
		Finish();
		return;
	}

	if ( Registry == nullptr )
	{
		ACS_LOG_WARN( "CAssetLoaderSubsystem::Begin: アセットレジストリが未接続です" );
		SetFailureSummary();
		return;
	}
	if ( !m_Entries.TryReserve( m_RequestedCount ) )
	{
		ACS_LOG_WARN( "CAssetLoaderSubsystem::Begin: entry staging failed" );
		SetFailureSummary();
		return;
	}

	for ( usize Index = 0; Index < m_RequestedCount; ++Index )
	{
		FEntry Entry;
		const FString& InputPath = Paths[Index];
		if ( !Entry.Path.TryReserve( InputPath.Size() ) || !Entry.Path.TryAppend( FStringView( InputPath.Data(), InputPath.Size() ) ) )
		{
			ACS_LOG_WARN( "CAssetLoaderSubsystem::Begin: path staging failed" );
			SetFailureSummary();
			return;
		}
		if ( Entry.Path.IsEmpty() )
		{
			Entry.bFinished = true;
			Entry.bFailed = true;
		}

		if ( !m_Entries.TryAdd( Move( Entry ) ) )
		{
			SetFailureSummary();
			return;
		}
	}

	for ( usize Index = 0; Index < m_Entries.Num(); ++Index )
	{
		FEntry& Entry = m_Entries[Index];
		if ( Entry.bFinished )
		{
			m_bFailed = true;
			++m_FinishedCount;
			continue;
		}

		wchar_t Wide[kMaxAssetPathLength] = {};
		if ( !AcsToWide( Entry.Path, Wide, kMaxAssetPathLength ) )
		{
			ACS_LOG_WARN( "CAssetLoaderSubsystem::Begin: パスを変換できません '%s'", Entry.Path.Data() );
			Entry.bFinished = true;
			Entry.bFailed = true;
			m_bFailed = true;
			++m_FinishedCount;
			continue;
		}

		Entry.Future = Registry->LoadAsync( Wide );
		if ( !Entry.Future.Valid() )
		{
			ACS_LOG_WARN( "CAssetLoaderSubsystem::Begin: 読み込みを開始できません '%s'", Entry.Path.Data() );
			Entry.bFinished = true;
			Entry.bFailed = true;
			m_bFailed = true;
			++m_FinishedCount;
		}
	}

	m_bLoading = true;
	if ( m_FinishedCount >= m_RequestedCount ) Finish();
}

void FAssetLoadBatch::Cancel_Internal() noexcept
{
	if ( !m_bLoading ) return;

	m_bLoading = false;
	m_OnComplete = FSimpleDelegate();
	m_Entries.Reset();
	m_RequestedCount = 0u;
	m_FinishedCount = 0u;
	m_bFailed = false;
}

void FAssetLoadBatch::Update_Internal() noexcept
{
	if ( !m_bLoading ) return;

	for ( usize Index = 0; Index < m_Entries.Num(); ++Index )
	{
		FEntry& Entry = m_Entries[Index];
		if ( Entry.bFinished || !Entry.Future.IsReady() ) continue;

		auto Result = Entry.Future.Get();
		if ( Result.IsOk() )
		{
			Entry.Asset = Move( Result.Value() );
		}
		else
		{
			Entry.bFailed = true;
			m_bFailed = true;
			ACS_LOG_WARN( "CAssetLoaderSubsystem: 読み込みに失敗 '%s'", Entry.Path.Data() );
		}

		Entry.bFinished = true;
		++m_FinishedCount;
	}

	if ( m_FinishedCount >= m_RequestedCount ) Finish();
}

void FAssetLoadBatch::SetFailureSummary() noexcept
{
	m_Entries.Reset();
	m_FinishedCount = m_RequestedCount;
	m_bLoading = false;
	m_bFailed = m_RequestedCount != 0u;
	Finish();
}

void FAssetLoadBatch::Finish() noexcept
{
	m_bLoading = false;
	m_FinishedCount = m_RequestedCount;
	const FSimpleDelegate Completed = m_OnComplete;
	m_OnComplete.Unbind();
	Completed.ExecuteIfBound();
}

void FAssetLoadBatch::ResetObservation() noexcept
{
	m_bLoading = false;
	m_OnComplete = FSimpleDelegate();
	m_Entries.Reset();
	m_RequestedCount = 0u;
	m_FinishedCount = 0u;
	m_bFailed = false;
}

f32 FAssetLoadBatch::GetProgress_Internal() const noexcept
{
	if ( m_RequestedCount == 0u ) return 1.0f;

	return static_cast<f32>( m_FinishedCount ) / static_cast<f32>( m_RequestedCount );
}

TSharedPtr<AAsset> FAssetLoadBatch::GetAsset_Internal( usize Index ) const noexcept
{
	if ( Index >= m_RequestedCount || Index >= m_Entries.Num() ) return TSharedPtr<AAsset>();

	return m_Entries[Index].Asset;
}
