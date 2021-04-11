#pragma once
#include "math.h"
#include "template.h"
#include "surface.h"
#include <stdio.h>
#include <string>
#include <string.h>
#include <fstream>

namespace Tmpl8 {

class Surface;

static Sprite antCEO(new Surface("assets/antCEO.png"), 1);
static Sprite dialogueSpr(new Surface("assets/dialogue.png"), 1);

enum class unitDirection { NORTH, EAST, SOUTH, WEST };
	
enum class unitType { MORTAR, RANGED, MELEE };

enum class unitMovementType { GROUND, NAVAL, AERIAL };

enum class levelState { COMPLETED, STARTED, INACCESSIBLE };

enum class winloss { WON, LOSS, NA };

enum class dPerson { ANT, GNRL };

const int worldsX = 11, worldsY = 11, friendlyUnitsAm = 3, enemyUnitsAm = 3, worldsAm = 4, levelsPerWorld = 6;

class Button {
public:
	int x, y, width, height;

	Button(int X, int Y, int Width, int Height) {
		x = X;
		y = Y;
		width = Width;
		height = Height;
	}

	Button() = default;

	int getX2() {
		return x + width;
	}

	int getY2() {
		return y + height;
	}
};

class LevelButton : public Button {
public:
	int type;

	LevelButton(int X, int Y, int Width, int Height, int Type) {
		x = X;
		y = Y;
		width = Width;
		height = Height;
		type = Type;
	}
};

class coords {
public:
	int x = -1, y = -1;

	coords(int X, int Y) { x = X, y = Y; };

	bool equals( coords coordsToCompare) {
		if (x == coordsToCompare.x && y == coordsToCompare.y) {
			return true;
		} else return false;
	}

	coords() = default;

	coords multiply(int multiplier) {
		return coords{
			x *= multiplier,
			y *= multiplier
		};
	}

	coords divide(int divisor) {
		return coords{
			x /= divisor,
			y /= divisor
		};
	}

	coords add(coords addend) {
		return coords{
			x += addend.x,
			y += addend.y
		};
	}

	coords subtract(coords subtrahend) {
		return coords{
			x += subtrahend.x,
			y += subtrahend.y
		};
	}

	void set(int X, int Y) {
		x = X;
		y = Y;
	}

	void se2(coords XY) {
		x = XY.x;
		y = XY.y;
	}
};

class Objective {
private: int health;
public:
	int x, y; //make health private- maybe

	Objective(int X, int Y, int Health) {
		x = X;
		y = Y;
		health = Health;
	}

	Objective() = default;

	bool isAlive() {
		if (health > 0) return true;
		else return false;
	}

	void takeDamage(int damageValue) {
		health -= damageValue;
	}

	int getHealth() {
		return health;
	}

	coords getCoords() {
		return {x, y};
	}

	void setHealth(int h) {
		health = h;
	}
};



class Unit {
private:
	int health = -1,
		speed = -1, //speed is private because it should remain unchanged, only being readable
		damageN = 0, //amount of damage unit can deal
		moveCycle = 0; //indicates how many tiles a unit has moved

		unitType attackType = unitType::MELEE;
		unitMovementType movementType = unitMovementType::GROUND;
		coords pastPlaces[5];
public: 
	int x = -1,
		y = -1,
		moveState = 0, //0 = hasn't moved, 1 = is moving, 2 = has moved
		attackState = 0; //0 = hasn't attacked, 1 = is selecting, 2 = is attacking

	unitDirection uDir = unitDirection::NORTH;

	Unit(int X, int Y, int Health, int Speed, unitType Type, unitMovementType mvType) {
		x = X;
		y = Y;
		health = Health;
		speed = Speed;
		attackType = Type;
		movementType = mvType;

		switch (attackType) {
		case unitType::MELEE: damageN = 2;
			break;
		case unitType::RANGED: damageN = 1;
			break;
		case unitType::MORTAR: damageN = 1;
			break;
		default: damageN = 1;
		}
	}

	Unit() = default;

	void set(Unit u) {
		memcpy(pastPlaces, u.pastPlaces, sizeof(pastPlaces) / sizeof(pastPlaces[0]));

		x = u.x;
		y = u.y;
		health = u.health;
		speed = u.speed;
		damageN = u.damageN;
		moveState = u.moveState;
		attackState = u.attackState;
		moveCycle = u.moveCycle;
		attackType = u.attackType;
		movementType = u.movementType;
		uDir = u.uDir;
	}

	void kill() {
		health = -1;
	}

	bool isAlive() {
		if (health > 0) return true;
		else return false;
	}

	int getHealth() {
		return health;
	}

	unitType getUnitType() {
		return attackType;
	}

	unitMovementType getMvType() {
		return movementType;
	}

	coords getPastPlaces(int index) {
		return pastPlaces[index];
	}

	int getSpeed() {
		return speed;
	}

	void setCoords(coords newCoords) {
		x = newCoords.x;
		y = newCoords.y;
	}

	coords getCoords() {
		return coords{ x, y };
	}

	void damageUnit(Unit * unitToAttack) {
		unitToAttack->takeDamage(damageN);
	}

	void damageObj(Objective * objToAttack) {
		objToAttack->takeDamage(damageN);
	}

	void takeDamage(int damageValue) {
		health -= damageValue;
	}

	void move(coords coordsToMoveTo) {

		int tempx = x - coordsToMoveTo.x;
		int tempy = y - coordsToMoveTo.y;

		if (tempx == 1 && tempy == 0) { uDir = unitDirection::WEST; }
		if (tempx == 0 && tempy == 1) { uDir = unitDirection::NORTH; }
		if (tempx == -1 && tempy == 0) { uDir = unitDirection::EAST; }
		if (tempx == 0 && tempy == -1) { uDir = unitDirection::SOUTH; }


		//code below stores array of last moves for units to go back without spending more tiles

		pastPlaces[moveCycle] = { x, y };

		int temp;
		if ((moveCycle - 1) <= 0) {
			temp = 0;
		}
		else {
			temp = moveCycle - 1;
		}

		if (coordsToMoveTo.x == pastPlaces[temp].x && coordsToMoveTo.y == pastPlaces[temp].y) {
			x = coordsToMoveTo.x;
			y = coordsToMoveTo.y;
			moveCycle--;
		}
		else if (moveCycle < speed) {
			x = coordsToMoveTo.x;
			y = coordsToMoveTo.y;
			moveCycle++;
		}

		//fills coords of array with (-1, -1), a spot outside the map
		for (int t = moveCycle; t < speed; t++) {
			pastPlaces[t] = { -1, -1 };
		}
	}

	void clearPastPlaces() {
		moveCycle = 0;
		for (int t = 0; t < sizeof(pastPlaces) / sizeof(pastPlaces[0]); t++) {
			pastPlaces[t].set(-1, -1);
		}
	}

	int returnAttackArray(coords selected, int X, int Y) { //unitLocation is used exclusively because the AI requires checking a units attack array from any position on the map

		int diffX, diffY;
		if (selected.x - x != 0) {
			diffX = (selected.x - x) / abs(selected.x - x);
		}
		else diffX = 0;
		if (selected.y - y != 0) {
			diffY = (selected.y - y) / abs(selected.y - y);
		}
		else diffY = 0;

		int startVal, endVal;

		switch (attackType) {
		case unitType::MELEE: startVal = 1; endVal = 2;
			break;
		case unitType::RANGED: startVal = 1; endVal = 4;
			break;
		case unitType::MORTAR: startVal = 4; endVal = 5;
			break;
		default: startVal = 1; endVal = 2;
		}

		//finds coordinates
		if ((x + diffX < worldsX && x + diffX >= 0 && y + diffY < worldsY && y + diffY >= 0)) {

			for (int t = startVal; t < endVal; t++) {
				if (x + diffX * t == X && y + diffY * t == Y) {
					return 1;
				}
			}
		}
		return 0;
	};
};

class GameState {
private:
	int unitPlacement[worldsX][worldsY] = { 0 }; //map of all current units
	int worldField[worldsX][worldsY] = { 0 }; //terrain map

public: 

	//---------------------IMPORTANT-NOTE---------------------//
	
	//this is all gross. 
	//std::vector would've been better. Too late to change now. 
	//not that big a problem, it just limits levels to 11x11 squares.

	//3 units max; (again, std::vector would've been better)

	Unit fUnits[friendlyUnitsAm];
	Unit eUnits[enemyUnitsAm];

	Objective eObjtv = {1, 1, 1};
	Objective fObjtv = { 1, 1, 1 };

	bool areAttacking = false;

	//initializerlist was used so we can use coords and objective in the constructor class

	GameState(std::string path) {

		//////////////////////////////////////////////////////////////////////
		//
		//	explanation;
		//	it reads 2D array from file
		//	0 = dirt, 1 = mountain,  2 = water, 3 = sand, 4 = lava
		//	if it reads anything else; namely the ascii values for 'o' or 'u'; check following values
		//	according to those, units and objectives are placed on the map. Pretty clean solution, in my opinion.
		//

		std::fstream mapFile;
		mapFile.open(path);
		char readChar;

		int fUnitCounter = 0;
		int eUnitCounter = 0;

		for (int x = 0; x < worldsX; x++) {
			for (int y = 0; y < worldsY; y++) {
				mapFile.get(readChar);
				switch (readChar) {
				case 111: //ascii code for 'o' (as in objective) is read;
					mapFile.get(readChar); //check following char
					if (readChar == 102) { //if following char is 'f'; set friendly objective on this tile
						fObjtv.x = x;
						fObjtv.y = y;
						fObjtv.setHealth(7); //set fObjective to this location
					}
					else if (readChar == 101) { //if following char is 'e'; set enemy objective on this tile
						eObjtv.x = x;
						eObjtv.y = y;
						eObjtv.setHealth(7);  //set eObjective to this location
					}
					y -= 1; // y-1 because we skip one char, so we don't read outside of array
					break;

				case 117: //ascii code for 'u' (as in unit) is read;

					mapFile.get(readChar); //check following char

					if (readChar == 102) { //if following char is 'f';

						mapFile.get(readChar);
						unitType utype;
						unitMovementType umvType;

						switch (readChar) {
						case 77: utype = unitType::MELEE; //code for 'M'
							break;
						case 109: utype = unitType::MORTAR; //code for 'm'
							break;
						case 114: utype = unitType::RANGED; //code for 'r'
							break;
						default: utype = unitType::MELEE;
						}

						mapFile.get(readChar);

						switch (readChar) {
						case 110: umvType = unitMovementType::NAVAL; //code for 'n'
							break;
						case 103:  umvType = unitMovementType::GROUND; //code for 'g'
							break;
						case 97:  umvType = unitMovementType::AERIAL; //code for 'a'
							break;
						default:  umvType = unitMovementType::GROUND; 
						}

						fUnits[fUnitCounter].set(Unit(x, y, 3, 3, utype, umvType)); //capital 'M'
						fUnitCounter++;
					}
					else if (readChar == 101) { //if following char is 'e';
						
						mapFile.get(readChar);
						unitType utype;
						unitMovementType umvType;

						switch (readChar) {
						case 77: utype = unitType::MELEE; //code for 'M'
							break;
						case 109: utype = unitType::MORTAR; //code for 'm'
							break;
						case 114: utype = unitType::RANGED; //code for 'r'
							break;
						default: utype = unitType::MELEE;
						}

						mapFile.get(readChar);

						switch (readChar) {
						case 110: umvType = unitMovementType::NAVAL; //code for 'n'
							break;
						case 103:  umvType = unitMovementType::GROUND; //code for 'g'
							break;
						case 97:  umvType = unitMovementType::AERIAL; //code for 'a'
							break;
						default:  umvType = unitMovementType::GROUND;
						}

						eUnits[eUnitCounter].set(Unit(x, y, 3, 3, utype, umvType)); //capital 'M'
						eUnitCounter++;
					}
					y -= 1; // y-1 because we skip one char, so we don't read outside of array
					break;
				default:
					worldField[x][y] = atoi(&readChar);
					mapFile.ignore();
				}
				
			}
		}

		mapFile.close();
	}

	GameState() = default;

	void set(GameState gs) {

		for (int x = 0; x < worldsX; x++) {
			for (int y = 0; y < worldsY; y++) {
				unitPlacement[x][y] = gs.getUnitPlacement(x, y);
				worldField[x][y] = gs.getWorldField(x, y);
			}
		}

		areAttacking = gs.areAttacking;

		for (int t = 0; t < friendlyUnitsAm; t++) {
			fUnits[t] = gs.fUnits[t];
		}
		for (int t = 0; t < enemyUnitsAm; t++) {
			eUnits[t] = gs.eUnits[t];
		}

		fObjtv = gs.fObjtv;
		eObjtv = gs.eObjtv;
	}

	winloss winloss() {

		if (!fObjtv.isAlive()) {
			return winloss::LOSS;
		}

		if (!eObjtv.isAlive()) {
			return winloss::WON;
		}

		bool fAlive = false;
		for (int t = 0; t < friendlyUnitsAm; t++) {
			if (fUnits[t].isAlive()) {
				fAlive = true;
			}
		}
		if (!fAlive) {
			return winloss::LOSS;
		}

		bool eAlive = false;
		for (int t = 0; t < enemyUnitsAm; t++) {
			if (eUnits[t].isAlive()) {
				eAlive = true;
			}
		}
		if (!eAlive) {
			return winloss::WON;
		}

		return winloss::NA;
	}

	bool areMoving() {
		bool areMoving = false;

		for (int t = 0; t < friendlyUnitsAm; t++) {
			if (fUnits[t].moveState == 1) {
				areMoving = true;
			}
		}

		return areMoving;
	}

	//arrays

	void setUnitPlacement(int x, int y, int val) {
		unitPlacement[x][y] = val;
	}

	int getUnitPlacement(int x, int y) {
		return unitPlacement[x][y];
	}

	void setWorldField(int x, int y, int val) {
		worldField[x][y] = val;
	}

	int getWorldField(int x, int y) {
		return worldField[x][y];
	}


	//----------//


	void setFUnits(Unit FUnits[]) {

		//this could be done way cleaner; too bad!
		for (int t = 0; t < sizeof(fUnits) / sizeof(fUnits[0]); t++) {
			fUnits[t] = Unit(0, 0, 0, 0, unitType::MELEE, unitMovementType::GROUND);
		}

		for (int t = 0; t < friendlyUnitsAm; t++) {
			fUnits[t] = FUnits[t];
		}
	}
};
/*
class sentence
{
private: 
	std::string text;
	dPerson person;
public:
	sentence(std::string Text, dPerson DP) {
		text = Text;
		person = DP;
	}

	sentence() = default;

	void show(Surface* screen) {
		antCEO.Draw(screen, 700, 30);
		dialogueSpr.Draw(screen, 50, 600);

		char* char_array;
		std::string str_obj(text);
		char_array = &str_obj[0];

		screen->Print(char_array, 70, 630, 0xffffff);
	}
};

class dialogue
{
private:
	std::vector<sentence> dlog;
	unsigned int index = 0;
public: 
	dialogue(std::vector<sentence> dialogues) {
		dlog.resize(dialogues.size());
		dlog = dialogues;
	}

	void showD(Surface* screen) {
		if (index < dlog.size()) dlog[index].show(screen);
	}

	void addToIndex() {
		if (index + 1 < dlog.size()) index++;
	}

	void nullifyIndex() {
		index = 0;
	}
};
*/
class Game
{
private:
	Surface* screen;
	int mousex, mousey;
public:
	void SetTarget( Surface* surface ) { screen = surface; }
	void Init();
	void Shutdown();
	void Tick( float deltaTime );
	void MouseUp(int button) { /* implement if you want to detect mouse button presses */ };
	void MouseDown(int button);
	void MouseMove(int x, int y) { mousex = x, mousey = y; };
	void KeyUp(int key) { /* implement if you want to handle keys */ };
	void KeyDown(int key);

	vec2 getMousePos() {
		return vec2 {
			(float)mousex, (float)mousey
		};
	};

	coords getMousePosInt() {
		return coords{
			mousex, mousey
		};
	};
};
}; // namespace Tmpl8