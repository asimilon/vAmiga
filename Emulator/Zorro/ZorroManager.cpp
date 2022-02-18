// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "ZorroManager.h"
#include "Memory.h"

ZorroManager::ZorroManager(Amiga& ref) : SubComponent(ref)
{
    subComponents = std::vector<AmigaComponent *> {
        
        &ramExpansion,
        &hdrController
    };
}

u8
ZorroManager::peek(u32 addr) const
{
    for (isize i = 0; slots[i]; i++) {

        if (slots[i]->state == STATE_AUTOCONF) {
            
            return slots[i]->peekAutoconf8(addr);
        }
    }
    return 0xFF;
}

void
ZorroManager::poke(u32 addr, u8 value)
{
    for (isize i = 0; slots[i]; i++) {

        if (slots[i]->state == STATE_AUTOCONF) {
            
            slots[i]->pokeAutoconf8(addr, value);
            return;
        }
    }
}

void
ZorroManager::updateMemSrcTables()
{
    for (isize i = 0; slots[i]; i++) {
        
        slots[i]->updateMemSrcTables();
    }
}