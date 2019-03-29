#include <janice.h>
#include <janice_io_opencv.h>

#include <janice_test_utils.hpp>

#include <doctest/doctest.h>

// ----------------------------------------------------------------------------
// Media Cluster Tests
TEST_CASE("janice_cluster_media"
          * doctest::test_suite("janice_tests"))
{
    JaniceMediaIterators its;
    its.length = 3;
    its.media = new JaniceMediaIterator[3];

    its.media[0] = test::lena1();
    its.media[1] = test::lena2();
    its.media[2] = test::ellen_famous();

    JaniceContext context;
    REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

    JaniceDetectionsGroup detections;
    JaniceClusterIdsGroup grouped_cluster_ids;
    JaniceClusterConfidencesGroup grouped_cluster_confidences;
    REQUIRE(janice_cluster_media(&its, &context, &grouped_cluster_ids, &grouped_cluster_confidences, &detections) == JANICE_SUCCESS);

    REQUIRE(detections.length == its.length);
    REQUIRE(grouped_cluster_ids.length == its.length);
    REQUIRE(grouped_cluster_confidences.length == its.length);

    for (size_t i = 0; i < its.length; ++i) {
        int expected_length = i < 2 ? 1 : 10;
        REQUIRE(detections.group[i].length == expected_length);
        REQUIRE(grouped_cluster_ids.group[i].length == expected_length);
        REQUIRE(grouped_cluster_confidences.group[i].length == expected_length);
    }

    for (size_t i = 0; i < its.length; ++i) {
        REQUIRE(its.media[i].free(&its.media[i]) == JANICE_SUCCESS);
    }
    delete[] its.media;

    REQUIRE(janice_clear_detections_group(&detections) == JANICE_SUCCESS);
    REQUIRE(janice_clear_cluster_ids_group(&grouped_cluster_ids) == JANICE_SUCCESS);
    REQUIRE(janice_clear_cluster_confidences_group(&grouped_cluster_confidences) == JANICE_SUCCESS);
}

// ----------------------------------------------------------------------------
// Template Cluster Tests
TEST_CASE("janice_cluster_templates"
          * doctest::test_suite("janice_tests"))
{
    JaniceContext context;
    REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

    JaniceTemplates tmpls = test::get_tmpls();
    
    JaniceClusterIds cluster_ids;
    JaniceClusterConfidences cluster_confidences;
    REQUIRE(janice_cluster_templates(&tmpls, &context, &cluster_ids, &cluster_confidences) == JANICE_SUCCESS);

    REQUIRE(cluster_ids.length == tmpls.length);
    REQUIRE(cluster_confidences.length == tmpls.length);

    REQUIRE(janice_clear_cluster_ids(&cluster_ids) == JANICE_SUCCESS);
    REQUIRE(janice_clear_cluster_confidences(&cluster_confidences) == JANICE_SUCCESS);
}  
