#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#include <assert.h>

#include "utils/gpio_utils.h"
#include "utils/time_utils.h"

#include "hal/distance_sensor.h"

#define ECHO_PIN "p8.10"
#define ECHO_GPIO "68"

#define TRIGGER_PIN "p8.15"
#define TRIGGER_GPIO "47"

#define TRIGGER_HIGH "1"
#define TRIGGER_LOW "0"

#define SAMPLE_SIZE 7

static bool is_initialized = false;
static bool is_running = false;

float distance = 0.0;

pthread_t sensorThread;

// Helper function to compare two float values, used in qsort
static int compareFloats(const void* a, const void* b)
{
    float fa = *(const float*)a;
    float fb = *(const float*)b;
    return (fa < fb) - (fa > fb);
}

static float medianFilter(float newMeasurement)
{
    static float measurements[SAMPLE_SIZE];
    static int count = 0;
    int index = count % SAMPLE_SIZE;
    measurements[index] = newMeasurement;
    count++;

    if (count < SAMPLE_SIZE) return newMeasurement; // Not enough data yet

    float sortedMeasurements[SAMPLE_SIZE];
    memcpy(sortedMeasurements, measurements, sizeof(measurements));
    qsort(sortedMeasurements, SAMPLE_SIZE, sizeof(float), compareFloats);

    // If SAMPLE_SIZE is odd
    if (SAMPLE_SIZE % 2 != 0) {
        return sortedMeasurements[SAMPLE_SIZE / 2];
    }

    // If SAMPLE_SIZE is even
    return (sortedMeasurements[SAMPLE_SIZE / 2 - 1] + sortedMeasurements[SAMPLE_SIZE / 2]) / 2.0;
}

static float exponentialMovingAverage(float newMeasurement)
{
    static float ema = 0.0;
    static bool isInitialized = false;
    const float alpha = 0.5;

    if (!isInitialized) {
        ema = newMeasurement;
        isInitialized = true;
        return ema;
    }

    ema = alpha * newMeasurement + (1 - alpha) * ema;
    return ema;
}

static float filterDistance(float newMeasurement) {
    float medianFiltered = medianFilter(newMeasurement);
    return exponentialMovingAverage(medianFiltered);
}

float DistanceSensor_getRawDistance()
{
    assert(is_initialized);
    setGpioValue(TRIGGER_GPIO, TRIGGER_HIGH);

    sleepForNs(10000);
    setGpioValue(TRIGGER_GPIO, TRIGGER_LOW);

    long long timeout = 25000;
    long long startWaitTime = getTimeInUs();
    long long startTime = 0, endTime = 0;

    while (getGpioValue(ECHO_GPIO) == 0) {
        startTime = getTimeInUs();
        if((startTime - startWaitTime) > timeout) {
            printf("Echo pulse start wait timeout\n");
            return -1;
        }
    }

    while(getGpioValue(ECHO_GPIO) == 1) {
        endTime = getTimeInUs();
        if ((endTime - startWaitTime) > timeout) {
            printf("Echo pulse end wait timeout\n");
            return -1; // Indicate a timeout error
        }
    }

    float pulseDuration = (endTime - startTime) / 1000000.0f;
    float distance = (pulseDuration * 34300) / 2.0f;

    // Check if the distance is within the sensor's range
    if (distance < 2.0f || distance > 450.0f) {
        return -1;
    }

    return distance;
}


static void *sensorThreadFunction(void* arg)
{
    (void) arg;
    float rawDist;

    while(is_running) {
        rawDist = DistanceSensor_getRawDistance();

        if (rawDist == -1) {
            //printf("Measurement error or out of range\n");
            continue; // Skip this loop iteration on error
        }

        distance = rawDist;
        usleep(60000);
    }

    return NULL;
}

float DistanceSensor_getDistance(void) {
    return filterDistance(distance);
}

void DistanceSensor_init()
{
    assert(!is_initialized);

    exportGpioPin(ECHO_GPIO);
    exportGpioPin(TRIGGER_GPIO);

    configurePinGpio(ECHO_PIN);
    configurePinGpio(TRIGGER_PIN);

    setGpioDirection(ECHO_GPIO, "in");
    setGpioDirection(TRIGGER_GPIO, "out");

    is_initialized = true;
    is_running = true;

    pthread_create(&sensorThread, NULL, sensorThreadFunction, NULL);
}

void DistanceSensor_cleanup()
{
    assert(is_initialized);

    is_initialized = false;
    is_running = false;

    pthread_join(sensorThread, NULL);
}

