# The MIT License (MIT)
#
# Copyright (c) 2017 Nathan Osman
#               2020-2021 Mehdi Chinoune
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

if( WIN32 AND QT_VERSION_MAJOR VERSION_GREATER_EQUAL 6 )
    find_package(Qt${QT_VERSION_MAJOR}Tools REQUIRED)
    get_target_property(WINDEPLOYQT_EXECUTABLE Qt${QT_VERSION_MAJOR}::windeployqt IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${WINDEPLOYQT_EXECUTABLE}" DIRECTORY)
else()
    find_package(Qt${QT_VERSION_MAJOR}Core REQUIRED)
    # Retrieve the absolute path to qmake and then use that path to find
    # the windeployqt and macdeployqt binaries
    get_target_property(_qmake_executable Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)

    if( WIN32 )
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}" REQUIRED)
    endif()

    if(APPLE)
        find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}" REQUIRED)
    endif()
endif()

# Add commands that copy the Qt runtime to the target's output directory after
# build and install the Qt runtime to the specified directory
function(windeployqt target directory)

    if( QT_VERSION_MAJOR VERSION_GREATER_EQUAL 6 )
        set( no_angle "" )
    else()
        set( no_angle "--no-angle" )
    endif()
    # Run windeployqt immediately after build
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
            env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
                --verbose 0
                --no-compiler-runtime
                --no-opengl-sw
                ${no_angle}
                \"$<TARGET_FILE:${target}>\"
        COMMENT "Deploying Qt..."
    )

    # install(CODE ...) doesn't support generator expressions, but
    # file(GENERATE ...) does - store the path in a file
    file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${target}_$<CONFIG>_path"
        CONTENT "$<TARGET_FILE:${target}>"
    )

    # Before installation, run a series of commands that copy each of the Qt
    # runtime files to the appropriate directory for installation
    install(CODE
        "
        file(READ \"${CMAKE_CURRENT_BINARY_DIR}/${target}_\$<CONFIG>_path\" _file)
        execute_process(
            COMMAND \"${CMAKE_COMMAND}\" -E
                env PATH=\"${_qt_bin_dir}\" \"${WINDEPLOYQT_EXECUTABLE}\"
                    --dry-run
                    --no-compiler-runtime
                    --no-opengl-sw
                    ${no_angle}
                    --list mapping
                    \${_file}
            OUTPUT_VARIABLE _output
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        separate_arguments(_files WINDOWS_COMMAND \${_output})
        while(_files)
            list(GET _files 0 _src)
            list(GET _files 1 _dest)
            execute_process(
                COMMAND \"${CMAKE_COMMAND}\" -E
                    copy \${_src} \"\${CMAKE_INSTALL_PREFIX}/${directory}/\${_dest}\"
            )
            list(REMOVE_AT _files 0 1)
        endwhile()
        "
    )

    # windeployqt doesn't work correctly with the system runtime libraries,
    # so we fall back to one of CMake's own modules for copying them over
    include(InstallRequiredSystemLibraries)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
            copy_if_different ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} \"$<TARGET_FILE_DIR:${target}>\"
        COMMENT "Copying System Libraries..."
    )
endfunction()

# Add commands that copy the required Qt files to the application bundle
# represented by the target.
function(macdeployqt target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${MACDEPLOYQT_EXECUTABLE}"
            \"$<TARGET_BUNDLE_DIR:${target}>\"
            -always-overwrite
        COMMENT "Deploying Qt..."
    )
endfunction()

mark_as_advanced(WINDEPLOYQT_EXECUTABLE MACDEPLOYQT_EXECUTABLE)
