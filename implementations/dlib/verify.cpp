#include "types.hpp"
#include "utils.hpp"


JaniceError janice_verify(const JaniceTemplate ref, const JaniceTemplate ver, double* similarity)
{
    assert(ref != nullptr);
    assert(ver != nullptr);
    assert(similarity != nullptr);

    *similarity = 1 - dlib::length(ref->fv - ver->fv);

    return JANICE_SUCCESS;
}

JaniceError janice_verify_batch(const JaniceTemplates* refs, const JaniceTemplates* vers, const JaniceContext* context, JaniceSimilarities* similarities, JaniceErrors* errors)
{
    assert(refs != nullptr);
    assert(vers != nullptr);
    assert(context != nullptr);
    assert(similarities != nullptr);
    assert(errors != nullptr);

    assert(refs->length == vers->length);

    similarities->length = refs->length;
    similarities->similarities = new double[similarities->length];

    errors->length = refs->length;
    errors->errors = new JaniceError[errors->length];

    JaniceError return_code = JANICE_SUCCESS;
    for (size_t i = 0; i < refs->length; ++i) {
        errors->errors[i] = janice_verify(refs->tmpls[i], vers->tmpls[i], &similarities->similarities[i]);
        if (errors->errors[i] != JANICE_SUCCESS) {
            std::cerr << "Comparison: [" << i << "] failed with error: [" << janice_error_to_string(errors->errors[i]) << "]" << std::endl;
            if (context->batch_policy == JaniceAbortEarly) {
                similarities->length = i + 1;
                errors->length = i + 1;

                return JANICE_BATCH_ABORTED_EARLY;
            } else {
                return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
            }
        }
    }

    return return_code;
}

JaniceError janice_clear_similarities(JaniceSimilarities* similarities)
{
    if (similarities) {
        delete[] similarities->similarities;
        similarities->length = 0;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_similarities_group(JaniceSimilaritiesGroup* similarities_group)
{
    if (similarities_group) {
        for (size_t i = 0; i < similarities_group->length; ++i) {
            janice_clear_similarities(&similarities_group->group[i]);
        }

        delete[] similarities_group->group;
        similarities_group->length = 0;
    }

    return JANICE_SUCCESS;
}
