package com.example.zstd;

import android.content.res.AssetManager;

public class Zstd {
    static {
        System.loadLibrary("zstd_jni");
    }

    // Native byte array methods
    public static native byte[] compressBytes(byte[] input, int level);
    public static native byte[] decompressBytes(byte[] input, int originalSize);

    // Native asset methods (require AssetManager)
    public static native byte[] compressAsset(AssetManager assetManager, String assetName, int level);
    public static native byte[] decompressAsset(AssetManager assetManager, String assetName, int originalSize);

    // Convenience wrappers that can be called from anywhere in the app
    public static byte[] compress(byte[] input, int level) {
        return compressBytes(input, level);
    }

    public static byte[] decompress(byte[] input, int originalSize) {
        return decompressBytes(input, originalSize);
    }
}
