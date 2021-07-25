#include "skyplug.h"
MenuManager *const mm = MenuManager::GetSingleton();
InputStringHolder *const ish = InputStringHolder::GetSingleton();

unsigned short threadcheck = 0;

unsigned long destpause = 0;
unsigned long disablearm = 0;
bool ininventory = false;
bool ismenuopen[36] = { false };
enum struct started_mode started_mode = started_mode::none;

bool disable_game_by_console = false;
bool speccheck = false;
UInt8 wait_for_troll = 0;
bool controlsdebug = false;

BaseFormComponent::BaseFormComponent() {}
BaseFormComponent::~BaseFormComponent() {}
void BaseFormComponent::Init() {}
void BaseFormComponent::ReleaseRefs() {}
void BaseFormComponent::CopyFromBase(BaseFormComponent * rhs) {}