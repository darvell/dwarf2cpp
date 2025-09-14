cmake_minimum_required(VERSION 3.17)
project(dwarf2cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    main.cpp
    cpp.cpp
)

# Header files
set(HEADERS
    cpp.h
    dwarf.h
    elf.h
)

# Create executable
add_executable(dwarf2cpp ${SOURCES} ${HEADERS})

# Platform-specific settings
if(APPLE)
    # macOS specific settings
    target_compile_definitions(dwarf2cpp PRIVATE MACOS_BUILD)
    # Use std::filesystem instead of experimental on modern systems
    target_compile_definitions(dwarf2cpp PRIVATE USE_STD_FILESYSTEM)
elseif(WIN32)
    # Windows specific settings
    target_compile_definitions(dwarf2cpp PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Use experimental filesystem on Windows if needed
    target_link_libraries(dwarf2cpp stdc++fs)
elseif(UNIX)
    # Linux specific settings
    target_link_libraries(dwarf2cpp stdc++fs)
endif()

# Compiler-specific settings
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(dwarf2cpp stdc++fs)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Check if we need to link filesystem library
    if(NOT APPLE OR CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
        target_link_libraries(dwarf2cpp stdc++fs)
    endif()
endif()