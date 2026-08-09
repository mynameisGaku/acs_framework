#pragma once

#include <acs.h>

#include "Debug/DebugTop/Render/DebugTopDraw.h"
#include "Debug/DebugTop/Widget/DebugTopToast.h"

using namespace acs;

/** 同時に出せる通知の数。これを超えると古いものから閉じる。 */
inline constexpr usize kDebugTopToastMax = 3;

/**
 * 画面の右下へ出す通知をまとめて面倒を見るサブシステム。
 *
 * @details
 * 新しいものが一番下に出て、既にあるものは上へずれる。同時に 3 件まで持ち、それを超えると
 * 一番古いものから閉じる。GameInstance スコープなので、シーンを切り替えても出したものは
 * 残り続ける (遷移した先で「保存しました」が消えないため)。
 *
 * 出し方はこれだけ。
 * @code
 * DebugTopNotify( "保存しました", Path );
 * DebugTopNotifyError( "読み込みに失敗しました", Reason );
 * @endcode
 *
 * ボタンを付けるときは、そのまま続けて書ける。
 * @code
 * DebugTopNotifySuccess( "保存しました", Path )
 *     .AddButton( FString( "フォルダを開く" ),
 *                 FSimpleDelegate::CreateRaw<&AMyScene::OpenSaveFolder>( this ) );
 * @endcode
 *
 * 更新と描画は ADebugTopHUD が呼ぶ。描画にフォントが要るので、通知自体は画面を持つ側に任せる。
 */
class CDebugTopToastSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CDebugTopToastSubsystem )

	/** 自身を簡易関数から引ける場所へ登録する。 */
	CDebugTopToastSubsystem();

	/** 登録を外す。 */
	~CDebugTopToastSubsystem() override;

	/**
	 * 簡易関数が使う実体を返す。
	 *
	 * @details 生成されていなければ nullptr。
	 * @return 生きているサブシステム。
	 */
	static CDebugTopToastSubsystem* GetActive() noexcept { return s_Active; }

	/**
	 * 通知を 1 件出す。
	 *
	 * @details
	 * 既に 3 件出ていれば、一番古いものを閉じてから足す。返した参照へ続けてボタンを足せる。
	 * @param Kind 通知の種類。
	 * @param Title 見出し。
	 * @param Message 本文 (空なら出さない)。
	 * @return 出した通知。
	 */
	CDebugTopToast& Push( EDebugTopToastKind Kind, const FString& Title, const FString& Message );

	/** 出ている通知を全て閉じ始める。 */
	void DismissAll() noexcept;

	/** 出ている通知を返す (古い順)。 */
	const TArray<TSharedPtr<CDebugTopToast>>& GetToasts() const noexcept { return m_Toasts; }

	/**
	 * 通知を 1 フレーム進める。
	 *
	 * @details
	 * 押されたボタンの処理もここで行う。通知は画面の一番手前に出るので、後ろのメニューが
	 * 同じクリックを拾わないよう、受け取ったかどうかを返す。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 * @return 通知がクリックを受け取ったなら true。
	 */
	bool Update( f32 DeltaSeconds );

	/**
	 * 通知を描く。
	 *
	 * @details メニューより後に呼ぶこと (通知は一番手前に出るため)。
	 * @param RenderContext 画面サイズの取得元。
	 * @param Batch 描画コマンドを積む先。
	 * @param Text 描画に使うフォントと文字サイズ。
	 */
	void Draw( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text );

	/**
	 * 画面右下で通知が占めている高さを返す。
	 *
	 * @details
	 * 同じ右下へ出すもの (説明文) が重ならないよう、避ける量を教えるためのもの。直前の
	 * 描画で積み上げた高さなので、通知が無ければ 0 になる。
	 * @return 占めている高さ (ピクセル)。
	 */
	f32 GetOccupiedHeight() const noexcept { return m_OccupiedHeight; }

private:
	/** 簡易関数から引くための実体。生成時に自分を入れ、破棄時に外す。 */
	static CDebugTopToastSubsystem* s_Active;

	/** 出ている通知 (古い順。末尾が一番新しい = 一番下に出る)。 */
	TArray<TSharedPtr<CDebugTopToast>> m_Toasts;

	/** 直前の描画で下から積み上げた高さ。 */
	f32 m_OccupiedHeight = 0.0f;
};


/**
 * どこも参照していないときに返す捨て場。
 *
 * @details
 * サブシステムが無い場面で簡易関数を呼んでも書き方を変えずに済むよう、必ず参照を返せる
 * ようにするための置き場。ここへ出した通知はどこにも表示されない。
 * @return 捨て場の通知。
 */
CDebugTopToast& DebugTopToastSink();

/**
 * お知らせの通知を出す。
 *
 * @param Title 見出し。
 * @param Message 本文 (省略可)。
 * @return 出した通知 (続けて AddButton を書ける)。
 */
CDebugTopToast& DebugTopNotify( const FString& Title, const FString& Message = FString() );

/**
 * うまくいったことの通知を出す。
 *
 * @param Title 見出し。
 * @param Message 本文 (省略可)。
 * @return 出した通知。
 */
CDebugTopToast& DebugTopNotifySuccess( const FString& Title, const FString& Message = FString() );

/**
 * 気に留めてほしいことの通知を出す。
 *
 * @param Title 見出し。
 * @param Message 本文 (省略可)。
 * @return 出した通知。
 */
CDebugTopToast& DebugTopNotifyWarning( const FString& Title, const FString& Message = FString() );

/**
 * 失敗したことの通知を出す。
 *
 * @param Title 見出し。
 * @param Message 本文 (省略可)。
 * @return 出した通知。
 */
CDebugTopToast& DebugTopNotifyError( const FString& Title, const FString& Message = FString() );
