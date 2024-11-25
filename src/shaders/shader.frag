#version 330 core
in vec3 ourColor;
in vec2 texCoord;

out vec4 FragColor;

uniform vec3 sunDir;
uniform vec3 sunAng;//site, az, 0.
uniform vec3 camera;//angleX, angleZ, render->f/w
uniform float iTime;
uniform sampler2D textureCol;
uniform sampler2D textureZbuf;
uniform sampler2D textureNorm;

const float ambiant = 0.2;

/*
  TODO:
   - sky with correct proj
   - add ang X
   - special texture for sky/background/vox/water
   - slowly changing normale for water
  */



vec2 screen2uv(vec2 c)
{
    /*float screenRatio = iResolution.x / iResolution.y;
    vec2 uv = c/iResolution.xy;
    uv -= vec2(0.5);
    uv *= 2.0;
    uv.x *= screenRatio; // uv: -1:1 for smaller dim
    return uv;*/
	return c;
}

//simplex noise from https://www.shadertoy.com/view/Msf3WH
vec2 hash( vec2 p ) // replace this by something better
{
    //return sin(vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) ));
	p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float noise( vec2 p )
{
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;

	vec2  i = floor( p + (p.x+p.y)*K1 );
    vec2  a = p - i + (i.x+i.y)*K2;
    float m = step(a.y,a.x);
    vec2  o = vec2(m,1.0-m);
    vec2  b = a - o + K2;
	vec2  c = a - 1.0 + 2.0*K2;
    vec3  h = max( 0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );
	vec3  n = h*h*h*h*vec3( dot(a,hash(i+0.0)), dot(b,hash(i+o)), dot(c,hash(i+1.0)));
    return dot( n, vec3(70.0) );
}

float octaveNoise( vec2 uv )
{
    uv *= 2.0;
    mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );
    float f  = 0.5000*noise( uv ); uv = m*uv;
    f += 0.2500*0.5*noise( uv ); uv = m*uv;
    f += 0.1250*0.5*noise( uv ); uv = m*uv;
    f += 0.0625*0.5*noise( uv ); uv = m*uv;

	f = 0.5 + 0.5*f;
    return f;
}

float distSpecial(vec2 uv, vec2 mouseuv)
{
    return sqrt( (uv.x-mouseuv.x)*(uv.x-mouseuv.x)*(1.01+mouseuv.y) + (uv.y-mouseuv.y)*(uv.y-mouseuv.y) );
}

vec2 site_az2uv( vec2 angs )
{
	if (dot(-sunDir,vec3(sin(camera.y),cos(camera.y),0))<0) return vec2(-100,-100);
	vec2 uv = vec2(-sunAng.x, -sunAng.y+camera.y);
	uv = -camera.z * tan(uv) + 0.5;
	return uv;
}

vec4 skyImage2( vec2 fragCoord )
{
    vec2 uv = screen2uv(fragCoord);
    vec2 mouseuv = site_az2uv(sunAng.xy);
    float d = 0.1/distance(uv, mouseuv);
    return vec4(d,d,0.5+d/2.,1);
}

vec4 skyImage( vec2 fragCoord )
{
    vec2 uv = screen2uv(fragCoord);
    vec2 mouseuv = site_az2uv(sunAng.xy);
    //float sunDist = 0.02/(distance(uv,mouseuv)+0.01);
    float sunDist = 0.02/(distSpecial(uv,mouseuv)+0.01);
    vec3 sunCol = vec3(sunDist*(1.-uv.y/3.0)*abs(2.+uv.y),sunDist*0.5*abs(2.+uv.y),0.0);
    //vec3 sunCol = vec3(0);

    //float skyNoise = (sin(uv.x/3.0)+sin(uv.y/7.+1.)*cos(iTime*0.1)*3.)/3.0;
    float skyNoise = octaveNoise(uv)/2.0+octaveNoise( vec2(uv.x+iTime/2.+(8.+uv.y)/2., uv.y))/8.+0.5;

    vec3 skyBlue = vec3( 0.5 + skyNoise/4., 0.5 + skyNoise/4., 1. + skyNoise/3.0 );

    return vec4(skyBlue+sunCol,1.0);
}






void main() {
/*    vec4 col = texture(textureCol, texCoord);
    float z = 1/(10*sqrt(texture(textureZbuf, texCoord).r));
    vec4 n = texture(textureNorm, texCoord);
    //FragColor = vec4(0.,0.,z,1.0);
    //FragColor = vec4(texCoord.x*col.g,texCoord.y,0.,1.0);
    //FragColor = vec4(0.,0.,col.b,1.0);
    //FragColor = vec4((col.rgb+0.5)*z,1.0);
    FragColor = col;//vec4(col.rgb,1.0);*/


	vec4 n = texture(textureNorm, texCoord);
	if (n.w==0.)
	{
		FragColor = skyImage2(texCoord);
		return;
	}
	n = 2*n-1;
	vec4 col = texture(textureCol, texCoord);
	float z = 2/(10*(texture(textureZbuf, texCoord).r));
	z = clamp(z,0,1);

	//FragColor = vec4(ourColor,1.0);
	//FragColor = vec4(0.0,1.0,0.0,1.0);
	//vec4 zbuf = texture(textureZbuf, texCoord);
	//float z = 1/(10*sqrt(texture(textureZbuf, texCoord).r));
	//vec4 n = texture(textureNorm, texCoord);
	float light = clamp(dot(vec3(n),-sunDir)*2,0.5,2);
	FragColor = vec4(col.rgb*z,1.0)*light;
	//FragColor = col+sunDir+n+z;
	//FragColor = n;
	//FragColor = vec4(sunDir,1,1,1);
	//FragColor = vec4(sunDir.b,0.0,texCoord.x,1.0);
	
};
