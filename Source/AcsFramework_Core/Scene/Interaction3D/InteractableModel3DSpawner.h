// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;
using namespace acs::game;

class CInteractionFocus3D;
class CModelLibrary;
struct FAnimatedModel3DSpawnParams;
struct FModel3DSpawnParams;

/** 3Dモデル生成と視線フォーカス対象登録を一括化する状態なしアダプター。 */
class CInteractableModel3DSpawner
{
public:
	/**
	 * 読み込み済みまたはプリミティブの静的モデルを操作対象として置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 静的モデルの配置と見た目。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 生成と対象登録を完了したノード。失敗時はnullptrで、半端な生成物を残さない。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, CInteractionFocus3D& Focus,
		const FModel3DSpawnParams& ModelParams, FStringView Prompt,
		FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 必要なら静的モデルを読み、操作対象として置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 静的モデルの配置と見た目。
	 * @param Library `MeshPath`の読み込み先。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 生成と対象登録を完了したノード。失敗時はnullptrで、半端な生成物を残さない。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, CInteractionFocus3D& Focus,
		const FModel3DSpawnParams& ModelParams, CModelLibrary& Library,
		FStringView Prompt, FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 読み込み済み骨格モデルを、初期再生付きの操作対象として置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 骨格モデルの配置と初期再生。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 生成と対象登録を完了したノード。失敗時はnullptrで、半端な生成物を残さない。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, CInteractionFocus3D& Focus,
		const FAnimatedModel3DSpawnParams& ModelParams, FStringView Prompt,
		FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 必要なら骨格モデルを読み、初期再生付きの操作対象として置く。
	 *
	 * @param Graph 置く場面のノードグラフ。
	 * @param Focus `Graph`へ接続済みの視線フォーカス。
	 * @param ModelParams 骨格モデルの配置と初期再生。
	 * @param Library `MeshPath`の読み込み先。
	 * @param Prompt フォーカス中だけ表示する1から4096byteのUTF-8文字列。
	 * @param WorldOffset ノード位置から操作案内までのworld空間のずれ。
	 * @param Parent 繋ぐ親。nullptrなら場面のルート。
	 * @return 生成と対象登録を完了したノード。失敗時はnullptrで、半端な生成物を残さない。
	 */
	static ANode* SpawnInto( CSceneNodeGraph& Graph, CInteractionFocus3D& Focus,
		const FAnimatedModel3DSpawnParams& ModelParams, CModelLibrary& Library,
		FStringView Prompt, FVec3 WorldOffset = FVec3{ 0.0f, 1.8f, 0.0f },
		ANode* Parent = nullptr ) noexcept;

private:
	/** 生成ノードを対象登録し、失敗時はノードを破棄予定へ戻す。 */
	static ANode* RegisterOrRollback_Internal( CSceneNodeGraph& Graph,
		CInteractionFocus3D& Focus, ANode* Node, FStringView Prompt,
		FVec3 WorldOffset ) noexcept;
};
