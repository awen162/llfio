/* map_handle.hpp
A handle to a source of mapped memory
(C) 2017 Niall Douglas http://www.nedprod.com/
File Created: Apr 2017


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

#include "../../../map_handle.hpp"
#include "../../../utils.hpp"

#include <sys/mman.h>

BOOST_AFIO_V2_NAMESPACE_BEGIN

result<section_handle> section_handle::section(file_handle &backing, extent_type maximum_size, flag _flag) noexcept
{
  if(!maximum_size)
  {
    if(backing.is_valid())
    {
      BOOST_OUTCOME_TRY(length, backing.length());
      maximum_size = length;
    }
    else
      return make_errored_result<section_handle>(stl11::errc::invalid_argument);
  }
  if(!backing.is_valid())
    maximum_size = utils::round_up_to_page_size(maximum_size);
  result<section_handle> ret(section_handle(native_handle_type(), backing.is_valid() ? &backing : nullptr, maximum_size, _flag));
  // There are no section handles on POSIX, so do nothing
  BOOST_AFIO_LOG_FUNCTION_CALL(ret.value()._v.fd);
  return ret;
}

result<section_handle::extent_type> section_handle::truncate(extent_type newsize) noexcept
{
  newsize = utils::round_up_to_page_size(newsize);
  // There are no section handles on POSIX, so do nothing
  _length = newsize;
  return make_result<extent_type>(newsize);
}


map_handle::~map_handle()
{
  if(_v)
  {
    // Unmap the view
    auto ret = map_handle::close();
    if(ret.has_error())
    {
      BOOST_AFIO_LOG_FATAL(_v.fd, "map_handle::~map_handle() close failed");
      abort();
    }
  }
}

result<void> map_handle::close() noexcept
{
  BOOST_AFIO_LOG_FUNCTION_CALL(_addr);
  if(_addr)
  {
    if(-1 == ::munmap(_addr, _length))
      return make_errored_result<void>(errno);
  }
  // We don't want ~handle() to close our borrowed handle
  _v = native_handle_type();
  _addr = nullptr;
  _length = 0;
  return make_valued_result<void>();
}

native_handle_type map_handle::release() noexcept
{
  BOOST_AFIO_LOG_FUNCTION_CALL(_v.h);
  // We don't want ~handle() to close our borrowed handle
  _v = native_handle_type();
  _addr = nullptr;
  _length = 0;
  return native_handle_type();
}

inline result<void *> do_mmap(native_handle_type &nativeh, void *ataddr, section_handle &section, map_handle::size_type bytes, map_handle::extent_type offset, section_handle::flag _flag) noexcept
{
  bool have_backing = section.backing();
  int prot = 0, flags = have_backing ? MAP_SHARED : MAP_ANONYMOUS;
  void *addr = nullptr;
  if(_flag == section_handle::flag::none)
  {
    prot |= PROT_NONE;
  }
  else if(_flag & section_handle::flag::cow)
  {
    prot |= PROT_READ|PROT_WRITE;
    flags &= ~MAP_SHARED;
    flags |= MAP_PRIVATE;
    nativeh.behaviour |= native_handle_type::disposition::seekable | native_handle_type::disposition::readable | native_handle_type::disposition::writable;
  }
  else if(_flag & section_handle::flag::write)
  {
    prot |= PROT_READ|PROT_WRITE;
    nativeh.behaviour |= native_handle_type::disposition::seekable | native_handle_type::disposition::readable | native_handle_type::disposition::writable;
  }
  else if(_flag & section_handle::flag::read)
  {
    prot |= PROT_READ;
    nativeh.behaviour |= native_handle_type::disposition::seekable | native_handle_type::disposition::readable;
  }
  if(_flag & section_handle::flag::execute)
    prot |= PROT_EXEC;
#ifdef MAP_NORESERVE
  if(_flag & section_handle::flag::nocommit)
    flags |= MAP_NORESERVE;
#endif
#ifdef MAP_POPULATE
  if(_flag & section_handle::flag::prefault)
    flags |= MAP_POPULATE;
#endif
#ifdef MAP_PREFAULT_READ
  if(_flag & section_handle::flag::prefault)
    flags |= MAP_PREFAULT_READ;
#endif
#ifdef MAP_NOSYNC
  if(have_backing && (section.backing()->kernel_caching() == handle::caching::temporary))
    flags |= MAP_NOSYNC;
#endif
  addr = ::mmap(ataddr, bytes, prot, flags, have_backing ? section.backing_native_handle().fd : -1, offset);
  if(!addr)
    return make_errored_result<void *>(errno);
  return addr;
}

result<map_handle> map_handle::map(section_handle &section, size_type bytes, extent_type offset, section_handle::flag _flag) noexcept
{
  if(!section.backing())
  {
    // Do NOT round up bytes to the nearest page size for backed maps, it causes an attempt to extend the file
    bytes = utils::round_up_to_page_size(bytes);
  }
  result<map_handle> ret(map_handle(io_handle(), &section));
  native_handle_type &nativeh = ret.get()._v;
  BOOST_OUTCOME_TRY(addr, do_mmap(nativeh, nullptr, section, bytes, offset, _flag));
  ret.get()._addr = (char *) addr;
  ret.get()._offset = offset;
  ret.get()._length = bytes;
  // Make my handle borrow the native handle of my backing storage
  ret.get()._v.fd = section.backing_native_handle().fd;
  BOOST_AFIO_LOG_FUNCTION_CALL(ret.get()._v.fd);
  return ret;
}

result<map_handle::buffer_type> map_handle::commit(buffer_type region, section_handle::flag _flag) noexcept
{
  BOOST_AFIO_LOG_FUNCTION_CALL(_v.fd);
  if(!region.first)
    return make_errored_result<map_handle::buffer_type>(stl11::errc::invalid_argument);
  // Set permissions on the pages
  region = utils::round_to_page_size(region);
  extent_type offset = _offset + (region.first - _addr);
  size_type bytes = region.second;
  BOOST_OUTCOME_TRYV(do_mmap(_v, region.first, section, bytes, offset, _flag));
  // Tell the kernel we will be using these pages soon
  if(-1 == ::madvise(region.first, region.second, MADV_WILLNEED))
    return make_errored_result<map_handle::buffer_type>(errno);
  return region;
}

result<map_handle::buffer_type> map_handle::decommit(buffer_type region) noexcept
{
  BOOST_AFIO_LOG_FUNCTION_CALL(_v.h);
  if(!region.first)
    return make_errored_result<map_handle::buffer_type>(stl11::errc::invalid_argument);
  region = utils::round_to_page_size(region);
  // Tell the kernel to kick these pages into storage
  if(-1 == ::madvise(region.first, region.second, MADV_DONTNEED))
    return make_errored_result<map_handle::buffer_type>(errno);
  // Set permissions on the pages to no access
  extent_type offset = _offset + (region.first - _addr);
  size_type bytes = region.second;
  BOOST_OUTCOME_TRYV(do_mmap(_v, region.first, section, bytes, offset, section_handle::flag::none));
  return region;
}

result<void> map_handle::zero(buffer_type region) noexcept
{
  BOOST_AFIO_LOG_FUNCTION_CALL(_v.fd);
  if(!region.first)
    return make_errored_result<void>(stl11::errc::invalid_argument);
#ifdef MADV_REMOVE
  buffer_type page_region { (char *) utils::round_up_to_page_size((uintptr_t) region.first), utils::round_down_to_page_size(region.second) };
  // Zero contents and punch a hole in any backing storage
  if(page_region.second && -1 != ::madvise(page_region.first, page_region.second, MADV_REMOVE))
  {
    memset(region.first, 0, page_region.first - region.first);
    memset(page_region.first + page_region.second, 0, (region.first + region.second) - (page_region.first + page_region.second));
    return make_valued_result<void>();
  }
#endif
  //! \todo Once you implement file_handle::zero(), please implement map_handle::zero()
  memset(region.first, 0, region.second);
  return make_valued_result<void>();
}

result<span<map_handle::buffer_type>> map_handle::prefetch(span<buffer_type> regions) noexcept
{
  BOOST_AFIO_LOG_FUNCTION_CALL(0);
  for(const auto &region : regions)
  {
    if(-1 == ::madvise(region.first, region.second, MADV_WILLNEED))
      return make_errored_result<span<map_handle::buffer_type>>(errno);
  }
  return regions;
}

result<map_handle::buffer_type> map_handle::do_not_store(buffer_type region) noexcept
{
  BOOST_AFIO_LOG_FUNCTION_CALL(0);
  region = utils::round_to_page_size(region);
  if(!region.first)
    return make_errored_result<map_handle::buffer_type>(stl11::errc::invalid_argument);
#ifdef MADV_FREE
  // Tell the kernel to throw away the contents of these pages
  if(-1 == ::madvise(_region.first, _region.second, MADV_FREE))
    return make_errored_result<map_handle::buffer_type>(errno);
  else
    return region;
#endif
  // No support on this platform
  region.second = 0;
  return region;
}

map_handle::io_result<map_handle::buffers_type> map_handle::read(io_request<buffers_type> reqs, deadline) noexcept
{
  BOOST_AFIO_LOG_FUNCTION_CALL(_v.fd);
  char *addr = _addr + reqs.offset;
  size_type togo = (size_type)(_length - reqs.offset);
  for(buffer_type &req : reqs.buffers)
  {
    if(togo)
    {
      req.first = addr;
      if(req.second > togo)
        req.second = togo;
      addr += req.second;
      togo -= req.second;
    }
    else
      req.second = 0;
  }
  return reqs.buffers;
}

map_handle::io_result<map_handle::const_buffers_type> map_handle::write(io_request<const_buffers_type> reqs, deadline) noexcept
{
  BOOST_AFIO_LOG_FUNCTION_CALL(_v.fd);
  char *addr = _addr + reqs.offset;
  size_type togo = (size_type)(_length - reqs.offset);
  for(const_buffer_type &req : reqs.buffers)
  {
    if(togo)
    {
      if(req.second > togo)
        req.second = togo;
      memcpy(addr, req.first, req.second);
      req.first = addr;
      addr += req.second;
      togo -= req.second;
    }
    else
      req.second = 0;
  }
  return reqs.buffers;
}

BOOST_AFIO_V2_NAMESPACE_END
