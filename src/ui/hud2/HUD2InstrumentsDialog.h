#ifndef HUD2INSTRUMENTSDIALOG_H
#define HUD2INSTRUMENTSDIALOG_H

#include <QDialog>
#include "HUD2Ribbon.h"

#include "HUD2RibbonForm.h"
#include "HUD2IndicatorHorizon.h"
#include "HUD2IndicatorFps.h"
#include "HUD2IndicatorRoll.h"
#include "HUD2IndicatorSpeed.h"
#include "HUD2IndicatorClimb.h"
#include "HUD2IndicatorCompass.h"

namespace Ui {
class HUD2InstrumentsDialog;
}

class HUD2InstrumentsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit HUD2InstrumentsDialog(HUD2IndicatorHorizon *horizon, HUD2IndicatorRoll *roll, HUD2IndicatorSpeed *speed, HUD2IndicatorClimb *climb, HUD2IndicatorCompass *compass, HUD2IndicatorFps *fps, QWidget *parent = 0);
    ~HUD2InstrumentsDialog();
    
private:
    Ui::HUD2InstrumentsDialog *ui;
};

#endif // HUD2INSTRUMENTSDIALOG_H
