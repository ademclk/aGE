include(FetchContent)

# SDL3
find_package(SDL3 3.2 CONFIG QUIET)

if(NOT TARGET SDL3::SDL3)
    message(STATUS "aGE: SDL3 not found. Fetching SDL3...")

    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG        release-3.4.20
            GIT_SHALLOW    TRUE
    )

    FetchContent_MakeAvailable(SDL3)
endif()

message(STATUS "aGE: SDL3 configured")
