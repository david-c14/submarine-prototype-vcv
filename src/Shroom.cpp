#include <string.h>
#include "SubmarinePrototype.hpp"
#include "dsp/digital.hpp"

//#include <math.h>
//#include <stdio.h>
//#include <cstdint>
//#include <cstdlib>
#include <cfloat>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#if FLT_MANT_DIG == 24
inline float Q_rsqrt(float number) {
	int32_t i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = number * 0.5f;
	y = number;
	i = * (int32_t *) &y;
	i = 0x5f3759df - (i >> 1);
	y = * (float *) &i;
	y = y * (threehalfs - (x2 * y * y));
	return y;
}
#elif FLT_MANT_DIG == 53
#pragma GCC warning "Floating point numbers might be IEEE 754 binary64 - Reciprocal Square Root function is untested"
inline float Q_rsqrt(float number) {
	int64_t i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = number * 0.5f;
	y = number;
	i = * (int64_t *) &y;
	i = 0x5FE6EB50C7B537A9 - (i >> 1);
	y = * (float *) &i;
	y = y * (threehalfs - (x2 * y * y));
	return y;
}
#else
#pragma GCC warning "Not using IEEE 754 floating point numbers - Reciprocal Square Root function will be slow"
inline float Q_rsqrt(float number) {
	return 1.0f/sqrt(number);
}
#endif
#pragma GCC diagnostic pop

#define BUFFER_SIZE 32768

struct Scope : Module {
	enum ParamIds {
		X_SCALE_PARAM,
		Y_SCALE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		X_INPUT,
		Y_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float bufferX[BUFFER_SIZE] = {};
	float bufferY[BUFFER_SIZE] = {};
	int bufferIndex = 0;

	Scope() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

};


void Scope::step() {
	// Add frame to buffer
	if (bufferIndex >= BUFFER_SIZE) bufferIndex = 0;
	bufferX[bufferIndex] = inputs[X_INPUT].value;
	bufferY[bufferIndex] = inputs[Y_INPUT].value;
	bufferIndex++;
}


struct ScopeDisplay : TransparentWidget {
	Scope *module;

	void drawWaveform(NVGcontext *vg, float *valuesX, float *valuesY) {
		if (!valuesX)
			return;
		nvgSave(vg);
		Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 15*2)));
		nvgScissor(vg, b.pos.x, b.pos.y, b.size.x, b.size.y);
		nvgLineCap(vg, NVG_ROUND);
		nvgMiterLimit(vg, 2.0f);
		nvgStrokeWidth(vg, 1.5f);
		nvgGlobalCompositeOperation(vg, NVG_LIGHTER);
		int j = module->bufferIndex;
		float oldx = valuesX[j] / 2.0f + 0.5f;
		float oldy = valuesY[j] / 2.0f + 0.5f;
		float bright = 255.0f;
		float brightdelta = 2550.0f / engineGetSampleRate();
		for (int i = 0; i < BUFFER_SIZE; i++) {
			bright -= brightdelta;
			float x, y;
			x = valuesX[j] / 2.0f + 0.5f;
			y = valuesY[j] / 2.0f + 0.5f;
			if (bright > 1.0f) {
				float dist2 = (oldx - x) * (oldx - x) + (oldy - y) * (oldy - y);
				float thisBright = bright * Q_rsqrt(dist2);
				if (thisBright > 1.0f) {
					Vec p;
					p.x = b.pos.x + b.size.x * x;
					p.y = b.pos.y + b.size.y * (1.0f - y);
					Vec o;
					o.x = b.pos.x + b.size.x * oldx;
					o.y = b.pos.y + b.size.y * (1.0f - oldy);
					nvgBeginPath(vg);
					nvgStrokeColor(vg, nvgRGBA(0x00, 0xff, 0x00, (int)(thisBright)));
					nvgMoveTo(vg, o.x, o.y);
					nvgLineTo(vg, p.x, p.y);
					nvgStroke(vg);
				}
			}
			if (j-- <= 0)
				j = BUFFER_SIZE - 1;
			oldx = x;
			oldy = y;
		}
		debug ("%f", bright);
		nvgResetScissor(vg);
		nvgRestore(vg);
	}

	void draw(NVGcontext *vg) override {
		float gainX = powf(2.0f, roundf(module->params[Scope::X_SCALE_PARAM].value));
		float gainY = powf(2.0f, roundf(module->params[Scope::Y_SCALE_PARAM].value));

		float valuesX[BUFFER_SIZE];
		float valuesY[BUFFER_SIZE];
		for (int i = 0; i < BUFFER_SIZE; i++) {
			int j = i;
			j = (i + module->bufferIndex) % BUFFER_SIZE;
			valuesX[i] = module->bufferX[j] * gainX / 10.0f;
			valuesY[i] = module->bufferY[j] * gainY / 10.0f;
		}

		// Draw waveforms
		if (module->inputs[Scope::X_INPUT].active || module->inputs[Scope::Y_INPUT].active) {
			drawWaveform(vg, valuesX, valuesY);
		}
	}
};


struct ScopeWidget : ModuleWidget {
	ScopeWidget(Scope *module);
};

struct RoundSmallBlackSnapKnob : RoundSmallBlackKnob {
	RoundSmallBlackSnapKnob() {
		snap = true;
		smooth = false;
	}
};

ScopeWidget::ScopeWidget(Scope *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Shroom.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	{
		ScopeDisplay *display = new ScopeDisplay();
		display->module = module;
		display->box.pos = Vec(0, 30);
		display->box.size = Vec(box.size.x, 300);
		addChild(display);
	}

	addParam(ParamWidget::create<RoundSmallBlackSnapKnob>(Vec(55, 332), module, Scope::X_SCALE_PARAM, -2.0f, 8.0f, 0.0f));
	addParam(ParamWidget::create<RoundSmallBlackSnapKnob>(Vec(155, 332), module, Scope::Y_SCALE_PARAM, -2.0f, 8.0f, 0.0f));

	addInput(Port::create<PJ301MPort>(Vec(25, 332), Port::INPUT, module, Scope::X_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(125, 332), Port::INPUT, module, Scope::Y_INPUT));

}


Model *modelScope = Model::create<Scope, ScopeWidget>("SubmarinePrototype", "Shroom-a-Scope", "Shroom-a-Scope", VISUAL_TAG);
