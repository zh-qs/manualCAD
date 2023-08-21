#version 410 compatibility

layout (lines_adjacency) in;
layout (line_strip, max_vertices = 256) out;
//layout(points, max_vertices = 256) out;

uniform float u_width;
uniform float u_height;

vec2 get_pixel(vec4 p) {
	return vec2(0.5f * u_width * p.x / p.w, 0.5f * u_height * p.y / p.w);
}

bool fits_in_screen(vec4 p) {
	vec4 scr = p / p.w;
	return scr.x >= -1 && scr.x <= 1 && scr.y >= -1 && scr.y <= 1 && scr.z >= -1 && scr.z <= 1;
}

void bezier_with_2_vertices() {
	// calculate length of 2-segments in pixels
	//vec2 pix0 = get_pixel(gl_in[0].gl_Position);
	//vec2 pix1 = get_pixel(gl_in[1].gl_Position);

	//float length_pixels = length(pix1 - pix0);
	//length_pixels = min(256.0f, round(length_pixels) + 1.0f);
	//int segments = int(length_pixels);

	//for (int i = 0; i < segments; ++i) {
	//	float t = i / (length_pixels - 1.0f);
	//	// de casteljau
	//	vec4 b0 = gl_in[0].gl_Position,
	//		b1 = gl_in[1].gl_Position;

	//	gl_Position = (1 - t) * b0 + t * b1;
	//	EmitVertex();
	//} -> this is unnecessary since bezier on 2 vertices is a straight line

	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	EndPrimitive();
}

void bezier_with_3_vertices() {
	// calculate length of 3-segments in pixels
	vec2 pix0 = get_pixel(gl_in[0].gl_Position);
	vec2 pix1 = get_pixel(gl_in[1].gl_Position);
	vec2 pix2 = get_pixel(gl_in[2].gl_Position);

	float length_pixels = length(pix1 - pix0) + length(pix2 - pix1);
	length_pixels = min(256.0f, round(length_pixels) + 1.0f);
	int segments = int(length_pixels);

	// calculate bounds for t to draw more smooth curve
	float t_start = 2, t_end = -1;
	float invlen = 1 / (length_pixels - 1.0f);
	for (int i = 0; i < segments; ++i) {
		float t = i * invlen;
		// de casteljau
		vec4 b0 = gl_in[0].gl_Position,
			b1 = gl_in[1].gl_Position,
			b2 = gl_in[2].gl_Position;

		b0 = (1 - t) * b0 + t * b1;
		b1 = (1 - t) * b1 + t * b2;

		b0 = (1 - t) * b0 + t * b1;

		if (fits_in_screen(b0))
		{
			t_end = t + invlen;
			if (t_start > 1)
				t_start = t - invlen;
		}
	}

	if (t_start >= t_end) return;

	invlen *= (t_end - t_start);
	for (int i = 0; i < segments; ++i) {
		float t = t_start + i * invlen;
		// de casteljau
		vec4 b0 = gl_in[0].gl_Position,
			b1 = gl_in[1].gl_Position,
			b2 = gl_in[2].gl_Position;

		b0 = (1 - t) * b0 + t * b1;
		b1 = (1 - t) * b1 + t * b2;

		gl_Position = (1 - t) * b0 + t * b1;
		EmitVertex();
	}
	EndPrimitive();
}

void bezier_with_4_vertices() {
	// calculate length of 4-segments in pixels
	vec2 pix0 = get_pixel(gl_in[0].gl_Position);
	vec2 pix1 = get_pixel(gl_in[1].gl_Position);
	vec2 pix2 = get_pixel(gl_in[2].gl_Position);
	vec2 pix3 = get_pixel(gl_in[3].gl_Position);

	float length_pixels = length(pix1 - pix0) + length(pix2 - pix1) + length(pix3 - pix2);
	length_pixels = min(256.0f, round(length_pixels) + 1.0f);
	int segments = int(length_pixels);

	// calculate bounds for t to draw more smooth curve
	// TODO recurring subdivisions
	float t_start = 2, t_end = -1;
	float invlen = 1 / (length_pixels - 1.0f);
	for (int i = 0; i < segments; ++i) {
		float t = i * invlen;
		// de casteljau
		vec4 b0 = gl_in[0].gl_Position,
			b1 = gl_in[1].gl_Position,
			b2 = gl_in[2].gl_Position,
			b3 = gl_in[3].gl_Position;

		b0 = (1 - t) * b0 + t * b1;
		b1 = (1 - t) * b1 + t * b2;
		b2 = (1 - t) * b2 + t * b3;

		b0 = (1 - t) * b0 + t * b1;
		b1 = (1 - t) * b1 + t * b2;

		b0 = (1 - t) * b0 + t * b1;

		if (fits_in_screen(b0))
		{
			t_end = t + invlen;
			if (t_start > 1)
				t_start = t - invlen;
		}
	}

	if (t_start >= t_end) return;

	invlen *= (t_end - t_start);
	for (int i = 0; i < segments; ++i) {
		float t = t_start + i * invlen; // without checking -> t_start = 0, t_end = 1;
		// de casteljau
		vec4 b0 = gl_in[0].gl_Position,
			b1 = gl_in[1].gl_Position,
			b2 = gl_in[2].gl_Position,
			b3 = gl_in[3].gl_Position;

		b0 = (1 - t) * b0 + t * b1;
		b1 = (1 - t) * b1 + t * b2;
		b2 = (1 - t) * b2 + t * b3;

		b0 = (1 - t) * b0 + t * b1;
		b1 = (1 - t) * b1 + t * b2;

		gl_Position = (1 - t) * b0 + t * b1;
		EmitVertex();
	}
	EndPrimitive();
}

void main() {
	if (isnan(gl_in[2].gl_Position).x)
		bezier_with_2_vertices();
	else if (isnan(gl_in[3].gl_Position).x)
		bezier_with_3_vertices();
	else
		bezier_with_4_vertices();
}