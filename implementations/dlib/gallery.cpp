#include "types.hpp"
#include "utils.hpp"

JaniceError janice_create_gallery(const JaniceTemplates* tmpls, const JaniceTemplateIds* ids, JaniceGallery* gallery)
{
    assert(tmpls != nullptr);
    assert(ids != nullptr);

    assert(tmpls->length == ids->length);

    *gallery = nullptr;
    try {
        *gallery = new JaniceGalleryType();

        (*gallery)->tmpls.resize(tmpls->length);
        for (size_t i = 0; i < tmpls->length; ++i) {
            if ((*gallery)->ids.find(ids->ids[i]) != (*gallery)->ids.end()) {
                std::cerr << "Duplicate id: [" << ids->ids[i] << "] in id list at index: [" << i << "]" << std::endl;

                delete *gallery;
                *gallery = nullptr;

                return JANICE_DUPLICATE_ID;
            }

            (*gallery)->tmpls[i].fv = tmpls->tmpls[i]->fv;
            (*gallery)->ids[ids->ids[i]] = i;
            (*gallery)->reverse_lut[i] = ids->ids[i];
        }
    } catch (const std::exception& e) {
        std::cerr << "Create gallery failed. Error: [" << e.what() << "]" << std::endl;

        if (*gallery) {
            delete *gallery;
            *gallery = nullptr;
        }

        return JANICE_UNKNOWN_ERROR;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_gallery_reserve(JaniceGallery gallery, const size_t n)
{
    try {
        gallery->tmpls.reserve(n);
    } catch (const std::exception& e) {
        std::cerr << "Gallery reserve failed. Error: [" << e.what() << "]" << std::endl;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_gallery_insert(JaniceGallery gallery, const JaniceTemplate tmpl, const uint64_t id)
{
    try {
        if (gallery->ids.find(id) != gallery->ids.end()) {
            std::cerr << "Gallery insert failed. Id: [" << id << "] is already in the gallery." << std::endl;

            return JANICE_DUPLICATE_ID;
        }

        JaniceTemplateType tmpl_copy;
        tmpl_copy.fv = tmpl->fv;

        gallery->tmpls.push_back(tmpl_copy);
        gallery->ids[id] = gallery->tmpls.size() - 1;
        gallery->reverse_lut[gallery->tmpls.size() - 1] = id;
    } catch (const std::exception& e) {
        std::cerr << "Failed to insert into gallery. Error: [" << e.what() << "]" << std::endl;

        return JANICE_UNKNOWN_ERROR;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_gallery_insert_batch(JaniceGallery gallery, const JaniceTemplates* tmpls, const JaniceTemplateIds* ids, const JaniceContext* context, JaniceErrors* errors)
{
    assert(gallery != nullptr);
    assert(tmpls != nullptr);
    assert(ids != nullptr);
    assert(context != nullptr);
    assert(errors != nullptr);

    assert(tmpls->length == ids->length);

    JaniceError return_code = JANICE_SUCCESS;
    try {
        errors->length = tmpls->length;
        errors->errors = new JaniceError[errors->length];

        for (size_t i = 0; i < tmpls->length; ++i) {
            errors->errors[i] = janice_gallery_insert(gallery, tmpls->tmpls[i], ids->ids[i]);
            if (errors->errors[i] != JANICE_SUCCESS) {
                std::cerr << "Inserting template: [" << i << "] with id: [" << ids->ids[i] << "] failed. Error: [" << janice_error_to_string(errors->errors[i]) << "]" << std::endl;

                if (context->batch_policy == JaniceAbortEarly) {
                    errors->length = i + 1;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else {
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Batch insert failed. Error: [" << e.what() << "]" << std::endl;
        return JANICE_UNKNOWN_ERROR;
    }

    return return_code;
}

JaniceError janice_gallery_remove(JaniceGallery gallery, const uint64_t id)
{
    assert(gallery != nullptr);

    try {
        if (gallery->ids.find(id) == gallery->ids.end()) {
            return JANICE_MISSING_ID;
        }

        size_t id_pos = gallery->ids[id];
        size_t last_pos = gallery->tmpls.size() - 1;
        uint64_t last_id = gallery->reverse_lut[last_pos];

        std::swap(gallery->tmpls[id_pos], gallery->tmpls[last_pos]);

        gallery->tmpls.resize(gallery->tmpls.size() - 1);

        gallery->ids[last_id] = id_pos;
        gallery->reverse_lut[id_pos] = last_id;

        gallery->ids.erase(gallery->ids.find(id));
        gallery->reverse_lut.erase(gallery->reverse_lut.find(last_pos));
    } catch (const std::exception& e) {
        std::cerr << "Remove failed. Error: [" << e.what() << "]" << std::endl;
        return JANICE_UNKNOWN_ERROR;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_gallery_remove_batch(JaniceGallery gallery, const JaniceTemplateIds* ids, const JaniceContext* context, JaniceErrors* errors)
{
    assert(gallery != nullptr);
    assert(ids != nullptr);
    assert(context != nullptr);
    assert(errors != nullptr);

    JaniceError return_code = JANICE_SUCCESS;
    try {
        errors->length = ids->length;
        errors->errors = new JaniceError[errors->length];

        for (size_t i = 0; i < ids->length; ++i) {
            errors->errors[i] = janice_gallery_remove(gallery, ids->ids[i]);
            if (errors->errors[i] != JANICE_SUCCESS) {
                std::cerr << "Removing template: [" << i << "] with id: [" << ids->ids[i] << "] failed. Error: [" << janice_error_to_string(errors->errors[i]) << "]" << std::endl;

                if (context->batch_policy == JaniceAbortEarly) {
                    errors->length = i + 1;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else {
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Batch remove failed. Error: [" << e.what() << "]" << std::endl;
        return JANICE_UNKNOWN_ERROR;
    }

    return return_code;
}

JaniceError janice_gallery_prepare(JaniceGallery)
{
    return JANICE_SUCCESS;
}

JaniceError janice_serialize_gallery(const JaniceGallery gallery, uint8_t** buffer, size_t* len)
{
    assert(gallery != nullptr);
    assert(len != nullptr);

    uint8_t* _buffer = nullptr;
    try {
        *len = sizeof(size_t); // Start with the size of the gallery

        if (!gallery->tmpls.empty()) {
            *len += gallery->tmpls.size() * gallery->tmpls.front().fv.nr() * sizeof(float);
            *len += gallery->ids.size() * (sizeof(uint64_t) + sizeof(size_t));
        }

        _buffer = new uint8_t[*len];

        size_t pos = 0;

        size_t num_items = gallery->tmpls.size();
        memcpy(_buffer + pos, (void*) &num_items, sizeof(size_t));
        pos += sizeof(size_t);

        for (const JaniceTemplateType& tmpl : gallery->tmpls) {
            size_t tmpl_size = tmpl.fv.nr() * sizeof(float);
            memcpy(_buffer + pos, tmpl.fv.begin(), tmpl_size);
            pos += tmpl_size;
        }

        for (auto it : gallery->ids) {
            uint64_t id = it.first;
            size_t id_pos = it.second;

            memcpy(_buffer + pos, (void*) &id, sizeof(uint64_t));
            pos += sizeof(uint64_t);
            memcpy(_buffer + pos, (void*) &id_pos, sizeof(size_t));
            pos += sizeof(size_t);
        }

        *buffer = (uint8_t*) _buffer;
    } catch (const std::exception& e) {
        std::cerr << "Failed to serialize gallery. Error: [" << e.what() << "]" << std::endl;

        if (_buffer) {
            delete[] _buffer;
        }

        return JANICE_FAILURE_TO_SERIALIZE;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_deserialize_gallery(const uint8_t* buffer, const size_t len, JaniceGallery* gallery)
{
    assert(buffer != nullptr);
    assert(len > sizeof(size_t));

    *gallery = nullptr;
    try {
        *gallery = new JaniceGalleryType();

        size_t pos = 0;

        size_t num_items;
        memcpy((void*) &num_items, buffer + pos, sizeof(size_t));
        pos += sizeof(size_t);

        (*gallery)->tmpls.resize(num_items);

        size_t ids_total_size = num_items * (sizeof(uint64_t) + sizeof(size_t));
        size_t fv_total_size = len - sizeof(size_t) - ids_total_size;

        assert(fv_total_size % sizeof(float) == 0);

        fv_total_size /= sizeof(float);

        assert(fv_total_size % num_items == 0);

        size_t fv_size = fv_total_size / num_items;
        for (size_t i = 0; i < num_items; ++i) {
            (*gallery)->tmpls[i].fv = dlib::matrix<float, 0, 1>(fv_size);
            memcpy((*gallery)->tmpls[i].fv.begin(), buffer + pos, fv_size * sizeof(float));
            pos += fv_size * sizeof(float);
        }

        for (size_t i = 0; i < num_items; ++i) {
            uint64_t id;
            size_t id_pos;

            memcpy((void*) &id, buffer + pos, sizeof(uint64_t));
            pos += sizeof(uint64_t);

            memcpy((void*) &id_pos, buffer + pos, sizeof(size_t));
            pos += sizeof(size_t);

            (*gallery)->ids[id] = id_pos;
            (*gallery)->reverse_lut[id_pos] = id;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to deserialize gallery. Error: [" << e.what() << "]" << std::endl;

        if (*gallery) {
            delete *gallery;
            *gallery = nullptr;
        }

        return JANICE_FAILURE_TO_DESERIALIZE;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_read_gallery(const char* filename, JaniceGallery* gallery)
{
    FILE* file = fopen(filename, "rb");
    if (!file) {
        std::cerr << "Unable to open [" << filename << "] in read mode" << std::endl;
        return JANICE_READ_ERROR;
    }

    size_t len;
    fread((void*) &len, sizeof(size_t), 1, file);

    uint8_t* buffer = new uint8_t[len];
    fread(buffer, sizeof(uint8_t), len, file);

    fclose(file);

    JaniceError err = janice_deserialize_gallery(buffer, len, gallery);

    delete[] buffer;

    if (err != JANICE_SUCCESS) {
        janice_free_gallery(gallery);
    }

    return err;
}

JaniceError janice_write_gallery(const JaniceGallery gallery, const char* filename)
{
    FILE* file = fopen(filename, "wb+");
    if (!file) {
        std::cerr << "Unable to open [" << filename << "] in write mode" << std::endl;
        return JANICE_WRITE_ERROR;
    }

    uint8_t* buffer = nullptr;
    size_t len;
    JaniceError err = janice_serialize_gallery(gallery, &buffer, &len);

    if (err != JANICE_SUCCESS) {
        fclose(file);
        return err;
    }

    fwrite((void*) &len, sizeof(size_t), 1, file);
    fwrite(buffer, sizeof(uint8_t), len, file);

    fclose(file);

    return JANICE_SUCCESS;
}

JaniceError janice_free_gallery(JaniceGallery* gallery)
{
    if (gallery && *gallery) {
        delete *gallery;
        *gallery = nullptr;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_template_ids(JaniceTemplateIds* ids)
{
    if (ids) {
        delete[] ids->ids;
        ids->length = 0;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_template_ids_group(JaniceTemplateIdsGroup* ids_group)
{
    if (ids_group) {
        for (size_t i = 0; i < ids_group->length; ++i) {
            janice_clear_template_ids(&ids_group->group[i]);
        }

        delete[] ids_group->group;
        ids_group->length = 0;
    }

    return JANICE_SUCCESS;
}
