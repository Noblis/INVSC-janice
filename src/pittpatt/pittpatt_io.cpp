#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <pittpatt_sdk.h>
#include <pittpatt_raw_image_io.h>
#include <pittpatt_video_io.h>

#include <iarpa_janus_io.h>

extern ppr_context_type ppr_context;

#define JANUS_TRY_PPR_IMAGE_IO(PPR_API_CALL)             \
{                                                        \
    ppr_raw_image_error_type ppr_error = (PPR_API_CALL); \
    if (ppr_error != PPR_RAW_IMAGE_SUCCESS)              \
        return JANUS_INVALID_MEDIA;                      \
}

#define JANUS_TRY_PPR_VIDEO_IO(PPR_API_CALL)            \
{                                                       \
    ppr_video_io_error_type ppr_error = (PPR_API_CALL); \
    if (ppr_error != PPR_VIDEO_IO_SUCCESS)              \
        return JANUS_INVALID_MEDIA;                     \
}

static janus_error janus_from_pittpatt(ppr_raw_image_type *ppr_image, janus_media &media)
{
    if (!ppr_image)
        return JANUS_INVALID_MEDIA;

    if ((ppr_image->color_space != PPR_RAW_IMAGE_GRAY8) && (ppr_image->color_space != PPR_RAW_IMAGE_BGR24))
        JANUS_TRY_PPR_IMAGE_IO(ppr_raw_image_convert(ppr_image, PPR_RAW_IMAGE_BGR24))

    if (int(media.width) != ppr_image->width ||
        int(media.height) != ppr_image->height ||
        ((ppr_image->color_space == PPR_RAW_IMAGE_GRAY8 && media.color_space != JANUS_GRAY8) ||
         (ppr_image->color_space == PPR_RAW_IMAGE_BGR24 && media.color_space != JANUS_BGR24)))
        return JANUS_INVALID_MEDIA;

    janus_data *data = new janus_data[media.width * media.height * (media.color_space == JANUS_BGR24 ? 3 : 1)];
    const unsigned long elements_per_row = media.width * (media.color_space == JANUS_BGR24 ? 3 : 1);
    for (int i = 0; i < ppr_image->height; i++)
        memcpy(data + i*elements_per_row, ppr_image->data + i*ppr_image->bytes_per_line, elements_per_row);
    media.data.push_back(data);

    return JANUS_SUCCESS;
}

janus_error janus_load_media(const std::string &filename, janus_media *media_)
{
    janus_media media = *media_;

    ppr_raw_image_type ppr_image;
    ppr_raw_image_error_type error = ppr_raw_image_io_read(filename.c_str(), &ppr_image);
    if (error != PPR_RAW_IMAGE_SUCCESS) { // Not a valid image. Maybe it is a video.
        printf("Trying to load a video.");
        ppr_video_io_type ppr_video;
        JANUS_TRY_PPR_VIDEO_IO(ppr_video_io_open(&ppr_video, filename.c_str()))

        ppr_raw_image_type ppr_frame;
        JANUS_TRY_PPR_VIDEO_IO(ppr_video_io_get_frame(ppr_video, &ppr_frame))
        media.width = ppr_frame.width;
        media.height = ppr_frame.height;
        media.color_space = JANUS_BGR24;

        janus_from_pittpatt(&ppr_frame, media);
        ppr_raw_image_free(ppr_frame);

        while (ppr_video_io_is_frame_available(ppr_video) == PPR_VIDEO_IO_TRUE) {
            JANUS_TRY_PPR_VIDEO_IO(ppr_video_io_step_forward(ppr_video))
            JANUS_TRY_PPR_VIDEO_IO(ppr_video_io_get_frame(ppr_video, &ppr_frame))
            janus_from_pittpatt(&ppr_frame, media);
            ppr_raw_image_free(ppr_frame);
        }
    }

    media.width = ppr_image.width;
    media.height = ppr_image.height;
    media.color_space = (ppr_image.color_space == PPR_RAW_IMAGE_GRAY8 ? JANUS_GRAY8 : JANUS_BGR24);
    janus_from_pittpatt(&ppr_image, media);

    return JANUS_SUCCESS;
}

janus_error janus_free_media(janus_media media)
{
    for (size_t i = 0; i < media.data.size(); i++)
        delete media.data[i];
    return JANUS_SUCCESS;
}
