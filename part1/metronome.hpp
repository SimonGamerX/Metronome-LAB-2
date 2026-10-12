#pragma once

#include <cstddef>

class metronome
{
public:
	// Set to 4 to record the most recent 4 taps (giving 3 intervals),
	// satisfying the requirement: "At least 4 taps are required to create a valid BPM measurement."
	enum { beat_samples = 4 };

public:
	metronome()
	: m_timing(false), m_beat_count(0) {
		for (size_t i = 0; i < beat_samples; ++i) {
			m_beats[i] = 0;
		}
	}
	~metronome() {}

public:
	// Call when entering "learn" mode
	void start_timing();
	// Call when leaving "learn" mode
	void stop_timing();

	// Should only record the current time when timing
	// Insert the time at the next free position of m_beats
	void tap();

	bool is_timing() const { return m_timing; }
	// Calculate the BPM from the deltas between m_beats
	// Return 0 if there are not enough samples
	size_t get_bpm() const;

private:
	bool m_timing;

	// Insert new samples at the end of the array, removing the oldest
	size_t m_beats[beat_samples];
	size_t m_beat_count;
};
