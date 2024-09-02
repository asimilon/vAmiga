// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "EXEFile.h"
#include "AmigaFile.h"
#include "MutableFileSystem.h"
#include "IOUtils.h"
#include "OSDescriptors.h"

namespace vamiga {

bool
EXEFile::isCompatible(const std::filesystem::path &path)
{
    auto suffix = util::uppercased(path.extension().string());
    return suffix == ".EXE";
}

bool
EXEFile::isCompatible(const u8 *buf, isize len)
{
    u8 signature[] = { 0x00, 0x00, 0x03, 0xF3 };

    // Only accept the file if it fits onto a HD disk
    if (len > 1710000) return false;

    return util::matchingBufferHeader(buf, signature, sizeof(signature));
}

bool
EXEFile::isCompatible(const Buffer<u8> &buf)
{
    return isCompatible(buf.ptr, buf.size);
}

bool
EXEFile::isCompatible(std::istream &stream)
{
    u8 signature[] = { 0x00, 0x00, 0x03, 0xF3 };

    // Only accept the file if it fits onto a HD disk
    if (util::streamLength(stream) > 1710000) return false;

    return util::matchingStreamHeader(stream, signature, sizeof(signature));
}

void
EXEFile::finalizeRead()
{    
    // Check if this file requires a high-density disk
    bool hd = data.size > 853000;

    // Create a new file system
    MutableFileSystem volume(INCH_35, hd ? DENSITY_HD : DENSITY_DD, FS_OFS);
    volume.setName(FSName("Disk"));
    
    // Make the volume bootable
    volume.makeBootable(BB_AMIGADOS_13);
    
    // Add the executable
    FSBlock *file = volume.createFile("file", data.ptr, data.size);
    if (!file) throw Error(VAERROR_FS_OUT_OF_SPACE);
    
    // Add a script directory
    volume.createDir("s");
    volume.changeDir("s");
    
    // Add a startup sequence
    file = volume.createFile("startup-sequence", "file");
    if (!file) throw Error(VAERROR_FS_OUT_OF_SPACE);

    // Finalize
    volume.updateChecksums();
    
    // Move to the to root directory
    volume.changeDir("/");

    // Print some debug information about the volume
    if (FS_DEBUG) {
        volume.dump(Category::State);
        volume.printDirectory(true);
    }
    
    // Check file system integrity
    FSErrorReport report = volume.check(true);
    if (report.corruptedBlocks > 0) {
        warn("Found %ld corrupted blocks\n", report.corruptedBlocks);
        if (FS_DEBUG) volume.dump(Category::Blocks);
    }

    // Convert the volume into an ADF
    adf.init(volume);
}

}
