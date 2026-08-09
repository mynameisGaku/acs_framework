#include "AppStateSubsystem.h"

// GameInstance スコープへ登録する。置いたものがシーンを跨いで残ることと足並みを揃える。
ACS_REGISTER_SUBSYSTEM( CAppStateSubsystem, ESubsystemScope::GameInstance )
