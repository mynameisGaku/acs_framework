# Common/Test

ゲームを起動せずに回す単体テストの土台と入口。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## 走らせ方

```powershell
.\Tools\RunUnitTests.ps1                    # 素の部品を確かめる (これ)
.\Tools\RunSimulationDeterminismTest.ps1    # 記録と再生が一致するかを確かめる
```

どちらも exe を 1 つ作って走らせるだけ。窓も描画も音も要らない。終了コード 0 が PASS。

2 つに分かれているのは見ているものが違うため。前者は**部品が単体で正しいか**、後者は
**組み合わせた結果が再現するか**。前者が通って後者が落ちたなら、部品ではなく繋ぎ方が疑わしい。

---

## 何を置くか

| 置き場所 | 置くもの |
|---|---|
| `Common/Test/TestHarness.*` | 確かめる土台 (数える・落ちた場所を出す) |
| `Common/Test/UnitTestMain.cpp` | 入口。各モジュールの `RunXxxTests` を呼ぶ |
| `<モジュール>/Test/XxxTest.cpp` | そのモジュールのテスト本体 |

テストは**そのモジュールの隣**に置く。土台の側へ集めると、モジュールを消したときに
テストだけが残る。

---

## テストを足す手順

1. `<モジュール>/Test/XxxTest.cpp` に `void RunXxxTests( CTestHarness& Harness )` を書く
2. `UnitTestMain.cpp` に宣言と呼び出しを 1 行ずつ足す
3. `Tools/RunUnitTests.ps1` の `$sources` に、テストと対象の `.cpp` を足す
4. `.vcxproj` へは **`<None>` として登録する**（ビルドへ入れない。`main()` が 2 つになる）

```cpp
void RunXxxTests( CTestHarness& Harness )
{
    Harness.BeginSuite( "CXxx / 何を確かめるか" );

    ACS_TEST_CHECK( Harness, Thing.IsValid() );
    Harness.CheckEqualU64( Thing.Num(), 3u, "件数" );
    Harness.CheckEqualF32( Thing.GetValue(), 0.5f, "値" );
}
```

---

## ここへ入れる基準

**窓・描画・音・装置を要らないもの**だけを入れる。具体的には、壊れても静かに間違う種類のもの。

- 数値や文字列の読み取り (`CConsoleArgumentReader`)
- バイト列の書き出しと読み込み (`CActionInputTape`)
- 寿命とポインタの安定性 (`CInternedNamePool`、`CPerfCategoryPlan`)
- 優先順位や並べ替えの決め方 (`CMusicStateArbiter`、`CPerfBudgetSnapshot`)
- 番号の配り直し (`CSpatialSourceRegistry`)
- 上限と、そこに達したときの振る舞い (`CSimulationEventQueue`)
- 結果の分類 (`CSceneSnapshotStatus`)
- ファイルへの往復と、版違い・欠損の扱い (`CAcsArchiveFile`、`CSimulationSnapshot`)
- 所有の行き先 (`CPrefabSpawner` の Attached / Detached)
- 誰に配るかの判断と上限 (`CHotReloadDispatcher`)

DebugTop のページや描画は入れない。**外の世界を読むものは、読む相手を差し替えられる形にしてから
入れる** (`IActionDeviceReader`、`IHotReloadEventSource`)。差し替えられないまま入れようとすると、
テストのために本物の装置やファイル変更が要るようになり、結局動かせない。

---

## 気をつけること

- **最初の失敗で止めない。** 止めると 1 回の実行で 1 つしか分からない。
- **「常に通るテスト」を疑う。** 入口に土台自身の負のコントロールを置いてある
  (わざと落として、落ちたと数えられることを見る)。実行時に出る 2 行の `[NG]` はそれ。
  期待どおりに動いていない場合は、逆にここが静かになる。
- 小数は既定で完全一致で見る。再現性を確かめる場所で «だいたい同じ» を許すと意味が無い。
  近似で足りるところだけ `CheckNearF32` を使う。
- 2026-08-16 時点: 271 件・0 失敗。
