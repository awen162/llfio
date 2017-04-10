/* config.hpp
Configures Boost.AFIO
(C) 2015 Niall Douglas http://www.nedprod.com/
File Created: Dec 2015


Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

//#include <iostream>
//#define BOOST_AFIO_LOG_TO_OSTREAM std::cout
//#define BOOST_AFIO_LOGGING_LEVEL 99

//! \file config.hpp Configures a compiler environment for AFIO header and source code

//! \defgroup config Configuration macros

#define BOOST_AFIO_CONFIGURED

#if !defined(BOOST_AFIO_HEADERS_ONLY) && !defined(BOOST_ALL_DYN_LINK)
//! \brief Whether AFIO is a headers only library. Defaults to 1 unless BOOST_ALL_DYN_LINK is defined. \ingroup config
#define BOOST_AFIO_HEADERS_ONLY 1
#endif

#if !defined(BOOST_AFIO_LOGGING_LEVEL)
#ifdef NDEBUG
#define BOOST_AFIO_LOGGING_LEVEL 2  // error
#else
//! \brief How much detail to log. 0=disabled, 1=fatal, 2=error, 3=warn, 4=info, 5=debug, 6=all.
//! Defaults to error if NDEBUG defined, else info level. \ingroup config
#define BOOST_AFIO_LOGGING_LEVEL 4  // info
#endif
#endif

#if !defined(BOOST_AFIO_LOG_BACKTRACE_LEVELS)
//! \brief Bit mask of which log levels should be stack backtraced
//! which will slow those logs thirty fold or so. Defaults to (1<<1)|(1<<2)|(1<<3) i.e. stack backtrace
//! on fatal, error and warn logs. \ingroup config
#define BOOST_AFIO_LOG_BACKTRACE_LEVELS ((1 << 1) | (1 << 2) | (1 << 3))
#endif

#if !defined(BOOST_AFIO_LOGGING_MEMORY)
#ifdef NDEBUG
#define BOOST_AFIO_LOGGING_MEMORY 4096
#else
//! \brief How much memory to use for the log.
//! Defaults to 4Kb if NDEBUG defined, else 1Mb. \ingroup config
#define BOOST_AFIO_LOGGING_MEMORY (1024 * 1024)
#endif
#endif


#if defined(_WIN32)
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0600
#elif _WIN32_WINNT < 0x0600
#error _WIN32_WINNT must at least be set to Windows Vista for Boost AFIO to work
#endif
#if defined(NTDDI_VERSION) && NTDDI_VERSION < 0x06000000
#error NTDDI_VERSION must at least be set to Windows Vista for Boost AFIO to work
#endif
#endif

// Pull in detection of __MINGW64_VERSION_MAJOR
#ifdef __MINGW32__
#include <_mingw.h>
#endif

// If I'm on winclang, I can't stop the deprecation warnings from MSVCRT unless I do this
#if defined(_MSC_VER) && defined(__clang__)
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include "../boost-lite/include/cpp_feature.h"

#ifndef __cpp_exceptions
#error Boost.AFIO needs C++ exceptions to be turned on
#endif
#ifndef __cpp_alias_templates
#error Boost.AFIO needs template alias support in the compiler
#endif
#ifndef __cpp_variadic_templates
#error Boost.AFIO needs variadic template support in the compiler
#endif
#ifndef __cpp_constexpr
#error Boost.AFIO needs constexpr (C++ 11) support in the compiler
#endif
#ifndef __cpp_init_captures
#error Boost.AFIO needs lambda init captures support in the compiler (C++ 14)
#endif
#ifndef __cpp_attributes
#error Boost.AFIO needs attributes support in the compiler
#endif
#ifndef __cpp_variable_templates
#error Boost.AFIO needs variable template support in the compiler
#endif
#ifndef __cpp_generic_lambdas
#error Boost.AFIO needs generic lambda support in the compiler
#endif
#if(defined(__GNUC__) && !defined(__clang__))
#define BOOST_AFIO_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if BOOST_AFIO_GCC_VERSION < 40900
#error Boost.AFIO needs GCC 4.9 or later as the <regex> shipped in libstdc++ < 4.9 does not work
#endif
#endif


#include "../boost-lite/include/import.h"
#undef BOOST_AFIO_V2_STL11_IMPL
#undef BOOST_AFIO_V2_FILESYSTEM_IMPL
#undef BOOST_AFIO_V2
#undef BOOST_AFIO_V2_NAMESPACE
#undef BOOST_AFIO_V2_NAMESPACE_BEGIN
#undef BOOST_AFIO_V2_NAMESPACE_END

// Default to the C++ 11 STL for atomic, chrono, mutex and thread except on Mingw32
#if(defined(BOOST_AFIO_USE_BOOST_THREAD) && BOOST_AFIO_USE_BOOST_THREAD) || (defined(__MINGW32__) && !defined(__MINGW64__) && !defined(__MINGW64_VERSION_MAJOR))
#if defined(BOOST_OUTCOME_USE_BOOST_THREAD) && BOOST_OUTCOME_USE_BOOST_THREAD != 1
#error You must configure Boost.Outcome and Boost.AFIO to both use Boost.Thread together or both not at all.
#endif
#define BOOST_OUTCOME_USE_BOOST_THREAD 1
#define BOOST_AFIO_V2_STL11_IMPL boost
#ifndef BOOST_THREAD_VERSION
#define BOOST_THREAD_VERSION 3
#endif
#if BOOST_THREAD_VERSION < 3
#error Boost.AFIO requires that Boost.Thread be configured to v3 or later
#endif
#else
#if defined(BOOST_OUTCOME_USE_BOOST_THREAD) && BOOST_OUTCOME_USE_BOOST_THREAD != 0
#error You must configure Boost.Outcome and Boost.AFIO to both use Boost.Thread together or both not at all.
#endif
#define BOOST_OUTCOME_USE_BOOST_THREAD 0
//! \brief The C++ 11 STL to use (std|boost). Defaults to std. \ingroup config
#define BOOST_AFIO_V2_STL11_IMPL std
#ifndef BOOST_AFIO_USE_BOOST_THREAD
//! \brief Whether to use Boost.Thread instead of the C++ 11 STL `std::thread`. Defaults to the C++ 11 STL thread. \ingroup config
#define BOOST_AFIO_USE_BOOST_THREAD 0
#endif
#endif
// Default to the C++ 11 STL if on VS2015 or has <experimental/filesystem>
#ifndef BOOST_AFIO_USE_BOOST_FILESYSTEM
#ifdef __has_include
// clang-format off
#if __has_include(<filesystem>) || __has_include(<experimental/filesystem>)
// clang-format on
#define BOOST_AFIO_USE_BOOST_FILESYSTEM 0
#endif
#endif
#if !defined(BOOST_AFIO_USE_BOOST_FILESYSTEM) && _MSC_VER >= 1900 /* >= VS2015 */
#define BOOST_AFIO_USE_BOOST_FILESYSTEM 0
#endif
#endif
#ifndef BOOST_AFIO_USE_BOOST_FILESYSTEM
//! \brief Whether to use Boost.Filesystem instead of the C++ 17 TS `std::filesystem`.
//! Defaults to the C++ 17 TS filesystem if that is available, else Boost. \ingroup config
#define BOOST_AFIO_USE_BOOST_FILESYSTEM 1
#endif
#if BOOST_AFIO_USE_BOOST_FILESYSTEM
#define BOOST_AFIO_V2_FILESYSTEM_IMPL boost
#define BOOST_AFIO_USE_LEGACY_FILESYSTEM_SEMANTICS 1
#else
#define BOOST_AFIO_V2_FILESYSTEM_IMPL std
#endif
#ifdef BOOST_AFIO_UNSTABLE_VERSION
#include "../revision.hpp"
#define BOOST_AFIO_V2 (boost), (afio), (BOOSTLITE_BIND_NAMESPACE_VERSION(, BOOST_AFIO_NAMESPACE_VERSION, BOOST_AFIO_V2_STL11_IMPL, BOOST_AFIO_V2_FILESYSTEM_IMPL, BOOST_AFIO_PREVIOUS_COMMIT_UNIQUE), inline)
#elif BOOST_AFIO_LATEST_VERSION == 2
#define BOOST_AFIO_V2 (boost), (afio), (BOOSTLITE_BIND_NAMESPACE_VERSION(, BOOST_AFIO_NAMESPACE_VERSION, BOOST_AFIO_V2_STL11_IMPL, BOOST_AFIO_V2_FILESYSTEM_IMPL), inline)
#else
#define BOOST_AFIO_V2 (boost), (afio), (BOOSTLITE_BIND_NAMESPACE_VERSION(, BOOST_AFIO_NAMESPACE_VERSION, BOOST_AFIO_V2_STL11_IMPL, BOOST_AFIO_V2_FILESYSTEM_IMPL))
#endif
/*! \def BOOST_AFIO_V2
\ingroup config
\brief The namespace configuration of this Boost.AFIO v2. Consists of a sequence
of bracketed tokens later fused by the preprocessor into namespace and C++ module names.
*/
#if DOXYGEN_SHOULD_SKIP_THIS
//! The Boost namespace
namespace boost
{
  //! The AFIO namespace
  namespace afio
  {
    //! Inline namespace for this version of AFIO
    inline namespace v2_xxx
    {
      //! Collection of file system based algorithms
      namespace algorithm
      {
      }
      //! YAML databaseable empirical testing of a storage's behaviour
      namespace storage_profile
      {
      }
      //! Utility routines often useful when using AFIO
      namespace utils
      {
      }
    }
  }
}
/*! \brief The namespace of this Boost.AFIO v2 which will be some unknown inline
namespace starting with `v2_` inside the `boost::afio` namespace.
\ingroup config
*/
#define BOOST_AFIO_V2_NAMESPACE boost::afio::v2_xxx
/*! \brief Expands into the appropriate namespace markup to enter the AFIO v2 namespace.
\ingroup config
*/
#define BOOST_AFIO_V2_NAMESPACE_BEGIN                                                                                                                                                                                                                                                                                          \
  namespace boost                                                                                                                                                                                                                                                                                                              \
  {                                                                                                                                                                                                                                                                                                                            \
    namespace afio                                                                                                                                                                                                                                                                                                             \
    {                                                                                                                                                                                                                                                                                                                          \
      inline namespace v2_xxx                                                                                                                                                                                                                                                                                                  \
      {
/*! \brief Expands into the appropriate namespace markup to enter the C++ module
exported AFIO v2 namespace.
\ingroup config
*/
#define BOOST_AFIO_V2_NAMESPACE_EXPORT_BEGIN                                                                                                                                                                                                                                                                                   \
  export namespace boost                                                                                                                                                                                                                                                                                                       \
  {                                                                                                                                                                                                                                                                                                                            \
    namespace afio                                                                                                                                                                                                                                                                                                             \
    {                                                                                                                                                                                                                                                                                                                          \
      inline namespace v2_xxx                                                                                                                                                                                                                                                                                                  \
      {
/*! \brief Expands into the appropriate namespace markup to exit the AFIO v2 namespace.
\ingroup config
*/
#define BOOST_AFIO_V2_NAMESPACE_END                                                                                                                                                                                                                                                                                            \
  }                                                                                                                                                                                                                                                                                                                            \
  }                                                                                                                                                                                                                                                                                                                            \
  }
#elif defined(GENERATING_AFIO_MODULE_INTERFACE)
#define BOOST_AFIO_V2_NAMESPACE BOOSTLITE_BIND_NAMESPACE(BOOST_AFIO_V2)
#define BOOST_AFIO_V2_NAMESPACE_BEGIN BOOSTLITE_BIND_NAMESPACE_BEGIN(BOOST_AFIO_V2)
#define BOOST_AFIO_V2_NAMESPACE_EXPORT_BEGIN BOOSTLITE_BIND_NAMESPACE_EXPORT_BEGIN(BOOST_AFIO_V2)
#define BOOST_AFIO_V2_NAMESPACE_END BOOSTLITE_BIND_NAMESPACE_END(BOOST_AFIO_V2)
#else
#define BOOST_AFIO_V2_NAMESPACE BOOSTLITE_BIND_NAMESPACE(BOOST_AFIO_V2)
#define BOOST_AFIO_V2_NAMESPACE_BEGIN BOOSTLITE_BIND_NAMESPACE_BEGIN(BOOST_AFIO_V2)
#define BOOST_AFIO_V2_NAMESPACE_EXPORT_BEGIN BOOSTLITE_BIND_NAMESPACE_BEGIN(BOOST_AFIO_V2)
#define BOOST_AFIO_V2_NAMESPACE_END BOOSTLITE_BIND_NAMESPACE_END(BOOST_AFIO_V2)
#endif

// From automated matrix generator
#undef BOOST_AFIO_NEED_DEFINE
#undef BOOST_AFIO_NEED_DEFINE_DESCRIPTION
#if !BOOST_AFIO_USE_BOOST_THREAD && !BOOST_AFIO_USE_BOOST_FILESYSTEM
#ifndef BOOST_AFIO_NEED_DEFINE_00
#define BOOST_AFIO_NEED_DEFINE_DESCRIPTION "BOOST_AFIO_USE_BOOST_THREAD=0 BOOST_AFIO_USE_BOOST_FILESYSTEM=0"
#define BOOST_AFIO_NEED_DEFINE_00
#define BOOST_AFIO_NEED_DEFINE 1
#endif
#elif BOOST_AFIO_USE_BOOST_THREAD && !BOOST_AFIO_USE_BOOST_FILESYSTEM
#ifndef BOOST_AFIO_NEED_DEFINE_10
#define BOOST_AFIO_NEED_DEFINE_DESCRIPTION "BOOST_AFIO_USE_BOOST_THREAD=1 BOOST_AFIO_USE_BOOST_FILESYSTEM=0"
#define BOOST_AFIO_NEED_DEFINE_10
#define BOOST_AFIO_NEED_DEFINE 1
#endif
#elif !BOOST_AFIO_USE_BOOST_THREAD && BOOST_AFIO_USE_BOOST_FILESYSTEM
#ifndef BOOST_AFIO_NEED_DEFINE_01
#define BOOST_AFIO_NEED_DEFINE_DESCRIPTION "BOOST_AFIO_USE_BOOST_THREAD=0 BOOST_AFIO_USE_BOOST_FILESYSTEM=1"
#define BOOST_AFIO_NEED_DEFINE_01
#define BOOST_AFIO_NEED_DEFINE 1
#endif
#elif BOOST_AFIO_USE_BOOST_THREAD && BOOST_AFIO_USE_BOOST_FILESYSTEM
#ifndef BOOST_AFIO_NEED_DEFINE_11
#define BOOST_AFIO_NEED_DEFINE_DESCRIPTION "BOOST_AFIO_USE_BOOST_THREAD=1 BOOST_AFIO_USE_BOOST_FILESYSTEM=1"
#define BOOST_AFIO_NEED_DEFINE_11
#define BOOST_AFIO_NEED_DEFINE 1
#endif
#endif

#ifdef BOOST_AFIO_NEED_DEFINE
#undef BOOST_AFIO_AFIO_H

#define BOOST_STL11_ATOMIC_MAP_NO_ATOMIC_CHAR32_T  // missing VS14
#define BOOST_STL11_ATOMIC_MAP_NO_ATOMIC_CHAR16_T  // missing VS14
// Match Dinkumware's TR2 implementation
#define BOOST_STL1z_FILESYSTEM_MAP_NO_SYMLINK_OPTION
#define BOOST_STL1z_FILESYSTEM_MAP_NO_COPY_OPTION
#define BOOST_STL1z_FILESYSTEM_MAP_NO_CHANGE_EXTENSION
#define BOOST_STL1z_FILESYSTEM_MAP_NO_WRECURSIVE_DIRECTORY_ITERATOR
#define BOOST_STL1z_FILESYSTEM_MAP_NO_EXTENSION
#define BOOST_STL1z_FILESYSTEM_MAP_NO_TYPE_PRESENT
#define BOOST_STL1z_FILESYSTEM_MAP_NO_PORTABLE_FILE_NAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_PORTABLE_DIRECTORY_NAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_PORTABLE_POSIX_NAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_LEXICOGRAPHICAL_COMPARE
#define BOOST_STL1z_FILESYSTEM_MAP_NO_WINDOWS_NAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_PORTABLE_NAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_BASENAME
#define BOOST_STL1z_FILESYSTEM_MAP_NO_COMPLETE
#define BOOST_STL1z_FILESYSTEM_MAP_NO_IS_REGULAR
#define BOOST_STL1z_FILESYSTEM_MAP_NO_INITIAL_PATH
#define BOOST_STL1z_FILESYSTEM_MAP_NO_PERMISSIONS_PRESENT
#define BOOST_STL1z_FILESYSTEM_MAP_NO_CODECVT_ERROR_CATEGORY
#define BOOST_STL1z_FILESYSTEM_MAP_NO_WPATH
#define BOOST_STL1z_FILESYSTEM_MAP_NO_SYMBOLIC_LINK_EXISTS
#define BOOST_STL1z_FILESYSTEM_MAP_NO_COPY_DIRECTORY
#define BOOST_STL1z_FILESYSTEM_MAP_NO_NATIVE
#define BOOST_STL1z_FILESYSTEM_MAP_NO_UNIQUE_PATH

#include "../boost-lite/include/bind/stl11/std/atomic"
BOOST_AFIO_V2_NAMESPACE_BEGIN
namespace stl11
{
  using namespace boost_lite::bind::std::atomic;
}
BOOST_AFIO_V2_NAMESPACE_END
#if BOOST_OUTCOME_USE_BOOST_THREAD
#include "../boost-lite/include/bind/stl11/boost/chrono"
#include "../boost-lite/include/bind/stl11/boost/mutex"
#include "../boost-lite/include/bind/stl11/boost/ratio"
#include "../boost-lite/include/bind/stl11/boost/thread"
BOOST_AFIO_V2_NAMESPACE_BEGIN
namespace stl11
{
  namespace chrono = boost_lite::bind::boost::chrono;
  using namespace boost_lite::bind::boost::mutex;
  using namespace boost_lite::bind::boost::ratio;
  using namespace boost_lite::bind::boost::thread;
}
#else
#include "../boost-lite/include/bind/stl11/std/chrono"
#include "../boost-lite/include/bind/stl11/std/mutex"
#include "../boost-lite/include/bind/stl11/std/ratio"
#include "../boost-lite/include/bind/stl11/std/thread"
BOOST_AFIO_V2_NAMESPACE_BEGIN
namespace stl11
{
  namespace chrono = boost_lite::bind::std::chrono;
  using namespace boost_lite::bind::std::mutex;
  using namespace boost_lite::bind::std::ratio;
  using namespace boost_lite::bind::std::thread;
}
BOOST_AFIO_V2_NAMESPACE_END
#endif
#if BOOST_AFIO_USE_BOOST_FILESYSTEM
#include "../boost-lite/include/bind/stl1z/boost/filesystem"
BOOST_AFIO_V2_NAMESPACE_BEGIN
namespace stl1z
{
  namespace filesystem = boost_lite::bind::boost::filesystem;
}
BOOST_AFIO_V2_NAMESPACE_END
#else
#include "../boost-lite/include/bind/stl1z/std/filesystem"
BOOST_AFIO_V2_NAMESPACE_BEGIN
namespace stl1z
{
  namespace filesystem = boost_lite::bind::std::filesystem;
}
BOOST_AFIO_V2_NAMESPACE_END
#endif


// Bring in the Boost-lite macros
#include "../boost-lite/include/config.hpp"

// Configure BOOST_AFIO_DECL
#if(defined(BOOST_AFIO_DYN_LINK) || defined(BOOST_ALL_DYN_LINK)) && !defined(BOOST_AFIO_STATIC_LINK)

#if defined(BOOST_AFIO_SOURCE)
#define BOOST_AFIO_DECL BOOSTLITE_SYMBOL_EXPORT
#define BOOST_AFIO_BUILD_DLL
#else
#define BOOST_AFIO_DECL BOOSTLIE_SYMBOL_IMPORT
#endif
#else
#define BOOST_AFIO_DECL
#endif  // building a shared library


// Bring in bitfields
#include "../boost-lite/include/bitfield.hpp"
// Bring in scoped undo
#include "../boost-lite/include/scoped_undo.hpp"
BOOST_AFIO_V2_NAMESPACE_BEGIN
using BOOSTLITE_NAMESPACE::scoped_undo::undoer;
BOOST_AFIO_V2_NAMESPACE_END


// Bring in a span implementation
#include "../boost-lite/include/span.hpp"
BOOST_AFIO_V2_NAMESPACE_BEGIN
using namespace boost_lite::span;
BOOST_AFIO_V2_NAMESPACE_END


#if BOOST_AFIO_LOGGING_LEVEL
#include "../boost-lite/include/ringbuffer_log.hpp"
#include "../boost-lite/include/utils/thread.hpp"

/*! \todo TODO FIXME Replace in-memory log with memory map file backed log.
*/
BOOST_AFIO_V2_NAMESPACE_BEGIN
//! The log used by AFIO
inline BOOST_AFIO_DECL boost_lite::ringbuffer_log::simple_ringbuffer_log<BOOST_AFIO_LOGGING_MEMORY> &log() noexcept
{
  static boost_lite::ringbuffer_log::simple_ringbuffer_log<BOOST_AFIO_LOGGING_MEMORY> _log(static_cast<boost_lite::ringbuffer_log::level>(BOOST_AFIO_LOGGING_LEVEL));
#ifdef BOOST_AFIO_LOG_TO_OSTREAM
  _log.immediate(&BOOST_AFIO_LOG_TO_OSTREAM);
#endif
  return _log;
}
inline void record_error_into_afio_log(boost_lite::ringbuffer_log::level _level, const char *_message, unsigned _code1, unsigned _code2, const char *_function, unsigned lineno)
{
  // Here is a VERY useful place to breakpoint!
  log().emplace_back(_level, _message, _code1, _code2, _function, lineno);
}
BOOST_AFIO_V2_NAMESPACE_END
#endif

#ifndef BOOST_AFIO_LOG_FATAL_TO_CERR
#include <stdio.h>
#define BOOST_AFIO_LOG_FATAL_TO_CERR(expr)                                                                                                                                                                                                                                                                                     \
  fprintf(stderr, "%s\n", (expr));                                                                                                                                                                                                                                                                                             \
  fflush(stderr)
#endif

#if BOOST_AFIO_LOGGING_LEVEL >= 1
#define BOOST_AFIO_LOG_FATAL(inst, message)                                                                                                                                                                                                                                                                                    \
  {                                                                                                                                                                                                                                                                                                                            \
    BOOST_AFIO_V2_NAMESPACE::log().emplace_back(boost_lite::ringbuffer_log::level::fatal, (message), (unsigned) (uintptr_t)(inst), boost_lite::utils::thread::this_thread_id(), (BOOST_AFIO_LOG_BACKTRACE_LEVELS & (1 << 1)) ? nullptr : __func__, __LINE__);                                                                  \
    BOOST_AFIO_LOG_FATAL_TO_CERR(message);                                                                                                                                                                                                                                                                                     \
  }
#else
#define BOOST_AFIO_LOG_FATAL(inst, message) BOOST_AFIO_LOG_FATAL_TO_CERR(message)
#endif
#if BOOST_AFIO_LOGGING_LEVEL >= 2
#define BOOST_AFIO_LOG_ERROR(inst, message) BOOST_AFIO_V2_NAMESPACE::log().emplace_back(boost_lite::ringbuffer_log::level::error, (message), (unsigned) (uintptr_t)(inst), boost_lite::utils::thread::this_thread_id(), (BOOST_AFIO_LOG_BACKTRACE_LEVELS & (1 << 2)) ? nullptr : __func__, __LINE__)
// Intercept when Outcome creates an error_code_extended and log it to our log too
#ifndef BOOST_OUTCOME_ERROR_CODE_EXTENDED_CREATION_HOOK
#define BOOST_OUTCOME_ERROR_CODE_EXTENDED_CREATION_HOOK                                                                                                                                                                                                                                                                        \
  if(*this)                                                                                                                                                                                                                                                                                                                    \
  BOOST_AFIO_V2_NAMESPACE::record_error_into_afio_log(boost_lite::ringbuffer_log::level::error, this->message().c_str(), this->value(), (unsigned) this->_unique_id, (BOOST_AFIO_LOG_BACKTRACE_LEVELS & (1 << 2)) ? nullptr : __func__, __LINE__)
#endif
#else
#define BOOST_AFIO_LOG_ERROR(inst, message)
#endif
#if BOOST_AFIO_LOGGING_LEVEL >= 3
#define BOOST_AFIO_LOG_WARN(inst, message) BOOST_AFIO_V2_NAMESPACE::log().emplace_back(boost_lite::ringbuffer_log::level::warn, (message), (unsigned) (uintptr_t)(inst), boost_lite::utils::thread::this_thread_id(), (BOOST_AFIO_LOG_BACKTRACE_LEVELS & (1 << 3)) ? nullptr : __func__, __LINE__)
#else
#define BOOST_AFIO_LOG_WARN(inst, message)
#endif

// Need Outcome in play before I can define logging level 4
#include "../outcome/include/boost/outcome.hpp"
BOOST_AFIO_V2_NAMESPACE_BEGIN
// We are so heavily tied into Outcome we just import it wholesale into our namespace
using namespace BOOST_OUTCOME_V1_NAMESPACE;
// Force these to the same overloading precedence as if they were defined in the AFIO namespace
using BOOST_OUTCOME_V1_NAMESPACE::outcome;
using BOOST_OUTCOME_V1_NAMESPACE::make_errored_result;
using BOOST_OUTCOME_V1_NAMESPACE::make_errored_outcome;
namespace stl11
{
  using BOOST_OUTCOME_V1_NAMESPACE::stl11::errc;
}
#if DOXYGEN_SHOULD_SKIP_THIS
/*! \brief Please see https://ned14.github.io/boost.outcome/classboost_1_1outcome_1_1v1__xxx_1_1basic__monad.html
*/
template <class T> using result = boost::outcome::result<T>;
/*! \brief Please see https://ned14.github.io/boost.outcome/classboost_1_1outcome_1_1v1__xxx_1_1basic__monad.html
*/
template <class T> using outcome = boost::outcome::outcome<T>;
#endif
BOOST_AFIO_V2_NAMESPACE_END


#if BOOST_AFIO_LOGGING_LEVEL >= 4
#define BOOST_AFIO_LOG_INFO(inst, message) BOOST_AFIO_V2_NAMESPACE::log().emplace_back(boost_lite::ringbuffer_log::level::info, (message), (unsigned) (uintptr_t)(inst), boost_lite::utils::thread::this_thread_id(), (BOOST_AFIO_LOG_BACKTRACE_LEVELS & (1 << 4)) ? nullptr : __func__, __LINE__)

// Need to expand out our namespace into a string
#define BOOST_AFIO_LOG_STRINGIFY9(s) #s "::"
#define BOOST_AFIO_LOG_STRINGIFY8(s) BOOST_AFIO_LOG_STRINGIFY9(s)
#define BOOST_AFIO_LOG_STRINGIFY7(s) BOOST_AFIO_LOG_STRINGIFY8(s)
#define BOOST_AFIO_LOG_STRINGIFY6(s) BOOST_AFIO_LOG_STRINGIFY7(s)
#define BOOST_AFIO_LOG_STRINGIFY5(s) BOOST_AFIO_LOG_STRINGIFY6(s)
#define BOOST_AFIO_LOG_STRINGIFY4(s) BOOST_AFIO_LOG_STRINGIFY5(s)
#define BOOST_AFIO_LOG_STRINGIFY3(s) BOOST_AFIO_LOG_STRINGIFY4(s)
#define BOOST_AFIO_LOG_STRINGIFY2(s) BOOST_AFIO_LOG_STRINGIFY3(s)
#define BOOST_AFIO_LOG_STRINGIFY(s) BOOST_AFIO_LOG_STRINGIFY2(s)
BOOST_AFIO_V2_NAMESPACE_BEGIN
//! Returns the AFIO namespace as a string
inline span<char> afio_namespace_string()
{
  static char buffer[64];
  static size_t length;
  if(length)
    return span<char>(buffer, length);
  const char *src = BOOST_AFIO_LOG_STRINGIFY(BOOST_AFIO_V2_NAMESPACE);
  char *bufferp = buffer;
  for(; *src && (bufferp - buffer) < (ptrdiff_t) sizeof(buffer); src++)
  {
    if(*src != ' ')
      *bufferp++ = *src;
  }
  *bufferp = 0;
  length = bufferp - buffer;
  return span<char>(buffer, length);
}
//! Returns the Outcome namespace as a string
inline span<char> outcome_namespace_string()
{
  static char buffer[64];
  static size_t length;
  if(length)
    return span<char>(buffer, length);
  const char *src = BOOST_AFIO_LOG_STRINGIFY(BOOST_OUTCOME_V1_NAMESPACE);
  char *bufferp = buffer;
  for(; *src && (bufferp - buffer) < (ptrdiff_t) sizeof(buffer); src++)
  {
    if(*src != ' ')
      *bufferp++ = *src;
  }
  *bufferp = 0;
  length = bufferp - buffer;
  return span<char>(buffer, length);
}
//! Strips a __PRETTY_FUNCTION__ of all instances of boost::afio:: and boost::outcome::
inline void strip_pretty_function(char *out, size_t bytes, const char *in)
{
  const span<char> remove1 = afio_namespace_string();
  const span<char> remove2 = outcome_namespace_string();
  for(--bytes; bytes && *in; --bytes)
  {
    if(!memcmp(in, remove1.data(), remove1.size()))
      in += remove1.size();
    if(!memcmp(in, remove2.data(), remove2.size()))
      in += remove2.size();
    *out++ = *in++;
  }
  *out = 0;
}
BOOST_AFIO_V2_NAMESPACE_END
#ifdef _MSC_VER
#define BOOST_AFIO_LOG_FUNCTION_CALL(inst)                                                                                                                                                                                                                                                                                     \
  {                                                                                                                                                                                                                                                                                                                            \
    char buffer[256];                                                                                                                                                                                                                                                                                                          \
    BOOST_AFIO_V2_NAMESPACE::strip_pretty_function(buffer, sizeof(buffer), __FUNCSIG__);                                                                                                                                                                                                                                       \
    BOOST_AFIO_LOG_INFO(inst, buffer);                                                                                                                                                                                                                                                                                         \
  }
#else
#define BOOST_AFIO_LOG_FUNCTION_CALL(inst)                                                                                                                                                                                                                                                                                     \
  {                                                                                                                                                                                                                                                                                                                            \
    char buffer[256];                                                                                                                                                                                                                                                                                                          \
    BOOST_AFIO_V2_NAMESPACE::strip_pretty_function(buffer, sizeof(buffer), __PRETTY_FUNCTION__);                                                                                                                                                                                                                               \
    BOOST_AFIO_LOG_INFO(inst, buffer);                                                                                                                                                                                                                                                                                         \
  }
#endif
#else
#define BOOST_AFIO_LOG_INFO(inst, message)
#define BOOST_AFIO_LOG_FUNCTION_CALL(inst)
#endif
#if BOOST_AFIO_LOGGING_LEVEL >= 5
#define BOOST_AFIO_LOG_DEBUG(inst, message) BOOST_AFIO_V2_NAMESPACE::log().emplace_back(boost_lite::ringbuffer_log::level::debug, (message), (unsigned) (uintptr_t)(inst), boost_lite::utils::thread::this_thread_id(), (BOOST_AFIO_LOG_BACKTRACE_LEVELS & (1 << 5)) ? nullptr : __func__, __LINE__)
#else
#define BOOST_AFIO_LOG_DEBUG(inst, message)
#endif
#if BOOST_AFIO_LOGGING_LEVEL >= 6
#define BOOST_AFIO_LOG_ALL(inst, message) BOOST_AFIO_V2_NAMESPACE::log().emplace_back(boost_lite::ringbuffer_log::level::all, (message), (unsigned) (uintptr_t)(inst), boost_lite::utils::thread::this_thread_id(), (BOOST_AFIO_LOG_BACKTRACE_LEVELS & (1 << 6)) ? nullptr : __func__, __LINE__)
#else
#define BOOST_AFIO_LOG_ALL(inst, message)
#endif

#include <time.h>  // for struct timespec

BOOST_AFIO_V2_NAMESPACE_BEGIN

// The C++ 11 runtime is much better at exception state than Boost so no choice here
using std::make_exception_ptr;
using std::error_code;
using std::generic_category;
using std::system_category;
using std::system_error;

// Too darn useful
using std::to_string;
// Used to send the last 190 chars instead of the first 190 chars to extended_error_code
using boost_lite::ringbuffer_log::last190;
namespace detail
{
  // A move only capable lightweight std::function, as std::function can't handle move only callables
  template <class F> class function_ptr;
  template <class R, class... Args> class function_ptr<R(Args...)>
  {
    struct function_ptr_storage
    {
      virtual ~function_ptr_storage() {}
      virtual R operator()(Args &&... args) = 0;
    };
    template <class U> struct function_ptr_storage_impl : public function_ptr_storage
    {
      U c;
      template <class... Args2>
      constexpr function_ptr_storage_impl(Args2 &&... args)
        : c(std::forward<Args2>(args)...)
      {
      }
      virtual R operator()(Args &&... args) override final { return c(std::move(args)...); }
    };
    function_ptr_storage *ptr;
    template <class U> struct emplace_t
    {
    };
    template <class U, class V> friend inline function_ptr<U> make_function_ptr(V &&f);
    template <class U>
    explicit function_ptr(std::nullptr_t, U &&f)
      : ptr(new function_ptr_storage_impl<typename std::decay<U>::type>(std::forward<U>(f)))
    {
    }
    template <class R_, class U, class... Args2> friend inline function_ptr<R_> emplace_function_ptr(Args2 &&... args);
    template <class U, class... Args2>
    explicit function_ptr(emplace_t<U>, Args2 &&... args)
      : ptr(new function_ptr_storage_impl<U>(std::forward<Args2>(args)...))
    {
    }

  public:
    constexpr function_ptr() noexcept : ptr(nullptr) {}
    constexpr function_ptr(function_ptr_storage *p) noexcept : ptr(p) {}
    BOOSTLITE_CONSTEXPR function_ptr(function_ptr &&o) noexcept : ptr(o.ptr) { o.ptr = nullptr; }
    function_ptr &operator=(function_ptr &&o)
    {
      delete ptr;
      ptr = o.ptr;
      o.ptr = nullptr;
      return *this;
    }
    function_ptr(const function_ptr &) = delete;
    function_ptr &operator=(const function_ptr &) = delete;
    ~function_ptr() { delete ptr; }
    explicit constexpr operator bool() const noexcept { return !!ptr; }
    BOOSTLITE_CONSTEXPR R operator()(Args... args) const { return (*ptr)(std::move(args)...); }
    BOOSTLITE_CONSTEXPR function_ptr_storage *get() noexcept { return ptr; }
    BOOSTLITE_CONSTEXPR void reset(function_ptr_storage *p = nullptr) noexcept
    {
      delete ptr;
      ptr = p;
    }
    BOOSTLITE_CONSTEXPR function_ptr_storage *release() noexcept
    {
      auto p = ptr;
      ptr = nullptr;
      return p;
    }
  };
  template <class R, class U> inline function_ptr<R> make_function_ptr(U &&f) { return function_ptr<R>(nullptr, std::forward<U>(f)); }
  template <class R, class U, class... Args> inline function_ptr<R> emplace_function_ptr(Args &&... args) { return function_ptr<R>(typename function_ptr<R>::template emplace_t<U>(), std::forward<Args>(args)...); }
}

// Temporary in lieu of full fat afio::path
/* \todo Full fat afio::path needs to be able to variant a win32 path
and a nt kernel path.
\todo A variant of an open handle as base and a relative path fragment
from there is also needed, though I have no idea how to manage lifetime
for such a thing.
\todo It would make a great deal of sense if afio::path were
a linked list of filesystem::path fragments as things like directory
hierarchy walks do a lot of leaf node splitting which for a 32k path
means a ton load of memory copying. Something like LLVM's list of
string fragments would be far faster - look for an existing implementation
before writing our own! One of those path fragments could variant onto
an open handle to solve the earlier issue.
*/
using fixme_path = stl1z::filesystem::path;

// Native handle support
namespace win
{
  using handle = void *;
  using dword = unsigned long;
}

BOOST_AFIO_V2_NAMESPACE_END


#if 0
///////////////////////////////////////////////////////////////////////////////
//  Auto library naming
#if !defined(BOOST_AFIO_SOURCE) && !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_AFIO_NO_LIB) && !AFIO_STANDALONE && !BOOST_AFIO_HEADERS_ONLY

#define BOOST_LIB_NAME boost_afio

// tell the auto-link code to select a dll when required:
#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_AFIO_DYN_LINK)
#define BOOST_DYN_LINK
#endif

#include <boost/config/auto_link.hpp>

#endif  // auto-linking disabled
#endif

//#define BOOST_THREAD_VERSION 4
//#define BOOST_THREAD_PROVIDES_VARIADIC_THREAD
//#define BOOST_THREAD_DONT_PROVIDE_FUTURE
//#define BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#if BOOST_AFIO_HEADERS_ONLY == 1 && !defined(BOOST_AFIO_SOURCE)
/*! \brief Expands into the appropriate markup to declare an `extern`
function exported from the AFIO DLL if not building headers only.
\ingroup config
*/
#define BOOST_AFIO_HEADERS_ONLY_FUNC_SPEC inline
/*! \brief Expands into the appropriate markup to declare a class member
function exported from the AFIO DLL if not building headers only.
\ingroup config
*/
#define BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC inline
/*! \brief Expands into the appropriate markup to declare a virtual class member
function exported from the AFIO DLL if not building headers only.
\ingroup config
*/
#define BOOST_AFIO_HEADERS_ONLY_VIRTUAL_SPEC inline virtual
#else
#define BOOST_AFIO_HEADERS_ONLY_FUNC_SPEC extern BOOST_AFIO_DECL
#define BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC
#define BOOST_AFIO_HEADERS_ONLY_VIRTUAL_SPEC virtual
#endif

#endif  // BOOST_AFIO_NEED_DEFINE
