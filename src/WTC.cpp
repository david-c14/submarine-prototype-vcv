#include "SubmarinePrototype.hpp"

struct Tunings {
	
	enum TuningNames {
		EQUAL,
		PYTHAGOREAN,
		WERCKMEISTER_III,
		YOUNG,
		NUM_TUNINGS
	};
	

	static constexpr float tuningSets[4][12] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 13.7f, 3.9f, -5.9f, 7.8f, -2.0f, 11.7f, 2.0f, -7.8f, 5.9f, -3.9f, 9.8f },
		{ 0.0f, -9.775f, -7.82f, -5.865f, -9.775f, -1.955f, -11.73f, -3.94f, -7.82f, -11.93f, -3.91f, -7.82f },
		{ 0.0f, -6.1f, -4.2f, -2.2f, -8.3f, -0.1f, -8.1f, -2.1f, -4.2f, -6.2f, -0.2f, -8.2f }
	};
};

constexpr float Tunings::tuningSets[][12];

struct WTC_Nano : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float tunings[12] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	WTC_Nano() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
};


void WTC_Nano::step() {
	int quantized = floor((12.0f * inputs[INPUT].value) + 0.5f);
	int note = (120 + quantized) % 12;
	outputs[OUTPUT].value = (tunings[note] / 1200.0f) + (quantized / 12.0f);
}

json_t *WTC_Nano::toJson(void) {
	json_t *rootJ = json_object();
	json_object_set_new(rootJ, "tuning0", json_real(tunings[0]));
	json_object_set_new(rootJ, "tuning1", json_real(tunings[1]));
	json_object_set_new(rootJ, "tuning2", json_real(tunings[2]));
	json_object_set_new(rootJ, "tuning3", json_real(tunings[3]));
	json_object_set_new(rootJ, "tuning4", json_real(tunings[4]));
	json_object_set_new(rootJ, "tuning5", json_real(tunings[5]));
	json_object_set_new(rootJ, "tuning6", json_real(tunings[6]));
	json_object_set_new(rootJ, "tuning7", json_real(tunings[7]));
	json_object_set_new(rootJ, "tuning8", json_real(tunings[8]));
	json_object_set_new(rootJ, "tuning9", json_real(tunings[9]));
	json_object_set_new(rootJ, "tuning10", json_real(tunings[10]));
	json_object_set_new(rootJ, "tuning11", json_real(tunings[11]));
	return rootJ;
}

void WTC_Nano::fromJson(json_t *rootJ) {
	json_t *j0 = json_object_get(rootJ, "tuning0");
	if (j0)
		tunings[0] = json_number_value(j0);
	json_t *j1 = json_object_get(rootJ, "tuning1");
	if (j1)
		tunings[1] = json_number_value(j1);
	json_t *j2 = json_object_get(rootJ, "tuning2");
	if (j2)
		tunings[2] = json_number_value(j2);
	json_t *j3 = json_object_get(rootJ, "tuning3");
	if (j3)
		tunings[3] = json_number_value(j3);
	json_t *j4 = json_object_get(rootJ, "tuning4");
	if (j4)
		tunings[4] = json_number_value(j4);
	json_t *j5 = json_object_get(rootJ, "tuning5");
	if (j5)
		tunings[5] = json_number_value(j5);
	json_t *j6 = json_object_get(rootJ, "tuning6");
	if (j6)
		tunings[6] = json_number_value(j6);
	json_t *j7 = json_object_get(rootJ, "tuning7");
	if (j7)
		tunings[7] = json_number_value(j7);
	json_t *j8 = json_object_get(rootJ, "tuning8");
	if (j8)
		tunings[8] = json_number_value(j8);
	json_t *j9 = json_object_get(rootJ, "tuning9");
	if (j9)
		tunings[9] = json_number_value(j9);
	json_t *j10 = json_object_get(rootJ, "tuning10");
	if (j10)
		tunings[10] = json_number_value(j10);
	json_t *j11 = json_object_get(rootJ, "tuning11");
	if (j11)
		tunings[11] = json_number_value(j11);
}

struct WTC_MenuItem : MenuItem {
	WTC_Nano *module;
	int setting;
	void onAction(EventAction &e) override;
};	

void WTC_MenuItem::onAction(EventAction &e) {
	for (int i = 0; i < 12; i++) {
		module->tunings[i] = Tunings::tuningSets[setting][i];
	}
}

struct WTC_NanoWidget : ModuleWidget {
	WTC_NanoWidget(WTC_Nano *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/WTC_Nano.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(Port::create<PJ301MPort>(Vec(10, 75), Port::INPUT, module, WTC_Nano::INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(10, 175), Port::OUTPUT, module, WTC_Nano::OUTPUT));

	}
	void appendContextMenu(Menu *menu) override;
};

void WTC_NanoWidget::appendContextMenu(Menu *menu) {
	WTC_Nano *wtc = dynamic_cast<WTC_Nano*>(this->module);
	menu->addChild(MenuEntry::create());
	WTC_MenuItem *m = MenuItem::create<WTC_MenuItem>("Equal");
	m->module = wtc;
	m->setting = 0;
	menu->addChild(m);
	m = MenuItem::create<WTC_MenuItem>("Pythagorean");
	m->module = wtc;
	m->setting = 1;
	menu->addChild(m);
	m = MenuItem::create<WTC_MenuItem>("Werckmeister III");
	m->module = wtc;
	m->setting = 2;
	menu->addChild(m);
	m = MenuItem::create<WTC_MenuItem>("Young");
	m->module = wtc;
	m->setting = 3;
	menu->addChild(m);
}


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelWTC_Nano = Model::create<WTC_Nano, WTC_NanoWidget>("SubmarinePrototype", "WTC_Nano", "Das Wohltemperierte Klavier", TUNER_TAG);
