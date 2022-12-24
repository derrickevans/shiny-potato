#include <stdio.h>

/* PSEUDO CODE ************************************************************

   	CanvasToViewport(x, y) {
		return (x * Vw / Cw, y * Vh / Ch, d)
	}

	TraceRay(O, D, t_min, t_max) {
		closest_t = inf
		closest_sphere = NULL

		for sphere in scene.spheres {
			t1, t2 = IntersectRaySphere(O, D, sphere)
			if t1 in [t_min, t_max] and t1 < closest_t {
				closest_t = t1
				closest_sphere = sphere
			}
			if t2 in [t_min, t_max] and t2 < closest_t {
				closest_t = t2
				closest_sphere = sphere
			}
		}

		if closest_sphere == NULL {
			return BACKGROUND_COLOR
		}

		return closest_sphere.color
	}

   	IntersectRaySphere(O, D, sphere) {
		r = sphere.radius
		CO = O - sphere.center

		a = dot(D, D)
		b = 2 * dot(CO, D)
		c = dot(CO, CO) - r * r

		discriminant = b * b - 4 * a * c
		if discriminant < 0 {
			return inf, inf
		}

		t1 = (-b + sqrt(discriminant)) / (2 * a)
		t2 = (-b - sqrt(discriminant)) / (2 * a)
		return t1, t2
	}

pseudoscene:
	viewport_size = 1 x 1
	projection_plane_d = 1
	sphere {
		center = (0, -1, 3)
		radius = 1
		color = (255, 0, 0) # Red
	}
	sphere {
		center = (2, 0, 4)
		radius = 1
		color = (0, 0, 255) # Blue
	}
	sphere {
		center = (-2, 0, 4)
		radius = 1
		color = (0, 255, 0) # Green
	}
***************************************************************************/

int main(void) {
	printf("Hello, Shiny Potato!\n");
	return 0;
}
