# Boost search pats. please fill these

set(BOOST_ROOT "C:\\Program Files\\boost")
set(BOOST_INCLUDEDIR "C:\\Program Files\\boost\\build\\include\\boost-1_82")
set(BOOST_LIBRARYDIR "C:\\Program Files\\boost\\build\\lib")
set(Boost_NO_WARN_NEW_VERSIONS 1)


set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS        ON)  # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost COMPONENTS locale date_time filesystem timer thread system)

if(Boost_FOUND)
  include_directories(${BOOST_INCLUDEDIR})
endif()
