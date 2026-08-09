#pragma once

#include <acs.h>

#include "AcsFramework_Core/Assets/AssetLoaderSubsystem.h"

using namespace acs;

/**
 * いつでも被せられるロード画面。
 *
 * @details
 * シーンを差し替えず、いま動いている画面の上へ幕として重ねる。押している間だけ出したい、
 * 何かが終わるまで出したい、といった用途をシーン構成に影響させずに済ませる。
 * acs にロード画面の仕組みは無いので、フェード幕と同じ作法 (専用の SpriteBatch を遅らせて
 * 用意し、シーン描画の後に重ねる) で自前に持つ。
 *
 * 出すもの・進み具合を「どう見せるか」だけを持つ。何を待っているかは知らないので、
 * アセットの読み込みに限らず何にでも被せられる。読み込みに繋ぎたいときは Follow で
 * CAssetLoaderSubsystem を見に行かせる (読み込み側はこの画面を知らない)。
 *
 * 出し入れは滑らかに繋ぐので、一瞬で終わる処理に被せてもちらつかない。
 *
 * @code
 * if ( CLoadingScreenSubsystem* Loading = GetSubsystem<CLoadingScreenSubsystem>() )
 * {
 *     // 自分で出し入れする
 *     Loading->Show( FString( "読み込み中..." ) );
 *     Loading->SetProgress( 0.4f );   // 割合が分かるなら渡す (省略時はスピナーだけ)
 *     Loading->Disable();
 *
 *     // 読み込みに任せる (出るのも、バーが進むのも、消えるのも自動)
 *     Loading->Follow( *Loader, FString( "アセットを読み込んでいます" ) );
 * }
 * @endcode
 */
class CLoadingScreenSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CLoadingScreenSubsystem )

	/**
	 * ロード画面を出す。
	 *
	 * @details 既に出ている場合は表示だけ差し替える (出し直しでちらつかせない)。
	 * @param Message 中央へ出す文言 (空なら文字を出さない)。
	 */
	void Show( const FString& Message = FString() );

	/** ロード画面を出す (文言はそのまま)。 */
	void Enable() noexcept { SetEnabled( true ); }

	/** ロード画面を消す (すぐには消えず、薄くなってから消える)。 */
	void Disable() noexcept { SetEnabled( false ); }

	/** 出ていれば消し、消えていれば出す。 */
	void Toggle() noexcept { SetEnabled( !m_bVisible ); }

	/**
	 * 出す / 消すを切り替える。
	 *
	 * @param bEnabled 出すなら true。
	 */
	void SetEnabled( bool bEnabled ) noexcept;

	/**
	 * 読み込みを見に行き、その進み具合でバーを進める。
	 *
	 * @details
	 * 読み込んでいる間だけ自動で出て、終わると自分で消える。読み込みの中身には触らないので、
	 * 何をどう読むかは相手が決める。読み込みが既に終わっていれば一度も出さない
	 * (すぐ終わる読み込みで画面をちらつかせないため)。
	 * @param Loader 見に行く読み込み。
	 * @param Message 出している間に中央へ出す文言。
	 */
	void Follow( const CAssetLoaderSubsystem& Loader, const FString& Message = FString() );

	/** 見に行くのをやめる (出したままにはしない)。 */
	void Unfollow() noexcept;

	/**
	 * 進捗を設定する。
	 *
	 * @details
	 * 0..1 を渡すとバーが出る。負を渡すと「どれだけ掛かるか分からない」状態に戻り、
	 * スピナーだけになる。
	 * @param Ratio 進捗 (0..1。負で不定)。
	 */
	void SetProgress( f32 Ratio ) noexcept;

	/**
	 * 文言だけを差し替える。
	 *
	 * @param Message 中央へ出す文言。
	 */
	void SetMessage( const FString& Message );

	/**
	 * 文言に使うフォントを差し替える。
	 *
	 * @details
	 * 既定はエンジン共有の UI フォントだが、これは ASCII と仮名しか焼いていないので漢字が
	 * 無言で消える。漢字を出すなら、漢字を含めて焼いたフォントをここへ渡すこと。
	 * 渡したフォントは呼び出し側が生かし続けること。
	 * @param Font 使うフォント (nullptr で共有フォントへ戻す)。
	 */
	void SetFont( const FFont* Font ) noexcept { m_Font = Font; }

	/** 出す指示が生きているかを返す (薄くなっている最中は false)。 */
	bool IsVisible() const noexcept { return m_bVisible; }

	/** 画面に何か出ているかを返す (消えかけも含む)。 */
	bool IsOnScreen() const noexcept { return m_bVisible || m_Alpha > 0.0f; }

	/**
	 * 1 フレーム進める。
	 *
	 * @details アプリの更新から毎フレーム呼ぶ。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( f32 DeltaSeconds ) noexcept;

	/**
	 * ロード画面を描く。
	 *
	 * @details
	 * シーンを描き終えた後に呼ぶこと。描画資源は最初に出すときだけ用意するので、
	 * 一度も出さなければ GPU 資源を 1 つも作らない。
	 * @param Renderer 描画資源の取得元。
	 * @param SharedFont エンジン共有の UI フォント (SetFont の指定が無いときに使う)。
	 */
	void Draw( CRenderer& Renderer, const FFont* SharedFont ) noexcept;

private:
	/** 見に行っている読み込みの進み具合を、出し入れと進捗へ反映する。 */
	void UpdateFollow() noexcept;

	/** 中央へ出す文言。 */
	FString m_Message;

	/** 文言に使うフォント (nullptr なら共有フォント)。所有はしない。 */
	const FFont* m_Font = nullptr;

	/** 幕の濃さ (0 で透明、1 で出し切り)。 */
	f32 m_Alpha = 0.0f;

	/** スピナーを回すのに使う経過秒。 */
	f32 m_Elapsed = 0.0f;

	/** 進捗 (負なら不定)。 */
	f32 m_Progress = -1.0f;

	/** 幕を描くための SpriteBatch (最初に出すときだけ用意する)。 */
	CSpriteBatch m_Overlay;

	/** 見に行っている読み込み。所有はしない (nullptr なら誰も見ていない)。 */
	const CAssetLoaderSubsystem* m_Followed = nullptr;

	/** 出す指示が生きているか。 */
	bool m_bVisible = false;

	/** 描画資源を用意しようとしたか (失敗を毎フレーム繰り返さないため)。 */
	bool m_bOverlayTried = false;

	/** 描画資源が使える状態か。 */
	bool m_bOverlayReady = false;
};
