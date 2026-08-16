// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <acs.h>

using namespace acs;

/**
 * ゲームを起動せずに回す単体テストの、共通の土台。
 *
 * @details
 * 期待どおりかを 1 つずつ確かめ、落ちた場所と中身を残す。**最初の失敗で止めない**。
 * 止めてしまうと、1 回の実行で 1 つしか分からず、直しては走らせるのを繰り返すことになる。
 *
 * 表示は printf だけに頼る。テストのために別の仕組みを持ち込むと、その仕組み自体が
 * 疑わしくなったときに調べる手段が無くなる。
 *
 * @code
 * void RunMyTests( CTestHarness& Harness )
 * {
 *     Harness.BeginSuite( "MyThing" );
 *     ACS_TEST_CHECK( Harness, Thing.IsValid() );
 *     Harness.CheckEqualU64( Thing.Count(), 3u, "件数" );
 * }
 * @endcode
 */
class CTestHarness
{
public:
	/**
	 * これから確かめる塊の名前を出す。
	 *
	 * @param SuiteName 塊の名前。
	 */
	void BeginSuite( const char* SuiteName ) noexcept;

	/**
	 * 条件が成り立つかを確かめる。
	 *
	 * @param bCondition 成り立つべき条件。
	 * @param Expression 条件をそのまま文字にしたもの (落ちたときに出す)。
	 * @return 成り立てば true。
	 */
	bool Check( bool bCondition, const char* Expression ) noexcept;

	/**
	 * 2 つの整数が同じかを確かめる。
	 *
	 * @param Actual 実際の値。
	 * @param Expected 期待する値。
	 * @param Label 落ちたときに出す名前。
	 * @return 同じなら true。
	 */
	bool CheckEqualU64( u64 Actual, u64 Expected, const char* Label ) noexcept;

	/**
	 * 2 つの小数が同じかを確かめる。
	 *
	 * @details 完全一致で見る。再現性を確かめるテストで «だいたい同じ» を許すと意味が無い。
	 * @param Actual 実際の値。
	 * @param Expected 期待する値。
	 * @param Label 落ちたときに出す名前。
	 * @return 同じなら true。
	 */
	bool CheckEqualF32( f32 Actual, f32 Expected, const char* Label ) noexcept;

	/**
	 * 2 つの小数が十分近いかを確かめる。
	 *
	 * @param Actual 実際の値。
	 * @param Expected 期待する値。
	 * @param Tolerance 許す差。
	 * @param Label 落ちたときに出す名前。
	 * @return 近ければ true。
	 */
	bool CheckNearF32( f32 Actual, f32 Expected, f32 Tolerance, const char* Label ) noexcept;

	/** 結果をまとめて出す。 */
	void Report() const noexcept;

	/** 全て通ったかを返す。 */
	bool IsAllPassed() const noexcept { return m_FailureCount == 0u; }

	/** 確かめた数を返す。 */
	u32 GetCheckCount() const noexcept { return m_CheckCount; }

	/** 落ちた数を返す。 */
	u32 GetFailureCount() const noexcept { return m_FailureCount; }

private:
	/** 落ちたことを記録して出す。 */
	void ReportFailure( const char* Label ) noexcept;

	/** いま確かめている塊の名前。所有はしない。 */
	const char* m_SuiteName = "";

	/** 確かめた数。 */
	u32 m_CheckCount = 0u;

	/** 落ちた数。 */
	u32 m_FailureCount = 0u;
};

/** 条件式をそのまま名前として残しつつ確かめる。 */
#define ACS_TEST_CHECK( Harness, Condition ) ( Harness ).Check( ( Condition ), #Condition )
