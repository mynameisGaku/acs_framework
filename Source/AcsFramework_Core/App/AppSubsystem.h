// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * アプリ本体への窓口。
 *
 * @details
 * 終了を頼む、どれくらいの速さで回っているかを聞く、といった「アプリそのもの」への用事を
 * まとめる。どれも CApplication が答えられるが、その参照は普通のゲームコードからは辿れない
 * ので、この層で受け取って GetSubsystem<CAppSubsystem>() から頼めるようにする。
 *
 * 窓の見え方は扱わない (CScreenSubsystem が持つ)。ゲームの時間の進み方も扱わない
 * (CTimeSubsystem が持つ)。
 *
 * **経過秒に時間の倍率は掛かっていない。** 止めていてもスローにしていても、ここが返すのは
 * 実時間そのもの。幕やメニューのように「止まっていても動き続けるもの」はこちらを使う。
 */
class CAppSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CAppSubsystem )

	/**
	 * アプリ本体を受け取る。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。渡さない間は何を頼まれても何も起きない。
	 * @param Application アプリ本体。
	 */
	void Bind( CApplication& Application ) noexcept { m_Application = &Application; }

	/**
	 * アプリを終わらせるよう頼む。
	 *
	 * @details
	 * その場では終わらない。いま処理しているフレームを終えてから畳まれるので、呼んだ後の
	 * 行も普通に走る。
	 */
	void Quit() noexcept;

	/**
	 * 1 秒あたりのフレーム数の目安を返す。
	 *
	 * @return フレーム毎秒 (まだ配線されていなければ 0)。
	 */
	f32 GetFps() const noexcept;

	/**
	 * 前フレームからの実経過秒を返す。
	 *
	 * @details 時間の倍率 (CTimeSubsystem) は掛かっていない。
	 * @return 経過秒 (まだ配線されていなければ 0)。
	 */
	f32 GetUnscaledDeltaSeconds() const noexcept;

	/**
	 * 起動からのフレーム数を返す。
	 *
	 * @return フレーム数 (まだ配線されていなければ 0)。
	 */
	u64 GetFrameCount() const noexcept;

private:
	/** アプリ本体。所有はしない。 */
	CApplication* m_Application = nullptr;
};
