// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 書き出し・読み込みで使うバイト列の入れ物。
 *
 * @details
 * エンジンの書き出しは「呼ぶ側が用意した領域へ詰める」形なので、大きさの見積もりと確保は
 * 呼ぶ側の仕事になる。毎回確保し直すと、シーンを保存するたびにヒープを叩くことになるので、
 * ここが持って使い回す。
 *
 * 中身の意味は知らない。詰めるのも読むのも別の係が行う。
 */
class CSceneSnapshotBuffer
{
public:
	/**
	 * 少なくともこの大きさまで使えるようにする。
	 *
	 * @details 足りていれば何もしない。縮めることはない。
	 * @param Bytes 必要な大きさ。
	 * @return 用意できたら true。
	 */
	bool EnsureSize( usize Bytes ) noexcept;

	/** 使える大きさを返す。 */
	usize Size() const noexcept { return m_Bytes.Num(); }

	/** 先頭を返す (まだ何も確保していなければ nullptr)。 */
	u8* Data() noexcept { return m_Bytes.Num() != 0u ? m_Bytes.GetData() : nullptr; }

	/** 先頭を const で返す。 */
	const u8* Data() const noexcept { return m_Bytes.Num() != 0u ? m_Bytes.GetData() : nullptr; }

	/** 中身を捨てる (確保した領域も返す)。 */
	void Release() noexcept { m_Bytes.Reset(); }

private:
	/** バイト列の実体。 */
	TArray<u8> m_Bytes;
};
