#include <janice.h>
#include <janice_io_opencv.h>

#include <janice_test_utils.hpp>

#include <doctest/doctest.h>

// ----------------------------------------------------------------------------
// Gallery Constructor

TEST_CASE("janice_create_gallery"
          * doctest::test_suite("janice_tests"))
{
    JaniceTemplates tmpls = test::get_tmpls();
    const JaniceTemplateIds ids = test::get_ids(tmpls);

    JaniceGallery gallery = nullptr;
    REQUIRE(janice_create_gallery(&tmpls, &ids, &gallery) == JANICE_SUCCESS);

    REQUIRE(test::gallery_does_contain(gallery, tmpls, ids));

    // Don't free the templates or ids! They are cached and will be freed later
    REQUIRE(janice_free_gallery(&gallery) == JANICE_SUCCESS);
}

// ----------------------------------------------------------------------------
// Gallery Insert / Remove

TEST_CASE("janice_gallery_insert_remove"
          * doctest::test_suite("janice_tests"))
{
    JaniceTemplates tmpls = test::get_tmpls();
    JaniceTemplateIds ids = test::get_ids(tmpls);
   
    JaniceGallery gallery = test::create_empty_gallery();

    for (size_t i = 0; i < tmpls.length; ++i) {
        REQUIRE(janice_gallery_insert(gallery, tmpls.tmpls[i], ids.ids[i]) == JANICE_SUCCESS);
    }

    REQUIRE(test::gallery_does_contain(gallery, tmpls, ids));

    REQUIRE(janice_gallery_insert(gallery, tmpls.tmpls[0], ids.ids[0]) == JANICE_DUPLICATE_ID);

    for (size_t i = 0; i < tmpls.length; ++i) {
        REQUIRE(janice_gallery_remove(gallery, ids.ids[i]) == JANICE_SUCCESS);
    }

    REQUIRE(test::gallery_is_empty(gallery));

    REQUIRE(janice_gallery_remove(gallery, ids.ids[0]) == JANICE_MISSING_ID);

    // Don't free the templates or ids! They are cached and will be freed later
    REQUIRE(janice_free_gallery(&gallery) == JANICE_SUCCESS);
}

TEST_CASE("janice_gallery_insert_remove_batch"
          * doctest::test_suite("janice_tests"))
{
    JaniceContext context;
    REQUIRE(janice_init_default_context(&context) == JANICE_SUCCESS);

    const JaniceTemplates tmpls = test::get_tmpls();
    const JaniceTemplateIds ids = test::get_ids(tmpls);

    JaniceGallery gallery = test::create_empty_gallery();

    JaniceErrors errors;
    REQUIRE(janice_gallery_insert_batch(gallery, &tmpls, &ids, &context, &errors) == JANICE_SUCCESS);

    REQUIRE(test::gallery_does_contain(gallery, tmpls, ids));
    REQUIRE(janice_gallery_insert(gallery, tmpls.tmpls[0], ids.ids[0]) == JANICE_DUPLICATE_ID);

    REQUIRE(janice_gallery_remove_batch(gallery, &ids, &context, &errors) == JANICE_SUCCESS);

    REQUIRE(test::gallery_is_empty(gallery));
    REQUIRE(janice_gallery_remove(gallery, ids.ids[0]) == JANICE_MISSING_ID);

    // Don't free the templates or ids! They are cached and will be freed later
    REQUIRE(janice_free_gallery(&gallery) == JANICE_SUCCESS);
}

// ----------------------------------------------------------------------------
// Gallery Serialize / Deserialize

TEST_CASE("janice_gallery_serialize_deserialize"
          * doctest::test_suite("janice_tests"))
{
    const JaniceTemplates tmpls = test::get_tmpls();
    const JaniceTemplateIds ids = test::get_ids(tmpls);

    JaniceGallery gallery = nullptr;
    REQUIRE(janice_create_gallery(&tmpls, &ids, &gallery) == JANICE_SUCCESS);

    // Serialize the gallery
    uint8_t* buffer;
    size_t len;
    REQUIRE(janice_serialize_gallery(gallery, &buffer, &len) == JANICE_SUCCESS);

    // Deserialize the gallery it should be the same!
    JaniceGallery new_gallery;
    REQUIRE(janice_deserialize_gallery(buffer, len, &new_gallery) == JANICE_SUCCESS);

    REQUIRE(janice_free_buffer(&buffer) == JANICE_SUCCESS);

    REQUIRE(test::galleries_are_the_same(gallery, new_gallery));

    // Don't free the templates or ids! They are cached and will be freed later
    REQUIRE(janice_free_gallery(&gallery) == JANICE_SUCCESS);
    REQUIRE(janice_free_gallery(&new_gallery) == JANICE_SUCCESS);
}

// ----------------------------------------------------------------------------
// Gallery Read / Write

TEST_CASE("janice_gallery_read_write"
          * doctest::test_suite("janice_tests"))
{
    const JaniceTemplates tmpls = test::get_tmpls();
    const JaniceTemplateIds ids = test::get_ids(tmpls);

    JaniceGallery gallery = nullptr;
    REQUIRE(janice_create_gallery(&tmpls, &ids, &gallery) == JANICE_SUCCESS);

    // Write the gallery to disk
    REQUIRE(janice_write_gallery(gallery, (test::constants::test_dir + "/gallery.bin").c_str()) == JANICE_SUCCESS);

    // Read the gallery it should be the same!
    JaniceGallery new_gallery;
    REQUIRE(janice_read_gallery((test::constants::test_dir + "/gallery.bin").c_str(), &new_gallery) == JANICE_SUCCESS);

    REQUIRE(test::galleries_are_the_same(gallery, new_gallery));

    // Don't free the templates or ids! They are cached and will be freed later
    REQUIRE(janice_free_gallery(&gallery) == JANICE_SUCCESS);
    REQUIRE(janice_free_gallery(&new_gallery) == JANICE_SUCCESS);
}
