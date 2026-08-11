// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Text/UiFontResource.h"
#include "Debug/DebugTop/Render/DebugTopDraw.h"

using namespace acs;

/**
 * メニューが使う文字を、指定のサイズと収録範囲で焼いて持っておくもの。
 *
 * @details
 * エンジン共有の UI フォントは 18px の ASCII と仮名しか焼いていないので、大きく出すと
 * にじみ、漢字は無言で消える。そこで指定されたサイズ専用のアトラスを焼き直して使う。
 *
 * 焼くのは高いので、サイズか収録範囲が変わったときだけ焼き直す。失敗しても毎フレーム
 * やり直さず、共有フォントの拡縮で見た目を保つ。
 *
 * 何を描くかは知らない。渡すのは「この文脈で描くならこれを使え」という CDebugTopText だけ。
 */
class CDebugTopFontCache
{
public:
	/** 空で構築する (最初に Resolve を呼んだときに焼く)。 */
	CDebugTopFontCache() noexcept = default;

	/** 専用フォント資源を解放する。 */
	~CDebugTopFontCache() noexcept;

	/** コピー禁止 (GPU 資源を単独所有するため)。 */
	CDebugTopFontCache( const CDebugTopFontCache& ) = delete;

	/** コピー代入も禁止。 */
	CDebugTopFontCache& operator=( const CDebugTopFontCache& ) = delete;

	/** 焼く文字のピクセルサイズを返す (0 なら共有フォントをそのまま使う)。 */
	f32 GetFontSize() const noexcept { return m_FontSize; }

	/**
	 * 焼く文字のピクセルサイズを設定する。
	 *
	 * @details 変えると次の Resolve で焼き直す。0 を渡すと共有フォントへ戻る。
	 * @param FontSize 焼くサイズ (ピクセル)。
	 */
	void SetFontSize( f32 FontSize ) noexcept;

	/** 漢字を焼き込む設定かを返す。 */
	bool IsIncludeCjk() const noexcept { return m_bIncludeCjk; }

	/**
	 * 漢字を焼き込むかを設定する。
	 *
	 * @details
	 * 漢字を入れるとアトラスが大きくなり焼く時間も延びる。仮名と英数だけで足りるなら
	 * false の方が軽い。変えると次の Resolve で焼き直す。
	 * @param bIncludeCjk 漢字を入れるなら true。
	 */
	void SetIncludeCjk( bool bIncludeCjk ) noexcept;

	/**
	 * 1 フレーム進める。
	 *
	 * @details
	 * 焼き直しは手が止まってから行う。その待ち時間をここで数える。毎フレーム呼ぶこと。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void Update( f32 DeltaSeconds ) noexcept;

	/**
	 * いまの設定で描くための束を返す。
	 *
	 * @details
	 * 必要なら焼き直す。焼けなければ共有フォントを拡縮したものを返す (にじむが表示は保つ)。
	 * 共有フォントも無ければ、何も描かない無効な束を返す。
	 * @param RenderContext フォントを焼くデバイスと共有フォントの取得元。
	 * @return 描画に使う束。
	 */
	CDebugTopText Resolve( FRenderContext& RenderContext ) noexcept;

private:
	/** 設定が変わっていて、焼き直しが要る状態かを返す。 */
	bool IsRebakePending() const noexcept;

	/** 焼き直しが要るなら焼く (手が止まってから)。 */
	void Ensure( FRenderContext& RenderContext ) noexcept;

	/** 専用フォントのGPU資源と読み込み状態。 */
	FUiFontResource m_FontResource;

	/** 焼く文字のピクセルサイズ (0 で共有フォントをそのまま使う)。 */
	f32 m_FontSize = 0.0f;

	/** 漢字を焼き込むか。 */
	bool m_bIncludeCjk = true;

	/** 現在の設定で焼こうとしたか (失敗を毎フレーム繰り返さないため)。 */
	bool m_bTried = false;

	/** 設定が変わってからの経過秒 (これが待ち時間を超えたら焼く)。 */
	f32 m_SettleSeconds = 0.0f;
};
