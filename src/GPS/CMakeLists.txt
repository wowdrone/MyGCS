find_package(Qt6 REQUIRED COMPONENTS Core)

qt_add_library(GPS STATIC)

if(QGC_NO_SERIAL_LINK)
    return()
endif()

find_package(Qt6 REQUIRED COMPONENTS QmlIntegration)

# include(FetchContent)
# FetchContent_Declare(gps_drivers
#     GIT_REPOSITORY https://github.com/PX4/PX4-GPSDrivers.git
#     GIT_TAG main
#     GIT_SHALLOW TRUE
#     SOURCE_SUBDIR src
#     # SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/PX4-GPSDrivers
# )
# FetchContent_MakeAvailable(gps_drivers)

set(gps_drivers_SOURCE_DIR Drivers/src)

qt_add_library(GPSDrivers STATIC
    definitions.h
    ${gps_drivers_SOURCE_DIR}/ashtech.cpp
    ${gps_drivers_SOURCE_DIR}/ashtech.h
    ${gps_drivers_SOURCE_DIR}/base_station.h
    ${gps_drivers_SOURCE_DIR}/crc.cpp
    ${gps_drivers_SOURCE_DIR}/crc.h
    ${gps_drivers_SOURCE_DIR}/emlid_reach.cpp
    ${gps_drivers_SOURCE_DIR}/emlid_reach.h
    ${gps_drivers_SOURCE_DIR}/femtomes.cpp
    ${gps_drivers_SOURCE_DIR}/femtomes.h
    ${gps_drivers_SOURCE_DIR}/gps_helper.cpp
    ${gps_drivers_SOURCE_DIR}/gps_helper.h
    ${gps_drivers_SOURCE_DIR}/mtk.cpp
    ${gps_drivers_SOURCE_DIR}/mtk.h
    ${gps_drivers_SOURCE_DIR}/nmea.cpp
    ${gps_drivers_SOURCE_DIR}/nmea.h
    ${gps_drivers_SOURCE_DIR}/rtcm.cpp
    ${gps_drivers_SOURCE_DIR}/rtcm.h
    ${gps_drivers_SOURCE_DIR}/sbf.cpp
    ${gps_drivers_SOURCE_DIR}/sbf.h
    ${gps_drivers_SOURCE_DIR}/ubx.cpp
    ${gps_drivers_SOURCE_DIR}/ubx.h
    ${gps_drivers_SOURCE_DIR}/unicore.cpp
    ${gps_drivers_SOURCE_DIR}/unicore.h
)

target_link_libraries(GPSDrivers
    PUBLIC
        Qt6::Core
        Utilities
)

target_include_directories(GPSDrivers
    PRIVATE
        # ${CMAKE_CURRENT_SOURCE_DIR}/PX4-GPSDrivers/include
    PUBLIC
        ${gps_drivers_SOURCE_DIR}
)

target_sources(GPS
    PRIVATE
        GPSManager.cc
        GPSManager.h
        GPSProvider.cc
        GPSProvider.h
        GPSRtk.cc
        GPSRtk.h
        RTCMMavlink.cc
        RTCMMavlink.h
        satellite_info.h
        sensor_gnss_relative.h
        sensor_gps.h
)

target_link_libraries(GPS
    PRIVATE
        Comms
        FactSystem
        GPSDrivers
        MAVLink
        Settings
        Utilities
        Vehicle
    PUBLIC
        Qt6::Core
        Qt6::QmlIntegration
)

target_include_directories(GPS
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
