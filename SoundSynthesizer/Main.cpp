#include <cstdio>
#include <iostream>

#include "olcNoiseMaker.h"

atomic<double> dFrequencyOutput = 0.0;

// Function used by olcNoiseMaker to generate sound waves
// Returns amplitude (-1.0 to +1.0) as a function of time
double MakeNoise(double dTime)
{
	// Sine functions doesn't take Hz as parameters, but Angular velocities, thus we multiply by it by (2 * Pi
	double dOutput = 1.0 * sin(dFrequencyOutput * (2 * PI) * dTime);// +sin((dFrequencyOutput + 20.0) * (2 * PI) * dTime);;

	//return dOutput * 0.4;

	// emulating a Square wave
	if (dOutput > 0.0)
	{
		return 0.2;
	}
	else
	{
		return -0.2;
	}
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

	const double dOctaveBaseFrequency = 110.0; // A2
	const double d12thRootOf2 = pow(2.0, 1.0 / 12.0);
	
	while (true)
	{
		// Add a keyboard like a piano
		bool bKeyPressed = false;
		for (int k = 0; k < 15; k++)
		{
			if (GetAsyncKeyState(static_cast<unsigned char>("ZSXCFVVGBNJMK\xbcL\xbe"[k])) & 0x8000)
			{
				dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
				bKeyPressed = true;
			}
		}
		if (!bKeyPressed)
		{
			dFrequencyOutput = 0.0f;
		}
	}
	
	return 0;
}
