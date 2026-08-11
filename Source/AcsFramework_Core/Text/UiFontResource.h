// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/** UI フォントの GPU atlas と読み込み状態を単独所有する通常型。 */
class FUiFontResource final
{
public:
	/** 既定の 20px、CJK 文字を含む設定で空の資源を作る。 */
	FUiFontResource() noexcept = default;

	/** 読み込み済みの GPU atlas を解放する。 */
	~FUiFontResource() noexcept;

	/** GPU 資源の重複所有を防ぐためコピーを禁止する。 */
	FUiFontResource( const FUiFontResource& ) = delete;

	/** GPU 資源の重複所有を防ぐためコピー代入を禁止する。 */
	FUiFontResource& operator=( const FUiFontResource& ) = delete;

	/**
	 * 次回の読み込みに使う文字サイズと収録範囲を設定する。
	 *
	 * @param SizePixels 0 より大きい文字サイズ。
	 * @param bIncludeCjk CJK 文字を収録する場合は true。
	 */
	void Configure( f32 SizePixels, bool bIncludeCjk ) noexcept;

	/** 設定中の文字サイズを返す。 */
	f32 GetSize() const noexcept { return m_SizePixels; }

	/** CJK 文字を収録する設定かを返す。 */
	bool IsIncludeCjk() const noexcept { return m_bIncludeCjk; }

	/** 指定した設定が現在の読み込み対象と一致するかを返す。 */
	bool MatchesConfiguration( f32 SizePixels, bool bIncludeCjk ) const noexcept;

	/**
	 * 現在の設定に合うフォントを用意して返す。
	 *
	 * @param Device GPU atlas を作る描画 device。
	 * @return 利用できるフォント。読み込み失敗時は nullptr。
	 */
	FFont* Acquire( IRhiDevice& Device ) noexcept;

	/** 読み込み済みのフォントを返し、未準備なら nullptr を返す。 */
	FFont* Peek() noexcept { return m_bReady ? &m_Font : nullptr; }

	/** 読み込み済みのフォントを返し、未準備なら nullptr を返す。 */
	const FFont* Peek() const noexcept { return m_bReady ? &m_Font : nullptr; }

	/** フォントを利用できる状態かを返す。 */
	bool IsReady() const noexcept { return m_bReady; }

private:
	/** GPU atlas を持つフォント。 */
	FFont m_Font;

	/** 次回の読み込みに使う文字サイズ。 */
	f32 m_SizePixels = 20.0f;

	/** 読み込み済み atlas の文字サイズ。 */
	f32 m_BakedSize = 0.0f;

	/** CJK 文字を収録する設定。 */
	bool m_bIncludeCjk = true;

	/** 読み込み済み atlas が CJK 文字を含むか。 */
	bool m_bBakedCjk = false;

	/** フォントを利用できる状態か。 */
	bool m_bReady = false;

	/** 現在の設定で読み込みに失敗したか。 */
	bool m_bFailed = false;
};
