// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/SceneTravelController.h"
#include "AcsFramework_Core/Scene/SceneTransition.h"

using namespace acs;

/** GameInstance が共有するシーン遷移の窓口とゲーム配線を所有する。 */
class CSceneTravelSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CSceneTravelSubsystem )

	/**
	 * シーン操作に使うゲームを配線する。
	 * @param Game サブシステムより長く有効なゲーム。
	 */
	void Bind( CGame& Game ) noexcept { m_Game = &Game; }

	/**
	 * シーンを即時または暗転付きで置き換える。
	 * @param Next 切り替え先のシーン。
	 * @param Transition 遷移方法。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void TravelTo( TUniquePtr<AScene> Next, ESceneTransition Transition = ESceneTransition::Fade, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/**
	 * 遷移情報を渡してシーンを即時または暗転付きで置き換える。
	 * @param Next 切り替え先のシーン。
	 * @param Context 遷移先へ渡す情報。所有権を移し、失敗時は破棄する。
	 * @param Transition 遷移方法。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void TravelTo( TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition = ESceneTransition::Fade, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/**
	 * 現在のシーンを残して新しいシーンを重ねる。
	 * @param Next 上へ重ねるシーン。
	 * @param Transition 遷移方法。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void PushScene( TUniquePtr<AScene> Next, ESceneTransition Transition = ESceneTransition::Cut, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/**
	 * 遷移情報を渡して新しいシーンを重ねる。
	 * @param Next 上へ重ねるシーン。
	 * @param Context 重ねる先へ渡す情報。所有権を移し、失敗時は破棄する。
	 * @param Transition 遷移方法。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void PushScene( TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition = ESceneTransition::Cut, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/**
	 * 重ねたシーンを下ろす。深さが足りない場合は何もしない。
	 * @param Transition 遷移方法。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void PopScene( ESceneTransition Transition = ESceneTransition::Cut, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/**
	 * 遷移情報を渡して重ねたシーンを下ろす。
	 * @param Context 戻り先へ渡す情報。所有権を移し、失敗時は破棄する。
	 * @param Transition 遷移方法。
	 * @param OutSeconds 暗転にかける秒数 (Cut では無視)。
	 * @param InSeconds 明転にかける秒数 (Cut では無視)。
	 */
	void PopScene( TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition = ESceneTransition::Cut, f32 OutSeconds = 0.3f, f32 InSeconds = 0.3f ) noexcept;

	/** 積んでいるシーンの枚数を返す。配線前は 0 を返す。 */
	u32 GetDepth() const noexcept;

	/** 重ねたものを下ろせるか (2 枚以上積んでいるか) を返す。 */
	bool CanPop() const noexcept { return GetDepth() > 1; }

	/** 暗転が完了した待機中の積み下ろし操作を進める。 */
	void Update() noexcept;

private:
	/** シーンを持っている CGame。所有はしない (アプリが所有する)。 */
	CGame* m_Game = nullptr;

	/** 遷移方法と暗転待ちの積み下ろし状態。 */
	FSceneTravelController m_Controller;
};
