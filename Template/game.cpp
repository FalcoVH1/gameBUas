#include "game.h"
#include "surface.h"
#include "template.h"
#include <cmath>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h> 
#include <string>
#include <cstring>
#include <vector>


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

	//sprite declarations

	//these aren't sprites because they use a different rendering function
	Surface* colTile = new Surface("assets/inGame/specialTile.png");
	Surface* defaultAnt = new Surface("assets/inGame/defaultAnt.png");
	Surface* defaultEnAnt = new Surface("assets/inGame/defaultEnAnt.png");

	//all the sprites
	Sprite planetSelect(new Surface("assets/UI/menuScreen.png"), 1);
	Sprite planetSelectTxt(new Surface("assets/UI/planetSelectTxt.png"), 1);
	Sprite planet0(new Surface("assets/UI/planet0.png"), 1);
	Sprite planet1(new Surface("assets/UI/planet1.png"), 1);
	Sprite planet2(new Surface("assets/UI/planet2.png"), 1);
	Sprite planet3(new Surface("assets/UI/planet3.png"), 1);
	Sprite button(new Surface("assets/UI/button.png"), 11);

	Sprite selectTileS(new Surface("assets/inGame/selectTile.png"), 1);
	Sprite pathTile(new Surface("assets/inGame/pathTile.png"), 1);
	Sprite groundTiles(new Surface("assets/inGame/defTiles.png"), 8);
	Sprite healthBar(new Surface("assets/inGame/healthBar.png"), 4);
	Sprite objHealthBar(new Surface("assets/inGame/objHealthBar.png"), 8);
	Sprite levelBackground(new Surface("assets/inGame/levelBackground.png"), 1);
	Sprite mountain(new Surface("assets/inGame/mountain.png"), 1);
	Sprite dmountain(new Surface("assets/inGame/dmountain.png"), 1);
	Sprite antHill(new Surface("assets/inGame/antsObj.png"), 1);
	Sprite enAntHill(new Surface("assets/inGame/enAntsObj.png"), 1);
	Sprite unitInfo(new Surface("assets/UI/unitInfo.png"), 1);
	Sprite menuHeader(new Surface("assets/UI/menuHeader.png"), 1);
	Sprite phaseInd(new Surface("assets/UI/phaseInd.png"), 2);
	Sprite attackPattern(new Surface("assets/UI/attackPattern.png"), 4);
	Sprite environmentalHazardInfo(new Surface("assets/UI/environmentalHazardInfo.png"), 1);

	int AItimer = 0, AIstep = 0, turnState = 0, lvID = 0, wrldID = 0, moveStep = 0, worldSelect = 100, levelSelect = 100;
	bool inGame = false, inDialogue = false;

	coords mousePos(0, 0), currentCell(0, 0), offset(0, 0), selected(0, 0), selectedWorldPx(0, 0), selectedUnit(0, 0);

	static const coords tileSize(64, 48), Origin(7, 3);

	levelState lvState[worldsAm][levelsPerWorld];

	static const coords directions[4] = {
		coords(0, 1),
		coords(0, -1),
		coords(1, 0),
		coords(-1, 0),
	};

	LevelButton goBackBtn(20, 20, 50, 50, 6), continueBtn(845, 630, 50, 50, 8), cancelBtn(905, 630, 50, 50, 9), confirmBtn(905, 630, 50, 50, 8);

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

	Unit infoSelectedUnit;

	GameState level, levelBackup;

	std::vector<coords> pathToTake(0);

	//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

	//function prototypes for important functions
	void setDistanceMap(coords c1);
	bool onMap(coords c);
	bool canMoveHere(Unit* unit, int x, int y);
	int getDist(coords c1, coords c2);
	void setUnitMap();
	const void resetUnits();


	//function to move units
	void moveUnits(Unit* fUnits, Unit* eUnits, coords selectedC) {

		//log all units currently alive and on the map
		setUnitMap();

			//resets attacking before move phase
			memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
			for (int i = 0; i < friendlyUnitsAm; i++) {
				fUnits[i].attackState = 0;
			};

			for (int i = 0; i < enemyUnitsAm; i++) {
				eUnits[i].attackState = 0;
			};

			for (int t = 0; t < friendlyUnitsAm; t++) {

				//if units haven't moved, transition to them moving

				if (fUnits[t].moveState == 0) {
					if (selectedC.equals(fUnits[t].getCoords())) {
						fUnits[t].moveState = 1;
					}
				}
				else if (fUnits[t].moveState == 1) {

					bool targetOccupied = false;
					//first checks if the move is eligible, then sets a new destination for the unit to move to. if the move is ineligible, the unit is set back to movestate 0, aka "hasn't moved".
					if (selectedC.equals(level.eObjtv.getCoords()) || selectedC.equals(level.fObjtv.getCoords())) {
						targetOccupied = true;
					}

					//if the player clicks on a tile 1 tile away from unit; move the unit 1 tile, else; cancel movement by turning movement to 0
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

	void attackUnits(Unit* fUnits, Unit* eUnits, coords selectedC, Objective* Obj) {

		//resets movement before attack phase
		for (int i = 0; i < friendlyUnitsAm; i++) {
			fUnits[i].moveState = 0;
			fUnits[i].moveState = 0;
		};

		for (int i = 0; i < enemyUnitsAm; i++) {
			eUnits[i].moveState = 0;
			eUnits[i].moveState = 0;
		};

		for (int t = 0; t < friendlyUnitsAm; t++) {
			if (fUnits[t].attackState == 0) {
				//starts target selection
				if (selectedC.equals(fUnits[t].getCoords())) {
					fUnits[t].attackState = 1;
				}
			}
			else if (fUnits[t].attackState == 1) {

				//resets attack when clicking other friendly unit

				fUnits[t].attackState = 3;
				for (int t1 = 0; t1 < friendlyUnitsAm; t1++) {
					if (selectedC.equals(fUnits[t1].getCoords())) {
						fUnits[t].attackState = 0;
					}
				}

				//execute attack by damaging all enemy units in attack range
				for (int t1 = 0; t1 < enemyUnitsAm; t1++) {
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

		//check if the player beat the level or not; if so, unlock next level, else; send player back to menu
		if (level.winloss() == winloss::WON) {

			level.set(levelBackup);
			resetUnits();
			turnState = 0;
			worldSelect = 100;
			lvState[wrldID][lvID]= levelState::COMPLETED;
			if (lvID + 1 <= levelsPerWorld) lvState[wrldID][lvID + 1] = levelState::STARTED;
			else if (lvID + 1 == levelsPerWorld && wrldID + 1 <= worldsAm) lvState[wrldID + 1][0] = levelState::STARTED;
			inGame = false;
		}
		else if (level.winloss() == winloss::LOSS) {
			level.set(levelBackup);
			resetUnits();
			turnState = 0;
			worldSelect = 100;
			lvState[wrldID][lvID] = levelState::STARTED;
			inGame = false;
		}
	}

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

	void setUnitMap() {
		//marks where the units are on the field
		for (int i = 0; i < friendlyUnitsAm; i++) {
			if (level.fUnits[i].isAlive()) {
				level.setUnitPlacement(level.fUnits[i].x, level.fUnits[i].y, 1);
			}
			else {
				level.setUnitPlacement(level.fUnits[i].x, level.fUnits[i].y, 0);
			}
		}

		for (int i = 0; i < enemyUnitsAm; i++) {
			if (level.eUnits[i].isAlive()) {
				level.setUnitPlacement(level.eUnits[i].x, level.eUnits[i].y, 1);
			}
			else {
				level.setUnitPlacement(level.eUnits[i].x, level.eUnits[i].y, 0);
			}
		}

		level.setUnitPlacement(level.eObjtv.x, level.eObjtv.y, 1);
		level.setUnitPlacement(level.fObjtv.x, level.fObjtv.y, 1);

	}

	//converts ingame coordinates to on screen coordinates
	coords toScreen(int x, int y) {
		return coords{
				(Origin.x * tileSize.x) + (x - y) * (tileSize.x / 2),
				(Origin.y * tileSize.y) + (x + y) * (tileSize.y / 2)
		};
	}

	//draws units
	void drawUnits(Surface* screen, Unit* fUnits, int fUnitsAm, bool side) {
		
		for (int t = 0; t < fUnitsAm; t++) {

			//kills land units if they somehow arrive on water
			if (fUnits[t].getMvType() == unitMovementType::GROUND && level.getWorldField(fUnits[t].x, fUnits[t].y) == 2) {
				fUnits[t].kill();
			}

			//kills naval units if they somehow arrive on land
			if (fUnits[t].getMvType() == unitMovementType::NAVAL && !(level.getWorldField(fUnits[t].x, fUnits[t].y) == 2 || level.getWorldField(fUnits[t].x, fUnits[t].y) == 4)) {
				fUnits[t].kill();
			}

			if (fUnits[t].isAlive()) {

				coords temp = toScreen(fUnits[t].x, fUnits[t].y);

				int dir, type;

				//gets direction from the unit to set the angle of the texture
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

				//set sprite based on unit type
				switch (fUnits[t].getMvType()) {
				case unitMovementType::AERIAL: type = 0;
					break;
				case unitMovementType::GROUND: type = 1;
					break;
				case unitMovementType::NAVAL: type = 2;
					break;
				}

				Pixel* src;

				//depending on what side, change texture to enemy ants or allied ants
				if (side) {
					src = defaultAnt->GetBuffer() + dir * tileSize.x + (type * tileSize.x) * (64 * 4); //uses tilesize because units are the same size as the tiles :)
				}
				else {
					src = defaultEnAnt->GetBuffer() + dir * tileSize.x + (type * tileSize.x) * (64 * 4);
				}
					
				//wrote this code way back, decided to keep it. No reason in particular, but I find it to be something neat that I wrote myself (somehow) that I kind of wanted to keep in
				//draws ants lol
				
				Pixel* dst = screen->GetBuffer() + temp.x + (temp.y - 14) * ScreenWidth;
				for (int i = 0; i < tileSize.x; i++, src += (64 * 4), dst += ScreenWidth)
					for (int j = 0; j < tileSize.x; j++)
						if (src[j] != NULL) {
							dst[j] = src[j];
						}


				//gets the center of the movable-unit-circle thingy
				if (fUnits[t].moveState == 1) {
					selectedUnit.set(fUnits[t].getCoords().x, fUnits[t].getCoords().y);
				}

				if (fUnits[t].attackState == 1) {

					level.areAttacking = true;

					//draws cancel attack button
					//it's here because it's easier to do it here

					button.SetFrame(cancelBtn.type);
					button.Draw(screen, cancelBtn.x, cancelBtn.y);

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

		//if objective is alive; draw it on the field

		if (fObj.getHealth() > 0) {

			coords temp = toScreen(fObj.x, fObj.y);

			obj->DrawScaled(temp.x, temp.y - 20, 64, 64, screen);
		}
	}

	const void resetUnits() {
		memset(AIPMoves, 0, sizeof(AIPMoves));
		memset(threatenedPlaces, 0, sizeof(threatenedPlaces));

		for (int i = 0; i < friendlyUnitsAm; i++) {
			level.fUnits[i].attackState = 0;

			level.fUnits[i].moveState = 0;

			level.fUnits[i].clearPastPlaces();
		};

		for (int i = 0; i < enemyUnitsAm; i++) {
			level.eUnits[i].attackState = 0;

			level.eUnits[i].moveState = 0;
		};
	}

	////////////////////////////////////////////////////////////////////////
	//
	// handles what happens when the mouse is clicked;
	//
	// when clicking a button, execute the button's function.
	// -clicking on a planet selects that world
	// -selects level on a planet and loads it
	// -goes back when clicking the return button
	// -set selected unit when clicked on a unit
	//

	void Game::MouseDown(int button) {

		//lambda expression to enter the level by loading it from file
		auto enterLevel = [](int t) {
			lvID = t;
			wrldID = worldSelect;

			//loads file based on currently selected world and level
			string path = "assets/levels/level" + to_string(wrldID) + to_string(lvID) + ".txt";
			level.set(GameState(path));

			//sets the levelbackup to the loaded level, so the base layout of the level is preserved in case the player wants to restart the level
			levelBackup.set(level);

			//enters the game
			inGame = true;
		};

		//if the mouseclick is outside of the field or the selected tile isn't an actual unit; sets the selected unit to a nonexistent one
		if (selected.x >= worldsX || selected.x < 0 || selected.y >= worldsY || selected.y < 0 || level.getUnitPlacement(selected.x, selected.y) == 0) 
			infoSelectedUnit.set({-1, -1, -1, -1, unitType::MELEE, unitMovementType::GROUND});;
		
		
		if (inGame) {

			//if the player is located on the go back button while in game, return to menu and set the level back to its default state.
			if (mouseLocated(goBackBtn.x, goBackBtn.y, goBackBtn.getX2(), goBackBtn.getY2())) {

				level.set(levelBackup);
				resetUnits();
				turnState = 0;
				worldSelect = 100;

				inGame = false;
			}


			//-------------------------------------------------player-turn------------------------------------------------//
			
			//if it's the player's turn
			if (turnState < 2) {
				//button to transition between move and attack phase
				if (mouseLocated(continueBtn.x, continueBtn.y, continueBtn.getX2(), continueBtn.getY2())) {
					turnState++;

					//sets AItimer so the AI always moves at the exact same time after the player's turn has ended
					if (turnState >= 2) {
					AItimer = 61;
					}
				}

				//cancels attacking phase
				if (mouseLocated(cancelBtn.x, cancelBtn.y, cancelBtn.getX2(), cancelBtn.getY2()) && level.areAttacking) {
					memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
					for (int t = 0; t < friendlyUnitsAm; t++) {
						level.fUnits[t].attackState = 0;
					}
				}


				if (mouseLocated(confirmBtn.x, confirmBtn.y, confirmBtn.getX2(), confirmBtn.getY2()) && level.areMoving()) {

					for (int t = 0; t < friendlyUnitsAm; t++) {
						if (level.fUnits[t].moveState == 1) {
							level.fUnits[t].moveState = 2;
						}
					}
				}
			}
			//--------------------------------------------------------------------------------------//

			for (int i = 0; i < friendlyUnitsAm; i++) {
				if (level.fUnits[i].getCoords().equals(selected)) infoSelectedUnit.set(level.fUnits[i]);
			};
			for (int i = 0; i < enemyUnitsAm; i++) {
				if (level.eUnits[i].getCoords().equals(selected)) infoSelectedUnit.set(level.eUnits[i]);
			};

			switch (turnState) {
			case 0:

				//move friendly units
				moveUnits(level.fUnits, level.eUnits, selected);

				break;
			case 1:
				Objective * eObjtvPointer;
				eObjtvPointer = &level.eObjtv;

				//attack with friendly units
				attackUnits(level.fUnits, level.eUnits, selected, eObjtvPointer);

				break;
			}
		}
		else { //else if the player isn't in game, rather in the menu
			if (worldSelect == 100) { //100 is arbitrary; if worldSelect == 100, no world has been selected yet. 

				//if the mouse is located on the back button while in the planet select screen, exit out of the game
				if (mouseLocated(goBackBtn.x, goBackBtn.y, goBackBtn.getX2(), goBackBtn.getY2())) {
					exit(0);
				}

				//checks if the player clicks any of the 4 planets by looping over worldSelectBtns
				for (int t = 0; t < sizeof(worldSelectBtns) / sizeof(worldSelectBtns[0]); t++) {

					if (mouseLocated(worldSelectBtns[t].x, worldSelectBtns[t].y, worldSelectBtns[t].getX2(), worldSelectBtns[t].getY2())) {
						worldSelect = t;
					}
				}
			}
			else { //if worldselect isn't 100, meaning a planet has been selected, ask which level they want to play

				//if the player clicks the back button, return to the planet select screen.
				if (mouseLocated(goBackBtn.x, goBackBtn.y, goBackBtn.getX2(), goBackBtn.getY2())) {
					worldSelect = 100;
				}
				else { 
					for (int t = 0; t < levelsPerWorld; t++) { //checks if the player clicks a level

						if (mouseLocated(levelSelectBtns[worldSelect][t].x, levelSelectBtns[worldSelect][t].y, levelSelectBtns[worldSelect][t].getX2(), levelSelectBtns[worldSelect][t].getY2())) {

							if (lvState[worldSelect][t] != levelState::INACCESSIBLE) { //if the state of the level isn't 'INACCESSIBLE', enter the level
								enterLevel(t);
							}
						}
					}
				}
			}
		}
	};

	void Game::KeyDown(int key) {
		//does nothing on key down.
	}

	void Game::Init() {

		//at the start of the game, set all the levels to INACCESSIBLE. However; this doesn't load them in just yet, that happens when a level is actually selected.
		//doing this the amount of time reading from the level files in minimized 
		//(not that it really matters, but efficiency's fun, you know? Although, if I wanted to be even more efficient, storing it in bin files would've probs been better than txt files, but that seemed a little too complicated at the time.)

		for (int i = 0; i < worldsAm; i++) {
			for (int ii = 0; ii < levelsPerWorld; ii++) {
				lvState[i][ii] = levelState::INACCESSIBLE;
			}
		}

		//set the first level to started
		levelBackup.set(level);
		lvState[0][0] = levelState::STARTED;
	}


	void Game::Shutdown() {

		//delete all pointers to avoid memory leaks
		delete colTile;
		delete defaultAnt;
		delete defaultEnAnt;
	}

	bool onMap(coords c) {
		//checks if given coordinates are on the map
		return (c.x < worldsX && c.x >= 0 && c.y < worldsY && c.y >= 0);
	}

	int getDist(coords c1, coords c2) {

		setDistanceMap(c1);
		return distanceMap[c2.x][c2.y];
	}

	//calculates distance between 2 positions
	void setDistanceMap(coords c1) {


		/*

		This was the previous iteration of distance calculation; clean, compact, simple pythagoras calculation..
		absolute rubbish!
		doesn't coincide with the movement of the game
		for context; say that from a certain point, I would mark every point on the map with a distance of 2 or less to that fixed point
		using pythagoras, that'd look like this;

		0 0 0 0 0 0 0
		0 0 X X X 0 0
		0 X X X X X 0
		0 X X O X X 0
		0 X X X X X 0
		0 0 X X X 0 0
		0 0 0 0 0 0 0

		which isn't the shape I want!

		the shape I that I needed is the following;

		0 0 0 0 0 0 0
		0 0 0 X 0 0 0
		0 0 X X X 0 0
		0 X X O X X 0
		0 0 X X X 0 0
		0 0 0 X 0 0 0
		0 0 0 0 0 0 0

		thus, this was deemed unusable

		float diffX = float(c1.x) - float(c2.x), diffY = float(c1.y) - float(c2.y);

		return sqrt(pow(diffX, 2.0f) + pow(diffY, 2.0f));
		*/ 



		//now this! This is how you calculate distance. ;)

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
	}


	float closestUnitDistance, closestU; 
	//int targetX, targetY;
	bool ObjectiveThreatened, unitClose, closeUnitThreatened, unitAttacked;
	

	//-----------------------------------------------------  AI  -----------------------------------------------------//

	bool canMoveHere(Unit* unit, int x, int y) {
	
		//checks if the unit can move to the given coords, using various restrictions based on movement

		/*
		How it goes;

		naval units can walk on water and lava tiles

		aerial unit scan walk on every tile

		grounded units can walk on every tile except from water and mountain tiles
		
		*/

		switch (unit->getMvType()) {
		case unitMovementType::GROUND:

			if ((level.getWorldField(x, y) == 4 || level.getWorldField(x, y) == 3 || level.getWorldField(x, y) == 0 || level.getWorldField(x, y) == 7 || level.getWorldField(x, y) == 8) && level.getUnitPlacement(x, y) == 0) {

				return true;
			}
			else return false;

			break;
		case unitMovementType::NAVAL:

			if ((level.getWorldField(x, y) == 4 || level.getWorldField(x, y) == 2) && level.getUnitPlacement(x, y) == 0) {

				return true;
			}
			else return false;

			break;
		case unitMovementType::AERIAL:

			if (level.getUnitPlacement(x, y) == 0 && level.getWorldField(x, y) != 5) {

				return true;
			}
			else return false;

			break;

		default:
			return false;
		}
		

		return true;
	}

	//finds a path from 1 tile to another tile, according to the unit's specifications
	bool findPath(Unit* unit, coords startLoc, coords dest, int maxRange = (worldsX * worldsY)) { //maxrange is usually enough to cover the entire map, but can be a given parameter to check if a destination is within a certain distance of the startlocation
		//startlocation and unit are 2 different parameters, so we can test possible moves that don't start from the unit's actual starting position

		std::vector<std::vector<coords>> possiblePaths(1, std::vector<coords>(1, {startLoc.x, startLoc.y}));
		int scannedAreas[worldsX][worldsY] = { 0 };

		scannedAreas[startLoc.x][startLoc.y] = 1;

		int it = 1;
		
		//how this works; the game checks for tiles with where scannedAreas isn't 0; if it finds one, it checks all neighbouring tiles where scannedAreas is 0.
		//doing this (additionally ignoring tiles where either units are or where walls are), the entire board is checked (at least until the target is found) and a path is formed
		//'it' is the amount of steps the unit has to take to reach any given position

		//for any residual confusion after my horrible explanation, I recommend this video; https://youtu.be/KiCBXu4P-2Y
		//It's a fairly standard algorithm and not all that hard either

		//The reason this function is a boolean function is so it can also be used as a method to check whether a unit can reach a certain destination on the map given their current leftover movement
		//so if the function finds a path the unit could realistically take, it returns true; aka the unit can actually move there

		pathToTake.resize(0);
		//possiblePaths is the array the actual path will be stored in; a 2D array of vectors, which stores a path on every X vector, and a set of coordinates along the Y vector
		//pathToTake is the end result of the algorithm; so the actual path that's found
		//to visualise;

		/* this is how the array looks, going from [0,1] to [2,4]
			[0,1] [0,2] [0,3] [0,4] [1,4] [2,4]
			[0,1] [1,1] [2,1] [2,2] [2,3] [2,4]
			...

			this extends for every possible path the unit could take
		*/

		if (startLoc.equals(dest)) {

			//when the destination is equal to the start location; 
			//first clears the pathToTake array, then adds the starting location to it
			//then returns true
			pathToTake.resize(0);

			pathToTake.push_back({startLoc.x, startLoc.y});

			return true;
		}
		
		do {
			int size = possiblePaths.size();

			for (int x = 0; x < worldsX; x++) {
				for (int y = 0; y < worldsY; y++) {  //checks entire map

					if (scannedAreas[x][y] == it && onMap({ x, y })) { //if the scannedareas equals 'it', so this iteration of this loop, and this tile is actually on the map, scan its neighbouring tiles
					
						for (int dir = 0; dir < 4; dir++) { //scans all tiles around the already scanned tiles

							if (dest.x == x + directions[dir].x && dest.y == y + directions[dir].y && onMap({ (x + directions[dir].x), (y + directions[dir].y) }) && canMoveHere(unit, (x + directions[dir].x), (y + directions[dir].y))) { //if a destination is found in these neighbouring tiles, stop the function and return true
								//if any of the neighbouring tiles are the destination, return the path and end the function

								pathToTake.resize(0);

								//sets the pathToTake array to the correct path
								for (int t = 0; t < size; t++) {
									if (possiblePaths[t].back().equals({ x, y })) {

										for (size_t t2 = 0; t2 < possiblePaths[t].size(); t2++) {

											pathToTake.push_back(possiblePaths[t][t2]); //fill pathToTake with the correct path
										}
									}
								}

								pathToTake.push_back( { x + directions[dir].x ,y + directions[dir].y } ); //adds the final destination to pathToTake
								possiblePaths.clear(); //clears the array
								return true; //end the function

							} else if (scannedAreas[x + directions[dir].x][y + directions[dir].y] == 0 && onMap({ (x + directions[dir].x), (y + directions[dir].y) }) && canMoveHere(unit, (x + directions[dir].x), (y + directions[dir].y))) {
								//else if the scanned neighbours aren't the destination; mark them as scanned in the scannedAreas array and add their coordinates to possiblePaths
								
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
		} while (it <= maxRange);
		possiblePaths.clear();
		return false;
	}
	
	//checks if unit at certain location is being threatened or not
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
						unit->y = (y);  //I had to move the unit itself instead of evaluating different coordinates -not that I didn't try, it just didn't work.

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

		return AIThreatenedPlaces[c.x][c.y]; //returns 1 if c is a threatened field, 0 if not
	}
	
	//finds all tiles a unit can move to in order to attack another unit
	void AIPossibleMoves(Unit* unit, coords target) {

		memset(AIPMoves, 0, sizeof(AIPMoves));
		
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

								if (level.getWorldField(x, y) == 0) {

									AIPMoves[x][y] = 1;
								}

								break;
							case unitMovementType::NAVAL:

								if (level.getWorldField(x, y) == 2) {

									AIPMoves[x][y] = 1;
								}

								break;
							case unitMovementType::AERIAL:

								AIPMoves[x][y] = 1;

								break;
							default:
								AIPMoves[x][y] = 0;
							}

							//above switch statement fills the AIPMoves array with all the fields the unit could possibly threaten with its attack and within the confines of his movement
						}
					}
				}
			}
		}

		unit->setCoords(xy);
	}
	

	
	std::vector<coords> destinations(friendlyUnitsAm);

	//sets the actual target for the AI
	coords getTarget(int uIndex) {

		//lambda expression which checks if a field is being attacked or not
		auto AIattack = [&](Unit* unit, coords selected, int x, int y) {
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
			else if (selected.x > unit->x&& selected.y == unit->y) {

				return unit->returnAttackArray(selected, x, y);
			}
			else return 0;
		};


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

		destinations.resize(friendlyUnitsAm);
		int am = friendlyUnitsAm;
		std::vector<int> priority(am);
		int distance = 20000;

		Unit target;
		target.set(Unit(-1, -1, 0, 0, unitType::MELEE, unitMovementType::GROUND));
		bool isBeingAttacked = false;

		//AIPossibleMoves();

		for (int t = 0; t < friendlyUnitsAm; t++) {
			if (level.fUnits[t].isAlive()) {

				//default priority of 0 for all units
				priority[t] = 0;

				if (!isBeingAttacked && getAttackMap(&level.eUnits[uIndex], level.fObjtv.getCoords())) { //checks if the unit isn't currently being attack by a friendly unit and if it can attack the allied objective; if both of these are true,
																										//set the target to the allied objective

					destinations[uIndex].set(level.fObjtv.x, level.fObjtv.y);
					return level.fObjtv.getCoords(); //immediately returns the objective as the target for the AI

				} else if ((AIattack(&level.fUnits[t], level.eObjtv.getCoords(), level.eObjtv.x, level.eObjtv.y) == 1 && getDist(level.fUnits[t].getCoords(), level.eUnits[uIndex].getCoords()) <= level.fUnits[t].getSpeed()) || getDist(level.fUnits[t].getCoords(), level.eObjtv.getCoords()) <= 3) {

					//checks if a friendly unit is in range of an enemy unit, and if said friendly unit is threatening the enemy unit. If it is; set enemy unit to target friendly unit
					priority[t] = 6;

				}
				else if (AIattack(&level.fUnits[t], level.eUnits[uIndex].getCoords(), level.eUnits[uIndex].x, level.eUnits[uIndex].y) == 1) { //checks if unit is being attacked

					priority[t] = 5;
					isBeingAttacked = true;

				}
			}
		}

		int dist = 20000;
		int temp = 0;
		bool allZero = true;

		//the next piece of code goes over every single unit and gets the one with the highest priority, assigned in the previous block of code,
		//and takes the highest priority one. If all units are zero, this block of code is basically skipped and the next block of code is initiated.

		for (int t = 0; t < friendlyUnitsAm; t++) {

			if (level.fUnits[t].isAlive()) {

				if (priority[t] != 0) { //test if all enemies have priority of 0
					allZero = false; 
				}

				if (priority[t] == temp) {

					if (getDist(level.eUnits[uIndex].getCoords(), level.fUnits[t].getCoords()) <= dist) {

						dist = getDist(level.eUnits[uIndex].getCoords(), level.fUnits[t].getCoords());

						target.set(level.fUnits[t]);
						temp = priority[t];
					}

				} else if (priority[t] > temp) {

					temp = priority[t];
					target.set(level.fUnits[t]);
				}
			}
		}
		
		//if all friendly units have a priority of 0; attack the closest one
		if (allZero) {

			int objDist = getDist(level.eUnits[uIndex].getCoords(), level.fObjtv.getCoords());
			coords tempCoords = level.fObjtv.getCoords();

			for (int t = 0; t < friendlyUnitsAm; t++) {
				if (level.fUnits[t].isAlive()) {
					if (objDist > getDist(level.eUnits[uIndex].getCoords(), level.fUnits[t].getCoords())) {

						objDist = getDist(level.eUnits[uIndex].getCoords(), level.fUnits[t].getCoords());
						tempCoords.set(level.fUnits[t].x, level.fUnits[t].y);
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
	
	void AI(int turnSt, int uIndex) { //no need to pass a pointer to eUnits; AI will only address enemy units anyways --a shame, otherwise we could have AI vs AI battles :(
	

		///////////////////////////////////////////////////////////////
		//
		// This function handles everything the AI does; it receives the parameters turnstate -to checks if it's the attacking or moving phase- and uIndex, to determine which enemy unit is being moved.
		// 
		// 1. Sets the distancemap for the acting unit
		// 2. Calls getTarget, to get a target for the unit. Duh ;)
		// 3. Fills the AIPMoves array with all possible moves the enemy unit could take to attack the target, and picks the closest one
		// 4. In the case that AIPMoves is empty, and the unit has no spaces to move to that are able to threaten its target; it looks for the tile that's closest to the target within its range.
	
		resetUnits();

		if (turnSt == 2) {

			setDistanceMap(level.eUnits[uIndex].getCoords());

			coords targetedUnit = getTarget(uIndex);
			
			int closestDistance = 20000;
			coords closestField;
			bool NoAIPMoves = true;

				//get target
				//get all tiles that can attack target
				//get closest tile to attack target with
				//find path to this closest tile

			//gets all possible moves
			AIPossibleMoves(&level.eUnits[uIndex], targetedUnit);
			closestField.set(targetedUnit.x, targetedUnit.y);

			//get closest tile in AIPMoves
			for (int x = 0; x < worldsX; x++) {
				for (int y = 0; y < worldsY; y++) {

					if (AIPMoves[x][y] == 1) {
						if (closestDistance >= getDist(level.eUnits[uIndex].getCoords(), { x, y })) {
							closestDistance = getDist(level.eUnits[uIndex].getCoords(), { x, y });

							closestField.set(x, y);
							NoAIPMoves = false;
						}
					}
				}
			}


			//if AIPMoves contains no possible tiles; find closest tile to target.
			if (NoAIPMoves) {
				closestDistance = 20000;

				for (int x = 0; x < worldsX; x++) {
					for (int y = 0; y < worldsY; y++) {
						if (closestDistance >= getDist(targetedUnit, { x, y }) && findPath(&level.eUnits[uIndex], level.eUnits[uIndex].getCoords(), {x, y}, level.eUnits[uIndex].getSpeed())) {
							
							closestDistance = getDist(targetedUnit, { x, y });

							closestField.set(x, y);
						}
					}
				}
			}

			
			//If the closest tile ISN'T the tile the unit is standing on; calculate the path to travel there using findPath
			//If it is; just do nothing! That means the unit is already in a good position.

			if (!level.eUnits[uIndex].getCoords().equals(closestField)) {
				//still something wrong here :( -not anymore! Ha!

				int cl = 20000;
				coords newCoords;
				if (!canMoveHere(&level.eUnits[uIndex], closestField.x, closestField.y)) {
					for (auto& dir : directions) {

						coords temp;
						temp.set(closestField.add(dir).x, closestField.add(dir).y);

						if (canMoveHere(&level.eUnits[uIndex], temp.x, temp.y) && getDist(temp, level.eUnits[uIndex].getCoords()) < cl) {

							cl = getDist(temp, level.eUnits[uIndex].getCoords());
							newCoords.set(temp.x, temp.y);
						}
					}

					closestField.set(newCoords.x, newCoords.y);
				}


				findPath(&level.eUnits[uIndex], level.eUnits[uIndex].getCoords(), closestField);
			}

			AIMoving = true;
			AItimer = 0;

		}
		else {

			//checks if the destination isn't on a diagonal; if it is, don't attack
			//does this by checking whether the absolute values of the 2 coordinates subtracted are equal; ex: [1, 1], which would be a diagonal.
			if (abs(destinations[uIndex].x - level.eUnits[uIndex].x) != abs(destinations[uIndex].y - level.eUnits[uIndex].y)) {
				level.eUnits[uIndex].attackState = 1;
				returnThreatArray(&level.eUnits[uIndex], destinations[uIndex], threatenedPlaces);
				attackUnits(level.eUnits, level.fUnits, destinations[uIndex], &level.fObjtv);
			}
		} 
	}

	void Game::Tick(float deltaTime)
	{
		
		//screen->Clear(0);
		levelBackground.Draw(screen, 0, 0);
		mousePos = getMousePosInt();

		if (inGame) {

			//draws the menu if the player is in game

			menuHeader.Draw(screen, 0, 0);
			button.SetFrame(goBackBtn.type);
			button.Draw(screen, goBackBtn.x, goBackBtn.y);

			turnState == 0 || turnState == 2 ? phaseInd.SetFrame(1) : phaseInd.SetFrame(0);
			
			phaseInd.Draw(screen, (ScreenWidth/2) - 251, 0);

			setUnitMap();

			if (turnState < 2) {
				setDistanceMap(selectedUnit);
			}

			//activates x times per second -- 60 fps / amount of frames per interval = interval per second

			if (AItimer % 60 == 0 && !AIMoving) {
				if (turnState >= 2) {

					resetUnits();

					if (AIstep < enemyUnitsAm) {

						if (level.eUnits[AIstep].isAlive()) {

							AI(turnState, AIstep); //essentially; when the turnstate is higher than 2, aka the enemy's turn; every second, call this function, and do this for every enemy unit (if the unit is alive) 
													//
						}
						else {
							AItimer += 55;
						}

						AIstep++; //AIStep is the iterator to select which unit is performing an action
					}
					else {
						if (turnState == 3) { //if the turnstate is equal to 3; (so if the next turnstate would be 0 again), perform all end of turn actions
							turnState = 0;

							//add end-of-turn events here!!


							//damages all units standing on lava tiles
							for (auto &Unit : level.fUnits) {
								if (level.getWorldField(Unit.x, Unit.y) == 4) {
									Unit.takeDamage(1);
								}	
							}

							for (auto& Unit : level.eUnits) {
								if (level.getWorldField(Unit.x, Unit.y) == 4) {
									Unit.takeDamage(1);
								}
							}
							
							//depending on which world the player is in; do different things
							switch (worldSelect) {
							case 1: 
								
								for (auto& Unit : level.fUnits) { //deal damage in the second world
									Unit.takeDamage(1);
								}

								for (auto& Unit : level.eUnits) {
									Unit.takeDamage(1);
								}
								
								break;
							case 3:
								
								for (auto& Unit : level.fUnits) { //heal when in the last world
									Unit.heal(1);
								}

								for (auto& Unit : level.eUnits) {
									Unit.heal(1);
								}
								
								break;
							}
							
						}
						else {
							turnState++;
						}
						AIstep = 0;
					}
				}
				AItimer = 0;
			}

			//the code below is what actually makes the units move
			if (AItimer % 10 == 0 && AIMoving) {
				if (turnState == 2) {

					//gets the shortest of either the pathToTake length or the unit's movement speed;
					//e.g.: when the unit has a movement speed of 5 but the path is only 3 long, this line of code takes 3, and vice versa
					size_t shortest = static_cast<int>(pathToTake.size()) < level.eUnits[AIstep - 1].getSpeed() + 1 ? static_cast<int>(pathToTake.size()) : level.eUnits[AIstep - 1].getSpeed() + 1;

					//for all stepts to be taken; move the unit to the next step
					if (moveStep < static_cast<int>(shortest)) {

						coords eDir = { pathToTake[moveStep].x - level.eUnits[AIstep - 1].x , pathToTake[moveStep].y - level.eUnits[AIstep - 1].y };

						//set the unit direction based on which direction it's going; this is for spriting purposes
						if (!(eDir.x == 0 && eDir.y == 0)) {
							if (eDir.x == 1 && eDir.y == 0) { level.eUnits[AIstep - 1].uDir = unitDirection::EAST; }
							if (eDir.x == 0 && eDir.y == 1) { level.eUnits[AIstep - 1].uDir = unitDirection::SOUTH; }
							if (eDir.x == -1 && eDir.y == 0) { level.eUnits[AIstep - 1].uDir = unitDirection::WEST; }
							if (eDir.x == 0 && eDir.y == -1) { level.eUnits[AIstep - 1].uDir = unitDirection::NORTH; }
						}
						
						
						level.eUnits[AIstep - 1].setCoords(pathToTake[moveStep]);
						

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
			unitInfo.Draw(screen, 845, 150);

			//sets the frame for the attack pattern thingy on the right side of the screen
			if (infoSelectedUnit.isAlive()) {
				switch (infoSelectedUnit.getUnitType()) {
				case unitType::RANGED: attackPattern.SetFrame(0);
					break;
				case unitType::MELEE: attackPattern.SetFrame(1);
					break;
				case unitType::MORTAR: attackPattern.SetFrame(2);
					break;
				}
			}
			else attackPattern.SetFrame(3);

			//draws the attack pattern thingy on the right side of the screen
			attackPattern.Draw(screen, 860, 270);

			//draws the environmental hazard info box on the right side of the screen, depending on what world the player is in
			environmentalHazardInfo.Draw(screen, 860, 180);
			switch (wrldID) {
			case 0: screen->Print("Environmental Hazard: None", 870, 190, 0xffffff);
				
				break;
			case 1: screen->Print("Environmental Hazard: Sandstorm", 870, 190, 0xffffff);
				screen->Print("Deadly sandstorms sweep by every turn,", 870, 220, 0xffffff);
				screen->Print("dealing 1 damage to ALL units.", 870, 230, 0xffffff);

				break;
			case 2: screen->Print("Environmental Hazard: Molten Shell", 870, 190, 0xffffff);
				screen->Print("Dangerous lava tiles cause damage when", 870, 220, 0xffffff);
				screen->Print("standing on them.", 870, 230, 0xffffff);

				break;
			case 3: screen->Print("Environmental Hazard: Gravity Flux", 870, 190, 0xffffff);
				screen->Print("The magical athmosphere of this planet", 870, 220, 0xffffff);
				screen->Print("heals every unit by 1 every turn.", 870, 230, 0xffffff);

				break;
			}

			screen->Print("Click on the units to move/attack!", 870, 470, 0xffffff);
			screen->Print("Kill all enemy units or the enemy", 870, 490, 0xffffff);
			screen->Print("objective to win.", 870, 500, 0xffffff);

			//if the turnstate is less than 2, aka it's the player's turn, draw the buttons to continue
			if (turnState < 2) {
				button.SetFrame(continueBtn.type);
				button.Draw(screen, continueBtn.x, continueBtn.y);
			}


			//calculates which tile the player has selected---------------------//
			currentCell = { mousePos.x / tileSize.x, mousePos.y / tileSize.y };

			offset = { mousePos.x % tileSize.x, mousePos.y % tileSize.y };

			selected = {
				(currentCell.y - Origin.y) + (currentCell.x - Origin.x),
				(currentCell.y - Origin.y) - (currentCell.x - Origin.x)
			};
			//-----------------------------------------------------------------//

			//see if cursor hovers right outside of tile
			//kinda disgusting with the +256, -256 but hey ho gets the job done

			//how this works is actually fairly clever; there is an image stored in the assets which is basically 4 colours on the outside of the tile.
			//these colours are on each of the 4 sides of the tile, and are different for each side.
			//depending on where the cursor is in reference to the selected tile, it also checks this special tile image, and depending on what colour it finds, it moves the selected tile
			//a simple, fairly dirty way to avoid doing actual maths
			//(I didn't come up with this)

			Pixel* col = colTile->GetBuffer() + offset.x + offset.y * tileSize.x;

			if (*col == -256) selected.x += 1;
			if (*col == -0xff0100) selected.y -= 1;
			if (*col == -(0x00ff00 + 256)) selected.x -= 1;
			if (*col == -(0xffff00 + 1)) selected.y += 1;

			//transform selected coords into screen px
			selectedWorldPx = toScreen(selected.x, selected.y);

			//generate map with loop
			for (int x = 0; x < worldsX; x++) {
				for (int y = 0; y < worldsY; y++) {

					coords vWorld = toScreen(x, y);

					//sets the frame for the gound tiles, setting the frame to '1' if the tile in worldField is 8, because 8 and 1 use the same tile sprite, the difference being that one has a mountain on top of it.
					if (level.getWorldField(x, y) != 8) {
						groundTiles.SetFrame(level.getWorldField(x, y));
					}
					else {
						groundTiles.SetFrame(1);
					}

					groundTiles.Draw(screen, vWorld.x, vWorld.y);

					//generates square indicators of where unit can move to
					for (int t = 0; t < friendlyUnitsAm; t++) {
						if (level.fUnits[t].moveState == 1) {
							//draws path unit has taken this turn
							for (int t1 = 0; t1 < level.fUnits[t].getSpeed(); t1++) {
								if (level.fUnits[t].getPastPlaces(t1).x == x && level.fUnits[t].getPastPlaces(t1).y == y && level.fUnits[t].moveState == 1) {
									//draw(1, 0, screen, tiles, vWorld.x, vWorld.y, tileSize.y + 32, tileSize.x * 3, tileSize.y + 32, tileSize.x);
									pathTile.DrawScaled(vWorld.x, vWorld.y, 64, 48, screen);
								}
							}

							//draws fields unit can directly move to
							if (distanceMap[x][y] <= 1 && level.fUnits[t].moveState == 1 && canMoveHere(&level.fUnits[t], x, y)) {

								selectTileS.DrawScaled(vWorld.x, vWorld.y, 64, 48, screen);
							}

							level.setUnitPlacement(x, y, 0);
						}
					}

					//draws selected tile
					if (selected.x == x && selected.y == y) {
						//draw(0, 0, screen, selectTile, selectedWorldPx.x, selectedWorldPx.y, tileSize.y, tileSize.x, tileSize.y, tileSize.x);
						if (level.getWorldField(x, y) != 5) {
							selectTileS.DrawScaled(selectedWorldPx.x, selectedWorldPx.y, 64, 48, screen);
						}
					}

					if (turnState != 2 && threatenedPlaces[x][y] == 1 && level.getWorldField(x, y) != 5) {
						selectTileS.DrawScaled(vWorld.x, vWorld.y, 64, 48, screen);
					}
				}
			}

			//goes over the entire map again to draw mountains on the correct tiles.
			for (int x = 0; x < worldsX; x++) {
				for (int y = 0; y < worldsY; y++) {
					if (level.getWorldField(x, y) == 1) {

						coords vWorld = toScreen(x, y);

						mountain.DrawScaled(vWorld.x, vWorld.y - 20, 64, 64, screen);
					}
					else if (level.getWorldField(x, y) == 6) {
						
						coords vWorld = toScreen(x, y);

						dmountain.DrawScaled(vWorld.x, vWorld.y - 20, 64, 64, screen);
					}
				}
			}

			//draws friendly units
			drawUnits(screen, level.fUnits, friendlyUnitsAm, true);

			//draws enemy units
			drawUnits(screen, level.eUnits, enemyUnitsAm, false);

			//draws objectives for both teams
			drawObjectives(level.fObjtv, screen, &antHill);
			drawObjectives(level.eObjtv, screen, &enAntHill);

			if (level.areMoving()) {
				button.SetFrame(8);
				button.Draw(screen, confirmBtn.x, confirmBtn.y);
			}

			//----------------------------------Draws Health Bars----------------------------------//

			for (auto &Unit : level.fUnits) {
				if (Unit.isAlive()) {
					healthBar.SetFrame(3 - Unit.getHealth());
					coords vWorld = toScreen(Unit.x, Unit.y);

					healthBar.Draw(screen, vWorld.x + 16, vWorld.y - 25);
				}
			}

			for (auto& Unit : level.eUnits) {
				if (Unit.isAlive()) {
					healthBar.SetFrame(3 - Unit.getHealth());

					coords vWorld = toScreen(Unit.x, Unit.y);

					healthBar.Draw(screen, vWorld.x + 16, vWorld.y - 25);
				}
			}

			if (level.fObjtv.isAlive()) {
				objHealthBar.SetFrame(level.fObjtv.getHealth());

				coords vWorld = toScreen(level.fObjtv.x, level.fObjtv.y);

				objHealthBar.Draw(screen, vWorld.x, vWorld.y - 30);
			}

			if (level.eObjtv.isAlive()) {
				objHealthBar.SetFrame(level.eObjtv.getHealth());

				coords vWorld = toScreen(level.eObjtv.x, level.eObjtv.y);

				objHealthBar.Draw(screen, vWorld.x, vWorld.y - 30);
			}

			//-------------------------------------------------------------------------------------//
		}
		else {

			//draws the menu
			if (worldSelect == 100) {

				planetSelect.Draw(screen, 0, 0);

				menuHeader.Draw(screen, 0, 0);

				button.SetFrame(9);
				button.Draw(screen, goBackBtn.x, goBackBtn.y);

				planetSelectTxt.Draw(screen, 0, 550);
			}
			else {

				switch (worldSelect) {
				case 0: planet0.Draw(screen, 0, 0);
					break; 
				case 1: planet1.Draw(screen, 0, 0);
					break; 
				case 2: planet2.Draw(screen, 0, 0);
					break;
				case 3: planet3.Draw(screen, 0, 0);
					break;
				}
				
				menuHeader.Draw(screen, 0, 0);
				button.SetFrame(goBackBtn.type);
				button.Draw(screen, goBackBtn.x, goBackBtn.y);
				
				for (int t = 0; t < sizeof(levelSelectBtns[worldSelect]) / sizeof(levelSelectBtns[worldSelect][0]); t++) {

					if (lvState[worldSelect][t] == levelState::INACCESSIBLE) {
						button.SetFrame(10);
					}
					else {
						button.SetFrame(levelSelectBtns[worldSelect][t].type);
					}
					button.Draw(screen, levelSelectBtns[worldSelect][t].x, levelSelectBtns[worldSelect][t].y);
				}
			}
		}
	}
};