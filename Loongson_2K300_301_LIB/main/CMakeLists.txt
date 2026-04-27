add_compile_options(-fopenmp)
add_link_options(-fopenmp)
cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_PATH /opt/loongson-gnu-toolchain-8.3-x86_64-loongarch64-linux-gnu-rc1.4)
set(CMAKE_C_COMPILER "${CMAKE_TOOLCHAIN_PATH}/bin/loongarch64-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "${CMAKE_TOOLCHAIN_PATH}/bin/loongarch64-linux-gnu-g++")

project(main)

aux_source_directory(${PROJECT_SOURCE_DIR}/ SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../libraries/app/ SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../libraries/app/gyro SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../libraries/app/display SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../libraries/common/src SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/../libraries/drv/src SRC)

# 🔥 把有问题的 UDP demo 暂时关掉，先让主程序跑起来
 # aux_source_directory(${PROJECT_SOURCE_DIR}/../example/src SRC)
 aux_source_directory(${PROJECT_SOURCE_DIR}/../workspace/ SRC) #new

include_directories(
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/../libraries/app/
    ${PROJECT_SOURCE_DIR}/../libraries/app/gyro
    ${PROJECT_SOURCE_DIR}/../libraries/app/display
    ${PROJECT_SOURCE_DIR}/../libraries/common/inc
    ${PROJECT_SOURCE_DIR}/../libraries/drv/inc
    ${PROJECT_SOURCE_DIR}/../example/inc

    /home/guan/ncnn/build/install/include
    /home/guan/work/opencv-4.10.0/install/include/opencv4
    ${PROJECT_SOURCE_DIR}/../workspace/ #new
)

link_directories(
    ${PROJECT_SOURCE_DIR}/../libraries/drv/src/
    /home/guan/ncnn/build/install/lib
    /home/guan/work/opencv-4.10.0/install/lib
)

add_executable(main ${SRC})

target_link_libraries(main PUBLIC
    pthread
    lqncnn
    lq_camera_ex
    ncnn -fopenmp


    opencv_core
    opencv_imgproc
    opencv_highgui
    opencv_videoio
    opencv_imgcodecs
)

add_compile_definitions(LQ_HAVE_OPENCV)