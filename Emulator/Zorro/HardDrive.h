// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ZorroBoard.h"

class HardDrive : public ZorroBoard {
            
    //
    // Constructing
    //
    
public:
    
    using ZorroBoard::ZorroBoard;
    
    
    //
    // Methods from AmigaObject
    //
    
private:
    
    const char *getDescription() const override { return "HardDrive"; }
    void _dump(dump::Category category, std::ostream& os) const override;

    
    //
    // Methods from AmigaComponent
    //
    
private:
    
    void _reset(bool hard) override;
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {

    }

    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        if (hard) {
            
            worker
            
            << state
            << baseAddr;
        }
    }
    
    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Methods from ZorroBoard
    //
    
    virtual u8 type() const override            { return ERTF_DIAGVALID | 1; }
    virtual u8 product() const override         { return 0x88; }
    virtual u8 flags() const override           { return 0x80; }
    virtual u16 manufacturer() const override   { return 0x0539; }
    virtual u32 serialNumber() const override   { return 0x5041554C; }
    virtual u16 initDiagVec() const override    { return 0x40; }
        
    void updateMemSrcTables() override;
    
    
    //
    // Accessing the board
    //
       
    virtual u8 peek8(u32 addr) const;
    virtual u8 spypeek8(u32 addr) const { return peek8(addr); }
    void poke8(u32 addr, u8 value);
};
