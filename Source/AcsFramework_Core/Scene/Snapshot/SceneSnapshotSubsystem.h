// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Snapshot/SceneSnapshotBuffer.h"

using namespace acs;
using namespace acs::game;

/**
 * ノードの木をファイルへ落とし、ファイルから戻すサブシステム。
 *
 * @details
 * 落とす / 起こすはエンジン (`TrySaveNodeTree` / `TryLoadNodeTree`)、ファイルの置き換えは
 * `CSaveArchive` が持っている。ここが引き受けるのは、その 3 つを繋ぐ順番と、使い回す入れ物。
 *
 * 1. 入れ物を持って使い回す (保存のたびにヒープを叩かない)
 * 2. 落とす → 置く / 読む → 起こす の順を固定する
 * 3. 最後に何が起きたかを覚えておく (画面へ出せるように)
 *
 * **どのノードを保存するかは決めない。** 呼ぶ側が起点を渡す。
 *
 * @code
 * Snapshot->SaveToFile( RootNode, FString( "Saved/Scene/Stage1.acssave" ) );
 *
 * TObjectPtr<ANode> Restored = Snapshot->LoadFromFile( FString( "Saved/Scene/Stage1.acssave" ) );
 * @endcode
 */
class CSceneSnapshotSubsystem : public ASubsystem
{
public:
	/** サブシステムの型 ID と診断名を実装する。 */
	ACS_SUBSYSTEM_KIND( CSceneSnapshotSubsystem )

	/**
	 * 木をファイルへ落とす。
	 *
	 * @param Root 起点のノード。
	 * @param Path 置き先のパス (UTF-8)。
	 * @return 置けたら true。
	 */
	bool SaveToFile( const ANode& Root, const FString& Path ) noexcept;

	/**
	 * ファイルから木を起こす。
	 *
	 * @details 起こした木はどこにも属していない。呼ぶ側が親へ付けるか、自分で持つこと。
	 * @param Path 読み元のパス (UTF-8)。
	 * @return 起こした木 (失敗したら空)。
	 */
	TObjectPtr<ANode> LoadFromFile( const FString& Path ) noexcept;

	/** 最後の書き出し / 読み込みの結果を返す。 */
	ESceneSerializeError GetLastError() const noexcept { return m_LastError; }

	/** 最後の結果を日本語で返す。 */
	FString MakeLastErrorMessage() const;

	/** 最後に書き出した大きさ (バイト) を返す。 */
	usize GetLastWrittenBytes() const noexcept { return m_LastWrittenBytes; }

	/** これまでに書き出した回数を返す。 */
	u64 GetSaveCount() const noexcept { return m_SaveCount; }

	/** これまでに読み込んだ回数を返す。 */
	u64 GetLoadCount() const noexcept { return m_LoadCount; }

	/** 使い回している入れ物を手放す (しばらく保存しないと分かっているとき)。 */
	void ReleaseBuffer() noexcept { m_Buffer.Release(); }

private:
	/** 落とす / 読むで使い回す入れ物。 */
	CSceneSnapshotBuffer m_Buffer;

	/** 最後の結果。 */
	ESceneSerializeError m_LastError = ESceneSerializeError::None;

	/** 最後に書き出した大きさ。 */
	usize m_LastWrittenBytes = 0u;

	/** 書き出した回数。 */
	u64 m_SaveCount = 0u;

	/** 読み込んだ回数。 */
	u64 m_LoadCount = 0u;
};
