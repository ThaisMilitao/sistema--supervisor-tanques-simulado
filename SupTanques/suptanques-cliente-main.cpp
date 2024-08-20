#include <iostream>
#include <cmath>      /* round */
#include "suptanques-cliente.h"

using namespace std;

/// ==============================
/// Funcao principal (dialogo com o usuario)
/// ==============================

int main(int argc, char *argv[])
{
  SupTanksClient ST_Client;

  // Parametros da conexao com servidor
  string IP,Login, Senha;

  // Variaveis auxiliares para digitacao de dados
  uint16_t BInput;     // Entrada da bomba: 0 a 65535
  double BInput_perc;  // Entrada % da bomba: 0 a 100.0
  string texto;
  int opcao;

  // Imprimir numeros em ponto flutuante sempre com 1 casa decimal
  cout.precision(1);
  cout << fixed;

  do
  {
    do
    {
      cout << "\n=================\n";
      if (!ST_Client.isConnected())
      {
        cout << " 0 - Conectar cliente ao servidor\n";
        cout << "=================\n";
      }
      if (ST_Client.isConnected())
      {
        cout << " 1 - Requisitar estado da planta\n";
        cout << " 2 - Imprimir estado da planta mais recente\n";
        cout << "=================\n";
        if (ST_Client.isAdmin())
        {
          cout << "11 - Alterar a entrada da bomba\n";
          cout << "12 - Abrir a valvula do tanque 1\n";
          cout << "13 - Abrir a valvula do tanque 2\n";
          cout << "14 - Fechar a valvula do tanque 1\n";
          cout << "15 - Fechar a valvula do tanque 2\n";
          cout << "=================\n";
        }
        cout << "98 - Desconectar cliente do servidor\n";
      }
      cout << "99 - Sair\n";
      cout << "=================\n";
      cout << "Opcao: ";
      cin >> texto;
      try
      {
        opcao = stoi(texto);
      }
      catch(...)
      {
        opcao = -1;
      }
    }
    while (opcao<0 || opcao>99);

    if (!ST_Client.isConnected()) // Cliente nao estah conectado
    {
      if (opcao == 0)
      {
        do
        {
          cout << "IP do servidor: ";
          cin >> IP;
        }
        while (IP.size()<7);
        do
        {
          cout << "Login do usuario [6-12 caracteres]: ";
          cin >> Login;
        }
        while (Login.size()<6 || Login.size()>12);
        do
        {
          cout << "Senha do usuario [6-12 caracteres]: ";
          cin >> Senha;
        }
        while (Senha.size()<6 || Senha.size()>12);
        if (!ST_Client.connect(IP, Login, Senha))
        {
          cout << "Erro na conexao com IP "+IP+" e login "+Login << endl;
        }
        else
        {
          cout << "Conectado ao IP "+IP+" com login "+Login << endl;
        }
      }
      else
      {
        if (opcao != 99) cerr << "Cliente nao estah conectado!\n";
      }
    }
    else  // Cliente estah conectado
    {
      // Executa a opcao escolhida
      switch(opcao)
      {
      case 0:
        cerr << "Cliente jah estah conectado!\n";
        break;
      case 1:
        // Solicita ao servidor o estado atual da planta
        if (!ST_Client.requestState()) cout << "Erro na requisicao do estado\n";
        break;
      case 2:
        // Imprime o ultimo estado da planta recebido do servidor
        if (!ST_Client.printState()) cout << "Erro na impressao do estado\n";
        break;
      case 11:
        if (ST_Client.isAdmin())
        {
          do
          {
            cout << "Entrada % da bomba [0.0 a 100.0]: ";
            cin >> texto;
            try
            {
              BInput_perc = stof(texto);
            }
            catch(...)
            {
              BInput_perc = -1.0;
            }
          }
          while (BInput_perc<0.0 || BInput_perc>100.0);
          BInput = round(UINT16_MAX*BInput_perc/100.0);
          if (!ST_Client.setPumpInput(BInput)) cerr << "Erro na alteracao da bomba\n";
        }
        else
        {
          cerr << "Cliente nao eh administrador!\n";
        }
        break;
      case 12:
        if (ST_Client.isAdmin())
        {
          if (!ST_Client.setV1Open(true)) cerr << "Erro na abertura da valvula 1\n";
        }
        else
        {
          cerr << "Cliente nao eh administrador!\n";
        }
        break;
      case 13:
        if (ST_Client.isAdmin())
        {
          if (!ST_Client.setV2Open(true)) cerr << "Erro na abertura da valvula 2\n";
        }
        else
        {
          cerr << "Cliente nao eh administrador!\n";
        }
        break;
      case 14:
        if (ST_Client.isAdmin())
        {
          if (!ST_Client.setV1Open(false)) cerr << "Erro no fechamento da valvula 1\n";
        }
        else
        {
          cerr << "Cliente nao eh administrador!\n";
        }
        break;
      case 15:
        if (ST_Client.isAdmin())
        {
          if (!ST_Client.setV2Open(false)) cerr << "Erro no fechamento da valvula 2\n";
        }
        else
        {
          cerr << "Cliente nao eh administrador!\n";
        }
        break;
      case 98:
      case 99:
        ST_Client.disconnect();
        break;
      default:
        // Opcao inexistente: nao faz nada
        cerr << "Opcao inexistente!\n";
        break;
      }
    }
  }
  while (opcao!=99);

  return 0;
}
