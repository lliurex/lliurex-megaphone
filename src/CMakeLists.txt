
#main 
add_executable(lliurex-megaphone main.cpp)

#require pkg-config support
find_package(PkgConfig REQUIRED)

#required pkg-config libs
pkg_check_modules(PULSE REQUIRED libpulse)
pkg_check_modules(PULSE_GLIB REQUIRED libpulse-mainloop-glib)
pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)

include_directories(${PULSE_INCLUDE_DIRS} ${PULSE_GLIB_INCLUDE_DIRS} ${GTKMM3_INCLUDE_DIRS})

target_link_libraries(lliurex-megaphone ${PULSE_LIBRARIES} ${PULSE_GLIB_LIBRARIES} ${GTKMM3_LIBRARIES})

#add custom lib flags
add_definitions(-std=c++11 -O3)

install(TARGETS lliurex-megaphone RUNTIME DESTINATION bin)



