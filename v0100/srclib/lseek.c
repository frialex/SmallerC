/*
  Copyright (c) 2014-2016, Alexey Frunze
  2-clause BSD license.
*/
#include <unistd.h>

#ifdef __HUGE__
#define __HUGE_OR_UNREAL__
#endif
#ifdef __UNREAL__
#define __HUGE_OR_UNREAL__
#endif

#ifdef _DOS

#ifdef __HUGE_OR_UNREAL__
static
int DosSeek(int handle, unsigned offset, int whence, unsigned* offsetOrError)
{
  asm("mov ah, 0x42\n"
      "mov bx, [bp + 8]\n"
      "mov dx, [bp + 12]\n"
      "mov cx, [bp + 12 + 2]\n"
      "mov al, [bp + 16]\n"
      "int 0x21");
  asm("mov bx, ax\n"
      "cmc\n"
      "sbb ax, ax\n"
      "and dx, ax\n"
      "and eax, 1");
  asm("mov esi, [bp + 20]");
#ifdef __HUGE__
  asm("ror esi, 4\n"
      "mov ds, si\n"
      "shr esi, 28\n"
      "mov [si], bx\n"
      "mov [si + 2], dx");
#else
  asm("mov [esi], bx\n"
      "mov [esi + 2], dx");
#endif
}
#endif // __HUGE_OR_UNREAL__

#ifdef _DPMI
static
int DosSeek(int handle, unsigned offset, int whence, unsigned* offsetOrError)
{
  asm("mov ah, 0x42\n"
      "mov bx, [ebp + 8]\n"
      "mov dx, [ebp + 12]\n"
      "mov cx, [ebp + 12 + 2]\n"
      "mov al, [ebp + 16]\n"
      "int 0x21");
  asm("mov bx, ax\n"
      "cmc\n"
      "sbb ax, ax\n"
      "and dx, ax\n"
      "and eax, 1");
  asm("mov esi, [ebp + 20]\n"
      "mov [esi], bx\n"
      "mov [esi + 2], dx");
}
#endif // _DPMI

#ifdef __SMALLER_C_32__
off_t lseek(int fd, off_t offset, int whence)
{
  unsigned offsetOrError;
  if (DosSeek(fd, offset, whence, &offsetOrError))
    return offsetOrError;
  return -1;
}
#endif

#endif // _DOS

#ifdef _WINDOWS

#include "iwin32.h"

off_t lseek(int fd, off_t offset, int whence)
{
  unsigned res;

  // TBD??? Fix this hack???
  // GetStdHandle(STD_INPUT_HANDLE), GetStdHandle(STD_OUTPUT_HANDLE) and GetStdHandle(STD_ERROR_HANDLE)
  // appear to always return 3, 7 and 11 when there's no redirection. Other handles (e.g. those of files)
  // appear to have values that are multiples of 4. I'm not sure if GetStdHandle() can ever return values
  // 0, 1 and 2 or if any other valid handle can ever be equal to 0, 1 or 2.
  // If 0, 1 and 2 can be valid handles in the system, I'll need to renumber/translate handles in the C library.
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    fd = GetStdHandle(STD_INPUT_HANDLE - fd);

  res = SetFilePointer(fd, offset, 0, whence);
  return res;
}

#endif // _WINDOWS


#ifdef _LINUX

off_t lseek(int fd, off_t offset, int whence)
{
  asm("mov eax, 19\n" // sys_lseek
      "mov ebx, [ebp + 8]\n"
      "mov ecx, [ebp + 12]\n"
      "mov edx, [ebp + 16]\n"
      "int 0x80");
}

#endif // _LINUX
