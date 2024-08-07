/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include <QtCore/QLoggingCategory>
#include <QtCore/QString>

#ifdef Q_OS_ANDROID
#include "qserialport.h"
#else
#include <QtSerialPort/QSerialPort>
#endif

#include "LinkConfiguration.h"
#include "LinkInterface.h"

Q_DECLARE_LOGGING_CATEGORY(SerialLinkLog)

class QSerialPort;

class SerialConfiguration : public LinkConfiguration
{
    Q_OBJECT

    Q_PROPERTY(qint32                   baud            READ baud            WRITE setBaud        NOTIFY baudChanged)
    Q_PROPERTY(QSerialPort::DataBits    dataBits        READ dataBits        WRITE setDataBits    NOTIFY dataBitsChanged)
    Q_PROPERTY(QSerialPort::FlowControl flowControl     READ flowControl     WRITE setFlowControl NOTIFY flowControlChanged)
    Q_PROPERTY(QSerialPort::StopBits    stopBits        READ stopBits        WRITE setStopBits    NOTIFY stopBitsChanged)
    Q_PROPERTY(QSerialPort::Parity      parity          READ parity          WRITE setParity      NOTIFY parityChanged)
    Q_PROPERTY(QString                  portName        READ portName        WRITE setPortName    NOTIFY portNameChanged)
    Q_PROPERTY(QString                  portDisplayName READ portDisplayName                      NOTIFY portDisplayNameChanged)
    Q_PROPERTY(bool                     usbDirect       READ usbDirect       WRITE setUsbDirect   NOTIFY usbDirectChanged)

public:
    explicit SerialConfiguration(const QString &name, QObject *parent = nullptr);
    explicit SerialConfiguration(SerialConfiguration *copy, QObject *parent = nullptr);
    virtual ~SerialConfiguration();

    qint32 baud() const { return _baud; }
    void setBaud(qint32 baud);
    QSerialPort::DataBits dataBits() const { return _dataBits; }
    void setDataBits(QSerialPort::DataBits databits);
    QSerialPort::FlowControl flowControl() const { return _flowControl; }
    void setFlowControl(QSerialPort::FlowControl flowControl);
    QSerialPort::StopBits stopBits() const { return _stopBits; }
    void setStopBits(QSerialPort::StopBits stopBits);
    QSerialPort::Parity parity() const { return _parity; }
    void setParity(QSerialPort::Parity parity);
    QString portName() const { return _portName; }
    void setPortName(const QString& name);
    QString portDisplayName() const { return _portDisplayName; }
    void setPortDisplayName(const QString &portDisplayName);
    bool usbDirect() const { return _usbDirect; }
    void setUsbDirect(bool usbDirect);

    LinkType type() override { return LinkConfiguration::TypeSerial; }
    void copyFrom(LinkConfiguration *source) override;
    void loadSettings(QSettings &settings, const QString &root) override;
    void saveSettings(QSettings &settings, const QString &root) override;
    QString settingsURL() override { return QStringLiteral("SerialSettings.qml"); }
    QString settingsTitle() override { return QStringLiteral("Serial Link Settings"); }

    static QStringList supportedBaudRates();
    static QString cleanPortDisplayName(const QString &name);

signals:
    void baudChanged();
    void dataBitsChanged();
    void flowControlChanged();
    void stopBitsChanged();
    void parityChanged();
    void portNameChanged();
    void portDisplayNameChanged();
    void usbDirectChanged(bool usbDirect);

private:
    qint32 _baud = QSerialPort::Baud57600;
    QSerialPort::DataBits _dataBits = QSerialPort::Data8;
    QSerialPort::FlowControl _flowControl = QSerialPort::NoFlowControl;
    QSerialPort::StopBits _stopBits = QSerialPort::OneStop;
    QSerialPort::Parity _parity = QSerialPort::NoParity;
    QString _portName;
    QString _portDisplayName;
    bool _usbDirect = false;
};

//------------------------------------------------------------------------------

class SerialLink : public LinkInterface
{
    Q_OBJECT

public:
    explicit SerialLink(SharedLinkConfigurationPtr &config, bool isPX4Flow = false, QObject *parent = nullptr);
    virtual ~SerialLink();

    void run() override {}
    bool isConnected() const override;
    void disconnect() override;

    const QSerialPort *port() const { return _port; }

private slots:
    void _writeBytes(const QByteArray &data) override;
    void _readBytes();

private:
    bool _connect() override;
    bool _hardwareConnect();

    const SerialConfiguration *_serialConfig = nullptr;
    QSerialPort *_port = nullptr;
};
