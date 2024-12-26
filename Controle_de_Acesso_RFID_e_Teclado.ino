/*
  Projeto: Controle de Acesso
  Autor: Rennan
  Data: 25/12/2024
  Descrição: Código de controle de acesso utilizando
  M5Cardputer e um modulo RFID

   Hardwares ultilizado:
  - M5Cardputer
  - WS1850S (RFID 2)

  Conexões:
  - RFID:
    -> Porta I2C
      - GND
      - VCC
      - G1
      - G2

  Observação:
  - Acesse o site da M5Stack para baixar, instalar e configurar 
    suas placas e bibliotecas
  - Para subir o código no M5Cardputer, segure Btn G0
    antes de conectar no pc
*/

#include <MFRC522_I2C.h>
#include <Wire.h>
#include "M5Cardputer.h"
#include "Cadastros.h"


// Definição dos pinos
#define MFRC522_I2C_ADDRESS 0x28
#define RST_PIN -1
// Instancia o objeto MFRC522
MFRC522_I2C mfrc522(MFRC522_I2C_ADDRESS, RST_PIN, &Wire);


String data = ""; // Variável para armazenar o texto digitado.
byte coluna = 0;
String usuario_liberado = "";

void setup()
{
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextFont(&fonts::FreeSansBold18pt7b);
    M5Cardputer.Display.setTextSize(0.5);
    M5Cardputer.Display.fillScreen(BLACK);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.drawString("Aproxime o cartão", 10, 10);
    M5Cardputer.Display.drawString("ou digite sua matricula", 10, 30);
    M5Cardputer.Display.drawString("e pressione Enter::", 10, 50);

    Wire.begin(G2, G1);   // Inicializa o barramento I2C
    mfrc522.PCD_Init();   // Inicializa o módulo RFID
}

void loop()
{
    M5Cardputer.update();

    /*Se alguma tecla foi pressionada, executar*/
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed())
    {
       tecla_pressionada();
       return;
    }

    /* Caso não detectar nenhum cartão, executar*/
    if (!mfrc522.PICC_IsNewCardPresent())
    {
      delay(50);
      return;
    }

     /*Caso detecte mais de um cartão seleciona um deles*/
    if (!mfrc522.PICC_ReadCardSerial())
    {
      delay(50);
      return;
    }

    /*Se chegou aqui, é porque detectou um cartão e nao pressionou nenhuma tecla*/
    leitura_do_cartao();

} //Fim da Função Principal


void tecla_pressionada()
{
   Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
   coluna = 1;
        /*Função que vai adicionando a String 'data' cada tecla que for pressionada*/
        for (auto i : status.word)
        {
                data += i;
        }
        /*Caso aperte o botao delete, e a String não está vazia, irá apaga a ultima String*/
        if (status.del && !data.isEmpty()) {
            data.remove(data.length() - 1);
        }

        /*Caso aperte Enter irá executar esta função*/
        if (status.enter)
        {
          /*Se verifica_cadastro verdadeiro, libera_usuario(), se nao, nao_libera()*/
          verifica_cadastro(data, 1) ? libera_usuario() : nao_libera();

          estado_inicial();
        } else { /*Enquanto não apertar Enter, irá executar esta função*/
            M5Cardputer.Display.fillRect(10, 115, M5Cardputer.Display.width(), 30, BLACK);
            M5Cardputer.Display.drawString(data, 10, 115);
        }
}

void leitura_do_cartao()
{
  data = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    data.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    data.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  data.toUpperCase();

  /*Se verifica_cadastro verdadeiro, libera_usuario, se nao, nao_libera*/
  verifica_cadastro(data.substring(1), 2) ? libera_usuario() : nao_libera();
  
}

bool verifica_cadastro(String valor_de_entrada, int coluna)
{
    for (int i = 0; i < (sizeof(cadastros) / sizeof(cadastros[0])); i++)
    {
        if (valor_de_entrada.equalsIgnoreCase(cadastros[i][coluna]))
        {
            usuario_liberado = cadastros[i][0];
            return true;
        }
    }
    return false;
}

void libera_usuario()
{
  M5Cardputer.Speaker.tone(10000, 100);
  M5Cardputer.Display.fillScreen(GREEN);
  M5Cardputer.Display.setTextDatum(middle_center);
  M5Cardputer.Display.drawString(usuario_liberado,
                                 M5Cardputer.Display.width() / 2,
                                 M5Cardputer.Display.height() / 2);
  M5Cardputer.Display.setTextDatum(top_left);
  delay(5000);
  estado_inicial();
}

void nao_libera()
{
  M5Cardputer.Speaker.tone(600, 100);
  M5Cardputer.Display.fillScreen(RED);
  M5Cardputer.Display.setTextDatum(top_center);
  M5Cardputer.Display.drawString("Nao Liberado",
                                  M5Cardputer.Display.width() / 2,
                                  M5Cardputer.Display.height() / 2);
  M5Cardputer.Display.setTextDatum(top_left);
  delay(5000);
  estado_inicial();
}

void estado_inicial()
{
  coluna           =  0;
  data             = "";
  usuario_liberado = "";
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.drawString("Aproxime o cartão", 10, 10);
  M5Cardputer.Display.drawString("ou digite sua matricula", 10, 30);
  M5Cardputer.Display.drawString("e pressione Enter::", 10, 50);
}
