/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "SerialLink.h"

#include <QGCLoggingCategory.h>
#include <QGCSerialPortInfo.h>
#ifdef Q_OS_ANDROID
#include <QGCApplication.h>
#include "LinkManager.h"
#include <qserialport.h>
#else
#include <QtSerialPort/QSerialPort>
#endif
#include <QtCore/QSettings>
#include <QtCore/QThread>

QGC_LOGGING_CATEGORY(SerialLinkLog, "qgc.comms.seriallink")

SerialLink::SerialLink(SharedLinkConfigurationPtr& config, bool isPX4Flow, QObject *parent)
    : LinkInterface(config, isPX4Flow, parent)
    , _serialConfig(qobject_cast<const SerialConfiguration*>(config.get()))
    , _port(new QSerialPort(_serialConfig->portName(), this))
{
    // qCDebug(SerialLinkLog) << Q_FUNC_INFO << this;

    (void) qRegisterMetaType<QSerialPort::SerialPortError>("QSerialPort::SerialPortError");

    (void) connect(_port, &QSerialPort::aboutToClose, this, &SerialLink::disconnected, Qt::AutoConnection);

    (void) QObject::connect(_port, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error) {
        qCWarning(SerialLinkLog) << "Serial Link Error:" << error;
        emit communicationError(QStringLiteral("Serial Link Error"), QStringLiteral("Link: %1, %2.").arg(_serialConfig->name(), _port->errorString()));
        /*if (error == QSerialPort::ResourceError) {
            _connectionRemoved();
        }*/
    }, Qt::AutoConnection);
}

SerialLink::~SerialLink()
{
    SerialLink::disconnect();

    // qCDebug(SerialLinkLog) << Q_FUNC_INFO << this;
}

bool SerialLink::isConnected() const
{
    return _port->isOpen();
}

void SerialLink::disconnect()
{
    (void) QObject::disconnect(_port, &QIODevice::readyRead, this, &SerialLink::_readBytes);
    _port->close();

#ifdef Q_OS_ANDROID
    qgcApp()->toolbox()->linkManager()->suspendConfigurationUpdates(false);
#endif
}

bool SerialLink::_connect()
{
    if (isConnected()) {
        return true;
    }

#ifdef Q_OS_ANDROID
    qgcApp()->toolbox()->linkManager()->suspendConfigurationUpdates(true);
#endif

    if (_hardwareConnect()) {
        emit connected();
        return true;
    }

    if (_serialConfig->isAutoConnect()) {
        if (_port->error() == QSerialPort::PermissionError) {
            emit communicationError(QStringLiteral("Link Error"), QStringLiteral("Link %1: %2.").arg(_serialConfig->name(), QStringLiteral("Permission Error.")));
            return false;
        }
    }

    emit communicationError(QStringLiteral("Link Error"), QStringLiteral("Link %1: %2.").arg(_serialConfig->name(), QStringLiteral("Failed to Connect.")));
    return false;
}

bool SerialLink::_hardwareConnect()
{
    const QGCSerialPortInfo portInfo(*_port);
    if (portInfo.isBootloader()) {
        emit communicationError(QStringLiteral("Link Error"), QStringLiteral("Link %1: %2.").arg(_serialConfig->name(), QStringLiteral("Not Connecting to a Bootloader.")));
        return false;
    }

    if (!_port->open(QIODevice::ReadWrite)) {
        emit communicationError(QStringLiteral("Link Error"), QStringLiteral("Link %1: %2.").arg(_serialConfig->name(), QStringLiteral("Could Not Open Port.")));
        return false;
    }

    (void) QObject::connect(_port, &QSerialPort::readyRead, this, &SerialLink::_readBytes, Qt::AutoConnection);

    _port->setDataTerminalReady(true);

    _port->setBaudRate(_serialConfig->baud());
    _port->setDataBits(static_cast<QSerialPort::DataBits>(_serialConfig->dataBits()));
    _port->setFlowControl(static_cast<QSerialPort::FlowControl>(_serialConfig->flowControl()));
    _port->setStopBits(static_cast<QSerialPort::StopBits>(_serialConfig->stopBits()));
    _port->setParity(static_cast<QSerialPort::Parity>(_serialConfig->parity()));

    return true;
}

void SerialLink::_writeBytes(const QByteArray &data)
{
    if (!isConnected()) {
        emit communicationError(QStringLiteral("Link Error"), QStringLiteral("Link %1: %2.").arg(_serialConfig->name(), QStringLiteral("Could Not Send Data - Link is Disconnected!")));
        return;
    }

    if (_port->write(data) <= 0) {
        emit communicationError(QStringLiteral("Link Error"), QStringLiteral("Link %1: %2.").arg(_serialConfig->name(), QStringLiteral("Could Not Send Data - Write Failed!")));
        return;
    }

    emit bytesSent(this, data);
}

void SerialLink::_readBytes()
{
    if (!isConnected()) {
        emit communicationError(QStringLiteral("Link Error"), QStringLiteral("Link %1: %2.").arg(_serialConfig->name(), QStringLiteral("Could Not Read Data - link is Disconnected!")));
        return;
    }

    const qint64 byteCount = _port->bytesAvailable();
    if (byteCount <= 0) {
        emit communicationError(QStringLiteral("Link Error"), QStringLiteral("Link %1: %2.").arg(_serialConfig->name(), QStringLiteral("Could Not Read Data - No Data Available!")));
        return;
    }

    QByteArray buffer(byteCount, Qt::Initialization::Uninitialized);
    if (_port->read(buffer.data(), buffer.size()) < 0) {
        emit communicationError(QStringLiteral("Link Error"), QStringLiteral("Link %1: %2.").arg(_serialConfig->name(), QStringLiteral("Could Not Read Data - Read Failed!")));
        return;
    }

    emit bytesReceived(this, buffer);
}

//------------------------------------------------------------------------------

SerialConfiguration::SerialConfiguration(const QString &name, QObject *parent)
    : LinkConfiguration(name, parent)
{
    // qCDebug(SerialLinkLog) << Q_FUNC_INFO << this;
}

SerialConfiguration::SerialConfiguration(SerialConfiguration *copy, QObject *parent)
    : LinkConfiguration(copy, parent)
{
    // qCDebug(SerialLinkLog) << Q_FUNC_INFO << this;

    Q_CHECK_PTR(copy);

    SerialConfiguration::copyFrom(copy);
}

SerialConfiguration::~SerialConfiguration()
{
    // qCDebug(SerialLinkLog) << Q_FUNC_INFO << this;
}

void SerialConfiguration::copyFrom(LinkConfiguration *source)
{
    Q_CHECK_PTR(source);
    LinkConfiguration::copyFrom(source);

    const SerialConfiguration *serialSource = qobject_cast<const SerialConfiguration*>(source);
    Q_CHECK_PTR(serialSource);

    setBaud(serialSource->baud());
    setDataBits(serialSource->dataBits());
    setFlowControl(serialSource->flowControl());
    setStopBits(serialSource->stopBits());
    setParity(serialSource->parity());
    setPortName(serialSource->portName());
    setPortDisplayName(serialSource->portDisplayName());
    setUsbDirect(serialSource->usbDirect());
}

void SerialConfiguration::setBaud(qint32 baud)
{
    if (baud != _baud) {
        _baud = baud;
        emit baudChanged();
    }
}

void SerialConfiguration::setDataBits(QSerialPort::DataBits databits)
{
    if (databits != _dataBits) {
        _dataBits = databits;
        emit dataBitsChanged();
    }
}

void SerialConfiguration::setFlowControl(QSerialPort::FlowControl flowControl)
{
    if (flowControl != _flowControl) {
        _flowControl = flowControl;
        emit flowControlChanged();
    }
}

void SerialConfiguration::setStopBits(QSerialPort::StopBits stopBits)
{
    if (stopBits != _stopBits) {
        _stopBits = stopBits;
        emit stopBitsChanged();
    }
}

void SerialConfiguration::setParity(QSerialPort::Parity parity)
{
    if (parity != _parity) {
        _parity = parity;
        emit parityChanged();
    }
}

void SerialConfiguration::setPortName(const QString &name)
{
    const QString portName = name.trimmed();
    if (portName.isEmpty()) {
        return;
    }

    if (portName != _portName) {
        _portName = portName;
        emit portNameChanged();
    }

    const QString portDisplayName = cleanPortDisplayName(portName);
    setPortDisplayName(portDisplayName);
}

void SerialConfiguration::setPortDisplayName(const QString &portDisplayName)
{
    if (portDisplayName != _portDisplayName) {
        _portDisplayName = portDisplayName;
        emit portDisplayNameChanged();
    }
}

void SerialConfiguration::setUsbDirect(bool usbDirect)
{
    if (usbDirect != _usbDirect) {
        _usbDirect = usbDirect;
        emit usbDirectChanged(_usbDirect);
    }
}

void SerialConfiguration::loadSettings(QSettings &settings, const QString &root)
{
    settings.beginGroup(root);

    setBaud(settings.value("baud", _baud).toInt());
    setDataBits(static_cast<QSerialPort::DataBits>(settings.value("dataBits", _dataBits).toInt()));
    setFlowControl(static_cast<QSerialPort::FlowControl>(settings.value("flowControl", _flowControl).toInt()));
    setStopBits(static_cast<QSerialPort::StopBits>(settings.value("stopBits", _stopBits).toInt()));
    setParity(static_cast<QSerialPort::Parity>(settings.value("parity", _parity).toInt()));
    setPortName(settings.value("portName", _portName).toString());
    _portDisplayName = settings.value("portDisplayName", _portDisplayName).toString();

    settings.endGroup();
}

void SerialConfiguration::saveSettings(QSettings &settings, const QString &root)
{
    settings.beginGroup(root);

    settings.setValue("baud", _baud);
    settings.setValue("dataBits", _dataBits);
    settings.setValue("flowControl", _flowControl);
    settings.setValue("stopBits", _stopBits);
    settings.setValue("parity", _parity);
    settings.setValue("portName", _portName);
    settings.setValue("portDisplayName", _portDisplayName);

    settings.endGroup();
}

QStringList SerialConfiguration::supportedBaudRates()
{
    QStringList supportBaudRateStrings;
    for (qint32 rate : QSerialPortInfo::standardBaudRates()) {
        (void) supportBaudRateStrings.append(QString::number(rate));
    }

    return supportBaudRateStrings;
}

QString SerialConfiguration::cleanPortDisplayName(const QString &name)
{
    QString portName = name.trimmed();
#ifdef Q_OS_WIN
    (void) portName.replace("\\\\.\\", "");
#else
    (void) portName.replace("/dev/cu.", "");
    (void) portName.replace("/dev/", "");
#endif

    return portName;
}
