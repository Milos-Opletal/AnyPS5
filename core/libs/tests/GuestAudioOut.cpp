#include "prx/libSceAudioOut/include/AudioOut.hpp"
#include "SceTypes.hpp"
#include "prx/libc/include/Shutdown.hpp"
#include <cstdlib>
#include <stdexcept>

static void Require(bool value) {
    if (!value) {
        std::abort();
    }
}

int main() {
    constexpr int invalidPointer = static_cast<int>(0x80260004);
    constexpr int invalidHandle = static_cast<int>(0x80260001);

    Require(sceAudioOutInit() == 0);
    Require(sceAudioOutGetPortState(invalidHandle, nullptr) == invalidPointer);
    Require(sceAudioOutGetPortState(0, nullptr) == invalidPointer);

    AudioOut2MasteringStates states{};
    states.states_header.states_id = 1;
    for (size_t c = 0; c < 3; ++c) {
        for (size_t i = 0; i < 8; ++i) {
            states.compressor_states.input_rms[c][i] = 1.0f;
            states.compressor_states.compression_coeff[c][i] = 1.0f;
        }
    }
    for (size_t i = 0; i < 8; ++i) {
        states.limiter_states.input_peak[i] = 1.0f;
        states.limiter_states.output_peak[i] = 1.0f;
        states.limiter_states.gain_peak[i] = 1.0f;
    }
    Require(sceAudioOutMasteringGetState(&states.states_header) == 0);
    for (size_t c = 0; c < 3; ++c) {
        for (size_t i = 0; i < 8; ++i) {
            Require(states.compressor_states.input_rms[c][i] == 0.0f);
            Require(states.compressor_states.compression_coeff[c][i] == 0.0f);
        }
    }
    for (size_t i = 0; i < 8; ++i) {
        Require(states.limiter_states.input_peak[i] == 0.0f);
        Require(states.limiter_states.output_peak[i] == 0.0f);
        Require(states.limiter_states.gain_peak[i] == 0.0f);
    }

    bool threw = false;
    try {
        sceAudioOutMasteringGetState(nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);

    threw = false;
    AudioOut2MasteringStates invalidState{};
    invalidState.states_header.states_id = 999;
    try {
        sceAudioOutMasteringGetState(&invalidState.states_header);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);

    LibcRunShutdown_nid_postfix();
    return 0;
}
