/* async_file_handle.hpp
An async handle to a file
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

#include "file_handle.hpp"
#include "io_service.hpp"

//! \file async_file_handle.hpp Provides async_file_handle

#ifndef BOOST_AFIO_ASYNC_FILE_HANDLE_H
#define BOOST_AFIO_ASYNC_FILE_HANDLE_H

BOOST_AFIO_V2_NAMESPACE_EXPORT_BEGIN

/*! An asynchronous handle to an open something
*/
class BOOST_AFIO_DECL async_file_handle : public file_handle
{
  friend class io_service;
public:
  using dev_t = file_handle::dev_t;
  using ino_t = file_handle::ino_t;
  using path_type = io_handle::path_type;
  using extent_type = io_handle::extent_type;
  using size_type = io_handle::size_type;
  using mode = io_handle::mode;
  using creation = io_handle::creation;
  using caching = io_handle::caching;
  using flag = io_handle::flag;
  using buffer_type = io_handle::buffer_type;
  using const_buffer_type = io_handle::const_buffer_type;
  using buffers_type = io_handle::buffers_type;
  using const_buffers_type = io_handle::const_buffers_type;
  template <class T> using io_request = io_handle::io_request<T>;
  template <class T> using io_result = io_handle::io_result<T>;

protected:
  // Do NOT declare variables here, put them into file_handle to preserve up-conversion

public:
  //! Default constructor
  async_file_handle() = default;

  //! Construct a handle from a supplied native handle
  async_file_handle(io_service *service, native_handle_type h, dev_t devid, ino_t inode, path_type path, caching caching = caching::none, flag flags = flag::none)
      : file_handle(std::move(h), devid, inode, std::move(path), std::move(caching), std::move(flags))
  {
    this->_service = service;
  }
  //! Implicit move construction of async_file_handle permitted
  async_file_handle(async_file_handle &&o) noexcept = default;
  //! Explicit conversion from file_handle permitted
  explicit async_file_handle(file_handle &&o) noexcept : file_handle(std::move(o)) {}
  //! Explicit conversion from handle and io_handle permitted
  explicit async_file_handle(handle &&o, io_service *service, path_type path, dev_t devid, ino_t inode) noexcept : file_handle(std::move(o), std::move(path), devid, inode) { this->_service = service; }
  //! Move assignment of async_file_handle permitted
  async_file_handle &operator=(async_file_handle &&o) noexcept
  {
    this->~async_file_handle();
    new(this) async_file_handle(std::move(o));
    return *this;
  }
  //! Swap with another instance
  void swap(async_file_handle &o) noexcept
  {
    async_file_handle temp(std::move(*this));
    *this = std::move(o);
    o = std::move(temp);
  }

  /*! Create an async file handle opening access to a file on path
  using the given io_service.

  \errors Any of the values POSIX open() or CreateFile() can return.
  */
  //[[bindlib::make_free]]
  static BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC result<async_file_handle> async_file(io_service &service, path_type _path, mode _mode = mode::read, creation _creation = creation::open_existing, caching _caching = caching::all, flag flags = flag::none) noexcept
  {
    // Open it overlapped, otherwise no difference.
    BOOST_OUTCOME_TRY(v, file_handle::file(std::move(_path), std::move(_mode), std::move(_creation), std::move(_caching), flags | flag::overlapped));
    async_file_handle ret(std::move(v));
    ret._service = &service;
    return std::move(ret);
  }

  /*! Create an async file handle creating a randomly named file on a path.
  The file is opened exclusively with `creation::only_if_not_exist` so it
  will never collide with nor overwrite any existing file. Note also
  that caching defaults to temporary which hints to the OS to only
  flush changes to physical storage as lately as possible.

  \errors Any of the values POSIX open() or CreateFile() can return.
  */
  //[[bindlib::make_free]]
  static inline result<async_file_handle> async_random_file(io_service &service, path_type dirpath, mode _mode = mode::write, caching _caching = caching::temporary, flag flags = flag::none) noexcept
  {
    try
    {
      result<async_file_handle> ret;
      do
      {
        auto randomname = utils::random_string(32);
        randomname.append(".random");
        ret = async_file(service, dirpath / randomname, _mode, creation::only_if_not_exist, _caching, flags);
        if(!ret && ret.get_error().value() != EEXIST)
          return ret;
      } while(!ret);
      return ret;
    }
    BOOST_OUTCOME_CATCH_ALL_EXCEPTION_TO_RESULT
  }
  /*! Create an async file handle creating the named file on some path which
  the OS declares to be suitable for temporary files. Most OSs are
  very lazy about flushing changes made to these temporary files.
  Note the default flags are to have the newly created file deleted
  on first handle close.
  Note also that an empty name is equivalent to calling
  `async_random_file(fixme_temporary_files_directory())` and the creation
  parameter is ignored.

  \note If the temporary file you are creating is not going to have its
  path sent to another process for usage, this is the WRONG function
  to use. Use `temp_inode()` instead, it is far more secure.

  \errors Any of the values POSIX open() or CreateFile() can return.
  */
  //[[bindlib::make_free]]
  static inline result<async_file_handle> async_temp_file(io_service &service, path_type name = path_type(), mode _mode = mode::write, creation _creation = creation::if_needed, caching _caching = caching::temporary, flag flags = flag::unlink_on_close) noexcept
  {
    return name.empty() ? async_random_file(service, fixme_temporary_files_directory(), _mode, _caching, flags) : async_file(service, fixme_temporary_files_directory() / name, _mode, _creation, _caching, flags);
  }
  /*! \em Securely create an async file handle creating a temporary anonymous inode in
  the filesystem referred to by \em dirpath. The inode created has
  no name nor accessible path on the filing system and ceases to
  exist as soon as the last handle is closed, making it ideal for use as
  a temporary file where other processes do not need to have access
  to its contents via some path on the filing system (a classic use case
  is for backing shared memory maps).

  \errors Any of the values POSIX open() or CreateFile() can return.
  */
  //[[bindlib::make_free]]
  static BOOST_AFIO_HEADERS_ONLY_MEMFUNC_SPEC result<async_file_handle> async_temp_inode(io_service &service, path_type dirpath = fixme_temporary_files_directory(), mode _mode = mode::write, flag flags = flag::none) noexcept
  {
    // Open it overlapped, otherwise no difference.
    BOOST_OUTCOME_TRY(v, file_handle::temp_inode(std::move(dirpath), std::move(_mode), flags | flag::overlapped));
    async_file_handle ret(std::move(v));
    ret._service = &service;
    return std::move(ret);
  }

  /*! Clone this handle to a different io_service (copy constructor is disabled to avoid accidental copying)

  \errors Any of the values POSIX dup() or DuplicateHandle() can return.
  */
  BOOST_AFIO_HEADERS_ONLY_VIRTUAL_SPEC result<async_file_handle> clone(io_service &service) const noexcept;
  using file_handle::clone;

#if DOXYGEN_SHOULD_SKIP_THIS
private:
#else
protected:
#endif
  using shared_size_type = size_type;
  enum class operation_t
  {
    read,
    write
  };
  // Holds state for an i/o in progress. Will be subclassed with platform specific state and how to implement completion.
  // Note this is allocated using malloc not new to avoid memory zeroing, and therefore it has a custom deleter.
  struct _erased_io_state_type
  {
    async_file_handle *parent;
    operation_t operation;
    size_t items;
    shared_size_type items_to_go;
    constexpr _erased_io_state_type(async_file_handle *_parent, operation_t _operation, size_t _items)
        : parent(_parent)
        , operation(_operation)
        , items(_items)
        , items_to_go(0)
    {
    }
    /*
    For Windows:
      - errcode: GetLastError() code
      - bytes_transferred: obvious
      - internal_state: LPOVERLAPPED for this op

    For POSIX AIO:
      - errcode: errno code
      - bytes_transferred: return from aio_return(), usually bytes transferred
      - internal_state: address of pointer to struct aiocb in io_service's _aiocbsv
    */
    virtual void operator()(long errcode, long bytes_transferred, void *internal_state) noexcept = 0;
    BOOST_AFIO_HEADERS_ONLY_VIRTUAL_SPEC ~_erased_io_state_type()
    {
      // i/o still pending is very bad, this should never happen
      assert(!items_to_go);
      if(items_to_go)
      {
        BOOST_AFIO_LOG_FATAL(parent->native_handle().h, "FATAL: io_state destructed while i/o still in flight, the derived class should never allow this.");
        abort();
      }
    }
  };
  // State for an i/o in progress, but with the per operation typing
  template <class CompletionRoutine, class BuffersType> struct _io_state_type : public _erased_io_state_type
  {
    io_result<BuffersType> result;
    CompletionRoutine completion;
    constexpr _io_state_type(async_file_handle *_parent, operation_t _operation, CompletionRoutine &&f, size_t _items)
        : _erased_io_state_type(_parent, _operation, _items)
        , result(make_result(BuffersType()))
        , completion(std::forward<CompletionRoutine>(f))
    {
    }
  };
  struct _io_state_deleter
  {
    template <class U> void operator()(U *_ptr) const
    {
      _ptr->~U();
      char *ptr = (char *) _ptr;
      ::free(ptr);
    }
  };

public:
  /*! Smart pointer to state of an i/o in progress. Destroying this before an i/o has completed
  is <b>blocking</b> because the i/o must be cancelled before the destructor can safely exit.
  */
  using erased_io_state_ptr = std::unique_ptr<_erased_io_state_type, _io_state_deleter>;
  /*! Smart pointer to state of an i/o in progress. Destroying this before an i/o has completed
  is <b>blocking</b> because the i/o must be cancelled before the destructor can safely exit.
  */
  template <class CompletionRoutine, class BuffersType> using io_state_ptr = std::unique_ptr<_io_state_type<CompletionRoutine, BuffersType>, _io_state_deleter>;

#if DOXYGEN_SHOULD_SKIP_THIS
private:
#else
protected:
#endif
  template <class CompletionRoutine, class BuffersType, class IORoutine> result<io_state_ptr<CompletionRoutine, BuffersType>> _begin_io(operation_t operation, io_request<BuffersType> reqs, CompletionRoutine &&completion, IORoutine &&ioroutine) noexcept;

public:
  /*! \brief Schedule a read to occur asynchronously.

  \return Either an io_state_ptr to the i/o in progress, or an error code.
  \param reqs A scatter-gather and offset request.
  \param completion A callable to call upon i/o completion. Spec is void(async_file_handle *, io_result<buffers_type> &).
  Note that buffers returned may not be buffers input, see documentation for read().
  \errors As for read(), plus ENOMEM.
  \mallocs One calloc, one free. The allocation is unavoidable due to the need to store a type
  erased completion handler of unknown type.
  */
  //[[bindlib::make_free]]
  template <class CompletionRoutine> result<io_state_ptr<CompletionRoutine, buffers_type>> async_read(io_request<buffers_type> reqs, CompletionRoutine &&completion) noexcept;

  /*! \brief Schedule a write to occur asynchronously.

  \return Either an io_state_ptr to the i/o in progress, or an error code.
  \param reqs A scatter-gather and offset request.
  \param completion A callable to call upon i/o completion. Spec is void(async_file_handle *, io_result<const_buffers_type> &).
  Note that buffers returned may not be buffers input, see documentation for write().
  \errors As for write(), plus ENOMEM.
  \mallocs One calloc, one free. The allocation is unavoidable due to the need to store a type
  erased completion handler of unknown type.
  */
  //[[bindlib::make_free]]
  template <class CompletionRoutine> result<io_state_ptr<CompletionRoutine, const_buffers_type>> async_write(io_request<const_buffers_type> reqs, CompletionRoutine &&completion) noexcept;

  BOOST_AFIO_HEADERS_ONLY_VIRTUAL_SPEC io_result<buffers_type> read(io_request<buffers_type> reqs, deadline d = deadline()) noexcept override;
  BOOST_AFIO_HEADERS_ONLY_VIRTUAL_SPEC io_result<const_buffers_type> write(io_request<const_buffers_type> reqs, deadline d = deadline()) noexcept override;
};

BOOST_AFIO_V2_NAMESPACE_END

#if BOOST_AFIO_HEADERS_ONLY == 1 && !defined(DOXYGEN_SHOULD_SKIP_THIS)
#define BOOST_AFIO_INCLUDED_BY_HEADER 1
#ifdef _WIN32
#include "detail/impl/windows/async_file_handle.ipp"
#else
#include "detail/impl/posix/async_file_handle.ipp"
#endif
#undef BOOST_AFIO_INCLUDED_BY_HEADER
#endif

#endif
