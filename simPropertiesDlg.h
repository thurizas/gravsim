#ifndef _simpropertiesdlg_h_
#define _simpropertiesdlg_h_

#include "common.h"

#include <QDialog>

class QLineEdit;
class QComboBox;
class QCheckBox;


class simPropertiesDlg :public QDialog
{
  Q_OBJECT

public:
  simPropertiesDlg(pctxT pctx, QWidget* parent = nullptr);

  void getParams(pctxT pctx);
  bool didFileNameChange() { return m_bNewFile; }

private slots:
  void onOK();
  void onCancel();
  void onChooseFile();

private:
  void setupUI();

  ctxT*        m_pcontext;

  QLineEdit*   m_edtStart = nullptr;
  QLineEdit*   m_edtDelta = nullptr;
  QLineEdit*   m_edtEnd = nullptr;
  QLineEdit*   m_edtFileName = nullptr;
  QComboBox*   m_cboUnits = nullptr;
  QComboBox*   m_cboUnits2 = nullptr;
  QCheckBox*   m_chkOrbits = nullptr;
  QPushButton* m_btnChoose = nullptr;

  // accrete model variables
  QLineEdit* m_edtDisp = nullptr;
  QLineEdit* m_edtMinDiskMass = nullptr;
  QLineEdit* m_edtAvgDiskMass = nullptr;
  QLineEdit* m_edtMaxDiskMass = nullptr;
  QLineEdit* m_edtGasDust = nullptr;
  QLineEdit* m_edtCntBands = nullptr;
  QLineEdit* m_edtCntNucleii = nullptr;
  QLineEdit* m_edtInitialMass = nullptr;
  QLineEdit* m_edtAlpha = nullptr;
  QLineEdit* m_edtPowerExp = nullptr;

  bool         m_bNewFile = false;
};

#endif

