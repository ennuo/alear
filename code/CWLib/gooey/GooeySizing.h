#ifndef GOOEY_SIZING_H
#define GOOEY_SIZING_H

struct SizingBehaviour {
    float Value;

    inline SizingBehaviour() { Value = -1.0f; }
    inline SizingBehaviour(float value) { Value = value; }

    static inline SizingBehaviour Absolute(float value) { return SizingBehaviour(value); }
    static inline SizingBehaviour Relative(float value) { return SizingBehaviour(-value); }
    static inline SizingBehaviour Contents() { return SizingBehaviour(-1.0f); }
};

#endif // GOOEY_SIZING_H