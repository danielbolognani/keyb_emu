// Keyb_Emu.cpp : Defines the entry point for the application.
//

#ifdef _WINDOWS
#include <windows.h>
#else
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>
#include <stdio.h>
#include <linux/input.h>
#include <linux/uinput.h>
#endif
#include <map>
#include "Keyb_Emu.h"

using namespace std;

char keyCode;
std::string sendToKeyboard;
bool bSend;
int gSleep;

#ifndef _WINDOWS

#define  WM_KEYUP       0
#define  WM_KEYDOWN     1
#define  WM_KEYREPEAT   2
#define  WM_SYSKEYUP    10
#define  WM_SYSKEYDOWN  11


static const char *const evval[3] = {
    "RELEASED",
    "PRESSED ",
    "REPEATED"
};
#define Sleep usleep

std::map<char, int> key_ascii {
{'a', KEY_A},
{'b', KEY_B},
{'c', KEY_C},
{'d', KEY_D},
{'e', KEY_E},
{'f', KEY_F},
{'g', KEY_G},
{'h', KEY_H},
{'i', KEY_I},
{'j', KEY_J},
{'k', KEY_K},
{'l', KEY_L},
{'m', KEY_M},
{'n', KEY_N},
{'o', KEY_O},
{'p', KEY_P},
{'q', KEY_Q},
{'r', KEY_R},
{'s', KEY_S},
{'t', KEY_T},
{'u', KEY_U},
{'v', KEY_V},
{'w', KEY_W},
{'x', KEY_X},
{'y', KEY_Y},
{'z', KEY_Z},
{'1', KEY_1},
{'2', KEY_2},
{'3', KEY_3},
{'4', KEY_4},
{'5', KEY_5},
{'6', KEY_6},
{'7', KEY_7},
{'8', KEY_8},
{'9', KEY_9},
{'0', KEY_0},
{'-', KEY_MINUS},
{'=', KEY_EQUAL},
{'/', KEY_SLASH},
{'\\', KEY_BACKSLASH},
{',', KEY_COMMA},
{'.', KEY_DOT},
{' ', KEY_SPACE}
};

// Por eqto aceito apenas teclas de função como atalho
std::map<std::string, int> fkeys {
{"F1", KEY_F1},
{"F2", KEY_F2},
{"F3", KEY_F3},
{"F4", KEY_F4},
{"F5", KEY_F5},
{"F6", KEY_F6},
{"F7", KEY_F7},
{"F8", KEY_F8},
{"F9", KEY_F9},
{"F10", KEY_F10},
{"F11", KEY_F11},
{"F12", KEY_F12}
};

// create uinput file descriptor
int fd_key_emulator;

int sendKey(int nCode, int bKeyPress);
#else //windows

// Por eqto aceito apenas teclas de função como atalho
std::map<std::string, int> fkeys {
{"F1", VK_F1},
{"F2", VK_F2},
{"F3", VK_F3},
{"F4", VK_F4},
{"F5", VK_F5},
{"F6", VK_F6},
{"F7", VK_F7},
{"F8", VK_F8},
{"F9", VK_F9},
{"F10", VK_F10},
{"F11", VK_F11},
{"F12", VK_F12}
};

#endif

// Recebe o string para enviar para o buffer do teclado e envia
// para o SO (infelizmente essa parte é diferente para Windows/Linux)
void sendKeyEvent(std::string keystrokes) 
{
  bSend = true;

  bool lShift = false;

  for (char& c : keystrokes) {
    if (c >= 0x41 && c <= 0x5a) {
#ifdef _WINDOWS
      keybd_event('\xa0', 0, 0, 0);
#else
// No linux estou usando o código com o ASCII minúsculo, por isso tira 0x20 do codigo original
// (Tabela ASCII)
      sendKey(KEY_LEFTSHIFT, 1);
      c += 0x20;
#endif
      lShift = true;
    }
#ifdef _WINDOWS
// No windows nem precisou de um de/para dos códigos de tecla para ascii, só precisa garantir que 
// seja tudo em maiúsculo, por isso subtrai 0x20 (Tabela ASCII)
    if (c >= 0x61 && c <= 0x7a) {
      c -= 0x20;
    }
    keybd_event(c, 0, 0, 0);
#else
    int nkey = key_ascii[c];
    sendKey(nkey, 1);
#endif
//Sleep global entre pressionar e soltar em milisegundos
    Sleep(gSleep);
#ifdef _WINDOWS
    keybd_event(c, 0, KEYEVENTF_KEYUP, 0);
#else
    sendKey(nkey, 0);
#endif
    if (lShift) {
#ifdef _WINDOWS
      keybd_event('\xa0', 0, KEYEVENTF_KEYUP, 0);
#else
      sendKey(KEY_LEFTSHIFT, 0);
#endif
    }
    Sleep(gSleep);
  }
//Por enquanto to sempre enviando um ENTER no final do texto
#ifdef _WINDOWS
  keybd_event(VK_RETURN, 0, 0, 0);
#else
  sendKey(KEY_ENTER, 1);
#endif
  Sleep(gSleep);
#ifdef _WINDOWS
  keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);
#else
  sendKey(KEY_ENTER, 0);
#endif
  bSend = false;
}

// Em windows isso é uma função de callback
// Em Linux eu chamo manualmente e mudo um pouco os parametros
#ifdef _WINDOWS
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
#else
int LowLevelKeyboardProc(int nCode, int wParam)
#endif
{
  bool fEatKeystroke = false;
#ifdef _WINDOWS
  if (nCode == HC_ACTION)
  {
#endif
    switch (wParam)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
#ifdef _WINDOWS
       PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
       if (fEatKeystroke = (p->vkCode == keyCode)) {     //redirect a to b
#else
       if (nCode == keyCode) {
#endif
         if ((wParam == WM_KEYUP) || (wParam == WM_SYSKEYUP))
         {
           if (!bSend){
             std::cout << "Shortcut recognized, sending buffer to keyboard" << std::endl;
             sendKeyEvent(sendToKeyboard);
           }
         }
         break;
      }         
#ifdef _WINDOWS
      std::cout << "Key - " << std::hex << std::uppercase << p->vkCode << std::endl;
#else
      std::cout << "Key - " << std::hex << std::uppercase << nCode << " (keycode - " << keyCode << ")" << std::endl;
#endif
      break;
    }
#ifdef _WINDOWS
  }
  return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
#else
  return 0;
#endif
}

// Salva a tecla de atalho e qual o código deve enviar para o buffer de teclado
void RegisterKey(char* key, char* sub)
{
  keyCode = fkeys[key];
  sendToKeyboard = sub;
}

#ifndef _WINDOWS
// Inicia um teclado virtual no linux
// adiciona um input no /dev como se fosse um teclado mesmo
// nessa situação precisa enviar todas as teclas que esse teclado possui
// As teclas estão no map key_ascii global, se necessitar de alguma outra tecla
// é necessário adicionar nesse map global
int initLinuxKb()
{

  // open file descriptor
  fd_key_emulator = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if(fd_key_emulator < 0)
  {
    std::cout << "error in open : " << strerror(errno) << std::endl;
    return -1;
  }
  // uinput_user_dev struct for fake keyboard
  struct uinput_user_dev dev_fake_keyboard;
  memset(&dev_fake_keyboard, 0, sizeof(uinput_user_dev));
  snprintf(dev_fake_keyboard.name, UINPUT_MAX_NAME_SIZE, "kb-emulator");
  dev_fake_keyboard.id.bustype = BUS_USB;
  dev_fake_keyboard.id.vendor = 0x01;
  dev_fake_keyboard.id.product = 0x01;
  dev_fake_keyboard.id.version = 1;

  /**configure the input device to send type of events, inform to subsystem which 
  * type of input events we are using via ioctl calls. 
  * UI_SET_EVBIT ioctl request is used to applied on uinput descriptor to enable a type of event.
  **/
  // enable key press/release event
  if(ioctl(fd_key_emulator, UI_SET_EVBIT, EV_KEY))
  {
    std::cout << "Error in ioctl : UI_SET_EVBIT : EV_KEY : " << strerror(errno) << std::endl;
    return -1;
  }

  // enable set of KEY events here
  for (const auto& [key, value] : key_ascii)
    ioctl(fd_key_emulator, UI_SET_KEYBIT, value);
  ioctl(fd_key_emulator, UI_SET_KEYBIT, KEY_TAB);
  ioctl(fd_key_emulator, UI_SET_KEYBIT, KEY_LEFTBRACE);
  ioctl(fd_key_emulator, UI_SET_KEYBIT, KEY_RIGHTBRACE);
  ioctl(fd_key_emulator, UI_SET_KEYBIT, KEY_ENTER);
  ioctl(fd_key_emulator, UI_SET_KEYBIT, KEY_LEFTCTRL);
  ioctl(fd_key_emulator, UI_SET_KEYBIT, KEY_SEMICOLON);
  ioctl(fd_key_emulator, UI_SET_KEYBIT, KEY_APOSTROPHE);
  ioctl(fd_key_emulator, UI_SET_KEYBIT, KEY_GRAVE);
  ioctl(fd_key_emulator, UI_SET_KEYBIT, KEY_LEFTSHIFT);
  ioctl(fd_key_emulator, UI_SET_KEYBIT, KEY_RIGHTSHIFT);
  

  // enable synchronization event
  if(ioctl(fd_key_emulator, UI_SET_EVBIT, EV_SYN))
  {
    std::cout << "Error in ioctl : UI_SET_EVBIT : EV_SYN : " << strerror(errno) << std::endl;
    return -1;
  }

  // now write the uinput_user_dev structure into uinput file descriptor
  if(write(fd_key_emulator, &dev_fake_keyboard, sizeof(uinput_user_dev)) < 0)
  {
    std::cout << "Error in write(): uinput_user_dev struct into uinput file descriptor: " << strerror(errno) << std::endl;
    return -1;
  }

  // create the device via an IOCTL call 
  if(ioctl(fd_key_emulator, UI_DEV_CREATE))
  {
    std::cout << "Error in ioctl : UI_DEV_CREATE : " << strerror(errno) << std::endl;
    return -1;
  }

  // now fd_key_emulator represents the end-point file descriptor of the new input device. 
  // Apenas para dar tempo ao linux para criar o teclado "virtual"
  sleep(1);
  return 0;
}

// Envia de fato a tecla para o buffer (função para linux apenas)
int sendKey(int nCode, int bKeyPress)
{
  //struct member for input events
  struct input_event key_input_event;
  memset(&key_input_event, 0, sizeof(input_event));
  
  // key press event for 'a'
  key_input_event.type = EV_KEY;
  key_input_event.code = nCode;
  key_input_event.value = bKeyPress;
  
  // now write to the file descriptor
  if(write(fd_key_emulator, &key_input_event, sizeof(input_event)) < 0)
  {
    std::cout << "Error write : KEY_A press : " << strerror(errno) << std::endl;
  }

  memset(&key_input_event, 0, sizeof(input_event));
  // EV_SYN for key press event
  key_input_event.type = EV_SYN;
  key_input_event.code = SYN_REPORT;
  key_input_event.value = 0;
  
  // now write to the file descriptor
  if(write(fd_key_emulator, &key_input_event, sizeof(input_event)) < 0)
  {
    std::cout << "Error write : EV_SYN for key release : " << strerror(errno) << std::endl;
  }                                      

  return 0;
}
#endif

int main(int argc, char **argv)
{
#ifndef _WINDOWS
  if (initLinuxKb() < 0) {
    std::cout << "Failed to start, make sure your are running this program as ROOT" << std::endl;
    return -1;
  }
#endif
  bSend = false;
  if (argc < 3) {
    std::cout << "Invalid arguments" << std::endl;
    std::cout << " Keyb_emu <shortcut> <code> <time>" << std::endl;
    std::cout << " shortcut - F1 to F12" << std::endl;
    std::cout << " code - String text to send to keyboard buffer" << std::endl;
    std::cout << " time (optional) - Time in miliseconds to wait between keydown/keyup and between keys" << std::endl;
    return 0;
  }

  if (argc >= 3) {
    RegisterKey(argv[1],argv[2]);
    std::cout << "Hello, shortcut registered as " << argv[1] << std::endl;
  }
  if (argc >= 4)
    gSleep = atoi(argv[3]);
  else
    gSleep = 0;

#ifdef _WINDOWS

  // Install the low-level keyboard & mouse hooks
  HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);

  // Keep this app running until we're told to stop
  MSG msg;
  while (!GetMessage(&msg, NULL, NULL, NULL)) {    //this while loop keeps the hook
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnhookWindowsHookEx(hhkLowLevelKybd);
#else

  const char *dev = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
  struct input_event ev;
  ssize_t n;
  int fd;

  fd = open(dev, O_RDONLY);
  if (fd == -1) {
    std::cout << "Cannot open " << dev << ":" << strerror(errno) << "." << std::endl;
    return EXIT_FAILURE;
  }

  while (1) {
    n = read(fd, &ev, sizeof ev);
    if (n == (ssize_t)-1) {
      if (errno == EINTR)
        continue;
      else
        break;
    }
    else if (n != sizeof ev) {
      errno = EIO;
      break;
    }
    if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2) {
      LowLevelKeyboardProc(ev.code, ev.value);
    }
  }
  std::cout << "Error " << strerror(errno) << "." << std::endl;
  return EXIT_FAILURE;
#endif

  return(0);

}
