/*
Jerom Estrada
CS134 Computer Game Design - Sec 01

Final Project - 3D Lunar Lander

A landing simulation that uses Particle Physics and Octree to simulate the landing of LEM on the Moon (terrain is not faithful to the moon's real landscape when it was generated).

05 / 20 / 2021


Initial code written by Prof. Kevin M. Smith for CS134
*/

//  CS 134 - In-Class exercise - Getting started with Particle Physics
//
//  Kevin M. Smith - CS 134 - SJSU CS


//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name:   Jerom Estrada
//  Date: 05 / 01 / 2021


#include "ofApp.h"
#include "Util.h"
#include <glm/gtx/intersect.hpp>


//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
	// start up with simulation off
	ofToggleFullscreen();
	bStartSim = false;
	bAGLToggle = false;

	// we fill up the ship with 2-min worth of fuel.
	fuel = 120000.0f; // 120 000 milliseconds, which is 2 mins. Using ms for a more accurate representation.
	fuelTracker = fuel;
	startThrust = 0; // initialize the start thrust marker.
	expending = false;

	moveThrust = 0; // initialize the move thrust marker
	movingF = false;
	movingB = false;

	rotThrust = 0; // initialize the rotational thrust marker.
	rotatingL = false;
	rotatingR = false;

	thrustPower = 0.126;
	exploded = false;
	failed = false;
	success = false;

	// texture loading
	//
	ofDisableArbTex();     // disable rectangular textures

	thrusters.loadSound("sounds/Rocket Thrusters-SoundBible.com-1432176431.mp3");
	thrusters.play();
	thrusters.setVolume(0.09);
	thrusters.setLoop(true);

	outOfFuelAlert.loadSound("sounds/BOMB_SIREN-BOMB_SIREN-247265934.mp3");
	outOfFuelAlert.setVolume(0.09);
	outOfFuelAlert.setLoop(true);
	alertPlaying = false;

	boom.loadSound("sounds/Explosion_Ultra_Bass-Mark_DiAngelo-1810420658.mp3");
	boom.setVolume(1);


	// load textures
	//
	if (!ofLoadImage(particleTex, "images/dot.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	if (!ofLoadImage(particleTexEx, "images/Explosion.png")) {
		cout << "Particle Texture File: images/dot.png not found" << endl;
		ofExit();
	}

	// load the shader
	//
#ifdef TARGET_OPENGLES
	shader.load("shaders_gles/shader");
	shaderEx.load("shaders_gles/shader");
#else
	shader.load("shaders/shader");
	shaderEx.load("shaders/shader");
#endif

	bg.loadImage("images/Background.png");

	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
//	ofSetWindowShape(1024, 768);


	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	mars.loadModel("geo/moon-houdini.obj");
	mars.setScaleNormalization(false);

	lander.loadModel("geo/lander.obj");
	lander.setScaleNormalization(false);
	bLanderLoaded = true;

	landerPos = lander.getPosition();
	landerPos += glm::vec3(0, 15, 0); // move the lander up a bit so it doesn't collide with the terrain right away.
	startPoint = glm::vec3(ofRandom(-200, 200), 100, ofRandom(-200, 200));
	lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
	shipHeading = glm::vec3(1, 0, 0); // the ship is facing towards the positive x-axis.
	

	trailerCam.setTarget(glm::vec3(0));
	trailerCam.setDistance(50);
	trailerCam.setNearClip(0.01);
	trailerCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	// setup the cameras
	cam.setPosition(lander.getPosition() - ofVec3f(15, -5, 0));
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	cam.setTarget(lander.getPosition());
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

	explosionCam.setNearClip(.1);
	explosionCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format

	scoutCam.setDistance(30);
	scoutCam.setNearClip(.1);
	scoutCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
	scoutCam.setTarget(lander.getPosition());
	scoutCam.disableMouseInput();

	sceneCam.setPosition(-150, 25, -150);
	sceneCam.lookAt(lander.getPosition());

	landerBotCam.setPosition(lander.getPosition() + ofVec3f(1.8, 4, 1.8));
	landerBotCam.lookAt(lander.getPosition() - ofVec3f(0, 10, 0));
	landerBotCam.setNearClip(.1);
	landerBotCam.setFov(65.5);   // approx equivalent to 28mm in 35mm format

	theCam = &cam;

	keyLight.setup();
	keyLight.enable();
	keyLight.setAreaLight(1, 1);
	keyLight.setAmbientColor(ofFloatColor(0.1, 0.001, 0.001));
	keyLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	keyLight.setSpecularColor(ofFloatColor(1, 1, 1));
	keyLight.setPosition(-950, 260, 20);

	fillLight.setup();
	fillLight.enable();
	fillLight.setSpotlight();
	fillLight.setScale(.05);
	fillLight.setSpotlightCutOff(15);
	fillLight.setAttenuation(2, .00009, .00009);
	fillLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	fillLight.setDiffuseColor(ofFloatColor(0.1, 1, 0.1));
	fillLight.setSpecularColor(ofFloatColor(1, 1, 1));
	fillLight.setPosition(0, 35, 0);

	rimLight.setup();
	rimLight.enable();
	rimLight.setSpotlight();
	rimLight.setScale(.05);
	rimLight.setSpotlightCutOff(30);
	rimLight.setAttenuation(.2, .00001, .00001);
	rimLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	rimLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	rimLight.setSpecularColor(ofFloatColor(1, 1, 1));
	rimLight.setPosition(163, 714, -193);

	// setup camera gui
	camGui.setup();
	camGui.add(camPos.setup("Camera Position", lander.getPosition(), ofVec3f(-20, -20, -20), ofVec3f(20, 20, 20)));


	camGui.add(sKeyPos.setup("Key Pos: ", keyLight.getPosition(), ofVec3f(-1000, -1000, -1000), ofVec3f(1000, 1000, 1000)));
	camGui.add(sFillPos.setup("Fill Pos: ", fillLight.getPosition(), ofVec3f(-1000, -1000, -1000), ofVec3f(1000, 1000, 1000)));
	camGui.add(sRimPos.setup("Rim Pos: ", rimLight.getPosition(), ofVec3f(-1000, -1000, -1000), ofVec3f(1000, 1000, 1000)));

	camGui.add(sKeyLookAt.setup("Key Pos Look At: ", ofVec3f(0, 0, 0), ofVec3f(-1000, -1000, -1000), ofVec3f(1000, 1000, 1000)));
	camGui.add(sFillLookAt.setup("Fill Pos Look At: ", ofVec3f(0, 0, 0), ofVec3f(-1000, -1000, -1000), ofVec3f(1000, 1000, 1000)));
	camGui.add(sRimLookAt.setup("Rim Pos Look At: ", ofVec3f(-40, -194, 30), ofVec3f(-1000, -1000, -1000), ofVec3f(1000, 1000, 1000)));
	/*
	camGui.add(sKeySelect.setup("Key selected axis: ", ofVec3f(1, 0, 0), ofVec3f(0, 0, 0), ofVec3f(1, 1, 1)));
	camGui.add(sKeyDeg.setup("Key Degrees: ", 1, 0, 360));
	camGui.add(sFillSelect.setup("Key selected axis: ", ofVec3f(1, 0, 0), ofVec3f(0, 0, 0), ofVec3f(1, 1, 1)));
	camGui.add(sFillDeg.setup("Fill Degrees: ", 1, 0, 360));
	camGui.add(sRimSelect.setup("Key selected axis: ", ofVec3f(1, 0, 0), ofVec3f(0, 0, 0), ofVec3f(1, 1, 1)));
	camGui.add(sRimDeg.setup("Rim Degrees: ", 1, 0, 360));
	*/

	// create sliders for testing
	//
	gui.setup();

	// Lander Sliders
	gui.add(numLevels.setup("Number of Octree Levels: ", 1, 1, 10));
	gui.add(gravityVal.setup("Gravity: ", 0.0715, 0.01, 1));
	gui.add(thrustVal.setup("Lander's Main Thrusters: ", 0.126, 0.01, 3));
	gui.add(rotationThrustRatioVal.setup("Rotation Ratio to Main Thrust: ", 3, 1, 5));
	gui.add(movementThrustRatioVal.setup("Movement Ratio to Main Thrust: ", 1.25, 1, 5));

	// Particle Sliders
	gui.add(emitterPos.setup("Exhaust offset (Y): ", 1, -2, 1));
	gui.add(numParticles.setup("Number of Particles", 120, 0, 1000));
	gui.add(lifespanRange.setup("Lifespan Range", ofVec2f(0.2, 0.9), ofVec2f(.1, .2), ofVec2f(3, 10)));
	gui.add(mass.setup("Mass", 5, .1, 10));
	gui.add(damping.setup("Damping", .99, .9, 1.0));
	gui.add(gravity.setup("Gravity", 10, -20, 20));
	gui.add(radius.setup("Radius", 6, 1, 10));
	gui.add(turbMin.setup("Turbulence Min", ofVec3f(-20, -20, -20), ofVec3f(-20, -20, -20), ofVec3f(20, 20, 20)));
	gui.add(turbMax.setup("Turbulence Max", ofVec3f(20, 20, 20), ofVec3f(-20, -20, -20), ofVec3f(20, 20, 20)));
	gui.add(radialForceVal.setup("Radial Force", 100, 100, 5000));
	gui.add(radialHight.setup("Radial Height", .2, .1, 1.0));
	gui.add(cyclicForceVal.setup("Cyclic Force", 10, 10, 500));

	bHide = false;

	// Create Forces for the exhaust
	//
	turbForce = new TurbulenceForce(ofVec3f(turbMin->x, turbMin->y, turbMin->z), ofVec3f(turbMax->x, turbMax->y, turbMax->z));
	gravityForce = new GravityForce(ofVec3f(0, -gravity, 0));
	radialForce = new ImpulseRadialForce(radialForceVal);
	directionalForce = new ImpulseDirectionalForce(radialForceVal / 2, ofVec3f(0, -1, 0));
	cyclicForce = new CyclicForce(cyclicForceVal);


	// set up the emitter
	// 
	emitter.sys->addForce(turbForce);
	emitter.sys->addForce(gravityForce);
	emitter.sys->addForce(radialForce);
	emitter.sys->addForce(directionalForce);
	emitter.sys->addForce(cyclicForce);

	emitter.setPosition(ofVec3f(lander.getPosition() + ofVec3f(0, emitterPos, 0)));
	emitter.setVelocity(ofVec3f(0, 0, 0));
	emitter.setOneShot(true);
	emitter.setEmitterType(DirectionalEmitter);
	emitter.setGroupSize(numParticles);
	emitter.setRandomLife(true);
	emitter.setLifespanRange(ofVec2f(lifespanRange->x, lifespanRange->y));

	exTurbForce = new TurbulenceForce(ofVec3f(-50, -50, -20), ofVec3f(50, 50, 50));
	exRadialForce = new ImpulseRadialForce(500);
	exCyclicForce = new CyclicForce(5);

	explosion.sys->addForce(exTurbForce);
	explosion.sys->addForce(exRadialForce);
	explosion.sys->addForce(exCyclicForce);

	explosion.setPosition(ofVec3f(lander.getPosition()));
	explosion.setVelocity(ofVec3f(0, 0, 0));
	explosion.setOneShot(true);
	explosion.setEmitterType(RadialEmitter);
	explosion.setGroupSize(1000);
	explosion.setRandomLife(true);
	explosion.setLifespanRange(ofVec2f(5, 10));
	
	boundingBox = Octree::meshBounds(mars.getMesh(0));
	//  Create Octree for testing.
	//
	float currentTime = ofGetElapsedTimeMillis();
	octree.create(mars.getMesh(0), 50);
	float elapsed = ofGetElapsedTimeMillis() - currentTime;
	cout << "Octree Creation took " << elapsed << " ms" << endl;
	
	cout << "Number of Verts: " << mars.getMesh(0).getNumVertices() << endl;

	testBox = Box(Vector3(3, 3, 0), Vector3(5, 5, 2));

}

// load vertex buffer in preparation for rendering
//
void ofApp::loadVbo() {
	if (emitter.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < emitter.sys->particles.size(); i++) {
		points.push_back(emitter.sys->particles[i].position);
		sizes.push_back(ofVec3f(radius));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vbo.clear();
	vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

void ofApp::loadExVbo() {
	if (explosion.sys->particles.size() < 1) return;

	vector<ofVec3f> sizes;
	vector<ofVec3f> points;
	for (int i = 0; i < explosion.sys->particles.size(); i++) {
		points.push_back(explosion.sys->particles[i].position);
		sizes.push_back(ofVec3f(10));
	}
	// upload the data to the vbo
	//
	int total = (int)points.size();
	vboEx.clear();
	vboEx.setVertexData(&points[0], total, GL_STATIC_DRAW);
	vboEx.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}

// Integrator for simple trajectory physics
//
void ofApp::integrate() {

	float dt = 1.0 / (1.0 + ofGetFrameRate()); // this line is bugging out so much, I don't know why... ofGetFrameRate sometimes returns 0 and it messes the calculations

	if (fuel <= 0 || fuelTracker <= 0) {
		fForwardThrust = fBackThrust = fMainThrust = fCounterClockThrust = fClockwiseThrust = 0; // no more fuel
		bEmitting = false;
		thrusters.setVolume(0.01);
		isPlaying = false;
	}
	// handle the rotation integration
	fLanderRot += fRotationSpeed *dt;
	lander.setRotation(1, fLanderRot-90, 0, 1, 0);
	fRotationSpeed += (fCounterClockThrust - fClockwiseThrust) * rotationThrustRatioVal;
	fRotationSpeed *= 0.99; // hardcoded  damping, for now

	shipAcceleration = glm::vec3((fForwardThrust * movementThrustRatioVal) - (fBackThrust * movementThrustRatioVal),
		-(gravityVal - (fMainThrust)), 0);

	glm::mat4 rot = glm::rotate(glm::mat4(1.0), glm::radians(fLanderRot), glm::vec3(0, 1, 0));

	shipAcceleration = rot * glm::vec4(shipAcceleration, 1);
	shipAcceleration += glm::vec3(ofRandom(-0.01, 0.01), ofRandom(-0.01, 0.01), ofRandom(-0.01, 0.01)); // turbulence
	shipHeading = rot * glm::vec4(glm::vec3(1,0,0), 1);

	// save lander's current position for collisions
	landerPos = lander.getPosition();
	glm::vec3 lastLanderPos = lander.getPosition();

	if (bIsColliding) {
		shipAcceleration = glm::vec3(0);
		outOfFuelAlert.setVolume(0.01);
	}
	// handle the transform integration
	landerPos += shipVelocity * dt;
	lander.setPosition(landerPos.x, landerPos.y, landerPos.z);

	ofVec3f min = lander.getSceneMin() + lander.getPosition();
	ofVec3f max = lander.getSceneMax() + lander.getPosition();

	landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

	// now that the integrator determined where the lander should be, we detect if that is a collision.
	colBoxList.clear();

	if (octree.intersect(landerBounds, octree.root, colBoxList, collidedNode)) {
		glm::vec3 normal = octree.mesh.getNormal(collidedNode.points[0]);

		glm::vec3 bounce = (glm::dot(-(shipVelocity), normal) * normal)/3;
		//cout << bounce << endl;
		if (bounce.y >= 0.85) {
			bounce = bounce * 100;
			bounce += glm::vec3(200, 0, 200);
			explosion.setPosition(ofVec3f(lander.getPosition())); // set the explosion's location based on the lander, immediately.
			explosionCam.setPosition(lander.getPosition() + glm::vec3(-20, 15, -20)); // same goes for the explosion camera, so that we can use the current position of the lander.
			exploded = true;
			failed = true;
		}

		bIsColliding = true;
		// if it is, we just reset the lander to the last known position that doesn't have any collisions.
		lander.setPosition(lastLanderPos.x, lastLanderPos.y, lastLanderPos.z);
		landerPos = lander.getPosition();
		landerPos += bounce * dt;
		shipVelocity = bounce;
		lander.setPosition(landerPos.x, landerPos.y, landerPos.z);

		// we also reset the lander's bounds to keep it accurate.
		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
	}
	else {
		bIsColliding = false;
	}
	emitter.setPosition(lander.getPosition() + ofVec3f(0, emitterPos, 0));

	cam.setTarget(lander.getPosition());
	if (!bIsColliding) {
		cam.setPosition(cam.getPosition() + (shipVelocity * dt));
	}
	else {
		landedPosition = lander.getPosition();
	}
	
	landerBotCam.setPosition(lander.getPosition() + (shipHeading * ofVec3f(1.25, 0, 1.25)) + ofVec3f(0, 3.15, 0));
	landerBotCam.lookAt(lander.getPosition() + (shipHeading * ofVec3f(9, 0, 9)) + ofVec3f(0, -1, 0));
	// update velocity using the acceleration.
	shipVelocity += shipAcceleration;
	shipVelocity *= 0.99;
}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {

	if ((landedPosition.x <= 21 && landedPosition.x >= -21) && (landedPosition.z <= 21 && landedPosition.z >= -21)) {
		//cout << "Landed Successfully!" << endl;
		if (bIsColliding) {
			success = true;
		}
	}
	else {
		success = false;
		//cout << "FAILED!" << endl;
		cout << "FAILED!" << endl;
	}

	keyLight.setPosition(sKeyPos);
	float elapsed = startTime - ofGetElapsedTimeMillis();
	fillLight.setPosition(sFillPos - glm::vec3(0, (int)elapsed % 4000, 0));
	rimLight.setPosition(sRimPos);

	keyLight.lookAt(sKeyLookAt);
	fillLight.lookAt(sFillLookAt);
	rimLight.lookAt(sRimLookAt);


	if (bStartSim) {
		sceneCam.lookAt(lander.getPosition());
		cam.setPosition(lander.getPosition() - (shipHeading * 20) + glm::vec3(0, 10, 0));
		integrate();
		landerPos = lander.getPosition();
		if (bAGLToggle) {
			aglDistance = getAGL(aglPoint, landerPos);
			scoutCam.setTarget(aglPoint);
		}
		float totalExpended = 0;
		if (expending) {
			float expendedFuel = ofGetElapsedTimeMillis() - startThrust; // we keep track of how much fuel was expended from the start of this Thrust.
			totalExpended += expendedFuel;										// and always subtract it from the fuel;
		}
		if (movingF || movingB) {
			float expendedFuel = ofGetElapsedTimeMillis() - moveThrust; // we keep track of how much fuel was expended from the start of this Thrust.
			expendedFuel /= 10; // moving expends less fuel
			totalExpended += expendedFuel;										// and always subtract it from the fuel;
		}
		if (rotatingL || rotatingR) {
			float expendedFuel = ofGetElapsedTimeMillis() - rotThrust; // we keep track of how much fuel was expended from the start of this Thrust.
			expendedFuel /= 20; // rotating expends even less fuel;
			totalExpended += expendedFuel;										// and always subtract it from the fuel;
		}
		fuelTracker = fuel - totalExpended;

		if (fuel <= 0 || fuelTracker <= 0) {
			thrustPower = 0;
			if (!alertPlaying) {
				alertPlaying = true;
				outOfFuelAlert.play();
			}

		}
	}

	ofSeedRandom();

	// live update of emmitter parameters (with sliders)
	//
	emitter.setParticleRadius(radius);
	emitter.setLifespanRange(ofVec2f(lifespanRange->x, lifespanRange->y));
	emitter.setMass(mass);
	emitter.setDamping(damping);
	emitter.setGroupSize(numParticles);

	explosion.setParticleRadius(10);

	// live update of forces  (with sliders)
	//
	gravityForce->set(ofVec3f(0, -gravity, 0));
	turbForce->set(ofVec3f(turbMin->x, turbMin->y, turbMin->z), ofVec3f(turbMax->x, turbMax->y, turbMax->z));
	radialForce->set(radialForceVal);
	radialForce->setHeight(radialHight);
	cyclicForce->set(cyclicForceVal);

	// don't forget to update emitter
	//
	if (bEmitting) {
		emitter.sys->reset();
		emitter.start();
	}
	if (exploded) {
		exploded = false;
		explosion.sys->reset();
		explosion.start();
		boom.play();
		explosionCam.lookAt(lander.getPosition());
		theCam = &explosionCam;
	}
	explosion.update();
	emitter.update();

}
//--------------------------------------------------------------
void ofApp::draw() {

	loadVbo();

	loadExVbo();

	ofNoFill(); // this solves the problem where the bounding boxes are filled, putting it up and out here makes sure that all the boxes will be drawn with lines.
	ofBackground(ofColor::black);
	glDepthMask(false);
	bg.draw(0, 0, 0);
	glDepthMask(true);
	theCam->begin();
	ofPushMatrix();
	
	//octree.draw(numLevels, 1);
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();
		//Octree::drawBox(boundingBox);
		ofMesh mesh;
		if (bLanderLoaded) {
			lander.drawFaces();
			if (bAGLToggle) {
				ofSetColor(ofColor::green);
				ofDrawLine(aglPoint, lander.getPosition());
			}
			if (!bTerrainSelected) drawAxis(lander.getPosition());
			/*
				if (bDisplayBBoxes) {
				ofNoFill();
				ofSetColor(ofColor::white);
				for (int i = 0; i < lander.getNumMeshes(); i++) {
					ofPushMatrix();
					ofMultMatrix(lander.getModelMatrix());
					ofRotate(-90, 1, 0, 0);
					//Octree::drawBox(bboxList[i]);
					ofPopMatrix();
				}
			}
			*/
			
			/*
			if (bLanderSelected) {

				ofVec3f min = lander.getSceneMin() + lander.getPosition();
				ofVec3f max = lander.getSceneMax() + lander.getPosition();

				Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
				ofSetColor(ofColor::white);
				Octree::drawBox(bounds);

				// draw colliding boxes
				//
				ofSetColor(ofColor::lightBlue);
				for (int i = 0; i < colBoxList.size(); i++) {
					//cout << "Coliding	" << colBoxList.size() << endl;
					Octree::drawBox(colBoxList[i]);
				}
			}
			
			
			if (bStartSim) {
				// draw colliding boxes
				//
				ofSetColor(ofColor::lightBlue);
				for (int i = 0; i < colBoxList.size(); i++) {
					//cout << "Coliding	" << colBoxList.size() << endl;
					Octree::drawBox(colBoxList[i]);
				}
			}
			*/
		}
	}
	//if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));



	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}


	// recursively draw octree
	//
	ofDisableLighting();
	int level = 0;
	//	ofNoFill();

	if (bDisplayLeafNodes) {
		octree.drawLeafNodes(octree.root);
		//cout << "num leaf: " << octree.numLeaf << endl;
	}
	else if (bDisplayOctree) {
		ofNoFill();
		ofSetColor(ofColor::white);
		octree.draw(numLevels, 0);
	}

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - theCam->getPosition();
		ofSetColor(ofColor::lightGreen);
		ofDrawSphere(p, .02 * d.length());
	}

	if (bAGLToggle) {
		ofSetColor(ofColor::limeGreen);
		ofDrawSphere(aglPoint, 1);
	}

	ofPopMatrix();
	theCam->end();

	glDepthMask(GL_FALSE);

	ofSetColor(255, 100, 90);

	// this makes everything look glowy :)
	//
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnablePointSprites();

	// begin drawing in the camera
	//
	shader.begin();
	theCam->begin();

	// draw particle emitter here..
	//
//	emitter.draw();
	particleTex.bind();
	vbo.draw(GL_POINTS, 0, (int)emitter.sys->particles.size());
	particleTex.unbind();

	theCam->end();
	shader.end();

	ofSetColor(ofColor::white);
	// begin drawing in the camera
//
	shaderEx.begin();
	theCam->begin();

	// draw particle emitter here..
	//
//	emitter.draw();
	particleTexEx.bind();
	vboEx.draw(GL_POINTS, 0, (int)explosion.sys->particles.size());
	particleTexEx.unbind();

	theCam->end();
	shaderEx.end();

	ofDisableBlendMode();
	ofDisablePointSprites();
	ofEnableAlphaBlending();

	// set back the depth mask
	//
	glDepthMask(GL_TRUE);
	
	/*
	glDepthMask(false);
	if (!bHide) {
		gui.draw();
	}
	else {
		camGui.draw();
	}
	glDepthMask(true);
	*/
	

	
	string str, state;
	state = (bStartSim ? "ON" : "OFF");
	str += "Simulation : " + state;
	ofSetColor(ofColor::white);
	ofDrawBitmapString(str, ofGetWindowWidth() - 170, 15);

	string fuelStr;
	if (fuelTracker <= 0) {
		fuelTracker = 0;
	}
	fuelStr += "Fuel Level: " + to_string((int)fuelTracker);
	ofDrawBitmapString(fuelStr, ofGetWindowWidth() - 170, 30);

	string landing, status;
	status = (success ? "SUCCESSFULLY LANDED" : "STILL LANDING");
	if (failed) {
		status = "EXPLODED";
	}
	landing += "Status: " + status;
	ofDrawBitmapString(landing, (ofGetWindowWidth() / 2.0) - 80, 50);

	string aglMessage;
	aglMessage = "AGL: " + to_string(aglDistance) + " meters";

	ofDrawBitmapString(aglMessage, (ofGetWindowWidth() / 2.0) - 80, 75);
}


// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {
	if (fuel <= 0 || fuelTracker <= 0) {
		thrustPower = 0;
	}
	
	switch (key) {
	case 'a':
		bAGLToggle = !bAGLToggle;
		break;
	case 'B':
	case 'b':
		bDisplayBBoxes = !bDisplayBBoxes;
		break;
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) {
			cam.disableMouseInput();
		}
		else {
			cam.enableMouseInput();
			scoutCam.disableMouseInput();
		}
		break;
	case 'r': {
		shipVelocity = glm::vec3(0);
		startPoint = glm::vec3(ofRandom(-200, 200), 100, ofRandom(-200, 200));
		fuel = 120000.0f;
		fuelTracker = fuel;
		thrustPower = 0.126;
		exploded = false;
		exploded = false;
		failed = false;
		success = false;

		alertPlaying = false;
		outOfFuelAlert.setVolume(0.09);
		outOfFuelAlert.stop();
		lander.setPosition(startPoint.x, startPoint.y, startPoint.z);
		cam.setPosition(lander.getPosition() - ofVec3f(15, -5, 0));
		theCam = &cam;

		landerPos = lander.getPosition();
		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		break;
	}
	case 'x':
		if (scoutCam.getMouseInputEnabled()) {
			scoutCam.disableMouseInput();
		}
		else {
			scoutCam.enableMouseInput();
			cam.disableMouseInput();
		}
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		bHide = !bHide;
		break;
	case 'L':
	case 'l':
		bDisplayLeafNodes = !bDisplayLeafNodes;
		break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
	case 's':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':
		toggleWireframeMode();
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	case OF_KEY_UP:   // move the lander forward
		if (bStartSim) {
			fForwardThrust = thrustPower; // moving forward uses less fuel.
			if (!movingF) {
				movingF = true;
				moveThrust = ofGetElapsedTimeMillis(); // mark the start time of this thrustPower.
			}
		}
		break;
	case OF_KEY_DOWN:  // move the lander backwards
		if (bStartSim) {
			fBackThrust = thrustPower; // moving backward uses less fuel.
			if (!movingB) {
				movingB = true;
				moveThrust = ofGetElapsedTimeMillis(); // mark the start time of this thrustPower.
			}
		}
		break;
	case OF_KEY_LEFT:   // turns the lander counter clockwise.
		if (bStartSim) {
			fCounterClockThrust = thrustPower;
			if (!rotatingL) {
				rotatingL = true;
				rotThrust = ofGetElapsedTimeMillis(); // mark the start time of this thrustPower.
			}
		}
		break;
	case OF_KEY_RIGHT:   // turns the lander clockwise.
		if (bStartSim) {
			fClockwiseThrust = thrustPower;
			if (!rotatingR) {
				rotatingR = true;
				rotThrust = ofGetElapsedTimeMillis(); // mark the start time of this thrustPower.
			}
		}
		break;
	case ' ':     // boosts the lander up
		if (bStartSim && thrustPower != 0) {
			fMainThrust = thrustPower;
			if (!expending) {
				expending = true;
				startThrust = ofGetElapsedTimeMillis(); // mark the start time of this thrustPower.
			}
			
			bEmitting = true;
			if (!isPlaying) {
				thrusters.setVolume(1);
				isPlaying = true;
			}
		}
		break;
	case 'm':
		startTime = ofGetElapsedTimeMillis();
		bStartSim = !bStartSim; // toggle the simulation ON/OFF
		break;
	case OF_KEY_F1:
		theCam = &cam;
		break;
	case OF_KEY_F2:
		theCam = &sceneCam;
		break;
	case OF_KEY_F3:
		theCam = &landerBotCam;
		break;
	case OF_KEY_F4:
		theCam = &scoutCam;
		break;
	case OF_KEY_F5:
		theCam = &trailerCam;
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_UP:   // move the lander forward
		if (bStartSim) {
			fForwardThrust = 0; // moving forward uses less fuel.
			movingF = false;
			fuel = fuelTracker;
		}
		break;
	case OF_KEY_DOWN:  // move the lander backwards
		if (bStartSim) {
			fBackThrust = 0; // moving backward uses less fuel.
			movingB = false;
			fuel = fuelTracker;
		}
		break;
	case OF_KEY_LEFT:   // turns the lander counter clockwise.
		if (bStartSim) {
			fCounterClockThrust = 0;
			rotatingL = false;
			fuel = fuelTracker;
		}
		break;
	case OF_KEY_RIGHT:   // turns the lander clockwise.
		if (bStartSim) {
			fClockwiseThrust = 0;
			rotatingR = false;
			fuel = fuelTracker;
		}
		break;
	case ' ': 
		if (bStartSim) {
			fMainThrust = 0;

			expending = false;
			fuel = fuelTracker; // once the thrusters are released, we then update the actual fuel level with the tracker.

			bEmitting = false;
			thrusters.setVolume(0.1);
			isPlaying = false;
		}
		break;
	default:
		break;
	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	/*
	if (theCam == &scoutCam) {
		mouseLastPos = ofVec3f(x, y, 1);
		return;
	}*/
	
	

	if (cam.getMouseInputEnabled()) {
		return;
	}

	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = theCam->getPosition();
		glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			scoutCam.setTarget(lander.getPosition());
			mouseDownPos = getMousePointOnPlane(lander.getPosition(), theCam->getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	if (theCam == &cam) {
		ofVec3f p;
		raySelectWithOctree(p, theCam);
		scoutCam.setTarget(p);
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet, ofCamera *theCam) {
	ofVec3f mouse(mouseX, mouseY);
	glm::vec3 origin = theCam->getPosition();
	ofVec3f rayPoint = theCam->screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - theCam->getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	//float currentTime = ofGetElapsedTimeMillis();
	pointSelected = octree.intersect(ray, octree.root, selectedNode);
	//float elapsed = ofGetElapsedTimeMillis() - currentTime;
	//cout << "Ray select took " << elapsed << " ms" << endl;

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
	}
	return pointSelected;
}

float ofApp::getAGL(ofVec3f &pointRet, ofVec3f rayStart) {
	// the Telemetry fires a ray downwards from the starting location, which would be the lander's position.
	Ray ray = Ray(Vector3(rayStart.x, rayStart.y, rayStart.z),
		Vector3(0, rayStart.y - 100000, 0));

	if (octree.intersect(ray, octree.root, selectedNode)) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);
		aglPoint = pointRet;
		return pointRet.distance(rayStart); // get the distance between the two points.
	}
	return 0;
}


//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	/*if (theCam == &scoutCam) {
		glm::vec3 mousePos = ofVec3f(x, y, 1);
		glm::vec3 delta = mousePos - mouseLastPos;
		theCam->setPosition((theCam->getPosition() + delta));
		mouseLastPos = mousePos;
	}*/

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) {
		cam.setTarget(lander.getPosition());
		return;
	}

	if (bInDrag) {

		landerPos = lander.getPosition();
		// will be used for resetting the lander's pos in case of collision with the terrain
		glm::vec3 lastLanderPos = lander.getPosition(); 

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, theCam->getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;
	
		landerPos += delta;
		lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		//float currentTime = ofGetElapsedTimeMillis();
		// Collision Detection: If the lander intersects with atleast 1 leaf node box in the terrain's octree,
		// we then consider it a collision, and will reset any position update to the last known position
		// that doesn't have any collision.
		if (octree.intersect(landerBounds, octree.root, colBoxList, collidedNode)) {
			lander.setPosition(lastLanderPos.x, lastLanderPos.y, lastLanderPos.z);

			ofVec3f min = lander.getSceneMin() + lander.getPosition();
			ofVec3f max = lander.getSceneMax() + lander.getPosition();

			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
		emitter.setPosition(lander.getPosition() + ofVec3f(0, emitterPos, 0));

		cam.setTarget(lander.getPosition());

		landerBotCam.setPosition(lander.getPosition() + ofVec3f(5, 10, 5));
		landerBotCam.lookAt(lander.getPosition() - ofVec3f(0, 10, 0));
		//float elapsed = ofGetElapsedTimeMillis() - currentTime;
		//cout << "Leaf search took " << elapsed << " ms" << endl;
	

		/*if (bounds.overlap(testBox)) {
			cout << "overlap" << endl;
		}
		else {
			cout << "OK" << endl;
		}*/


	}
	else {
		if (theCam == &cam) {
			ofVec3f p;
			raySelectWithOctree(p, theCam);
			scoutCam.setTarget(p);
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	colBoxList.clear();
	bInDrag = false;
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent2(ofDragInfo dragInfo) {

	ofVec3f point;
	mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);
	if (lander.loadModel(dragInfo.files[0])) {
		lander.setScaleNormalization(false);
//		lander.setScale(.1, .1, .1);
	//	lander.setPosition(point.x, point.y, point.z);
		lander.setPosition(1, 1, 0);

		bLanderLoaded = true;
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		cout << "Mesh Count: " << lander.getMeshCount() << endl;
	}
	else cout << "Error: Can't load model" << dragInfo.files[0] << endl;
}

bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (lander.loadModel(dragInfo.files[0])) {
		bLanderLoaded = true;
		lander.setScaleNormalization(false);
		lander.setPosition(0, 0, 0);
		cout << "number of meshes: " << lander.getNumMeshes() << endl;
		bboxList.clear();
		for (int i = 0; i < lander.getMeshCount(); i++) {
			bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
		}

		//		lander.setRotation(1, 180, 1, 0, 0);

				// We want to drag and drop a 3D object in space so that the model appears 
				// under the mouse pointer where you drop it !
				//
				// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
				// once we find the point of intersection, we can position the lander/lander
				// at that location.
				//

				// Setup our rays
				//
		glm::vec3 origin = cam.getPosition();
		glm::vec3 camAxis = cam.getZAxis();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the lander's origin at that intersection point
			//
			glm::vec3 min = lander.getSceneMin();
			glm::vec3 max = lander.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			lander.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for lander while we are at it
			//
			landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}
