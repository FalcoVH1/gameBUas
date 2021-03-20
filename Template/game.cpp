#include "game.h"
#include "surface.h"
#include "template.h"
#include <cmath>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h> 
#include <string>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

namespace Tmpl8
{
	/*
	/____________________________________________________________________________________________________________________________________/

										   )/_
					 _.--..---"-,--c_
				\L..'           ._O__)_
		,-.	   _.+  _  \..--( /
		  `\.-''__.-' \ (     \_
			`'''       `\__   /\
					   ')
	Hey man, friendly reminder: this game might seem like a daunting task, but you're a fuckin genius dude! You can do it, sure you can!
	 -You from the past
	 */



	 //------------------------------------------------------------------------------------------------global variables------------------------------------------------------------------------------------------------//


	 //Surface* tiles = new Surface("assets/defTiles.png");
	Surface* colTile = new Surface("assets/specialTile.png");
	//Surface* selectTile = new Surface("assets/selectTile.png");
	Surface* defaultAnt = new Surface("assets/defaultAnt.png");

	Sprite planetSelect(new Surface("assets/menuScreen.png"), 1);
	Sprite planet0(new Surface("assets/planet0.png"), 1);
	Sprite planet1(new Surface("assets/planet1.png"), 1);
	Sprite planet2(new Surface("assets/planet2.png"), 1);
	Sprite planet3(new Surface("assets/planet3.png"), 1);
	Sprite button(new Surface("assets/button.png"), 10);

	Sprite selectTileS(new Surface("assets/selectTile.png"), 1);
	Sprite pathTile(new Surface("assets/pathTile.png"), 1);
	Sprite groundTiles(new Surface("assets/defTiles.png"), 3);
	Sprite Mountain(new Surface("assets/mountain.png"), 1);
	Sprite antHill(new Surface("assets/antsObj.png"), 1);
	Sprite menu(new Surface("assets/menu.png"), 1);

	Sprite antCEO(new Surface("assets/antCEO.png"), 1);

	int AItimer = 0, AIstep = 0, turnState = 0, lvID = 0, wrldID = 0, moveStep = 0, worldSelect = 100, levelSelect = 100;
	bool inGame = false;

	coords mousePos(0, 0), currentCell(0, 0), offset(0, 0), selected(0, 0), selectedUnit(0, 0), selectedWorldPx(0, 0), tileSize(64, 48);

	const coords directions[4] = {
		coords(0, 1),
		coords(0, -1),
		coords(1, 0),
		coords(-1, 0),
	};

	LevelButton goBackBtn(50, 50, 50, 50, 6);

	Button worldSelectBtns[4] = {
		Button(110, 246, 120, 120),
		Button(350, 422, 133, 133),
		Button(648, 158, 238, 238),
		Button(878, 468, 154, 154)
	};

	LevelButton levelSelectBtns[4][6] = {
		LevelButton(90, 300, 50, 50, 0),
		LevelButton(290, 260, 50, 50, 1),
		LevelButton(480, 200, 50, 50, 2),
		LevelButton(640, 120, 50, 50, 3),
		LevelButton(800, 180, 50, 50, 4),
		LevelButton(910, 340, 50, 50, 5),

		LevelButton(130, 235, 50, 50, 0),
		LevelButton(252, 382, 50, 50, 1),
		LevelButton(453, 482, 50, 50, 2),
		LevelButton(605, 511, 50, 50, 3),
		LevelButton(735, 398, 50, 50, 4),
		LevelButton(912, 464, 50, 50, 5),

		LevelButton(93, 233, 50, 50, 0),
		LevelButton(287, 283, 50, 50, 1),
		LevelButton(461, 377, 50, 50, 2),
		LevelButton(582, 521, 50, 50, 3),
		LevelButton(722, 605, 50, 50, 4),
		LevelButton(934, 490, 50, 50, 5),

		LevelButton(337, 130, 50, 50, 0),
		LevelButton(251, 252, 50, 50, 1),
		LevelButton(282, 450, 50, 50, 2),
		LevelButton(531, 445, 50, 50, 3),
		LevelButton(736, 392, 50, 50, 4),
		LevelButton(1006, 392, 50, 50, 5)
	};

	int distanceMap[worldsX][worldsY] = { 0 }; //map of the distance any object is from a selected unit
	int threatenedPlaces[worldsX][worldsY] = { 0 }; //map of all places on the map which are threatened
	int AIPMoves[worldsX][worldsY] = { 0 }; //places AI can move to
	int scannedAreas[worldsX][worldsY] = { 0 };

	int worldField[4][6][9][9] = {

		//world 1 

		//level 1
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 2, 2, 0, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		//level 2
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		//level 3
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 1, 1, 1, 1, 1, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2,
		//level 4
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 2, 2, 0, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		//level 5
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		//level 6
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 1, 1, 1, 1, 1, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2,

		//world 2

		//level 1
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 2, 2, 0, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		//level 2
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		//level 3
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 1, 1, 1, 1, 1, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2,
		//level 4
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 2, 2, 0, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		//level 5
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		//level 6
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 1, 1, 1, 1, 1, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2,

		//world 3

		//level 1
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 2, 2, 0, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		//level 2
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		//level 3
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 1, 1, 1, 1, 1, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2,
		//level 4
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 2, 2, 0, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		//level 5
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		//level 6
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 1, 1, 1, 1, 1, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2,

		//world 4

		//level 1
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 2, 2, 0, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		//level 2
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		//level 3
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 1, 1, 1, 1, 1, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2,
		//level 4
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 2, 2, 0, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		//level 5
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		//level 6
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 1, 1, 1, 1, 1, 2, 2,
		2, 2, 2, 2, 1, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 1, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2,

	}; //terrain map

	Unit fUnits[4][6][3] = {
		Unit(1, 7, 3, 15, unitType::RANGED, unitMovementType::AERIAL), Unit(4, 7, 3, 15, unitType::RANGED, unitMovementType::GROUND), Unit(7, 7, 3, 15, unitType::MORTAR, unitMovementType::NAVAL)
	};

	Unit eUnits[4][6][3] = {
		Unit(1, 1, 3, 3, unitType::RANGED, unitMovementType::GROUND), Unit(4, 1, 3, 3, unitType::MELEE, unitMovementType::GROUND), Unit(7, 2, 3, 3, unitType::MORTAR, unitMovementType::NAVAL)
	};

	GameState level[4][6];
	GameState levelBackup;

	std::vector<coords> pathToTake(0);

	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//


	void setDistanceMap(coords c1);
	bool onMap(coords c);
	bool canMoveHere(Unit* unit, int x, int y);
	int getDist(coords c1, coords c2);
	void resetUnits();

	void moveUnits(Unit* fUnits, Unit* eUnits, coords selectedC, int fUnitsAm, int eUnitsAm) {

			//resets attacking before move phase
			memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
			for (int i = 0; i < fUnitsAm; i++) {
				fUnits[i].attackState = 0;
				//fUnits[i].clearPastPlaces();
			};

			for (int i = 0; i < eUnitsAm; i++) {
				eUnits[i].attackState = 0;
				//eUnits[i].clearPastPlaces();
			};

			for (int t = 0; t < fUnitsAm; t++) {

				//if units haven't moved, transition to them moving

				if (fUnits[t].moveState == 0) {
					if (selectedC.equals(fUnits[t].getCoords())) {
						fUnits[t].moveState = 1;
					}
				}
				else if (fUnits[t].moveState == 1) {

					bool targetOccupied = false;
					//first checks if the move is eligible, then sets a new destination for the unit to move to. if the move is ineligible, the unit is set back to movestate 0, aka "hasn't moved".
					if (selectedC.equals(level[wrldID][lvID].eObjtv.getCoords()) || selectedC.equals(level[wrldID][lvID].fObjtv.getCoords())) {
						targetOccupied = true;
					}

					//currently checks how far fields are away from unit, but this might have to change to accomodate for terrain, etc.

					if (onMap(selectedC) && getDist(selectedC, fUnits[t].getCoords()) == 1.0 && !targetOccupied && canMoveHere(&fUnits[t], selectedC.x, selectedC.y)) {

						//-1 to getSpeed because the last square of movement gets done in the else- part of this statement
						fUnits[t].move(selectedC);
					}
					else {
						fUnits[t].moveState = 0;
					}
				}
		}
	}

	void attackUnits(Unit* fUnits, Unit* eUnits, coords selectedC, int fUnitsAm, int eUnitsAm, Objective* Obj) {

		//resets movement before attack phase
		for (int i = 0; i < fUnitsAm; i++) {
			fUnits[i].moveState = 0;
			fUnits[i].moveState = 0;
		};

		for (int i = 0; i < eUnitsAm; i++) {
			eUnits[i].moveState = 0;
			eUnits[i].moveState = 0;
		};

		for (int t = 0; t < fUnitsAm; t++) {
			if (fUnits[t].attackState == 0) {
				//starts target selection
				if (selectedC.equals(fUnits[t].getCoords())) {
					fUnits[t].attackState = 1;
				}
			}
			else if (fUnits[t].attackState == 1) {

				//resets attack when clicking other friendly unit

				fUnits[t].attackState = 3;
				for (int t1 = 0; t1 < fUnitsAm; t1++) {
					if (selectedC.equals(fUnits[t1].getCoords())) {
						fUnits[t].attackState = 0;
					}
				}

				//execute attack by damaging all enemy units in attack range
				for (int t1 = 0; t1 < eUnitsAm; t1++) {
					if (threatenedPlaces[eUnits[t1].x][eUnits[t1].y] == 1) {
						//eUnits[t1].takeDamage(fUnits[t].getDamage());
						fUnits[t].damageUnit(&eUnits[t1]);
					}
				}
				if (threatenedPlaces[Obj->x][Obj->y] == 1) {
					fUnits[t].damageObj(Obj);
				}
				memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
			}
		}

		//printf(level[wrldID][lvID].winloss() ? "true" : "false");
		if (level[wrldID][lvID].winloss()) {

			level[wrldID][lvID].set(levelBackup);
			resetUnits();
			turnState = 0;
			worldSelect = 100;
			level[wrldID][lvID].completed = true;
			
			inGame = false;
		}
		//level[wrldID][lvID].winloss();
	}


	void draw(int tx, int ty, Surface* screen, Surface* tiles, int x, int y, int tilemapH, int tilemapW, int tileH, int tileW)
	{
		//draws tiles lol
		Pixel* src = tiles->GetBuffer() + tx * tileW + (ty * tileH) * tilemapW;
		Pixel* dst = screen->GetBuffer() + x + y * ScreenWidth;
		for (int i = 0; i < tileH; i++, src += tilemapW, dst += ScreenWidth)
			for (int j = 0; j < tileW; j++)
				if (src[j] != NULL) {
					dst[j] = src[j];
				}
	};

	//checks if cursor is located in between given coords
	bool mouseLocated(int x1, int y1, int x2, int y2) {
		if (mousePos.x >= x1 && mousePos.x < x2 && mousePos.y >= y1 && mousePos.y < y2) {
			return true;
		}
		else return false;
	}

	//sets full attack array for the game to know which squares to mark
	void returnThreatArray(Unit* Unit, coords s, int arr[worldsX][worldsY]) {
		for (int i = 0; i < worldsX; i++) {
			for (int j = 0; j < worldsY; j++) {
				//wonky code, kinda; essentially, returns a value of 0 if the field given as parameters is outside of attack range of unit, returns 1 if it's inside.
				//could be optimized with an array of coordinates of a variable size

				//ended up not being as wonky as I expected it to be :)
				arr[i][j] = Unit->returnAttackArray(s, i, j);
			}
		}
	};

	//converts ingame coordinates to on screen coordinates
	coords toScreen(int x, int y) {
		return coords{
				(level[wrldID][lvID].getOrigin().x * tileSize.x) + (x - y) * (tileSize.x / 2),
				(level[wrldID][lvID].getOrigin().y * tileSize.y) + (x + y) * (tileSize.y / 2)
		};
	}

	//draws units
	void drawUnits(Surface* screen, Unit* fUnits, int fUnitsAm) {
		for (int t = 0; t < fUnitsAm; t++) {

			//kills land units if they somehow arrive on water
			if (fUnits[t].getMvType() == unitMovementType::GROUND && level[wrldID][lvID].getWorldField(fUnits[t].x, fUnits[t].y) == 2) {
				fUnits[t].kill();
			}

			//kills naval units if they somehow arrive on land
			if (fUnits[t].getMvType() == unitMovementType::NAVAL && level[wrldID][lvID].getWorldField(fUnits[t].x, fUnits[t].y) != 2) {
				fUnits[t].kill();
			}

			if (fUnits[t].isAlive()) {

				coords temp = toScreen(fUnits[t].x, fUnits[t].y);

				int dir, type;

				switch (fUnits[t].uDir) {
				case unitDirection::NORTH: dir = 2;
					break;
				case unitDirection::EAST: dir = 1;
					break;
				case unitDirection::SOUTH: dir = 0;
					break;
				case unitDirection::WEST: dir = 3;
					break;
				}

				switch (fUnits[t].getMvType()) {
				case unitMovementType::AERIAL: type = 0;
					break;
				case unitMovementType::GROUND: type = 1;
					break;
				case unitMovementType::NAVAL: type = 2;
					break;
				}
					
				draw(dir, type, screen, defaultAnt, temp.x, temp.y - 14, 64, 64 * 4, 64, 64);


				//gets the center of the movable-unit-circle thingy
				if (fUnits[t].moveState == 1) {
					selectedUnit.set(fUnits[t].getCoords().x, fUnits[t].getCoords().y);
				}

				if (fUnits[t].attackState == 1) {

					level[wrldID][lvID].areAttacking = true;

					//draws cancel attack button
					screen->Bar(270, 450, 330, 500, 0x0000ff);

					//gets threatened fields for attacking units
					if (selected.x == fUnits[t].x && selected.y < fUnits[t].y) {
						memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
						fUnits[t].uDir = unitDirection::NORTH;
						returnThreatArray(&fUnits[t], selected, threatenedPlaces);
					}
					else if (selected.x == fUnits[t].x && selected.y > fUnits[t].y) {
						memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
						fUnits[t].uDir = unitDirection::SOUTH;
						returnThreatArray(&fUnits[t], selected, threatenedPlaces);
					}
					else if (selected.x < fUnits[t].x && selected.y == fUnits[t].y) {
						memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
						fUnits[t].uDir = unitDirection::WEST;
						returnThreatArray(&fUnits[t], selected, threatenedPlaces);
					}
					else if (selected.x > fUnits[t].x && selected.y == fUnits[t].y) {
						memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
						fUnits[t].uDir = unitDirection::EAST;
						returnThreatArray(&fUnits[t], selected, threatenedPlaces);
					}
				}
			}
		}
	}

	void drawObjectives(Objective fObj, Surface* screen, Sprite* obj) {
		if (fObj.getHealth() > 0) {

			coords temp = toScreen(fObj.x, fObj.y);

			//draw(2, 0, screen, tiles, temp.x, temp.y, tileSize.y, tileSize.x * 3, tileSize.y, tileSize.x);

			obj->DrawScaled(temp.x, temp.y - 20, 64, 64, screen);
		}
	}

	void resetUnits() {
		memset(AIPMoves, 0, sizeof(AIPMoves));
		memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
		memset(scannedAreas, 0, sizeof(scannedAreas));
		//pathToTake.resize(0);

		for (int i = 0; i < level[wrldID][lvID].getFUnitsAm(); i++) {
			level[wrldID][lvID].fUnits[i].attackState = 0;

			level[wrldID][lvID].fUnits[i].moveState = 0;

			level[wrldID][lvID].fUnits[i].clearPastPlaces();
		};

		for (int i = 0; i < level[wrldID][lvID].getEUnitsAm(); i++) {
			level[wrldID][lvID].eUnits[i].attackState = 0;

			level[wrldID][lvID].eUnits[i].moveState = 0;
		};
	}

	void Game::MouseDown(int button) {

		if (inGame) {

			if (mouseLocated(goBackBtn.x, goBackBtn.y, goBackBtn.getX2(), goBackBtn.getY2())) {

				level[wrldID][lvID].set(levelBackup);
				resetUnits();
				turnState = 0;
				worldSelect = 100;

				inGame = false;
			}


			//------------------------------- clean up this part btw -------------------------------//

			//button to transition between move and attack phase
			if (mouseLocated(770, 450, 830, 500)) {
				turnState++;

				//sets AItimer so the AI always moves at the exact same time after the player's turn has ended
				if (turnState >= 2) {
					AItimer = 61;
				}
			}

			//cancels attacking phase
			if (mouseLocated(270, 450, 330, 500) && level[wrldID][lvID].areAttacking == true) {
				memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
				for (int t = 0; t < level[wrldID][lvID].getFUnitsAm(); t++) {
					level[wrldID][lvID].fUnits[t].attackState = 0;
				}
			}

			//--------------------------------------------------------------------------------------//



			switch (turnState) {
			case 0:

				//move friendly units
				moveUnits(level[wrldID][lvID].fUnits, level[wrldID][lvID].eUnits, selected, level[wrldID][lvID].getFUnitsAm(), level[wrldID][lvID].getEUnitsAm());

				break;
			case 1:
				Objective * eObjtvPointer;
				eObjtvPointer = &level[wrldID][lvID].eObjtv;

				//attack with friendly units
				attackUnits(level[wrldID][lvID].fUnits, level[wrldID][lvID].eUnits, selected, level[wrldID][lvID].getFUnitsAm(), level[wrldID][lvID].getEUnitsAm(), eObjtvPointer);

				break;
				/*
				case 2:

					//move enemy units <-- add AI to do this instead
					moveUnits(level[lvID].eUnits, level[lvID].fUnits, selected, level[lvID].getEUnitsAm(), level[lvID].getFUnitsAm());

					break;
				case 3:
					Objective * fObjtvPointer;
					fObjtvPointer = &level[lvID].fObjtv;

					//attack with enemy units <-- add AI to do this instead
					attackUnits(level[lvID].eUnits, level[lvID].fUnits, selected, level[lvID].getEUnitsAm(), level[lvID].getFUnitsAm(), fObjtvPointer);

					break;
					*/
			}
		}
		else {
			if (worldSelect == 100) {

				if (mouseLocated(goBackBtn.x, goBackBtn.y, goBackBtn.getX2(), goBackBtn.getY2())) {
					exit(0);
				}

				for (int t = 0; t < sizeof(worldSelectBtns) / sizeof(worldSelectBtns[0]); t++) {

					if (mouseLocated(worldSelectBtns[t].x, worldSelectBtns[t].y, worldSelectBtns[t].getX2(), worldSelectBtns[t].getY2())) {
						worldSelect = t;
					}
					//screen->Box(worldSelectBtns[t].getX(), worldSelectBtns[t].getY(), worldSelectBtns[t].getX2(), worldSelectBtns[t].getY2(), 0x000000);
				}
			}
			else {

				if (mouseLocated(goBackBtn.x, goBackBtn.y, goBackBtn.getX2(), goBackBtn.getY2())) {
					worldSelect = 100;
				}
				else {
					for (int t = 0; t < 4; t++) {

						if (mouseLocated(levelSelectBtns[worldSelect][t].x, levelSelectBtns[worldSelect][t].y, levelSelectBtns[worldSelect][t].getX2(), levelSelectBtns[worldSelect][t].getY2())) {
							if (worldSelect == 0) {
								if (t - 1 < 0) {
									lvID = t;
									wrldID = worldSelect;
									levelBackup.set(level[wrldID][lvID]);
									level[wrldID][lvID].set(levelBackup);
									inGame = true;
								}
								else if (level[worldSelect][t - 1].completed) {
									lvID = t;
									wrldID = worldSelect;
									levelBackup.set(level[wrldID][lvID]);
									level[wrldID][lvID].set(levelBackup);
									inGame = true;
								}
								else
								{
									printf("You can't access this level yet!\n");
								}
							}
							else {
								if (level[worldSelect - 1][5].completed) {
									if (t - 1 < 0) {
										lvID = t;
										wrldID = worldSelect;
										levelBackup.set(level[wrldID][lvID]);
										level[wrldID][lvID].set(levelBackup);
										inGame = true;
									}
									else if (level[worldSelect][t - 1].completed) {
										lvID = t;
										wrldID = worldSelect;
										levelBackup.set(level[wrldID][lvID]);
										level[wrldID][lvID].set(levelBackup);
										inGame = true;
									}
									else
									{
										printf("You can't access this level yet!\n");
									}
								}
								else
								{
									printf("You can't access this level yet!\n");
								}
							}
						}
					}
				}
			}
		}
	};

	void Game::KeyDown(int key) {

		if (key == 44) {
			if (inGame) {
				worldSelect = 100;
				inGame = false;
			}
			else inGame = true;
		}

		if (inGame) {

			resetUnits();

			/*
			switch (key) {
			case 80: if (lvID - 1 >= 0) {
				level[wrldID][lvID].set(levelBackup);
				lvID--;
			}
				   break;

			case 79: if (lvID + 1 < sizeof(level) / sizeof(level[0])) {
				level[wrldID][lvID].set(levelBackup);
				lvID++;
			}
				   break;
			}
			*/
			//resets both attack and movement for all units

			turnState = 0;
			pathToTake.resize(0);

			levelBackup.set(level[wrldID][lvID]);
		}
	}

	//thx https://www.techiedelight.com/split-string-cpp-using-delimiter/
	void tokenize(std::string const& str, const char delim,
		std::vector<std::string>& out)
	{
		size_t start;
		size_t end = 0;

		while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
		{
			end = str.find(delim, start);
			out.push_back(str.substr(start, end - start));
		}
	}

	void Game::Init() {
		/*
		int mapLayout[worldsX][worldsY];

		ifstream levels;
		levels.open("levels.dat", ios::in);

		if (!levels) {
			exit(1);
		}

		string nms;
		*/
		/*
		for (int t = 0; t < worldsX; t++) {
			for (int t2 = 0; t2 < worldsY; t2++) {
				printf("%i ", mapLayout[t][t2]);
			}
			printf("\n");
		}
		*/

		for (int wrldInd = 0; wrldInd < sizeof(level) / sizeof(level[0]); wrldInd++) {
			for (int lvInd = 0; lvInd < sizeof(level[wrldInd]) / sizeof(level[wrldInd][0]); lvInd++) {

				/*
				//printf("%i\n", (lvInd + (wrldInd * 6)) * 9);
				int cycle = 0, startLoc = 0;

				printf("level: %i\n", (lvInd + (wrldInd * 6)));

				while (!levels.eof()) {
					while (getline(levels, nms)) { //read data from file object and put it into string.
						//if (startLoc >= (lvInd + (wrldInd * 6)) * 9 && startLoc < ((lvInd + (wrldInd * 6)) * 9) + 9) {

							printf("StartLoc: %i, Cycle: %i\n", startLoc, cycle);
							nms = nms.c_str();

							std::vector<std::string> out;
							tokenize(nms.c_str(), ',', out);

							for (int t = 0; t < out.size(); t++) {

								int temp = stoi(out[t].c_str());

								//mapLayout[cycle][t] = temp;
							}

							//printf("%i\n", cycle);

							cycle++;
						} else { cycle = 0; };

						startLoc++;
					}
				}
				*/
				
				level[wrldInd][lvInd].set(GameState({8, 2} ,{4, 0, 7}, {4, 8, 7}, &worldField[wrldInd][lvInd][0][0], fUnits[wrldInd][lvInd], eUnits[wrldInd][lvInd], sizeof(fUnits[0][0]) / sizeof(fUnits[0][0][0]), sizeof(fUnits[0][0]) / sizeof(fUnits[0][0][0])));
			}
		}

		levelBackup.set(level[wrldID][lvID]);
		//levels.close();
	}


	void Game::Shutdown() {
		delete colTile;
		delete defaultAnt;
	}

	bool onMap(coords c) {
		//checks if given coordinates are on teh map
		return (c.x < worldsX && c.x >= 0 && c.y < worldsY && c.y >= 0);
	}

	int getDist(coords c1, coords c2) {

		setDistanceMap(c1);
		//printf("%i\n", distanceMap[c2.x][c2.y]);
		return distanceMap[c2.x][c2.y];
	}

	//calculates distance between 2 positions
	void setDistanceMap(coords c1) {


		/*
		//clean, compact, simple mathematical..
		//absolute rubbish!
		//doesn't coincide with the movement of the game, thus is deemed unusable

		float diffX = float(c1.x) - float(c2.x), diffY = float(c1.y) - float(c2.y);

		return sqrt(pow(diffX, 2.0f) + pow(diffY, 2.0f));
		*/ 



		//now this! This is how you calculate distance.

		for (int x = 0; x < worldsX; x++) {
			for (int y = 0; y < worldsY; y++) {
				distanceMap[x][y] = 20000; //couldn't use memset for some reason :(
			}
		}

		distanceMap[c1.x][c1.y] = 0;
		
		//loops over the entire map, giving each field a value of distance.
		//how this is done is as follows; the entire map is set to 20000 initially, but the coordinate given as a parameter is set to 0.
		//it then loops, using 'it' to check the value, checking if it finds any values equal to 'it'
		//if it does, it sets all neighbouring tiles to 'it + 1'
		//doing this, the next time it loops, those are the tiles that are selected, and the process repeats itself until a complete map of all the units on the field is formed.

		for (int it = 0; it < (worldsX + worldsY) ; it++) {
			for (int x = 0; x < worldsX; x++) {
				for (int y = 0; y < worldsY; y++) {

					if (distanceMap[x][y] == it) {

						for (int dir = 0; dir < 4; dir++) {

							if (distanceMap[x + directions[dir].x][y + directions[dir].y] == 20000 && onMap({ (x + directions[dir].x), (y + directions[dir].y) })) {

								distanceMap[x + directions[dir].x][y + directions[dir].y] = it + 1;

							}
						}
					}
				}
			}
		}
		/*
		for (int x = 0; x < level[lvID].getWorld().x; x++) {
			for (int y = 0; y < level[lvID].getWorld().y; y++) {
				printf("%i ", distanceMap[x][y]);
			}
			printf("\n");
		}
		*/
	}


	float closestUnitDistance, closestU; 
	//int targetX, targetY;
	bool ObjectiveThreatened, unitClose, closeUnitThreatened, unitAttacked;
	

	//-----------------------------------------------------  AI  -----------------------------------------------------//

	int AIattack(Unit* unit, coords selected, int x, int y) {
		//gets threatened fields for attacking units

		if (selected.x == unit->x && selected.y < unit->y) {

			return unit->returnAttackArray(selected, x, y);
		}
		else if (selected.x == unit->x && selected.y > unit->y) {

			return unit->returnAttackArray(selected, x, y);
		}
		else if (selected.x < unit->x && selected.y == unit->y) {

			return unit->returnAttackArray(selected, x, y);
		}
		else if (selected.x > unit->x && selected.y == unit->y) {

			return unit->returnAttackArray(selected, x, y);
		} else return 0;
	}


	bool canMoveHere(Unit* unit, int x, int y) {
	
		//checks if the unit can move to the given coords

		
		switch (unit->getMvType()) {
		case unitMovementType::GROUND:

			if (level[wrldID][lvID].getWorldField(x, y) == 0 && level[wrldID][lvID].getUnitPlacement(x, y) == 0) {

				return true;
			}
			else return false;

			break;
		case unitMovementType::NAVAL:

			if (level[wrldID][lvID].getWorldField(x, y) == 2 && level[wrldID][lvID].getUnitPlacement(x, y) == 0) {

				return true;
			}
			else return false;

			break;
		case unitMovementType::AERIAL:

			if (level[wrldID][lvID].getUnitPlacement(x, y) == 0) {

				return true;
			}
			else return false;

			break;

		default:
			return false;
		}
		

		return true;
	}


	bool findPath(Unit* unit, coords startLoc, coords dest, int maxMoves = (worldsX * worldsY)) {

		std::vector<std::vector<coords>> possiblePaths(1, std::vector<coords>(1, {startLoc.x, startLoc.y}));

		memset(scannedAreas, 0, sizeof(scannedAreas));

		scannedAreas[startLoc.x][startLoc.y] = 1;

		//possiblePaths[0][0] = { startLoc.x, startLoc.y };
		
		int it = 1;
		
		//how this works; the game checks for tiles with where scannedAreas isn't 0; if it finds one, it checks all neighbouring tiles where scannedAreas is 0.
		//doing this (additionally ignoring tiles where either units are or where walls are), the entire board is checked (at least until the target is found) and a path is formed
		//'it' is the amount of steps the unit has to take to reach any given position

		//for any residual confusion after my horrible explanation; https://youtu.be/KiCBXu4P-2Y
		//watch that
		
		if (startLoc.equals(dest)) {

			pathToTake.resize(0);

			pathToTake.push_back({startLoc.x, startLoc.y});

			return true;
		}
		
		do {
			

			int size = possiblePaths.size();

			for (int x = 0; x < worldsX; x++) {
				for (int y = 0; y < worldsY; y++) {

					if (scannedAreas[x][y] == it && onMap({ x, y })) { //if the scannedareas equals it, so this iteration of this loop, and this tile is actually on the map, scan its neighbouring tiles
					
						for (int dir = 0; dir < 4; dir++) {

							if (dest.x == x + directions[dir].x && dest.y == y + directions[dir].y && onMap({ (x + directions[dir].x), (y + directions[dir].y) }) && canMoveHere(unit, (x + directions[dir].x), (y + directions[dir].y))) { //iuf a destination is found in these neighbouring tiles, stop the function and return true
								
								pathToTake.resize(0);

								for (int t = 0; t < size; t++) {
									if (possiblePaths[t].back().equals({ x, y })) {

										for (int t2 = 0; t2 < possiblePaths[t].size(); t2++) {

											pathToTake.push_back(possiblePaths[t][t2]);
										}
									}
								}

								pathToTake.push_back( { x + directions[dir].x ,y + directions[dir].y } );
								
								return true;

							} else if (scannedAreas[x + directions[dir].x][y + directions[dir].y] == 0 && onMap({ (x + directions[dir].x), (y + directions[dir].y) }) && canMoveHere(unit, (x + directions[dir].x), (y + directions[dir].y))) {

								
								for (int t = 0; t < size; t++) {
									if (possiblePaths[t].back().equals({ x, y })) {

										possiblePaths.push_back(possiblePaths[t]);
										possiblePaths.back().push_back({ x + directions[dir].x, y + directions[dir].y });
									}
								}
								
								scannedAreas[x + directions[dir].x][y + directions[dir].y] = it + 1;  //if it isn't found, give every tile neighbouring tot the tile being checked that is still unchecked 'it' as value, thus indicating that they shouldn't be checked again
							}
						}
					}
				}
			}
			it++;

		} while (it <= maxMoves);

		return false;
	}
	
	int getAttackMap(Unit* unit, coords c) {

		int AIThreatenedPlaces[worldsX][worldsY] = { 0 }; //map of all places threatened by AI

		//creates a map of all the spots on the map the unit could possibly attack 

		coords xy = unit->getCoords(); //saves unit's location

		//checks all 4 directions
		for (int dir = 0; dir < 4; dir++) {

			//for every spot on the map
			for (int x = 0; x < worldsX; x++) {
				for (int y = 0; y < worldsY; y++) {

					//max movement range gets checked
					if (findPath(unit, xy, { x, y }, unit->getSpeed())) {

						unit->x = x;  //sets unit location to every square on the field
						unit->y = (y);  //I had to move the unit itself instead of evaluating different coordinates -not that I didn't try, it just didn't work. Too bad!

						//gets threat array for that current location in every direction
						returnThreatArray(unit, unit->getCoords().add(directions[dir]), threatenedPlaces);

						for (int x1 = 0; x1 < worldsX; x1++) {
							for (int y1 = 0; y1 < worldsY; y1++) {
								if (AIThreatenedPlaces[x1][y1] == 0) {
									AIThreatenedPlaces[x1][y1] = threatenedPlaces[x1][y1];
								}
							}
						}
					}
				}
			}
		}

		unit->setCoords(xy);

		return AIThreatenedPlaces[c.x][c.y];
	}

	void AIPossibleMoves(Unit* unit, coords target) {

		memset(AIPMoves, 0, sizeof(AIPMoves));
		

		//printf("X: %i\n", target->getX());
		//printf("Y: %i\n", target->getY());
		coords xy = unit->getCoords(); //saves unit's location

		//for every spot on the map
		for (int x = 0; x < worldsX; x++) {
			for (int y = 0; y < worldsY; y++) {

				//max movement range gets checked
				if (findPath(unit, xy, { x, y }, unit->getSpeed())) {

					//checks all 4 directions
					for (int dir = 0; dir < 4; dir++) {
						unit->x = x;  //sets unit location to every square on the field
						unit->y = y;  //I had to move the unit itself instead of evaluating different coordinates -not that I didn't try, it just didn't work. Too bad!

						//gets threat array for that current location in every direction
						returnThreatArray(unit, unit->getCoords().add(directions[dir]), threatenedPlaces);


						//if the unit can actually move to that place, and the target is being threatened in this spot, add the spot to the AIPMoves array;
						//this way AIPmoves is an array of all the tiles that are threatening the friendly units, that the unit can actually move to
						if (threatenedPlaces[target.x][target.y] == 1) {

							switch (unit->getMvType()) {
							case unitMovementType::GROUND:

								if (level[wrldID][lvID].getWorldField(x, y) == 0) {

									AIPMoves[x][y] = 1;
								}

								break;
							case unitMovementType::NAVAL:

								if (level[wrldID][lvID].getWorldField(x, y) == 2) {

									AIPMoves[x][y] = 1;
								}

								break;
							case unitMovementType::AERIAL:

								AIPMoves[x][y] = 1;

								break;
							default:
								AIPMoves[x][y] = 0;
							}
						}
					}
				}
			}
		}

		unit->setCoords(xy);
		memset(scannedAreas, 0, sizeof(scannedAreas));
		
		/*
		//disables AIPMoves if the unit can't actually move there with the pathfinding algorithm
		for (int x = 0; x < level[lvID].getWorld().x; x++) {
			for (int y = 0; y < level[lvID].getWorld().y; y++) {

				if (AIPMoves[x][y] == 1) {

					if (!findPath(unit, unit->getCoords(), { x, y }, unit->getSpeed())) {
						AIPMoves[x][y] = 0;
					}
				}
			}
		}
		*/
	}

	std::vector<coords> destinations(level[wrldID][lvID].getEUnitsAm());

	coords getTarget(int uIndex) {

		//--------------------------------------------//
		//											  //
		// Priority Shopping List:					  //
		//											  //
		// 1. attack units threatening objective	  //
		//											  //
		// 2. attack units threatening you			  //
		//											  //
		// 3. attack enemy objective if in range	  //
		//											  //
		// 4. attack closest by						  //
		//											  //
		//--------------------------------------------//

		destinations.resize(level[wrldID][lvID].getEUnitsAm());
		int am = level[wrldID][lvID].getFUnitsAm();
		std::vector<int> priority(am);
		int distance = 20000;

		Unit target;
		target.set(Unit(-1, -1, 0, 0, unitType::MELEE, unitMovementType::GROUND));
		bool isBeingAttacked = false;

		//AIPossibleMoves();

		for (int t = 0; t < level[wrldID][lvID].getFUnitsAm(); t++) {
			if (level[wrldID][lvID].fUnits[t].isAlive()) {


				priority[t] = 0;
				//getDist(level[lvID].fUnits[t].getCoords(), level[lvID].eObjtv.getCoords()) <=
				if ((AIattack(&level[wrldID][lvID].fUnits[t], level[wrldID][lvID].eObjtv.getCoords(), level[wrldID][lvID].eObjtv.x, level[wrldID][lvID].eObjtv.y) == 1 && getDist(level[wrldID][lvID].fUnits[t].getCoords(), level[wrldID][lvID].eUnits[uIndex].getCoords()) <= level[wrldID][lvID].fUnits[t].getSpeed()) || getDist(level[wrldID][lvID].fUnits[t].getCoords(), level[wrldID][lvID].eObjtv.getCoords()) <= 3) {

					priority[t] = 6;

				}
				else if (AIattack(&level[wrldID][lvID].fUnits[t], level[wrldID][lvID].eUnits[uIndex].getCoords(), level[wrldID][lvID].eUnits[uIndex].x, level[wrldID][lvID].eUnits[uIndex].y) == 1) { //checks if unit is being attacked

					priority[t] = 5;
					isBeingAttacked = true;

				}
				else if (!isBeingAttacked && getAttackMap(&level[wrldID][lvID].eUnits[uIndex], level[wrldID][lvID].fObjtv.getCoords())) {

					//printf("%i\n", level[lvID].fObjtv.getX());
					//printf("%i\n", level[lvID].fObjtv.getY());
					return level[wrldID][lvID].fObjtv.getCoords();

				}
			}
		}

		int dist = 20000;
		int temp = 0;
		bool allZero = true;



		for (int t = 0; t < level[wrldID][lvID].getFUnitsAm(); t++) {

			if (level[wrldID][lvID].fUnits[t].isAlive()) {

				if (priority[t] != 0) { //test if all enemies have priority of 0
					allZero = false; 
				}

				if (priority[t] == temp) {

					if (getDist(level[wrldID][lvID].eUnits[uIndex].getCoords(), level[wrldID][lvID].fUnits[t].getCoords()) <= dist) {

						dist = getDist(level[wrldID][lvID].eUnits[uIndex].getCoords(), level[wrldID][lvID].fUnits[t].getCoords());

						target.set(level[wrldID][lvID].fUnits[t]);
						temp = priority[t];
					}

				} else if (priority[t] > temp) {

					temp = priority[t];
					target.set(level[wrldID][lvID].fUnits[t]);
				}
			}
		}
		
		//if all friendly units have a priority of 0; attack the closest one
		if (allZero) {

			int objDist = getDist(level[wrldID][lvID].eUnits[uIndex].getCoords(), level[wrldID][lvID].fObjtv.getCoords());
			coords tempCoords = level[wrldID][lvID].fObjtv.getCoords();

			for (int t = 0; t < level[wrldID][lvID].getFUnitsAm(); t++) {
				if (level[wrldID][lvID].fUnits[t].isAlive()) {
					if (objDist > getDist(level[wrldID][lvID].eUnits[uIndex].getCoords(), level[wrldID][lvID].fUnits[t].getCoords())) {

						objDist = getDist(level[wrldID][lvID].eUnits[uIndex].getCoords(), level[wrldID][lvID].fUnits[t].getCoords());
						tempCoords.set(level[wrldID][lvID].fUnits[t].x, level[wrldID][lvID].fUnits[t].y);
					}
				}
			}
			destinations[uIndex].set(tempCoords.x, tempCoords.y);
			return tempCoords;
		}
		destinations[uIndex].set(target.x, target.y);
		return target.getCoords();
	}

	
	bool AIMoving = false;

	void AIstsep(int turnSt, int uIndex) { //no need to pass a pointer to eUnits; AI will only address enemy units anyways --a shame, otherwise we could have AI vs AI battles :(
	
		//destinations.resize(level[lvID].getEUnitsAm());
		resetUnits();

		if (turnSt == 2) {

			setDistanceMap(level[wrldID][lvID].eUnits[uIndex].getCoords());

			coords loc = getTarget(uIndex);

			int closestDistance = 20000;
			coords closestField;
			bool NoAIPMoves = true;

				//get target
				//get all tiles that can attack target
				//get closest tile to attack target with
				//find path to this closest tile

			AIPossibleMoves(&level[wrldID][lvID].eUnits[uIndex], loc);
			closestField.set(loc.x, loc.y);

			//get closest tile in AIPMoves
			for (int x = 0; x < worldsX; x++) {
				for (int y = 0; y < worldsY; y++) {

					if (AIPMoves[x][y] == 1) {
						if (closestDistance >= getDist(level[wrldID][lvID].eUnits[uIndex].getCoords(), { x, y })) {
							closestDistance = getDist(level[wrldID][lvID].eUnits[uIndex].getCoords(), { x, y });

							closestField.set(x, y);
							NoAIPMoves = false;
						}
					}
				}
			}

			//if AIPMoves contains no possible tiles; find closest tile to target.
			closestDistance = 20000;
			if (NoAIPMoves) {
				for (int x = 0; x < worldsX; x++) {
					for (int y = 0; y < worldsY; y++) {
						if (findPath(&level[wrldID][lvID].eUnits[uIndex], level[wrldID][lvID].eUnits[uIndex].getCoords(), { x, y }, level[wrldID][lvID].eUnits[uIndex].getSpeed()) && closestDistance >= getDist({x, y}, loc)) {
							
							closestDistance = getDist({ x, y }, loc);
							closestField.set(x, y);
						}
					}
				}
			}

			//destinations[uIndex].set(closestField.x, closestField.y);
			findPath(&level[wrldID][lvID].eUnits[uIndex], level[wrldID][lvID].eUnits[uIndex].getCoords(), closestField);

			AIMoving = true;
			AItimer = 0;

		}
		else {

			level[wrldID][lvID].eUnits[uIndex].attackState = 1;
			returnThreatArray(&level[wrldID][lvID].eUnits[uIndex], destinations[uIndex], threatenedPlaces);
			attackUnits(level[wrldID][lvID].eUnits, level[wrldID][lvID].fUnits, destinations[uIndex], level[wrldID][lvID].getEUnitsAm(), level[wrldID][lvID].getFUnitsAm(), &level[wrldID][lvID].fObjtv);
		} 
	}


	void Game::Tick(float deltaTime)
	{
		//printf("%i\n", level[wrldID][lvID].fObjtv.getHealth());

		//printf("%s", level[wrldID][lvID].winloss() ? "true" : "false");
		//system("CLS");
		//printf("X: %i Y: %i\n", mousePos.x, mousePos.y);
		

		screen->Clear(0xffffff);

		mousePos = getMousePosInt();

		if (inGame) {

			button.SetFrame(goBackBtn.type);
			button.Draw(screen, goBackBtn.x, goBackBtn.y);
			//screen->Box(goBackBtn.x, goBackBtn.y, goBackBtn.getX2(), goBackBtn.getY2(), 0x000000);

			//marks where the units are on the field
			for (int i = 0; i < level[wrldID][lvID].getFUnitsAm(); i++) {
				if (level[wrldID][lvID].fUnits[i].isAlive()) {
					level[wrldID][lvID].setUnitPlacement(level[wrldID][lvID].fUnits[i].x, level[wrldID][lvID].fUnits[i].y, 1);
				}
				else {
					level[wrldID][lvID].setUnitPlacement(level[wrldID][lvID].fUnits[i].x, level[wrldID][lvID].fUnits[i].y, 0);
				} 
			}

			for (int i = 0; i < level[wrldID][lvID].getEUnitsAm(); i++) {
				if (level[wrldID][lvID].eUnits[i].isAlive()) {
					level[wrldID][lvID].setUnitPlacement(level[wrldID][lvID].eUnits[i].x, level[wrldID][lvID].eUnits[i].y, 1);
				}
				else {
					level[wrldID][lvID].setUnitPlacement(level[wrldID][lvID].eUnits[i].x, level[wrldID][lvID].eUnits[i].y, 0);
				}
			}

			level[wrldID][lvID].setUnitPlacement(level[wrldID][lvID].eObjtv.x, level[wrldID][lvID].eObjtv.y, 1);
			level[wrldID][lvID].setUnitPlacement(level[wrldID][lvID].fObjtv.x, level[wrldID][lvID].fObjtv.y, 1);

			if (turnState < 2) {
				setDistanceMap(selectedUnit);
			}

			//printf("TurnState: %i\n", turnState);

			//activates x times per second -- 60 fps / amount of frames per interval = interval per second

			if (AItimer % 60 == 0 && !AIMoving) {
				if (turnState >= 2) {

					resetUnits();

					if (AIstep < level[wrldID][lvID].getEUnitsAm()) {

						if (level[wrldID][lvID].eUnits[AIstep].isAlive()) {

							AIstsep(turnState, AIstep);
						}
						else {
							AItimer = 59;
						}

						AIstep++;
					}
					else {
						if (turnState + 1 == 4) {
							turnState = 0;
						}
						else {
							turnState++;
						}
						AIstep = 0;
					}
				}
				AItimer = 0;
			}

			if (AItimer % 10 == 0 && AIMoving) {
				if (turnState == 2) {
					if (moveStep < pathToTake.size()) {

						//printf("%s\n", AIMoving ? "True" : "False");

						level[wrldID][lvID].eUnits[AIstep - 1].setCoords(pathToTake[moveStep]);

						moveStep++;
					}
					else {

						AIMoving = false;;
						moveStep = 0;
						AItimer = 0;
					}
				}
			}
			AItimer++;

			//draws turn button
			switch (turnState) {
			case 0: screen->Bar(770, 450, 830, 500, 0x0000ff);
				break;
			case 1: screen->Bar(770, 450, 830, 500, 0xff0000);
				break;
			}

			currentCell = { mousePos.x / tileSize.x, mousePos.y / tileSize.y };

			offset = { mousePos.x % tileSize.x, mousePos.y % tileSize.y };

			selected = {
				(currentCell.y - level[wrldID][lvID].getOrigin().y) + (currentCell.x - level[wrldID][lvID].getOrigin().x),
				(currentCell.y - level[wrldID][lvID].getOrigin().y) - (currentCell.x - level[wrldID][lvID].getOrigin().x)
			};

			Pixel* col = colTile->GetBuffer() + offset.x + offset.y * tileSize.x;

			//see if cursor hovers right outside of tile
			//kinda disgusting with the +256, -256 but hey ho gets the job done
			if (*col == -256) selected.x += 1;
			if (*col == -(0xff0000 + 256)) selected.y -= 1;
			if (*col == -(0x00ff00 + 256)) selected.x -= 1;
			if (*col == -(0xffff00 + 1)) selected.y += 1;

			//transform selected coords into screen px
			selectedWorldPx = toScreen(selected.x, selected.y);

			//selectTileS.DrawScaled(40, 20, 64, 48, screen);

			//generate map with loop
			for (int x = 0; x < worldsX; x++) {
				for (int y = 0; y < worldsY; y++) {

					coords vWorld = toScreen(x, y);

					//draws tiles
					switch (level[wrldID][lvID].getWorldField(x, y)) {
					case 0:
						groundTiles.SetFrame(0);
						groundTiles.Draw(screen, vWorld.x, vWorld.y);

						break;
					case 1:
						groundTiles.SetFrame(1);
						groundTiles.Draw(screen, vWorld.x, vWorld.y);

						break;
					case 2:
						groundTiles.SetFrame(2);
						groundTiles.Draw(screen, vWorld.x, vWorld.y);

						break;
					}

					//generates square indicators of where unit can move to
					for (int t = 0; t < level[wrldID][lvID].getFUnitsAm(); t++) {
						if (level[wrldID][lvID].fUnits[t].moveState == 1) {
							//draws path unit has taken this turn
							for (int t1 = 0; t1 < level[wrldID][lvID].fUnits[t].getSpeed(); t1++) {
								if (level[wrldID][lvID].fUnits[t].getPastPlaces(t1).x == x && level[wrldID][lvID].fUnits[t].getPastPlaces(t1).y == y && level[wrldID][lvID].fUnits[t].moveState == 1) {
									//draw(1, 0, screen, tiles, vWorld.x, vWorld.y, tileSize.y + 32, tileSize.x * 3, tileSize.y + 32, tileSize.x);
									pathTile.DrawScaled(vWorld.x, vWorld.y, 64, 48, screen);
								}
							}

							//draws fields unit can directly move to
							if (distanceMap[x][y] <= 1 && level[wrldID][lvID].fUnits[t].moveState == 1 && canMoveHere(&level[wrldID][lvID].fUnits[t], x, y)) {

								selectTileS.DrawScaled(vWorld.x, vWorld.y, 64, 48, screen);
							}

							level[wrldID][lvID].setUnitPlacement(x, y, 0);
						}
					}

					//draws selected tile
					if (selected.x == x && selected.y == y) {
						//draw(0, 0, screen, selectTile, selectedWorldPx.x, selectedWorldPx.y, tileSize.y, tileSize.x, tileSize.y, tileSize.x);
						selectTileS.DrawScaled(selectedWorldPx.x, selectedWorldPx.y, 64, 48, screen);
					}

					if (turnState != 2 && threatenedPlaces[x][y] == 1) {
						selectTileS.DrawScaled(vWorld.x, vWorld.y, 64, 48, screen);
					}
				}
			}

			for (int x = 0; x < worldsX; x++) {
				for (int y = 0; y < worldsY; y++) {
					if (level[wrldID][lvID].getWorldField(x, y) == 1) {

						coords vWorld = toScreen(x, y);

						Mountain.DrawScaled(vWorld.x, vWorld.y - 20, 64, 64, screen);
					}
				}
			}

			//draws friendly units
			drawUnits(screen, level[wrldID][lvID].fUnits, level[wrldID][lvID].getFUnitsAm());

			//draws enemy units
			drawUnits(screen, level[wrldID][lvID].eUnits, level[wrldID][lvID].getEUnitsAm());

			//draws objectives for both teams
			drawObjectives(level[wrldID][lvID].fObjtv, screen, &antHill);
			drawObjectives(level[wrldID][lvID].eObjtv, screen, &antHill);
		}
		else {
			
			//add menu here

			//screen->Box(300, 300, 350, 350, 0x000000);

			

			if (worldSelect == 100) {

				planetSelect.DrawScaled(0, 0, ScreenWidth, ScreenHeight, screen);

				button.SetFrame(9);
				button.Draw(screen, goBackBtn.x, goBackBtn.y);

				//menu.Draw(screen, 0, 0);
			}
			else {

				switch (worldSelect) {
				case 0: planet0.DrawScaled(0, 0, ScreenWidth, ScreenHeight, screen);
					break; 
				case 1: planet1.DrawScaled(0, 0, ScreenWidth, ScreenHeight, screen);
					break; 
				case 2: planet2.DrawScaled(0, 0, ScreenWidth, ScreenHeight, screen);
					break;
				case 3: planet3.DrawScaled(0, 0, ScreenWidth, ScreenHeight, screen);
					break;
				}
				

				//screen->Box(goBackBtn.x, goBackBtn.y, goBackBtn.getX2(), goBackBtn.getY2(), 0x000000);
				button.SetFrame(goBackBtn.type);
				button.Draw(screen, goBackBtn.x, goBackBtn.y);
				
				for (int t = 0; t < sizeof(levelSelectBtns[worldSelect]) / sizeof(levelSelectBtns[worldSelect][0]); t++) {
					
					//printf("%i\n", levelSelectBtns[worldSelect][1].getX2());
					//screen->Box(levelSelectBtns[worldSelect][t].x, levelSelectBtns[worldSelect][t].y, levelSelectBtns[worldSelect][t].getX2(), levelSelectBtns[worldSelect][t].getY2(), 0x000000);

					button.SetFrame(levelSelectBtns[worldSelect][t].type);
					button.Draw(screen, levelSelectBtns[worldSelect][t].x, levelSelectBtns[worldSelect][t].y);
				}
			}
		}

		antCEO.DrawScaled(600, 0, 384, 648, screen);
	}
};