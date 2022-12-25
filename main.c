#include <stdio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Widths and Heights
#define C_WIDTH 640
#define C_HEIGHT 480
#define V_WIDTH 1
#define V_HEIGHT 1

// SP_COLOR            AABBGGRR
#define SP_COLOR_WHITE 0xFFFFFFFF
#define SP_COLOR_BLACK 0xFF000000
#define SP_COLOR_RED   0xFF0000FF
#define SP_COLOR_GREEN 0xFF00FF00
#define SP_COLOR_BLUE  0xFFFF0000

// END SP_COLOR

// SP_MATH
#define SP_INFINITY 10000 // Just an arbitrary number selected for the max distance.

#include <stdint.h>
#include <math.h>
typedef struct {
	float x, y, z;
} sp_vec3_t;

typedef struct {
	int32_t t1;
	int32_t t2;
} sp_tuple_t;

int32_t sp_dot(const sp_vec3_t *vec1, const sp_vec3_t *vec2) {	
	return ((vec1->x * vec2->x) + (vec1->y * vec2->y) + (vec1->z * vec2->z));
}

sp_vec3_t sp_vec3_sub(sp_vec3_t vec1, sp_vec3_t vec2) {
	return (sp_vec3_t) {
		.x = vec1.x - vec2.x,
		.y = vec1.y - vec2.y,
		.z = vec1.z - vec2.z
	};
}
// END SP_MATH

// SHAPES
#define SPHERE_COUNT 3

typedef struct {
	sp_vec3_t center;
	uint32_t radius;
	uint32_t color;
} sp_sphere_t;
// END SHAPES

// Converts canvas coords to viewport coords.
sp_vec3_t sp_canvas_to_viewport(sp_vec3_t canvas_coords) {
	return (sp_vec3_t) {.x = canvas_coords.x * V_WIDTH / C_WIDTH, .y = canvas_coords.y * V_HEIGHT / C_HEIGHT, .z = canvas_coords.z};
}

sp_tuple_t sp_intersect_ray_sphere(sp_vec3_t camera_pos, sp_vec3_t ray_direction, sp_sphere_t sphere) {
	int32_t r = sphere.radius;
	sp_vec3_t CO = sp_vec3_sub(camera_pos, sphere.center);

	int32_t a = sp_dot(&ray_direction, &ray_direction);
	int32_t b = 2 * sp_dot(&CO, &ray_direction);
	int32_t c = sp_dot(&CO, &CO) - r * r;

	int32_t discriminant = b * b - 4 * a * c;
	if (discriminant < 0) {
		return (sp_tuple_t) {.t1 = SP_INFINITY, .t2 = SP_INFINITY};
	}

	return (sp_tuple_t) {
		.t1 = (-b + sqrt(discriminant)) / (2 * a),
		.t2 = (-b - sqrt(discriminant)) / (2 * a)
	};
}

#include <stdbool.h>
bool sp_in_range(int32_t val, int32_t min, int32_t max) {	
	return (val > min && val < max);
}

int32_t sp_trace_ray(sp_vec3_t camera_pos, sp_vec3_t ray_direction, int32_t t_min, int32_t t_max, sp_sphere_t scene[]) {
	int32_t t_closest = SP_INFINITY;
	sp_sphere_t *closest_sphere = NULL;

	for (size_t i = 0; i < SPHERE_COUNT; ++i) {
		sp_tuple_t tuple = sp_intersect_ray_sphere(camera_pos, ray_direction, scene[i]);

		if (sp_in_range(tuple.t1, t_min, t_max) && tuple.t1 < t_closest) {
			t_closest = tuple.t1;
			closest_sphere = &scene[i];
		}

		if (sp_in_range(tuple.t2, t_min, t_max) && tuple.t2 < t_closest) {
			t_closest = tuple.t2;
			closest_sphere = &scene[i];
		}
	}

	if (closest_sphere == NULL) {
		return SP_COLOR_BLACK;
	}

	return closest_sphere->color;
}

int32_t canvas[C_WIDTH * C_HEIGHT];

void sp_canvas_put_pixel(sp_vec3_t coords, uint32_t color) {
	int32_t screen_x = (C_WIDTH / 2) + coords.x;
	int32_t screen_y = (C_HEIGHT / 2) - coords.y - 1;

	if (screen_x < 0 || screen_x >= C_WIDTH || screen_y < 0 || screen_y >= C_HEIGHT) {
		return;
	}

	canvas[screen_x + screen_y * C_WIDTH] = color;
}

#define ALPHA 4
#define NO_ALPHA 3
int main(void) {
	sp_vec3_t camera_pos = {
		.x = 0.0f,
		.y = 0.0f,
		.z = 0.0f
	};

	// Scene Objects
	sp_sphere_t sphere_red = {
		.center = {
			.x = 0.0f,
			.y = -1.0,
			.z = 3.0f
		},
		.radius = 1,
		.color = SP_COLOR_RED
	};

	sp_sphere_t sphere_green = {
		.center = {
			.x = -2.0f,
			.y = 0.0f,
			.z = 4.0f
		},
		.radius = 1,
		.color = SP_COLOR_GREEN
	};

	sp_sphere_t sphere_blue = {
		.center = {
			.x = 2.0f,
			.y = 0.0f,
			.z = 4.0f
		},
		.radius = 1,
		.color = SP_COLOR_BLUE
	};

	sp_sphere_t scene[SPHERE_COUNT] = {
		sphere_red, sphere_green, sphere_blue
	};

	for (int32_t xx = -C_WIDTH / 2; xx < C_WIDTH / 2; ++xx) {
		for (int32_t yy = -C_HEIGHT / 2; yy < C_HEIGHT / 2; ++yy) {
			sp_vec3_t coords = {
				.x = xx,
				.y = yy,
				.z = 1
			};

			sp_vec3_t ray_direction = sp_canvas_to_viewport(coords);

			uint32_t color = sp_trace_ray(camera_pos, ray_direction, 1, SP_INFINITY, scene);
			sp_canvas_put_pixel(coords, color);
		}
	}

	stbi_write_png("test.png", C_WIDTH, C_HEIGHT, ALPHA, canvas, sizeof(int32_t) * C_WIDTH);
	
	return 0;
}

