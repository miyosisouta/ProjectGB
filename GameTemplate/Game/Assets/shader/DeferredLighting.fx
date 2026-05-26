///////////////////////////////////////
// PBR�x�[�X�̃f�B�t�@�[�h���C�e�B���O
///////////////////////////////////////


///////////////////////////////////////
// �萔
///////////////////////////////////////

static const int NUM_REFLECTION_TEXTURE = 5;    // ���˃e�N�X�`���B
///////////////////////////////////////
// �\���́B
///////////////////////////////////////

//���_�V�F�[�_�[�ւ̓��͍\���́B
struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};
//�s�N�Z���V�F�[�_�[�ւ̓��͍\���́B
struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};
///////////////////////////////////////
// PBR���C�e�B���O�֌W�̒萔
///////////////////////////////////////
#include "PBRLighting_const.h"

///////////////////////////////////////
// �萔�o�b�t�@�B
///////////////////////////////////////
#include "DeferredLighting_cav_register.h"

///////////////////////////////////////
// SRV_UAV�̃��W�X�^�ݒ�
///////////////////////////////////////
#include "DeferredLighting_srv_uav_register.h"

///////////////////////////////////////
// PBR���C�e�B���O
///////////////////////////////////////
#include "PBRLighting.h"

///////////////////////////////////////
// �V���h�E�C���O
///////////////////////////////////////
#include "Shadowing.h"

///////////////////////////////////////
// IBL
///////////////////////////////////////
#include "IBL.h"

///////////////////////////////////////
// �֐�
///////////////////////////////////////



//���_�V�F�[�_�[�B
PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}

/*!
 *@brief	GI���C�g���T���v�����O
 *@param[in]	uv				uv���W
 *@param[in]	level           ���˃��x��
 */
float4 SampleReflectionColor( float2 uv, float level )
{
    int iLevel = (int)level;
    float4 col_0;
    float4 col_1;
    if( iLevel == 0){
        col_0 = g_reflectionTextureArray[0].Sample( Sampler, uv);
        col_1 = g_reflectionTextureArray[1].Sample( Sampler, uv);
    }else if( iLevel == 1){
        col_0 = g_reflectionTextureArray[1].Sample( Sampler, uv);
        col_1 = g_reflectionTextureArray[2].Sample( Sampler, uv);
    }else if( iLevel == 2){
        col_0 = g_reflectionTextureArray[2].Sample( Sampler, uv);
        col_1 = g_reflectionTextureArray[3].Sample( Sampler, uv);
    }else if( iLevel == 3){
        col_0 = g_reflectionTextureArray[3].Sample( Sampler, uv);
        col_1 = g_reflectionTextureArray[4].Sample( Sampler, uv);
    }

    return lerp( col_0, col_1, frac(level));
}

/*!
 * @brief	UV���W�Ɛ[�x�l���烏�[���h���W���v�Z����B
 *@param[in]	uv				uv���W
 *@param[in]	zInScreen		�X�N���[�����W�n�̐[�x�l
 *@param[in]	mViewProjInv	�r���[�v���W�F�N�V�����s��̋t�s��B
 */
float3 CalcWorldPosFromUVZ( float2 uv, float zInScreen, float4x4 mViewProjInv )
{
	float3 screenPos;
	screenPos.xy = (uv * float2(2.0f, -2.0f)) + float2( -1.0f, 1.0f);
	screenPos.z = zInScreen;//depthMap.Sample(Sampler, uv).r;
	
	float4 worldPos = mul(mViewProjInv, float4(screenPos, 1.0f));
	worldPos.xyz /= worldPos.w;
	return worldPos.xyz;
}
/*!
 * @brief	�f�B���N�V�������C�g�̔��ˌ����v�Z
 *@param[in]	normal			�T�[�t�F�C�X�̖@���B
 *@param[in]	toEye           �T�[�t�F�C�X���王�_�ւ̕����x�N�g��(�P�ʃx�N�g��)
 *@param[in]	albedoColor     �A���x�h�J���[
 *@param[in]    metaric         ���^���b�N
 *@param[in]    smooth          ���炩��
 *@param[in]    specColor       �X�y�L�����J���[
 *@param[in]    worldPos        �T�[�t�F�C�X�̃��[���h���W
 *@param[in]    isSoftShadow    �\�t�g�V���h�E�H
 *@param[in]    shadowParam     �V���h�E���V�[�o�[�t���O�B
 */
float3 CalcDirectionLight(
    float3 normal, 
    float3 toEye, 
    float4 albedoColor, 
    float metaric, 
    float smooth, 
    float3 specColor,
    float3 worldPos,
    bool isSoftShadow,
    float shadowParam
){
    float3 lig = 0;
    for(int ligNo = 0; ligNo < NUM_DIRECTIONAL_LIGHT; ligNo++)
    {
        // �e�̗�������v�Z����B
        float shadow = 0.0f;
        if( light.directionalLight[ligNo].castShadow == 1){
            //�e�𐶐�����Ȃ�B
            shadow = CalcShadowRate( 
                g_shadowMap, 
                light.mlvp, 
                ligNo, 
                worldPos, 
                isSoftShadow ) * shadowParam;
        }
        
        lig += CalcLighting(
            light.directionalLight[ligNo].direction,
            light.directionalLight[ligNo].color,
            normal,
            toEye,
            albedoColor,
            metaric,
            smooth,
            specColor
        ) * ( 1.0f - shadow );
    }
    return lig;
}
/*!
 *@brief	�|�C���g���C�g�̔��ˌ����v�Z
 *@param[in]	normal			�T�[�t�F�C�X�̖@���B
 *@param[in]	toEye           �T�[�t�F�C�X���王�_�ւ̕����x�N�g��(�P�ʃx�N�g��)
 *@param[in]	albedoColor     �A���x�h�J���[
 *@param[in]    metaric         ���^���b�N
 *@param[in]    smooth          ���炩��
 *@param[in]    specColor       �X�y�L�����J���[
 *@param[in]    worldPos        �T�[�t�F�C�X�̃��[���h���W
 */
float3 CalcPointLight(
    float3 normal, 
    float3 toEye, 
    float4 albedoColor, 
    float metaric, 
    float smooth, 
    float3 specColor,
    float3 worldPos,
    float2 viewportPos
){
    float3 lig = 0;
    
    // �X�N���[���̍����(0,0)�A�E����(1,1)�Ƃ�����W�n�ɕϊ�����
    // �r���[�|�[�g���W�n�ɕϊ�����
    // ���̃s�N�Z�����܂܂�Ă���^�C���̔ԍ����v�Z����
    // �X�N���[�����^�C���ŕ��������Ƃ��̃Z����X���W�����߂�
    uint numCellX = (screenParam.z + TILE_WIDTH - 1) / TILE_WIDTH;

    // �^�C���C���f�b�N�X���v�Z����
    uint tileIndex = floor(viewportPos.x / TILE_WIDTH) + floor(viewportPos.y / TILE_HEIGHT) * numCellX;

    // �܂܂��^�C���̉e�����X�g�̊J�n�ʒu�ƏI���ʒu���v�Z����
    uint lightStart = tileIndex * light.numPointLight;
    uint lightEnd = lightStart + light.numPointLight;
    for (uint lightListIndex = lightStart; lightListIndex < lightEnd; lightListIndex++)
    {
        uint ligNo = pointLightListInTile[lightListIndex];
        if (ligNo == 0xffffffff)
        {
            // ���̃^�C���Ɋ܂܂��|�C���g���C�g�͂����Ȃ�
            break;
        }
        
        float3 ligDir = normalize(worldPos - light.pointLight[ligNo].position);
        // 2. ��������T�[�t�F�C�X�܂ł̋������v�Z
        float distance = length(worldPos - light.pointLight[ligNo].position);
        float3 ligColor = light.pointLight[ligNo].color;
        float3 ptLig = CalcLighting(
            ligDir,
            ligColor,
            normal,
            toEye,
            albedoColor,
            metaric,
            smooth,
            specColor
        );
        // 3. �e�������v�Z����B�e������0.0�`1.0�͈̔͂ŁA
        //     �w�肵�������ipointsLights[i].range�j�𒴂�����A�e������0.0�ɂȂ�
        float affect = 1.0f - min(1.0f, distance / light.pointLight[ligNo].attn.x);
        affect = pow( affect, light.pointLight[ligNo].attn.y );
        lig += ptLig * affect;
    }
    return lig;
}
/*!
 *@brief	�X�|�b�g���C�g�̔��ˌ����v�Z
 *@param[in]	normal			�T�[�t�F�C�X�̖@���B
 *@param[in]	toEye           �T�[�t�F�C�X���王�_�ւ̕����x�N�g��(�P�ʃx�N�g��)
 *@param[in]	albedoColor     �A���x�h�J���[
 *@param[in]    metaric         ���^���b�N
 *@param[in]    smooth          ���炩��
 *@param[in]    specColor       �X�y�L�����J���[
 *@param[in]    worldPos        �T�[�t�F�C�X�̃��[���h���W
 */
float3 CalcSpotLight(
    float3 normal, 
    float3 toEye, 
    float4 albedoColor, 
    float metaric, 
    float smooth, 
    float3 specColor,
    float3 worldPos,
    float2 viewportPos
){
    float3 lig = 0;
    // �X�N���[���̍����(0,0)�A�E����(1,1)�Ƃ�����W�n�ɕϊ�����
    // �r���[�|�[�g���W�n�ɕϊ�����

    // ���̃s�N�Z�����܂܂�Ă���^�C���̔ԍ����v�Z����
    // �X�N���[�����^�C���ŕ��������Ƃ��̃Z����X���W�����߂�
    uint numCellX = (screenParam.z + TILE_WIDTH - 1) / TILE_WIDTH;

    // �^�C���C���f�b�N�X���v�Z����
    uint tileIndex = floor(viewportPos.x / TILE_WIDTH) + floor(viewportPos.y / TILE_HEIGHT) * numCellX;

    // �����ăX�|�b�g���C�g�B
    uint lightStart = tileIndex * light.numSpotLight;
    uint lightEnd = lightStart + light.numSpotLight;
    for (uint lightListIndex = lightStart; lightListIndex < lightEnd; lightListIndex++)
    {
        uint ligNo = spotLightListInTile[lightListIndex];
        if (ligNo == 0xffffffff)
        {
            // ���̃^�C���Ɋ܂܂��|�C���g���C�g�͂����Ȃ�
            break;
        }
        
        float3 ligDir = normalize(worldPos - light.spotLight[ligNo].position);
        // 2. ��������T�[�t�F�C�X�܂ł̋������v�Z
        float distance = length(worldPos - light.spotLight[ligNo].position);
        float3 ligColor = light.spotLight[ligNo].color;
        float3 ptLig = CalcLighting(
            ligDir,
            ligColor,
            normal,
            toEye,
            albedoColor,
            metaric,
            smooth,
            specColor
        );
        // 3. �e�������v�Z����B�e������0.0�`1.0�͈̔͂ŁA
        //     �w�肵�������ipointsLights[i].range�j�𒴂�����A�e������0.0�ɂȂ�
        float affect = pow( 1.0f - min(1.0f, distance / light.spotLight[ligNo].range), light.spotLight[ligNo].rangePow.x);

        // ���ˌ��Ǝˏo�����̊p�x�ɂ�錸�����v�Z����
        // dot()�𗘗p���ē��ς����߂�
        float angleLigToPixel = dot(ligDir, light.spotLight[ligNo].direction);
        // dot()�ŋ��߂��l��acos()�ɓn���Ċp�x�����߂�
        angleLigToPixel = abs(acos(angleLigToPixel));
        // step-12 �p�x�ɂ��e���������߂�
        // �p�x�ɔ�Ⴕ�ď������Ȃ��Ă����e�������v�Z����
        float angleAffect = pow( max( 0.0f, 1.0f - 1.0f / light.spotLight[ligNo].angle.x * angleLigToPixel ), light.spotLight[ligNo].anglePow.x);
        affect *= angleAffect;

        
        lig += ptLig * affect * angleAffect;
    }
    return lig;
}
//�s�N�Z���V�F�[�_�[�̃R�A�B
float4 PSMainCore(PSInput In, uniform int isSoftShadow) 
{
    //G-Buffer�̓��e���g���ă��C�e�B���O
    //�A���x�h�J���[���T���v�����O�B
    float4 albedoColor = albedoTexture.Sample(Sampler, In.uv);
    //�@�����T���v�����O�B
    float3 normal = normalTexture.Sample(Sampler, In.uv).xyz;
    // 自発光ピクセル (AttackRange 等): normal=(0,0,0) → ライティングをバイパスして albedo 直接出力
    [branch]
    if (dot(normal, normal) < 0.01f)
        return float4(albedoColor.rgb, 1.0f);
    //���[���h���W���T���v�����O�B
    float3 worldPos = CalcWorldPosFromUVZ(In.uv, albedoColor.w, light.mViewProjInv);
    //�X�y�L�����J���[���T���v�����O�B
    float3 specColor = albedoColor.xyz;
    //�����x���T���v�����O�B
    float metaric = metallicShadowSmoothTexture.SampleLevel(Sampler, In.uv, 0).r;
    //�X���[�X
    float smooth = metallicShadowSmoothTexture.SampleLevel(Sampler, In.uv, 0).a;

    //�e�����p�̃p�����[�^�B
    float shadowParam = metallicShadowSmoothTexture.Sample(Sampler, In.uv).g;
    
    float2 viewportPos = In.pos.xy;

    // �����Ɍ������ĐL�т�x�N�g�����v�Z����
    float3 toEye = normalize(light.eyePos - worldPos);
    
    // �f�B���N�V�������C�g���v�Z
    float3 lig = CalcDirectionLight(
        normal, 
        toEye, 
        albedoColor, 
        metaric, 
        smooth, 
        specColor, 
        worldPos,
        isSoftShadow,
        shadowParam
    );
    
    lig += CalcPointLight(
        normal, 
        toEye, 
        albedoColor, 
        metaric, 
        smooth, 
        specColor, 
        worldPos,
        viewportPos
    );
    
    lig += CalcSpotLight(
        normal, 
        toEye, 
        albedoColor, 
        metaric, 
        smooth, 
        specColor, 
        worldPos,
        viewportPos
    );
    
    if(light.isEnableRaytracing){
        // ���C�g�����s���ꍇ�̓��C�g���ō�������˃e�N�X�`����IBL�e�N�X�`������������B
        // GL�e�N�X�`��
        float reflectionRate = 1.0f - ( ( smooth - 0.5f ) * 2.0f );
        float level = lerp(0.0f, (float)(NUM_REFLECTION_TEXTURE - 1 ), pow( reflectionRate, 3.0f ));
        if( level < NUM_REFLECTION_TEXTURE-1){
            lig += albedoColor * SampleReflectionColor(In.uv, level);
        }else if (light.isIBL == 1) {
            // IBL������Ȃ�B
            lig += albedoColor * SampleIBLColorFromSkyCube(
                g_skyCubeMap, 
                toEye, 
                normal, 
                smooth,
                light.iblIntencity
            );
        }else{
            // �����ɂ���グ
            lig += light.ambientLight * albedoColor;    
        }
    }else if (light.isIBL == 1) {
        // ��������̔��˃x�N�g�������߂�B
        lig += albedoColor * SampleIBLColorFromSkyCube(
            g_skyCubeMap, 
            toEye, 
            normal, 
            smooth,
            light.iblIntencity 
        );
    }
    else {
        // �����ɂ���グ
        lig += light.ambientLight * albedoColor;
    }
   
    float4 finalColor = 1.0f;
    finalColor.xyz = lig;
    return finalColor;
}
float4 PSMainSoftShadow(PSInput In) : SV_Target0
{
    return PSMainCore( In, true);
}
//�n�[�h�V���h�E���s���s�N�Z���V�F�[�_�[�B
float4 PSMainHardShadow(PSInput In) : SV_Target0
{
    return PSMainCore( In, false);
} 