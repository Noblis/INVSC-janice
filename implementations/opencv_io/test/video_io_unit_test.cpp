#include <janice_io_opencv.h>

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
// Check media iterator

// Check the iterator next and tell functions
int check_media_iterator_next(JaniceMediaIterator it)
{
    JaniceImage image = nullptr;
    uint32_t frame, frame_count = 0;

    while (true) {
        JaniceError err = it->next(it, &image);

        JaniceError expected = JANICE_SUCCESS;
        ++frame_count;
        
        // On the last frame we loop back to the beginning
        if (frame_count == 90) {
            expected = JANICE_MEDIA_AT_END;
            frame_count = 0;
        }

        CHECK(err == expected,
            "Next should return JANICE_SUCCESS except for on the last frame",
            []() {})

        auto cleanup = [&]() {
            it->free_image(&image);
        };

        // Next, query the iterator for it's internal frame count
        JANICE_CALL(it->tell(it, &frame),
                    // Cleanup
                    cleanup)

        cleanup();

        if (expected == JANICE_MEDIA_AT_END)
            break;
    }

    return 0;
}

#define CHECKED_RANDOM_SEEK(desired, actual)                                     \
{                                                                                \
    JANICE_CALL(it->seek(it, desired), [](){})                                   \
    JANICE_CALL(it->tell(it, &actual), [](){})                                   \
    CHECK(desired == actual, "Seeked frame does not match actual frame", [](){}) \
}

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
    CHECK(it->seek(it, 100) == JANICE_OUT_OF_BOUNDS_ACCESS,
          "Out of bounds seek returned incorrect error",
          [](){})

    return 0;
}

int check_media_iterator(const char* media)
{
    JaniceMediaIterator it = nullptr;
    JANICE_CALL(janice_io_opencv_create_media_iterator(media, &it),
                // Cleanup
                []() {})

    if (check_media_iterator_next(it) == 1) {
        it->free(&it);
        return 1;
    }

    if (check_media_iterator_seek(it) == 1) {
        it->free(&it);
        return 1;
    }

    JANICE_CALL(it->free(&it),
                // Cleanup
                [](){})

    return 0;
}

// ----------------------------------------------------------------------------
// Check image pixel values. Due to lossy compression in video codecs, we can't
// reliably check the exact value, so use a 10 unit tolerance.

static inline int check_pixel(JaniceConstImage image,
                               uint32_t x,
                               uint32_t y,
                               uint8_t red,
                               uint8_t green,
                               uint8_t blue)
{
    // Variable to hold pixel values
    uint8_t pixel;

    // Check the blue pixel first, remember OpenCV stores images in BGR order
    CHECK(abs(int(janice_image_access(image, 0, y, x)) - int(blue)) < 10,
          "Blue pixel doesn't match.",
          // Cleanup
          [](){})

    // Check the green pixel
    CHECK(abs(int(janice_image_access(image, 1, y, x)) - int(green)) < 10,
          "Green pixel doesn't match.",
          // Cleanup
          [](){})

    // Check the red pixel
    CHECK(abs(int(janice_image_access(image, 2, y, x)) - int(red)) < 10,
          "Red pixel doesn't match.",
          // Cleanup
          [](){})

    return 0;
}

uint8_t frame_color_lookup[9][3] = 
{
    {0,   0,   254},
    {0,   255, 0},
    {255, 0,   0},
    {0,   127, 127},
    {127, 127, 0},
    {127, 0,   127},
    {0,   0,   0},
    {127, 127, 127},
    {255, 255, 255}
};

// look up expecteed color for the specified frame of test video
int expected_frame_colors(int frame_number, uint8_t* r, uint8_t* g, uint8_t* b)
{
    // past end of video 
    if (frame_number >= 90)
        return 0;

    // solid colors, repeated for 10 frames each 
    int idx = frame_number / 10;

    *r = frame_color_lookup[idx][0];
    *g = frame_color_lookup[idx][1];
    *b = frame_color_lookup[idx][2];

    return 1;
}

int check_media_pixel_values(const char* media)
{
    JaniceMediaIterator it = nullptr;
    JANICE_CALL(janice_io_opencv_create_media_iterator(media, &it),
                // Cleanup
                [](){})

    // Variables to be filled during iterator calls
    JaniceImage image = nullptr;

    // loop over video 
    uint32_t frame_count = 0;

    uint8_t r, g, b;

    while (true) {
        JaniceError err = it->next(it, &image);
        JaniceError expected = JANICE_SUCCESS;

        ++frame_count;

        auto cleanup = [&]() {
            it->free_image(&image);
            it->free(&it);
        };

        // On the last frame we loop back to the beginning
        if (frame_count == 90) {
            expected = JANICE_MEDIA_AT_END;
            frame_count = 0;
        }

        CHECK(err == expected,
              "Next should return JANICE_SUCCESS except for on the last frame",
              [](){})

        if (expected == JANICE_MEDIA_AT_END) {
            it->free_image(&image);
            break;
        }

        // verify that the frame number we are using is correct, pull expected rgb values for this frame
        CHECK(expected_frame_colors(frame_count - 1, &r, &g, &b) == 1,
              "Advanced past expected end of video, for unknown reasons.",
              // Cleanup
              cleanup)

        // check that actual and expected colors match at some point in the image
        CHECK(check_pixel(image, 50, 50, r, g, b) == 0,
              "Pixel mismatch",
              // Cleanup
              cleanup)

        cleanup();

        if (expected == JANICE_MEDIA_AT_END)
            break;
    }

    it->free(&it);

    return 0;
}

// ----------------------------------------------------------------------------
// Main test function

int main(int, char*[])
{
    const char* test_video = "media/test_video.mp4";

    // Check that an iterator can be created and its functions work as expected
    // for a video
    if (check_media_iterator(test_video) == 1) {
        return 1;
    }

    // Check the loaded pixel values of the test image
    if (check_media_pixel_values(test_video) == 1) {
        return 1;
    }

    return 0;
}
