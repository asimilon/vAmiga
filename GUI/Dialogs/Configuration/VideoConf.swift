// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension ConfigurationController {
    
    func awakeVideoPrefsFromNib() {

        // Check for available enhancers
        let enhancers = parent.renderer.ressourceManager.enhancerGallery
        for i in 0 ..< enhancers.count {
            if let item = vidEnhancerPopUp.menu?.item(withTag: i) {
                item.isEnabled = (enhancers[i] != nil)
            }
        }

        // Check for available upscalers
        let upscalers = parent.renderer.ressourceManager.upscalerGallery
        for i in 0 ..< upscalers.count {
            if let item = vidUpscalerPopUp.menu?.item(withTag: i) {
                item.isEnabled = (upscalers[i] != nil)
            }
        }
        
        // Update dot mask textures
        parent.renderer.ressourceManager.buildDotMasks()
    }
    
    func refreshVideoTab() {

        let renderer = parent.renderer!

        // Colors
        vidPalettePopUp.selectItem(withTag: config.palette)
        vidBrightnessSlider.integerValue = config.brightness
        vidContrastSlider.integerValue = config.contrast
        vidSaturationSlider.integerValue = config.saturation

        // Geometry
        vidZoom.selectItem(withTag: config.zoom)
        vidHZoom.floatValue = config.hZoom * 1000
        vidVZoom.floatValue = config.vZoom * 1000
        vidHZoom.isEnabled = config.zoom == 0
        vidVZoom.isEnabled = config.zoom == 0
        vidHZoomLabel.textColor = config.zoom == 0 ? .labelColor : .disabledControlTextColor
        vidVZoomLabel.textColor = config.zoom == 0 ? .labelColor : .disabledControlTextColor
        vidCenter.selectItem(withTag: config.center)
        vidHCenter.floatValue = config.hCenter * 1000
        vidVCenter.floatValue = config.vCenter * 1000
        vidHCenter.isEnabled = config.center == 0
        vidVCenter.isEnabled = config.center == 0
        vidHCenterLabel.textColor = config.center == 0 ? .labelColor : .disabledControlTextColor
        vidVCenterLabel.textColor = config.center == 0 ? .labelColor : .disabledControlTextColor

        // Upscalers
        vidEnhancerPopUp.selectItem(withTag: config.enhancer)
        vidUpscalerPopUp.selectItem(withTag: config.upscaler)

        // Effects
        vidBlurPopUp.selectItem(withTag: Int(config.blur))
        vidBlurRadiusSlider.floatValue = config.blurRadius
        vidBlurRadiusSlider.isEnabled = config.blur > 0
        
        vidBloomPopUp.selectItem(withTag: Int(config.bloom))
        vidBloomRadiusSlider.floatValue = config.bloomRadius
        vidBloomRadiusSlider.isEnabled = config.bloom > 0
        vidBloomBrightnessSlider.floatValue = config.bloomBrightness
        vidBloomBrightnessSlider.isEnabled = config.bloom > 0
        vidBloomWeightSlider.floatValue = config.bloomWeight
        vidBloomWeightSlider.isEnabled = config.bloom > 0

        vidFlickerPopUp.selectItem(withTag: Int(config.flicker))
        vidFlickerWeightSlider.floatValue = config.flickerWeight
        vidFlickerWeightSlider.isEnabled = config.flicker > 0

        vidDotMaskPopUp.selectItem(withTag: Int(config.dotMask))
        for i in 0 ... 4 {
            vidDotMaskPopUp.item(at: i)?.image = renderer.ressourceManager.dotmaskImages[i]
        }
        vidDotMaskBrightnessSlider.floatValue = config.dotMaskBrightness
        vidDotMaskBrightnessSlider.isEnabled = config.dotMask > 0
        
        vidScanlinesPopUp.selectItem(withTag: Int(config.scanlines))
        vidScanlineBrightnessSlider.floatValue = config.scanlineBrightness
        vidScanlineBrightnessSlider.isEnabled = config.scanlines > 0
        vidScanlineWeightSlider.floatValue = config.scanlineWeight
        vidScanlineWeightSlider.isEnabled = config.scanlines == 2
        
        vidMisalignmentPopUp.selectItem(withTag: Int(config.disalignment))
        vidMisalignmentXSlider.floatValue = config.disalignmentH
        vidMisalignmentXSlider.isEnabled = config.disalignment > 0
        vidMisalignmentYSlider.floatValue = config.disalignmentV
        vidMisalignmentYSlider.isEnabled = config.disalignment > 0
  
        // Buttons
        vidPowerButton.isHidden = !bootable
    }

    //
    // Action methods (Colors)
    //
    
    @IBAction func vidPaletteAction(_ sender: NSPopUpButton!) {
        
        config.palette = sender.selectedTag()
    }
    
    @IBAction func vidBrightnessAction(_ sender: NSSlider!) {
        
        config.brightness = sender.integerValue
    }
    
    @IBAction func vidContrastAction(_ sender: NSSlider!) {
        
        config.contrast = sender.integerValue
    }
    
    @IBAction func vidSaturationAction(_ sender: NSSlider!) {
        
        config.saturation = sender.integerValue
    }

    //
    // Action methods (Geometry)
    //

    @IBAction func vidZoomAction(_ sender: NSPopUpButton) {

        config.zoom = sender.selectedTag()
        refresh()
    }

    @IBAction func vidHZoomAction(_ sender: NSSlider!) {

        let value = sender.floatValue / 1000
        config.hZoom = value
    }

    @IBAction func vidVZoomAction(_ sender: NSSlider!) {

        let value = sender.floatValue / 1000
        config.vZoom = value
    }

    @IBAction func vidCenterAction(_ sender: NSPopUpButton) {

        config.center = sender.selectedTag()
        refresh()
    }

    @IBAction func vidHCenterAction(_ sender: NSSlider!) {

        let value = sender.floatValue / 1000
        config.hCenter = value
    }

    @IBAction func vidVCenterAction(_ sender: NSSlider!) {

        let value = sender.floatValue / 1000
        config.vCenter = value
    }

    //
    // Action methods (Effects)
    //
    
    @IBAction func vidEnhancerAction(_ sender: NSPopUpButton!) {
        
        config.enhancer = sender.selectedTag()
    }
    
    @IBAction func vidUpscalerAction(_ sender: NSPopUpButton!) {

        config.upscaler = sender.selectedTag()
    }
    
    @IBAction func vidBlurAction(_ sender: NSPopUpButton!) {
        
        config.blur = sender.selectedTag()
    }
    
    @IBAction func vidBlurRadiusAction(_ sender: NSSlider!) {
        
        config.blurRadius = sender.floatValue
    }
    
    @IBAction func vidBloomAction(_ sender: NSPopUpButton!) {
        
        config.bloom = sender.selectedTag()
    }
    
    @IBAction func vidBloomRadiusAction(_ sender: NSSlider!) {
        
        config.bloomRadius = sender.floatValue
    }

    @IBAction func vidBloomBrightnessAction(_ sender: NSSlider!) {
        
        config.bloomBrightness = sender.floatValue
    }
    
    @IBAction func vidBloomWeightAction(_ sender: NSSlider!) {
        
        config.bloomWeight = sender.floatValue
    }

    @IBAction func vidFlickerAction(_ sender: NSPopUpButton!) {

        config.flicker = sender.selectedTag()
    }

    @IBAction func vidFlickerWeightAction(_ sender: NSSlider!) {

        config.flickerWeight = sender.floatValue
    }
    
    @IBAction func vidDotMaskAction(_ sender: NSPopUpButton!) {
        
        config.dotMask = sender.selectedTag()
    }
    
    @IBAction func vidDotMaskBrightnessAction(_ sender: NSSlider!) {
        
        config.dotMaskBrightness = sender.floatValue
    }
    
    @IBAction func vidScanlinesAction(_ sender: NSPopUpButton!) {
        
        config.scanlines = sender.selectedTag()
    }

    @IBAction func vidScanlineBrightnessAction(_ sender: NSSlider!) {
        
        config.scanlineBrightness = sender.floatValue
    }
    
    @IBAction func vidScanlineWeightAction(_ sender: NSSlider!) {
        
        config.scanlineWeight = sender.floatValue
    }
    
    @IBAction func vidDisalignmentAction(_ sender: NSPopUpButton!) {
        
        config.disalignment = sender.selectedTag()
    }

    @IBAction func vidDisalignmentHAction(_ sender: NSSlider!) {
        
        config.disalignmentH = sender.floatValue
    }
    
    @IBAction func vidDisalignmentVAction(_ sender: NSSlider!) {

        config.disalignmentV = sender.floatValue
    }

    //
    // Action methods (Misc)
    //
    
    @IBAction func vidPresetAction(_ sender: NSMenuItem!) {
              
        let defaults = EmulatorProxy.defaults!

        switch sender.tag {
            
        case 0: // Recommended settings (all)
            
            EmulatorProxy.defaults.removeVideoUserDefaults()

        case 10: // Recommended settings (geometry)

            EmulatorProxy.defaults.removeGeometryUserDefaults()

        case 11: // My personal monitor (ViewSonic VP191b)

            debug(1, "ViewSonic VP191b")
            EmulatorProxy.defaults.removeGeometryUserDefaults()
            defaults.set(Keys.Vid.zoom, 0)
            defaults.set(Keys.Vid.hZoom, 0.6763221)
            defaults.set(Keys.Vid.vZoom, 0.032)
            defaults.set(Keys.Vid.center, 0)
            defaults.set(Keys.Vid.hCenter, 0.39813587)
            defaults.set(Keys.Vid.vCenter, 1.0)

        case 20: // Recommended settings (colors + shader)

            EmulatorProxy.defaults.removeColorUserDefaults()
            EmulatorProxy.defaults.removeShaderUserDefaults()

        case 21: // TFT monitor

            EmulatorProxy.defaults.removeColorUserDefaults()
            EmulatorProxy.defaults.removeShaderUserDefaults()

        case 22: // CRT monitor

            EmulatorProxy.defaults.removeColorUserDefaults()
            EmulatorProxy.defaults.removeShaderUserDefaults()
            defaults.set(Keys.Vid.blurRadius, 1.5)
            defaults.set(Keys.Vid.bloom, 1)
            defaults.set(Keys.Vid.dotMask, 1)
            defaults.set(Keys.Vid.scanlines, 2)

        default:
            fatalError()
        }
        
        config.applyVideoUserDefaults()
    }
    
    @IBAction func vidDefaultsAction(_ sender: NSButton!) {
        
        config.saveVideoUserDefaults()
    }
}
