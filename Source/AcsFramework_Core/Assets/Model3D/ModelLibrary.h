// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

/**
 * `Assets` に置いたモデルを読み、置ける形にして返す。
 *
 * @details
 * ## 何をしているか
 *
 * 読み込み自体は ACS が持っている (`CAssetRegistry` と、拡張子ごとのローダ)。**FBX の
 * 解釈も ACS の `CFbxAssetLoader` (ufbx)。** この層が足すのは 3 つだけ。
 *
 * 1. `Assets` からの相対名で書けるようにする (`Robot.fbx`)
 * 2. 対応していない拡張子を**置く前に**弾く
 * 3. 読めなかった理由を 1 行残す
 *
 * ## 対応している形式
 *
 * `.fbx` `.gltf` `.glb` `.obj`。**この枠組みが薦めるのは FBX** で、他は ACS のローダが
 * 在るので通しているだけ。
 *
 * ## 二重読み込みについて
 *
 * `CAssetRegistry` がパスで覚えているので、同じモデルを 100 個置いても読むのは 1 回。
 * ここでは何も覚えない。
 */
class CModelLibrary
{
public:
	/**
	 * 使う登録簿を渡す。
	 *
	 * @details 渡す前に呼んだ `Load` は必ず失敗する。
	 *
	 * @param Registry `CApplication::GetAssets()`。
	 */
	void Bind( CAssetRegistry& Registry ) noexcept { m_Registry = &Registry; }

	/**
	 * 使える状態か。
	 *
	 * @return 登録簿を渡してあれば true。
	 */
	bool IsBound() const noexcept { return m_Registry != nullptr; }

	/**
	 * モデルを読む。
	 *
	 * @details
	 * 失敗しても例外は投げない。空を返し、理由を 1 行残す。**«置いたのに見えない» を
	 * 黙って起こさない**ことを優先している。
	 *
	 * @param RelativePath `Assets` からの相対名 (`Robot.fbx`、`Enemy/Slime.fbx`)。
	 * @return 読めたアセット。読めなければ空。
	 */
	TSharedPtr<AAsset> Load( FStringView RelativePath ) noexcept;

	/**
	 * 骨付きモデルを読む。
	 *
	 * @details
	 * **静的モデルとは別の口。** 同じ `.fbx` でも、骨の要る読み方と要らない読み方で
	 * 欲しいものが違うため、拡張子で自動に選ばせない。
	 *
	 * 骨の入っていないファイルを渡すと失敗する (黙って «動かないモデル» にはしない)。
	 * 読み込みは `CAssetRegistry` を通らないので、**同じファイルを 2 回渡すと 2 回読む**。
	 * 何体も置くなら結果を持ち回すこと。
	 *
	 * @param RelativePath `Assets` からの相対名。
	 * @return 読めた骨付きメッシュ。読めなければ空。
	 */
	TSharedPtr<ASkinnedMeshAsset> LoadSkinned( FStringView RelativePath ) noexcept;

	/**
	 * この拡張子を読めるか。
	 *
	 * @param RelativePath 調べる名前。
	 * @return 読めるなら true。
	 */
	static bool IsSupported( FStringView RelativePath ) noexcept;

private:
	/** 読み込みを頼む先。所有しない。 */
	CAssetRegistry* m_Registry = nullptr;
};
