// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Animation3D/CharacterAnimation3DInput.h"

using namespace acs;
using namespace acs::game;

/**
 * 移動入力からキャラクターの3Dアニメーション状態を選ぶ規則。
 *
 * @details
 * `Input + CurrentState + Profile -> NextState`だけで結果が決まり、場面や描画器を参照しない。
 * 歩きと走りには開始・終了で異なるしきい値を持たせ、境界付近の速度揺れで状態が往復するのを防ぐ。
 */
struct FCharacterAnimation3DProfile
{
	/** 待機状態で再生するクリップ名。 */
	FString IdleClip{ "Idle" };

	/** 歩き状態で再生するクリップ名。 */
	FString WalkClip{ "Walk" };

	/** 走り状態で再生するクリップ名。 */
	FString RunClip{ "Run" };

	/** 空中状態で再生するクリップ名。 */
	FString JumpClip{ "Jump" };

	/** 待機から歩きへ移る速度。 */
	f32 WalkEnterSpeed = 0.15f;

	/** 歩きから待機へ戻る速度。 */
	f32 WalkExitSpeed = 0.08f;

	/** 歩きから走りへ移る速度。 */
	f32 RunEnterSpeed = 4.0f;

	/** 走りから歩きへ戻る速度。 */
	f32 RunExitSpeed = 3.2f;

	/** クリップを切り替えるときに姿勢を混ぜる秒数。0なら即時切替。 */
	f32 BlendSeconds = 0.20f;

	/** ジャンプクリップを繰り返すか。 */
	bool bLoopJump = false;

	/**
	 * 入力と現在状態から次状態を選ぶ。
	 *
	 * @param Input 現フレームの移動情報。
	 * @param CurrentState 現在再生している待機・歩き・走り・ジャンプのいずれか。
	 * @param OutState 成功時だけ更新する次状態。
	 * @return 入力、規則、現在状態が有効ならtrue。
	 */
	bool TrySelectState( const FCharacterAnimation3DInput& Input,
		EAnimationGraphState CurrentState, EAnimationGraphState& OutState ) const noexcept;

	/**
	 * 状態に対応するクリップ名を返す。
	 *
	 * @param State 待機・歩き・走り・ジャンプのいずれか。
	 * @return 対応名。不明な状態なら空。
	 */
	FStringView ClipFor( EAnimationGraphState State ) const noexcept;

	/**
	 * 状態を繰り返し再生するか返す。
	 *
	 * @param State 確認する状態。
	 * @return ジャンプは`bLoopJump`、待機・歩き・走りはtrue。不明な状態はfalse。
	 */
	bool LoopsFor( EAnimationGraphState State ) const noexcept;

	/**
	 * クリップ名、速度境界、補間時間が使えるか返す。
	 *
	 * @return 全項目が有効ならtrue。
	 */
	bool IsValid() const noexcept;
};
