# Localization

画面へ出す文を、言語ごとに持つ。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## エンジンから借りているもの

| ACS | 役割 |
|---|---|
| `CLocalizationDirector` | (言語, 鍵) → 文 の辞書 |
| `ELocale` | 言語の種類 (En/Ja/Fr/De/Es/ZhCn/ZhTw/Ko/Pt/Ru/It、既定は En) |

**辞書を自前で書かない。** 引くときの落とし込み（今の言語 → 既定の言語 → 鍵をそのまま）も
向こうの実装で、鍵が無くても `nullptr` が返らないので画面側で null を気にしなくてよい。

エンジンが**対象外と決めているもの**がここの担当。

| 対象外 | 引き受ける先 |
|---|---|
| 文字列の寿命 (`const char*` を複製しない) | `CLocaleCatalog` |
| `{0}` の差し込み | `CTextFormatter` |
| 表の読み込み | `CLocalizationTableParser` |

---

## いちばん危ないところ

`CLocalizationDirector::RegisterString` は渡された `const char*` を**複製しない**。
一時的な文字列を渡すと、**しばらく動いた後で突然おかしな文字が出る**という追いにくい壊れ方をする。

`CLocaleCatalog` が鍵と文を写し取ってから登録するので、**呼ぶ側は寿命を気にしなくてよい**。
写しは `Common/Text/CInternedNamePool` が持つ（同じ文は 1 つに寄るので、言語をまたいでも膨らまない）。
自己テストで「渡した文字列が消えた後でも引ける」ことを毎回確かめている。

---

## 分け方

```
  表 (テキスト) ──▶ CLocalizationTableParser ──┐
                                                ├──▶ CLocaleCatalog ──▶ 文
  コードからの登録 ─────────────────────────────┘         (寿命を持つ)      │
                                                                            ▼
                                                                     CTextFormatter
                                                                      ({0} を差し替え)

  言語が変わった ──▶ CLocaleChangeBroadcaster ──▶ ILocaleChangeListener (張り替える側)
```

`CLocalizationSubsystem` はこの順番と入れ物を持つだけ。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | 文を持つ / 引く | `CLocaleCatalog` |
| 直下 | 文を組み立てる | `CTextFormatter`、`FTextArgument` |
| 直下 | 外から読む | `CLocalizationTableParser`、`CLocaleName` |
| 直下 | 変わったことを配る | `CLocaleChangeBroadcaster`、`ILocaleChangeListener` |
| 直下 | 所有と順番 | `CLocalizationSubsystem` |

デバッグ画面に出す文はここではない。それは `Debug/DebugTop` の担当（開発者しか見ない）。
ここが扱うのは**遊ぶ人が読む文**。

---

## 表の書き方

```
# 行頭の # は覚え書き
[ja]
ui.start = はじめる
ui.title = 冒険の書
battle.damage = {0} に {1} のダメージ

[en]
ui.start = Start
battle.damage = {1} damage to {0}
```

| 書き方 | どう読むか |
|---|---|
| `[ja]` `[zh-cn]` | ここから下はその言語。`-` `_` と大文字小文字は問わない |
| `key = value` | `=` の前後の空白は落とす |
| `key =` | 空の文として登録する（わざと空にしたいことがある） |
| 知らない言語の見出し | **その節をまるごと飛ばす**。落とした数に 1 数える |
| `=` の無い行 | 飛ばして数える |

**黙って落とさない。** 読めなかった行の数を返し、0 でなければ警告を出す。
翻訳が出ない原因のほとんどはここなので、数を見れば分かるようにしてある。

---

## 使い方

```cpp
Localization->LoadTable( TableText );
Localization->SetLocale( ELocale::Ja );

const FString Title = Localization->GetText( FString( "ui.title" ) );

const FTextArgument Args[] = { FTextArgument::FromText( FStringView( "スライム" ) ),
                               FTextArgument::FromInteger( 12 ) };
const FString Line = Localization->FormatText( FString( "battle.damage" ), Args, 2u );
```

---

## 気をつけること

- **語順は言語で変わる。** 文をつなぎ合わせて作らず、必ず `{0}` を使うこと。
  「{0} に {1} のダメージ」と「{1} damage to {0}」が同じ表から作れるのはこのため。
- **画面に鍵がそのまま出ていたら、その鍵が表に無い。** 落とし込みの最後の段が鍵を返す。
- 言語を変えたら、既に引いてある文は**古いまま**。`ILocaleChangeListener` で張り替えること。
  同じ言語を渡したときは配らない（変わっていないのに張り替えさせない）。
- 配る相手は**所有しない**。相手が先に消えるなら、消える前に外すこと。
- **まだ無いもの**: ファイルからの読み込み（いまは文字列を渡す形だけ）、複数形・性別の扱い、
  右から書く言語、言語に合わせたフォント切り替え。
