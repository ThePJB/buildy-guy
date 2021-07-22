#pragma once

struct vec2 {
    float x = 0.0f;
    float y = 0.0f;

    vec2(float x, float y): x{x}, y{y} {};

    // dot dist etc

    vec2 operator+(const vec2& p) {
    return vec2(x + p.x, y + p.y);
    }

    vec2 operator*(const vec2& p) {
        return vec2(x * p.x, y * p.y);
    }

    vec2 operator*(float f) {
        return vec2(x * f, y * f);
    }
};

inline vec2 operator*(float f, const vec2& p) {
    return vec2(p.x * f, p.y * f);    
}