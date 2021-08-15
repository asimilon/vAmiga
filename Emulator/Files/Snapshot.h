// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFile.h"
#include "Constants.h"

class Amiga;

struct Thumbnail {
    
    // Image size
    u16 width, height;
    
    // Raw texture data
    u32 screen[(HPIXELS / 2) * (VPIXELS / 1)];
    
    // Creation date and time
    time_t timestamp;
    
    // Takes a screenshot from a given Amiga
    void take(Amiga &amiga, isize dx = 2, isize dy = 1);
};

struct SnapshotHeader {
    
    // Magic bytes ('V','A','S','N','A','P')
    char magic[6];
    
    // Version number (V major.minor.subminor)
    u8 major;
    u8 minor;
    u8 subminor;
    
    // Preview image
    Thumbnail screenshot;
};

class Snapshot : public AmigaFile {
 
public:
    
    static bool isCompatible(const string &path);
    static bool isCompatible(std::istream &stream);

    bool isCompatiblePath(const string &path) override { return isCompatible(path); }
    bool isCompatibleStream(std::istream &stream) override { return isCompatible(stream); }

    //
    // Initializing
    //
    
    Snapshot(const string &path) throws { init(path); }
    Snapshot(const u8 *buf, isize len) throws { init(buf, len); }
    Snapshot(isize capacity);
    Snapshot(Amiga &amiga);
    
    const char *getDescription() const override { return "Snapshot"; }
            
    
    //
    // Methods from AmigaFile
    //
    
    FileType type() const override { return FILETYPE_SNAPSHOT; }
    
    
    //
    // Accessing
    //
    
public:
    
    // Checks the snapshot version number
    bool isTooOld() const;
    bool isTooNew() const;
    bool matches() { return !isTooOld() && !isTooNew(); }
    
    // Returns a pointer to the snapshot header
    const SnapshotHeader *getHeader() const { return (SnapshotHeader *)data; }
    
    // Returns a pointer to the thumbnail image
    const Thumbnail &getThumbnail() const { return getHeader()->screenshot; }
    
    // Returns pointer to the core data
    u8 *getData() const { return data + sizeof(SnapshotHeader); }
    
    // Takes a screenshot
    void takeScreenshot(Amiga &amiga);
};
