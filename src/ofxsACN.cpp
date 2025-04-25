#include "ofxsACN.hpp"

namespace ofxsACN {

    // Calculate number of universes required for a given pixel configuration
    int calculateRequiredUniverses(
        int width,
        int height,
        bool use16Bit,
        bool useAlpha,
        int maxBytesPerUniverse
    ) {
        // Calculate channels per pixel (RGB or RGBA)
        int channelsPerPixel = useAlpha ? 4 : 3;
        
        // Calculate bytes per pixel based on precision
        int bytesPerPixel = use16Bit ? channelsPerPixel * 2 : channelsPerPixel;
        
        // Calculate total number of pixels
        int totalPixels = width * height;
        
        // Calculate how many pixels can fit in one universe
        int pixelsPerUniverse = maxBytesPerUniverse / bytesPerPixel;
        
        // Calculate total number of universes needed (ceiling division)
        int numUniverses = (totalPixels + pixelsPerUniverse - 1) / pixelsPerUniverse;
        
        return numUniverses;
    }

    // Template specialization for ofPixels
    template<>
    std::vector<std::pair<uint8_t*, size_t>> pixelsToSacnDmx<ofPixels>(
        const ofPixels& pixels, 
        std::vector<uint8_t>& outputBuffer,
        bool use16Bit,
        bool useAlpha,
        bool highByteFirst
    ) {
        // Determine channel count (RGB or RGBA)
        int inputChannels = pixels.getNumChannels();
        int outputChannels = useAlpha ? 4 : 3;
        
        // If input doesn't have alpha but output needs it, we'll add it
        bool needToAddAlpha = useAlpha && inputChannels < 4;
        
        // Calculate total size of output buffer
        size_t pixelCount = pixels.getWidth() * pixels.getHeight();
        size_t bytesPerChannel = use16Bit ? 2 : 1;
        size_t totalOutputSize = pixelCount * outputChannels * bytesPerChannel;
        
        // Resize the output buffer
        outputBuffer.resize(totalOutputSize);
        
        // Convert pixels to appropriate format
        size_t outputIndex = 0;
        
        for (size_t i = 0; i < pixelCount; i++) {
            // Get the pixel color (handles pixel format internally)
            // Note: getColor(index) doesn't access at a pixel index; it accesses
            // at a channel index. So we need to multiply by the number of channels
            // to get the correct index for the pixel.
            ofColor color = pixels.getColor(i * pixels.getNumChannels());
            
            // Convert channels - either 8-bit or 16-bit
            if (use16Bit) {
                // 16-bit conversion (0-255 to 0-65535)
                uint16_t r = static_cast<uint16_t>(color.r * 257);  // 255 * 257 ≈ 65535
                uint16_t g = static_cast<uint16_t>(color.g * 257);
                uint16_t b = static_cast<uint16_t>(color.b * 257);
                uint16_t a = needToAddAlpha ? 65535 : static_cast<uint16_t>(color.a * 257);
                
                // Store each 16-bit value as two bytes in the appropriate order
                if (highByteFirst) {
                    // High byte first
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(r >> 8);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(r & 0xFF);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(g >> 8);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(g & 0xFF);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(b >> 8);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(b & 0xFF);
                    
                    if (useAlpha) {
                        outputBuffer[outputIndex++] = static_cast<uint8_t>(a >> 8);
                        outputBuffer[outputIndex++] = static_cast<uint8_t>(a & 0xFF);
                    }
                } else {
                    // Low byte first
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(r & 0xFF);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(r >> 8);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(g & 0xFF);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(g >> 8);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(b & 0xFF);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(b >> 8);
                    
                    if (useAlpha) {
                        outputBuffer[outputIndex++] = static_cast<uint8_t>(a & 0xFF);
                        outputBuffer[outputIndex++] = static_cast<uint8_t>(a >> 8);
                    }
                }
            } else {
                // 8-bit conversion (already in correct range)
                outputBuffer[outputIndex++] = static_cast<uint8_t>(color.r);
                outputBuffer[outputIndex++] = static_cast<uint8_t>(color.g);
                outputBuffer[outputIndex++] = static_cast<uint8_t>(color.b);
                
                if (useAlpha) {
                    outputBuffer[outputIndex++] = needToAddAlpha ? 255 : static_cast<uint8_t>(color.a);
                }
            }
        }
        
        // Now divide the data into universe chunks
        // Each universe can have at most 512 bytes (per DMX512 standard)
        const size_t MAX_UNIVERSE_SIZE = 512;
        std::vector<std::pair<uint8_t*, size_t>> universes;
        
        // Calculate bytes per pixel in the output
        size_t bytesPerPixel = outputChannels * bytesPerChannel;
        
        // Calculate the maximum number of pixels per universe
        // Making sure not to split pixels across universes
        size_t maxPixelsPerUniverse = MAX_UNIVERSE_SIZE / bytesPerPixel;
        
        // Calculate how many universes we need
        size_t numUniverses = (pixelCount + maxPixelsPerUniverse - 1) / maxPixelsPerUniverse;
        
        // Populate the universe data pointers
        for (size_t u = 0; u < numUniverses; u++) {
            size_t startPixel = u * maxPixelsPerUniverse;
            size_t pixelsInThisUniverse = std::min(maxPixelsPerUniverse, pixelCount - startPixel);
            size_t bytesInThisUniverse = pixelsInThisUniverse * bytesPerPixel;
            size_t startOffset = startPixel * bytesPerPixel;
            
            // Add a pointer to this universe's data and its size
            universes.push_back(std::make_pair(&outputBuffer[startOffset], bytesInThisUniverse));
        }
        
        return universes;
    }
    
    // Template specialization for ofFloatPixels
    template<>
    std::vector<std::pair<uint8_t*, size_t>> pixelsToSacnDmx<ofFloatPixels>(
        const ofFloatPixels& pixels, 
        std::vector<uint8_t>& outputBuffer,
        bool use16Bit,
        bool useAlpha,
        bool highByteFirst
    ) {
        // Determine channel count (RGB or RGBA)
        int inputChannels = pixels.getNumChannels();
        int outputChannels = useAlpha ? 4 : 3;
        
        // If input doesn't have alpha but output needs it, we'll add it
        bool needToAddAlpha = useAlpha && inputChannels < 4;
        
        // Calculate total size of output buffer
        size_t pixelCount = pixels.getWidth() * pixels.getHeight();
        size_t bytesPerChannel = use16Bit ? 2 : 1;
        size_t totalOutputSize = pixelCount * outputChannels * bytesPerChannel;
        
        // Resize the output buffer
        outputBuffer.resize(totalOutputSize);
        
        // Convert pixels to appropriate format
        size_t outputIndex = 0;
        
        for (size_t i = 0; i < pixelCount; i++) {
            // Get the pixel color (handles pixel format internally)
            // Note: getColor(index) doesn't access at a pixel index; it accesses
            // at a channel index. So we need to multiply by the number of channels
            // to get the correct index for the pixel.
            ofFloatColor color = pixels.getColor(i * pixels.getNumChannels());
            
            // Convert channels - either 8-bit or 16-bit
            if (use16Bit) {
                // 16-bit conversion (0.0-1.0 to 0-65535)
                uint16_t r = static_cast<uint16_t>(color.r * 65535.0f);
                uint16_t g = static_cast<uint16_t>(color.g * 65535.0f);
                uint16_t b = static_cast<uint16_t>(color.b * 65535.0f);
                uint16_t a = needToAddAlpha ? 65535 : static_cast<uint16_t>(color.a * 65535.0f);
                
                // Store each 16-bit value as two bytes in the appropriate order
                if (highByteFirst) {
                    // High byte first
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(r >> 8);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(r & 0xFF);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(g >> 8);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(g & 0xFF);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(b >> 8);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(b & 0xFF);
                    
                    if (useAlpha) {
                        outputBuffer[outputIndex++] = static_cast<uint8_t>(a >> 8);
                        outputBuffer[outputIndex++] = static_cast<uint8_t>(a & 0xFF);
                    }
                } else {
                    // Low byte first
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(r & 0xFF);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(r >> 8);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(g & 0xFF);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(g >> 8);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(b & 0xFF);
                    outputBuffer[outputIndex++] = static_cast<uint8_t>(b >> 8);
                    
                    if (useAlpha) {
                        outputBuffer[outputIndex++] = static_cast<uint8_t>(a & 0xFF);
                        outputBuffer[outputIndex++] = static_cast<uint8_t>(a >> 8);
                    }
                }
            } else {
                // 8-bit conversion (0.0-1.0 to 0-255)
                outputBuffer[outputIndex++] = static_cast<uint8_t>(color.r * 255.0f);
                outputBuffer[outputIndex++] = static_cast<uint8_t>(color.g * 255.0f);
                outputBuffer[outputIndex++] = static_cast<uint8_t>(color.b * 255.0f);
                
                if (useAlpha) {
                    outputBuffer[outputIndex++] = needToAddAlpha ? 255 : static_cast<uint8_t>(color.a * 255.0f);
                }
            }
        }
        
        // Now divide the data into universe chunks
        // Each universe can have at most 512 bytes (per DMX512 standard)
        const size_t MAX_UNIVERSE_SIZE = 512;
        std::vector<std::pair<uint8_t*, size_t>> universes;
        
        // Calculate bytes per pixel in the output
        size_t bytesPerPixel = outputChannels * bytesPerChannel;
        
        // Calculate the maximum number of pixels per universe
        // Making sure not to split pixels across universes
        size_t maxPixelsPerUniverse = MAX_UNIVERSE_SIZE / bytesPerPixel;
        
        // Calculate how many universes we need
        size_t numUniverses = (pixelCount + maxPixelsPerUniverse - 1) / maxPixelsPerUniverse;
        
        // Populate the universe data pointers
        for (size_t u = 0; u < numUniverses; u++) {
            size_t startPixel = u * maxPixelsPerUniverse;
            size_t pixelsInThisUniverse = std::min(maxPixelsPerUniverse, pixelCount - startPixel);
            size_t bytesInThisUniverse = pixelsInThisUniverse * bytesPerPixel;
            size_t startOffset = startPixel * bytesPerPixel;
            
            // Add a pointer to this universe's data and its size
            universes.push_back(std::make_pair(&outputBuffer[startOffset], bytesInThisUniverse));
        }
        
        return universes;
    }

} // namespace ofxsACN
