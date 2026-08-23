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
	 * PBRではなくACS既定の二段影と縁光を使うか。
	 *
	 * @details trueならイラスト調の陰影になる。透明な上塗りなどPBR専用の値は使われない。
	 */
	bool bToonShading = false;

	/**
	 * 透明な上塗り層の強さ。0で無し、1で最大。
	 *
	 * @details 車の塗装、ラッカー、濡れた面のように、元の材質の上へ細い反射を重ねる。
	 */
	f32 Clearcoat = 0.0f;

	/**
	 * 上塗り層だけの粗さ。0で鋭い反射、1でぼけた反射。
	 *
	 * @details `Clearcoat`が0なら見た目へ影響しない。
	 */
	f32 ClearcoatRoughness = 0.1f;

	/**
	 * 表面のすぐ下へ光を回り込ませる強さ。0で無し、1で最大。
	 *
	 * @details 肌、蝋、乳白素材のような、影の境目が柔らかい非金属に使う。
	 */
	f32 SubsurfaceStrength = 0.0f;

	/** 内部を通って見える0から1のRGB。`SubsurfaceStrength`が0なら見た目へ影響しない。 */
	FVec3 SubsurfaceColor{ 1.0f, 0.3f, 0.2f };

	/**
	 * 照明を受けずに加算する自己発光色。
	 *
	 * @details 各成分は0から1。`EmissiveStrength`が0なら発光しない。
	 */
	FVec3 EmissiveColor{ 0.0f, 0.0f, 0.0f };

	/**
	 * 自己発光色へ掛けるHDR強度。
	 *
	 * @details 1を超えるとbloomへ光が広がる。0から10の範囲で指定する。
	 */
	f32 EmissiveStrength = 0.0f;

	/**
	 * 読み込み済みのモデル。
	 *
	 * @details
	 * **`MeshPath` だけでは映らない。** 部品はパスを覚えるだけで、読み込みは別の仕事だから。
	 * `CModelLibrary::Load` の結果をここへ入れるか、置き場を渡す `SpawnInto` の
	 * 多重定義を使う (そちらは中で読む)。`AMeshAsset`以外は受け付けない。
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
	 * ACS既定のトゥーン陰影を使う形を、色と位置から作る。
	 *
	 * @param InPrimitive 使う形。
	 * @param InPosition 置く場所。
	 * @param InColor 表面へ使うRGB。
	 * @return 二段影と縁光を使う形指定。
	 */
	static FModel3DSpawnParams FromToonPrimitive( EMeshPrimitive3D InPrimitive, FVec3 InPosition, FVec3 InColor ) noexcept;

	/**
	 * 光沢のある透明な上塗りを持つ形を、色と位置から作る。
	 *
	 * @param InPrimitive 使う形。
	 * @param InPosition 置く場所。
	 * @param InColor 表面へ使う0から1のRGB。
	 * @param InCoatRoughness 上塗り層の0から1の粗さ。
	 * @return 上塗り強度1の形指定。上塗り粗さの不正値はIsValidで拒否される。
	 */
	static FModel3DSpawnParams FromCoatedPrimitive( EMeshPrimitive3D InPrimitive, FVec3 InPosition, FVec3 InColor, f32 InCoatRoughness = 0.08f ) noexcept;

	/**
	 * 光が表面のすぐ下へ回り込む形を、色と位置から作る。
	 *
	 * @param InPrimitive 使う形。
	 * @param InPosition 置く場所。
	 * @param InColor 表面へ使う0から1のRGB。
	 * @param InSubsurfaceColor 内部を通って見える0から1のRGB。
	 * @param InStrength 表面下へ光を回す0から1の強さ。
	 * @return 肌や蝋に使える形指定。不正値はIsValidで拒否される。
	 */
	static FModel3DSpawnParams FromSubsurfacePrimitive( EMeshPrimitive3D InPrimitive, FVec3 InPosition, FVec3 InColor, FVec3 InSubsurfaceColor = FVec3{ 1.0f, 0.3f, 0.2f }, f32 InStrength = 0.55f ) noexcept;

	/**
	 * 自己発光する形を色、位置、強度から作る。
	 *
	 * @param InPrimitive 使う形。
	 * @param InPosition 置く場所。
	 * @param InColor 表面色と自己発光へ使う0から1のRGB。
	 * @param InStrength bloomへ渡す0から10のHDR強度。
	 * @return そのまま配置へ渡せる自己発光形の指定。不正値はIsValidで拒否される。
	 */
	static FModel3DSpawnParams FromEmissivePrimitive( EMeshPrimitive3D InPrimitive, FVec3 InPosition, FVec3 InColor, f32 InStrength = 3.0f ) noexcept;

	/**
	 * 置ける指定かどうかを返す。
	 *
	 * @details
	 * 置けないのは次の条件。どれも**何も見えないのに失敗もしない**、または描画値を壊すため、
	 * 一番たちの悪い形になるので、置く前に弾く。
	 * - 大きさに 0 が入っている
	 * - 形として `Mesh` を指しているのに、モデルの場所も読込済みモデルも無い
	 * - 読込済みモデルが `AMeshAsset` ではない
	 * - 上塗り強度または上塗り粗さが0から1の有限値ではない
	 * - 表面下へ光を回す強さまたは色が0から1の有限値ではない
	 * - 自己発光色が0から1の有限RGBではない
	 * - 自己発光強度が0から10の有限値ではない
	 */
	bool IsValid() const noexcept;
};
