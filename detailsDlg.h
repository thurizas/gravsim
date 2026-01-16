#ifndef _detailsdlg_h_
#define _detailsdlg_h_

#include "common.h"
#include "constants.h"

#include <QDialog>

class QPushButton;
class QTableWidget;
class QLineEdit;

class detailsDlg : public QDialog
{
public:
  detailsDlg(std::vector<orbitalPropT>, psystemT, QWidget* parent = nullptr);
  ~detailsDlg();

  void populateControls();


private slots:
  void onOK();
  void onCancel();


private:
  void setupUI();

  QPushButton*    m_btnOK;
  //QPushButton*    m_btnCancel;

  QTableWidget*   m_tblDetails;
  QLineEdit*      m_edtPrimaryMass;
  QLineEdit*      m_edtISL;
  QLineEdit*      m_edtOSL;
  //QLineEdit*      m_edtTimeStep;

  QWidget*        m_parent;
  system::starT   m_primary;
  std::vector<orbitalPropT>*  m_pProps;
};

#endif

