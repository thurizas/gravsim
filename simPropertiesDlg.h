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

  QLineEdit*   m_edtStart;
  QLineEdit*   m_edtDelta;
  QLineEdit*   m_edtEnd;
  QLineEdit*   m_edtFileName;
  QComboBox*   m_cboUnits;
  QComboBox*   m_cboUnits2;
  QCheckBox*   m_chkOrbits;
  QPushButton* m_btnChoose;

  bool         m_bNewFile = false;
};

#endif

