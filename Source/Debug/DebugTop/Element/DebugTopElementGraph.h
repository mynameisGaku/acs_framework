#pragma once

#include <acs.h>

#include "Debug/DebugTop/Element/DebugTopElement.h"

using namespace acs;

// 値の移り変わりを折れ線で見る行。値は持たず、毎フレーム取って溜める。

using FDebugTopValueDelegate = TDelegate<f32()>;

/**
 * 値の移り変わりを折れ線で見せる行。
 *
 * @details
 * 毎フレーム値を取って溜め、直近ぶんを折れ線で描く。数字だけでは分からない「跳ねている」
 * 「じわじわ増えている」が見える。値は持たないので保存の対象にはしない。
 */
class CDebugTopElementGraph : public CDebugTopElement
{
public:
	/**
	 * 行を構築する。
	 *
	 * @param Label 左カラムへ出す表示名。
	 * @param Provider 値をその場で取るデリゲート。
	 * @param SampleCount 溜める標本の数 (横幅ぶん。0 なら既定)。
	 */
	CDebugTopElementGraph( const FString& Label, FDebugTopValueDelegate Provider, usize SampleCount = 0 );

	/**
	 * 縦軸を固定する。
	 *
	 * @details
	 * 指定しなければ溜まっている標本の最小・最大に合わせて自動で伸縮する。自動だと僅かな
	 * 揺れが画面いっぱいに見えてしまうので、範囲が分かっている値は固定した方が読みやすい。
	 * @param Min 縦軸の下端。
	 * @param Max 縦軸の上端。
	 */
	void SetRange( f32 Min, f32 Max ) noexcept;

	/** 標本を 1 つ溜める。 */
	void OnTick( f32 DeltaSeconds ) override;

	/** 開いている間だけ折れ線を描く行として振る舞う。 */
	bool TryGetGraph( const f32*& OutSamples, usize& OutCount, f32& OutMin, f32& OutMax ) const noexcept override;

	/**
	 * 折れ線を出しているときだけ背を高くする。
	 *
	 * @details 畳んでいる間は普通の行と同じ高さで、右カラムに最新の値だけが出る。
	 */
	f32 GetHeightRatio() const noexcept override { return IsExpanded() ? 4.0f : 1.0f; }

	/** 子行を持たないが、折れ線の出し入れのために開閉できる。 */
	bool CanCollapse() const noexcept override { return true; }

	/** 右カラムへ最新の値を出す。 */
	FString GetValueText() const override;

	/**
	 * 直近の標本を数値として返す。
	 *
	 * @details 注意色の範囲判定 (SetWarnRange) から使われる。まだ 1 つも溜まっていなければ false。
	 * @param OutValue 書き込み先。
	 * @return 標本があれば true。
	 */
	bool TryGetFloat( f32& OutValue ) const noexcept override;

	/** 値を持たないので保存の対象にはしない。 */
	bool IsSaveable() const noexcept override { return false; }

private:
	/** 値をその場で取るデリゲート。 */
	FDebugTopValueDelegate m_Provider;

	/** 溜めた標本 (古い順。先頭から捨てて後ろへ足す)。 */
	TArray<f32> m_Samples;

	/** 溜める標本の数。 */
	usize m_Capacity = 0;

	/** 縦軸の下端 (自動のときは無視)。 */
	f32 m_Min = 0.0f;

	/** 縦軸の上端 (自動のときは無視)。 */
	f32 m_Max = 0.0f;

	/** 縦軸を固定しているか。 */
	bool m_bFixedRange = false;
};
