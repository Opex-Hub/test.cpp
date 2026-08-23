#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <cstdint>
#include <cstring>

#define LOG_TAG "NativeWindowRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Global ANativeWindow reference
static ANativeWindow* nativeWindow = nullptr;

// Lock the native window for direct pixel manipulation
extern "C" JNIEXPORT jint JNICALL
Java_com_roblox_client_NativeWindowRenderer_lockNativeWindow(
        JNIEnv *env,
        jobject /* this */,
        jobject surface) {
    
    // Get the native window from the Surface object
    ANativeWindow* newWindow = ANativeWindow_fromSurface(env, surface);
    
    if (newWindow == nullptr) {
        LOGE("Failed to get ANativeWindow from Surface");
        return -1;
    }
    
    // Release previous window if exists
    if (nativeWindow != nullptr) {
        ANativeWindow_release(nativeWindow);
    }
    
    nativeWindow = newWindow;
    LOGI("Successfully locked native window");
    return 0;
}

// Unlock and post the frame to the screen
extern "C" JNIEXPORT jint JNICALL
Java_com_roblox_client_NativeWindowRenderer_unlockAndPost(
        JNIEnv *env,
        jobject /* this */) {
    
    if (nativeWindow == nullptr) {
        LOGE("Native window not initialized");
        return -1;
    }
    
    // Unlock the window and post the buffer
    ANativeWindow_unlockAndPost(nativeWindow);
    return 0;
}

// Get window dimensions
extern "C" JNIEXPORT jobject JNICALL
Java_com_roblox_client_NativeWindowRenderer_getWindowSize(
        JNIEnv *env,
        jobject /* this */) {
    
    if (nativeWindow == nullptr) {
        LOGE("Native window not initialized");
        return nullptr;
    }
    
    // Get window dimensions
    int32_t width = ANativeWindow_getWidth(nativeWindow);
    int32_t height = ANativeWindow_getHeight(nativeWindow);
    
    // Create Point object to return dimensions
    jclass pointClass = env->FindClass("android/graphics/Point");
    if (pointClass == nullptr) {
        LOGE("Could not find Point class");
        return nullptr;
    }
    
    jmethodID constructor = env->GetMethodID(pointClass, "<init>", "(II)V");
    if (constructor == nullptr) {
        LOGE("Could not find Point constructor");
        return nullptr;
    }
    
    jobject point = env->NewObject(pointClass, constructor, width, height);
    return point;
}

// Draw a simple gradient pattern directly to the pixel buffer
extern "C" JNIEXPORT jint JNICALL
Java_com_roblox_client_NativeWindowRenderer_drawGradient(
        JNIEnv *env,
        jobject /* this */) {
    
    if (nativeWindow == nullptr) {
        LOGE("Native window not initialized");
        return -1;
    }
    
    // Lock the window buffer
    ANativeWindow_Buffer buffer;
    int32_t lockResult = ANativeWindow_lock(nativeWindow, &buffer, nullptr);
    
    if (lockResult != 0) {
        LOGE("Failed to lock window buffer: %d", lockResult);
        return -1;
    }
    
    // Check format (we expect RGBA_8888)
    if (buffer.format != WINDOW_FORMAT_RGBA_8888) {
        LOGE("Unsupported window format: %d", buffer.format);
        ANativeWindow_unlockAndPost(nativeWindow);
        return -1;
    }
    
    // Draw gradient pattern
    uint8_t* pixels = static_cast<uint8_t*>(buffer.bits);
    int32_t stride = buffer.stride; // This might differ from width
    int32_t width = buffer.width;
    int32_t height = buffer.height;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate pixel position in buffer (accounting for stride)
            uint8_t* pixel = pixels + (y * stride + x) * 4;
            
            // Create RGB gradient
            pixel[0] = static_cast<uint8_t>((x * 255) / width);     // Red
            pixel[1] = static_cast<uint8_t>((y * 255) / height);    // Green
            pixel[2] = static_cast<uint8_t>(128);                   // Blue
            pixel[3] = 255;                                         // Alpha
        }
    }
    
    // Unlock and post (this is done in unlockAndPost function)
    // Just unlock here since we'll call unlockAndPost separately
    ANativeWindow_unlockAndPost(nativeWindow);
    return 0;
}

// Fill the entire buffer with a solid color
extern "C" JNIEXPORT jint JNICALL
Java_com_roblox_client_NativeWindowRenderer_fillColor(
        JNIEnv *env,
        jobject /* this */,
        jint red,
        jint green,
        jint blue,
        jint alpha) {
    
    if (nativeWindow == nullptr) {
        LOGE("Native window not initialized");
        return -1;
    }
    
    // Clamp color values to [0, 255]
    red = std::max(0, std::min(255, red));
    green = std::max(0, std::min(255, green));
    blue = std::max(0, std::min(255, blue));
    alpha = std::max(0, std::min(255, alpha));
    
    // Lock the window buffer
    ANativeWindow_Buffer buffer;
    int32_t lockResult = ANativeWindow_lock(nativeWindow, &buffer, nullptr);
    
    if (lockResult != 0) {
        LOGE("Failed to lock window buffer: %d", lockResult);
        return -1;
    }
    
    // Check format
    if (buffer.format != WINDOW_FORMAT_RGBA_8888) {
        LOGE("Unsupported window format: %d", buffer.format);
        ANativeWindow_unlockAndPost(nativeWindow);
        return -1;
    }
    
    // Fill with solid color
    uint8_t* pixels = static_cast<uint8_t*>(buffer.bits);
    int32_t stride = buffer.stride;
    int32_t width = buffer.width;
    int32_t height = buffer.height;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t* pixel = pixels + (y * stride + x) * 4;
            pixel[0] = static_cast<uint8_t>(red);
            pixel[1] = static_cast<uint8_t>(green);
            pixel[2] = static_cast<uint8_t>(blue);
            pixel[3] = static_cast<uint8_t>(alpha);
        }
    }
    
    // Unlock and post
    ANativeWindow_unlockAndPost(nativeWindow);
    return 0;
}

// Draw custom pixel data to the buffer
extern "C" JNIEXPORT jint JNICALL
Java_com_roblox_client_NativeWindowRenderer_drawPixels(
        JNIEnv *env,
        jobject /* this */,
        jbyteArray pixelData,
        jint width,
        jint height) {
    
    if (nativeWindow == nullptr) {
        LOGE("Native window not initialized");
        return -1;
    }
    
    if (pixelData == nullptr) {
        LOGE("Pixel data is null");
        return -1;
    }
    
    // Lock the window buffer
    ANativeWindow_Buffer buffer;
    int32_t lockResult = ANativeWindow_lock(nativeWindow, &buffer, nullptr);
    
    if (lockResult != 0) {
        LOGE("Failed to lock window buffer: %d", lockResult);
        return -1;
    }
    
    // Check format
    if (buffer.format != WINDOW_FORMAT_RGBA_8888) {
        LOGE("Unsupported window format: %d", buffer.format);
        ANativeWindow_unlockAndPost(nativeWindow);
        return -1;
    }
    
    // Get pixel data from Java
    jsize dataSize = env->GetArrayLength(pixelData);
    jbyte* inputPixels = env->GetByteArrayElements(pixelData, nullptr);
    
    // Validate data size
    if (dataSize != width * height * 4) {
        LOGE("Pixel data size mismatch: expected %d, got %d", width * height * 4, dataSize);
        env->ReleaseByteArrayElements(pixelData, inputPixels, JNI_ABORT);
        ANativeWindow_unlockAndPost(nativeWindow);
        return -1;
    }
    
    // Copy pixel data to window buffer
    uint8_t* outputPixels = static_cast<uint8_t*>(buffer.bits);
    int32_t stride = buffer.stride;
    int32_t bufferWidth = buffer.width;
    int32_t bufferHeight = buffer.height;
    
    // Clamp dimensions to buffer size
    int32_t copyWidth = std::min(width, bufferWidth);
    int32_t copyHeight = std::min(height, bufferHeight);
    
    for (int y = 0; y < copyHeight; y++) {
        memcpy(outputPixels + y * stride * 4, 
               inputPixels + y * width * 4, 
               copyWidth * 4);
    }
    
    // Release resources
    env->ReleaseByteArrayElements(pixelData, inputPixels, JNI_ABORT);
    ANativeWindow_unlockAndPost(nativeWindow);
    return 0;
}

// Release native window resources
extern "C" JNIEXPORT void JNICALL
Java_com_roblox_client_NativeWindowRenderer_releaseNativeWindow(
        JNIEnv *env,
        jobject /* this */) {
    
    if (nativeWindow != nullptr) {
        ANativeWindow_release(nativeWindow);
        nativeWindow = nullptr;
        LOGI("Native window released");
    }
}
