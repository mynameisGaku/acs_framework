# Snapshot

ノードの木をファイルへ落とし、ファイルから戻す。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## エンジンから借りているもの

| ACS | 役割 |
|---|---|
| `TrySaveNodeTree(root, buf, cap)` | 名前以外の木 → ACS バイト列。足りなければ必要な大きさを返す |
| `TryLoadNodeTree(data, size)` | ACS バイト列 → 名前以外の木。コンポーネントは名前から作り直す |
| `CSaveArchive::WriteToFile / ReadFromFile` | 一時ファイル → 置き換え。CRC 付き |
| `ESceneSerializeError` | 何が起きたか |

**ファイルの置き換えを自前で書かない。** `CSaveArchive` は一時ファイルへ書いてから差し替える
ので、書いている途中で落ちても前のファイルが壊れない。自前で書くとこの安全性を失う。

---

## 4 段に分けてある

```
  木 ──▶ CSceneSnapshotWriter ──▶ CSceneSnapshotBuffer ──▶ CSceneSnapshotFile ──▶ ファイル
         (ACS バイト列 + 名前表)             (使い回す)              (置き換え)

  ファイル ──▶ CSceneSnapshotFile ──▶ CSceneSnapshotBuffer ──▶ CSceneSnapshotReader ──▶ 木
```

`CSceneSnapshotStatus` はどの段からも使える「結果の読み解き方」。
`CSceneSnapshotSubsystem` はこの順番と入れ物を持つだけ。
ファイルの置き換えは `Common/File/CAcsArchiveFile` へ集約してある (親フォルダの作成込み)。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | 落とす / 戻す / 置くの各段 | `CSceneSnapshotWriter`、`CSceneSnapshotReader`、`CSceneSnapshotFile` |
| 直下 | 使い回す入れ物 | `CSceneSnapshotBuffer` |
| 直下 | 結果の読み解き | `CSceneSnapshotStatus` |
| 直下 | 所有と順番 | `CSceneSnapshotSubsystem` |

セーブデータ (プレイヤーの進行) はここではない。それは `Save/` の担当。ここが扱うのは
**シーンの形そのもの**で、用途はステージの保存や、開発中の状態の持ち出し。

---

## 使い方

```cpp
Snapshot->SaveToFile( RootNode, FString( "Saved/Scene/Stage1.acssave" ) );

TObjectPtr<ANode> Restored = Snapshot->LoadFromFile( FString( "Saved/Scene/Stage1.acssave" ) );
if ( !Restored ) ACS_LOG_WARN( "%s", Snapshot->MakeLastErrorMessage().Data() );
```

---

## 気をつけること

- **ノード名は Framework 形式で保存する。** Engine の形式 v4 自体には名前欄が無いため、
  `FSceneSnapshotFormat` が ACS バイト列を変更せず内包し、その後ろへ DFS 先行順の UTF-8
  名前表を添える。1 ノード名は 64 KiB まで。名前表を全て検証してから木へ反映する。
- 旧 Framework が保存した **ACS v2/v3/v4 の生バイト列も読み込める**。その形式に名前は無いので、
  従来ファイルから復元したノード名だけは空になる。新形式から旧形式への読み込みはできない。
- **コンポーネントは名前から作り直される** (`CreateComponentByName`)。ゲーム固有の
  コンポーネントは、その名前でエンジンに知られていなければ復元されない。復元したい型が
  出てこないときは、まずここを疑うこと。
- エンジン側の上限: ノード 65536 / 深さ 512 / 1 ノードあたりコンポーネント 1024 /
  コンポーネントの中身 4096 バイト。
- 入れ物は使い回す。しばらく保存しないと分かっているなら `ReleaseBuffer()` で手放せる。
- 「入れ物が足りない」だけは広げてやり直す価値がある。それ以外の失敗はやり直しても同じ
  結果になる (`CSceneSnapshotStatus::IsCorruptData` で切り分けられる)。
