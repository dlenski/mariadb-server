/*****************************************************************************

Copyright (c) 2021, MariaDB Corporation.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA

*****************************************************************************/

#pragma once

#if !defined __s390x__ && (defined __zarch__ || defined __SYSC_ZARCH__)
# define __s390x__
#endif

#if defined __powerpc64__ || defined __s390x__ || defined __s390__
#elif defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
#elif defined __GNUC__ && (defined __i386__ || defined __x86_64__)
# if __GNUC__ >= 8
# elif defined __clang__ && (__clang_major__ > 3 || (__clang_major__ == 3 && __clang__minor >= 8))
# else
#  define NO_ELISION
# endif
#else /* Transactional memory has not been implemented for this ISA */
# define NO_ELISION
#endif

#ifdef NO_ELISION
constexpr bool have_transactional_memory= false;
# define TRANSACTIONAL_TARGET /* nothing */
#else
# if defined __i386__||defined __x86_64__||defined _M_IX86||defined _M_X64
extern bool have_transactional_memory;
bool transactional_lock_enabled();

#  include <immintrin.h>
#  ifdef __GNUC__
#   define TRANSACTIONAL_TARGET __attribute__((target("rtm")))
#   define TRANSACTIONAL_INLINE __attribute__((target("rtm"),always_inline))
#  else
#   define TRANSACTIONAL_TARGET /* nothing */
#   define TRANSACTIONAL_INLINE /* nothing */
#  endif

TRANSACTIONAL_INLINE static inline bool xbegin()
{
  return have_transactional_memory && _xbegin() == _XBEGIN_STARTED;
}

template<unsigned char i>
TRANSACTIONAL_INLINE static inline void xabort() { _xabort(i); }

TRANSACTIONAL_INLINE static inline void xend() { _xend(); }
# elif defined __powerpc64__ || defined __s390x__ || defined __s390__
constexpr bool have_transactional_memory= true;
static inline bool transactional_lock_enabled() { return true; }
#  include <htmxlintrin.h>
#  define TRANSACTIONAL_TARGET
#  define TRANSACTIONAL_INLINE

static inline bool xbegin() { return __TM_begin() == _HTM_TBEGIN_STARTED; }

template<unsigned char i> static inline void xabort() { __TM_abort(); }

static inline void xend() { __TM_end(); }
# endif
#endif

template<class mutex>
class transactional_lock_guard
{
  mutex &m;

public:
  TRANSACTIONAL_INLINE transactional_lock_guard(mutex &m) : m(m)
  {
#ifndef NO_ELISION
    if (xbegin())
    {
      if (was_elided())
        return;
      xabort<0xff>();
    }
#endif
    m.lock();
  }
  transactional_lock_guard(const transactional_lock_guard &)= delete;
  TRANSACTIONAL_INLINE ~transactional_lock_guard()
  {
#ifndef NO_ELISION
    if (was_elided()) xend(); else
#endif
    m.unlock();
  }

#ifndef NO_ELISION
  bool was_elided() const noexcept { return !m.is_locked_or_waiting(); }
#else
  bool was_elided() const noexcept { return false; }
#endif
};

template<class mutex>
class transactional_shared_lock_guard
{
  mutex &m;
#ifndef NO_ELISION
  bool elided;
#else
  static constexpr bool elided= false;
#endif

public:
  TRANSACTIONAL_INLINE transactional_shared_lock_guard(mutex &m) : m(m)
  {
#ifndef NO_ELISION
    if (xbegin())
    {
      if (!m.is_locked())
      {
        elided= true;
        return;
      }
      xabort<0xff>();
    }
    elided= false;
#endif
    m.lock_shared();
  }
  transactional_shared_lock_guard(const transactional_shared_lock_guard &)=
    delete;
  TRANSACTIONAL_INLINE ~transactional_shared_lock_guard()
  {
#ifndef NO_ELISION
    if (was_elided()) xend(); else
#endif
    m.unlock_shared();
  }

  bool was_elided() const noexcept { return elided; }
};
