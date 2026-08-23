#include <atomic>
#include <mutex>
#include <unordered_map>
#include <string>
#include <utility>

/**
 * Thread-safe shared counter and data storage.
 *
 * The counter is implemented using std::atomic for lock-free increments and reads.
 * The data storage is protected by a std::mutex to ensure exclusive access when
 * modifying or reading the underlying container.
 *
 * This class is non-copyable and non-movable to prevent accidental sharing of
 * mutexes or atomic variables.
 */
class SharedDataManager {
public:
    SharedDataManager() = default;
    ~SharedDataManager() = default;

    // Disable copy and move semantics
    SharedDataManager(const SharedDataManager&) = delete;
    SharedDataManager& operator=(const SharedDataManager&) = delete;
    SharedDataManager(SharedDataManager&&) = delete;
    SharedDataManager& operator=(SharedDataManager&&) = delete;

    /**
     * Atomically increment the shared counter by 1.
     * This operation is lock-free and can be called from any thread.
     */
    void incrementCounter() noexcept {
        counter_.fetch_add(1, std::memory_order_relaxed);
    }

    /**
     * Atomically decrement the shared counter by 1.
     */
    void decrementCounter() noexcept {
        counter_.fetch_sub(1, std::memory_order_relaxed);
    }

    /**
     * Get the current value of the shared counter.
     * The read is atomic and lock-free.
     */
    int getCounter() const noexcept {
        return counter_.load(std::memory_order_relaxed);
    }

    /**
     * Store or update a key-value pair in the shared data storage.
     * The mutex ensures exclusive access to the map.
     *
     * @param key   The key to associate with the value.
     * @param value The value to store.
     */
    void setData(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        data_[key] = value;
    }

    /**
     * Retrieve a value from the shared data storage.
     * If the key does not exist, an empty string is returned.
     *
     * @param key The key to look up.
     * @return The stored value, or an empty string if not found.
     */
    std::string getData(const std::string& key) const {
        std::lock_guard<std::mutex> lock(data_mutex_);
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }
        return {};
    }

    /**
     * Remove a key-value pair from the shared data storage.
     *
     * @param key The key to remove.
     * @return true if the key was found and removed, false otherwise.
     */
    bool removeData(const std::string& key) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        return data_.erase(key) > 0;
    }

    /**
     * Clear all data from the shared storage.
     */
    void clearData() {
        std::lock_guard<std::mutex> lock(data_mutex_);
        data_.clear();
    }

    /**
     * Get the number of key-value pairs currently stored.
     * This acquires the mutex to safely read the size.
     */
    size_t dataSize() const {
        std::lock_guard<std::mutex> lock(data_mutex_);
        return data_.size();
    }

private:
    std::atomic<int> counter_{0};                     // Lock-free shared counter
    mutable std::mutex data_mutex_;                   // Protects data_
    std::unordered_map<std::string, std::string> data_; // Shared data storage
};
