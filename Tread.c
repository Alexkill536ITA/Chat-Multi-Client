#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

DWORD WINAPI ThreadFunc(void *data)
{
  // Do stuff.  This will be the first function called on the new thread.
  // When this function returns, the thread goes away.  See MSDN for more details.
  int nt = (int)data;
  for (int i = 0; i < 10; i++)
  {
    printf("Thread Numero %d: %d\n", nt, i);
    Sleep(1000);
  }

  return 0;
}

int main()
{
  HANDLE thread[3];
  DWORD ThrID[3];
  for (int x = 0; x < 3; x++)
  {
    thread[x] = CreateThread(NULL, 0, ThreadFunc, (void *)x, 0x00000004, &ThrID[x]);
    if (thread[x] != NULL)
    {
      // Optionally do stuff, such as wait on the thread.+
      printf("[  INFO ]ID Thread %d: %lu\n",x , ThrID[x]);
      ResumeThread(thread[x]);
      //WaitForSingleObject(thread[x], INFINITE); //singlo Thread 
    }
    Sleep(1000);
  }

  WaitForMultipleObjects(0x00000003, thread, TRUE, INFINITE); //multipli Thread

  for (int x = 0; x < 3; x++)
  {
    CloseHandle(thread[x]);
  }
  system("pause");
  return 0;
}