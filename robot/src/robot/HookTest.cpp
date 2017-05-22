#include <windows.h>
#include "HookTest.h"

HINSTANCE hInst = NULL;
JavaVM * jvm = NULL;
jobject hookObj = NULL;
jmethodID processKeyID = NULL;
DWORD hookThreadId = 0;

extern "C" BOOL APIENTRY DllMain(HINSTANCE _hInst, DWORD reason, LPVOID
reserved) {
switch (reason) {
case DLL_PROCESS_ATTACH:
printf("C++: DllMain - DLL_PROCESS_ATTACH.\n");
hInst = _hInst;
break;
default:
break;
}

return TRUE;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM
lParam) {
JNIEnv * env;
KBDLLHOOKSTRUCT * p = (KBDLLHOOKSTRUCT *)lParam;

if (jvm->AttachCurrentThread((void **)&env, NULL) >= 0) {
switch (wParam) {
case WM_KEYDOWN:
case WM_SYSKEYDOWN:
printf("C++: LowLevelKeyboardProc - Key pressed\n");
env->CallVoidMethod(hookObj, processKeyID, p->vkCode,
true);
break;
case WM_KEYUP:
case WM_SYSKEYUP:
printf("C++: LowLevelKeyboardProc - Key released\n");
env->CallVoidMethod(hookObj, processKeyID, p->vkCode,
false);
break;
default:
break;
}
}
else {
printf("C++: LowLevelKeyboardProc - Error on the attach current
thread.\n");
}

return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void MsgLoop() {
MSG message;

while (GetMessage(&message, NULL, 0, 0)) {
TranslateMessage(&message);
DispatchMessage(&message);
}
}

JNIEXPORT void JNICALL Java_HookTest_registerHook(JNIEnv * env, jobject
obj) {
HHOOK hookHandle = SetWindowsHookEx(WH_KEYBOARD_LL,
LowLevelKeyboardProc, hInst, 0);

if (hookHandle == NULL) {
printf("C++: Java_HookTest_registerHook - Hook failed!\n");
return;
}
else {
printf("C++: Java_HookTest_registerHook - Hook successful\n");
}

hookObj = env->NewGlobalRef(obj);
jclass cls = env->GetObjectClass(hookObj);
processKeyID = env->GetMethodID(cls, "processKey", "(IZ)V");
env->GetJavaVM(&jvm);
hookThreadId = GetCurrentThreadId();

MsgLoop();

if (!UnhookWindowsHookEx(hookHandle))
printf("C++: Java_HookTest_registerHook - Unhook failed\n");

else
printf("C++: Java_HookTest_registerHook - Unhook
successful\n");
}

JNIEXPORT void JNICALL Java_HookTest_unRegisterHook(JNIEnv *env,
jobject object) {
if (hookThreadId == 0)
return;

printf("C++: Java_HookTest_unRegisterHook - call
PostThreadMessage.\n");
PostThreadMessage(hookThreadId, WM_QUIT, 0, 0L);
}