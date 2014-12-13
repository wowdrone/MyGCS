/*=====================================================================
 
 QGroundControl Open Source Ground Control Station
 
 (c) 2009 - 2014 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 
 This file is part of the QGROUNDCONTROL project
 
 QGROUNDCONTROL is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 QGROUNDCONTROL is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.
 
 ======================================================================*/

/// @file
///     @author Don Gagne <don@thegagnes.com>

#include "SetupView.h"

#include "UASManager.h"
#include "AutoPilotPluginManager.h"
#include "VehicleComponent.h"
#include "VehicleComponentButton.h"
#include "SummaryPage.h"
#include "PX4FirmwareUpgrade.h"

#
#include <QQmlError>
#include <QQmlContext>
#include <QDebug>

SetupView::SetupView(QWidget* parent) :
    QGCQuickWidget(parent),
    _uasCurrent(NULL),
    _initComplete(false),
    _autoPilotPlugin(NULL)
{
    bool fSucceeded = connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(_setActiveUAS(UASInterface*)));
    Q_UNUSED(fSucceeded);
    Q_ASSERT(fSucceeded);
}

SetupView::~SetupView()
{

}

void SetupView::_setActiveUAS(UASInterface* uas)
{
    if (_uasCurrent) {
        Q_ASSERT(_autoPilotPlugin);
        disconnect(_autoPilotPlugin);
        _autoPilotPlugin = NULL;
    }

    _uasCurrent = uas;
    
    if (_uasCurrent) {
        _autoPilotPlugin = AutoPilotPluginManager::instance()->getInstanceForAutoPilotPlugin(_uasCurrent);
        
        connect(_autoPilotPlugin, &AutoPilotPlugin::pluginReady, this, &SetupView::_pluginReady);
        if (_autoPilotPlugin->pluginIsReady()) {
            _pluginReady();
        }
    }
}

void SetupView::_pluginReady(void)
{
    Q_ASSERT(_uasCurrent);
    Q_ASSERT(_autoPilotPlugin);
    
    setUAS(_uasCurrent);
    setSource(QUrl::fromUserInput("qrc:qml/SetupView.qml"));
    disconnect(_autoPilotPlugin);
    
    QObject* flowObject = rootObject()->findChild<QObject*>("flow");
    Q_ASSERT(flowObject);
    QQuickItem* flowItem = qobject_cast<QQuickItem*>(flowObject);
    Q_ASSERT(flowItem);
    
    QList<VehicleComponent*> components = _autoPilotPlugin->getVehicleComponents();
    foreach(VehicleComponent* component, components) {
        component->addSummaryQmlComponent(rootContext(), flowItem);
    }
}
