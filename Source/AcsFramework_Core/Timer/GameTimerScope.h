// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "GameTimer.h"

using namespace acs;

class CTimerSubsystem;

/** シーンまたは所有者単位で登録したタイマーをまとめて取り消す通常型。 */
class CGameTimerScope final
{
public:
	/** GameInstance側のタイマー窓口を非所有で受け取る。呼出し側は窓口を生存させる。 */
	explicit CGameTimerScope( CTimerSubsystem& TimerService ) noexcept;

	/** 追跡中のタイマーをすべて取り消す。窓口は自身より長く生存する必要がある。 */
	~CGameTimerScope() noexcept;

	/** 所有者の二重管理を防ぐためコピー構築を禁止する。 */
	CGameTimerScope( const CGameTimerScope& ) = delete;

	/** 所有者の二重管理を防ぐためコピー代入を禁止する。 */
	CGameTimerScope& operator=( const CGameTimerScope& ) = delete;

	/** 追跡配列の二重管理を防ぐためムーブ構築を禁止する。 */
	CGameTimerScope( CGameTimerScope&& ) = delete;

	/** 追跡配列の二重管理を防ぐためムーブ代入を禁止する。 */
	CGameTimerScope& operator=( CGameTimerScope&& ) = delete;

	/** ゲーム時間で一度だけ呼ぶ処理を登録し、成功した値を追跡する。失敗時は無効値を返す。 */
	FGameTimer After( f32 Seconds, FSimpleDelegate Delegate );

	/** ゲーム時間で繰り返す処理を登録し、成功した値を追跡する。失敗時は無効値を返す。 */
	FGameTimer Every( f32 Seconds, FSimpleDelegate Delegate );

	/** 実時間で一度だけ呼ぶ処理を登録し、成功した値を追跡する。失敗時は無効値を返す。 */
	FGameTimer AfterUnscaled( f32 Seconds, FSimpleDelegate Delegate );

	/** 実時間で繰り返す処理を登録し、成功した値を追跡する。失敗時は無効値を返す。 */
	FGameTimer EveryUnscaled( f32 Seconds, FSimpleDelegate Delegate );

	/** コンストラクタで受けた同じ窓口から得た自身の識別子だけを取り消す。同じ窓口の別スコープの値または古い値なら false を返す。 */
	bool Cancel( const FGameTimer& Timer ) noexcept;

	/** コンストラクタで受けた同じ窓口から得た自身の識別子が有効なら true を返す。同じ窓口の別スコープの値または古い値は false。 */
	bool IsActive( const FGameTimer& Timer ) const noexcept;

	/** 自身が追跡する識別子をすべて先に外してから取り消す。複数回呼んでも安全。 */
	void CancelAll() noexcept;

private:
	/** 発火済みまたは外部で取り消された値を追跡配列から除く。 */
	void PruneInactive() noexcept;

	/** 次の値を追跡できる容量を先に確保する。失敗時は窓口へ登録しない。 */
	bool TryReserveForOneMore() noexcept;

	/** 識別番号、世代番号、時計種別が一致するかを返す。 */
	static bool IsSameTimer( const FGameTimer& Left, const FGameTimer& Right ) noexcept;

	/** GameInstance側で管理されるタイマー窓口。 */
	CTimerSubsystem* m_TimerService = nullptr;

	/** この所有者が登録したタイマーの控え。 */
	TArray<FGameTimer> m_OwnedTimers;
};
