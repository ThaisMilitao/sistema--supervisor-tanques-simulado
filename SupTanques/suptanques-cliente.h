#ifndef _SUP_TANQUES_CLIENT_H_
#define _SUP_TANQUES_CLIENT_H_

#include <string>
#include <thread>
#include <mutex>
#include "suptanques.h"
#include "../MySocket/mysocket.h"

/// Periodo (em segundos) para solicitar novo estado da planta
/// (atualizacao periodica do cliente)
#define SUPTANKS_CLIENT_REFRESH_PERIOD 30

class SupTanksClient
{
public:
  // Construtor default
  SupTanksClient();
  // Destrutor
  ~SupTanksClient();

  // Funcoes de consulta
  // Cliente conectado (true) ou desconectado (false)
  bool isConnected() const {return sock_client.connected();}
  // Cliente administrador (true) ou visualizador (false)
  bool isAdmin() const {return is_admin;}

  // Conecta ao servidor
  // Retorna false em caso de erro, true se OK
  bool connect(const std::string& IP, const std::string& Login, const std::string& Senha);

  // Desconecta do servidor
  void disconnect();

  // Requisita ao servidor o envio do estado atual da planta
  bool requestState();

  // Impressao em console do ultimo estado da planta recebido do servidor
  bool printState();

  // Funcoes de atuacao
  bool setV1Open(bool Open);         // Fixa o estado da valvula 1: aberta (true) ou fechada (false)
  bool setV2Open(bool Open);         // Fixa o estado da valvula 2: aberta (true) ou fechada (false)
  bool setPumpInput(uint16_t Input); // Fixa a entrada da bomba: 0 a 65535

private:
  // Construtores e operadores de atribuicao suprimidos (nao existem na classe)
  SupTanksClient(const SupTanksClient& other) = delete;
  SupTanksClient(SupTanksClient&& other) = delete;
  SupTanksClient& operator=(const SupTanksClient& other) = delete;
  SupTanksClient& operator=(SupTanksClient&& other) = delete;

  // Estado da planta
  SupState S;

  // Cliente eh administrador
  bool is_admin;

  // Socket de comunicacaco
  tcp_mysocket sock_client;

  // Identificador da thread de solicitacao periodica de dados
  std::thread thr_client;

  // Exclusao mutua para nao enviar novo comando antes de
  // receber a resposta do comando anterior
  std::mutex mtx_client;

  // Thread de solicitacao periodica de dados
  void thr_client_main(void);
};

#endif // _SUP_TANQUES_CLIENT_H_
