// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Page/DebugTopEntity.h"
#include "Debug/Perf/PerfBudgetSnapshot.h"

using namespace acs;

class CPerfBudgetSubsystem;

/**
 * 予算の使われ方を並べるデバッグメニューのページ。
 *
 * @details
 * このページは**測らない・数えない**。1 フレームに 1 度だけ数字を写し取り、行はそれを読む。
 * 写す係が 1 か所なので、並んでいる行が全て同じ瞬間の値になる。
 *
 * カテゴリが後から増えたら (ゲーム側が DefineCategory した場合) 行を組み直す。
 *
 * @code
 * Overlay->GetHUD().AddEntity( NewObject<APerfBudgetPage>( FString( "Perf" ), *Perf ) );
 * @endcode
 */
class APerfBudgetPage : public ADebugTopEntity
{
public:
	/**
	 * ページを構築する。
	 *
	 * @param Name パンくずへ出すページ名。
	 * @param Perf 数字の出どころ。ページより長く生きること。
	 */
	APerfBudgetPage( const FString& Name, CPerfBudgetSubsystem& Perf );

	/** 数字を写し直し、カテゴリが増えていれば組み直してから 1 フレーム進める。 */
	void Update( f32 DeltaSeconds ) noexcept override;

	/** 直近に写し取った数字を返す (行が読む)。 */
	const CPerfBudgetSnapshot& GetSnapshot() const noexcept { return m_Snapshot; }

protected:
	/** 概要の行とカテゴリの行を並べる。 */
	void OnBuild() noexcept override;

private:
	/** 数字を写し直す。 */
	void RefreshSnapshot() noexcept;

	/** カテゴリが増減していれば行を組み直す。 */
	void RebuildCategoryRowsIfChanged();

	/** 平均フレーム時間と目標を並べる行を足す。 */
	void BuildSummaryRows();

	/** カテゴリごとの行を足す。 */
	void BuildCategoryRows();

	/** 平均フレーム時間の文字列を作る。 */
	FString MakeFrameText() const;

	/** 上限を超えている数の文字列を作る。 */
	FString MakeOverCountText() const;

	/** 数字の出どころ。所有はしない。 */
	CPerfBudgetSubsystem* m_Perf = nullptr;

	/** 直近に写し取った数字。 */
	CPerfBudgetSnapshot m_Snapshot;

	/** 行を組んだ時点のカテゴリ数。 */
	usize m_BuiltCategoryCount = 0u;
};
