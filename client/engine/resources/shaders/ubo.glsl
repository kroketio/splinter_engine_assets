layout (std140) uniform CameraInfo { // uniform size: 144
    //                    // base align  // aligned offset
    mat4 projMat;         // 64          // 0
    mat4 viewMat;         // 64          // 64
    vec4 viewPos;         // 16          // 128
};

layout(std140) uniform ModelInfo {  // uniform size: 160 bytes
    mat4 modelMat;        // offset 0
    mat4 normalMat;       // offset 64
    int sizeFixed;        // offset 128
    int selected;         // offset 132
    int highlighted;      // offset 136
    uint pickingID;       // offset 140
    int padding1;         // offset 144
    int padding2;         // offset 148
    int padding3;         // offset 152
};

layout (std140) uniform MaterialInfo { // uniform size: 48
    PhongMaterial material;
};

layout (std140) uniform LightInfo { // uniform size: 1424
    //                                    // base align  // aligned offset
    int ambientLightNum;                  // 4           // 0
    int directionalLightNum;              // 4           // 4
    int pointLightNum;                    // 4           // 8
    int spotLightNum;                     // 4           // 12
    AmbientLight ambientLight[8];         // 16          // 16
    DirectionalLight directionalLight[8]; // 32          // 144
    PointLight pointLight[8];             // 48          // 400
    SpotLight spotLight[8];               // 80          // 784
};
