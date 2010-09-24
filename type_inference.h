/* -*- coding: utf-8-dos; mode: C -*- */
#ifndef TYPE_INFERENCE_H_INCLUDED
#define TYPE_INFERENCE_H_INCLUDED

/**
 * 型変数を正規化する
 * @param TC_TypePtr ttp 型構造へのポインタ
 * @return TC_TypePtr 型構造へのポインタ
 */
TC_TypePtr norm(const TC_TypePtr ttp);

/**
 * 単純型の領域を確保
 * @return TC_TypePtr 確保した領域へのポインタ
 */
TC_TypePtr sty_alloc(void);

/**
 * フレッシュな型変数を生成し、その領域を確保
 * @return TC_TypePtr 確保した領域へのポインタ
 */
TC_TypePtr tyvar_alloc(void);

/**
 * 単一化を実行する
 * @param TC_TypePtr ttp1
 * @param TC_TypePtr ttp2
 * @return int 成否
 */
int unify(const TC_TypePtr ttp1, const TC_TypePtr ttp2);

/**
 * 型推論を実行する
 * @param TEnvPtr env
 * @param ProgPtr pp
 * @return TC_TypePtr
 */
TC_TypePtr typing_prog(TEnvPtr tenv, ProgPtr pp);
#endif
