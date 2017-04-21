# include libraries
include_directories (${aia_SOURCE_DIR}/utils)
include_directories (${aia_SOURCE_DIR}/3rdparty)

# find sources
file(GLOB project0_sources *.h *.hpp *.cpp)

# create executable from sources
add_executable(project0 ${project0_sources})

# link the executable to other modules / libraries 
target_link_libraries(project0 aiaUtils ucasUtils ${OpenCV_LIBS})
