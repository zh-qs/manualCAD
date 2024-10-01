#version 410 compatibility

in vec2 fragment_uv;
layout(location = 0) out vec4 output_color;

struct bilinear_form_drawable {
	vec4 color;
	mat4 form;
};

uniform bilinear_form_drawable u_drawable;
uniform float u_specular_exponent;

void main() {
	vec2 screen_uv = fragment_uv * 2 - vec2(1, 1);

	vec4 pixel = vec4(screen_uv, 0, 1);

	float a = u_drawable.form[2][2];
	float b = dot(pixel, u_drawable.form[2]) 
		+ dot(vec4(u_drawable.form[0][2], u_drawable.form[1][2], u_drawable.form[2][2], u_drawable.form[3][2]),
			  pixel);
	float c = dot(pixel, u_drawable.form * pixel);

	float delta = b * b - 4 * a * c;

	if (delta >= 0) {
		float sqrtdelta = sqrt(delta);
		float t = (-b - sqrtdelta) / (2 * a);
		if (t >= -1) {
			// calculate normal vector
			float w = t;
			vec4 point_on_drawable = pixel + vec4(0, 0, t, 0);
			vec4 gradient = u_drawable.form * point_on_drawable; // works only when form is symmetric! (here it is)
			vec3 normal = normalize(gradient.xyz); // we are outside the shape
			vec3 observer = -normalize(point_on_drawable.xyz);
			output_color = u_drawable.color * pow(max(0, dot(normal, observer)), u_specular_exponent);
			output_color.w = (w + 1) / 2;
			//gl_FragDepth = w;
			//(1 / t - 10) / (0.01f - 10);
			// ratio: 3.94385
		}
		else {
			t += sqrtdelta / a;
			if (t >= -1) {
				// calculate normal vector
				float w = t;
				vec4 point_on_drawable = pixel + vec4(0, 0, t, 0);
				vec4 gradient = u_drawable.form * point_on_drawable; // works only when form is symmetric! (here is)
				vec3 normal = -normalize(gradient.xyz); // we are inside the shape
				vec3 observer = -normalize(point_on_drawable.xyz);
				output_color = u_drawable.color * pow(max(0, dot(normal, observer)), u_specular_exponent);
				output_color.w = (w + 1) / 2;
				//gl_FragDepth = w;
			}
			else {
				discard;
			}
		}
	}
	else {
		discard;
	}
}

