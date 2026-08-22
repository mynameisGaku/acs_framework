// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * 画像を固定向きの3D板として置くときの指定。
 *
 * @details 位置と大きさはworld単位、回転は度で受け取る。画像は`CImageLibrary`を渡す
 * `CSprite3DSpawner::SpawnInto`でパスから読めるほか、読込済みアセットも直接渡せる。
 */
struct FSprite3DSpawnParams
{
	/** `Assets`からの画像相対名。保存と再読込にも使う。 */
	FStringView TexturePath;

	/** 読込済み画像。空なら画像ライブラリ付き生成で`TexturePath`から読む。 */
	TSharedPtr<AAsset> ImageAsset;

	/** 親ノード内での板中心位置。 */
	FVec3 Position{ 0.0f, 0.0f, 0.0f };

	/** 親ノード内での向き。XYZの度数で指定する。 */
	FVec3 RotationDeg{ 0.0f, 0.0f, 0.0f };

	/** world単位の幅と高さ。0は見えないため受け付けない。 */
	FVec2 Size{ 1.0f, 1.0f };

	/** ノードへ付ける名前。空なら名前を付けない。 */
	FStringView Name;

	/**
	 * 画像名、位置、大きさから配置指定を作る。
	 *
	 * @param Path `Assets`からの画像相対名。
	 * @param InPosition 親ノード内での板中心位置。
	 * @param InSize world単位の幅と高さ。
	 */
	static FSprite3DSpawnParams FromImage( FStringView Path, FVec3 InPosition,
		FVec2 InSize = FVec2{ 1.0f, 1.0f } ) noexcept;

	/** 画像源、位置、回転、大きさが配置可能な値ならtrueを返す。 */
	bool IsValid() const noexcept;

	/** 有効かつ読込済み画像を持ち、直ちに配置できるならtrueを返す。 */
	bool IsReady() const noexcept;
};
