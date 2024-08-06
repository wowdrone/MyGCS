/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "RTCMMavlink.h"
#include "QGCApplication.h"
#include "MultiVehicleManager.h"
#include "Vehicle.h"
#include "MAVLinkProtocol.h"
#include <QGCLoggingCategory.h>

QGC_LOGGING_CATEGORY(RTCMMavlinkLog, "qgc.gps.rtcmmavlink")

RTCMMavlink::RTCMMavlink(QObject *parent)
    : QObject(parent)
{
    // qCDebug(RTCMMavlinkLog) << Q_FUNC_INFO << this;

    m_bandwidthTimer.start();
}

RTCMMavlink::~RTCMMavlink()
{
    // qCDebug(RTCMMavlinkLog) << Q_FUNC_INFO << this;
}

void RTCMMavlink::RTCMDataUpdate(QByteArrayView data)
{
    if (m_bandwidthTimer.isValid()) {
        m_bandwidthByteCounter += data.size();

        const qint64 elapsed = m_bandwidthTimer.elapsed();
        if (elapsed > 1000) {
            qCDebug(RTCMMavlinkLog) << QStringLiteral("RTCM bandwidth: %1 kB/s").arg(((m_bandwidthByteCounter / elapsed) * 1000.f) / 1024.f);
            (void) m_bandwidthTimer.restart();
            m_bandwidthByteCounter = 0;
        }
    }

    mavlink_gps_rtcm_data_t gpsRtcmData;
    (void) memset(&gpsRtcmData, 0, sizeof(mavlink_gps_rtcm_data_t));

    static constexpr qsizetype maxMessageLength = MAVLINK_MSG_GPS_RTCM_DATA_FIELD_DATA_LEN;
    if (data.size() < maxMessageLength) {
        gpsRtcmData.len = data.size();
        gpsRtcmData.flags = (m_sequenceId & 0x1FU) << 3;
        (void) memcpy(&gpsRtcmData.data, data.data(), data.size());
        _sendMessageToVehicle(gpsRtcmData);
    } else {
        uint8_t fragmentId = 0;
        qsizetype start = 0;
        while (start < data.size()) {
            gpsRtcmData.flags = 0x01U; // LSB set indicates message is fragmented
            gpsRtcmData.flags |= fragmentId++ << 1; // Next 2 bits are fragment id
            gpsRtcmData.flags |= (m_sequenceId & 0x1FU) << 3; // Next 5 bits are sequence id

            const qsizetype length = std::min(data.size() - start, maxMessageLength);
            gpsRtcmData.len = length;

            (void) memcpy(gpsRtcmData.data, data.constData() + start, length);
            _sendMessageToVehicle(gpsRtcmData);

            start += length;
        }
    }

    ++m_sequenceId;
}

void RTCMMavlink::_sendMessageToVehicle(const mavlink_gps_rtcm_data_t &data)
{
    QmlObjectListModel* const vehicles = qgcApp()->toolbox()->multiVehicleManager()->vehicles();
    for (qsizetype i = 0; i < vehicles->count(); i++) {
        Vehicle* const vehicle = qobject_cast<Vehicle*>(vehicles->get(i));
        const WeakLinkInterfacePtr weakLink = vehicle->vehicleLinkManager()->primaryLink();
        if (!weakLink.expired()) {
            const MAVLinkProtocol* const mavlinkProtocol = qgcApp()->toolbox()->mavlinkProtocol();
            const SharedLinkInterfacePtr sharedLink = weakLink.lock();
            mavlink_message_t message;
            (void) mavlink_msg_gps_rtcm_data_encode_chan(
                mavlinkProtocol->getSystemId(),
                mavlinkProtocol->getComponentId(),
                sharedLink->mavlinkChannel(),
                &message,
                &data
            );
            (void) vehicle->sendMessageOnLinkThreadSafe(sharedLink.get(), message);
        }
    }
}
