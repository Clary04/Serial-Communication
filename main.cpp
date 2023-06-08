/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "UnbufferedSerial.h"
#include "mbed.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

// Blinking rate in milliseconds
#define BLINKING_RATE 50ms

#define SAMPLE_FLAG_BTUP (1UL << 0)
#define SAMPLE_FLAG_BTDOWN (1UL << 1)
#define SAMPLE_FLAG_JOYSTICK (1UL << 2)

EventFlags event_flags;

Timer timer;
InterruptIn btUp(BUTTON1);
InterruptIn btDown(PA_1);

PwmOut ledG(PA_6);
PwmOut ledF(PC_7);
PwmOut ledA(PA_7);
PwmOut ledB(PB_6);

PwmOut ledE(PC_10);
PwmOut ledD(PC_12);
PwmOut ledC(PC_11);
PwmOut ledDP(PB_7);

AnalogIn vrx(PC_0);
AnalogIn vry(PC_1);

BufferedSerial mensagemSerial1(PC_4, PA_10, 9600);

typedef struct Mensagem {
  char codigo;
  float valorJoystick;
} Msg;

bool btUp_pressionado = false;
bool btDown_pressionado = false;

int contador = 0;

char up = '+';
char down = '-';

void enviandoStruct(char c) {

  Msg msg;
  msg.codigo = c;

  if (mensagemSerial1.writable() == 1) {
    mensagemSerial1.write(&msg, sizeof(msg));
  }
}

void aumentarValor() {
  static int last_interrupt_time = 0;
  std::chrono::microseconds interrupt_time = timer.elapsed_time();
  int interrupt_time_int = static_cast<int>(interrupt_time.count());
  if (interrupt_time_int - last_interrupt_time > 200000) {
     event_flags.set(SAMPLE_FLAG_BTUP);
  }
  last_interrupt_time = interrupt_time_int;
}

void diminuirValor() {
  static int last_interrupt_time = 0;
  std::chrono::microseconds interrupt_time = timer.elapsed_time();
  int interrupt_time_int = static_cast<int>(interrupt_time.count());
  if (interrupt_time_int - last_interrupt_time > 200000) {
    event_flags.set(SAMPLE_FLAG_BTDOWN);
  }
  last_interrupt_time = interrupt_time_int;
}

void controleDeLeds(float eixoJoystick) {

  float valor = eixoJoystick;

  if (contador > 9) {
    contador = 9;
  }

  if (contador < 0) {
    contador = 0;
  }

  switch (contador) {
  
    case 0:
        ledA = valor;
        ledB = valor;
        ledC = valor;
        ledD = valor;
        ledE = valor;
        ledF = valor;
      break;
    case 1:
        ledB = valor;
        ledC = valor;
        ledA = 0;
        ledD = 0;
        ledE = 0;
        ledF = 0;
        ledG = 0;
      break;
    case 2:
        ledA = valor;
        ledB = valor;
        ledG = valor;
        ledE = valor;
        ledD = valor;
        ledC = 0;
        ledF = 0;
      break;
    case 3:
        ledA = valor;
        ledB = valor;
        ledG = valor;
        ledC = valor;
        ledD = valor;
        ledE = 0;
        ledF = 0;
      break;
    case 4:
        ledF = valor;
        ledG = valor;
        ledB = valor;
        ledC = valor;
        ledA = 0;
        ledD = 0;
      break;
    case 5:
        ledA = valor;
        ledF = valor;
        ledG = valor;
        ledC = valor;
        ledD = valor;
        ledB = 0;
        ledE = 0;
      break;
    case 6:
        ledA = valor;
        ledF = valor;
        ledG = valor;
        ledC = valor;
        ledD = valor;
        ledE = valor;
        ledB = 0;
      break;
    case 7:
        ledA = valor;
        ledB = valor;
        ledC = valor;
        ledF = 0;
        ledG = 0;
        ledD = 0;
        ledE = 0;
      break;
    case 8:
        ledA = valor;
        ledB = valor;
        ledC = valor;
        ledD = valor;
        ledE = valor;
        ledF = valor;
        ledG = valor;
      break;
    case 9:
     ledA = valor;
     ledB = valor;
     ledC = valor;
     ledF = valor;
     ledG = valor;
     ledE = 0;
     ledD = 0;
      break;  
  }

}

void SendJoystick() {

  while (true) {
    Msg msg;
    msg.codigo = 'j';
    msg.valorJoystick = vrx.read();
    mensagemSerial1.write(&msg, sizeof(msg));
    
    ThisThread::sleep_for(BLINKING_RATE);

  }
}

void ReceiveMessages() {

  while (true) {

    Msg receivedMessage;

    if (mensagemSerial1.readable() == 1) {
      mensagemSerial1.read(&receivedMessage, sizeof(receivedMessage));

      if (receivedMessage.codigo == '+') {
        contador += 1;
      } else if (receivedMessage.codigo == '-') {
        contador -= 1;
      }
      controleDeLeds(receivedMessage.valorJoystick);
    }
    ThisThread::sleep_for(BLINKING_RATE);

  }
}

void VerificarBotaoUp() {

  uint8_t flag_btUP_read = 0;

  while (true) {

  flag_btUP_read = event_flags.wait_any(SAMPLE_FLAG_BTUP);

    if(flag_btUP_read == 1){
        enviandoStruct(up);
    }
  }
}

void VerificarBotaoDown() {

  uint8_t flag_btDOWN_read = 0;

  while (true) {

   flag_btDOWN_read = event_flags.wait_any(SAMPLE_FLAG_BTDOWN);

    if(flag_btDOWN_read == 2){
        enviandoStruct(down);
    }

  }
}

int main() {

  printf("Porta serial aberta\n");

  timer.start();
  btUp.fall(&aumentarValor);
  btDown.fall(&diminuirValor);

  //Defining leds's period
   ledA.period(0.01f);
   ledB.period(0.01f);
   ledC.period(0.01f);
   ledD.period(0.01f);
   ledE.period(0.01f);
   ledF.period(0.01f);
   ledG.period(0.01f);

  Thread thread_btUp;
  thread_btUp.start(&VerificarBotaoUp);

  Thread thread_btDown;
  thread_btDown.start(&VerificarBotaoDown);

  Thread thread_sendJoystick;
  thread_sendJoystick.start(&SendJoystick);

  Thread thread_receiveMessages;
  thread_receiveMessages.start(&ReceiveMessages);

  while (true) {
    ThisThread::sleep_for(BLINKING_RATE);
  }
}