// This file is based on the 'RadioFX' and 'QT Common' work by Thorsten Weinz,
// released under the MIT license:
//
// MIT License
// 
// Copyright (c) 2017 Thorsten Weinz
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// The original source can be found at https://github.com/thorwe/teamspeak-plugin-radiofx/blob/2e217d2c4394d4404583569b98686293cb84a529/LICENSE

#include <StdInc.h>
#include <audDspEffect.h>

#include <DspFilters/Dsp.h>

//
const float VOLUME_0DB = (0.0f);
const float VOLUME_MUTED = (-200.0f);

class DspVolume
{
public:
	explicit DspVolume();

	// Properties
	void setGainCurrent(float val);
	float getGainCurrent() const;
	void setGainDesired(float val);
	float getGainDesired() const;
	bool isProcessing() const;
	virtual void setProcessing(bool val);
	void setMuted(bool val);
	bool isMuted() const;

	virtual void process(short* samples, int sampleCount, int channels);
	virtual float GetFadeStep(int sampleCount);

protected:
	unsigned short m_sampleRate = 48000;
	void doProcess(short* samples, int sampleCount);
	bool m_isProcessing = false;

private:
	float m_gainCurrent = VOLUME_0DB; // decibels
	float m_gainDesired = VOLUME_0DB; // decibels
	bool m_muted = false;
};

const float GAIN_FADE_RATE = (400.0f); // Rate to fade at (dB per second)

DspVolume::DspVolume()
{
}

// Properties

//! Sets the current gain (dB) either set by user interaction or gain adjustment
/*!
  Intended for internal use
  \param val the current gain (dB)
*/
void DspVolume::setGainCurrent(float val)
{
	if (val != m_gainCurrent)
	{
		m_gainCurrent = val;
	}
}

//! Gets the current gain (dB) either set by user interaction or gain adjustment
/*!
  \return the current gain (dB)
*/
float DspVolume::getGainCurrent() const
{
	return m_gainCurrent;
}

//! Sets the desired gain (dB) either set by user interaction or gain adjustment
/*!
  Intended for internal use
  \param val the desired gain (dB)
*/
void DspVolume::setGainDesired(float val)
{
	if (val != m_gainDesired)
	{
		m_gainDesired = val;
	}
}

//! Gets the desired gain (dB) either set by user interaction or gain adjustment
/*!
  \return the desired gain (dB)
*/
float DspVolume::getGainDesired() const
{
	return m_gainDesired;
}

bool DspVolume::isProcessing() const
{
	return m_isProcessing;
}

void DspVolume::setProcessing(bool val)
{
	m_isProcessing = val;
}

//! Mutes the volume
/*!
 * \brief DspVolume::setMuted
 * \param value To mute or to unmute, that is the question
 */
void DspVolume::setMuted(bool val)
{
	m_muted = val;
}

//! Is the volume muted?
/*!
 * \brief DspVolume::isMuted
 * \return or am I just deaf?
 */
bool DspVolume::isMuted() const
{
	return m_muted;
}

void DspVolume::process(short* samples, int sampleCount, int channels)
{
	sampleCount = sampleCount * channels;
	setGainCurrent(GetFadeStep(sampleCount));
	doProcess(samples, sampleCount);
}

float DspVolume::GetFadeStep(int sampleCount)
{
	// compute manual gain
	float current_gain = getGainCurrent();
	float desired_gain = getGainDesired();
	if (isMuted())
	{
		float fade_step = (GAIN_FADE_RATE / m_sampleRate) * sampleCount;
		if (current_gain < VOLUME_MUTED - fade_step)
		{
			current_gain += fade_step;
		}
		else if (current_gain > VOLUME_MUTED + fade_step)
		{
			current_gain -= fade_step;
		}
		else
		{
			current_gain = VOLUME_MUTED;
		}
	}
	else if (current_gain != desired_gain)
	{
		float fade_step = (GAIN_FADE_RATE / m_sampleRate) * sampleCount;
		if (current_gain < desired_gain - fade_step)
		{
			current_gain += fade_step;
		}
		else if (current_gain > desired_gain + fade_step)
		{
			current_gain -= fade_step;
		}
		else
		{
			current_gain = desired_gain;
		}
	}
	return current_gain;
}

/*
 *  db.h
 *
 *  Copyright (C) 2003,2005 Steve Harris, Nicholas Humfrey
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  Originally Stolen from JAMIN <http://jamin.sf.net/>
 */
static inline float
db2lin_alt2(float db)
{
	if (db <= -200.0f)
		return 0.0f;
	else
		return exp(db / 20 * log(10.0f)); // went mad with ambigous call with 10 (identified as int)
}

static inline float
lin2db(float lin)
{
	if (lin == 0.0f)
		return -200.0f;
	else
		return (20.0f * log10f(lin));
}

//! Apply volume (no need to care for channels)
void DspVolume::doProcess(short* samples, int sampleCount)
{
	float mix_gain = db2lin_alt2(m_gainCurrent);
	for (int i_sample = 0; i_sample < sampleCount; ++i_sample)
	{
		int temp = samples[i_sample] * mix_gain;
		samples[i_sample] = std::clamp(temp, -32768, 32767);
	}
}

class DspVolumeAGMU : public DspVolume
{
public:
	void process(int16_t* samples, int32_t sample_count, int32_t channels);
	float GetFadeStep(int32_t sample_count);
	int16_t GetPeak() const;
	void setPeak(int16_t val); //Overwrite peak; use for reinitializations with cache values etc.
	float computeGainDesired();

	void reset_peak()
	{
		m_peak = 0;
	}

private:
	const float kRateLouder = 90.0f;
	const float kRateQuieter = 120.0f;
	int16_t m_peak = 0;
};

// Peak
static inline float getPeak(float* samples, int sampleCount)
{
	float val = 0.0f;
	for (int i = 0; i < sampleCount; ++i)
		val = std::max(std::abs(samples[i]), val);

	return val;
}

// Peak unsigned 16bit
static inline short getPeak(short* samples, int sampleCount)
{
	short val = 0.0f;
	for (int i = 0; i < sampleCount; ++i)
		val = std::max(std::abs(samples[i]), int32_t(val));

	return val;
}

void DspVolumeAGMU::process(int16_t* samples, int32_t sample_count, int32_t channels)
{
	sample_count = sample_count * channels;
	auto peak = getPeak(samples, sample_count);
	peak = std::max(m_peak, peak);
	if (peak != m_peak)
	{
		m_peak = peak;
		setGainDesired(computeGainDesired());
	}
	setGainCurrent(GetFadeStep(sample_count));
	doProcess(samples, sample_count);
}

// Compute gain change
float DspVolumeAGMU::GetFadeStep(int sampleCount)
{
	auto current_gain = getGainCurrent();
	auto desired_gain = getGainDesired();
	if (current_gain != desired_gain)
	{
		float fade_step_down = (kRateQuieter / m_sampleRate) * sampleCount;
		float fade_step_up = (kRateLouder / m_sampleRate) * sampleCount;
		if (current_gain < desired_gain - fade_step_up)
			current_gain += fade_step_up;
		else if (current_gain > desired_gain + fade_step_down)
			current_gain -= fade_step_down;
		else
			current_gain = desired_gain;
	}
	return current_gain;
}

int16_t DspVolumeAGMU::GetPeak() const
{
	return m_peak;
}

void DspVolumeAGMU::setPeak(int16_t val)
{
	m_peak = val;
}

float DspVolumeAGMU::computeGainDesired()
{
	return std::min((lin2db(32768.f / m_peak)) - 2, 12.0f); // leave some headroom
}

struct RadioFX_Settings
{
	RadioFX_Settings() = default;
	RadioFX_Settings(const RadioFX_Settings&) = delete;
	RadioFX_Settings(RadioFX_Settings&& other) noexcept
	{
		enabled.store(other.enabled.load());
		freq_hi = other.freq_hi.load();
		fudge = other.fudge.load();
		rm_mod_freq = other.rm_mod_freq.load();
		rm_mix = other.rm_mix.load();
		o_freq_lo = other.o_freq_lo.load();
		o_freq_hi = other.o_freq_hi.load();
	};

	std::atomic_bool enabled = false;
	std::atomic<double> freq_low = 0.0;
	std::atomic<double> freq_hi = 0.0;
	std::atomic<double> fudge = 0.0;
	std::atomic<double> rm_mod_freq = 0.0;
	std::atomic<double> rm_mix = 0.0;
	std::atomic<double> o_freq_lo = 0.0;
	std::atomic<double> o_freq_hi = 0.0;
};

class RadioDSPEffect : public rage::audDspEffect
{
	virtual bool Init(uint32_t, uint32_t) override;
	virtual void Shutdown() override;
	virtual void Process(rage::audDspEffectBuffer& buffers) override;
	virtual void SetParam(uint32_t param, uint32_t value) override;
	virtual void SetParam(uint32_t param, float value) override;

private:
	void do_process(float* samples, int frame_count, float& volFollow);
	void do_process_ring_mod(float* samples, int frame_count, double& modAngle);

	std::unique_ptr<Dsp::Filter> f = std::make_unique<Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass<4>, 6, Dsp::DirectFormII>>(1024);
	std::unique_ptr<Dsp::Filter> f_o = std::make_unique<Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass<4>, 6, Dsp::DirectFormII>>(1024);

	float m_vol_follow[6];
	double m_rm_mod_angle[6];

	std::pair<double, double> m_last_eq_in; // low, high
	std::pair<double, double> m_last_eq_out; // low, high

	DspVolumeAGMU dsp_volume_agmu;

	RadioFX_Settings m_settings;
};

bool RadioDSPEffect::Init(uint32_t a, uint32_t b)
{
	Dsp::Params params;
	params[0] = 48000; // sample rate
	params[1] = 4; // order
	params[2] = 1600; // center frequency
	params[3] = 1300; // band width
	f->setParams(params);
	f_o->setParams(params);

	memset(m_rm_mod_angle, 0, sizeof(m_rm_mod_angle));
	memset(m_vol_follow, 0, sizeof(m_vol_follow));

	return true;
}

void RadioDSPEffect::Shutdown()
{
	// Fixes memory leak
	delete this;
}

void RadioDSPEffect::do_process(float* samples, int frame_count, float& volFollow)
{
	// ALL INPUTS AND OUTPUTS IN THIS ARE -1.0f and +1.0f

	const auto fudge = static_cast<float>(m_settings.fudge);

	// Find volume of current block of frames...
	float vol = 0.0f;
	//   float min = 1.0f, max = -1.0f;
	for (int i = 0; i < frame_count; i++)
	{
		vol += (samples[i] * samples[i]);
	}
	vol /= (float)frame_count;

	// Fudge factor, inrease for more noise
	vol *= fudge;

	// Smooth follow from last frame, both multiplies add up to 1...
	volFollow = volFollow * 0.5f + vol * 0.5f;

	// Between -1.0f and 1.0f...
	float random = (((float)(rand() & 32767)) / 16384.0f) - 1.0f;

	// Between 1 and 128...
	int count = (rand() & 127) + 1;
	float temp;
	for (int i = 0; i < frame_count; i++)
	{
		if (!count--)
		{
			//          // Between -1.0f and 1.0f...
			random = (((float)(rand() & 32767)) / 16384.0f) - 1.0f;
			//          // Between 1 and 128...
			count = (rand() & 127) + 1;
		}
		// Add random to inputs * by current volume;
		temp = samples[i] + random * volFollow;

		// Make it an integer between -60 and 60
		temp = (int)(temp * 40.0f);

		// Drop it back down but massively quantised and too high
		temp = (temp / 40.0f);
		temp *= 0.05 * (float)m_settings.fudge;
		temp += samples[i] * (1 - (0.05 * fudge));
		samples[i] = std::clamp(-1.0f, temp, 1.0f);
	}
}

const double TWO_PI_OVER_SAMPLE_RATE = 2 * 3.141592653 / 48000;

void RadioDSPEffect::do_process_ring_mod(float* samples, int frame_count, double& modAngle)
{
	const auto rm_mod_freq = m_settings.rm_mod_freq.load();
	const auto rm_mix = m_settings.rm_mix.load();
	if ((rm_mod_freq != 0.0f) && (rm_mix != 0.0f))
	{
		for (int i = 0; i < frame_count; ++i)
		{
			float sample = samples[i];
			sample = (sample * (1 - rm_mix)) + (rm_mix * (sample * sin(modAngle)));
			samples[i] = std::clamp(-1.0f, sample, 1.0f);
			modAngle += rm_mod_freq * TWO_PI_OVER_SAMPLE_RATE;
		}
	}
}

namespace
{
double calculate_bandwidth(double low_frequency, double high_frequency)
{
	return high_frequency - low_frequency;
}
double calculate_center_frequency(double low_frequency, double high_frequency)
{
	return low_frequency + (calculate_bandwidth(low_frequency, high_frequency) / 2.0);
}
void update_filter_frequencies(
double low_frequency,
double high_frequency,
std::pair<double, double>& last,
const std::unique_ptr<Dsp::Filter>& mono_filter)
{
	if (last.first == low_frequency && last.second == high_frequency)
		return;

	// calculate center frequency and bandwidth
	const auto new_center_frequency = calculate_center_frequency(low_frequency, high_frequency);
	const auto new_bandwidth = calculate_bandwidth(low_frequency, high_frequency);
	if (mono_filter)
	{
		mono_filter->setParam(2, new_center_frequency); // center frequency
		mono_filter->setParam(3, new_bandwidth); // bandwidth
	}
	last.first = low_frequency;
	last.second = high_frequency;
}
}


void RadioDSPEffect::Process(rage::audDspEffectBuffer& buffers)
{
	if (!m_settings.enabled)
	{
		return;
	}

	update_filter_frequencies(m_settings.freq_low.load(), m_settings.freq_hi.load(), m_last_eq_in, f);
	update_filter_frequencies(m_settings.o_freq_lo.load(), m_settings.o_freq_hi.load(), m_last_eq_out, f_o);

	const auto fudge = static_cast<float>(m_settings.fudge);
	constexpr auto frame_count = 256;

	float* audioData[6];
	for (int i = 0; i < 6; i++)
	{
		if (buffers.channels[i])
		{
			audioData[i] = buffers.channels[i]->data();
		}
	}
	f->process(frame_count, audioData);

	for (int i = 0; i < 6; i++)
	{
		do_process_ring_mod(audioData[i], frame_count, m_rm_mod_angle[i]);

		if (fudge > 0.0f)
			do_process(audioData[i], frame_count, m_vol_follow[i]);
	}

	f_o->process(frame_count, audioData);

	// does this even do anything if the samples are copied out later?!
	//dsp_volume_agmu.process(samples, frame_count, channels);

	//for (int i = 0; i < frame_count; ++i)
	//	samples[i] = static_cast<int16_t>(m_samples[i] * 32768.f);
}

void RadioDSPEffect::SetParam(uint32_t param, float value)
{
	if (param == HashString("freq_low"))
	{
		m_settings.freq_low = value;
	}
	else if (param == HashString("freq_hi"))
	{
		m_settings.freq_hi = value;
	}
	else if (param == HashString("fudge"))
	{
		m_settings.fudge = value;
	}
	else if (param == HashString("rm_mod_freq"))
	{
		m_settings.rm_mod_freq = value;
	}
	else if (param == HashString("rm_mix"))
	{
		m_settings.rm_mix = value;
	}
	else if (param == HashString("o_freq_lo"))
	{
		m_settings.o_freq_lo = value;
	}
	else if (param == HashString("o_freq_hi"))
	{
		m_settings.o_freq_hi = value;
	}
}

void RadioDSPEffect::SetParam(uint32_t param, uint32_t value)
{
	if (param == HashString("enabled"))
	{
		m_settings.enabled = (value) ? true : false;
	}
	else if (param == HashString("default"))
	{
		m_settings.enabled = true;
		m_settings.freq_low = 389.0;
		m_settings.freq_hi = 3248.0;
		m_settings.fudge = 0.0;
		m_settings.rm_mod_freq = 0.0;
		m_settings.rm_mix = 0.16;
		m_settings.o_freq_lo = 348.0;
		m_settings.o_freq_hi = 4900.0;
	}
}

rage::audDspEffect* MakeRadioFX()
{
	return new RadioDSPEffect();
}
