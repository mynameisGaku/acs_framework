// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 言語が変わったときに張り替える側が申告する口。
 *
 * @details
 * 引いた文を持ち回している画面は、言語が変わった瞬間に**古い文を持ったまま**になる。
 * 気付く手立てが無いと、設定で言語を変えても一部だけ前の言語のままになる。
 *
 * 差込口の向きは他のモジュールと揃えてある ―― **張り替えたい側が申告する**。
 * 言語の側が、誰が居るかを知る必要は無い。
 *
 * @code
 * class CTitlePage final : public ILocaleChangeListener
 * {
 *     void OnLocaleChanged( ELocale Locale ) noexcept override { RebuildLabels(); }
 * };
 * @endcode
 */
class ILocaleChangeListener
{
public:
	virtual ~ILocaleChangeListener() noexcept = default;

	/**
	 * 言語が変わったときに呼ばれる。
	 *
	 * @details
	 * **この中で足したり外したりしない。** 配っている最中に並びが変わると、飛ばされる相手が出る。
	 * @param Locale 変わった後の言語。
	 */
	virtual void OnLocaleChanged( ELocale Locale ) noexcept = 0;
};
