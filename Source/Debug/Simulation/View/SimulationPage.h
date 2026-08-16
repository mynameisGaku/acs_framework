// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/SimulationSnapshot.h"
#include "Debug/DebugTop/Page/DebugTopEntity.h"

using namespace acs;

class CDebugTopElementInt;
class CDebugTopElementString;
class CSimulationSubsystem;

/**
 * シミュレーションの様子を見て、その場で記録・再生を切り替えるページ。
 *
 * @details
 * 記録と再生は「バグが出た後に、あの操作をもう一度」やるための機能だが、そのために
 * コードを書き足してビルドし直していては間に合わない。**出た瞬間に画面から保存できる**
 * ことが要る。ここはそのための操作盤。
 *
 * 見えるもの: 回り方 (Live / Recording / Replaying)、ティック、テープの件数と種、
 * 溜まっているイベント、処理落ちで捨てた秒数。
 *
 * できること: 記録開始 / 再生開始 / 通常へ戻す / テープをファイルへ保存 / ファイルから読込。
 *
 * **状態は持たない。** 数字は毎フレーム CSimulationSubsystem から取り直す。
 */
class ASimulationPage : public ADebugTopEntity
{
public:
	/**
	 * ページを構築する。
	 *
	 * @param Name パンくずへ出すページ名。
	 * @param Simulation 見る相手。ページより長く生きること。
	 */
	ASimulationPage( const FString& Name, CSimulationSubsystem& Simulation );

protected:
	/** 様子を映す行と、操作の行を並べる。 */
	void OnBuild() noexcept override;

private:
	/** 様子を映す行を足す。 */
	void BuildWatchRows();

	/** 記録・再生の操作行を足す。 */
	void BuildControlRows();

	/** ファイルの保存・読込の行を足す。 */
	void BuildFileRows();

	/** 途中から始めるための行を足す。 */
	void BuildSnapshotRows();

	/** 記録を始める。 */
	void StartRecording();

	/** 記録したものを再生する。 */
	void StartReplay();

	/** 通常の回り方へ戻す。 */
	void StartLive();

	/** テープをファイルへ保存する。 */
	void SaveTape();

	/** テープをファイルから読み込む。 */
	void LoadTape();

	/** 溜まっているイベントを捨てる。 */
	void ClearEvents();

	/** いまの様子を写し取る。 */
	void CaptureSnapshot();

	/** 写し取った様子へ戻す。 */
	void RestoreSnapshot();

	/** 写し取った様子をファイルへ保存する。 */
	void SaveSnapshot();

	/** 写し取った様子をファイルから読み込む。 */
	void LoadSnapshot();

	/** 回り方の文字列を作る。 */
	FString MakeModeText() const;

	/** ティックと補間位置の文字列を作る。 */
	FString MakeTickText() const;

	/** テープの文字列を作る。 */
	FString MakeTapeText() const;

	/** イベントと処理落ちの文字列を作る。 */
	FString MakeRuntimeText() const;

	/** 写し取った様子の文字列を作る。 */
	FString MakeSnapshotText() const;

	/** 直近の操作の結果を作る。 */
	FString MakeLastResultText() const;

	/** 見る相手。所有はしない。 */
	CSimulationSubsystem* m_Simulation = nullptr;

	/** 記録を始めるときの種。行の所有はページの行配列。 */
	CDebugTopElementInt* m_SeedField = nullptr;

	/** テープの保存・読込のパス。行の所有はページの行配列。 */
	CDebugTopElementString* m_PathField = nullptr;

	/** 様子の保存・読込のパス。行の所有はページの行配列。 */
	CDebugTopElementString* m_SnapshotPathField = nullptr;

	/** 写し取った様子。ここが持つ 1 つだけを使い回す。 */
	CSimulationSnapshot m_Snapshot;

	/** 直近の操作の結果。 */
	FString m_LastResult;
};
