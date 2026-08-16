// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

class CAudioSubsystem;

/**
 * 差し込みの一音 (スティンガー) を、実際に鳴らす側へ渡す係。
 *
 * @details
 * CMusicDirector は「鳴らしてほしい一音」を溜めるところまでを引き受け、取り出すのは
 * 呼び出し側の仕事になっている (`ConsumeStinger`)。取り出して渡す係をここへ切り出す。
 *
 * 溜まっているだけ取り出す。取り出さないままにすると、次の一音で上書きされて消える。
 */
class CMusicStingerPump
{
public:
	/**
	 * 溜まっている一音を取り出して鳴らす。
	 *
	 * @param Director 取り出し元。
	 * @param Audio 鳴らす先。
	 * @return 鳴らした数。
	 */
	usize ConsumeInto( CMusicDirector& Director, CAudioSubsystem& Audio ) noexcept;

	/** これまでに鳴らした数を返す。 */
	u64 GetPlayedCount() const noexcept { return m_PlayedCount; }

private:
	/** これまでに鳴らした数。 */
	u64 m_PlayedCount = 0u;
};
