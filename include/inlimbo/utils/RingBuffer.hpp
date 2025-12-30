#pragma once

#include <cstddef>
#include <vector>

struct RingBuffer {

    std::vector<float> data;
    size_t r = 0, w = 0;
    size_t size = 0;

    explicit RingBuffer(size_t frames = 0) {
        resize(frames);
    }

    void resize(size_t frames) {
        data.assign(frames * 2, 0.0f); // stereo float
        size = frames;
        r = w = 0;
    }

    [[nodiscard]] auto available() const -> size_t {
        return (w >= r) ? (w - r) : (size - (r - w));
    }

    [[nodiscard]] auto free() const -> size_t {
        return size - available() - 1;
    }

    void write(const float* in, size_t frames) {
        for (size_t i = 0; i < frames * 2; ++i) {
            data[(w * 2 + i) % (size * 2)] = in[i];
        }
        w = (w + frames) % size;
    }

    void read(float* out, size_t frames) {
        for (size_t i = 0; i < frames * 2; ++i) {
            out[i] = data[(r * 2 + i) % (size * 2)];
        }
        r = (r + frames) % size;
    }

    void clear() {
        r = w = 0;
    }
};
