// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
DiskFile::readSector(u8 *dst, long t, long s)
{
    readSector(dst, t * numSectorsPerTrack() + s);
}

void
DiskFile::readSector(u8 *dst, long s)
{
    long sectorSize = 512;
    long offset = s * sectorSize;

    assert(dst != NULL);
    assert(offset + sectorSize <= size);

    for (unsigned i = 0; i < 512; i++) {
        dst[i] = data[offset + i];
    }
}
