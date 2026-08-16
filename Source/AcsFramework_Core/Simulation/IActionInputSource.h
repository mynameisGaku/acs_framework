// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"

using namespace acs;

/**
 * 1 ティックぶんの入力を差し出す口。
 *
 * @details
 * 人の操作でも、AI の判断でも、記録の再生でも、**ここから先は区別が付かない**。
 * ロジックは「誰が操っているか」を知らないまま動く。
 *
 * 実装側はキーやパッドを読んでもよいし、経路計算の結果を返してもよい。
 * どこから来た値でも、同じ列を流せば同じ結果になることが大事。
 *
 * @code
 * bool CEnemyBrain::TryGetActionInput( FActionInput& OutInput ) noexcept override
 * {
 *     OutInput.SetAxis( 0u, m_ToTarget.x );
 *     OutInput.SetDown( kActionFire, m_bInRange );
 *     return true;
 * }
 * @endcode
 */
class IActionInputSource
{
public:
	/** 派生を正しく破棄するための仮想デストラクタ。 */
	virtual ~IActionInputSource() noexcept = default;

	/**
	 * このティックの入力を差し出す。
	 *
	 * @details
	 * 毎ティック呼ばれる。false を返した場合は「入力なし」として扱われ、
	 * 中立の入力が使われる (記録には残る)。
	 * @param OutInput 入力の入れ先。
	 * @return 差し出すなら true。
	 */
	virtual bool TryGetActionInput( FActionInput& OutInput ) noexcept = 0;
};
