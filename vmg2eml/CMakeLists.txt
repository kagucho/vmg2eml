cmake_minimum_required(VERSION 2.8)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 14) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

set( PROJECT_NAME "vmg2eml" )
set(LIBMBFL_ROOT "" CACHE PATH "libmbfl home path")
set(LIBMBFL_LIB "" CACHE PATH "libmbfl lib path")
#set(LIBMBFL_ROOT "../../mbfl")
#set(LIBMBFL_LIB "../../mbfl/build/mbfl")
project(${PROJECT_NAME})

set(
	vmg2eml_src
	main.cpp
)
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-long-long -pedantic")
endif()

include_directories(${LIBMBFL_ROOT})
link_directories(${LIBMBFL_LIB})
## Define the executable
add_executable(${PROJECT_NAME} ${vmg2eml_src})
target_link_libraries(${PROJECT_NAME} libmbfl)
