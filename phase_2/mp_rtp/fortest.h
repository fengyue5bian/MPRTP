/*
 * fortest.h
 *
 */

#ifndef FORTEST_H_
#define FORTEST_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sysexits.h>

#define FORTEST_LEN (12)

void for_test(void)
{
int here = FORTEST_LEN;
fprintf(stderr, "******just check if its working number: %d\n", here);
}

#endif /* RTP_H_ */
