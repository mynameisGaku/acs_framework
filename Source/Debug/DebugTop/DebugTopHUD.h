#pragma once

#include <acs.h>

#include "Debug/DebugTop/Page/DebugTopEntity.h"
#include "Debug/DebugTop/Render/DebugTopFontCache.h"
#include "Debug/DebugTop/Widget/DebugTopFilterBox.h"
#include "Debug/DebugTop/Widget/DebugTopPathBrowser.h"
#include "Debug/DebugTop/Widget/DebugTopSearchBox.h"
#include "Debug/DebugTop/Widget/DebugTopTooltip.h"
#include "Debug/DebugTop/Service/DebugTopHistory.h"
#include "Debug/DebugTop/Service/DebugTopSearchIndex.h"

using namespace acs;

/**
 * デバッグメニュー全体のルート。派生して使うことを想定した基底。
 *
 * @details
 * ページ (ADebugTopEntity) の木を所有し、いま表示しているページを 1 つ指す。画面上部へ
 * タイトルとパンくずを、右下へ現在ページの説明文を描き、行の入力処理と描画は現在ページへ
 * 委譲する。描画は本クラスと ADebugTopEntity だけが行い、CDebugTopElement は文字列とデータを
 * 返すだけにする。
 * どのページにどの行を置くかはここでは決めない。呼び出し側が AddEntity で組み立てる。
 * 文字サイズは SetFontSize でメニュー全体へ一括で効く。
 * 派生では Update / Draw を、部分的に差し替えたいだけなら OnDrawTitle / OnDrawDescription /
 * BuildBreadcrumb / WantsReturnToParent / GetOriginX / GetOriginY を override する。
 */
class ADebugTopHUD : public AObject
{
public:
	/** 空のメニューを構築する。 */
	ADebugTopHUD();

	/** ページと専用フォントを解放する (TObjectPtr の解体をこの翻訳単位に閉じる)。 */
	~ADebugTopHUD() noexcept override;

	/**
	 * ルート直下のページを作って追加する。
	 *
	 * @details 最初に追加したページが初期表示ページになる。
	 * @param Name ページ名。
	 * @return 生成したページ (中身はこの戻り値へ積む)。確保に失敗したら nullptr。
	 */
	ADebugTopEntity* AddEntity( const FString& Name );

	/**
	 * 生成済みのページをルート直下へ追加する。
	 *
	 * @details ADebugTopEntity を派生したページを置くときはこちらを使う。
	 * @param Entity 追加するページ (所有権を受け取る)。
	 * @return 追加したページ (Entity が空なら nullptr)。
	 */
	ADebugTopEntity* AddEntity( TObjectPtr<ADebugTopEntity> Entity );

	/**
	 * ルート直下のページを取り外し、所有権を返す。
	 *
	 * @details 取り外したページ (またはその子孫) を表示中だった場合は、残っている先頭ページへ移る。
	 * @param Entity 取り外すページ。
	 * @return 取り外したページ (見つからなければ空)。
	 */
	TObjectPtr<ADebugTopEntity> RemoveEntity( ADebugTopEntity* Entity ) noexcept;

	/** ルート直下のページを返す。 */
	const TArray<TObjectPtr<ADebugTopEntity>>& GetEntities() const noexcept { return m_Entities; }

	/**
	 * メニュー全体を組み立てる。
	 *
	 * @details OnBuild を 1 回だけ呼ぶ。二度目以降の呼び出しは無視する。
	 */
	void Build() noexcept;

	/** Build 済みかを返す。 */
	bool IsBuilt() const noexcept { return m_bBuilt; }

	/** いま表示しているページを返す (未設定なら nullptr)。 */
	ADebugTopEntity* GetCurrentEntity() const noexcept { return m_CurrentEntity; }

	/**
	 * 表示するページを差し替える。
	 *
	 * @param Entity 表示するページ (この HUD が所有する木の中のページであること)。
	 */
	void SetCurrentEntity( ADebugTopEntity* Entity ) noexcept { m_CurrentEntity = Entity; }

	/** メニュー全体の文字サイズ (ピクセル) を返す。0 ならエンジン共有フォントのまま。 */
	f32 GetFontSize() const noexcept { return m_FontCache.GetFontSize(); }

	/**
	 * メニュー全体の文字サイズを設定する。
	 *
	 * @details
	 * 行の高さ・段差・右カラムの位置は全てこのサイズに追従するので、大きくしても配置は崩れない。
	 * 焼き直しの面倒は CDebugTopFontCache が見る (詳しくはそちら)。
	 * 既定は 0 (共有フォントをそのまま使う。追加コスト無しだが漢字は出ない)。
	 * @param FontSize 文字のピクセルサイズ (0 以下でエンジン共有フォントをそのまま使う)。
	 */
	void SetFontSize( f32 FontSize ) noexcept { m_FontCache.SetFontSize( FontSize ); }

	/** 専用フォントへ漢字を焼き込むかを返す。 */
	bool IsFontIncludeCjk() const noexcept { return m_FontCache.IsIncludeCjk(); }

	/**
	 * 専用フォントへ漢字を焼き込むかを設定する。
	 *
	 * @details
	 * SetFontSize で専用フォントを使う場合にのみ効く。既定は true (漢字を含むラベルや説明文を
	 * そのまま書けるようにするため)。漢字を焼くとアトラスが 4096 四方まで大きくなり、最初の
	 * 描画で数百 ms かかることがあるので、英数字しか使わないなら false にすると軽くなる。
	 * @param bIncludeCjk true で漢字を含める。
	 */
	void SetFontIncludeCjk( bool bIncludeCjk ) noexcept { m_FontCache.SetIncludeCjk( bIncludeCjk ); }

	/** 現在ページに説明文が無いときへ出す、メニュー共通の説明文を返す。 */
	const FString& GetDescription() const noexcept { return m_Description; }

	/**
	 * メニュー共通の説明文を設定する。
	 *
	 * @details
	 * 画面右下へ出る。説明文を持たないページを表示している間だけ出るので、全ページ共通の
	 * ボタンヒントを 1 か所に書いておく用途に使う。\\n で複数行にできる。
	 * @param Description 設定する説明文 (空なら何も出さない)。
	 */
	void SetDescription( const FString& Description ) { m_Description = Description; }

	/**
	 * いま表示しているページのカーソル行を返す。
	 *
	 * @details 値を取り出すには GetValueKind() で種別を確かめてから TryGetInt / TryGetFloat /
	 * TryGetBool を使う。配列を展開して要素行にカーソルがある場合はその要素の値が取れる。
	 * @return カーソル行 (ページ未設定または可視行が無ければ nullptr)。
	 */
	CDebugTopElement* GetCursorElement() const noexcept;

	/**
	 * 入力を処理して 1 フレーム進める (ページ遷移と親ページへの復帰もここで行う)。
	 *
	 * @param DeltaSeconds 前フレームからの経過秒 (ダブルクリックの判定に使う)。
	 */
	virtual void Update( f32 DeltaSeconds ) noexcept;

	/**
	 * タイトル・パンくず・現在ページ・説明文を描画する。
	 *
	 * @param RenderContext 画面サイズとフォントの取得元。
	 * @param Batch 描画コマンドを積む先。
	 */
	virtual void Draw( FRenderContext& RenderContext, CSpriteBatch& Batch ) noexcept;

protected:
	/**
	 * ルート直下のページを積む。派生 HUD はここでメニュー全体を登録する。
	 *
	 * @details
	 * コンストラクタからでは仮想呼び出しが効かないため、Build から呼ばれる。
	 * AddEntity で足したページはその場で Build (= OnBuild) が走る。
	 */
	virtual void OnBuild() noexcept {}

	/**
	 * タイトルとパンくずを描き、消費した高さを返す。
	 *
	 * @param RenderContext 画面サイズの取得元。
	 * @param Batch 描画コマンドを積む先。
	 * @param Text 描画に使うフォントと文字サイズ。
	 * @param OriginX 左端 X (ピクセル)。
	 * @param OriginY 上端 Y (ピクセル)。
	 * @return 消費した高さ (ピクセル)。この下からページの描画が始まる。
	 */
	virtual f32 OnDrawTitle( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text, f32 OriginX, f32 OriginY ) noexcept;

	/**
	 * 説明文を画面右下へ描く。
	 *
	 * @details 行の一覧に重なっても読めるよう、下敷きを敷いてから描く。
	 * @param RenderContext 画面サイズの取得元。
	 * @param Batch 描画コマンドを積む先。
	 * @param Text 描画に使うフォントと文字サイズ。
	 * @param Description 描く説明文 (空なら何も描かない)。
	 * @param Color 文字色。
	 */
	virtual void OnDrawDescription( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text, const FString& Description, const FVec4& Color ) noexcept;

	/**
	 * ルートから現在ページまでのパンくずを組み立てる。
	 *
	 * @return パンくず文字列。
	 */
	virtual FString BuildBreadcrumb() const;

	/**
	 * 親ページへ戻る操作が入ったかを返す。
	 *
	 * @return 戻る操作なら true。既定は Backspace か Escape かマウス右クリック。
	 */
	virtual bool WantsReturnToParent() const noexcept;

	/** 描画の左端 X (ピクセル) を返す。 */
	virtual f32 GetOriginX() const noexcept;

	/** 描画の上端 Y (ピクセル) を返す。 */
	virtual f32 GetOriginY() const noexcept;

private:
	/**
	 * 検索語に合う行を集める (検索欄から呼ばれる)。
	 *
	 * @details 何を対象に探すかは検索欄ではなくメニューの構造の話なので、こちらが答える。
	 * @param Query 検索語。
	 * @param OutHits 候補の書き込み先。
	 */
	void CollectSearchHits( const FString& Query, TArray<FDebugTopSearchHit>& OutHits );

	/**
	 * 指した行の吹き出しを 1 フレーム進める。
	 *
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	void UpdateTooltip( f32 DeltaSeconds ) noexcept;

	/**
	 * 取り消し / やり直しのキーを見る。
	 *
	 * @details Ctrl+Z で 1 手戻し、Ctrl+Y (または Ctrl+Shift+Z) で戻した手をやり直す。
	 * @return 受け付けたら true (この後の操作は見ない)。
	 */
	bool UpdateHistoryKeys() noexcept;

	/**
	 * 選ばれた候補のページへ移り、その行へカーソルを合わせる。
	 *
	 * @param Hit 選ばれた候補。
	 */
	void JumpToHit( const FDebugTopSearchHit& Hit ) noexcept;

	/** ルート直下のページ。子ページは各ページが所有する。 */
	TArray<TObjectPtr<ADebugTopEntity>> m_Entities;

	/** 現在ページが説明文を持たないときに出す、メニュー共通の説明文。 */
	FString m_Description;

	/** 文字を焼いて持っておくもの。サイズと収録範囲の面倒はこちらが見る。 */
	CDebugTopFontCache m_FontCache;

	/** いま表示しているページ。実体は m_Entities を根とする木が所有するので所有しない。 */
	ADebugTopEntity* m_CurrentEntity = nullptr;

	/** 検索欄。どのページにいても同じ操作で開けるよう、ページ側ではなくここが持つ。 */
	CDebugTopSearchBox m_SearchBox;

	/** ページ内の絞り込み欄。検索と違い、ページを移らずいまの一覧を狭める。 */
	CDebugTopFilterBox m_FilterBox;

	/** 指した行の説明を出す吹き出し。右下のパネルはカーソル行、こちらは指した行を出す。 */
	CDebugTopTooltip m_Tooltip;

	/** 値の変更の控え。Ctrl+Z / Ctrl+Y で 1 手ずつ戻したり進めたりする。 */
	CDebugTopHistory m_History;

	/** パスを選ぶ一覧。どのページの行から頼まれても同じものを出すので、ここが持つ。 */
	CDebugTopPathBrowser m_PathBrowser;

	/** OnBuild を呼び終えたか (二重構築の防止)。 */
	bool m_bBuilt = false;

	/**
	 * 控えを始めたか。
	 *
	 * @details
	 * ページを組み終え、保存してあった設定も戻し終えた状態を「元の値」にしたい。それが済むのは
	 * 最初の更新なので、構築時ではなくそこで始める。
	 */
	bool m_bHistoryBegun = false;
};
