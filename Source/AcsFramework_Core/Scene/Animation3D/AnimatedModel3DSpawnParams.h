// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 骨で動く3Dモデルを1つ置くときの指定。
 *
 * @details
 * `MeshPath`だけを指定すれば、読み込み、ノード作成、`ASkinnedMeshComponent3D`の追加、
 * 最初のアニメーション再生までをまとめて行える。読み込み済みの`MeshAsset`も渡せる。
 */
struct FAnimatedModel3DSpawnParams
{
	/** `Assets`からの骨付きFBX相対パス。読み込み済みアセットがある場合は空でもよい。 */
	FStringView MeshPath;

	/** 読み込み済みの骨付きモデル。指定時は`MeshPath`から再読込しない。 */
	TSharedPtr<ASkinnedMeshAsset> MeshAsset;

	/** 置く場所。 */
	FVec3 Position{ 0.0f, 0.0f, 0.0f };

	/** 向き。書きやすい度数法で指定する。 */
	FVec3 RotationDeg{ 0.0f, 0.0f, 0.0f };

	/** 大きさ。0を含む指定は見えなくなるため受け付けない。 */
	FVec3 Scale{ 1.0f, 1.0f, 1.0f };

	/** モデルへ掛ける色。1より大きい値は明るい色として利用できる。 */
	FVec3 Color{ 1.0f, 1.0f, 1.0f };

	/** 最初に再生するクリップ名。空なら`InitialAnimationIndex`を使う。 */
	FStringView InitialAnimation;

	/** クリップ名を省略したときに再生する番号。 */
	u32 InitialAnimationIndex = 0u;

	/** 配置直後からアニメーションを再生するか。 */
	bool bAutoPlay = true;

	/** 最初のアニメーションを繰り返すか。 */
	bool bLoop = true;

	/** ノードに付ける名前。空なら名前を付けない。 */
	FStringView Name;

	/**
	 * パスと位置から配置指定を作る。
	 *
	 * @param Path `Assets`からの骨付きFBX相対パス。
	 * @param InPosition 置く場所。
	 * @return そのまま読み込み付き配置へ渡せる指定。
	 */
	static FAnimatedModel3DSpawnParams FromModel( FStringView Path, FVec3 InPosition ) noexcept;

	/**
	 * 数値と入力元が配置に使えるか返す。
	 *
	 * @return パスかアセットがあり、座標・色が有限で大きさに0が無ければtrue。
	 */
	bool IsValid() const noexcept;
};
