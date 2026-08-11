// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 入れっぱなしの入力を、一定間隔の「1 回ぶん」へ均す。
 *
 * @details
 * 押した瞬間に 1 回出し、そのまま入れ続けると少し待ってから一定間隔で出し続ける。
 * ワープロのキーリピートと同じ感触。行数の多い一覧やカウンタを、キーを叩き続けずに
 * 送れるようにするためのもの。
 *
 * どこから来た入力かは知らない。キーボードでもゲームパッドでもマウスでも、呼び出し側が
 * 「いまどちらへ入っているか」を渡すだけで同じ触り心地になる。入力を読む API に依存しない
 * ので、入力の出所が増えても手を入れる必要はない。
 *
 * 待ち時間と間隔は用途ごとに変えてよい。押しっぱなしで大きく動かしたい所は短く、
 * 1 つずつ選びたい所は長めにする。
 */
class FInputRepeat
{
public:
	/** 既定の間隔 (押し始めから連射に入るまで 0.4 秒、以降 0.08 秒ごと) で構築する。 */
	FInputRepeat() noexcept = default;

	/**
	 * 間隔を指定して構築する。
	 *
	 * @param DelaySeconds 入れ始めてから連射に入るまでの待ち (秒)。
	 * @param IntervalSeconds 連射の間隔 (秒)。
	 */
	FInputRepeat( f32 DelaySeconds, f32 IntervalSeconds ) noexcept;

	/**
	 * 間隔を設定する。
	 *
	 * @details 入れている最中に変えてもよい (次の 1 回から効く)。
	 * @param DelaySeconds 入れ始めてから連射に入るまでの待ち (秒)。
	 * @param IntervalSeconds 連射の間隔 (秒)。
	 */
	void SetTiming( f32 DelaySeconds, f32 IntervalSeconds ) noexcept;

	/**
	 * 1 フレーム進めて、この回に出す量を返す。
	 *
	 * @details
	 * 離した / 向きが変わったときは、待たずにその場で 1 回出して測り直す。入れっぱなしの
	 * 間は、待ち時間を過ぎてから一定間隔で出す。
	 * @param Raw いま入っている向き (0 なら入れていない)。符号と大きさはそのまま返る。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 * @return この回に効かせる量 (0 なら何もしない)。
	 */
	i32 Step( i32 Raw, f32 DeltaSeconds ) noexcept;

	/**
	 * 押しっぱなしのボタン 1 つを 1 フレーム進めて、この回に効かせるかを返す。
	 *
	 * @details 向きを持たない操作 (ボタン連打) 用。Step( i32, f32 ) の言い換え。
	 * @param bHeld いま押されているか。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 * @return この回に効かせるなら true。
	 */
	bool StepHeld( bool bHeld, f32 DeltaSeconds ) noexcept { return Step( bHeld ? 1 : 0, DeltaSeconds ) != 0; }

	/**
	 * 入れていない状態へ戻す。
	 *
	 * @details
	 * 掴んでいたものを離した、画面が切り替わった等で連射を断ち切りたいときに呼ぶ。
	 * 次に入れたときは、また待ち時間から始まる。
	 */
	void Reset() noexcept;

	/** 待ち時間を過ぎて連射に入っているかを返す。 */
	bool IsRepeating() const noexcept { return m_bRepeating; }

private:
	/** 入れ始めてから連射に入るまでの待ち (秒)。短すぎると 1 つだけ動かしたいときに行き過ぎる。 */
	f32 m_DelaySeconds = 0.4f;

	/** 連射の間隔 (秒)。 */
	f32 m_IntervalSeconds = 0.08f;

	/** 直前に入っていた向き。 */
	i32 m_Last = 0;

	/** 同じ向きを入れ続けている秒数。 */
	f32 m_Timer = 0.0f;

	/** 待ち時間を過ぎて連射に入っているか。 */
	bool m_bRepeating = false;
};
