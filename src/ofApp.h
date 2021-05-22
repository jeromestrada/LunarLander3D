#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include  "ofxAssimpModelLoader.h"
#include "Octree.h"
#include "Particle.h"
#include "ParticleEmitter.h"



class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent2(ofDragInfo dragInfo);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f &pointRet, ofCamera *theCam);
		glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 p , glm::vec3 n);
		void loadVbo();
		void loadExVbo();

		float getAGL(ofVec3f &pointRet, ofVec3f rayStart);

		// integrator to move the Lander in a more physically realistic way.
		void integrate();

		bool bStartSim; // toggles the simulation of the scene, if TRUE, the lander can move using physics, otherwise, the lander doesn't move
		
		ofxAssimpModelLoader mars, lander;
		// attributes of the lander are listed below, decided not to create a separate class to encapsulate the lander
		glm::vec3 landerPos;

		glm::vec3 landedPosition;

		// physics values
		glm::vec3 shipAcceleration;
		glm::vec3 shipVelocity;
		glm::vec3 shipHeading;

		ofSoundPlayer thrusters;
		bool isPlaying;

		ofSoundPlayer outOfFuelAlert;
		bool alertPlaying;

		ofSoundPlayer boom;
		bool success;

		ofVec3f aglPoint;
		float aglDistance;
		bool bAGLToggle;

		// Thruster values
		float fMainThrust; // represents the force of the main thrusters that keeps the lander afloat
		float fForwardThrust;
		float fBackThrust;
		float fClockwiseThrust; // thrust to spin the lander clockwise on the y-axis
		float fCounterClockThrust;// thrust to spin the lander clockwise on the x-axis
		float fRotationSpeed;
		float fLanderRot; // represents the Rotation Transformation of the Lander from its original orientation.

		float fuel;
		float fuelTracker;
		float startThrust; // this will be used to mark at which point of time the thrusters are turned on. Useful for calculating the expended fuel.
		bool expending;

		float moveThrust;
		bool movingF;
		bool movingB;


		float rotThrust;
		bool rotatingL;
		bool rotatingR;

		float thrustPower;
		bool exploded;
		bool failed;

		// multi cameras
		ofEasyCam trailerCam;
		ofEasyCam cam;
		ofEasyCam scoutCam;
		ofCamera sceneCam;
		ofCamera landerCam;
		ofCamera landerFrontCam;
		ofCamera landerBotCam;
		ofCamera explosionCam;
		ofCamera *theCam;

		ofLight light;
		Box boundingBox, landerBounds;
		Box testBox;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		TreeNode collidedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		bool bInDrag = false;
		bool bIsColliding = false;
		glm::vec3 startPoint;

		// Emitter and some forces;
		//
		ParticleEmitter emitter;

		TurbulenceForce *turbForce;
		GravityForce *gravityForce;
		ImpulseRadialForce *radialForce;
		ImpulseDirectionalForce * directionalForce;
		CyclicForce *cyclicForce;

		ParticleEmitter explosion;

		ofTexture  particleTexEx;
		ofVbo vboEx;
		ofShader shaderEx;

		TurbulenceForce *exTurbForce;
		ImpulseRadialForce *exRadialForce;
		CyclicForce *exCyclicForce;


		bool bHide;
		// Lander Sliders
		ofxIntSlider numLevels;
		ofxFloatSlider gravityVal;
		ofxFloatSlider thrustVal;
		ofxFloatSlider rotationThrustRatioVal;
		ofxFloatSlider movementThrustRatioVal;

		// Particle Sliders
		bool bEmitting;
		ofxFloatSlider emitterPos;
		ofxFloatSlider gravity;
		ofxFloatSlider damping;
		ofxFloatSlider radius;
		ofxVec3Slider velocity;
		ofxIntSlider numParticles;
		ofxFloatSlider lifespan;
		ofxVec2Slider lifespanRange;
		ofxVec3Slider turbMin;
		ofxVec3Slider turbMax;
		ofxFloatSlider mass;
		ofxFloatSlider radialForceVal;
		ofxFloatSlider radialHight;
		ofxFloatSlider cyclicForceVal;
		ofxFloatSlider rate;
		ofxPanel gui;


		ofLight keyLight, fillLight, rimLight;
		// lights sliders
		ofxVec3Slider sKeyPos;
		ofxVec3Slider sFillPos;
		ofxVec3Slider sRimPos;

		ofxVec3Slider sKeyLookAt;
		ofxVec3Slider sFillLookAt;
		ofxVec3Slider sRimLookAt;

		float startTime;

		


		ofxVec3Slider camPos;
		ofxVec3Slider camLookAt;
		ofxPanel camGui;

		// textures
		//
		ofTexture  particleTex;

		// shaders
		//
		ofVbo vbo;
		ofShader shader;
		ofImage bg;

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		bool pointSelected = false;
		bool bDisplayLeafNodes = false;
		bool bDisplayOctree = false;
		bool bDisplayBBoxes = false;
		
		bool bLanderLoaded;
		bool bTerrainSelected;
	
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;

		vector<Box> bboxList;

		const float selectionRange = 4.0;
};
