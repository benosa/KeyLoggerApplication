#pragma once
#ifndef _PIPE_H_
#define _PIPE_H_

#include "includes.h"

#define PIPE_NAME _T("\\\\.\\pipe\\injectdll1")

HANDLE StartPipeServerThread(LPVOID);

#endif /* _PIPE_H_ */
