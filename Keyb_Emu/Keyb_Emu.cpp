// Keyb_Emu.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include "Keyb_Emu.h"

using namespace std;

char keyCode;
std::string sendToKeyboard;
bool bSend;
int gSleep;

void sendKeyEvent(std::string keystrokes) 
{
  bSend = true;
  for (char& c : keystrokes) {
    if (c >= 65 && c <= 90) 
      keybd_event('\xa0', 0, 0, 0);
    if (c >= 97 && c <= 122)
      c -= 0x20;
    keybd_event(c, 0, 0, 0);
    Sleep(gSleep);
    keybd_event(c, 0, KEYEVENTF_KEYUP, 0);
    if (c >= 65 && c <= 90)
      keybd_event('\xa0', 0, KEYEVENTF_KEYUP, 0);
    Sleep(gSleep);
  }
  bSend = false;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  BOOL fEatKeystroke = FALSE;

  if (nCode == HC_ACTION)
  {
    switch (wParam)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
       PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
       if (fEatKeystroke = (p->vkCode == keyCode)) {     //redirect a to b
         if ((wParam == WM_KEYUP) || (wParam == WM_SYSKEYUP))
         {
           if (!bSend)
             sendKeyEvent(sendToKeyboard);
         }
//          printf("sending %s\n", sendToKeyboard.c_str());
//          sendKeyEvent(sendToKeyboard);
         break;
//         keybd_event('B', 0, 0, 0);
//         keybd_event('B', 0, KEYEVENTF_KEYUP, 0);
//         break;
       }
      printf("Key - %X\n", p->vkCode);
      break;
    }
  }
  return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
}

void RegisterKey(char* key, char* sub)
{
  keyCode = *key;
  sendToKeyboard = sub;
}

int main(int argc, char **argv)
{
	cout << "Hello CMake." << endl;
  bSend = false;
  if (argc < 3)
    return 0;

  if (argc >= 3) {
    RegisterKey(argv[1],argv[2]);
  }
  if (argc >= 4)
    gSleep = atoi(argv[3]);
  else
    gSleep = 0;

  // Install the low-level keyboard & mouse hooks
  HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);

  // Keep this app running until we're told to stop
  MSG msg;
  while (!GetMessage(&msg, NULL, NULL, NULL)) {    //this while loop keeps the hook
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnhookWindowsHookEx(hhkLowLevelKybd);

  return(0);

	return 0;
}
