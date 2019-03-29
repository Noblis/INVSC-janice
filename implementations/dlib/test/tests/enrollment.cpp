#include <janice.h>
#include <janice_io_opencv.h>

#include <janice_test_utils.hpp>

#include <doctest/doctest.h>


// ----------------------------------------------------------------------------
// Template Constructor Tests

TEST_CASE("janice_enroll_from_media"
          * doctest::test_suite("janice_tests"))
{
    JaniceContext context;
    REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

    JaniceMediaIterator lena1 = test::lena1();

    JaniceTemplates tmpls;
    JaniceDetections detections;
    REQUIRE(janice_enroll_from_media(&lena1, &context, &tmpls, &detections) == JANICE_SUCCESS);

    REQUIRE(tmpls.length == 1);
    REQUIRE(detections.length == 1);

    REQUIRE(janice_clear_templates(&tmpls) == JANICE_SUCCESS);
    REQUIRE(janice_clear_detections(&detections) == JANICE_SUCCESS);

    REQUIRE(lena1.free(&lena1) == JANICE_SUCCESS);
}

TEST_CASE("janice_enroll_from_media_with_callback"
          * doctest::test_suite("janice_tests"))
{

}

TEST_CASE("janice_enroll_from_media_batch"
          * doctest::test_suite("janice_tests"))
{
    JaniceContext context;
    REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

    JaniceMediaIterators its;
    its.length = 1;
    its.media = new JaniceMediaIterator[1];

    its.media[0] = test::lena1();

    JaniceTemplatesGroup tmpls;
    JaniceDetectionsGroup detections;
    JaniceErrors errors;
    REQUIRE(janice_enroll_from_media_batch(&its, &context, &tmpls, &detections, &errors) == JANICE_SUCCESS);

    REQUIRE(tmpls.length == 1);
    REQUIRE(detections.length == 1);
    REQUIRE(errors.length == 1);

    REQUIRE(tmpls.group[0].length == 1);
    REQUIRE(detections.group[0].length == 1);

    REQUIRE(janice_clear_templates_group(&tmpls) == JANICE_SUCCESS);
    REQUIRE(janice_clear_detections_group(&detections) == JANICE_SUCCESS);
    REQUIRE(janice_clear_errors(&errors) == JANICE_SUCCESS);

    REQUIRE(its.media[0].free(&its.media[0]) == JANICE_SUCCESS);
    delete[] its.media;
}

TEST_CASE("janice_enroll_from_detections"
          * doctest::test_suite("janice_tests"))
{
    JaniceContext context;
    REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

    JaniceMediaIterators its;
    its.length = 1;
    its.media = new JaniceMediaIterator[1];
    its.media[0] = test::lena1();

    JaniceDetections detections;
    detections.length = 1;
    detections.detections = new JaniceDetection[1];

    { // Pick random coordinates
        JaniceRect rect;
        rect.x = 17;
        rect.y = 28;
        rect.width = 111;
        rect.height = 256;

        REQUIRE(janice_create_detection_from_rect(&its.media[0], &rect, 0, &detections.detections[0]) == JANICE_SUCCESS);

        its.media[0].reset(&its.media[0]);
    }

    JaniceTemplate tmpl;
    REQUIRE(janice_enroll_from_detections(&its, &detections, &context, &tmpl) == JANICE_SUCCESS);

    REQUIRE(tmpl->fv.nr() == 128);

    REQUIRE(its.media[0].free(&its.media[0]) == JANICE_SUCCESS);
    delete[] its.media;

    REQUIRE(janice_free_detection(&detections.detections[0]) == JANICE_SUCCESS);
    delete[] detections.detections;
}

TEST_CASE("janice_enroll_from_detections_batch"
          * doctest::test_suite("janice_tests"))
{

}

TEST_CASE("janice_enroll_from_detections_batch_with_callback"
          * doctest::test_suite("janice_tests"))
{
    auto enrollment_callback = [](const JaniceTemplate* tmpl, size_t, void* user_data) {
        int fte = 1; // default to true to make sure it's being changed
        REQUIRE(janice_template_is_fte(*tmpl, &fte) == JANICE_SUCCESS);
        REQUIRE(fte == 0);

        int* tmpl_count = (int*) user_data;
        *tmpl_count += 1;

        return JANICE_SUCCESS;
    };
}

TEST_CASE("janice_template_get_attribute"
          * doctest::test_suite("janice_tests"))
{

}

TEST_CASE("janice_template_get_feature_vector"
          * doctest::test_suite("janice_tests"))
{
    JaniceTemplates tmpls = test::get_tmpls();

    for (size_t i = 0; i < tmpls.length; ++i) {
        JaniceFeatureVectorType fv_type;
        void* fv = nullptr;
        size_t len;
        REQUIRE(janice_template_get_feature_vector(tmpls.tmpls[i], &fv_type, &fv, &len) == JANICE_SUCCESS);

        REQUIRE(fv_type == JaniceFloat);
        REQUIRE(fv != nullptr);
        REQUIRE(len == 128);

        REQUIRE(janice_free_feature_vector(&fv) == JANICE_SUCCESS);
    }
}

// ----------------------------------------------------------------------------
// Template I/O tests

TEST_CASE("janice_template_serialize_deserialize"
          * doctest::test_suite("janice_tests"))
{
    JaniceTemplates tmpls = test::get_tmpls();

    // Serialize / Deserialize
    for (size_t i = 0; i < tmpls.length; ++i) {
        // Serialize
        uint8_t* buffer = nullptr;
        size_t len;
        REQUIRE(janice_serialize_template(tmpls.tmpls[i], &buffer, &len) == JANICE_SUCCESS);

        // Deserialize
        JaniceTemplate new_tmpl;
        REQUIRE(janice_deserialize_template(buffer, len, &new_tmpl) == JANICE_SUCCESS);

        REQUIRE(test::templates_are_the_same(tmpls.tmpls[i], new_tmpl));

        REQUIRE(janice_free_buffer(&buffer) == JANICE_SUCCESS);
        REQUIRE(janice_free_template(&new_tmpl) == JANICE_SUCCESS);
    }
}

TEST_CASE("janice_template_read_write"
          * doctest::test_suite("janice_tests"))
{
    JaniceTemplates tmpls = test::get_tmpls();

    // Read / Write
    for (size_t i = 0; i < tmpls.length; ++i) {
        // Write
        REQUIRE(janice_write_template(tmpls.tmpls[i], (test::constants::test_dir + "/template.bin").c_str()) == JANICE_SUCCESS);

        // Deserialize
        JaniceTemplate new_tmpl;
        REQUIRE(janice_read_template((test::constants::test_dir + "/template.bin").c_str(), &new_tmpl) == JANICE_SUCCESS);

        REQUIRE(test::templates_are_the_same(tmpls.tmpls[i], new_tmpl));

        REQUIRE(janice_free_template(&new_tmpl) == JANICE_SUCCESS);
    }
}
