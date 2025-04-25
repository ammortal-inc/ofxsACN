#ifndef ofxsACN_hpp
#define ofxsACN_hpp

#include "ofMain.h"
#include "sacn/cpp/common.h"
#include "sacn/cpp/source.h"
#include "etcpal/cpp/log.h"  // For etcpal::Logger
#include "etcpal/cpp/netint.h" // For etcpal::NetintInfo

namespace ofxsACN {
    
    /**
     * Converts OpenFrameworks pixel data to sACN DMX data, dividing into appropriate universe chunks
     * 
     * @tparam PixelType ofPixels or ofFloatPixels
     * @param pixels Input pixel data
     * @param outputBuffer Reference to vector to store converted data
     * @param use16Bit Whether to convert to 16-bit precision (otherwise 8-bit)
     * @param useAlpha Whether to include alpha channel in output
     * @param highByteFirst For 16-bit data, whether to store high byte first (true) or low byte first (false)
     * @return Vector of pairs containing pointers to each universe's data and its size in bytes
     */
    template<typename PixelType>
    std::vector<std::pair<uint8_t*, size_t>> pixelsToSacnDmx(
        const PixelType& pixels, 
        std::vector<uint8_t>& outputBuffer,
        bool use16Bit = false,
        bool useAlpha = false,
        bool highByteFirst = true
    );
    
    /**
     * Calculates the number of sACN universes needed for a given pixel configuration
     * 
     * @param width Width of the pixel grid
     * @param height Height of the pixel grid
     * @param use16Bit Whether using 16-bit precision (otherwise 8-bit)
     * @param useAlpha Whether including alpha channel in output
     * @param maxBytesPerUniverse Maximum bytes per universe (usually 512 for DMX512)
     * @return Number of universes needed
     */
    int calculateRequiredUniverses(
        int width,
        int height,
        bool use16Bit = false,
        bool useAlpha = false,
        int maxBytesPerUniverse = 512
    );
    
} // namespace ofxsACN

#endif /* ofxsACN_hpp */

// Static helper function that assists with converting native openframeworks
// types to the types used by the sACN library.
// - This function should be able to accept either an ofPixels or an ofFloatPixels
// object. These inputs may have different channel counts, so the function will
// need to handle that as well. 
// - This function will also take as input whether the data to be converted
// shall be converted to 8-bit or 16-bit data, regardless of the input data type precision.
// - This function will also take as input whether the 16-bit data, if specified,
// should be high->low or low->high byte order in the output.
// - This function should output into a vector<uint8_t> (possibly specified as a reference).
// The output vector should be resized to the appropriate size based on the input
// data type and the specified output data type. This vector will contain a continuous 
// array of uint8_t data, since this is the only data type accepted by the sACN E1.31 spec.
// - Returned from the function should also be a vector<pair<uint8_t*, size_t>>
// that contains the pointers to the data in the output vector, as well as the size of
// the data in that section in bytes. This will be used to send the data to the sACN library. 
// Each of these sections represents a different universe. Data from a single pixel
// should not be split across different universes. A universe should contain no more than 512 bytes.
// The function will use as many universes as necessary to encapsulate and convert 
// all of the input data.
