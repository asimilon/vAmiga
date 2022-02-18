// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HardDrive.h"
#include "IOUtils.h"

HardDrive::HardDrive(Amiga& ref, isize n) : SubComponent(ref), nr(n)
{
    assert(usize(nr) < 4);
}

HardDrive::~HardDrive()
{
    dealloc();
}

void
HardDrive::alloc(const DiskGeometry &geometry)
{
    // Save disk geometry
    this->geometry = geometry;
    
    // Delete previously allocated memory (if any)
    if (data) delete [] data;

    // Allocate memory
    data = new u8[geometry.numBytes()];
}

void
HardDrive::dealloc()
{
    // Wipe out disk data
    if (data) delete [] data;
    data = nullptr;

    // Wipe out geometry information
    geometry = DiskGeometry();
}

const char *
HardDrive::getDescription() const
{
    assert(usize(nr) < 4);
    return nr == 0 ? "Dh0" : nr == 1 ? "Dh1" : nr == 2 ? "Dh2" : "Dh3";
}

void
HardDrive::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

void
HardDrive::_inspect() const
{
    {   SYNCHRONIZED
        
        info.attached = isAttached();
        info.cylinder = currentCylinder;
    }
}

void
HardDrive::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;
        
    if (category & dump::State) {
        
        os << tab("Nr");
        os << dec(nr) << std::endl;
        os << tab("Capacity");
        os << dec(geometry.numBytes() / MB(1)) << " MB" << std::endl;
        os << tab("Cylinders");
        os << dec(geometry.cylinders) << std::endl;
        os << tab("Head");
        os << dec(geometry.heads) << std::endl;
        os << tab("Sectors");
        os << dec(geometry.sectors) << std::endl;
        os << tab("Block size");
        os << dec(geometry.bsize) << std::endl;
    }
}

isize
HardDrive::_size()
{
    util::SerCounter counter;

    applyToPersistentItems(counter);
    applyToResetItems(counter);
    
    // Add the disk size
    counter.count += geometry.numBytes();
    
    return counter.count;
}

isize
HardDrive::didLoadFromBuffer(const u8 *buffer)
{
    util::SerReader reader(buffer);

    // Allocate memory for storing the disk data
    alloc(geometry);
    
    // Load disk data
    reader.copy(data, geometry.numBytes());

    return (isize)(reader.ptr - buffer);
}

isize
HardDrive::didSaveToBuffer(u8 *buffer) const
{
    util::SerWriter writer(buffer);

    // Save memory contents
    writer.copy(data, geometry.numBytes());
        
    return (isize)(writer.ptr - buffer);
}

void
HardDrive::format(FSVolumeType fsType, BootBlockId bb)
{
    debug(HDR_DEBUG, "Formatting drive\n");
    debug(HDR_DEBUG, "    File system: %s\n", FSVolumeTypeEnum::key(fsType));
    debug(HDR_DEBUG, "     Boot block: %s\n", BootBlockIdEnum::key(bb));

    // Only proceed if a disk is present
    if (!data) return;

    if (fsType != FS_NODOS) {
        
        // Create a device descriptor matching this drive
        auto layout = FSDeviceDescriptor(geometry, fsType);
        
        // Create a file system
        auto fs = FSDevice(layout);
        
        // Add a boot block
        fs.makeBootable(bb);
        
        // REMOVE ASAP
        msg("File system\n");
        fs.dump();
        
        // Copy all blocks over
        fs.exportVolume(data, geometry.numBytes());
    }
}

void
HardDrive::attach(const DiskGeometry &geometry)
{
    debug(HDR_DEBUG, "Attaching new hard drive\n");

    // Throw an exception if the geometry is not supported
    checkCompatibility(geometry);
    
    // Trash the old disk
    dealloc();
    
    // Create new disk
    alloc(geometry);

    // Remove asap
    msg("Hard drive attached successfully\n");
    dump();
}

void
HardDrive::attach(const FSDevice &fs)
{
    auto geometry = fs.getGeometry();
    
    // Throw an exception if the geometry is not supported
    checkCompatibility(geometry);

    // Allocate memory
    alloc(geometry);
    
    // Copy all blocks over
    fs.exportVolume(data, geometry.numBytes());
}

void
HardDrive::attach(const HDFFile &hdf)
{
    auto geometry = hdf.getGeometry();
    
    // Throw an exception if the geometry is not supported
    checkCompatibility(geometry);

    // Allocate memory
    alloc(geometry);
    
    // Copy all blocks over
    hdf.flash(data);
}

void
HardDrive::checkCompatibility(const DiskGeometry &geometry)
{
    if (geometry.numBytes() > MAX_HDF_SIZE || FORCE_HDR_TOO_LARGE) {
        
        throw VAError(ERROR_HDR_TOO_LARGE);
    }
    if (geometry.bsize != 512 || FORCE_HDR_UNSUPPORTED_BSIZE) {
        
        throw VAError(ERROR_HDR_UNSUPPORTED_BSIZE);
    }
}

void
HardDrive::checkCompatibility(const HDFFile &hdf)
{
    // TODO
}