// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Input/DebugTopCursor.h"
#include "Debug/DebugTop/Render/DebugTopDraw.h"
#include "Debug/DebugTop/Render/DebugTopColorField.h"
#include "Debug/DebugTop/Widget/DebugTopColorPicker.h"
#include "Debug/DebugTop/Element/DebugTopElements.h"
#include "Debug/DebugTop/Input/DebugTopGamepad.h"
#include "Debug/DebugTop/Input/DebugTopKeyNav.h"
#include "Debug/DebugTop/Page/DebugTopRowMouse.h"
#include "Debug/DebugTop/Page/DebugTopRowScroller.h"
#include "Debug/DebugTop/Page/DebugTopValueEditor.h"

using namespace acs;

/**
 * 子 Entity を親ページへ組み込む方法。
 */
enum class EDebugTopAttachMode : u8
{
	/** 遷移行を 1 行置き、決定で画面ごと切り替える。項目が多いページ向け。 */
	Page,

	/** 子 Entity の行を親ページの中へその場で展開する。画面は切り替わらない。 */
	Inline,
};


/**
 * 子ページへ移動する行。
 *
 * @details
 * ADebugTopEntity::AddChildEntity が自動で 1 行追加するため、通常は直接生成しない。
 * 遷移先は自分では切り替えず、置かれているページへ要求を出すだけにして、
 * Element 層が ADebugTopHUD を知らずに済むようにしている。
 */
class CDebugTopElementEntityLink : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param SubTitle 右カラムへ出す文字列。
	 * @param Owner この行が置かれるページ (遷移要求の受け取り先)。
	 * @param Target 遷移先のページ。
	 * @param Focus 遷移先で合わせたい行 (省略すると先頭のまま)。
	 */
	CDebugTopElementEntityLink( const FString& Label, const FString& SubTitle, ADebugTopEntity& Owner, ADebugTopEntity& Target, const CDebugTopElement* Focus = nullptr );

	/** 決定キーで所有ページへ遷移要求を出す。 */
	void OnDecide() override;

	/** 遷移先のページを返す。 */
	ADebugTopEntity* GetLinkedEntity() const noexcept override { return m_Target; }

private:
	/** この行が置かれているページ。所有はしない。 */
	ADebugTopEntity* m_Owner;

	/** 遷移先のページ。所有はしない (親ページが所有する)。 */
	ADebugTopEntity* m_Target;

	/**
	 * 遷移先で合わせたい行。所有はしない。
	 *
	 * @details
	 * 一覧から選んだものが、飛んだ先でも選ばれた状態になるようにする。指定が無ければ
	 * ページを開くだけ。
	 */
	const CDebugTopElement* m_Focus = nullptr;
};


/**
 * 子 Entity の行を親ページの中へその場で展開する行。
 *
 * @details
 * 画面を切り替えるほどでもない小さなまとまりを、Entity として作ったまま親ページへ埋め込む。
 * 子行は自分では持たず対象 Entity の行をそのまま見せるので、対象へ行を足せば即座に反映される。
 * この行に AddChild しても表示されない (GetChildren が対象 Entity 側を返すため)。
 * ADebugTopEntity::AddChildEntity に EDebugTopAttachMode::Inline を渡すと自動で追加される。
 */
class CDebugTopElementEntityGroup : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param SubTitle 右カラムへ出す文字列。
	 * @param Target 行を借りてくる対象の Entity。
	 */
	CDebugTopElementEntityGroup( const FString& Label, const FString& SubTitle, ADebugTopEntity& Target );

	/** 対象 Entity の行をそのまま子行として返す。 */
	const TArray<TSharedPtr<CDebugTopElement>>& GetChildren() const noexcept override;

	/** 展開元の Entity を返す。 */
	ADebugTopEntity* GetLinkedEntity() const noexcept override { return m_Target; }

private:
	/** 行を借りてくる対象の Entity。所有はしない (親ページが所有する)。 */
	ADebugTopEntity* m_Target;
};


/**
 * デバッグメニューの 1 ページ。派生して使うことを想定した基底。
 *
 * @details
 * Entity 同士の親子は「ページごと切り替わる遷移」を、Element 同士の親子は「同じページ内での
 * 展開」を表す。役割が違うので混ぜないこと。
 * カーソル移動と描画は木を直接たどらず、展開状態を反映して可視行だけを深さ優先で平坦化した
 * 可視行配列の上で行う。こうすると上下移動が添字の ±1 に、描画が 1 重ループになる。
 * 描画は本クラスと ADebugTopHUD だけが行い、CDebugTopElement は文字列とデータを返すだけにする。
 * マウス判定は直前の Draw が記録した行の矩形を見る (行の位置は画面サイズと文字サイズが
 * 決まる Draw でしか確定しないため)。したがってマウス操作は 1 フレーム遅れて効く。
 * 派生では Update / Draw を、部分的に差し替えたいだけなら OnDrawHeader / OnDrawRow /
 * BuildValueText を override する。
 */
class ADebugTopEntity : public AObject
{
public:
	/**
	 * 行 1 つを描くために確定した配置と文字列。
	 *
	 * @details
	 * 描画とマウス判定で同じ値を使うため、Draw が 1 か所で組み立てて OnDrawRow へ渡す。
	 * 座標は全て画面ピクセル。文字サイズに追従するよう、行の高さを基準に決める。
	 */
	struct FRowDraw
	{
		/** 行の左端 X (インデント前の基準)。 */
		f32 OriginX = 0.0f;

		/** 行の上端 Y。 */
		f32 Y = 0.0f;

		/** 行の幅 (カーソルの下敷きの幅)。 */
		f32 Width = 0.0f;

		/** 行の高さ。 */
		f32 Height = 0.0f;

		/**
		 * 文字 1 行分の高さ。
		 *
		 * @details
		 * 段差・右カラムの位置・マーカーの大きさは全てこちらを基準にする。背の高い行
		 * (色を選ぶ面など) で Height を基準にすると、その行だけ段差や桁位置が数倍に膨らむ。
		 */
		f32 BaseHeight = 0.0f;

		/** ピン留めの星ボタンの左端 X (インデントの外。どの階層でも同じ位置に並ぶ)。 */
		f32 StarX = 0.0f;

		/** ピン留めの星ボタンの一辺。 */
		f32 StarSize = 0.0f;

		/** 展開マーカーの左端 X (インデント済み)。 */
		f32 MarkerX = 0.0f;

		/** ラベルの左端 X。 */
		f32 LabelX = 0.0f;

		/** 右カラムの左端 X。 */
		f32 ValueX = 0.0f;

		/** 右カラムの文字の左端 X (矢印を出す行では矢印 1 個ぶん右へ寄る)。 */
		f32 ValueTextX = 0.0f;

		/** 右カラムへ描く文字列。 */
		FString ValueText;

		/** 矢印 1 個の一辺 (0 なら左右キーに反応しない行)。 */
		f32 ArrowSize = 0.0f;

		/** 左矢印の左端 X。 */
		f32 LeftArrowX = 0.0f;

		/** 右矢印の左端 X。 */
		f32 RightArrowX = 0.0f;

		/** スライダーの溝の左端 X (0 なら範囲を持たない行)。 */
		f32 SliderX = 0.0f;

		/** スライダーの溝の幅 (0 なら範囲を持たない行)。 */
		f32 SliderWidth = 0.0f;

		/**
		 * 打ち込み中の入力欄の幅 (0 なら打ち込み中ではない)。
		 *
		 * @details
		 * 値の文字をそのまま出すだけだと、いま打ち込めるのかどうかが見た目で分からない。
		 * 枠を敷いて「ここへ入る」ことを示す。
		 */
		f32 EditWidth = 0.0f;

		/** 打ち込んでいない状態の欄か (縁を控えめにする)。 */
		bool bIdleField = false;

		/**
		 * 右カラムの文字へ敷く選択の下敷きの幅 (0 なら敷かない)。
		 *
		 * @details 打ち込みを始めた直後の全選択を示す。次の入力で丸ごと置き換わることが分かる。
		 */
		f32 SelectionWidth = 0.0f;

		/** 色見本の左端 X (0 幅なら色の行ではない)。 */
		f32 SwatchX = 0.0f;

		/** 色見本の上端 Y。 */
		f32 SwatchY = 0.0f;

		/** 色見本の幅 (0 なら色の行ではない)。 */
		f32 SwatchWidth = 0.0f;

		/** 色見本の高さ。 */
		f32 SwatchHeight = 0.0f;

		/**
		 * 行全体の濃さ (0..1)。
		 *
		 * @details
		 * 続きがある側の端の行を薄くして、まだ先があることを見た目で示すために使う。
		 * OnDrawRow を override する場合も、色の不透明度へ掛けること。
		 */
		f32 Opacity = 1.0f;
	};

	/**
	 * ページを構築する。
	 *
	 * @param Name パンくずへ出すページ名。
	 */
	explicit ADebugTopEntity( const FString& Name );

	/** 子ページを解放する (TObjectPtr の解体をこの翻訳単位に閉じる)。 */
	~ADebugTopEntity() noexcept override;

	/** パンくずへ出すページ名を返す。 */
	const FString& GetName() const noexcept { return m_Name; }

	/**
	 * パンくずへ出すページ名を設定する。
	 *
	 * @param Name 設定するページ名。
	 */
	void SetName( const FString& Name ) { m_Name = Name; }

	/** 親ページを返す (ルートなら nullptr)。 */
	ADebugTopEntity* GetParent() const noexcept { return m_Parent; }

	/**
	 * 親がまだ決まっていなければ設定する。
	 *
	 * @details
	 * 遷移行で飛べるようにしただけのページは親を持たず、戻る操作で抜けられない。行を張った
	 * 時点で戻り先を決めておく。既に親を持つページ (子ページとして積んだもの) は、そちらの
	 * 経路を壊さないよう変えない。
	 * @param Parent 戻り先にするページ。
	 */
	void SetParentIfUnset( ADebugTopEntity* Parent ) noexcept
	{
		if ( m_Parent == nullptr && Parent != this ) m_Parent = Parent;
	}

	/** ページ上部へ出す見出しを返す。 */
	const FString& GetHeader() const noexcept { return m_Header; }

	/**
	 * ページ上部へ出す見出しを設定する。
	 *
	 * @details 行の一覧より上、パンくずより下に 1 行だけ描く。実行中に変えてよい。
	 * @param Header 設定する見出し (空にすると見出し行を描かず、そのぶん行が上に詰まる)。
	 */
	void SetHeader( const FString& Header ) { m_Header = Header; }

	/** 見出しの色指定を返す。 */
	const FDebugTopColor& GetHeaderColor() const noexcept { return m_HeaderColor; }

	/**
	 * 見出しの色を明示指定する。
	 *
	 * @param Color 設定する色。
	 */
	void SetHeaderColor( const FVec4& Color ) noexcept;

	/** 見出しの色指定を解除して既定色へ戻す。 */
	void ClearHeaderColor() noexcept { m_HeaderColor.bSet = false; }

	/** 画面右下へ出す説明文を返す。 */
	const FString& GetDescription() const noexcept { return m_Description; }

	/**
	 * 画面右下へ出す説明文を設定する。
	 *
	 * @details
	 * ページを表示している間ずっと右下に出る。ボタンヒント (「Enter: 決定」等) や、その
	 * ページで何ができるかの補足に使う。\\n で複数行にできる。実行中に変えてよい。
	 * 空にすると何も描かず、ADebugTopHUD 側に設定された説明文が代わりに出る。
	 * 既定のフォントは漢字を持たないため、漢字を出すなら ADebugTopHUD::SetFontSize で
	 * 専用アトラスを焼かせること。
	 * @param Description 設定する説明文。
	 */
	void SetDescription( const FString& Description ) { m_Description = Description; }

	/** 説明文の色指定を返す。 */
	const FDebugTopColor& GetDescriptionColor() const noexcept { return m_DescriptionColor; }

	/**
	 * 説明文の色を明示指定する。
	 *
	 * @param Color 設定する色。
	 */
	void SetDescriptionColor( const FVec4& Color ) noexcept;

	/** 説明文の色指定を解除して既定色へ戻す。 */
	void ClearDescriptionColor() noexcept { m_DescriptionColor.bSet = false; }

	/**
	 * このページ直下へ行を追加する。
	 *
	 * @param Element 追加する行。
	 * @return 追加した行 (呼び出し側がそのまま設定を続けられるように返す)。
	 */
	TSharedPtr<CDebugTopElement> AddElement( TSharedPtr<CDebugTopElement> Element );

	/**
	 * このページ直下へ行を生成して追加する。
	 *
	 * @details
	 * メニュー構築を短く書くための入口。戻り値が具体型なので、そのまま AddData や
	 * SetTextColor を続けて呼べる。子行を積むときは戻り値の Add<T>() を使う。
	 * @tparam TElement 生成する行の型 (CDebugTopElement 派生)。
	 * @param Arguments TElement のコンストラクタへ渡す引数。
	 * @return 追加した行 (確保に失敗したら nullptr)。所有はこのページが持つ。
	 */
	template<typename TElement, typename... TArgs>
	TElement* Add( TArgs&&... Arguments )
	{
		TSharedPtr<TElement> Element = MakeShared<TElement>( Forward<TArgs>( Arguments )... );
		TElement* const Raw = Element.Get();
		AddElement( Element );
		return Raw;
	}

	/** このページ直下の行を返す。 */
	const TArray<TSharedPtr<CDebugTopElement>>& GetElements() const noexcept { return m_Elements; }

	/**
	 * このページの中身を組み立てる。
	 *
	 * @details
	 * OnBuild を 1 回だけ呼ぶ。AddEntity / AddChildEntity で木へ組み込まれた時点で自動的に
	 * 呼ばれるので、通常は明示的に呼ぶ必要はない。二度目以降の呼び出しは無視する。
	 */
	void Build() noexcept;

	/** Build 済みかを返す。 */
	bool IsBuilt() const noexcept { return m_bBuilt; }

	/**
	 * 直下の行を取り外す。
	 *
	 * @details 取り外した時点で可視行を組み直すので、GetCursorElement が消えた行を指すことはない。
	 * @param Element 取り外す行。
	 * @return 取り外せたら true。
	 */
	bool RemoveElement( const CDebugTopElement* Element ) noexcept;

	/** 直下の行を全て取り外す。 */
	void ClearElements() noexcept;

	/**
	 * このページの値を全て構築時の状態へ戻す。
	 *
	 * @details
	 * 子行までたどって戻す。子ページ (別画面) は対象にしないので、ページごとに区切って戻せる。
	 */
	void ResetToDefaults() noexcept;

	/**
	 * 子 Entity を作って追加する。
	 *
	 * @details 派生ページを子にしたい場合は NewObject で作ってから TObjectPtr を取る方の
	 * オーバーロードへ渡す。
	 * @param Name 子 Entity 名。親ページに出る行の表示名にも使う。
	 * @param SubTitle 親ページに出る行の右カラムへ出す文字列。
	 * @param Mode Page なら決定で画面ごと切り替え、Inline ならその場で展開する。
	 * @return 生成した子 Entity (中身はこの戻り値へ積む)。確保に失敗したら nullptr。
	 */
	ADebugTopEntity* AddChildEntity( const FString& Name, const FString& SubTitle = FString(), EDebugTopAttachMode Mode = EDebugTopAttachMode::Page );

	/**
	 * 生成済みの子 Entity を追加する。
	 *
	 * @details
	 * ADebugTopEntity を派生した Entity を子にするときはこちらを使う。追加した時点で
	 * その子の Build (= OnBuild) が走る。
	 * @param Child 追加する子 Entity (所有権を受け取る)。
	 * @param SubTitle 親ページに出る行の右カラムへ出す文字列。
	 * @param Mode Page なら決定で画面ごと切り替え、Inline ならその場で展開する。
	 * @param ParentElement 行を置く先の行 (nullptr ならページ直下)。既存のカテゴリ行の下へ
	 *        ぶら下げたいときに渡す。インライン展開行を指定しても表示されないので注意。
	 * @return 追加した子 Entity (Child が空なら nullptr)。
	 */
	ADebugTopEntity* AddChildEntity( TObjectPtr<ADebugTopEntity> Child, const FString& SubTitle = FString(), EDebugTopAttachMode Mode = EDebugTopAttachMode::Page, CDebugTopElement* ParentElement = nullptr );

	/**
	 * 子 Entity を取り外し、所有権を返す。
	 *
	 * @details
	 * 対応する行 (遷移行・インライン展開行) も一緒に取り外し、可視行を組み直す。
	 * 戻り値をそのまま別のページの AddChildEntity へ渡せば付け替えになる。
	 * @param Child 取り外す子 Entity。
	 * @return 取り外した子 Entity (見つからなければ空)。
	 */
	TObjectPtr<ADebugTopEntity> RemoveChildEntity( ADebugTopEntity* Child ) noexcept;

	/** 子 Entity を返す。 */
	const TArray<TObjectPtr<ADebugTopEntity>>& GetChildEntities() const noexcept { return m_ChildEntities; }

	/**
	 * 入力を処理してカーソルと行の状態を 1 フレーム進める。
	 *
	 * @details
	 * キーボードは上下でカーソル移動、左右で値変更、Enter で決定。
	 * マウスは直前の Draw が記録した行の矩形を見て、次のように扱う。
	 * - 重ねるだけ: 押したら選ばれる行を薄く光らせる (選択は動かさない)
	 * - 左クリック: 押した位置に最も近い行を選び、開閉できる行なら開閉、矢印なら値を増減
	 * - ダブルクリック: 決定 (遷移行ならページを開き、Action 行なら実行する)
	 * - ホイール: スクロール
	 * @param DeltaSeconds 前フレームからの経過秒 (ダブルクリックの判定に使う)。
	 */
	virtual void Update( f32 DeltaSeconds ) noexcept;

	/**
	 * ページ内容を描画する。
	 *
	 * @details
	 * 画面に収まる行数はここでしか分からないため、スクロール追従もこの中で行う。
	 * 併せて各行の矩形を記録し、次の Update のマウス判定へ渡す。
	 * @param RenderContext 画面サイズの取得元。
	 * @param Text 描画に使うフォントと文字サイズ。
	 * @param Batch 描画コマンドを積む先。
	 * @param OriginX 行の左端 X (ピクセル)。
	 * @param OriginY 見出し (無ければ先頭行) の上端 Y (ピクセル)。
	 */
	virtual void Draw( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text, f32 OriginX, f32 OriginY ) noexcept;

	/**
	 * カーソルが乗っている行を返す。
	 *
	 * @details
	 * 直近の Update / Draw で組み直した可視行を見る。配列を展開して要素行にカーソルがある場合は
	 * その要素行が返るので、TryGetInt / TryGetFloat でその要素 1 つぶんの値が取れる。
	 * 配列そのものが欲しい場合は GetValueKind() が IntArray / FloatArray であることを確かめてから
	 * CDebugTopElementIntArray / CDebugTopElementFloatArray へ static_cast する。
	 * @return カーソル行 (可視行が 1 つも無ければ nullptr)。
	 */
	CDebugTopElement* GetCursorElement() const noexcept;

	/** ページ内の絞り込み語を返す (空なら絞り込んでいない)。 */
	const FString& GetFilter() const noexcept { return m_Filter; }

	/**
	 * ページ内の絞り込み語を設定する。
	 *
	 * @details
	 * 表示名に含む行だけを残す (英字の大小は区別しない)。子孫が引っかかる親も残すので、
	 * どこにあるかが分かる。絞り込み中は畳んだ行の下も降りる。
	 * 全体検索と違ってページを移らないので、いま見ているページを手早く狭めるためのもの。
	 * @param Filter 絞り込み語 (空で解除)。
	 */
	void SetFilter( const FString& Filter ) { m_Filter = Filter; }

	/** カーソルが乗っている可視行の添字を返す。 */
	i32 GetCursorRow() const noexcept { return m_Scroller.GetCursorRow(); }

	/**
	 * いま並べている行数を返す。
	 *
	 * @details 絞り込みで何件に減ったかを外から出すのに使う (可視行そのものは見せない)。
	 * @return 可視行の数。
	 */
	usize GetVisibleRowCount() const noexcept { return m_VisibleRows.Num(); }

	/**
	 * 値を打ち込んでいる最中かを返す。
	 *
	 * @details
	 * 打ち込み中は文字が全てバッファへ入るので、呼び出し側は「戻る」等の割り当てを止める。
	 * @return 打ち込み中なら true。
	 */
	bool IsTyping() const noexcept { return m_ValueEditor.IsActive(); }

	/**
	 * マウスを重ねている行を返す。
	 *
	 * @details 押したときに選ばれる行。描画側が「ここが選ばれる」を示すために使う。
	 * @return 重ねている行 (重ねていなければ nullptr)。
	 */
	CDebugTopElement* GetHoverElement() const noexcept;

	/**
	 * 指定の行へカーソルを合わせる。
	 *
	 * @details
	 * 畳まれていて見えない行は、途中の親を開いてから合わせる。検索から飛んできたときに
	 * 「ページは開いたが目的の行が見つからない」とならないようにする。
	 * @param Element 合わせたい行。
	 * @return 見つけて合わせられたら true。
	 */
	bool FocusElement( const CDebugTopElement& Element ) noexcept;

	/**
	 * カーソル位置を設定する (可視行の範囲外は無視する)。
	 *
	 * @param CursorRow 設定する可視行の添字。
	 */
	void SetCursorRow( i32 CursorRow ) noexcept;

	/**
	 * 遷移要求を出す (CDebugTopElementEntityLink から呼ばれる)。
	 *
	 * @param Target 遷移先のページ。
	 * @param Focus 遷移先で合わせたい行 (不要なら nullptr)。
	 */
	void RequestTransition( ADebugTopEntity* Target, const CDebugTopElement* Focus = nullptr ) noexcept
	{
		m_PendingTransition = Target;
		m_PendingFocus = Focus;
	}

	/**
	 * 溜まっている遷移要求を取り出してクリアする。
	 *
	 * @return 遷移先 (要求が無ければ nullptr)。
	 */
	ADebugTopEntity* ConsumePendingTransition() noexcept;

	/**
	 * 溜まっている遷移先での合わせ先を取り出してクリアする。
	 *
	 * @details ConsumePendingTransition より後に呼ぶこと。
	 * @return 合わせたい行 (指定が無ければ nullptr)。
	 */
	const CDebugTopElement* ConsumePendingFocus() noexcept;

protected:
	/**
	 * このページの行と子ページを積む。派生ページはここへメニューを書く。
	 *
	 * @details
	 * コンストラクタからでは仮想呼び出しが効かないため、木へ組み込まれた直後に呼ばれる。
	 * Add<T>() / AddElement() / AddChildEntity() / SetHeader() はここから使う。
	 * 子ページを AddChildEntity で足すと、その子の OnBuild もその場で走る。
	 */
	virtual void OnBuild() noexcept {}

	/**
	 * 見出しを描き、消費した高さを返す。
	 *
	 * @param RenderContext 画面サイズの取得元。
	 * @param Batch 描画コマンドを積む先。
	 * @param Text 描画に使うフォントと文字サイズ。
	 * @param OriginX 見出しの左端 X (ピクセル)。
	 * @param OriginY 見出しの上端 Y (ピクセル)。
	 * @return 見出しが消費した高さ (ピクセル)。見出しが空なら 0。
	 */
	virtual f32 OnDrawHeader( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text, f32 OriginX, f32 OriginY ) noexcept;

	/**
	 * 可視行 1 つを描く。
	 *
	 * @details
	 * 配置は Draw が決めて RowDraw で渡す (マウス判定が同じ矩形を見るため)。override する
	 * 場合も RowDraw の座標に従って描くこと。従わないと見た目とクリック位置がずれる。
	 * @param RenderContext 画面サイズの取得元。
	 * @param Batch 描画コマンドを積む先。
	 * @param Text 描画に使うフォントと文字サイズ。
	 * @param Row 描く可視行。
	 * @param RowDraw この行の配置と右カラムの文字列。
	 * @param bSelected カーソルが乗っているか。
	 */
	virtual void OnDrawRow( FRenderContext& RenderContext, CSpriteBatch& Batch, const CDebugTopText& Text, const FDebugTopVisibleRow& Row, const FRowDraw& RowDraw, bool bSelected ) noexcept;

	/**
	 * 右カラムへ出す文字列を組み立てる。
	 *
	 * @details
	 * 左右キーで値が変わる行は「値 n/n」の形にして、選択肢のどこにいるかが分かるようにする。
	 * 値を挟む矢印は文字ではなく図形で描くため (既定のフォントが ← → を持たないため)、
	 * ここには含めない。装飾を変えたい場合はここを override する。
	 * @param Element 対象の行。
	 * @return 右カラムへ描く文字列。
	 */
	virtual FString BuildValueText( const CDebugTopElement& Element ) const;

	/** 展開状態を反映して可視行を組み直す。 */
	void RebuildVisibleRows() noexcept;

	/** 可視行を返す (直近の RebuildVisibleRows 時点の内容)。 */
	const FDebugTopVisibleRows& GetVisibleRows() const noexcept { return m_VisibleRows; }

	/** 画面最上段へ出す可視行の添字を返す。 */
	i32 GetScrollTop() const noexcept { return m_Scroller.GetScrollTop(); }

	/**
	 * カーソルを動かす (可視行の範囲で上下端を回り込む)。
	 *
	 * @param Delta 上で -1、下で +1。
	 */
	void MoveCursor( i32 Delta ) noexcept { m_Scroller.MoveCursor( m_VisibleRows, Delta ); }

private:

	/**
	 * 1 行とその可視な子孫を可視行配列へ積む。
	 *
	 * @param Element 積む行。
	 * @param Depth この行の深さ。
	 */
	void FlattenElement( const TSharedPtr<CDebugTopElement>& Element, i32 Depth ) noexcept;

	/**
	 * 行か、その子孫が絞り込み語に引っかかるかを返す。
	 *
	 * @param Element 調べる行。
	 * @return 引っかかれば true。
	 */
	bool MatchesFilterDeep( const CDebugTopElement& Element ) const noexcept;

	/**
	 * 行 1 つの配置を決める。
	 *
	 * @param Text 描画に使うフォントと文字サイズ。
	 * @param Row 対象の可視行。
	 * @param OriginX 行の左端 X (ピクセル)。
	 * @param Y 行の上端 Y (ピクセル)。
	 * @param Width 行の幅 (ピクセル)。
	 * @param Height 行の高さ (ピクセル。背の高い行ではここだけが伸びる)。
	 * @param BaseHeight 文字 1 行分の高さ (ピクセル。段差と桁位置の基準)。
	 * @return 確定した配置と右カラムの文字列。
	 */
	FRowDraw BuildRowDraw( const CDebugTopText& Text, const FDebugTopVisibleRow& Row, f32 OriginX, f32 Y, f32 Width, f32 Height, f32 BaseHeight ) const;

	/**
	 * 打ち込める行へ、打っていない状態の入力欄を割り当てる。
	 *
	 * @details 矢印とスライダーは欄の右へ寄せ直す (欄と重ならないように)。
	 * @param RowDraw 書き換える配置。
	 * @param Row 対象の可視行。
	 */
	void ApplyIdleFieldLayout( FRowDraw& RowDraw, const FDebugTopVisibleRow& Row ) const;

	/**
	 * ゲームパッド操作 (カーソル移動・値変更・決定) を処理する。
	 *
	 * @param DeltaSeconds 前フレームからの経過秒 (倒しっぱなしのリピートに使う)。
	 */
	void UpdateGamepad( f32 DeltaSeconds ) noexcept;

	/** パンくずへ出すページ名。 */
	FString m_Name;

	/** ページ上部へ出す見出し (空なら描かない)。 */
	FString m_Header;

	/** 画面右下へ出す説明文 (空なら HUD 側の説明文が出る)。 */
	FString m_Description;

	/** 見出しの色指定。 */
	FDebugTopColor m_HeaderColor;

	/** 説明文の色指定。 */
	FDebugTopColor m_DescriptionColor;

	/** 親ページ (ルートなら nullptr)。所有はしない。 */
	ADebugTopEntity* m_Parent = nullptr;

	/** 子ページ。親がライフタイムを持つ。 */
	TArray<TObjectPtr<ADebugTopEntity>> m_ChildEntities;

	/** このページ直下の行。 */
	TArray<TSharedPtr<CDebugTopElement>> m_Elements;

	/** ページ内の絞り込み語 (空なら絞り込んでいない)。 */
	FString m_Filter;

	/** 展開状態を反映した可視行。RebuildVisibleRows が作り直す。 */
	FDebugTopVisibleRows m_VisibleRows;

	/** どの行を選び、どこから何行出すか。行の高さが一定でないので数え方もこちらが持つ。 */
	CDebugTopRowScroller m_Scroller;

	/** マウスの押下・ドラッグ・ホイールの振り分け。時間を跨ぐ状態はこちらが持つ。 */
	CDebugTopRowMouse m_Mouse;

	/** 矢印キーの方向入力 (押しっぱなしのリピートを内側で均す)。 */
	CDebugTopKeyNav m_KeyNav;

	/** ゲームパッドの方向入力 (倒しっぱなしのリピートを内側で均す)。 */
	CDebugTopGamepadNav m_Gamepad;

	/** 値の打ち込み。どの行を打っているか・確定した文字の行き先はこちらが持つ。 */
	CDebugTopValueEditor m_ValueEditor;

	/** 色見本を押すと横へ浮くパネル。出す位置以外はこちらが面倒を見る。 */
	CDebugTopColorPicker m_ColorPicker;

	/** 決定された遷移先。ADebugTopHUD が取り出してクリアする。 */
	ADebugTopEntity* m_PendingTransition = nullptr;

	/** 遷移先で合わせたい行 (指定が無ければ nullptr)。所有はしない。 */
	const CDebugTopElement* m_PendingFocus = nullptr;

	/** OnBuild を呼び終えたか (二重構築の防止)。 */
	bool m_bBuilt = false;
};
