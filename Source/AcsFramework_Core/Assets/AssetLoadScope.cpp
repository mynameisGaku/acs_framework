// SPDX-License-Identifier: Apache-2.0
#include "AssetLoadScope.h"

#include "AcsFramework_Core/Assets/AssetLoaderSubsystem.h"

struct CAssetLoadScope::FState
{
	/** GameInstanceが所有するLoaderへの非所有ポインタ。参照先はscopeより長く生存する。 */
	CAssetLoaderSubsystem* Loader = nullptr;

	/** このscopeが現在追跡する要求。無効値は追跡対象がないことを示す。 */
	FAssetLoadRequest OwnedRequest;

	/** 再入後の外側処理による古い追跡状態の書き戻しを防ぐ値。 */
	u64 Epoch = 0u;
};

CAssetLoadScope::CAssetLoadScope( CAssetLoaderSubsystem& Loader ) noexcept
{
	// scope破棄後も外部呼出しの後処理を安全に続ける共有状態。
	m_State = MakeShared<FState>();
	if ( m_State ) m_State->Loader = &Loader;
}

CAssetLoadScope::~CAssetLoadScope() noexcept
{
	// facade破棄後も同じ共有状態で最後の取消しを完了する所有移動。
	TSharedPtr<FState> State = Move( m_State );
	if ( !State || !State->Loader ) return;

	// Loader再入で新しい追跡を巻き込まないよう、先に状態を切り離す。
	const FAssetLoadRequest Owned = State->OwnedRequest;
	AdvanceEpoch( *State );
	State->OwnedRequest = FAssetLoadRequest();
	if ( Owned.IsValid() ) State->Loader->CancelRequest( Owned );
}

void CAssetLoadScope::AdvanceEpoch( FState& State ) noexcept
{
	++State.Epoch;
	if ( State.Epoch == 0u ) State.Epoch = 1u;
}

FAssetLoadRequest CAssetLoadScope::Begin( const TArray<FString>& Paths, FSimpleDelegate OnComplete )
{
	// 外部Loader呼出し中のscope破棄に備えて状態を共有する強参照。
	const TSharedPtr<FState> State = m_State;
	if ( !State || !State->Loader ) return FAssetLoadRequest();

	// 既存追跡を再入前の復元候補として保持する値。
	const FAssetLoadRequest Previous = State->OwnedRequest;
	AdvanceEpoch( *State );
	// 外側のBeginが古い要求を再採用しないための状態世代。
	const u64 BeginEpoch = State->Epoch;
	State->OwnedRequest = FAssetLoadRequest();
	// Loaderが返す要求。空入力や同期完了では無効または非処理中になり得る。
	const FAssetLoadRequest Request = State->Loader->BeginRequest( Paths, Move( OnComplete ) );
	if ( State->Epoch != BeginEpoch ) return Request;

	if ( !Request.IsValid() )
	{
		if ( Previous.IsValid() && State->Loader->IsCurrent( Previous ) && State->Loader->IsLoading() )
		{
			State->OwnedRequest = Previous;
		}
		return Request;
	}

	if ( State->Loader->IsCurrent( Request ) && State->Loader->IsLoading() )
	{
		State->OwnedRequest = Request;
	}
	return Request;
}

bool CAssetLoadScope::Cancel( FAssetLoadRequest Request ) noexcept
{
	// scope破棄後にも共有状態だけで取消し判定を完了する参照。
	const TSharedPtr<FState> State = m_State;
	if ( !State || !State->Loader || !Request.IsValid() || Request != State->OwnedRequest ) return false;

	// 外部Loader呼出しより先に取り外す取消し対象。
	const FAssetLoadRequest Owned = State->OwnedRequest;
	AdvanceEpoch( *State );
	State->OwnedRequest = FAssetLoadRequest();
	return State->Loader->CancelRequest( Owned );
}

void CAssetLoadScope::CancelAll() noexcept
{
	// 外部Loader呼出し中の再入から分離した共有状態。
	const TSharedPtr<FState> State = m_State;
	if ( !State || !State->Loader ) return;

	// 再入するLoader呼出しから分離した取消し対象。
	const FAssetLoadRequest Owned = State->OwnedRequest;
	AdvanceEpoch( *State );
	State->OwnedRequest = FAssetLoadRequest();
	if ( Owned.IsValid() ) State->Loader->CancelRequest( Owned );
}

bool CAssetLoadScope::IsActive( FAssetLoadRequest Request ) const noexcept
{
	// 外部呼出しを越えて保持しない共有状態の参照。
	const TSharedPtr<FState> State = m_State;
	return State && State->Loader && Request.IsValid() && Request == State->OwnedRequest && State->Loader->IsCurrent( Request ) && State->Loader->IsLoading();
}
