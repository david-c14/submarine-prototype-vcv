#include <stdio.h>
#include <string.h>
#include <math.h>
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

int main(int argc, char *argv[]) {

	if (argc < 2) {
		printf("%s: USAGE\n\n\t%s <svgfile>\n\n", argv[0], argv[0]);
		return 0;
	}
	printf("//%s : ", argv[1]);
	// Load SVG
	NSVGimage* image;
	image = nsvgParseFromFile(argv[1], "px", 96);
	printf("%f x %f\n\n", image->width, image->height);

	for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {

		printf("nvgBeginPath(vg);\n");
		for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
			printf("nvgMoveTo(vg, %f, %f);\n", (&path->pts[0])[0], (&path->pts[0])[1]);
			for (int i = 0; i < path->npts-1; i += 3) {
				float* p = &path->pts[i*2];
				printf("nvgBezierTo(vg, %f, %f, %f, %f, %f, %f);\n", p[2], p[3], p[4], p[5], p[6], p[7]);
				//drawCubicBez(p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
			}
			if (path->closed) {
				printf("nvgClosePath(vg);\n");
			}
		}	
		// Fill
		if (shape->fill.type == NSVG_PAINT_COLOR) {
			printf("nvgFillColor(vg, nvgRGB(0x%x, 0x%x, 0x%x));\n", shape->fill.color & 0xff, (shape->fill.color & 0xff00) >> 8, (shape->fill.color & 0xff0000) >> 16);
			printf("nvgFill(vg);\n");
		}
		// Stroke
		if (shape->stroke.type == NSVG_PAINT_COLOR) {
			printf("nvgStrokeColor(vg, nvgRGB(0x%x, 0x%x, 0x%x));\n", shape->stroke.color & 0xff, (shape->stroke.color & 0xff00) >> 8, (shape->stroke.color & 0xff0000) >> 16);
			printf("nvgStrokeWidth(vg, %f);\n", shape->strokeWidth);
			printf("nvgLineCap(vg, %d);\n", (int)shape->strokeLineCap);
			printf("nvgLineJoin(vg, %d);\n", (int)shape->strokeLineJoin);
			printf("nvgStroke(vg);\n");
		}
		printf("\n");
	}

	// Delete
	nsvgDelete(image);
	return 0;
}
