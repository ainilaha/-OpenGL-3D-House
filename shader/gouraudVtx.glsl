#version 330 core
#ifdef SYNTHCLIPSE
#include <synthclipse>
#endif
// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};






uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


uniform vec3 viewPos;
uniform Material material;
uniform int light_flag; // 0: dirLight, 1:PointLight,2:spotLight
uniform int light_model_flag; // 0:phong,1:cook-torrance,2:Oren-Nyar, 3:Ward

//light define
#define NR_POINT_LIGHTS 4
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;



vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

/**************************
 *lighting models
 */
vec3 cookTorrance(PointLight light, vec3 normal,vec3 fragPos, vec3 viewDir);
vec3 orenNayar(PointLight light, vec3 normal,vec3 fragPos, vec3 viewDir);
vec3 ward(PointLight light, vec3 normal,vec3 fragPos, vec3 viewDir);



out vec3 LightingColor;

void main(){
    vec3 FragPos = vec3(view * model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vec3 Normal = mat3(transpose(inverse(model))) * aNormal;
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    
    
    // directional lighting
    vec3 dirlight_result = CalcDirLight(dirLight, norm, viewDir);
    // spot light
    vec3 splot_result = CalcSpotLight(spotLight, norm, FragPos, viewDir);
    
    //point light
    vec3 point_light_result = vec3(0.0,0.0,0.0);
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
    {
    
        if(light_model_flag==1)
        {
            point_light_result += cookTorrance(pointLights[i], norm, FragPos, viewDir);
        }else if(light_model_flag==2)
        {
            point_light_result += orenNayar(pointLights[i], norm, FragPos, viewDir);
        }else if(light_model_flag==3)
        {
            point_light_result += ward(pointLights[i], norm, FragPos, viewDir);
        }
        else{
            point_light_result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
        }
        
    }
    
    
    
    
    vec3 result = vec3(0.0,0.0,0.0);
    if(light_flag==0){
        result += dirlight_result;
    } else if(light_flag==1)
    {
        result += splot_result;
    }else if(light_flag==2)
    {
        result += point_light_result;
    }else
    {
        result += dirlight_result;
        result += splot_result;
        result += point_light_result;
        
    }

    LightingColor = result;
    
}





vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient  = light.ambient  * material.ambient;
    vec3 diffuse  = light.diffuse  * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
    

}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

/**********************************************
 *Cook-Torrance start
 *
 ***********************************************/


uniform float RoughnessValue = 1.0; //! slider[0, 1, 1]
uniform float RefAtNormIncidence = 1; //! slider[0, 1, 1]
uniform int RoughnessMode = 0; //! slider[0, 0, 1]

#define ROUGHNESS_BECKMANN 0
#define ROUGHNESS_GAUSSIAN 1

vec3 cookTorrance(PointLight light, vec3 normal,vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // Compute any aliases and intermediary values
    // -------------------------------------------
    vec3 half_vector = normalize(lightDir + viewDir);
    float NdotL = clamp(dot(normal, lightDir), 0.0, 1.0);
    float NdotH = clamp(dot(normal, half_vector), 0.0, 1.0);
    float NdotV = clamp(dot(normal, viewDir), 0.0, 1.0);
    float VdotH = clamp(dot(viewDir, half_vector), 0.0, 1.0);
    float r_sq = RoughnessValue * RoughnessValue;

    // Evaluate the geometric term
    // --------------------------------
    float geo_numerator = 2.0 * NdotH;
    float geo_denominator = VdotH;

    float geo_b = (geo_numerator * NdotV) / geo_denominator;
    float geo_c = (geo_numerator * NdotL) / geo_denominator;
    float geo = min(1.0, min(geo_b, geo_c));

    // Now evaluate the roughness term
    // -------------------------------
    float roughness;

    if (ROUGHNESS_BECKMANN == RoughnessMode) {
        float roughness_a = 1.0 / (4.0 * r_sq * pow(NdotH, 4));
        float roughness_b = NdotH * NdotH - 1.0;
        float roughness_c = r_sq * NdotH * NdotH;

        roughness = roughness_a * exp(roughness_b / roughness_c);
    }
    if (ROUGHNESS_GAUSSIAN == RoughnessMode) {
        // This variable could be exposed as a variable
        // for the application to control:
        float c = 1.0;
        float alpha = acos(dot(normal, half_vector));
        roughness = c * exp(-(alpha / r_sq));
    }

    // Next evaluate the Fresnel value
    // -------------------------------
    float fresnel = pow(1.0 - VdotH, 5.0);
    fresnel *= (1.0 - RefAtNormIncidence);
    fresnel += RefAtNormIncidence;

    // Put all the terms together to compute
    // the specular term in the equation
    // -------------------------------------
    vec3 Rs_numerator = vec3(fresnel * geo * roughness);
    float Rs_denominator = NdotV * NdotL;
    vec3 Rs = Rs_numerator / Rs_denominator;
    
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Put all the parts together to generate
    // the final colour
    // --------------------------------------
    vec3 final = max(0.0, NdotL) * (material.specular * Rs + material.diffuse)*attenuation;

    // Return the result
    // -----------------
    return final;
}

/**********************************************
*Cook-Torrance end
*
***********************************************/






/**********************************************
 *Oren-Nayar start
 *
 ***********************************************/
uniform bool OrenNayarSimple = false; //! checkbox[false]
uniform float OrenRoughness = 1.0; //! slider[0, 1, 1]
uniform float PI = 3.1415926f;

vec3 orenNayarComplex(vec3 n, vec3 v, vec3 l) {
    // Compute the other aliases
    float alpha = max(acos(dot(v, n)), acos(dot(l, n)));
    float beta = min(acos(dot(v, n)), acos(dot(l, n)));
    float gamma = dot(v - n * dot(v, n), l - n * dot(l, n));
    float rough_sq = OrenRoughness * OrenRoughness;

    float C1 = 1.0 - 0.5 * (rough_sq / (rough_sq + 0.33));

    float C2 = 0.45 * (rough_sq / (rough_sq + 0.09));
    if (gamma >= 0) {
        C2 *= sin(alpha);
    } else {
        C2 *= (sin(alpha) - pow((2 * beta) / PI, 3));
    }

    float C3 = (1.0 / 8.0);
    C3 *= (rough_sq / (rough_sq + 0.09));
    C3 *= pow((4.0 * alpha * beta) / (PI * PI), 2);

    float A = gamma * C2 * tan(beta);
    float B = (1 - abs(gamma)) * C3 * tan((alpha + beta) / 2.0);

    vec3 final = material.diffuse * max(0.0, dot(n, l)) * (C1 + A + B);

    return final;
}

vec3 orenNayarSimple(vec3 n, vec3 v, vec3 l) {
    // Compute the other aliases
    float gamma = dot(v - n * dot(v, n), l - n * dot(l, n));

    float rough_sq = OrenRoughness * OrenRoughness;

    float A = 1.0 - 0.5 * (rough_sq / (rough_sq + 0.57));

    float B = 0.45 * (rough_sq / (rough_sq + 0.09));

    float alpha = max(acos(dot(v, n)), acos(dot(l, n)));
    float beta = min(acos(dot(v, n)), acos(dot(l, n)));

    float C = sin(alpha) * tan(beta);

    vec3 final = vec3(A + B * max(0.0, gamma) * C);

    return vec3(material.diffuse * max(0.0, dot(n, l)) * final);
}

vec3 orenNayar(PointLight light, vec3 normal,vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    if (OrenNayarSimple) {
        return orenNayarSimple(normal, viewDir, lightDir)*attenuation;
    }
    return orenNayarComplex(normal, viewDir, lightDir)*attenuation;
}

/**********************************************
*Oren-Nayar end
*
***********************************************/





/**********************************************
 *Ward start
 *
 ***********************************************/
uniform bool WardAnisotropic = false; //! checkbox[false]
uniform vec2 WardAnisotropicRoughness = vec2(0.5, 0.5); //! slider[(0, 0), (0.5, 0.5), (1, 1)]
uniform float WardRoughness = 1.0; //! slider[0, 1, 1]

vec3 wardIsotropic(vec3 n, vec3 v, vec3 l) {
    vec3 h = normalize(l + v);

    // Generate any useful aliases
    float VdotN = dot(v, n);
    float LdotN = dot(l, n);
    float HdotN = dot(h, n);
    float r_sq = (WardRoughness * WardRoughness) + 1e-5;
    // (Adding a small bias to r_sq stops unexpected
    //  results caused by divide-by-zero)

    // Define material properties
    vec3 Ps = vec3(1.0, 1.0, 1.0);

    // Compute the specular term
    float exp_a = -pow(tan(acos(HdotN)), 2);
    float spec_num = exp(exp_a / r_sq);

    float spec_den = 4.0 * 3.14159 * r_sq;
    spec_den *= sqrt(LdotN * VdotN);

    vec3 Specular = Ps * (spec_num / spec_den);

    // Composite the final value:
    return vec3(dot(n, l) * (material.diffuse + Specular));
}

vec3 wardAnispotropic(vec3 n, vec3 v, vec3 l) {
    vec3 h = normalize(l + v);

    // Apply a small bias to the roughness
    // coefficients to avoid divide-by-zero
    vec2 anisotropicRoughness = WardAnisotropicRoughness + vec2(1e-5, 1e-5);

    // Define the coordinate frame
    vec3 epsilon = vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(n, epsilon));
    vec3 bitangent = normalize(cross(n, tangent));

    // Define material properties
    vec3 Ps = vec3(1.0, 1.0, 1.0);

    // Generate any useful aliases
    float VdotN = dot(v, n);
    float LdotN = dot(l, n);
    float HdotN = dot(h, n);
    float HdotT = dot(h, tangent);
    float HdotB = dot(h, bitangent);

    // Evaluate the specular exponent
    float beta_a = HdotT / anisotropicRoughness.x;
    beta_a *= beta_a;

    float beta_b = HdotB / anisotropicRoughness.y;
    beta_b *= beta_b;

    float beta = -2.0 * ((beta_a + beta_b) / (1.0 + HdotN));

    // Evaluate the specular denominator
    float s_den = 4.0 * 3.14159;
    s_den *= anisotropicRoughness.x;
    s_den *= anisotropicRoughness.y;
    s_den *= sqrt(LdotN * VdotN);

    // Compute the final specular term
    vec3 Specular = Ps * (exp(beta) / s_den);

    // Composite the final value:
    return vec3(dot(n, l) * (material.diffuse + Specular));
}

vec3 ward(PointLight light, vec3 normal,vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    if (WardAnisotropic) {
        return wardAnispotropic(normal, viewDir, lightDir)*attenuation;
    } else {
        return wardIsotropic(normal, viewDir, lightDir)*attenuation;
    }
}


/**********************************************
*Ward end
*
***********************************************/
