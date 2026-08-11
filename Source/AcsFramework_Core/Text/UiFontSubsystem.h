// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Text/UiFontResource.h"

using namespace acs;

/** GameInstance の寿命で共有 UI フォントを所有し、利用側へ公開するサブシステム。 */
class CUiFontSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CUiFontSubsystem )

	/**
	 * 文字の大きさを設定する。
	 *
	 * @details 変えると次に Acquire したときに焼き直す。
	 * @param SizePixels 文字のピクセルサイズ。
	 */
	void SetSize( f32 SizePixels ) noexcept;

	/** 文字の大きさを返す。 */
	f32 GetSize() const noexcept { return m_Resource.GetSize(); }

	/**
	 * 漢字を焼き込むかを設定する。
	 *
	 * @details
	 * 焼くとアトラスが 4096 四方まで大きくなり、最初の 1 回に数百 ms かかる。日本語を出すなら
	 * 必要。既定は true (黙って文字が消えるより、起動が少し重い方がましなため)。
	 * @param bIncludeCjk 焼き込むなら true。
	 */
	void SetIncludeCjk( bool bIncludeCjk ) noexcept;

	/** 漢字を焼き込む設定かを返す。 */
	bool IsIncludeCjk() const noexcept { return m_Resource.IsIncludeCjk(); }

	/**
	 * フォントを用意して返す (必要なら焼く)。
	 *
	 * @details
	 * 焼くのに描画資源が要るので、これを呼べるのは描画側だけ。毎フレーム呼んでよい
	 * (設定が変わっていなければ焼き直さない)。
	 * @param Renderer 描画資源の取得元。
	 * @return 使えるフォント (用意できなければ nullptr)。
	 */
	FFont* Acquire( CRenderer& Renderer ) noexcept;

	/**
	 * 既に焼けているフォントを返す (焼かない)。
	 *
	 * @details
	 * 描画資源を持たない場所 (シーンの HUD 描画など) から使う。まだ焼けていなければ nullptr。
	 * @return 焼けているフォント (無ければ nullptr)。
	 */
	FFont* Peek() noexcept { return m_Resource.Peek(); }

	/** 焼けているかを返す。 */
	bool IsReady() const noexcept { return m_Resource.IsReady(); }

private:
	/** UI フォントの GPU atlas と読み込み状態。 */
	FUiFontResource m_Resource;
};
