file(GLOB_RECURSE SOURCES *.cpp)
add_executable(tutorial-01 ${SOURCES})

target_compile_options(tutorial-01
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8 /DUNICODE /D_UNICODE /DNOMINMAX /Zc:preprocessor /Zc:__cplusplus /wd5054 /bigobj>
        $<$<CXX_COMPILER_ID:MSVC>:/D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR> # VS 17.10: https://github.com/microsoft/STL/issues/4730
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wno-deprecated-enum-enum-conversion>
)

target_link_libraries(tutorial-01
    PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets Qt6::GuiPrivate
)

qt_add_shaders(tutorial-01 "shaders"
    PREFIX
        "/"
    FILES
        "vertex.vert"
        "fragment.frag"
)