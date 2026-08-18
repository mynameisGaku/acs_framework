// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElementTypes.h"

using namespace acs;

/**
 * デバッグメニュー 1 行分の基底クラス。
 *
 * @details
 * ラベル・サブタイトル・子行・展開状態・色は全型で共通なのでここへ集約し、型ごとの差分は
 * GetValueText / OnDecide / OnLeftRight の 3 つだけに閉じる。行は自分の表示位置を知らないため
 * 自分では描画せず、ADebugTopEntity が可視行を平坦化してから一括で描く。
 * 本クラスをそのまま使えば「子行を束ねるカテゴリ行」になる。
 * 設定は全て実行中に変更してよい (可視行は毎フレーム組み直される)。
 */
class CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param SubTitle 右カラムへ出す文字列 (値を持つ派生では現在値の表示に置き換わる)。
	 */
	CDebugTopElement( const FString& Label, const FString& SubTitle = FString() );

	/** 派生を正しく破棄するための仮想デストラクタ。 */
	virtual ~CDebugTopElement() = default;

	/**
	 * 右カラムへ出す文字列を返す。
	 *
	 * @return 既定ではサブタイトル。値を持つ派生は現在値の文字列を返す。
	 */
	virtual FString GetValueText() const;

	/** 決定キーで呼ばれる。既定は展開が許可されていて子行を持つときのトグル。 */
	virtual void OnDecide();

	/**
	 * 左右キーで呼ばれる。既定は何もしない。
	 *
	 * @param Delta 左キーで -1、右キーで +1。
	 */
	virtual void OnLeftRight( i32 Delta );

	/**
	 * 左右キーで値が変わる行かを返す。
	 *
	 * @details 描画側が「← 値 →」の矢印を出すかの判定に使う。行は自分では描かない。
	 * @return 左右キーに反応するなら true。既定は false。
	 */
	virtual bool IsLeftRightAdjustable() const noexcept { return false; }

	/**
	 * 選択位置と候補数を返す (描画側の「n/n」表示用)。
	 *
	 * @param OutIndex 選択位置 (0 起点。表示時に +1 する)。
	 * @param OutCount 候補の総数。
	 * @return 候補列を持つ行なら true。既定は false。
	 */
	virtual bool TryGetSelection( i32& OutIndex, i32& OutCount ) const noexcept
	{
		(void)OutIndex;
		(void)OutCount;
		return false;
	}

	/**
	 * この行が持つ値の種類を返す。
	 *
	 * @return 既定は None。値を持つ派生がそれぞれの種別を返す。
	 */
	virtual EDebugTopValueKind GetValueKind() const noexcept { return EDebugTopValueKind::None; }

	/**
	 * i32 として値を取り出す。
	 *
	 * @param OutValue 取り出せた場合に値を書き込む先。
	 * @return 取り出せたら true (Int と Enum が対応。Enum は選択位置を返す)。
	 */
	virtual bool TryGetInt( i32& OutValue ) const noexcept { (void)OutValue; return false; }

	/**
	 * f32 として値を取り出す。
	 *
	 * @param OutValue 取り出せた場合に値を書き込む先。
	 * @return 取り出せたら true。
	 */
	virtual bool TryGetFloat( f32& OutValue ) const noexcept { (void)OutValue; return false; }

	/**
	 * bool として値を取り出す。
	 *
	 * @param bOutValue 取り出せた場合に値を書き込む先。
	 * @return 取り出せたら true。
	 */
	virtual bool TryGetBool( bool& bOutValue ) const noexcept { (void)bOutValue; return false; }

	/**
	 * 保存の対象にする行かを返す。
	 *
	 * @details
	 * 値を持っていても保存したくない行 (コンボボックスの選択肢のように、親の状態を映して
	 * いるだけの行) は false を返す。
	 * @return 保存対象なら true。既定は true。
	 */
	virtual bool IsSaveable() const noexcept { return true; }

	/**
	 * 既定値から変えられているかを返す。
	 *
	 * @details
	 * 「どこを触ったか」を見た目で示すために使う。値を持たない行は常に false。
	 * @return 構築時の値と違っていれば true。
	 */
	virtual bool IsModified() const noexcept { return false; }

	/**
	 * 値を構築時の状態へ戻す。
	 *
	 * @details 値を持たない行は何もしない。子行がある場合は呼び出し側がたどること。
	 */
	virtual void ResetToDefault() {}

	/**
	 * 値の範囲を返す (スライダー表示に使う)。
	 *
	 * @param OutRatio 下限を 0、上限を 1 としたときの現在位置。
	 * @return 範囲を持つ行なら true。既定は false。
	 */
	virtual bool TryGetRatio( f32& OutRatio ) const noexcept { (void)OutRatio; return false; }

	/**
	 * 下限から上限までの位置を指定して値を書き込む (スライダーのドラッグ用)。
	 *
	 * @param Ratio 下限を 0、上限を 1 とした位置 (範囲外は端へ丸める)。
	 * @return 書き込めたら true。範囲を持たない行は false。
	 */
	virtual bool TrySetRatio( f32 Ratio ) { (void)Ratio; return false; }

	/**
	 * 右カラムへ色見本を出す行かを返す。
	 *
	 * @param OutColor 見本に使う色の書き込み先。
	 * @return 色として扱う行なら true。既定は false。
	 */
	virtual bool TryGetColorSwatch( FVec4& OutColor ) const noexcept { (void)OutColor; return false; }

	/**
	 * 行 1 つ分の高さに対する倍率を返す。
	 *
	 * @details
	 * 色を選ぶ面のように、文字 1 行では収まらないものを描く行が背を高くするために使う。
	 * @return 高さの倍率 (既定は 1)。
	 */
	virtual f32 GetHeightRatio() const noexcept { return 1.0f; }

	/**
	 * 色を選ぶ面を持つ行かを返す。
	 *
	 * @details 描画側は受け取った 3 つの値から、面と帯のどこを選んでいるかの印も描く。
	 * @param OutHue いま選んでいる色相 (0..1) の書き込み先。
	 * @param OutSaturation いま選んでいる彩度 (0..1) の書き込み先。
	 * @param OutValue いま選んでいる明度 (0..1) の書き込み先。
	 * @return 面を持つ行なら true。既定は false。
	 */
	virtual bool TryGetColorField( f32& OutHue, f32& OutSaturation, f32& OutValue ) const noexcept
	{
		(void)OutHue; (void)OutSaturation; (void)OutValue;
		return false;
	}

	/**
	 * 行の中で押された位置を受け取る。
	 *
	 * @details
	 * 面や帯を持つ行が、押された場所から値を決めるために使う。行の矩形に入った押下と
	 * ドラッグ中の移動が渡ってくる。
	 * @param LocalX 行の左端からの距離 (ピクセル)。
	 * @param LocalY 行の上端からの距離 (ピクセル)。
	 * @param Width 行の幅 (ピクセル)。
	 * @param Height 行の高さ (ピクセル)。
	 * @return 押下を受け取った (ドラッグを続ける) なら true。既定は false。
	 */
	virtual bool OnPointer( f32 LocalX, f32 LocalY, f32 Width, f32 Height )
	{
		(void)LocalX; (void)LocalY; (void)Width; (void)Height;
		return false;
	}

	/** ピン留めされているかを返す。 */
	bool IsFavorite() const noexcept { return m_bFavorite; }

	/**
	 * ピン留めを設定する。
	 *
	 * @details
	 * 留めた行はお気に入りのページへ集まる。項目が数百になっても、よく使うものだけを
	 * 1 ページから触れる。留め外しのたびに版を進めて、集めた側が組み直せるようにする。
	 * @param bFavorite 留めるなら true。
	 */
	void SetFavorite( bool bFavorite ) noexcept;

	/** ピン留めの版を返す (どこかで留め外しがあると増える)。 */
	static u32 GetFavoriteVersion() noexcept { return s_FavoriteVersion; }

	/**
	 * 値の版を返す (どこかで値が変わると増える)。
	 *
	 * @details
	 * メニュー全体を毎フレーム走査せずに「何か変わったか」を知るためのもの。保管庫への
	 * 吸い上げ (CDebugTopSettings::CaptureFrom) を、変わったフレームだけに絞るのに使う。
	 * @return 値の版。
	 */
	static u32 GetValueVersion() noexcept { return s_ValueVersion; }

	/**
	 * 値が変わった行を、変わった直後に受け取る係を差す。
	 *
	 * @details
	 * 版 (GetValueVersion) では「どこかで変わった」しか分からない。どの行が変わったかを
	 * 知りたい側 (取り消しの控え) のための口。行の側は誰が聞いているかを知らない。
	 * 差せるのは 1 つだけ。上書きすると前の係は外れる。
	 * @param Listener 変わった行を受け取る係 (外すなら空のものを差す)。
	 */
	static void SetChangeListener( TDelegate<void( CDebugTopElement& )> Listener ) noexcept { s_ChangeListener = Listener; }

	/**
	 * 色を編集する行なら自分自身を返す。
	 *
	 * @details RTTI を使わずに色の行だけを見分けるための口。
	 * @return 色の行なら自分自身、そうでなければ nullptr。
	 */
	virtual class CDebugTopElementColor* AsColor() noexcept { return nullptr; }

	/**
	 * 開閉できる行かを返す。
	 *
	 * @details
	 * 既定では子行を持つ行だけが開閉できる。子を持たなくても中身を出し入れしたい行
	 * (折れ線を畳めるようにする等) は、これを override する。
	 * @return 開閉できるなら true。
	 */
	virtual bool CanCollapse() const noexcept { return HasChildren(); }

	/**
	 * 1 フレーム進める。
	 *
	 * @details
	 * 画面に出ている行だけが呼ばれる。値を追い続ける行 (グラフ) が標本を溜めるために使う。
	 * @param DeltaSeconds 前フレームからの経過秒。
	 */
	virtual void OnTick( f32 DeltaSeconds ) { (void)DeltaSeconds; }

	/**
	 * 折れ線を描く行かを返す。
	 *
	 * @param OutSamples 標本の先頭 (古い順に並ぶ) の書き込み先。
	 * @param OutCount 標本の数の書き込み先。
	 * @param OutMin 縦軸の下端の書き込み先。
	 * @param OutMax 縦軸の上端の書き込み先。
	 * @return 折れ線を描く行なら true。既定は false。
	 */
	virtual bool TryGetGraph( const f32*& OutSamples, usize& OutCount, f32& OutMin, f32& OutMax ) const noexcept
	{
		(void)OutSamples; (void)OutCount; (void)OutMin; (void)OutMax;
		return false;
	}

	/**
	 * 値を直接打ち込める行かを返す。
	 *
	 * @details
	 * true を返す行は、決定したときに開閉や実行ではなく打ち込みが始まる。左右キーで送るには
	 * 遠い値 (大きな数、任意の文字列) をひと息で入れるために使う。
	 * @return 打ち込める行なら true。既定は false。
	 */
	virtual bool CanTypeValue() const noexcept { return false; }

	/**
	 * 決定キーで、打ち込みではなく OnDecide を行いたいかを返す。
	 *
	 * @details
	 * 打ち込める行は、決定すると欄へ入るのが既定。ただし打ち込みより先に決まった操作を
	 * 持つ行 (パスの行は一覧を開く) もあるので、そちらを選べるようにしてある。
	 * この場合でも欄をクリックすれば打ち込める。
	 * @return 決定で OnDecide を行うなら true。既定は false。
	 */
	virtual bool PrefersDecide() const noexcept { return false; }

	/**
	 * 打ち込みを始めるときの初期文字列を返す。
	 *
	 * @details 現在値をそのまま入れておくと、打ち直しでも消してから入れ直さずに済む。
	 * @return 初期文字列。
	 */
	virtual FString GetEditText() const { return FString(); }

	/**
	 * 打ち終えた文字列を値へ反映する。
	 *
	 * @param Text 打ち終えた文字列。
	 * @return 値として解釈できたら true。できなければ false (呼び出し側は元の値を保つ)。
	 */
	virtual bool CommitEditText( const FString& Text ) { (void)Text; return false; }

	/**
	 * i32 として値を書き込む。
	 *
	 * @details
	 * 保存した設定の復元や、ゲーム側から現在値を押し込むのに使う。上下限や候補の
	 * 制約は各行の SetValue と同じように働き、変化したときだけ通知される。
	 * @param Value 設定する値。
	 * @return 書き込めたら true (Int と Enum が対応。Enum は選択位置として扱う)。
	 */
	virtual bool TrySetInt( i32 Value ) { (void)Value; return false; }

	/**
	 * f32 として値を書き込む。
	 *
	 * @param Value 設定する値。
	 * @return 書き込めたら true。
	 */
	virtual bool TrySetFloat( f32 Value ) { (void)Value; return false; }

	/**
	 * bool として値を書き込む。
	 *
	 * @param bValue 設定する値。
	 * @return 書き込めたら true。
	 */
	virtual bool TrySetBool( bool bValue ) { (void)bValue; return false; }

	/**
	 * 子行を末尾へ追加する。
	 *
	 * @param Child 追加する子行。
	 * @return 追加した子行 (呼び出し側がそのまま設定を続けられるように返す)。
	 */
	TSharedPtr<CDebugTopElement> AddChild( TSharedPtr<CDebugTopElement> Child );

	/**
	 * 子行を生成して追加する。
	 *
	 * @details
	 * メニュー構築を短く書くための入口。戻り値が具体型なので、そのまま AddData や
	 * SetTextColor を続けて呼べる。
	 * @tparam TElement 生成する行の型 (CDebugTopElement 派生)。
	 * @param Arguments TElement のコンストラクタへ渡す引数。
	 * @return 追加した行 (確保に失敗したら nullptr)。所有はこの行が持つ。
	 */
	template<typename TElement, typename... TArgs>
	TElement* Add( TArgs&&... Arguments )
	{
		TSharedPtr<TElement> Element = MakeShared<TElement>( Forward<TArgs>( Arguments )... );
		TElement* const Raw = Element.Get();
		AddChild( Element );
		return Raw;
	}

	/**
	 * 左カラムへ出す表示名を返す。
	 *
	 * @details 保存キーの既定値にも使うため、プロバイダを設定していてもこちらは変わらない。
	 * @return SetLabel で設定した表示名。
	 */
	const FString& GetLabel() const noexcept { return m_Label; }

	/**
	 * 左カラムへ出す表示名を設定する。
	 *
	 * @details 実行中に呼んでよい。次の描画から反映される。
	 * @param Label 設定する表示名。
	 */
	void SetLabel( const FString& Label ) { m_Label = Label; }

	/**
	 * 実際に描く表示名を返す。
	 *
	 * @details 描画側はこちらを使う。プロバイダが設定されていればその戻り値を優先する。
	 * @return プロバイダの戻り値。未設定なら SetLabel で設定した表示名。
	 */
	FString GetDisplayLabel() const;

	/**
	 * 表示名をその場で作るデリゲートを設定する。
	 *
	 * @details
	 * 毎フレーム呼ばれるので、監視中の値をそのまま表示名へ出せる (「HP 120/200」等)。
	 * SetLabel で都度書き換える必要が無くなる。解除は ClearLabelProvider。
	 * @param Provider 表示名を返すデリゲート。
	 */
	void SetLabelProvider( FDebugTopTextDelegate Provider ) noexcept { m_LabelProvider = Provider; }

	/** 表示名のデリゲートを解除し、SetLabel で設定した表示名へ戻す。 */
	void ClearLabelProvider() noexcept { m_LabelProvider.Unbind(); }

	/** 画面右下へ出す説明文を返す。 */
	const FString& GetDescription() const noexcept { return m_Description; }

	/**
	 * 画面右下へ出す説明文を設定する。
	 *
	 * @details
	 * カーソルがこの行に乗っている間だけ出る。行ごとの補足 (この値が何に効くか、単位、
	 * 注意点など) を書く。空ならページ側、それも空なら HUD 側の説明文が出る。
	 * \\n で複数行にできる。
	 * @param Description 設定する説明文。
	 */
	void SetDescription( const FString& Description ) { m_Description = Description; }

	/** 保存・読み込みで使うキーを返す (空ならメニュー内の位置から自動生成される)。 */
	const FString& GetSaveKey() const noexcept { return m_SaveKey; }

	/**
	 * 保存・読み込みで使うキーを明示指定する。
	 *
	 * @details
	 * 指定するとメニュー内の位置と表示名に依存しない絶対キーになる。ゲーム側は
	 * CDebugTopSettings からこのキーで値を受け取れるので、行を別ページへ移したり
	 * 表示名を変えたりしても受け取り側のコードを直さずに済む。
	 * 未指定なら「ページ名/親行/表示名」から自動生成する。
	 * @param SaveKey 設定するキー (空にすると自動生成へ戻る)。
	 */
	void SetSaveKey( const FString& SaveKey ) { m_SaveKey = SaveKey; }

	/** 右カラムへ出すサブタイトルを返す。 */
	const FString& GetSubTitle() const noexcept { return m_SubTitle; }

	/**
	 * 右カラムへ出すサブタイトルを設定する。
	 *
	 * @details 値を持つ派生 (Int / Float など) は右カラムに現在値を出すため、設定しても表示されない。
	 * @param SubTitle 設定するサブタイトル。
	 */
	void SetSubTitle( const FString& SubTitle ) { m_SubTitle = SubTitle; }

	/**
	 * 値がこの範囲の外なら注意色で描く、という範囲を設定する。
	 *
	 * @details
	 * フレーム時間の予算超過や、体力が負になった等を目で拾えるようにする。値を数値として
	 * 取れる行 (TryGetFloat / TryGetInt に答える行) にだけ効く。
	 * @param Min 下限 (これを下回ると注意)。
	 * @param Max 上限 (これを上回ると注意)。
	 */
	void SetWarnRange( f32 Min, f32 Max ) noexcept
	{
		m_WarnMin = Min < Max ? Min : Max;
		m_WarnMax = Min < Max ? Max : Min;
		m_bHasWarnRange = true;
	}

	/** 注意色の範囲を解除する。 */
	void ClearWarnRange() noexcept { m_bHasWarnRange = false; }

	/**
	 * いまの値が注意すべき範囲に入っているかを返す。
	 *
	 * @details 範囲を設定していない行と、値を数値として取れない行は常に false。
	 * @return 注意色で描くべきなら true。
	 */
	bool IsValueWarned() const noexcept
	{
		if ( !m_bHasWarnRange ) return false;

		f32 Value = 0.0f;
		if ( !TryGetFloat( Value ) )
		{
			i32 IntValue = 0;
			if ( !TryGetInt( IntValue ) ) return false;

			Value = static_cast<f32>( IntValue );
		}
		return Value < m_WarnMin || Value > m_WarnMax;
	}

	/** 割り当てたショートカットキーを返す (EKey::Unknown なら未割り当て)。 */
	EKey GetShortcut() const noexcept { return m_Shortcut; }

	/**
	 * この行を直接叩くキーを割り当てる。
	 *
	 * @details
	 * どのページを開いていても効く。行まで辿らずに実行したい操作 (再読み込み・表示の切替など)
	 * のためのもの。押されると OnDecide が呼ばれる (決定キーを押したのと同じ)。
	 * メニューの操作に使うキー (上下左右・Enter・Esc・F) は避けること。
	 * @param Key 割り当てるキー (EKey::Unknown で解除)。
	 */
	void SetShortcut( EKey Key ) noexcept { m_Shortcut = Key; }

	/** 値へ添える単位を返す (空なら添えない)。 */
	const FString& GetUnit() const noexcept { return m_Unit; }

	/**
	 * 値へ添える単位を設定する。
	 *
	 * @details
	 * 右カラムの表示にだけ足す。打ち込みの中身 (GetEditText / CommitEditText) には入らないので、
	 * 単位を付けても数値をそのまま打ち込める。先頭に空白は要らない (描画側が空けて出す)。
	 * @param Unit 単位 ("m" / "m/s" / "ms" / "%" など)。
	 */
	void SetUnit( const FString& Unit ) { m_Unit = Unit; }

	/**
	 * 子行を取り外す。
	 *
	 * @details 表示は次の Update / Draw で組み直されたときに反映される。
	 * @param Child 取り外す子行。
	 * @return 取り外せたら true。
	 */
	bool RemoveChild( const CDebugTopElement* Child );

	/** 子行を全て取り外す。 */
	void ClearChildren() noexcept { m_Children.Reset(); }

	/**
	 * 子行の配列を返す。
	 *
	 * @details
	 * 子行の実体を別の場所 (Entity の行など) から借りる派生は、ここを override して
	 * 借り先の配列を返す。表示・展開・入力は全てこの戻り値を通る。
	 * @return 子行の配列。
	 */
	virtual const TArray<TSharedPtr<CDebugTopElement>>& GetChildren() const noexcept { return m_Children; }

	/** 子行を 1 つでも持つかを返す。 */
	bool HasChildren() const noexcept { return !GetChildren().IsEmpty(); }

	/**
	 * この行が指している Entity を返す。
	 *
	 * @return 遷移行やインライン展開行なら対象の Entity、それ以外は nullptr。
	 */
	virtual ADebugTopEntity* GetLinkedEntity() const noexcept { return nullptr; }

	/** 展開マーカーを出すべきかを返す (描画側が使う)。 */
	bool ShouldShowMarker() const noexcept;

	/** 展開マーカーの表示方針を返す。 */
	EDebugTopMarkerVisibility GetMarkerVisibility() const noexcept { return m_MarkerVisibility; }

	/**
	 * 展開マーカーの表示方針を設定する。
	 *
	 * @details
	 * 子行を持たない行にも見出しとしてマーカーを出したい場合に Always を使う。この場合は
	 * 開閉できないため、三角形ではなく薄い横棒で描かれる。
	 * @param Visibility 設定する表示方針。
	 */
	void SetMarkerVisibility( EDebugTopMarkerVisibility Visibility ) noexcept { m_MarkerVisibility = Visibility; }

	/** 子行を表示中かを返す。 */
	bool IsExpanded() const noexcept { return m_bExpanded; }

	/**
	 * 子行の表示・非表示を設定する。
	 *
	 * @details 開いた状態で始めたい行はメニュー構築時にここへ true を渡す。
	 * SetExpandable(false) と併用すると「常に開いたまま畳めない行」になる。
	 * 開閉可否にかかわらず反映するので、プログラムからの強制開閉にも使える。
	 * @param bExpanded true で子行を表示する。
	 */
	void SetExpanded( bool bExpanded ) noexcept { m_bExpanded = bExpanded; }

	/** 決定キーでの開閉が許可されているかを返す。 */
	bool IsExpandable() const noexcept { return m_bExpandable; }

	/**
	 * 決定キーでの開閉可否を設定する。
	 *
	 * @details
	 * false にした行は展開マーカーが三角形ではなく薄い横棒になり、押しても開かないことが
	 * 見た目で分かるようになる。
	 * @param bExpandable false にすると決定キーで開閉できなくなる (SetExpanded は効く)。
	 */
	void SetExpandable( bool bExpandable ) noexcept { m_bExpandable = bExpandable; }

	/** ラベルの色指定を返す。 */
	const FDebugTopColor& GetTextColor() const noexcept { return m_TextColor; }

	/**
	 * ラベルの色を明示指定する。
	 *
	 * @param Color 設定する色。
	 */
	void SetTextColor( const FVec4& Color ) noexcept;

	/** ラベルの色指定を解除し、選択状態に応じた既定色へ戻す。 */
	void ClearTextColor() noexcept { m_TextColor.bSet = false; }

	/** 右カラムの色指定を返す。 */
	const FDebugTopColor& GetValueColor() const noexcept { return m_ValueColor; }

	/**
	 * 右カラムの色を明示指定する。
	 *
	 * @param Color 設定する色。
	 */
	void SetValueColor( const FVec4& Color ) noexcept;

	/** 右カラムの色指定を解除し、ラベルと同じ扱いへ戻す。 */
	void ClearValueColor() noexcept { m_ValueColor.bSet = false; }

	/**
	 * 値が変化したときに呼ぶデリゲートを設定する。
	 *
	 * @param OnChanged 値変更時に呼ぶデリゲート。
	 */
	void SetOnChanged( FSimpleDelegate OnChanged ) noexcept { m_OnChanged = OnChanged; }

protected:
	/**
	 * 値変更を通知する (値を持つ派生が値を書き換えた直後に呼ぶ)。
	 *
	 * @details
	 * 版も併せて進める。メニュー全体を毎フレーム走査しなくても、外から「何か変わった」を
	 * 知れるようにするため。
	 */
	void NotifyChanged() const noexcept
	{
		++s_ValueVersion;
		m_OnChanged.ExecuteIfBound();

		// 変わった行そのものを聞きたい側へ渡す。NotifyChanged は const だが、受け取る側は
		// 行を書き戻す (取り消し) ことがあるので const を外して渡す。
		s_ChangeListener.ExecuteIfBound( const_cast<CDebugTopElement&>( *this ) );
	}

private:
	/** 左カラムへ出す表示名。 */
	FString m_Label;

	/** 右カラムへ出すサブタイトル (GetValueText の既定の戻り値)。 */
	FString m_SubTitle;

	/** 値へ添える単位 (空なら添えない)。表示にだけ足し、打ち込みには混ぜない。 */
	FString m_Unit;

	/** 注意色にならない範囲の下限。 */
	f32 m_WarnMin = 0.0f;

	/** 注意色にならない範囲の上限。 */
	f32 m_WarnMax = 0.0f;

	/** 注意色の範囲を設定してあるか。 */
	bool m_bHasWarnRange = false;

	/** この行を直接叩くキー (EKey::Unknown なら未割り当て)。 */
	EKey m_Shortcut = EKey::Unknown;

	/** 画面右下へ出す説明文 (空ならページ側の説明文が出る)。 */
	FString m_Description;

	/** 保存・読み込みで使う絶対キー (空ならメニュー内の位置から自動生成)。 */
	FString m_SaveKey;

	/** 表示名をその場で作るデリゲート (未設定なら m_Label を使う)。 */
	FDebugTopTextDelegate m_LabelProvider;

	/** 同じページ内で展開したときに現れる子行。 */
	TArray<TSharedPtr<CDebugTopElement>> m_Children;

	/** 値変更時に呼ぶデリゲート。 */
	FSimpleDelegate m_OnChanged;

	/** ラベルの色指定。 */
	FDebugTopColor m_TextColor;

	/** 右カラムの色指定。 */
	FDebugTopColor m_ValueColor;

	/** 展開マーカーの表示方針。 */
	EDebugTopMarkerVisibility m_MarkerVisibility = EDebugTopMarkerVisibility::Auto;

	/** 子行を表示中か。 */
	bool m_bExpanded = false;

	/** 決定キーでの開閉が許可されているか。 */
	bool m_bExpandable = true;

	/** ピン留めされているか。 */
	bool m_bFavorite = false;

	/** ピン留めの版 (どこかで留め外しがあると増える。集めた側が組み直しの合図に使う)。 */
	static u32 s_FavoriteVersion;

	/** 値の版 (どこかで値が変わると増える)。NotifyChanged は const なので mutable にする。 */
	static u32 s_ValueVersion;

	/** 値が変わった行を受け取る係 (差されていなければ何も起きない)。 */
	static TDelegate<void( CDebugTopElement& )> s_ChangeListener;
};

/** i32 1 個を右カラム用に文字列化する。 */
FString DebugTopFormatValue( i32 Value );

/** f32 1 個を右カラム用に文字列化する。 */
FString DebugTopFormatValue( f32 Value );

/** bool 1 個を右カラム用に文字列化する。 */
FString DebugTopFormatValue( bool bValue );
