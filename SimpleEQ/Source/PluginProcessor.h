/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope {
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48

};

struct ChainSettings {
    float peakFreq{ 0 }, peakGainInDecibels{ 0 }, peakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    Slope lowCutSlope { Slope::Slope_12 }, highCutSlope { Slope::Slope_12 };
};
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& aptvs);
//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout();
    juce::AudioProcessorValueTreeState aptvs{ *this, nullptr, "Parameters", createParameterLayout()};

private:

    using Filter = juce::dsp::IIR::Filter<float>;

    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

    MonoChain leftChain, rightChain;

    enum ChainPositions {
        LowCut,
        Peak,
        HighCut,

    };

    void updatePeakFilter(const ChainSettings& chainSettings);
    using Coefficients = Filter::CoefficientsPtr;
    static void replaceCoefficients(Coefficients& old, Coefficients& replacements);
   
	template<int index, typename ChainType, typename CoefficientsType>
    void update(ChainType& chain, const CoefficientsType& coefficients) {
        replaceCoefficients(chain.template get<index>().coefficients, coefficients[index]);
        chain.template setBypassed<index>(false);
    };
    template<typename ChainType, typename CoefficientsType>
    void updateCutCoefficients(ChainType& chainType, const CoefficientsType& cutCoefficients, Slope cutSlope) {
		chainType.template setBypassed<0>(true);
		chainType.template setBypassed<1>(true);
		chainType.template setBypassed<2>(true);
		chainType.template setBypassed<3>(true);
        switch (cutSlope) {
        case Slope_48: {
            update<3>(chainType, cutCoefficients);
        }
        case Slope_36: {
            
            update<2>(chainType, cutCoefficients);

        }
        case Slope_24: {

            update<1>(chainType, cutCoefficients);
        }
        case Slope_12: {

            update<0>(chainType, cutCoefficients);
        }
        }

            /*
		case Slope_12: {
			*chainType.template get<0>().coefficients = *cutCoefficients[0];
			chainType.template setBypassed<0>(false);
			break;
		}
		case Slope_24: {
			*chainType.template get<0>().coefficients = *cutCoefficients[0];
			chainType.template setBypassed<0>(false);
			*chainType.template get<1>().coefficients = *cutCoefficients[1];
			chainType.template setBypassed<1>(false);
			break;
		}
		case Slope_36: {
			*chainType.template get<0>().coefficients = *cutCoefficients[0];
			chainType.template setBypassed<0>(false);
			*chainType.template get<1>().coefficients = *cutCoefficients[1];
		    chainType.template setBypassed<1>(false);
			*chainType.template get<2>().coefficients = *cutCoefficients[2];
			chainType.template setBypassed<2>(false);
			break;
		}
		case Slope_48: {
			*chainType.template get<0>().coefficients = *cutCoefficients[0];
		    chainType.template setBypassed<0>(false);
			*chainType.template get<1>().coefficients = *cutCoefficients[1];
			chainType.template setBypassed<1>(false);
			*chainType.template get<2>().coefficients = *cutCoefficients[2];
			chainType.template setBypassed<2>(false);
			*chainType.template get<3>().coefficients = *cutCoefficients[3];
			chainType.template setBypassed<3>(false);
			break;
		}
		}
        */
    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
