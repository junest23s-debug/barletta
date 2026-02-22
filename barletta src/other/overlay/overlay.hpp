#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui.h"
#include "../libraries/opus/include/opus.h"

#include "d3d9.h"
#include "tchar.h"
#include <string> // For std::string

static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Window handle declaration
extern HWND hwnd;
extern bool show_imgui_window;

// Reverb effect class forward declaration
class FreeverbReverb;
extern FreeverbReverb reverbProcessor;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Configuration functions
void CreateConfiguration(const char* file_path);
void SaveConfiguration(const char* file_path);
void LoadConfiguration(const char* file_path);
void ResetConfiguration();
const char* GetKeyName(int key);
void ChangeHotkey(int newKey);

// Audio processing functions
void ApplyAudioEffects(float* audioBuffer, int bufferSize, int channels);
extern "C" opus_int32 custom_opus_encode(OpusEncoder *st, const opus_int16 *pcm, int frame_size,
                                   unsigned char *data, opus_int32 max_data_bytes);

namespace utilities::ui {
	void start();
}

// Spoofing feature variables
namespace Spoofing {
    extern bool ProcessIsolation;
    extern bool Hider;
}

// Background template structure
struct BackgroundTemplate {
    const char* name;
    void (*renderFunc)(const ImVec2&, const ImVec2&, float, float);
    bool requiresUpdate;
    float animationSpeed;
};

// Animation utilities
namespace Animation {
    // Easing functions
    float EaseInOutQuad(float t);
    float EaseOutElastic(float t);
    float EaseInOutBack(float t);
    float EaseOutBounce(float t);
    
    // Animation state tracking
    struct AnimationState {
        float value = 0.0f;
        float target = 0.0f;
        float speed = 1.0f;
        float time = 0.0f;
        
        void Update(float deltaTime, bool isActive) {
            if (isActive) {
                target = 1.0f;
            } else {
                target = 0.0f;
            }
            
            float direction = (target > value) ? 1.0f : -1.0f;
            value += direction * speed * deltaTime;
            value = (direction > 0) ? fminf(value, target) : fmaxf(value, target);
            time += deltaTime;
        }
        
        float GetValue() const { return value; }
    };
}

// Background template implementations
void RenderDefaultBackground(const ImVec2& min, const ImVec2& max, float alpha, float time);
void RenderParticleBackground(const ImVec2& min, const ImVec2& max, float alpha, float time);
void RenderGridBackground(const ImVec2& min, const ImVec2& max, float alpha, float time);
void RenderGradientBackground(const ImVec2& min, const ImVec2& max, float alpha, float time);
void RenderNoiseBackground(const ImVec2& min, const ImVec2& max, float alpha, float time);
void RenderWaveBackground(const ImVec2& min, const ImVec2& max, float alpha, float time);

// Forward declarations for animation functions
float AnimateSin(float speed, float min, float max, float phase);
float AnimateExpo(float speed, float min, float max);
float AnimateElastic(float speed, float min, float max);
void AnimateBorderCorners(const ImVec2& topLeft, const ImVec2& bottomRight, const ImVec4& color, float thickness);
bool AnimatedButton(const char* label, const ImVec2& size);
void AnimatedSlider(const char* label, float* value, float min, float max, const char* tooltip);
bool AnimatedToggleButton(const char* label, bool* value);
bool AnimatedPanningSlider(const char* label, float* value, float min, float max);
void DrawAdvancedSeparator(bool rgbMode, float widthFactor, float yFactor);
void DrawEnhancedPanel(const char* label, float alpha);
void EndEnhancedPanel(bool rgbMode);
bool AnimatedProfessionalSlider(const char* label, float* value, float min, float max, const char* format = "%.2f", const char* tooltip = nullptr);
bool PremiumButton(const char* label, const ImVec2& size);
void DrawInnerContentBorder();