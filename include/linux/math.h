/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MATH_H
#define _LINUX_MATH_H
#if 0
/*
 * Divide positive or negative dividend by positive or negative divisor
 * and round to closest integer. Result is undefined for negative
 * divisors if the dividend variable type is unsigned and for negative
 * dividends if the divisor variable type is unsigned.
 */
#define DIV_ROUND_CLOSEST(x, divisor)(			\
{
	int __x = x;				\
	int __d = divisor;			\
	(((int)-1) > 0 ||				\
	 ((int)-1) > 0 ||			\
	 (((__x) > 0) == ((__d) > 0))) ?		\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}
)
#endif
#endif
