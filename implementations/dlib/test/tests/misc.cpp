#include <janice.h>
#include <janice_io_opencv.h>

#include <doctest/doctest.h>

#include <limits>

TEST_CASE("janice_set_log_level"
          * doctest::test_suite("janice_tests"))
{
    REQUIRE(janice_set_log_level(JaniceLogDebug) == JANICE_SUCCESS);
    REQUIRE(janice_set_log_level(JaniceLogInfo) == JANICE_SUCCESS);
    REQUIRE(janice_set_log_level(JaniceLogWarning) == JANICE_SUCCESS);
    REQUIRE(janice_set_log_level(JaniceLogError) == JANICE_SUCCESS);
    REQUIRE(janice_set_log_level(JaniceLogCritical) == JANICE_SUCCESS);
}

TEST_CASE("janice_version"
          * doctest::test_suite("janice_tests"))
{
    uint32_t major, minor, patch;
    REQUIRE(janice_api_version(&major, &minor, &patch) == JANICE_SUCCESS);
    printf("API: major: %u minor: %u patch: %u\n", major, minor, patch);
    REQUIRE(major == 7);
    REQUIRE(minor == 0);
    REQUIRE(patch == 0);

    REQUIRE(janice_sdk_version(&major, &minor, &patch) == JANICE_SUCCESS);
    printf("SDK: major: %u minor: %u patch: %u\n", major, minor, patch);

    REQUIRE(major == 1);
    REQUIRE(minor == 0);
    REQUIRE(patch == 0);
}

TEST_CASE("janice_configuration"
          * doctest::test_suite("janice_tests"))
{
    JaniceConfiguration config;
    REQUIRE(janice_get_current_configuration(&config) == JANICE_SUCCESS);

    REQUIRE(config.values == nullptr);
    REQUIRE(config.length == 0);

    REQUIRE(janice_clear_configuration(&config) == JANICE_SUCCESS);
}

TEST_CASE("janice_context"
          * doctest::test_suite("janice_tests"))
{
    JaniceContext context;
    REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

    REQUIRE(context.policy == JaniceDetectAll);
    REQUIRE(context.min_object_size == 36);
    REQUIRE(context.role == Janice1NProbe);
    REQUIRE(context.threshold == 0.6);
    REQUIRE(context.max_returns == 50);
    REQUIRE(context.hint == 0.5);
    REQUIRE(context.batch_policy == JaniceFlagAndFinish);
}
