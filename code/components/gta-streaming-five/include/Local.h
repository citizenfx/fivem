#pragma once

#include <cstdint>

constexpr int kNumWeaponInfoBlobs = 512;

bool ReloadOverriddenMetaData(uint32_t globalIndex, uint32_t nameHash);
