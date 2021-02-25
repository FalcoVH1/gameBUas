#include "game.h"
#include "surface.h"
#include "template.h"
#include <cmath>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h> 
#include <iostream>
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

	Surface* tiles = new Surface("assets/defTiles.png");
	Surface* colTile = new Surface("assets/specialTile.png");

	int AItimer = 0, AIstep = 0, turnState = 0, lvID = 0, moveStep = 0;

	coords mousePos(0, 0), currentCell(0, 0), offset(0, 0), selected(0, 0), selectedUnit(0, 0), selectedWorldPx(0, 0), tileSize(64, 32);
	coords directions[4] = {
		coords(0, 1),
		coords(0, -1),
		coords(1, 0),
		coords(-1, 0),
	};

	int distanceMap[9][9] = { 0 }; //map of the distance any object is from a selected unit
	int threatenedPlaces[9][9] = { 0 }; //map of all places on the map which are threatened
	int AIPMoves[9][9] = { 0 }; //places AI can move to
	int scannedAreas[9][9] = { 0 };

	std::vector<coords> pathToTake(0);

	int worldField[10][9][9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 0, 0, 0, 0,
		0, 0, 2, 2, 2, 2, 2, 0, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,
		0, 2, 2, 2, 2, 2, 2, 2, 0,

								 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 0, 0, 0, 0, 0, 0, 0,

								 2, 2, 2, 2, 1, 1, 2, 2, 2,
								 2, 2, 2, 2, 1, 1, 2, 2, 2,
								 2, 2, 2, 2, 1, 1, 2, 2, 2,
								 2, 2, 2, 2, 1, 1, 2, 2, 2,
								 2, 2, 1, 1, 1, 1, 1, 2, 2,
								 2, 2, 2, 2, 1, 1, 2, 2, 2,
								 2, 2, 2, 2, 2, 1, 2, 2, 2,
								 2, 2, 2, 2, 2, 1, 2, 2, 2,
								 2, 2, 2, 2, 2, 2, 2, 2, 2, }; //terrain map

	Unit fUnits[10][3] = { Unit(1, 7, 3, 15, 5, unitType::RANGED, unitMovementType::AERIAL), Unit(4, 7, 3, 15, 1, unitType::RANGED, unitMovementType::GROUND), Unit(7, 7, 3, 15, 2, unitType::MORTAR, unitMovementType::NAVAL) };

	Unit eUnits[10][3] = { Unit(1, 1, 3, 3, 3, unitType::MELEE, unitMovementType::GROUND), Unit(4, 1, 3, 3, 1, unitType::MELEE, unitMovementType::GROUND), Unit(7, 2, 3, 3, 2, unitType::MORTAR, unitMovementType::NAVAL) };

	gameState level[10] = {
		gameState({9, 9}, {8, 4} ,{4, 0, 7}, {4, 8, 7}, &worldField[0][0][0], fUnits[0], eUnits[0], sizeof(fUnits[0]) / sizeof(fUnits[0][0]), sizeof(fUnits[0]) / sizeof(fUnits[0][0])),
		gameState({9, 9}, {8, 4} ,{4, 0, 7}, {4, 8, 7},&worldField[1][0][0], fUnits[0], eUnits[0], sizeof(fUnits[0]) / sizeof(fUnits[0][0]), sizeof(fUnits[0]) / sizeof(fUnits[0][0])),
		gameState({9, 9}, {8, 4} ,{4, 0, 7}, {4, 8, 7},& worldField[2][0][0], fUnits[0], eUnits[0], sizeof(fUnits[0]) / sizeof(fUnits[0][0]), sizeof(fUnits[0]) / sizeof(fUnits[0][0]))
	};

	gameState levelBackup;

	void setDistanceMap(coords c1);
	bool onMap(coords c);

	void moveUnits(Unit* fUnits, Unit* eUnits, coords selectedC, int fUnitsAm, int eUnitsAm) {

		if (level[lvID].getCanMove()) {

			//resets attacking before move phase
			memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
			for (int i = 0; i < fUnitsAm; i++) {
				fUnits[i].setAttackState(0);
			};

			for (int i = 0; i < eUnitsAm; i++) {
				eUnits[i].setAttackState(0);
			};

			for (int t = 0; t < fUnitsAm; t++) {

				//if units haven't moved, transition to them moving

				if (fUnits[t].getMoveState() == 0) {
					if (selectedC.equals(fUnits[t].getCoords())) {
						fUnits[t].setMoveState(1);
					}
				}
				else if (fUnits[t].getMoveState() == 1) {

					//first checks if the move is eligible, then sets a new destination for the unit to move to. if the move is ineligible, the unit is set back to movestate 0, aka "hasn't moved".

					bool targetOccupied = false;
					for (int t = 0; t < fUnitsAm; t++) {
						if (selectedC.equals(fUnits[t].getCoords())) {
							targetOccupied = true;
						}
					}
					for (int t = 0; t < eUnitsAm; t++) {
						if (selectedC.equals(eUnits[t].getCoords())) {
							targetOccupied = true;
						}
					}

					//currently checks how far fields are away from unit, but this might have to change to accomodate for terrain, etc.

					if (onMap(selectedC) && distanceMap[selectedC.x][selectedC.y] <= 1.0f && !targetOccupied) {

						//-1 to getSpeed because the last square of movement gets done in the else- part of this statement
						if (fUnits[t].getMoveCycle() <= fUnits[t].getSpeed() - 1) {
							fUnits[t].move(selectedC);
						}
						else {
							fUnits[t].move(selectedC);
							fUnits[t].setMoveState(2);
						}
					}
					else {
						fUnits[t].setMoveState(0);
					}
				}
			}
		}
	}

	void attackUnits(Unit* fUnits, Unit* eUnits, coords selectedC, int fUnitsAm, int eUnitsAm, Objective* Obj) {

		//resets movement before attack phase
		for (int i = 0; i < fUnitsAm; i++) {
			fUnits[i].setMoveState(0);
			fUnits[i].setMoveCycle(0);
		};

		for (int i = 0; i < eUnitsAm; i++) {
			eUnits[i].setMoveState(0);
			eUnits[i].setMoveCycle(0);
		};

		for (int t = 0; t < fUnitsAm; t++) {
			if (fUnits[t].getAttackState() == 0) {
				//starts target selection
				if (selectedC.equals(fUnits[t].getCoords())) {
					fUnits[t].setAttackState(1);
				}
			}
			else if (fUnits[t].getAttackState() == 1) {

				//resets attack when clicking other friendly unit

				fUnits[t].setAttackState(3);
				for (int t1 = 0; t1 < fUnitsAm; t1++) {
					if (selectedC.equals(fUnits[t1].getCoords())) {
						fUnits[t].setAttackState(0);
					}
				}

				//execute attack by damaging all enemy units in attack range
				for (int t1 = 0; t1 < eUnitsAm; t1++) {
					if (threatenedPlaces[eUnits[t1].getX()][eUnits[t1].getY()] == 1) {
						eUnits[t1].takeDamage(fUnits[t].getDamage());
					}
				}
				if (threatenedPlaces[Obj->getX()][Obj->getY()] == 1) {
					Obj->takeDamage(fUnits[t].getDamage());
				}
				memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
			}
		}
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
	void returnThreatArray(Unit* Unit, coords s, int arr[9][9]) {
		for (int i = 0; i < level[lvID].getWorld().x; i++) {
			for (int j = 0; j < level[lvID].getWorld().y; j++) {
				//wonky code, kinda; essentially, returns a value of 0 if the field given as parameters is outside of attack range of unit, returns 1 if it's inside.
				//could be optimized with an array of coordinates of a variable size

				//ended up not being as wonky as I expected it to be :)
				threatenedPlaces[i][j] = Unit->returnAttackArray(s, i, j);
			}
		}
	};

	//converts ingame coordinates to on screen coordinates
	coords toScreen(int x, int y) {
		return coords{
				(level[lvID].getOrigin().x * tileSize.x) + (x - y) * (tileSize.x / 2),
				(level[lvID].getOrigin().y * tileSize.y) + (x + y) * (tileSize.y / 2)
		};
	}

	//draws units
	void drawUnits(Surface* screen, Unit* fUnits, int fUnitsAm, int X, int Y) {
		for (int t = 0; t < fUnitsAm; t++) {

			//kills land units if they somehow arrive on water
			if (fUnits[t].getMvType() == unitMovementType::GROUND && level[lvID].getWorldField(fUnits[t].getX(), fUnits[t].getY()) == 2) {
				fUnits[t].takeDamage(fUnits[t].getHealth());
			}

			//kills naval units if they somehow arrive on land
			if (fUnits[t].getMvType() == unitMovementType::NAVAL && level[lvID].getWorldField(fUnits[t].getX(), fUnits[t].getY()) != 2) {
				fUnits[t].takeDamage(fUnits[t].getHealth());
			}

			if (fUnits[t].getHealth() > 0 && fUnits[t].getX() == X && fUnits[t].getY() == Y) {

				coords temp = toScreen(fUnits[t].getX(), fUnits[t].getY());

				if (fUnits[t].getHealth() > 0) {
					draw(1, 1, screen, tiles, temp.x, temp.y, 64, 128, tileSize.y, tileSize.x);
				}


				//gets the center of the movable-unit-circle thingy
				if (fUnits[t].getMoveState() == 1) {
					selectedUnit.set(fUnits[t].getCoords().x, fUnits[t].getCoords().y);
				}

				if (fUnits[t].getAttackState() == 1) {

					level[lvID].setAreAttacking(true);

					//draws cancel attack button
					screen->Bar(270, 450, 330, 500, 0x0000ff);

					//gets threatened fields for attacking units
					if (selected.x == fUnits[t].getX() && selected.y < fUnits[t].getY()) {
						memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
						fUnits[t].setDir(unitDirection::SOUTH);
						returnThreatArray(&fUnits[t], selected, threatenedPlaces);
					}
					else if (selected.x == fUnits[t].getX() && selected.y > fUnits[t].getY()) {
						memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
						fUnits[t].setDir(unitDirection::NORTH);
						returnThreatArray(&fUnits[t], selected, threatenedPlaces);
					}
					else if (selected.x < fUnits[t].getX() && selected.y == fUnits[t].getY()) {
						memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
						fUnits[t].setDir(unitDirection::WEST);
						returnThreatArray(&fUnits[t], selected, threatenedPlaces);
					}
					else if (selected.x > fUnits[t].getX() && selected.y == fUnits[t].getY()) {
						memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
						fUnits[t].setDir(unitDirection::EAST);
						returnThreatArray(&fUnits[t], selected, threatenedPlaces);
					}
				}
			}
		}
	}

	void drawObjectives(Objective fObj, Surface* screen, int X, int Y) {
		if (fObj.getX() == X && fObj.getY() == Y) {
			if (fObj.getHealth() > 0) {

				coords temp = toScreen(fObj.getX(), fObj.getY());

				draw(1, 1, screen, tiles, temp.x, temp.y, 64, 128, tileSize.y, tileSize.x);
			}
		}
	}

	void resetUnits() {
		memset(AIPMoves, 0, sizeof(AIPMoves));
		memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
		//pathToTake.resize(0);

		for (int i = 0; i < level[lvID].getFUnitsAm(); i++) {
			level[lvID].fUnits[i].setAttackState(0);

			level[lvID].fUnits[i].setMoveState(0);
			level[lvID].fUnits[i].setMoveCycle(0);

			level[lvID].fUnits[i].clearPastPlaces();
		};

		for (int i = 0; i < level[lvID].getEUnitsAm(); i++) {
			level[lvID].eUnits[i].setAttackState(0);

			level[lvID].eUnits[i].setMoveState(0);
			level[lvID].eUnits[i].setMoveCycle(0);
		};
	}

	void Game::MouseDown(int button) {

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
		if (mouseLocated(270, 450, 330, 500) && level[lvID].getAreAttacking() == true) {
			memset(threatenedPlaces, 0, sizeof(threatenedPlaces));
			for (int t = 0; t < level[lvID].getFUnitsAm(); t++) {
				level[lvID].fUnits[t].setAttackState(0);
			}
		}

		//--------------------------------------------------------------------------------------//



		switch (turnState) {
		case 0:

			//move friendly units
			moveUnits(level[lvID].fUnits, level[lvID].eUnits, selected, level[lvID].getFUnitsAm(), level[lvID].getEUnitsAm());

			break;
		case 1:
			Objective * eObjtvPointer;
			eObjtvPointer = &level[lvID].eObjtv;

			//attack with friendly units
			attackUnits(level[lvID].fUnits, level[lvID].eUnits, selected, level[lvID].getFUnitsAm(), level[lvID].getEUnitsAm(), eObjtvPointer);

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
	};

	void Game::KeyDown(int key) {

		resetUnits();

		switch (key) {
		case 80: if (lvID - 1 >= 0) {
			level[lvID].set(levelBackup);
			lvID--;
		}
			   break;
		case 79: if (lvID + 1 < sizeof(level) / sizeof(level[0])) {
			level[lvID].set(levelBackup);
			lvID++;
		}
			   break;

		}

		//resets both attack and movement for all units
		resetUnits();
		memset(AIPMoves, 0, sizeof(AIPMoves));
		turnState = 0;
		pathToTake.resize(0);

		levelBackup.set(level[lvID]);
	}

	void Game::Init() {
		levelBackup.set(level[lvID]);
	}


	void Game::Shutdown() {
		delete tiles;
		delete colTile;
	}

	bool onMap(coords c) {
		//checks if given coordinates are on teh map
		return (c.x < level[lvID].getWorld().x && c.x >= 0 && c.y < level[lvID].getWorld().y && c.y >= 0);
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

		for (int x = 0; x < level[lvID].getWorld().x; x++) {
			for (int y = 0; y < level[lvID].getWorld().y; y++) {
				distanceMap[x][y] = 20000; //couldn't use memset for some reason :(
			}
		}

		distanceMap[c1.x][c1.y] = 0;
		
		//loops over the entire map, giving each field a value of distance.
		//how this is done is as follows; the entire map is set to 20000 initially, but the coordinate given as a parameter is set to 0.
		//it then loops, using 'it' to check the value, checking if it finds any values equal to 'it'
		//if it does, it sets all neighbouring tiles to 'it + 1'
		//doing this, the next time it loops, those are the tiles that are selected, and the process repeats itself until a complete map of all the units on the field is formed.

		for (int it = 0; it < (level[lvID].getWorld().x + level[lvID].getWorld().y) ; it++) {
			for (int x = 0; x < level[lvID].getWorld().x; x++) {
				for (int y = 0; y < level[lvID].getWorld().y; y++) {

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

		if (selected.x == unit->getX() && selected.y < unit->getY()) {

			return unit->returnAttackArray(selected, x, y);
		}
		else if (selected.x == unit->getX() && selected.y > unit->getY()) {

			return unit->returnAttackArray(selected, x, y);
		}
		else if (selected.x < unit->getX() && selected.y == unit->getY()) {

			return unit->returnAttackArray(selected, x, y);
		}
		else if (selected.x > unit->getX() && selected.y == unit->getY()) {

			return unit->returnAttackArray(selected, x, y);
		} else return 0;
	}


	bool canMoveHere(Unit* unit, int x, int y) {
	
		//checks if the unit can move to the given coords

		
		switch (unit->getMvType()) {
		case unitMovementType::GROUND:

			if (level[lvID].getWorldField(x, y) == 0 && level[lvID].getUnitPlacement(x, y) == 0) {

				return true;
			}
			else return false;

			break;
		case unitMovementType::NAVAL:

			if (level[lvID].getWorldField(x, y) == 2 && level[lvID].getUnitPlacement(x, y) == 0) {

				return true;
			}
			else return false;

			break;
		case unitMovementType::AERIAL:

			if (level[lvID].getUnitPlacement(x, y) == 0) {

				return true;
			}
			else return false;

			break;

		default:
			return false;
		}
		

		return true;
	}


	bool findPath(Unit* unit, coords startLoc, coords dest, int maxMoves = (level[lvID].getWorld().x * level[lvID].getWorld().y)) {

		std::vector<std::vector<coords>> possiblePaths(1, std::vector<coords>(1, {startLoc.x, startLoc.y}));
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

			for (int x = 0; x < level[lvID].getWorld().x; x++) {
				for (int y = 0; y < level[lvID].getWorld().y; y++) {

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

		int AIThreatenedPlaces[9][9] = { 0 }; //map of all places threatened by AI

		//creates a map of all the spots on the map the unit could possibly attack 

		coords xy = unit->getCoords(); //saves unit's location

		//checks all 4 directions
		for (int dir = 0; dir < 4; dir++) {

			//for every spot on the map
			for (int x = 0; x < level[lvID].getWorld().x; x++) {
				for (int y = 0; y < level[lvID].getWorld().y; y++) {

					//max movement range gets checked
					if (findPath(unit, xy, { x, y }, unit->getSpeed())) {

						unit->setX(x);  //sets unit location to every square on the field
						unit->setY(y);  //I had to move the unit itself instead of evaluating different coordinates -not that I didn't try, it just didn't work. Too bad!

						//gets threat array for that current location in every direction
						returnThreatArray(unit, unit->getCoords().add(directions[dir]), threatenedPlaces);

						for (int x1 = 0; x1 < level[lvID].getWorld().x; x1++) {
							for (int y1 = 0; y1 < level[lvID].getWorld().y; y1++) {
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
		for (int x = 0; x < level[lvID].getWorld().x; x++) {
			for (int y = 0; y < level[lvID].getWorld().y; y++) {

				//max movement range gets checked
				if (findPath(unit, xy, { x, y }, unit->getSpeed())) {

					//checks all 4 directions
					for (int dir = 0; dir < 4; dir++) {
						unit->setX(x);  //sets unit location to every square on the field
						unit->setY(y);  //I had to move the unit itself instead of evaluating different coordinates -not that I didn't try, it just didn't work. Too bad!

						//gets threat array for that current location in every direction
						returnThreatArray(unit, unit->getCoords().add(directions[dir]), threatenedPlaces);


						//if the unit can actually move to that place, and the target is being threatened in this spot, add the spot to the AIPMoves array;
						//this way AIPmoves is an array of all the tiles that are threatening the friendly units, that the unit can actually move to
						if (threatenedPlaces[target.x][target.y] == 1) {

							switch (unit->getMvType()) {
							case unitMovementType::GROUND:

								if (level[lvID].getWorldField(x, y) == 0) {

									AIPMoves[x][y] = 1;
								}

								break;
							case unitMovementType::NAVAL:

								if (level[lvID].getWorldField(x, y) == 2) {

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

		int am = level[lvID].getFUnitsAm();
		std::vector<int> priority(am);
		int distance = 20000;

		Unit target;
		target.set(Unit(-1, -1, 0, 0, 0, unitType::MELEE, unitMovementType::GROUND));
		bool isBeingAttacked = false;

		//AIPossibleMoves();

		for (int t = 0; t < level[lvID].getFUnitsAm(); t++) {
			if (level[lvID].fUnits[t].getHealth() > 0) {


				priority[t] = 0;
				//getDist(level[lvID].fUnits[t].getCoords(), level[lvID].eObjtv.getCoords()) <=
				if ((AIattack(&level[lvID].fUnits[t], level[lvID].eObjtv.getCoords(), level[lvID].eObjtv.getX(), level[lvID].eObjtv.getY()) == 1 && getDist(level[lvID].fUnits[t].getCoords(), level[lvID].eUnits[uIndex].getCoords()) <= level[lvID].fUnits[t].getSpeed()) || getDist(level[lvID].fUnits[t].getCoords(), level[lvID].eObjtv.getCoords()) <= 3) {

					priority[t] = 6;

				}
				else if (AIattack(&level[lvID].fUnits[t], level[lvID].eUnits[uIndex].getCoords(), level[lvID].eUnits[uIndex].getX(), level[lvID].eUnits[uIndex].getY()) == 1) { //checks if unit is being attacked

					priority[t] = 5;
					isBeingAttacked = true;

				}
				else if (!isBeingAttacked && getAttackMap(&level[lvID].eUnits[uIndex], level[lvID].fObjtv.getCoords())) {

					//printf("%i\n", level[lvID].fObjtv.getX());
					//printf("%i\n", level[lvID].fObjtv.getY());
					return level[lvID].fObjtv.getCoords();

				}
			}
		}

		int dist = 20000;
		int temp = 0;
		bool allZero = true;

		

		for (int t = 0; t < level[lvID].getFUnitsAm(); t++) {

			if (level[lvID].fUnits[t].getHealth() > 0) {

				if (priority[t] != 0) { //test if all enemies have priority of 0
					allZero = false; 
				}

				if (priority[t] == temp) {

					if (getDist(level[lvID].eUnits[uIndex].getCoords(), level[lvID].fUnits[t].getCoords()) <= dist) {

						dist = getDist(level[lvID].eUnits[uIndex].getCoords(), level[lvID].fUnits[t].getCoords());

						target.set(level[lvID].fUnits[t]);
						temp = priority[t];
					}

				} else if (priority[t] > temp) {

					temp = priority[t];
					target.set(level[lvID].fUnits[t]);
				}
			}
		}
		
		if (allZero) {

			int objDist = getDist(level[lvID].eUnits[uIndex].getCoords(), level[lvID].fObjtv.getCoords());
			coords tempCoords = level[lvID].fObjtv.getCoords();

			for (int t = 0; t < level[lvID].getFUnitsAm(); t++) {
				if (level[lvID].fUnits[t].getHealth() > 0) {
					if (objDist > getDist(level[lvID].eUnits[uIndex].getCoords(), level[lvID].fUnits[t].getCoords())) {

						objDist = getDist(level[lvID].eUnits[uIndex].getCoords(), level[lvID].fUnits[t].getCoords());
						tempCoords.set(level[lvID].fUnits[t].getX(), level[lvID].fUnits[t].getY());
					}
				}
			}

			return tempCoords;
		}
		
		return target.getCoords();
	}

	std::vector<coords> destinations(level[lvID].getFUnitsAm());
	bool AIMoving = false;

	void AIstsep(int turnSt, int uIndex) { //no need to pass a pointer to eUnits; AI will only address enemy units anyways --a shame, otherwise we could have AI vs AI battles :(
	
		resetUnits();

		if (turnSt == 2) {

			printf("Movement phase\n");

			setDistanceMap(level[lvID].eUnits[uIndex].getCoords());
			

			coords loc;
			loc.set(getTarget(uIndex).x, getTarget(uIndex).y);

			int closestDistance = 20000;
			coords closestField;

				//get target
				//get all tiles that can attack target
				//get closest tile to attack target with
				//find path to this closest tile

			AIPossibleMoves(&level[lvID].eUnits[uIndex], loc);
			closestField.set(loc.x, loc.y);

			for (int x = 0; x < level[lvID].getWorld().x; x++) {
				for (int y = 0; y < level[lvID].getWorld().y; y++) {

					if (AIPMoves[x][y] == 1) {
						if (closestDistance >= getDist(level[lvID].eUnits[uIndex].getCoords(), { x, y })) {
							closestDistance = getDist(level[lvID].eUnits[uIndex].getCoords(), { x, y });

							closestField.set(x, y);
						}
					}
				}
			}

			printf("u X: %i\n", level[lvID].eUnits[uIndex].getX());
			printf("u Y: %i\n", level[lvID].eUnits[uIndex].getY());

			printf("cl X: %i\n", closestField.x);
			printf("cl Y: %i\n", closestField.y);


			//fix one last problem here;
			//if units out of range, focus them


			//findPath(&level[lvID].eUnits[uIndex], level[lvID].eUnits[uIndex].getCoords(), closestField);
			findPath(&level[lvID].eUnits[uIndex], level[lvID].eUnits[uIndex].getCoords(), closestField);

			for (int t = 0; t < pathToTake.size(); t++) {
				printf("----------------------\n");
				printf("X: %i\n", pathToTake[t].x);
				printf("Y: %i\n", pathToTake[t].y);
			}

			AIMoving = true;
			AItimer = 0;
		}
		else {
			printf("Cringe phase\n");
		
		} 
	}


	void Game::Tick(float deltaTime)
	{
		screen->Clear(0x59d6ce);

		int fUnitsAm = 0, eUnitsAm = 0;
		//marks where the units are on the field
		for (int i = 0; i < level[lvID].getFUnitsAm(); i++) {
			level[lvID].setUnitPlacement(level[lvID].fUnits[i].getX(), level[lvID].fUnits[i].getY(), 1);
		}

		for (int i = 0; i < level[lvID].getEUnitsAm(); i++) {
			level[lvID].setUnitPlacement(level[lvID].eUnits[i].getX(), level[lvID].eUnits[i].getY(), 1);
		}

		level[lvID].setUnitPlacement(level[lvID].eObjtv.getX(), level[lvID].eObjtv.getY(), 1);
		level[lvID].setUnitPlacement(level[lvID].fObjtv.getX(), level[lvID].fObjtv.getY(), 1);

		if (turnState < 2) {
			setDistanceMap(selectedUnit);
		}

		//printf("TurnState: %i\n", turnState);

		//activates x times per second -- 60 fps / amount of frames per interval = interval per second
		
		if (AItimer % 60 == 0 && !AIMoving) {
			if (turnState >= 2) {

				if (AIstep < level[lvID].getEUnitsAm()) {

					if (level[lvID].eUnits[AIstep].getHealth() > 0) {
						

						AIstsep(turnState, AIstep);
					}

					AIstep++;
				}
				else {
					if (turnState + 1 == 4) {
						turnState = 0;
						AIstep = 0;
					}
					else {
						turnState++;
						AIstep = 0;
					}
				}
			}
			AItimer = 0;
		}

		if (AItimer % 20 == 0 && AIMoving) {
			if (turnState == 2) {
				if (moveStep < pathToTake.size()) {

					//printf("%s\n", AIMoving ? "True" : "False");
					
					level[lvID].eUnits[AIstep - 1].setCoords(pathToTake[moveStep]);

					moveStep++;
				}
				else {

					AIMoving = false;
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

		mousePos = getMousePosInt();

		currentCell = { mousePos.x / tileSize.x, mousePos.y / tileSize.y };

		offset = { mousePos.x % tileSize.x, mousePos.y % tileSize.y };

		selected = {
			(currentCell.y - level[lvID].getOrigin().y) + (currentCell.x - level[lvID].getOrigin().x),
			(currentCell.y - level[lvID].getOrigin().y) - (currentCell.x - level[lvID].getOrigin().x)
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

		//generate map with loop
		for (int x = 0; x < level[lvID].getWorld().x; x++) {
			for (int y = 0; y < level[lvID].getWorld().y; y++) {

				coords vWorld = toScreen(x, y);

				//draws tiles
				switch (level[lvID].getWorldField(x, y)) {
				case 0: draw(0, 0, screen, tiles, vWorld.x, vWorld.y, 64, 128, tileSize.y, tileSize.x);
					break;
				case 1: draw(1, 1, screen, tiles, vWorld.x, vWorld.y, 64, 128, tileSize.y, tileSize.x);
					break;
				case 2: draw(0, 1, screen, tiles, vWorld.x, vWorld.y, 64, 128, tileSize.y, tileSize.x);
					break;
				}

				//draws friendly units
				drawUnits(screen, level[lvID].fUnits, level[lvID].getFUnitsAm(), x, y);  //this function could probably be optimised; let's save it for later ;-)

				//draws enemy units
				drawUnits(screen, level[lvID].eUnits, level[lvID].getEUnitsAm(), x, y);

				//draws objectives for both teams
				drawObjectives(level[lvID].fObjtv, screen, x, y);
				drawObjectives(level[lvID].eObjtv, screen, x, y);

				//draws selected tile
				if (selected.x == x && selected.y == y) {
					draw(1, 0, screen, tiles, selectedWorldPx.x, selectedWorldPx.y, 64, 128, tileSize.y, tileSize.x);
				}

				if (turnState != 2 && threatenedPlaces[x][y] == 1) {
					draw(1, 0, screen, tiles, vWorld.x, vWorld.y, 64, 128, tileSize.y, tileSize.x);
				}

				/*
				if (AIPMoves[x][y] == 1) {
					draw(1, 0, screen, tiles, vWorld.x, vWorld.y, 64, 128, tileSize.y, tileSize.x);
				}
				*/
				/*
				for (int t = 0; t < pathToTake.size(); t++) {
					if (pathToTake[t].equals({x, y})) {
						draw(1, 0, screen, tiles, vWorld.x, vWorld.y, 64, 128, tileSize.y, tileSize.x);
					}
				}
				*/

				//generates square indicators of where unit can move to
				for (int t = 0; t < level[lvID].getFUnitsAm(); t++) {
					if (level[lvID].fUnits[t].getMoveState() == 1) {
						//draws path unit has taken this turn
						for (int t1 = 0; t1 < level[lvID].fUnits[t].getSpeed(); t1++) {
							if (level[lvID].fUnits[t].getPastPlaces(t1).x == x && level[lvID].fUnits[t].getPastPlaces(t1).y == y && level[lvID].fUnits[t].getMoveState() == 1) {
								draw(0, 1, screen, tiles, vWorld.x, vWorld.y, 64, 128, tileSize.y, tileSize.x);
							}
						}

						//draws fields unit can directly move to
						if (distanceMap[x][y] <= 1 && level[lvID].fUnits[t].getMoveState() == 1 && canMoveHere(&level[lvID].fUnits[t], x, y)) {

							draw(1, 0, screen, tiles, vWorld.x, vWorld.y, 64, 128, tileSize.y, tileSize.x);
						}

						level[lvID].setUnitPlacement(x, y, 0);
					}
				}

				for (int t = 0; t < level[lvID].getEUnitsAm(); t++) {
					if (distanceMap[x][y] == 1 && level[lvID].eUnits[t].getMoveState() == 1) {

						draw(1, 0, screen, tiles, vWorld.x, vWorld.y, 64, 128, tileSize.y, tileSize.x);
					}
					level[lvID].setUnitPlacement(x, y, 0);
				}
			}
		}

		level[lvID].setCanMove(false);
		for (int i = 0; i < level[lvID].getFUnitsAm(); i++) {
			if (!level[lvID].fUnits[i].getIsMoving()) {
				level[lvID].setCanMove(true);
			}
		};

		for (int i = 0; i < level[lvID].getEUnitsAm(); i++) {
			if (!level[lvID].eUnits[i].getIsMoving()) {
				level[lvID].setCanMove(true);
			}
		};
	}
};