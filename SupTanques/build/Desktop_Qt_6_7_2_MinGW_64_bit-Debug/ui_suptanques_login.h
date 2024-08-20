/********************************************************************************
** Form generated from reading UI file 'suptanques_login.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SUPTANQUES_LOGIN_H
#define UI_SUPTANQUES_LOGIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_SupTanquesLogin
{
public:
    QDialogButtonBox *buttonBox;
    QLabel *labelLogin;
    QLineEdit *lineEditLogin;
    QLabel *labelPassword;
    QLineEdit *lineEditPassword;
    QLabel *labelServer;
    QLineEdit *lineEditServer;

    void setupUi(QDialog *SupTanquesLogin)
    {
        if (SupTanquesLogin->objectName().isEmpty())
            SupTanquesLogin->setObjectName("SupTanquesLogin");
        SupTanquesLogin->resize(240, 200);
        buttonBox = new QDialogButtonBox(SupTanquesLogin);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setGeometry(QRect(20, 150, 200, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        buttonBox->setCenterButtons(true);
        labelLogin = new QLabel(SupTanquesLogin);
        labelLogin->setObjectName("labelLogin");
        labelLogin->setGeometry(QRect(20, 60, 60, 16));
        lineEditLogin = new QLineEdit(SupTanquesLogin);
        lineEditLogin->setObjectName("lineEditLogin");
        lineEditLogin->setGeometry(QRect(80, 60, 140, 22));
        labelPassword = new QLabel(SupTanquesLogin);
        labelPassword->setObjectName("labelPassword");
        labelPassword->setGeometry(QRect(20, 100, 60, 16));
        lineEditPassword = new QLineEdit(SupTanquesLogin);
        lineEditPassword->setObjectName("lineEditPassword");
        lineEditPassword->setGeometry(QRect(80, 100, 140, 22));
        lineEditPassword->setEchoMode(QLineEdit::Password);
        labelServer = new QLabel(SupTanquesLogin);
        labelServer->setObjectName("labelServer");
        labelServer->setGeometry(QRect(20, 20, 60, 16));
        lineEditServer = new QLineEdit(SupTanquesLogin);
        lineEditServer->setObjectName("lineEditServer");
        lineEditServer->setGeometry(QRect(80, 20, 140, 22));

        retranslateUi(SupTanquesLogin);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, SupTanquesLogin, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, SupTanquesLogin, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(SupTanquesLogin);
    } // setupUi

    void retranslateUi(QDialog *SupTanquesLogin)
    {
        SupTanquesLogin->setWindowTitle(QCoreApplication::translate("SupTanquesLogin", "Dialog", nullptr));
        labelLogin->setText(QCoreApplication::translate("SupTanquesLogin", "Login:", nullptr));
        labelPassword->setText(QCoreApplication::translate("SupTanquesLogin", "Password:", nullptr));
        labelServer->setText(QCoreApplication::translate("SupTanquesLogin", "Server (IP):", nullptr));
        lineEditServer->setText(QCoreApplication::translate("SupTanquesLogin", "127.0.0.1", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SupTanquesLogin: public Ui_SupTanquesLogin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SUPTANQUES_LOGIN_H
