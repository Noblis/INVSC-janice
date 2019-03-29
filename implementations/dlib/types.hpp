#ifndef JANICE_DLIB_TYPES_HPP_INCLUDED
#define JANICE_DLIB_TYPES_HPP_INCLUDED

#include <janice.h>

#include <dlib/dnn.h>
#include <dlib/image_processing.h>

struct JaniceDetectionType
{
    JaniceTrack track;
};

struct JaniceTemplateType
{
    dlib::matrix<float,0,1> fv;

    int left_eye_x, left_eye_y;
    int right_eye_x, right_eye_y;
    int nose_x, nose_y;
};

struct JaniceGalleryType
{
    std::vector<JaniceTemplateType> tmpls;
    std::map<uint64_t, size_t> ids; // id -> position
    std::map<size_t, uint64_t> reverse_lut; // position -> id
};

namespace janice_dlib
{

// ----------------------------------------------------------------------------------------
// The following defines the DLIB deep face detector

template <long num_filters, typename SUBNET> using con5d = dlib::con<num_filters,5,5,2,2,SUBNET>;
template <long num_filters, typename SUBNET> using con5  = dlib::con<num_filters,5,5,1,1,SUBNET>;

template <typename SUBNET> using downsampler = dlib::relu<dlib::affine<con5d<32, dlib::relu<dlib::affine<con5d<32, dlib::relu<dlib::affine<con5d<16,SUBNET>>>>>>>>>;
template <typename SUBNET> using rcon5       = dlib::relu<dlib::affine<con5<45,SUBNET>>>;

using face_detector_net_type =
    dlib::loss_mmod<
      dlib::con<1,9,9,1,1,
        rcon5<
          rcon5<
            rcon5<
              downsampler<
                dlib::input_rgb_image_pyramid<
                  dlib::pyramid_down<6>
                >
              >
            >
          >
        >
      >
    >;

// ----------------------------------------------------------------------------------------
//
// The next bit of code defines a ResNet network.  It's basically copied
// and pasted from the dnn_imagenet_ex.cpp example, except we replaced the loss
// layer with loss_metric and made the network somewhat smaller.  Go read the introductory
// dlib DNN examples to learn what all this stuff means.
//
// Also, the dnn_metric_learning_on_images_ex.cpp example shows how to train this network.
// The dlib_face_recognition_resnet_model_v1 model used by this example was trained using
// essentially the code shown in dnn_metric_learning_on_images_ex.cpp except the
// mini-batches were made larger (35x15 instead of 5x5), the iterations without progress
// was set to 10000, and the training dataset consisted of about 3 million images instead of
// 55.  Also, the input layer was locked to images of size 150.

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = dlib::add_prev1<block<N,BN,1,dlib::tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = dlib::add_prev2<dlib::avg_pool<2,2,2,2,dlib::skip1<dlib::tag2<block<N,BN,2,dlib::tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<dlib::con<N,3,3,1,1,dlib::relu<BN<dlib::con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = dlib::relu<residual<block,N,dlib::affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = dlib::relu<residual_down<block,N,dlib::affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using feature_extractor_net_type =
    dlib::loss_metric<
      dlib::fc_no_bias<128,
        dlib::avg_pool_everything<
          alevel0<
            alevel1<
              alevel2<
                alevel3<
                  alevel4<
                    dlib::max_pool<3,3,2,2,
                      dlib::relu<
                        dlib::affine<
                          dlib::con<32,7,7,2,2,
                            dlib::input_rgb_image_sized<150>
                          >
                        >
                      >
                    >
                  >
                >
              >
            >
          >
        >
      >
    >;


struct Globals
{
    face_detector_net_type fd;
    dlib::shape_predictor sp;
    feature_extractor_net_type fe;

    int num_threads;
};

extern std::unique_ptr<Globals> _globals;

} // namespace janice_dlib

#endif // JANICE_DLIB_TYPES_HPP
