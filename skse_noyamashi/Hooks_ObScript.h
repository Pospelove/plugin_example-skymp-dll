#pragma once
#include "CommandTable.h"

void Hooks_ObScript_Init(void);
void Hooks_ObScript_Commit(void);

void ObScript_DumpCommands(void);

extern CommandTable	g_commandTable;