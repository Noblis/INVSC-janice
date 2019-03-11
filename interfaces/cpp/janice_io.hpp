#ifndef JANICE_IO_CPP_HPP
#define JANICE_IO_CPP_HPP

#include <janice_io.h>

#include <string>
#include <vector>
#include <stdexcept>

#define JANICE_C_CHECK(func)               \
{                                          \
    JaniceError result = (func);           \
    if (result != JANICE_SUCCESS)          \
        throw std::runtime_error(          \
            janice_error_to_string(result) \
        );                                 \
}

namespace ice
{



// ----------------------------------------------------------------------------
// Media I/O

class JANICE_EXPORT MediaIterator
{
public:
    MediaIterator() :
        _impl(nullptr) 
    {}

    MediaIterator(const JaniceMediaIterator& it)
    {
        this->_impl = new JaniceMediaIterator();
	this->_impl->is_video = it.is_video;
	this->_impl->get_frame_rate = it.get_frame_rate;
        this->_impl->get_physical_frame_rate = it.get_physical_frame_rate;
	this->_impl->next = it.next;
        this->_impl->seek = it.seek;
	this->_impl->get = it.get;
	this->_impl->tell = it.tell;
	this->_impl->reset = it.reset;
        this->_impl->physical_frame = it.physical_frame;
        this->_impl->free_image = it.free_image;
	this->_impl->free = it.free;

	// Invalidate the other iterator. Note that it's free function must be
	// able to handle a null _internal member
        this->_impl->_internal = it._internal;
	it._internal = nullptr;
    }

    MediaIterator(const MediaIterator&) = delete;
    MediaIterator(MediaIterator&&) = delete;

    ~MediaIterator()
    {
        if (_impl) {
            JANICE_C_CHECK(_impl->free(_impl));
	    _impl = nullptr;
	}
    }

    JaniceMediaIterator* impl() const
    {
	return _impl;
    }

private:
    JaniceMediaIterator* _impl;
};



} // namespace ice

#endif // JANICE_CPP_HPP
