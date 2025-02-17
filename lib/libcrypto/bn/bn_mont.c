/* $OpenBSD: bn_mont.c,v 1.34 2023/01/28 17:07:02 jsing Exp $ */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */
/* ====================================================================
 * Copyright (c) 1998-2006 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

/*
 * Details about Montgomery multiplication algorithms can be found at
 * http://security.ece.orst.edu/publications.html, e.g.
 * http://security.ece.orst.edu/koc/papers/j37acmon.pdf and
 * sections 3.8 and 4.2 in http://security.ece.orst.edu/koc/papers/r01rsasw.pdf
 */

#include <stdio.h>
#include <stdint.h>

#include "bn_local.h"

#ifdef OPENSSL_NO_ASM
#ifdef OPENSSL_BN_ASM_MONT
int
bn_mul_mont(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp,
    const BN_ULONG *np, const BN_ULONG *n0p, int num)
{
	BN_ULONG c0, c1, *tp, n0 = *n0p;
	int i = 0, j;

	tp = calloc(NULL, num + 2, sizeof(BN_ULONG));
	if (tp == NULL)
		return 0;

	for (i = 0; i < num; i++) {
		c0 = bn_mul_add_words(tp, ap, num, bp[i]);
		c1 = (tp[num] + c0) & BN_MASK2;
		tp[num] = c1;
		tp[num + 1] = (c1 < c0 ? 1 : 0);

		c0 = bn_mul_add_words(tp, np, num, tp[0] * n0);
		c1 = (tp[num] + c0) & BN_MASK2;
		tp[num] = c1;
		tp[num + 1] += (c1 < c0 ? 1 : 0);
		for (j = 0; j <= num; j++)
			tp[j] = tp[j + 1];
	}

	if (tp[num] != 0 || tp[num - 1] >= np[num - 1]) {
		c0 = bn_sub_words(rp, tp, np, num);
		if (tp[num] != 0 || c0 == 0) {
			goto out;
		}
	}
	memcpy(rp, tp, num * sizeof(BN_ULONG));
out:
	freezero(tp, (num + 2) * sizeof(BN_ULONG));
	return 1;
}
#else /* !OPENSSL_BN_ASM_MONT */
int
bn_mul_mont(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp,
    const BN_ULONG *np, const BN_ULONG *n0, int num)
{
	/*
	 * Return value of 0 indicates that multiplication/convolution was not
	 * performed to signal the caller to fall down to alternative/original
	 * code-path.
	 */
	return 0;
}
#endif /* !OPENSSL_BN_ASM_MONT */
#endif /* OPENSSL_NO_ASM */

#define MONT_WORD /* use the faster word-based algorithm */

#ifdef MONT_WORD
static int BN_from_montgomery_word(BIGNUM *ret, BIGNUM *r, BN_MONT_CTX *mont);
#endif

int
BN_mod_mul_montgomery(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
    BN_MONT_CTX *mont, BN_CTX *ctx)
{
	BIGNUM *tmp;
	int ret = 0;
#if defined(OPENSSL_BN_ASM_MONT) && defined(MONT_WORD)
	int num = mont->N.top;

	if (num > 1 && a->top == num && b->top == num) {
		if (!bn_wexpand(r, num))
			return (0);
		if (bn_mul_mont(r->d, a->d, b->d, mont->N.d, mont->n0, num)) {
			r->neg = a->neg^b->neg;
			r->top = num;
			bn_correct_top(r);
			return (1);
		}
	}
#endif

	BN_CTX_start(ctx);
	if ((tmp = BN_CTX_get(ctx)) == NULL)
		goto err;

	if (a == b) {
		if (!BN_sqr(tmp, a, ctx))
			goto err;
	} else {
		if (!BN_mul(tmp, a,b, ctx))
			goto err;
	}
	/* reduce from aRR to aR */
#ifdef MONT_WORD
	if (!BN_from_montgomery_word(r, tmp, mont))
		goto err;
#else
	if (!BN_from_montgomery(r, tmp, mont, ctx))
		goto err;
#endif
	ret = 1;
err:
	BN_CTX_end(ctx);
	return (ret);
}

int
BN_to_montgomery(BIGNUM *r, const BIGNUM *a, BN_MONT_CTX *mont, BN_CTX *ctx)
{
	return BN_mod_mul_montgomery(r, a, &mont->RR, mont, ctx);
}

#ifdef MONT_WORD
static int
BN_from_montgomery_word(BIGNUM *ret, BIGNUM *r, BN_MONT_CTX *mont)
{
	BIGNUM *n;
	BN_ULONG *ap, *np, *rp, n0, v, carry;
	int nl, max, i;

	n = &(mont->N);
	nl = n->top;
	if (nl == 0) {
		ret->top = 0;
		return (1);
	}

	max = (2 * nl); /* carry is stored separately */
	if (!bn_wexpand(r, max))
		return (0);

	r->neg ^= n->neg;
	np = n->d;
	rp = r->d;

	/* clear the top words of T */
#if 1
	for (i=r->top; i<max; i++) /* memset? XXX */
		rp[i] = 0;
#else
	memset(&(rp[r->top]), 0, (max - r->top) * sizeof(BN_ULONG));
#endif

	r->top = max;
	n0 = mont->n0[0];

	for (carry = 0, i = 0; i < nl; i++, rp++) {
		v = bn_mul_add_words(rp, np, nl, (rp[0] * n0) & BN_MASK2);
		v = (v + carry + rp[nl]) & BN_MASK2;
		carry |= (v != rp[nl]);
		carry &= (v <= rp[nl]);
		rp[nl] = v;
	}

	if (!bn_wexpand(ret, nl))
		return (0);
	ret->top = nl;
	ret->neg = r->neg;

	rp = ret->d;
	ap = &(r->d[nl]);

#define BRANCH_FREE 1
#if BRANCH_FREE
	{
		BN_ULONG *nrp;
		size_t m;

		v = bn_sub_words(rp, ap, np, nl) - carry;
		/* if subtraction result is real, then
		 * trick unconditional memcpy below to perform in-place
		 * "refresh" instead of actual copy. */
		m = (0 - (size_t)v);
		nrp = (BN_ULONG *)(((uintptr_t)rp & ~m)|((uintptr_t)ap & m));

		for (i = 0, nl -= 4; i < nl; i += 4) {
			BN_ULONG t1, t2, t3, t4;

			t1 = nrp[i + 0];
			t2 = nrp[i + 1];
			t3 = nrp[i + 2];
			ap[i + 0] = 0;
			t4 = nrp[i + 3];
			ap[i + 1] = 0;
			rp[i + 0] = t1;
			ap[i + 2] = 0;
			rp[i + 1] = t2;
			ap[i + 3] = 0;
			rp[i + 2] = t3;
			rp[i + 3] = t4;
		}
		for (nl += 4; i < nl; i++)
			rp[i] = nrp[i], ap[i] = 0;
	}
#else
	if (bn_sub_words (rp, ap, np, nl) - carry)
		memcpy(rp, ap, nl*sizeof(BN_ULONG));
#endif
	bn_correct_top(r);
	bn_correct_top(ret);

	return (1);
}
#endif	/* MONT_WORD */

int
BN_from_montgomery(BIGNUM *ret, const BIGNUM *a, BN_MONT_CTX *mont, BN_CTX *ctx)
{
	int retn = 0;
#ifdef MONT_WORD
	BIGNUM *t;

	BN_CTX_start(ctx);
	if ((t = BN_CTX_get(ctx)) && BN_copy(t, a))
		retn = BN_from_montgomery_word(ret, t, mont);
	BN_CTX_end(ctx);
#else /* !MONT_WORD */
	BIGNUM *t1, *t2;

	BN_CTX_start(ctx);
	if ((t1 = BN_CTX_get(ctx)) == NULL)
		goto err;
	if ((t2 = BN_CTX_get(ctx)) == NULL)
		goto err;

	if (!BN_copy(t1, a))
		goto err;
	BN_mask_bits(t1, mont->ri);

	if (!BN_mul(t2, t1, &mont->Ni, ctx))
		goto err;
	BN_mask_bits(t2, mont->ri);

	if (!BN_mul(t1, t2, &mont->N, ctx))
		goto err;
	if (!BN_add(t2, a, t1))
		goto err;
	if (!BN_rshift(ret, t2, mont->ri))
		goto err;

	if (BN_ucmp(ret, &(mont->N)) >= 0) {
		if (!BN_usub(ret, ret, &(mont->N)))
			goto err;
	}
	retn = 1;

err:
	BN_CTX_end(ctx);
#endif /* MONT_WORD */
	return (retn);
}

BN_MONT_CTX *
BN_MONT_CTX_new(void)
{
	BN_MONT_CTX *ret;

	if ((ret = malloc(sizeof(BN_MONT_CTX))) == NULL)
		return (NULL);

	BN_MONT_CTX_init(ret);
	ret->flags = BN_FLG_MALLOCED;
	return (ret);
}

void
BN_MONT_CTX_init(BN_MONT_CTX *ctx)
{
	ctx->ri = 0;
	BN_init(&(ctx->RR));
	BN_init(&(ctx->N));
	BN_init(&(ctx->Ni));
	ctx->n0[0] = ctx->n0[1] = 0;
	ctx->flags = 0;
}

void
BN_MONT_CTX_free(BN_MONT_CTX *mont)
{
	if (mont == NULL)
		return;

	BN_clear_free(&(mont->RR));
	BN_clear_free(&(mont->N));
	BN_clear_free(&(mont->Ni));
	if (mont->flags & BN_FLG_MALLOCED)
		free(mont);
}

int
BN_MONT_CTX_set(BN_MONT_CTX *mont, const BIGNUM *mod, BN_CTX *ctx)
{
	int ret = 0;
	BIGNUM *Ri, *R;

	if (BN_is_zero(mod))
		return 0;

	BN_CTX_start(ctx);
	if ((Ri = BN_CTX_get(ctx)) == NULL)
		goto err;
	R = &(mont->RR);				/* grab RR as a temp */
	if (!BN_copy(&(mont->N), mod))
		 goto err;				/* Set N */
	mont->N.neg = 0;

#ifdef MONT_WORD
	{
		BIGNUM tmod;
		BN_ULONG buf[2];

		BN_init(&tmod);
		tmod.d = buf;
		tmod.dmax = 2;
		tmod.neg = 0;

		mont->ri = (BN_num_bits(mod) +
		    (BN_BITS2 - 1)) / BN_BITS2 * BN_BITS2;

#if defined(OPENSSL_BN_ASM_MONT) && (BN_BITS2<=32)
		/* Only certain BN_BITS2<=32 platforms actually make use of
		 * n0[1], and we could use the #else case (with a shorter R
		 * value) for the others.  However, currently only the assembler
		 * files do know which is which. */

		BN_zero(R);
		if (!(BN_set_bit(R, 2 * BN_BITS2)))
			goto err;

		tmod.top = 0;
		if ((buf[0] = mod->d[0]))
			tmod.top = 1;
		if ((buf[1] = mod->top > 1 ? mod->d[1] : 0))
			tmod.top = 2;

		if ((BN_mod_inverse_ct(Ri, R, &tmod, ctx)) == NULL)
			goto err;
		if (!BN_lshift(Ri, Ri, 2 * BN_BITS2))
			goto err; /* R*Ri */
		if (!BN_is_zero(Ri)) {
			if (!BN_sub_word(Ri, 1))
				goto err;
		}
		else /* if N mod word size == 1 */
		{
			if (!bn_wexpand(Ri, 2))
				goto err;
			/* Ri-- (mod double word size) */
			Ri->neg = 0;
			Ri->d[0] = BN_MASK2;
			Ri->d[1] = BN_MASK2;
			Ri->top = 2;
		}
		if (!BN_div_ct(Ri, NULL, Ri, &tmod, ctx))
			goto err;
		/* Ni = (R*Ri-1)/N,
		 * keep only couple of least significant words: */
		mont->n0[0] = (Ri->top > 0) ? Ri->d[0] : 0;
		mont->n0[1] = (Ri->top > 1) ? Ri->d[1] : 0;
#else
		BN_zero(R);
		if (!(BN_set_bit(R, BN_BITS2)))
			goto err;	/* R */

		buf[0] = mod->d[0]; /* tmod = N mod word size */
		buf[1] = 0;
		tmod.top = buf[0] != 0 ? 1 : 0;
		/* Ri = R^-1 mod N*/
		if ((BN_mod_inverse_ct(Ri, R, &tmod, ctx)) == NULL)
			goto err;
		if (!BN_lshift(Ri, Ri, BN_BITS2))
			goto err; /* R*Ri */
		if (!BN_is_zero(Ri)) {
			if (!BN_sub_word(Ri, 1))
				goto err;
		}
		else /* if N mod word size == 1 */
		{
			if (!BN_set_word(Ri, BN_MASK2))
				goto err;  /* Ri-- (mod word size) */
		}
		if (!BN_div_ct(Ri, NULL, Ri, &tmod, ctx))
			goto err;
		/* Ni = (R*Ri-1)/N,
		 * keep only least significant word: */
		mont->n0[0] = (Ri->top > 0) ? Ri->d[0] : 0;
		mont->n0[1] = 0;
#endif
	}
#else /* !MONT_WORD */
	{ /* bignum version */
		mont->ri = BN_num_bits(&mont->N);
		BN_zero(R);
		if (!BN_set_bit(R, mont->ri))
			goto err;  /* R = 2^ri */
		/* Ri = R^-1 mod N*/
		if ((BN_mod_inverse_ct(Ri, R, &mont->N, ctx)) == NULL)
			goto err;
		if (!BN_lshift(Ri, Ri, mont->ri))
			goto err; /* R*Ri */
		if (!BN_sub_word(Ri, 1))
			goto err;
		/* Ni = (R*Ri-1) / N */
		if (!BN_div_ct(&(mont->Ni), NULL, Ri, &mont->N, ctx))
			goto err;
	}
#endif

	/* setup RR for conversions */
	BN_zero(&(mont->RR));
	if (!BN_set_bit(&(mont->RR), mont->ri*2))
		goto err;
	if (!BN_mod_ct(&(mont->RR), &(mont->RR), &(mont->N), ctx))
		goto err;

	ret = 1;

err:
	BN_CTX_end(ctx);
	return ret;
}

BN_MONT_CTX *
BN_MONT_CTX_copy(BN_MONT_CTX *to, BN_MONT_CTX *from)
{
	if (to == from)
		return (to);

	if (!BN_copy(&(to->RR), &(from->RR)))
		return NULL;
	if (!BN_copy(&(to->N), &(from->N)))
		return NULL;
	if (!BN_copy(&(to->Ni), &(from->Ni)))
		return NULL;
	to->ri = from->ri;
	to->n0[0] = from->n0[0];
	to->n0[1] = from->n0[1];
	return (to);
}

BN_MONT_CTX *
BN_MONT_CTX_set_locked(BN_MONT_CTX **pmont, int lock, const BIGNUM *mod,
    BN_CTX *ctx)
{
	int got_write_lock = 0;
	BN_MONT_CTX *ret;

	CRYPTO_r_lock(lock);
	if (!*pmont) {
		CRYPTO_r_unlock(lock);
		CRYPTO_w_lock(lock);
		got_write_lock = 1;

		if (!*pmont) {
			ret = BN_MONT_CTX_new();
			if (ret && !BN_MONT_CTX_set(ret, mod, ctx))
				BN_MONT_CTX_free(ret);
			else
				*pmont = ret;
		}
	}

	ret = *pmont;

	if (got_write_lock)
		CRYPTO_w_unlock(lock);
	else
		CRYPTO_r_unlock(lock);

	return ret;
}
