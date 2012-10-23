# =version 120 


// Mine is an old machine.  For version 130 or higher, do 
// out vec4 color ;  
// out vec4 mynormal ; 
// out vec4 myvertex ;
// That is certainly more modern

varying vec4 color ; 
varying vec3 mynormal ; 
varying vec4 myvertex ; 
varying vec2 mytexcoord;

void main() {
  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex ; 
  color = gl_Color ; 
  mynormal = gl_Normal ; 
  myvertex = gl_Vertex ; 
  mytexcoord = vec2(gl_MultiTexCoord0);
}

