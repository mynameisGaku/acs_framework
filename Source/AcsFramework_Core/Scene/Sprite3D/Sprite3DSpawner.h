// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Assets/Image/ImageLibrary.h"
#include "AcsFramework_Core/Scene/Sprite3D/Sprite3DSpawnParams.h"

using namespace acs;
using namespace acs::game;

/**
 * 画像をノードのローカルXY板へ結び、3Dシーンへ置く状態なしの接続層。
 *
 * @details 画像のデコードは`CImageLibrary`、GPU画像の所有と透過描画はACSの
 * `ALegacyScene3DAdapter`へ任せる。この型はノード、変換、`ASprite3DComponent`だけを作る。
 */
class CSprite3DSpawner
{
public:
	/** 読込済み画像を識別子付きノードとしてシーンへ置く。 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, const FSprite3DSpawnParams& Params,
		ANode* Parent = nullptr ) noexcept;

	/** 必要なら画像を読み、識別子付きノードとしてシーンへ置く。 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, const FSprite3DSpawnParams& Params,
		CImageLibrary& Library, ANode* Parent = nullptr ) noexcept;

	/** 読込済み画像を指定した親ノードの直下へ置く。 */
	static ANode* SpawnInto( ANode& Parent, const FSprite3DSpawnParams& Params ) noexcept;

	/** 必要なら画像を読み、指定した親ノードの直下へ置く。 */
	static ANode* SpawnInto( ANode& Parent, const FSprite3DSpawnParams& Params,
		CImageLibrary& Library ) noexcept;

private:
	/** 読込済みなら入力を保ち、未読込ならライブラリから画像を補う。 */
	static bool Prepare_Internal( const FSprite3DSpawnParams& Params, CImageLibrary& Library,
		FSprite3DSpawnParams& OutPrepared ) noexcept;

	/** 位置、度指定の回転、幅と高さをノードへ反映する。 */
	static void ApplyTransform_Internal( ANode& Node, const FSprite3DSpawnParams& Params ) noexcept;

	/** ACSの固定向き3Dスプライト部品へ画像パスと画像を渡す。 */
	static void ApplySprite_Internal( ANode& Node, const FSprite3DSpawnParams& Params ) noexcept;
};
