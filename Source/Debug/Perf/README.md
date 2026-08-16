# Perf

フレームの予算を決め、どこで使い切っているかを見る。開発中の道具。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## エンジンから借りているもの

数える仕組みは ACS が持っている。ここは**再実装しない**。

| ACS | 役割 |
|---|---|
| `CPerfBudget` | カテゴリごとの ms / bytes の積算、フレーム平均、超過判定 |
| `CClock` | tick の取得 (`Ticks` / `TicksPerSecond`) |

このモジュールが足すのは「誰が持つか」「いつ開け閉めするか」「どう見せるか」の 3 つだけ。

---

## 層と、依存してよい向き

上の層は下の層を知ってよい。**下から上へは参照しない。**

```
          ┌──────────────────────────────┐
  入口 ─▶ │ CPerfBudgetSubsystem          │  所有・寿命・開け閉め。処理は持たない
          └───────┬───────────────┬───────┘
                  ▼               ▼
     ┌─────────────────┐   ┌──────────────────────┐
     │ CPerfCategoryPlan│   │ CPerfBudgetSnapshot   │  写す・並べ替える・数える
     └────────┬─────────┘   └───────────┬──────────┘
              ▼                         ▼
   FPerfCategoryDefinition        FPerfBudgetRow        値だけ。判定は自分の中で完結
```

`View/` は上記すべてを知ってよいが、**上記から View を参照しない**。計測される側
(`FScopedPerfSample` を書く場所) はサブシステムだけを知る。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | 予算そのものに関わる型 | `CPerfBudgetSubsystem`、`CPerfCategoryPlan` |
| 直下 | 値型 (データと、そのデータだけで決まる判定) | `FPerfCategoryDefinition`、`FPerfBudgetRow` |
| 直下 | 計測の道具 | `FScopedPerfSample` |
| `View/` | デバッグメニューへの見せ方 | `APerfBudgetPage`、`CPerfCategoryRow` |

ゲーム固有のカテゴリ定義はここへ置かない。`DefineCategory` でゲーム側から足す。

---

## 関数の分け方

| 役割 | 例 | 決まり |
|---|---|---|
| 流れ | `Configure` / `BeginFrame` / `EndFrame` / `Update` | 呼ぶ順番だけ。計算しない |
| 収集 | `CPerfCategoryPlan::Add` | 溜めるだけ。エンジンへはまだ流さない |
| 実処理 | `ApplyTo` / `CaptureFrom` / `SortByTimePressure` | 副作用ひとつ |
| 判定 | `IsOverBudget` / `IsOverFrameBudget` / `GetTimePressure` | `const noexcept` の純関数 |

---

## 使い方

```cpp
// アプリの起動時に 1 度だけ (枠組みの既定カテゴリは Configure が入れる)
Perf->Configure( 16.6f );
Perf->DefineCategory( FString( "Gameplay/AI" ), 2.0f );

// 測りたい範囲を囲む。抜ける経路がいくつあっても 1 回だけ積まれる
{
    const FScopedPerfSample Sample( Perf, "Gameplay/AI" );
    UpdateEnemies( DeltaSeconds );
}
```

カテゴリ名は**フレームより長生きする文字列**を渡すこと。`CPerfBudget` は名前を複製せず
ポインタのまま持つ。文字列リテラルか `DefineCategory` の戻り値なら安全。`FString` の
中身をそのまま渡してはいけない (伸縮した瞬間に別の場所を指す)。

---

## 気をつけること

- **数字は 1 フレームに 1 度だけ写す。** 行ごとにエンジンを覗くと、同じ画面に別々の
  瞬間の値が並ぶ。`APerfBudgetPage` が `Update` で 1 度写し、行はそれを読む。
- `BeginFrame` を呼ばずに `EndFrame` を呼んでも何も起きない (フレーム外の積算を
  平均へ混ぜないため)。
- 計測そのものにも時間がかかる。1 フレームに数千回囲むような使い方はしない。
