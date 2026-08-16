# Prefab

名前でノードを出せるようにする。

このファイルは**フォルダに何を置いてよいか**を決めるもの。迷ったらここに戻ること。

---

## エンジンから借りているもの

| ACS | 役割 |
|---|---|
| `CPrefabSystem` | 名前と作り方の対応表、識別子の発行、生成 |
| `PrefabFactoryFn` | `TObjectPtr<ANode>(*)(void*)`。1 つ作って返す関数 |
| `ANode` | 出てくるもの。`AddChild` で所有ごと親へ移る |

---

## 向きを逆にしてある

何をどう組み立てるかはゲーム側にしか分からない。枠組みは対応表を預かるだけにする。

```
  ゲーム側 ──「この名前でこれを作れる」──▶ IPrefabProvider
                                              │ ProvidePrefabs
                                              ▼
                                        CPrefabRegistrar ──▶ CPrefabSystem
```

`CPrefabSubsystem` は**何が登録されているかを知らない**。

---

## 所有の行き先で 2 つに分けてある

| 出し方 | 返るもの | 所有 |
|---|---|---|
| `SpawnAttached( 名前, 親, 置き方 )` | `ANode*` (観測用) | 親が持つ |
| `SpawnDetached( 名前, 置き方 )` | `TObjectPtr<ANode>` | 受け取った側が持つ |

ひとつの関数で両方を返そうとすると、呼ぶ側が「返り値を持ち続けてよいのか」を判断できない。

---

## フォルダに置いてよいもの

| 置き場所 | 置いてよいもの | 例 |
|---|---|---|
| 直下 | 対応表と生成に関わる型 | `CPrefabSubsystem`、`CPrefabRegistrar`、`CPrefabSpawner` |
| 直下 | 値型 | `FPrefabSpawnParams` |
| 直下 | 差込口 | `IPrefabProvider` |

**ゲーム固有の作り方はここへ置かない。** ゲーム側で `IPrefabProvider` を実装する。

---

## 使い方

```cpp
Prefabs->AddProvider( MakeUnique<CEnemyPrefabs>() );

FPrefabSpawnParams Params;
Params.Name = FString( "Slime_01" );
Params.bApplyTransform = true;
Params.LocalTransform.position = FVec3{ 3.0f, 0.0f, 0.0f };

ANode* const Slime = Prefabs->SpawnAttached( FString( "Enemy/Slime" ), RootNode, Params );
```

同じものを何度も出すなら `FindId` で識別子を控え、識別子のほうを渡すと名前引きを省ける。

---

## 気をつけること

- **名前はエンジンが複製しない。** `CPrefabRegistrar` が名前プールへ写してから渡している。
  `CPrefabSystem::Register` を直接呼ばないこと。
- 置き方 (`FPrefabSpawnParams`) は**親へ付ける前に**施される。付けた後だと、親の側の
  並び替えなどが走った後の状態へ触ることになる。
- `FPrefabSpawnParams` の `bApplyTransform` / `bApplyEnabled` を立てない限り、作り方が
  決めた値のままにする。「何も指定しなければ手を加えない」を守るため。
