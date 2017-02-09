#include <janice_io.h>

#include <string>
#include <cstring>

// ----------------------------------------------------------------------------
// Helpful macros for repeated checks

#define JANICE_CALL(func, cleanup)             \
{                                              \
    JaniceError error = func;                  \
    if (error != JANICE_SUCCESS) {             \
        printf("\nError Detected!"             \
               "\n\tLocation: %s:%d"           \
               "\n\tError: %s\n",              \
               __FILE__, __LINE__,             \
               janice_error_to_string(error)); \
        cleanup();                             \
        return 1;                              \
    }                                          \
}

#define CHECK(condition, msg, cleanup)         \
{                                              \
    bool ret = (condition);                    \
    if (!ret) {                                \
        printf("\nCheck Failed!"               \
               "\n\tLocation: %s:%d"           \
               "\n\tMessage: %s\n",            \
               __FILE__, __LINE__, msg);       \
        cleanup();                             \
        return 1;                              \
    }                                          \
}

using namespace std;

// ----------------------------------------------------------------------------
// Check basic media properties

int check_media_basics(JaniceConstMedia media)
{
    CHECK(strncmp(media->filename, "media/test_image.png", 20) == 0, // condition
          "Media filename != 'media/test_image.png'", // message
          [](){}) // cleanup function

    CHECK(media->category == Image,
          "Media category != Image",
          [](){})

    CHECK(media->channels == 3,
          "Media channels != 3",
          [](){})

    CHECK(media->rows == 300,
          "Media rows != 300",
          [](){})

    CHECK(media->cols == 300,
          "Media columns != 300",
          [](){})
    CHECK(media->frames == 1,
          "Media frames != 1",
          [](){})

    return 0;
}

// ----------------------------------------------------------------------------
// Check media iterator

int check_media_iterator(JaniceConstMedia media)
{
    JaniceMediaIterator it = nullptr;
    JANICE_CALL(janice_media_get_iterator(media, &it),
                // Cleanup
                [](){})

    // Variables to be filled during iterator calls
    JaniceImage image = nullptr;
    uint32_t frame;

    CHECK(janice_media_it_next(it, &image) == JANICE_MEDIA_AT_END,
          "Calling next on a media iterator for an image should return JANICE_MEDIA_AT_END",
          // Cleanup
          [&]() {
              janice_free_media_iterator(&it);
          })

    // We will check the actual image later. Just delete it now
    JANICE_CALL(janice_free_image(&image),
                // Cleanup
                [&]() {
                    janice_free_media_iterator(&it);
                })

    CHECK(janice_media_it_seek(it, 100) == JANICE_INVALID_MEDIA,
          "Calling seek on a media iterator for an image should always return JANICE_INVALID_MEDIA",
          // Cleanup
          [&]() {
            janice_free_media_iterator(&it);
          })

    CHECK(janice_media_it_get(it, &image, 100) == JANICE_INVALID_MEDIA,
          "Calling get on a media iterator for an image should always return JANICE_INVALID_MEDIA",
          // Cleanup
          [&]() {
            janice_free_media_iterator(&it);
          })

    CHECK(janice_media_it_tell(it, &frame) == JANICE_INVALID_MEDIA,
          "Calling tell on a media iterator for an image should always return JANICE_INVALID_MEDIA",
          // Cleanup
          [&]() {
            janice_free_media_iterator(&it);
          })

    return 0;
}

// ----------------------------------------------------------------------------
// Check image pixel values

static inline int check_pixel(JaniceConstImage image,
                               uint32_t x,
                               uint32_t y,
                               uint8_t red,
                               uint8_t green,
                               uint8_t blue)
{
    // Variable to hold pixel values
    uint8_t pixel;

    // Get the blue pixel first, remember OpenCV stores images in BGR order
    JANICE_CALL(janice_image_access(image,
                                    0, // channel
                                    y, // row
                                    x, // col
                                    &pixel),
                // Cleanup
                [](){})
    CHECK(pixel == blue,
          "Blue pixel doesn't match.",
          [](){})

    // Get the green pixel
    JANICE_CALL(janice_image_access(image,
                                    1, // channel
                                    y, // row
                                    x, // col
                                    &pixel),
                // Cleanup
                [](){})
    CHECK(pixel == green,
          "Green pixel doesn't match.",
          [](){})

    // Get the red pixel
    JANICE_CALL(janice_image_access(image,
                                    2, // channel
                                    y, // row
                                    x, // col
                                    &pixel),
                // Cleanup
                [](){})
    CHECK(pixel == red,
          "Red pixel doesn't match.",
          [](){})

    return 0;
}

int check_media_pixel_values(JaniceConstMedia media)
{
    JaniceMediaIterator it = nullptr;
    JANICE_CALL(janice_media_get_iterator(media, &it),
                // Cleanup
                [](){})

    // Variables to be filled during iterator calls
    JaniceImage image = nullptr;

    CHECK(janice_media_it_next(it, &image) == JANICE_MEDIA_AT_END,
          "Calling next on a media iterator for an image should return JANICE_MEDIA_AT_END",
          // Cleanup
          [&]() {
              janice_free_media_iterator(&it);
          })

    // Utility function to free image and iterator memory if a check fails
    auto cleanup = [&]() {
        janice_free_image(&image);
        janice_free_media_iterator(&it);
    };

    // Our test image is a 3x3 grid of 100px x 100px blocks, each of which
    // is a different, solid, color.
    CHECK(check_pixel(image,  50,  50,   0,   0, 255) == 0,
          "Pixel mismatch",
          cleanup)
    CHECK(check_pixel(image, 150,  50,   0, 255,   0) == 0,
          "Pixel mismatch",
          cleanup)
    CHECK(check_pixel(image, 250,  50, 255,   0,   0) == 0,
          "Pixel mismatch",
          cleanup)
    CHECK(check_pixel(image,  50, 150,   0, 127, 127) == 0,
          "Pixel mismatch",
          cleanup)
    CHECK(check_pixel(image, 150, 150, 127, 127,   0) == 0,
          "Pixel mismatch",
          cleanup)
    CHECK(check_pixel(image, 250, 150, 127,   0, 127) == 0,
          "Pixel mismatch",
          cleanup)
    CHECK(check_pixel(image,  50, 250,   0,   0,   0) == 0,
          "Pixel mismatch",
          cleanup)
    CHECK(check_pixel(image, 150, 250, 127, 127, 127) == 0,
          "Pixel mismatch",
          cleanup)
    CHECK(check_pixel(image, 250, 250, 255, 255, 255) == 0,
          "Pixel mismatch",
          cleanup)

    cleanup();

    return 0;
}

// ----------------------------------------------------------------------------
// Main test function

int main(int, char*[])
{
    const string test_image = "media/test_image.png";

    // Create a media object from our test image
    JaniceMedia media = nullptr;
    JANICE_CALL(janice_create_media(test_image.c_str(),
                                    &media),
                 // Cleanup
                 [](){})

    // Check basic image properties
    if (check_media_basics(media) == 1) {
        janice_free_media(&media);
        return 1;
    }

    // Check that an iterator can be created and its functions work as expected
    // for an image
    if (check_media_iterator(media) == 1) {
        janice_free_media(&media);
        return 1;
    }

    // Check the loaded pixel values of the test image
    if (check_media_pixel_values(media) == 1) {
        janice_free_media(&media);
        return 1;
    }

    janice_free_media(&media);

    return 0;
}
