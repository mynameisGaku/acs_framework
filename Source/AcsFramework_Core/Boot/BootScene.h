// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>
using namespace acs;

/** 利用側が開始シーンを差し替えない場合に使う、処理を持たない起動シーン。 */
class ABootScene : public AScene
{
public:
	/** 起動シーンへ入り、標準では別のシーンへ遷移しない。 */
	void OnEnter() noexcept override;
};
