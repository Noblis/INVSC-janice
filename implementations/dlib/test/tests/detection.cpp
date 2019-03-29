#include <janice.h>
#include <janice_io_opencv.h>

#include <janice_test_utils.hpp>

#include <doctest/doctest.h>

// ----------------------------------------------------------------------------
// Detection Constructor Tests

TEST_CASE("janice_create_detection_from_rect"
          * doctest::test_suite("janice_tests"))
{
    JaniceMediaIterator lena1 = test::lena1();

    // Pick random coordinates
    JaniceRect rect;
    rect.x = 17;
    rect.y = 28;
    rect.width = 111;
    rect.height = 256;

    JaniceDetection detection = nullptr;
    REQUIRE(janice_create_detection_from_rect(&lena1, &rect, 0, &detection) == JANICE_SUCCESS);

    JaniceRect created_rect = test::get_first_rect_from_detection(detection);
    REQUIRE(rect.x      == created_rect.x);
    REQUIRE(rect.y      == created_rect.y);
    REQUIRE(rect.width  == created_rect.width);
    REQUIRE(rect.height == created_rect.height);

    REQUIRE(lena1.free(&lena1) == JANICE_SUCCESS);
    REQUIRE(janice_free_detection(&detection) == JANICE_SUCCESS);
}

TEST_CASE("janice_create_detection_from_track"
          * doctest::test_suite("janice_tests"))
{
    JaniceMediaIterator lena1 = test::lena1();

    // Pick random coordinates
    JaniceTrack track;
    track.length = 3;
    track.rects = new JaniceRect[track.length];
    track.frames = new uint32_t[track.length];
    track.confidences = new float[track.length];

    for (size_t i = 0; i < track.length; ++i) {
        track.rects[i].x      = i + 1;
        track.rects[i].y      = i + 2;
        track.rects[i].width  = i + 3;
        track.rects[i].height = i + 4;
        track.frames[i]       = i + 5;
        track.confidences[i]  = i + 6;
    }

    JaniceDetection detection = nullptr;
    REQUIRE(janice_create_detection_from_track(&lena1, &track, &detection) == JANICE_SUCCESS);

    JaniceTrack created_track = detection->track;
    REQUIRE(created_track.length == track.length);

    for (size_t i = 0; i < track.length; ++i) {
        REQUIRE(created_track.rects[i].x      == track.rects[i].x);
        REQUIRE(created_track.rects[i].y      == track.rects[i].y);
        REQUIRE(created_track.rects[i].width  == track.rects[i].width);
        REQUIRE(created_track.rects[i].height == track.rects[i].height);
        REQUIRE(created_track.frames[i]       == track.frames[i]);
        REQUIRE(created_track.confidences[i]  == track.confidences[i]);
    }

    REQUIRE(lena1.free(&lena1) == JANICE_SUCCESS);
    REQUIRE(janice_free_detection(&detection) == JANICE_SUCCESS);
}

// ----------------------------------------------------------------------------
// Detection Serialize / Deserialize tests

TEST_CASE("janice_detection_serialize_deserialize"
          * doctest::test_suite("janice_tests"))
{
    JaniceMediaIterator lena1 = test::lena1();

    // Pick random coordinates
    JaniceRect rect;
    rect.x = 17;
    rect.y = 28;
    rect.width = 111;
    rect.height = 256;

    JaniceDetection detection = nullptr;
    REQUIRE(janice_create_detection_from_rect(&lena1, &rect, 0, &detection) == JANICE_SUCCESS);

    REQUIRE(lena1.free(&lena1) == JANICE_SUCCESS);

    // Serialize
    uint8_t* buffer = nullptr;
    size_t len;
    REQUIRE(janice_serialize_detection(detection, &buffer, &len) == JANICE_SUCCESS);

    // Deserialize
    JaniceDetection new_detection;
    REQUIRE(janice_deserialize_detection(buffer, len, &new_detection) == JANICE_SUCCESS);

    REQUIRE(test::detections_are_the_same(detection, new_detection));

    REQUIRE(janice_free_detection(&detection) == JANICE_SUCCESS);
    REQUIRE(janice_free_detection(&new_detection) == JANICE_SUCCESS);
}


// ----------------------------------------------------------------------------
// Detection Load / Store tests

TEST_CASE("janice_detection_read_write"
          * doctest::test_suite("janice_tests"))
{
    JaniceMediaIterator lena1 = test::lena1();

    // Pick random coordinates
    JaniceRect rect;
    rect.x = 17;
    rect.y = 28;
    rect.width = 111;
    rect.height = 256;

    JaniceDetection detection = nullptr;
    REQUIRE(janice_create_detection_from_rect(&lena1, &rect, 0, &detection) == JANICE_SUCCESS);

    REQUIRE(lena1.free(&lena1) == JANICE_SUCCESS);

    // Write
    REQUIRE(janice_write_detection(detection, (test::constants::test_dir + "/detection.bin").c_str()) == JANICE_SUCCESS);

    // Deserialize
    JaniceDetection new_detection;
    REQUIRE(janice_read_detection((test::constants::test_dir + "/detection.bin").c_str(), &new_detection) == JANICE_SUCCESS);

    REQUIRE(test::detections_are_the_same(detection, new_detection));

    REQUIRE(janice_free_detection(&detection) == JANICE_SUCCESS);
    REQUIRE(janice_free_detection(&new_detection) == JANICE_SUCCESS);
}

// ----------------------------------------------------------------------------
// Detection tests


TEST_CASE("janice_detect"
          * doctest::test_suite("janice_tests"))
{
    JaniceContext context;
    REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

    JaniceDetections all_detections;
    {
        context.policy = JaniceDetectAll;

        JaniceMediaIterator ellen_famous = test::ellen_famous();
        REQUIRE(janice_detect(&ellen_famous, &context, &all_detections) == JANICE_SUCCESS);

        REQUIRE(ellen_famous.free(&ellen_famous) == JANICE_SUCCESS);
    }

    REQUIRE(all_detections.length == 10);

    size_t best_idx = 0, biggest_idx = 0;
    for (size_t i = 1; i < all_detections.length; ++i) {
        if (test::detection_is_better(all_detections.detections[best_idx],
                                      all_detections.detections[i])) {
            best_idx = i;
        }

        if (test::detection_is_bigger(all_detections.detections[biggest_idx],
                                      all_detections.detections[i])) {
            biggest_idx = i;
        }
    }

    //std::cout << "All:" << std::endl;
    //for (size_t i = 0; i < all_detections.length; ++i) {
    //    test::print_detection(all_detections.detections[i]);
    //}
    //std::cout << "Best:" << std::endl;
    //test::print_detection(all_detections.detections[best_idx]);
    //std::cout << "Biggest:" << std::endl;
    //test::print_detection(all_detections.detections[biggest_idx]);

    JaniceDetections best_detections;
    {
        context.policy = JaniceDetectBest;

        JaniceMediaIterator ellen_famous = test::ellen_famous();
        REQUIRE(janice_detect(&ellen_famous, &context, &best_detections) == JANICE_SUCCESS);

        REQUIRE(ellen_famous.free(&ellen_famous) == JANICE_SUCCESS);
    }

    REQUIRE(best_detections.length == 1);
    REQUIRE(test::detections_are_the_same(all_detections.detections[best_idx], best_detections.detections[0]));

    JaniceDetections biggest_detections;
    {
        context.policy = JaniceDetectLargest;

        JaniceMediaIterator ellen_famous = test::ellen_famous();
        REQUIRE(janice_detect(&ellen_famous, &context, &biggest_detections) == JANICE_SUCCESS);

        REQUIRE(ellen_famous.free(&ellen_famous) == JANICE_SUCCESS);
    }

    REQUIRE(biggest_detections.length == 1);
    REQUIRE(test::detections_are_the_same(all_detections.detections[biggest_idx], biggest_detections.detections[0]));
}

TEST_CASE("janice_detect_with_callback"
          * doctest::test_suite("janice_tests"))
{
    auto detection_callback = [](const JaniceDetection* detection, size_t, void* user_data) {
        std::vector<JaniceTrack>* tracks = (std::vector<JaniceTrack>*) user_data;

        JaniceTrack track;
        REQUIRE(janice_detection_get_track(*detection, &track) == JANICE_SUCCESS);

        tracks->push_back(track);

        return JANICE_SUCCESS;
    };

    JaniceContext context;
    REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

    std::vector<JaniceTrack> all_tracks;
    {
        context.policy = JaniceDetectAll;

        JaniceMediaIterator ellen_famous = test::ellen_famous();
        REQUIRE(janice_detect_with_callback(&ellen_famous, &context, detection_callback, (void*) &all_tracks) == JANICE_SUCCESS);

        REQUIRE(ellen_famous.free(&ellen_famous) == JANICE_SUCCESS);
    }

    REQUIRE(all_tracks.size() == 10);

    size_t best_idx = 0, biggest_idx = 0;
    for (size_t i = 1; i < all_tracks.size(); ++i) {
        if (test::track_is_better(all_tracks[best_idx],
                                  all_tracks[i])) {
            best_idx = i;
        }

        if (test::track_is_bigger(all_tracks[biggest_idx],
                                  all_tracks[i])) {
            biggest_idx = i;
        }
    }

    //std::cout << "All:" << std::endl;
    //for (size_t i = 0; i < all_detections.length; ++i) {
    //    test::print_detection(all_detections.detections[i]);
    //}
    //std::cout << "Best:" << std::endl;
    //test::print_detection(all_detections.detections[best_idx]);
    //std::cout << "Biggest:" << std::endl;
    //test::print_detection(all_detections.detections[biggest_idx]);

    std::vector<JaniceTrack> best_tracks;
    {
        context.policy = JaniceDetectBest;

        JaniceMediaIterator ellen_famous = test::ellen_famous();
        REQUIRE(janice_detect_with_callback(&ellen_famous, &context, detection_callback, (void*) &best_tracks) == JANICE_SUCCESS);

        REQUIRE(ellen_famous.free(&ellen_famous) == JANICE_SUCCESS);
    }

    REQUIRE(best_tracks.size() == 1);
    REQUIRE(test::tracks_are_the_same(all_tracks[best_idx], best_tracks[0]));

    std::vector<JaniceTrack> biggest_tracks;
    {
        context.policy = JaniceDetectLargest;

        JaniceMediaIterator ellen_famous = test::ellen_famous();
        REQUIRE(janice_detect_with_callback(&ellen_famous, &context, detection_callback, (void*) &biggest_tracks) == JANICE_SUCCESS);

        REQUIRE(ellen_famous.free(&ellen_famous) == JANICE_SUCCESS);
    }

    REQUIRE(biggest_tracks.size()== 1);
    REQUIRE(test::tracks_are_the_same(all_tracks[biggest_idx], biggest_tracks[0]));
}

/*
TEST_CASE("janice_detect_batch"
          * doctest::test_suite("janice_tests"))
{

}

TEST_CASE("janice_detect_batch_with_callback"
          * doctest::test_suite("janice_tests"))
{

}
*/
