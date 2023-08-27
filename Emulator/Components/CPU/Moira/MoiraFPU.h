// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraTypes.h"
#include "FpuFormats.h"

namespace vamiga::moira {

class FPUReg {

    // Reference to the FPU
    class FPU &fpu;

public:

    // Register value
    FpuExtended val;


    //
    // Constructing
    //

    FPUReg(FPU& fpu) : fpu(fpu) { }
    void reset() { val = FpuExtended::NaN(); }


    //
    // Getting and setting
    //

    FpuExtended asExtended();
    FpuExtended get() const { return val; }
    void set(const FpuExtended other);


    //
    // Classifying
    //

    bool isNegative() const { return val.isNegative(); }
    bool isZero() const  { return val.isZero(); }
    bool isInfinity() const { return val.isInfinity(); }
    bool isNaN() const { return val.isNaN(); }
    bool isSignalingNaN() const { return val.isSignalingNaN(); }
    bool isNonsignalingNaN() const { return val.isNonsignalingNaN(); }
    bool isNormalized() const { return val.isNormalized(); }
};

class FPU {
    
    // Reference to the CPU
    class Moira &moira;

    // Emulated FPU model
    FPUModel model = FPU_NONE;

public:

    // Registers
    FPUReg fpr[8] = {
        FPUReg(*this), FPUReg(*this), FPUReg(*this), FPUReg(*this),
        FPUReg(*this), FPUReg(*this), FPUReg(*this), FPUReg(*this)
    };

    u32 fpiar;
    u32 fpsr;
    u32 fpcr;

    // Experimental
    int oldRoundingMode;


    //
    // Constructing
    //

public:

    FPU(Moira& ref);
    void reset();


    //
    // Configuring
    //

public:

    // Selects the emulated CPU model
    void setModel(FPUModel model);
    FPUModel getModel() const { return model; }

    // Returns the precision and rounding mode, as specified in the FPCR
    FpuPrecision getPrecision() const;
    FpuRoundingMode getRoundingMode() const;

    // Configures the rounding mode of the host FPU
    static FpuRoundingMode fesetround(FpuRoundingMode mode);


    //
    // Accessing registers
    //

public:

    // Accesses the control register
    u32 getFPCR() const { return fpcr & 0x0000FFF0; }
    void setFPCR(u32 value);

    // Accesses the status register
    void clearFPSR() { fpsr &= 0xFFFF00F8; }
    u32 getFPSR() const { return fpsr & 0x0FFFFFF8; }
    void setFPSR(u32 value);

    void setExcStatusBit(u32 mask);
    void clearExcStatusBit(u32 mask);

    void setConditionCodes(int reg);
    void setConditionCodes(const FpuExtended &value);

    // Accesses the instruction address register
    u32 getFPIAR() const { return fpiar; }
    void setFPIAR(u32 value);


    //
    // Accessing the constant Rom
    //

    // Reads a value from the constant Rom
    FpuExtended readCR(unsigned nr);

    
    //
    // Analyzing instructions
    //

public:

    // Checks the validity of the extension words
    bool isValidExt(Instr I, Mode M, u16 op, u32 ext) const;


    //
    // Executing instructions
    //

    FpuExtended fsin(const FpuExtended &value); 

};

}
