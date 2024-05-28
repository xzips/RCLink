uniform vec2 center;      // Center of the circle
uniform float radius;     // Radius of the circle

void main() {
    vec2 pos = gl_FragCoord.xy - center; // Using screen coordinates directly
    float dist = length(pos);
    
    if (dist < radius) {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); // Green inside the circle
    } else {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red outside the circle
    }
}
