/**
 * ARM64 Hook Utility (libtrampoline)
 * 
 * Provides function detouring on ARM64 (AArch64) Linux/Android.
 * The hook overwrites the first 16 bytes of the target function with
 * a far jump to the detour. A trampoline is allocated to execute the
 * original prologue and then jump back to the rest of the function.
 */

#include <sys/mman.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <cstdio>   // for demonstration main

#ifndef __aarch64__
#error "This code is for ARM64 (AArch64) only"
#endif

// -----------------------------------------------------------------------------
// Memory page permission helpers
// -----------------------------------------------------------------------------

static size_t get_page_size() {
    static size_t size = sysconf(_SC_PAGESIZE);
    return size;
}

static bool make_writable(void* addr, size_t len) {
    uintptr_t page_start = reinterpret_cast<uintptr_t>(addr) & ~(get_page_size() - 1);
    size_t aligned_len = (reinterpret_cast<uintptr_t>(addr) + len - page_start + get_page_size() - 1) 
                         & ~(get_page_size() - 1);
    return mprotect(reinterpret_cast<void*>(page_start), aligned_len,
                    PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
}

static bool make_executable(void* addr, size_t len) {
    uintptr_t page_start = reinterpret_cast<uintptr_t>(addr) & ~(get_page_size() - 1);
    size_t aligned_len = (reinterpret_cast<uintptr_t>(addr) + len - page_start + get_page_size() - 1) 
                         & ~(get_page_size() - 1);
    return mprotect(reinterpret_cast<void*>(page_start), aligned_len,
                    PROT_READ | PROT_EXEC) == 0;
}

static void flush_cache(void* addr, size_t len) {
    __builtin___clear_cache(reinterpret_cast<char*>(addr),
                            reinterpret_cast<char*>(addr) + len);
}

// -----------------------------------------------------------------------------
// ARM64 instruction encoding
// -----------------------------------------------------------------------------

// LDR X16, #8   (load 64-bit literal from PC + 8)
static uint32_t encode_ldr_x16_literal() {
    // Opcode: 0x58000050 | (imm19 << 5) | Rt (X16)
    // imm19 = 2 (8 bytes / 4)
    return 0x58000050 | (2 << 5);
}

// BR X16
static uint32_t encode_br_x16() {
    return 0xD61F0200;
}

// B <offset>   (near branch, offset in 4-byte units from PC)
static uint32_t encode_b_imm(int32_t offset) {
    return 0x14000000 | ((offset >> 2) & 0x3FFFFFF);
}

// -----------------------------------------------------------------------------
// Executable memory allocation for trampolines
// -----------------------------------------------------------------------------

static void* allocate_executable_memory(size_t size) {
    void* mem = mmap(nullptr, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (mem == MAP_FAILED) ? nullptr : mem;
}

// -----------------------------------------------------------------------------
// TrampolineHook class
// -----------------------------------------------------------------------------

class TrampolineHook {
public:
    using OriginalFunc = void (*)();

    TrampolineHook() = default;
    ~TrampolineHook() { uninstall(); }

    // Install a hook on target_function, redirecting to detour_function.
    // On success, *original_out is set to a trampoline that calls the original function.
    // Returns true on success.
    bool install(void* target, void* detour, OriginalFunc* original_out) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (installed_) return false;

        target_ = reinterpret_cast<uintptr_t>(target);
        detour_ = reinterpret_cast<uintptr_t>(detour);
        constexpr size_t far_jump_size = 16; // 4 instructions, last 8 bytes literal

        // 1. Make target code writable
        if (!make_writable(target, far_jump_size)) {
            return false;
        }

        // 2. Save original prologue (16 bytes)
        memcpy(original_bytes_, target, far_jump_size);
        original_size_ = far_jump_size;

        // 3. Allocate trampoline: original code + jump back
        trampoline_size_ = original_size_ + far_jump_size;
        trampoline_ = allocate_executable_memory(trampoline_size_);
        if (!trampoline_) {
            make_executable(target, far_jump_size);
            return false;
        }

        // 4. Copy original prologue to trampoline
        memcpy(trampoline_, original_bytes_, original_size_);

        // 5. Write far jump back to target + original_size_ inside trampoline
        uint8_t* tramp_jump = reinterpret_cast<uint8_t*>(trampoline_) + original_size_;
        write_far_jump(tramp_jump, target_ + original_size_);

        // 6. Write far jump from target to detour
        write_far_jump(reinterpret_cast<uint8_t*>(target), detour_);

        // 7. Flush caches
        flush_cache(target, far_jump_size);
        flush_cache(trampoline_, trampoline_size_);

        // 8. Restore target page to RX
        make_executable(target, far_jump_size);

        installed_ = true;
        *original_out = reinterpret_cast<OriginalFunc>(trampoline_);
        return true;
    }

    void uninstall() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!installed_) return;

        // Restore original instructions
        if (make_writable(reinterpret_cast<void*>(target_), original_size_)) {
            memcpy(reinterpret_cast<void*>(target_), original_bytes_, original_size_);
            flush_cache(reinterpret_cast<void*>(target_), original_size_);
            make_executable(reinterpret_cast<void*>(target_), original_size_);
        }

        // Free trampoline
        if (trampoline_) {
            munmap(trampoline_, trampoline_size_);
            trampoline_ = nullptr;
        }

        installed_ = false;
    }

private:
    static constexpr size_t far_jump_size = 16;

    void write_far_jump(uint8_t* where, uint64_t target_addr) {
        uint32_t ldr = encode_ldr_x16_literal();
        uint32_t br = encode_br_x16();
        memcpy(where, &ldr, 4);
        memcpy(where + 4, &br, 4);
        memcpy(where + 8, &target_addr, 8);
    }

    std::mutex mutex_;
    bool installed_ = false;
    uintptr_t target_ = 0;
    uintptr_t detour_ = 0;
    uint8_t original_bytes_[far_jump_size];
    size_t original_size_ = 0;
    void* trampoline_ = nullptr;
    size_t trampoline_size_ = 0;
};

// -----------------------------------------------------------------------------
// Demonstration (optional)
// -----------------------------------------------------------------------------

#ifdef HOOK_DEMO

void original_function(int x) {
    printf("Original: %d\n", x);
}

void detour_function(int x) {
    printf("Detour: %d\n", x * 2);
}

int main() {
    TrampolineHook hook;
    TrampolineHook::OriginalFunc original = nullptr;

    if (hook.install(reinterpret_cast<void*>(original_function),
                     reinterpret_cast<void*>(detour_function),
                     &original)) {
        original_function(5);   // Calls detour -> "Detour: 10"
        original(5);            // Calls original via trampoline -> "Original: 5"
    }

    hook.uninstall();
    return 0;
}

#endif // HOOK_DEMO
