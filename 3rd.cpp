#include <jni.h>
#include <android/log.h>
#include <cstdint>
#include <memory>
#include <algorithm>

// Enable NEON vectorization for ARM64
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

#define LOG_TAG "ImageProcessor"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ============= Image Processing Functions =============

// Convert RGBA to grayscale using NEON optimization
void rgbaToGrayscaleNeon(const uint8_t* input, uint8_t* output, int pixelCount) {
#ifdef __ARM_NEON
    const int simdSize = 16; // Process 16 pixels at once
    const int pixelBatch = 4; // 4 channels per pixel

    // Coefficients for grayscale conversion (0.299R + 0.587G + 0.114B)
    // Using fixed point arithmetic for performance
    uint8x16x4_t coeffs = {{
        vdupq_n_u8(77),   // 0.299 * 256 ≈ 77
        vdupq_n_u8(150),  // 0.587 * 256 ≈ 150
        vdupq_n_u8(29),   // 0.114 * 256 ≈ 29
        vdupq_n_u8(0)
    }};

    int simdPixels = pixelCount / simdSize;
    int remainingPixels = pixelCount % simdSize;

    for (int i = 0; i < simdPixels; ++i) {
        // Load 16 RGBA pixels (64 bytes)
        uint8x16x4_t rgba = vld4q_u8(input);
        input += 64;

        // Convert to grayscale using weighted sum
        uint16x8_t lowSum = vmull_u8(vget_low_u8(rgba.val[0]), vget_low_u8(coeffs.val[0]));
        lowSum = vmlal_u8(lowSum, vget_low_u8(rgba.val[1]), vget_low_u8(coeffs.val[1]));
        lowSum = vmlal_u8(lowSum, vget_low_u8(rgba.val[2]), vget_low_u8(coeffs.val[2]));

        uint16x8_t highSum = vmull_u8(vget_high_u8(rgba.val[0]), vget_high_u8(coeffs.val[0]));
        highSum = vmlal_u8(highSum, vget_high_u8(rgba.val[1]), vget_high_u8(coeffs.val[1]));
        highSum = vmlal_u8(highSum, vget_high_u8(rgba.val[2]), vget_high_u8(coeffs.val[2]));

        // Divide by 256 (right shift by 8) and narrow to 8-bit
        uint8x8_t grayLow = vshrn_n_u16(lowSum, 8);
        uint8x8_t grayHigh = vshrn_n_u16(highSum, 8);

        // Store result
        vst1q_u8(output, vcombine_u8(grayLow, grayHigh));
        output += 16;
    }

    // Process remaining pixels
    for (int i = 0; i < remainingPixels; ++i) {
        int r = input[0];
        int g = input[1];
        int b = input[2];
        *output = (77 * r + 150 * g + 29 * b) >> 8;
        input += 4;
        output++;
    }
#else
    // Fallback for non-NEON platforms
    for (int i = 0; i < pixelCount; ++i) {
        int r = input[0];
        int g = input[1];
        int b = input[2];
        *output = (77 * r + 150 * g + 29 * b) >> 8;
        input += 4;
        output++;
    }
#endif
}

// Scale image using bilinear interpolation
void scaleImageBilinear(const uint8_t* input, uint8_t* output, 
                        int srcWidth, int srcHeight, int dstWidth, int dstHeight) {
    float xRatio = (float)(srcWidth - 1) / dstWidth;
    float yRatio = (float)(srcHeight - 1) / dstHeight;

    for (int y = 0; y < dstHeight; y++) {
        for (int x = 0; x < dstWidth; x++) {
            float px = xRatio * x;
            float py = yRatio * y;
            
            int x1 = (int)px;
            int y1 = (int)py;
            int x2 = (x1 + 1) < srcWidth ? x1 + 1 : x1;
            int y2 = (y1 + 1) < srcHeight ? y1 + 1 : y1;
            
            float fx = px - x1;
            float fy = py - y1;
            float fx1 = 1.0f - fx;
            float fy1 = 1.0f - fy;

            int index1 = (y1 * srcWidth + x1) * 4;
            int index2 = (y1 * srcWidth + x2) * 4;
            int index3 = (y2 * srcWidth + x1) * 4;
            int index4 = (y2 * srcWidth + x2) * 4;
            int dstIndex = (y * dstWidth + x) * 4;

            for (int c = 0; c < 4; c++) {
                float p1 = input[index1 + c] * fx1 + input[index2 + c] * fx;
                float p2 = input[index3 + c] * fx1 + input[index4 + c] * fx;
                output[dstIndex + c] = (uint8_t)(p1 * fy1 + p2 * fy);
            }
        }
    }
}

// Apply simple box blur filter
void applyBoxBlur(uint8_t* imageData, int width, int height, int radius) {
    int windowSize = 2 * radius + 1;
    int windowArea = windowSize * windowSize;
    
    // Temporary buffer for processing
    std::unique_ptr<uint8_t[]> tempBuffer(new uint8_t[width * height * 4]);
    
    // Horizontal pass
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int r = 0, g = 0, b = 0, a = 0;
            
            for (int dx = -radius; dx <= radius; dx++) {
                int nx = std::max(0, std::min(width - 1, x + dx));
                int idx = (y * width + nx) * 4;
                r += imageData[idx];
                g += imageData[idx + 1];
                b += imageData[idx + 2];
                a += imageData[idx + 3];
            }
            
            int outIdx = (y * width + x) * 4;
            tempBuffer[outIdx] = r / windowSize;
            tempBuffer[outIdx + 1] = g / windowSize;
            tempBuffer[outIdx + 2] = b / windowSize;
            tempBuffer[outIdx + 3] = a / windowSize;
        }
    }
    
    // Vertical pass
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int r = 0, g = 0, b = 0, a = 0;
            
            for (int dy = -radius; dy <= radius; dy++) {
                int ny = std::max(0, std::min(height - 1, y + dy));
                int idx = (ny * width + x) * 4;
                r += tempBuffer[idx];
                g += tempBuffer[idx + 1];
                b += tempBuffer[idx + 2];
                a += tempBuffer[idx + 3];
            }
            
            int outIdx = (y * width + x) * 4;
            imageData[outIdx] = r / windowSize;
            imageData[outIdx + 1] = g / windowSize;
            imageData[outIdx + 2] = b / windowSize;
            imageData[outIdx + 3] = a / windowSize;
        }
    }
}

// JNI function to convert RGBA to grayscale
extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_roblox_client_ImageProcessor_convertRgbaToGrayscale(
        JNIEnv *env,
        jobject /* this */,
        jbyteArray rgbaData,
        jint width,
        jint height) {
    
    jsize dataSize = env->GetArrayLength(rgbaData);
    jbyte* inputData = env->GetByteArrayElements(rgbaData, nullptr);
    
    // Allocate output array (1 byte per pixel for grayscale)
    jbyteArray grayData = env->NewByteArray(width * height);
    jbyte* outputData = env->GetByteArrayElements(grayData, nullptr);
    
    // Process image
    rgbaToGrayscaleNeon(reinterpret_cast<const uint8_t*>(inputData),
                       reinterpret_cast<uint8_t*>(outputData),
                       width * height);
    
    // Release arrays
    env->ReleaseByteArrayElements(rgbaData, inputData, JNI_ABORT);
    env->ReleaseByteArrayElements(grayData, outputData, 0);
    
    return grayData;
}

// JNI function to scale image
extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_roblox_client_ImageProcessor_scaleImage(
        JNIEnv *env,
        jobject /* this */,
        jbyteArray inputData,
        jint srcWidth,
        jint srcHeight,
        jint dstWidth,
        jint dstHeight) {
    
    jsize dataSize = env->GetArrayLength(inputData);
    jbyte* inputBytes = env->GetByteArrayElements(inputData, nullptr);
    
    // Allocate output array (RGBA format)
    jbyteArray outputData = env->NewByteArray(dstWidth * dstHeight * 4);
    jbyte* outputBytes = env->GetByteArrayElements(outputData, nullptr);
    
    // Process image
    scaleImageBilinear(reinterpret_cast<const uint8_t*>(inputBytes),
                      reinterpret_cast<uint8_t*>(outputBytes),
                      srcWidth, srcHeight, dstWidth, dstHeight);
    
    // Release arrays
    env->ReleaseByteArrayElements(inputData, inputBytes, JNI_ABORT);
    env->ReleaseByteArrayElements(outputData, outputBytes, 0);
    
    return outputData;
}

// JNI function to apply box blur
extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_roblox_client_ImageProcessor_applyBoxBlur(
        JNIEnv *env,
        jobject /* this */,
        jbyteArray inputData,
        jint width,
        jint height,
        jint radius) {
    
    jsize dataSize = env->GetArrayLength(inputData);
    jbyte* inputBytes = env->GetByteArrayElements(inputData, nullptr);
    
    // Create output array (copy input data)
    jbyteArray outputData = env->NewByteArray(dataSize);
    env->SetByteArrayRegion(outputData, 0, dataSize, inputBytes);
    jbyte* outputBytes = env->GetByteArrayElements(outputData, nullptr);
    
    // Process image
    applyBoxBlur(reinterpret_cast<uint8_t*>(outputBytes), width, height, radius);
    
    // Release arrays
    env->ReleaseByteArrayElements(inputData, inputBytes, JNI_ABORT);
    env->ReleaseByteArrayElements(outputData, outputBytes, 0);
    
    return outputData;
}
