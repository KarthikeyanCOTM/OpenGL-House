// CS370 Final Project
// Fall 2023

// Create solid color buffer
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer) {
    vector<vec4> obj_colors;
    for (int i = 0; i < num_vertices; i++) {
        obj_colors.push_back(color);
    }

    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[buffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*colCoords*num_vertices, obj_colors.data(), GL_STATIC_DRAW);
}

void load_model(const char * filename, GLuint obj) {
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    // Load model and set number of vertices
    loadOBJ(filename, vertices, uvCoords, normals);
    numVertices[obj] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[obj], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void load_texture(const char * filename, GLuint texID, GLint magFilter, GLint minFilter, GLint sWrap, GLint tWrap, bool mipMap, bool invert) {
    int w, h, n;
    int force_channels = 4;
    unsigned char *image_data;

    // Activate unit 0
    glActiveTexture( GL_TEXTURE0 );

    image_data = stbi_load(filename, &w, &h, &n, force_channels);
    if (!image_data) {
        fprintf(stderr, "ERROR: could not load %s\n", filename);
    }
    // NPOT check for power of 2 dimensions
    if ((w & (w - 1)) != 0 || (h & (h - 1)) != 0) {
        fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n", filename);
    }
    
    // Invert image (e.g. jpeg, png)
    if (invert) {
        int width_in_bytes = w * 4;
        unsigned char *top = NULL;
        unsigned char *bottom = NULL;
        unsigned char temp = 0;
        int half_height = h / 2;

        for ( int row = 0; row < half_height; row++ ) {
            top = image_data + row * width_in_bytes;
            bottom = image_data + ( h - row - 1 ) * width_in_bytes;
            for ( int col = 0; col < width_in_bytes; col++ ) {
                temp = *top;
                *top = *bottom;
                *bottom = temp;
                top++;
                bottom++;
            }
        }
	}

    // Bind current texture id
    glBindTexture(GL_TEXTURE_2D, TextureIDs[texID]);
    // Load image data into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 image_data);
    // Generate mipmaps for texture
    if (mipMap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    // Set scaling modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    // Set wrapping modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tWrap);
    // Set maximum anisotropic filtering for system
    GLfloat max_aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
    // set the maximum!
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
}

// Draw object with color
void draw_color_obj(GLuint obj, GLuint color) {
    // Select default shader program
    glUseProgram(default_program);

    // Pass projection matrix to default shader
    glUniformMatrix4fv(default_proj_mat_loc, 1, GL_FALSE, proj_matrix);

    // Pass camera matrix to default shader
    glUniformMatrix4fv(default_cam_mat_loc, 1, GL_FALSE, camera_matrix);

    // Pass model matrix to default shader
    glUniformMatrix4fv(default_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes for default shader
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(default_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(default_vPos);

    // Bind color buffer and set attributes for default shader
    glBindBuffer(GL_ARRAY_BUFFER, ColorBuffers[color]);
    glVertexAttribPointer(default_vCol, colCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(default_vCol);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_mat_object(GLuint obj, GLuint material){
    // Reference appropriate shader variables
    if (shadow) {
        // Use shadow shader
        glUseProgram(shadow_program);
        // Pass shadow projection and camera matrices to shader
        glUniformMatrix4fv(shadow_proj_mat_loc, 1, GL_FALSE, shadow_proj_matrix);
        glUniformMatrix4fv(shadow_camera_mat_loc, 1, GL_FALSE, shadow_camera_matrix);

        // Set object attributes to shadow shader
        vPos = shadow_vPos;
        model_mat_loc = shadow_model_mat_loc;
    } else {
        // Use lighting shader with shadows
        glUseProgram(phong_shadow_program);

        // Pass object projection and camera matrices to shader
        glUniformMatrix4fv(phong_shadow_proj_mat_loc, 1, GL_FALSE, proj_matrix);
        glUniformMatrix4fv(phong_shadow_camera_mat_loc, 1, GL_FALSE, camera_matrix);

        // Bind lights
        glUniformBlockBinding(phong_shadow_program, phong_shadow_lights_block_idx, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size() * sizeof(LightProperties));

        // Bind materials
        glUniformBlockBinding(phong_shadow_program, phong_shadow_materials_block_idx, 1);
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0,
                          Materials.size() * sizeof(MaterialProperties));

        // Set camera position
        glUniform3fv(phong_shadow_eye_loc, 1, eye);

        // Set num lights and lightOn
        glUniform1i(phong_shadow_num_lights_loc, Lights.size());
        glUniform1iv(phong_shadow_light_on_loc, numLights, lightOn);

        // Pass normal matrix to shader
        glUniformMatrix4fv(phong_shadow_norm_mat_loc, 1, GL_FALSE, normal_matrix);

        // Pass material index to shader
        glUniform1i(phong_shadow_material_loc, material);

        glUniformMatrix4fv(phong_shadow_shad_proj_mat_loc, 1, GL_FALSE, shadow_proj_matrix);
        glUniformMatrix4fv(phong_shadow_shad_cam_mat_loc, 1, GL_FALSE, shadow_camera_matrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureIDs[ShadowTex]);

        // Set object attributes for phong shadow shader
        vPos = phong_shadow_vPos;
        vNorm = phong_shadow_vNorm;
        model_mat_loc = phong_shadow_model_mat_loc;
    }

    // Pass model matrix to shader
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(vPos);

    if (!shadow) {
        // Bind object normal buffer if using phong shadow shader
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
        glVertexAttribPointer(vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(vNorm);
    }

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_tex_object(GLuint obj, GLuint texture){
    // Select shader program
    glUseProgram(texture_program);

    // Pass projection matrix to shader
    glUniformMatrix4fv(texture_proj_mat_loc, 1, GL_FALSE, proj_matrix);

    // Pass camera matrix to shader
    glUniformMatrix4fv(texture_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Pass model matrix to shader
    glUniformMatrix4fv(texture_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureIDs[texture]);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(texture_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(texture_vPos);

    // Bind texture object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glVertexAttribPointer(texture_vTex, texCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(texture_vTex);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    ww = width;
    hh = height;
}

