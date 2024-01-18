//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul VIII - 08_04_Shader.frag|
// ======================================
// 
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
//

#version 330 core

//	Variabile de intrare (dinspre Shader.vert);
in vec3 ex_Color;
in vec2 tex_Coord;	

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;


//  Variabile uniforme;
uniform int codCol;
uniform sampler2D myTexture;


void main(void)
{
    switch (codCol)
    {
        case 1: 
            out_Color = mix(texture(myTexture, tex_Coord), vec4(ex_Color, 1.0), 0.2);
            break;
        default: 
            out_Color= vec4(ex_Color, 1.0);
    }
}