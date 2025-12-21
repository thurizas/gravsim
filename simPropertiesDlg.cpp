#include "simPropertiesDlg.h"

#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QLineEdit>
#include <QComboBox>

simPropertiesDlg::simPropertiesDlg(pctxT pctx, QWidget* p) : QDialog(p), m_pcontext(pctx)
{
  setupUI();
}

void simPropertiesDlg::getParams(pctxT pctx)
{
  pctx = m_pcontext;
}

void simPropertiesDlg::setupUI()
{
  if (this->objectName().isEmpty())
    this->setObjectName("propertiesDlg");
  this->setWindowTitle("Simulation Parameters");
  this->resize(450, 350);

  QHBoxLayout* btnLayout = new QHBoxLayout();
  QPushButton* btnOK = new QPushButton("OK");
  QPushButton* btnCancel = new QPushButton("Cancel");

  btnLayout->addSpacerItem(new QSpacerItem(100, 16));
  btnLayout->addWidget(btnOK);
  btnLayout->addWidget(btnCancel);

  QFormLayout* formLayout = new QFormLayout;

  m_edtStart = new QLineEdit;
  if (nullptr != m_pcontext) m_edtStart->setText(QString("%1").arg(m_pcontext->start));
  

  QHBoxLayout* row2layout = new QHBoxLayout;
  QStringList qslUnits = { "seconds", "minutes", "hours", "days", "months(30 day)", "years" };
  m_edtDelta = new QLineEdit;
  if(nullptr != m_pcontext) m_edtDelta->setText(QString("%1").arg(m_pcontext->deltaT));
  m_cboUnits = new QComboBox;
  m_cboUnits->addItems(qslUnits);

  row2layout->addWidget(m_edtDelta);
  row2layout->addWidget(m_cboUnits);

  m_edtEnd = new QLineEdit;
  if (nullptr != m_pcontext) m_edtEnd->setText(QString("%1").arg(m_pcontext->duration));
  m_edtFileName = new QLineEdit;
  if (nullptr != m_pcontext) m_edtFileName->setText(QString("%1").arg(m_pcontext->datafile.c_str()));
  
  formLayout->addRow("Starting date (years)    : ", m_edtStart);
  formLayout->addRow("Simulation step          : ", row2layout);
  formLayout->addRow("Simulation length (years): ", m_edtEnd);
  formLayout->addRow("Data file (if any)       : ", m_edtFileName);

  QVBoxLayout*  mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(formLayout);
  mainLayout->addLayout(btnLayout); 

  connect(btnOK, &QPushButton::clicked, this, &simPropertiesDlg::onOK);
  connect(btnCancel, &QPushButton::clicked, this, &simPropertiesDlg::onCancel);
}

void simPropertiesDlg::onOK()
{
  double factor = 1.0;                               // used to convert time interval into seconds. default value assumes input in seconds.
  int32_t ndx = m_cboUnits->currentIndex();
  switch (ndx)
  {
      case 0:                // using seconds
        factor = 1.0; break;
      case 1:                // using minutes
        factor = 60.0; break;
      case 2:                // using hours
        factor = 3600.0; break;
      case 3:                // days
        factor = 86400.0; break;
      case 4:                // months
        factor = 30.0 * 86400.0; break;
      case 5:
        factor = 365.25 * 86400.0; break;
  }


  // TODO : read the values from teh form and stach in the context
  m_pcontext->start = (m_edtStart->text()).toDouble();
  m_pcontext->deltaT = factor * (m_edtDelta->text()).toDouble();
  m_pcontext->duration = (m_edtEnd->text()).toDouble();


  QDialog::accept();              // call base-class accept funtion
}

void simPropertiesDlg::onCancel()
{
  QDialog::reject();              // call base-class reject funtion
}
