#ifndef JANICE_DLIB_TEST_UTILS_HPP
#define JANICE_DLIB_TEST_UTILS_HPP

#include <janice.h>
#include <janice_io_opencv.h>

#include <types.hpp>

#include <doctest/doctest.h>

#include <fstream>
#include <unordered_map>

namespace test
{

namespace constants
{

static std::string test_dir = DLIB_TEST_DIR;

} // namespace constants

inline JaniceMediaIterator lena1()
{
    JaniceMediaIterator it;
    REQUIRE(janice_io_opencv_create_media_iterator((constants::test_dir + "/lena1.png").c_str(), &it) == JANICE_SUCCESS);

    return it;
}

inline JaniceMediaIterator lena2()
{
    JaniceMediaIterator it;
    REQUIRE(janice_io_opencv_create_media_iterator((constants::test_dir + "/lena2.png").c_str(), &it) == JANICE_SUCCESS);

    return it;
}

inline JaniceMediaIterator ellen_famous()
{
    JaniceMediaIterator it;
    REQUIRE(janice_io_opencv_create_media_iterator((constants::test_dir + "/ellen-famous.png").c_str(), &it) == JANICE_SUCCESS);

    return it;
}

inline JaniceRect get_first_rect_from_detection(JaniceDetection detection)
{
    JaniceTrack track = detection->track;
    REQUIRE(track.length >= 1);

    return track.rects[0];
}

inline bool track_is_better(const JaniceTrack& t1, const JaniceTrack& t2)
{
    if (t1.length == 0 && t2.length == 0) {
        return false;
    } else if (t1.length == 0) {
        return true;
    } else if (t2.length == 0) {
        return false;
    }

    return t2.confidences[0] > t1.confidences[0];
}

inline bool detection_is_better(const JaniceDetection& d1, const JaniceDetection& d2)
{
    return track_is_better(d1->track, d2->track);
}

inline bool track_is_bigger(const JaniceTrack& t1, const JaniceTrack& t2)
{
    if (t1.length == 0 && t2.length == 0) {
        return false;
    } else if (t1.length == 0) {
        return true;
    } else if (t2.length == 0) {
        return false;
    }

    return (t2.rects[0].width * t2.rects[0].height) > (t1.rects[0].width * t1.rects[0].height);
}

inline bool detection_is_bigger(const JaniceDetection& d1, const JaniceDetection& d2)
{
    return track_is_bigger(d1->track, d2->track);
}

inline bool tracks_are_the_same(const JaniceTrack& t1, const JaniceTrack& t2)
{
    REQUIRE(t1.length == t2.length);

    for (size_t i = 0; i < t1.length; ++i) {
        REQUIRE(t1.rects[i].x      == t2.rects[i].x);
        REQUIRE(t1.rects[i].y      == t2.rects[i].y);
        REQUIRE(t1.rects[i].width  == t2.rects[i].width);
        REQUIRE(t1.rects[i].height == t2.rects[i].height);
        REQUIRE(t1.frames[i]       == t2.frames[i]);
        REQUIRE(t1.confidences[i]  == t2.confidences[i]);
    }

    return true;
}

inline bool detections_are_the_same(const JaniceDetection& d1, const JaniceDetection& d2)
{
    return tracks_are_the_same(d1->track, d2->track);
}

inline void print_detection(const JaniceDetection& d)
{
    JaniceTrack t = d->track;

    std::cout << "Detection: (" << t.length << "): [" << std::endl;
    for (size_t i = 0; i < t.length; ++i) {
        std::cout << "  Rect(x:" << t.rects[i].x <<
                           ",y:" << t.rects[i].y <<
                           ",w:" << t.rects[i].width <<
                           ",h:" << t.rects[i].height << ") "
                  << "Conf: " << t.confidences[i] << " "
                  << "Frame: " << t.frames[i] << std::endl;
    }
    std::cout << "]" << std::endl;
}

inline bool templates_are_the_same(const JaniceTemplate& t1, const JaniceTemplate& t2)
{
    REQUIRE(t1->fv == t2->fv);

    return true;
}

static JaniceTemplates cached_tmpls;
static JaniceTemplates cached_lenas;
static JaniceTemplateIds cached_ids;

inline void clear_cache()
{
    if (cached_tmpls.length != 0) {
        for (size_t i = 0; i < cached_tmpls.length; ++i) {
            janice_free_template(&cached_tmpls.tmpls[i]);
        }
        delete[] cached_tmpls.tmpls;

        for (size_t i = 0; i < cached_lenas.length; ++i) {
            janice_free_template(&cached_lenas.tmpls[i]);
        }
        delete[] cached_lenas.tmpls;

        delete[] cached_ids.ids;
    }

    cached_tmpls.length = 0;
    cached_lenas.length = 0;
    cached_ids.length = 0;
}

inline JaniceTemplates get_tmpls()
{
    if (cached_tmpls.length == 0) {
        JaniceContext context;
        REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

        JaniceMediaIterator ellen_pic = ellen_famous();
        JaniceDetections detections;
        REQUIRE(janice_enroll_from_media(&ellen_pic, &context, &cached_tmpls, &detections) == JANICE_SUCCESS);

        REQUIRE(detections.length == 10);
        REQUIRE(cached_tmpls.length == 10);

        REQUIRE(ellen_pic.free(&ellen_pic) == JANICE_SUCCESS);
        REQUIRE(janice_clear_detections(&detections) == JANICE_SUCCESS);
    }

    return cached_tmpls;
}

inline JaniceTemplateIds get_ids(const JaniceTemplates& tmpls)
{
    if (cached_ids.length == 0) {
        cached_ids.length = tmpls.length;
        cached_ids.ids = new uint64_t[cached_ids.length];

        for (size_t i = 0; i < cached_ids.length; ++i) {
            cached_ids.ids[i] = i * tmpls.length;
        }
    }

    return cached_ids;
}

inline JaniceTemplates get_lena_tmpls()
{
    if (cached_lenas.length == 0) {
        JaniceContext context;
        REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

        JaniceTemplates lena1_tmpls;
        {
            JaniceMediaIterator lena1 = test::lena1();
            JaniceDetections detections;
            REQUIRE(janice_enroll_from_media(&lena1, &context, &lena1_tmpls, &detections) == JANICE_SUCCESS);

            REQUIRE(detections.length == 1);
            REQUIRE(lena1_tmpls.length == 1);

            REQUIRE(lena1.free(&lena1) == JANICE_SUCCESS);
            REQUIRE(janice_clear_detections(&detections) == JANICE_SUCCESS);
        }

        JaniceTemplates lena2_tmpls;
        {
            JaniceMediaIterator lena2 = test::lena2();
            JaniceDetections detections;
            REQUIRE(janice_enroll_from_media(&lena2, &context, &lena2_tmpls, &detections) == JANICE_SUCCESS);

            REQUIRE(detections.length == 1);
            REQUIRE(lena2_tmpls.length == 1);

            REQUIRE(lena2.free(&lena2) == JANICE_SUCCESS);
            REQUIRE(janice_clear_detections(&detections) == JANICE_SUCCESS);
        }

        cached_lenas.length = 2;
        cached_lenas.tmpls = new JaniceTemplate[2];

        cached_lenas.tmpls[0] = lena1_tmpls.tmpls[0];
        cached_lenas.tmpls[1] = lena2_tmpls.tmpls[0];
    }

    return cached_lenas;
}

inline JaniceGallery create_empty_gallery()
{
    JaniceTemplates temp_tmpls;
    temp_tmpls.length = 0;

    JaniceTemplateIds temp_ids;
    temp_ids.length = 0;

    JaniceGallery gallery = nullptr;
    REQUIRE(janice_create_gallery(&temp_tmpls, &temp_ids, &gallery) == JANICE_SUCCESS);

    return gallery;
}

inline bool gallery_does_contain(const JaniceGallery& gallery, const JaniceTemplates& tmpls, const JaniceTemplateIds& ids)
{
    REQUIRE(gallery->tmpls.size() == tmpls.length);
    REQUIRE(gallery->ids.size() == tmpls.length);
    REQUIRE(gallery->reverse_lut.size() == tmpls.length);

    for (size_t i = 0; i < tmpls.length; ++i) {
        REQUIRE(templates_are_the_same(tmpls.tmpls[i], &gallery->tmpls[i]));
        REQUIRE(gallery->ids.find(ids.ids[i]) != gallery->ids.end());
        REQUIRE(gallery->reverse_lut.find(i)  != gallery->reverse_lut.end());
    }

    return true;
}

inline bool gallery_is_empty(const JaniceGallery& gallery)
{
    REQUIRE(gallery->tmpls.empty());
    REQUIRE(gallery->ids.empty());
    REQUIRE(gallery->reverse_lut.empty());

    return true;
}

inline bool galleries_are_the_same(const JaniceGallery& g1, const JaniceGallery& g2)
{
    REQUIRE(g1->tmpls.size() == g2->tmpls.size());
    REQUIRE(g1->ids.size() == g2->ids.size());
    REQUIRE(g1->reverse_lut.size() == g2->reverse_lut.size());

    for (size_t i = 0; i < g1->tmpls.size(); ++i) {
        REQUIRE(templates_are_the_same(&g1->tmpls[i], &g2->tmpls[i]));

        uint64_t g1_id_at_pos = g1->reverse_lut[i];
        uint64_t g2_id_at_pos = g2->reverse_lut[i];
        REQUIRE(g1_id_at_pos == g2_id_at_pos);

        REQUIRE(g1->ids[g1_id_at_pos] == g2->ids[g2_id_at_pos]);
    }

    return true;
}

/*
inline void compare_all_faces(const json& image, const JaniceDetections& detections)
{
    REQUIRE(image["faces"].size() == detections.length);

    // Sort to get a consistent order
    auto sort_by_x = [](const JaniceDetection& left, const JaniceDetection& right) {
        JaniceTrack left_track;
        REQUIRE(janice_detection_get_track(left, &left_track) == JANICE_SUCCESS);

        JaniceTrack right_track;
        REQUIRE(janice_detection_get_track(right, &right_track) == JANICE_SUCCESS);

        bool cmp = left_track.rects[0].x < right_track.rects[0].x;

        REQUIRE(janice_clear_track(&left_track) == JANICE_SUCCESS);
        REQUIRE(janice_clear_track(&right_track) == JANICE_SUCCESS);

        return cmp;
    };

    std::sort(detections.detections, detections.detections + detections.length, sort_by_x);

    for (size_t i = 0; i < detections.length; ++i) {
        const json& face = image["faces"][i]["track"];

        JaniceTrack track;
        REQUIRE(janice_detection_get_track(detections.detections[i], &track) == JANICE_SUCCESS);

        REQUIRE(track.length == face.size());

        for (size_t j = 0; j < track.length; ++j) {
            REQUIRE(track.rects[j].x      == face[j]["x"].get<int>());
            REQUIRE(track.rects[j].y      == face[j]["y"].get<int>());
            REQUIRE(track.rects[j].width  == face[j]["width"].get<int>());
            REQUIRE(track.rects[j].height == face[j]["height"].get<int>());

            REQUIRE(track.confidences[j]  == doctest::Approx(face[j]["confidence"].get<float>()));

            REQUIRE(track.frames[j] == face[j]["frame"].get<uint32_t>());
        }

        REQUIRE(janice_clear_track(&track) == JANICE_SUCCESS);
    }
}

inline void compare_face(const json& image, const JaniceDetection& detection, const std::string& type)
{
    const json& face = image["faces"][image[type].get<int>()]["track"];

    JaniceTrack track;
    REQUIRE(janice_detection_get_track(detection, &track) == JANICE_SUCCESS);

    REQUIRE(track.length == face.size());

    for (size_t i = 0; i < track.length; ++i) {
        REQUIRE(track.rects[i].x      == face[i]["x"].get<int>());
        REQUIRE(track.rects[i].y      == face[i]["y"].get<int>());
        REQUIRE(track.rects[i].width  == face[i]["width"].get<int>());
        REQUIRE(track.rects[i].height == face[i]["height"].get<int>());

        REQUIRE(track.confidences[i]  == doctest::Approx(face[i]["confidence"].get<float>()));

        REQUIRE(track.frames[i] == face[i]["frame"].get<uint32_t>());
    }

    REQUIRE(janice_clear_track(&track) == JANICE_SUCCESS);
}

// This strongly assumes that template ids are less than INT_MAX
inline uint64_t key(uint64_t left, uint64_t right)
{
    return (uint64_t) (left << 32 | right); // Combines keys
}

static std::unordered_map<uint64_t, double> l2_norm_matches {
    {key(0, 0),  0.000000}, {key(0, 1), -1.082104}, {key(0, 2), -1.071451}, {key(0, 3), -1.150636}, {key(0, 4), -1.000022}, {key(0, 5), -1.140889}, {key(0, 6), -1.078331},
    {key(1, 0), -1.082104}, {key(1, 1),  0.000000}, {key(1, 2), -1.064663}, {key(1, 3), -1.183342}, {key(1, 4), -1.019879}, {key(1, 5), -1.053857}, {key(1, 6), -1.070062},
    {key(2, 0), -1.071451}, {key(2, 1), -1.064663}, {key(2, 2),  0.000000}, {key(2, 3), -1.188169}, {key(2, 4), -1.015495}, {key(2, 5), -1.101490}, {key(2, 6), -1.118546},
    {key(3, 0), -1.150636}, {key(3, 1), -1.183342}, {key(3, 2), -1.188169}, {key(3, 3),  0.000000}, {key(3, 4), -1.132501}, {key(3, 5), -1.076974}, {key(3, 6), -1.135702},
    {key(4, 0), -1.000022}, {key(4, 1), -1.019879}, {key(4, 2), -1.015495}, {key(4, 3), -1.132501}, {key(4, 4),  0.000000}, {key(4, 5), -1.017314}, {key(4, 6), -1.159047},
    {key(5, 0), -1.140889}, {key(5, 1), -1.053857}, {key(5, 2), -1.101490}, {key(5, 3), -1.076974}, {key(5, 4), -1.017314}, {key(5, 5),  0.000000}, {key(5, 6), -1.063128},
    {key(6, 0), -1.078331}, {key(6, 1), -1.070062}, {key(6, 2), -1.118546}, {key(6, 3), -1.135702}, {key(6, 4), -1.159047}, {key(6, 5), -1.063128}, {key(6, 6),  0.000000}
};

static std::unordered_map<uint64_t, double> l2_no_norm_matches {
    {key(0, 0), 0.000000}, {key(0, 1), -13.049514}, {key(0, 2), -12.978262}, {key(0, 3), -12.984823}, {key(0, 4), -12.495678}, {key(0, 5), -12.561567},
    {key(1, 0), -13.049514}, {key(1, 1), 0.000000}, {key(1, 2), -13.207459}, {key(1, 3), -13.296777}, {key(1, 4), -12.867401}, {key(1, 5), -12.820041},
    {key(2, 0), -12.978262}, {key(2, 1), -13.207459}, {key(2, 2), 0.000000}, {key(2, 3), -13.254453}, {key(2, 4), -12.798164}, {key(2, 5), -12.813466},
    {key(3, 0), -12.984823}, {key(3, 1), -13.296777}, {key(3, 2), -13.254453}, {key(3, 3), 0.000000}, {key(3, 4), -12.843656}, {key(3, 5), -12.642898},
    {key(4, 0), -12.495678}, {key(4, 1), -12.867401}, {key(4, 2), -12.798164}, {key(4, 3), -12.843656}, {key(4, 4), 0.000000}, {key(4, 5), -12.204082},
    {key(5, 0), -12.561567}, {key(5, 1), -12.820041}, {key(5, 2), -12.813466}, {key(5, 3), -12.642898}, {key(5, 4), -12.204082}, {key(5, 5), 0.000000}
};

// ----------------------------------------------------------------------------
// Utility functions

static JaniceTemplates cached_tmpls;
static JaniceTemplateIds cached_ids;

inline void clear_cache()
{
    if (cached_tmpls.length != 0) {
        for (size_t i = 0; i < cached_tmpls.length; ++i) {
            janice_free_template(&cached_tmpls.tmpls[i]);
        }
        delete[] cached_tmpls.tmpls;
        delete[] cached_ids.ids;
    }

    cached_tmpls.length = 0;
    cached_ids.length = 0;
}

inline JaniceTemplates get_tmpls()
{
    if (cached_tmpls.length == 0) {
        JaniceContext context = val::create_context_from_config(val::test_config(), JaniceDetectAll, Janice1NProbe);

        json images = val::test_config()["images"];

        cached_tmpls.length = images.size();
        cached_tmpls.tmpls = new JaniceTemplate[cached_tmpls.length];

        for (size_t i = 0; i < images.size(); ++i) {
            JaniceDetections detections;
            detections.detections = new JaniceDetection[1];
            detections.length = 1;

            JaniceMediaIterators media;
            media.media = new JaniceMediaIterator[1];
            media.length = 1;

            const json& image = images[i];
            const json& face  = image["faces"][image["best"].get<int>()];

            media.media[0] = create_media_iterator(images[i]["path"]);
            detections.detections[0] = create_detection(media.media[0], face);

            REQUIRE(janice_enroll_from_detections(&media, &detections, &context, &cached_tmpls.tmpls[i]) == JANICE_SUCCESS);

            REQUIRE(media.media[0].free(&media.media[0]) == JANICE_SUCCESS);
            REQUIRE(janice_free_detection(&detections.detections[0]) == JANICE_SUCCESS);

            delete[] media.media;
            delete[] detections.detections;
        }
    }

    return cached_tmpls;
}

inline JaniceTemplateIds get_ids(const JaniceTemplates& tmpls)
{
    if (cached_ids.length == 0) {
        cached_ids.length = tmpls.length;
        cached_ids.ids = new uint64_t[cached_ids.length];

        for (size_t i = 0; i < cached_ids.length; ++i) {
            cached_ids.ids[i] = i;
        }
    }

    return cached_ids;
}

inline JaniceGallery create_empty_gallery()
{
    JaniceTemplates temp_tmpls;
    temp_tmpls.length = 0;

    JaniceTemplateIds temp_ids;
    temp_ids.length = 0;

    JaniceGallery gallery = nullptr;
    REQUIRE(janice_create_gallery(&temp_tmpls, &temp_ids, &gallery) == JANICE_SUCCESS);

    return gallery;
}

inline void check_gallery_contains(JaniceGallery gallery, const JaniceTemplates& tmpls, const JaniceTemplateIds& ids)
{
    REQUIRE(gallery->g.size() == tmpls.length);
    REQUIRE(gallery->tids_to_pos.size() == tmpls.length);

    for (size_t i = 0; i < ids.length; ++i) {
        REQUIRE(gallery->tids_to_pos.find(ids.ids[i]) != gallery->tids_to_pos.end());
    }
}

inline void check_gallery_empty(JaniceGallery gallery)
{
    REQUIRE(gallery->g.empty());
    REQUIRE(gallery->tids_to_pos.empty());
}
*/

} // namespace test

#endif // JANICE_DLIB_TEST_UTILS_HPP
