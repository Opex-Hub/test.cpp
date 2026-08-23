#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <vector>
#include <cstring>
#include <zstd.h>

#define LOG_TAG "ZstdJNI"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Helper: convert jbyteArray to std::vector<uint8_t>
static std::vector<uint8_t> jbyteArrayToVector(JNIEnv* env, jbyteArray array) {
    jsize len = env->GetArrayLength(array);
    std::vector<uint8_t> buffer(len);
    if (len > 0) {
        env->GetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte*>(buffer.data()));
    }
    return buffer;
}

// Helper: convert std::vector<uint8_t> to jbyteArray
static jbyteArray vectorToJbyteArray(JNIEnv* env, const std::vector<uint8_t>& data) {
    jbyteArray result = env->NewByteArray(data.size());
    if (data.size() > 0) {
        env->SetByteArrayRegion(result, 0, data.size(), reinterpret_cast<const jbyte*>(data.data()));
    }
    return result;
}

// Helper: read an entire asset file into a vector
static bool readAsset(AAssetManager* mgr, const char* filename, std::vector<uint8_t>& out) {
    AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_BUFFER);
    if (!asset) {
        LOGE("Failed to open asset: %s", filename);
        return false;
    }
    off_t size = AAsset_getLength(asset);
    out.resize(size);
    if (size > 0) {
        int bytesRead = AAsset_read(asset, out.data(), size);
        if (bytesRead != size) {
            LOGE("Failed to read asset fully: %s", filename);
            AAsset_close(asset);
            return false;
        }
    }
    AAsset_close(asset);
    return true;
}

// Helper: write a vector to an asset file (for demonstration; not typically needed)
static bool writeAsset(AAssetManager* mgr, const char* filename, const std::vector<uint8_t>& data) {
    // Assets are read-only in Android; this is just a placeholder
    (void)mgr; (void)filename; (void)data;
    return false;
}

extern "C" {

// ------------------- Byte Array Compression -------------------

JNIEXPORT jbyteArray JNICALL
Java_com_example_zstd_Zstd_compressBytes(JNIEnv* env, jclass, jbyteArray input, jint level) {
    std::vector<uint8_t> src = jbyteArrayToVector(env, input);
    if (src.empty()) {
        return env->NewByteArray(0);
    }

    size_t maxCompressedSize = ZSTD_compressBound(src.size());
    std::vector<uint8_t> dst(maxCompressedSize);

    size_t compressedSize = ZSTD_compress(dst.data(), dst.size(),
                                          src.data(), src.size(),
                                          level);
    if (ZSTD_isError(compressedSize)) {
        LOGE("ZSTD_compress error: %s", ZSTD_getErrorName(compressedSize));
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, ZSTD_getErrorName(compressedSize));
        return nullptr;
    }

    dst.resize(compressedSize);
    return vectorToJbyteArray(env, dst);
}

JNIEXPORT jbyteArray JNICALL
Java_com_example_zstd_Zstd_decompressBytes(JNIEnv* env, jclass, jbyteArray input, jint originalSize) {
    std::vector<uint8_t> src = jbyteArrayToVector(env, input);
    if (src.empty()) {
        return env->NewByteArray(0);
    }

    std::vector<uint8_t> dst(originalSize);

    size_t decompressedSize = ZSTD_decompress(dst.data(), dst.size(),
                                              src.data(), src.size());
    if (ZSTD_isError(decompressedSize)) {
        LOGE("ZSTD_decompress error: %s", ZSTD_getErrorName(decompressedSize));
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, ZSTD_getErrorName(decompressedSize));
        return nullptr;
    }

    dst.resize(decompressedSize);
    return vectorToJbyteArray(env, dst);
}

// ------------------- Asset File Compression -------------------
// These functions read from and write to Android assets using AAssetManager.
// Assets are read-only, so we only provide compression of assets and return the result.
// Decompression of an asset would require the original size and a writable destination.

JNIEXPORT jbyteArray JNICALL
Java_com_example_zstd_Zstd_compressAsset(JNIEnv* env, jclass, jobject assetManager,
                                         jstring assetName, jint level) {
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, "Invalid AssetManager");
        return nullptr;
    }

    const char* filename = env->GetStringUTFChars(assetName, nullptr);
    std::vector<uint8_t> src;
    if (!readAsset(mgr, filename, src)) {
        env->ReleaseStringUTFChars(assetName, filename);
        jclass exClass = env->FindClass("java/io/IOException");
        env->ThrowNew(exClass, "Failed to read asset");
        return nullptr;
    }
    env->ReleaseStringUTFChars(assetName, filename);

    if (src.empty()) {
        return env->NewByteArray(0);
    }

    size_t maxCompressedSize = ZSTD_compressBound(src.size());
    std::vector<uint8_t> dst(maxCompressedSize);

    size_t compressedSize = ZSTD_compress(dst.data(), dst.size(),
                                          src.data(), src.size(),
                                          level);
    if (ZSTD_isError(compressedSize)) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, ZSTD_getErrorName(compressedSize));
        return nullptr;
    }

    dst.resize(compressedSize);
    return vectorToJbyteArray(env, dst);
}

// Decompress an asset: read compressed asset, decompress using known original size.
JNIEXPORT jbyteArray JNICALL
Java_com_example_zstd_Zstd_decompressAsset(JNIEnv* env, jclass, jobject assetManager,
                                           jstring assetName, jint originalSize) {
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, "Invalid AssetManager");
        return nullptr;
    }

    const char* filename = env->GetStringUTFChars(assetName, nullptr);
    std::vector<uint8_t> src;
    if (!readAsset(mgr, filename, src)) {
        env->ReleaseStringUTFChars(assetName, filename);
        jclass exClass = env->FindClass("java/io/IOException");
        env->ThrowNew(exClass, "Failed to read asset");
        return nullptr;
    }
    env->ReleaseStringUTFChars(assetName, filename);

    std::vector<uint8_t> dst(originalSize);

    size_t decompressedSize = ZSTD_decompress(dst.data(), dst.size(),
                                              src.data(), src.size());
    if (ZSTD_isError(decompressedSize)) {
        jclass exClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exClass, ZSTD_getErrorName(decompressedSize));
        return nullptr;
    }

    dst.resize(decompressedSize);
    return vectorToJbyteArray(env, dst);
}

} // extern "C"
