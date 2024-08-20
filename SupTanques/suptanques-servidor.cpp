#include <iostream>     /* cerr */
#include <algorithm>
#include "suptanques-servidor.h"

using namespace std;

/* ========================================
   CLASSE SUPTANKSSERVER
   ======================================== */

/// Construtor
SupTanksServer::SupTanksServer():
  Tanks(),
  server_on(false),
  LU(),
  thr_server(),
  sock_server()
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
SupTanksServer::~SupTanksServer()
{
  // Deve parar a thread do servidor
  server_on = false;
  // Fecha todos os sockets dos clientes
  for (auto& U : LU) U.close();
  // Fecha o socket de conexoes
  sock_server.close(); 
  // Encerra a biblioteca de sockets
  mysocket::end(); 

  // Espera o fim da thread do servidor
  if (thr_server.joinable()) thr_server.join();
}

/// Liga o servidor
bool SupTanksServer::setServerOn()
{
  // Se jah estah ligado, nao faz nada
  if (server_on) return true;

  // Liga os tanques
  setTanksOn();

  // Indica que o servidor estah ligado a partir de agora
  server_on = true;

  try
  {
    // Coloca o socket de conexoes em escuta
    if(sock_server.listen(SUPTANKS_PORT) != mysocket_status::SOCK_OK)
      throw 1;

    // Lanca a thread do servidor que comunica com os clientes
    thr_server = thread([this](){this->thr_server_main();} );
    if (!thr_server.joinable()) throw 2; 

  }
  catch(int i)
  {
    cerr << "Erro " << i << " ao iniciar o servidor\n";

    // Deve parar a thread do servidor
    server_on = false;

    // Fecha o socket do servidor
    sock_server.close();

    return false;
  }

  // Tudo OK
  return true;
}

/// Desliga o servidor
void SupTanksServer::setServerOff()
{
  // Se jah estah desligado, nao faz nada
  if (!server_on) return;

  // Deve parar a thread do servidor
  server_on = false;

  // Fecha todos os sockets dos clientes
  for (auto& U : LU) U.close();
  // Fecha o socket de conexoes
  sock_server.close();

  // Espera pelo fim da thread do servidor
  if (thr_server.joinable()) thr_server.join();
  thr_server = thread();

  // Desliga os tanques
  setTanksOff();
}

/// Leitura do estado dos tanques
void SupTanksServer::readStateFromSensors(SupState& S) const
{
  // Estados das valvulas: OPEN, CLOSED
  S.V1 = v1isOpen();
  S.V2 = v2isOpen();
  // Niveis dos tanques: 0 a 65535
  S.H1 = hTank1();
  S.H2 = hTank2();
  // Entrada da bomba: 0 a 65535
  S.PumpInput = pumpInput();
  // Vazao da bomba: 0 a 65535
  S.PumpFlow = pumpFlow();
  // Estah transbordando (true) ou nao (false)
  S.ovfl = isOverflowing();
}

/// Leitura e impressao em console do estado da planta
void SupTanksServer::readPrintState() const
{
  if (tanksOn())
  {
    SupState S;
    readStateFromSensors(S);
    S.print();
  }
  else
  {
    cout << "Tanques estao desligados!\n";
  }
}

/// Impressao em console dos usuarios do servidor
void SupTanksServer::printUsers() const
{
  for (const auto& U : LU)
  {
    cout << U.login << '\t'
         << "Admin=" << (U.isAdmin ? "SIM" : "NAO") << '\t'
         << "Conect=" << (U.isConnected() ? "SIM" : "NAO") << '\n';
  }
}

/// Adicionar um novo usuario
bool SupTanksServer::addUser(const string& Login, const string& Senha,
                             bool Admin)
{
  // Testa os dados do novo usuario
  if (Login.size()<6 || Login.size()>12) return false;
  if (Senha.size()<6 || Senha.size()>12) return false;

  // Testa se jah existe usuario com mesmo login
  auto itr = find(LU.begin(), LU.end(), Login);
  if (itr != LU.end()) return false;

  // Insere
  LU.push_back( User(Login,Senha,Admin) );

  // Insercao OK
  return true;
}

/// Remover um usuario
bool SupTanksServer::removeUser(const string& Login)
{
  // Testa se existe usuario com esse login
  auto itr = find(LU.begin(), LU.end(), Login);
  if (itr == LU.end()) return false;

  // Remove
  LU.erase(itr);

  // Remocao OK
  return true;
}

/// A thread que implementa o servidor.
/// Comunicacao com os clientes atraves dos sockets.
void SupTanksServer::thr_server_main(void)
{
  // Fila de sockets para aguardar chegada de dados
  mysocket_queue fila;
  // Socket temporario
  tcp_mysocket sock_temp;
  // Dados da nova conexao
  string login, senha;
  // Variaveis auxiliares:
  typedef std::list<User>::iterator iterUser;
  iterUser iU;
  mysocket_status iResult;
  int16_t val;
  int16_t cmd;
  // Estado da planta
  SupState S;

  while (server_on)
  {
    try // Erros mais graves que encerram o servidor
    {
      // Encerra se o socket de conexoes estiver fechado
      if(!sock_server.accepting()) throw 1;
      // Inclui na fila de sockets todos os sockets que eu
      // quero monitorar para ver se houve chegada de dados

      // Limpa a fila de sockets
      fila.clear();
      // Inclui na fila o socket de conexoes
      fila.include(sock_server);
      // Inclui na fila todos os sockets dos clientes conectados
      for (auto& U : LU)
      {
        if (U.isConnected()) fila.include(U.sock);
      }

      // Espera ateh que chegue dado em algum socket (com timeout)
      iResult = fila.wait_read(SUPTANKS_TIMEOUT*1000);

      // De acordo com o resultado da espera:
      switch (iResult){
      // SOCK_TIMEOUT:
      // Saiu por timeout: nao houve atividade em nenhum socket
      // Aproveita para salvar dados ou entao nao faz nada
      case mysocket_status::SOCK_TIMEOUT:
        break;
      // SOCK_ERROR:
      // Erro no select: encerra o servidor
      case mysocket_status::SOCK_ERROR:
      default:
        throw 1;
        break;
      // SOCK_OK:
      // Houve atividade em algum socket da fila:
      case mysocket_status::SOCK_OK:
        try
          {
            for (iU=LU.begin(); server_on && iU!=LU.end(); ++iU)
            {
              //   Testa se houve atividade nos sockets dos clientes. Se sim:
              if (server_on && iU->isConnected() && fila.had_activity(iU->sock))
              {
                //   - Leh o comando
                iResult = iU->sock.read_int16(cmd);
                if (iResult != mysocket_status::SOCK_OK) throw 1;
                //   - Executa a acao
                //   - Envia resposta
                switch(cmd)
                {
                  case CMD_LOGIN:
                  case CMD_ADMIN_OK:
                  case CMD_OK:
                  case CMD_ERROR:
                  case CMD_DATA:
                  default:
                      throw 2;
                      break;
                  case CMD_GET_DATA:
                    iResult = iU->sock.write_uint16(CMD_DATA);
                    if (iResult != mysocket_status::SOCK_OK) throw 2;
                    readStateFromSensors(S);
                    iResult = iU->sock.write_bytes((mybyte*)&S, sizeof(S));
                    if (iResult != mysocket_status::SOCK_OK) throw 3;
                    break;
                  case CMD_SET_V1:
                        if (!iU->isAdmin) throw 4;
                        iResult = iU->sock.read_int16(val);
                        if (iResult != mysocket_status::SOCK_OK) throw 5;
                        setV1Open(val);
                        iResult = iU->sock.write_int16(CMD_OK);
                        if (iResult != mysocket_status::SOCK_OK) throw 6;
                  break;
                  case CMD_SET_V2:
                      if (!iU->isAdmin) throw 7;
                        iResult = iU->sock.read_int16(val);
                        if (iResult != mysocket_status::SOCK_OK) throw 8;
                        setV2Open(val);
                        iResult = iU->sock.write_int16(CMD_OK);
                        if (iResult != mysocket_status::SOCK_OK) throw 9;
                  break;
                  case CMD_SET_PUMP:
                        if (!iU->isAdmin) throw 18;
                        iResult = iU->sock.read_int16(val);
                        if (iResult != mysocket_status::SOCK_OK) throw 10;
                        setPumpInput(val);
                        iResult = iU->sock.write_int16(CMD_OK);
                        if (iResult != mysocket_status::SOCK_OK) throw 11;
                      break;
                  case CMD_LOGOUT:
                    iU->close();
                    break;
                }
              }
            }
          }
        catch (int erro){
          // Erro na comunicacao
          // Fecha o socket
          iU->close();
          // Informa o erro imprevisto
          cerr << "Erro " << erro << " na leitura de nova mensagem do cliente " <<endl;
        }
      //   Depois, testa se houve atividade no socket de conexao. Se sim:
      if (server_on && fila.had_activity(sock_server) && sock_server.connected())
        {
          //- Estabelece nova conexao em socket temporario
          iResult = sock_server.accept(sock_temp);
          if (iResult != mysocket_status::SOCK_OK) throw 2;
          try
            {
              // - Leh comando, login e senha
              iResult = sock_temp.read_int16(cmd, SUPTANKS_TIMEOUT*1000);
              if (iResult != mysocket_status::SOCK_OK) throw 1;

              if (cmd!=CMD_LOGIN) throw 2;

              iResult = sock_temp.read_string(login, SUPTANKS_TIMEOUT*1000);
              if (iResult != mysocket_status::SOCK_OK) throw 3;

              iResult = sock_temp.read_string(senha, SUPTANKS_TIMEOUT*1000);
              if (iResult != mysocket_status::SOCK_OK) throw 4;
              //   - Testa usuario
              iU = find(LU.begin(), LU.end(), login);
              //   - Conecta cliente e envia confirmacao
              //usuario nao existe
              if (iU==LU.end()) throw 5;
              // Testa se a senha confere
              if (iU->password != senha) throw 6;
              // Testa se o cliente jah estah conectado
              if (iU->isConnected()) throw 7;
              // Associa o socket que se conectou a um usuario cadastrado
              iU->sock.swap(sock_temp);
              if(iU->isAdmin){
                iResult = iU->sock.write_int16(CMD_ADMIN_OK);
              }else{
                iResult = iU->sock.write_int16(CMD_OK);
              }
              if (iResult != mysocket_status::SOCK_OK) throw 8;

            } catch (int erro){
              if (erro>=5 && erro<=7)
              {
                // Envia comando informando login invalido
                sock_temp.write_int16(CMD_ERROR);
                sock_temp.close();

              }else{
                  if(erro==8){
                      // Erro na comunicacao
                      // Fecha o socket
                      iU->close();
                  }else{
                      // Fecha o socket temporario
                      sock_temp.close();
                  }
              }
            } // fim catch
        } // fim if (had_activity) no socket de conexoes
        break; // fim do case mysocket_status::SOCK_OK - resultado do wait_read
      }
    } // fim try - Erros mais graves que encerram o servidor
    catch(const char* err)  // Erros mais graves que encerram o servidor
    {
      cerr << "Erro no servidor: " << err << endl;

      // Sai do while e encerra a thread
      server_on = false;

      // Fecha todos os sockets dos clientes
      for (auto& U : LU) U.close();
      // Fecha o socket de conexoes
      sock_server.close();
      // Os tanques continuam funcionando

    } // fim catch - Erros mais graves que encerram o servidor
  } // fim while (server_on)
}
