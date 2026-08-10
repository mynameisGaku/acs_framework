#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/SceneTransition.h"

using namespace acs;

/**
 * シーンの切り替えを、どこからでも頼めるようにするサブシステム。
 *
 * @details
 * 切り替えそのものはエンジン (CSceneManager::ChangeScene / CGame::TransitionTo) が行う。
 * ただし CGame への参照は普通のゲームコードからは辿れないので、この層で受け取って
 * GetSubsystem<CSceneTravelSubsystem>() から頼めるようにする。
 *
 * 幕を張るかどうかを ESceneTransition で選べる以外は、エンジンの遷移そのまま。
 * 幕の進行も描画もエンジンが持っているので、ここは幕の状態を持たない
 * (幕そのものを触りたいときは CFadeSubsystem を使う)。
 *
 * @code
 * if ( CSceneTravelSubsystem* Travel = GetSubsystem<CSceneTravelSubsystem>() )
 * {
 *     Travel->TravelTo( MakeUnique<AMyScene>() );                            // 暗転して切替
 *     Travel->TravelTo( MakeUnique<AMyScene>(), ESceneTransition::Cut );     // その場で切替
 * }
 * @endcode
 */
class CSceneTravelSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CSceneTravelSubsystem )

	/**
	 * 切り替えの持ち主を配線する。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。配線前の呼び出しは黙って無視される。
	 * @param Game シーンを持っている CGame。
	 */
	void Bind( CGame& Game ) noexcept { m_Game = &Game; }

	/**
	 * シーンを切り替える。幕を張るかどうかはここで決める。
	 *
	 * @details
	 * Fade を指定すると、切り替えは暗転しきった時点で行われるので繋ぎ目が見えない。しかも
	 * 明転はエンジンが行うため、遷移先のシーンにフェード明けの処理を書かなくてよい。
	 * Cut は幕を使わずその場で切り替える (演出を遷移先が自分で作りたいとき)。
	 * @param Next 切り替え先のシーン。
	 * @param Transition 見せ方 (既定は暗転して切り替える)。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void TravelTo( TUniquePtr<AScene> Next, ESceneTransition Transition = ESceneTransition::Fade, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/**
	 * 持たせるものを添えてシーンを切り替える。
	 *
	 * @details
	 * 遷移先は OnEnter の時点で TravelContext<T>() から読める。遷移元が遷移先の型を知らずに
	 * 済むので、コンストラクタ引数で渡すより結びつきが弱くなる。
	 * 遷移が成立しなかった場合、持たせたものは捨てられる。
	 * @param Next 切り替え先のシーン。
	 * @param Context 遷移先へ持たせるもの (所有権が移る。空でもよい)。
	 * @param Transition 見せ方。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void TravelTo( TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition = ESceneTransition::Fade, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/**
	 * いまのシーンを残したまま、上へ重ねる。
	 *
	 * @details
	 * 切り替えと違い、下のシーンは畳まれずに残る (下は止まり、OnPause が呼ばれる)。戻ってきた
	 * ときに元の続きから始まるので、ポーズ画面・ダイアログ・リザルトのように「後で元へ戻る」
	 * ものはこちらを使う。
	 *
	 * 既定は幕なし。重ねるものは即座に出る方が自然なため。
	 * @param Next 上へ重ねるシーン。
	 * @param Transition 見せ方 (既定は幕なし)。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void PushScene( TUniquePtr<AScene> Next, ESceneTransition Transition = ESceneTransition::Cut, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/**
	 * 持たせるものを添えて重ねる。
	 *
	 * @details 重ねる側が「何のために開いたか」を渡す用途 (どの項目を選んで開いたか等)。
	 * @param Next 上へ重ねるシーン。
	 * @param Context 重ねる先へ持たせるもの (所有権が移る。空でもよい)。
	 * @param Transition 見せ方。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void PushScene( TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition = ESceneTransition::Cut, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/**
	 * 重ねたシーンを下ろして、1 つ下へ戻る。
	 *
	 * @details
	 * 残しておいた下のシーンがそのまま続く (OnResume が呼ばれる)。1 枚しか積んでいなければ
	 * 何も起きない (最後の 1 枚を下ろすと画面が無くなるため)。
	 * @param Transition 見せ方 (既定は幕なし)。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void PopScene( ESceneTransition Transition = ESceneTransition::Cut, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/**
	 * 結果を持たせて下ろす。
	 *
	 * @details
	 * **重ねた画面の «答え» を戻す口。** 「続ける / タイトルへ戻る」「はい / いいえ」のように、
	 * 重ねた側が決めたことを下のシーンへ返す。戻り先は OnResume の時点で
	 * TravelContext<T>() から読める。
	 * 下ろせなかった場合 (1 枚しか積んでいない)、持たせたものは捨てられる。
	 * @param Context 戻り先へ持たせるもの (所有権が移る。空でもよい)。
	 * @param Transition 見せ方。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void PopScene( TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition = ESceneTransition::Cut, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/**
	 * 積んでいるシーンの枚数を返す。
	 *
	 * @return 枚数 (配線前は 0)。
	 */
	u32 GetDepth() const noexcept;

	/** 重ねたものを下ろせるか (2 枚以上積んでいるか) を返す。 */
	bool CanPop() const noexcept { return GetDepth() > 1; }

	/**
	 * 1 フレーム進める。
	 *
	 * @details
	 * 幕を張って積み下ろしするときだけ意味がある。暗転しきってから実際に積み下ろし、
	 * そのあと明転させる。アプリの更新から毎フレーム呼ぶ。
	 */
	void Update() noexcept;

private:
	/**
	 * 幕が下りきるのを待っている操作。
	 */
	enum class EPending : u8
	{
		/** 待っているものは無い。 */
		None,

		/** 暗転しきったら重ねる。 */
		Push,

		/** 暗転しきったら下ろす。 */
		Pop,
	};

	/** シーンを持っている CGame。所有はしない (アプリが所有する)。 */
	CGame* m_Game = nullptr;

	/** 暗転しきってから重ねるシーン (Pop 待ちのときは空)。 */
	TUniquePtr<AScene> m_PendingScene;

	/** 暗転しきってから一緒に渡すもの (無ければ空)。 */
	TUniquePtr<CSceneTravelContext> m_PendingContext;

	/** 幕が下りきるのを待っている操作。 */
	EPending m_Pending = EPending::None;

	/** 積み下ろした後の明転にかける秒数。 */
	f32 m_PendingInSeconds = 0.3f;
};
