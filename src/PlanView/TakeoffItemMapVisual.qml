/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick          2.3
import QtQuick.Controls 1.2
import QtLocation       5.3
import QtPositioning    5.3

import QGroundControl               1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controls      1.0
import QGroundControl.FlightMap     1.0

/// Simple Mission Item visuals
Item {
    id: _root

    property var map        ///< Map control to place item in
    property var vehicle    ///< Vehicle associated with this item

    property var    _missionItem:           object
    property var    _takeoffIndicatorItem
    property var    _launchIndicatorItem

    signal clicked(int sequenceNumber)

    function addCommonVisuals() {
        if (_objMgrCommonVisuals.empty) {
            _takeoffIndicatorItem = _objMgrCommonVisuals.createObject(takeoffIndicatorComponent, map, true /* addToMap */)
            _launchIndicatorItem = _objMgrCommonVisuals.createObject(launchIndicatorComponent, map, true /* addToMap */)
        }
    }

    function addEditingVisuals() {
        if (_objMgrEditingVisuals.empty) {
            _objMgrEditingVisuals.createObjects([ takeoffDragComponent, launchDragComponent ], map, false /* addToMap */)
        }
    }

    QGCDynamicObjectManager { id: _objMgrCommonVisuals }
    QGCDynamicObjectManager { id: _objMgrEditingVisuals }

    Component.onCompleted: {
        addCommonVisuals()
        if (_missionItem.isCurrentItem && map.planView) {
            addEditingVisuals()
        }
    }

    Connections {
        target: _missionItem

        onIsCurrentItemChanged: {
            if (_missionItem.isCurrentItem && map.planView) {
                addEditingVisuals()
            } else {
                _objMgrEditingVisuals.destroyObjects()
            }
        }
    }

    Component {
        id: takeoffDragComponent

        MissionItemIndicatorDrag {
            mapControl:     _root.map
            itemIndicator:  _takeoffIndicatorItem
            itemCoordinate: _missionItem.specifiesCoordinate ? _missionItem.coordinate : _missionItem.launchCoordinate

            onItemCoordinateChanged: {
                if (_missionItem.specifiesCoordinate) {
                    _missionItem.coordinate = itemCoordinate
                } else {
                    _missionItem.launchCoordinate = itemCoordinate
                }
            }
        }
    }

    Component {
        id: launchDragComponent

        MissionItemIndicatorDrag {
            mapControl:     _root.map
            itemIndicator:  _launchIndicatorItem
            itemCoordinate: _missionItem.launchCoordinate
            visible:        !_missionItem.launchTakeoffAtSameLocation

            onItemCoordinateChanged: _missionItem.launchCoordinate = itemCoordinate
        }
    }

    Component {
        id: takeoffIndicatorComponent

        MissionItemIndicator {
            coordinate:     _missionItem.specifiesCoordinate ? _missionItem.coordinate : _missionItem.launchCoordinate
            z:              QGroundControl.zOrderMapItems
            missionItem:    _missionItem
            sequenceNumber: _missionItem.sequenceNumber
            onClicked:      _root.clicked(_missionItem.sequenceNumber)
        }
    }

    Component {
        id: launchIndicatorComponent

        MapQuickItem {
            coordinate:     _missionItem.launchCoordinate
            anchorPoint.x:  sourceItem.anchorPointX
            anchorPoint.y:  sourceItem.anchorPointY
            visible:        !_missionItem.launchTakeoffAtSameLocation

            sourceItem:
                MissionItemIndexLabel {
                    checked:            _missionItem.isCurrentItem
                    label:              qsTr("Launch")
                    highlightSelected:  true
                    onClicked:          _root.clicked(_missionItem.sequenceNumber)
                }
        }
    }
}
