#include "detailsDlg.h"
#include "mainWnd.h"

#include <QPushButton>
#include <QTableWidget>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

QStringList qslHeaders = {"name","semi-major axis, AU","mass, Earth masses", "speed, km/s", "eccentricity", "inclination, deg", "RAAN, deg", "arg periapsis, deg", "color"};

detailsDlg::detailsDlg(psystemT sys, QWidget* p) : QDialog(p), m_parent(p), m_system(sys)
{
  m_primary = sys->primary;

  setupUI();
  populateControls();
}

detailsDlg::~detailsDlg()
{
 
}

void detailsDlg::setupUI()
{
  QHBoxLayout* btnBox = new QHBoxLayout;

  m_btnOK = new QPushButton("OK");
  m_btnOK->setObjectName("okbutton");

  btnBox->addSpacerItem(new QSpacerItem(100, 16));
  btnBox->addWidget(m_btnOK);

  QLabel* label1 = new QLabel("Primary Star");
  QFormLayout* frmLayout = new QFormLayout();
  m_edtPrimaryMass = new QLineEdit();
  m_edtISL = new QLineEdit();
  m_edtOSL = new QLineEdit();
  frmLayout->addRow("mass, kg              :", m_edtPrimaryMass);
  frmLayout->addRow("inner system limit, AU:", m_edtISL);
  frmLayout->addRow("outer system limit, AU:", m_edtOSL);

  m_tblDetails = new QTableWidget();
  m_tblDetails->setObjectName("detailsTable");
  m_tblDetails->setColumnCount(qslHeaders.size());
  m_tblDetails->setHorizontalHeaderLabels(qslHeaders);

  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(label1);
  mainLayout->addLayout(frmLayout);
  mainLayout->addWidget(m_tblDetails); 
  mainLayout->addLayout(btnBox);

  connect(m_btnOK, &QPushButton::clicked, this, &detailsDlg::onOK);
}



void detailsDlg::populateControls()
{
  int row = 0;
  if (m_system != nullptr)
  {
    m_edtPrimaryMass->setText(QString("%1").arg(m_primary.mass*MSOL));
    m_edtISL->setText(QString("%1").arg(m_primary.isl));
    m_edtOSL->setText(QString("%1").arg(m_primary.osl));

    //for (orbitalPropT prop : *m_pProps)
    //for(system::objectT obj : m_system->objects)
    for(uint32_t ndx = 1; ndx < m_system->cntObjects; ndx++)
    {
      system::objectT* obj = m_system->objects.at(ndx);

      m_tblDetails->insertRow(row);
      m_tblDetails->setItem(row, 0, new QTableWidgetItem(QString("%1").arg(obj->name.c_str())));
      m_tblDetails->setItem(row, 1, new QTableWidgetItem(QString("%1").arg(obj->orbProps.a)));
      m_tblDetails->setItem(row, 2, new QTableWidgetItem(QString("%1").arg(obj->mass/MEARTH)));
      m_tblDetails->setItem(row, 3, new QTableWidgetItem(QString("%1").arg(obj->orbProps.s)));
      m_tblDetails->setItem(row, 4, new QTableWidgetItem(QString("%1").arg(obj->orbProps.e)));
      m_tblDetails->setItem(row, 5, new QTableWidgetItem(QString("%1").arg(obj->orbProps.i)));
      m_tblDetails->setItem(row, 6, new QTableWidgetItem(QString("%1").arg(obj->orbProps.W)));
      m_tblDetails->setItem(row, 7, new QTableWidgetItem(QString("%1").arg(obj->orbProps.w)));
      QLabel* pLabel = new QLabel();
      pLabel->setText("not set");
      if (row+1 < 9)
      {
        pLabel->setStyleSheet(QString("background-color: %1;").arg(colors[row + 1].name()));
        pLabel->setText(colors[row+1].name());
      }
      
      m_tblDetails->setCellWidget(row, 8, pLabel);

      row++;
    }

  }

}

void detailsDlg::onOK()
{
  QDialog::accept();
}

void detailsDlg::onCancel()
{
  QDialog::reject();
}



