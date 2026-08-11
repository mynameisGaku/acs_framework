// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Assets/AssetLoadRequest.h"
#include "AcsFramework_Core/Assets/AssetLoaderSubsystem.h"
#include "AcsFramework_Core/Loading/LoadingScreenRenderer.h"

using namespace acs;

/**
 * GameInstance の寿命でロード表示の状態と外部窓口を所有するサブシステム。
 *
 * 読み込み追従、表示世代、フェード、フォント優先順位を管理し、GPU 描画は専用の描画型へ任せる。
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
	 * Requestが現在のLoaderの識別を示す場合だけ追従する。完了後に保持された要求も受理する。
	 *
	 * @param Loader 対象の読み込み窓口。非所有で、UnfollowRequestまたは自動完了まで生存させる。同じGameInstanceの寿命で管理する。
	 * @param Request 追従する要求。
	 * @param Message 画面中央へ表示する文言。
	 * @return Loaderの現在要求と一致した場合はtrue。無効または古い要求ではfalseで画面状態を変えない。
	 */
	bool FollowRequest( const CAssetLoaderSubsystem& Loader, FAssetLoadRequest Request, const FString& Message = FString() );

	/**
	 * 保存した要求と一致する場合だけ追従を解除する。
	 *
	 * @param Request 解除する要求。
	 * @return 保存した要求と完全一致した場合はtrue。別要求、無効値、未追従ではfalseで状態を変えない。
	 */
	bool UnfollowRequest( FAssetLoadRequest Request ) noexcept;

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
	void SetFont( const FFont* Font ) noexcept;

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
	/** この通常型だけが要求と追従世代を照合して局所解除を行う。 */
	friend class CLoadingScreenFollowScope;

	/** この通常型だけが表示世代を照合して局所表示を解除する。 */
	friend class CLoadingScreenDisplayScope;

	/** LoaderとRequestをscope用に登録し、成功時の追従世代を返す。無効または現在でない要求はfalseで世代0を返す。 */
	bool FollowScopedRequest( const CAssetLoaderSubsystem& Loader, FAssetLoadRequest Request, const FString& Message, u64& Revision );

	/** Requestと追従世代が現在のLoader追従と一致するか返す。無効または外部変更後はfalseを返す。 */
	bool IsScopedFollowCurrent( FAssetLoadRequest Request, u64 Revision ) const noexcept;

	/** Requestと追従世代が一致する場合だけ追従を解除する。不一致はfalseで状態を変えない。 */
	bool UnfollowRequest( FAssetLoadRequest Request, u64 Revision ) noexcept;

	/** 追従世代を進め、0を無効値として避ける。 */
	void AdvanceFollowRevision() noexcept;

	/** 見に行っている読み込みの進み具合を、出し入れと進捗へ反映する。 */
	void UpdateFollow() noexcept;

	/** 読み込み窓口、要求、表示状態を同時に解除する。 */
	void ClearFollow() noexcept;

	/** 手動表示用フォントを解除し、永続設定または共有フォントへ戻す。 */
	void ClearDisplayScopeFont() noexcept;

	/** 手動表示を取得し、成功時の表示世代を返す。追従中はfalseで状態を変えない。 */
	bool AcquireDisplayScope( const FString& Message, u64& Revision );

	/** 表示世代が現在値で追従中でないかを返す。無効世代はfalseを返す。 */
	bool IsDisplayScopeCurrent( u64 Revision ) const noexcept;

	/** 現在の表示世代だけ文言を差し替える。古い世代はfalseで状態を変えない。 */
	bool SetDisplayScopeMessage( u64 Revision, const FString& Message );

	/** 現在の表示世代だけ進捗を差し替える。古い世代はfalseで状態を変えない。 */
	bool SetDisplayScopeProgress( u64 Revision, f32 Ratio ) noexcept;

	/** 現在の表示世代だけフォントを差し替える。古い世代はfalseで状態を変えない。 */
	bool SetDisplayScopeFont( u64 Revision, const FFont* Font ) noexcept;

	/** 現在の表示世代だけ表示を解除する。古い世代はfalseで状態を変えない。 */
	bool ReleaseDisplayScope( u64 Revision ) noexcept;

	/** 手動表示用フォントを解除して表示世代を進め、0を無効値として避ける。 */
	void AdvanceDisplayRevision() noexcept;

	/** 進捗値を範囲へ収めて内部状態へ書き込む。 */
	void SetProgressValue( f32 Ratio ) noexcept;

	/** 中央へ出す文言。 */
	FString m_Message;

	/** 公開設定で使うフォント (nullptr なら共有フォント)。所有せず、設定中は置換またはnullptrまで生存させる。 */
	const FFont* m_Font = nullptr;

	/** 手動表示だけで使うフォント。所有せず、表示世代の交代時に解除する。 */
	const FFont* m_DisplayScopeFont = nullptr;

	/** 幕の濃さ (0 で透明、1 で出し切り)。 */
	f32 m_Alpha = 0.0f;

	/** 進捗 (負なら不定)。 */
	f32 m_Progress = -1.0f;

	/** ロード画面の GPU 描画を受け持つ通常型。 */
	FLoadingScreenRenderer m_Renderer;

	/** 追従中に参照する読み込み窓口。非所有で、Unfollowまたは自動完了まで生存させる。同じGameInstanceの寿命で管理する。 */
	const CAssetLoaderSubsystem* m_Followed = nullptr;

	/** 見に行っている読み込みに対応する要求。無効値は要求を持たない追従を示す。 */
	FAssetLoadRequest m_FollowedRequest;

	/** 追従所有者の交代を識別する非0世代。 */
	u64 m_FollowRevision = 1u;

	/** 手動表示と追従表示の交代を識別する非0世代。 */
	u64 m_DisplayRevision = 1u;

	/** 出す指示が生きているか。 */
	bool m_bVisible = false;

};
