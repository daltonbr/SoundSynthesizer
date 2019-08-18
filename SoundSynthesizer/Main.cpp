#include <cstdio>
#include <iostream>

#include "olcNoiseMaker.h"


// Function used by olcNoiseMaker to generate sound waves
// Returns amplitude (-1.0 to +1.0) as a function of time
double MakeNoise(double dTime)
{
	// Sine functions doesn't take Hz as parameters, but Angular velocities, thus we (2 * Pi)
	// Playing A 440Hz
	return 0.5 * sin(440.0 * (2 * PI) * dTime);
}

int main()
{
	std::wcout << "Sound Synthesizer" << std::endl;

	// Get all sound hardware
	std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto d : devices) std::wcout << "Found Output Device:" << d << std::endl;

	// Create sound machine
	// the number of bits (T, normally short (16 bits) or char(8 bits)) accuracy of the Amplitude
	// the Sample rate (44100 Hz) accuracy of Frequency - double of the highest frequency of the human hearing range (22 Hz ~ 22 KHz)
	// Channels (mono or stereo)
	// Latency Management: Blocks and BlockSamples
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	// Link noise function with sound machine
	sound.SetUserFunction(MakeNoise);
	

	while (true)
	{
		
	}
	
	return 0;
}
