cmake_minimum_required(VERSION 3.11)

project(ImgLib CXX)
set(CMAKE_CXX_STANDARD 17)

# добавляем новую переменную - путь к LibJPEG
set(LIBJPEG_DIR CACHE STRING "LibJPEG static library directory")
if(NOT LIBJPEG_DIR)
    message(FATAL_ERROR "Please, specify LibJPEG directory via -DLIBJPEG_DIR=<dir>")
endif()
message(STATUS "LibJPEG dir is ${LIBJPEG_DIR}, change via -DLIBJPEG_DIR=<dir>")

set(IMGLIB_MAIN_FILES 
    img_lib.h img_lib.cpp
)

# к файлам форматов добавим JPEG и BMP
set(IMGLIB_FORMAT_FILES 
    ppm_image.h ppm_image.cpp 
    jpeg_image.h jpeg_image.cpp
    bmp_image.h bmp_image.cpp
)

add_library(ImgLib STATIC 
    ${IMGLIB_MAIN_FILES} 
    ${IMGLIB_FORMAT_FILES}
)

# Include-директории теперь включают LibJPEG
target_include_directories(ImgLib PUBLIC 
    "${LIBJPEG_DIR}/include"
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Флаг INTERFACE обозначает видимость параметра - только для зависимых целей.
# Для самой ImgLib не нужно указывать зависимости компоновки, поскольку
# статическая библиотека не компонуется, а просто архивируется
target_link_directories(ImgLib INTERFACE
    "${LIBJPEG_DIR}/lib/$<IF:$<CONFIG:Debug>,Debug,Release>"
)

# В качестве зависимости указано jpeg. Компоновщик будет искать
# файл libjpeg.a
target_link_libraries(ImgLib INTERFACE jpeg)

# Добавляем исполняемый файл конвертера, если это основной проект
if(CMAKE_PROJECT_NAME STREQUAL ImgLib)
    add_executable(image_converter main.cpp)
    target_link_libraries(image_converter ImgLib)
endif()
