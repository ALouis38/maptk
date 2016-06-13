/*ckwg +29
 * Copyright 2015 by Kitware, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  * Neither name of Kitware, Inc. nor the names of any contributors may be used
 *    to endorse or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "LaunchCDIReconstruction.h"

#include "ui_LaunchCDIReconstruction.h"

#include "OutputDialog.h"

#include <qtUiState.h>
#include <qtUiStateItem.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <QProcess>
#include <QByteArray>
#include <QFileDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

///////////////////////////////////////////////////////////////////////////////

//BEGIN LaunchCDIReconstructionPrivate

//-----------------------------------------------------------------------------
class LaunchCDIReconstructionPrivate
{
public:

  Ui::LaunchCDIReconstruction UI;
  qtUiState uiState;

  QStringList args;

  QProcess *rec;

  void addArg(QWidget* item, std::string value="");
  void addArg(QWidget *item, double value);

  OutputDialog* dialog;
};

QTE_IMPLEMENT_D_FUNC(LaunchCDIReconstruction)

//END LaunchCDIReconstructionPrivate

//BEGIN LaunchCDIReconstruction

//-----------------------------------------------------------------------------
LaunchCDIReconstruction::LaunchCDIReconstruction(QWidget* parent, Qt::WindowFlags flags)
  : QWidget(parent, flags), d_ptr(new LaunchCDIReconstructionPrivate)
{
  QTE_D();

  this->setAttribute(Qt::WA_DeleteOnClose);

  // Set up UI
  d->UI.setupUi(this);

  // Set up UI persistence and restore previous state
  d->uiState.setCurrentGroup("LaunchCDIReconstruction");


  // Set up signals/slots
  connect(d->UI.pushButtonCompute, SIGNAL(clicked(bool)),
          this, SLOT(compute()));

  d->dialog = new OutputDialog();

  d->rec = new QProcess();

  connect(d->rec, SIGNAL(readyRead()),
         d->dialog, SLOT(ouputProcess()));

  connect(d->rec, SIGNAL(finished(int)),
         this, SLOT(initialState()));

  connect(d->UI.pushButtonStop, SIGNAL(clicked(bool)),
         d->rec, SLOT(kill()));

  connect(d->UI.pushButtonExplore, SIGNAL(clicked(bool)),
         this, SLOT(openFileExplorer()));

  connect(d->UI.pushButtonExploreDM, SIGNAL(clicked(bool)),
         this, SLOT(openDMFileExplorer()));
}

//-----------------------------------------------------------------------------
LaunchCDIReconstruction::~LaunchCDIReconstruction()
{
  QTE_D();
  d->uiState.save();
}

//-----------------------------------------------------------------------------
void LaunchCDIReconstruction::compute()
{
  QTE_D();

  std::string recPath = d->UI.lineEditReconstructionPath->text().toStdString();


  //Parsing arguments from form

  for (int i = 0; i < this->children().size(); ++i)
  {
    if(this->children().at(i)->property("configField").isValid())
    {
      if (this->children().at(i)->inherits("QCheckBox"))
      {
        QCheckBox *child = (QCheckBox*) this->children().at(i);

        if (child->isChecked())
        {
          d->addArg(child);
        }
      }
      else if (this->children().at(i)->inherits("QComboBox"))
      {
        QComboBox *child = (QComboBox*) this->children().at(i);

        d->addArg(child, child->currentText().toStdString());
      }
      else if (this->children().at(i)->inherits("QDoubleSpinBox"))
      {
        QDoubleSpinBox *child = (QDoubleSpinBox*) this->children().at(i);

        d->addArg(child,child->value());
      }
      else if (this->children().at(i)->inherits("QLineEdit"))
      {
        QLineEdit *child = (QLineEdit*) this->children().at(i);

        d->addArg(child,child->text().toStdString());
      }
    }
    else if (this->children().at(i)->property("configFieldPart").isValid())
    {
      QSpinBox *child = (QSpinBox*) this->children().at(i);
      if (child->property("Part").toInt() == 1)
      {
        std::string value = findValue(child->property("configFieldPart").toString(), child->value());

        d->addArg(child,value);
      }
    }

  }

  d->args << "--outputMeshFilename" << QString(d->UI.lineEditOutputsName->text() + ".vtp");
  d->args <<"--outputGridFilename" <<  QString(d->UI.lineEditOutputsName->text() + ".vts");
  d->args << "--verbose";
  d->dialog->show();
  d->dialog->setOutputToDisplay(d->rec);

  runningState();

  d->rec->setProcessChannelMode(QProcess::MergedChannels);
  d->rec->start(QString::fromStdString(recPath),d->args);
}

//-----------------------------------------------------------------------------
void LaunchCDIReconstruction::initialState()
{
  QTE_D();

  d->args.clear();
  for (int i = 0; i < this->children().size(); ++i) {
    if (this->children().at(i)->inherits("QWidget")) {
      QWidget *child = (QWidget*) this->children().at(i);
      child->setEnabled(true);
    }
  }
  d->UI.pushButtonStop->setEnabled(false);

}

//-----------------------------------------------------------------------------
void LaunchCDIReconstruction::openFileExplorer()
{
  QTE_D();
  QString path = QFileDialog::getOpenFileName();

  d->UI.lineEditReconstructionPath->setText(path);

}

void LaunchCDIReconstruction::openDMFileExplorer()
{

  QTE_D();

  QString path = QFileDialog::getExistingDirectory();

  d->UI.lineEditDMPath->setText(path);
}

std::string LaunchCDIReconstruction::findValue(QString configFieldPart, double firstValue)
{
  double secondValue, thirdValue;

  for (int i = 0; i < this->children().size(); ++i)
  {
    if (this->children().at(i)->property("configFieldPart").isValid()
        && this->children().at(i)->property("configFieldPart").toString() == configFieldPart)
    {
      QSpinBox *child = (QSpinBox*) this->children().at(i);

      if (child->property("Part").toInt() == 2)
      {
        secondValue = child->value();
      }
      else if (child->property("Part").toInt() == 3)
      {
        thirdValue = child->value();
      }
    }
  }

  std::ostringstream sstream;
  sstream << firstValue << " " << secondValue << " " << thirdValue;

  return sstream.str();
}

//-----------------------------------------------------------------------------
void LaunchCDIReconstruction::runningState()
{
  QTE_D();

  for (int i = 0; i < this->children().size(); ++i)
  {
    if (this->children().at(i)->inherits("QWidget"))
    {
      QWidget *child = (QWidget*) this->children().at(i);
      child->setEnabled(false);
    }
  }
  d->UI.pushButtonStop->setEnabled(true);
}

//END LaunchCDIReconstruction

//-----------------------------------------------------------------------------
void LaunchCDIReconstructionPrivate::addArg(QWidget *item, std::string value)
{
  std::string arg ;

  if (item->property("configField").isValid())
  {
    arg = item->property("configField").toString().toStdString();
  }
  else if (item->property("configFieldPart").isValid())
  {
    arg = item->property("configFieldPart").toString().toStdString();
  }

  args << QString::fromStdString(arg);

  if(!value.empty())
  {
    args << QString::fromStdString(value);
  }
}

//-----------------------------------------------------------------------------
void LaunchCDIReconstructionPrivate::addArg(QWidget *item, double value)
{
  std::ostringstream sstream;
  sstream << value;
  std::string valueStr = sstream.str();

  addArg(item,valueStr);
}
