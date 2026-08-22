// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/Scene/Billboard3D/Billboard3DMode.h"

using namespace acs;
using namespace acs::game;

class CImageLibrary;
struct FSprite3DSpawnParams;

/**
 * 3D画像板を現在カメラへ向け続ける、場面寿命の追従レイヤー。
 *
 * @details ノードは世代付き`FNodeId`で追い、破棄済みノードとグラフ差し替えを自動で外す。
 * GPU資源は持たず、描画直前に対象ノードのローカル回転だけを更新する。
 */
class CBillboard3DLayer
{
public:
	/** 未接続で空の追従レイヤーを作る。 */
	CBillboard3DLayer() noexcept = default;

	/** 追従一覧を解放する。 */
	~CBillboard3DLayer() noexcept = default;

	/** 場面固有状態を重複所有しないためコピーを禁止する。 */
	CBillboard3DLayer( const CBillboard3DLayer& ) = delete;

	/** 場面固有状態を重複所有しないためコピー代入を禁止する。 */
	CBillboard3DLayer& operator=( const CBillboard3DLayer& ) = delete;

	/** 非所有グラフ接続を持ち越さないためムーブを禁止する。 */
	CBillboard3DLayer( CBillboard3DLayer&& ) = delete;

	/** 非所有グラフ接続を持ち越さないためムーブ代入を禁止する。 */
	CBillboard3DLayer& operator=( CBillboard3DLayer&& ) = delete;

	/** 場面グラフへ接続し、以前の追従一覧を全消去する。 */
	void Bind( CSceneNodeGraph& Graph ) noexcept;

	/** 追従一覧を消してグラフ接続を外す。 */
	void Unbind() noexcept;

	/** 場面グラフへ接続済みならtrueを返す。 */
	bool IsBound() const noexcept { return m_Graph != nullptr; }

	/** 指定グラフへ接続中ならtrueを返す。 */
	bool IsBoundTo( const CSceneNodeGraph& Graph ) const noexcept { return m_Graph == &Graph; }

	/**
	 * 画像を読み、3D画像板の生成とカメラ追従登録を1回で行う。
	 *
	 * @param Params 画像名、位置、大きさ、ノード名。
	 * @param Library 未読込画像を読む場面共通ライブラリ。
	 * @param Mode 上下も追うか、worldのY軸を保つか。
	 * @param RollDegrees 正面軸まわりへ加える度数。
	 * @param Parent 接続中グラフが所有する親。空ならroot直下。
	 * @return 生成して追従登録したノード。失敗時はnullptrで半端なノードを作らない。
	 */
	ANode* Spawn( const FSprite3DSpawnParams& Params, CImageLibrary& Library,
		EBillboard3DMode Mode = EBillboard3DMode::FaceCamera, f32 RollDegrees = 0.0f,
		ANode* Parent = nullptr ) noexcept;

	/**
	 * 既存の3D画像板をカメラ追従へ加える。同じノードは設定だけを更新する。
	 *
	 * @return 接続中グラフが所有する画像板で、設定を保存できたらtrue。
	 */
	bool Track( ANode& Node, EBillboard3DMode Mode = EBillboard3DMode::FaceCamera,
		f32 RollDegrees = 0.0f ) noexcept;

	/** 追従中画像板の向き設定を変更する。 */
	bool SetFacing( ANode& Node, EBillboard3DMode Mode, f32 RollDegrees = 0.0f ) noexcept;

	/** 指定画像板のカメラ追従だけを外し、ノード自体は残す。 */
	bool Remove( ANode& Node ) noexcept;

	/** 全画像板の追従を外し、グラフ接続は維持する。 */
	void Clear() noexcept;

	/** 追従中の画像板数を返す。 */
	u32 TrackedCount() const noexcept { return static_cast<u32>( m_Entries.Num() ); }

	/**
	 * 現在位置から全画像板のローカル回転を更新する。
	 *
	 * @param Camera このフレームを描く3Dカメラ。
	 * @return 有効な向きを反映できた画像板数。
	 */
	u32 UpdateFacing( const CCamera& Camera ) noexcept;

private:
	/** 追従対象1件の世代付き識別子と向き指定。 */
	struct FEntry
	{
		/** 接続中グラフで画像板を指す識別子。 */
		FNodeId Node;

		/** カメラへ向ける回転軸の制限。 */
		EBillboard3DMode Mode = EBillboard3DMode::FaceCamera;

		/** 正面軸まわりへ加える度数。 */
		f32 RollDegrees = 0.0f;
	};

	/** グラフのroot差し替えを検出し、古い識別子の追従を全て外す。 */
	bool RefreshGraphIdentity_Internal() noexcept;

	/** ノード識別子に一致する要素番号を返す。 */
	usize FindEntryIndex_Internal( FNodeId Node ) const noexcept;

	/** 接続中の場面ノードグラフ。所有しない。 */
	CSceneNodeGraph* m_Graph = nullptr;

	/** 接続時または直近確認時のrootポインタ。差し替え検出だけに使う。 */
	ANode* m_RootIdentity = nullptr;

	/** 追従対象を登録順に保持する一覧。 */
	TArray<FEntry> m_Entries;
};
