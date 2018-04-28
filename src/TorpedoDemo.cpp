#include "SubmarinePrototype.hpp"
#include "torpedo.cpp"

struct TorpedoDemo : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		TORPEDO_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		TORPEDO_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	TorpedoDemo() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void TorpedoDemo::step() {
}

struct TorpedoDemoWidget : ModuleWidget {
	TorpedoDemoWidget(TorpedoDemo *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CrossFader.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(Port::create<PJ301MPort>(Vec(25, 75), Port::INPUT, module, TorpedoDemo::TORPEDO_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(105, 125), Port::OUTPUT, module, TorpedoDemo::TORPEDO_OUTPUT));

	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelTorpedoDemo = Model::create<TorpedoDemo, TorpedoDemoWidget>("SubmarinePrototype", "TorpedoDemo", "Torpedo Demo", ATTENUATOR_TAG, MIXER_TAG);
