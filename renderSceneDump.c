
  void renderScene(GLuint curShader){
    /*    glUseProgram(shadersId[lightSourceShader]);

    for (int i = 0; i < lightsStoreSize; i++) {
      //	  renderCube(lightsStore[i].pos, lightsStore[i].id);
      float intersectionDistance;

      bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lightsStore[i].lb, lightsStore[i].rt, NULL, &intersectionDistance);

      if (isIntersect// && minDistToCamera > intersectionDistance
	  ) {
	mouse.selectedThing = &lightsStore[i];
	mouse.selectedType = mouseLightT;

	// minDistToCamera = intersectionDistance;
      }

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, solidColorTx);

      if (lightsStore[i].off) {
	setSolidColorTx(greyColor, 1.0f);
      }
      else {
	setSolidColorTx(whiteColor, 1.0f);
      }

      glBindBuffer(GL_ARRAY_BUFFER, cube.VBO);
      glBindVertexArray(cube.VAO);

      //	    glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, lightsStore[i].mat.m);
      uniformMat4(lightSourceShader, "model", lightsStore[i].mat.m);

      glDrawArrays(GL_TRIANGLES, 0, cube.vertexNum);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, 0);
      
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      }

      glUseProgram(shadersId[curShader]);

      glStencilMask(0x00);*/

    
    for (int i = 0; i < curModelsSize; i++) {

      int name = curModels[i].name;

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, loadedModels1D[name].tx);

      // glStencilFunc(GL_ALWAYS, 1, 0xFF);
      //      glStencilMask(0xFF);

      glBindVertexArray(loadedModels1D[name].VAO);
      glBindBuffer(GL_ARRAY_BUFFER, loadedModels1D[name].VBO);

      //	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, curModels[i].mat.m);
      uniformMat4(curShader, "model", curModels[i].mat.m);

      glDrawArrays(GL_TRIANGLES, 0, loadedModels1D[name].size);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    Matrix out2 = IDENTITY_MATRIX;
   
    uniformMat4(curShader, "model", out2.m);

    for (int i = 0; i < loadedTexturesCounter; i++) {
      //   if(loadedTextures1D[i].tx == 13 && curShader == 5) continue; // 12 - zemlia
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, loadedTextures1D[i].tx);

      glBindBuffer(GL_ARRAY_BUFFER, geometry[i].pairs.VBO);
      glBindVertexArray(geometry[i].pairs.VAO);

      glDrawArrays(GL_TRIANGLES, 0, geometry[i].tris);

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }


    
    /*

    {
      // plane things
      {
	// draw plane on cur creation
	glBindVertexArray(planePairs.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, planePairs.VBO);

	if (planeOnCreation) {
	  //	  glActiveTexture(errorTx);

	  glActiveTexture(GL_TEXTURE0);
	  glBindTexture(GL_TEXTURE_2D, errorTx);

	  float w = planeOnCreation->w / 2;
	  float h = planeOnCreation->h / 2;

	  float planeModel[] = {
	    -w, h, 0.0f, 0.0f, 1.0f,
	    w, h, 0.0f , 1.0f, 1.0f,
	    -w, -h, 0.0f , 0.0f, 0.0f,

	    w, h, 0.0f , 1.0f, 1.0f,
	    -w, -h, 0.0f, 0.0f, 0.0f,
	    w, -h, 0.0f, 1.0f, 0.0f
	  };

	  glBufferData(GL_ARRAY_BUFFER, sizeof(planeModel), planeModel, GL_STATIC_DRAW);

	  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	  glEnableVertexAttribArray(1);

	  //  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, planeOnCreation->mat.m);
	  uniformMat4(curShader, "model", planeOnCreation->mat.m);


	  glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	// draw planes
	for (int i = 0; i < createdPlanesSize; i++) {
	  float intersectionDistance;

	  Picture plane = createdPlanes[i];

	  float w = plane.w / 2;
	  float h = plane.h / 2;

	  vec3 lb = { -w, -h, -0.05f };
	  vec3 rt = { w, h, 0.05f };

	  lb.x += plane.mat.m[12];
	  lb.y += plane.mat.m[13];
	  lb.z += plane.mat.m[14];

	  rt.x += plane.mat.m[12];
	  rt.y += plane.mat.m[13];
	  rt.z += plane.mat.m[14];

	  bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, lb, rt, NULL, &intersectionDistance);

	  if (isIntersect //&& minDistToCamera > intersectionDistance
	      ) {
	    mouse.selectedThing = &createdPlanes[i];
	    mouse.selectedType = mousePlaneT;

	    //  minDistToCamera = intersectionDistance;
	  }

	  glActiveTexture(GL_TEXTURE0);
	  glBindTexture(GL_TEXTURE_2D, loadedTextures1D[plane.txIndex].tx);


	  float planeModel[] = {
	    -w, h, 0.0f, 0.0f, 1.0f,
	    w, h, 0.0f , 1.0f, 1.0f,
	    -w, -h, 0.0f , 0.0f, 0.0f,

	    w, h, 0.0f , 1.0f, 1.0f,
	    -w, -h, 0.0f, 0.0f, 0.0f,
	    w, -h, 0.0f, 1.0f, 0.0f
	  };

	  glBufferData(GL_ARRAY_BUFFER, sizeof(planeModel), planeModel, GL_STATIC_DRAW);

	  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
	  glEnableVertexAttribArray(0);

	  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	  glEnableVertexAttribArray(1);

	  //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, plane.mat.m);
	  uniformMat4(curShader, "model", plane.mat.m);


	  glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
      }
    }

    instances[curInstance][render3DFunc]();

	
    //      editor3dRender();
    // snowParticles rendering
    if (false)
      {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, solidColorTx);

	setSolidColorTx(whiteColor, 1.0f);

	//   glBindVertexArray(snowFlakesMeshVAO);
	// glBindBuffer(GL_ARRAY_BUFFER, snowFlakesMeshVBO);

	int vertexIndex = 0;
	for (int loop = 0; loop < snowAmount; loop++)
	  {
	    if (snowParticle[loop].active) {
	      float x = snowParticle[loop].x;
	      float y = snowParticle[loop].y;
	      float z = snowParticle[loop].z;

	      snowMeshVertixes[vertexIndex] = x;
	      snowMeshVertixes[vertexIndex + 1] = y;
	      snowMeshVertixes[vertexIndex + 2] = z;

	      snowMeshVertixes[vertexIndex + 3] = x;
	      snowMeshVertixes[vertexIndex + 4] = y - 0.015f / 4.0f;
	      snowMeshVertixes[vertexIndex + 5] = z;

	      vertexIndex += 6;

	      vec3i gridIndexes = xyz_coordsToIndexes(x, y, z);

	      GroundType type = -1;

	      //	      if (gridIndexes.y < gridY - 1) {
	      //		type = valueIn(grid[gridIndexes.y + 1][gridIndexes.z][gridIndexes.x]->ground, 0);
	      //	      }

	      if (snowParticle[loop].y < 0.0f || type == texturedTile) {
		snowParticle[loop].life -= snowParticle[loop].fade / 10.0f;

		if (snowParticle[loop].life < 0.0f) {
		  snowParticle[loop].active = true;
		  snowParticle[loop].life = 1.0f;
		  snowParticle[loop].fade = (float)(rand() % 100) / 1000.0f + 0.003f;

		  snowParticle[loop].x = (float)(rand() % gridX) + (float)(rand() % 100 / 1000.0f);
		  snowParticle[loop].y = (gridY - 1) * floorH;
		  snowParticle[loop].z = (float)(rand() % gridZ) + (float)(rand() % 100 / 1000.0f);
		}
	      }
	      else {
		snowParticle[loop].y += snowSpeed / (1000 / 2.0f);
	      }
	    }
	  }

	Matrix out = IDENTITY_MATRIX;

	out.m[12] = 0.0;
	out.m[13] = 0.0;
	out.m[14] = 0.0f;

	// glUniformMatrix4fv(modelLoc, 1, GL_FALSE, out.m);
	uniformMat4(curShader, "model", out.m);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 3 * snowAmount
		     , snowMeshVertixes, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_LINES, 0, 2 * snowAmount);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
      }

    //minDistToCamera = 1000.0f;

    for (int i = 0; i < curModelsSize; i++) {
      float intersectionDistance = 0.0f;

      bool isIntersect = rayIntersectsTriangle(curCamera->pos, mouse.rayDir, curModels[i].lb, curModels[i].rt, NULL, &intersectionDistance);

      int name = curModels[i].name;

      if (isIntersect// && minDistToCamera > intersectionDistance
	  ) {
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	mouse.selectedThing = &curModels[i];
	mouse.selectedType = mouseModelT;

	//                minDistToCamera = intersectionDistance;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, loadedModels1D[name].tx);

	glBindVertexArray(loadedModels1D[name].VAO);
	glBindBuffer(GL_ARRAY_BUFFER, loadedModels1D[name].VBO);

	Matrix borderMat;
	memcpy(borderMat.m, curModels[i].mat.m, sizeof(float) * 16);

	float xTemp = borderMat.m[12];
	float yTemp = borderMat.m[13];
	float zTemp = borderMat.m[14];

	borderMat.m[12] = 0;
	borderMat.m[13] = 0;
	borderMat.m[14] = 0;

	//	scale(borderMat.m, 1.05f, 1.05f, 1.05f);

	borderMat.m[12] = xTemp;
	borderMat.m[13] = yTemp;
	borderMat.m[14] = zTemp;

	glUseProgram(shadersId[borderShader]);

	uniformVec3(borderShader, "borderColor", (vec3) { redColor });
	uniformMat4(borderShader, "model", borderMat.m);

	//glUniformMatrix4fv(borderModelLoc, 1, GL_FALSE, borderMat.m);     

	glDrawArrays(GL_TRIANGLES, 0, loadedModels1D[name].size);

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(shadersId[curShader]);
	glStencilMask(0x00);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	break;
      }
    }

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should pass the stencil test
    glStencilMask(0xFF);
*/
  }
