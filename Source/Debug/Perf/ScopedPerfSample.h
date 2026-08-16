// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

class CPerfBudgetSubsystem;

/**
 * 囲んだ範囲の時間を測って、抜けるときに 1 回だけ積む。
 *
 * @details
 * 測り始めと測り終わりを別々に書くと、途中の `return` で片方だけ実行されて数字が壊れる。
 * 生存期間に任せておけば、どの経路で抜けても必ず 1 回だけ積まれる。
 *
 * 呼ぶ側に書くのは 1 行だけで、積む処理はこの型の中にある。**計測のために流れ側の
 * コードを増やさない**のが狙いなので、これを使う場所で時間を計算しないこと。
 *
 * カテゴリ名は**フレームより長生きする文字列**を渡す。エンジンは名前を複製せず、
 * ポインタのまま持つ。文字列リテラルか `CPerfBudgetSubsystem::DefineCategory` の
 * 戻り値を使えばよい。
 *
 * @code
 * {
 *     const FScopedPerfSample Sample( Perf, "Scene/Update" );
 *     CGame::OnUpdate( DeltaSeconds );
 * }
 * @endcode
 */
class FScopedPerfSample
{
public:
	/**
	 * 計測を始める。
	 *
	 * @param Perf 積む先。nullptr なら何もしない (デバッグ機能が無い構成でもそのまま書ける)。
	 * @param Category カテゴリ名。フレームより長生きする文字列を渡すこと。
	 */
	FScopedPerfSample( CPerfBudgetSubsystem* Perf, const char* Category ) noexcept;

	/** 計測を終えて 1 回だけ積む。 */
	~FScopedPerfSample() noexcept;

	/** コピー禁止 (二重に積まれるため)。 */
	FScopedPerfSample( const FScopedPerfSample& ) = delete;

	/** コピー代入も禁止。 */
	FScopedPerfSample& operator=( const FScopedPerfSample& ) = delete;

	/** ムーブ禁止。 */
	FScopedPerfSample( FScopedPerfSample&& ) = delete;

	/** ムーブ代入も禁止。 */
	FScopedPerfSample& operator=( FScopedPerfSample&& ) = delete;

private:
	/** 積む先。所有はしない。 */
	CPerfBudgetSubsystem* m_Perf = nullptr;

	/** カテゴリ名。所有はしない。 */
	const char* m_Category = nullptr;

	/** 測り始めた時点の tick。 */
	u64 m_StartTicks = 0u;
};
