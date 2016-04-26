//
//  EdenTime.h
//
//  Copyright (c) 2001-2012 Philip Lamb (PRL) phil@eden.net.nz. All rights reserved.
//
//      Rev             Date            Who             Changes
//      1.0.0   2001-11-26      PRL             Initial version for The SRMS simulator.
//      1.0.1   2005-09-07      PRL             Misc changes till now. Added headerdoc.
//

// @@BEGIN_EDEN_LICENSE_HEADER@@
//
//  This file is part of The Eden Library.
//
//  The Eden Library is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  The Eden Library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with The Eden Library.  If not, see <http://www.gnu.org/licenses/>.
//
//  As a special exception, the copyright holders of this library give you
//  permission to link this library with independent modules to produce an
//  executable, regardless of the license terms of these independent modules, and to
//  copy and distribute the resulting executable under terms of your choice,
//  provided that you also meet, for each linked independent module, the terms and
//  conditions of the license of that module. An independent module is a module
//  which is neither derived from nor based on this library. If you modify this
//  library, you may extend this exception to your version of the library, but you
//  are not obligated to do so. If you do not wish to do so, delete this exception
//  statement from your version.
//
// @@END_EDEN_LICENSE_HEADER@@

// HeaderDoc documentation included. See http://developer.apple.com/darwin/projects/headerdoc/

/*!
        @header EdenTime
        @abstract Time utility routines.
        @version 1.0.1
        @updated 2005-09-07
        @discussion
                EdenTime forms one part of the Eden library, and it makes
                use of other parts of the library for its internal operations.
        @related EdenLog
        @copyright 2005 Philip Lamb
 */
#ifndef __EdenTime_h__
#define __EdenTime_h__

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
//      Public types, defines and includes
// ============================================================================

#ifndef __Eden_h__
#  include <Eden/Eden.h>
#endif

#if defined(EDEN_UNIX)
#  include <sys/time.h>                                 // struct timespec, struct timeval, gettimeofday()
#else
#  ifndef PTHREAD_H                                             // pthreads_win32 also defines struct timespec.
struct timespec
{
    long tv_sec;
    long tv_nsec;
};
#  endif // !PTHREAD_H
#endif

// ============================================================================
//      Public functions
// ============================================================================

/*!
    @function
    @abstract   Get current time in fractional seconds.
    @discussion
                Supported via system calls on Unix, WIN32, and MacOS.
                Other systems which support GLUT are also supported (with lesser accuracy.)
    @result     The time.
 */
double EdenTimeInSeconds(void);

/*!
    @function
    @abstract   Get current time, add microseconds offset to it, and return as a timespec.
    @discussion
                Gets an absolute time in seconds and nanoseconds elapsed since
                epoch (Jan 1, 1970), add parameter 'microseconds' microseconds
                and return in a timespec structure, suitable e.g. for passing to
                functions such as pthread_cond_timedwait().
                Supported via system calls on Unix, and WIN32.
        @param      result Timespec structure which will be filled with the result.
        @param      microseconds How many microseconds in the future the timespec will be.

 */
void EdenTimeAbsolutePlusOffset(struct timespec *result, const long microseconds);

/*!
    @function
    @abstract   Generate human-readable string representing time.
    @discussion (description)
                Put character string with a human-readable representation of the
                time passed in parameter 'seconds' into 's'.
                On Unix, Windows and other platforms supporting the ctime();
                system function call, the string returned will be in ctime
                format, i.e. of the format "Thu Nov 24 18:22:48 1986", which
                is a string with constant field widths, and taking into account
                the current time zone and daylight savings time.
        @param      seconds The time value, should be the number of seconds
                elapsed since epoch (1st Jan 1970), such as generated by
                EdenTimeInSeconds();
        @param      s Pointer to a character buffer, no smaller than 25
                bytes in size, which will be filled with the string.
        @result Pointer to the passed in buffer.
 */
char*EdenTimeInSecondsToText(const double seconds, char s[25]);

#ifndef _WINRT
/*!
    @function
    @abstract   Sleep the specified number of microseconds.
    @discussion
                Uses an OS-dependent routine. On Unix platforms, usleep; On Windows
                platforms, Sleep; on Mac OS (Classic) platform, Delay.
        @param      microseconds How many microseconds (millionths of a second) to sleep.
    @availability Not available on Windows Runtime (WinRT).
 */
void EdenTime_usleep(const unsigned int microseconds);
#endif // !_WINRT

/*!
    @function
    @abstract   Sleep the specified number of seconds.
        @discussion
                Uses an OS-dependent routine. On Unix platforms, sleep; On Windows
                platforms, Sleep; on Mac OS (Classic) platform, Delay.
    @param      seconds How many seconds to sleep.
 */
void EdenTime_sleep(const unsigned int seconds);

#ifdef __cplusplus
}
#endif
#endif                                  /* !__EdenTime_h__ */