// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Simulation/ActionInput.h"
#include "AcsFramework_Core/Simulation/Input/ActionCommandSequenceTrackerState.h"

using namespace acs;

class CActionInputTracker;

/**
 * 異なるアクションの押下順と間隔から、短いコマンド列の完了を判定する局所状態。
 *
 * @details
 * 方向アクションを含む技、連続操作、メニューの短縮操作などに使う。列に含まれない操作は
 * 無視し、列に含まれる複数操作が同じ更新で押された場合は順序を推測しない。入力装置や時計は
 * 所有せず、通常フレームまたは固定ステップの入力履歴と明示経過秒だけで進む。同じ入力標本を
 * 複数回渡さず、1フレームまたは1固定ステップにつき`Update()`を1回呼ぶ。
 */
class FActionCommandSequenceTracker
{
public:
	/** 0.25秒間隔の未設定状態を構築する。 */
	FActionCommandSequenceTracker() noexcept = default;

	/**
	 * アクション列と押下間隔を指定して構築する。
	 *
	 * @param ActionIndices 順番に押す2個以上のアクション番号。
	 * @param ActionCount 2以上かつ容量以下の要素数。
	 * @param MaximumIntervalSeconds 連続する押下間に許す有限かつ正の秒数。
	 * @details 不正値なら既定の未設定状態を保つ。
	 */
	FActionCommandSequenceTracker( const u32* ActionIndices,
		u32 ActionCount, f32 MaximumIntervalSeconds ) noexcept;

	/** 要素数を配列から決めてアクション列を構築する。 */
	template<usize ActionCount>
	FActionCommandSequenceTracker( const u32 ( &ActionIndices )[ActionCount],
		f32 MaximumIntervalSeconds ) noexcept
	{
		Configure( ActionIndices, static_cast<u32>( ActionCount ),
			MaximumIntervalSeconds );
	}

	/**
	 * 今後使うアクション列と最大間隔をまとめて変更する。
	 *
	 * @details 変更に成功すると途中入力と今回完了結果を空にする。
	 * @param ActionIndices 順番に押す2個以上のアクション番号。
	 * @param ActionCount 2以上かつ容量以下の要素数。
	 * @param MaximumIntervalSeconds 連続する押下間に許す有限かつ正の秒数。
	 * @return 全項目を反映できたらtrue。不正値では従来状態を一切変えずfalse。
	 */
	bool Configure( const u32* ActionIndices,
		u32 ActionCount, f32 MaximumIntervalSeconds ) noexcept;

	/** 要素数を配列から決めてアクション列を変更する。 */
	template<usize ActionCount>
	bool Configure( const u32 ( &ActionIndices )[ActionCount],
		f32 MaximumIntervalSeconds ) noexcept
	{
		return Configure( ActionIndices, static_cast<u32>( ActionCount ),
			MaximumIntervalSeconds );
	}

	/** 2個以上の有効なアクション列が設定されていればtrue。 */
	bool IsConfigured() const noexcept;

	/** 設定済みアクション数を返す。 */
	u32 GetActionCount() const noexcept { return m_ActionCount; }

	/**
	 * 指定位置のアクション番号を返す。
	 *
	 * @param ActionOffset 先頭を0とする列内位置。
	 * @return 設定済み位置ならアクション番号。範囲外なら`kActionButtonCount`。
	 */
	u32 GetActionIndex( u32 ActionOffset ) const noexcept;

	/** 連続する押下間に許す最大秒数を返す。 */
	f32 GetMaximumIntervalSeconds() const noexcept
	{
		return m_MaximumIntervalSeconds;
	}

	/**
	 * 通常フレームの入力履歴から順序入力を進める。
	 *
	 * @param Input 現在と前フレームを保持する入力。
	 * @param DeltaSeconds 前回の入力標本から進んだ有限かつ0以上の秒数。
	 * @return 更新できたらtrue。未設定または不正時間では全状態を変えずfalse。
	 */
	bool Update( const CActionInputTracker& Input, f32 DeltaSeconds ) noexcept;

	/**
	 * 明示した現在と前回の入力から順序入力を進める。
	 *
	 * @details AI、入力再生、固定ステップ、単体テストなど、装置を使わない経路で使う。
	 * @param CurrentInput 現在のアクション入力。
	 * @param PreviousInput 1回前のアクション入力。
	 * @param DeltaSeconds 前回の入力標本から進んだ有限かつ0以上の秒数。
	 * @return 更新できたらtrue。未設定または不正時間では全状態を変えずfalse。
	 */
	bool Update( const FActionInput& CurrentInput,
		const FActionInput& PreviousInput, f32 DeltaSeconds ) noexcept;

	/** 途中入力と今回だけの完了結果を空にする。設定は維持する。 */
	void Reset() noexcept;

	/** 設定、途中位置、経過秒と今回結果を保存可能な値として返す。 */
	FActionCommandSequenceTrackerState CaptureState() const noexcept;

	/**
	 * 保存した順序入力追跡状態を復元する。
	 *
	 * @param State `CaptureState`で取得した有限かつ矛盾のない状態。
	 * @return 復元できたらtrue。不正な状態では現在値を一切変えずfalse。
	 */
	bool RestoreState(
		const FActionCommandSequenceTrackerState& State ) noexcept;

	/** 次のアクションを待っている途中の入力列があればtrue。 */
	bool IsWaitingForNextAction() const noexcept
	{
		return m_MatchedActionCount > 0u;
	}

	/** 先頭から順番どおりに受理済みのアクション数を返す。 */
	u32 GetMatchedActionCount() const noexcept
	{
		return m_MatchedActionCount;
	}

	/** 直前の受理から次の押下を待てる残り秒を返す。待機していなければ0。 */
	f32 GetRemainingSeconds() const noexcept;

	/** 今回の更新でアクション列を最後まで受理したならtrue。 */
	bool WasCompleted() const noexcept { return m_bWasCompleted; }

private:
	/** 待機中なら時間を進め、最大間隔を超えた途中入力を空にする。 */
	void Advance_Internal( f32 DeltaSeconds ) noexcept;

	/** 新しい押下を足した後に、先頭から一致している最長の要素数を返す。 */
	u32 FindNextMatchedActionCount_Internal(
		u32 PressedActionIndex ) const noexcept;

	/** 途中位置と経過秒を空にする。設定と今回結果は変えない。 */
	void ClearProgress_Internal() noexcept;

	/** 順番に押すアクション番号。 */
	u32 m_ActionIndices[kActionCommandSequenceCapacity] = {};

	/** 設定済みアクション数。 */
	u32 m_ActionCount = 0u;

	/** 先頭から順番どおりに受理済みのアクション数。 */
	u32 m_MatchedActionCount = 0u;

	/** 連続するアクション押下の間に許す最大秒数。 */
	f32 m_MaximumIntervalSeconds = 0.25f;

	/** 直前に受理したアクションから経過した秒数。 */
	f64 m_ElapsedSinceLastActionSeconds = 0.0;

	/** 今回の更新でアクション列を最後まで受理したならtrue。 */
	bool m_bWasCompleted = false;
};
