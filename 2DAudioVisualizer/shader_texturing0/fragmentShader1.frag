precision mediump float;
varying vec4 fColor;
varying vec2 fTexCoord;

uniform sampler2D sampler2d;
uniform float Factor1;

// Octaves Array
uniform float Octave0;
uniform float Octave1;
uniform float Octave2;
uniform float Octave3;
uniform float Octave4;
uniform float Octave5;
uniform float Octave6;
uniform float Octave7;
uniform float Octave8;

void main()
{	
	// Sine wave
	vec4 texColor = texture2D(sampler2d, fTexCoord);
	
	float originY = 90.0;
	float thickness = 13.0;
	float amplitude = 20.0 + (5.0 * Octave1);
	float frequency = 0.05 * (Octave0 + Octave1 + Octave2);
	float speed = 20.0 + (Octave1 * 2.0);
	float waveOrigin = originY + sin(gl_FragCoord.x * frequency + (Factor1 * speed)) * amplitude;
	float gradientEdge = abs(gl_FragCoord.y - waveOrigin) / thickness;
	vec4 colourOrigin = vec4(0.3, 0.3, 0.3, 0.5);
	vec4 colourEdge = vec4(-0.7, 0.0, 0.0, 0.5);
	
	float origin1 = 630.0;
	float thickness1= 13.0;
	float amplitude1 = 45.0 * Octave7;
	float frequency1 = 0.05;
	float speed1 = 20.0 + (Octave1 * 3.0);
	float waveOrigin1 = origin1 + sin(gl_FragCoord.x * frequency1 + (Factor1 * speed1)) * amplitude1;
	float gradientEdge1 = abs(gl_FragCoord.y - waveOrigin1) / thickness1;
	vec4 colourOrigin1 = vec4(0.5, 0.5, 0.5, 0.5);
	vec4 colourEdge1 = vec4(0.0, 0.7, 0.7, 0.5);
	
	// Circle
	float circleRadius = sin(gl_FragCoord.y * (0.01 * Octave4) + (Factor1 * 1.0)) * 50.0;
	float circleRadius1 = sin(gl_FragCoord.y * (0.01 * Octave3) + (Factor1 * 1.0)) * 50.0;
	
	// Background
	gl_FragColor = texColor;	
	
	// Wave 1
	if (gl_FragCoord.y <= waveOrigin + thickness && gl_FragCoord.y >= waveOrigin - thickness)
	{
		gl_FragColor = texColor + ((1.0 - gradientEdge) * colourOrigin) + (gradientEdge * colourEdge);
	}
	// Wave 2
	else if (gl_FragCoord.y <= waveOrigin1 + thickness1 && gl_FragCoord.y >= waveOrigin1 - thickness1)
	{
		gl_FragColor = texColor + ((1.0 - gradientEdge1) * colourOrigin1) + (gradientEdge1 * colourEdge1);
	}
	// Ring
	else if (distance(gl_FragCoord.xy, vec2(640.0, 360.0)) < 70.0 + abs(circleRadius) && distance(gl_FragCoord.xy, vec2(640.0, 360.0)) > 50.0 + abs(circleRadius))
	{
		gl_FragColor = texColor + vec4((0.1 + (0.7 * Octave4)), 0.0, (0.5 + (1.0 * sin(Factor1))), 0.7);
	}
	// Ring 1
	else if (distance(gl_FragCoord.xy, vec2(640.0, 360.0)) < 25.0 + abs(circleRadius1) && distance(gl_FragCoord.xy, vec2(640.0, 360.0)) > 0.0 + circleRadius1)
	{
		gl_FragColor = texColor + vec4(0.0, (0.5 + (1.0 * sin(Factor1))), (0.1 + (0.7 * Octave5)), 0.7);
	}
	// Square
	else if (gl_FragCoord.x >= 200.0 - ((Octave0 + Octave1) * 100.0) && gl_FragCoord.x <= 300.0 + ((Octave0 + Octave1) * 100.0) && 
	gl_FragCoord.y >= 320.0 && gl_FragCoord.y <= 400.0)
	{
		gl_FragColor = texColor + vec4((sin(Factor1) - Octave2), (sin(Factor1) - Octave2), 0.0, 0.5);
	}
	else if (gl_FragCoord.x >= 210.0 && gl_FragCoord.x <= 290.0 && 
	gl_FragCoord.y >= 310.0 - (Octave7 * 50.0) && gl_FragCoord.y <= 410.0 + (Octave7 * 50.0))
	{
		gl_FragColor = texColor + vec4((sin(Factor1) - Octave6), 0.0, (sin(Factor1) - Octave6), 0.5);
	}
	// Circle
	else if (distance(gl_FragCoord.xy, vec2(1030.0, 360.0)) < 30.0 + (20.0 * Octave6))
	{
		gl_FragColor = texColor + vec4(sin(Factor1), 0.0, Octave6, 0.5);
	}
	else if (distance(gl_FragCoord.xy, vec2(1030.0, 360.0)) < 70.0 + (20.0 * Octave7))
	{
		gl_FragColor = texColor + vec4(0.0, sin(Factor1), Octave7, 0.5);
	}
	
	// *Gray scale*
	// float average = (fColor.r + fColor.g + fColor.b) / 3.0;
	// vec4 finalColor = vec4(average, average, average, 1.0);

	// gl_FragColor = finalColor;


	// *Texture movement*
	// vec2 finalTexCoord;
	// finalTexCoord.x = fTexCoord.x + 0.05 * sin(gl_FragCoord.y * 0.02 + Factor1);
	// finalTexCoord.y = fTexCoord.y;

	// finalTexCoord.x = fTexCoord.x + 0.1 * sin(gl_FragCoord.x * 0.02 + Factor1);
	// finalTexCoord.y = fTexCoord.y + 0.1 * sin(gl_FragCoord.y * 0.02 + Factor1);

	// finalTexCoord.x = fTexCoord.x + 0.1 * sin(gl_FragCoord.y * 0.05 + Factor1);
	// finalTexCoord.y = fTexCoord.y;

	// gl_FragColor = texture2D(sampler2d, finalTexCoord);


	// *Shader pattern*
	// vec4 texColor = texture2D(sampler2d, fTexCoord);
	// vec4 combinedColor;
	// combinedColor = fColor * texColor;
	
	// vec4 resultColor;
	
	// resultColor.r = mod(combinedColor.r + Factor1, 1.0);
	// resultColor.b = mod(combinedColor.b + Factor1, 1.0);
	// resultColor.a = combinedColor.a;
	
	// if (distance(gl_FragCoord.xy, vec2(640.0, 360.0)) < 90.0 && distance(gl_FragCoord.xy, vec2(640.0, 360.0)) > 70.0)
	// {	
		// resultColor.g = mod(combinedColor.g + Factor1, 1.0);
	// }
	
	// gl_FragColor = resultColor;
	
	
	// *Use texture color only*
	// gl_FragColor = texture2D(sampler2d, fTexCoord);
	
	
	// *Use texture and vertex color*
	// vec4 texColor = texture2D(sampler2d, fTexCoord);
	// gl_FragColor = fColor * texColor;
}