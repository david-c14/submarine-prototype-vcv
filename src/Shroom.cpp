#include <string.h>
#include "SubmarinePrototype.hpp"
#include "dsp/digital.hpp"

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
	float oldx = 0;
	float oldy = 0;

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
		for (int i = 0; i < BUFFER_SIZE; i++) {
			float x, y;
			x = valuesX[j] / 2.0f + 0.5f;
			y = valuesY[j] / 2.0f + 0.5f;
			float dist = (oldx - x) * (oldx - x) + (oldy - y) * (oldy - y);
			dist = sqrt(dist);
			
			Vec p;
			p.x = b.pos.x + b.size.x * x;
			p.y = b.pos.y + b.size.y * (1.0f - y);
			Vec o;
			o.x = b.pos.x + b.size.x * oldx;
			o.y = b.pos.y + b.size.y * (1.0f - oldy);
			nvgBeginPath(vg);
			nvgStrokeColor(vg, nvgRGBA(0x00, 0xff, 0x00, (int)(0xff / dist)));
			nvgMoveTo(vg, o.x, o.y);
			nvgLineTo(vg, p.x, p.y);
			nvgStroke(vg);
			if (j++ >= BUFFER_SIZE)
				j = 0;
			oldx = x;
			oldy = y;
		}
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
