// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 停止理由、通常速度、1 フレームの進行判断を保持する非サブシステムの状態。
 */
class FTimeControlState final
{
public:
	/**
	 * 理由を重複させずに追加する。
	 *
	 * @param Reason 停止理由。空文字列も 1 つの理由として扱う。
	 */
	void Pause( const FString& Reason );

	/**
	 * 一致する理由を取り除く。
	 *
	 * @param Reason 解除する停止理由。見つからない場合は状態を変えない。
	 */
	void Resume( const FString& Reason );

	/** 全ての停止理由を取り除く。 */
	void ResumeAll() noexcept;

	/** 停止理由が 1 つ以上あるかを返す。 */
	bool IsPaused() const noexcept { return m_Reasons.Num() > 0; }

	/**
	 * 指定した理由で停止しているかを返す。
	 *
	 * @param Reason 調べる停止理由。
	 * @return 一致する理由があれば true。
	 */
	bool IsPausedBy( const FString& Reason ) const noexcept { return FindReason( Reason ) < m_Reasons.Num(); }

	/** 停止理由の数を返す。 */
	usize GetPauseReasonCount() const noexcept { return m_Reasons.Num(); }

	/**
	 * 指定位置の停止理由を返す。
	 *
	 * @param Index 取得する位置。
	 * @return 停止理由。範囲外の場合は空文字列。
	 */
	const FString& GetPauseReason( usize Index ) const noexcept;

	/**
	 * 停止していないフレームの速度を設定する。
	 *
	 * @param Speed 通常速度。0 以下は 0 に丸める。
	 */
	void SetSpeed( f32 Speed ) noexcept { m_Speed = Speed > 0.0f ? Speed : 0.0f; }

	/** 停止していないフレームの速度を返す。 */
	f32 GetSpeed() const noexcept { return m_Speed; }

	/** 次の進行判断で停止中の 1 フレームだけを進めるよう要求する。 */
	void StepOnce() noexcept { m_bStepRequested = true; }

	/**
	 * 現在の理由、速度、1 フレーム進行要求から次の進行可否と実効速度を決める。
	 */
	void AdvanceFrame() noexcept;

	/** 直近の AdvanceFrame() が決めた実効速度を返す。 */
	f32 GetEffectiveScale() const noexcept { return m_EffectiveScale; }

	/** 直近の AdvanceFrame() がシーン更新を許可したかを返す。 */
	bool ShouldTickScenes() const noexcept { return m_bTickThisFrame; }

private:
	/**
	 * 一致する停止理由の位置を探す。
	 *
	 * @param Reason 探す停止理由。
	 * @return 一致位置。見つからない場合は理由の数。
	 */
	usize FindReason( const FString& Reason ) const noexcept;

	/** 現在有効な停止理由。 */
	TArray<FString> m_Reasons;

	/** 停止していないフレームの速度。 */
	f32 m_Speed = 1.0f;

	/** 次の進行判断で 1 フレーム進める要求があるか。 */
	bool m_bStepRequested = false;

	/** 直近の進行判断でシーン更新を許可したか。 */
	bool m_bTickThisFrame = true;

	/** 直近の進行判断で決めた実効速度。 */
	f32 m_EffectiveScale = 1.0f;
};
