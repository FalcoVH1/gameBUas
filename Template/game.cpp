#include "game.h"
#include "surface.h"
#include "template.h"
#include <cmath>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <math.h> 
#include <iostream>

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

	int AItimer = 0, turnState = 0, lvID = 0;

	coords mousePos(0, 0), currentCell(0, 0), offset(0, 0), selected(0, 0), selectedUnit(0, 0), selectedWorldPx(0, 0), tileSize(64, 32);


	int worldField[10][9][9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 0, 0, 1, 0, 0, 0, 0,
								 0, 0, 0, 0, 1, 0, 0, 0, 0,
								 0, 0, 0, 0, 1, 0, 0, 0, 0,
								 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 0, 0, 1, 0, 0, 0, 0,
								 0, 0, 2, 2, 2, 2, 2, 0, 0,
								 0, 2, 2, 2, 2, 2, 2, 2, 0,
								 0, 2, 2, 2, 2, 2, 2, 2, 0,
	
								 1, 1, 1, 1, 1, 1, 1, 1, 1,
								 0, 0, 0, 1, 1, 1, 0, 0, 0,
								 0, 0, 0, 0, 1, 1, 0, 0, 0,
								 0, 0, 0, 0, 1, 1, 0, 0, 0,
								 0, 0, 0, 0, 0, 1, 0, 0, 0,
								 0, 0, 0, 0, 0, 0, 0, 0, 0,
								 0, 0, 2, 2, 1, 2, 2, 0, 0,
								 0, 2, 2, 2, 1, 2, 2, 2, 0,
								 0, 2, 2, 2, 1, 2, 2, 2, 0, }; //terrain map

	Unit fUnits[10][3] = { Unit(1, 7, 3, 15, unitType::MELEE, unitMovementType::AERIAL), Unit(4, 6, 3, 15, unitType::RANGED, unitMovementType::GROUND), Unit(7, 7, 3, 15, unitType::MORTAR, unitMovementType::NAVAL) };
	Unit eUnits[10][3] = { Unit(1, 1, 3, 3, unitType::MELEE, unitMovementType::GROUND), Unit(4, 2, 3, 3, unitType::RANGED, unitMovementType::GROUND), Unit(7, 1, 3, 3, unitType::MORTAR, unitMovementType::NAVAL) };

	gameState level[10] = {
		gameState({9, 9}, {8, 4} ,{4, 0, 7}, {4, 8, 7}, & worldField[0][0][0], fUnits[0], eUnits[0], sizeof(fUnits[0]) / sizeof(fUnits[0][0]), sizeof(fUnits[0]) / sizeof(fUnits[0][0])),
		gameState({9, 9}, {8, 4} ,{4, 0, 7}, {4, 8, 7},& worldField[1][0][0], fUnits[0], eUnits[0], sizeof(fUnits[0]) / sizeof(fUnits[0][0]), sizeof(fUnits[0]) / sizeof(fUnits[0][0]))
	};
	
	gameState levelBackup;

	//replace all const values to dynamic values

	void moveUnits(Unit* fUnits, Unit* eUnits, coords selectedC, int fUnitsAm, int eUnitsAm) {

		if (level[lvID].getCanMove()) {

			//resets attacking before move phase
			level[lvID].clearThreatenedPlaces();
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

					if ((selectedC.x < level[lvID].getWorld().x && selectedC.x >= 0 && selectedC.y < level[lvID].getWorld().y && selectedC.y >= 0) &&
						level[lvID].getDistanceMap(selectedC.x, selectedC.y) <= 1 && !targetOccupied) {

						//-1 to getSpeed because the last square of movement gets done in the else- part of this statement
						if (fUnits[t].getMoveCycle() < fUnits[t].getSpeed() - 1) {
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
					if (level[lvID].getThreatenedPlaces(eUnits[t1].getX(), eUnits[t1].getY()) == 1) {
						eUnits[t1].takeDamage(fUnits[t].getDamage());
					}
				}
				if (level[lvID].getThreatenedPlaces(Obj->getX(), Obj->getY()) == 1) {
					Obj->takeDamage(fUnits[t].getDamage());
				}
				level[lvID].clearThreatenedPlaces();
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
	void returnThreatArray(int identifier, Unit* Unit) {
		for (int i = 0; i < level[lvID].getWorld().x; i++) {
			for (int j = 0; j < level[lvID].getWorld().y; j++) {
				//wonky code, kinda; essentially, returns a value of 0 if the field given as parameters is outside of attack range of unit, returns 1 if it's inside.
				//could be optimized with an array of coordinates with of a variable size
				level[lvID].setThreatenedPlaces(i, j, Unit[identifier].returnAttackArray(selected, i, j));
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

			if (fUnits[t].getHealth() > 0 && fUnits[t].getX() == X && fUnits[t].getY() == Y) {

				coords temp = toScreen(fUnits[t].getX(), fUnits[t].getY());

				if (fUnits[t].getHealth() > 0) {
					draw(1, 1, screen, tiles, temp.x, temp.y, 64, 128, tileSize.y, tileSize.x);
				}

				
				//gets the center of the movable-unit-circle thingy
				if (fUnits[t].getMoveState() == 1) {
					selectedUnit = fUnits[t].getCoords();
				}

				if (fUnits[t].getAttackState() == 1) {

					level[lvID].setAreAttacking(true);

					//draws cancel attack button
					screen->Bar(270, 450, 330, 500, 0x0000ff);

					//gets threatened fields for attacking units
					if (selected.x == fUnits[t].getX() && selected.y < fUnits[t].getY()) {
						level[lvID].clearThreatenedPlaces();
						returnThreatArray(t, fUnits);
					}
					else if (selected.x == fUnits[t].getX() && selected.y > fUnits[t].getY()) {
						level[lvID].clearThreatenedPlaces();
						returnThreatArray(t, fUnits);
					}
					else if (selected.x < fUnits[t].getX() && selected.y == fUnits[t].getY()) {
						level[lvID].clearThreatenedPlaces();
						returnThreatArray(t, fUnits);
					}
					else if (selected.x > fUnits[t].getX() && selected.y == fUnits[t].getY()) {
						level[lvID].clearThreatenedPlaces();
						returnThreatArray(t, fUnits);
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

	void Game::MouseDown(int button) {

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
			level[lvID].clearThreatenedPlaces();
			for (int t = 0; t < level[lvID].getFUnitsAm(); t++) {
				level[lvID].fUnits[t].setAttackState(0);
			}
		}

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

	unitMovementType getSelectedUnitMvType(coords selected) {
		for (int t = 0; t < level[lvID].getFUnitsAm(); t++) {
			if (level[lvID].fUnits[t].getCoords().equals(selected)) {
				return level[lvID].fUnits[t].getMvType();
			}
		}

		for (int t = 0; t < level[lvID].getEUnitsAm(); t++) {
			if (level[lvID].eUnits[t].getCoords().equals(selected)) {
				return level[lvID].eUnits[t].getMvType();
			}
		}

		return unitMovementType::GROUND;
	}

	void Game::KeyDown(int key) {

		switch (key) {
		case 80: if (lvID - 1 >= 0) {
						level[lvID].equals(levelBackup);
						lvID--;
					}
			break;
		case 79: if (lvID + 1 < sizeof(level) / sizeof(level[0])) {
						level[lvID].equals(levelBackup);
						lvID++;
					}
			break;
		
		}

		//resets both attack and movement for all units

		level[lvID].clearThreatenedPlaces();
		for (int i = 0; i < level[lvID].getFUnitsAm(); i++) {
			level[lvID].fUnits[i].setAttackState(0);
		};

		for (int i = 0; i < level[lvID].getEUnitsAm(); i++) {
			level[lvID].eUnits[i].setAttackState(0);
		};

		for (int i = 0; i < level[lvID].getFUnitsAm(); i++) {
			level[lvID].fUnits[i].setMoveState(0);
			level[lvID].fUnits[i].setMoveCycle(0);
		};

		for (int i = 0; i < level[lvID].getEUnitsAm(); i++) {
			level[lvID].eUnits[i].setMoveState(0);
			level[lvID].eUnits[i].setMoveCycle(0);
		};


		levelBackup.equals(level[lvID]);
	}

	void Game::Init() {
		levelBackup.equals(level[lvID]);
	}


	void Game::Shutdown() {

	}


	void AIstsep() {
		
	}


	void Game::Tick(float deltaTime)
	{
		screen->Clear(0x59d6ce);


		//activates x times per second -- 60 fps / amount of frames per interval = interval per second
		if (AItimer % 60 == 0) {
			if (turnState >= 2) {
				printf("%i\n", turnState);
			}

			AItimer == 0;
		}
		
		AItimer++;

		//draws turn button
		switch (turnState) {
		case 0: screen->Bar(770, 450, 830, 500, 0x0000ff);
			break;
		case 1: screen->Bar(770, 450, 830, 500, 0xff0000);
			break;
		}

		//marks where the units are on the field
		for (int i = 0; i < level[lvID].getFUnitsAm(); i++) {
			level[lvID].setUnitPlacement(level[lvID].fUnits[i].getX(), level[lvID].fUnits[i].getY(), 1);
		}
		for (int i = 0; i < level[lvID].getEUnitsAm(); i++) {
			level[lvID].setUnitPlacement(level[lvID].eUnits[i].getX(), level[lvID].eUnits[i].getY(), 1);
		}
		level[lvID].setUnitPlacement(level[lvID].eObjtv.getX(), level[lvID].eObjtv.getY(), 1);
		level[lvID].setUnitPlacement(level[lvID].fObjtv.getX(), level[lvID].fObjtv.getY(), 1);

		

		

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


				//calculates distance away from unit for each field
				float diffX = float(selectedUnit.x) - float(x), diffY = float(selectedUnit.y) - float(y);

				float dist = sqrt(pow(diffX, 2.0f) + pow(diffY, 2.0f));

				switch(getSelectedUnitMvType(selectedUnit)) {
				case unitMovementType::GROUND: 

					if (level[lvID].getWorldField(x, y) == 0 && level[lvID].getUnitPlacement(x, y) == 0) {

						level[lvID].setDistanceMap(x, y, dist);
					}
					else level[lvID].setDistanceMap(x, y, 20000); //20000 as an arbitrary value

					break;
				case unitMovementType::NAVAL:

					if (level[lvID].getWorldField(x, y) == 2 && level[lvID].getUnitPlacement(x, y) == 0) {

						level[lvID].setDistanceMap(x, y, dist);
					}
					else level[lvID].setDistanceMap(x, y, 20000); //20000 as an arbitrary value

					break;
				case unitMovementType::AERIAL:
					if (level[lvID].getUnitPlacement(x, y) == 0) {

						level[lvID].setDistanceMap(x, y, dist);
					}
					else level[lvID].setDistanceMap(x, y, 20000); //20000 as an arbitrary value

					break;

				default:
					level[lvID].setDistanceMap(x, y, dist);
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


				if (level[lvID].getThreatenedPlaces(x, y) == 1) {
					draw(1, 0, screen, tiles, vWorld.x, vWorld.y, 64, 128, tileSize.y, tileSize.x);
				}


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
						if (level[lvID].getDistanceMap(x, y) <= 1 && level[lvID].fUnits[t].getMoveState() == 1) {

							draw(1, 0, screen, tiles, vWorld.x, vWorld.y, 64, 128, tileSize.y, tileSize.x);
						}

						level[lvID].setUnitPlacement(x, y, 0);
					}
				}

				for (int t = 0; t < level[lvID].getEUnitsAm(); t++) {
					if (level[lvID].getDistanceMap(x, y) <= 1 && level[lvID].eUnits[t].getMoveState() == 1) {

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