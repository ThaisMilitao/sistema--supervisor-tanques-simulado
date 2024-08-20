#ifndef _SUP_TANQUES_SERVER_H_
#define _SUP_TANQUES_SERVER_H_

#include <string>
#include <list>
#include "tanques.h"
#include "suptanques.h"
#include "../MySocket/mysocket.h" 

/// A classe que implementa o servidor do sistema de tanques
class SupTanksServer: public Tanks
{
private:
  // Subclasse privada para representar os usuarios cadastrados no servidor
  struct User
  {
    // Dados
    std::string login;    // Nome de login
    std::string password; // Senha
    bool isAdmin;         // Pode alterar (true) ou soh consultar (false) o sistema
    tcp_mysocket sock;    // Socket de comunicacao
    // Construtores
    User(const std::string& Login, const std::string& Senha, bool Admin):
      login(Login),password(Senha),isAdmin(Admin),sock() {}
    // Comparacao com string
    bool operator==(const std::string& S) const {return login==S;}
    // Usuario estah conectado ou nao?
    inline bool isConnected() const {return sock.connected();}
    // Desconecta usuario
    inline void close() { sock.close();}
  };

public:
  // Construtor default
  SupTanksServer();
  // Destrutor
  ~SupTanksServer();

  // Funcoes de consulta
  // Servidor ligado (true) ou desligado (false)
  bool serverOn() const {return server_on;}

  // Funcoes de atuacao
  bool setServerOn();                // Liga o servidor: retorna true se OK
  void setServerOff();               // Desliga o servidor

  // Leitura e impressao em console do estado da planta
  void readPrintState() const;
  // Impressao em console dos usuarios do servidor
  void printUsers() const;

  // Adicionar um novo usuario
  bool addUser(const std::string& Login, const std::string& Senha, bool Admin);
  // Remover um usuario
  bool removeUser(const std::string& Login);

private:
  // Construtores e operadores de atribuicao suprimidos (nao existem na classe)
  SupTanksServer(const SupTanksServer& other) = delete;
  SupTanksServer(SupTanksServer&& other) = delete;
  SupTanksServer& operator=(const SupTanksServer& other) = delete;
  SupTanksServer& operator=(SupTanksServer&& other) = delete;

  // Estado do servidor como um todo (ligado/desligado)
  bool server_on;

  // Lista de usuarios do servidor
  std::list<User> LU;
  // Identificador da thread do servidor
  std::thread thr_server;
  // Socket de conexoes
  tcp_mysocket_server sock_server;  

  // Leitura do estado dos tanques a partir dos sensores
  void readStateFromSensors(SupState& S) const;

  // A funcao que implementa a thread do servidor
  // Leitura e envio de dados pelos sockets
  void thr_server_main(void);
};

#endif // _SUP_TANQUES_SERVER_H_
