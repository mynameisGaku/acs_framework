// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInputTape.h"
#include "AcsFramework_Core/Simulation/DeterministicRandom.h"
#include "AcsFramework_Core/Simulation/FixedStepDriver.h"
#include "AcsFramework_Core/Simulation/IActionInputSource.h"
#include "AcsFramework_Core/Simulation/ISimulationRule.h"
#include "AcsFramework_Core/Simulation/SimulationEventQueue.h"
#include "AcsFramework_Core/Simulation/SimulationSnapshot.h"

using namespace acs;

/** シミュレーションの回り方。 */
enum class ESimulationMode : u8
{
	/** 入力元から取って進める。記録はしない。 */
	Live,

	/** 入力元から取って進め、テープへ残す。 */
	Recording,

	/** テープから読んで進める。入力元は見ない。 */
	Replaying,
};

/**
 * ゲームロジックを «同じ入力なら同じ結果» の形で回すサブシステム。
 *
 * @details
 * ここが持つのは**順番だけ**である。
 *
 * 1. 入力を 1 つ決める (Live/Recording は入力元から、Replaying はテープから)
 * 2. 記録中ならテープへ残す
 * 3. 実時間を固定ステップへ割り直し、その回数だけ規則を呼ぶ
 * 4. 規則が置いた «起きたこと» を、読む側が取りに来るまで持っておく
 *
 * **ルールも盤面も持たない。** それはゲーム側 (ISimulationRule) の担当。
 * ここは「いつ・何回・どの入力で呼ぶか」だけを決める。
 *
 * ゲーム全体を起動せずに回せる。`Update()` へ好きな dt を流し込めば、Actor も
 * 描画も音も無しで 1 万ステップ進められる (バランス調整や当たりの検証に使う)。
 *
 * @code
 * Simulation->SetRule( MakeUnique<CMyRule>() );
 * Simulation->SetInputSource( MakeUnique<CPlayerActions>() );
 * Simulation->StartRecording( 12345u );
 *
 * // 毎フレーム
 * Simulation->Update( DeltaSeconds );
 * for ( usize i = 0u; i < Simulation->GetEvents().Num(); ++i ) { ...鳴らす・出す... }
 * Simulation->ClearEvents();
 * @endcode
 */
class CSimulationSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CSimulationSubsystem )

	/** subsystem 終了時に規則と入力元を手放す。 */
	void OnDeinitialize() noexcept override;

	/**
	 * ステップ幅を決める。
	 *
	 * @details アプリの起動時に 1 度だけ呼ぶ。呼ばなければ 1/60 秒で回る。
	 * @param StepSeconds 1 ステップの秒数。
	 * @param MaximumStepsPerAdvance 1 フレームで進める最大ステップ数。
	 * @return 設定できたら true。
	 */
	bool Configure( f64 StepSeconds, u32 MaximumStepsPerAdvance = 8u ) noexcept;

	/**
	 * 進める規則を差す。
	 *
	 * @details 渡したものの寿命はここが持つ。差し替えると盤面は初期状態へ戻る。
	 * @param Rule ゲーム側の規則。
	 * @return 差せたら true。
	 */
	bool SetRule( TUniquePtr<ISimulationRule> Rule ) noexcept;

	/**
	 * 入力元を差す。
	 *
	 * @details 渡したものの寿命はここが持つ。人でも AI でも同じ口から入る。
	 * @param Source 入力元。
	 * @return 差せたら true。
	 */
	bool SetInputSource( TUniquePtr<IActionInputSource> Source ) noexcept;

	/**
	 * 記録を始める。
	 *
	 * @details 盤面と時計を初期状態へ戻し、種を蒔き直してからテープを取り始める。
	 * @param Seed 乱数の種。
	 */
	void StartRecording( u64 Seed ) noexcept;

	/**
	 * 記録したテープを再生する。
	 *
	 * @details
	 * 盤面と時計を初期状態へ戻し、テープが覚えている種を蒔き直す。以降、入力元は見ない。
	 * @return 再生を始められたら true (テープが空なら false)。
	 */
	bool StartReplay() noexcept;

	/**
	 * 記録も再生もせず、そのまま回す状態へ戻す。
	 *
	 * @param Seed 乱数の種。
	 */
	void StartLive( u64 Seed ) noexcept;

	/**
	 * 1 フレームぶん進める。
	 *
	 * @details
	 * 渡した秒数は固定ステップへ割り直される。0 ステップのことも、複数ステップのこともある。
	 * @param DeltaSeconds 進めたい秒数。
	 * @return 実際に進めたステップ数。
	 */
	u32 Update( f64 DeltaSeconds ) noexcept;

	/**
	 * ステップ数を指定して進める。
	 *
	 * @details
	 * 実時間を介さずに回す。テストや大量実行で使う (時計の溜まりには触らない)。
	 * @param StepCount 進めるステップ数。
	 * @return 実際に進めたステップ数。
	 */
	u32 AdvanceSteps( u32 StepCount ) noexcept;

	/**
	 * いまの様子 (盤面・時計・乱数・入力履歴) を写し取る。
	 *
	 * @details
	 * 規則が盤面を差し出せる (ISimulationRule::TrySaveState を実装している) 必要がある。
	 * 入力のテープは含まれない。途中から再生するときは、同じテープと組み合わせる。
	 * @param OutSnapshot 写し先。
	 * @return 写せたら true。
	 */
	bool TryCaptureSnapshot( CSimulationSnapshot& OutSnapshot ) const noexcept;

	/**
	 * 写し取った様子へ戻す。
	 *
	 * @details
	 * 4 つのうち 1 つでも戻せなければ何も変えない。戻した後は、そのティックから続きを回せる。
	 * 溜まっているイベントは捨てる (戻る前のフレームのものが残っていると二重に効く)。
	 * @param Snapshot 戻す先の様子。
	 * @return 戻せたら true。
	 */
	bool TryRestoreSnapshot( const CSimulationSnapshot& Snapshot ) noexcept;

	/** 進める規則が差してあるかを返す。差していなければ回す意味がない。 */
	bool HasRule() const noexcept { return static_cast<bool>( m_Rule ); }

	/** いまの回り方を返す。 */
	ESimulationMode GetMode() const noexcept { return m_Mode; }

	/** いまのティック番号を返す。 */
	u32 GetTick() const noexcept { return m_Driver.GetTick(); }

	/** ステップ間のどこに居るか (0..1) を返す。描画の補間に使う。 */
	f32 GetAlpha() const noexcept { return m_Driver.GetAlpha(); }

	/** 起きたことの置き場を返す。 */
	const CSimulationEventQueue& GetEvents() const noexcept { return m_Events; }

	/** 起きたことを捨てる。読み終えたフレームの末尾で呼ぶ。 */
	void ClearEvents() noexcept { m_Events.Clear(); }

	/** 入力のテープを返す (保存・読込はこれ越しに行う)。 */
	CActionInputTape& GetTape() noexcept { return m_Tape; }

	/** 入力のテープを const で返す。 */
	const CActionInputTape& GetTape() const noexcept { return m_Tape; }

	/** 乱数を返す。ゲーム側が規則の外で引きたいときに使う。 */
	CDeterministicRandom& GetRandom() noexcept { return m_Random; }

	/** 直近のステップで使った入力を返す。 */
	const FActionInput& GetLastInput() const noexcept { return m_LastInput; }

	/** 処理落ちで捨てた秒数を返す。 */
	f64 GetDroppedSeconds() const noexcept { return m_Driver.GetDroppedSeconds(); }

private:
	/**
	 * このティックの入力を 1 つ決める。
	 *
	 * @param Tick ティック番号。
	 * @param OutInput 入力の入れ先。
	 */
	void ResolveInput( u32 Tick, FActionInput& OutInput ) noexcept;

	/**
	 * 1 ステップだけ進める。
	 *
	 * @details 入力の決定・記録・規則の呼び出し・ティックの前進をこの順で行う。
	 */
	void AdvanceOneStep() noexcept;

	/** 盤面・時計・置き場を初期状態へ戻す。 */
	void ResetForNewRun() noexcept;

	/** 実時間を固定ステップへ割り直す係。 */
	CFixedStepDriver m_Driver;

	/** 種を握れる乱数。 */
	CDeterministicRandom m_Random;

	/** 入力のテープ。 */
	CActionInputTape m_Tape;

	/** 起きたことの置き場。 */
	CSimulationEventQueue m_Events;

	/** ゲーム側の規則。寿命をここで持つ。 */
	TUniquePtr<ISimulationRule> m_Rule;

	/** 入力元。寿命をここで持つ。 */
	TUniquePtr<IActionInputSource> m_InputSource;

	/** 直近のステップで使った入力。 */
	FActionInput m_LastInput;

	/** 1 つ前のステップで使った入力。 */
	FActionInput m_PreviousInput;

	/** いまの回り方。 */
	ESimulationMode m_Mode = ESimulationMode::Live;
};
