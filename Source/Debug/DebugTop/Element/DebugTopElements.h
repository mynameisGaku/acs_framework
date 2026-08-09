#pragma once

// 行の型を全部まとめて取り込む傘。
//
// ページを組む側 (利用者) は種類を選ばず使うことが多いので、これ 1 本で足りるようにする。
// モジュールの内側は、要るものだけを名指しで include すること (依存を増やさないため)。

#include "Debug/DebugTop/Element/DebugTopElement.h"
#include "Debug/DebugTop/Element/DebugTopElementAction.h"
#include "Debug/DebugTop/Element/DebugTopElementArray.h"
#include "Debug/DebugTop/Element/DebugTopElementBool.h"
#include "Debug/DebugTop/Element/DebugTopElementColor.h"
#include "Debug/DebugTop/Element/DebugTopElementEnum.h"
#include "Debug/DebugTop/Element/DebugTopElementGraph.h"
#include "Debug/DebugTop/Element/DebugTopElementNumber.h"
#include "Debug/DebugTop/Element/DebugTopElementText.h"
#include "Debug/DebugTop/Element/DebugTopElementVector.h"
#include "Debug/DebugTop/Element/DebugTopElementWatch.h"
