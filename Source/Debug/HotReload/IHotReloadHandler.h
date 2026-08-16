// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 差し替わったファイルを受け取る差込口。
 *
 * @details
 * 何を作り直すかは、そのアセットを持っている側にしか分からない。監視の側は「変わった」と
 * 伝えるだけにして、作り直しは実装した側が引き受ける。
 *
 * `CanHandle` で自分の担当かを答え、`OnFileChanged` で作り直す。担当外を弾く判定を
 * 分けてあるので、配る側は「誰の担当か」を知らずに済む。
 *
 * @code
 * class CMyTextures : public IHotReloadHandler
 * {
 *     bool CanHandle( const FHotReloadEvent& Event ) const noexcept override;
 *     void OnFileChanged( const FHotReloadEvent& Event ) noexcept override;
 * };
 * @endcode
 */
class IHotReloadHandler
{
public:
	/** 派生を正しく破棄するための仮想デストラクタ。 */
	virtual ~IHotReloadHandler() noexcept = default;

	/**
	 * この変更が自分の担当かを返す。
	 *
	 * @details 拡張子や置き場所で判断する。ここでは作り直さない。
	 * @param Event 起きた変更。
	 * @return 自分が引き受けるなら true。
	 */
	virtual bool CanHandle( const FHotReloadEvent& Event ) const noexcept = 0;

	/**
	 * 変わったものを作り直す。
	 *
	 * @details CanHandle が true を返したときだけ呼ばれる。
	 * @param Event 起きた変更。
	 */
	virtual void OnFileChanged( const FHotReloadEvent& Event ) noexcept = 0;
};
