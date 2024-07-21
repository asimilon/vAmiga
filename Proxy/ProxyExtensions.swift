// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Darwin

//
// Factory extensions
//

extension MediaFileProxy {

    static func makeWith(buffer: UnsafeRawPointer, length: Int, type: FileType) throws -> Self {

        let exc = ExceptionWrapper()
        let obj = make(withBuffer: buffer, length: length, type: type, exception: exc)
        if exc.errorCode != .OK { throw VAError(exc) }
        return obj!
    }

    static func make(with data: Data, type: FileType) throws -> Self {

        let exc = ExceptionWrapper()
        let obj = make(with: data, type: type, exception: exc)
        if exc.errorCode != .OK { throw VAError(exc) }
        return obj!
    }

    private static func make(with data: Data, type: FileType, exception: ExceptionWrapper) -> Self? {

        return data.withUnsafeBytes { uwbp -> Self? in

            return make(withBuffer: uwbp.baseAddress!, length: uwbp.count, type: type, exception: exception)
        }
    }

    static func make(with url: URL) throws -> Self {

        let exc = ExceptionWrapper()
        let obj = make(withFile: url.path, exception: exc)
        if exc.errorCode != .OK { throw VAError(exc) }
        return obj!
    }

    static func make(with url: URL, type: FileType) throws -> Self {

        let exc = ExceptionWrapper()
        let obj = make(withFile: url.path, type: type, exception: exc)
        if exc.errorCode != .OK { throw VAError(exc) }
        return obj!
    }

    static func make(with drive: FloppyDriveProxy, type: FileType) throws -> Self {

        let exc = ExceptionWrapper()
        let obj = make(withDrive: drive, type: type, exception: exc)
        if exc.errorCode != .OK { throw VAError(exc) }
        return obj!
    }

    static func make(with hardDrive: HardDriveProxy, type: FileType) throws -> Self {

        let exc = ExceptionWrapper()
        let obj = make(withHardDrive: hardDrive, type: type, exception: exc)
        if exc.errorCode != .OK { throw VAError(exc) }
        return obj!
    }

    static func make(with fs: FileSystemProxy, type: FileType) throws -> Self {

        let exc = ExceptionWrapper()
        let obj = make(withFileSystem: fs, type: type, exception: exc)
        if exc.errorCode != .OK { throw VAError(exc) }
        return obj!
    }

    func writeToFile(url: URL) throws {

        let exception = ExceptionWrapper()
        write(toFile: url.path, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func writeToFile(url: URL, partition: Int) throws {

        let exception = ExceptionWrapper()
        write(toFile: url.path, partition: partition, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    var fileTypeInfo: String {

        switch type {

        case .ADF:  return "Amiga Floppy Disk"
        case .EADF: return "Amiga Floppy Disk (Ext)"
        case .IMG:  return "PC Disk"
        case .HDF:  return "Amiga Hard Drive"
        default:    return ""
        }
    }

    var typeInfo: String {

        var result = ""
        let info = floppyDiskInfo

        if info.diameter == .INCH_35 { result += "3.5\"" }
        if info.diameter == .INCH_525 { result += "5.25\"" }

        return result
    }

    var layoutInfo: String {

        var result = ""
        let info = diskInfo
        let floppyInfo = floppyDiskInfo

        if info.heads == 1 { result += "Single sided, " }
        if info.heads == 2 { result += "Double sided, " }
        if floppyInfo.density == .SD { result += "Single density" }
        if floppyInfo.density == .DD { result += "Double density" }
        if floppyInfo.density == .HD { result += "High density" }

        return result
    }

    var bootInfo: String {

        let info = floppyDiskInfo
        let name = String(cString: info.bootBlockName)

        if info.bootBlockType == .VIRUS {
            return "Contagious boot block (\(name))"
        } else {
            return name
        }
    }

    func icon(protected: Bool = false) -> NSImage {

        var name = ""

        switch type {

        case .ADF, .EADF, .IMG:

            let info = floppyDiskInfo
            name = (info.density == .HD ? "hd" : "dd") +
            (type == .IMG ? "_dos" : info.dos == .NODOS ? "_other" : "_adf")

        case .HDF:

            name = "hdf"

        default:

            name = ""
        }

        if protected { name += "_protected" }
        return NSImage(named: name)!
    }
}

extension MakeWithBuffer {
    
    static func makeWith(buffer: UnsafeRawPointer, length: Int) throws -> Self {
                
        let exc = ExceptionWrapper()
        let obj = make(withBuffer: buffer, length: length, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }

    static func make(with data: Data) throws -> Self {
        
        let exception = ExceptionWrapper()
        let obj = make(with: data, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
        return obj!
    }

    private static func make(with data: Data, exception: ExceptionWrapper) -> Self? {
        
        return data.withUnsafeBytes { uwbp -> Self? in
            
            return make(withBuffer: uwbp.baseAddress!, length: uwbp.count, exception: exception)
        }
    }
}

extension MakeWithFile {
    
    static func make(with url: URL) throws -> Self {
        
        let exc = ExceptionWrapper()
        let obj = make(withFile: url.path, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }
}

extension MakeWithDrive {
    
    static func make(with drive: FloppyDriveProxy) throws -> Self {
        
        let exc = ExceptionWrapper()
        let obj = make(withDrive: drive, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }
}

extension MakeWithHardDrive {
    
    static func make(with hdr: HardDriveProxy) throws -> Self {
        
        let exc = ExceptionWrapper()
        let obj = make(withHardDrive: hdr, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }
}

extension MakeWithFileSystem {
    
    static func make(with fs: FileSystemProxy) throws -> Self {
        
        let exc = ExceptionWrapper()
        let obj = make(withFileSystem: fs, exception: exc)
        if exc.errorCode != ErrorCode.OK { throw VAError(exc) }
        return obj!
    }
}

//
// Exception passing
//

extension EmulatorProxy {

    func isReady() throws {
        
        let exception = ExceptionWrapper()
        isReady(exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func run() throws {
        
        let exception = ExceptionWrapper()
        run(exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func exportConfig(url: URL) throws {

        let exception = ExceptionWrapper()
        exportConfig(url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func loadSnapshot(_ proxy: MediaFileProxy) throws {

        let exception = ExceptionWrapper()
        loadSnapshot(proxy, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

extension MemProxy {
 
    func loadRom(_ proxy: MediaFileProxy) throws {

        let exception = ExceptionWrapper()
        loadRom(proxy, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func loadRom(buffer: Data) throws {

        let exception = ExceptionWrapper()
        loadRom(fromBuffer: buffer, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func loadRom(_ url: URL) throws {

        let exception = ExceptionWrapper()
        loadRom(fromFile: url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func loadExt(_ proxy: MediaFileProxy) throws {

        let exception = ExceptionWrapper()
        loadExt(proxy, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func loadExt(buffer: Data) throws {

        let exception = ExceptionWrapper()
        loadExt(fromBuffer: buffer, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
    
    func loadExt(_ url: URL) throws {

        let exception = ExceptionWrapper()
        loadExt(fromFile: url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func saveRom(_ url: URL) throws {

        let exception = ExceptionWrapper()
        saveRom(url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func saveWom(_ url: URL) throws {

        let exception = ExceptionWrapper()
        saveWom(url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func saveExt(_ url: URL) throws {

        let exception = ExceptionWrapper()
        saveExt(url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

extension FloppyDriveProxy {

    func swap(file: MediaFileProxy) throws {

        let exception = ExceptionWrapper()
        insertMedia(file, protected: false, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func insertNew(fileSystem: FSVolumeType, bootBlock: BootBlockId, name: String) throws {
        
        let exception = ExceptionWrapper()
        insertBlankDisk(fileSystem, bootBlock: bootBlock, name: name, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func exportDisk(type: FileType) throws -> MediaFileProxy? {

        let exception = ExceptionWrapper()
        let result = exportDisk(type, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }

        return result;
    }
}

extension HardDriveProxy {

    func attach(url: URL) throws {

        let exception = ExceptionWrapper()
        attachFile(url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func attach(file: MediaFileProxy) throws {

        let exception = ExceptionWrapper()
        attach(file, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func attach(c: Int, h: Int, s: Int, b: Int) throws {

        let exception = ExceptionWrapper()
        attach(c, h: h, s: s, b: b, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func format(fs: FSVolumeType, name: String) throws {

        let exception = ExceptionWrapper()
        format(fs, name: name, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func changeGeometry(c: Int, h: Int, s: Int, b: Int = 512) throws {

        let exception = ExceptionWrapper()
        changeGeometry(c, h: h, s: s, b: b, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func writeToFile(_ url: URL) throws {

        let exception = ExceptionWrapper()
        write(toFile: url, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }

    func enableWriteThrough() throws {

        let exception = ExceptionWrapper()
        enableWriteThrough(exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

extension AmigaFileProxy {
    
    @discardableResult
    func writeToFile(url: URL) throws -> Int {
        
        let exception = ExceptionWrapper()
        let result = write(toFile: url.path, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
        
        return result
    }
}

extension FileSystemProxy {

    static func make(with file: MediaFileProxy, partition: Int = 0) throws -> FileSystemProxy {

        let exception = ExceptionWrapper()
        let result = FileSystemProxy.make(withMedia: file, partition: partition, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }

        return result!
    }

    func export(url: URL) throws {
            
        let exception = ExceptionWrapper()
        export(url.path, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

extension RecorderProxy {
    
    func startRecording(rect: NSRect, rate: Int, ax: Int, ay: Int) throws {
        
        let exception = ExceptionWrapper()
        startRecording(rect, bitRate: rate, aspectX: ax, aspectY: ay, exception: exception)
        if exception.errorCode != .OK { throw VAError(exception) }
    }
}

//
// Other extensions
//

public extension EmulatorProxy {
    
    func df(_ nr: Int) -> FloppyDriveProxy? {
        
        switch nr {
            
        case 0: return df0
        case 1: return df1
        case 2: return df2
        case 3: return df3
            
        default:
            return nil
        }
    }

    func df(_ item: NSButton!) -> FloppyDriveProxy? { return df(item.tag) }
    func df(_ item: NSMenuItem!) -> FloppyDriveProxy? { return df(item.tag) }

    func hd(_ nr: Int) -> HardDriveProxy? {
        
        switch nr {

        case 0: return hd0
        case 1: return hd1
        case 2: return hd2
        case 3: return hd3

        default:
            return nil
        }
    }

    func hd(_ item: NSButton!) -> HardDriveProxy? { return hd(item.tag) }
    func hd(_ item: NSMenuItem!) -> HardDriveProxy? { return hd(item.tag) }
    
    func image(data: UnsafeMutablePointer<UInt8>?, size: NSSize) -> NSImage {
        
        var bitmap = data
        let width = Int(size.width)
        let height = Int(size.height)
                
        let imageRep = NSBitmapImageRep(bitmapDataPlanes: &bitmap,
                                        pixelsWide: width,
                                        pixelsHigh: height,
                                        bitsPerSample: 8,
                                        samplesPerPixel: 4,
                                        hasAlpha: true,
                                        isPlanar: false,
                                        colorSpaceName: NSColorSpaceName.calibratedRGB,
                                        bytesPerRow: 4 * width,
                                        bitsPerPixel: 32)
        
        let image = NSImage(size: (imageRep?.size)!)
        image.addRepresentation(imageRep!)
        // image.makeGlossy()
        
        return image
    }
}

extension FloppyDriveProxy {
    
    var templateIcon: NSImage? {

        let info = info
        var name: String

        if !info.hasDisk { return nil }

        if info.hasProtectedDisk {
            name = info.hasModifiedDisk ? "diskUPTemplate" : "diskPTemplate"
        } else {
            name = info.hasModifiedDisk ? "diskUTemplate" : "diskTemplate"
        }
        
        return NSImage(named: name)!
    }
    
    var toolTip: String? {
        
        return nil
    }
    
    var ledIcon: NSImage? {
        
        let info = info

        if !info.isConnected { return nil }

        if info.motor {
            if info.writing {
                return NSImage(named: "ledRed")
            } else {
                return NSImage(named: "ledGreen")
            }
        } else {
            return NSImage(named: "ledGrey")
        }
    }
}

extension HardDriveProxy {
    
    var templateIcon: NSImage? {
        
        var name: String
                
        switch controller.info.state {

        case .UNDETECTED, .INITIALIZING:
            name = "hdrETemplate"
            
        default:
            name = info.hasModifiedDisk ? "hdrUTemplate" : "hdrTemplate"
        }
        
        return NSImage(named: name)!
    }
    
    var toolTip: String? {
        
        switch controller.info.state {
            
        case .UNDETECTED:
            return "The hard drive is waiting to be initialized by the OS."
            
        case .INITIALIZING:
            return "The OS has started to initialize the hard drive. If the " +
            "condition persists the hard drive is not valid or incompatible " +
            "with the chosen setup."
            
        default:
            return nil
        }
    }
    
    func ledIcon(info: HardDriveInfo) -> NSImage? {

        if !info.isConnected { return nil }

        switch info.state {
            
        case .IDLE: return NSImage(named: "ledGrey")
        case .READING: return NSImage(named: "ledGreen")
        case .WRITING: return NSImage(named: "ledRed")
            
        default: fatalError()
        }
    }
}

public extension RemoteManagerProxy {
    
    var icon: NSImage? {

        let info = info

        if info.numConnected > 0 {
            return NSImage(named: "srvConnectTemplate")!
        }
        if info.numListening > 0 {
            return NSImage(named: "srvListenTemplate")!
        }
        if info.numLaunching > 0 {
            return NSImage(named: "srvLaunchTemplate")!
        }
        if info.numErroneous > 0 {
            return NSImage(named: "srvErrorTemplate")!
        }

        return nil
    }
}
