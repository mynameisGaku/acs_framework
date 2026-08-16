// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/Perf/PerfBudgetSnapshot.h"
#include "Debug/Perf/PerfCategoryPlan.h"

using namespace acs;
using namespace acs::game;

/**
 * フレームの予算を持って、測る場所へ渡すサブシステム。
 *
 * @details
 * 数える仕組みはエンジン (CPerfBudget) が持っている。ただし**誰も持っていない**ので、
 * ゲームごとに用意して毎フレーム開け閉めすることになる。ここが引き受けるのはその 3 つ。
 *
 * 1. 予算表 (CPerfCategoryPlan) を作ってエンジンへ流し込む
 * 2. フレームの開始と終了を伝える
 * 3. 測った値を、表示できる形 (CPerfBudgetSnapshot) で渡す
 *
 * **測るのはここの仕事ではない。** 実際に時間を積むのは FScopedPerfSample で、
 * ここは受け取った ms をエンジンへ渡すだけにしてある。どこを測るかは呼ぶ側が決める。
 *
 * @code
 * // アプリの起動時に 1 度だけ
 * Perf->Configure( 16.6f );
 * Perf->DefineCategory( FString( "Gameplay/AI" ), 2.0f );
 *
 * // アプリの更新から
 * Perf->BeginFrame();
 * {
 *     const FScopedPerfSample Sample( Perf, "Scene/Update" );
 *     CGame::OnUpdate( DeltaSeconds );
 * }
 * Perf->EndFrame();
 * @endcode
 */
class CPerfBudgetSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CPerfBudgetSubsystem )

	/**
	 * 目標のフレーム時間を決め、枠組みの既定カテゴリを用意する。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。呼ばなくても計測はできる (上限なしになる)。
	 * @param FrameBudgetMilliseconds 1 フレームの目標時間 (ms)。60fps なら 16.6。
	 */
	void Configure( f32 FrameBudgetMilliseconds ) noexcept;

	/**
	 * カテゴリを 1 件足す。
	 *
	 * @details ゲーム側の区分はここで足す。同じ名前を二度足すと予算だけが差し替わる。
	 * @param Category カテゴリ名。
	 * @param BudgetMilliseconds 1 フレームあたりの時間の上限 (ms)。
	 * @param BudgetBytes 保持してよいメモリの上限 (bytes)。0 なら見ない。
	 * @return 計測へ渡せる «動かない名前»。足せなければ nullptr。
	 */
	const char* DefineCategory( const FString& Category, f32 BudgetMilliseconds, u32 BudgetBytes = 0u ) noexcept;

	/**
	 * フレームの計測を始める。
	 *
	 * @details アプリの更新の先頭で呼ぶ。前フレームの積算はここで捨てられる。
	 */
	void BeginFrame() noexcept;

	/**
	 * フレームの計測を終える。
	 *
	 * @details アプリの更新の末尾で呼ぶ。BeginFrame を呼んでいなければ何もしない。
	 */
	void EndFrame() noexcept;

	/**
	 * 測った時間を積む。
	 *
	 * @details
	 * 普通は FScopedPerfSample が呼ぶ。カテゴリ名は**寿命がフレームより長い文字列**
	 * (文字列リテラルか DefineCategory の戻り値) を渡すこと。
	 * @param Category カテゴリ名。
	 * @param ElapsedMilliseconds 積む時間 (ms)。
	 */
	void RecordTimeMilliseconds( const char* Category, f32 ElapsedMilliseconds ) noexcept;

	/**
	 * 確保したメモリを積む。
	 *
	 * @param Category カテゴリ名 (寿命の長い文字列)。
	 * @param Bytes 確保した bytes。
	 */
	void RecordMemoryAlloc( const char* Category, u32 Bytes ) noexcept;

	/**
	 * 解放したメモリを引く。
	 *
	 * @param Category カテゴリ名 (寿命の長い文字列)。
	 * @param Bytes 解放した bytes。
	 */
	void RecordMemoryFree( const char* Category, u32 Bytes ) noexcept;

	/**
	 * いまの数字を写し取る。
	 *
	 * @details 表示側はこれを 1 フレームに 1 度だけ呼び、写した値を読むこと。
	 * @param OutSnapshot 写し先。
	 */
	void CaptureSnapshot( CPerfBudgetSnapshot& OutSnapshot ) const noexcept;

	/** 直近 60 フレームの平均フレーム時間 (ms) を返す。 */
	f32 GetAverageFrameMilliseconds() const noexcept;

	/** 目標のフレーム時間 (ms) を返す。 */
	f32 GetFrameBudgetMilliseconds() const noexcept { return m_FrameBudgetMilliseconds; }

	/** 平均が目標を超えているかを返す。 */
	bool IsOverFrameBudget() const noexcept;

	/** 決めたカテゴリの一覧を返す。 */
	const CPerfCategoryPlan& GetPlan() const noexcept { return m_Plan; }

private:
	/** 予算を数える本体。 */
	CPerfBudget m_Budget;

	/** 決めたカテゴリと、その名前の実体。 */
	CPerfCategoryPlan m_Plan;

	/** 目標のフレーム時間 (ms)。 */
	f32 m_FrameBudgetMilliseconds = 0.0f;

	/** BeginFrame を呼んだまま EndFrame を待っているか。 */
	bool m_bFrameOpen = false;
};
