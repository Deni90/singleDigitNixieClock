/******************************************************************************
 * File:    mutex.h
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Declaration of a Mutex wrapper class
 ******************************************************************************/

#ifndef mutex_h
#define mutex_h

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @brief RAII wrapper for a FreeRTOS mutex, compatible with std::lock_guard
 *
 * Provides lock() and unlock() methods so the mutex can be used with
 * std::lock_guard and other C++ RAII-based synchronization helpers.
 *
 * * @note
 * - This class is not copyable or movable.
 * - lock() blocks indefinitely (portMAX_DELAY).
 */
class Mutex {
  public:
    /**
     * @brief Construct the mutex.
     *
     * Creates a FreeRTOS mutex using xSemaphoreCreateMutex().
     */
    Mutex();

    /**
     * @brief Destroy the mutex.
     *
     * Deletes the underlying FreeRTOS mutex.
     */
    ~Mutex();

    /// @brief Non-copyable
    Mutex(const Mutex&) = delete;

    /// @brief Non-moveable
    Mutex(const Mutex&&) = delete;

    /// @brief Copy assignment disabled
    Mutex& operator=(const Mutex&) = delete;

    /// @brief Move assignment disabled
    Mutex& operator=(const Mutex&&) = delete;

    /**
     * @brief Lock the mutex.
     *
     * Blocks indefinitely until the mutex becomes available.
     */
    void lock();

    /**
     * @brief Unlock the mutex.
     */
    void unlock();

    /**
     * @brief Get the underlying FreeRTOS semaphore handle.
     *
     * @return SemaphoreHandle_t associated with this mutex.
     *
     * @warning Exposing the handle bypasses RAII guarantees.
     */
    SemaphoreHandle_t handle();

  private:
    SemaphoreHandle_t mMutex;
};

#endif   // mutex_h