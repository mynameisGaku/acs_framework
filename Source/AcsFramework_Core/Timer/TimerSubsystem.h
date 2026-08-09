// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 仕掛けたタイマーの控え。
 *
 * @details
 * 止められる時計と実時間の時計は別物なので、どちらに仕掛けたかまで含めて持つ。これが無いと
 * 取り消すときにどちらを探せばよいか分からない。
 */
struct FGameTimer
{
	/** Engine のタイマーを参照する値。無効な値では取り消しや状態取得を行わない。 */
	acs::FTimerHandle Handle{};

	/** 実時間の時計に仕掛けたか (false ならゲーム時間)。 */
	bool bUnscaled = false;

	/** 仕掛かっているものを指せる値かを返す。 */
	bool IsValid() const noexcept { return Handle.IsValid(); }
};


/**
 * 「n 秒後に呼ぶ」を、どこからでも頼めるようにするサブシステム。
 *
 * @details
 * 仕組みそのものはエンジン (CTimerManager) が持っている。ただしアプリが持っている実体は
 * 実時間で進むので、それをそのまま配るとゲームを止めても敵の出現やクールダウンが進んでしまう。
 *
 * そこでこの層で時計を 2 つ持つ。
 *
 * | 時計 | 進み方 | 向く用途 |
 * |---|---|---|
 * | ゲーム時間 (既定) | 時間の倍率が乗る。止めれば止まる | 敵の出現、クールダウン、演出 |
 * | 実時間 | 倍率に関わらず進む | 通信の再試行、自動保存、ポーズ画面の演出 |
 *
 * どちらに仕掛けたかは FGameTimer が覚えているので、取り消すときに呼び分けなくてよい。
 *
 * 進める材料 (実経過秒と倍率) は自分では取りに行かず、アプリから渡してもらう。時間の担当を
 * 直接見に行くと、時計の話と止め方の話が絡まる。
 *
 * @code
 * if ( CTimerSubsystem* Timers = GetSubsystem<CTimerSubsystem>() )
 * {
 *     // 3 秒後に 1 回 (止めている間は進まない)
 *     m_Spawn = Timers->After( 3.0f, FSimpleDelegate::CreateRaw<&AMyScene::Spawn>( this ) );
 *
 *     // 30 秒ごとに (止めていても進む)
 *     Timers->EveryUnscaled( 30.0f, FSimpleDelegate::CreateRaw<&AMyScene::AutoSave>( this ) );
 *
 *     Timers->Cancel( m_Spawn );
 * }
 * @endcode
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
	 * @param Timer 取り消す控え。
	 * @return 取り消せたら true (既に呼ばれた後なら false)。
	 */
	bool Cancel( const FGameTimer& Timer ) noexcept;

	/**
	 * まだ仕掛かっているかを返す。
	 *
	 * @param Timer 調べる控え。
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
	 * アプリの更新から毎フレーム呼ぶ。止まっている間も呼ぶこと (実時間の時計は進めたいため)。
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
