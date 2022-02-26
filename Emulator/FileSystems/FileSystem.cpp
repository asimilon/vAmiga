// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "IOUtils.h"
#include "MutableFileSystem.h"
#include "MemUtils.h"
#include <climits>
#include <set>
#include <stack>

void
FileSystem::init(const HDFFile &hdf, isize part)
{
    printf("Getting layout for partition %ld\n", part);

    // Get a file system descriptor from the HDF
    auto descriptor = hdf.getFileSystemDescriptor(part);
    
    // descriptor.dump();
    
    printf("Done\n");

    // Only proceed if the HDF is formatted
    if (descriptor.dos == FS_NODOS) throw VAError(ERROR_HDR_UNPARTITIONED);
    
    // Create the device
    // init(descriptor);

    // Import file system from HDF
    auto *ptr = hdf.dataForPartition(part);
    auto diff = ptr - hdf.data;
    printf("Skipping %ld.%ld blocks\n", diff / 512, diff % 512);
    
    init(descriptor, ptr, descriptor.numBlocks * 512); }

void
FileSystem::init(FileSystemDescriptor layout, u8 *buf, isize len)
{
    assert(buf);

    debug(FS_DEBUG, "Importing file system...\n");

    // Copy layout parameters
    dos         = layout.dos;
    numReserved = layout.numReserved;
    rootBlock   = layout.rootBlock;
    bmBlocks    = layout.bmBlocks;
    bmExtBlocks = layout.bmExtBlocks;
    
    // Create all blocks
    assert(blocks.empty());
    for (isize i = 0; i < layout.numBlocks; i++) {
        
        const u8 *data = buf + i * bsize;
                
        // Determine the type of the new block
        FSBlockType type = predictBlockType((Block)i, data);
        
        // Create new block
        blocks.push_back(FSBlock::make(*this, (Block)i, type));

        // Import block data
        blocks[i]->importBlock(data, bsize);
    }
    
    // Print some debug information
    debug(FS_DEBUG, "Success\n");
    // info();
    // dump();
    // util::hexdump(blocks[0]->data, 512);
    printDirectory(true);
}

void
FileSystem::_dump(dump::Category category, std::ostream& os) const
{
    using namespace util;

    if (category & dump::Summary) {
        
        auto total = numBlocks();
        auto used = usedBlocks();
        auto free = freeBlocks();
        auto fill = (isize)(100.0 * used / total);
        
        os << "DOS" << dec(dos);
        os << "   ";
        os << std::setw(6) << std::left << std::setfill(' ') << total;
        os << " (x ";
        os << std::setw(3) << std::left << std::setfill(' ') << bsize;
        os << ")  ";
        os << std::setw(6) << std::left << std::setfill(' ') << used;
        os << "  ";
        os << std::setw(6) << std::left << std::setfill(' ') << free;
        os << "  ";
        os << std::setw(3) << std::right << std::setfill(' ') << fill;
        os << "%  ";
        os << getName().c_str() << std::endl;
    }
    
    if (category & dump::Partitions) {
        
        os << tab("Root block");
        os << dec(rootBlock) << std::endl;
        os << tab("Bitmap blocks");
        for (auto& it : bmBlocks) { os << dec(it) << " "; }
        os << std::endl;
        os << util::tab("Extension blocks");
        for (auto& it : bmExtBlocks) { os << dec(it) << " "; }
        os << std::endl;
    }

    if (category & dump::Blocks) {
                
        for (isize i = 0; i < numBlocks(); i++)  {
            
            if (blocks[i]->type == FS_EMPTY_BLOCK) continue;
            
            msg("\nBlock %ld (%d):", i, blocks[i]->nr);
            msg(" %s\n", FSBlockTypeEnum::key(blocks[i]->type));
            
            blocks[i]->dump();
        }
    }
}

isize
FileSystem::freeBlocks() const
{
    isize result = 0;
    
    for (isize i = 0; i < numBlocks(); i++) {
        if (isFree((Block)i)) result++;
    }

    return result;
}

isize
FileSystem::usedBlocks() const
{
    return numBlocks() - freeBlocks();
}

isize
FileSystem::freeBytes() const
{
    return freeBlocks() * bsize;
}

isize
FileSystem::usedBytes() const
{
    return usedBlocks() * bsize;
}

FSName
FileSystem::getName() const
{
    FSBlock *rb = rootBlockPtr(rootBlock);
    return rb ? rb->getName() : FSName("");
}

FSBlockType
FileSystem::blockType(Block nr) const
{
    return blockPtr(nr) ? blocks[nr]->type : FS_UNKNOWN_BLOCK;
}

FSItemType
FileSystem::itemType(Block nr, isize pos) const
{
    return blockPtr(nr) ? blocks[nr]->itemType(pos) : FSI_UNUSED;
}

FSBlock *
FileSystem::blockPtr(Block nr) const
{
    return nr < blocks.size() ? blocks[nr] : nullptr;
}

FSBlock *
FileSystem::bootBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BOOT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::rootBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_ROOT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::bitmapBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BITMAP_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::bitmapExtBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_BITMAP_EXT_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::userDirBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_USERDIR_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::fileHeaderBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_FILEHEADER_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::fileListBlockPtr(Block nr) const
{
    if (nr < blocks.size() && blocks[nr]->type == FS_FILELIST_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::dataBlockPtr(Block nr) const
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type : FS_UNKNOWN_BLOCK;

    if (t == FS_DATA_BLOCK_OFS || t == FS_DATA_BLOCK_FFS) {
        return blocks[nr];
    }
    return nullptr;
}

FSBlock *
FileSystem::hashableBlockPtr(Block nr) const
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type : FS_UNKNOWN_BLOCK;
    
    if (t == FS_USERDIR_BLOCK || t == FS_FILEHEADER_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

u8
FileSystem::readByte(Block nr, isize offset) const
{
    assert(offset < bsize);

    if (isize(nr) < numBlocks()) {
        return blocks[nr]->data ? blocks[nr]->data[offset] : 0;
    }
    
    return 0;
}

bool
FileSystem::isFree(Block nr) const
{
    assert(isBlockNumber(nr));

    // The first two blocks are always allocated and not part of the bitmap
    if (nr < 2) return false;
    
    // Locate the allocation bit in the bitmap block
    isize byte, bit;
    FSBlock *bm = locateAllocationBit(nr, &byte, &bit);
        
    // Read the bit
    return bm ? GET_BIT(bm->data[byte], bit) : false;
}

FSBlock *
FileSystem::locateAllocationBit(Block nr, isize *byte, isize *bit) const
{
    assert(isBlockNumber(nr));

    // The first two blocks are always allocated and not part of the map
    if (nr < 2) return nullptr;
    nr -= 2;
    
    // Locate the bitmap block which stores the allocation bit
    isize bitsPerBlock = (bsize - 4) * 8;
    isize bmNr = nr / bitsPerBlock;

    // Get the bitmap block
    FSBlock *bm;
    bm = (bmNr < (isize)bmBlocks.size()) ? bitmapBlockPtr(bmBlocks[bmNr]) : nullptr;
    if (bm == nullptr) {
        warn("Failed to lookup allocation bit for block %d\n", nr);
        warn("bmNr = %ld\n", bmNr);
        return nullptr;
    }
    
    // Locate the byte position (note: the long word ordering will be reversed)
    nr = nr % bitsPerBlock;
    isize rByte = nr / 8;
    
    // Rectifiy the ordering
    switch (rByte % 4) {
        case 0: rByte += 3; break;
        case 1: rByte += 1; break;
        case 2: rByte -= 1; break;
        case 3: rByte -= 3; break;
    }

    // Skip the checksum which is located in the first four bytes
    rByte += 4;
    assert(rByte >= 4 && rByte < bsize);
    
    *byte = rByte;
    *bit = nr % 8;
    
    // debug(FS_DEBUG, "Alloc bit for %d: block: %d byte: %d bit: %d\n",
    //       ref, bm->nr, *byte, *bit);

    return bm;
}

FSBlock *
FileSystem::currentDirBlock()
{
    FSBlock *cdb = blockPtr(cd);
    
    if (cdb) {
        if (cdb->type == FS_ROOT_BLOCK || cdb->type == FS_USERDIR_BLOCK) {
            return cdb;
        }
    }
    
    // The block reference is invalid. Switch back to the root directory
    cd = rootBlock;
    return blockPtr(cd);
}

FSBlock *
FileSystem::changeDir(const string &name)
{
    FSBlock *cdb = currentDirBlock();

    if (name == "/") {
                
        // Move to top level
        cd = rootBlock;
        return currentDirBlock();
    }

    if (name == "..") {
                
        // Move one level up
        cd = cdb->getParentDirRef();
        return currentDirBlock();
    }
    
    FSBlock *subdir = seekDir(name);
    if (subdir == nullptr) return cdb;
    
    // Move one level down
    cd = subdir->nr;
    return currentDirBlock();
}

void
FileSystem::printDirectory(bool recursive)
{
    std::vector<Block> items;
    collect(cd, items);
    
    for (auto const& i : items) {
        msg("%s\n", getPath(i).c_str());
    }
    msg("%zu items\n", items.size());
}

string
FileSystem::getPath(FSBlock *block)
{
    string result = "";
    std::set<Block> visited;
 
    while(block) {

        // Break the loop if this block has an invalid type
        if (!hashableBlockPtr(block->nr)) break;

        // Break the loop if this block was visited before
        if (visited.find(block->nr) != visited.end()) break;
        
        // Add the block to the set of visited blocks
        visited.insert(block->nr);
                
        // Expand the path
        string name = block->getName().c_str();
        result = (result == "") ? name : name + "/" + result;
        
        // Continue with the parent block
        block = block->getParentDirBlock();
    }
    
    return result;
}

Block
FileSystem::seekRef(FSName name)
{
    std::set<Block> visited;
    
    // Only proceed if a hash table is present
    FSBlock *cdb = currentDirBlock();
    if (!cdb || cdb->hashTableSize() == 0) return 0;
    
    // Compute the table position and read the item
    u32 hash = name.hashValue() % cdb->hashTableSize();
    u32 ref = cdb->getHashRef(hash);
    
    // Traverse the linked list until the item has been found
    while (ref && visited.find(ref) == visited.end())  {
        
        FSBlock *item = hashableBlockPtr(ref);
        if (item == nullptr) break;
        
        if (item->isNamed(name)) return item->nr;

        visited.insert(ref);
        ref = item->getNextHashRef();
    }

    return 0;
}

void
FileSystem::collect(Block nr, std::vector<Block> &result, bool recursive)
{
    std::stack<Block> remainingItems;
    std::set<Block> visited;
    
    // Start with the items in this block
    collectHashedRefs(nr, remainingItems, visited);
    
    // Move the collected items to the result list
    while (remainingItems.size() > 0) {
        
        Block item = remainingItems.top();
        remainingItems.pop();
        result.push_back(item);

        // Add subdirectory items to the queue
        if (userDirBlockPtr(item) && recursive) {
            collectHashedRefs(item, remainingItems, visited);
        }
    }
}

void
FileSystem::collectHashedRefs(Block nr,
                            std::stack<Block> &result, std::set<Block> &visited)
{
    if (FSBlock *b = blockPtr(nr)) {
        
        // Walk through the hash table in reverse order
        for (isize i = (isize)b->hashTableSize(); i >= 0; i--) {
            collectRefsWithSameHashValue(b->getHashRef((u32)i), result, visited);
        }
    }
}

void
FileSystem::collectRefsWithSameHashValue(Block nr,
                                         std::stack<Block> &result, std::set<Block> &visited)
{
    std::stack<Block> refs;
    
    // Walk down the linked list
    for (FSBlock *b = hashableBlockPtr(nr); b; b = b->getNextHashBlock()) {

        // Only proceed if we haven't seen this block yet
        if (visited.find(b->nr) != visited.end()) throw VAError(ERROR_FS_HAS_CYCLES);

        visited.insert(b->nr);
        refs.push(b->nr);
    }
  
    // Push the collected elements onto the result stack
    while (refs.size() > 0) { result.push(refs.top()); refs.pop(); }
}

FSBlock *
FileSystem::lastFileListBlockInChain(Block start)
{
    FSBlock *block = fileListBlockPtr(start);
    return block ? lastFileListBlockInChain(block) : nullptr;
}

FSBlock *
FileSystem::lastFileListBlockInChain(FSBlock *block)
{
    std::set<Block> visited;

    while (block && visited.find(block->nr) == visited.end()) {

        FSBlock *next = block->getNextListBlock();
        if (next == nullptr) return block;

        visited.insert(block->nr);
        block = next;
    }
    return nullptr;
}

FSBlock *
FileSystem::lastHashBlockInChain(Block start)
{
    FSBlock *block = hashableBlockPtr(start);
    return block ? lastHashBlockInChain(block) : nullptr;
}

FSBlock *
FileSystem::lastHashBlockInChain(FSBlock *block)
{
    std::set<Block> visited;

    while (block && visited.find(block->nr) == visited.end()) {

        FSBlock *next = block->getNextHashBlock();
        if (next == nullptr) return block;

        visited.insert(block->nr);
        block =next;
    }
    return nullptr;
}

FSErrorReport
FileSystem::check(bool strict) const
{
    FSErrorReport result = { };

    isize total = 0, min = INT_MAX, max = 0;
    
    // Analyze the allocation table
    for (Block i = 0; i < numBlocks(); i++) {

        FSBlock *block = blocks[i];
        if (block->type == FS_EMPTY_BLOCK && !isFree((Block)i)) {
            result.bitmapErrors++;
            debug(FS_DEBUG, "Empty block %d is marked as allocated\n", i);
        }
        if (block->type != FS_EMPTY_BLOCK && isFree((Block)i)) {
            result.bitmapErrors++;
            debug(FS_DEBUG, "Non-empty block %d is marked as free\n", i);
        }
    }

    // Analyze all blocks
    for (isize i = 0; i < numBlocks(); i++) {

        if (blocks[i]->check(strict) > 0) {
            min = std::min(min, i);
            max = std::max(max, i);
            blocks[i]->corrupted = ++total;
        } else {
            blocks[i]->corrupted = 0;
        }
    }

    // Record findings
    if (total) {
        result.corruptedBlocks = total;
        result.firstErrorBlock = min;
        result.lastErrorBlock = max;
    } else {
        result.corruptedBlocks = 0;
        result.firstErrorBlock = min;
        result.lastErrorBlock = max;
    }
    
    return result;
}

ErrorCode
FileSystem::check(Block nr, isize pos, u8 *expected, bool strict) const
{
    return blocks[nr]->check(pos, expected, strict);
}

ErrorCode
FileSystem::checkBlockType(Block nr, FSBlockType type) const
{
    return checkBlockType(nr, type, type);
}

ErrorCode
FileSystem::checkBlockType(Block nr, FSBlockType type, FSBlockType altType) const
{
    FSBlockType t = blockType(nr);
    
    if (t != type && t != altType) {
        
        switch (t) {
                
            case FS_EMPTY_BLOCK:      return ERROR_FS_PTR_TO_EMPTY_BLOCK;
            case FS_BOOT_BLOCK:       return ERROR_FS_PTR_TO_BOOT_BLOCK;
            case FS_ROOT_BLOCK:       return ERROR_FS_PTR_TO_ROOT_BLOCK;
            case FS_BITMAP_BLOCK:     return ERROR_FS_PTR_TO_BITMAP_BLOCK;
            case FS_BITMAP_EXT_BLOCK: return ERROR_FS_PTR_TO_BITMAP_EXT_BLOCK;
            case FS_USERDIR_BLOCK:    return ERROR_FS_PTR_TO_USERDIR_BLOCK;
            case FS_FILEHEADER_BLOCK: return ERROR_FS_PTR_TO_FILEHEADER_BLOCK;
            case FS_FILELIST_BLOCK:   return ERROR_FS_PTR_TO_FILELIST_BLOCK;
            case FS_DATA_BLOCK_OFS:   return ERROR_FS_PTR_TO_DATA_BLOCK;
            case FS_DATA_BLOCK_FFS:   return ERROR_FS_PTR_TO_DATA_BLOCK;
            default:                  return ERROR_FS_PTR_TO_UNKNOWN_BLOCK;
        }
    }

    return ERROR_OK;
}

isize
FileSystem::getCorrupted(Block nr)
{
    return blockPtr(nr) ? blocks[nr]->corrupted : 0;
}

bool
FileSystem::isCorrupted(Block nr, isize n)
{
    for (isize i = 0, cnt = 0; i < numBlocks(); i++) {
        
        if (isCorrupted((Block)i)) {
            cnt++;
            if ((i64)nr == i) return cnt == n;
        }
    }
    return false;
}

Block
FileSystem::nextCorrupted(Block nr)
{
    isize i = (isize)nr;
    while (++i < numBlocks()) { if (isCorrupted((Block)i)) return (Block)i; }
    return nr;
}

Block
FileSystem::prevCorrupted(Block nr)
{
    isize i = (isize)nr - 1;
    while (i-- >= 0) { if (isCorrupted((Block)i)) return (Block)i; }
    return nr;
}

Block
FileSystem::seekCorruptedBlock(isize n)
{
    for (isize i = 0, cnt = 0; i < numBlocks(); i++) {

        if (isCorrupted((Block)i)) {
            cnt++;
            if (cnt == n) return (Block)i;
        }
    }
    return (Block)-1;
}

FSBlockType
FileSystem::predictBlockType(Block nr, const u8 *buffer)
{
    assert(buffer != nullptr);
    
    // Is it a boot block?
    if (nr == 0 || nr == 1) return FS_BOOT_BLOCK;
    
    // Is it a bitmap block?
    if (std::find(bmBlocks.begin(), bmBlocks.end(), nr) != bmBlocks.end())
        return FS_BITMAP_BLOCK;
    
    // is it a bitmap extension block?
    if (std::find(bmExtBlocks.begin(), bmExtBlocks.end(), nr) != bmExtBlocks.end())
        return FS_BITMAP_EXT_BLOCK;

    // For all other blocks, check the type and subtype fields
    u32 type = FSBlock::read32(buffer);
    u32 subtype = FSBlock::read32(buffer + bsize - 4);

    if (type == 2  && subtype == 1)       return FS_ROOT_BLOCK;
    if (type == 2  && subtype == 2)       return FS_USERDIR_BLOCK;
    if (type == 2  && subtype == (u32)-3) return FS_FILEHEADER_BLOCK;
    if (type == 16 && subtype == (u32)-3) return FS_FILELIST_BLOCK;

    // Check if this block is a data block
    if (isOFS()) {
        if (type == 8) return FS_DATA_BLOCK_OFS;
    } else {
        for (isize i = 0; i < bsize; i++) if (buffer[i]) return FS_DATA_BLOCK_FFS;
    }
    
    return FS_EMPTY_BLOCK;
}