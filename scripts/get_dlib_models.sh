if [ ! $# -eq 1 ]; then
  echo "Usage: ./get_dlib_models.sh <sdk_path>"
  exit
fi

SDK_PATH=$1

mkdir -p $SDK_PATH

cd $SDK_PATH
if [ ! -f mmod_human_face_detector.dat ]; then
  curl http://dlib.net/files/mmod_human_face_detector.dat.bz2 --output mmod_human_face_detector.dat.bz2
  bzip2 -d mmod_human_face_detector.dat.bz2
fi

if [ ! -f shape_predictor_5_face_landmarks.dat ]; then
  curl http://dlib.net/files/shape_predictor_5_face_landmarks.dat.bz2 --output shape_predictor_5_face_landmarks.dat.bz2
  bzip2 -d shape_predictor_5_face_landmarks.dat.bz2
fi

if [ ! -f dlib_face_recognition_resnet_model_v1.dat ]; then
  curl http://dlib.net/files/dlib_face_recognition_resnet_model_v1.dat.bz2 --output dlib_face_recognition_resnet_model_v1.dat.bz2
  bzip2 -d dlib_face_recognition_resnet_model_v1.dat.bz2
fi
