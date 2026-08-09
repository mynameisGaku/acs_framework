#pragma once

#include <acs.h>

using namespace acs;

/**
 * アセットをまとめて非同期に読み込み、進み具合を答えるサブシステム。
 *
 * @details
 * 読み込みそのものはエンジンの非同期ローダ (CAssetRegistry::LoadAsync) が別スレッドで行う。
 * この層が足しているのは「複数をひとまとめに扱い、何件終わったかを言えるようにする」ことだけ。
 *
 * 画面には一切触れない。待っている間に何を出すか (ロード画面を被せる・自前の演出を出す・
 * 何も出さない) は呼び出し側が決める。ロード画面へ繋ぎたいときは、ロード画面の側から
 * CLoadingScreenSubsystem::Follow でこちらを見に来させる。
 *
 * @code
 * CAssetLoaderSubsystem* const Loader = GetSubsystem<CAssetLoaderSubsystem>();
 *
 * TArray<FString> Assets;
 * Assets.Add( FString( "Assets/card.jpg" ) );
 * Loader->Begin( Assets, FSimpleDelegate::CreateRaw<&AMyScene::OnLoaded>( this ) );
 *
 * // 待っている間にロード画面を出したいなら、こう繋ぐ (出さないなら書かなくてよい)
 * GetSubsystem<CLoadingScreenSubsystem>()->Follow( *Loader, FString( "読み込み中..." ) );
 * @endcode
 */
class CAssetLoaderSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CAssetLoaderSubsystem )

	/**
	 * 読み込み先を配線する。
	 *
	 * @details
	 * アプリの起動時に 1 度だけ呼ぶ。レジストリはアプリが持っていて普通のゲームコードからは
	 * 辿れないので、ここで受け取る。
	 * @param Registry 読み込みに使うレジストリ。
	 */
	void Bind( CAssetRegistry& Registry ) noexcept { m_Registry = &Registry; }

	/**
	 * 読み込みを始める。
	 *
	 * @details
	 * 呼んだ側は止まらない。進み具合は GetProgress で毎フレーム取れる。走っている最中に
	 * 呼び直すと、前の分は捨てて新しい方だけを見る (進捗の分母が動いて戻って見えないように)。
	 * @param Paths 読み込むアセットのパス。
	 * @param OnComplete 全部終わったときに呼ぶ (省略可)。失敗があっても呼ばれる。
	 */
	void Begin( const TArray<FString>& Paths, FSimpleDelegate OnComplete = FSimpleDelegate() );

	/**
	 * 読み込みを打ち切る。
	 *
	 * @details
	 * 走っているワーカーは止められないので、こちらが見るのをやめるだけ。読み終わったものは
	 * レジストリのキャッシュに残る。完了のコールバックは呼ばれない。
	 */
	void Cancel() noexcept;

	/** 読み込みが走っている最中かを返す。 */
	bool IsLoading() const noexcept { return m_bLoading; }

	/**
	 * 読み終わった割合を返す。
	 *
	 * @return 0..1 (読むものが無ければ 1)。
	 */
	f32 GetProgress() const noexcept;

	/** 直前の読み込みに 1 つでも読めなかったものがあれば true。 */
	bool HasFailed() const noexcept { return m_bFailed; }

	/** 直前の読み込みで頼んだ件数を返す。 */
	usize Num() const noexcept { return m_Entries.Num(); }

	/**
	 * 読み込んだ実体を取り出す。
	 *
	 * @details
	 * 次に Begin を呼ぶまではこのサブシステムが持ち続けるので、その間は解放されない。
	 * @param Index Begin へ渡した配列の添字。
	 * @return 読み込めた実体 (未完了・範囲外・失敗なら空)。
	 */
	TSharedPtr<AAsset> GetAsset( usize Index ) const noexcept;

	/**
	 * 1 フレーム進める。
	 *
	 * @details
	 * 完了を見に行って、全部終わっていればコールバックを呼ぶ。アプリの更新から毎フレーム
	 * 呼ぶこと。ロード画面より先に呼ぶと、同じフレームの進捗が画面へ乗る。
	 */
	void Update() noexcept;

private:
	/** 頼んだ読み込み 1 件分。 */
	struct FEntry
	{
		/** アセットのパス (診断に使う)。 */
		FString Path;

		/** 完了確認用のハンドル。 */
		FAssetFuture Future;

		/** 読み終わったか (成否は問わない)。 */
		bool bFinished = false;

		/** 読めなかったか。 */
		bool bFailed = false;

		/** 読み込んだ実体 (失敗時は空)。 */
		TSharedPtr<AAsset> Asset;
	};

	/** 読み込みを畳んで、完了のコールバックを呼ぶ。 */
	void Finish() noexcept;

	/** 読み込み先。所有はしない (アプリが所有する)。 */
	CAssetRegistry* m_Registry = nullptr;

	/** 頼んだ読み込みの一覧。 */
	TArray<FEntry> m_Entries;

	/** 全部読み終わったときに呼ぶもの。 */
	FSimpleDelegate m_OnComplete;

	/** 読み終わった件数 (毎フレーム数え直さないように控える)。 */
	usize m_FinishedCount = 0;

	/** 読み込みが走っているか。 */
	bool m_bLoading = false;

	/** 直前の読み込みに失敗が含まれていたか。 */
	bool m_bFailed = false;
};
