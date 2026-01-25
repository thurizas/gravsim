#include "simPropertiesDlg.h"

#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QCheckBox>


static QStringList qslUnits = { "seconds", "minutes", "hours", "days", "months(30 day)", "years" };

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
  QFormLayout* accreteFormLayout = new QFormLayout;

  m_edtStart = new QLineEdit;
  if (nullptr != m_pcontext) m_edtStart->setText(QString("%1").arg(m_pcontext->start));
  
  QHBoxLayout* row2layout = new QHBoxLayout;
  m_edtDelta = new QLineEdit;
  m_cboUnits = new QComboBox;
  m_cboUnits->addItems(qslUnits);
  if (nullptr != m_pcontext)
  {
    m_edtDelta->setText(QString("%1").arg(m_pcontext->deltaT));
    m_cboUnits->setCurrentIndex(0);                                 // internally time-step is stored in seconds.
  }

  row2layout->addWidget(m_edtDelta);
  row2layout->addWidget(m_cboUnits);

  QHBoxLayout* row3layout = new QHBoxLayout;
  m_edtEnd = new QLineEdit;
  m_cboUnits2 = new QComboBox;
  m_cboUnits2->addItems(qslUnits);
  if (nullptr != m_pcontext)
  {
    m_edtEnd->setText(QString("%1").arg(m_pcontext->duration));
    m_cboUnits2->setCurrentIndex(qslUnits.indexOf(QString(m_pcontext->durUnits.c_str())));
  }

  m_chkOrbits = new QCheckBox;
  if(m_pcontext != nullptr) m_chkOrbits->setChecked(m_pcontext->showOrbits);

  row3layout->addWidget(m_edtEnd);
  row3layout->addWidget(m_cboUnits2);

  QHBoxLayout* row4layout = new QHBoxLayout;

  m_edtFileName = new QLineEdit;
  if (nullptr != m_pcontext) m_edtFileName->setText(QString("%1").arg(m_pcontext->datafile.c_str()));
  m_btnChoose = new QPushButton();
  m_btnChoose->setText("...");

  row4layout->addWidget(m_edtFileName);
  row4layout->addWidget(m_btnChoose);
  
  formLayout->addRow("Starting date (years)    : ", m_edtStart);
  formLayout->addRow("Simulation step          : ", row2layout);
  formLayout->addRow("Simulation length (years): ", row3layout);
  formLayout->addRow("Show orbits              : ", m_chkOrbits);
  formLayout->addRow("Data file (if any)       : ", row4layout);

  QFrame* line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setStyleSheet("background-color: black;");

  // setup properties for accretion model
  m_edtDisp = new QLineEdit(); if (m_pcontext != nullptr) m_edtDisp->setText(QString("%1").arg(m_pcontext->accreteCtx.dispAngle));

  QHBoxLayout* row5Layout = new QHBoxLayout;
  m_edtMinDiskMass = new QLineEdit; if (m_pcontext != nullptr) m_edtMinDiskMass->setText(QString("%1").arg(m_pcontext->accreteCtx.minMass));
  m_edtAvgDiskMass = new QLineEdit; if (m_pcontext != nullptr) m_edtAvgDiskMass->setText(QString("%1").arg(m_pcontext->accreteCtx.avgMass));
  m_edtMaxDiskMass = new QLineEdit; if (m_pcontext != nullptr) m_edtMaxDiskMass->setText(QString("%1").arg(m_pcontext->accreteCtx.maxMass));
  row5Layout->addWidget(m_edtMinDiskMass);
  row5Layout->addWidget(m_edtAvgDiskMass);
  row5Layout->addWidget(m_edtMaxDiskMass);

  m_edtGasDust     = new QLineEdit(); if (m_pcontext != nullptr) m_edtGasDust->setText(QString("%1").arg(m_pcontext->accreteCtx.gasDustRatio));
  m_edtCntBands    = new QLineEdit(); if (m_pcontext != nullptr) m_edtCntBands->setText(QString("%1").arg(m_pcontext->accreteCtx.cntBands));
  m_edtCntNucleii  = new QLineEdit(); if (m_pcontext != nullptr) m_edtCntNucleii->setText(QString("%1").arg(m_pcontext->accreteCtx.cntNuclei));
  m_edtInitialMass = new QLineEdit(); if (m_pcontext != nullptr) m_edtInitialMass->setText(QString("%1").arg(m_pcontext->accreteCtx.initialMass));
  m_edtAlpha       = new QLineEdit(); if (m_pcontext != nullptr) m_edtAlpha->setText(QString("%1").arg(m_pcontext->accreteCtx.alpha));
  m_edtPowerExp    = new QLineEdit(); if (m_pcontext != nullptr) m_edtPowerExp->setText(QString("%1").arg(m_pcontext->accreteCtx.n));

  accreteFormLayout->addRow("dispersion angle (degrees)           :", m_edtDisp);
  accreteFormLayout->addRow("disk mass (min/avg/mass)             :", row5Layout);
  accreteFormLayout->addRow("gas to dust ratio                    :", m_edtGasDust);
  accreteFormLayout->addRow("number of band disk partitioned into :", m_edtCntBands);
  accreteFormLayout->addRow("number of initial nucleii present    :", m_edtCntNucleii);
  accreteFormLayout->addRow("initial mass of nucleii (solar mass) :", m_edtInitialMass);
  accreteFormLayout->addRow("alpha                                :", m_edtAlpha);
  accreteFormLayout->addRow("inverse power law exponent (Dole's n):", m_edtPowerExp);
  
  QVBoxLayout*  mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(formLayout);
  mainLayout->addWidget(line);
  mainLayout->addLayout(accreteFormLayout);
  mainLayout->addLayout(btnLayout); 


  connect(btnOK, &QPushButton::clicked, this, &simPropertiesDlg::onOK);
  connect(btnCancel, &QPushButton::clicked, this, &simPropertiesDlg::onCancel);
  connect(m_btnChoose, &QPushButton::clicked, this, &simPropertiesDlg::onChooseFile);
}

// time step is stored internally as seconds.
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


  // read the values from teh form and stach in the context
  m_pcontext->start = (m_edtStart->text()).toDouble();
  m_pcontext->deltaT = factor * (m_edtDelta->text()).toDouble();
  m_pcontext->duration = (m_edtEnd->text()).toDouble();
  m_pcontext->durUnits = (qslUnits.at(m_cboUnits2->currentIndex()).toStdString());
  m_pcontext->datafile = (m_edtFileName->text()).toStdString();
  m_pcontext->showOrbits = m_chkOrbits->isChecked();

  m_pcontext->accreteCtx.dispAngle = (m_edtDisp->text()).toDouble();
  m_pcontext->accreteCtx.minMass = (m_edtMinDiskMass->text()).toDouble();
  m_pcontext->accreteCtx.avgMass = (m_edtAvgDiskMass->text()).toDouble();
  m_pcontext->accreteCtx.maxMass = (m_edtMaxDiskMass->text()).toDouble();
  m_pcontext->accreteCtx.gasDustRatio = (m_edtGasDust->text()).toDouble();
  m_pcontext->accreteCtx.cntBands = (m_edtCntBands->text()).toInt();
  m_pcontext->accreteCtx.cntNuclei = (m_edtCntNucleii->text()).toInt();
  m_pcontext->accreteCtx.initialMass = (m_edtInitialMass->text()).toDouble();
  m_pcontext->accreteCtx.alpha = (m_edtAlpha->text()).toDouble();
  m_pcontext->accreteCtx.n = (m_edtPowerExp->text()).toDouble();

  QDialog::accept();              // call base-class accept funtion
}

void simPropertiesDlg::onCancel()
{
  QDialog::reject();              // call base-class reject funtion
}

void simPropertiesDlg::onChooseFile()
{
  QString name = QFileDialog::getOpenFileName(this, "Select data file to use", QDir::currentPath(), "Json files (*.json);;text file (*.txt *.dat)");
  if (!name.isEmpty())
  {
    m_edtFileName->setText(name);
    m_bNewFile = true;
  }
}
