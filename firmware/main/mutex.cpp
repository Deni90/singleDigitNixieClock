/******************************************************************************
 * File:    mutex.cpp
 * Author:  Daniel Knezevic
 * Year:    2025
 * Brief:   Implements Mutex class
 ******************************************************************************/

#include "mutex.h"

Mutex::Mutex() { mMutex = xSemaphoreCreateMutex(); }

Mutex::~Mutex() { vSemaphoreDelete(mMutex); }

void Mutex::lock() { xSemaphoreTake(mMutex, portMAX_DELAY); }

void Mutex::unlock() { xSemaphoreGive(mMutex); }

SemaphoreHandle_t Mutex::handle() { return mMutex; }