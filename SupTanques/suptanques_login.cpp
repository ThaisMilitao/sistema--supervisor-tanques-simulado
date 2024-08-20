#include "suptanques_login.h"
#include "ui_suptanques_login.h"

SupTanquesLogin::SupTanquesLogin(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SupTanquesLogin)
{
  ui->setupUi(this);
}

SupTanquesLogin::~SupTanquesLogin()
{
  delete ui;
}

void SupTanquesLogin::clear()
{
  ui->lineEditServer->setText("127.0.0.1");
  ui->lineEditLogin->clear();
  ui->lineEditPassword->clear();
}

void SupTanquesLogin::on_buttonBox_accepted()
{
  emit signLogin(ui->lineEditServer->text(), ui->lineEditLogin->text(), ui->lineEditPassword->text());
}

