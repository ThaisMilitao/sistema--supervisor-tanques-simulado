#include <iostream>
#include "suptanques-cliente.h"

using namespace std;

/// Construtor default
SupTanksClient::SupTanksClient():
  S(),is_admin(false), sock_client(), thr_client(), mtx_client()
{
  // Inicializa a biblioteca de sockets
  mysocket_status iResult = mysocket::init();
  if (iResult != mysocket_status::SOCK_OK)
  {
    std::cerr <<  "Biblioteca mysocket nao pode ser inicializada";
    exit(-1);
  }
}

/// Destrutor
SupTanksClient::~SupTanksClient()
{
  // Desconecta do servidor, caso esteja conectado
  disconnect();

  // Encerra a biblioteca de sockets
  mysocket::end();
}

/// Conecta ao servidor
/// Retorna false em caso de erro, true se OK
bool SupTanksClient::connect(const string& IP, const string& Login, const string& Senha)
{
  // Se jah estah conectado, nao faz nada
  if (isConnected()) return true;

  // Resultado das operacoes com socket
  mysocket_status iResult;
  // Comando recebido
  uint16_t cmd;

  try
  {
    // Conecta o socket
    iResult = sock_client.connect(IP, SUPTANKS_PORT);
    if (iResult != mysocket_status::SOCK_OK) throw 1;

    // Envia o cmd de conexao para o servidor, atraves do socket.
    // Nao precisa bloquear o mutex para garantir exclusao mutua
    // pq nesse momento ainda nao foi lancada a thread.
    // Entao, essa funcao eh a unica enviando dados pelo socket.
    iResult = sock_client.write_uint16(CMD_LOGIN);
    if (iResult != mysocket_status::SOCK_OK) throw 2;

    // Envia os parametros do comando (login, senha)
    iResult = sock_client.write_string(Login);
    if (iResult != mysocket_status::SOCK_OK) throw 3;
    iResult = sock_client.write_string(Senha);
    if (iResult != mysocket_status::SOCK_OK) throw 4;

    // Leh a resposta do servidor ao pedido de conexao
    iResult = sock_client.read_uint16(cmd,1000*SUPTANKS_TIMEOUT);
    if (iResult != mysocket_status::SOCK_OK) throw 5;
    if (cmd == CMD_ERROR) throw 6;
    is_admin = (cmd==CMD_ADMIN_OK);

    // Lanca a thread de solicitacao periodica de dados
    thr_client = thread( [this]()
    {
      this->thr_client_main();
    } );
    if (!thr_client.joinable()) throw 7;
  }
  catch (int i)
  {
    cerr << "Erro " << i << " na conexao com servidor\n";
    sock_client.close();
    if (thr_client.joinable()) thr_client.join();
    return false;
  }

  // Tudo OK!
  return true;
}

/// Desconecta do servidor
void SupTanksClient::disconnect()
{
  // Escreve o comando CMD_LOGOUT
  sock_client.write_uint16(CMD_LOGOUT);

  // Fecha o socket de comunicacao, caso esteja aberto
  // Deve encerrar a thread
  sock_client.close();

  // Limpa os dados armazenados
  S = SupState();
  // Torna nao administrador
  is_admin = false;

  // Espera encerramento da thread de solicitacao periodica de dados
  if (thr_client.joinable()) thr_client.join();
}

/// Requisita ao servidor o envio do estado atual da planta
bool SupTanksClient::requestState()
{
  if (!isConnected()) return false;

  // Resultado das operacoes com socket
  mysocket_status iResult;
  // Comando recebido
  uint16_t cmd;
  // Resultado da execucao da funcao
  bool resultado(true);

  // Bloqueia o mutex para garantir exclusao mutua no envio pelo socket
  // de comandos que ficam aguardando resposta, para evitar que a resposta
  // de um comando seja recebida por outro comando em outra thread.
  mtx_client.lock();

  try
  {
    // Escreve o comando CMD_GET_DATA
    iResult = sock_client.write_uint16(CMD_GET_DATA);
    if (iResult != mysocket_status::SOCK_OK) throw false;

    // Leh a resposta do servidor ao pedido de dados
    // Leh o comando CMD_DATA
    iResult = sock_client.read_uint16(cmd,1000*SUPTANKS_TIMEOUT);
    if (iResult != mysocket_status::SOCK_OK) throw false;
    if (cmd != CMD_DATA) throw false;
    // Leh os dados
    iResult = sock_client.read_bytes((mybyte*)&S, sizeof(SupState), 1000*SUPTANKS_TIMEOUT);
    if (iResult != mysocket_status::SOCK_OK) throw false;
  }
  catch(...)
  {
    resultado = false;
  }

  // Libera o mutex para sair da zona de exclusao mutua.
  mtx_client.unlock();

  // Resultado
  return resultado;
}

/// Impressao em console do ultimo estado da planta recebido do servidor
bool SupTanksClient::printState()
{
  if (!isConnected()) return false;

  S.print();
  return true;
}

/// Fixa o estado da valvula 1: aberta (true) ou fechada (false)
bool SupTanksClient::setV1Open(bool Open)
{
  if (!isConnected() || !isAdmin()) return false;

  // Resultado das operacoes com socket
  mysocket_status iResult;
  // Comando recebido
  uint16_t cmd;
  // Resultado da execucao da funcao
  bool resultado(true);

  // Bloqueia o mutex para garantir exclusao mutua no envio pelo socket
  // de comandos que ficam aguardando resposta, para evitar que a resposta
  // de um comando seja recebida por outro comando em outra thread.
  mtx_client.lock();

  try
  {
    // Escreve o comando CMD_SET_V1
    iResult = sock_client.write_uint16(CMD_SET_V1);
    if (iResult != mysocket_status::SOCK_OK) throw false;

    // Escreve o parametro do comando CMD_SET_V1
    iResult = sock_client.write_uint16(Open ? uint16_t(1) : uint16_t(0));
    if (iResult != mysocket_status::SOCK_OK) throw false;

    // Leh a resposta do servidor ao comando
    iResult = sock_client.read_uint16(cmd,1000*SUPTANKS_TIMEOUT);
    if (iResult != mysocket_status::SOCK_OK) throw false;
    if (cmd != CMD_OK) throw false;
  }
  catch(...)
  {
    resultado = false;
  }

  // Libera o mutex para sair da zona de exclusao mutua.
  mtx_client.unlock();

  // Resultado
  return resultado;
}

/// Fixa o estado da valvula 2: aberta (true) ou fechada (false)
bool SupTanksClient::setV2Open(bool Open)
{
  if (!isConnected() || !isAdmin()) return false;

  // Resultado das operacoes com socket
  mysocket_status iResult;
  // Comando recebido
  uint16_t cmd;
  // Resultado da execucao da funcao
  bool resultado(true);

  // Bloqueia o mutex para garantir exclusao mutua no envio pelo socket
  // de comandos que ficam aguardando resposta, para evitar que a resposta
  // de um comando seja recebida por outro comando em outra thread.
  mtx_client.lock();

  try
  {
    // Escreve o comando CMD_SET_V2
    iResult = sock_client.write_uint16(CMD_SET_V2);
    if (iResult != mysocket_status::SOCK_OK) throw false;

    // Escreve o parametro do comando CMD_SET_V2
    iResult = sock_client.write_uint16(Open ? uint16_t(1) : uint16_t(0));
    if (iResult != mysocket_status::SOCK_OK) throw false;

    // Leh a resposta do servidor ao comando
    iResult = sock_client.read_uint16(cmd,1000*SUPTANKS_TIMEOUT);
    if (iResult != mysocket_status::SOCK_OK) throw false;
    if (cmd != CMD_OK) throw false;
  }
  catch(...)
  {
    resultado = false;
  }

  // Libera o mutex para sair da zona de exclusao mutua.
  mtx_client.unlock();

  // Resultado
  return resultado;
}

/// Fixa a entrada da bomba: 0 a 65535
bool SupTanksClient::setPumpInput(uint16_t Input)
{
  if (!isConnected() || !isAdmin()) return false;

  // Resultado das operacoes com socket
  mysocket_status iResult;
  // Comando recebido
  uint16_t cmd;
  // Resultado da execucao da funcao
  bool resultado(true);

  // Bloqueia o mutex para garantir exclusao mutua no envio pelo socket
  // de comandos que ficam aguardando resposta, para evitar que a resposta
  // de um comando seja recebida por outro comando em outra thread.
  mtx_client.lock();

  try
  {
    // Escreve o comando CMD_SET_PUMP
    iResult = sock_client.write_uint16(CMD_SET_PUMP);
    if (iResult != mysocket_status::SOCK_OK) throw false;

    // Escreve o paramentro do comando CMD_SET_PUMP
    iResult = sock_client.write_uint16(Input);
    if (iResult != mysocket_status::SOCK_OK) throw false;

    // Leh a resposta do servidor ao comando
    iResult = sock_client.read_uint16(cmd,1000*SUPTANKS_TIMEOUT);
    if (iResult != mysocket_status::SOCK_OK) throw false;
    if (cmd != CMD_OK) throw false;
  }
  catch(...)
  {
    resultado = false;
  }

  // Libera o mutex para sair da zona de exclusao mutua.
  mtx_client.unlock();

  // Resultado
  return resultado;
}

/// Thread de solicitacao periodica de dados
void SupTanksClient::thr_client_main(void)
{
  while (isConnected())
  {
    requestState();
    printState();
    // Espera SUPTANKS_CLIENT_REFRESH_PERIOD segundos
    this_thread::sleep_for(chrono::seconds(SUPTANKS_CLIENT_REFRESH_PERIOD));
  }
}

