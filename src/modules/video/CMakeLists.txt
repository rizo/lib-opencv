set(the_description "Video Analysis")
ocv_add_dispatched_file(lkpyramid SSE2 SSE4_1 AVX2 AVX512_SKX AVX512_ICL)
ocv_define_module(video
    opencv_imgproc
    OPTIONAL
      opencv_geometry
      opencv_features
      opencv_dnn
    WRAP
      java
      objc
      python
      js
)
