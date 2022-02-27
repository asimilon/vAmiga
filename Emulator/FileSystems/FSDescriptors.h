// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSTypes.h"
#include "FloppyDisk.h"
#include "Error.h"
#include "FSObjects.h"
#include "FSBlock.h"
#include "DriveDescriptors.h"

/* To create a FileSystem, several layout parameters need to to be provided.
 * This is done by passing a FileSystemDescriptor which contains the necessary
 * information.
 *
 * A FileSystemDescriptor can be obtained in several ways. If a descriptor for
 * a floppy disk is needed, it can be created by specifiying the form factor
 * and density of the disk. In addition, a suitabe descriptors can be extracted
 * directly from an ADF or HDF.
 */
struct FileSystemDescriptor {

    // Capacity of the file system in blocks
    isize numBlocks = 0;
    
    // Size of a block in bytes
    isize bsize = 512;
    
    // Number of reserved blocks
    isize numReserved = 0;
        
    // File system type
    FSVolumeType dos = FS_NODOS;
            
    // Location of the root block
    Block rootBlock = 0;
    
    // References to all bitmap blocks and bitmap extension blocks
    std::vector<Block> bmBlocks;
    std::vector<Block> bmExtBlocks;
    
    // Initializing
    FileSystemDescriptor() { };
    FileSystemDescriptor(isize numBlocks, FSVolumeType dos);
    FileSystemDescriptor(const Geometry &geometry, FSVolumeType dos);
    FileSystemDescriptor(Diameter dia, Density den, FSVolumeType dos);
    
    void init(isize numBlocks, FSVolumeType dos);
    void init(const Geometry &geometry, FSVolumeType dos);
    void init(Diameter type, Density density, FSVolumeType dos);
        
    // Computed values
    isize numBytes() const { return numBlocks * bsize; }
    
    // Prints debug information
    void dump() const;
    void dump(std::ostream& os) const;
    
    // Throws an exception if the descriptor contains unsupported values
    void checkCompatibility() const;
};
