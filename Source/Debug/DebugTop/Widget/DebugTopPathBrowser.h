#pragma once

#include <acs.h>

#include "Debug/DebugTop/Input/DebugTopKeyNav.h"
#include "Debug/DebugTop/Render/DebugTopDraw.h"
#include "Debug/DebugTop/Service/DebugTopDirectory.h"
#include "Debug/DebugTop/Service/DebugTopFileDialog.h"

using namespace acs;

/**
 * 選ばれたパスを受け取るデリゲート。
 *
 * @details
 * 選ばれた先で何をするかは一覧の知るところではないので、頼む側が差す。頼んだものは
 * 一覧が開いている間ずっと生きていること。
 */
using FDebugTopPathChosen = TDelegate<void( const FString& )>;


/**
 * メニューの中でフォルダを辿ってパスを選ぶ、浮かぶ一覧。
 *
 * @details
 * OS の選択ダイアログ (DebugTopPickPath) はゲームを止めるうえ、フルスクリーンだと裏へ回って
 * 見えなくなる。こちらはメニューの一部として描くので、動かしたまま選べる。
 *
 * 操作はメニューの一覧と同じにしてある。上下で選び (押しっぱなしで送れる)、決定でフォルダへ
 * 降りる / 選び終える、Backspace で 1 つ上へ、Esc で閉じる。文字を打つと名前で絞り込む。
 *
 * ファイルシステムのことは自分では読まない (Service/DebugTopDirectory に任せる)。開いた理由も、
 * 選ばれた先で何をするかも知らない。選ばれたパスは、頼むときに差されたデリゲートへ渡すだけ。
 *
 * @code
 * // 持ち主の Update
 * if ( m_Browser.Update( DeltaSeconds ) ) return;   // 開いている間は入力を後ろへ流さない
 * @endcode
 */
class CDebugTopPathBrowser
{
public:
	/** 閉じた状態で構築する。 */
	CDebugTopPathBrowser() noexcept = default;

	/** 開いている一覧を、この実体にする (メニューの構築時に 1 度だけ呼ぶ)。 */
	void MakeActive() noexcept { s_Active = this; }

	/** 自分が受け皿なら外す。 */
	void ClearActive() noexcept { if ( s_Active == this ) s_Active = nullptr; }

	/** いま入力を受けられる一覧を返す (無ければ nullptr)。 */
	static CDebugTopPathBrowser* GetActive() noexcept { return s_Active; }

	/**
	 * 開く。
	 *
	 * @param Kind 選ばせるものの種類 (フォルダ / ファイル)。
	 * @param Initial 最初に開いておく場所 (ファイルを渡せばその親を開く。空ならドライブの一覧)。
	 * @param OnChosen 選ばれたパスの受け取り先。
	 */
	void Open( EDebugTopPickKind Kind, const FString& Initial, FDebugTopPathChosen OnChosen );

	/** 閉じる (選ばれていないものとして扱う)。 */
	void Close() noexcept;

	/** 開いているかを返す。 */
	bool IsOpen() const noexcept { return m_bOpen; }

	/**
	 * 入力を 1 フレーム進める。
	 *
	 * @details 開いている間は入力を全て受け取るので、持ち主は true が返ったら後ろへ流さないこと。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 * @return 開いていて入力を受け取ったなら true。
	 */
	bool Update( f32 DeltaSeconds );

	/**
	 * 一覧を描く。
	 *
	 * @details 押されたかの判定に使う矩形をここで控えるので、開いている間は毎フレーム呼ぶこと。
	 * @param Batch 描画コマンドを積む先。
	 * @param Text 描画に使うフォントと文字サイズ。
	 * @param ScreenWidth 画面の幅 (中央へ置くために使う)。
	 * @param ScreenHeight 画面の高さ。
	 */
	void Draw( CSpriteBatch& Batch, const CDebugTopText& Text, f32 ScreenWidth, f32 ScreenHeight ) noexcept;

private:
	/**
	 * いまのフォルダを読み直して、絞り込みも掛け直す。
	 *
	 * @param Focus 読み直した後、名前が一致するものへカーソルを合わせる (空なら先頭)。
	 */
	void Rebuild( const FString& Focus );

	/**
	 * 1 つ上のフォルダへ上がる (根まで来ていればドライブの一覧を出す)。
	 */
	void GoUp();

	/**
	 * 選び終えて閉じる。
	 *
	 * @param Path 選ばれたパス。
	 */
	void Finish( const FString& Path );

	/**
	 * カーソルの位置のものを決定する。
	 *
	 * @details フォルダなら降り、ファイルなら選び終える。フォルダを選ばせているときは、
	 * 決定は「降りる」であって「選ぶ」ではない (選ぶのは別のキー)。
	 */
	void Decide();

	/** いま開いているフォルダを選び終える (フォルダを選ばせているときだけ意味を持つ)。 */
	void ChooseCurrentFolder();

	/** 表示している一覧 (絞り込み済み)。 */
	TArray<FDebugTopDirEntry> m_Visible;

	/** 読み込んだままの一覧 (絞り込み前)。 */
	TArray<FDebugTopDirEntry> m_Entries;

	/** いま開いているフォルダ (空ならドライブの一覧を出している)。 */
	FString m_Directory;

	/** 名前の絞り込み (空なら全て出す)。 */
	FString m_Filter;

	/** 選ばれたパスの受け取り先。 */
	FDebugTopPathChosen m_OnChosen;

	/** 一覧を上下に送る矢印キー (押しっぱなしで連射する)。 */
	CDebugTopKeyNav m_KeyNav;

	/** 選ばせるものの種類。 */
	EDebugTopPickKind m_Kind = EDebugTopPickKind::Folder;

	/** 一覧の中で選んでいる位置。 */
	i32 m_Cursor = 0;

	/** 一覧の一番上に出している位置 (行数が入りきらないときにずらす)。 */
	i32 m_Scroll = 0;

	/** 直近の描画で出せた行数 (スクロールの量を決めるのに使う)。 */
	i32 m_VisibleRowCount = 1;

	/** 開いているか。 */
	bool m_bOpen = false;

	/** 簡易関数から引くための実体。 */
	static CDebugTopPathBrowser* s_Active;
};


/**
 * メニューの中の一覧でパスを選ばせる。
 *
 * @details
 * OS のダイアログ (DebugTopPickPath) と違い、ゲームを止めずに選べる。行の側は誰が一覧を
 * 持っているかを知らずに済むよう、ここを通して頼む。
 * 受け皿が無い場面 (メニューを出していない等) では何も起きず false を返す。
 * @param Kind 選ばせるものの種類。
 * @param Initial 最初に開いておく場所。
 * @param OnChosen 選ばれたパスの受け取り先。
 * @return 開けたら true。
 */
bool DebugTopBrowsePath( EDebugTopPickKind Kind, const FString& Initial, FDebugTopPathChosen OnChosen );
