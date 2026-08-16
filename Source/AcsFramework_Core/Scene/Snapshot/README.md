# Snapshot

ノードの木をファイルへ落とし、ファイルから戻す。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## エンジンから借りているもの

| ACS | 役割 |
|---|---|
| `TrySaveNodeTree(root, buf, cap)` | 木 → バイト列。足りなければ必要な大きさを返す |
| `TryLoadNodeTree(data, size)` | バイト列 → 木。コンポーネントは名前から作り直す |
| `CSaveArchive::WriteToFile / ReadFromFile` | 一時ファイル → 置き換え。CRC 付き |
| `ESceneSerializeError` | 何が起きたか |

**ファイルの置き換えを自前で書かない。** `CSaveArchive` は一時ファイルへ書いてから差し替える
ので、書いている途中で落ちても前のファイルが壊れない。自前で書くとこの安全性を失う。

---

## 4 段に分けてある

```
  木 ──▶ CSceneSnapshotWriter ──▶ CSceneSnapshotBuffer ──▶ CSceneSnapshotFile ──▶ ファイル
         (足りなければ広げて                (使い回す)              (置き換え)
          もう一度)

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

- **ノード名は保存されない。** Engine の形式 (version 4) に名前の欄が無く、保存されるのは
  親子関係・変換・enabled/visible/drawLayer/drawPriority/ySort とコンポーネントだけ。
  復元すると全ノードが**無名**になるので、**名前で引くゲームは静かに壊れる**。
  どれがどれかを見分けるには、並び順かコンポーネントの中身を使うこと
  (自己テストでこの前提を毎回確かめている。通らなくなったら Engine が名前を持った合図)。
- **コンポーネントは名前から作り直される** (`CreateComponentByName`)。ゲーム固有の
  コンポーネントは、その名前でエンジンに知られていなければ復元されない。復元したい型が
  出てこないときは、まずここを疑うこと。
- エンジン側の上限: ノード 65536 / 深さ 512 / 1 ノードあたりコンポーネント 1024 /
  コンポーネントの中身 4096 バイト。
- 入れ物は使い回す。しばらく保存しないと分かっているなら `ReleaseBuffer()` で手放せる。
- 「入れ物が足りない」だけは広げてやり直す価値がある。それ以外の失敗はやり直しても同じ
  結果になる (`CSceneSnapshotStatus::IsCorruptData` で切り分けられる)。
