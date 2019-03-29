#include <janice.h>
#include <janice_io_opencv.h>

#include <janice_test_utils.hpp>

#include <doctest/doctest.h>

#include <limits>

// ----------------------------------------------------------------------------
// Search

TEST_CASE("janice_search"
          * doctest::test_suite("janice_tests"))
{
    JaniceContext context;
    REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

    JaniceTemplates tmpls = test::get_tmpls();
    JaniceTemplateIds ids = test::get_ids(tmpls);

    JaniceGallery gallery = nullptr;
    REQUIRE(janice_create_gallery(&tmpls, &ids, &gallery) == JANICE_SUCCESS);

    JaniceTemplates lenas = test::get_lena_tmpls();
    uint64_t lena2_id = 100; // random unique offset
    REQUIRE(janice_gallery_insert(gallery, lenas.tmpls[1], lena2_id) == JANICE_SUCCESS);

    int test_k = 5; // Return top half of data
    float test_threshold = 0.2; // Arbitrary threshold in the middle of the data

    { // All returns
        context.max_returns = 0;
        context.threshold = -std::numeric_limits<float>::max();

        JaniceSimilarities search_scores;
        JaniceTemplateIds  search_ids;
        REQUIRE(janice_search(lenas.tmpls[0], gallery, &context, &search_scores, &search_ids) == JANICE_SUCCESS);

        REQUIRE(search_scores.length == tmpls.length + 1); // + 1 for the lena we added
        REQUIRE(search_ids.length == ids.length + 1);

        // Lena 2 should be the first match
        REQUIRE(search_ids.ids[0] == lena2_id);

        REQUIRE(janice_clear_similarities(&search_scores) == JANICE_SUCCESS);
        REQUIRE(janice_clear_template_ids(&search_ids) == JANICE_SUCCESS);
    }

    { // All returns above threshold
        context.max_returns = 0;
        context.threshold = test_threshold;

        JaniceSimilarities search_scores;
        JaniceTemplateIds  search_ids;
        REQUIRE(janice_search(lenas.tmpls[0], gallery, &context, &search_scores, &search_ids) == JANICE_SUCCESS);

        REQUIRE(search_scores.length == search_ids.length);

        // Lena 2 should be the first match
        REQUIRE(search_ids.ids[0] == lena2_id);

        // Every score should be above test_threshold
        for (size_t i = 0; i < search_ids.length; ++i) {
            REQUIRE(search_scores.similarities[i] >= test_threshold);
        }

        REQUIRE(janice_clear_similarities(&search_scores) == JANICE_SUCCESS);
        REQUIRE(janice_clear_template_ids(&search_ids) == JANICE_SUCCESS);
    }


    { // Top k returns
        context.max_returns = test_k;
        context.threshold = -std::numeric_limits<double>::max(); // All scores should be above this

        JaniceSimilarities search_scores;
        JaniceTemplateIds  search_ids;
        REQUIRE(janice_search(lenas.tmpls[0], gallery, &context, &search_scores, &search_ids) == JANICE_SUCCESS);

        REQUIRE(search_scores.length == test_k);
        REQUIRE(search_ids.length == test_k);

        // Lena 2 should be the first match
        REQUIRE(search_ids.ids[0] == lena2_id);

        REQUIRE(janice_clear_similarities(&search_scores) == JANICE_SUCCESS);
        REQUIRE(janice_clear_template_ids(&search_ids) == JANICE_SUCCESS);
    }

    { // Top k returns above threshold
        context.max_returns = test_k;
        context.threshold = test_threshold;

        JaniceSimilarities search_scores;
        JaniceTemplateIds  search_ids;
        REQUIRE(janice_search(lenas.tmpls[0], gallery, &context, &search_scores, &search_ids) == JANICE_SUCCESS);

        REQUIRE(search_scores.length == search_ids.length);
        REQUIRE(search_scores.length <= test_k);
        REQUIRE(search_ids.length <= test_k);

        // Lena 2 should be the first match
        REQUIRE(search_ids.ids[0] == lena2_id);

        for (size_t i = 0; i < search_ids.length; ++i) {
            REQUIRE(search_scores.similarities[i] > test_threshold);
        }

        REQUIRE(janice_clear_similarities(&search_scores) == JANICE_SUCCESS);
        REQUIRE(janice_clear_template_ids(&search_ids) == JANICE_SUCCESS);
    }

    REQUIRE(janice_free_gallery(&gallery) == JANICE_SUCCESS);
}

/*
// ----------------------------------------------------------------------------
TEST_CASE("janice_search_batch"
          * doctest::test_suite("janice_tests"))
{
    JaniceContext context = val::create_context_from_config(val::test_config(), JaniceDetectAll, Janice1NProbe);

    JaniceTemplates tmpls = val::get_tmpls();
    JaniceTemplateIds ids = val::get_ids(tmpls);

    JaniceGallery gallery = nullptr;
    REQUIRE(janice_create_gallery(&tmpls, &ids, &gallery) == JANICE_SUCCESS);

    int test_k = 2; // Return top half of data
    float test_threshold = -1.1; // Arbitrary threshold in the middle of the data

    { // All returns
        context.max_returns = 0;
        context.threshold = -std::numeric_limits<float>::max();

        JaniceSimilaritiesGroup search_scores_group;
        JaniceTemplateIdsGroup  search_ids_group;
        JaniceErrors errors;
        REQUIRE(janice_search_batch(&tmpls, gallery, &context, &search_scores_group, &search_ids_group, &errors) == JANICE_SUCCESS);

        REQUIRE(search_scores_group.length == tmpls.length);
        REQUIRE(search_ids_group.length == tmpls.length);
        REQUIRE(errors.length == tmpls.length);

        for (size_t i = 0; i < tmpls.length; ++i) {
            JaniceSimilarities search_scores = search_scores_group.group[i];
            JaniceTemplateIds  search_ids    = search_ids_group.group[i];

            for (size_t j = 0; j < search_ids.length; ++j) {
                REQUIRE(val::l2_norm_matches.at(val::key(ids.ids[i], search_ids.ids[j]))
                            == doctest::Approx(search_scores.similarities[j]).epsilon(.005));
            }
        }

        REQUIRE(janice_clear_similarities_group(&search_scores_group) == JANICE_SUCCESS);
        REQUIRE(janice_clear_template_ids_group(&search_ids_group) == JANICE_SUCCESS);
        REQUIRE(janice_clear_errors(&errors) == JANICE_SUCCESS);
    }

    { // All returns above threshold
        context.max_returns = 0;
        context.threshold = test_threshold;

        JaniceSimilaritiesGroup search_scores_group;
        JaniceTemplateIdsGroup  search_ids_group;
        JaniceErrors errors;
        REQUIRE(janice_search_batch(&tmpls, gallery, &context, &search_scores_group, &search_ids_group, &errors) == JANICE_SUCCESS);

        REQUIRE(search_scores_group.length == tmpls.length);
        REQUIRE(search_ids_group.length == tmpls.length);
        REQUIRE(errors.length == tmpls.length);

        for (size_t i = 0; i < tmpls.length; ++i) {
            REQUIRE(errors.errors[i] == JANICE_SUCCESS);

            JaniceSimilarities search_scores = search_scores_group.group[i];
            JaniceTemplateIds  search_ids    = search_ids_group.group[i];

            REQUIRE(search_scores.length == search_ids.length);

            for (size_t j = 0; j < search_ids.length; ++j) {
                REQUIRE(search_scores.similarities[j] >= test_threshold);
                REQUIRE(val::l2_norm_matches.at(val::key(ids.ids[i], search_ids.ids[j]))
                            == doctest::Approx(search_scores.similarities[j]).epsilon(0.005));
            }
        }

        REQUIRE(janice_clear_similarities_group(&search_scores_group) == JANICE_SUCCESS);
        REQUIRE(janice_clear_template_ids_group(&search_ids_group) == JANICE_SUCCESS);
        REQUIRE(janice_clear_errors(&errors) == JANICE_SUCCESS);
    }

    { // Top k returns
        context.max_returns = test_k;
        context.threshold = -std::numeric_limits<double>::max();

        JaniceSimilaritiesGroup search_scores_group;
        JaniceTemplateIdsGroup  search_ids_group;
        JaniceErrors errors;
        REQUIRE(janice_search_batch(&tmpls, gallery, &context, &search_scores_group, &search_ids_group, &errors) == JANICE_SUCCESS);

        REQUIRE(search_scores_group.length == tmpls.length);
        REQUIRE(search_ids_group.length == tmpls.length);
        REQUIRE(errors.length == tmpls.length);

        for (size_t i = 0; i < tmpls.length; ++i) {
            REQUIRE(errors.errors[i] == JANICE_SUCCESS);

            JaniceSimilarities search_scores = search_scores_group.group[i];
            JaniceTemplateIds  search_ids    = search_ids_group.group[i];

            REQUIRE(search_scores.length == test_k);
            REQUIRE(search_ids.length == test_k);

            for (size_t j = 0; j < search_ids.length; ++j) {
                REQUIRE(val::l2_norm_matches.at(val::key(ids.ids[i], search_ids.ids[j]))
                            == doctest::Approx(search_scores.similarities[j]).epsilon(0.005));
            }
        }

        REQUIRE(janice_clear_similarities_group(&search_scores_group) == JANICE_SUCCESS);
        REQUIRE(janice_clear_template_ids_group(&search_ids_group) == JANICE_SUCCESS);
        REQUIRE(janice_clear_errors(&errors) == JANICE_SUCCESS);
    }

    { // Top k returns above threshold
        context.max_returns = test_k;
        context.threshold = test_threshold;

        JaniceSimilaritiesGroup search_scores_group;
        JaniceTemplateIdsGroup  search_ids_group;
        JaniceErrors errors;
        REQUIRE(janice_search_batch(&tmpls, gallery, &context, &search_scores_group, &search_ids_group, &errors) == JANICE_SUCCESS);

        REQUIRE(search_scores_group.length == tmpls.length);
        REQUIRE(search_ids_group.length == tmpls.length);
        REQUIRE(errors.length == tmpls.length);

        for (size_t i = 0; i < tmpls.length; ++i) {
            REQUIRE(errors.errors[i] == JANICE_SUCCESS);

            JaniceSimilarities search_scores = search_scores_group.group[i];
            JaniceTemplateIds  search_ids    = search_ids_group.group[i];

            REQUIRE(search_scores.length <= test_k);
            REQUIRE(search_ids.length <= test_k);

            for (size_t j = 0; j < search_ids.length; ++j) {
                REQUIRE(search_scores.similarities[j] > test_threshold);
                REQUIRE(val::l2_norm_matches.at(val::key(ids.ids[i], search_ids.ids[j]))
                            == doctest::Approx(search_scores.similarities[j]).epsilon(0.005));
            }
        }

        REQUIRE(janice_clear_similarities_group(&search_scores_group) == JANICE_SUCCESS);
        REQUIRE(janice_clear_template_ids_group(&search_ids_group) == JANICE_SUCCESS);
        REQUIRE(janice_clear_errors(&errors) == JANICE_SUCCESS);
    }

    REQUIRE(janice_free_gallery(&gallery) == JANICE_SUCCESS);
}
*/
