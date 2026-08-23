// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 登録済み3D衝突形状を現在Transformへ変換した読み取り用の値。
 *
 * @details `Kind`で有効な形状を選ぶ。無効なノードでも形状調整に使えるようworld形状を返すが、
 * 実際の問い合わせ対象かどうかは`bQueryable`で明示する。
 */
struct FWorldCollisionShape3D
{
	/** 現在のworld形状の種類。 */
	enum class EKind : u8
	{
		/** `Box`が有効な軸平行箱。 */
		Box,

		/** `Sphere`が有効な球。 */
		Sphere,
	};

	/** ACS衝突集合内の世代付き形状番号。 */
	FCollisionShapeId3D Shape;

	/** 形状を登録したシーングラフ内の世代付きノード番号。 */
	FNodeId Node;

	/** 現在有効なworld形状の種類。 */
	EKind Kind = EKind::Box;

	/** `Kind`が`Box`の場合に有効なworld軸平行箱。 */
	FAabb3 Box{};

	/** `Kind`が`Sphere`の場合に有効なworld球。 */
	FSphere Sphere{};

	/** ノードが有効なときに使う登録レイヤー。 */
	u32 Layer = CCollisionWorld3D::kAllLayers;

	/** ノードと祖先が有効で、0以外のレイヤーを持つ問い合わせ対象ならtrue。 */
	bool bQueryable = false;
};
