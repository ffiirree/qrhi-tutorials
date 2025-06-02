file(GLOB_RECURSE SOURCES *.cpp tutorial-24.qrc)
add_executable(tutorial-24 ${SOURCES})

target_compile_options(tutorial-24
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8 /DUNICODE /D_UNICODE /DNOMINMAX /Zc:preprocessor /Zc:__cplusplus /wd5054 /bigobj>
        $<$<CXX_COMPILER_ID:MSVC>:/D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR> # VS 17.10: https://github.com/microsoft/STL/issues/4730
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Wno-deprecated-enum-enum-conversion>
)

target_link_libraries(tutorial-24
    PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets Qt6::GuiPrivate
)

qt_add_shaders(tutorial-24 "shaders"
    PREFIX
        "/"
    FILES
        "vertex.vert"
        "fragment.frag"
)