#ifndef SUPTANQUES_LOGIN_H
#define SUPTANQUES_LOGIN_H

#include <QDialog>
#include <QString>

namespace Ui {
class SupTanquesLogin;
}

class SupTanquesLogin : public QDialog
{
  Q_OBJECT

public:
  explicit SupTanquesLogin(QWidget *parent = nullptr);
  ~SupTanquesLogin();

  void clear();

signals:
  void signLogin(QString, QString, QString);

private slots:
  void on_buttonBox_accepted();

private:
  Ui::SupTanquesLogin *ui;
};

#endif // SUPTANQUES_LOGIN_H
