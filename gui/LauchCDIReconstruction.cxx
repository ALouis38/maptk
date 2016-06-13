#include "LauchCDIReconstruction.h"
#include "ui_LauchCDIReconstruction.h"

LauchCDIReconstruction::LauchCDIReconstruction(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::LauchCDIReconstruction)
{
  ui->setupUi(this);
}

LauchCDIReconstruction::~LauchCDIReconstruction()
{
  delete ui;
}
