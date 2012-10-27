#include "QGCHilConfiguration.h"
#include "ui_QGCHilConfiguration.h"

#include "QGCHilFlightGearConfiguration.h"
#include "QGCHilXPlaneConfiguration.h"

QGCHilConfiguration::QGCHilConfiguration(UAS *mav, QWidget *parent) :
    QWidget(parent),
    mav(mav),
    ui(new Ui::QGCHilConfiguration)
{
    ui->setupUi(this);
}

void QGCHilConfiguration::receiveStatusMessage(const QString& message)
{
    ui->statusLabel->setText(message);
}

QGCHilConfiguration::~QGCHilConfiguration()
{
    delete ui;
}

void QGCHilConfiguration::on_simComboBox_currentIndexChanged(int index)
{
    if(1 == index)
    {
        // Ensure the sim exists and is disabled
        mav->enableHilFlightGear(false, "");
        QGCHilFlightGearConfiguration* hfgconf = new QGCHilFlightGearConfiguration(mav, this);
        hfgconf->show();
        ui->simulatorConfigurationDockWidget->setWidget(hfgconf);

    }
    else if(2 == index || 3 == index)
    {
        // Ensure the sim exists and is disabled
        mav->enableHilXPlane(false);
        QGCHilXPlaneConfiguration* hxpconf = new QGCHilXPlaneConfiguration(mav->getHILSimulation(), this);
        hxpconf->show();
        ui->simulatorConfigurationDockWidget->setWidget(hxpconf);

    }
}
