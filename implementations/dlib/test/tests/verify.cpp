#include <janice.h>
#include <janice_io_opencv.h>

#include <janice_test_utils.hpp>

#include <doctest/doctest.h>


TEST_CASE("janice_verify"
          * doctest::test_suite("janice_tests"))
{
    JaniceTemplates background_celebs = test::get_tmpls();
    JaniceTemplates lenas             = test::get_lena_tmpls();

    // We expect the similarity between the 2 lena images to be higher than
    // either lena compared to a random celebrity
    std::vector<double> background_scores;
    for (size_t i = 0; i < background_celebs.length; ++i) {
        double similarity;
        REQUIRE(janice_verify(lenas.tmpls[0], background_celebs.tmpls[i], &similarity) == JANICE_SUCCESS);
        background_scores.push_back(similarity);

        REQUIRE(janice_verify(lenas.tmpls[1], background_celebs.tmpls[i], &similarity) == JANICE_SUCCESS);
        background_scores.push_back(similarity);
    }

    double lena_score;
    REQUIRE(janice_verify(lenas.tmpls[0], lenas.tmpls[1], &lena_score) == JANICE_SUCCESS);

    bool lena_is_the_highest = true;
    for (double background_score : background_scores) {
        if (background_score > lena_score) {
            lena_is_the_highest = false;
            break;
        }
    }

    REQUIRE(lena_is_the_highest);
}

TEST_CASE("janice_verify_batch"
          * doctest::test_suite("janice_tests"))
{
    JaniceContext context;
    REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

    JaniceTemplates background_celebs = test::get_tmpls();
    JaniceTemplates lenas             = test::get_lena_tmpls();

    // We expect the similarity between the 2 lena images to be higher than
    // either lena compared to a random celebrity
    std::vector<double> background_scores;

    { // lena1 batch
        JaniceTemplates repeated_lena1;
        repeated_lena1.length = background_celebs.length;
        repeated_lena1.tmpls = new JaniceTemplate[repeated_lena1.length];
        for (size_t i = 0; i < background_celebs.length; ++i) {
            repeated_lena1.tmpls[i] = lenas.tmpls[0];
        }

        JaniceSimilarities similarities;
        JaniceErrors errors;
        REQUIRE(janice_verify_batch(&repeated_lena1, &background_celebs, &context, &similarities, &errors) == JANICE_SUCCESS);

        REQUIRE(similarities.length == background_celebs.length);
        REQUIRE(errors.length == background_celebs.length);

        for (size_t i = 0; i < similarities.length; ++i) {
            background_scores.push_back(similarities.similarities[i]);
        }

        delete[] repeated_lena1.tmpls;
        REQUIRE(janice_clear_similarities(&similarities) == JANICE_SUCCESS);
        REQUIRE(janice_clear_errors(&errors) == JANICE_SUCCESS);
    }

    { // lena2 batch
        JaniceTemplates repeated_lena2;
        repeated_lena2.length = background_celebs.length;
        repeated_lena2.tmpls = new JaniceTemplate[repeated_lena2.length];
        for (size_t i = 0; i < background_celebs.length; ++i) {
            repeated_lena2.tmpls[i] = lenas.tmpls[1];
        }

        JaniceSimilarities similarities;
        JaniceErrors errors;
        REQUIRE(janice_verify_batch(&repeated_lena2, &background_celebs, &context, &similarities, &errors) == JANICE_SUCCESS);

        REQUIRE(similarities.length == background_celebs.length);
        REQUIRE(errors.length == background_celebs.length);

        for (size_t i = 0; i < similarities.length; ++i) {
            background_scores.push_back(similarities.similarities[i]);
        }

        delete[] repeated_lena2.tmpls;
        REQUIRE(janice_clear_similarities(&similarities) == JANICE_SUCCESS);
        REQUIRE(janice_clear_errors(&errors) == JANICE_SUCCESS);
    }

    double lena_score;
    REQUIRE(janice_verify(lenas.tmpls[0], lenas.tmpls[1], &lena_score) == JANICE_SUCCESS);

    bool lena_is_the_highest = true;
    for (double background_score : background_scores) {
        if (background_score > lena_score) {
            lena_is_the_highest = false;
            break;
        }
    }

    REQUIRE(lena_is_the_highest);
}
