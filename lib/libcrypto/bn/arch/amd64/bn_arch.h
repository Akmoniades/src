/*	$OpenBSD: bn_arch.h,v 1.8 2023/01/28 16:33:34 jsing Exp $ */
/*
 * Copyright (c) 2023 Joel Sing <jsing@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <openssl/bn.h>

#ifndef HEADER_BN_ARCH_H
#define HEADER_BN_ARCH_H

#ifndef OPENSSL_NO_ASM

#define HAVE_BN_ADD_WORDS

#define HAVE_BN_DIV_WORDS

#define HAVE_BN_MUL_ADD_WORDS
#define HAVE_BN_MUL_COMBA4
#define HAVE_BN_MUL_COMBA8
#define HAVE_BN_MUL_WORDS

#define HAVE_BN_SQR
#define HAVE_BN_SQR_COMBA4
#define HAVE_BN_SQR_COMBA8
#define HAVE_BN_SQR_WORDS

#define HAVE_BN_SUB_WORDS

#if defined(__GNUC__)
#define HAVE_BN_DIV_REM_WORDS_INLINE

static inline void
bn_div_rem_words_inline(BN_ULONG h, BN_ULONG l, BN_ULONG d, BN_ULONG *out_q,
    BN_ULONG *out_r)
{
	BN_ULONG q, r;

	/*
	 * Unsigned division of %rdx:%rax by d with quotient being stored in
	 * %rax and remainder in %rdx.
	 */
	__asm__ volatile ("divq %4"
	    : "=a"(q), "=d"(r)
	    : "d"(h), "a"(l), "rm"(d)
	    : "cc");

	*out_q = q;
	*out_r = r;
}
#endif /* __GNUC__ */

#endif
#endif
