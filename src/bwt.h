#pragma once

#include "move_to_front.h"
#include "entropy.h"

azgra::ByteArray apply_burrows_wheeler_transform(const azgra::ByteSpan &data);