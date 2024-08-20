#include <QMessageBox>
#include <iostream>       // cerr
#include <cmath>          // pow, round
#include <thread>         // this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include "suptanques_main.h"
#include "ui_suptanques_main.h"

#include "tanques-param.h"

// Arredonda um numero para N casas decimais apos o ponto
static double roundN(double val, int N)
{
  double potencia = std::pow(10.0, N);
  return std::round(potencia*val)/potencia;
}

SupTanquesMain::SupTanquesMain(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::SupTanquesMain)
  , loginWindow(new SupTanquesLogin(this))
  , statusMsg(new QLabel(this))
  , image(new SupTanquesImg(this))
  , connected(false)
  , admin(false)
  , timeRefresh(30)
  , start_t(-1)
  , sock()
  , mtx()
  , thr()
{
  ui->setupUi(this);

  // A imagem
  ui->horizontalLayout->insertWidget(0,image);

  // Os titulos das secoes do supervisorio
  ui->labelActuators->setStyleSheet("background-color: white");
  ui->labelSensors->setStyleSheet("background-color: white");
  ui->labelVisualization->setStyleSheet("background-color: white");

  // Os botoes push botton das valvulas

  // Sao checaveis (alternam on/off)
  ui->buttonV1->setCheckable(true);
  ui->buttonV2->setCheckable(true);
  // Valvula fechada (nao checado) - cinza esverdeado, valvula aberta (checado) - cinza avermelhado
  ui->buttonV1->setStyleSheet("QPushButton {background-color: #F0FFF0;"
                              "border-style: outset;"
                              "border-width: 2px;"
                              "padding: 3px}"
                              "QPushButton:checked { background-color: #FFF0F0;"
                              "border-style: inset;"
                              "border-width: 2px;"
                              "padding: 3px}");
  ui->buttonV2->setStyleSheet("QPushButton {background-color: #F0FFF0;"
                              "border-style: outset;"
                              "border-width: 2px;"
                              "padding: 3px}"
                              "QPushButton:checked { background-color: #FFF0F0;"
                              "border-style: inset;"
                              "border-width: 2px;"
                              "padding: 3px}");

  // O slider da bomba

  ui->sliderPump->setMinimum(0);
  ui->sliderPump->setMaximum(65535);
  ui->sliderPump->setPageStep(1000);
  // Soh gera um unico sinal valueChanged ao final do movimento do slider
  ui->sliderPump->setTracking(false);

  // Os displays LCD

  // Cor de fundo branco
  qApp->setStyleSheet("QLCDNumber { background-color: white }");
  // Paleta de cores para todos os displays (exceto overflow)
  auto paleta = ui->lcdH1Cm->palette();
  paleta.setColor(paleta.WindowText, Qt::red);
  // Numeros nao ressaltados (flat), 5 digitos, ponto ocupa um digito, paleta letra vermelha
  // H1
  ui->lcdH1Cm->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdH1Cm->setDigitCount(5);
  ui->lcdH1Cm->setSmallDecimalPoint(false);
  ui->lcdH1Cm->setPalette(paleta);
  ui->lcdH1Perc->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdH1Perc->setDigitCount(5);
  ui->lcdH1Perc->setSmallDecimalPoint(false);
  ui->lcdH1Perc->setPalette(paleta);
  // H2
  ui->lcdH2Cm->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdH2Cm->setDigitCount(5);
  ui->lcdH2Cm->setSmallDecimalPoint(false);
  ui->lcdH2Cm->setPalette(paleta);
  ui->lcdH2Perc->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdH2Perc->setDigitCount(5);
  ui->lcdH2Perc->setSmallDecimalPoint(false);
  ui->lcdH2Perc->setPalette(paleta);
  // Bomba
  ui->lcdPumpVal->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdPumpVal->setDigitCount(5);
  ui->lcdPumpVal->setSmallDecimalPoint(false);
  ui->lcdPumpVal->setPalette(paleta);
  ui->lcdPumpPerc->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdPumpPerc->setDigitCount(5);
  ui->lcdPumpPerc->setSmallDecimalPoint(false);
  ui->lcdPumpPerc->setPalette(paleta);
  // Vazao
  ui->lcdFlowLMin->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdFlowLMin->setDigitCount(5);
  ui->lcdFlowLMin->setSmallDecimalPoint(false);
  ui->lcdFlowLMin->setPalette(paleta);
  ui->lcdFlowPerc->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdFlowPerc->setDigitCount(5);
  ui->lcdFlowPerc->setSmallDecimalPoint(false);
  ui->lcdFlowPerc->setPalette(paleta);
  // overflow
  // Fundo vermelho, numeros nao ressaltados (flat), 5 digitos, paleta letra branca,
  // conteudo texto (nao numeros), widget normalmente oculto
  ui->lcdOverflow->setStyleSheet("background-color: red");
  ui->lcdOverflow->setSegmentStyle(QLCDNumber::Flat);
  ui->lcdOverflow->setDigitCount(5);
  paleta.setColor(paleta.WindowText, Qt::white);
  ui->lcdOverflow->setPalette(paleta);
  ui->lcdOverflow->display("O-FLO");

  // A barra de status
  statusBar()->addWidget(statusMsg);

  // O icone da aplicacao
  QPixmap pixIcon;
  if (pixIcon.load("scada_icon.png","PNG"))
  {
    setWindowIcon(QIcon(pixIcon));
  }
  else if (pixIcon.load("..\\SupTanques\\scada_icon.png","PNG"))
  {
    setWindowIcon(QIcon(pixIcon));
  }

  // Para poder utilizar SupState como parametro de sinais e slots entre threads diferentes
  qRegisterMetaType<SupState>();

  // Conexoes sinais - slots
  connect(loginWindow, &SupTanquesLogin::signLogin,
          this, &SupTanquesMain::slotLogin);
  connect(this, &SupTanquesMain::signDisconnect,
          this, &SupTanquesMain::slotDisconnect);
  connect(this, &SupTanquesMain::signShowData,
          this, &SupTanquesMain::slotShowData);

  // Inicializa a biblioteca de sockets
  if (mysocket::init() != mysocket_status::SOCK_OK)
  {
      std::cerr << "Biblioteca mysocket nao pode ser inicializada\n";
      exit(-1);
  }

  // Coloca o cliente no estado desconectado
  slotDisconnect();
}

SupTanquesMain::~SupTanquesMain()
{
  delete ui;

  // Encerra a biblioteca de sockets
  mysocket::end();
}

void SupTanquesMain::on_actionLogin_triggered()
{
  loginWindow->clear();
  loginWindow->show();
}

void SupTanquesMain::on_actionLogout_triggered()
{
  slotDisconnect();
}

void SupTanquesMain::on_actionQuit_triggered()
{
  // Envia CMD_LOGOUT caso esteja conectado
    // Testa se estah conectado
    if (sock.connected())
    {
        // Envia o comando de logout para o servidor
        sock.write_int16(CMD_LOGOUT);
        // Fecha o socket
        sock.close();
    }
  QCoreApplication::quit();
}

void SupTanquesMain::on_buttonV1_clicked(bool open)
{
  setValve(open, true);
}

void SupTanquesMain::on_buttonV2_clicked(bool open)
{
  setValve(open, false);
}

void SupTanquesMain::on_sliderPump_valueChanged(int value)
{
  // Se nao estiver conectado, nao faz nada
  if (!connected) return;

  // Entra na zona de exclusao mutua, para escrever comando no socket
  // e depois ler do socket resposta ao comando
  mtx.lock();
  uint16_t cmd;
  try
  {
    if (!admin) throw 1;
    // Envia comando
    if (sock.write_uint16(CMD_SET_PUMP) != mysocket_status::SOCK_OK) throw 2;
    if (sock.write_uint16(value) != mysocket_status::SOCK_OK) throw 3;

    // Aguarda resposta do servidor
    if (sock.read_uint16(cmd,1000*SUPTANKS_TIMEOUT) != mysocket_status::SOCK_OK) throw 4;
    if (cmd != CMD_OK) throw 5;

    // Sai da zona de exclusao mutua
    mtx.unlock();

    // Exibe valores nos displays LCD
    ui->lcdPumpVal->display(value);
    double PumpPerc = roundN( (100.0*value)/UINT16_MAX, 2 );
    ui->lcdPumpPerc->display(PumpPerc);
  }
  catch (int erro)
  {
    // Sai da zona de exclusao mutua
    mtx.unlock();

    // Se houve erro no socket, disconecta
   if (erro>=2 && erro<=4) emit signDisconnect();

    // Msg de erro
    QString msg;
    switch(erro)
    {
    case 1:
      msg = "Not admin user";
      break;
    case 2:
    case 3:
      msg = "Error sending command to the server";
      break;
    case 4:
      msg = "Error receiving reply from the server!";
      break;
    case 5:
      msg = "Command rejected by the server";
      break;
    default:
      msg = "Unknown error";
      break;
    }
    QMessageBox::warning(this, "Set pump input", msg);
  }
}

void SupTanquesMain::on_showLevel_toggled(bool checked)
{
  image->setDisplayMode(checked);
}

void SupTanquesMain::on_spinRefresh_valueChanged(int arg1)
{
  timeRefresh = arg1;
}

/// Conectar ao servidor
void SupTanquesMain::slotLogin(QString Server, QString Login, QString Passwd)
{
  // Se estiver conectado, nao faz nada
  if (connected) return;

  // Nao precisa entrar na zona de exclusao mutua para escrever no socket,
  // pois a outra thread nao estah lancada.
  uint16_t cmd;
  try
  {
    // Conecta com servidor
    if (sock.connect(Server.toStdString(), SUPTANKS_PORT) != mysocket_status::SOCK_OK) throw 1;

    // Envia comando de login, nome do usuario e senha
    if (sock.write_uint16(CMD_LOGIN) != mysocket_status::SOCK_OK) throw 2;
    if (sock.write_string(Login.toStdString()) != mysocket_status::SOCK_OK) throw 3;
    if (sock.write_string(Passwd.toStdString()) != mysocket_status::SOCK_OK) throw 4;

    // Aguarda resposta do servidor
    if (sock.read_uint16(cmd,1000*SUPTANKS_TIMEOUT) != mysocket_status::SOCK_OK) throw 5;

    // Soh chega aqui se nao houve nenhum erro (throw)
    // Estah conectado
    connected = true;
    // Eh administrador (de acordo com resposta do servidor)?
    admin = (cmd==CMD_ADMIN_OK);

    // Lanca thread
    thr = std::thread( [this]()
    {
        this->main_thread();
    } );
    if (!thr.joinable()) throw 6;


    // Desabilita opcao conectar
    ui->actionLogin->setEnabled(false);
    // Habilita opcao desconectar
    ui->actionLogout->setEnabled(true);

    // Habilita atuadores na planta, se for administrador
    if (admin)
    {
      // Os botoes push botton
      ui->buttonV1->setEnabled(true);
      ui->buttonV2->setEnabled(true);
      // O slider da bomba
      ui->sliderPump->setEnabled(true);
    }

    // Habilita os botoes para escolher o modo de visualizacao
    ui->showLevel->setEnabled(true);
    ui->showGraph->setEnabled(true);

    // Habilita o spin para escolher o periodo de visualizacao
    ui->spinRefresh->setEnabled(true);

    // Barra de status
    QString msg = " CONNECTED: " + Login + " (" + (admin ? "admin" : "viewer") + ")";
    statusMsg->setText(msg);

    // Exibe a imagem
    image->drawImg();

    // Fixa que ainda nao foi enviado o primeiro ponto para visualizacao
    // Jah deve estar com esse valor (-1), jah que ele eh fixado no construtor
    start_t = time_t(-1);
  }
  catch(int erro)
  {
    // Fixa que nao estah conectado e nao eh administrador
    connected = admin = false;
    // Fecha o socket
    sock.close();

    // Aguarda fim da thread, embora nunca deva estar lancada
    if (thr.joinable()) thr.join();

    // Msg de erro
    QString msg;
    switch(erro)
    {
    case 1:
      msg = "Socket connection failed";
      break;
    case 2:
    case 3:
    case 4:
      msg = "Error sending login command to the server";
      break;
    case 5:
      msg = "Error receiving reply from the server!";
      break;
    case 6:
      msg = "Login rejected by the server";
      break;
    default:
      msg = "Unknown error";
      break;
    }
    QMessageBox::critical(this, "Login error", msg);
  }

}

/// Coloca o cliente no estado desconectado do servidor (interface, socket, thread, etc.)
void SupTanquesMain::slotDisconnect()
{
  // Envia CMD_LOGOUT e fecha o socket caso esteja conectado
  if (sock.write_uint16(CMD_LOGOUT) == mysocket_status::SOCK_OK) sock.close();

  // Aguarda fim da thread
  if (thr.joinable()) thr.join();

  // Fixa que nao estah conectado e nao eh administrador
  // Deve parar a thread
  connected = admin = false;

  // Faz as acoes a seguir mesmo que nao esteja conectado,
  // pois esta funcao eh chamada no construtor da janela principal
  // para colocar a interface no modo desconectado.

  // Limpa o instante em que iniciou a coleta de dados
  start_t = time_t(-1);

  // Limpa a imagem
  image->clear();

  // Habilita opcao conectar
  ui->actionLogin->setEnabled(true);
  // Desabilita opcao desconectar
  ui->actionLogout->setEnabled(false);

  // Os botoes push botton
  showValves(0,0);
  ui->buttonV1->setEnabled(false);
  ui->buttonV2->setEnabled(false);

  // O slider da bomba
  showPump(0);
  ui->sliderPump->setEnabled(false);

  // Zera os demais mostradores LCD
  showH(0,0,0);
  showFlow(0);

  // Mostrador de overflow
  ui->lcdOverflow->hide();

  // A barra de status
  statusMsg->setText(" DISCONNECTED");

  // O modo de visualizacao
  ui->showLevel->setChecked(true);
  ui->showLevel->setEnabled(false);
  ui->showGraph->setEnabled(false);

  // O periodo de visualizacao
  ui->spinRefresh->setEnabled(false);
}

void SupTanquesMain::slotShowData(SupState S)
{
  // Instante de tempo atual
  time_t current_t = std::time(nullptr);

  // Inicializa o instante inicial, se for o primeiro ponto
  if (start_t < 0) start_t = current_t;

  // Acrescenta o ponto no grafico
  image->addPoint(current_t-start_t, S);

  // Exibe o ponto nos visualizadores
  showValves(S.V1,S.V2);
  showH(S.H1, S.H2, S.ovfl);
  showPump(S.PumpInput);
  showFlow(S.PumpFlow);
}

void SupTanquesMain::showValves(uint16_t V1, uint16_t V2)
{
  // Altera os push buttom das valvulas.
  // Como vai alterar o estado do pushButtom com setChecked, precisa anter bloquear a geracao de sinais.
  // Senao, vai executar on_buttonV?_clicked, chamar setValve e enviar um comando para o servidor.
  //
  // V1
  ui->buttonV1->blockSignals(true);
  if (V1 != 0)
  {
    // Valvula aberta
    ui->buttonV1->setChecked(true);
    ui->buttonV1->setText("OPEN");
  }
  else
  {
    // Valvula fechada
    ui->buttonV1->setChecked(false);
    ui->buttonV1->setText("CLOSED");
  }
  ui->buttonV1->blockSignals(false);
  //
  // V2
  ui->buttonV2->blockSignals(true);
  if (V2 != 0)
  {
    // Valvula aberta
    ui->buttonV2->setChecked(true);
    ui->buttonV2->setText("OPEN");
  }
  else
  {
    // Valvula fechada
    ui->buttonV2->setChecked(false);
    ui->buttonV2->setText("CLOSED");
  }
  ui->buttonV2->blockSignals(false);
}

void SupTanquesMain::showH(uint16_t H1, uint16_t H2, uint16_t OverFlow)
{
  // Exibe valores nos displays LCD
  // H1
  double H1Perc = roundN( (100.0*H1)/UINT16_MAX, 2 );
  ui->lcdH1Perc->display(H1Perc);
  double H1Cm = roundN( MaxTankLevelMeasurement*H1Perc, 2 );
  ui->lcdH1Cm->display(H1Cm);
  // H2
  double H2Perc = roundN( (100.0*H2)/UINT16_MAX, 2 );
  ui->lcdH2Perc->display(H2Perc);
  double H2Cm = roundN( MaxTankLevelMeasurement*H2Perc, 2 );
  ui->lcdH2Cm->display(H2Cm);
  // Overflow
  if (OverFlow != 0) ui->lcdOverflow->show(); // Estah transbordando
  else ui->lcdOverflow->hide();               // Nao estah transbordando
}

void SupTanquesMain::showPump(uint16_t PInput)
{
  // Altera o valor do slider da entrada da bomba.
  // Como vai alterar o estado do slider com setValue, precisa anter bloquear a geracao de sinais.
  // Senao, vai executar on_sliderPump_valueChanged e enviar um comando para o servidor.
  ui->sliderPump->blockSignals(true);
  ui->sliderPump->setValue(PInput);
  ui->sliderPump->blockSignals(false);

  // Exibe valores nos displays LCD
  ui->lcdPumpVal->display(PInput);
  double PumpPerc = roundN( (100.0*PInput)/UINT16_MAX, 2 );
  ui->lcdPumpPerc->display(PumpPerc);
}

void SupTanquesMain::showFlow(uint16_t Flow)
{
  double FlowPerc = roundN( (100.0*Flow)/UINT16_MAX, 2 );
  ui->lcdFlowPerc->display(FlowPerc);
  double FlowLMin = roundN( 600.0*MaxPumpFlowMeasurement*FlowPerc, 3 );
  ui->lcdFlowLMin->display(FlowLMin);
}

void SupTanquesMain::main_thread()
{
    uint16_t cmd;
    SupState S;
    try
    {
        while(connected)
        {
            // Entra na zona de exclusao mutua, para escrever comando no socket
            // e depois ler do socket resposta ao comando
            mtx.lock();

            // Envia comando CMD_GET_DATA
            if ((sock.write_uint16(CMD_GET_DATA)) != mysocket_status::SOCK_OK) throw 1;
            // Aguarda resposta
            if ((sock.read_uint16(cmd,1000*SUPTANKS_TIMEOUT)) != mysocket_status::SOCK_OK) throw 2;
            if (cmd != CMD_DATA) throw 3;
            // Leh o parametro do comando CMD_DATA
            if ((sock.read_bytes((mybyte*)&S, sizeof(SupState), 1000*SUPTANKS_TIMEOUT)) != mysocket_status::SOCK_OK) throw 3;
            // Sai da zona de exclusao mutua
            mtx.unlock();

            // Envia dados para exibicao na interface
            emit signShowData(S);

            // Espera para solicitar novos dados
            std::this_thread::sleep_for(std::chrono::seconds(timeRefresh));
        }
    }
    catch(int erro)
    {
        // Sai da zona de exclusao mutua
        mtx.unlock();

        // Soh emite sinal para desconectar se saiu por erro detectado na propria thread.
        // Se saiu pq a condicao do while nao eh mais verdadeira, foi a thread principal
        // que gerou o encerramento, e jah deve ter chamado o slotDisconnect
        if (erro>=1 && erro<=3) emit signDisconnect();
    }
}

/// Abrir/fechar valvula
void SupTanquesMain::setValve(bool open, bool V1)
{
    // Se nao estiver conectado, nao faz nada
    if (!connected) return;

    // Botao de qual valvula (V1 ou V2)?
    auto but = ui->buttonV1;
    if (!V1) but = ui->buttonV2;

    // Entra na zona de exclusao mutua, para escrever comando no socket
    // e depois ler do socket resposta ao comando
    mtx.lock();

    uint16_t cmd;

    try
    {
        if (!admin) throw 1;
        // Envia comando CMD_SET_V1 ou CMD_SET_V2
        if ((sock.write_uint16(V1 ? CMD_SET_V1 : CMD_SET_V2)) != mysocket_status::SOCK_OK) throw 2;
        if ((sock.write_uint16(open ? uint16_t(1) : uint16_t(0))) != mysocket_status::SOCK_OK) throw 3;

        // Aguarda resposta
        if ((sock.read_uint16(cmd,1000*SUPTANKS_TIMEOUT)) != mysocket_status::SOCK_OK) throw 4;
        if (cmd != CMD_OK) throw 5;

        // Sai da zona de exclusao mutua
        mtx.unlock();

        // Altera texto do botao
        if (open) but->setText("OPEN");
        else but->setText("CLOSED");
    }
    catch (int erro)
    {
        // Sai da zona de exclusao mutua
        mtx.unlock();

        // Retorna o botao ao estado anterior.
        // Nao gera sinal e, consequentemente, nao chama on_buttonV?_clicked
        but->toggle();

        // Se houve erro no socket, disconecta
        if (erro>=2 && erro<=4) emit signDisconnect();

        // Msg de erro
        QString msg;
        switch(erro)
        {
        case 1:
            msg = "Not admin user";
            break;
        case 2:
        case 3:
            msg = "Error sending command to the server";
            break;
        case 4:
            msg = "Error receiving reply from the server!";
            break;
        case 5:
            msg = "Command rejected by the server";
            break;
        default:
            msg = "Unknown error";
            break;
        }
        QMessageBox::warning(this, "Set valve error", msg);
    }
}
