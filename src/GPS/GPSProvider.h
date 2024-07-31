/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#pragma once

#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QMetaType>
#include <QtCore/QLoggingCategory>

#include "Drivers/src/gps_helper.h"
#include "sensor_gps.h"
#include "sensor_gnss_relative.h"
#include "satellite_info.h"

Q_DECLARE_LOGGING_CATEGORY(GPSProviderLog)

class QSerialPort;
class QThread;

class GPSProvider : public QObject
{
    Q_OBJECT

public:
    enum class GPSType {
        u_blox,
        trimble,
        septentrio,
        femto
    };

    GPSProvider(
        const QString &device,
        GPSType type,
        double surveyInAccMeters,
        int surveryInDurationSecs,
        bool useFixedBaseLocation,
        double fixedBaseLatitude,
        double fixedBaseLongitude,
        float fixedBaseAltitudeMeters,
        float fixedBaseAccuracyMeters,
        const std::atomic_bool &requestStop,
        QObject *parent = nullptr
    );
    ~GPSProvider();

    int callback(GPSCallbackType type, void *data1, int data2);

signals:
    void satelliteInfoUpdate(const satellite_info_s &message);
    void sensorGnssRelativeUpdate(const sensor_gnss_relative_s &message);
    void sensorGpsUpdate(const sensor_gps_s &message);
    void RTCMDataUpdate(const QByteArray &message);
    void surveyInStatus(float duration, float accuracyMM, double latitude, double longitude, float altitude, bool valid, bool active);

private:
    void _publishSensorGPS();
    void _publishSatelliteInfo();
    void _publishSensorGNSSRelative();

    void _gotRTCMData(const uint8_t *data, size_t len);

    static int _callbackEntry(GPSCallbackType type, void *data1, int data2, void *user);

    QString m_device;
    GPSType m_type;
    const std::atomic_bool &m_requestStop;
    double m_surveyInAccMeters = 0.;
    int m_surveryInDurationSecs = 0;
    bool m_useFixedBaseLoction = false;
    double m_fixedBaseLatitude = 0.;
    double m_fixedBaseLongitude = 0.;
    float m_fixedBaseAltitudeMeters = 0.f;
    float m_fixedBaseAccuracyMeters = 0.f;
    GPSHelper::GPSConfig m_gpsConfig{};

    struct satellite_info_s m_satelliteInfo{};
    struct sensor_gnss_relative_s m_sensorGnssRelative{};
    struct sensor_gps_s m_sensorGps{};

    QSerialPort *m_serial = nullptr;
    QThread *m_thread = nullptr
};
