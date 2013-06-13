A CVAC service around OpenCV's "cascade" training and detection functionality.

Modifications and steps:

1) copy code from OpenCV/apps/traincascade into traincascade subdir, except traincascade.cpp, 
2) modify traincascade/CMakeLists to create a library not an executable,
3) copy code from OpenCV/apps/haartraining/ into the haartraining subdir, except createsamples.cpp,

