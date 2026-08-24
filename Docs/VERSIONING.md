# 版管理と互換性

この文書を、ACS Frameworkの公開版、公開API、互換性、公開手順の正本とする。

## 現在版

リポジトリ最上位の`VERSION`を唯一の現在版とする。C++からは
`AcsFramework_Core/Version/FrameworkVersion.h`の`kAcsFrameworkVersion`と
`kAcsFrameworkVersionText`で同じ値を読める。両者の不一致は`Tools/RunRepoChecks.ps1`で止める。

現在は`0.5.0-dev`であり、正式版ではない。`dev`と`main`のブランチ名だけでは公開版を表さず、
`vMAJOR.MINOR.PATCH`形式のGitタグとGitHub Releaseを揃えた時点を公開版とする。

## 公開APIの範囲

次を利用側との互換性契約に含める。

- `AcsFramework_Core/AcsFramework.h`から再公開する宣言
- 3Dの代表的な利用入口として`Docs/PUBLIC_API.md`に記載した宣言
- 機能READMEでゲーム側の入口として案内するpublic宣言と、派生用のvirtual hook
- セーブ、スナップショット、リプレイなど公開済みの永続形式
- `Docs/RUNTIME_CONTRACTS.md`と各機能READMEに明記した実行時の振る舞い

cppの無名namespace、private宣言、`Foo_Internal`形式の内部処理、テスト、サンプル、Debug実装は
互換性契約に含めない。protectedでも`Foo_Internal`と明記した処理は派生側の拡張点ではない。

現在保証するのはソース互換性であり、DLL境界のbinary ABIは保証しない。ACS配布物、Framework、
Effekseer、Diligentは`Docs/SUPPORTED_PLATFORMS.md`に従って同じツールセットでビルドする。

## 版の上げ方

版番号はSemantic Versioningの`MAJOR.MINOR.PATCH`を使う。

- `MAJOR`: v1.0.0以降の公開API削除、互換でない意味変更、対応済み永続形式の打切り
- `MINOR`: 後方互換な機能追加、新API、既存APIの非推奨化
- `PATCH`: 公開契約を維持する不具合修正、性能改善、文書と検査の修正

v1.0.0より前は公開APIを固める途中なので、互換でない変更を`MINOR`で行える。ただし、変更履歴、
移行方法、必要なら旧名の`using`別名または薄いラッパーを同じ変更へ含める。単なる名前変更では、
利用側が移行できる期間を設けずに旧名を消さない。

開発版は`0.5.0-dev`のような開発版識別子を付ける。正式公開時は識別子を外し、同じ版を
変更履歴へ確定してからタグを付ける。一度公開したタグと配布物を差し替えない。

`FFrameworkVersion`は数値部と開発版かどうかを比較する値であり、同じ数値部を持つ`dev`、`alpha`、
`rc`どうしの順序は区別しない。厳密な版識別には`kAcsFrameworkVersionText`を使う。

## 変更履歴

利用側のコード、素材、保存データ、見た目、操作へ影響する変更は最上位の`CHANGELOG.md`へ記録する。
作業中は`[未公開]`へ追加し、公開時に版と日付の見出しへ移す。

破壊的変更には、旧API、置換先、機械的に置換できる範囲、手作業が必要な差、保存移行の有無を書く。
内部整理だけで公開契約が変わらない変更は、利用側の判断材料にならないため列挙しない。

## 公開手順

1. `VERSION`、C++版定数、`CHANGELOG.md`を同じ公開版へ揃える。
2. `Tools/RunRepoChecks.ps1`で公開API監査を通し、`Tools/RunCiChecks.ps1`を通す。
3. 対応GPUを持つ実機でサンプルの見た目を確認する。
4. clean cloneでACS取得からRelease起動まで再現する。
5. `vMAJOR.MINOR.PATCH`タグを公開対象コミットへ付ける。
6. GitHub ReleaseへACS配布物とSHA-256を登録し、Frameworkのpinから取得確認する。

どれかが未完了なら開発版のままにし、タグまたは正式版の番号だけを先に公開しない。
