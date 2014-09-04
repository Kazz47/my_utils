# This file loads slog (https://github.com/Kazz47/slog) during build time.

include(ExternalProject)

# Add gtest
ExternalProject_Add(slog
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
    GIT_REPOSITORY https://github.com/Kazz47/slog.git
    # Disable install step
    INSTALL_COMMAND ""
)

# Specify include dir
ExternalProject_Get_Property(slog source_dir)
set(SLOG_INCLUDE_DIRS ${source_dir}/include PARENT_SCOPE)

# Specify MainTest's link libraries
ExternalProject_Get_Property(slog binary_dir)
set(SLOG_LIBS_DIR ${binary_dir} PARENT_SCOPE)

