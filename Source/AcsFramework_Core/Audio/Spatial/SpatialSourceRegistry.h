// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 鳴っている場所に番号を振り、使い終わったら返してもらう係。
 *
 * @details
 * エンジン (CSpatialAudio) は「番号で位置を更新する」形なので、番号の採り方と返し方は
 * 呼ぶ側の仕事になっている。各所で好きな番号を使うと、別のものが同じ番号を掴む。
 *
 * 返された番号は使い回す。使い回さないと、長く遊ぶうちに番号が尽きる。
 */
class CSpatialSourceRegistry
{
public:
	/**
	 * 新しい番号を借りる。
	 *
	 * @return 借りた番号 (0 は «無効» として使うので返らない)。
	 */
	u32 Acquire() noexcept;

	/**
	 * 番号を返す。
	 *
	 * @details エンジン側からの取り外しは呼ぶ側の仕事 (ここは番号だけを預かる)。
	 * @param Id 返す番号。
	 */
	void Release( u32 Id ) noexcept;

	/** いま貸している数を返す。 */
	usize GetActiveCount() const noexcept { return m_ActiveCount; }

	/** これまでに貸した延べ数を返す。 */
	u64 GetAcquiredCount() const noexcept { return m_AcquiredCount; }

private:
	/** 返ってきて再び貸せる番号。 */
	TArray<u32> m_Free;

	/** まだ一度も貸していない次の番号。 */
	u32 m_NextId = 1u;

	/** いま貸している数。 */
	usize m_ActiveCount = 0u;

	/** これまでに貸した延べ数。 */
	u64 m_AcquiredCount = 0u;
};
