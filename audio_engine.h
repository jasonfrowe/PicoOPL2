/**
 * audio_engine.h
 * 
 * Multi-Core Audio Engine
 * Runs on Core 1, processes MIDI events from queue and drives OPL2 synthesis
 */

#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "queue.h"

/**
 * Initialize the audio engine
 * Sets up the event queue
 * 
 * @param queue_size Number of events the queue can hold
 */
void audio_engine_init(uint16_t queue_size);

/**
 * Start the audio engine on Core 1
 * Launches the audio processing thread
 */
void audio_engine_start(void);

/**
 * Add an event to the audio engine queue
 * Blocking call - waits if queue is full
 * 
 * @param event Pointer to SongEvent to add
 */
void audio_engine_add_event(const SongEvent *event);

#endif // AUDIO_ENGINE_H
