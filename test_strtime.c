/*
 * Copyright (C) 2019--2020 Empirical Software Solutions, LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "strtime.h"

#include <string.h>
#include <stdio.h>

int main_return;

/* check that expected time-since-epoch (UTC) matches */
static void verify_p(const char *str, const char *format,
                     time_t expected_seconds, int expected_nanos) {
  struct tm tm_ret;
  int nanos_ret;
  char *func_ret;
  time_t seconds_ret;

  memset(&tm_ret, 0, sizeof(tm_ret));
  nanos_ret = 0;

  func_ret = strptime_ns(str, format, &tm_ret, &nanos_ret);

  if (func_ret == NULL) {
    printf("%s with %s -- strptime_ns() failed\n", str, format);
    main_return = 1;
  }
  else {
    if (tm_ret.tm_year == 0) {
      tm_ret.tm_year = 70;
    }
    if (tm_ret.tm_mday == 0) {
      tm_ret.tm_mday = 1;
    }
#ifdef _MSC_VER
    seconds_ret = _mkgmtime(&tm_ret);
#else /* _MSC_VER */
    seconds_ret = timegm(&tm_ret);
#endif /* _MSC_VER */

    if (seconds_ret != expected_seconds || nanos_ret != expected_nanos) {
      printf("%s with %s -- %ld, %d\n", str, format, seconds_ret, nanos_ret);
      main_return = 1;
    }
  }
}

/* check that expected string matches */
static void verify_f(time_t seconds, int nanos, const char *format,
                     const char* expected_str) {
  struct tm tm_ret;
  size_t func_ret;
  char str_ret[80];

  memset(&tm_ret, 0, sizeof(tm_ret));
#ifdef _MSC_VER
  gmtime_s(&tm_ret, &seconds);
#else /* _MSC_VER */
  gmtime_r(&seconds, &tm_ret);
#endif /* _MSC_VER */


  func_ret = strftime_ns(str_ret, sizeof(str_ret), format, &tm_ret, nanos);

  if (func_ret == 0) {
    printf("%ld, %d with %s -- strftime_ns() failed\n", seconds, nanos, format);
    main_return = 1;
  }
  else {
    if (strcmp(str_ret, expected_str) != 0) {
      printf("%ld, %d with %s -- %s\n", seconds, nanos, format, str_ret);
      main_return = 1;
    }
  }
}

/* check that expected format matches */
static void verify_i(const char *str, const char *expected_fmt) {
  char *func_ret;
  char fmt_ret[80];

  func_ret = istrtime(str, fmt_ret, sizeof(fmt_ret));

  if (func_ret == NULL) {
    printf("%s -- istrtime() failed\n", str);
    main_return = 1;
  }
  else {
    if (strcmp(fmt_ret, expected_fmt) != 0) {
      printf("%s -- %s\n", str, fmt_ret);
      main_return = 1;
    }
  }
}

/* check that expected time_t matchs */
static void verify_gm(int year, int month, int day) {
  struct tm tm_;
  time_t result, expected;

  memset(&tm_, 0, sizeof(tm_));
  tm_.tm_year = year;
  tm_.tm_mon = month;
  tm_.tm_mday = day;

  result = fast_timegm(&tm_);

#ifdef _MSC_VER
  expected = _mkgmtime(&tm_);
#else /* _MSC_VER */
  expected = timegm(&tm_);
#endif /* _MSC_VER */

  if (result != expected) {
    printf("%d/%d/%d -- %ld vs %ld\n", year, month, day, expected, result);
    main_return = 1;
  }
}

/* check that expected struct tm matches */
static void verify_tm(time_t seconds) {
  struct tm result, expected;
  char str_result[80], str_expected[80];

  memset(&result, 0, sizeof(result));
  fast_gmtime(&seconds, &result);

  memset(&expected, 0, sizeof(expected));
#ifdef _MSC_VER
  gmtime_s(&expected, &seconds);
#else /* _MSC_VER */
  gmtime_r(&seconds, &expected);
#endif /* _MSC_VER */

  char format[] = "%Y-%m-%d %H:%M:%S";
  strftime_ns(str_result, sizeof(str_result), format, &result, 0);
  strftime_ns(str_expected, sizeof(str_expected), format, &expected, 0);

  if (strcmp(str_result, str_expected) != 0) {
    printf("%ld -- %s vs %s\n", seconds, str_expected, str_result);
    main_return = 1;
  }
}

int main() {
  main_return = 0;

  verify_p("2019-03-25 05:30:24.051421", "%Y-%m-%d %H:%M:%S.%f", 1553491824, 51421000);
  verify_p("2019-03-25 05:30:24.051", "%Y-%m-%d %H:%M:%S.%f", 1553491824, 51000000);
  verify_p("2019-01-01", "%Y-%m-%d", 1546300800, 0);
  verify_p("12:15:00", "%H:%M:%S", 44100, 0);
  verify_p("12:15:00.123", "%H:%M:%S.%f", 44100, 123000000);

  verify_f(1553497665, 551950000, "%Y-%m-%d %H:%M:%S.%f", "2019-03-25 07:07:45.551950000");
  verify_f(1553497665, 51950000, "%Y-%m-%d %H:%M:%S.%f", "2019-03-25 07:07:45.051950000");
  verify_f(1546300800, 0, "%Y-%m-%d", "2019-01-01");
  verify_f(13510, 0, "%H:%M:%S", "03:45:10");
  verify_f(13510, 123000, "%H:%M:%S.%f", "03:45:10.000123000");

  verify_i("2017", "%Y");
  verify_i("2017-01", "%Y-%m");
  verify_i("2017-01-01", "%Y-%m-%d");
  verify_i("2017/01/01", "%Y/%m/%d");
  verify_i("2017/01-01", "%Y/%m-%H");
  verify_i("2017- 01-01", "%Y- %H-01");
  verify_i("2017-1-01", "%Y-1-%H");
  verify_i("2017-01-1", "%Y-%m-1");
  verify_i("12", "%H");
  verify_i("12:20", "%H:%M");
  verify_i("12:20:55", "%H:%M:%S");
  verify_i("12:20:55.455763000", "%H:%M:%S.%f");
  verify_i("12:20:55.455763", "%H:%M:%S.%f");
  verify_i("12:20:55.055", "%H:%M:%S.%f");
  verify_i("12:20:55.", "%H:%M:%S.");
  verify_i("12:20:55.1234567890", "%H:%M:%S.1234567890");
  verify_i("12-22:35", "%H-22:35");
  verify_i("2017-01-01 12:20:55", "%Y-%m-%d %H:%M:%S");
  verify_i("2017-01-01 12:20:55.455763", "%Y-%m-%d %H:%M:%S.%f");

  verify_gm(46, 5, 4);
  verify_gm(67, 0, 1);
  verify_gm(68, 1, 28);
  verify_gm(68, 1, 29);
  verify_gm(68, 2, 1);
  verify_gm(70, 0, 1);
  verify_gm(72, 1, 28);
  verify_gm(72, 1, 29);
  verify_gm(72, 2, 1);
  verify_gm(85, 4, 15);
  verify_gm(99, 5, 4);
  verify_gm(100, 1, 1);
  verify_gm(105, 3, 8);
  verify_gm(112, 7, 31);
  verify_gm(117, 11, 30);

  verify_tm(-746625600);
  verify_tm(-94564800);
  verify_tm(-58104000);
  verify_tm(-58017600);
  verify_tm(-57931200);
  verify_tm(0);
  verify_tm(68126400);
  verify_tm(68212800);
  verify_tm(68299200);
  verify_tm(1553491824);

  return main_return;
}

