#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Widths and Heights
#define C_WIDTH 640
#define C_HEIGHT 480
#define V_WIDTH 1
#define V_HEIGHT 1

// SP_COLOR               AABBGGRR
#define SP_COLOR_WHITE 	0xFFFFFFFF
#define SP_COLOR_BLACK 	0xFF000000
#define SP_COLOR_RED   	0xFF0000FF
#define SP_COLOR_GREEN 	0xFF00FF00
#define SP_COLOR_BLUE  	0xFFFF0000
#define SP_COLOR_YELLOW 0xFF00FFFF

// Get the specified channel value.
#define SP_RED_CHANNEL(color) (((color) & 0x000000FF) >> (8 * 0))
#define SP_GREEN_CHANNEL(color) (((color) & 0x0000FF00) >> (8 * 1))
#define SP_BLUE_CHANNEL(color) (((color) & 0x00FF0000) >> (8 * 2))
#define SP_ALPHA_CHANNEL(color) (((color) & 0xFF000000) >> (8 * 3))

// Set the color by channel.
#define SP_RGBA(r, g, b, a) ((((r) & 0xFF) << (8 * 0)) | (((g) & 0xFF) << (8 * 1)) | (((b) & 0xFF) << (8 * 2)) | (((a) & 0xFF) << (8 * 3)))
// END SP_COLOR

// SP_MATH
#define SP_INFINITY 100000 // Just an arbitrary number selected for the max distance.

typedef struct {
	float x, y, z;
} sp_vec3_t;

typedef struct {
	float t1;
	float t2;
} sp_tuple_t;

float sp_dot(const sp_vec3_t *vec1, const sp_vec3_t *vec2) {	
	return ((vec1->x * vec2->x) + (vec1->y * vec2->y) + (vec1->z * vec2->z));
}

sp_vec3_t sp_vec3_add(const sp_vec3_t *vec1, const sp_vec3_t *vec2) {
	return (sp_vec3_t) {
		.x = vec1->x + vec2->x,
		.y = vec1->y + vec2->y,
		.z = vec1->z + vec2->z
	};
}

sp_vec3_t sp_vec3_sub(const sp_vec3_t *vec1, const sp_vec3_t *vec2) {
	return (sp_vec3_t) {
		.x = vec1->x - vec2->x,
		.y = vec1->y - vec2->y,
		.z = vec1->z - vec2->z
	};
}

sp_vec3_t sp_vec3_mult(const sp_vec3_t *vec, float scalar) {
	return (sp_vec3_t) {
		.x = vec->x * scalar,
		.y = vec->y * scalar,
		.z = vec->z * scalar
	};
}

float sp_length(const sp_vec3_t *vec) {
	return (float) sqrt(sp_dot(vec, vec));
}
// END SP_MATH

// SHAPES
#define SPHERE_COUNT 4

typedef struct {
	sp_vec3_t center;
	uint32_t radius;
	uint32_t color;
	uint32_t specular;
} sp_sphere_t;
// END SHAPES

// LIGHT
#define LIGHT_COUNT 3

typedef enum {
	sp_ambient,
	sp_directional,
	sp_point
} sp_light_type;

typedef struct {
	sp_light_type type;
	float intensity;
	sp_vec3_t position; // Position will act as the direction for directional light. Position will be ignored for ambient light type.
} sp_light_t;

float sp_compute_light(const sp_light_t *lights, const sp_vec3_t *p, const sp_vec3_t *n, const sp_vec3_t *v, uint32_t s) {
	float i = 0.0f;
	sp_vec3_t L = {
		.x = 0.0f,
		.y = 0.0f,
		.z = 0.0f
	};

	for (int index = 0; index < LIGHT_COUNT; index++) {
		sp_light_t light = lights[index];
		if (light.type == sp_ambient) {
			i += light.intensity;
		} else {
			if (light.type == sp_point) {
				L = sp_vec3_sub(&light.position, p);
			} else {
				L = light.position; // Position is the direction for this case.
			}
			
			// Diffuse
			float n_dot_l = sp_dot(n,&L);
			if (n_dot_l > 0.0f) {
				i += light.intensity * n_dot_l / (sp_length(n) * sp_length(&L));
			}

			// Specular
			if (s != -1) {
				sp_vec3_t temp_v_0 = sp_vec3_mult(n, 2.0f);
				sp_vec3_t temp_v_1 = sp_vec3_mult(&temp_v_0, sp_dot(n, &L));
				sp_vec3_t R = sp_vec3_sub(&temp_v_1, &L); 
				float r_dot_v = sp_dot(&R, v);
				if (r_dot_v > 0) {
					float val = r_dot_v / (sp_length(&R) * sp_length(v));
					i += light.intensity * powf(val, s);
				}
			}
		}	
	}
	return i;
}
// END LIGHT

// Converts canvas coords to viewport coords.
sp_vec3_t sp_canvas_to_viewport(sp_vec3_t canvas_coords) {
	return (sp_vec3_t) {.x = canvas_coords.x * V_WIDTH / C_WIDTH, .y = canvas_coords.y * V_HEIGHT / C_HEIGHT, .z = canvas_coords.z};
}

sp_tuple_t sp_intersect_ray_sphere(sp_vec3_t camera_pos, sp_vec3_t ray_direction, sp_sphere_t sphere) {
	uint32_t r = sphere.radius;
	sp_vec3_t CO = sp_vec3_sub(&camera_pos, &sphere.center);

	float a = sp_dot(&ray_direction, &ray_direction);
	float b = 2 * sp_dot(&CO, &ray_direction);
	float c = sp_dot(&CO, &CO) - r * r;

	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0) {
		return (sp_tuple_t) {.t1 = (float) SP_INFINITY, .t2 = (float) SP_INFINITY};
	}

	return (sp_tuple_t) {
		.t1 = (float) (-b + sqrt(discriminant)) / (2.0f * a),
		.t2 = (float) (-b - sqrt(discriminant)) / (2.0f * a)
	};
}

bool sp_in_range(float val, float min, float max) {	
	return (val > min && val < max);
}

uint32_t sp_trace_ray(sp_vec3_t camera_pos, sp_vec3_t ray_direction, float t_min, float t_max, sp_sphere_t scene[], sp_light_t lights[]) {
	float t_closest = (float) SP_INFINITY;
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

	if (closest_sphere == NULL) return SP_COLOR_BLACK;
	
	sp_vec3_t temp = sp_vec3_mult(&ray_direction, t_closest);
	sp_vec3_t P = sp_vec3_add(&camera_pos, &temp);
	sp_vec3_t N1 = sp_vec3_sub(&P, &closest_sphere->center);
	sp_vec3_t N = sp_vec3_mult(&N1, 1.0f / sp_length(&N1));

	sp_vec3_t view = sp_vec3_mult(&ray_direction, -1.0f);
	float intensity = sp_compute_light(lights, &P, &N, &view, closest_sphere->specular);
	uint32_t red_channel = SP_RED_CHANNEL(closest_sphere->color) * intensity;
	if (red_channel > 0xFF) red_channel = 0xFF;
	uint32_t green_channel = SP_GREEN_CHANNEL(closest_sphere->color) * intensity;
	if (green_channel > 0xFF) green_channel = 0xFF;
	uint32_t blue_channel = SP_BLUE_CHANNEL(closest_sphere->color) * intensity;
	if (blue_channel > 0xFF) blue_channel = 0xFF;
	uint32_t alpha_channel = SP_ALPHA_CHANNEL(closest_sphere->color);
	if (alpha_channel > 0xFF) alpha_channel = 0xFF;

 	return SP_RGBA(red_channel, green_channel, blue_channel, alpha_channel);
}

int32_t canvas[C_WIDTH * C_HEIGHT];

void sp_canvas_put_pixel(sp_vec3_t coords, uint32_t color) {
	float screen_x = (C_WIDTH / 2.0f) + coords.x;
	float screen_y = (C_HEIGHT / 2.0f) - coords.y - 1;

	if (screen_x < 0 || screen_x >= C_WIDTH || screen_y < 0 || screen_y >= C_HEIGHT) {
		return;
	}

	canvas[(int32_t) screen_x + (int32_t) screen_y * C_WIDTH] = color;
}

#define ALPHA 4
#define NO_ALPHA 3
int main(void) {
	// Camera Position
	sp_vec3_t camera_pos = {
		.x = 0.0f,
		.y = 0.0f,
		.z = 0.0f
	};

	// Scene Objects
	sp_sphere_t sphere_red = {
		.center = {
			.x = 0.0f,
			.y = -1.0f,
			.z = 3.0f
		},
		.radius = 1,
		.color = SP_COLOR_RED,
		.specular = 500 // Shiny
	};

	sp_sphere_t sphere_green = {
		.center = {
			.x = -2.0f,
			.y = 0.0f,
			.z = 4.0f
		},
		.radius = 1,
		.color = SP_COLOR_GREEN,
		.specular = 10 // Somewhate shiny
	};

	sp_sphere_t sphere_blue = {
		.center = {
			.x = 2.0f,
			.y = 0.0f,
			.z = 4.0f
		},
		.radius = 1,
		.color = SP_COLOR_BLUE,
		.specular = 500 // Shiny
	};

	sp_sphere_t sphere_yellow = {
		.center = {
			.x = 0.0f,
			.y = -5001.0f,
			.z = 0.0f
		},
		.radius = 5000,
		.color = SP_COLOR_YELLOW,
		.specular = 1000 // Very shiny
	};

	sp_sphere_t scene[SPHERE_COUNT] = {
		sphere_red, sphere_green, sphere_blue, sphere_yellow
	};

	// Scene light.
	sp_light_t ambient = {
		.type = sp_ambient,
		.intensity = 0.2f,
		.position = {
			.x = 0.0f,
			.y = 0.0f,
			.z = 0.0f
		}
	};

	sp_light_t point = {
		.type = sp_point,
		.intensity = 0.6f,
		.position = {
			.x = 2.0f,
			.y = 1.0f,
			.z = 0.0f
		}
	};

	sp_light_t directional = {
		.type = sp_directional,
		.intensity = 0.2f,
		// Position vec3 is the direction vec3
		.position = {
			.x = 1.0f,
			.y = 4.0f,
			.z = 4.0f
		}
	};

	sp_light_t scene_lights[LIGHT_COUNT] = {
		ambient, point, directional
	};

	for (float xx = -C_WIDTH / 2.0f; xx < C_WIDTH / 2.0f; ++xx) {
		for (float yy = -C_HEIGHT / 2.0f; yy < C_HEIGHT / 2.0f; ++yy) {
			sp_vec3_t coords = {
				.x = xx,
				.y = yy,
				.z = 1
			};

			sp_vec3_t ray_direction = sp_canvas_to_viewport(coords);

			uint32_t color = sp_trace_ray(camera_pos, ray_direction, 1, SP_INFINITY, scene, scene_lights);
			sp_canvas_put_pixel(coords, color);
		}
	}

	stbi_write_png("test.png", C_WIDTH, C_HEIGHT, ALPHA, canvas, sizeof(int32_t) * C_WIDTH);
	
	return 0;
}

