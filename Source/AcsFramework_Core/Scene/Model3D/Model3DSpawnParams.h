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

	/**
	 * 金属らしさ。0 で非金属、1 で金属。
	 *
	 * @details
	 * **中間の値に物理的な意味は無い。** 金属か、そうでないか、どちらかにする
	 * (境目をぼかしたいときだけ 0.5 を使う)。
	 *
	 * 1 にすると拡散反射が消えるので、**環境が映らない場所では真っ黒になる**。
	 * 金属にするなら環境光か反射を用意すること。
	 */
	f32 Metallic = 0.0f;

	/**
	 * 表面の粗さ。0 で鏡、1 で完全に拡散。
	 *
	 * @details
	 * **«綺麗さ» の印象をいちばん変える値。** 既定の 0.5 は «少し艶のあるプラスチック»。
	 * 磨いた床や濡れた地面は 0.1〜0.2、布や石は 0.8 以上。
	 *
	 * 低くするほど反射 (SSR) が効く。粗い面には反射はほとんど乗らない。
	 */
	f32 Roughness = 0.5f;

	/**
	 * 読み込み済みのモデル。
	 *
	 * @details
	 * **`MeshPath` だけでは映らない。** 部品はパスを覚えるだけで、読み込みは別の仕事だから。
	 * `CModelLibrary::Load` の結果をここへ入れるか、置き場を渡す `SpawnInto` の
	 * 多重定義を使う (そちらは中で読む)。
	 */
	TSharedPtr<AAsset> MeshAsset;

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
