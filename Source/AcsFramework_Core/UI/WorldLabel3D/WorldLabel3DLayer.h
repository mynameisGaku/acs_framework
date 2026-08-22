// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabel3DHandle.h"
#include "AcsFramework_Core/UI/WorldLabel3D/WorldLabel3DParams.h"

using namespace acs;
using namespace acs::game;

/**
 * ノードまたはworld位置へ追従する文字を、3D描画後のHUDへ重ねる場面所有レイヤー。
 *
 * @details `AUi3DScene`が場面のノードグラフへ接続し、描画時に現在カメラから射影する。
 * ノードは`FNodeId`で解決し、グラフのroot差し替え時は登録を全消去するため、破棄やscene読込後に
 * 別物へ誤追従しない。
 * GPU資源は所有せず、ACSの共有`FFont`と開かれている`CSpriteBatch`だけを借りる。
 */
class CWorldLabel3DLayer
{
public:
	/** 未接続で空のレイヤーを作る。 */
	CWorldLabel3DLayer() noexcept = default;

	/** 所有文字列を解放する。 */
	~CWorldLabel3DLayer() noexcept = default;

	/** 場面固有の状態を重複所有しないためコピーを禁止する。 */
	CWorldLabel3DLayer( const CWorldLabel3DLayer& ) = delete;

	/** 場面固有の状態を重複所有しないためコピー代入を禁止する。 */
	CWorldLabel3DLayer& operator=( const CWorldLabel3DLayer& ) = delete;

	/** 非所有のグラフ接続を移動で持ち越さないためムーブを禁止する。 */
	CWorldLabel3DLayer( CWorldLabel3DLayer&& ) = delete;

	/** 非所有のグラフ接続を移動で持ち越さないためムーブ代入を禁止する。 */
	CWorldLabel3DLayer& operator=( CWorldLabel3DLayer&& ) = delete;

	/**
	 * 場面のノードグラフへ接続し、以前のラベルを全て消す。
	 *
	 * @param Graph 追従対象を解決する場面所有グラフ。
	 */
	void Bind( CSceneNodeGraph& Graph ) noexcept;

	/** 全ラベルを消してグラフ接続を外す。 */
	void Unbind() noexcept;

	/** グラフへ接続済みならtrueを返す。 */
	bool IsBound() const noexcept { return m_Graph != nullptr; }

	/**
	 * 指定したノードグラフへ接続中ならtrueを返す。
	 *
	 * @param Graph 同じ場面か確認するグラフ。
	 * @return 接続中の非所有ポインタが指定グラフならtrue。
	 */
	bool IsBoundTo( const CSceneNodeGraph& Graph ) const noexcept { return m_Graph == &Graph; }

	/**
	 * 1つのノード位置へ追従するラベルを加える。
	 *
	 * @param Node 接続中グラフが所有し、有効な`FNodeId`を持つノード。
	 * @param Params 複製する文字と見た目。
	 * @return 追加したラベルのhandle。未接続、不正ノード、不正値、確保失敗なら無効。
	 */
	FWorldLabel3DHandle AddNodeLabel( ANode& Node, const FWorldLabel3DParams& Params ) noexcept;

	/**
	 * 固定world位置へラベルを加える。
	 *
	 * @param WorldPosition 基準にする有限のworld位置。
	 * @param Params 複製する文字と見た目。
	 * @return 追加したラベルのhandle。未接続、不正値、確保失敗なら無効。
	 */
	FWorldLabel3DHandle AddWorldLabel( FVec3 WorldPosition, const FWorldLabel3DParams& Params ) noexcept;

	/**
	 * 表示文字列を先に複製してから安全に差し替える。
	 *
	 * @param Handle 変更するラベル。
	 * @param Text 新しい1から4096byteのUTF-8文字列。
	 * @return 差し替えられたらtrue。失敗時は以前の文字列を保つ。
	 */
	bool SetText( FWorldLabel3DHandle Handle, FStringView Text ) noexcept;

	/**
	 * 固定worldラベルの基準位置を変更する。
	 *
	 * @param Handle 変更する固定worldラベル。
	 * @param WorldPosition 新しい有限位置。
	 * @return 変更できたらtrue。ノード追従ラベルまたは不正値ではfalse。
	 */
	bool SetWorldPosition( FWorldLabel3DHandle Handle, FVec3 WorldPosition ) noexcept;

	/**
	 * ラベルの表示状態を変更する。
	 *
	 * @param Handle 変更するラベル。
	 * @param bVisible 表示候補にするならtrue。
	 * @return 対象が見つかればtrue。
	 */
	bool SetVisible( FWorldLabel3DHandle Handle, bool bVisible ) noexcept;

	/**
	 * 現在の表示文字列を返す。
	 *
	 * @param Handle 読むラベル。
	 * @return レイヤー所有文字列へのview。見つからなければ空。
	 */
	FStringView Text( FWorldLabel3DHandle Handle ) const noexcept;

	/**
	 * ラベルを1件削除する。
	 *
	 * @param Handle 削除するラベル。
	 * @return 削除できたらtrue。古いhandleまたは無効値ではfalse。
	 */
	bool Remove( FWorldLabel3DHandle Handle ) noexcept;

	/** 全ラベルを削除し、グラフ接続とhandle発行値は維持する。 */
	void Clear() noexcept;

	/** 登録中のラベル数を返す。 */
	u32 LabelCount() const noexcept { return static_cast<u32>( m_Entries.Num() ); }

	/**
	 * 1件の現在位置を画面へ射影する公開アダプター。
	 *
	 * @param Handle 射影するラベル。
	 * @param Camera 現在の3Dカメラ。
	 * @param ViewportWidth 画面幅。
	 * @param ViewportHeight 画面高さ。
	 * @param OutScreenPosition 成功時だけ書き換えるpixel位置。
	 * @return ラベル、追従ノード、表示状態、画面内射影がすべて有効ならtrue。
	 */
	bool TryProjectLabel( FWorldLabel3DHandle Handle, const CCamera& Camera, u32 ViewportWidth, u32 ViewportHeight, FVec2& OutScreenPosition ) noexcept;

	/**
	 * 画面内の全ラベルを、ACSの共有フォントとHUDバッチへ積む。
	 *
	 * @param Camera 現在描画に使う3Dカメラ。
	 * @param Context 画面寸法と共有フォントを持つ描画文脈。
	 * @param Sprites 開かれているHUD用スプライトバッチ。
	 */
	void Draw( const CCamera& Camera, FRenderContext& Context, CSpriteBatch& Sprites ) noexcept;

private:
	/** ラベル1件の所有文字列、追従先、見た目。 */
	struct FEntry
	{
		/** 場面内で一意な公開handle。 */
		FWorldLabel3DHandle Handle;

		/** レイヤーが所有するNUL終端UTF-8文字列。 */
		FString Text;

		/** ノード追従時の世代付き識別子。固定位置では無効。 */
		FNodeId Node;

		/** 固定ラベルの基準world位置。 */
		FVec3 WorldPosition;

		/** 基準world位置へ足すずれ。 */
		FVec3 WorldOffset;

		/** 射影後へ足すpixel単位のずれ。 */
		FVec2 ScreenOffset;

		/** 文字色。 */
		FVec4 TextColor;

		/** 背景色。 */
		FVec4 BackgroundColor;

		/** カメラから表示する最大距離。 */
		f32 MaximumDistance = 1.0f;

		/** 背景の左右余白。 */
		f32 HorizontalPadding = 0.0f;

		/** 背景の上下余白。 */
		f32 VerticalPadding = 0.0f;

		/** 呼び出し側が表示候補にしているか。 */
		bool bVisible = true;

		/** 半透明背景を描くか。 */
		bool bDrawBackground = true;

		/** 射影点を文字列の横中央に合わせるか。 */
		bool bCentered = true;

		/** ノード追従ならtrue、固定world位置ならfalse。 */
		bool bAttachedToNode = false;
	};

	/** 指定から所有文字列を含む候補1件を作る。 */
	static bool TryMakeEntry_Internal( const FWorldLabel3DParams& Params, FEntry& OutEntry ) noexcept;

	/** 文字列を確保失敗時に元を変えず複製する。 */
	static bool TryCopyText_Internal( FStringView Text, FString& OutText ) noexcept;

	/** ノード自身と全祖先が表示可能ならtrueを返す。 */
	static bool IsNodeVisible_Internal( const ANode& Node ) noexcept;

	/** 1件の追従先を解決して画面へ射影する。 */
	bool TryProjectEntry_Internal( const FEntry& Entry, const CCamera& Camera, u32 ViewportWidth, u32 ViewportHeight, FVec2& OutScreenPosition ) const noexcept;

	/** グラフのroot差し替えを検出し、古い識別子の登録を全て消す。 */
	bool RefreshGraphIdentity_Internal() noexcept;

	/** handleに一致する要素番号を返す。 */
	usize FindEntryIndex_Internal( FWorldLabel3DHandle Handle ) const noexcept;

	/** 接続中の場面ノードグラフ。所有しない。 */
	CSceneNodeGraph* m_Graph = nullptr;

	/** 接続時または直近確認時のrootポインタ。差し替え検出だけに使う。 */
	ANode* m_RootIdentity = nullptr;

	/** 文字列と見た目を所有する登録順一覧。 */
	TArray<FEntry> m_Entries;

	/** 次に発行する値。0へ回った後は追加を拒否する。 */
	u32 m_NextHandle = 1u;
};
