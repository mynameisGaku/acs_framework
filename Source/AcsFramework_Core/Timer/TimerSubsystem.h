// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "GameTimer.h"

using namespace acs;
/**
 * GameInstance に属する予約をゲーム時間と実時間で更新し、取消しと照会の窓口を提供する。
 * 返す FGameTimer は生成元の CTimerSubsystem 実体内だけで使い、別実体の値とは相互利用しない。
 * 所有者スコープの終了と本型のデストラクタは、それぞれが追跡する残存予約を回収する。
 */
class CTimerSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CTimerSubsystem )

	/** 仕掛かっているものを全て捨てる。 */
	~CTimerSubsystem() noexcept override;

	/**
	 * ゲーム時間で n 秒後に 1 回呼ぶ。
	 *
	 * @details 時間を止めている間は進まない。遅くしていればその分だけ延びる。
	 * @param Seconds 何秒後か。
	 * @param Delegate 呼ぶもの。
	 * @return 仕掛けた控え (取り消しに使う)。
	 */
	FGameTimer After( f32 Seconds, FSimpleDelegate Delegate );

	/**
	 * ゲーム時間で n 秒ごとに呼び続ける。
	 *
	 * @param Seconds 何秒ごとか。
	 * @param Delegate 呼ぶもの。
	 * @return 仕掛けた控え (取り消しに使う)。
	 */
	FGameTimer Every( f32 Seconds, FSimpleDelegate Delegate );

	/**
	 * 実時間で n 秒後に 1 回呼ぶ。
	 *
	 * @details 時間を止めていても進む。
	 * @param Seconds 何秒後か。
	 * @param Delegate 呼ぶもの。
	 * @return 仕掛けた控え (取り消しに使う)。
	 */
	FGameTimer AfterUnscaled( f32 Seconds, FSimpleDelegate Delegate );

	/**
	 * 実時間で n 秒ごとに呼び続ける。
	 *
	 * @param Seconds 何秒ごとか。
	 * @param Delegate 呼ぶもの。
	 * @return 仕掛けた控え (取り消しに使う)。
	 */
	FGameTimer EveryUnscaled( f32 Seconds, FSimpleDelegate Delegate );

	/**
	 * 仕掛けたものを取り消す。
	 *
	 * @param Timer この CTimerSubsystem 実体が返した、取り消す控え。
	 * @return 取り消せたら true (既に呼ばれた後なら false)。
	 */
	bool Cancel( const FGameTimer& Timer ) noexcept;

	/**
	 * まだ仕掛かっているかを返す。
	 *
	 * @param Timer この CTimerSubsystem 実体が返した、調べる控え。
	 * @return 仕掛かっていれば true。
	 */
	bool IsActive( const FGameTimer& Timer ) const noexcept;

	/** 仕掛かっているものを全て取り消す。 */
	void CancelAll() noexcept;

	/** ゲーム時間で仕掛かっている数を返す。 */
	u32 GetActiveCount() const noexcept;

	/** 実時間で仕掛かっている数を返す。 */
	u32 GetActiveUnscaledCount() const noexcept;

	/**
	 * 1 フレーム進める。
	 *
	 * @details
	 * 実時間の時計は UnscaledDeltaSeconds で常時進み、ゲーム時間の時計だけに TimeScale を適用する。
	 * @param UnscaledDeltaSeconds 前フレームからの実経過秒。
	 * @param TimeScale いま掛かっている時間の倍率 (CTimeSubsystem::GetEffectiveScale)。
	 */
	void Update( f32 UnscaledDeltaSeconds, f32 TimeScale ) noexcept;

private:
	/** ゲーム時間で進む時計。 */
	CTimerManager m_GameTimers;

	/** 実時間で進む時計。 */
	CTimerManager m_UnscaledTimers;
};
