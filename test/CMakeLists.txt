cmake_minimum_required(VERSION 3.16)


################################################################################
# Macros
################################################################################


macro (DownloadAndPopulateCMakeHelpers)
    include(FetchContent)
    FetchContent_Declare(
        cmake_helpers
        GIT_REPOSITORY https://github.com/KKoovalsky/CMakeHelpers.git
        GIT_TAG e81be067115c349a55715e325ebed98795d55cfa
    )
    FetchContent_MakeAvailable(cmake_helpers)
endmacro()


macro (DownloadAndPopulateFreeRTOSKernel)
    JunglesHelpers_DownloadAndPopulateFreeRTOSKernel(
        freertos
        18f714f786ddbc137c632d6845c5fa2b175d3cbd
        heap_3
    )

    set(freertos_port_dir ${FREERTOS_SOURCE_DIR}/portable/ThirdParty/GCC/Posix)
    target_include_directories(freertos PUBLIC
        ${CMAKE_SOURCE_DIR}/test/freertos
        ${freertos_port_dir}
    )
    target_sources(freertos PRIVATE ${freertos_port_dir}/port.c ${freertos_port_dir}/utils/wait_for_event.c)

    find_package(Threads REQUIRED)
    target_link_libraries(freertos Threads::Threads)
endmacro()


macro(DownloadAndPopulateCatch2)
    set(CATCH_BUILD_TESTING OFF CACHE BOOL "Internal Catch2's option to disable Catch2 self-test")
    set(BUILD_TESTING OFF CACHE BOOL "Internal Catch2's option to disable Catch2 self-test")

    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG 7cf2f88e50f0d1de324489c31db0314188423b6d
    )
    FetchContent_MakeAvailable(Catch2)
endmacro()


macro (CreateFreeRTOSHelpersTests)
    add_executable(freertos_helpers_tests
        freertos/main.cpp
        freertos/test_event_group.cpp
        freertos/test_simple_thread.cpp
        generic/test_active.cpp
        generic/test_thread.cpp
        generic/test_queue.cpp
        generic/test_flag.cpp
        generic/test_thread_pool.cpp
    )
    target_include_directories(freertos_helpers_tests PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/freertos
    )
    target_link_libraries(freertos_helpers_tests PRIVATE freertos Catch2::Catch2)
    target_compile_features(freertos_helpers_tests PRIVATE cxx_std_17)
    target_compile_options(freertos_helpers_tests PRIVATE -Wall -Wextra)
    target_link_libraries(freertos_helpers_tests PRIVATE JunglesOsHelpers JunglesOs)
    add_test(NAME freertos_helpers COMMAND freertos_helpers_tests)
    set_tests_properties(freertos_helpers PROPERTIES TIMEOUT 30)
endmacro()


macro(CreateNativeHelpersTests)
    add_executable(native_helpers_tests
        native/test_poller.cpp
        generic/test_thread_pool.cpp
        generic/test_active.cpp
        generic/test_lockable.cpp
    )
    target_include_directories(native_helpers_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/native)
    target_link_libraries(native_helpers_tests PRIVATE Catch2::Catch2WithMain Threads::Threads)
    target_compile_features(native_helpers_tests PRIVATE cxx_std_17)
    target_compile_options(native_helpers_tests PRIVATE -Wall -Wextra)
    target_link_libraries(native_helpers_tests PRIVATE JunglesOsHelpers)
    add_test(NAME native_helpers COMMAND valgrind --leak-check=full $<TARGET_FILE:native_helpers_tests>)
endmacro()


################################################################################
# Main script
################################################################################

target_link_libraries(JunglesOs PRIVATE freertos)

DownloadAndPopulateCMakeHelpers()
DownloadAndPopulateFreeRTOSKernel()
DownloadAndPopulateCatch2()

CreateFreeRTOSHelpersTests()
CreateNativeHelpersTests()
