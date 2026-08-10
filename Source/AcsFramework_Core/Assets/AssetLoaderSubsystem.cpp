// SPDX-License-Identifier: Apache-2.0
#include "AssetLoaderSubsystem.h"

namespace
{
	/** 要求発行元を再利用しないためのプロセス全体の採番器。0は枯渇状態に予約する。 */
	TAtomic<u64> g_NextAssetOwnerId{ 1u };

	/** u64の最大値を次の値へ進めず枯渇へ遷移させる。 */
	constexpr u64 kMaxAssetRequestValue = ~static_cast<u64>( 0u );

	/** 比較交換で競合を避け、0を発行せずに発行元識別子を確保する。 */
	u64 AcquireAssetOwnerId() noexcept
	{
		for ( ;; )
		{
			// 現在の発行元識別子を比較するための値。
			const u64 Current = g_NextAssetOwnerId.Load();
			if ( Current == 0u ) return 0u;

			// 次回の発行に使う識別子で、最大値の次は枯渇を示す0にする。
			const u64 Next = Current == kMaxAssetRequestValue ? 0u : Current + 1u;
			// 比較交換へ渡す現在値の一致確認用の値。
			u64 Expected = Current;
			if ( g_NextAssetOwnerId.CompareExchange( Expected, Next ) ) return Current;
		}
	}

	/** 世代を0へ戻さず、最大値を最後の発行値として確保する。 */
	u64 AcquireAssetGeneration( u64& NextGeneration ) noexcept
	{
		// 現在の要求で使う世代番号。
		const u64 Current = NextGeneration;
		if ( Current == 0u ) return 0u;

		NextGeneration = Current == kMaxAssetRequestValue ? 0u : Current + 1u;
		return Current;
	}
}

// GameInstanceの寿命に合わせて1件の読み込み窓口を登録する。
ACS_REGISTER_SUBSYSTEM( CAssetLoaderSubsystem, ESubsystemScope::GameInstance )

void CAssetLoaderSubsystem::Begin( const TArray<FString>& Paths, FSimpleDelegate OnComplete )
{
	m_CurrentRequest = FAssetLoadRequest();
	m_Batch.Start( m_Registry, Paths, Move( OnComplete ) );
}

void CAssetLoaderSubsystem::Cancel() noexcept
{
	// 取消し前に処理中かどうかを保持する値。
	const bool bWasLoading = m_Batch.IsLoading();
	m_Batch.Cancel();
	if ( bWasLoading ) m_CurrentRequest = FAssetLoadRequest();
}

FAssetLoadRequest CAssetLoaderSubsystem::BeginRequest( const TArray<FString>& Paths, FSimpleDelegate OnComplete )
{
	// 開始する処理へ割り当てる要求識別子。
	const FAssetLoadRequest Request = AcquireRequest();
	if ( !Request.IsValid() ) return FAssetLoadRequest();

	m_CurrentRequest = Request;
	m_Batch.Start( m_Registry, Paths, Move( OnComplete ) );
	return Request;
}

FAssetLoadRequest CAssetLoaderSubsystem::AcquireRequest() noexcept
{
	if ( m_OwnerId == 0u )
	{
		m_OwnerId = AcquireAssetOwnerId();
		if ( m_OwnerId == 0u ) return FAssetLoadRequest();
	}

	// 新しい要求へ割り当てる世代番号。
	const u64 Generation = AcquireAssetGeneration( m_NextGeneration );
	if ( Generation == 0u ) return FAssetLoadRequest();

	return FAssetLoadRequest( m_OwnerId, Generation );
}

FAssetLoadRequest CAssetLoaderSubsystem::GetCurrentRequest() const noexcept
{
	return m_CurrentRequest;
}

bool CAssetLoaderSubsystem::IsCurrent( FAssetLoadRequest Request ) const noexcept
{
	return Request.IsValid() && Request == m_CurrentRequest;
}

bool CAssetLoaderSubsystem::CancelRequest( FAssetLoadRequest Request ) noexcept
{
	if ( !IsCurrent( Request ) || !m_Batch.IsLoading() ) return false;

	m_Batch.Cancel();
	m_CurrentRequest = FAssetLoadRequest();
	return true;
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
