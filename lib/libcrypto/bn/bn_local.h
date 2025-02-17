/* $OpenBSD: bn_local.h,v 1.6 2023/01/28 16:33:34 jsing Exp $ */
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
 * Copyright (c) 1998-2000 The OpenSSL Project.  All rights reserved.
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

#ifndef HEADER_BN_LOCAL_H
#define HEADER_BN_LOCAL_H

#include <openssl/opensslconf.h>

#include <openssl/bn.h>

__BEGIN_HIDDEN_DECLS

struct bignum_st {
	BN_ULONG *d;	/* Pointer to an array of 'BN_BITS2' bit chunks. */
	int top;	/* Index of last used d +1. */
	/* The next are internal book keeping for bn_expand. */
	int dmax;	/* Size of the d array. */
	int neg;	/* one if the number is negative */
	int flags;
};

/* Used for montgomery multiplication */
struct bn_mont_ctx_st {
	int ri;        /* number of bits in R */
	BIGNUM RR;     /* used to convert to montgomery form */
	BIGNUM N;      /* The modulus */
	BIGNUM Ni;     /* R*(1/R mod N) - N*Ni = 1
	                * (Ni is only stored for bignum algorithm) */
	BN_ULONG n0[2];/* least significant word(s) of Ni;
	                  (type changed with 0.9.9, was "BN_ULONG n0;" before) */
	int flags;
};

/* Used for reciprocal division/mod functions
 * It cannot be shared between threads
 */
struct bn_recp_ctx_st {
	BIGNUM N;	/* the divisor */
	BIGNUM Nr;	/* the reciprocal */
	int num_bits;
	int shift;
	int flags;
};

/* Used for slow "generation" functions. */
struct bn_gencb_st {
	unsigned int ver;	/* To handle binary (in)compatibility */
	void *arg;		/* callback-specific data */
	union {
		/* if(ver==1) - handles old style callbacks */
		void (*cb_1)(int, int, void *);
		/* if(ver==2) - new callback style */
		int (*cb_2)(int, int, BN_GENCB *);
	} cb;
};

/*
 * BN_window_bits_for_exponent_size -- macro for sliding window mod_exp functions
 *
 *
 * For window size 'w' (w >= 2) and a random 'b' bits exponent,
 * the number of multiplications is a constant plus on average
 *
 *    2^(w-1) + (b-w)/(w+1);
 *
 * here  2^(w-1)  is for precomputing the table (we actually need
 * entries only for windows that have the lowest bit set), and
 * (b-w)/(w+1)  is an approximation for the expected number of
 * w-bit windows, not counting the first one.
 *
 * Thus we should use
 *
 *    w >= 6  if        b > 671
 *     w = 5  if  671 > b > 239
 *     w = 4  if  239 > b >  79
 *     w = 3  if   79 > b >  23
 *    w <= 2  if   23 > b
 *
 * (with draws in between).  Very small exponents are often selected
 * with low Hamming weight, so we use  w = 1  for b <= 23.
 */
#define BN_window_bits_for_exponent_size(b) \
		((b) > 671 ? 6 : \
		 (b) > 239 ? 5 : \
		 (b) >  79 ? 4 : \
		 (b) >  23 ? 3 : 1)


/* BN_mod_exp_mont_consttime is based on the assumption that the
 * L1 data cache line width of the target processor is at least
 * the following value.
 */
#define MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH	( 64 )
#define MOD_EXP_CTIME_MIN_CACHE_LINE_MASK	(MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH - 1)

/* Window sizes optimized for fixed window size modular exponentiation
 * algorithm (BN_mod_exp_mont_consttime).
 *
 * To achieve the security goals of BN_mode_exp_mont_consttime, the
 * maximum size of the window must not exceed
 * log_2(MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH).
 *
 * Window size thresholds are defined for cache line sizes of 32 and 64,
 * cache line sizes where log_2(32)=5 and log_2(64)=6 respectively. A
 * window size of 7 should only be used on processors that have a 128
 * byte or greater cache line size.
 */
#if MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH == 64

#  define BN_window_bits_for_ctime_exponent_size(b) \
		((b) > 937 ? 6 : \
		 (b) > 306 ? 5 : \
		 (b) >  89 ? 4 : \
		 (b) >  22 ? 3 : 1)
#  define BN_MAX_WINDOW_BITS_FOR_CTIME_EXPONENT_SIZE	(6)

#elif MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH == 32

#  define BN_window_bits_for_ctime_exponent_size(b) \
		((b) > 306 ? 5 : \
		 (b) >  89 ? 4 : \
		 (b) >  22 ? 3 : 1)
#  define BN_MAX_WINDOW_BITS_FOR_CTIME_EXPONENT_SIZE	(5)

#endif


/* Pentium pro 16,16,16,32,64 */
/* Alpha       16,16,16,16.64 */
#define BN_MULL_SIZE_NORMAL			(16) /* 32 */
#define BN_MUL_RECURSIVE_SIZE_NORMAL		(16) /* 32 less than */
#define BN_SQR_RECURSIVE_SIZE_NORMAL		(16) /* 32 */
#define BN_MUL_LOW_RECURSIVE_SIZE_NORMAL	(32) /* 32 */
#define BN_MONT_CTX_SET_SIZE_WORD		(64) /* 32 */

#if !defined(OPENSSL_NO_ASM) && !defined(OPENSSL_NO_INLINE_ASM)
/*
 * BN_UMULT_HIGH section.
 *
 * No, I'm not trying to overwhelm you when stating that the
 * product of N-bit numbers is 2*N bits wide:-) No, I don't expect
 * you to be impressed when I say that if the compiler doesn't
 * support 2*N integer type, then you have to replace every N*N
 * multiplication with 4 (N/2)*(N/2) accompanied by some shifts
 * and additions which unavoidably results in severe performance
 * penalties. Of course provided that the hardware is capable of
 * producing 2*N result... That's when you normally start
 * considering assembler implementation. However! It should be
 * pointed out that some CPUs (most notably Alpha, PowerPC and
 * upcoming IA-64 family:-) provide *separate* instruction
 * calculating the upper half of the product placing the result
 * into a general purpose register. Now *if* the compiler supports
 * inline assembler, then it's not impossible to implement the
 * "bignum" routines (and have the compiler optimize 'em)
 * exhibiting "native" performance in C. That's what BN_UMULT_HIGH
 * macro is about:-)
 *
 *					<appro@fy.chalmers.se>
 */
# if defined(__alpha)
#  if defined(__GNUC__) && __GNUC__>=2
#   define BN_UMULT_HIGH(a,b)	({	\
	BN_ULONG ret;		\
	asm ("umulh	%1,%2,%0"	\
	     : "=r"(ret)		\
	     : "r"(a), "r"(b));		\
	ret;			})
#  endif	/* compiler */
# elif defined(_ARCH_PPC) && defined(_LP64)
#  if defined(__GNUC__) && __GNUC__>=2
#   define BN_UMULT_HIGH(a,b)	({	\
	BN_ULONG ret;		\
	asm ("mulhdu	%0,%1,%2"	\
	     : "=r"(ret)		\
	     : "r"(a), "r"(b));		\
	ret;			})
#  endif	/* compiler */
# elif defined(__x86_64) || defined(__x86_64__)
#  if defined(__GNUC__) && __GNUC__>=2
#   define BN_UMULT_HIGH(a,b)	({	\
	BN_ULONG ret,discard;	\
	asm ("mulq	%3"		\
	     : "=a"(discard),"=d"(ret)	\
	     : "a"(a), "g"(b)		\
	     : "cc");			\
	ret;			})
#   define BN_UMULT_LOHI(low,high,a,b)	\
	asm ("mulq	%3"		\
		: "=a"(low),"=d"(high)	\
		: "a"(a),"g"(b)		\
		: "cc");
#  endif
# elif defined(__mips) && defined(_LP64)
#  if defined(__GNUC__) && __GNUC__>=2
#   if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4) /* "h" constraint is no more since 4.4 */
#     define BN_UMULT_HIGH(a,b)		 (((__uint128_t)(a)*(b))>>64)
#     define BN_UMULT_LOHI(low,high,a,b) ({	\
	__uint128_t ret=(__uint128_t)(a)*(b);	\
	(high)=ret>>64; (low)=ret;	 })
#   else
#     define BN_UMULT_HIGH(a,b)	({	\
	BN_ULONG ret;		\
	asm ("dmultu	%1,%2"		\
	     : "=h"(ret)		\
	     : "r"(a), "r"(b) : "l");	\
	ret;			})
#     define BN_UMULT_LOHI(low,high,a,b)\
	asm ("dmultu	%2,%3"		\
	     : "=l"(low),"=h"(high)	\
	     : "r"(a), "r"(b));
#    endif
#  endif
# endif		/* cpu */
#endif		/* OPENSSL_NO_ASM */

/*************************************************************
 * Using the long long type
 */
#define Lw(t)    (((BN_ULONG)(t))&BN_MASK2)
#define Hw(t)    (((BN_ULONG)((t)>>BN_BITS2))&BN_MASK2)

#ifdef BN_LLONG
#define mul_add(r,a,w,c) { \
	BN_ULLONG t; \
	t=(BN_ULLONG)w * (a) + (r) + (c); \
	(r)= Lw(t); \
	(c)= Hw(t); \
	}

#define mul(r,a,w,c) { \
	BN_ULLONG t; \
	t=(BN_ULLONG)w * (a) + (c); \
	(r)= Lw(t); \
	(c)= Hw(t); \
	}

#define sqr(r0,r1,a) { \
	BN_ULLONG t; \
	t=(BN_ULLONG)(a)*(a); \
	(r0)=Lw(t); \
	(r1)=Hw(t); \
	}

#elif defined(BN_UMULT_LOHI)
#define mul_add(r,a,w,c) {		\
	BN_ULONG high,low,ret,tmp=(a);	\
	ret =  (r);			\
	BN_UMULT_LOHI(low,high,w,tmp);	\
	ret += (c);			\
	(c) =  (ret<(c))?1:0;		\
	(c) += high;			\
	ret += low;			\
	(c) += (ret<low)?1:0;		\
	(r) =  ret;			\
	}

#define mul(r,a,w,c)	{		\
	BN_ULONG high,low,ret,ta=(a);	\
	BN_UMULT_LOHI(low,high,w,ta);	\
	ret =  low + (c);		\
	(c) =  high;			\
	(c) += (ret<low)?1:0;		\
	(r) =  ret;			\
	}

#define sqr(r0,r1,a)	{		\
	BN_ULONG tmp=(a);		\
	BN_UMULT_LOHI(r0,r1,tmp,tmp);	\
	}

#elif defined(BN_UMULT_HIGH)
#define mul_add(r,a,w,c) {		\
	BN_ULONG high,low,ret,tmp=(a);	\
	ret =  (r);			\
	high=  BN_UMULT_HIGH(w,tmp);	\
	ret += (c);			\
	low =  (w) * tmp;		\
	(c) =  (ret<(c))?1:0;		\
	(c) += high;			\
	ret += low;			\
	(c) += (ret<low)?1:0;		\
	(r) =  ret;			\
	}

#define mul(r,a,w,c)	{		\
	BN_ULONG high,low,ret,ta=(a);	\
	low =  (w) * ta;		\
	high=  BN_UMULT_HIGH(w,ta);	\
	ret =  low + (c);		\
	(c) =  high;			\
	(c) += (ret<low)?1:0;		\
	(r) =  ret;			\
	}

#define sqr(r0,r1,a)	{		\
	BN_ULONG tmp=(a);		\
	(r0) = tmp * tmp;		\
	(r1) = BN_UMULT_HIGH(tmp,tmp);	\
	}

#else
/*************************************************************
 * No long long type
 */

#define LBITS(a)	((a)&BN_MASK2l)
#define HBITS(a)	(((a)>>BN_BITS4)&BN_MASK2l)
#define	L2HBITS(a)	(((a)<<BN_BITS4)&BN_MASK2)

#define mul64(l,h,bl,bh) \
	{ \
	BN_ULONG m,m1,lt,ht; \
 \
	lt=l; \
	ht=h; \
	m =(bh)*(lt); \
	lt=(bl)*(lt); \
	m1=(bl)*(ht); \
	ht =(bh)*(ht); \
	m=(m+m1)&BN_MASK2; if (m < m1) ht+=L2HBITS((BN_ULONG)1); \
	ht+=HBITS(m); \
	m1=L2HBITS(m); \
	lt=(lt+m1)&BN_MASK2; if (lt < m1) ht++; \
	(l)=lt; \
	(h)=ht; \
	}

#define sqr64(lo,ho,in) \
	{ \
	BN_ULONG l,h,m; \
 \
	h=(in); \
	l=LBITS(h); \
	h=HBITS(h); \
	m =(l)*(h); \
	l*=l; \
	h*=h; \
	h+=(m&BN_MASK2h1)>>(BN_BITS4-1); \
	m =(m&BN_MASK2l)<<(BN_BITS4+1); \
	l=(l+m)&BN_MASK2; if (l < m) h++; \
	(lo)=l; \
	(ho)=h; \
	}

#define mul_add(r,a,bl,bh,c) { \
	BN_ULONG l,h; \
 \
	h= (a); \
	l=LBITS(h); \
	h=HBITS(h); \
	mul64(l,h,(bl),(bh)); \
 \
	/* non-multiply part */ \
	l=(l+(c))&BN_MASK2; if (l < (c)) h++; \
	(c)=(r); \
	l=(l+(c))&BN_MASK2; if (l < (c)) h++; \
	(c)=h&BN_MASK2; \
	(r)=l; \
	}

#define mul(r,a,bl,bh,c) { \
	BN_ULONG l,h; \
 \
	h= (a); \
	l=LBITS(h); \
	h=HBITS(h); \
	mul64(l,h,(bl),(bh)); \
 \
	/* non-multiply part */ \
	l+=(c); if ((l&BN_MASK2) < (c)) h++; \
	(c)=h&BN_MASK2; \
	(r)=l&BN_MASK2; \
	}
#endif /* !BN_LLONG */

/* mul_add_c(a,b,c0,c1,c2)  -- c+=a*b for three word number c=(c2,c1,c0) */
/* mul_add_c2(a,b,c0,c1,c2) -- c+=2*a*b for three word number c=(c2,c1,c0) */
/* sqr_add_c(a,i,c0,c1,c2)  -- c+=a[i]^2 for three word number c=(c2,c1,c0) */
/* sqr_add_c2(a,i,c0,c1,c2) -- c+=2*a[i]*a[j] for three word number c=(c2,c1,c0) */

#ifdef BN_LLONG
/*
 * Keep in mind that additions to multiplication result can not
 * overflow, because its high half cannot be all-ones.
 */
#define mul_add_c(a,b,c0,c1,c2)		do {	\
	BN_ULONG hi;				\
	BN_ULLONG t = (BN_ULLONG)(a)*(b);	\
	t += c0;		/* no carry */	\
	c0 = (BN_ULONG)Lw(t);			\
	hi = (BN_ULONG)Hw(t);			\
	c1 = (c1+hi)&BN_MASK2; if (c1<hi) c2++;	\
	} while(0)

#define mul_add_c2(a,b,c0,c1,c2)	do {	\
	BN_ULONG hi;				\
	BN_ULLONG t = (BN_ULLONG)(a)*(b);	\
	BN_ULLONG tt = t+c0;	/* no carry */	\
	c0 = (BN_ULONG)Lw(tt);			\
	hi = (BN_ULONG)Hw(tt);			\
	c1 = (c1+hi)&BN_MASK2; if (c1<hi) c2++;	\
	t += c0;		/* no carry */	\
	c0 = (BN_ULONG)Lw(t);			\
	hi = (BN_ULONG)Hw(t);			\
	c1 = (c1+hi)&BN_MASK2; if (c1<hi) c2++;	\
	} while(0)

#define sqr_add_c(a,i,c0,c1,c2)		do {	\
	BN_ULONG hi;				\
	BN_ULLONG t = (BN_ULLONG)a[i]*a[i];	\
	t += c0;		/* no carry */	\
	c0 = (BN_ULONG)Lw(t);			\
	hi = (BN_ULONG)Hw(t);			\
	c1 = (c1+hi)&BN_MASK2; if (c1<hi) c2++;	\
	} while(0)

#define sqr_add_c2(a,i,j,c0,c1,c2) \
	mul_add_c2((a)[i],(a)[j],c0,c1,c2)

#elif defined(BN_UMULT_LOHI)
/*
 * Keep in mind that additions to hi can not overflow, because
 * the high word of a multiplication result cannot be all-ones.
 */
#define mul_add_c(a,b,c0,c1,c2)		do {	\
	BN_ULONG ta = (a), tb = (b);		\
	BN_ULONG lo, hi;			\
	BN_UMULT_LOHI(lo,hi,ta,tb);		\
	c0 += lo; hi += (c0<lo)?1:0;		\
	c1 += hi; c2 += (c1<hi)?1:0;		\
	} while(0)

#define mul_add_c2(a,b,c0,c1,c2)	do {	\
	BN_ULONG ta = (a), tb = (b);		\
	BN_ULONG lo, hi, tt;			\
	BN_UMULT_LOHI(lo,hi,ta,tb);		\
	c0 += lo; tt = hi+((c0<lo)?1:0);	\
	c1 += tt; c2 += (c1<tt)?1:0;		\
	c0 += lo; hi += (c0<lo)?1:0;		\
	c1 += hi; c2 += (c1<hi)?1:0;		\
	} while(0)

#define sqr_add_c(a,i,c0,c1,c2)		do {	\
	BN_ULONG ta = (a)[i];			\
	BN_ULONG lo, hi;			\
	BN_UMULT_LOHI(lo,hi,ta,ta);		\
	c0 += lo; hi += (c0<lo)?1:0;		\
	c1 += hi; c2 += (c1<hi)?1:0;		\
	} while(0)

#define sqr_add_c2(a,i,j,c0,c1,c2)	\
	mul_add_c2((a)[i],(a)[j],c0,c1,c2)

#elif defined(BN_UMULT_HIGH)
/*
 * Keep in mind that additions to hi can not overflow, because
 * the high word of a multiplication result cannot be all-ones.
 */
#define mul_add_c(a,b,c0,c1,c2)		do {	\
	BN_ULONG ta = (a), tb = (b);		\
	BN_ULONG lo = ta * tb;			\
	BN_ULONG hi = BN_UMULT_HIGH(ta,tb);	\
	c0 += lo; hi += (c0<lo)?1:0;		\
	c1 += hi; c2 += (c1<hi)?1:0;		\
	} while(0)

#define mul_add_c2(a,b,c0,c1,c2)	do {	\
	BN_ULONG ta = (a), tb = (b), tt;	\
	BN_ULONG lo = ta * tb;			\
	BN_ULONG hi = BN_UMULT_HIGH(ta,tb);	\
	c0 += lo; tt = hi + ((c0<lo)?1:0);	\
	c1 += tt; c2 += (c1<tt)?1:0;		\
	c0 += lo; hi += (c0<lo)?1:0;		\
	c1 += hi; c2 += (c1<hi)?1:0;		\
	} while(0)

#define sqr_add_c(a,i,c0,c1,c2)		do {	\
	BN_ULONG ta = (a)[i];			\
	BN_ULONG lo = ta * ta;			\
	BN_ULONG hi = BN_UMULT_HIGH(ta,ta);	\
	c0 += lo; hi += (c0<lo)?1:0;		\
	c1 += hi; c2 += (c1<hi)?1:0;		\
	} while(0)

#define sqr_add_c2(a,i,j,c0,c1,c2)	\
	mul_add_c2((a)[i],(a)[j],c0,c1,c2)

#else /* !BN_LLONG */
/*
 * Keep in mind that additions to hi can not overflow, because
 * the high word of a multiplication result cannot be all-ones.
 */
#define mul_add_c(a,b,c0,c1,c2)		do {	\
	BN_ULONG lo = LBITS(a), hi = HBITS(a);	\
	BN_ULONG bl = LBITS(b), bh = HBITS(b);	\
	mul64(lo,hi,bl,bh);			\
	c0 = (c0+lo)&BN_MASK2; if (c0<lo) hi++;	\
	c1 = (c1+hi)&BN_MASK2; if (c1<hi) c2++;	\
	} while(0)

#define mul_add_c2(a,b,c0,c1,c2)	do {	\
	BN_ULONG tt;				\
	BN_ULONG lo = LBITS(a), hi = HBITS(a);	\
	BN_ULONG bl = LBITS(b), bh = HBITS(b);	\
	mul64(lo,hi,bl,bh);			\
	tt = hi;				\
	c0 = (c0+lo)&BN_MASK2; if (c0<lo) tt++;	\
	c1 = (c1+tt)&BN_MASK2; if (c1<tt) c2++;	\
	c0 = (c0+lo)&BN_MASK2; if (c0<lo) hi++;	\
	c1 = (c1+hi)&BN_MASK2; if (c1<hi) c2++;	\
	} while(0)

#define sqr_add_c(a,i,c0,c1,c2)		do {	\
	BN_ULONG lo, hi;			\
	sqr64(lo,hi,(a)[i]);			\
	c0 = (c0+lo)&BN_MASK2; if (c0<lo) hi++;	\
	c1 = (c1+hi)&BN_MASK2; if (c1<hi) c2++;	\
	} while(0)

#define sqr_add_c2(a,i,j,c0,c1,c2) \
	mul_add_c2((a)[i],(a)[j],c0,c1,c2)
#endif /* !BN_LLONG */

/* The least significant word of a BIGNUM. */
#define BN_lsw(n) (((n)->top == 0) ? (BN_ULONG) 0 : (n)->d[0])

void bn_mul_normal(BN_ULONG *r, BN_ULONG *a, int na, BN_ULONG *b, int nb);
void bn_mul_comba4(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b);
void bn_mul_comba8(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b);

void bn_sqr_normal(BN_ULONG *r, const BN_ULONG *a, int n, BN_ULONG *tmp);
void bn_sqr_comba4(BN_ULONG *r, const BN_ULONG *a);
void bn_sqr_comba8(BN_ULONG *r, const BN_ULONG *a);

int bn_cmp_words(const BN_ULONG *a, const BN_ULONG *b, int n);
int bn_cmp_part_words(const BN_ULONG *a, const BN_ULONG *b,
    int cl, int dl);
void bn_mul_recursive(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b, int n2,
    int dna, int dnb, BN_ULONG *t);
void bn_mul_part_recursive(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b,
    int n, int tna, int tnb, BN_ULONG *t);
void bn_sqr_recursive(BN_ULONG *r, const BN_ULONG *a, int n2, BN_ULONG *t);
BN_ULONG bn_sub_part_words(BN_ULONG *r, const BN_ULONG *a, const BN_ULONG *b,
    int cl, int dl);
int bn_mul_mont(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp,
    const BN_ULONG *np, const BN_ULONG *n0, int num);

void bn_correct_top(BIGNUM *a);
int bn_expand(BIGNUM *a, int bits);
int bn_wexpand(BIGNUM *a, int words);

BN_ULONG bn_add_words(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp,
    int num);
BN_ULONG bn_sub_words(BN_ULONG *rp, const BN_ULONG *ap, const BN_ULONG *bp,
    int num);
BN_ULONG bn_mul_add_words(BN_ULONG *rp, const BN_ULONG *ap, int num, BN_ULONG w);
BN_ULONG bn_mul_words(BN_ULONG *rp, const BN_ULONG *ap, int num, BN_ULONG w);
void     bn_sqr_words(BN_ULONG *rp, const BN_ULONG *ap, int num);
BN_ULONG bn_div_words(BN_ULONG h, BN_ULONG l, BN_ULONG d);
void bn_div_rem_words(BN_ULONG h, BN_ULONG l, BN_ULONG d, BN_ULONG *out_q,
    BN_ULONG *out_r);

int BN_bntest_rand(BIGNUM *rnd, int bits, int top, int bottom);
int bn_rand_interval(BIGNUM *rnd, const BIGNUM *lower_inc, const BIGNUM *upper_exc);

/* Explicitly const time / non-const time versions for internal use */
int BN_mod_exp_ct(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
    const BIGNUM *m, BN_CTX *ctx);
int BN_mod_exp_nonct(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
    const BIGNUM *m, BN_CTX *ctx);
int BN_mod_exp_mont_ct(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
    const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx);
int BN_mod_exp_mont_nonct(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
    const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx);
int BN_div_nonct(BIGNUM *dv, BIGNUM *rem, const BIGNUM *m, const BIGNUM *d,
    BN_CTX *ctx);
int BN_div_ct(BIGNUM *dv, BIGNUM *rem, const BIGNUM *m, const BIGNUM *d,
    BN_CTX *ctx);
#define BN_mod_ct(rem,m,d,ctx) BN_div_ct(NULL,(rem),(m),(d),(ctx))
#define BN_mod_nonct(rem,m,d,ctx) BN_div_nonct(NULL,(rem),(m),(d),(ctx))
BIGNUM *BN_mod_inverse_ct(BIGNUM *ret, const BIGNUM *a, const BIGNUM *n,
    BN_CTX *ctx);
BIGNUM *BN_mod_inverse_nonct(BIGNUM *ret, const BIGNUM *a, const BIGNUM *n,
    BN_CTX *ctx);
int	BN_gcd_ct(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx);
int	BN_gcd_nonct(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx);

int	BN_swap_ct(BN_ULONG swap, BIGNUM *a, BIGNUM *b, size_t nwords);

int bn_isqrt(BIGNUM *out_sqrt, int *out_perfect, const BIGNUM *n, BN_CTX *ctx);
int bn_is_perfect_square(int *out_perfect, const BIGNUM *n, BN_CTX *ctx);

int bn_is_prime_bpsw(int *is_prime, const BIGNUM *n, BN_CTX *in_ctx);

__END_HIDDEN_DECLS
#endif /* !HEADER_BN_LOCAL_H */
