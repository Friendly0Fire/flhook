#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    const std::unique_ptr<SubmitData> chatData = std::make_unique<SubmitData>();
}

#define FarProcCast reinterpret_cast<FARPROC>
HookEntry IServerImplEntries[] = {
    {             FarProcCast(IServerImplHook::SubmitChat), -0x008, nullptr},
    {             FarProcCast(IServerImplHook::FireWeapon),  0x000, nullptr},
    {          FarProcCast(IServerImplHook::ActivateEquip),  0x004, nullptr},
    {         FarProcCast(IServerImplHook::ActivateCruise),  0x008, nullptr},
    {      FarProcCast(IServerImplHook::ActivateThrusters),  0x00C, nullptr},
    {              FarProcCast(IServerImplHook::SetTarget),  0x010, nullptr},
    {         FarProcCast(IServerImplHook::TractorObjects),  0x014, nullptr},
    {            FarProcCast(IServerImplHook::GoTradelane),  0x018, nullptr},
    {          FarProcCast(IServerImplHook::StopTradelane),  0x01C, nullptr},
    {          FarProcCast(IServerImplHook::JettisonCargo),  0x020, nullptr},
    {             FarProcCast(IServerImplHook::DisConnect),  0x040, nullptr},
    {              FarProcCast(IServerImplHook::OnConnect),  0x044, nullptr},
    {                  FarProcCast(IServerImplHook::Login),  0x048, nullptr},
    {       FarProcCast(IServerImplHook::CharacterInfoReq),  0x04C, nullptr},
    {        FarProcCast(IServerImplHook::CharacterSelect),  0x050, nullptr},
    {     FarProcCast(IServerImplHook::CreateNewCharacter),  0x058, nullptr},
    {       FarProcCast(IServerImplHook::DestroyCharacter),  0x05C, nullptr},
    {            FarProcCast(IServerImplHook::ReqShipArch),  0x064, nullptr},
    {             FarProcCast(IServerImplHook::ReqHulatus),  0x068, nullptr},
    {     FarProcCast(IServerImplHook::ReqCollisionGroups),  0x06C, nullptr},
    {           FarProcCast(IServerImplHook::ReqEquipment),  0x070, nullptr},
    {             FarProcCast(IServerImplHook::ReqAddItem),  0x078, nullptr},
    {          FarProcCast(IServerImplHook::ReqRemoveItem),  0x07C, nullptr},
    {          FarProcCast(IServerImplHook::ReqModifyItem),  0x080, nullptr},
    {             FarProcCast(IServerImplHook::ReqSetCash),  0x084, nullptr},
    {          FarProcCast(IServerImplHook::ReqChangeCash),  0x088, nullptr},
    {              FarProcCast(IServerImplHook::BaseEnter),  0x08C, nullptr},
    {               FarProcCast(IServerImplHook::BaseExit),  0x090, nullptr},
    {          FarProcCast(IServerImplHook::LocationEnter),  0x094, nullptr},
    {           FarProcCast(IServerImplHook::LocationExit),  0x098, nullptr},
    {        FarProcCast(IServerImplHook::BaseInfoRequest),  0x09C, nullptr},
    {    FarProcCast(IServerImplHook::LocationInfoRequest),  0x0A0, nullptr},
    {            FarProcCast(IServerImplHook::GFObjSelect),  0x0A4, nullptr},
    {        FarProcCast(IServerImplHook::GFGoodVaporized),  0x0A8, nullptr},
    {        FarProcCast(IServerImplHook::MissionResponse),  0x0AC, nullptr},
    {          FarProcCast(IServerImplHook::TradeResponse),  0x0B0, nullptr},
    {              FarProcCast(IServerImplHook::GFGoodBuy),  0x0B4, nullptr},
    {             FarProcCast(IServerImplHook::GFGoodSell),  0x0B8, nullptr},
    {FarProcCast(IServerImplHook::SystemSwitchOutComplete),  0x0BC, nullptr},
    {           FarProcCast(IServerImplHook::PlayerLaunch),  0x0C0, nullptr},
    {         FarProcCast(IServerImplHook::LaunchComplete),  0x0C4, nullptr},
    {         FarProcCast(IServerImplHook::JumpInComplete),  0x0C8, nullptr},
    {                   FarProcCast(IServerImplHook::Hail),  0x0CC, nullptr},
    {            FarProcCast(IServerImplHook::SPObjUpdate),  0x0D0, nullptr},
    {    FarProcCast(IServerImplHook::SPMunitionCollision),  0x0D4, nullptr},
    {         FarProcCast(IServerImplHook::SPObjCollision),  0x0DC, nullptr},
    {       FarProcCast(IServerImplHook::SPRequestUseItem),  0x0E0, nullptr},
    { FarProcCast(IServerImplHook::SPRequestInvincibility),  0x0E4, nullptr},
    {           FarProcCast(IServerImplHook::RequestEvent),  0x0F0, nullptr},
    {          FarProcCast(IServerImplHook::RequestCancel),  0x0F4, nullptr},
    {           FarProcCast(IServerImplHook::MineAsteroid),  0x0F8, nullptr},
    {      FarProcCast(IServerImplHook::RequestCreateShip),  0x100, nullptr},
    {            FarProcCast(IServerImplHook::SPScanCargo),  0x104, nullptr},
    {            FarProcCast(IServerImplHook::SetManeuver),  0x108, nullptr},
    {      FarProcCast(IServerImplHook::InterfaceItemUsed),  0x10C, nullptr},
    {           FarProcCast(IServerImplHook::AbortMission),  0x110, nullptr},
    {         FarProcCast(IServerImplHook::SetWeaponGroup),  0x118, nullptr},
    {        FarProcCast(IServerImplHook::SetVisitedState),  0x11C, nullptr},
    {        FarProcCast(IServerImplHook::RequestBestPath),  0x120, nullptr},
    {     FarProcCast(IServerImplHook::RequestPlayerStats),  0x124, nullptr},
    {            FarProcCast(IServerImplHook::PopupDialog),  0x128, nullptr},
    {  FarProcCast(IServerImplHook::RequestGroupPositions),  0x12C, nullptr},
    {      FarProcCast(IServerImplHook::SetInterfaceState),  0x134, nullptr},
    {       FarProcCast(IServerImplHook::RequestRankLevel),  0x138, nullptr},
    {          FarProcCast(IServerImplHook::InitiateTrade),  0x13C, nullptr},
    {         FarProcCast(IServerImplHook::TerminateTrade),  0x140, nullptr},
    {            FarProcCast(IServerImplHook::AcceptTrade),  0x144, nullptr},
    {          FarProcCast(IServerImplHook::SetTradeMoney),  0x148, nullptr},
    {          FarProcCast(IServerImplHook::AddTradeEquip),  0x14C, nullptr},
    {          FarProcCast(IServerImplHook::DelTradeEquip),  0x150, nullptr},
    {           FarProcCast(IServerImplHook::RequestTrade),  0x154, nullptr},
    {       FarProcCast(IServerImplHook::StopTradeRequest),  0x158, nullptr},
    {                   FarProcCast(IServerImplHook::Dock),  0x16C, nullptr},
};

#undef FarProcCast
