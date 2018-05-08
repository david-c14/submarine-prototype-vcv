#include "SubmarinePrototype.hpp"

struct Tunings {
	
	enum Tunings {
		EQUAL,
		PYTHAGOREAN,
		WERCKMEISTER_III,
		YOUNG,
		NUM_TUNINGS
	};

	static const float tunings[3][12] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 13.7f, 3.9f, -5.9f, 7.8f, -2.0f, 11.7f, 2.0f, -7.8f, 5.9f, -3.9f, 9.8f },
		{ -9.775f, -7.82f, -5.865f, -9.775f, -1.955f, -11.73f, -3.94f, -7.82f, -11.93f, -3.91f, -7.82f },
		{ -6.1f, -4.2f, -2.2f, -8.3f, -0.1f, -8.1f, -2.1f, -4.2f, -6.2f, -0.2f, -8.2f }
	};
}


struct CrossFader : Module {
	enum ParamIds {
		FADE_PARAM,
		GAIN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		A_INPUT,
		B_INPUT,
		FADE_CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		FADE_OUTPUT,
		CORREL_OUTPUT,
		LIN_A_OUTPUT,
		LIN_B_OUTPUT,
		FADE_LIN_OUTPUT,
		LOG_A_OUTPUT,
		LOG_B_OUTPUT,
		FADE_LOG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};


	float samples_a[256];
	float samples_b[256];
	int n = 0;
	int sp = 0;
	float covariance = 0;
	float sigma_a = 0;
	float sigma_b = 0;
	float sigma_a2 = 0;
	float sigma_b2 = 0;
	
	
	CrossFader() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	float correlation(float, float);

};


void CrossFader::step() {
	float a = inputs[A_INPUT].value;
	float b = inputs[B_INPUT].value;
	float fader = params[FADE_PARAM].value;
	if (inputs[FADE_CV_INPUT].active)
		fader = inputs[FADE_CV_INPUT].value;
	fader += 5.0f;
	fader /= 10.0f;
	fader = clamp(fader, 0.0f, 1.0f);
	float b_fade_lin = fader;
	float a_fade_lin = 1.0f - b_fade_lin;
	float b_fade_log = powf(fader, 0.5f);
	float a_fade_log = powf(1.0f - fader, 0.5f);
	float fade_output_lin = a * a_fade_lin + b * b_fade_lin;
	float fade_output_log = a * a_fade_log + b * b_fade_log;
	if (params[GAIN_PARAM].value < 0.5f) {
		outputs[FADE_OUTPUT].value = fade_output_log;
	}
	else {
		outputs[FADE_OUTPUT].value = fade_output_lin;
	}
	outputs[CORREL_OUTPUT].value = correlation(a,b);
	outputs[LIN_A_OUTPUT].value = a * a_fade_lin;
	outputs[LIN_B_OUTPUT].value = b * b_fade_lin;
	outputs[LOG_A_OUTPUT].value = a * a_fade_log;
	outputs[LOG_B_OUTPUT].value = b * b_fade_log; 
	outputs[FADE_LIN_OUTPUT].value = fade_output_lin;
	outputs[FADE_LOG_OUTPUT].value = fade_output_log;
}

float CrossFader::correlation(float a, float b) {
	//Remove old samples
	if (n == 256) {
		covariance -= (samples_a[sp] * samples_b[sp]);
		sigma_a -= samples_a[sp];
		sigma_b -= samples_b[sp];
		sigma_a2 -= (samples_a[sp] * samples_a[sp]);
		sigma_b2 -= (samples_b[sp] * samples_b[sp]);
	}
	else {
		n++;
	}
	//Add new samples
	covariance += (a * b);
	sigma_a += samples_a[sp] = a;
	sigma_b += samples_b[sp] = b;
	sigma_a2 += (a * a);
	sigma_b2 += (b * b);
	sp++;
	if (sp > 255)
		sp -= 256;
	float stdev_a = powf(sigma_a2 - (sigma_a * sigma_a / n), 0.5f);
	float stdev_b = powf(sigma_b2 - (sigma_b * sigma_b / n), 0.5f);
	if (stdev_a * stdev_b == 0.0f)
		return (stdev_a == stdev_b);
	return covariance / (stdev_a * stdev_b);
}


struct CrossFaderWidget : ModuleWidget {
	CrossFaderWidget(CrossFader *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CrossFader.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<Davies1900hLargeBlackKnob>(Vec(50, 85), module, CrossFader::FADE_PARAM, -5.0, 5.0, 0.0));
		addParam(ParamWidget::create<CKSS>(Vec(30, 102.5), module, CrossFader::GAIN_PARAM, 0.0f, 1.0f, 1.0f));

		addInput(Port::create<PJ301MPort>(Vec(25, 75), Port::INPUT, module, CrossFader::A_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(105, 75), Port::INPUT, module, CrossFader::B_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(25, 125), Port::INPUT, module, CrossFader::FADE_CV_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(105, 125), Port::OUTPUT, module, CrossFader::FADE_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(105, 200), Port::OUTPUT, module, CrossFader::CORREL_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(35, 250), Port::OUTPUT, module, CrossFader::LIN_A_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(70, 250), Port::OUTPUT, module, CrossFader::LIN_B_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(105, 250), Port::OUTPUT, module, CrossFader::FADE_LIN_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(35, 300), Port::OUTPUT, module, CrossFader::LOG_A_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(70, 300), Port::OUTPUT, module, CrossFader::LOG_B_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(105, 300), Port::OUTPUT, module, CrossFader::FADE_LOG_OUTPUT));

	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelCrossFader = Model::create<CrossFader, CrossFaderWidget>("SubmarinePrototype", "CrossFader", "Cross Fader", ATTENUATOR_TAG, MIXER_TAG);
