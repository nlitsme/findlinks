file(GLOB ITSLIB_DIRS itslib  submodules/itslib ../itslib ../../itslib)
find_path(ITSLIB_PATH NAMES include/itslib/util/HiresTimer.h PATHS ${ITSLIB_DIRS})
if(NOT ITSLIB_PATH)
	message(FATAL " itslib not found")
endif()

add_library(itslib ${ITSLIB_PATH}/src/utfcvutils.cpp ${ITSLIB_PATH}/src/stringutils.cpp)
target_include_directories(itslib PUBLIC ${ITSLIB_PATH}/include/itslib)
