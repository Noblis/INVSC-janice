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
    CHECK(strcmp(media->filename, "media/second_test.mp4") == 0, // condition
          "Media filename != 'media/test_video.mp4'", // message
          [](){}) // cleanup function

    CHECK(media->category == Video,
          "Media category != Video",
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

    CHECK(media->frames == 90,
          "Media frames != 90",
          [](){})

    return 0;
}

// ----------------------------------------------------------------------------
// Check media iterator

// Check the iterator next and tell functions
int check_media_iterator_next(JaniceMediaIterator it)
{
    JaniceImage image = nullptr;
    uint32_t frame, frame_count = 0;

    while (true) {
        JaniceError err = janice_media_it_next(it, &image);

        JaniceError expected = JANICE_SUCCESS;
        ++frame_count;

        // On the last frame we loop back to the beginning
        if (frame_count == 90) {
            expected = JANICE_MEDIA_AT_END;
            frame_count = 0;
        }

        CHECK(err == expected,
              "Next should return JANICE_SUCCESS except for on the last frame",
              [](){})

        // Next, query the iterator for it's internal frame count
        JANICE_CALL(janice_media_it_tell(it, &frame),
                    // Cleanup
                    [&]() {
                        janice_free_image(&image);
                    })
        printf("frame: %u frame_count: %u\n", frame, frame_count);
        // Confirm the queried results match the excepted
        //CHECK(frame == frame_count,
        //      "Queried frame doesn't match expected",
        //      [&]() {
        //        janice_free_image(&image);
        //      })

        JANICE_CALL(janice_free_image(&image),
                    // Cleanup
                    [](){})

        if (expected == JANICE_MEDIA_AT_END)
            break;
    }

    return 0;
}

#define CHECKED_RANDOM_SEEK(desired, actual)      \
JANICE_CALL(janice_media_it_seek(it, desired),    \
            [](){})                               \
JANICE_CALL(janice_media_it_tell(it, &actual),    \
            [](){})                               \
CHECK(desired == actual,                          \
      "Seeked frame does not match actual frame", \
      [](){})

// Check the iterator seek and tell functions
int check_media_iterator_seek(JaniceMediaIterator it)
{
    uint32_t frame;

    // Let's do some random seeks
    CHECKED_RANDOM_SEEK( 0, frame)
    CHECKED_RANDOM_SEEK( 8, frame)
    CHECKED_RANDOM_SEEK(15, frame)
    CHECKED_RANDOM_SEEK(33, frame)
    CHECKED_RANDOM_SEEK(47, frame)
    CHECKED_RANDOM_SEEK(58, frame)
    CHECKED_RANDOM_SEEK(74, frame)
    CHECKED_RANDOM_SEEK(89, frame)

    // And let's do an out of bounds seek
    CHECK(janice_media_it_seek(it, 100) == JANICE_OUT_OF_BOUNDS_ACCESS,
          "Out of bounds seek returned incorrect error",
          [](){})

    return 0;
}

int check_media_iterator(JaniceConstMedia media)
{
    JaniceMediaIterator it = nullptr;
    JANICE_CALL(janice_media_get_iterator(media, &it),
                // Cleanup
                [](){})

    if (check_media_iterator_next(it) == 1) {
        janice_free_media_iterator(&it);
        return 1;
    }

    if (check_media_iterator_seek(it) == 1) {
        janice_free_media_iterator(&it);
        return 1;
    }

    JANICE_CALL(janice_free_media_iterator(&it),
                // Cleanup
                [](){})

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
    const string test_video = "media/test_video.mp4";

    // Create a media object from our test image
    JaniceMedia media = nullptr;
    JANICE_CALL(janice_create_media(test_video.c_str(),
                                    &media),
                 // Cleanup
                 [](){})

    // Check basic image properties
    //if (check_media_basics(media) == 1) {
    //    janice_free_media(&media);
    //    return 1;
    //}

    // Check that an iterator can be created and its functions work as expected
    // for a video
    if (check_media_iterator(media) == 1) {
        janice_free_media(&media);
        return 1;
    }

    // Check the loaded pixel values of the test image
    //if (check_media_pixel_values(media) == 1) {
    //    janice_free_media(&media);
    //    return 1;
    //}

    janice_free_media(&media);

    return 0;
}
