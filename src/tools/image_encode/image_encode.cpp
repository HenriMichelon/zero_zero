#include <iostream>
#include <vector>
#include <array>
#include "bc7enc.h"
#include <stb_image.h>
#include <stb_image_write.h>

// Constants
constexpr int IMAGE_WIDTH = 2048;
constexpr int IMAGE_HEIGHT = 2048;
constexpr int BLOCK_WIDTH = 4;
constexpr int BLOCK_HEIGHT = 4;
constexpr int BYTES_PER_PIXEL = 4;           // 32 bits per pixel (RGBA)
constexpr int BLOCK_PIXEL_COUNT = BLOCK_WIDTH * BLOCK_HEIGHT;
constexpr int BLOCK_SIZE_BYTES = BLOCK_PIXEL_COUNT * BYTES_PER_PIXEL;
constexpr int BC7_BLOCK_SIZE_BYTES = 16;     // BC7 compressed block is 16 bytes

int main() {
    // Initialize encoder parameters
    bc7enc_compress_block_params params;
    bc7enc_compress_block_params_init(&params);
    // params.m_quality = 1.0f;     // Adjust for desired quality vs. performance
    // params.m_rdo_quality = 1.0f; // Adjust for desired rate-distortion optimization

    // Input image (RGBA format, 2048x2048 pixels)
    std::vector<uint8_t> rgba_image(IMAGE_WIDTH * IMAGE_HEIGHT * BYTES_PER_PIXEL);

    // Fill `rgba_image` with actual data here (e.g., from an image loading library)
    // For now, this is left as an example, with a dummy pattern.

    // Output buffer for compressed BC7 data
    const int num_blocks_x = IMAGE_WIDTH / BLOCK_WIDTH;
    const int num_blocks_y = IMAGE_HEIGHT / BLOCK_HEIGHT;
    std::vector<uint8_t> bc7_data(num_blocks_x * num_blocks_y * BC7_BLOCK_SIZE_BYTES);

    // Loop over each 4x4 block in the image
    for (int by = 0; by < num_blocks_y; ++by) {
        for (int bx = 0; bx < num_blocks_x; ++bx) {
            // Extract 4x4 block of RGBA pixels from the image
            std::array<uint8_t, BLOCK_SIZE_BYTES> rgba_block{};
            for (int y = 0; y < BLOCK_HEIGHT; ++y) {
                for (int x = 0; x < BLOCK_WIDTH; ++x) {
                    int src_x = bx * BLOCK_WIDTH + x;
                    int src_y = by * BLOCK_HEIGHT + y;
                    int src_index = (src_y * IMAGE_WIDTH + src_x) * BYTES_PER_PIXEL;

                    // Copy pixel data into the block array
                    std::copy_n(&rgba_image[src_index], BYTES_PER_PIXEL, &rgba_block[(y * BLOCK_WIDTH + x) * BYTES_PER_PIXEL]);
                }
            }

            // Compress the block using BC7 encoding
            std::array<uint8_t, BC7_BLOCK_SIZE_BYTES> bc7_block{};
            int result = bc7enc_compress_block(bc7_block.data(), rgba_block.data(), &params);
            if (result != 0) {
                std::cerr << "BC7 encoding failed at block (" << bx << ", " << by << ")." << std::endl;
                return -1;
            }

            // Store compressed block in output buffer
            int block_index = (by * num_blocks_x + bx) * BC7_BLOCK_SIZE_BYTES;
            std::copy(bc7_block.begin(), bc7_block.end(), bc7_data.begin() + block_index);
        }
    }

    std::cout << "BC7 encoding completed for the entire image!" << std::endl;

    // `bc7_data` now contains the entire compressed image in BC7 format.
    // You can save it to a file or process it further as needed.

    return 0;
}
