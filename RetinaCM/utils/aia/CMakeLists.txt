# find source files
file(GLOB aia_src *.h *.cpp *.hpp)

add_definitions(-DOPENCV_FACE_DETECTOR_PATH="${aia_SOURCE_DIR}/utils/aia/haarcascade_frontalface_alt.xml")

# create static library from source files
add_library(aiaUtils STATIC ${aia_src})