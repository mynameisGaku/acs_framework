#include "AssetLoaderSubsystem.h"

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace
{
	/** アセットのパスの長さの上限 (ワイド文字数)。engine の bundle と同じ長さに揃える。 */
	constexpr i32 kMaxAssetPathLength = 260;

	/**
	 * UTF-8 のパスをワイド文字へ直す。
	 *
	 * @details CAssetRegistry がワイド文字のパスを取るため。engine 側の作法に合わせている。
	 * @param Path 変換元のパス。
	 * @param OutBuffer 書き込み先。
	 * @param Capacity 書き込み先の容量 (wchar_t 数)。
	 * @return 変換できたら true。
	 */
	bool WidenAssetPath( const FString& Path, wchar_t* OutBuffer, i32 Capacity ) noexcept
	{
		if ( OutBuffer == nullptr || Capacity <= 0 ) return false;

		OutBuffer[0] = 0;
		if ( Path.IsEmpty() ) return false;

		return ::MultiByteToWideChar( CP_UTF8, 0, Path.Data(), -1, OutBuffer, Capacity ) > 0;
	}
}


// GameInstance スコープへ登録する。シーンを切り替えても読み込みは続く。
ACS_REGISTER_SUBSYSTEM( CAssetLoaderSubsystem, ESubsystemScope::GameInstance )


void CAssetLoaderSubsystem::Begin( const TArray<FString>& Paths, FSimpleDelegate OnComplete )
{
	if ( m_Registry == nullptr )
	{
		// 配線前に呼ばれた。黙って何もしないと「読み込みが終わらない」ように見えるので、
		// 終わったことにしてコールバックだけ返す。
		ACS_LOG_WARN( "CAssetLoaderSubsystem::Begin: アセットレジストリが未配線" );
		OnComplete.ExecuteIfBound();
		return;
	}

	m_Entries.Reset();
	m_OnComplete = OnComplete;
	m_FinishedCount = 0;
	m_bFailed = false;

	for ( usize Index = 0; Index < Paths.Num(); ++Index )
	{
		wchar_t Wide[kMaxAssetPathLength];
		if ( !WidenAssetPath( Paths[Index], Wide, kMaxAssetPathLength ) )
		{
			ACS_LOG_WARN( "CAssetLoaderSubsystem::Begin: パスを変換できません '%s'", Paths[Index].Data() );
			continue;
		}

		FEntry Entry;
		Entry.Path = Paths[Index];
		// 読み込みそのものはエンジンのワーカーが行う。ここは完了を見に行くだけ。
		Entry.Future = m_Registry->LoadAsync( Wide );
		if ( !Entry.Future.Valid() )
		{
			ACS_LOG_WARN( "CAssetLoaderSubsystem::Begin: 読み込みを開始できません '%s'", Paths[Index].Data() );
			Entry.bFinished = true;
			Entry.bFailed = true;
			m_bFailed = true;
			++m_FinishedCount;
		}
		m_Entries.Add( Move( Entry ) );
	}

	m_bLoading = true;

	// 読むものが無ければ待たせない。
	if ( m_FinishedCount >= m_Entries.Num() ) Finish();
}

void CAssetLoaderSubsystem::Cancel() noexcept
{
	if ( !m_bLoading ) return;

	// ワーカーは止められないので、こちらが見るのをやめるだけ。読み終わったものは
	// レジストリのキャッシュに残るので、次に要るときは即座に返る。
	m_bLoading = false;
	m_OnComplete = FSimpleDelegate();
	m_Entries.Reset();
	m_FinishedCount = 0;
}

f32 CAssetLoaderSubsystem::GetProgress() const noexcept
{
	if ( m_Entries.IsEmpty() ) return 1.0f;

	return static_cast<f32>( m_FinishedCount ) / static_cast<f32>( m_Entries.Num() );
}

TSharedPtr<AAsset> CAssetLoaderSubsystem::GetAsset( usize Index ) const noexcept
{
	if ( Index >= m_Entries.Num() ) return TSharedPtr<AAsset>();

	return m_Entries[Index].Asset;
}

void CAssetLoaderSubsystem::Update() noexcept
{
	if ( !m_bLoading ) return;

	for ( usize Index = 0; Index < m_Entries.Num(); ++Index )
	{
		FEntry& Entry = m_Entries[Index];
		if ( Entry.bFinished || !Entry.Future.IsReady() ) continue;

		// IsReady が true なので Get は待たずに返る。実体を持っておくと、使うまでの間に
		// 他から解放されない。
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

	if ( m_FinishedCount >= m_Entries.Num() ) Finish();
}

void CAssetLoaderSubsystem::Finish() noexcept
{
	// コールバックの中で次の読み込みを始められるよう、呼ぶ前に状態を畳んでおく。
	m_bLoading = false;

	const FSimpleDelegate Completed = m_OnComplete;
	m_OnComplete = FSimpleDelegate();
	Completed.ExecuteIfBound();
}
