// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskFile.h"

#define IMGSIZE_35_DD     737280  //  720 KB PC formatted disk

class IMGFile : public DiskFile {
    
public:
        
    static bool isCompatible(const string &path);
    static bool isCompatible(std::istream &stream);

    bool isCompatiblePath(const string &path) override { return isCompatible(path); }
    bool isCompatibleStream(std::istream &stream) override { return isCompatible(stream); }

    
    //
    // Initializing
    //
    
public:
    
    IMGFile(const string &path) throws { AmigaFile::init(path); }
    IMGFile(const string &path, std::istream &stream) throws { AmigaFile::init(path, stream); }
    IMGFile(const u8 *buf, isize len) throws { AmigaFile::init(buf, len); }
    IMGFile(DiskDiameter dia, DiskDensity den) throws { init(dia, den); }
    IMGFile(class Disk &disk) throws { init(disk); }

private:
    
    void init(DiskDiameter dia, DiskDensity den) throws;
    void init(class Disk &disk) throws;

    
    //
    // Methods from AmigaObject
    //
    
public:
    
    const char *getDescription() const override { return "IMG"; }
    
    
    //
    // Methods from AmigaFile
    //
        
    FileType type() const override { return FILETYPE_IMG; }
    
    
    //
    // Methods from DiskFile
    //
      
    FSVolumeType getDos() const override { return FS_NODOS; }
    void setDos(FSVolumeType dos) override { };
    DiskDiameter getDiskDiameter() const override { return INCH_35; }
    DiskDensity getDiskDensity() const override { return DISK_DD; }
    isize numSides() const override;
    isize numCyls() const override;
    isize numSectors() const override;
    void encodeDisk(class Disk &disk) throws override;
    void decodeDisk(class Disk &disk) throws override;

private:
    
    void encodeTrack(class Disk &disk, Track t) throws;
    void encodeSector(class Disk &disk, Track t, Sector s) throws;

    void decodeTrack(class Disk &disk, Track t) throws;
    void decodeSector(u8 *dst, u8 *src);
};
