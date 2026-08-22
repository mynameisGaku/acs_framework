// SPDX-License-Identifier: Apache-2.0
#pragma once

/**
 * 第三者視点キャラクターの1回の更新で完了した処理。
 *
 * @details 各段階は実際に成功した場合だけtrueになり、途中失敗を成功として隠さない。
 */
struct FThirdPersonCharacter3DUpdateResult
{
	/** 視点回転、距離変更、揺れの時間更新を反映できたならtrue。 */
	bool bCameraInputApplied = false;

	/** カメラ基準の移動とジャンプを反映できたならtrue。 */
	bool bMovementApplied = false;

	/** 実際の水平速度へ向きを更新または維持できたならtrue。 */
	bool bFacingApplied = false;

	/** 移動後のノード位置へカメラ注視点を合わせられたならtrue。 */
	bool bCameraFollowApplied = false;

	/** 更新時に任意の骨格アニメーションが接続されていたならtrue。 */
	bool bAnimationWasBound = false;

	/** 接続中の骨格アニメーションが今回の状態を受理したならtrue。 */
	bool bAnimationApplied = false;

	/**
	 * 移動、向き、カメラの必須処理を全て完了したか返す。
	 *
	 * @return 任意のアニメーション結果を除く4段階が成功したらtrue。
	 */
	bool Succeeded() const noexcept
	{
		return bCameraInputApplied && bMovementApplied && bFacingApplied && bCameraFollowApplied;
	}

	/**
	 * 任意のアニメーションも含めて処理できたか返す。
	 *
	 * @return 未接続ならtrue、接続中なら今回の状態を受理したときだけtrue。
	 */
	bool AnimationSucceeded() const noexcept { return !bAnimationWasBound || bAnimationApplied; }
};
