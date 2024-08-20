#ifndef SUPTANQUESMAIN_H
#define SUPTANQUESMAIN_H

#include <QMainWindow>
#include "../MySocket/mysocket.h"
#include <thread>
#include <mutex>
#include "suptanques.h"
#include "suptanques_login.h"
#include "suptanques_img.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SupTanquesMain; }
QT_END_NAMESPACE

// Para poder utilizar SupState como parametro de sinais e slots
Q_DECLARE_METATYPE(SupState)

class SupTanquesMain : public QMainWindow
{
  Q_OBJECT

public:
  SupTanquesMain(QWidget *parent = nullptr);
  ~SupTanquesMain();

signals:
  void signDisconnect();

  void signShowData(SupState S);

private slots:
  void on_actionLogin_triggered();

  void on_actionLogout_triggered();

  void on_actionQuit_triggered();

  void on_buttonV1_clicked(bool open);

  void on_buttonV2_clicked(bool open);

  void on_sliderPump_valueChanged(int value);

  void on_showLevel_toggled(bool checked);

  void on_spinRefresh_valueChanged(int arg1);

  // Conectar ao servidor
  void slotLogin(QString Server, QString Login, QString Passwd);

  // Coloca o cliente no estado desconectado do servidor (interface, socket, thread, etc.)
  void slotDisconnect();

  // Exibir dados provenientes do servidor
  void slotShowData(SupState S);

private:
  Ui::SupTanquesMain *ui;

  // A janela de login
  SupTanquesLogin *loginWindow;

  // O widget da barra de status
  QLabel *statusMsg;

  // A imagem para exibicao do grafico
  SupTanquesImg* image;

  // Estah conectado ao servidor?
  bool connected;
  // Eh administrador?
  bool admin;
  // Periodo de solicitacao de novos dados
  int timeRefresh;
  // Instante de tempo quando iniciou a recepcao de dados do servidor
  std::time_t start_t;

  // O socket de comunicacao com o servidor
  tcp_mysocket sock;
  // Semaforo para exclusao mutua para escrita no socket
  std::mutex mtx;
  // A thread que solicita dados periodicamente
  std::thread thr;

  // Exibir os niveis
  void showValves(uint16_t V1, uint16_t V2);
  void showH(uint16_t H1, uint16_t H2, uint16_t OverFlow);
  void showPump(uint16_t PInput);
  void showFlow(uint16_t Flow);

  // Thread que solicita dados periodicamente
  void main_thread();
  // Abrir/fechar valvula
  void setValve(bool open, bool V1);
};
#endif // SUPTANQUESMAIN_H
