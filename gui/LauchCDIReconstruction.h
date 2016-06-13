#ifndef LAUCHCDIRECONSTRUCTION_H
#define LAUCHCDIRECONSTRUCTION_H

#include <QWidget>

namespace Ui {
class LauchCDIReconstruction;
}

class LauchCDIReconstruction : public QWidget
{
  Q_OBJECT

public:
  explicit LauchCDIReconstruction(QWidget *parent = 0);
  ~LauchCDIReconstruction();

private:
  Ui::LauchCDIReconstruction *ui;
};

#endif // LAUCHCDIRECONSTRUCTION_H
