// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * 3D の見えるものを 1 つ置くときの指定。
 *
 * @details
 * 何も書かなければ「原点に、等倍で、白い立方体を、影を落とす形で」置く。
 * **要るところだけ書けばよい**ようにしてあり、これが手数の少なさの土台になる。
 *
 * @code
 * // 置くだけ
 * Spawner.SpawnInto( Parent, FModel3DSpawnParams::FromMesh( FStringView( "hero.mdl" ), FVec3{ 0.0f, 0.0f, 5.0f } ) );
 *
 * // 形だけ欲しいとき (素材が無くても試せる)
 * FModel3DSpawnParams Params = FModel3DSpawnParams::FromPrimitive( EMeshPrimitive3D::Sphere, FVec3{ 2.0f, 0.0f, 0.0f } );
 * Params.Color = FVec4{ 1.0f, 0.2f, 0.2f, 1.0f };
 * @endcode
 */
struct FModel3DSpawnParams
{
	/**
	 * 読み込むモデルの場所。
	 *
	 * @details 空なら `Primitive` の形を使う。指定すると `Primitive` は無視される。
	 */
	FStringView MeshPath;

	/** モデルを指定しないときに使う形。 */
	EMeshPrimitive3D Primitive = EMeshPrimitive3D::Cube;

	/** 置く場所。 */
	FVec3 Position{ 0.0f, 0.0f, 0.0f };

	/**
	 * 向き (度、XYZ の順)。
	 *
	 * @details 度で受けるのは、書く人が度で考えるから。中でラジアンへ直す。
	 */
	FVec3 RotationDeg{ 0.0f, 0.0f, 0.0f };

	/** 大きさ。0 を含めると何も見えなくなるので受け付けない。 */
	FVec3 Scale{ 1.0f, 1.0f, 1.0f };

	/** 色 (RGBA)。 */
	FVec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };

	/** 影を落とすか。 */
	bool bCastsShadow = true;

	/**
	 * ノードに付ける名前。
	 *
	 * @details 空なら名前を付けない。**シーンを保存すると名前は消える**ので、
	 * 名前で探す作りにしないこと (`Scene/Snapshot/README.md`)。
	 */
	FStringView Name;

	/**
	 * モデルを指定して作る。
	 *
	 * @param Path モデルの場所。
	 * @param InPosition 置く場所。
	 */
	static FModel3DSpawnParams FromMesh( FStringView Path, FVec3 InPosition ) noexcept;

	/**
	 * 形を指定して作る。
	 *
	 * @param InPrimitive 使う形。
	 * @param InPosition 置く場所。
	 */
	static FModel3DSpawnParams FromPrimitive( EMeshPrimitive3D InPrimitive, FVec3 InPosition ) noexcept;

	/**
	 * 置ける指定かどうかを返す。
	 *
	 * @details
	 * 置けないのは次の 2 つ。どちらも**何も見えないのに失敗もしない**という、
	 * 一番たちの悪い形になるので、置く前に弾く。
	 * - 大きさに 0 が入っている
	 * - 形として `Mesh` を指しているのに、モデルの場所が空
	 */
	bool IsValid() const noexcept;
};
