// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/SceneTransition.h"

using namespace acs;

/** シーン遷移の方法を選び、暗転を待つ積み下ろし操作を保持する。 */
class FSceneTravelController
{
public:
	/** シーンを即時または暗転付きで置き換える。空の遷移先は何もしない。 */
	void TravelTo( CGame& Game, TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept;

	/** シーンを即時または暗転後に重ねる。空の遷移先は待機中の操作も変えない。 */
	void PushScene( CGame& Game, TUniquePtr<AScene> Next, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept;

	/** シーンを即時または暗転後に下ろす。深さが足りない場合は待機中の操作も変えない。 */
	void PopScene( CGame& Game, TUniquePtr<CSceneTravelContext> Context, ESceneTransition Transition, f32 OutSeconds, f32 InSeconds ) noexcept;

	/** 暗転が完了した待機操作を実行し、指定されたゲームで明転を始める。 */
	void Update( CGame& Game ) noexcept;

private:
	/** 暗転完了後に実行する積み下ろし操作。 */
	enum class EPending : u8
	{
		/** 待機している操作はない。 */
		None,

		/** 暗転完了後にシーンを重ねる。 */
		Push,

		/** 暗転完了後にシーンを下ろす。 */
		Pop,
	};

	/** 暗転完了後に重ねるシーン。下ろし待機中も置換時まで保持する。 */
	TUniquePtr<AScene> m_PendingScene;

	/** 暗転完了後の積み下ろしへ渡す遷移情報。 */
	TUniquePtr<CSceneTravelContext> m_PendingContext;

	/** 現在待機している積み下ろし操作。 */
	EPending m_Pending = EPending::None;

	/** 待機操作を実行した後の明転時間。 */
	f32 m_PendingInSeconds = 0.3f;
};
