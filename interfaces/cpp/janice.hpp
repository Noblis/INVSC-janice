#ifndef JANICE_CPP_HPP
#define JANICE_CPP_HPP

#include <janice.h>

#include <janice_io.hpp>

#include <string>

namespace ice
{

struct JANICE_EXPORT Version
{
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
};

class JANICE_EXPORT Implementation
{
public:
    Implementation(const std::string& sdk_path,
		   const std::string& temp_path,
		   const std::string& log_path,
		   const std::string& algorithm,
		   int num_threads,
		   const std::vector<int>& gpus)
    {
        JANICE_C_CHECK(janice_initialize(sdk_path.c_str(),
				         temp_path.c_str(),
					 log_path.c_str(),
					 algorithm.c_str(),
					 num_threads,
					 gpus.data(),
					 gpus.size()));
    }

    Implementation(const Implementation& other) = delete;
    Implementation(Implementation&& other) = delete;

    ~Implementation()
    {
        JANICE_C_CHECK(janice_finalize());
    }

    void set_log_level(JaniceLogLevel level)
    {
	JANICE_C_CHECK(janice_set_log_level(level));
    }

    Version api_version()
    {
	Version version;
	JANICE_C_CHECK(janice_api_version(&version.major,
				          &version.minor,
					  &version.patch));

	return version;
    }

    Version sdk_version()
    {
	Version version;
	JANICE_C_CHECK(janice_sdk_version(&version.major,
				          &version.minor,
					  &version.patch));

	return version;
    }

    std::map<std::string, std::string> current_configuration()
    {
        JaniceConfiguration c_config;
	JANICE_C_CHECK(janice_get_current_configuration(&c_config));

	std::map<std::string, std::string> config;
	for (size_t i = 0; i < c_config.length; ++i) {
	    config[std::string(c_config.values[i].key)] = std::string(c_config.value[i].value);
	}

	JANICE_C_CHECK(janice_clear_configuration(&c_config));

	return config;
    }
};

class JANICE_EXPORT Context
{
public:
    Context()
    {
	this->_impl = new JaniceContext();
	JANICE_C_CHECK(janice_init_default_context(this->_impl));
    }

    Context(const Context& other)
    {
        this->_impl = new JaniceContext();
	*this->_impl = other._impl;
    }

    Context(Context&& other)
    {
	this->_impl = other._impl;
	other._impl = nullptr;
    }

    ~Context()
    {
	if (this->_impl) {
	    delete this->_impl;
	}
    }

    JaniceDetectionPolicy& policy()       { return this->_impl->policy; }
    JaniceDetectionPolicy  policy() const { return this->_impl->policy; }

    uint32_t& min_object_size()       { return this->_impl->min_object_size; }
    uint32_t  min_object_size() const { return this->_impl->min_object_size; }

    JaniceEnrollmentType& role()       { return this->_impl->role; }
    JaniceEnrollmentType  role() const { return this->_impl->role; }

    double& threshold()       { return this->_impl.threshold; }
    double  threshold() const { return this->_impl.threshold; }

    uint32_t& max_returns       { return this->_impl.max_returns; }
    uint32_t  max_returns const { return this->_impl.max_returns; }

    double& hint()       { return this->_impl.hint; }
    double  hint() const { return this->_impl.hint; }

    JaniceBatchPolicy& batch_policy()       { return this->_impl.batch_policy; }
    JaniceBatchPolicy  batch_policy() const { return this->_impl.batch_policy; }

    JaniceContext* impl() { return this->_impl; }

private:
    JaniceContext* _impl;
};

class Track
{
public:
    Track()
	: _impl(nullptr)
    {}

    Track(const JaniceTrack& track)
    {
        this->_impl = new JaniceTrack();
	this->_impl->length = track.length;
	this->_impl->rects = new JaniceRect[this->_impl->length];
	this->_impl->confidences = new float[this->_impl->length];
	this->_impl->frames = new uint32_t[this->_impl->length];

	for (size_t i = 0; i < this->_impl->length; ++i) {
	    this->_impl->rects[i] = track.rects[i];
	    this->_impl->confidences[i] = track.confidences[i];
            this->_impl->frames[i] = track.frames[i];
	}
    }

    Track(const Track& other)
    {
        this->_impl = new JaniceTrack();
	this->_impl->length = other._impl->length;
	this->_impl->rects = new JaniceRect[this->_impl->length];
	this->_impl->confidences = new float[this->_impl->length];
	this->_impl->frames = new uint32_t[this->_impl->length];

	for (size_t i = 0; i < this->_impl->length; ++i) {
	    this->_impl->rects[i] = other._impl->rects[i];
	    this->_impl->confidences[i] = other._impl->confidences[i];
            this->_impl->frames[i] = other._impl->frames[i];
	}
    }

    Track(Track&& other)
    {
        this->_impl = other._impl;
	other._impl = nullptr;
    }

    ~Track()
    {
        if (this->_impl) {
            	
	}
    }

    std::tuple<JaniceRect, float, uint32_t>& operator[](int i)
    {
        return std::make_tuple(this->_impl->rects[i], this->_impl->confidences[i], this->_impl->frames[i]);
    }

    std::tuple<JaniceRect, float, uint32_t> operator[](int i) const
    {
        return std::make_tuple(this->_impl->rects[i], this->_impl->confidences[i], this->_impl->frames[i]);
    }

    std::tuple<JaniceRect, float, uint32_t>& at(int i)
    {
        if (!(i < (size_t) this->_impl->length)) {
            throw std::out_of_range();
	}
        return std::make_tuple(this->_impl->rects[i], this->_impl->confidences[i], this->_impl->frames[i]);
    }

    std::tuple<JaniceRect, float, uint32_t> at(int i) const
    {
        if (!(i < (size_t) this->_impl->length)) {
            throw std::out_of_range();
	}
        return std::make_tuple(this->_impl->rects[i], this->_impl->confidences[i], this->_impl->frames[i]);
    }

    JaniceRect& rect(int i)
    {
        if (!(i < (size_t) this->_impl->length)) {
            throw std::out_of_range();
	}
	return this->_impl->rects[i];
    }
    
    JaniceRect rect(int i) const
    {
        if (!(i < (size_t) this->_impl->length)) {
            throw std::out_of_range();
	}
	return this->_impl->rects[i];
    }

    float& confidence(int i)
    {
        if (!(i < (size_t) this->_impl->length)) {
            throw std::out_of_range();
	}
	return this->_impl->confidences[i];
    }
    
    float confidence(int i) const
    {
        if (!(i < (size_t) this->_impl->length)) {
            throw std::out_of_range();
	}
	return this->_impl->confidences[i];
    }

    uint32_t& frame(int i)
    {
        if (!(i < (size_t) this->_impl->length)) {
            throw std::out_of_range();
	}
	return this->_impl->frames[i];
    }
    
    uint32_t frame(int i) const
    {
        if (!(i < (size_t) this->_impl->length)) {
            throw std::out_of_range();
	}
	return this->_impl->frames[i];
    }

    JaniceTrack* impl()
    {
	return this->_impl;
    }

private:
    JaniceTrack* _impl;
};

class Detection
{
public:
    Detection()
	: _impl(nullptr)
    {}

    Detection(JaniceDetection* detection)
        : _impl(detection)
    {}

    Detection(const MediaIterator& media, const Rect& rect, const uint32_t frame)
    {
        JANICE_C_CHECK(janice_create_detection_from_rect(media.impl(),
				                         &rect,
							 frame,
							 this->_impl));
    }

    Detection(const MediaIterator& media, const Track& track)
    {
        JANICE_C_CHECK(janice_create_detection_from_track(media.impl(),
				                          track.impl(),
							  this->_impl()));
    }

    Detection(const Detection& other) = delete;
    Detection(Detection&& other)
    {
        this->_impl = other._impl;
	other._impl = nullptr;
    }

    ~Detection()
    {
        if (this->_impl) {
            JANICE_C_CHECK(janice_free_detection(&this->_impl);
	}
    }
};

using Detections = std::vector<Detection>;
using DetectionsGroup = std::vector<Detections>;

} // namespace ice

#endif // JANICE_CPP_HPP
