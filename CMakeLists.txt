
# ***  lliurex megaphone main cmake ****
cmake_minimum_required (VERSION 2.8)
project (lliurex-megaphone)

add_subdirectory(src)

add_custom_target(update-pot COMMAND xgettext -d lliurex-megaphone -s --keyword=T -o lliurex-megaphone.pot ${CMAKE_SOURCE_DIR}/src/main.cpp)
add_custom_target(dist COMMAND cp ${CMAKE_SOURCE_DIR}/rsrc/* $<TARGET_FILE_DIR:lliurex-megaphone>/)


install(FILES ${CMAKE_SOURCE_DIR}/rsrc/interface.glade DESTINATION share/lliurex-megaphone/rsrc/)

