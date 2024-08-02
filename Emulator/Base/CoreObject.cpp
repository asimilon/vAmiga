// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the Mozilla Public License v2
//
// See https://mozilla.org/MPL/2.0 for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CoreObject.h"
#include <iostream>

namespace vamiga {

bool
CoreObject::verbose = true;

void
CoreObject::prefix(bool verbose) const
{
    fprintf(stderr, "%s: ", objectName());
}

}
