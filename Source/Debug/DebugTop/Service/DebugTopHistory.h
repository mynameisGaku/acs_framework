#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElementValue.h"

using namespace acs;

class ADebugTopHUD;
class CDebugTopElement;

/**
 * 値の変更を控えて、取り消し / やり直しを行う係。
 *
 * @details
 * デバッグメニューは「とりあえず動かしてみる」ための道具なので、行き過ぎたときに 1 手だけ
 * 戻せることが要る。長押しで送れるようになったぶん、行き過ぎやすくもなっている。
 *
 * 変更は行の側から知らせてもらう (CDebugTopElement::SetChangeListener)。行を触る全ての経路
 * (キー・マウス・パッド・ショートカット・既定値へ戻す) がここを通るので、経路が増えても
 * 手を入れずに済む。
 *
 * **連続した変更はひとまとめにする。** 長押しで 1 秒送ると十数回の変更になるが、取り消しは
 * 「送る前」へ 1 手で戻したい。同じ行への変更が続いている間は 1 つの手として畳む。
 *
 * 控えるのは値だけ。ページの開閉・ピン留め・カーソル位置は対象外 (戻して困るものではなく、
 * 戻ると却って迷うため)。
 */
class CDebugTopHistory
{
public:
	/** 何も控えていない状態で構築する。 */
	CDebugTopHistory() noexcept = default;

	/** 行からの通知を外す。 */
	~CDebugTopHistory() noexcept;

	/**
	 * 控え始める。
	 *
	 * @details
	 * メニュー全体を 1 度辿って、いまの値を「変更前の値」として控えておく。これが無いと
	 * 最初の 1 手を戻せない (変更の通知は変わった後に来るため)。
	 * @param HUD 控える対象のメニュー。
	 */
	void Begin( const ADebugTopHUD& HUD );

	/**
	 * 1 フレーム進める。
	 *
	 * @details 変更が途切れたかを測り、途切れていたら次の変更は別の手として積む。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( f32 DeltaSeconds ) noexcept;

	/** 戻せる手があるかを返す。 */
	bool CanUndo() const noexcept { return m_Undo.Num() > 0; }

	/** やり直せる手があるかを返す。 */
	bool CanRedo() const noexcept { return m_Redo.Num() > 0; }

	/**
	 * 直前の変更を取り消す。
	 *
	 * @param OutLabel 戻した行の表示名を書き込む先 (通知に出すため)。
	 * @return 戻せたら true。
	 */
	bool Undo( FString& OutLabel );

	/**
	 * 取り消した変更をやり直す。
	 *
	 * @param OutLabel 戻した行の表示名を書き込む先 (通知に出すため)。
	 * @return やり直せたら true。
	 */
	bool Redo( FString& OutLabel );

	/** 控えを全て捨てる (設定の読み込み直後など、履歴が意味を失ったときに呼ぶ)。 */
	void Clear() noexcept;

private:
	/**
	 * 変更 1 手ぶんの控え。
	 *
	 * @details
	 * 行は所有しない。ページは組み立てた後ずっと生きるため、生ポインタで足りる。それでも
	 * 戻すときには木を辿って生存を確かめる (ページを作り直す作りに変えても壊れないように)。
	 */
	struct FEntry
	{
		/** 変わった行。所有はしない。 */
		CDebugTopElement* Element = nullptr;

		/** 変わる前の値。 */
		FDebugTopElementValue Before;

		/** 変わった後の値 (やり直しに使う)。 */
		FDebugTopElementValue After;

		/** 通知へ出す表示名 (行が消えていても出せるよう控えておく)。 */
		FString Label;
	};

	/**
	 * 行の値が変わったときに呼ばれる。
	 *
	 * @param Element 変わった行。
	 */
	void OnElementChanged( CDebugTopElement& Element );

	/**
	 * 控えの山から 1 手取り出して書き戻す。
	 *
	 * @param From 取り出す山。
	 * @param To 取り出した手を積み直す山。
	 * @param bUseBefore 変更前の値へ戻すなら true、変更後の値へ進めるなら false。
	 * @param OutLabel 戻した行の表示名を書き込む先。
	 * @return 書き戻せたら true。
	 */
	bool Step( TArray<FEntry>& From, TArray<FEntry>& To, bool bUseBefore, FString& OutLabel );

	/**
	 * その行がまだメニューの中に居るかを返す。
	 *
	 * @param Element 確かめる行。
	 * @return 居れば true。
	 */
	bool IsAlive( const CDebugTopElement& Element ) const noexcept;

	/** 控える対象のメニュー。所有はしない。 */
	const ADebugTopHUD* m_HUD = nullptr;

	/** 行ごとの「いまの値」。次に変わったとき、これが変更前の値になる。 */
	THashMap<const CDebugTopElement*, FDebugTopElementValue> m_Current;

	/** 戻せる手の山 (末尾が直前の手)。 */
	TArray<FEntry> m_Undo;

	/** やり直せる手の山 (末尾が直前に戻した手)。 */
	TArray<FEntry> m_Redo;

	/** いま畳んでいる手の行。所有はしない (nullptr なら畳んでいる最中ではない)。 */
	const CDebugTopElement* m_MergeElement = nullptr;

	/** 最後に変更があってからの経過秒 (途切れたかの判定に使う)。 */
	f32 m_IdleSeconds = 0.0f;

	/** 書き戻している最中か (自分が起こした変更を控えないための札)。 */
	bool m_bApplying = false;
};
