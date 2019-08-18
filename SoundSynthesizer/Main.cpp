#include <cstdio>
#include <iostream>

#include "olcNoiseMaker.h"

// ADSR - Attack, Decay, Sustain, Release
struct sEnvelopeADSR
{
	double dAttackTime;
	double dDecayTime;
	double dReleaseTime;

	double dSustainAmplitude;
	double dStartAmplitude;

	double dTriggerOnTime;
	double dTriggerOffTime;

	bool bNoteOn;

	sEnvelopeADSR()
	{
		dAttackTime = 0.02;
		dDecayTime = 0.01;
		dStartAmplitude = 1.0;
		dSustainAmplitude = 0.8;
		dReleaseTime = 0.02;
		dTriggerOnTime = 0.0;
		dTriggerOffTime = 0.0;
	}

	double  GetAmplitude(double dTime)
	{
		double dAmplitude = 0.0;
		double dLifeTime = dTime - dTriggerOnTime;

		if (bNoteOn)
		{
			// ADS

			// Attack
			if (dLifeTime <= dAttackTime)
			{
				dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;
			}

			// Decay
			if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime))
			{
				dAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;
			}

			// Sustain
			if (dLifeTime > (dAttackTime + dDecayTime))
			{
				dAmplitude = dSustainAmplitude;
			}
		}
		else
		{
			// R - Release
			dAmplitude = ((dTime - dTriggerOffTime) / dReleaseTime) * (0.0 - dSustainAmplitude) + dSustainAmplitude;
		}

		// Epsilon check
		if (dAmplitude <= 0.0001)
		{
			dAmplitude = 0;
		}

		return dAmplitude;
	}

	void NoteOn(double dTimeOn)
	{
		dTriggerOnTime = dTimeOn;
		bNoteOn = true;
	}

	void NoteOff(double dTimeOff)
	{
		dTriggerOffTime = dTimeOff;
		bNoteOn = false;
	}
};

// Global synthetizer variables
atomic<double> dFrequencyOutput = 0.0;					// dominant output frequency of instrument, i.e. the note
sEnvelopeADSR envelope;									// amplitude modulation of output to give texture i.e. the timbre
const double dOctaveBaseFrequency = 110.0; // A2		// frequency of octave represented by keyboard
const double d12thRootOf2 = pow(2.0, 1.0 / 12.0); // assuming western 12 notes per octave

// Converts frequency (Hz) to angular velocity
double w(double dHertz)
{
	return dHertz * 2.0 * PI;
}

// General purpose oscillator
#define OSC_SINE		0
#define OSC_SQUARE		1
#define OSC_TRIANGLE	2
#define OSC_SAW_ANA		3
#define OSC_SAW_DIG		4
#define OSC_NOISE		5

double osc(double dHertz, double dTime, int nType)
{
	switch(nType)
	{
	case OSC_SINE: // Sine Wave
		return sin(w(dHertz) * dTime);
	case OSC_SQUARE: // Square Wave
		return sin(w(dHertz) * dTime) > 0.0 ? 1.0 : -1.0;
	case OSC_TRIANGLE: // Triangle Wave
		return asin(sin(w(dHertz) * dTime) * 2.0 / PI);
	case OSC_SAW_ANA: // Saw Wave (analogue / warm / slow)
	{
		double dOutput = 0.0;

		for (double n = 1.0; n < 10.0; n++)
		{
			dOutput += (sin(n * w(dHertz) * dTime)) / n;
		}
		return dOutput * (2.0 / PI);
	}
	case OSC_SAW_DIG: // Saw Wave (optimized / harsh / fast)
		return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));

	case OSC_NOISE: // Pseudo Random Noise
		return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;
	default:
		return 0.0;
	}
}

// Function used by olcNoiseMaker to generate sound waves
// Returns amplitude (-1.0 to +1.0) as a function of time
double MakeNoise(double dTime)
{
	// Sine functions doesn't take Hz as parameters, but Angular velocities, thus we multiply by it by (2 * Pi
	//double dOutput = envelope.GetAmplitude(dTime) * osc(dFrequencyOutput, dTime, 1);

	double dOutput = envelope.GetAmplitude(dTime) *
			(
			+ osc(dFrequencyOutput * 0.5, dTime, 3)
			+ osc(dFrequencyOutput * 1.0, dTime, 1)
			);
	
	return dOutput * 0.4; // Master Volume
}

int main()
{
	std::wcout << "Sound Synthesizer" << std::endl;

	// Get all sound hardware
	std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto d : devices) std::wcout << "Found Output Device:" << d << std::endl;

	// Display a keyboard
	std::wcout << std::endl <<
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |" << endl <<
		"|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << endl <<
		"|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << endl <<
		"|     |     |     |     |     |     |     |     |     |     |" << endl <<
		"|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << endl <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << endl << endl;
	
	// Create sound machine
	// the number of bits (T, normally short (16 bits) or char(8 bits)) accuracy of the Amplitude
	// the Sample rate (44100 Hz) accuracy of Frequency - double of the highest frequency of the human hearing range (22 Hz ~ 22 KHz)
	// Channels (mono or stereo)
	// Latency Management: Blocks and BlockSamples
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	// Link noise function with sound machine
	sound.SetUserFunction(MakeNoise);

	// Sit in loop, capturing keyboard state changes and modify
	// synthesizer output accordingly
	int nCurrentKey = -1;
	bool bKeyPressed = false;
	while (true)
	{
		bKeyPressed = false;
		for (int k = 0; k < 16; k++)
		{
			if (GetAsyncKeyState(static_cast<unsigned char>("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k])) & 0x8000)
			{
				if (nCurrentKey != k)
				{
					dFrequencyOutput = dOctaveBaseFrequency * pow(d12thRootOf2, k);
					envelope.NoteOn(sound.GetTime());
					std::wcout << "\rNote On: " << sound.GetTime() << "s " << dFrequencyOutput << std::endl;
					nCurrentKey = k;
				}
				
				bKeyPressed = true;
			}
		}
		if (!bKeyPressed)
		{
			if (nCurrentKey != -1)
			{
				std::wcout << "\rNote Off: " << sound.GetTime() << "s" << std::endl;
				envelope.NoteOff(sound.GetTime());
				nCurrentKey = -1;
			}
		}
	}
	
	return 0;
}
