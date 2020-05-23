### Nanosecond-aware date/time formatters

Standard `strftime()` and `strptime()` do not handle subsecond values. Python allows `"%f"` to indicate microseconds; the functions in this library extend that capability to nanoseconds.

```
#include <assert.h>
#include <string.h>

#include "strtime.h"

int main() {
  struct tm tm_;
  int nanos;

  memset(&tm_, 0, sizeof(tm_));
  nanos = 0;

  char timestr[] = "2019-03-25 05:30:24.051421000";
  strptime_ns(timestr, "%Y-%m-%d %H:%M:%S.%f", &tm_, &nanos);

  char buffer[80];
  strftime_ns(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S.%f", &tm_, nanos);

  assert(strcmp(timestr, buffer) == 0);
  return 0;
}
```

These routines are based on [tzcode](https://android.googlesource.com/platform/bionic/+/master/libc/tzcode) from Android's Bionic, available under the BSD license.

### Infer datetime format from string

One difficulty with strtime functions is that they require a precise format string. A quick-and-dirty solution is provided in this library.

```
char format[80];
istrtime(timestr, format, sizeof(format));

strptime_ns(timestr, format, &tm_, &nanos);
strftime_ns(buffer, sizeof(buffer), format, &tm_, nanos);
```

### Fast timegm() and gmtime()

The system `timegm()` handles quirks of the Gregorian calendar that we don't need. Similarly, the system `gmtime()` on Windows does not handle negative clocks (pre-epoch dates). This library offers faster functions that can handle a fuller range of timestamps.

```
time_t clock = fast_timegm(&tm_);
fast_gmtime(&clock, &tm_);
```

By necessity, this only works for UTC; timezones and daylight savings are too costly.
