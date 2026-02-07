project(implot LANGUAGES CXX)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS
    "*.cpp"
)


add_library(implot STATIC ${MY_SOURCES})

target_link_libraries(implot
  PRIVATE mai
)
